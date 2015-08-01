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
 * ProxyModelTest.cpp
 * Tests for the Proxy Model RDM responder.
 * Copyright (C) 2015 Simon Newton
 */

#include <gtest/gtest.h>

#include <ola/rdm/UID.h>
#include <ola/rdm/RDMCommand.h>
#include <ola/rdm/RDMEnums.h>
#include <ola/rdm/RDMResponseCodes.h>
#include <ola/network/NetworkUtils.h>
#include <string.h>
#include <memory>

#include "proxy_model.h"
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
using ola::rdm::RDMGetRequest;
using ola::rdm::RDMRequest;
using ola::rdm::RDMResponse;
using ola::rdm::RDMSetRequest;
using std::unique_ptr;

class ProxyModelTest : public ModelTest {
 public:
  ProxyModelTest()
      : ModelTest(&PROXY_MODEL_ENTRY),
        m_child_uid1(0x7a70, 0x12345679),
        m_child_uid2(0x7a70, 0x1234567a) {
  }

  void SetUp() {
    RDMResponderSettings settings;
    memcpy(settings.uid, TEST_UID, UID_LENGTH);
    RDMResponder_Initialize(&settings);
    ProxyModel_Initialize();
    PROXY_MODEL_ENTRY.activate_fn();
  }

 protected:
  UID m_child_uid1;
  UID m_child_uid2;

  unique_ptr<RDMRequest> BuildChildGetRequest(
      const UID &uid,
      uint16_t pid,
      const uint8_t *param_data = NULL,
      unsigned int param_data_size = 0) {
    return unique_ptr<RDMGetRequest>(new RDMGetRequest(
        m_controller_uid, uid, 0, 0, 0, pid, param_data,
        param_data_size));
  }

  RDMResponse *BuildAckTimerResponse(const RDMRequest *request,
                                     uint16_t ack_timer_delay) {
    uint16_t param_data = HostToNetwork(ack_timer_delay);
    return GetResponseFromData(request, reinterpret_cast<uint8_t*>(&param_data),
                               sizeof(uint16_t), ola::rdm::RDM_ACK_TIMER);
  }

  static const uint16_t ACK_TIMER_TIME = 1u;
};

TEST_F(ProxyModelTest, rootProxiedDeviceCount) {
  unique_ptr<RDMRequest> request = BuildGetRequest(PID_PROXIED_DEVICE_COUNT);

  const uint8_t expected_response[] = {
    0x00, 0x02, 0x00
  };

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), expected_response, arraysize(expected_response)));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(ProxyModelTest, rootProxiedDevices) {
  unique_ptr<RDMRequest> request = BuildGetRequest(PID_PROXIED_DEVICES);

  const uint8_t expected_response[] = {
    0x7a, 0x70, 0x12, 0x34, 0x56, 0x79,
    0x7a, 0x70, 0x12, 0x34, 0x56, 0x7a,
  };

  unique_ptr<RDMResponse> response(GetResponseFromData(
        request.get(), expected_response, arraysize(expected_response)));

  int size = InvokeRDMHandler(request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}

TEST_F(ProxyModelTest, basicQueuedMessage) {
  unique_ptr<RDMRequest> device_info_request = BuildChildGetRequest(
      m_child_uid1, PID_DEVICE_INFO);

  unique_ptr<RDMResponse> response(BuildAckTimerResponse(
        device_info_request.get(), ACK_TIMER_TIME));

  int size = InvokeRDMHandler(device_info_request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Now try the command again, we should get a NR_PROXY_BUFFER_FULL with a
  // queued message count of 1.
  response.reset(NackWithReason(
        device_info_request.get(), ola::rdm::NR_PROXY_BUFFER_FULL, 1));
  size = InvokeRDMHandler(device_info_request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Try to fetch the queued message.
  uint8_t status_type = ola::rdm::STATUS_ERROR;
  unique_ptr<RDMRequest> get_queued_error_request = BuildChildGetRequest(
        m_child_uid1, PID_QUEUED_MESSAGE, &status_type, sizeof(status_type));

  const uint8_t device_info_response[] = {
    0x01, 0x00, 0x01, 0x06, 0x71, 0x01,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x01, 0xff, 0xff,
    0x00, 0x00, 0x00
  };

  response.reset(GetResponseWithPid(get_queued_error_request.get(),
                                    PID_DEVICE_INFO,
                                    device_info_response,
                                    arraysize(device_info_response)));
  size = InvokeRDMHandler(get_queued_error_request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Now try a STATUS_GET_LAST_MESSAGE and make sure we get the same message.
  status_type = ola::rdm::STATUS_GET_LAST_MESSAGE;
  unique_ptr<RDMRequest> get_last_queued_request = BuildChildGetRequest(
        m_child_uid1, PID_QUEUED_MESSAGE, &status_type, sizeof(status_type));

  response.reset(GetResponseWithPid(get_last_queued_request.get(),
                                    PID_DEVICE_INFO,
                                    device_info_response,
                                    arraysize(device_info_response)));
  size = InvokeRDMHandler(get_last_queued_request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Try another GET, this time for PID_IDENTIFY_DEVICE
  unique_ptr<RDMRequest> identify_request = BuildChildGetRequest(
        m_child_uid1, PID_IDENTIFY_DEVICE);
  response.reset(BuildAckTimerResponse(identify_request.get(), ACK_TIMER_TIME));

  size = InvokeRDMHandler(identify_request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Try to get the last message again, it shouldn't have changed but now
  // queued message count is 1
  response.reset(GetResponseWithPid(get_last_queued_request.get(),
                                    PID_DEVICE_INFO,
                                    device_info_response,
                                    arraysize(device_info_response),
                                    ola::rdm::RDM_ACK,
                                    1));
  size = InvokeRDMHandler(get_last_queued_request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Now get the next queued message.
  uint8_t identify_device = 0;
  response.reset(GetResponseWithPid(get_queued_error_request.get(),
                                    PID_IDENTIFY_DEVICE,
                                    &identify_device,
                                    sizeof(identify_device)));

  size = InvokeRDMHandler(get_queued_error_request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));

  // Now the last message should be PID_IDENTIFY_DEVICE.
  size = InvokeRDMHandler(get_last_queued_request.get());
  EXPECT_THAT(ArrayTuple(g_rdm_buffer, size), ResponseIs(response.get()));
}
