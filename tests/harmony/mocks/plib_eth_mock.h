#ifndef TESTS_HARMONY_MOCKS_PLIB_ETH_MOCK_H_
#define TESTS_HARMONY_MOCKS_PLIB_ETH_MOCK_H_

#include <gmock/gmock.h>
#include "peripheral/eth/plib_eth.h"

class MockPeripheralEth {
 public:
  MOCK_METHOD2(StationAddressGet, uint8_t(ETH_MODULE_ID index, uint8_t which));
};

void PLIB_Eth_SetMock(MockPeripheralEth* mock);

#endif  // TESTS_HARMONY_MOCKS_PLIB_ETH_MOCK_H_
