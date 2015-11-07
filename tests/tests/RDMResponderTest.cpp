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
#include <ola/network/NetworkUtils.h>
#include <string.h>
#include <memory>

#include "rdm.h"
#include "rdm_buffer.h"
#include "rdm_responder.h"
#include "Array.h"
#include "Matchers.h"
#include "MessageHandlerMock.h"
#include "TestHelpers.h"

using ola::network::HostToNetwork;
using ola::rdm::UID;
using ola::rdm::GetResponseFromData;
using ola::rdm::NewDiscoveryUniqueBranchRequest;
using ola::rdm::NewMuteRequest;
using ola::rdm::NewUnMuteRequest;
using ola::rdm::RDMDiscoveryRequest;
using ola::rdm::RDMGetRequest;
using ola::rdm::RDMRequest;
using ola::rdm::RDMResponse;
using ola::rdm::RDMSetRequest;
using std::unique_ptr;
using ::testing::StrictMock;
using ::testing::Return;

namespace {

template<typename Func, typename Arg>
int InvokeGetHandler(Func function, const ola::rdm::RDMRequest *request,
                     Arg arg) {
  ola::io::ByteString data;
  data.push_back(RDM_START_CODE);
  EXPECT_TRUE(ola::rdm::RDMCommandSerializer::Pack(*request, &data));
  return function(AsHeader(data.data()), arg);
}

template<typename Func, typename Arg>
int InvokeSetHandler(Func function, const ola::rdm::RDMRequest *request,
                     Arg arg) {
  ola::io::ByteString data;
  data.push_back(RDM_START_CODE);
  EXPECT_TRUE(ola::rdm::RDMCommandSerializer::Pack(*request, &data));
  return function(AsHeader(data.data()), request->ParamData(), arg);
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

// Sensors
enum { NUMBER_OF_SENSORS = 2 };

const char SENSOR_NAME1[] = "Temperature";
const char SENSOR_NAME2[] = "Power";

const SensorDefinition SENSOR_DEFINITIONS[] = {
  {
    SENSOR_NAME1,
    1000,
    -400,
    500,
    300,
    SENSOR_SUPPORTS_RECORDING_MASK | SENSOR_SUPPORTS_LOWEST_HIGHEST_MASK,
    SENSOR_TEMPERATURE,
    UNITS_CENTIGRADE,
    PREFIX_DECI
  },
  {
    SENSOR_NAME2,
    882,  // +8G
    -686,  // -8G
    196,  // +1G
    0,  // -1G
    0u,
    SENSOR_ACCELERATION,
    UNITS_METERS_PER_SECOND_SQUARED,
    PREFIX_DECI
  }
};

SensorData g_sensors[NUMBER_OF_SENSORS];

// Slots & Personalities
enum { PERSONALITY_COUNT = 2 };

static const char SLOT_DIMMER_DESCRIPTION[] = "Dimmer";
static const char PERSONALITY_DESCRIPTION1[] = "8-bit mode";
static const char PERSONALITY_DESCRIPTION2[] = "16-bit mode";

static const SlotDefinition PERSONALITY_SLOTS1[] = {
  {
    SLOT_DIMMER_DESCRIPTION,
    SD_INTENSITY,
    ST_PRIMARY,
    0u,
  },
};

static const SlotDefinition PERSONALITY_SLOTS2[] = {
  {
    SLOT_DIMMER_DESCRIPTION,
    SD_INTENSITY,
    ST_PRIMARY,
    0u,
  },
  {
    SLOT_DIMMER_DESCRIPTION,
    0u,
    ST_SEC_FINE,
    0u,
  },
};

static const PersonalityDefinition PERSONALITIES[PERSONALITY_COUNT] = {
  {
    1u,
    PERSONALITY_DESCRIPTION1,
    PERSONALITY_SLOTS1,
    1u
  },
  {
    2u,
    PERSONALITY_DESCRIPTION2,
    PERSONALITY_SLOTS2,
    2u
  }
};

const char PIXEL_TYPE_STRING[] = "Type";

const ParameterDescription PIXEL_TYPE_DESCRIPTION = {
  .pdl_size = 2u,
  .data_type = DS_UNSIGNED_WORD,
  .command_class = CC_GET_SET,
  .unit = UNITS_NONE,
  .prefix = PREFIX_NONE,
  .min_valid_value = 0u,
  .max_valid_value = 1u,
  .default_value = 0u,
  .description = PIXEL_TYPE_STRING,
};

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
    def->sensors = SENSOR_DEFINITIONS;
    def->sensor_count = NUMBER_OF_SENSORS;
    def->personalities = PERSONALITIES;
    def->personality_count = PERSONALITY_COUNT;

    g_responder->sensors = g_sensors;

    for (unsigned int i = 0; i < NUMBER_OF_SENSORS; i++) {
      g_sensors[i].present_value = 0u;
      g_sensors[i].lowest_value = 0u;
      g_sensors[i].highest_value = 0u;
      g_sensors[i].recorded_value = 0u;
      g_sensors[i].should_nack = false;
    }

    g_responder->def = def;
  }

  void InitResponder() {
    RDMResponderSettings settings;
    memcpy(settings.uid, TEST_UID, UID_LENGTH);
    RDMResponder_Initialize(&settings);
  }

 protected:
  UID m_controller_uid;
  UID m_our_uid;
  StrictMock<MockPIDHandler> m_pid_handler;

  unique_ptr<RDMRequest> BuildGetRequest(uint16_t pid,
                                         const uint8_t *param_data = nullptr,
                                         unsigned int param_data_size = 0) {
    return unique_ptr<RDMGetRequest>(new RDMGetRequest(
        m_controller_uid, m_our_uid, 0, 0, 0, pid, param_data,
        param_data_size));
  }

  unique_ptr<RDMRequest> BuildSetRequest(uint16_t pid,
                                         const uint8_t *param_data = nullptr,
                                         unsigned int param_data_size = 0) {
    return unique_ptr<RDMSetRequest>(new RDMSetRequest(
        m_controller_uid, m_our_uid, 0, 0, 0, pid, param_data,
        param_data_size));
  }

  static const uint8_t TEST_UID[UID_LENGTH];
};

const uint8_t RDMResponderTest::TEST_UID[] = {
  0x7a, 0x70, 0x12, 0x34, 0x56, 0x78
};

TEST_F(RDMResponderTest, getUID) {
  InitResponder();
  uint8_t uid[UID_LENGTH];
  RDMResponder_GetUID(uid);
  EXPECT_THAT(uid, MatchesUID(TEST_UID));
}

TEST_F(RDMResponderTest, DiscoveryUniqueBranch) {
  InitResponder();

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
  g_responder->is_muted = true;
  CreateDUBParamData(UID(0, 0), UID::AllDevices(), param_data);
  EXPECT_EQ(0,
            RDMResponder_HandleDUBRequest(param_data, arraysize(param_data)));
}

TEST_F(RDMResponderTest, discoveryCommands) {
  unique_ptr<RDMDiscoveryRequest> unmute(NewUnMuteRequest(
      m_controller_uid, m_our_uid, 0));
  unique_ptr<RDMDiscoveryRequest> discovery(NewDiscoveryUniqueBranchRequest(
      m_controller_uid, UID(0, 0), UID::AllDevices(), 0));
  unique_ptr<RDMDiscoveryRequest> mute(NewMuteRequest(
      m_controller_uid, m_our_uid, 0));

  EXPECT_EQ(28, InvokeHandler(RDMResponder_HandleDiscovery, unmute.get()));
  EXPECT_EQ(-DUB_RESPONSE_LENGTH,
            InvokeHandler(RDMResponder_HandleDiscovery, discovery.get()));
  EXPECT_EQ(28, InvokeHandler(RDMResponder_HandleDiscovery, mute.get()));
}

TEST_F(RDMResponderTest, setUnMute) {
  InitResponder();

  unique_ptr<RDMDiscoveryRequest> unicast_unmute(NewUnMuteRequest(
      m_controller_uid, m_our_uid, 0));

  uint8_t control_bits[2] = {0, 0};
  unique_ptr<RDMResponse> response(GetResponseFromData(
        unicast_unmute.get(), control_bits, arraysize(control_bits)));

  g_responder->is_muted = true;
  int size = InvokeMuteHandler(RDMResponder_SetUnMute, unicast_unmute.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
  EXPECT_FALSE(g_responder->is_muted);

  // Try with a broadcast
  unique_ptr<RDMDiscoveryRequest> broadcast_unmute(NewUnMuteRequest(
      m_controller_uid, UID::AllDevices(), 0));
  g_responder->is_muted = true;
  EXPECT_EQ(0,
            InvokeMuteHandler(RDMResponder_SetUnMute, broadcast_unmute.get()));
  EXPECT_FALSE(g_responder->is_muted);
}

TEST_F(RDMResponderTest, setMute) {
  InitResponder();

  unique_ptr<RDMDiscoveryRequest> unicast_mute(NewMuteRequest(
      m_controller_uid, m_our_uid, 0));

  uint8_t control_bits[2] = {0, 0};
  unique_ptr<RDMResponse> response(GetResponseFromData(
        unicast_mute.get(), control_bits, arraysize(control_bits)));

  g_responder->is_muted = false;
  int size = InvokeMuteHandler(RDMResponder_SetMute, unicast_mute.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
  EXPECT_TRUE(g_responder->is_muted);

  // Try with a broadcast
  unique_ptr<RDMDiscoveryRequest> broadcast_mute(NewMuteRequest(
      m_controller_uid, UID::AllDevices(), 0));
  g_responder->is_muted = false;
  EXPECT_EQ(0, InvokeMuteHandler(RDMResponder_SetMute, broadcast_mute.get()));
  EXPECT_TRUE(g_responder->is_muted);
}

TEST_F(RDMResponderTest, testBuildNack) {
  unique_ptr<RDMRequest> request(new RDMGetRequest(
      m_controller_uid, m_our_uid, 0, 0, 0, PID_SUPPORTED_PARAMETERS,
      nullptr, 0));

  unique_ptr<RDMResponse> response(
      ola::rdm::NackWithReason(request.get(), ola::rdm::NR_UNKNOWN_PID));

  ola::io::ByteString data;
  data.push_back(RDM_START_CODE);
  EXPECT_TRUE(ola::rdm::RDMCommandSerializer::Pack(*request, &data));
  int size = RDMResponder_BuildNack(AsHeader(data.data()), NR_UNKNOWN_PID);
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(RDMResponderTest, testDispatch) {
  const PIDDescriptor pid_descriptors[] = {
    {PID_IDENTIFY_DEVICE, GetIdentifyDevice, 0, (PIDCommandHandler) nullptr},
    {PID_RECORD_SENSORS, (PIDCommandHandler) nullptr, 0, ClearSensors},
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
    {PID_DISC_UNIQUE_BRANCH, nullptr, 0, nullptr},
    {PID_DISC_MUTE, nullptr, 0, nullptr},
    {PID_DISC_UN_MUTE, nullptr, 0, nullptr},
    {PID_SUPPORTED_PARAMETERS, nullptr, 0, nullptr},
    {PID_PARAMETER_DESCRIPTION, nullptr, 0, nullptr},
    {PID_DEVICE_INFO, nullptr, 0, nullptr},
    {PID_SOFTWARE_VERSION_LABEL, nullptr, 0, nullptr},
    {PID_DMX_START_ADDRESS, nullptr, 0, nullptr},
    {PID_IDENTIFY_DEVICE, nullptr, 0, nullptr},
    {PID_RECORD_SENSORS, nullptr, 0, nullptr}
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

TEST_F(RDMResponderTest, deviceModelDescription) {
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

  const char sw_version_label[] = "Alpha";

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

  identify_mode = 0;
  request.reset(new RDMSetRequest(
      m_controller_uid, m_our_uid, 0, 0, 0, PID_IDENTIFY_DEVICE, &identify_mode,
      sizeof(identify_mode)));

  response.reset(GetResponseFromData(request.get()));
  size = InvokeHandler(RDMResponder_SetIdentifyDevice, request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(RDMResponderTest, commsStatus) {
  uint8_t sensor = 0;
  unique_ptr<RDMRequest> request = BuildGetRequest(
      PID_COMMS_STATUS, &sensor, sizeof(sensor));

  const uint8_t expected_response[] = {
    0, 0, 0, 0, 0, 0
  };

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(),
        expected_response,
        arraysize(expected_response)));

  ResponderDefinition responder_def;
  InitDefinition(&responder_def);

  int size = InvokeHandler(RDMResponder_GetCommsStatus, request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Try a set
  request = BuildSetRequest(PID_COMMS_STATUS);

  response.reset(GetResponseFromData(request.get()));

  size = InvokeHandler(RDMResponder_SetCommsStatus, request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(RDMResponderTest, bootSoftwareVersion) {
  unique_ptr<RDMRequest> request = BuildGetRequest(
      PID_BOOT_SOFTWARE_VERSION_ID);

  const uint8_t expected_response[] = {
    0, 0, 0, 1,
  };

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(),
        expected_response,
        arraysize(expected_response)));

  ResponderDefinition responder_def;
  InitDefinition(&responder_def);

  int size = InvokeHandler(RDMResponder_GetBootSoftwareVersion, request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(RDMResponderTest, bootSoftwareVersionLabel) {
  unique_ptr<RDMRequest> request = BuildGetRequest(
      PID_BOOT_SOFTWARE_VERSION_LABEL);

  const uint8_t expected_response[] = {
    '0', '.', '0', '.', '1'
  };

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(),
        expected_response,
        arraysize(expected_response)));

  ResponderDefinition responder_def;
  InitDefinition(&responder_def);

  int size = InvokeHandler(RDMResponder_GetBootSoftwareVersionLabel,
                           request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(RDMResponderTest, dmxPersonality) {
  unique_ptr<RDMRequest> request = BuildGetRequest(PID_DMX_PERSONALITY);

  const uint8_t expected_response[] = {
    1, 2
  };

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(),
        expected_response,
        arraysize(expected_response)));

  ResponderDefinition responder_def;
  InitDefinition(&responder_def);

  int size = InvokeHandler(RDMResponder_GetDMXPersonality,
                           request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Set
  uint8_t new_personality = 2;
  request = BuildSetRequest(
      PID_DMX_PERSONALITY, &new_personality, sizeof(new_personality));

  response.reset(GetResponseFromData(request.get()));

  size = InvokeHandler(RDMResponder_SetDMXPersonality, request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(RDMResponderTest, dmxPersonalityDescription) {
  uint8_t personality = 1;
  unique_ptr<RDMRequest> request = BuildGetRequest(
      PID_DMX_PERSONALITY, &personality, sizeof(personality));

  const uint8_t expected_response[] = "\001\000\0018-bit mode";

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(),
        expected_response,
        arraysize(expected_response) - 1));

  ResponderDefinition responder_def;
  InitDefinition(&responder_def);

  int size = InvokeHandler(RDMResponder_GetDMXPersonalityDescription,
                           request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(RDMResponderTest, dmxStartAddress) {
  unique_ptr<RDMRequest> request = BuildGetRequest(PID_DMX_START_ADDRESS);

  const uint8_t expected_response[] = { 0, 1 };

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(),
        expected_response,
        arraysize(expected_response)));

  ResponderDefinition responder_def;
  InitDefinition(&responder_def);

  int size = InvokeHandler(RDMResponder_GetDMXStartAddress,
                           request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Set
  uint8_t request_data[] = { 0, 10 };
  request = BuildSetRequest(
      PID_DMX_START_ADDRESS, request_data, arraysize(request_data));

  response.reset(GetResponseFromData(request.get()));

  size = InvokeHandler(RDMResponder_SetDMXStartAddress, request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(RDMResponderTest, slotInfo) {
  unique_ptr<RDMRequest> request = BuildGetRequest(PID_SLOT_INFO);

  const uint8_t expected_response[] = {
    0, 0, 0, 0, 1, 0, 1, 1, 0, 0
  };

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(),
        expected_response,
        arraysize(expected_response)));

  ResponderDefinition responder_def;
  InitDefinition(&responder_def);

  int size = InvokeHandler(RDMResponder_GetSlotInfo,
                           request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(RDMResponderTest, slotDescription) {
  uint8_t request_data[] = { 0, 0 };
  unique_ptr<RDMRequest> request = BuildGetRequest(
      PID_SLOT_DESCRIPTION,
      request_data,
      arraysize(request_data));

  const uint8_t expected_response[] = "\000\000Dimmer";

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(),
        expected_response,
        arraysize(expected_response) - 1));

  ResponderDefinition responder_def;
  InitDefinition(&responder_def);

  int size = InvokeHandler(RDMResponder_GetSlotDescription,
                           request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(RDMResponderTest, slotDefaultValue) {
  uint8_t request_data[] = { 0, 0 };
  unique_ptr<RDMRequest> request = BuildGetRequest(
      PID_DEFAULT_SLOT_VALUE,
      request_data,
      arraysize(request_data));

  const uint8_t expected_response[] = {
    0, 0, 0,
    0, 1, 0,
  };

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(),
        expected_response,
        arraysize(expected_response)));

  ResponderDefinition responder_def;
  InitDefinition(&responder_def);

  int size = InvokeHandler(RDMResponder_GetDefaultSlotValue,
                           request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

// Generic Tests
TEST_F(RDMResponderTest, testGenericUInt8) {
  unique_ptr<RDMRequest> request(new RDMGetRequest(
      m_controller_uid, m_our_uid, 0, 0, 0, PID_DISPLAY_LEVEL,
      nullptr, 0));

  uint8_t level = 78;

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), &level, sizeof(level)));

  int size = InvokeGetHandler(RDMResponder_GenericGetUInt8, request.get(),
                              level);
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  level = 100;
  request.reset(new RDMSetRequest(
      m_controller_uid, m_our_uid, 0, 0, 0, PID_DISPLAY_LEVEL, &level,
      sizeof(level)));

  response.reset(GetResponseFromData(request.get()));
  uint8_t new_level = 0;
  size = InvokeSetHandler(RDMResponder_GenericSetUInt8, request.get(),
                          &new_level);
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
  EXPECT_EQ(level, new_level);
}

TEST_F(RDMResponderTest, testGenericUInt32) {
  unique_ptr<RDMRequest> request(new RDMGetRequest(
      m_controller_uid, m_our_uid, 0, 0, 0, PID_LAMP_HOURS,
      nullptr, 0));

  uint32_t level = 12345;
  uint32_t network_order_level = HostToNetwork(level);

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), reinterpret_cast<const uint8_t*>(&network_order_level),
        sizeof(network_order_level)));

  int size = InvokeGetHandler(RDMResponder_GenericGetUInt32, request.get(),
                              level);
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  level = 1000000;
  network_order_level = HostToNetwork(level);
  request.reset(new RDMSetRequest(
      m_controller_uid, m_our_uid, 0, 0, 0, PID_LAMP_HOURS,
      reinterpret_cast<const uint8_t*>(&network_order_level),
      sizeof(network_order_level)));

  response.reset(GetResponseFromData(request.get()));
  uint32_t new_level = 0;
  size = InvokeSetHandler(RDMResponder_GenericSetUInt32, request.get(),
                          &new_level);
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
  EXPECT_EQ(level, new_level);
}

// Sensor tests
TEST_F(RDMResponderTest, sensorDefinition) {
  uint8_t sensor = 0;
  unique_ptr<RDMRequest> request = BuildGetRequest(
      PID_SENSOR_DEFINITION, &sensor, sizeof(sensor));

  const uint8_t expected_response[] = {
    0, 0, 1, 1,
    0xfe, 0x70, 3, 0xe8, 1, 0x2c, 1, 0xf4, 3,
    'T', 'e', 'm', 'p', 'e', 'r', 'a', 't', 'u', 'r', 'e'
  };

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(),
        expected_response,
        arraysize(expected_response)));

  ResponderDefinition responder_def;
  InitDefinition(&responder_def);

  int size = InvokeHandler(RDMResponder_GetSensorDefinition, request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(RDMResponderTest, sensorValue) {
  uint8_t sensor = 0;
  unique_ptr<RDMRequest> request = BuildGetRequest(
      PID_SENSOR_VALUE, &sensor, sizeof(sensor));

  uint8_t expected_response[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0
  };

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(),
        expected_response,
        arraysize(expected_response)));

  ResponderDefinition responder_def;
  InitDefinition(&responder_def);

  int size = InvokeHandler(RDMResponder_GetSensorValue, request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Try a set
  request = BuildSetRequest(PID_SENSOR_VALUE, &sensor, sizeof(sensor));

  response.reset(GetResponseFromData(
        request.get(),
        expected_response,
        arraysize(expected_response)));

  size = InvokeHandler(RDMResponder_SetSensorValue, request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  sensor = 1;
  expected_response[0] = 1;
  request = BuildSetRequest(PID_SENSOR_DEFINITION, &sensor, sizeof(sensor));

  response.reset(GetResponseFromData(
        request.get(),
        expected_response,
        arraysize(expected_response)));

  size = InvokeHandler(RDMResponder_SetSensorValue, request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  sensor = 0xff;
  expected_response[0] = 0;
  request = BuildSetRequest(PID_SENSOR_DEFINITION, &sensor, sizeof(sensor));

  response.reset(GetResponseFromData(
        request.get(),
        expected_response,
        arraysize(expected_response)));

  size = InvokeHandler(RDMResponder_SetSensorValue, request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(RDMResponderTest, recordSensor) {
  uint8_t sensor = 0;
  unique_ptr<RDMRequest> request = BuildSetRequest(
      PID_RECORD_SENSORS, &sensor, sizeof(sensor));

  unique_ptr<RDMResponse> response(GetResponseFromData(request.get()));

  ResponderDefinition responder_def;
  InitDefinition(&responder_def);

  int size = InvokeHandler(RDMResponder_SetRecordSensor, request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  sensor = 0xff;
  request = BuildSetRequest(PID_RECORD_SENSORS, &sensor, sizeof(sensor));

  response.reset(GetResponseFromData(request.get()));

  size = InvokeHandler(RDMResponder_SetRecordSensor, request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

// Other tests
TEST_F(RDMResponderTest, ioctl) {
  uint8_t uid[UID_LENGTH];

  EXPECT_EQ(0, RDMResponder_Ioctl(IOCTL_GET_UID, uid, 0));
  EXPECT_EQ(1, RDMResponder_Ioctl(IOCTL_GET_UID, uid, arraysize(uid)));

  EXPECT_EQ(0, memcmp(TEST_UID, uid, UID_LENGTH));
}

TEST_F(RDMResponderTest, paramDescription) {
  uint16_t param_id = 0x8000;
  unique_ptr<RDMRequest> request = BuildGetRequest(
      PID_PARAMETER_DESCRIPTION,
      reinterpret_cast<uint8_t*>(&param_id),
      sizeof(param_id));

  ola::io::ByteString data;
  data.push_back(RDM_START_CODE);
  EXPECT_TRUE(ola::rdm::RDMCommandSerializer::Pack(*request, &data));

  const uint8_t expected_response[] = {
    0x80, 0,
    2, 5, 3, 0, 0, 0,
    0, 0, 0, 0,  // min
    0, 0, 0, 1,  // max
    0, 0, 0, 0,  // default
    'T', 'y', 'p', 'e'
  };

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(),
        expected_response,
        arraysize(expected_response)));

  int size = RDMResponder_BuildParamDescription(
      AsHeader(data.data()),
      param_id,
      &PIXEL_TYPE_DESCRIPTION);
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}
