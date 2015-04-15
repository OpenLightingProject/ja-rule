#ifndef TESTS_HARMONY_MOCKS_PLIB_TMR_MOCK_H_
#define TESTS_HARMONY_MOCKS_PLIB_TMR_MOCK_H_

#include <gmock/gmock.h>
#include "peripheral/tmr/plib_tmr.h"

class MockPeripheralTimer {
 public:
  MOCK_METHOD2(Counter16BitSet, void(TMR_MODULE_ID index, uint16_t value));
  MOCK_METHOD1(Counter16BitGet, uint16_t(TMR_MODULE_ID index));
  MOCK_METHOD2(Period16BitSet, void(TMR_MODULE_ID index, uint16_t period));
  MOCK_METHOD1(Counter16BitClear, void(TMR_MODULE_ID index));
  MOCK_METHOD1(Stop, void(TMR_MODULE_ID index));
  MOCK_METHOD1(Start, void(TMR_MODULE_ID index));
  MOCK_METHOD2(PrescaleSelect,
               void(TMR_MODULE_ID index, TMR_PRESCALE prescale));
  MOCK_METHOD1(CounterAsyncWriteDisable, void(TMR_MODULE_ID index));
  MOCK_METHOD2(ClockSourceSelect,
               void(TMR_MODULE_ID index, TMR_CLOCK_SOURCE source));
  MOCK_METHOD1(Mode16BitEnable, void(TMR_MODULE_ID index));
};

void PLIB_TMR_SetMock(MockPeripheralTimer* mock);

#endif  // TESTS_HARMONY_MOCKS_PLIB_TMR_MOCK_H_
