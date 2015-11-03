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
 * InterruptController.h
 * The interrupt controller for the simulator.
 * Copyright (C) 2015 Simon Newton
 */

#ifndef TESTS_SIM_INTERRUPTCONTROLLER_H_
#define TESTS_SIM_INTERRUPTCONTROLLER_H_

#include <map>
#include "ola/Callback.h"
#include "sys_int_mock.h"

class InterruptController : public SysIntInterface {
 public:
  typedef ola::Callback0<void> ISRCallback;

  ~InterruptController();

  // Ownership of the callback is transferred.
  void RegisterISR(INT_SOURCE source, ISRCallback *callback);

  void RaiseInterrupt(INT_SOURCE source);

  bool SourceStatusGet(INT_SOURCE source);
  void SourceStatusClear(INT_SOURCE source);
  void SourceEnable(INT_SOURCE source);
  bool SourceDisable(INT_SOURCE source);
  void VectorPrioritySet(INT_VECTOR vector,
                         INT_PRIORITY_LEVEL priority);
  void VectorSubprioritySet(INT_VECTOR vector,
                            INT_SUBPRIORITY_LEVEL subpriority);

 private:
  struct Interrupt {
   public:
    Interrupt();
    ~Interrupt();

    bool enabled;
    bool active;
    ISRCallback *callback;
  };

  std::map<INT_SOURCE, Interrupt*> m_interrupts;

  Interrupt *GetInterrupt(INT_SOURCE source);
};


#endif  // TESTS_SIM_INTERRUPTCONTROLLER_H_
