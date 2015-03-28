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
 * FlagsTest.cpp
 * Tests for the Flags code.
 * Copyright (C) 2015 Simon Newton
 */

#include <gtest/gtest.h>

#include "flags.h"
#include "Array.h"
#include "Matchers.h"
#include "TransportMock.h"

using ::testing::Args;
using ::testing::StrictMock;
using ::testing::Return;
using ::testing::_;

class FlagsTest : public testing::Test {
 public:
  void SetUp() {
    Transport_SetMock(&transport_mock);
    Flags_Initialize(Transport_Send);
  }

  void TearDown() {
    Transport_SetMock(nullptr);
  }

  StrictMock<MockTransport> transport_mock;
};

TEST_F(FlagsTest, testUnsetFlags) {
  EXPECT_FALSE(Flags_HasChanged());

  const uint8_t payload[] = {0};

  EXPECT_CALL(transport_mock, Send(GET_FLAGS, RC_OK, _, 1))
      .With(Args<2, 3>(PayloadIs(payload, arraysize(payload))))
      .WillOnce(Return(true));

  Flags_SendResponse();
  EXPECT_FALSE(Flags_HasChanged());
}

TEST_F(FlagsTest, testLogOverflow) {
  EXPECT_FALSE(Flags_HasChanged());
  Flags_SetLogOverflow();
  EXPECT_TRUE(Flags_HasChanged());

  const uint8_t payload[] = {1};

  EXPECT_CALL(transport_mock, Send(GET_FLAGS, RC_OK, _, 1))
      .With(Args<2, 3>(PayloadIs(payload, arraysize(payload))))
      .WillOnce(Return(true));

  Flags_SendResponse();
  EXPECT_FALSE(Flags_HasChanged());
}

TEST_F(FlagsTest, testTXDrop) {
  EXPECT_FALSE(Flags_HasChanged());
  Flags_SetTXDrop();
  EXPECT_TRUE(Flags_HasChanged());

  const uint8_t payload[] = {2};

  EXPECT_CALL(transport_mock, Send(GET_FLAGS, RC_OK, _, 1))
      .With(Args<2, 3>(PayloadIs(payload, arraysize(payload))))
      .WillOnce(Return(true));

  Flags_SendResponse();
  EXPECT_FALSE(Flags_HasChanged());
}

TEST_F(FlagsTest, testTXError) {
  EXPECT_FALSE(Flags_HasChanged());
  Flags_SetTXError();
  EXPECT_TRUE(Flags_HasChanged());

  const uint8_t payload[] = {4};

  EXPECT_CALL(transport_mock, Send(GET_FLAGS, RC_OK, _, 1))
      .With(Args<2, 3>(PayloadIs(payload, arraysize(payload))))
      .WillOnce(Return(true));

  Flags_SendResponse();
  EXPECT_FALSE(Flags_HasChanged());
}

TEST_F(FlagsTest, testSendFailure) {
  EXPECT_FALSE(Flags_HasChanged());
  Flags_SetLogOverflow();
  EXPECT_TRUE(Flags_HasChanged());

  const uint8_t payload[] = {1};

  // The first send fails, so the flag state is maintained.
  testing::InSequence seq;
  EXPECT_CALL(transport_mock, Send(GET_FLAGS, RC_OK, _, 1))
      .With(Args<2, 3>(PayloadIs(payload, arraysize(payload))))
      .WillOnce(Return(false));
  EXPECT_CALL(transport_mock, Send(GET_FLAGS, RC_OK, _, 1))
      .With(Args<2, 3>(PayloadIs(payload, arraysize(payload))))
      .WillOnce(Return(true));

  Flags_SendResponse();
  EXPECT_TRUE(Flags_HasChanged());

  Flags_SendResponse();
  EXPECT_FALSE(Flags_HasChanged());
}
