#include <gmock/gmock.h>
#include "plib_ic_mock.h"

namespace {
  MockPeripheralInputCapture *g_plib_ic_mock = NULL;
}

void PLIB_IC_SetMock(MockPeripheralInputCapture* mock) {
  g_plib_ic_mock = mock;
}

void PLIB_IC_Enable(IC_MODULE_ID index) {
  if (g_plib_ic_mock) {
    g_plib_ic_mock->Enable(index);
  }
}

void PLIB_IC_Disable(IC_MODULE_ID index) {
  if (g_plib_ic_mock) {
    g_plib_ic_mock->Disable(index);
  }
}

void PLIB_IC_FirstCaptureEdgeSelect(IC_MODULE_ID index,
                                    IC_EDGE_TYPES edgeType) {
  if (g_plib_ic_mock) {
    g_plib_ic_mock->FirstCaptureEdgeSelect(index, edgeType);
  }
}

uint16_t PLIB_IC_Buffer16BitGet(IC_MODULE_ID index) {
  if (g_plib_ic_mock) {
    return g_plib_ic_mock->Buffer16BitGet(index);
  }
  return 0;
}

void PLIB_IC_BufferSizeSelect(IC_MODULE_ID index, IC_BUFFER_SIZE bufSize) {
  if (g_plib_ic_mock) {
    g_plib_ic_mock->BufferSizeSelect(index, bufSize);
  }
}

void PLIB_IC_TimerSelect(IC_MODULE_ID index, IC_TIMERS tmr) {
  if (g_plib_ic_mock) {
    g_plib_ic_mock->TimerSelect(index, tmr);
  }
}

void PLIB_IC_ModeSelect(IC_MODULE_ID index, IC_INPUT_CAPTURE_MODES modeSel) {
  if (g_plib_ic_mock) {
    g_plib_ic_mock->ModeSelect(index, modeSel);
  }
}

void PLIB_IC_EventsPerInterruptSelect(IC_MODULE_ID index,
                                      IC_EVENTS_PER_INTERRUPT event) {
  if (g_plib_ic_mock) {
    g_plib_ic_mock->EventsPerInterruptSelect(index, event);
  }
}

bool PLIB_IC_BufferIsEmpty(IC_MODULE_ID index) {
  if (g_plib_ic_mock) {
    return g_plib_ic_mock->BufferIsEmpty(index);
  }
  return true;
}
