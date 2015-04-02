/*
 * File:   coarse_timer.c
 * Author: Simon Newton
 */

#include "coarse_timer.h"

typedef struct {
  CoarseTimer_Settings settings;
  uint32_t timer_count;
} CoarseTimer_Data;

CoarseTimer_Data g_coarse_timer;

void CoarseTimer_TimerEvent() {
  g_coarse_timer.timer_count++;
  SYS_INT_SourceStatusClear(g_coarse_timer.settings.interrupt_source);
}

void CoarseTimer_Initialize(const CoarseTimer_Settings *settings) {
  g_coarse_timer.timer_count = 0;
  g_coarse_timer.settings = *settings;

  PLIB_TMR_Stop(settings->timer_id);
  PLIB_TMR_ClockSourceSelect(settings->timer_id,
                             TMR_CLOCK_SOURCE_PERIPHERAL_CLOCK);
  PLIB_TMR_PrescaleSelect(settings->timer_id , TMR_PRESCALE_VALUE_1);
  PLIB_TMR_Mode16BitEnable(settings->timer_id);
  PLIB_TMR_CounterAsyncWriteDisable(settings->timer_id);

  PLIB_TMR_Counter16BitClear(settings->timer_id);
  PLIB_TMR_Period16BitSet(settings->timer_id, 100 * (SYS_CLK_FREQ / 1000000));
  PLIB_TMR_Start(settings->timer_id);

  SYS_INT_SourceStatusClear(settings->interrupt_source);
  SYS_INT_SourceEnable(settings->interrupt_source);
}

uint32_t CoarseTimer_GetTime() {
  return g_coarse_timer.timer_count;
}

uint32_t CoarseTimer_ElapsedTime(uint32_t start_time) {
  // This works because of unsigned int math.
  return g_coarse_timer.timer_count - start_time;
}

bool CoarseTimer_HasElapsed(uint32_t start_time, uint32_t duration) {
  // This works because of unsigned int math.
  uint32_t diff = g_coarse_timer.timer_count - start_time;
  return diff >= duration;
}

void CoarseTimer_SetCounter(uint32_t count) {
  g_coarse_timer.timer_count = count;
}
