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
 * PeripheralTimer.h
 * The timer used with the simulator.
 * Copyright (C) 2015 Simon Newton
 */

#ifndef TESTS_SIM_PERIPHERALTIMER_H_
#define TESTS_SIM_PERIPHERALTIMER_H_

#include <memory>
#include <map>
#include <vector>

#include "plib_tmr_mock.h"

#include "InterruptController.h"
#include "Simulator.h"
#include "ola/Callback.h"

class PeripheralTimer : public PeripheralTimerInterface {
 public:
  // Ownership is not transferred.
  PeripheralTimer(Simulator *simulator,
                  InterruptController *interrupt_controller);
  ~PeripheralTimer();

  void Tick();

  void Counter16BitSet(TMR_MODULE_ID index, uint16_t value);
  uint16_t Counter16BitGet(TMR_MODULE_ID index);
  void Counter16BitClear(TMR_MODULE_ID index);
  void Period16BitSet(TMR_MODULE_ID index, uint16_t period);
  void Stop(TMR_MODULE_ID index);
  void Start(TMR_MODULE_ID index);
  void PrescaleSelect(TMR_MODULE_ID index, TMR_PRESCALE prescale);
  void CounterAsyncWriteDisable(TMR_MODULE_ID index);
  void ClockSourceSelect(TMR_MODULE_ID index, TMR_CLOCK_SOURCE source);
  void Mode16BitEnable(TMR_MODULE_ID index);

 private:
  Simulator *m_simulator;
  InterruptController *m_interrupt_controller;
  std::auto_ptr<ola::Callback0<void>> m_callback;

  struct Timer {
   public:
     explicit Timer(INT_SOURCE source);

     bool enabled;
     bool in_isr;
     uint16_t counter;
     uint16_t period;
     INT_SOURCE interrupt_source;
     TMR_PRESCALE prescale;
  };

  std::vector<Timer> m_timers;
  std::map<TMR_PRESCALE, uint16_t> m_prescale_values;
};

#endif  // TESTS_SIM_PERIPHERALTIMER_H_
