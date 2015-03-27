#include <gmock/gmock.h>
#include "sys_clk_mock.h"

namespace {
  MockSysClk *g_sys_clk_mock = NULL;
}

void SYS_CLK_SetMock(MockSysClk* mock) {
  g_sys_clk_mock = mock;
}

uint32_t SYS_CLK_PeripheralFrequencyGet(CLK_BUSES_PERIPHERAL peripheralBus) {
  if (g_sys_clk_mock) {
    return g_sys_clk_mock->PeripheralFrequencyGet(peripheralBus);
  }
  return 80000000;
}
