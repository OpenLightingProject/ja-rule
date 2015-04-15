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
 * CoarseTimerMock.cpp
 * A mock coarse timer module.
 * Copyright (C) 2015 Simon Newton
 */

#include "CoarseTimerMock.h"

namespace {
MockCoarseTimer *g_coarse_timer_mock = NULL;
}

void CoarseTimer_SetMock(MockCoarseTimer* mock) {
  g_coarse_timer_mock = mock;
}

void CoarseTimer_Initialize(const CoarseTimer_Settings *settings) {
  if (g_coarse_timer_mock) {
    g_coarse_timer_mock->Initialize(settings);
  }
}

void CoarseTimer_TimerEvent() {
  if (g_coarse_timer_mock) {
    g_coarse_timer_mock->TimerEvent();
  }
}

CoarseTimer_Value CoarseTimer_GetTime() {
  if (g_coarse_timer_mock) {
    return g_coarse_timer_mock->GetTime();
  }
  return 0;
}

uint32_t CoarseTimer_ElapsedTime(CoarseTimer_Value start_time) {
  if (g_coarse_timer_mock) {
    return g_coarse_timer_mock->ElapsedTime(start_time);
  }
  return 0;
}

uint32_t CoarseTimer_Delta(CoarseTimer_Value start_time,
                           CoarseTimer_Value end_time) {
  if (g_coarse_timer_mock) {
    return g_coarse_timer_mock->Delta(start_time, end_time);
  }
  return end_time - start_time;
}

bool CoarseTimer_HasElapsed(CoarseTimer_Value start_time, uint32_t interval) {
  if (g_coarse_timer_mock) {
    return g_coarse_timer_mock->HasElapsed(start_time, interval);
  }
  return false;
}

void CoarseTimer_SetCounter(uint32_t count) {
  if (g_coarse_timer_mock) {
    g_coarse_timer_mock->SetCounter(count);
  }
}
