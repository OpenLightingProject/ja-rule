/*
 * File:   dmx.h
 * Author: Simon Newton
 */

#include "usb.h"

#include <stdint.h>
#include <stdlib.h>
#include "constants.h"
#include "system_definitions.h"


// The number of TX buffers we maintain for overlapping I/O
#define NUMBER_OF_TX_BUFFERS 3

typedef struct {
  int size;
  uint8_t data[DMX_FRAME_SIZE];
} TxBuffer;

typedef struct {
  TxBuffer* tx; // The Buffer that is being transmitted now
  TxBuffer* next; // The next buffer ready to be transmitted
  TxBuffer* working; // The DMX buffer we're constructing
  TxBuffer* free_list[NUMBER_OF_TX_BUFFERS];

  // The number of items on the free list.
  // Must be > 0.
  uint8_t free_size;
} DMXData;

TxBuffer buffers[NUMBER_OF_TX_BUFFERS];

DMXData dmx_data;

void DMX_Initialize() {
  int i;
  dmx_data.tx = NULL;
  dmx_data.next = NULL;
  dmx_data.working = NULL;
  for (i = 0; i < NUMBER_OF_TX_BUFFERS; i++) {
    dmx_data.free_list[i] = &buffers[i];
  }
  dmx_data.free_size = NUMBER_OF_TX_BUFFERS;
}

void DMX_Tasks() {
  // Dummy implementation.
  if (dmx_data.tx == NULL) {
    // Move next to tx
    // 'Start sending'
    if (dmx_data.next) {
      dmx_data.tx = dmx_data.next;
      dmx_data.next = NULL;
      BSP_LEDToggle(BSP_LED_3);
    }
  } else {
    // 'complete sending'
    dmx_data.free_list[dmx_data.free_size - 1] = dmx_data.tx;
    dmx_data.free_size++;
    dmx_data.tx = NULL;
    BSP_LEDToggle(BSP_LED_3);

  }
}

void DMX_BeginFrame(const uint8_t* data, unsigned int size) {
  if (dmx_data.working == NULL) {
    dmx_data.working = dmx_data.free_list[dmx_data.free_size - 1];
    dmx_data.free_size--;
  }
  dmx_data.working->size = size;
  memcpy(dmx_data.working->data, data, size);
}

void DMX_FinalizeFrame() {
  if (!dmx_data.working) {
    return;
  }

  if (dmx_data.next) {
    dmx_data.free_list[dmx_data.free_size] = dmx_data.next;
  }
  dmx_data.next = dmx_data.working;
}
