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
#include "peripheral/usart/plib_usart.h"
#include "peripheral/tmr/plib_tmr.h"

// The number of buffers we maintain for overlapping I/O
#define NUMBER_OF_BUFFERS 2

#define BREAK_FUDGE_FACTOR 55

#define MARK_FUDGE_FACTOR 170

typedef enum {
  TRANSCEIVER_UNINITIALIZED,
  TRANSCEIVER_READY,
  TRANSCEIVER_BREAK,
  TRANSCEIVER_IN_BREAK,
  TRANSCEIVER_IN_MARK,
  TRANSCEIVER_BEGIN_TX,
  TRANSCEIVER_TX,
  TRANSCEIVER_TX_BUFFER_EMPTY,
  TRANSCEIVER_RECEIVING,
  TRANSCEIVER_COMPLETE,
  TRANSCEIVER_IDLE,
  TRANSCEIVER_ERROR,
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
  CoarseTimer_Value last_break_time;
  int data_index;  //!< The index into the TransceiverBuffer's data, for transmit or receiving.
  bool rx_timeout;  //!< If an RX timeout occured.
  bool rx_got_break;  //!< If we've seen the break for a RDM response.
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
  uint16_t rdm_broadcast_listen_ticks;
  uint16_t rdm_wait_time;
  uint16_t rdm_wait_time_ticks;
  uint16_t rdm_dub_response_time;
  uint16_t rdm_dub_response_ticks;
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

/**
 * @brief Start a period timer.
 * @param ticks The number of ticks.
 */
static inline void Transceiver_StartTimer(unsigned int ticks) {
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
 * @brief Called when the timer expires.
 */
void __ISR(_TIMER_1_VECTOR, ipl6) Transceiver_TimerEvent() {
  // Switch uses more instructions than a simple if
  if (g_transceiver.state == TRANSCEIVER_IN_BREAK) {
    // Transition to MAB.
    Transceiver_SetMark();
    g_transceiver.state = TRANSCEIVER_IN_MARK;
    PLIB_TMR_Counter16BitClear(TMR_ID_1);
    PLIB_TMR_Period16BitSet(TMR_ID_1, g_transceiver.mark_ticks);
  } else if (g_transceiver.state == TRANSCEIVER_IN_MARK) {
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
    g_transceiver.state = TRANSCEIVER_TX;
    SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_TRANSMIT);
    SYS_INT_SourceEnable(INT_SOURCE_USART_1_TRANSMIT);
    PLIB_TMR_Stop(TMR_ID_1);
  } else if (g_transceiver.state == TRANSCEIVER_RECEIVING) {
    // BSP_LEDToggle(BSP_LED_1);
    // Timeout
    // TODO(simon): move this into Transceiver_ResetToMark() and clear the
    // interupts to avoid the race.
    PLIB_USART_ReceiverDisable(g_transceiver.settings.usart);
    Transceiver_ResetToMark();
    PLIB_TMR_Stop(TMR_ID_1);
    g_transceiver.state = TRANSCEIVER_COMPLETE;
    g_transceiver.rx_timeout = g_transceiver.data_index == 0;
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
    // BSP_LEDToggle(BSP_LED_1);
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
  // BSP_LEDToggle(BSP_LED_1);
  if (SYS_INT_SourceStatusGet(INT_SOURCE_USART_1_TRANSMIT)) {
    if (g_transceiver.state == TRANSCEIVER_TX_BUFFER_EMPTY) {
      // The last byte has been transmitted
      SYS_INT_SourceDisable(INT_SOURCE_USART_1_TRANSMIT);
      if (g_transceiver.active->type == T_OP_TRANSCEIVER_NO_RESPONSE) {
        PLIB_USART_TransmitterDisable(g_transceiver.settings.usart);
        Transceiver_SetMark();
        g_transceiver.state = TRANSCEIVER_COMPLETE;
      } else {
        // Switch to RX Mode.
        g_transceiver.last_break_time = CoarseTimer_GetTime();
        Transceiver_DisableTX();
        PLIB_USART_TransmitterDisable(g_transceiver.settings.usart);
        Transceiver_EnableRX();
        g_transceiver.state = TRANSCEIVER_RECEIVING;
        g_transceiver.data_index = 0;
        g_transceiver.rx_timeout = false;
        g_transceiver.rx_got_break = false;
        Transceiver_FlushRX();
        SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_RECEIVE);
        SYS_INT_SourceEnable(INT_SOURCE_USART_1_RECEIVE);
        SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_ERROR);
        SYS_INT_SourceEnable(INT_SOURCE_USART_1_ERROR);
        PLIB_USART_ReceiverEnable(g_transceiver.settings.usart);

        // Setup RX timer
        PLIB_TMR_PrescaleSelect(TMR_ID_1, TMR_PRESCALE_VALUE_8);
        Transceiver_StartTimer(g_transceiver.rdm_wait_time_ticks);
        PLIB_TMR_Start(TMR_ID_1);
      }
    } else if (g_transceiver.state == TRANSCEIVER_TX) {
      Transceiver_TXBytes();
      if (g_transceiver.data_index == g_transceiver.active->size) {
        PLIB_USART_TransmitterInterruptModeSelect(
            g_transceiver.settings.usart, USART_TRANSMIT_FIFO_IDLE);
        g_transceiver.state = TRANSCEIVER_TX_BUFFER_EMPTY;
      }
    }
    SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_TRANSMIT);
  } else if (SYS_INT_SourceStatusGet(INT_SOURCE_USART_1_RECEIVE)) {
    // BSP_LEDToggle(BSP_LED_2);
    if (g_transceiver.active->type == T_OP_RDM_DUB) {
      if (g_transceiver.data_index == 0) {
        Transceiver_StartTimer(g_transceiver.rdm_dub_response_ticks);
        PLIB_TMR_Start(TMR_ID_1);
      }
      Transceiver_RXBytes();
    } else if (g_transceiver.active->type == T_OP_RDM_WITH_RESPONSE ||
               g_transceiver.active->type == T_OP_RDM_BROADCAST) {
      if (g_transceiver.rx_got_break) {
        if (g_transceiver.data_index == 0) {
          // Stop the break timer.
          PLIB_TMR_Stop(TMR_ID_1);
        }
        Transceiver_RXBytes();
      } else {
        // Discard any data until we get a break.
        while (PLIB_USART_ReceiverDataIsAvailable(g_transceiver.settings.usart)) {
          PLIB_USART_ReceiverByteReceive(g_transceiver.settings.usart);
        }
      }
    }
    SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_RECEIVE);
  } else if (SYS_INT_SourceStatusGet(INT_SOURCE_USART_1_ERROR)) {
    // BSP_LEDToggle(BSP_LED_3);
    if (g_transceiver.state == TRANSCEIVER_RECEIVING) {
      if (g_transceiver.active->type == T_OP_RDM_DUB) {
        // End of the response.
        PLIB_USART_ReceiverDisable(g_transceiver.settings.usart);
        Transceiver_ResetToMark();
        g_transceiver.state = TRANSCEIVER_COMPLETE;
      } else if (g_transceiver.active->type == T_OP_RDM_WITH_RESPONSE) {
        if (g_transceiver.rx_got_break) {
          // End of the response
          PLIB_USART_ReceiverDisable(g_transceiver.settings.usart);
          Transceiver_ResetToMark();
          g_transceiver.state = TRANSCEIVER_COMPLETE;
        } else {
          // In break, stop the timer, and start a max-break timer.
          PLIB_TMR_Stop(TMR_ID_1);
          PLIB_TMR_Counter16BitClear(TMR_ID_1);
          PLIB_TMR_PrescaleSelect(TMR_ID_1 , TMR_PRESCALE_VALUE_1);
          PLIB_TMR_Period16BitSet(TMR_ID_1,
                                  g_transceiver.rdm_response_max_break_ticks);
          PLIB_TMR_Start(TMR_ID_1);
          // BSP_LEDToggle(BSP_LED_1);
          g_transceiver.rx_got_break = true;
        }
      }
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
  // PLIB_USART_TransmitterInterruptModeSelect(g_transceiver.settings.usart,
  //                                           USART_RECEIVE_FIFO_HALF_FULL);

  SYS_INT_VectorPrioritySet(INT_VECTOR_UART1, INT_PRIORITY_LEVEL6);
  SYS_INT_VectorSubprioritySet(INT_VECTOR_UART1, INT_SUBPRIORITY_LEVEL0);
  SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_TRANSMIT);
  PLIB_USART_ReceiverDisable(g_transceiver.settings.usart);
  PLIB_USART_Enable(g_transceiver.settings.usart);
}

void Transceiver_Tasks() {
  Transceiver_LogStateChange();

  switch (g_transceiver.state) {
    case TRANSCEIVER_UNINITIALIZED:
      g_transceiver.state = TRANSCEIVER_READY;
      break;
    case TRANSCEIVER_READY:
      if (!g_transceiver.next) {
        return;
      }
      g_transceiver.active = g_transceiver.next;
      g_transceiver.next = NULL;
      g_transceiver.state = TRANSCEIVER_BREAK;
      g_transceiver.data_index = 0;
      g_transceiver.found_expected_length = false;
      g_transceiver.expected_length = 0;
      // Fall through
    case TRANSCEIVER_BREAK:
      PLIB_USART_Disable(g_transceiver.settings.usart);
      // Set UART Interrupts when the buffer is empty.
      PLIB_USART_TransmitterInterruptModeSelect(g_transceiver.settings.usart,
                                                USART_TRANSMIT_FIFO_EMPTY);
      Transceiver_EnableTX();
      g_transceiver.state = TRANSCEIVER_IN_BREAK;
      PLIB_TMR_PrescaleSelect(TMR_ID_1 , TMR_PRESCALE_VALUE_1);
      Transceiver_StartTimer(g_transceiver.break_ticks);
      Transceiver_SetBreak();
      PLIB_TMR_Start(TMR_ID_1);
      break;
    case TRANSCEIVER_IN_BREAK:
    case TRANSCEIVER_IN_MARK:
      // Noop, wait for timer event
      break;
    case TRANSCEIVER_BEGIN_TX:
    case TRANSCEIVER_TX:
    case TRANSCEIVER_TX_BUFFER_EMPTY:
      // Noop, wait TX to complete.
      break;
    case TRANSCEIVER_RECEIVING:
      // Noop, wait for RX or timeout.
      break;
    case TRANSCEIVER_COMPLETE:
      Transceiver_FrameComplete();
      g_transceiver.free_list[g_transceiver.free_size] = g_transceiver.active;
      g_transceiver.free_size++;
      g_transceiver.active = NULL;
      g_transceiver.state = TRANSCEIVER_IDLE;
      // Fall through
    case TRANSCEIVER_IDLE:
      // TODO(simon): The break for the next frame can start immediately,
      // but the catch is the break-to-break time can't be less than 1.204ms
      // This means we may need to add a delay here if the frame is less than a
      // certain size.
      // TODO(simon): This is setup for RDM DUB right now. Fix me
      if (CoarseTimer_HasElapsed(g_transceiver.last_break_time, 58)) {
        g_transceiver.state = TRANSCEIVER_READY;
      }
      break;
    case TRANSCEIVER_ERROR:
      // This isn't used yet.
      // TODO(simon): think about what errors could lead to this state.
      // Noop
      {}
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
  g_transceiver.rdm_wait_time_ticks = Transceiver_TenthOfMilliSecondsToTicks(
      wait_time);
  SysLog_Print(SYSLOG_INFO, "Wait time ticks is %d",
               g_transceiver.rdm_wait_time_ticks);
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
  g_transceiver.rdm_dub_response_ticks = Transceiver_TenthOfMilliSecondsToTicks(
      wait_time);
  SysLog_Print(SYSLOG_INFO, "DUB Response ticks is %d",
               g_transceiver.rdm_dub_response_ticks);
  return true;
}

uint16_t Transceiver_GetRDMDUBResponseTime() {
  return g_transceiver.rdm_dub_response_time;
}
