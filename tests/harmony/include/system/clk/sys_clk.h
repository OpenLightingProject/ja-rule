/*
 * This is the stub for sys_clk.h used for the tests. It contains the bare
 * minimum required to implement the mock clock symbols.
 */

#ifndef TESTS_HARMONY_INCLUDE_SYSTEM_CLK_SYS_CLK_H_
#define TESTS_HARMONY_INCLUDE_SYSTEM_CLK_SYS_CLK_H_

#ifdef  __cplusplus
extern "C" {
#endif

typedef enum {
  CLK_BUS_PERIPHERAL_1 ,
  CLK_BUS_PERIPHERAL_2,
  CLK_BUS_PERIPHERAL_3,
  CLK_BUS_PERIPHERAL_4,
  CLK_BUS_PERIPHERAL_5,
  CLK_BUS_PERIPHERAL_7,
  CLK_BUS_PERIPHERAL_8,
} CLK_BUSES_PERIPHERAL;

uint32_t SYS_CLK_PeripheralFrequencyGet(CLK_BUSES_PERIPHERAL peripheralBus);

#ifdef  __cplusplus
}
#endif

#endif  // TESTS_HARMONY_INCLUDE_SYSTEM_CLK_SYS_CLK_H_
