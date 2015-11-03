#ifndef TESTS_HARMONY_MOCKS_PLIB_TMR_MOCK_H_
#define TESTS_HARMONY_MOCKS_PLIB_TMR_MOCK_H_

#include <gmock/gmock.h>
#include "peripheral/tmr/plib_tmr.h"

class PeripheralTimerInterface {
 public:
  virtual ~PeripheralTimerInterface() {}

  virtual void Counter16BitSet(TMR_MODULE_ID index, uint16_t value) = 0;
  virtual uint16_t Counter16BitGet(TMR_MODULE_ID index) = 0;
  virtual void Counter16BitClear(TMR_MODULE_ID index) = 0;
  virtual void Period16BitSet(TMR_MODULE_ID index, uint16_t period) = 0;
  virtual void Stop(TMR_MODULE_ID index) = 0;
  virtual void Start(TMR_MODULE_ID index) = 0;
  virtual void PrescaleSelect(TMR_MODULE_ID index, TMR_PRESCALE prescale) = 0;
  virtual void CounterAsyncWriteDisable(TMR_MODULE_ID index) = 0;
  virtual void ClockSourceSelect(TMR_MODULE_ID index,
                                 TMR_CLOCK_SOURCE source) = 0;
  virtual void Mode16BitEnable(TMR_MODULE_ID index) = 0;
};

class MockPeripheralTimer : public PeripheralTimerInterface {
 public:
  MOCK_METHOD2(Counter16BitSet, void(TMR_MODULE_ID index, uint16_t value));
  MOCK_METHOD1(Counter16BitGet, uint16_t(TMR_MODULE_ID index));
  MOCK_METHOD1(Counter16BitClear, void(TMR_MODULE_ID index));
  MOCK_METHOD2(Period16BitSet, void(TMR_MODULE_ID index, uint16_t period));
  MOCK_METHOD1(Stop, void(TMR_MODULE_ID index));
  MOCK_METHOD1(Start, void(TMR_MODULE_ID index));
  MOCK_METHOD2(PrescaleSelect,
               void(TMR_MODULE_ID index, TMR_PRESCALE prescale));
  MOCK_METHOD1(CounterAsyncWriteDisable, void(TMR_MODULE_ID index));
  MOCK_METHOD2(ClockSourceSelect,
               void(TMR_MODULE_ID index, TMR_CLOCK_SOURCE source));
  MOCK_METHOD1(Mode16BitEnable, void(TMR_MODULE_ID index));
};

void PLIB_TMR_SetMock(PeripheralTimerInterface* mock);

#endif  // TESTS_HARMONY_MOCKS_PLIB_TMR_MOCK_H_
