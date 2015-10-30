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
 * PeripheralTimer.cpp
 * The timer used with the simulator.
 * Copyright (C) 2015 Simon Newton
 */

#include "PeripheralTimer.h"

#include <gtest/gtest.h>

#include <vector>

#include "macros.h"
#include "Simulator.h"
#include "ola/Callback.h"

PeripheralTimer::Timer::Timer(INT_SOURCE source)
    : enabled(false),
      in_isr(false),
      counter(0),
      period(0),
      interrupt_source(source),
      prescale(TMR_PRESCALE_VALUE_1) {
}

PeripheralTimer::PeripheralTimer(Simulator *simulator,
                                 InterruptController *interrupt_controller)
    : m_simulator(simulator),
      m_interrupt_controller(interrupt_controller),
      m_callback(ola::NewCallback(this, &PeripheralTimer::Tick)) {
  m_simulator->AddTask(m_callback.get());
  std::vector<INT_SOURCE> timer_ids = {
    INT_SOURCE_TIMER_1,
    INT_SOURCE_TIMER_2,
    // For some really weird reason 3 and 4 are reversed.
    INT_SOURCE_TIMER_4,
    INT_SOURCE_TIMER_3,
    INT_SOURCE_TIMER_5
  };
  for (const auto& id : timer_ids) {
    m_timers.push_back(Timer(id));
  }

  m_prescale_values[TMR_PRESCALE_VALUE_1] = 1;
  m_prescale_values[TMR_PRESCALE_VALUE_2] = 2;
  m_prescale_values[TMR_PRESCALE_VALUE_4] = 4;
  m_prescale_values[TMR_PRESCALE_VALUE_8] = 8;
  m_prescale_values[TMR_PRESCALE_VALUE_16] = 16;
  m_prescale_values[TMR_PRESCALE_VALUE_32] = 32;
  m_prescale_values[TMR_PRESCALE_VALUE_64] = 64;
  m_prescale_values[TMR_PRESCALE_VALUE_256] = 256;
}

PeripheralTimer::~PeripheralTimer() {
  m_simulator->RemoveTask(m_callback.get());
}

void PeripheralTimer::Tick() {
  uint64_t ticks = m_simulator->Clock();
  for (auto& timer : m_timers) {
    if (timer.enabled && ticks % m_prescale_values[timer.prescale] == 0) {
      if (timer.counter == timer.period) {
        timer.counter = 0;
      } else {
        timer.counter++;
        if (timer.counter == timer.period) {
          timer.in_isr = true;
          m_interrupt_controller->RaiseInterrupt(timer.interrupt_source);
          timer.in_isr = false;
        }
      }
    }
  }
}

void PeripheralTimer::Counter16BitSet(TMR_MODULE_ID index, uint16_t value) {
  if (index >= m_timers.size()) {
    FAIL() << "Invalid timer " << index;
  }

  m_timers[index].counter = value;
}

uint16_t PeripheralTimer::Counter16BitGet(TMR_MODULE_ID index) {
  if (index < m_timers.size()) {
    return m_timers[index].counter;
  }
  return 0;
}

void PeripheralTimer::Counter16BitClear(TMR_MODULE_ID index) {
  Counter16BitSet(index, 0);
}

void PeripheralTimer::Period16BitSet(TMR_MODULE_ID index, uint16_t period) {
  if (index >= m_timers.size()) {
    FAIL() << "Invalid timer " << index;
  }

  Timer *timer = &m_timers[index];
  // Per the data sheet, writes to the period are only allowed when the timer
  // is disabled or we're within the ISR
  if (timer->enabled == false || timer->in_isr) {
    timer->period = period;
  } else {
    FAIL() << "Period modifed while timer " << static_cast<int>(index)
           << "was active";
  }
}

void PeripheralTimer::Stop(TMR_MODULE_ID index) {
  if (index < m_timers.size()) {
    // This does not reset the counter to 0
    m_timers[index].enabled = false;
  } else {
    FAIL() << "Invalid timer " << index;
  }
}

void PeripheralTimer::Start(TMR_MODULE_ID index) {
  if (index < m_timers.size()) {
    m_timers[index].enabled = true;
  } else {
    FAIL() << "Invalid timer " << index;
  }
}

void PeripheralTimer::PrescaleSelect(TMR_MODULE_ID index,
                                     TMR_PRESCALE prescale) {
  if (index >= m_timers.size()) {
    FAIL() << "Invalid timer " << index;
  }

  Timer *timer = &m_timers[index];
  if (timer->enabled) {
    FAIL() << "Prescale modifed while timer " << static_cast<int>(index)
           << "was active";
  } else {
    timer->prescale = prescale;
  }
}

void PeripheralTimer::CounterAsyncWriteDisable(TMR_MODULE_ID index) {
  if (index >= m_timers.size()) {
    FAIL() << "Invalid timer " << index;
  }

  const Timer &timer = m_timers[index];
  if (timer.enabled) {
    FAIL() << "CounterAsyncWrite modifed while timer "
           << static_cast<int>(index) << "was active";
  }
}

void PeripheralTimer::ClockSourceSelect(TMR_MODULE_ID index,
                                        UNUSED TMR_CLOCK_SOURCE source) {
  if (index >= m_timers.size()) {
    FAIL() << "Invalid timer " << index;
  }

  const Timer &timer = m_timers[index];
  if (timer.enabled) {
    FAIL() << "ClockSource modifed while timer "
           << static_cast<int>(index) << "was active";
  }
}

void PeripheralTimer::Mode16BitEnable(TMR_MODULE_ID index) {
  if (index >= m_timers.size()) {
    FAIL() << "Invalid timer " << index;
  }

  const Timer &timer = m_timers[index];
  if (timer.enabled) {
    FAIL() << "Mode16BitEnable modifed while timer "
           << static_cast<int>(index) << "was active";
  }
}

