/*
 * This is the stub for plib_tmr.h used for the tests. It contains the bare
 * minimum required to implement the mock timer symbols.
 */

#ifndef TESTS_HARMONY_INCLUDE_PERIPHERAL_TMR_PLIB_TMR_H_
#define TESTS_HARMONY_INCLUDE_PERIPHERAL_TMR_PLIB_TMR_H_

#ifdef  __cplusplus
extern "C" {
#endif

typedef enum {
  TMR_ID_1 = 0,
  TMR_ID_2,
  TMR_ID_4,
  TMR_ID_3,
  TMR_ID_5,
  TMR_NUMBER_OF_MODULES
} TMR_MODULE_ID;

typedef enum {
  TMR_PRESCALE_VALUE_1 = 0x00,
  TMR_PRESCALE_VALUE_2 = 0x01,
  TMR_PRESCALE_VALUE_4 = 0x02,
  TMR_PRESCALE_VALUE_8 = 0x03,
  TMR_PRESCALE_VALUE_16 = 0x04,
  TMR_PRESCALE_VALUE_32 = 0x05,
  TMR_PRESCALE_VALUE_64 = 0x06,
  TMR_PRESCALE_VALUE_256 = 0x07
} TMR_PRESCALE;

typedef enum {
  TMR_CLOCK_SOURCE_PERIPHERAL_CLOCK = 0,
  TMR_CLOCK_SOURCE_EXTERNAL_INPUT_PIN = 1
} TMR_CLOCK_SOURCE;

void PLIB_TMR_Counter16BitSet(TMR_MODULE_ID index, uint16_t value);

void PLIB_TMR_Period16BitSet(TMR_MODULE_ID index, uint16_t period);

void PLIB_TMR_Counter16BitClear(TMR_MODULE_ID index);

void PLIB_TMR_Stop(TMR_MODULE_ID index);

void PLIB_TMR_Start(TMR_MODULE_ID index);

void PLIB_TMR_PrescaleSelect(TMR_MODULE_ID index, TMR_PRESCALE prescale);

void PLIB_TMR_CounterAsyncWriteDisable(TMR_MODULE_ID index);

void PLIB_TMR_ClockSourceSelect(TMR_MODULE_ID index, TMR_CLOCK_SOURCE source);

void PLIB_TMR_Mode16BitEnable(TMR_MODULE_ID index);

#ifdef  __cplusplus
}
#endif

#endif  // TESTS_HARMONY_INCLUDE_PERIPHERAL_TMR_PLIB_TMR_H_
