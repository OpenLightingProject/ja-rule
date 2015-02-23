/*
 * File:   dmx.c
 * Author: Simon Newton
 */

#include "dmx.h"

#include <stdint.h>
#include <stdlib.h>
#include <sys/attribs.h>

#include "constants.h"
#include "syslog.h"
#include "system_definitions.h"
#include "peripheral/usart/plib_usart.h"
#include "peripheral/tmr/plib_tmr.h"


// The number of TX buffers we maintain for overlapping I/O
#define NUMBER_OF_TX_BUFFERS 3

// TODO(simon): Move this to the system_config file.
#define DMX_TX_UART USART_ID_1

// Port F, Bit 8 doubles as the UART1 TX pin
#define DMX_PORT PORT_CHANNEL_F
#define DMX_PORT_BIT PORTS_BIT_POS_8
#define DMX_TX_ENABLE PORTS_BIT_POS_0

// The RX Enable pin in inverted.
#define DMX_RX_ENABLE PORTS_BIT_POS_1

// About right for now
#define MARK_TICKS 10000
#define BREAK_TICKS 900

typedef enum {
  DMX_UNINITIALIZED,
  DMX_IDLE,
  DMX_BREAK,
  DMX_IN_BREAK,
  DMX_IN_MARK,
  DMX_BEGIN_TX,
  DMX_TX,
  DMX_TX_BUFFER_EMPTY,
  DMX_RECEIVING,
  DMX_COMPLETE,
  DMX_ERROR,
  DMX_RESET
} DMXState;

typedef enum {
  DMX_NO_RESPONSE,
  RDM_DUB,
  RDM_WITH_RESPONSE
} DMXFrameType;

typedef struct {
  int size;
  DMXFrameType type;
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
  int tx_offset;

  DMXState state;
} DMXData;

TXBuffer buffers[NUMBER_OF_TX_BUFFERS];

DMXData g_dmx;

DMXState tmp_state;
int tmp_offset;

void __ISR(_TIMER_1_VECTOR, ipl1) DMX_TimerEvent() {
  // switch uses more instructions than a simple if
  if (g_dmx.state == DMX_IN_BREAK) {
    PLIB_PORTS_PinSet(PORTS_ID_0, DMX_PORT, DMX_PORT_BIT);
    g_dmx.state = DMX_IN_MARK;
    PLIB_TMR_Period16BitSet(TMR_ID_1, BREAK_TICKS);
  } else if (g_dmx.state == DMX_IN_MARK) {
    g_dmx.state = DMX_BEGIN_TX;
    PLIB_TMR_Stop(TMR_ID_1);
  }
  SYS_INT_SourceStatusClear(INT_SOURCE_TIMER_1);
}

void DMX_TXBytes() {
  while (!PLIB_USART_TransmitterBufferIsFull(DMX_TX_UART) &&
         g_dmx.tx_offset != g_dmx.tx->size) {
    PLIB_USART_TransmitterByteSend(DMX_TX_UART,
                                   g_dmx.tx->data[g_dmx.tx_offset++]);
  }
}


void DMX_FlushRX() {
  while (PLIB_USART_ReceiverDataIsAvailable(DMX_TX_UART)) {
    PLIB_USART_ReceiverByteReceive(DMX_TX_UART);
  }
}

void DMX_RXBytes() {
  while (PLIB_USART_ReceiverDataIsAvailable(DMX_TX_UART) &&
         g_dmx.tx_offset != g_dmx.tx->size) {
    g_dmx.tx->data[g_dmx.tx_offset++] =
      PLIB_USART_ReceiverByteReceive(DMX_TX_UART);
  }
}

void __ISR(_UART_1_VECTOR, ipl6) DMX_TX_UART_Empty() {
  if (SYS_INT_SourceStatusGet(INT_SOURCE_USART_1_TRANSMIT)) {
    BSP_LEDToggle(BSP_LED_2);
    if (g_dmx.state == DMX_TX_BUFFER_EMPTY) {
      // The last byte has been transmitted
      SYS_INT_SourceDisable(INT_SOURCE_USART_1_TRANSMIT);
      if (g_dmx.tx->type == DMX_NO_RESPONSE) {
        g_dmx.state = DMX_COMPLETE;
      } else {
        // Switch to RX Mode.
        PLIB_PORTS_PinClear(PORTS_ID_0, DMX_PORT, DMX_TX_ENABLE);
        PLIB_USART_TransmitterDisable(DMX_TX_UART);
        PLIB_PORTS_PinClear(PORTS_ID_0, DMX_PORT, DMX_RX_ENABLE);
        g_dmx.state = DMX_RECEIVING;
        g_dmx.tx_offset = 0;
        DMX_FlushRX();
        SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_RECEIVE);
        SYS_INT_SourceEnable(INT_SOURCE_USART_1_RECEIVE);
        SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_ERROR);
        SYS_INT_SourceEnable(INT_SOURCE_USART_1_ERROR);
        PLIB_USART_ReceiverEnable(DMX_TX_UART);
      }
    } else if (g_dmx.state == DMX_TX) {
      DMX_TXBytes();
      if (g_dmx.tx_offset == g_dmx.tx->size) {
        PLIB_USART_TransmitterInterruptModeSelect(
            DMX_TX_UART, USART_TRANSMIT_FIFO_IDLE);
        g_dmx.state = DMX_TX_BUFFER_EMPTY;
      }
    }
    SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_TRANSMIT);
  } else if (SYS_INT_SourceStatusGet(INT_SOURCE_USART_1_RECEIVE)) {
    BSP_LEDToggle(BSP_LED_3);
    DMX_RXBytes();
    SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_RECEIVE);
  } else if (SYS_INT_SourceStatusGet(INT_SOURCE_USART_1_ERROR)) {
    BSP_LEDToggle(BSP_LED_1);
    if (g_dmx.state == DMX_RECEIVING) {
      PLIB_USART_ReceiverDisable(DMX_TX_UART);
      // TODO(simon): change back to TX state here?
      g_dmx.state = DMX_COMPLETE;
    }
    SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_ERROR);
  }
}

void DMX_Initialize() {
  int i;
  g_dmx.tx = NULL;
  g_dmx.next = NULL;
  g_dmx.working = NULL;
  for (i = 0; i < NUMBER_OF_TX_BUFFERS; i++) {
    g_dmx.free_list[i] = &buffers[i];
  }
  g_dmx.free_size = NUMBER_OF_TX_BUFFERS;
  g_dmx.state = DMX_UNINITIALIZED;

  tmp_state = DMX_UNINITIALIZED;
  tmp_offset = 0;

  // Setup the Break, TX Enable & RX Enable I/O Pins
  PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0, DMX_PORT, DMX_PORT_BIT);
  PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0, DMX_PORT, DMX_TX_ENABLE);
  PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0, DMX_PORT, DMX_RX_ENABLE);

  PLIB_PORTS_PinSet(PORTS_ID_0, DMX_PORT, DMX_RX_ENABLE);
  PLIB_PORTS_PinSet(PORTS_ID_0, DMX_PORT, DMX_PORT_BIT);
  PLIB_PORTS_PinSet(PORTS_ID_0, DMX_PORT, DMX_TX_ENABLE);

  // Setup the timer
  PLIB_TMR_ClockSourceSelect (TMR_ID_1, TMR_CLOCK_SOURCE_PERIPHERAL_CLOCK );
  PLIB_TMR_PrescaleSelect(TMR_ID_1 , TMR_PRESCALE_VALUE_1);
  PLIB_TMR_Mode16BitEnable(TMR_ID_1);
  PLIB_TMR_CounterAsyncWriteDisable(TMR_ID_1);
  SYS_INT_VectorPrioritySet(INT_VECTOR_T1, INT_PRIORITY_LEVEL1);
  SYS_INT_VectorSubprioritySet(INT_VECTOR_T1, INT_SUBPRIORITY_LEVEL0);

  // Setup the UART
  PLIB_USART_BaudRateSet(DMX_TX_UART,
                         SYS_CLK_PeripheralFrequencyGet(CLK_BUS_PERIPHERAL_1),
                         DMX_BAUD);
  PLIB_USART_HandshakeModeSelect(DMX_TX_UART, USART_HANDSHAKE_MODE_SIMPLEX);
  PLIB_USART_OperationModeSelect(DMX_TX_UART, USART_ENABLE_TX_RX_USED);
  PLIB_USART_LineControlModeSelect(DMX_TX_UART, USART_8N2);
  PLIB_USART_SyncModeSelect(DMX_TX_UART, USART_ASYNC_MODE);

  PLIB_USART_TransmitterInterruptModeSelect(DMX_TX_UART,
                                            USART_TRANSMIT_FIFO_EMPTY);
  PLIB_USART_TransmitterInterruptModeSelect(DMX_TX_UART,
                                            USART_RECEIVE_FIFO_HALF_FULL);

  SYS_INT_VectorPrioritySet(INT_VECTOR_UART1, INT_PRIORITY_LEVEL6);
  SYS_INT_VectorSubprioritySet(INT_VECTOR_UART1, INT_SUBPRIORITY_LEVEL0);
  SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_TRANSMIT);
  PLIB_USART_ReceiverDisable(DMX_TX_UART);
  PLIB_USART_Enable(DMX_TX_UART);
}

void DMX_Tasks() {
  int i;
  if (g_dmx.state != tmp_state) {
    SysLog_Print(SYSLOG_INFO, "Changed to %d", g_dmx.state);
    tmp_state = g_dmx.state;
  }
  
  switch (g_dmx.state) {
    case DMX_UNINITIALIZED:
      g_dmx.state = DMX_IDLE;
      break;
    case DMX_IDLE:
      if (!g_dmx.next) {
        return;
      }
      // TODO(simon: check if the frame size > 0 here
      g_dmx.tx = g_dmx.next;
      g_dmx.next = NULL;
      g_dmx.state = DMX_BREAK;
      g_dmx.tx_offset = 0;
      tmp_offset = 0;
      SysLog_Message(SYSLOG_INFO, "Begin Frame TX");
      // Fall through
    case DMX_BREAK:
      PLIB_USART_Disable(DMX_TX_UART);
      // Set UART Interrupts when the buffer is empty.
      PLIB_USART_TransmitterInterruptModeSelect(
        DMX_TX_UART, USART_TRANSMIT_FIFO_EMPTY);
      PLIB_PORTS_PinSet(PORTS_ID_0, DMX_PORT, DMX_TX_ENABLE);

      g_dmx.state = DMX_IN_BREAK;
      PLIB_TMR_Counter16BitClear(TMR_ID_1);
      PLIB_TMR_Period16BitSet(TMR_ID_1, MARK_TICKS);
      SYS_INT_SourceStatusClear(INT_SOURCE_TIMER_1);
      SYS_INT_SourceEnable(INT_SOURCE_TIMER_1);
      PLIB_PORTS_PinClear(PORTS_ID_0, DMX_PORT, DMX_PORT_BIT);
      PLIB_TMR_Start(TMR_ID_1);
      break;
    case DMX_IN_BREAK:
    case DMX_IN_MARK:
      // Noop, wait for timer event
      break;
    case DMX_BEGIN_TX:
      DMX_TXBytes();
      SYS_INT_SourceEnable(INT_SOURCE_USART_1_TRANSMIT);
      PLIB_USART_Enable(DMX_TX_UART);
      PLIB_USART_TransmitterEnable(DMX_TX_UART);
      g_dmx.state = DMX_TX;
    case DMX_TX:
    case DMX_TX_BUFFER_EMPTY:
      // Noop, wait TX to complete.
      break;
    case DMX_RECEIVING:
      // TODO(simon): how do we leave this state?
      if (tmp_offset != g_dmx.tx_offset) {
        SysLog_Print(SYSLOG_INFO, "Received %d: %d",
                     g_dmx.tx_offset,
                     g_dmx.tx->data[g_dmx.tx_offset - 1]);
        tmp_offset = g_dmx.tx_offset;
      }
      break;
    case DMX_COMPLETE:
      SysLog_Message(SYSLOG_INFO, "485 Complete");
      if (g_dmx.tx->type != DMX_NO_RESPONSE) {
        SysLog_Print(SYSLOG_INFO, "Received %d", g_dmx.tx_offset - 1);
      }
      g_dmx.free_list[g_dmx.free_size - 1] = g_dmx.tx;
      g_dmx.free_size++;
      g_dmx.tx = NULL;
      g_dmx.state = DMX_IDLE;
      // TODO(simon): delay for up to MBB time.
      break;
    case DMX_ERROR:
      // Noop
      {}
      break;
    case DMX_RESET:
      g_dmx.state = DMX_UNINITIALIZED;
  }
}

void DMX_QueueFrame(uint8_t start_code, DMXFrameType type,
                    const uint8_t* data, unsigned int size) {
  if (g_dmx.working == NULL) {
    g_dmx.working = g_dmx.free_list[g_dmx.free_size - 1];
    g_dmx.free_size--;
  }
  if (size > DMX_FRAME_SIZE - 1) {
    size = DMX_FRAME_SIZE;
  } else {
    size += 1;
  }
  g_dmx.working->size = size;
  g_dmx.working->type = type;
  g_dmx.working->data[0] = start_code;
  memcpy(&g_dmx.working->data[1], data, size - 1);

  // Move working to next.
  if (g_dmx.next) {
    g_dmx.free_list[g_dmx.free_size] = g_dmx.next;
  }
  g_dmx.next = g_dmx.working;
}

void DMX_QueueDMX(const uint8_t* data, unsigned int size) {
  DMX_QueueFrame(NULL_START_CODE, DMX_NO_RESPONSE, data, size);
}

void DMX_QueueDUB(const uint8_t* data, unsigned int size) {
  DMX_QueueFrame(RDM_START_CODE, RDM_DUB, data, size);
}

void DMX_QueueRDMRequest(const uint8_t* data, unsigned int size) {
    DMX_QueueFrame(RDM_START_CODE, RDM_WITH_RESPONSE, data, size);
}

void DMX_Reset() {
  g_dmx.state = DMX_RESET;
}
