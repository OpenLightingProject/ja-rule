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
 * TransceiverTest.cpp
 * Tests for the Transceiver code.
 * Copyright (C) 2015 Simon Newton
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Array.h"
#include "Transceiver.h"

using ::testing::Args;
using ::testing::StrictMock;
using ::testing::Return;
using ::testing::_;

class TransceiverTest : public testing::Test {
 public:
  void SetUp() {
  }

  void TearDown() {
  }
};

TEST_F(TransceiverTest, testUnsetTransceiver) {
  Transceiver_Settings settings = {


  };
  Transceiver_Initialize(&settings);
  /*
  const uint8_t payload[] = {0};

  EXPECT_CALL(transport_mock, Send(GET_FLAGS, RC_OK, _, 1))
      .With(Args<2, 3>(PayloadIs(payload, arraysize(payload))))
      .WillOnce(Return(true));

  Transceiver_SendResponse();
  EXPECT_FALSE(Transceiver_HasChanged());
  */
}
