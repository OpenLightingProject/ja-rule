#ifndef TESTS_HARMONY_MOCKS_PLIB_IC_MOCK_H_
#define TESTS_HARMONY_MOCKS_PLIB_IC_MOCK_H_

#include <gmock/gmock.h>
#include "peripheral/ic/plib_ic.h"

class MockPeripheralInputCapture {
 public:
  MOCK_METHOD1(Enable, void(IC_MODULE_ID index));
  MOCK_METHOD1(Disable, void(IC_MODULE_ID index));
  MOCK_METHOD2(FirstCaptureEdgeSelect,
               void(IC_MODULE_ID index, IC_EDGE_TYPES edgeType));
  MOCK_METHOD1(Buffer16BitGet, uint16_t(IC_MODULE_ID index));
  MOCK_METHOD2(BufferSizeSelect,
               void(IC_MODULE_ID index, IC_BUFFER_SIZE bufSize));
  MOCK_METHOD2(TimerSelect, void(IC_MODULE_ID index, IC_TIMERS tmr));
  MOCK_METHOD2(ModeSelect,
               void(IC_MODULE_ID index, IC_INPUT_CAPTURE_MODES modeSel));
  MOCK_METHOD2(EventsPerInterruptSelect,
               void(IC_MODULE_ID index, IC_EVENTS_PER_INTERRUPT event));
  MOCK_METHOD1(BufferIsEmpty, bool(IC_MODULE_ID index));
};

void PLIB_IC_SetMock(MockPeripheralInputCapture* mock);

#endif  // TESTS_HARMONY_MOCKS_PLIB_IC_MOCK_H_
