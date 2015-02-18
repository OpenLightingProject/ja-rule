/*
 * File:   dmx.c
 * Author: Simon Newton
 */

#include "dmx.h"

#include <stdint.h>
#include <stdlib.h>
#include "constants.h"
#include "system_definitions.h"


// The number of TX buffers we maintain for overlapping I/O
#define NUMBER_OF_TX_BUFFERS 3

typedef enum {
  DMX_IDLE,
  DMX_BREAK,
  DMX_IN_BREAK,
  DMX_MARK,
  DMX_IN_MARK,
  DMX_BEGIN_TX,
  DMX_TX
} DMXState;

typedef struct {
  int size;
  uint8_t data[DMX_FRAME_SIZE];
} TXBuffer;

typedef struct {
  TXBuffer* tx; // The Buffer that is being transmitted now
  TXBuffer* next; // The next buffer ready to be transmitted
  TXBuffer* working; // The DMX buffer we're constructing
  TXBuffer* free_list[NUMBER_OF_TX_BUFFERS];

  // The number of items on the free list.
  // Must be > 0.
  uint8_t free_size;

  DMXState state;

} DMXData;

TXBuffer buffers[NUMBER_OF_TX_BUFFERS];

DMXData g_dmx;

void DMX_Initialize() {
  int i;
  g_dmx.tx = NULL;
  g_dmx.next = NULL;
  g_dmx.working = NULL;
  for (i = 0; i < NUMBER_OF_TX_BUFFERS; i++) {
    g_dmx.free_list[i] = &buffers[i];
  }
  g_dmx.free_size = NUMBER_OF_TX_BUFFERS;

  g_dmx.state = DMX_IDLE;

  /*
  // TX UART configuration
  UARTConfigure(DMX_TX_UART, UART_ENABLE_PINS_TX_RX_ONLY);
  // FPB = 80Mhz, Baud Rate = 250kHz. Low speed mode (16x) gives BRG = 19.0
  UARTSetDataRate(DMX_TX_UART, pb_clock, DMX_FREQ);
  UARTSetLineControl(DMX_TX_UART,
                     UART_DATA_SIZE_8_BITS | UART_PARITY_NONE |
                     UART_STOP_BITS_2);

  INTSetVectorPriority(INT_VECTOR_UART(DMX_TX_UART), INT_PRIORITY_LEVEL_6);
  INTSetVectorSubPriority(INT_VECTOR_UART(DMX_TX_UART), INT_SUB_PRIORITY_LEVEL_0);
  UARTSetFifoMode(DMX_TX_UART,
                  UART_INTERRUPT_ON_TX_BUFFER_EMPTY |
                  UART_INTERRUPT_ON_RX_3_QUARTER_FULL);
  INTClearFlag(INT_U1TX);
  */
}

void DMX_Tasks() {
  switch (g_dmx.state) {
    case DMX_IDLE:
    case DMX_BREAK:
    case DMX_IN_BREAK:
    case DMX_MARK:
    case DMX_IN_MARK:
    case DMX_BEGIN_TX:
    case DMX_TX

  }

  // Dummy implementation.
  if (g_dmx.tx == NULL) {
    // Move next to tx
    // 'Start sending'
    if (g_dmx.next) {
      g_dmx.tx = g_dmx.next;
      g_dmx.next = NULL;
      BSP_LEDToggle(BSP_LED_3);
    }
  } else {
    // 'complete sending'
    g_dmx.free_list[g_dmx.free_size - 1] = g_dmx.tx;
    g_dmx.free_size++;
    g_dmx.tx = NULL;
    BSP_LEDToggle(BSP_LED_3);

  }
}

void DMX_BeginFrame(uint8_t start_code, const uint8_t* data,
                    unsigned int size) {
  if (g_dmx.working == NULL) {
    g_dmx.working = g_dmx.free_list[g_dmx.free_size - 1];
    g_dmx.free_size--;
  }
  g_dmx.working->size = size;
  memcpy(g_dmx.working->data, data, size);
}

void DMX_FinalizeFrame() {
  if (!g_dmx.working) {
    return;
  }

  if (g_dmx.next) {
    g_dmx.free_list[g_dmx.free_size] = g_dmx.next;
  }
  g_dmx.next = g_dmx.working;
}
