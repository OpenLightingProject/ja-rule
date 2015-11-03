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
 * PeripheralInputCapture.h
 * The input capture module used with the simulator.
 * Copyright (C) 2015 Simon Newton
 */

#ifndef TESTS_SIM_PERIPHERALINPUTCAPTURE_H_
#define TESTS_SIM_PERIPHERALINPUTCAPTURE_H_

#include <deque>
#include <memory>
#include <vector>

#include "plib_ic_mock.h"

#include "InterruptController.h"
#include "Simulator.h"
#include "ola/Callback.h"

class PeripheralInputCapture : public PeripheralInputCaptureInterface {
 public:
  // Ownership is not transferred.
  PeripheralInputCapture(Simulator *simulator,
                         InterruptController *interrupt_controller);
  ~PeripheralInputCapture();

  // Cause an IC event to fire.
  void TriggerEvent(IC_MODULE_ID index, IC_EDGE_TYPES edge_type);

  void Tick();

  void Enable(IC_MODULE_ID index);
  void Disable(IC_MODULE_ID index);
  void FirstCaptureEdgeSelect(IC_MODULE_ID index,
                              IC_EDGE_TYPES edgeType);
  uint16_t Buffer16BitGet(IC_MODULE_ID index);
  void BufferSizeSelect(IC_MODULE_ID index,
                        IC_BUFFER_SIZE bufSize);
  void TimerSelect(IC_MODULE_ID index, IC_TIMERS tmr);
  void ModeSelect(IC_MODULE_ID index,
                  IC_INPUT_CAPTURE_MODES modeSel);
  void EventsPerInterruptSelect(IC_MODULE_ID index,
                                IC_EVENTS_PER_INTERRUPT event);
  bool BufferIsEmpty(IC_MODULE_ID index);

 private:
  Simulator *m_simulator;
  InterruptController *m_interrupt_controller;
  std::unique_ptr<ola::Callback0<void>> m_callback;

  struct InputCapture {
   public:
    explicit InputCapture(INT_SOURCE source);

    void CaptureEvent(uint32_t value);

    bool enabled;
    IC_INPUT_CAPTURE_MODES mode;
    IC_BUFFER_SIZE buffer_size;
    IC_EDGE_TYPES edge_type;
    IC_TIMERS timer;
    IC_EVENTS_PER_INTERRUPT events_per_interrupt;

    std::deque<uint32_t> buffer;
    const INT_SOURCE interrupt_source;
    uint8_t prescale_counter;
    uint8_t capture_counter;
    bool got_trigger;

    static const uint8_t FIFO_SIZE = 4;
  };

  std::vector<InputCapture> m_ic;
};


#endif  // TESTS_SIM_PERIPHERALINPUTCAPTURE_H_
