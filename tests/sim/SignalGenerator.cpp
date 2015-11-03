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
 * SignalGenerator.cpp
 * Generates a series of IC and UART events to simulate an incoming signal.
 * Copyright (C) 2015 Simon Newton
 */

#include "SignalGenerator.h"

#include <stdint.h>
#include <queue>

#include "PeripheralInputCapture.h"
#include "PeripheralUART.h"
#include "Simulator.h"

SignalGenerator::SignalGenerator(Simulator *simulator,
                                 PeripheralInputCapture *input_capture,
                                 PeripheralUART *uart,
                                 IC_MODULE_ID ic_index,
                                 USART_MODULE_ID uart_index,
                                 uint32_t clock_speed,
                                 uint32_t uart_baud_rate)
    : m_simulator(simulator),
      m_input_capture(input_capture),
      m_uart(uart),
      m_ic_index(ic_index),
      m_uart_index(uart_index),
      m_cycles_per_microsecond(clock_speed / 1000000),
      m_cycles_per_bit(clock_speed / uart_baud_rate),
      m_stop_on_complete(false),
      m_next_event_at(0),
      m_line_state(HIGH),
      m_tx_byte(0),
      m_state(IDLE),
      m_callback(ola::NewCallback(this, &SignalGenerator::Tick)) {
  m_simulator->AddTask(m_callback.get());
}

SignalGenerator::~SignalGenerator() {
  m_simulator->RemoveTask(m_callback.get());
}

void SignalGenerator::Tick() {
  uint64_t clock = m_simulator->Clock();
  if (clock < m_next_event_at) {
    return;
  }

  switch (m_state) {
    case IDLE:
      ProcessNextEvent();
      break;
    case WAITING:
      // The event is complete, move onto the next one.
      ProcessNextEvent();
      return;
    case START_BIT:
    case BIT_0:
    case BIT_1:
    case BIT_2:
    case BIT_3:
    case BIT_4:
    case BIT_5:
    case BIT_6:
    case BIT_7:
    case STOP_BIT_1:
      // printf("@%lld processing bit %d\n", clock, m_state);
      SetLineState(GetNextBit() ? HIGH : LOW);
      m_next_event_at = clock + m_cycles_per_bit;
      m_state = static_cast<State>(m_state + 1);
      // printf("waiting until %lld\n", m_next_event_at);
      break;
    case STOP_BIT_2:
      printf("dispatching UART byte %d\n", m_tx_byte);
      m_uart->ReceiveByte(m_uart_index, m_tx_byte);
      m_state = IDLE;
      ProcessNextEvent();
      return;
    case HALTING:
      m_simulator->Stop();
      break;
  }
}

void SignalGenerator::SetStopOnComplete(bool stop_on_complete) {
  m_stop_on_complete = stop_on_complete;
}

void SignalGenerator::AddDelay(uint32_t duration) {
  m_events.push(Event(EVENT_DELAY, duration));
}

void SignalGenerator::AddBreak(uint32_t duration) {
  m_events.push(Event(EVENT_BREAK, duration));
}

void SignalGenerator::AddMark(uint32_t duration) {
  m_events.push(Event(EVENT_MARK, duration));
}

void SignalGenerator::AddByte(uint8_t byte) {
  m_events.push(Event(EVENT_BYTE, byte));
}

void SignalGenerator::AddFrame(const uint8_t *data, unsigned int size) {
  for (unsigned int i = 0; i < size; i++) {
    m_events.push(Event(EVENT_BYTE, data[i]));
  }
}

void SignalGenerator::AddFramingError(uint8_t byte) {
  m_events.push(Event(EVENT_FRAMING_ERROR, byte));
}

void SignalGenerator::ProcessNextEvent() {
  if (m_events.empty()) {
    if (m_stop_on_complete) {
      AddDurationToClock(10);
      m_state = HALTING;
    }
    return;
  }

  Event &event = m_events.front();
  // printf("clock %lld, event type %d %d %d\n", m_simulator->Clock(),
  //         event.type, event.duration, event.byte);
  switch (event.type) {
    case EVENT_DELAY:
      AddDurationToClock(event.duration);
      m_state = WAITING;
      break;
    case EVENT_BREAK:
      SetLineState(LOW);
      AddDurationToClock(event.duration);
      m_state = WAITING;
      break;
    case EVENT_MARK:
      SetLineState(HIGH);
      AddDurationToClock(event.duration);
      m_state = WAITING;
      break;
    case EVENT_BYTE:
    case EVENT_FRAMING_ERROR:
      m_tx_byte = event.byte;
      SetLineState(LOW);
      m_state = START_BIT;
      m_next_event_at = m_simulator->Clock() + m_cycles_per_bit;
      // printf("waiting until %lld\n", m_next_event_at);
      break;
  }
  m_events.pop();
}

void SignalGenerator::AddDurationToClock(uint32_t duration) {
  m_next_event_at = m_simulator->Clock() + duration * m_cycles_per_microsecond;
  // printf("waiting until %lld\n", m_next_event_at);
}

void SignalGenerator::SetLineState(LineState new_state) {
  if (m_line_state == new_state) {
    return;
  }
  m_line_state = new_state;
  // printf("firing IC event %d\n", new_state);
  m_input_capture->TriggerEvent(
      m_ic_index,
      new_state == LOW ? IC_EDGE_FALLING : IC_EDGE_RISING);
}

// Get the next-bit given the current state.
bool SignalGenerator::GetNextBit() const {
  // The uart is in little endian order
  switch (m_state) {
    case START_BIT:
      return m_tx_byte & 0x1;
    case BIT_0:
      return m_tx_byte & 0x2;
    case BIT_1:
      return m_tx_byte & 0x4;
    case BIT_2:
      return m_tx_byte & 0x8;
    case BIT_3:
      return m_tx_byte & 0x10;
    case BIT_4:
      return m_tx_byte & 0x20;
    case BIT_5:
      return m_tx_byte & 0x40;
    case BIT_6:
      return m_tx_byte & 0x80;
    case BIT_7:
    case STOP_BIT_1:
      return true;  // stop bit
    default:
      ADD_FAILURE() << "Invalid bit";
      return false;
    }
}
