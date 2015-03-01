/*
 * File:   transceiver.c
 * Author: Simon Newton
 */

#include "transceiver.h"

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

// About right for now
#define MARK_TICKS 10000
#define BREAK_TICKS 900

typedef enum {
  TRANSCEIVER_UNINITIALIZED,
  TRANSCEIVER_IDLE,
  TRANSCEIVER_BREAK,
  TRANSCEIVER_IN_BREAK,
  TRANSCEIVER_IN_MARK,
  TRANSCEIVER_BEGIN_TX,
  TRANSCEIVER_TX,
  TRANSCEIVER_TX_BUFFER_EMPTY,
  TRANSCEIVER_RECEIVING,
  TRANSCEIVER_COMPLETE,
  TRANSCEIVER_ERROR,
  TRANSCEIVER_RESET
} TransceiverState;

typedef enum {
  TRANSCEIVER_NO_RESPONSE,
  RDM_DUB,
  RDM_WITH_RESPONSE
} TransceiverFrameType;

typedef struct {
  int size;
  TransceiverFrameType type;
  uint8_t data[DMX_FRAME_SIZE];
} TXBuffer;

typedef struct {
  Transceiver_Settings settings;  //!< The transceiver hardware settings.
  TransceiverState state;  //!< The current state of the transceiver.
  int data_index;  //!< The index into the TXBuffer's data, for sending or receiving.

  TXBuffer* tx;  //!< The Buffer that is being transmitted now
  TXBuffer* next;  //!< The next buffer ready to be transmitted
  TXBuffer* working;  //!< The Transceiver buffer we're constructing

  TXBuffer* free_list[NUMBER_OF_TX_BUFFERS];
  // The number of items on the free list.
  // Must be > 0.
  uint8_t free_size;
} TransceiverData;

TXBuffer buffers[NUMBER_OF_TX_BUFFERS];

TransceiverData g_transceiver;

int tmp_offset;

/*
 * @brief Enable TX.
 */
static inline void Transceiver_EnableTX() {
  PLIB_PORTS_PinSet(PORTS_ID_0,
                    g_transceiver.settings.port,
                    g_transceiver.settings.tx_enable_bit);
}

/*
 * @brief Disable TX.
 */
static inline void Transceiver_DisableTX() {
  PLIB_PORTS_PinClear(PORTS_ID_0,
                      g_transceiver.settings.port,
                      g_transceiver.settings.tx_enable_bit);
}

/*
 * @brief Enable RX.
 */
static inline void Transceiver_EnableRX() {
  PLIB_PORTS_PinClear(PORTS_ID_0,
                      g_transceiver.settings.port,
                      g_transceiver.settings.rx_enable_bit);
}

/*
 * @brief Disable RX.
 */
static inline void Transceiver_DisableRX() {
  PLIB_PORTS_PinSet(PORTS_ID_0,
                    g_transceiver.settings.port,
                    g_transceiver.settings.rx_enable_bit);
}

/*
 * @brief Signal the break.
 */
static inline void Transceiver_SetBreak() {
  PLIB_PORTS_PinClear(PORTS_ID_0,
                      g_transceiver.settings.port,
                      g_transceiver.settings.break_bit);
}

/*
 * @brief Signal the MAB.
 */
static inline void Transceiver_SetMark() {
  PLIB_PORTS_PinSet(PORTS_ID_0,
                    g_transceiver.settings.port,
                    g_transceiver.settings.break_bit);
}

static inline void Transceiver_LogStateChange() {
  static TransceiverState last_state = TRANSCEIVER_UNINITIALIZED;

  if (g_transceiver.state != last_state) {
    SysLog_Print(SYSLOG_INFO, "Changed to %d", g_transceiver.state);
    last_state = g_transceiver.state;
  }
}

/*
 * @brief Called when the timer expires.
 */
void __ISR(_TIMER_1_VECTOR, ipl1) Transceiver_TimerEvent() {
  // Switch uses more instructions than a simple if
  if (g_transceiver.state == TRANSCEIVER_IN_BREAK) {
    // Transition to MAB.
    Transceiver_SetMark();
    g_transceiver.state = TRANSCEIVER_IN_MARK;
    PLIB_TMR_Period16BitSet(TMR_ID_1, BREAK_TICKS);
  } else if (g_transceiver.state == TRANSCEIVER_IN_MARK) {
    // Transition to sending the data.
    g_transceiver.state = TRANSCEIVER_BEGIN_TX;
    PLIB_TMR_Stop(TMR_ID_1);
  }
  SYS_INT_SourceStatusClear(INT_SOURCE_TIMER_1);
}

/*
 * @brief Push data into the UART TX queue.
 */
void Transceiver_TXBytes() {
  while (!PLIB_USART_TransmitterBufferIsFull(g_transceiver.settings.usart) &&
         g_transceiver.data_index != g_transceiver.tx->size) {
    PLIB_USART_TransmitterByteSend(
        g_transceiver.settings.usart,
        g_transceiver.tx->data[g_transceiver.data_index++]);
  }
}

void Transceiver_FlushRX() {
  while (PLIB_USART_ReceiverDataIsAvailable(g_transceiver.settings.usart)) {
    PLIB_USART_ReceiverByteReceive(g_transceiver.settings.usart);
  }
}

/*
 * @brief Pull data out the UART RX queue.
 */
void Transceiver_RXBytes() {
  while (PLIB_USART_ReceiverDataIsAvailable(g_transceiver.settings.usart) &&
         g_transceiver.data_index != g_transceiver.tx->size) {
    g_transceiver.tx->data[g_transceiver.data_index++] =
      PLIB_USART_ReceiverByteReceive(g_transceiver.settings.usart);
  }
}

/*
 * @brief USART Interrupt handler.
 *
 * This is called for any of the following:
 *  - The USART TX buffer is empty.
 *  - The USART RX buffer has data.
 *  - A USART RX error has occurred.
 */
void __ISR(_UART_1_VECTOR, ipl6) Transceiver_UARTEvent() {
  if (SYS_INT_SourceStatusGet(INT_SOURCE_USART_1_TRANSMIT)) {
    BSP_LEDToggle(BSP_LED_2);
    if (g_transceiver.state == TRANSCEIVER_TX_BUFFER_EMPTY) {
      // The last byte has been transmitted
      SYS_INT_SourceDisable(INT_SOURCE_USART_1_TRANSMIT);
      if (g_transceiver.tx->type == TRANSCEIVER_NO_RESPONSE) {
        g_transceiver.state = TRANSCEIVER_COMPLETE;
      } else {
        // Switch to RX Mode.
        Transceiver_DisableTX();
        PLIB_USART_TransmitterDisable(g_transceiver.settings.usart);
        Transceiver_EnableRX();
        g_transceiver.state = TRANSCEIVER_RECEIVING;
        g_transceiver.data_index = 0;
        Transceiver_FlushRX();
        SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_RECEIVE);
        SYS_INT_SourceEnable(INT_SOURCE_USART_1_RECEIVE);
        SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_ERROR);
        SYS_INT_SourceEnable(INT_SOURCE_USART_1_ERROR);
        PLIB_USART_ReceiverEnable(g_transceiver.settings.usart);
      }
    } else if (g_transceiver.state == TRANSCEIVER_TX) {
      Transceiver_TXBytes();
      if (g_transceiver.data_index == g_transceiver.tx->size) {
        PLIB_USART_TransmitterInterruptModeSelect(
            g_transceiver.settings.usart, USART_TRANSMIT_FIFO_IDLE);
        g_transceiver.state = TRANSCEIVER_TX_BUFFER_EMPTY;
      }
    }
    SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_TRANSMIT);
  } else if (SYS_INT_SourceStatusGet(INT_SOURCE_USART_1_RECEIVE)) {
    BSP_LEDToggle(BSP_LED_3);
    Transceiver_RXBytes();
    SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_RECEIVE);
  } else if (SYS_INT_SourceStatusGet(INT_SOURCE_USART_1_ERROR)) {
    BSP_LEDToggle(BSP_LED_1);
    if (g_transceiver.state == TRANSCEIVER_RECEIVING) {
      PLIB_USART_ReceiverDisable(g_transceiver.settings.usart);
      // TODO(simon): change back to TX state here?
      g_transceiver.state = TRANSCEIVER_COMPLETE;
    }
    SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_ERROR);
  }
}

void Transceiver_Initialize(const Transceiver_Settings* settings) {
  int i;
  g_transceiver.settings = *settings;
  g_transceiver.state = TRANSCEIVER_UNINITIALIZED;
  g_transceiver.data_index = 0;

  g_transceiver.tx = NULL;
  g_transceiver.next = NULL;
  g_transceiver.working = NULL;

  for (i = 0; i < NUMBER_OF_TX_BUFFERS; i++) {
    g_transceiver.free_list[i] = &buffers[i];
  }
  g_transceiver.free_size = NUMBER_OF_TX_BUFFERS;

  tmp_offset = 0;

  // Setup the Break, TX Enable & RX Enable I/O Pins
  PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0,
                                   g_transceiver.settings.port,
                                   g_transceiver.settings.break_bit);
  PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0,
                                   g_transceiver.settings.port,
                                   g_transceiver.settings.tx_enable_bit);
  PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0,
                                   g_transceiver.settings.port,
                                   g_transceiver.settings.rx_enable_bit);

  Transceiver_DisableRX();
  Transceiver_EnableTX();
  Transceiver_SetMark();

  // Setup the timer
  PLIB_TMR_ClockSourceSelect (TMR_ID_1, TMR_CLOCK_SOURCE_PERIPHERAL_CLOCK );
  PLIB_TMR_PrescaleSelect(TMR_ID_1 , TMR_PRESCALE_VALUE_1);
  PLIB_TMR_Mode16BitEnable(TMR_ID_1);
  PLIB_TMR_CounterAsyncWriteDisable(TMR_ID_1);
  SYS_INT_VectorPrioritySet(INT_VECTOR_T1, INT_PRIORITY_LEVEL1);
  SYS_INT_VectorSubprioritySet(INT_VECTOR_T1, INT_SUBPRIORITY_LEVEL0);

  // Setup the UART
  PLIB_USART_BaudRateSet(g_transceiver.settings.usart,
                         SYS_CLK_PeripheralFrequencyGet(CLK_BUS_PERIPHERAL_1),
                         DMX_BAUD);
  PLIB_USART_HandshakeModeSelect(g_transceiver.settings.usart,
                                 USART_HANDSHAKE_MODE_SIMPLEX);
  PLIB_USART_OperationModeSelect(g_transceiver.settings.usart,
                                 USART_ENABLE_TX_RX_USED);
  PLIB_USART_LineControlModeSelect(g_transceiver.settings.usart, USART_8N2);
  PLIB_USART_SyncModeSelect(g_transceiver.settings.usart, USART_ASYNC_MODE);

  PLIB_USART_TransmitterInterruptModeSelect(g_transceiver.settings.usart,
                                            USART_TRANSMIT_FIFO_EMPTY);
  PLIB_USART_TransmitterInterruptModeSelect(g_transceiver.settings.usart,
                                            USART_RECEIVE_FIFO_HALF_FULL);

  SYS_INT_VectorPrioritySet(INT_VECTOR_UART1, INT_PRIORITY_LEVEL6);
  SYS_INT_VectorSubprioritySet(INT_VECTOR_UART1, INT_SUBPRIORITY_LEVEL0);
  SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_TRANSMIT);
  PLIB_USART_ReceiverDisable(g_transceiver.settings.usart);
  PLIB_USART_Enable(g_transceiver.settings.usart);
}

void Transceiver_Tasks() {
  int i;
  Transceiver_LogStateChange();

  switch (g_transceiver.state) {
    case TRANSCEIVER_UNINITIALIZED:
      g_transceiver.state = TRANSCEIVER_IDLE;
      break;
    case TRANSCEIVER_IDLE:
      if (!g_transceiver.next) {
        return;
      }
      // TODO(simon: check if the frame size > 0 here
      g_transceiver.tx = g_transceiver.next;
      g_transceiver.next = NULL;
      g_transceiver.state = TRANSCEIVER_BREAK;
      g_transceiver.data_index = 0;
      tmp_offset = 0;
      SysLog_Message(SYSLOG_INFO, "Begin Frame TX");
      // Fall through
    case TRANSCEIVER_BREAK:
      PLIB_USART_Disable(g_transceiver.settings.usart);
      // Set UART Interrupts when the buffer is empty.
      PLIB_USART_TransmitterInterruptModeSelect(g_transceiver.settings.usart,
                                                USART_TRANSMIT_FIFO_EMPTY);
      Transceiver_EnableTX();

      g_transceiver.state = TRANSCEIVER_IN_BREAK;
      PLIB_TMR_Counter16BitClear(TMR_ID_1);
      PLIB_TMR_Period16BitSet(TMR_ID_1, MARK_TICKS);
      SYS_INT_SourceStatusClear(INT_SOURCE_TIMER_1);
      SYS_INT_SourceEnable(INT_SOURCE_TIMER_1);
      Transceiver_SetMark();
      PLIB_TMR_Start(TMR_ID_1);
      break;
    case TRANSCEIVER_IN_BREAK:
    case TRANSCEIVER_IN_MARK:
      // Noop, wait for timer event
      break;
    case TRANSCEIVER_BEGIN_TX:
      Transceiver_TXBytes();
      SYS_INT_SourceEnable(INT_SOURCE_USART_1_TRANSMIT);
      PLIB_USART_Enable(g_transceiver.settings.usart);
      PLIB_USART_TransmitterEnable(g_transceiver.settings.usart);
      g_transceiver.state = TRANSCEIVER_TX;
    case TRANSCEIVER_TX:
    case TRANSCEIVER_TX_BUFFER_EMPTY:
      // Noop, wait TX to complete.
      break;
    case TRANSCEIVER_RECEIVING:
      // TODO(simon): how do we leave this state?
      if (tmp_offset != g_transceiver.data_index) {
        SysLog_Print(SYSLOG_INFO, "Received %d: %d",
                     g_transceiver.data_index,
                     g_transceiver.tx->data[g_transceiver.data_index - 1]);
        tmp_offset = g_transceiver.data_index;
      }
      break;
    case TRANSCEIVER_COMPLETE:
      SysLog_Message(SYSLOG_INFO, "485 Complete");
      if (g_transceiver.tx->type != TRANSCEIVER_NO_RESPONSE) {
        SysLog_Print(SYSLOG_INFO, "Received %d", g_transceiver.data_index - 1);
      }
      g_transceiver.free_list[g_transceiver.free_size - 1] = g_transceiver.tx;
      g_transceiver.free_size++;
      g_transceiver.tx = NULL;
      g_transceiver.state = TRANSCEIVER_IDLE;
      // TODO(simon): delay for up to MBB time.
      break;
    case TRANSCEIVER_ERROR:
      // Noop
      {}
      break;
    case TRANSCEIVER_RESET:
      g_transceiver.state = TRANSCEIVER_UNINITIALIZED;
  }
}

void Transceiver_QueueFrame(uint8_t start_code, TransceiverFrameType type,
                            const uint8_t* data, unsigned int size) {
  if (g_transceiver.working == NULL) {
    g_transceiver.working = g_transceiver.free_list[g_transceiver.free_size - 1];
    g_transceiver.free_size--;
  }
  if (size > DMX_FRAME_SIZE - 1) {
    size = DMX_FRAME_SIZE;
  } else {
    size += 1;
  }
  g_transceiver.working->size = size;
  g_transceiver.working->type = type;
  g_transceiver.working->data[0] = start_code;
  memcpy(&g_transceiver.working->data[1], data, size - 1);

  // Move working to next.
  if (g_transceiver.next) {
    g_transceiver.free_list[g_transceiver.free_size] = g_transceiver.next;
  }
  g_transceiver.next = g_transceiver.working;
}

void Transceiver_QueueDMX(const uint8_t* data, unsigned int size) {
  Transceiver_QueueFrame(NULL_START_CODE, TRANSCEIVER_NO_RESPONSE, data, size);
}

void Transceiver_QueueDUB(const uint8_t* data, unsigned int size) {
  Transceiver_QueueFrame(RDM_START_CODE, RDM_DUB, data, size);
}

void Transceiver_QueueRDMRequest(const uint8_t* data, unsigned int size) {
  Transceiver_QueueFrame(RDM_START_CODE, RDM_WITH_RESPONSE, data, size);
}

void Transceiver_Reset() {
  g_transceiver.state = TRANSCEIVER_RESET;
}
