/*
 * This is the stub for plib_ports.h used for the tests. It contains the bare
 * minimum required to implement the mock port symbols.
 */

#ifndef TESTS_HARMONY_INCLUDE_PERIPHERAL_PORTS_PLIB_PORTS_H_
#define TESTS_HARMONY_INCLUDE_PERIPHERAL_PORTS_PLIB_PORTS_H_

#ifdef  __cplusplus
extern "C" {
#endif

typedef enum {
  PORT_CHANNEL_A = 0x00,
  PORT_CHANNEL_B = 0x01,
  PORT_CHANNEL_C = 0x02,
  PORT_CHANNEL_D = 0x03,
  PORT_CHANNEL_E = 0x04,
  PORT_CHANNEL_F = 0x05,
  PORT_CHANNEL_G = 0x06
} PORTS_CHANNEL;

typedef enum {
  PORTS_BIT_POS_0 = 0,
  PORTS_BIT_POS_1 = 1,
  PORTS_BIT_POS_2 = 2,
  PORTS_BIT_POS_3 = 3,
  PORTS_BIT_POS_4 = 4,
  PORTS_BIT_POS_5 = 5,
  PORTS_BIT_POS_6 = 6,
  PORTS_BIT_POS_7 = 7,
  PORTS_BIT_POS_8 = 8,
  PORTS_BIT_POS_9 = 9,
  PORTS_BIT_POS_10 = 10,
  PORTS_BIT_POS_11 = 11,
  PORTS_BIT_POS_12 = 12,
  PORTS_BIT_POS_13 = 13,
  PORTS_BIT_POS_14 = 14,
  PORTS_BIT_POS_15 = 15
} PORTS_BIT_POS;

typedef enum {
  PORTS_ID_0 = 0,
  PORTS_NUMBER_OF_MODULES
} PORTS_MODULE_ID;


void PLIB_PORTS_PinDirectionOutputSet(PORTS_MODULE_ID index,
                                      PORTS_CHANNEL channel,
                                      PORTS_BIT_POS bitPos);

bool PLIB_PORTS_PinGet(PORTS_MODULE_ID index,
                       PORTS_CHANNEL channel,
                       PORTS_BIT_POS bitPos);

void PLIB_PORTS_PinSet(PORTS_MODULE_ID index,
                       PORTS_CHANNEL channel,
                       PORTS_BIT_POS bitPos);

void PLIB_PORTS_PinClear(PORTS_MODULE_ID index,
                         PORTS_CHANNEL channel,
                         PORTS_BIT_POS bitPos);

void PLIB_PORTS_PinToggle(PORTS_MODULE_ID index,
                          PORTS_CHANNEL channel,
                          PORTS_BIT_POS bitPos);

#ifdef  __cplusplus
}
#endif

#endif  // TESTS_HARMONY_INCLUDE_PERIPHERAL_PORTS_PLIB_PORTS_H_
