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
 * DimmerModelTest.cpp
 * Tests for the Dimmer Model RDM responder.
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

#include "dimmer_model.h"
#include "rdm.h"
#include "rdm_buffer.h"
#include "rdm_responder.h"
#include "Array.h"
#include "Matchers.h"
#include "ModelTest.h"
#include "TestHelpers.h"

using ola::network::HostToNetwork;
using ola::rdm::UID;
using ola::rdm::GetResponseFromData;
using ola::rdm::NackWithReason;
using ola::rdm::RDMRequest;
using ola::rdm::RDMResponse;
using ola::rdm::RDMSetRequest;
using std::unique_ptr;

class DimmerModelTest : public ModelTest {
 public:
  DimmerModelTest() : ModelTest(&DIMMER_MODEL_ENTRY) {}

  void SetUp() {
    RDMResponderSettings settings;
    memcpy(settings.uid, TEST_UID, UID_LENGTH);
    RDMResponder_Initialize(&settings);
    DimmerModel_Initialize();
    DIMMER_MODEL_ENTRY.activate_fn();
  }
};

TEST_F(DimmerModelTest, testLifecycle) {
  EXPECT_EQ(DIMMER_MODEL_ID, DIMMER_MODEL_ENTRY.model_id);
  DIMMER_MODEL_ENTRY.tasks_fn();
  DIMMER_MODEL_ENTRY.deactivate_fn();
}

TEST_F(DimmerModelTest, dmxBlockAddress) {
  unique_ptr<RDMRequest> request = BuildGetRequest(PID_DMX_BLOCK_ADDRESS);

  // First request should be contiguous, starting at 1.
  const uint8_t expected_response[] = {
    0x00, 0x04, 0x00, 0x01
  };

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), expected_response, arraysize(expected_response)));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Set a sub-device to a different start address
  uint16_t start_address = HostToNetwork(static_cast<uint16_t>(3));
  request.reset(new RDMSetRequest(
      m_controller_uid, m_our_uid, 0, 0, 3, PID_DMX_START_ADDRESS,
      reinterpret_cast<const uint8_t*>(&start_address),
      sizeof(start_address)));

  response.reset(GetResponseFromData(request.get()));
  size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Try a GET again
  request = BuildGetRequest(PID_DMX_BLOCK_ADDRESS);

  const uint8_t expected_response2[] = {
    0x00, 0x04, 0xff, 0xff
  };

  response.reset(GetResponseFromData(
        request.get(), expected_response2, arraysize(expected_response2)));

  size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Set to a block address
  start_address = HostToNetwork(static_cast<uint16_t>(90));
  request = BuildSetRequest(
      PID_DMX_BLOCK_ADDRESS,
      reinterpret_cast<const uint8_t*>(&start_address),
      sizeof(start_address));
  response.reset(GetResponseFromData(request.get()));

  size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Now confirm with a GET
  request = BuildGetRequest(PID_DMX_BLOCK_ADDRESS);

  const uint8_t expected_response3[] = {
    0x00, 0x04, 0x00, 0x5a
  };

  response.reset(GetResponseFromData(
        request.get(), expected_response3, arraysize(expected_response3)));

  size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Check trying to set the block address out of range fails.
  start_address = HostToNetwork(static_cast<uint16_t>(510));
  request = BuildSetRequest(
      PID_DMX_BLOCK_ADDRESS,
      reinterpret_cast<const uint8_t*>(&start_address),
      sizeof(start_address));
  response.reset(NackWithReason(request.get(), ola::rdm::NR_DATA_OUT_OF_RANGE));

  size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, statusMessage) {
  uint8_t status_type = 0x02;
  unique_ptr<RDMRequest> request = BuildGetRequest(
      PID_STATUS_MESSAGES, &status_type, sizeof(status_type));

  unique_ptr<RDMResponse> response(GetResponseFromData(request.get()));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, statusIDDescription) {
  uint16_t status_id = HostToNetwork(static_cast<uint16_t>(STS_OLP_TESTING));
  unique_ptr<RDMRequest> request = BuildGetRequest(
      PID_STATUS_ID_DESCRIPTION,
      reinterpret_cast<const uint8_t*>(&status_id),
      sizeof(status_id));

  const uint8_t expected_response[] = "Counter cycle %d.%d";

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), expected_response, arraysize(expected_response) - 1));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, selfTest) {
  unique_ptr<RDMRequest> get_request = BuildGetRequest(PID_PERFORM_SELFTEST);

  uint8_t selftest = 0;

  unique_ptr<RDMResponse> response(GetResponseFromData(
        get_request.get(), &selftest, sizeof(selftest)));

  int size = InvokeRDMHandler(get_request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Now enable it
  selftest = 1;
  unique_ptr<RDMRequest> set_request = BuildSetRequest(
      PID_PERFORM_SELFTEST,
      reinterpret_cast<const uint8_t*>(&selftest),
      sizeof(selftest));

  response.reset(GetResponseFromData(set_request.get()));
  size = InvokeRDMHandler(set_request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Confirm self test is on
  response.reset(GetResponseFromData(
        get_request.get(), &selftest, sizeof(selftest)));

  size = InvokeRDMHandler(get_request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, selfTestDescription) {
  uint8_t test_id = 1;
  unique_ptr<RDMRequest> request = BuildGetRequest(PID_SELF_TEST_DESCRIPTION,
                                                   &test_id, sizeof(test_id));

  const uint8_t expected_response[] = "\001Quick test";

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), expected_response, arraysize(expected_response) - 1));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, capturePreset) {
  const uint8_t set_data[] = {
    0, 2, 0, 0, 0, 0, 0, 0
  };

  unique_ptr<RDMRequest> request = BuildSetRequest(
      PID_CAPTURE_PRESET,
      reinterpret_cast<const uint8_t*>(set_data),
      arraysize(set_data));

  unique_ptr<RDMResponse> response(GetResponseFromData(request.get()));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, presetPlayback) {
  unique_ptr<RDMRequest> request = BuildGetRequest(PID_PRESET_PLAYBACK);

  const uint8_t expected_response[] = { 0, 0, 0 };

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), expected_response, arraysize(expected_response)));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Try a set
  const uint8_t set_data[] = { 0, 2, 0xff };
  request = BuildSetRequest(
        PID_PRESET_PLAYBACK,
        reinterpret_cast<const uint8_t*>(set_data),
        arraysize(set_data));

  response.reset(GetResponseFromData(request.get()));
  size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, failMode) {
  unique_ptr<RDMRequest> request = BuildGetRequest(PID_DMX_FAIL_MODE);

  const uint8_t expected_response[] = { 0, 0, 0, 0, 0, 0, 0 };

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), expected_response, arraysize(expected_response)));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Try a set
  const uint8_t set_data[] = { 0, 2, 0, 0, 0, 0, 0};
  request = BuildSetRequest(
        PID_DMX_FAIL_MODE,
        reinterpret_cast<const uint8_t*>(set_data),
        arraysize(set_data));

  response.reset(GetResponseFromData(request.get()));
  size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, startupMode) {
  unique_ptr<RDMRequest> request = BuildGetRequest(PID_DMX_STARTUP_MODE);

  const uint8_t expected_response[] = { 0, 0, 0, 0, 0, 0, 0 };

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), expected_response, arraysize(expected_response)));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Try a set
  const uint8_t set_data[] = { 0, 2, 0, 0, 0, 0, 0};
  request = BuildSetRequest(
        PID_DMX_STARTUP_MODE,
        reinterpret_cast<const uint8_t*>(set_data),
        arraysize(set_data));

  response.reset(GetResponseFromData(request.get()));
  size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, lockPin) {
  unique_ptr<RDMRequest> request = BuildGetRequest(PID_LOCK_PIN);

  const uint8_t expected_response[] = { 0, 0 };

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), expected_response, arraysize(expected_response)));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Try a set
  const uint8_t set_data[] = { 0, 1, 0, 0 };
  request = BuildSetRequest(
        PID_LOCK_PIN,
        reinterpret_cast<const uint8_t*>(set_data),
        arraysize(set_data));

  response.reset(GetResponseFromData(request.get()));
  size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, lockState) {
  unique_ptr<RDMRequest> request = BuildGetRequest(PID_LOCK_STATE);

  const uint8_t expected_response[] = { 0, 2 };

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), expected_response, arraysize(expected_response)));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Try a set
  const uint8_t set_data[] = { 0, 0, 1};
  request = BuildSetRequest(
        PID_LOCK_STATE,
        reinterpret_cast<const uint8_t*>(set_data),
        arraysize(set_data));

  response.reset(GetResponseFromData(request.get()));
  size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, lockStateDescription) {
  uint8_t lock_state = 1;
  unique_ptr<RDMRequest> request = BuildGetRequest(
      PID_LOCK_STATE_DESCRIPTION,
      &lock_state, sizeof(lock_state));

  const uint8_t expected_response[] = "\001Subdevices locked";

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), expected_response, arraysize(expected_response) - 1));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, presetInfo) {
  unique_ptr<RDMRequest> request = BuildGetRequest(PID_PRESET_INFO);

  const uint8_t expected_response[] = {
    1, 1, 1, 1, 1, 1, 0, 3,
    0, 0, 0xff, 0xfe, 0, 0, 0xff, 0xfe,
    0, 0, 0xff, 0xfe, 0, 0, 0xff, 0xfe,
    0, 0, 0xff, 0xfe, 0, 0, 0xff, 0xfe,
  };

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), expected_response, arraysize(expected_response)));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, presetStatus) {
  const uint8_t get_data[] = { 0, 1 };
  unique_ptr<RDMRequest> request = BuildGetRequest(
      PID_PRESET_STATUS,
      reinterpret_cast<const uint8_t*>(get_data),
      arraysize(get_data));

  const uint8_t expected_response[] = {
    0, 1, 0, 0, 0, 0, 0, 0, 2
  };

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), expected_response, arraysize(expected_response)));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Try a set
  const uint8_t set_data[] = {
    0, 2, 0, 0, 0, 0, 0, 0, 0
  };
  request = BuildSetRequest(
        PID_PRESET_STATUS,
        reinterpret_cast<const uint8_t*>(set_data),
        arraysize(set_data));

  response.reset(GetResponseFromData(request.get()));
  size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Try a set with a clear
  const uint8_t set_data2[] = {
    0, 2, 0, 0, 0, 0, 0, 0, 1
  };
  request = BuildSetRequest(
        PID_PRESET_STATUS,
        reinterpret_cast<const uint8_t*>(set_data2),
        arraysize(set_data2));

  response.reset(GetResponseFromData(request.get()));
  size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, mergeMode) {
  unique_ptr<RDMRequest> request = BuildGetRequest(PID_PRESET_MERGEMODE);

  const uint8_t expected_response = 0;
  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), &expected_response, sizeof(expected_response)));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Try a set
  const uint8_t set_data = 1;
  request = BuildSetRequest(PID_PRESET_MERGEMODE, &set_data, sizeof(set_data));

  response.reset(GetResponseFromData(request.get()));
  size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, powerOnSelfTest) {
  unique_ptr<RDMRequest> request = BuildGetRequest(PID_POWER_ON_SELF_TEST);

  const uint8_t expected_response = 0;
  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), &expected_response, sizeof(expected_response)));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Try a set
  const uint8_t set_data = 1;
  request = BuildSetRequest(PID_POWER_ON_SELF_TEST, &set_data,
                            sizeof(set_data));

  response.reset(GetResponseFromData(request.get()));
  size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, clearStatusId) {
  unique_ptr<RDMRequest> request = BuildSubDeviceSetRequest(
      PID_CLEAR_STATUS_ID, 1);
  unique_ptr<RDMResponse> response(GetResponseFromData(request.get()));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, subDeviceReportingThreshold) {
  unique_ptr<RDMRequest> request = BuildSubDeviceGetRequest(
      PID_SUB_DEVICE_STATUS_REPORT_THRESHOLD, 1);

  const uint8_t expected_response = 2;
  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), &expected_response, sizeof(expected_response)));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Try a set
  const uint8_t set_data = 3;
  request = BuildSubDeviceSetRequest(PID_SUB_DEVICE_STATUS_REPORT_THRESHOLD,
                                     1, &set_data, sizeof(set_data));

  response.reset(GetResponseFromData(request.get()));
  size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, identifyMode) {
  unique_ptr<RDMRequest> request = BuildSubDeviceGetRequest(
      PID_IDENTIFY_MODE, 1);

  const uint8_t expected_response = 0;
  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), &expected_response, sizeof(expected_response)));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Try a set
  const uint8_t set_data = 0xff;
  request = BuildSubDeviceSetRequest(PID_IDENTIFY_MODE,
                                     1, &set_data, sizeof(set_data));

  response.reset(GetResponseFromData(request.get()));
  size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, burnIn) {
  unique_ptr<RDMRequest> request = BuildSubDeviceGetRequest(
      PID_BURN_IN, 1);

  const uint8_t expected_response = 0;
  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), &expected_response, sizeof(expected_response)));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Try a set
  const uint8_t set_data = 0xff;
  request = BuildSubDeviceSetRequest(PID_BURN_IN, 1, &set_data,
                                     sizeof(set_data));

  response.reset(GetResponseFromData(request.get()));
  size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, dimmerInfo) {
  unique_ptr<RDMRequest> request = BuildSubDeviceGetRequest(PID_DIMMER_INFO, 1);

  const uint8_t expected_response[] = {
    0, 0, 0xff, 0xfe, 0, 0, 0xff, 0xfe,
    4, 8, 1
  };

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), expected_response, arraysize(expected_response)));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, minimumLevel) {
  unique_ptr<RDMRequest> request = BuildSubDeviceGetRequest(
      PID_MINIMUM_LEVEL, 1);

  const uint8_t expected_response[] = { 0, 0, 0, 0, 0 };
  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(),
        reinterpret_cast<const uint8_t*>(&expected_response),
        sizeof(expected_response)));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Try a set
  const uint8_t set_data[] = { 0, 0, 0, 0, 0 };
  request = BuildSubDeviceSetRequest(
      PID_MINIMUM_LEVEL, 1,
      reinterpret_cast<const uint8_t*>(&set_data),
      sizeof(set_data));

  response.reset(GetResponseFromData(request.get()));
  size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, maximumLevel) {
  unique_ptr<RDMRequest> request = BuildSubDeviceGetRequest(
      PID_MAXIMUM_LEVEL, 1);

  const uint8_t expected_response[] = { 0, 0 };
  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(),
        reinterpret_cast<const uint8_t*>(&expected_response),
        sizeof(expected_response)));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Try a set
  const uint8_t set_data[] = { 0, 0 };
  request = BuildSubDeviceSetRequest(
      PID_MAXIMUM_LEVEL, 1,
      reinterpret_cast<const uint8_t*>(&set_data),
      sizeof(set_data));

  response.reset(GetResponseFromData(request.get()));
  size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, curve) {
  unique_ptr<RDMRequest> request = BuildSubDeviceGetRequest(
      PID_CURVE, 1);

  const uint8_t expected_response[] = { 1, 4 };
  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(),
        reinterpret_cast<const uint8_t*>(&expected_response),
        sizeof(expected_response)));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Try a set
  const uint8_t set_data[] = { 1 };
  request = BuildSubDeviceSetRequest(
      PID_CURVE, 1,
      reinterpret_cast<const uint8_t*>(&set_data),
      sizeof(set_data));

  response.reset(GetResponseFromData(request.get()));
  size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, curveDescription) {
  uint8_t curve = 1;
  unique_ptr<RDMRequest> request = BuildSubDeviceGetRequest(
      PID_CURVE_DESCRIPTION, 1, &curve, sizeof(curve));

  const uint8_t expected_response[] = "\001Linear";

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), expected_response, arraysize(expected_response) - 1));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, outputResponseTime) {
  unique_ptr<RDMRequest> request = BuildSubDeviceGetRequest(
      PID_OUTPUT_RESPONSE_TIME, 1);

  const uint8_t expected_response[] = { 1, 2 };
  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(),
        reinterpret_cast<const uint8_t*>(&expected_response),
        sizeof(expected_response)));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Try a set
  const uint8_t set_data[] = { 1 };
  request = BuildSubDeviceSetRequest(
      PID_OUTPUT_RESPONSE_TIME, 1,
      reinterpret_cast<const uint8_t*>(&set_data),
      sizeof(set_data));

  response.reset(GetResponseFromData(request.get()));
  size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, outputResponseTimeDescription) {
  uint8_t setting = 1;
  unique_ptr<RDMRequest> request = BuildSubDeviceGetRequest(
      PID_OUTPUT_RESPONSE_TIME_DESCRIPTION, 1, &setting, sizeof(setting));

  const uint8_t expected_response[] = "\001Fast";

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), expected_response, arraysize(expected_response) - 1));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, modulationFrequency) {
  unique_ptr<RDMRequest> request = BuildSubDeviceGetRequest(
      PID_MODULATION_FREQUENCY, 1);

  const uint8_t expected_response[] = { 1, 4 };
  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(),
        reinterpret_cast<const uint8_t*>(&expected_response),
        sizeof(expected_response)));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Try a set
  const uint8_t set_data[] = { 1 };
  request = BuildSubDeviceSetRequest(
      PID_MODULATION_FREQUENCY, 1,
      reinterpret_cast<const uint8_t*>(&set_data),
      sizeof(set_data));

  response.reset(GetResponseFromData(request.get()));
  size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(DimmerModelTest, modulationFrequencyDescription) {
  uint8_t setting = 1;
  unique_ptr<RDMRequest> request = BuildSubDeviceGetRequest(
      PID_MODULATION_FREQUENCY_DESCRIPTION, 1, &setting, sizeof(setting));

  const uint8_t expected_response[] = "\001\000\000\000250Hz";

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), expected_response, arraysize(expected_response) - 1));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}
