#ifndef TESTS_HARMONY_MOCKS_PLIB_IC_MOCK_H_
#define TESTS_HARMONY_MOCKS_PLIB_IC_MOCK_H_

#include <gmock/gmock.h>
#include "peripheral/ic/plib_ic.h"

class PeripheralInputCaptureInterface {
 public:
  virtual ~PeripheralInputCaptureInterface() {}

  virtual void Enable(IC_MODULE_ID index) = 0;
  virtual void Disable(IC_MODULE_ID index) = 0;
  virtual void FirstCaptureEdgeSelect(IC_MODULE_ID index,
                                      IC_EDGE_TYPES edgeType) = 0;
  virtual uint16_t Buffer16BitGet(IC_MODULE_ID index) = 0;
  virtual void BufferSizeSelect(IC_MODULE_ID index,
                                IC_BUFFER_SIZE bufSize) = 0;
  virtual void TimerSelect(IC_MODULE_ID index, IC_TIMERS tmr) = 0;
  virtual void ModeSelect(IC_MODULE_ID index,
                          IC_INPUT_CAPTURE_MODES modeSel) = 0;
  virtual void EventsPerInterruptSelect(IC_MODULE_ID index,
                                        IC_EVENTS_PER_INTERRUPT event) = 0;
  virtual bool BufferIsEmpty(IC_MODULE_ID index) = 0;
};

class MockPeripheralInputCapture : public PeripheralInputCaptureInterface {
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

void PLIB_IC_SetMock(PeripheralInputCaptureInterface* mock);

#endif  // TESTS_HARMONY_MOCKS_PLIB_IC_MOCK_H_
