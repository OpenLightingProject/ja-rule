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
 * SignalGenerator.h
 * Generates a series of IC and UART events to simulate an incoming signal.
 * Copyright (C) 2015 Simon Newton
 */

#ifndef TESTS_SIM_SIGNALGENERATOR_H_
#define TESTS_SIM_SIGNALGENERATOR_H_

#include <stdint.h>
#include <queue>

#include "PeripheralInputCapture.h"
#include "PeripheralUART.h"
#include "Simulator.h"

/*
 * @brief The signal generator generates the IC & UART events to simulate an
 * incoming DMX/RDM signal
 *
 * The signal generator is controlled by adding events to a queue. The
 * following events can be triggered:
 *  - a delay
 *  - a break
 *  - a mark
 *  - a byte
 *  - a framing error
 *
 * The duration of some events like the break can be specified. Where the
 * duration is variable, the units are in micro-seconds.
 * The following would send a 176uS break, a 12uS mark and a single byte.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  signal_generator.AddBreak(176);
 *  signal_generator.AddMark(12);
 *  signal_generator.AddByte(0);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
class SignalGenerator {
 public:
  SignalGenerator(Simulator *simulator,
                  PeripheralInputCapture *input_capture,
                  PeripheralUART *uart,
                  IC_MODULE_ID ic_index,
                  USART_MODULE_ID uart_index,
                  uint32_t clock_speed,
                  uint32_t uart_baud_rate);
  ~SignalGenerator();

  void Tick();

  /*
   * @brief Controls if we stop the simulator when we run out of events.
   *
   * If true, we'll run for an extra 10us to let the system settle.
   */
  void SetStopOnComplete(bool stop_on_complete);

  /*
   * @brief Queue a delay (noop)
   */
  void AddDelay(uint32_t duration);

  /*
   * @brief Queue a break
   */
  void AddBreak(uint32_t duration);

  /*
   * @brief Queue a mark
   */
  void AddMark(uint32_t duration);

  /*
   * @brief Queue a byte
   */
  void AddByte(uint8_t byte);

  /*
   * @brief Queue a series of bytes.
   */
  void AddFrame(const uint8_t *data, unsigned int size);

  /*
   * @brief Queue a framing error.
   *
   * For a framing error we send the data bits of the byte and then skip the
   * stop bits.
   */
  void AddFramingError(uint8_t byte);

 private:
  enum EventType {
    EVENT_DELAY,
    EVENT_BREAK,
    EVENT_MARK,
    EVENT_BYTE,
    EVENT_FRAMING_ERROR
  };

  struct Event {
   public:
    Event(EventType type, uint32_t duration)
      : type(type), duration(duration), byte(0) {}

    Event(EventType type, uint8_t byte)
      : type(type), duration(0), byte(byte) {}

    EventType type;
    uint32_t duration;
    uint8_t byte;  // set if type is EVENT_BYTE or EVENT_FRAMING_ERROR
  };

  enum State {
    IDLE,
    WAITING,
    START_BIT,
    BIT_0,
    BIT_1,
    BIT_2,
    BIT_3,
    BIT_4,
    BIT_5,
    BIT_6,
    BIT_7,
    STOP_BIT_1,
    STOP_BIT_2,
    HALTING,
  };

  enum LineState {
    LOW,
    HIGH,
  };

  Simulator *m_simulator;
  PeripheralInputCapture *m_input_capture;
  PeripheralUART *m_uart;
  const IC_MODULE_ID m_ic_index;
  const USART_MODULE_ID m_uart_index;
  const uint32_t m_cycles_per_microsecond;
  const uint32_t m_cycles_per_bit;
  bool m_stop_on_complete;

  uint64_t m_next_event_at;
  uint64_t m_framing_error_at;
  LineState m_line_state;
  uint8_t m_tx_byte;
  State m_state;
  std::unique_ptr<ola::Callback0<void>> m_callback;
  std::queue<Event> m_events;

  void ProcessNextEvent();
  void AddDurationToClock(uint32_t duration);
  void SetLineState(LineState new_state);
  bool GetNextBit() const;
};

#endif  // TESTS_SIM_SIGNALGENERATOR_H_
