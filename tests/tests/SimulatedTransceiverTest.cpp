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
#include <ola/rdm/UID.h>
#include <ola/rdm/RDMCommand.h>
#include <ola/rdm/RDMCommandSerializer.h>
#include <ola/rdm/RDMEnums.h>

#include <vector>

#include "Array.h"
#include "coarse_timer.h"
#include "constants.h"
#include "dmx_spec.h"
#include "setting_macros.h"
#include "transceiver.h"

#include "tests/sim/InterruptController.h"
#include "tests/sim/PeripheralInputCapture.h"
#include "tests/sim/PeripheralTimer.h"
#include "tests/sim/PeripheralUART.h"
#include "tests/sim/SignalGenerator.h"
#include "tests/sim/Simulator.h"

using ::testing::AllOf;
using ::testing::AnyOf;
using ::testing::DoAll;
using ::testing::ElementsAreArray;
using ::testing::Gt;
using ::testing::Le;
using ::testing::Lt;
using ::testing::InSequence;
using ::testing::InvokeWithoutArgs;
using ::testing::Return;
using ::testing::StrictMock;
using ::testing::Value;
using ::testing::_;
using ola::NewCallback;
using ola::rdm::NewDiscoveryUniqueBranchRequest;
using ola::rdm::RDMDiscoveryRequest;
using ola::rdm::RDMGetRequest;
using ola::rdm::UID;
using std::vector;

#ifdef __cplusplus
extern "C" {
#endif

// Declare the ISR symbols.
void InputCaptureEvent(void);
void Transceiver_TimerEvent();
void Transceiver_UARTEvent();

#ifdef __cplusplus
}
#endif

// Teach gmock how to print TransceiverEvents.
::std::ostream& operator<<(::std::ostream& os, const TransceiverEvent* event) {
    return os << "Event(token: " << event->token << ", op: " << event->op
              << ", result: " << event->result << ", length: " << event->length
              << ")";
}

// Check that the event has the correct token, op, result & data size.
MATCHER_P4(EventIs, token, op, result, data_size, "") {
  return Value(arg->token, token) &&
         Value(arg->op, op) &&
         Value(arg->result, result) &&
         Value(arg->length, data_size);
}

// Check that the event has the correct response timing.
// Remember the timing values are in 10ths of a microsecond
MATCHER_P2(RequestTimingIs, break_time, mark_time, "") {
  return arg->timing != nullptr &&
         Value(arg->timing->request.break_time, break_time) &&
         Value(arg->timing->request.mark_time, mark_time);
}

// Check that a vector contains the specified E1.11 frame.
MATCHER_P3(MatchesFrame, start_code, expected_data, expected_length, "") {
  if (arg.empty()) {
    *result_listener << "Frame is empty";
    return false;
  }
  if (arg[0] != start_code) {
    *result_listener << "Start code mismatch, was " << static_cast<int>(arg[0])
                     << ", expected " << static_cast<int>(start_code);
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

// Capture TransceiverEvents and store the data to a vector of bytes.
ACTION_P(AppendTo, output) {
  for (unsigned int i = output->size(); i < arg0->length; i++) {
    output->push_back(arg0->data[i]);
  }
  return true;
}

// This mock is used to capture Transceiver event handlers.
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
        m_callback(ola::NewCallback(&Transceiver_Tasks)),
        m_simulator(kClockSpeed),  // limit to 1s of CPU runtime.
        m_timer(&m_simulator, &m_interrupt_controller),
        m_ic(&m_simulator, &m_interrupt_controller),
        m_uart(&m_simulator, &m_interrupt_controller, m_tx_callback.get()),
        m_generator(&m_simulator, &m_ic, &m_uart, AS_IC_ID(2),
                    AS_USART_ID(1), kClockSpeed, kBaudRate),
        m_controller_uid(0x7a70, 0) {
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

    m_interrupt_controller.RegisterISR(INT_SOURCE_TIMER_1,
        NewCallback(&CoarseTimer_TimerEvent));
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

    m_simulator.AddTask(m_callback.get());

    TransceiverHardwareSettings settings = DefaultSettings();
    Transceiver_Initialize(&settings, &EventHandler, &EventHandler);

    CoarseTimer_Settings timer_settings = {
      .timer_id = AS_TIMER_ID(1),
      .interrupt_source = AS_TIMER_INTERRUPT_SOURCE(1)
    };
    CoarseTimer_Initialize(&timer_settings);
  }

  void TearDown() {
    g_event_handler = nullptr;
    PLIB_TMR_SetMock(nullptr);
    PLIB_IC_SetMock(nullptr);
    PLIB_USART_SetMock(nullptr);
    SYS_INT_SetMock(nullptr);

    m_simulator.RemoveTask(m_callback.get());
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
  std::unique_ptr<ola::Callback0<void>> m_callback;

  Simulator m_simulator;
  InterruptController m_interrupt_controller;
  PeripheralTimer m_timer;
  PeripheralInputCapture m_ic;
  PeripheralUART m_uart;
  SignalGenerator m_generator;

  UID m_controller_uid;

  StrictMock<MockEventHandler> m_event_handler;

  vector<uint8_t> m_sent_bytes;

  void SwitchToControllerMode();

  static const uint32_t kClockSpeed = 80000000;
  static const uint32_t kBaudRate = 250000;

  static const uint8_t kDMX1[];
  static const uint8_t kDMX2[];
  static const uint8_t kDMX3[];
};

const uint8_t TransceiverTest::kDMX1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
const uint8_t TransceiverTest::kDMX2[] = {0, 255, 0, 127, 128};
const uint8_t TransceiverTest::kDMX3[] = {0};

void TransceiverTest::SwitchToControllerMode() {
  uint8_t token = 1;
  EXPECT_CALL(m_event_handler,
              Run(EventIs(token, T_OP_MODE_CHANGE, T_RESULT_OK, 0)))
    .WillOnce(DoAll(InvokeWithoutArgs(&m_simulator, &Simulator::Stop),
                    Return(true)));

  EXPECT_TRUE(Transceiver_SetMode(T_MODE_CONTROLLER, token));

  m_simulator.Run();
}

// Tests to add:
//  - queue frame in controller mode, then switch to responder mode, confirm we
//    get a cancel
//  - rx a frame bigger than 512 bytes.

TEST_F(TransceiverTest, txDMX) {
  SwitchToControllerMode();

  uint8_t token = 1;
  EXPECT_CALL(m_event_handler,
              Run(EventIs(token, T_OP_TX_ONLY, T_RESULT_OK, 0)))
    .WillOnce(DoAll(InvokeWithoutArgs(&m_simulator, &Simulator::Stop),
                    Return(true)));

  const uint8_t dmx[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  Transceiver_QueueDMX(1, dmx, arraysize(dmx));
  m_simulator.Run();

  EXPECT_THAT(m_sent_bytes, MatchesFrame(NULL_START_CODE, dmx, arraysize(dmx)));
}

TEST_F(TransceiverTest, txShortDMX) {
  SwitchToControllerMode();

  uint8_t token = 1;
  EXPECT_CALL(m_event_handler,
              Run(EventIs(token, T_OP_TX_ONLY, T_RESULT_OK, 0)))
    .WillOnce(DoAll(InvokeWithoutArgs(&m_simulator, &Simulator::Stop),
                    Return(true)));

  Transceiver_QueueDMX(1, nullptr, 0);
  m_simulator.Run();

  const uint8_t dmx[] = {};
  EXPECT_THAT(m_sent_bytes, MatchesFrame(NULL_START_CODE, dmx, 0ul));
}

TEST_F(TransceiverTest, txJumboDMX) {
  SwitchToControllerMode();

  uint8_t dmx[1024];
  memset(dmx, 255, arraysize(dmx));
  uint8_t token = 1;
  EXPECT_CALL(m_event_handler,
              Run(EventIs(token, T_OP_TX_ONLY, T_RESULT_OK, 0)))
    .WillOnce(DoAll(InvokeWithoutArgs(&m_simulator, &Simulator::Stop),
                    Return(true)));

  Transceiver_QueueDMX(1, dmx, arraysize(dmx));
  m_simulator.Run();

  // Limited to 512 slots
  EXPECT_THAT(m_sent_bytes, MatchesFrame(NULL_START_CODE, dmx, 512ul));
}

TEST_F(TransceiverTest, txASCFrame) {
  const uint8_t ASC = 0xdd;
  SwitchToControllerMode();

  const uint8_t asc_frame[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  uint8_t token = 1;
  EXPECT_CALL(m_event_handler,
              Run(EventIs(token, T_OP_TX_ONLY, T_RESULT_OK, 0)))
    .WillOnce(DoAll(InvokeWithoutArgs(&m_simulator, &Simulator::Stop),
                    Return(true)));

  Transceiver_QueueASC(1, ASC, asc_frame, arraysize(asc_frame));
  m_simulator.Run();

  EXPECT_THAT(m_sent_bytes, MatchesFrame(ASC, asc_frame, arraysize(asc_frame)));
}

TEST_F(TransceiverTest, txRDMBroadcast) {
  SwitchToControllerMode();

  RDMGetRequest get_request(m_controller_uid, UID::AllDevices(), 0, 0, 0,
                            ola::rdm::PID_DEVICE_INFO, nullptr, 0);

  ola::io::ByteString data;
  EXPECT_TRUE(ola::rdm::RDMCommandSerializer::Pack(get_request, &data));

  uint8_t token = 1;
  EXPECT_CALL(m_event_handler,
              Run(EventIs(token, T_OP_RDM_BROADCAST, T_RESULT_RX_TIMEOUT, 0)))
    .WillOnce(DoAll(InvokeWithoutArgs(&m_simulator, &Simulator::Stop),
                    Return(true)));

  Transceiver_QueueRDMRequest(1, data.data(), data.size(), true);
  m_simulator.Run();

  EXPECT_THAT(m_sent_bytes,
              MatchesFrame(RDM_START_CODE, data.data(), data.size()));
}

TEST_F(TransceiverTest, txRDMDUBNoResponse) {
  SwitchToControllerMode();

  std::unique_ptr<RDMDiscoveryRequest> request(NewDiscoveryUniqueBranchRequest(
      m_controller_uid, UID(0, 0), UID::AllDevices(), 0, 0));

  ola::io::ByteString data;
  EXPECT_TRUE(ola::rdm::RDMCommandSerializer::Pack(*(request.get()), &data));

  uint8_t token = 1;
  EXPECT_CALL(m_event_handler,
              Run(EventIs(token, T_OP_RDM_DUB, T_RESULT_RX_TIMEOUT, 0)))
    .WillOnce(DoAll(InvokeWithoutArgs(&m_simulator, &Simulator::Stop),
                    Return(true)));

  Transceiver_QueueRDMDUB(1, data.data(), data.size());
  m_simulator.Run();

  EXPECT_THAT(m_sent_bytes,
              MatchesFrame(RDM_START_CODE, data.data(), data.size()));
}

TEST_F(TransceiverTest, rxDMX) {
  vector<uint8_t> rx_data;

  uint8_t token = 0;
  InSequence seq;
  EXPECT_CALL(
      m_event_handler,
      Run(EventIs(token, T_OP_RX,
                  AnyOf(T_RESULT_RX_START_FRAME, T_RESULT_RX_CONTINUE_FRAME),
                  Gt(0))))
    .WillRepeatedly(Return(true));
  EXPECT_CALL(m_event_handler, Run(AllOf(
          EventIs(token, T_OP_RX, T_RESULT_RX_CONTINUE_FRAME, arraysize(kDMX1)),
          RequestTimingIs(1760, 120))))
    .WillOnce(AppendTo(&rx_data));

  m_generator.SetStopOnComplete(true);
  m_generator.AddDelay(100);
  m_generator.AddBreak(176);
  m_generator.AddMark(12);
  m_generator.AddFrame(kDMX1, arraysize(kDMX1));

  m_simulator.Run();

  EXPECT_THAT(rx_data, ElementsAreArray(kDMX1, arraysize(kDMX1)));
}

TEST_F(TransceiverTest, rxShortBreak) {
  vector<uint8_t> rx_data;

  uint8_t token = 0;
  EXPECT_CALL(m_event_handler,
              Run(EventIs(token, T_OP_RX, _, Lt(arraysize(kDMX2)))))
    .WillRepeatedly(Return(true));
  EXPECT_CALL(
      m_event_handler, Run(AllOf(
          EventIs(token, T_OP_RX, T_RESULT_RX_CONTINUE_FRAME, arraysize(kDMX2)),
          RequestTimingIs(1900, 140))))
    .WillOnce(AppendTo(&rx_data));

  m_generator.SetStopOnComplete(true);
  m_generator.AddDelay(100);
  m_generator.AddBreak(87);  // min is 88uS
  m_generator.AddMark(12);
  m_generator.AddFrame(kDMX1, arraysize(kDMX1));
  m_generator.AddDelay(100);
  m_generator.AddBreak(190);
  m_generator.AddMark(14);
  m_generator.AddFrame(kDMX2, arraysize(kDMX2));
  m_simulator.Run();

  EXPECT_THAT(rx_data, ElementsAreArray(kDMX2, arraysize(kDMX2)));
}

TEST_F(TransceiverTest, rxShortMark) {
  vector<uint8_t> rx_data;

  uint8_t token = 0;
  EXPECT_CALL(m_event_handler,
              Run(EventIs(token, T_OP_RX, _, Lt(arraysize(kDMX2)))))
    .WillRepeatedly(Return(true));
  EXPECT_CALL(
      m_event_handler, Run(AllOf(
          EventIs(token, T_OP_RX, T_RESULT_RX_CONTINUE_FRAME, arraysize(kDMX2)),
          RequestTimingIs(1900, 140))))
    .WillOnce(AppendTo(&rx_data));

  m_generator.SetStopOnComplete(true);
  m_generator.AddDelay(100);
  m_generator.AddBreak(176);
  m_generator.AddMark(7);  // min is 8uS
  m_generator.AddFrame(kDMX1, arraysize(kDMX1));
  m_generator.AddDelay(100);
  m_generator.AddBreak(190);
  m_generator.AddMark(14);
  m_generator.AddFrame(kDMX2, arraysize(kDMX2));
  m_simulator.Run();

  EXPECT_THAT(rx_data, ElementsAreArray(kDMX2, arraysize(kDMX2)));
}

// Interslot delay, this test can take a while to run.
TEST_F(TransceiverTest, rxInterSlotDelay) {
  vector<uint8_t> rx_data;

  const uint8_t expected_frame[] = {0, 10, 20, 30, 40, 50};
  uint8_t token = 0;
  EXPECT_CALL(m_event_handler,
              Run(EventIs(token, T_OP_RX, _, Le(arraysize(expected_frame)))))
    .WillRepeatedly(Return(true));
  EXPECT_CALL(
      m_event_handler,
      Run(EventIs(token, T_OP_RX, T_RESULT_RX_FRAME_TIMEOUT,
                  arraysize(expected_frame))))
    .WillOnce(AppendTo(&rx_data));

  // we need more than 1s of runtime
  m_simulator.SetClockLimit(3 * kClockSpeed);
  m_generator.SetStopOnComplete(true);
  m_generator.AddDelay(100);
  m_generator.AddBreak(176);
  m_generator.AddMark(12);

  // We can have up to 1s between DMX slots.
  m_generator.AddByte(0);
  m_generator.AddDelay(100);  // 100us
  m_generator.AddByte(10);
  m_generator.AddDelay(1000);  // 1ms
  m_generator.AddByte(20);
  m_generator.AddDelay(10000);  // 10ms
  m_generator.AddByte(30);
  m_generator.AddDelay(100000);  // 100ms
  m_generator.AddByte(40);
  m_generator.AddDelay(999999);  // 0.999999s
  m_generator.AddByte(50);
  // This must be long enough for the coarse timer, which operates on 10s of
  // millisecond.
  m_generator.AddDelay(1010000);  // 1.01s
  m_generator.AddByte(60);

  m_simulator.Run();

  EXPECT_THAT(rx_data, ElementsAreArray(expected_frame,
              arraysize(expected_frame)));
}

// Test what happens if we send a break / mark sequence, followed by another
// break / mark sequence with data.
TEST_F(TransceiverTest, rxZeroLengthFrame) {
  vector<uint8_t> rx_data;

  uint8_t token = 0;
  EXPECT_CALL(m_event_handler,
              Run(EventIs(token, T_OP_RX, _, Lt(arraysize(kDMX2)))))
    .WillRepeatedly(Return(true));
  EXPECT_CALL(
      m_event_handler,
      Run(AllOf(
          EventIs(token, T_OP_RX, T_RESULT_RX_CONTINUE_FRAME, arraysize(kDMX2)),
          RequestTimingIs(1800, 140))))
    .WillOnce(AppendTo(&rx_data));

  m_generator.SetStopOnComplete(true);
  m_generator.AddDelay(100);
  m_generator.AddBreak(176);
  m_generator.AddMark(12);
  m_generator.AddBreak(180);
  m_generator.AddMark(14);
  m_generator.AddFrame(kDMX2, arraysize(kDMX2));
  m_generator.AddDelay(100);

  m_simulator.Run();

  EXPECT_THAT(rx_data, ElementsAreArray(kDMX2, arraysize(kDMX2)));
}

// Test we can send two frames back to back.
TEST_F(TransceiverTest, rxDoubleFrame) {
  vector<uint8_t> rx_data1, rx_data2;

  uint8_t token = 0;
  EXPECT_CALL(m_event_handler,
              Run(EventIs(_, T_OP_RX, _, _)))
    .WillRepeatedly(Return(true));
  EXPECT_CALL(
      m_event_handler,
      Run(AllOf(
          EventIs(token, T_OP_RX, T_RESULT_RX_CONTINUE_FRAME, arraysize(kDMX1)),
          RequestTimingIs(1760, 120))))
    .WillOnce(AppendTo(&rx_data1));
  EXPECT_CALL(
      m_event_handler,
      Run(AllOf(
          EventIs(token, T_OP_RX, T_RESULT_RX_CONTINUE_FRAME, arraysize(kDMX2)),
          RequestTimingIs(1800, 140))))
    .WillOnce(AppendTo(&rx_data2));

  m_generator.SetStopOnComplete(true);
  m_generator.AddDelay(100);
  m_generator.AddBreak(176);
  m_generator.AddMark(12);
  m_generator.AddFrame(kDMX1, arraysize(kDMX1));
  m_generator.AddBreak(180);
  m_generator.AddMark(14);
  m_generator.AddFrame(kDMX2, arraysize(kDMX2));
  m_generator.AddDelay(100);

  m_simulator.Run();

  EXPECT_THAT(rx_data1, ElementsAreArray(kDMX1, arraysize(kDMX1)));
  EXPECT_THAT(rx_data2, ElementsAreArray(kDMX2, arraysize(kDMX2)));
}

// Test we handle framing errors correctly.
// This ensures we deliver up to but not including the bad data
TEST_F(TransceiverTest, rxFramingError) {
  vector<uint8_t> rx_data;

  uint8_t token = 0;
  EXPECT_CALL(m_event_handler,
              Run(EventIs(token, T_OP_RX, _, Lt(arraysize(kDMX2)))))
    .WillRepeatedly(Return(true));
  EXPECT_CALL(
      m_event_handler,
      Run(AllOf(
          EventIs(token, T_OP_RX, T_RESULT_RX_CONTINUE_FRAME, arraysize(kDMX2)),
          RequestTimingIs(1760, 120))))
    .WillOnce(AppendTo(&rx_data));

  m_generator.SetStopOnComplete(true);
  m_generator.AddDelay(100);
  m_generator.AddBreak(176);
  m_generator.AddMark(12);
  m_generator.AddFrame(kDMX2, arraysize(kDMX2));
  m_generator.AddFramingError(255);

  m_simulator.Run();

  EXPECT_THAT(rx_data, ElementsAreArray(kDMX2, arraysize(kDMX2)));
}

