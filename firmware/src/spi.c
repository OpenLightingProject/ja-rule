/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * spi.c
 * Copyright (C) 2015 Simon Newton
 */

#include "spi.h"

#include <stdlib.h>

#include "system/int/sys_int.h"
#include "peripheral/spi/plib_spi.h"
#include "sys/attribs.h"
#include "system_config.h"

#define MY_SPI SPI_ID_2

/*
 * @brief The number of slots for transfers.
 */
enum { TRANSFER_SLOTS = 2 };

typedef enum {
  FREE,  // transfer slot is empty
  QUEUED,  // transfer is queued, but not being sent.
  IN_TRANSFER,
  DRAINING,
  COMPLETE
} TransferState;

typedef struct {
  const uint8_t *output;
  unsigned int output_remaining;
  unsigned int extra_zeros_to_send;
  uint8_t *input;
  unsigned int skip_input_bytes;
  unsigned int input_remaining;
  TransferState state;
  SPI_Callback callback;
} Transfer;

Transfer g_transfers[TRANSFER_SLOTS];

// The index of the active transfer, or -1 if no transfers are active.
int g_active_transfer = -1;

// Helper methods
// -----------------------------------------------------------------------------
static void PickNextTransfer() {
  unsigned int i = 0;
  for (; i < TRANSFER_SLOTS; i++) {
    if (g_transfers[i].state == QUEUED) {
      g_active_transfer = i;
      break;
    }
  }
}

static void QueueBytes(Transfer *transfer) {
  while (true) {
    if (PLIB_SPI_TransmitBufferIsFull(MY_SPI)) {
      return;
    }
    uint8_t data = 0;
    if (transfer->output_remaining) {
      data = *transfer->output;
      transfer->output++;
      transfer->output_remaining--;
    } else if (transfer->extra_zeros_to_send) {
      transfer->extra_zeros_to_send--;
    } else {
      // Switch to drain mode
      PLIB_SPI_FIFOInterruptModeSelect(
          MY_SPI,
          SPI_FIFO_INTERRUPT_WHEN_TRANSMISSION_IS_COMPLETE);
      transfer->state = DRAINING;
      return;
    }
    PLIB_SPI_BufferWrite(MY_SPI, data);
  }
}

static void ReadBytes(Transfer *transfer) {
  while (!PLIB_SPI_ReceiverFIFOIsEmpty(MY_SPI)) {
    uint8_t data = PLIB_SPI_BufferRead(MY_SPI);
    if (transfer->skip_input_bytes) {
      transfer->skip_input_bytes--;
    } else {
      if (transfer->input_remaining) {
        *transfer->input = data;
        transfer->input++;
        transfer->input_remaining--;
      }
      if (transfer->input_remaining == 0) {
        SYS_INT_SourceDisable(INT_SOURCE_SPI_2_RECEIVE);
      }
    }
  }
}

void __ISR(_SPI_2_VECTOR, ipl3AUTO) SPI_Event() {
  if (g_active_transfer < 0) {
    return;
  }
  Transfer *transfer = &g_transfers[g_active_transfer];

  if (SYS_INT_SourceStatusGet(INT_SOURCE_SPI_2_TRANSMIT)) {
    if (transfer->state == DRAINING) {
      transfer->state = COMPLETE;
      SYS_INT_SourceDisable(INT_SOURCE_SPI_2_TRANSMIT);
    } else {
      QueueBytes(transfer);
    }
    SYS_INT_SourceStatusClear(INT_SOURCE_SPI_2_TRANSMIT);
  }

  if (SYS_INT_SourceStatusGet(INT_SOURCE_SPI_2_RECEIVE)) {
    ReadBytes(transfer);
    SYS_INT_SourceStatusClear(INT_SOURCE_SPI_2_RECEIVE);
  }
}

static void StartTransfer(Transfer *transfer) {
  if (transfer->output_remaining == 0 && transfer->input_remaining == 0) {
    transfer->state = FREE;
    transfer->callback(SPI_COMPLETE_TRANSFER);
    return;
  }

  PLIB_SPI_BufferClear(MY_SPI);
  transfer->callback(SPI_BEGIN_TRANSFER);

  transfer->state = IN_TRANSFER;
  PLIB_SPI_FIFOInterruptModeSelect(
      MY_SPI,
      SPI_FIFO_INTERRUPT_WHEN_TRANSMIT_BUFFER_IS_1HALF_EMPTY_OR_MORE);

  PLIB_SPI_Enable(MY_SPI);
  QueueBytes(transfer);

  SYS_INT_SourceStatusClear(INT_SOURCE_SPI_2_TRANSMIT);
  SYS_INT_SourceEnable(INT_SOURCE_SPI_2_TRANSMIT);
  if (transfer->input_remaining) {
    SYS_INT_SourceStatusClear(INT_SOURCE_SPI_2_RECEIVE);
    SYS_INT_SourceEnable(INT_SOURCE_SPI_2_RECEIVE);
  }
}

// Public functions
// ----------------------------------------------------------------------------
bool SPI_QueueTransfer(const uint8_t *output,
                       unsigned int output_length,
                       uint8_t *input,
                       unsigned int input_length,
                       SPI_Callback callback) {
  Transfer *transfer = NULL;
  unsigned int i = 0;
  for (; i < TRANSFER_SLOTS; i++) {
    if (g_transfers[i].state == FREE) {
      transfer = &g_transfers[i];
      break;
    }
  }
  if (!transfer) {
    return false;
  }

  transfer->state = QUEUED;
  transfer->output = output;
  transfer->output_remaining = output_length;
  transfer->extra_zeros_to_send = input_length;
  transfer->input = input;
  transfer->input_remaining = input_length;
  transfer->skip_input_bytes = output_length;
  transfer->callback = callback;
  return true;
}

void SPI_Initialize() {
  PLIB_SPI_BaudRateSet(MY_SPI, SYS_CLK_FREQ, 1000000u);
  PLIB_SPI_CommunicationWidthSelect(MY_SPI, SPI_COMMUNICATION_WIDTH_8BITS);
  PLIB_SPI_ClockPolaritySelect(MY_SPI, SPI_CLOCK_POLARITY_IDLE_HIGH);
  PLIB_SPI_FIFOEnable(MY_SPI);  // use enhanced buffering
  PLIB_SPI_SlaveSelectDisable(MY_SPI);
  PLIB_SPI_PinDisable(MY_SPI, SPI_PIN_SLAVE_SELECT);
  PLIB_SPI_MasterEnable(MY_SPI);

  PLIB_SPI_FIFOInterruptModeSelect(
      MY_SPI,
      SPI_FIFO_INTERRUPT_WHEN_RECEIVE_BUFFER_IS_1HALF_FULL_OR_MORE);

  SYS_INT_VectorPrioritySet(INT_VECTOR_SPI2, INT_PRIORITY_LEVEL3);
  SYS_INT_VectorSubprioritySet(INT_VECTOR_SPI2, INT_SUBPRIORITY_LEVEL0);

  unsigned int i = 0;
  for (; i < TRANSFER_SLOTS; i++) {
    g_transfers[i].state = FREE;
  }
  g_active_transfer = -1;
}

void SPI_Tasks() {
  if (g_active_transfer < 0) {
    unsigned int i = 0;
    for (; i < TRANSFER_SLOTS; i++) {
      if (g_transfers[i].state == QUEUED) {
        g_active_transfer = i;
        break;
      }
    }
  }

  if (g_active_transfer < 0) {
    return;
  }
  Transfer *transfer = &g_transfers[g_active_transfer];

  switch (transfer->state) {
    case FREE:
      return;
    case QUEUED:
      StartTransfer(transfer);
      break;
    case IN_TRANSFER:
    case DRAINING:
      break;
    case COMPLETE:
      // Drain the RX buffer
      ReadBytes(transfer);
      PLIB_SPI_Disable(MY_SPI);
      transfer->state = FREE;
      transfer->callback(SPI_COMPLETE_TRANSFER);
      // Pick now so we don't starve the higher indices.
      PickNextTransfer();
      break;
  }
}
