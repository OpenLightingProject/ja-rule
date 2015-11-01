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
 * PeripheralInputCapture.cpp
 * The input capture module used with the simulator.
 * Copyright (C) 2015 Simon Newton
 */

#include "PeripheralInputCapture.h"

#include <gtest/gtest.h>

#include <vector>

#include "macros.h"
#include "Simulator.h"
#include "ola/Callback.h"
#include "peripheral/tmr/plib_tmr.h"

PeripheralInputCapture::InputCapture::InputCapture(INT_SOURCE source)
    : enabled(false),
      mode(IC_INPUT_CAPTURE_DISABLE_MODE),
      buffer_size(IC_BUFFER_SIZE_16BIT),
      edge_type(IC_EDGE_FALLING),
      timer(IC_TIMER_TMR3),
      events_per_interrupt(IC_INTERRUPT_ON_EVERY_CAPTURE_EVENT),
      interrupt_source(source),
      prescale_counter(0),
      capture_counter(0),
      got_trigger(false) {
}

void PeripheralInputCapture::InputCapture::CaptureEvent(uint32_t value) {
  if (buffer.size() == FIFO_SIZE) {
    return;
  }
  buffer.push_back(value);
}

PeripheralInputCapture::PeripheralInputCapture(
    Simulator *simulator,
    InterruptController *interrupt_controller)
    : m_simulator(simulator),
      m_interrupt_controller(interrupt_controller),
      m_callback(ola::NewCallback(this, &PeripheralInputCapture::Tick)) {
  m_simulator->AddTask(m_callback.get());
  std::vector<INT_SOURCE> timer_sources = {
    INT_SOURCE_INPUT_CAPTURE_1,
    INT_SOURCE_INPUT_CAPTURE_2,
    INT_SOURCE_INPUT_CAPTURE_3,
    INT_SOURCE_INPUT_CAPTURE_4,
    INT_SOURCE_INPUT_CAPTURE_5,
  };
  for (const auto& source : timer_sources) {
    m_ic.push_back(InputCapture(source));
  }
}

PeripheralInputCapture::~PeripheralInputCapture() {
  m_simulator->RemoveTask(m_callback.get());
}

void PeripheralInputCapture::Tick() {
  for (auto& ic : m_ic) {
    if (!ic.enabled) {
      continue;
    }
    // See 15.7.2 Interrupt Persistence
    if (ic.buffer.size() > ic.events_per_interrupt) {
      m_interrupt_controller->RaiseInterrupt(ic.interrupt_source);
    }
  }
}

void PeripheralInputCapture::TriggerEvent(IC_MODULE_ID index,
                                          IC_EDGE_TYPES edge_type) {
  if (index >= m_ic.size()) {
    FAIL() << "Invalid IC " << index;
  }

  InputCapture &ic = m_ic[index];

  if (!ic.enabled) {
    return;
  }

  bool trigger_interrupt = false;
  bool capture = false;
  switch (ic.mode) {
    case IC_INPUT_CAPTURE_DISABLE_MODE:
      return;
    case IC_INPUT_CAPTURE_EDGE_DETECT_MODE:
      capture = true;
      break;
    case IC_INPUT_CAPTURE_FALLING_EDGE_MODE:
      if (edge_type == IC_EDGE_FALLING) {
        capture = true;
      }
      break;
    case IC_INPUT_CAPTURE_RISING_EDGE_MODE:
      if (edge_type == IC_EDGE_RISING) {
        capture = true;
      }
      break;
    case IC_INPUT_CAPTURE_EVERY_4TH_EDGE_MODE:
      if (edge_type == IC_EDGE_RISING) {
        ic.prescale_counter++;
        if (ic.prescale_counter == 4) {
          capture = true;
          ic.prescale_counter = 0;
        }
      }
      break;
    case IC_INPUT_CAPTURE_EVERY_16TH_EDGE_MODE:
      if (edge_type == IC_EDGE_RISING) {
        ic.prescale_counter++;
        if (ic.prescale_counter == 16) {
          capture = true;
          ic.prescale_counter = 0;
        }
      }
      break;
    case IC_INPUT_CAPTURE_EVERY_EDGE_MODE:
      if (ic.got_trigger || ic.edge_type == edge_type) {
        ic.got_trigger = true;
        capture = true;
      }
      break;
    case IC_INPUT_CAPTURE_INTERRUPT_MODE:
      trigger_interrupt = true;
      break;
    default:
      FAIL() << "Unknown IC mode " << ic.mode;
  }

  if (capture) {
    // Add 32bit mode here when we need it
    ic.CaptureEvent(PLIB_TMR_Counter16BitGet(
          ic.timer == IC_TIMER_TMR3 ? TMR_ID_3 : TMR_ID_2));
    ic.capture_counter++;
    if (ic.capture_counter > ic.events_per_interrupt) {
      trigger_interrupt = true;
      ic.capture_counter = 0;
    }
  }

  if (trigger_interrupt) {
    m_interrupt_controller->RaiseInterrupt(ic.interrupt_source);
  }
}

void PeripheralInputCapture::Enable(IC_MODULE_ID index) {
  if (index >= m_ic.size()) {
    FAIL() << "Invalid IC " << index;
  }
  printf("enabled IC %d\n", index);
  m_ic[index].enabled = true;
}

void PeripheralInputCapture::Disable(IC_MODULE_ID index) {
  if (index >= m_ic.size()) {
    FAIL() << "Invalid IC " << index;
  }

  InputCapture &ic = m_ic[index];
  ic.enabled = false;
  // Clears the Overflow Condition Flag
  // Resets the FIFO to the empty state
  // Resets the event count (for interrupt generation)
  // Resets the prescaler count
  ic.buffer.clear();
  ic.capture_counter = 0;
  ic.prescale_counter = 0;
  ic.got_trigger = false;
}

void PeripheralInputCapture::FirstCaptureEdgeSelect(IC_MODULE_ID index,
                                                    IC_EDGE_TYPES edgeType) {
  if (index >= m_ic.size()) {
    ADD_FAILURE() << "Invalid IC " << index;
  }
  m_ic[index].edge_type = edgeType;
}

uint16_t PeripheralInputCapture::Buffer16BitGet(IC_MODULE_ID index) {
  if (index >= m_ic.size()) {
    ADD_FAILURE() << "Invalid IC " << index;
    return 0;
  }
  uint16_t value = 0;
  InputCapture &ic = m_ic[index];
  if (!ic.buffer.empty()) {
    value = ic.buffer.front();
    ic.buffer.pop_front();
  }
  return value;
}

void PeripheralInputCapture::BufferSizeSelect(IC_MODULE_ID index,
                                              IC_BUFFER_SIZE bufSize) {
  if (index >= m_ic.size()) {
    ADD_FAILURE() << "Invalid IC " << index;
  }
  m_ic[index].buffer_size = bufSize;
}

void PeripheralInputCapture::TimerSelect(IC_MODULE_ID index, IC_TIMERS tmr) {
  if (index >= m_ic.size()) {
    ADD_FAILURE() << "Invalid IC " << index;
  }
  m_ic[index].timer = tmr;
}

void PeripheralInputCapture::ModeSelect(IC_MODULE_ID index,
                                        IC_INPUT_CAPTURE_MODES modeSel) {
  if (index >= m_ic.size()) {
    ADD_FAILURE() << "Invalid IC " << index;
  }
  m_ic[index].mode = modeSel;
}

void PeripheralInputCapture::EventsPerInterruptSelect(
    IC_MODULE_ID index,
    IC_EVENTS_PER_INTERRUPT event) {
  if (index >= m_ic.size()) {
    ADD_FAILURE() << "Invalid IC " << index;
  }
  m_ic[index].events_per_interrupt = event;
}

bool PeripheralInputCapture::BufferIsEmpty(IC_MODULE_ID index) {
  if (index >= m_ic.size()) {
    ADD_FAILURE() << "Invalid IC " << index;
  }
  return m_ic[index].buffer.empty();
}
