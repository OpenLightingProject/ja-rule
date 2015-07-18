/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * RDMResponderTest.cpp
 * Tests for the RDMResponder code.
 * Copyright (C) 2015 Simon Newton
 */

#include <gtest/gtest.h>

#include <ola/rdm/UID.h>
#include <ola/rdm/RDMCommand.h>
#include <ola/rdm/RDMEnums.h>
#include <ola/rdm/RDMCommandSerializer.h>
#include <string.h>
#include <memory>

#include "rdm.h"
#include "rdm_buffer.h"
#include "rdm_responder.h"
#include "Array.h"
#include "Matchers.h"
#include "MessageHandlerMock.h"

using ola::rdm::UID;
/*
using ola::rdm::NewDiscoveryUniqueBranchRequest;
using ola::rdm::NewMuteRequest;
using ola::rdm::NewUnMuteRequest;
using ola::rdm::RDMDiscoveryRequest;
*/
using ola::rdm::GetResponseFromData;
using ola::rdm::RDMGetRequest;
using ola::rdm::RDMRequest;
using ola::rdm::RDMResponse;
using ola::rdm::RDMSetRequest;
using std::unique_ptr;
using ::testing::StrictMock;
using ::testing::Return;

MATCHER_P(ResponseIs, expected_response, "") {
  ola::io::ByteString data;
  data.push_back(RDM_START_CODE);
  EXPECT_TRUE(ola::rdm::RDMCommandSerializer::Pack(*expected_response, &data));

  return MemoryCompare(reinterpret_cast<const uint8_t*>(std::get<0>(arg)),
                       std::get<1>(arg), data.data(), data.size(),
                       result_listener);
}

namespace {

const RDMHeader *AsHeader(const uint8_t *data) {
  return reinterpret_cast<const RDMHeader*>(data);
}

template<typename Func>
int InvokeHandler(Func function, const RDMRequest *request) {
  ola::io::ByteString data;
  data.push_back(RDM_START_CODE);
  EXPECT_TRUE(ola::rdm::RDMCommandSerializer::Pack(*request, &data));
  return function(AsHeader(data.data()), request->ParamData());
}

class MockPIDHandler {
 public:
  MOCK_METHOD4(Call, int(RDMPid pid, bool is_get, const RDMHeader *header,
                         const uint8_t *param_data));
};

MockPIDHandler *g_pid_handler = nullptr;

int GetIdentifyDevice(const RDMHeader *header,
                      const uint8_t *param_data) {
  if (g_pid_handler) {
    return g_pid_handler->Call(PID_IDENTIFY_DEVICE, true, header, param_data);
  }
  return 0;
}

int ClearSensors(const RDMHeader *header,
                      const uint8_t *param_data) {
  if (g_pid_handler) {
    return g_pid_handler->Call(PID_RECORD_SENSORS, false, header, param_data);
  }
  return 0;
}

}  // namespace

class RDMResponderTest : public testing::Test {
 public:
  RDMResponderTest()
      : m_controller_uid(0x7a70, 0x00000000),
        m_our_uid(TEST_UID) {
  }

  void SetUp() {
    g_pid_handler = &m_pid_handler;
  }

  void TearDown() {
    g_pid_handler = nullptr;
  }

  void CreateDUBParamData(const UID &lower,
                          const UID &upper,
                          uint8_t *param_data) {
    lower.Pack(param_data, UID_LENGTH);
    upper.Pack(param_data + UID_LENGTH, UID_LENGTH);
    // reset g_rdm_buffer here as well
    memset(g_rdm_buffer, 0, DUB_RESPONSE_LENGTH);
  }

  void InitDefinition(ResponderDefinition *def) {
    def->descriptors = nullptr;
    def->descriptor_count = 0;
    def->software_version_label = nullptr;
    def->manufacturer_label = nullptr;
    def->model_description = nullptr;
    def->default_device_label = nullptr;
    def->product_detail_ids = nullptr;
    g_responder.def = def;
  }

 protected:
  UID m_controller_uid;
  UID m_our_uid;
  StrictMock<MockPIDHandler> m_pid_handler;

  static const uint8_t TEST_UID[UID_LENGTH];
};

const uint8_t RDMResponderTest::TEST_UID[] = {
  0x7a, 0x70, 0x12, 0x34, 0x56, 0x78
};

TEST_F(RDMResponderTest, getUID) {
  RDMResponder_Initialize(TEST_UID);
  uint8_t uid[UID_LENGTH];
  RDMResponder_GetUID(uid);
  EXPECT_THAT(uid, MatchesUID(TEST_UID));
}

TEST_F(RDMResponderTest, DiscoveryUniqueBranch) {
  RDMResponder_Initialize(TEST_UID);

  const uint8_t expected_data[] = {
    0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xaa,
    0xfa, 0x7f, 0xfa, 0x75, 0xba, 0x57, 0xbe, 0x75,
    0xfe, 0x57, 0xfa, 0x7d, 0xaf, 0x57, 0xfa, 0xfd
  };
  ArrayTuple tuple(g_rdm_buffer, DUB_RESPONSE_LENGTH);

  uint8_t param_data[UID_LENGTH * 2];
  CreateDUBParamData(UID(0, 0), UID::AllDevices(), param_data);
  EXPECT_EQ(-DUB_RESPONSE_LENGTH,
            RDMResponder_HandleDUBRequest(param_data, arraysize(param_data)));
  EXPECT_THAT(tuple, DataIs(expected_data, arraysize(expected_data)));

  CreateDUBParamData(m_our_uid, m_our_uid, param_data);
  EXPECT_EQ(-DUB_RESPONSE_LENGTH,
            RDMResponder_HandleDUBRequest(param_data, arraysize(param_data)));
  EXPECT_THAT(tuple, DataIs(expected_data, arraysize(expected_data)));

  CreateDUBParamData(UID(m_our_uid.ManufacturerId(), 0),
                     UID::AllDevices(), param_data);
  EXPECT_EQ(-DUB_RESPONSE_LENGTH,
            RDMResponder_HandleDUBRequest(param_data, arraysize(param_data)));
  EXPECT_THAT(tuple, DataIs(expected_data, arraysize(expected_data)));

  CreateDUBParamData(UID(m_our_uid.ManufacturerId(), 0),
                     UID::VendorcastAddress(m_our_uid), param_data);
  EXPECT_EQ(-DUB_RESPONSE_LENGTH,
            RDMResponder_HandleDUBRequest(param_data, arraysize(param_data)));
  EXPECT_THAT(tuple, DataIs(expected_data, arraysize(expected_data)));

  // Check we don't respond if muted
  g_responder.is_muted = true;
  CreateDUBParamData(UID(0, 0), UID::AllDevices(), param_data);
  EXPECT_EQ(0,
            RDMResponder_HandleDUBRequest(param_data, arraysize(param_data)));
}

TEST_F(RDMResponderTest, discoveryCommands) {
  const uint8_t mute[] = {
    0xcc, 0x01, 0x18, 0x7a, 0x70, 0x12, 0x34, 0x56, 0x78, 0x7a, 0x70, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x02, 0x00
  };

  const uint8_t unmute[] = {
    0xcc, 0x01, 0x18, 0x7a, 0x70, 0x12, 0x34, 0x56, 0x78, 0x7a, 0x70, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x03, 0x00
  };

  const uint8_t dub[] = {
    0xcc, 0x01, 0x24, 0x7a, 0x70, 0x12, 0x34, 0x56, 0x78, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x01, 0x0c
  };

  const uint8_t dub_param_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x09, 0xf0
  };

  EXPECT_EQ(28, RDMResponder_HandleDiscovery(AsHeader(mute), nullptr));
  EXPECT_EQ(28,
            RDMResponder_HandleDiscovery(AsHeader(unmute), nullptr));
  EXPECT_EQ(-DUB_RESPONSE_LENGTH,
            RDMResponder_HandleDiscovery(AsHeader(dub), dub_param_data));
}

TEST_F(RDMResponderTest, setUnMute) {
  RDMResponder_Initialize(TEST_UID);

  const uint8_t unicast_unmute[] = {
    0xcc, 0x01, 0x18, 0x7a, 0x70, 0x12, 0x34, 0x56, 0x78, 0x7a, 0x70, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x03, 0x00
  };

  const uint8_t expected_response[] = {
    0xcc, 0x01, 0x1a, 0x7a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x70, 0x12,
    0x34, 0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x03, 0x02,
    0x00, 0x00, 0x03, 0xe5
  };

  g_responder.is_muted = true;
  EXPECT_EQ(28, RDMResponder_SetUnMute(AsHeader(unicast_unmute)));
  EXPECT_FALSE(g_responder.is_muted);

  ArrayTuple tuple(g_rdm_buffer, 28);
  EXPECT_THAT(tuple, DataIs(expected_response, arraysize(expected_response)));

  // Try with a broadcast
  const uint8_t broadcast_unmute[] = {
    0xcc, 0x01, 0x18, 0x7a, 0x70, 0xff, 0xff, 0xff, 0xff, 0x7a, 0x70, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x03, 0x00
  };
  g_responder.is_muted = true;
  EXPECT_EQ(0, RDMResponder_SetUnMute(AsHeader(broadcast_unmute)));
  EXPECT_FALSE(g_responder.is_muted);
}

TEST_F(RDMResponderTest, setMute) {
  const uint8_t unicast_mute[] = {
    0xcc, 0x01, 0x18, 0x7a, 0x70, 0x12, 0x34, 0x56, 0x78, 0x7a, 0x70, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x02, 0x00
  };

  const uint8_t expected_response[] = {
    0xcc, 0x01, 0x1a, 0x7a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x70, 0x12,
    0x34, 0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x02, 0x02,
    0x00, 0x00, 0x03, 0xe4
  };

  g_responder.is_muted = false;
  EXPECT_EQ(28, RDMResponder_SetMute(AsHeader(unicast_mute)));
  EXPECT_TRUE(g_responder.is_muted);

  ArrayTuple tuple(g_rdm_buffer, 28);
  EXPECT_THAT(tuple, DataIs(expected_response, arraysize(expected_response)));

  // try a broadcast
  const uint8_t broadcast_mute[] = {
    0xcc, 0x01, 0x18, 0x7a, 0x70, 0xff, 0xff, 0xff, 0xff, 0x7a, 0x70, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x02, 0x00
  };
  g_responder.is_muted = false;
  EXPECT_EQ(0, RDMResponder_SetMute(AsHeader(broadcast_mute)));
  EXPECT_TRUE(g_responder.is_muted);
}

TEST_F(RDMResponderTest, testBuildNack) {
  const uint8_t request[] = {
    0xcc, 0x01, 0x18, 0x7a, 0x70, 0x12, 0x34, 0x56, 0x78, 0x7a, 0x70, 0x00,
    0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x01, 0x20, 0x03, 0x43, 0x00,
  };

  const uint8_t expected_response[] = {
    0xcc, 0x01, 0x1a, 0x7a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x70, 0x12,
    0x34, 0x56, 0x78, 0x12, 0x02, 0x00, 0x00, 0x01, 0x21, 0x03, 0x43, 0x02,
    0x00, 0x00, 0x04, 0x4d
  };

  EXPECT_EQ(28, RDMResponder_BuildNack(AsHeader(request), NR_UNKNOWN_PID));

  ArrayTuple tuple(g_rdm_buffer, 28);
  EXPECT_THAT(tuple, DataIs(expected_response, arraysize(expected_response)));
}

TEST_F(RDMResponderTest, testDispatch) {
  const PIDDescriptor pid_descriptors[] = {
    {PID_IDENTIFY_DEVICE, GetIdentifyDevice, (PIDCommandHandler) nullptr},
    {PID_RECORD_SENSORS, (PIDCommandHandler) nullptr, ClearSensors},
  };
  ResponderDefinition responder_def;
  InitDefinition(&responder_def);
  responder_def.descriptors = pid_descriptors;
  responder_def.descriptor_count = arraysize(pid_descriptors);

  const uint8_t get_identify_device_header[] = {
    0xcc, 0x01, 0x18, 0x7a, 0x70, 0x12, 0x34, 0x56, 0x78, 0x7a, 0x70, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x10, 0x00, 0x00
  };

  EXPECT_CALL(m_pid_handler,
              Call(PID_IDENTIFY_DEVICE, true,
                   AsHeader(get_identify_device_header), nullptr))
    .WillOnce(Return(27));
  EXPECT_EQ(27, RDMResponder_DispatchPID(AsHeader(get_identify_device_header),
                                         nullptr));

  // SET PID_IDENTIFY_DEVICE (no handler)
  const uint8_t set_identify_device_header[] = {
    0xcc, 0x01, 0x18, 0x7a, 0x70, 0x12, 0x34, 0x56, 0x78, 0x7a, 0x70, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x10, 0x00, 0x00
  };

  const uint8_t unsupported_set_command[] = {
    0xcc, 0x01, 0x1a, 0x7a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x70, 0x12,
    0x34, 0x56, 0x78, 0x00, 0x02, 0x00, 0x00, 0x00, 0x31, 0x10, 0x00, 0x02,
    0x00, 0x05, 0x04, 0x19
  };

  EXPECT_EQ(28, RDMResponder_DispatchPID(AsHeader(set_identify_device_header),
                                         nullptr));
  ArrayTuple tuple(g_rdm_buffer, 28);
  EXPECT_THAT(tuple,
              DataIs(unsupported_set_command,
                     arraysize(unsupported_set_command)));

  const uint8_t get_record_sensors[] = {
    0xcc, 0x01, 0x18, 0x7a, 0x70, 0x12, 0x34, 0x56, 0x78, 0x7a, 0x70, 0x00,
    0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x01, 0x20, 0x02, 0x02, 0x00
  };

  const uint8_t unsupported_get_command[] = {
    0xcc, 0x01, 0x1a, 0x7a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x70, 0x12,
    0x34, 0x56, 0x78, 0x09, 0x02, 0x00, 0x00, 0x01, 0x21, 0x02, 0x02, 0x02,
    0x00, 0x05, 0x04, 0x07
  };

  EXPECT_EQ(28, RDMResponder_DispatchPID(AsHeader(get_record_sensors),
                                         nullptr));
  ArrayTuple tuple2(g_rdm_buffer, 28);
  EXPECT_THAT(tuple2,
              DataIs(unsupported_get_command,
                     arraysize(unsupported_get_command)));

  const uint8_t set_record_sensors[] = {
    0xcc, 0x01, 0x1a, 0x7a, 0x70, 0x12, 0x34, 0x56, 0x78, 0x7a, 0x70, 0x00,
    0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x05, 0x30, 0x02, 0x02, 0x02,
  };

  EXPECT_CALL(m_pid_handler,
              Call(PID_RECORD_SENSORS, false,
                   AsHeader(set_record_sensors), nullptr))
    .WillOnce(Return(26));
  EXPECT_EQ(26, RDMResponder_DispatchPID(AsHeader(set_record_sensors),
                                         nullptr));

  // try an unsupported PID
  const uint8_t get_device_info[] = {
    0xcc, 0x01, 0x1a, 0x7a, 0x70, 0x12, 0x34, 0x56, 0x78, 0x7a, 0x70, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x60, 0x02
  };

  const uint8_t unknown_pid[] = {
    0xcc, 0x01, 0x1a, 0x7a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x70, 0x12,
    0x34, 0x56, 0x78, 0x00, 0x02, 0x00, 0x00, 0x00, 0x21, 0x00, 0x60, 0x02,
    0x00, 0x00, 0x04, 0x54
  };

  EXPECT_EQ(28, RDMResponder_DispatchPID(AsHeader(get_device_info),
                                         nullptr));
  ArrayTuple tuple3(g_rdm_buffer, 28);
  EXPECT_THAT(tuple3, DataIs(unknown_pid, arraysize(unknown_pid)));
}

TEST_F(RDMResponderTest, supportedParameters) {
  unique_ptr<RDMRequest> request(new RDMGetRequest(
      m_controller_uid, m_our_uid, 0, 0, 0, PID_SUPPORTED_PARAMETERS,
      nullptr, 0));

  const uint8_t param_data[] = { 0x02, 0x02 };

  const PIDDescriptor pid_descriptors[] = {
    {PID_DISC_UNIQUE_BRANCH, nullptr, nullptr},
    {PID_DISC_MUTE, nullptr, nullptr},
    {PID_DISC_UN_MUTE, nullptr, nullptr},
    {PID_SUPPORTED_PARAMETERS, nullptr, nullptr},
    {PID_PARAMETER_DESCRIPTION, nullptr, nullptr},
    {PID_DEVICE_INFO, nullptr, nullptr},
    {PID_SOFTWARE_VERSION_LABEL, nullptr, nullptr},
    {PID_DMX_START_ADDRESS, nullptr, nullptr},
    {PID_IDENTIFY_DEVICE, nullptr, nullptr},
    {PID_RECORD_SENSORS, nullptr, nullptr}
  };

  ResponderDefinition responder_def;
  InitDefinition(&responder_def);
  responder_def.descriptors = pid_descriptors,
  responder_def.descriptor_count = arraysize(pid_descriptors);

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), param_data, arraysize(param_data)));

  int size = InvokeHandler(RDMResponder_GetSupportedParameters,
                           request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(RDMResponderTest, productDetailIds) {
  unique_ptr<RDMRequest> request(new RDMGetRequest(
      m_controller_uid, m_our_uid, 0, 0, 0, PID_PRODUCT_DETAIL_ID_LIST,
      nullptr, 0));

  const ProductDetailIds product_detail_id_list = {
    .ids = {PRODUCT_DETAIL_TEST, PRODUCT_DETAIL_CHANGEOVER_MANUAL},
    .size = 2
  };

  ResponderDefinition responder_def;
  InitDefinition(&responder_def);
  responder_def.product_detail_ids = &product_detail_id_list;

  const uint8_t param_data[] = {0x9, 0x02, 0x9, 0x00};
  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), param_data, arraysize(param_data)));

  int size = InvokeHandler(RDMResponder_GetProductDetailIds,
                           request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(RDMResponderTest, deviceModelDescrption) {
  unique_ptr<RDMRequest> request(new RDMGetRequest(
      m_controller_uid, m_our_uid, 0, 0, 0, PID_DEVICE_MODEL_DESCRIPTION,
      nullptr, 0));

  const char device_model[] = "Ja Rule";

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), reinterpret_cast<const uint8_t*>(device_model),
        strlen(device_model)));

  ResponderDefinition responder_def;
  InitDefinition(&responder_def);
  responder_def.model_description = device_model;

  int size = InvokeHandler(RDMResponder_GetDeviceModelDescription,
                           request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(RDMResponderTest, manufacturerLabel) {
  unique_ptr<RDMRequest> request(new RDMGetRequest(
      m_controller_uid, m_our_uid, 0, 0, 0, PID_MANUFACTURER_LABEL,
      nullptr, 0));

  const char manufacturer_label[] = "Open Lighting";

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), reinterpret_cast<const uint8_t*>(manufacturer_label),
        strlen(manufacturer_label)));

  ResponderDefinition responder_def;
  InitDefinition(&responder_def);
  responder_def.manufacturer_label = manufacturer_label;

  int size = InvokeHandler(RDMResponder_GetManufacturerLabel, request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(RDMResponderTest, softwareVersionLabel) {
  unique_ptr<RDMRequest> request(new RDMGetRequest(
      m_controller_uid, m_our_uid, 0, 0, 0, PID_SOFTWARE_VERSION_LABEL,
      nullptr, 0));

  const char sw_version_label[] = "ALpha";

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), reinterpret_cast<const uint8_t*>(sw_version_label),
        strlen(sw_version_label)));

  ResponderDefinition responder_def;
  InitDefinition(&responder_def);
  responder_def.software_version_label = sw_version_label;

  int size = InvokeHandler(RDMResponder_GetSoftwareVersionLabel, request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(RDMResponderTest, deviceLabel) {
  unique_ptr<RDMRequest> request(new RDMGetRequest(
      m_controller_uid, m_our_uid, 0, 0, 0, PID_DEVICE_LABEL,
      nullptr, 0));

  const char default_device_label[] = "Test Device";

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), reinterpret_cast<const uint8_t*>(default_device_label),
        strlen(default_device_label)));

  ResponderDefinition responder_def;
  InitDefinition(&responder_def);
  responder_def.default_device_label = default_device_label;
  RDMResponder_ResetToFactoryDefaults();

  int size = InvokeHandler(RDMResponder_GetDeviceLabel, request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  const char new_device_label[] = "New label";

  request.reset(new RDMSetRequest(
      m_controller_uid, m_our_uid, 0, 0, 0, PID_DEVICE_LABEL,
      reinterpret_cast<const uint8_t*>(new_device_label),
      strlen(new_device_label)));

  response.reset(GetResponseFromData(request.get()));
  size = InvokeHandler(RDMResponder_SetDeviceLabel, request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(RDMResponderTest, identifyDevice) {
  unique_ptr<RDMRequest> request(new RDMGetRequest(
      m_controller_uid, m_our_uid, 0, 0, 0, PID_IDENTIFY_DEVICE,
      nullptr, 0));

  uint8_t identify_mode = 0;

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), &identify_mode, sizeof(identify_mode)));

  int size = InvokeHandler(RDMResponder_GetIdentifyDevice, request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  identify_mode = 1;
  request.reset(new RDMSetRequest(
      m_controller_uid, m_our_uid, 0, 0, 0, PID_IDENTIFY_DEVICE, &identify_mode,
      sizeof(identify_mode)));

  response.reset(GetResponseFromData(request.get()));
  size = InvokeHandler(RDMResponder_SetIdentifyDevice, request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}
