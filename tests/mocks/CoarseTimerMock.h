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
 * CoarseTimerMock.h
 * A mock coarse timer module.
 * Copyright (C) 2015 Simon Newton
 */

#ifndef TESTS_MOCKS_COARSETIMERMOCK_H_
#define TESTS_MOCKS_COARSETIMERMOCK_H_

#include <gmock/gmock.h>
#include "coarse_timer.h"

class MockCoarseTimer {
 public:
  MOCK_METHOD1(Initialize, void(const CoarseTimer_Settings *settings));
  MOCK_METHOD0(TimerEvent, void());
  MOCK_METHOD0(GetTime, CoarseTimer_Value());
  MOCK_METHOD1(ElapsedTime, uint32_t(CoarseTimer_Value start_time));
  MOCK_METHOD2(Delta,
               uint32_t(CoarseTimer_Value start_time,
                        CoarseTimer_Value end_time));
  MOCK_METHOD2(HasElapsed,
               bool(CoarseTimer_Value start_time, uint32_t interval));
  MOCK_METHOD1(SetCounter, void(uint32_t count));
};

void CoarseTimer_SetMock(MockCoarseTimer* mock);

#endif  // TESTS_MOCKS_COARSETIMERMOCK_H_
