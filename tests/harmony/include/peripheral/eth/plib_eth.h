/*
 * This is the stub for plib_eth.h used for the tests. It contains the bare
 * minimum required to implement the mock ethernet symbols.
 */

#ifndef TESTS_HARMONY_INCLUDE_PERIPHERAL_ETH_PLIB_ETH_H_
#define TESTS_HARMONY_INCLUDE_PERIPHERAL_ETH_PLIB_ETH_H_

#ifdef  __cplusplus
extern "C" {
#endif

typedef enum {
  ETH_ID_0 = 0,
  ETH_NUMBER_OF_MODULES
} ETH_MODULE_ID;

uint8_t PLIB_ETH_StationAddressGet(ETH_MODULE_ID index, uint8_t which);

#ifdef  __cplusplus
}
#endif

#endif  // TESTS_HARMONY_INCLUDE_PERIPHERAL_ETH_PLIB_ETH_H_
