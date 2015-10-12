#include <gmock/gmock.h>
#include "plib_eth_mock.h"

namespace {
  MockPeripheralEth *g_plib_eth_mock = NULL;
}

void PLIB_Eth_SetMock(MockPeripheralEth* mock) {
  g_plib_eth_mock = mock;
}

uint8_t PLIB_ETH_StationAddressGet(ETH_MODULE_ID index, uint8_t which) {
  if (g_plib_eth_mock) {
    return g_plib_eth_mock->StationAddressGet(index, which);
  }
  return 0;
}
