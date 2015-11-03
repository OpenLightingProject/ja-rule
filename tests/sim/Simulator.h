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
 * Simulator.h
 * Simulate PIC32 execution.
 * Copyright (C) 2015 Simon Newton
 */

#ifndef TESTS_SIM_SIMULATOR_H_
#define TESTS_SIM_SIMULATOR_H_

#include <stdint.h>
#include <set>

#include "ola/Callback.h"

class Simulator {
 public:
  typedef ola::Callback0<void> TaskFn;

  explicit Simulator(uint64_t clock_limit);

  void SetClockLimit(uint64_t clock_limit);

  void AddTask(TaskFn *fn);
  void RemoveTask(TaskFn *fn);

  // Monotomic clock
  uint64_t Clock() const;

  void Run();
  void Stop();

 private:
  typedef std::set<TaskFn*> Tasks;

  bool m_run;
  uint64_t m_clock_limit;
  uint64_t m_clock;
  Tasks m_tasks;
};

#endif  // TESTS_SIM_SIMULATOR_H_
