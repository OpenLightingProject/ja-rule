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
 * SimulatedTransceiverTest.cpp
 * Tests for the Transceiver code.
 * Copyright (C) 2015 Simon Newton
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

#include "Array.h"
#include "transceiver.h"
#include "setting_macros.h"

#include "tests/sim/InterruptController.h"
#include "tests/sim/PeripheralInputCapture.h"
#include "tests/sim/PeripheralTimer.h"
#include "tests/sim/PeripheralUART.h"
#include "tests/sim/Simulator.h"

using ola::NewCallback;
using ::testing::StrictMock;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::InvokeWithoutArgs;
using ::testing::_;
using std::vector;

#ifdef __cplusplus
extern "C" {
#endif

void InputCaptureEvent(void);
void Transceiver_TimerEvent();
void Transceiver_UARTEvent();

#ifdef __cplusplus
}
#endif

MATCHER_P3(EventIs, token, op, result, "") {
  return arg->token == token && arg->op == op && arg->result == result;
}

MATCHER_P2(MatchesFrame, expected_data, expected_length, "") {
  if (arg.empty()) {
    *result_listener << "Frame is empty";
    return false;
  }
  if (arg[0] != 0) {
    *result_listener << "Non-0 start code";
    return false;
  }
  if (arg.size() != expected_length + 1) {
    *result_listener << "Frame size mismatch, was " << arg.size()
                     << ", expected " << expected_length + 1;
    return false;
  }
  for (unsigned int i = 0; i < expected_length; i++) {
    if (arg[i + 1] != expected_data[i]) {
      *result_listener << "Index " << i << " mismatch, was " << arg[i + 1]
                       << ", expected " << expected_data[1];
      return false;
    }
  }
  return true;
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
  TransceiverTest()
      : m_tx_callback(NewCallback(this, &TransceiverTest::GotByte)),
        m_simulator(80000000),  // 1s of CPU runtime.
        m_timer(&m_simulator, &m_interrupt_controller),
        m_ic(&m_simulator, &m_interrupt_controller),
        m_uart(&m_simulator, &m_interrupt_controller, m_tx_callback.get()) {
  }

  void GotByte(USART_MODULE_ID uart_id, uint8_t byte) {
    if (uart_id == AS_USART_ID(1)) {
      m_sent_bytes.push_back(byte);
    }
  }

  void SetUp() {
    g_event_handler = &m_event_handler;
    PLIB_TMR_SetMock(&m_timer);
    PLIB_IC_SetMock(&m_ic);
    PLIB_USART_SetMock(&m_uart);
    SYS_INT_SetMock(&m_interrupt_controller);

    m_interrupt_controller.RegisterISR(INT_SOURCE_TIMER_3,
        NewCallback(&Transceiver_TimerEvent));
    m_interrupt_controller.RegisterISR(INT_SOURCE_INPUT_CAPTURE_2,
        NewCallback(&InputCaptureEvent));
    m_interrupt_controller.RegisterISR(INT_SOURCE_USART_1_ERROR,
        NewCallback(&Transceiver_UARTEvent));
    m_interrupt_controller.RegisterISR(INT_SOURCE_USART_1_TRANSMIT,
        NewCallback(&Transceiver_UARTEvent));
    m_interrupt_controller.RegisterISR(INT_SOURCE_USART_1_RECEIVE,
        NewCallback(&Transceiver_UARTEvent));

    m_simulator.AddTask(ola::NewCallback(&Transceiver_Tasks));
  }

  void TearDown() {
    g_event_handler = nullptr;
    PLIB_TMR_SetMock(nullptr);
    PLIB_IC_SetMock(nullptr);
    PLIB_USART_SetMock(nullptr);
    SYS_INT_SetMock(nullptr);
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
  std::auto_ptr<PeripheralUART::TXCallback> m_tx_callback;

  Simulator m_simulator;
  InterruptController m_interrupt_controller;
  PeripheralTimer m_timer;
  PeripheralInputCapture m_ic;
  PeripheralUART m_uart;

  StrictMock<MockEventHandler> m_event_handler;

  vector<uint8_t> m_sent_bytes;

  void SwitchToControllerMode();
};

void TransceiverTest::SwitchToControllerMode() {
  uint8_t token = 1;
  EXPECT_CALL(m_event_handler,
              Run(EventIs(token, T_OP_MODE_CHANGE, T_RESULT_OK)))
    .WillOnce(DoAll(InvokeWithoutArgs(&m_simulator, &Simulator::Stop),
                    Return(true)));

  EXPECT_TRUE(Transceiver_SetMode(T_MODE_CONTROLLER, token));

  m_simulator.Run();
}

// Tests to add:
//  - queue frame in controller mode, then switch to responder mode, confirm we
//    get a cancel

TEST_F(TransceiverTest, txDMXFrame) {
  TransceiverHardwareSettings settings = DefaultSettings();
  Transceiver_Initialize(&settings, &EventHandler, &EventHandler);

  SwitchToControllerMode();

  uint8_t token = 1;
  EXPECT_CALL(m_event_handler,
              Run(EventIs(token, T_OP_TX_ONLY, T_RESULT_OK)))
    .WillOnce(DoAll(InvokeWithoutArgs(&m_simulator, &Simulator::Stop),
                    Return(true)));

  const uint8_t dmx[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  Transceiver_QueueDMX(1, dmx, arraysize(dmx));
  m_simulator.Run();

  EXPECT_THAT(m_sent_bytes, MatchesFrame(dmx, arraysize(dmx)));
}
