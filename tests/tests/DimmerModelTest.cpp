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
    RDMResponder_Initialize(TEST_UID);
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

