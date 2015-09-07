/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * coarse_timer.c
 * Copyright (C) 2015 Simon Newton
 */

#include "coarse_timer.h"

typedef struct {
  CoarseTimer_Settings settings;
  volatile uint32_t timer_count;
} CoarseTimer_Data;

CoarseTimer_Data g_coarse_timer;

void CoarseTimer_TimerEvent() {
  g_coarse_timer.timer_count++;
  SYS_INT_SourceStatusClear(g_coarse_timer.settings.interrupt_source);
}

void CoarseTimer_Initialize(const CoarseTimer_Settings *settings) {
  g_coarse_timer.timer_count = 0u;
  g_coarse_timer.settings = *settings;

  PLIB_TMR_Stop(settings->timer_id);
  PLIB_TMR_ClockSourceSelect(settings->timer_id,
                             TMR_CLOCK_SOURCE_PERIPHERAL_CLOCK);
  PLIB_TMR_PrescaleSelect(settings->timer_id, TMR_PRESCALE_VALUE_1);
  PLIB_TMR_Mode16BitEnable(settings->timer_id);
  PLIB_TMR_CounterAsyncWriteDisable(settings->timer_id);

  PLIB_TMR_Counter16BitClear(settings->timer_id);
  PLIB_TMR_Period16BitSet(settings->timer_id, 100u * (SYS_CLK_FREQ / 1000000u));
  PLIB_TMR_Start(settings->timer_id);

  SYS_INT_SourceStatusClear(settings->interrupt_source);
  SYS_INT_SourceEnable(settings->interrupt_source);
}

CoarseTimer_Value CoarseTimer_GetTime() {
  SYS_INT_SourceDisable(g_coarse_timer.settings.interrupt_source);
  CoarseTimer_Value value = g_coarse_timer.timer_count;
  SYS_INT_SourceEnable(g_coarse_timer.settings.interrupt_source);
  return value;
}

uint32_t CoarseTimer_ElapsedTime(CoarseTimer_Value start_time) {
  // This works because of unsigned int math.
  return g_coarse_timer.timer_count - start_time;
}

uint32_t CoarseTimer_Delta(CoarseTimer_Value start_time,
                           CoarseTimer_Value end_time) {
  return end_time - start_time;
}

bool CoarseTimer_HasElapsed(uint32_t start_time, uint32_t duration) {
  if (duration == 0) {
    return true;
  }
  // This works because of unsigned int math.
  uint32_t diff = g_coarse_timer.timer_count - start_time;
  // The diff needs to be more than duration, since we don't want to fire an
  // event too early. If we use >=, consider:
  //   - start at 1.99ms (counter = 19)
  //   - end at 2.18ms (counter 21)
  //   - Check for 0.2 ms, counter delta is 2 (20ms) but actual elapsed time is
  //     0.19ms
  return diff > duration;
}

void CoarseTimer_SetCounter(uint32_t count) {
  g_coarse_timer.timer_count = count;
}
