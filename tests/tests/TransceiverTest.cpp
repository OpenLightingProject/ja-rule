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
#include "transceiver.h"

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

  Transceiver_Settings DefaultSettings() const {
    Transceiver_Settings settings = {
      .usart = USART_ID_1,
      .port = PORT_CHANNEL_F,
      .break_bit = PORTS_BIT_POS_8,
      .rx_enable_bit = PORTS_BIT_POS_0,
      .tx_enable_bit = PORTS_BIT_POS_1,
      .callback = nullptr
    };
    return settings;
  }
};

TEST_F(TransceiverTest, testUnsetTransceiver) {
  Transceiver_Settings settings = DefaultSettings();
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

TEST_F(TransceiverTest, testSetBreakTime) {
  Transceiver_Settings settings = DefaultSettings();
  Transceiver_Initialize(&settings);

  EXPECT_EQ(176, Transceiver_GetBreakTime());
  EXPECT_FALSE(Transceiver_SetBreakTime(43));
  EXPECT_EQ(176, Transceiver_GetBreakTime());
  EXPECT_TRUE(Transceiver_SetBreakTime(44));
  EXPECT_EQ(44, Transceiver_GetBreakTime());
  EXPECT_TRUE(Transceiver_SetBreakTime(800));
  EXPECT_EQ(800, Transceiver_GetBreakTime());
  EXPECT_FALSE(Transceiver_SetBreakTime(801));
  EXPECT_EQ(800, Transceiver_GetBreakTime());
}

TEST_F(TransceiverTest, testSetMarkTime) {
  Transceiver_Settings settings = DefaultSettings();
  Transceiver_Initialize(&settings);

  EXPECT_EQ(12, Transceiver_GetMarkTime());
  EXPECT_FALSE(Transceiver_SetMarkTime(3));
  EXPECT_EQ(12, Transceiver_GetMarkTime());
  EXPECT_TRUE(Transceiver_SetMarkTime(4));
  EXPECT_EQ(4, Transceiver_GetMarkTime());
  EXPECT_TRUE(Transceiver_SetMarkTime(800));
  EXPECT_EQ(800, Transceiver_GetMarkTime());
  EXPECT_FALSE(Transceiver_SetMarkTime(801));
  EXPECT_EQ(800, Transceiver_GetMarkTime());
}

TEST_F(TransceiverTest, testSetRDMBroadcastListen) {
  Transceiver_Settings settings = DefaultSettings();
  Transceiver_Initialize(&settings);

  EXPECT_EQ(28, Transceiver_GetRDMBroadcastListen());
  EXPECT_TRUE(Transceiver_SetRDMBroadcastListen(1));
  EXPECT_EQ(1, Transceiver_GetRDMBroadcastListen());
  EXPECT_TRUE(Transceiver_SetRDMBroadcastListen(50));
  EXPECT_EQ(50, Transceiver_GetRDMBroadcastListen());
  EXPECT_FALSE(Transceiver_SetRDMBroadcastListen(51));
  EXPECT_EQ(50, Transceiver_GetRDMBroadcastListen());
}

TEST_F(TransceiverTest, testSetRDMWaitTime) {
  Transceiver_Settings settings = DefaultSettings();
  Transceiver_Initialize(&settings);

  EXPECT_EQ(28, Transceiver_GetRDMWaitTime());
  EXPECT_FALSE(Transceiver_SetRDMWaitTime(9));
  EXPECT_EQ(28, Transceiver_GetRDMWaitTime());
  EXPECT_TRUE(Transceiver_SetRDMWaitTime(10));
  EXPECT_EQ(10, Transceiver_GetRDMWaitTime());
  EXPECT_TRUE(Transceiver_SetRDMWaitTime(50));
  EXPECT_EQ(50, Transceiver_GetRDMWaitTime());
  EXPECT_FALSE(Transceiver_SetRDMWaitTime(51));
  EXPECT_EQ(50, Transceiver_GetRDMWaitTime());
}