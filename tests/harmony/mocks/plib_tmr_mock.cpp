#include <gmock/gmock.h>
#include "plib_tmr_mock.h"

namespace {
  MockPeripheralTimer *g_plib_timer_mock = NULL;
}

void PLIB_TMR_SetMock(MockPeripheralTimer* mock) {
  g_plib_timer_mock = mock;
}

void PLIB_TMR_Counter16BitSet(TMR_MODULE_ID index, uint16_t value) {
  if (g_plib_timer_mock) {
    g_plib_timer_mock->Counter16BitSet(index, value);
  }
}

uint16_t PLIB_TMR_Counter16BitGet(TMR_MODULE_ID index) {
  if (g_plib_timer_mock) {
    return g_plib_timer_mock->Counter16BitGet(index);
  }
  return 0;
}

void PLIB_TMR_Period16BitSet(TMR_MODULE_ID index, uint16_t period) {
  if (g_plib_timer_mock) {
    g_plib_timer_mock->Period16BitSet(index, period);
  }
}

void PLIB_TMR_Counter16BitClear(TMR_MODULE_ID index) {
  if (g_plib_timer_mock) {
    g_plib_timer_mock->Counter16BitClear(index);
  }
}

void PLIB_TMR_Stop(TMR_MODULE_ID index) {
  if (g_plib_timer_mock) {
    g_plib_timer_mock->Stop(index);
  }
}

void PLIB_TMR_Start(TMR_MODULE_ID index) {
  if (g_plib_timer_mock) {
    g_plib_timer_mock->Start(index);
  }
}

void PLIB_TMR_PrescaleSelect(TMR_MODULE_ID index, TMR_PRESCALE prescale) {
  if (g_plib_timer_mock) {
    g_plib_timer_mock->PrescaleSelect(index, prescale);
  }
}

void PLIB_TMR_CounterAsyncWriteDisable(TMR_MODULE_ID index) {
  if (g_plib_timer_mock) {
    g_plib_timer_mock->CounterAsyncWriteDisable(index);
  }
}

void PLIB_TMR_ClockSourceSelect(TMR_MODULE_ID index, TMR_CLOCK_SOURCE source) {
  if (g_plib_timer_mock) {
    g_plib_timer_mock->ClockSourceSelect(index, source);
  }
}

void PLIB_TMR_Mode16BitEnable(TMR_MODULE_ID index) {
  if (g_plib_timer_mock) {
    g_plib_timer_mock->Mode16BitEnable(index);
  }
}
