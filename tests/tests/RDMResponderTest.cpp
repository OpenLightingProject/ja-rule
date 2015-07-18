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
#include <memory>

#include "rdm.h"
#include "rdm_buffer.h"
#include "rdm_responder.h"
#include "Array.h"
#include "Matchers.h"
#include "MessageHandlerMock.h"

using ola::rdm::UID;
using std::unique_ptr;
using ::testing::StrictMock;
using ::testing::Return;

namespace {

const RDMHeader *AsHeader(const uint8_t *data) {
  return reinterpret_cast<const RDMHeader*>(data);
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
      : m_controller_uid(0x7a70, 0x10000000),
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
  ResponderDefinition responder_def = {
    .descriptors = reinterpret_cast<const PIDDescriptor*>(&pid_descriptors),
    .descriptor_count = arraysize(pid_descriptors),
    .software_version_label = nullptr,
    .manufacturer_label = nullptr,
    .model_description = nullptr,
    .default_device_label = nullptr,
    .product_detail_ids = nullptr
  };
  g_responder.def = &responder_def;

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

TEST_F(RDMResponderTest, productDetailIds) {
  const uint8_t request[] = {
    0xcc, 0x01, 0x18, 0x7a, 0x70, 0x12, 0x34, 0x56, 0x78, 0x7a, 0x70, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x70, 0x00
  };

  const uint8_t expected_response[] = {
    0xcc, 0x01, 0x1c, 0x7a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x70, 0x12,
    0x34, 0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x70, 0x04,
    0x09, 0x02, 0x09, 0x00, 0x04, 0x7a
  };

  const ProductDetailIds product_detail_id_list = {
    .ids = {PRODUCT_DETAIL_TEST, PRODUCT_DETAIL_CHANGEOVER_MANUAL},
    .size = 2
  };

  ResponderDefinition responder_def = {
    .descriptors = nullptr,
    .descriptor_count = 0,
    .software_version_label = nullptr,
    .manufacturer_label = nullptr,
    .model_description = nullptr,
    .default_device_label = nullptr,
    .product_detail_ids = &product_detail_id_list
  };
  g_responder.def = &responder_def;

  EXPECT_EQ(30, RDMResponder_GetProductDetailIds(AsHeader(request), nullptr));

  ArrayTuple tuple(g_rdm_buffer, 30);
  EXPECT_THAT(tuple, DataIs(expected_response, arraysize(expected_response)));
}

TEST_F(RDMResponderTest, deviceModelDescrption) {
  const uint8_t request[] = {
    0xcc, 0x01, 0x18, 0x7a, 0x70, 0x12, 0x34, 0x56, 0x78, 0x7a, 0x70, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x80, 0x00
  };

  const uint8_t expected_response[] = {
    0xcc, 0x01, 0x1b, 0x7a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x70,
    0x12, 0x34, 0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x80,
    0x03, 'f', 'o', 'o', 0x05, 0xb8
  };

  const char device_model_description[] = "foo";

  ResponderDefinition responder_def = {
    .descriptors = nullptr,
    .descriptor_count = 0,
    .software_version_label = nullptr,
    .manufacturer_label = nullptr,
    .model_description = device_model_description,
    .default_device_label = nullptr,
    .product_detail_ids = nullptr
  };
  g_responder.def = &responder_def;

  EXPECT_EQ(29,
            RDMResponder_GetDeviceModelDescription(AsHeader(request), nullptr));

  ArrayTuple tuple(g_rdm_buffer, 29);
  EXPECT_THAT(tuple, DataIs(expected_response, arraysize(expected_response)));
}

TEST_F(RDMResponderTest, manufacturerLabel) {
  const uint8_t request[] = {
    0xcc, 0x01, 0x18, 0x7a, 0x70, 0x12, 0x34, 0x56, 0x78, 0x7a, 0x70, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x81, 0x00,
    0x04, 0x6e
  };

  const uint8_t expected_response[] = {
    0xcc, 0x01, 0x25, 0x7a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x70, 0x12,
    0x34, 0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x81, 0x0d,
    'O', 'p', 'e', 'n', ' ', 'L', 'i', 'g', 'h', 't', 'i', 'n', 'g',
    0x09, 0x71
  };

  const char manufacturer_label[] = "Open Lighting";

  ResponderDefinition responder_def = {
    .descriptors = nullptr,
    .descriptor_count = 0,
    .software_version_label = nullptr,
    .manufacturer_label = manufacturer_label,
    .model_description = nullptr,
    .default_device_label = nullptr,
    .product_detail_ids = nullptr
  };
  g_responder.def = &responder_def;

  EXPECT_EQ(39, RDMResponder_GetManufacturerLabel(AsHeader(request), nullptr));

  ArrayTuple tuple(g_rdm_buffer, 39);
  EXPECT_THAT(tuple, DataIs(expected_response, arraysize(expected_response)));
}

TEST_F(RDMResponderTest, supportedParameters) {
  const uint8_t request[] = {
    0xcc, 0x01, 0x18, 0x7a, 0x70, 0x12, 0x34, 0x56, 0x78, 0x7a, 0x70, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x50, 0x00,
    0x04, 0x3d
  };

  const uint8_t expected_response[] = {
    0xcc, 0x01, 0x1a, 0x7a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x70, 0x12,
    0x34, 0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x50, 0x02,
    0x02, 0x02, 0x04, 0x46
  };

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

  ResponderDefinition responder_def = {
    .descriptors = pid_descriptors,
    .descriptor_count = arraysize(pid_descriptors),
    .software_version_label = nullptr,
    .manufacturer_label = nullptr,
    .model_description = nullptr,
    .default_device_label = nullptr,
    .product_detail_ids = nullptr
  };
  g_responder.def = &responder_def;

  EXPECT_EQ(28,
            RDMResponder_GetSupportedParameters(AsHeader(request), nullptr));

  ArrayTuple tuple(g_rdm_buffer, 28);
  EXPECT_THAT(tuple, DataIs(expected_response, arraysize(expected_response)));
}
