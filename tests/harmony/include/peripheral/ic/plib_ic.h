/*
 * This is the stub for plib_ic.h used for the tests. It contains the bare
 * minimum required to implement the mock input capture symbols.
 */

#ifndef TESTS_HARMONY_INCLUDE_PERIPHERAL_IC_PLIB_IC_H_
#define TESTS_HARMONY_INCLUDE_PERIPHERAL_IC_PLIB_IC_H_

#ifdef  __cplusplus
extern "C" {
#endif

typedef enum {
  IC_ID_1 = 0,
  IC_ID_2,
  IC_ID_3,
  IC_ID_4,
  IC_ID_5,
  IC_NUMBER_OF_MODULES
} IC_MODULE_ID;

typedef enum {
  IC_INPUT_CAPTURE_DISABLE_MODE = 0,
  IC_INPUT_CAPTURE_EDGE_DETECT_MODE = 1,
  IC_INPUT_CAPTURE_FALLING_EDGE_MODE = 2,
  IC_INPUT_CAPTURE_RISING_EDGE_MODE = 3,
  IC_INPUT_CAPTURE_EVERY_4TH_EDGE_MODE = 4,
  IC_INPUT_CAPTURE_EVERY_16TH_EDGE_MODE = 5,
  IC_INPUT_CAPTURE_EVERY_EDGE_MODE = 6,
  IC_INPUT_CAPTURE_INTERRUPT_MODE = 7
} IC_INPUT_CAPTURE_MODES;

typedef enum {
  IC_BUFFER_SIZE_16BIT = 0,
  IC_BUFFER_SIZE_32BIT = 1
} IC_BUFFER_SIZE;

typedef enum {
  IC_EDGE_FALLING = 0,
  IC_EDGE_RISING = 1
} IC_EDGE_TYPES;

typedef enum {
  IC_TIMER_TMR3 = 0,
  IC_TIMER_TMR2 = 1
} IC_TIMERS;

typedef enum {
  IC_INTERRUPT_ON_EVERY_CAPTURE_EVENT = 0,
  IC_INTERRUPT_ON_EVERY_2ND_CAPTURE_EVENT = 1,
  IC_INTERRUPT_ON_EVERY_3RD_CAPTURE_EVENT = 2,
  IC_INTERRUPT_ON_EVERY_4TH_CAPTURE_EVENT = 3
} IC_EVENTS_PER_INTERRUPT;

void PLIB_IC_Enable(IC_MODULE_ID index);

void PLIB_IC_Disable(IC_MODULE_ID index);

void PLIB_IC_FirstCaptureEdgeSelect(IC_MODULE_ID index, IC_EDGE_TYPES edgeType);

uint16_t PLIB_IC_Buffer16BitGet(IC_MODULE_ID index);

void PLIB_IC_BufferSizeSelect(IC_MODULE_ID index, IC_BUFFER_SIZE bufSize);

void PLIB_IC_TimerSelect(IC_MODULE_ID index, IC_TIMERS tmr);

void PLIB_IC_ModeSelect(IC_MODULE_ID index, IC_INPUT_CAPTURE_MODES modeSel);

void PLIB_IC_EventsPerInterruptSelect(IC_MODULE_ID index,
                                      IC_EVENTS_PER_INTERRUPT event);

bool PLIB_IC_BufferIsEmpty(IC_MODULE_ID index);

#ifdef  __cplusplus
}
#endif

#endif  // TESTS_HARMONY_INCLUDE_PERIPHERAL_IC_PLIB_IC_H_
