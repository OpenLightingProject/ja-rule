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
#include "setting_macros.h"

using ::testing::Args;
using ::testing::StrictMock;
using ::testing::Return;
using ::testing::Field;
using ::testing::_;

MATCHER_P3(EventIs, token, op, result, "") {
  return arg->token == token && arg->op == op && arg->result == result;
}

class MockEventHandler {
 public:
  MOCK_METHOD1(Run, bool(const TransceiverEvent *event));
};

MockEventHandler *g_event_handler = nullptr;

bool EventHandler(const TransceiverEvent *event) {
  if (g_event_handler) {
    return g_event_handler->Run(event);
  }
  return true;
}

class TransceiverTest : public testing::Test {
 public:
  void SetUp() {
    g_event_handler = &m_event_handler;
  }

  void TearDown() {
    g_event_handler = nullptr;
  }

  TransceiverHardwareSettings DefaultSettings() const {
    TransceiverHardwareSettings settings = {
      .usart = AS_USART_ID(1),
      .usart_vector = AS_USART_INTERRUPT_VECTOR(1),
      .usart_tx_source = AS_USART_INTERRUPT_TX_SOURCE(1),
      .usart_rx_source = AS_USART_INTERRUPT_RX_SOURCE(1),
      .usart_error_source = AS_USART_INTERRUPT_ERROR_SOURCE(1),
      .port = PORT_CHANNEL_F,
      .break_bit = PORTS_BIT_POS_8,
      .tx_enable_bit = PORTS_BIT_POS_1,
      .rx_enable_bit = PORTS_BIT_POS_0,
      .input_capture_module = AS_IC_ID(2),
      .input_capture_vector = AS_IC_INTERRUPT_VECTOR(2),
      .input_capture_source = AS_IC_INTERRUPT_SOURCE(2),
      .timer_module_id = AS_TIMER_ID(3),
      .timer_vector = AS_TIMER_INTERRUPT_VECTOR(3),
      .timer_source = AS_TIMER_INTERRUPT_SOURCE(3),
      .input_capture_timer = AS_IC_TMR_ID(3),
    };
    return settings;
  }

 protected:
  StrictMock<MockEventHandler> m_event_handler;
};

TEST_F(TransceiverTest, testUnsetTransceiver) {
  TransceiverHardwareSettings settings = DefaultSettings();
  Transceiver_Initialize(&settings, NULL, NULL);
}

TEST_F(TransceiverTest, testModeChanges) {
  TransceiverHardwareSettings settings = DefaultSettings();
  Transceiver_Initialize(&settings, &EventHandler, &EventHandler);

  uint8_t token = 1;

  ASSERT_EQ(T_MODE_RESPONDER, Transceiver_GetMode());
  // In responder mode, the following are not permitted
  EXPECT_FALSE(Transceiver_QueueDMX(token, NULL, 0));
  EXPECT_FALSE(Transceiver_QueueASC(token, 0xdd, NULL, 0));
  EXPECT_FALSE(Transceiver_QueueRDMDUB(token, NULL, 0));
  EXPECT_FALSE(Transceiver_QueueRDMRequest(token, NULL, 0, false));
  EXPECT_FALSE(Transceiver_QueueSelfTest(token));

  // Switch to controller mode, note the switch doesn't actually take place
  // until _Tasks() is called.
  EXPECT_TRUE(Transceiver_SetMode(T_MODE_CONTROLLER, token));
  ASSERT_EQ(T_MODE_RESPONDER, Transceiver_GetMode());

  // We still can't queue frames since the mode change hasn't completed yet
  EXPECT_FALSE(Transceiver_QueueDMX(token, NULL, 0));
  EXPECT_FALSE(Transceiver_QueueASC(token, 0xdd, NULL, 0));
  EXPECT_FALSE(Transceiver_QueueRDMDUB(token, NULL, 0));
  EXPECT_FALSE(Transceiver_QueueRDMRequest(token, NULL, 0, false));
  EXPECT_FALSE(Transceiver_QueueSelfTest(token));

  // Allow the mode change to complete
  EXPECT_CALL(m_event_handler,
              Run(EventIs(token, T_OP_MODE_CHANGE, T_RESULT_OK)))
    .WillOnce(Return(true));
  Transceiver_Tasks();
  ASSERT_EQ(T_MODE_CONTROLLER, Transceiver_GetMode());

  token++;

  // In controller mode the follow are not permitted
  EXPECT_FALSE(Transceiver_QueueRDMResponse(token, NULL, 0));
  EXPECT_FALSE(Transceiver_QueueSelfTest(token));

  // Switch to self test mode.
  EXPECT_TRUE(Transceiver_SetMode(T_MODE_SELF_TEST, token));
  ASSERT_EQ(T_MODE_CONTROLLER, Transceiver_GetMode());
  EXPECT_CALL(m_event_handler,
              Run(EventIs(token, T_OP_MODE_CHANGE, T_RESULT_OK)))
    .WillOnce(Return(true));
  Transceiver_Tasks();
  ASSERT_EQ(T_MODE_SELF_TEST, Transceiver_GetMode());

  // In self-test mode the follow are not permitted
  EXPECT_FALSE(Transceiver_QueueDMX(token, NULL, 0));
  EXPECT_FALSE(Transceiver_QueueASC(token, 0xdd, NULL, 0));
  EXPECT_FALSE(Transceiver_QueueRDMDUB(token, NULL, 0));
  EXPECT_FALSE(Transceiver_QueueRDMRequest(token, NULL, 0, false));
  EXPECT_FALSE(Transceiver_QueueRDMResponse(token, NULL, 0));

  // Switch back to controller mode
  token++;
  EXPECT_TRUE(Transceiver_SetMode(T_MODE_CONTROLLER, token));
  // There is already a mode change pending, so this will fail
  EXPECT_FALSE(Transceiver_SetMode(T_MODE_CONTROLLER, ++token));
}

TEST_F(TransceiverTest, testSetBreakTime) {
  TransceiverHardwareSettings settings = DefaultSettings();
  Transceiver_Initialize(&settings, NULL, NULL);

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
  TransceiverHardwareSettings settings = DefaultSettings();
  Transceiver_Initialize(&settings, NULL, NULL);

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
  TransceiverHardwareSettings settings = DefaultSettings();
  Transceiver_Initialize(&settings, NULL, NULL);

  EXPECT_EQ(28, Transceiver_GetRDMBroadcastTimeout());
  EXPECT_TRUE(Transceiver_SetRDMBroadcastTimeout(1));
  EXPECT_EQ(1, Transceiver_GetRDMBroadcastTimeout());
  EXPECT_TRUE(Transceiver_SetRDMBroadcastTimeout(50));
  EXPECT_EQ(50, Transceiver_GetRDMBroadcastTimeout());
  EXPECT_FALSE(Transceiver_SetRDMBroadcastTimeout(51));
  EXPECT_EQ(50, Transceiver_GetRDMBroadcastTimeout());
}

TEST_F(TransceiverTest, testSetRDMWaitTime) {
  TransceiverHardwareSettings settings = DefaultSettings();
  Transceiver_Initialize(&settings, NULL, NULL);

  EXPECT_EQ(28, Transceiver_GetRDMResponseTimeout());
  EXPECT_FALSE(Transceiver_SetRDMResponseTimeout(9));
  EXPECT_EQ(28, Transceiver_GetRDMResponseTimeout());
  EXPECT_TRUE(Transceiver_SetRDMResponseTimeout(10));
  EXPECT_EQ(10, Transceiver_GetRDMResponseTimeout());
  EXPECT_TRUE(Transceiver_SetRDMResponseTimeout(50));
  EXPECT_EQ(50, Transceiver_GetRDMResponseTimeout());
  EXPECT_FALSE(Transceiver_SetRDMResponseTimeout(51));
  EXPECT_EQ(50, Transceiver_GetRDMResponseTimeout());
}

TEST_F(TransceiverTest, testSetDUBResponseTime) {
  TransceiverHardwareSettings settings = DefaultSettings();
  Transceiver_Initialize(&settings, NULL, NULL);

  EXPECT_EQ(29000, Transceiver_GetRDMDUBResponseLimit());
  EXPECT_FALSE(Transceiver_SetRDMDUBResponseLimit(9999));
  EXPECT_EQ(29000, Transceiver_GetRDMDUBResponseLimit());
  EXPECT_TRUE(Transceiver_SetRDMDUBResponseLimit(10000));
  EXPECT_EQ(10000, Transceiver_GetRDMDUBResponseLimit());
  EXPECT_TRUE(Transceiver_SetRDMDUBResponseLimit(35000));
  EXPECT_EQ(35000, Transceiver_GetRDMDUBResponseLimit());
  EXPECT_FALSE(Transceiver_SetRDMDUBResponseLimit(35001));
  EXPECT_EQ(35000, Transceiver_GetRDMDUBResponseLimit());
}

TEST_F(TransceiverTest, testSetResponderDelay) {
  TransceiverHardwareSettings settings = DefaultSettings();
  Transceiver_Initialize(&settings, NULL, NULL);

  EXPECT_EQ(1760, Transceiver_GetRDMResponderDelay());
  EXPECT_FALSE(Transceiver_SetRDMResponderDelay(1759));
  EXPECT_EQ(1760, Transceiver_GetRDMResponderDelay());
  EXPECT_TRUE(Transceiver_SetRDMResponderDelay(1761));
  EXPECT_EQ(1761, Transceiver_GetRDMResponderDelay());
  EXPECT_TRUE(Transceiver_SetRDMResponderDelay(20000));
  EXPECT_EQ(20000, Transceiver_GetRDMResponderDelay());
  EXPECT_FALSE(Transceiver_SetRDMResponderDelay(20001));
  EXPECT_EQ(20000, Transceiver_GetRDMResponderDelay());
}

TEST_F(TransceiverTest, testSetResponderJitter) {
  TransceiverHardwareSettings settings = DefaultSettings();
  Transceiver_Initialize(&settings, NULL, NULL);

  EXPECT_EQ(0, Transceiver_GetRDMResponderJitter());
  EXPECT_FALSE(Transceiver_SetRDMResponderJitter(20000));
  EXPECT_EQ(0, Transceiver_GetRDMResponderJitter());
  // 176uS + up to 1ms
  EXPECT_TRUE(Transceiver_SetRDMResponderJitter(1000));
  EXPECT_EQ(1000, Transceiver_GetRDMResponderJitter());
  EXPECT_TRUE(Transceiver_SetRDMResponderJitter(18240));
  EXPECT_EQ(18240, Transceiver_GetRDMResponderJitter());
  EXPECT_FALSE(Transceiver_SetRDMResponderJitter(18241));
  EXPECT_EQ(18240, Transceiver_GetRDMResponderJitter());

  // Test we can't wrap to a negative value
  EXPECT_FALSE(Transceiver_SetRDMResponderJitter(65535));

  // Now increase the delay, jitter should adjust
  EXPECT_TRUE(Transceiver_SetRDMResponderDelay(11000));
  EXPECT_EQ(11000, Transceiver_GetRDMResponderDelay());
  EXPECT_EQ(9000, Transceiver_GetRDMResponderJitter());
}
