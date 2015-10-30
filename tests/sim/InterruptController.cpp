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
 * InterruptController.cpp
 * The interrupt controller for the simulator.
 * Copyright (C) 2015 Simon Newton
 */

#include "InterruptController.h"

#include <map>

#include "macros.h"
#include "ola/Callback.h"
#include "ola/stl/STLUtils.h"

InterruptController::Interrupt::Interrupt()
    : enabled(false),
      active(false),
      callback(nullptr) {
}

InterruptController::~InterruptController() {
  ola::STLDeleteValues(&m_interrupts);
}

void InterruptController::RegisterISR(INT_SOURCE source,
                                      ISRCallback *callback) {
  Interrupt *interrupt = GetInterrupt(source);
  if (interrupt->callback) {
    delete interrupt->callback;
  }
  interrupt->callback = callback;
}

void InterruptController::RaiseInterrupt(INT_SOURCE source) {
  // printf("Raised int %d\n", source);
  Interrupt *interrupt = GetInterrupt(source);
  interrupt->active = true;
  while (interrupt->active) {
    if (interrupt->callback) {
      // The ISR is responsible for clearing the active flag
      interrupt->callback->Run();
    } else {
      FAIL() << "Interrupt " << source << " is active but no callback set!";
    }
  }
}

bool InterruptController::SourceStatusGet(INT_SOURCE source) {
  Interrupt *interrupt = GetInterrupt(source);
  return interrupt->active;
}

void InterruptController::SourceStatusClear(INT_SOURCE source) {
  Interrupt *interrupt = GetInterrupt(source);
  interrupt->active = false;
}

void InterruptController::SourceEnable(INT_SOURCE source) {
  Interrupt *interrupt = GetInterrupt(source);
  interrupt->enabled = true;
}

bool InterruptController::SourceDisable(INT_SOURCE source) {
  Interrupt *interrupt = GetInterrupt(source);
  interrupt->enabled = false;
  return true;
}

void InterruptController::VectorPrioritySet(
    UNUSED INT_VECTOR vector,
    UNUSED INT_PRIORITY_LEVEL priority) {
}

void InterruptController::VectorSubprioritySet(
    UNUSED INT_VECTOR vector,
    UNUSED INT_SUBPRIORITY_LEVEL subpriority) {
}

InterruptController::Interrupt *InterruptController::GetInterrupt(
    INT_SOURCE source) {
  return ola::STLLookupOrInsertNew(&m_interrupts, source)->second;
}
