/*
 * File:   transceiver.c
 * Author: Simon Newton
 */

#include "transceiver.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "sys/attribs.h"
#include "system/int/sys_int.h"
#include "system/clk/sys_clk.h"

#include "constants.h"
#include "coarse_timer.h"
#include "syslog.h"
#include "system_definitions.h"
#include "system_pipeline.h"
#include "peripheral/ic/plib_ic.h"
#include "peripheral/usart/plib_usart.h"
#include "peripheral/tmr/plib_tmr.h"

// The number of buffers we maintain for overlapping I/O
#define NUMBER_OF_BUFFERS 2

#define BREAK_FUDGE_FACTOR 55

#define MARK_FUDGE_FACTOR 170

#define INPUT_CAPTURE_MODULE IC_ID_2

typedef enum {
  TRANSCEIVER_UNINITIALIZED,
  TRANSCEIVER_TX_READY,  //!< Wait for a pending frame
  TRANSCEIVER_IN_BREAK,  //!< In the Break
  TRANSCEIVER_IN_MARK,  //!< In the Mark-after-break
  TRANSCEIVER_TX_DATA,  //!< Transmitting data
  TRANSCEIVER_TX_DATA_BUFFER_EMPTY,  //!< Last byte has been queued
  TRANSCEIVER_RX_WAIT_FOR_BREAK,  //!< Waiting for RX break
  TRANSCEIVER_RX_WAIT_FOR_MARK,  //!< Waiting for RX mark
  TRANSCEIVER_RX_DATA,  //!< Receiving data.
  TRANSCEIVER_RX_WAIT_FOR_DUB, //!< Waiting for DUB response
  TRANSCEIVER_RX_IN_DUB,  //!< In DUB response
  TRANSCEIVER_RX_TIMEOUT,  //!< A RX timeout occured.
  TRANSCEIVER_COMPLETE,  //!< Running the completion handler.
  TRANSCEIVER_BACKOFF,  //!< Waiting until we can send the next break
  TRANSCEIVER_RESET
} TransceiverState;

typedef struct {
  int size;
  TransceiverOperation type;
  uint8_t token;
  uint8_t data[DMX_FRAME_SIZE + 1];
} TransceiverBuffer;

typedef struct {
  Transceiver_Settings settings;  //!< The transceiver hardware settings.
  TransceiverState state;  //!< The current state of the transceiver.
  CoarseTimer_Value tx_frame_start;
  CoarseTimer_Value tx_frame_end;
  CoarseTimer_Value rx_frame_start;

  int data_index;  //!< The index into the TransceiverBuffer's data, for transmit or receiving.
  bool rx_timeout;  //!< If an RX timeout occured.
  uint8_t expected_length;  //!< If we're receiving a RDM response, this is the decoded length.
  bool found_expected_length;  //!< If expected_length is valid.

  TransceiverBuffer* active;  //!< The buffer current used for transmit / receive.
  TransceiverBuffer* next;  //!< The next buffer ready to be transmitted

  TransceiverBuffer* free_list[NUMBER_OF_BUFFERS];
  uint8_t free_size;  //!< The number of buffers in the free list, may be 0.

  // Timing params
  uint16_t break_time;
  uint16_t break_ticks;
  uint16_t mark_time;
  uint16_t mark_ticks;
  uint16_t rdm_broadcast_listen;
  uint16_t rdm_wait_time;
  uint16_t rdm_dub_response_time;
  uint16_t rdm_response_max_break_ticks;
} TransceiverData;

TransceiverBuffer buffers[NUMBER_OF_BUFFERS];

TransceiverData g_transceiver;

/*
 * @brief Convert microseconds to ticks.
 */
static inline uint16_t Transceiver_MicroSecondsToTicks(uint16_t micro_seconds) {
  return micro_seconds * (SYS_CLK_FREQ / 1000000);
}

/*
 * @brief Convert 10ths of a millisecond to ticks.
 * @param time A value between 0 and 650 (6.5mS).
 */
static inline uint16_t Transceiver_TenthOfMilliSecondsToTicks(uint16_t time) {
  return 100 * time * (SYS_CLK_FREQ / 1000000 / 8);
}

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

/*
 * @brief Put us into a MARK state
 */
static inline void Transceiver_ResetToMark() {
  Transceiver_SetMark();
  Transceiver_EnableTX();
  Transceiver_DisableRX();
}

static inline void Transceiver_LogStateChange() {
  static TransceiverState last_state = TRANSCEIVER_UNINITIALIZED;

  if (g_transceiver.state != last_state) {
    SysLog_Print(SYSLOG_INFO, "Changed to %d", g_transceiver.state);
    last_state = g_transceiver.state;
  }
}

/*
 * @brief Run the completion callback.
 */
static inline void Transceiver_FrameComplete() {
  uint8_t rc = RC_OK;
  const uint8_t* data = NULL;
  unsigned int length = 0;
  if (g_transceiver.active->type != T_OP_TRANSCEIVER_NO_RESPONSE) {
    if (g_transceiver.rx_timeout) {
      rc = T_RC_RX_TIMEOUT;
    } else if (g_transceiver.data_index) {
      // We actually got some data.
      data = g_transceiver.active->data;
      length = g_transceiver.data_index;
    }
  }

#ifdef PIPELINE_TRANSCEIVER_EVENT
  PIPELINE_TRANSCEIVER_EVENT(g_transceiver.active->token,
                             g_transceiver.active->type,
                             rc, data, length);
#else
  if (g_transceiver.settings.callback) {
    g_transceiver.settings.callback(g_transceiver.active->token,
                                    g_transceiver.active->type, rc, data,
                                    length);
  }
#endif
}

/*
 * @brief Start a period timer.
 * @param ticks The number of ticks.
 */
static inline void Transceiver_SetTimer(unsigned int ticks) {
  PLIB_TMR_Counter16BitClear(TMR_ID_1);
  PLIB_TMR_Period16BitSet(TMR_ID_1, ticks);
  SYS_INT_SourceStatusClear(INT_SOURCE_TIMER_1);
  SYS_INT_SourceEnable(INT_SOURCE_TIMER_1);
}

/*
 * @brief Setup the transceiver buffers.
 */
void Transceiver_InitializeBuffers() {
  g_transceiver.active = NULL;
  g_transceiver.next = NULL;

  int i = 0;
  for (; i < NUMBER_OF_BUFFERS; i++) {
    g_transceiver.free_list[i] = &buffers[i];
  }
  g_transceiver.free_size = NUMBER_OF_BUFFERS;
}

/*
 * @brief Reset the settings to their default values.
 */
void Transceiver_InitializeSettings() {
  Transceiver_SetBreakTime(DEFAULT_BREAK_TIME);
  Transceiver_SetMarkTime(DEFAULT_MARK_TIME);
  Transceiver_SetRDMBroadcastListen(DEFAULT_RDM_BROADCAST_LISTEN);
  Transceiver_SetRDMWaitTime(DEFAULT_RDM_WAIT_TIME);
  Transceiver_SetRDMDUBResponseTime(DEFAULT_RDM_DUB_RESPONSE_TIME);
  g_transceiver.rdm_response_max_break_ticks = Transceiver_MicroSecondsToTicks(
      DEFAULT_RDM_RESPONSE_MAX_BREAK_TIME);
}

/*
 * @brief Called when an input capture event occurs.
 */
void __ISR(_INPUT_CAPTURE_2_VECTOR, ipl6) _IntHandlerInstanceIC0(void) {
  switch (g_transceiver.state) {
    case TRANSCEIVER_RX_WAIT_FOR_DUB:
      g_transceiver.rx_frame_start = CoarseTimer_GetTime();
      SYS_INT_SourceDisable(INT_SOURCE_INPUT_CAPTURE_2);
      PLIB_IC_Disable(INPUT_CAPTURE_MODULE);
      g_transceiver.state = TRANSCEIVER_RX_IN_DUB;
      break;
    case TRANSCEIVER_RX_WAIT_FOR_BREAK:
      BSP_LEDToggle(BSP_LED_1);
      g_transceiver.rx_frame_start = CoarseTimer_GetTime();
      g_transceiver.state = TRANSCEIVER_RX_WAIT_FOR_MARK;
    case TRANSCEIVER_RX_WAIT_FOR_MARK:
      BSP_LEDToggle(BSP_LED_1);

      // TODO(simon): check that the break is long enough.

      SYS_INT_SourceDisable(INT_SOURCE_INPUT_CAPTURE_2);
      PLIB_IC_Disable(INPUT_CAPTURE_MODULE);

      // Enable UART
      SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_RECEIVE);
      SYS_INT_SourceEnable(INT_SOURCE_USART_1_RECEIVE);
      SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_ERROR);
      SYS_INT_SourceEnable(INT_SOURCE_USART_1_ERROR);
      PLIB_USART_ReceiverEnable(g_transceiver.settings.usart);
      g_transceiver.state = TRANSCEIVER_RX_DATA;
  }

  // Discard IC data for now.
  while (!PLIB_IC_BufferIsEmpty(IC_ID_2)) {
    uint32_t v = PLIB_IC_Buffer16BitGet(IC_ID_2);
    BSP_LEDToggle(BSP_LED_3);
  }
  SYS_INT_SourceStatusClear(INT_SOURCE_INPUT_CAPTURE_2);
}

/*
 * @brief Called when the timer expires.
 */
void __ISR(_TIMER_1_VECTOR, ipl6) Transceiver_TimerEvent() {
  // Switch uses more instructions than a simple if
  switch (g_transceiver.state) {
    case TRANSCEIVER_IN_BREAK:
      // Transition to MAB.
      Transceiver_SetMark();
      g_transceiver.state = TRANSCEIVER_IN_MARK;
      PLIB_TMR_Counter16BitClear(TMR_ID_1);
      PLIB_TMR_Period16BitSet(TMR_ID_1, g_transceiver.mark_ticks);
      break;
    case TRANSCEIVER_IN_MARK:
      // Stop the timer.
      SYS_INT_SourceDisable(INT_SOURCE_TIMER_1);
      PLIB_TMR_Stop(TMR_ID_1);

      // Transition to sending the data.
      // Only push a single byte into the TX queue at the begining, otherwise
      // we blow our timing budget.
      if (!PLIB_USART_TransmitterBufferIsFull(g_transceiver.settings.usart) &&
           g_transceiver.data_index != g_transceiver.active->size) {
        PLIB_USART_TransmitterByteSend(
            g_transceiver.settings.usart,
            g_transceiver.active->data[g_transceiver.data_index++]);
      }
      PLIB_USART_Enable(g_transceiver.settings.usart);
      PLIB_USART_TransmitterEnable(g_transceiver.settings.usart);
      g_transceiver.state = TRANSCEIVER_TX_DATA;
      SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_TRANSMIT);
      SYS_INT_SourceEnable(INT_SOURCE_USART_1_TRANSMIT);
      break;
  }
  SYS_INT_SourceStatusClear(INT_SOURCE_TIMER_1);
}

/*
 * @brief Push data into the UART TX queue.
 */
void Transceiver_TXBytes() {
  while (!PLIB_USART_TransmitterBufferIsFull(g_transceiver.settings.usart) &&
         g_transceiver.data_index != g_transceiver.active->size) {
    PLIB_USART_TransmitterByteSend(
        g_transceiver.settings.usart,
        g_transceiver.active->data[g_transceiver.data_index++]);
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
         g_transceiver.data_index != DMX_FRAME_SIZE) {
    g_transceiver.active->data[g_transceiver.data_index++] =
      PLIB_USART_ReceiverByteReceive(g_transceiver.settings.usart);
  }
  if (g_transceiver.active->type == T_OP_RDM_WITH_RESPONSE ||
      g_transceiver.active->type == T_OP_RDM_BROADCAST) {
    if (g_transceiver.found_expected_length) {
      if (g_transceiver.data_index == g_transceiver.expected_length) {
        // We've got enough data to move on
        PLIB_USART_ReceiverDisable(g_transceiver.settings.usart);
        Transceiver_ResetToMark();
        g_transceiver.state = TRANSCEIVER_COMPLETE;
      }
    } else {
      if (g_transceiver.data_index >= 3) {
        if (g_transceiver.active->data[0] == RDM_START_CODE &&
            g_transceiver.active->data[1] == RDM_SUB_START_CODE) {
          g_transceiver.found_expected_length = true;
          // Add two bytes for the checksum
          g_transceiver.expected_length = g_transceiver.active->data[2] + 2;
        }
      }
    }
  }
  // TODO(simon): if we've hit the buffer size here, we need to reset and
  // return an error.
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
    if (g_transceiver.state == TRANSCEIVER_TX_DATA) {
      Transceiver_TXBytes();
      if (g_transceiver.data_index == g_transceiver.active->size) {
        PLIB_USART_TransmitterInterruptModeSelect(
            g_transceiver.settings.usart, USART_TRANSMIT_FIFO_IDLE);
        g_transceiver.state = TRANSCEIVER_TX_DATA_BUFFER_EMPTY;
      }
    } else if (g_transceiver.state == TRANSCEIVER_TX_DATA_BUFFER_EMPTY) {
      // The last byte has been transmitted
      g_transceiver.tx_frame_end = CoarseTimer_GetTime();
      SYS_INT_SourceDisable(INT_SOURCE_USART_1_TRANSMIT);
      PLIB_USART_TransmitterDisable(g_transceiver.settings.usart);

      if (g_transceiver.active->type == T_OP_TRANSCEIVER_NO_RESPONSE) {
        PLIB_USART_Disable(g_transceiver.settings.usart);
        Transceiver_SetMark();
        g_transceiver.state = TRANSCEIVER_COMPLETE;
      } else {
        // Switch to RX Mode.
        if (g_transceiver.active->type == T_OP_RDM_DUB) {
          g_transceiver.state = TRANSCEIVER_RX_WAIT_FOR_DUB;
          g_transceiver.data_index = 0;

          // Turn around the line
          Transceiver_DisableTX();
          Transceiver_EnableRX();
          Transceiver_FlushRX();

          BSP_LEDToggle(BSP_LED_1);
          PLIB_IC_FirstCaptureEdgeSelect(INPUT_CAPTURE_MODULE, IC_EDGE_FALLING);
          PLIB_IC_Enable(INPUT_CAPTURE_MODULE);
          SYS_INT_SourceStatusClear(INT_SOURCE_INPUT_CAPTURE_2);
          SYS_INT_SourceEnable(INT_SOURCE_INPUT_CAPTURE_2);

          PLIB_USART_ReceiverEnable(g_transceiver.settings.usart);
          SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_RECEIVE);
          SYS_INT_SourceEnable(INT_SOURCE_USART_1_RECEIVE);
          SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_ERROR);
          SYS_INT_SourceEnable(INT_SOURCE_USART_1_ERROR);

        } else if (g_transceiver.active->type == T_OP_RDM_BROADCAST &&
                   g_transceiver.rdm_broadcast_listen == 0) {
          // Go directly to the complete state.
          g_transceiver.state = TRANSCEIVER_COMPLETE;
        } else {
          // Either T_OP_RDM_WITH_RESPONSE or a non-0 broadcast listen time.
          g_transceiver.state = TRANSCEIVER_RX_WAIT_FOR_BREAK;
          g_transceiver.data_index = 0;

          Transceiver_DisableTX();
          Transceiver_EnableRX();
          Transceiver_FlushRX();

          PLIB_IC_FirstCaptureEdgeSelect(INPUT_CAPTURE_MODULE, IC_EDGE_FALLING);
          PLIB_IC_Enable(INPUT_CAPTURE_MODULE);
          SYS_INT_SourceStatusClear(INT_SOURCE_INPUT_CAPTURE_2);
          SYS_INT_SourceEnable(INT_SOURCE_INPUT_CAPTURE_2);
        }
      }
    }
    SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_TRANSMIT);
  } else if (SYS_INT_SourceStatusGet(INT_SOURCE_USART_1_RECEIVE)) {
    switch (g_transceiver.state) {
      case TRANSCEIVER_RX_IN_DUB:
      case TRANSCEIVER_RX_DATA:
        Transceiver_RXBytes();
        break;
    }
    SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_RECEIVE);
  } else if (SYS_INT_SourceStatusGet(INT_SOURCE_USART_1_ERROR)) {
    switch (g_transceiver.state) {
      case TRANSCEIVER_RX_IN_DUB:
      case TRANSCEIVER_RX_DATA:
        SYS_INT_SourceDisable(INT_SOURCE_USART_1_RECEIVE);
        SYS_INT_SourceDisable(INT_SOURCE_USART_1_ERROR);
        PLIB_USART_ReceiverDisable(g_transceiver.settings.usart);
        Transceiver_ResetToMark();
        g_transceiver.state = TRANSCEIVER_COMPLETE;
        break;
    }
    SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_ERROR);
  }
}

void Transceiver_Initialize(const Transceiver_Settings* settings) {
  g_transceiver.settings = *settings;
  g_transceiver.state = TRANSCEIVER_UNINITIALIZED;
  g_transceiver.data_index = 0;

  Transceiver_InitializeBuffers();
  Transceiver_InitializeSettings();

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

  Transceiver_ResetToMark();

  // Setup the timer
  PLIB_TMR_ClockSourceSelect(TMR_ID_1, TMR_CLOCK_SOURCE_PERIPHERAL_CLOCK);
  PLIB_TMR_PrescaleSelect(TMR_ID_1, TMR_PRESCALE_VALUE_1);
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

  SYS_INT_VectorPrioritySet(INT_VECTOR_UART1, INT_PRIORITY_LEVEL6);
  SYS_INT_VectorSubprioritySet(INT_VECTOR_UART1, INT_SUBPRIORITY_LEVEL0);
  SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_TRANSMIT);
  PLIB_USART_ReceiverDisable(g_transceiver.settings.usart);
  PLIB_USART_Enable(g_transceiver.settings.usart);

  // Setup input capture
  PLIB_IC_Disable(INPUT_CAPTURE_MODULE);
  // TODO(simon): capture on TMR 2 for now since it's always running
  PLIB_IC_ModeSelect(INPUT_CAPTURE_MODULE, IC_INPUT_CAPTURE_EVERY_EDGE_MODE);
  PLIB_IC_FirstCaptureEdgeSelect(INPUT_CAPTURE_MODULE, IC_EDGE_RISING);
  PLIB_IC_TimerSelect(INPUT_CAPTURE_MODULE, IC_TIMER_TMR2);
  PLIB_IC_BufferSizeSelect(INPUT_CAPTURE_MODULE, IC_BUFFER_SIZE_16BIT);
  PLIB_IC_EventsPerInterruptSelect(INPUT_CAPTURE_MODULE,
                                   IC_INTERRUPT_ON_EVERY_CAPTURE_EVENT);

  SYS_INT_VectorPrioritySet(INT_VECTOR_IC2, INT_PRIORITY_LEVEL6);
  SYS_INT_VectorSubprioritySet(INT_VECTOR_IC2, INT_SUBPRIORITY_LEVEL0);
}

void Transceiver_Tasks() {
  bool ok;
  Transceiver_LogStateChange();

  switch (g_transceiver.state) {
    case TRANSCEIVER_UNINITIALIZED:
      g_transceiver.state = TRANSCEIVER_TX_READY;
      PLIB_USART_Disable(g_transceiver.settings.usart);
      Transceiver_EnableTX();
      // TODO(simon): Reset to mark here?
      break;
    case TRANSCEIVER_TX_READY:
      if (!g_transceiver.next) {
        return;
      }
      // TODO(simon): confirm this is all true
      // @pre Timer is not running.
      // @pre UART is disabled
      // @pre TX is enabled.
      // @pre RX is disabled.
      // @pre RX CN is disabled.
      // @pre line in marking state

      // Dequeue frame and set break.
      g_transceiver.active = g_transceiver.next;
      g_transceiver.next = NULL;

      // Reset params
      g_transceiver.data_index = 0;
      g_transceiver.found_expected_length = false;
      g_transceiver.expected_length = 0;
      g_transceiver.rx_timeout = false;

      // Prepare the UART
      // Set UART Interrupts when the buffer is empty.
      PLIB_USART_TransmitterInterruptModeSelect(g_transceiver.settings.usart,
                                                USART_TRANSMIT_FIFO_EMPTY);

      // Set break and start timer.
      g_transceiver.state = TRANSCEIVER_IN_BREAK;
      PLIB_TMR_PrescaleSelect(TMR_ID_1 , TMR_PRESCALE_VALUE_1);
      g_transceiver.tx_frame_start = CoarseTimer_GetTime();
      Transceiver_SetTimer(g_transceiver.break_ticks);
      Transceiver_SetBreak();
      PLIB_TMR_Start(TMR_ID_1);

    case TRANSCEIVER_IN_BREAK:
    case TRANSCEIVER_IN_MARK:
      // Noop, wait for timer event
      break;
    case TRANSCEIVER_TX_DATA:
    case TRANSCEIVER_TX_DATA_BUFFER_EMPTY:
      // Noop, wait TX to complete.
      break;

    case TRANSCEIVER_RX_WAIT_FOR_BREAK:
      if (CoarseTimer_HasElapsed(g_transceiver.tx_frame_end,
                                 g_transceiver.rdm_wait_time)) {
        SYS_INT_SourceDisable(INT_SOURCE_INPUT_CAPTURE_2);
        // Note: the IC ISR may have run between the case check and the
        // SourceDisable and switched us to TRANSCEIVER_RX_WAIT_FOR_MARK.
        SYS_INT_SourceDisable(INT_SOURCE_USART_1_RECEIVE);
        SYS_INT_SourceDisable(INT_SOURCE_USART_1_ERROR);
        PLIB_IC_Disable(INPUT_CAPTURE_MODULE);
        PLIB_USART_ReceiverDisable(g_transceiver.settings.usart);
        Transceiver_ResetToMark();
        g_transceiver.state = TRANSCEIVER_RX_TIMEOUT;
      }
      break;

    case TRANSCEIVER_RX_WAIT_FOR_MARK:
      // TODO(simon): handle the timeout case here.
      // If the break is longer than 352uS it's out of spec.
      // Use rdm_response_max_break_ticks ?
      break;

    case TRANSCEIVER_RX_DATA:
      // TODO(simon): handle the timeout case here.
      // It's not a static timeout, rather it varies with the slot count.
      break;

    case TRANSCEIVER_RX_WAIT_FOR_DUB:
      if (CoarseTimer_HasElapsed(g_transceiver.tx_frame_end,
                                 g_transceiver.rdm_wait_time)) {
        SYS_INT_SourceDisable(INT_SOURCE_INPUT_CAPTURE_2);
        // Note: the IC ISR may have run between the case check and the
        // SourceDisable and switched us to TRANSCEIVER_RX_IN_DUB.
        SYS_INT_SourceDisable(INT_SOURCE_USART_1_RECEIVE);
        SYS_INT_SourceDisable(INT_SOURCE_USART_1_ERROR);
        PLIB_IC_Disable(INPUT_CAPTURE_MODULE);
        PLIB_USART_ReceiverDisable(g_transceiver.settings.usart);
        Transceiver_ResetToMark();
        g_transceiver.state = TRANSCEIVER_RX_TIMEOUT;
      }
      break;
    case TRANSCEIVER_RX_IN_DUB:
      if (CoarseTimer_HasElapsed(g_transceiver.rx_frame_start,
                                 g_transceiver.rdm_dub_response_time)) {
        // The UART Error interupt may have fired, putting us into
        // TRANSCEIVER_COMPLETE, already.
        SYS_INT_SourceDisable(INT_SOURCE_USART_1_RECEIVE);
        SYS_INT_SourceDisable(INT_SOURCE_USART_1_ERROR);
        PLIB_USART_ReceiverDisable(g_transceiver.settings.usart);
        Transceiver_ResetToMark();
        // We got at least a falling edge, so this should probably be
        // considered a collision, rather than a timeout.
        g_transceiver.state = TRANSCEIVER_COMPLETE;
      }
      break;

    case TRANSCEIVER_RX_TIMEOUT:
      SysLog_Message(SYSLOG_INFO, "RX timeout");
      g_transceiver.state = TRANSCEIVER_COMPLETE;
      g_transceiver.rx_timeout = true;
      break;
    case TRANSCEIVER_COMPLETE:
      SysLog_Print(SYSLOG_INFO, "TX: %d",
                   CoarseTimer_Delta(g_transceiver.tx_frame_start,
                                     g_transceiver.tx_frame_end));
      SysLog_Print(SYSLOG_INFO, "Diff: %d",
                   CoarseTimer_Delta(g_transceiver.tx_frame_end,
                                     g_transceiver.rx_frame_start));
      Transceiver_FrameComplete();
      g_transceiver.state = TRANSCEIVER_BACKOFF;
      // Fall through
    case TRANSCEIVER_BACKOFF:
      BSP_LEDToggle(BSP_LED_2);
      // From E1.11, the min break-to-break time is 1.204ms.
      //
      // From E1.20:
      //  - If DUB, the min EOF to break is 5.8ms
      //  - If bcast, the min EOF to break is 0.176ms
      //  - If lost response, the min EOF to break is 3.0ms
      ok = CoarseTimer_HasElapsed(g_transceiver.tx_frame_start, 13);

      switch (g_transceiver.active->type) {
        case T_OP_TRANSCEIVER_NO_RESPONSE:
          break;
        case T_OP_RDM_DUB:
          // It would be nice to be able to reduce this if we didn't get a
          // response, but the standard doesn't allow this.
          ok &= CoarseTimer_HasElapsed(g_transceiver.tx_frame_end, 58);
          break;
        case T_OP_RDM_BROADCAST:
          ok &= CoarseTimer_HasElapsed(g_transceiver.tx_frame_end, 2);
          break;
        case T_OP_RDM_WITH_RESPONSE:
          // TODO(simon): We can probably make this faster, since the 3ms only
          // applies for no responses. If we do get a response, then it's only
          // a 0.176ms delay, from the end of the responder frame.
          ok &= CoarseTimer_HasElapsed(g_transceiver.tx_frame_end, 30);
          break;
      }

      if (ok) {
        g_transceiver.free_list[g_transceiver.free_size] = g_transceiver.active;
        g_transceiver.free_size++;
        g_transceiver.active = NULL;
        g_transceiver.state = TRANSCEIVER_TX_READY;
      }
      break;
    case TRANSCEIVER_RESET:
      g_transceiver.state = TRANSCEIVER_UNINITIALIZED;
  }
}

bool Transceiver_QueueFrame(uint8_t token, uint8_t start_code,
                            TransceiverOperation type, const uint8_t* data,
                            unsigned int size) {
  if (g_transceiver.free_size == 0) {
    return false;
  }

  g_transceiver.next = g_transceiver.free_list[g_transceiver.free_size - 1];
  g_transceiver.free_size--;

  if (size > DMX_FRAME_SIZE) {
    size = DMX_FRAME_SIZE;
  }
  g_transceiver.next->size = size + 1;  // include start code.
  g_transceiver.next->type = type;
  g_transceiver.next->token = token;
  g_transceiver.next->data[0] = start_code;
  SysLog_Print(SYSLOG_INFO, "Start code %d", start_code);
  memcpy(&g_transceiver.next->data[1], data, size);
  return true;
}

bool Transceiver_QueueDMX(uint8_t token, const uint8_t* data,
                          unsigned int size) {
  return Transceiver_QueueFrame(
      token, NULL_START_CODE, T_OP_TRANSCEIVER_NO_RESPONSE, data, size);
}

bool Transceiver_QueueASC(uint8_t token, uint8_t start_code,
                          const uint8_t* data, unsigned int size) {
  return Transceiver_QueueFrame(
      token, start_code, T_OP_TRANSCEIVER_NO_RESPONSE, data, size);
}

bool Transceiver_QueueRDMDUB(uint8_t token, const uint8_t* data,
                             unsigned int size) {
  return Transceiver_QueueFrame(
      token, RDM_START_CODE, T_OP_RDM_DUB,
      data, size);
}

bool Transceiver_QueueRDMRequest(uint8_t token, const uint8_t* data,
                                 unsigned int size, bool is_broadcast) {
  return Transceiver_QueueFrame(
      token, RDM_START_CODE,
      is_broadcast ? T_OP_RDM_BROADCAST : T_OP_RDM_WITH_RESPONSE,
      data, size);
}

void Transceiver_Reset() {
  SYS_INT_SourceDisable(INT_SOURCE_USART_1_TRANSMIT);
  SYS_INT_SourceDisable(INT_SOURCE_USART_1_RECEIVE);
  SYS_INT_SourceDisable(INT_SOURCE_USART_1_ERROR);
  SYS_INT_SourceDisable(INT_SOURCE_TIMER_1);

  Transceiver_InitializeBuffers();

  // Set us back into the TX Mark state.
  PLIB_USART_ReceiverDisable(g_transceiver.settings.usart);
  PLIB_USART_TransmitterDisable(g_transceiver.settings.usart);
  Transceiver_ResetToMark();
  Transceiver_InitializeSettings();

  g_transceiver.state = TRANSCEIVER_RESET;
}

bool Transceiver_SetBreakTime(uint16_t break_time_us) {
  if (break_time_us < 44 || break_time_us > 800) {
    return false;
  }
  g_transceiver.break_time = break_time_us;
  uint16_t ticks = Transceiver_MicroSecondsToTicks(break_time_us);
  g_transceiver.break_ticks = ticks - BREAK_FUDGE_FACTOR;
  SysLog_Print(SYSLOG_INFO, "Break ticks is %d", ticks);
  return true;
}

uint16_t Transceiver_GetBreakTime() {
  return g_transceiver.break_time;
}

bool Transceiver_SetMarkTime(uint16_t mark_time_us) {
  if (mark_time_us < 4 || mark_time_us > 800) {
    return false;
  }
  g_transceiver.mark_time = mark_time_us;
  uint16_t ticks = Transceiver_MicroSecondsToTicks(mark_time_us);
  g_transceiver.mark_ticks = ticks - MARK_FUDGE_FACTOR;
  SysLog_Print(SYSLOG_INFO, "MAB ticks is %d", ticks);
  return true;
}

uint16_t Transceiver_GetMarkTime() {
  return g_transceiver.mark_time;
}

bool Transceiver_SetRDMBroadcastListen(uint16_t delay) {
  if (delay > 50) {
    return false;
  }
  // TODO(simon): actually implement this.
  g_transceiver.rdm_broadcast_listen = delay;
  SysLog_Print(SYSLOG_INFO, "Bcast listen is %d",
               g_transceiver.rdm_broadcast_listen);
  return true;
}

uint16_t Transceiver_GetRDMBroadcastListen() {
  return g_transceiver.rdm_broadcast_listen;
}

bool Transceiver_SetRDMWaitTime(uint16_t wait_time) {
  if (wait_time < 10 || wait_time > 50) {
    return false;
  }
  g_transceiver.rdm_wait_time = wait_time;
  return true;
}

uint16_t Transceiver_GetRDMWaitTime() {
  return g_transceiver.rdm_wait_time;
}

bool Transceiver_SetRDMDUBResponseTime(uint16_t wait_time) {
  if (wait_time < 10 || wait_time > 50) {
    return false;
  }
  g_transceiver.rdm_dub_response_time = wait_time;
  return true;
}

uint16_t Transceiver_GetRDMDUBResponseTime() {
  return g_transceiver.rdm_dub_response_time;
}
