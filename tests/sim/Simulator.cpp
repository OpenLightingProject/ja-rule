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
 * Simulator.cpp
 * Simulate PIC32 execution.
 * Copyright (C) 2015 Simon Newton
 */

#include "Simulator.h"

#include <gtest/gtest.h>

Simulator::Simulator(uint32_t clock_speed)
    : m_clock_speed(clock_speed),
      m_run(true),
      m_clock_limit(0),
      m_clock_limit_fatal(false),
      m_clock(0) {
}

void Simulator::SetClockLimit(uint64_t duration, bool fatal) {
  m_clock_limit = m_clock + (m_clock_speed / 1000000) * duration;
  m_clock_limit_fatal = fatal;
}

void Simulator::AddTask(TaskFn *fn) {
  m_tasks.insert(fn);
}

void Simulator::RemoveTask(TaskFn *fn) {
  m_tasks.erase(fn);
}

uint64_t Simulator::Clock() const {
  return m_clock;
}

void Simulator::Run() {
  m_run = true;
  m_clock = 0;
  while (m_run) {
    for (const auto &task : m_tasks) {
      task->Run();
    }
    m_clock++;
    if (m_clock_limit && m_clock >= m_clock_limit) {
      if (m_clock_limit_fatal) {
        FAIL() << "Clock limit exceeded: " << m_clock_limit;
      }
      return;
    }
  }
}

void Simulator::Stop() {
  m_run = false;
}
