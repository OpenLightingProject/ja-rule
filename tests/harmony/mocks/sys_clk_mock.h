#ifndef TESTS_HARMONY_MOCKS_SYS_CLK_MOCK_H_
#define TESTS_HARMONY_MOCKS_SYS_CLK_MOCK_H_

#include <gmock/gmock.h>
#include "system/clk/sys_clk.h"

class MockSysClk {
 public:
  MOCK_METHOD1(PeripheralFrequencyGet,
               uint32_t(CLK_BUSES_PERIPHERAL peripheralBus));
};

void SYS_CLK_SetMock(MockSysClk* mock);

#endif  // TESTS_HARMONY_MOCKS_SYS_CLK_MOCK_H_
