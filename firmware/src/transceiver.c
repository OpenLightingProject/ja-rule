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
 * transceiver.c
 * Copyright (C) 2015 Simon Newton
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
#include "dmx_spec.h"
#include "syslog.h"
#include "system_definitions.h"
#include "system_pipeline.h"
#include "peripheral/ic/plib_ic.h"
#include "peripheral/usart/plib_usart.h"
#include "peripheral/tmr/plib_tmr.h"
#include "transceiver_timing.h"
#include "random.h"

#define INPUT_CAPTURE_MODULE IC_ID_2
#define BUFFER_SIZE (DMX_FRAME_SIZE + 1u)

// The number of buffers we maintain for overlapping I/O
enum { NUMBER_OF_BUFFERS = 2};

static const uint16_t BREAK_FUDGE_FACTOR = 74u;
static const uint16_t MARK_FUDGE_FACTOR = 217u;
static const uint16_t RESPONSE_FUDGE_FACTOR = 24u;

typedef enum {
  // Controller states
  STATE_C_INITIALIZE = 0,  //!< Initialize controller state.
  STATE_C_TX_READY = 1,  //!< Wait for a pending frame
  STATE_C_IN_BREAK = 2,  //!< In the Break
  STATE_C_IN_MARK = 3,  //!< In the Mark-after-break
  STATE_C_TX_DATA = 4,  //!< Transmitting data
  STATE_C_TX_DRAIN = 5,  //!< Wait for last byte to be sent
  STATE_C_RX_WAIT_FOR_BREAK = 6,  //!< Waiting for RX break
  STATE_C_RX_WAIT_FOR_MARK = 7,  //!< Waiting for RX mark
  STATE_C_RX_DATA = 8,  //!< Receiving data.
  STATE_C_RX_WAIT_FOR_DUB = 9,  //!< Waiting for DUB response
  STATE_C_RX_IN_DUB = 10,  //!< In DUB response
  STATE_C_RX_TIMEOUT = 11,  //!< A RX timeout occured.
  STATE_C_COMPLETE = 12,  //!< Running the completion handler.
  STATE_C_BACKOFF = 13,  //!< Waiting until we can send the next break

  // Responder states.
  STATE_R_INITIALIZE = 14,  //!< Initialze responder state
  STATE_R_RX_PREPARE = 15,  //!< Prepare to receive frame
  STATE_R_RX_MBB = 16,  //!< In mark before break
  STATE_R_RX_BREAK = 17,  //!< In break
  STATE_R_RX_MARK = 18,  //!< In mark after break
  STATE_R_RX_DATA = 19,  //!< Receiving data
  STATE_R_TX_WAITING = 20,  //!< Delay before response
  STATE_R_TX_BREAK = 21,  //!< In TX Break
  STATE_R_TX_MARK = 22,  //!< In TX Mark
  STATE_R_TX_DATA = 23,  //!< Transmitting data.
  STATE_R_TX_DRAIN = 24,  //!< Wait for last byte to be sent.
  STATE_R_TX_COMPLETE = 25,  //!< Response complete

  // Common states
  STATE_RESET = 99,
  STATE_ERROR = 100
} TransceiverState;

/*
 * @brief
 */
typedef enum {
  OP_TX_ONLY = T_OP_TX_ONLY,
  OP_RDM_DUB = T_OP_RDM_DUB,
  OP_RDM_BROADCAST = T_OP_RDM_BROADCAST,
  OP_RDM_WITH_RESPONSE = T_OP_RDM_WITH_RESPONSE,
  OP_RX = T_OP_RX,
  OP_RDM_DUB_RESPONSE,  //!< No break
  OP_RDM_RESEPONSE  //!< With a break
} InternalOperation;

typedef struct {
  uint16_t size;
  InternalOperation op;
  uint8_t token;
  uint8_t data[BUFFER_SIZE];
} TransceiverBuffer;

typedef struct {
  TransceiverState state;  //!< The current state of the transceiver.
  TransceiverMode mode;  //!< The operating mode of the transceiver.
  TransceiverMode desired_mode;  //!< The mode we'd like to be operating in.

  /**
   * @brief Stores the approximate time of the start of the outgoing frame.
   */
  CoarseTimer_Value tx_frame_start;

  /**
   * @brief Stores the approximate time of the end of the outgoing frame.
   */
  CoarseTimer_Value tx_frame_end;

  /**
   * @brief The time to wait for the RDM response.
   *
   * This is set to either g_timing_settings.rdm_response_timeout or
   * g_timing_settings.rdm_broadcast_timeout depending on the type of request.
   */
  uint16_t rdm_response_timeout;

  /**
   * @brief The index into the TransceiverBuffer's data, for transmit or
   * receiving.
   */
  uint16_t data_index;

  /**
   * @brief The index of the last byte delivered to the responder callback.
   */
  uint16_t event_index;

  /**
   * @brief The time of the last level change.
   */
  uint16_t last_change;

  /**
   * @brief The approximate time the last byte arrived.
   */
  uint16_t last_byte;

  /**
   * @brief The approximate time the last byte arrived, accurate to 10ths of a
   * millisecond.
   */
  CoarseTimer_Value last_byte_coarse;

  /**
   * @brief The result of the last operation.
   */
  TransceiverOperationResult result;

  /**
   * @brief If we're receiving a RDM response, this is the decoded length.
   */
  uint8_t expected_length;
  bool found_expected_length;  //!< If expected_length is valid.

  /**
   * @brief The buffer current used for transmit / receive.
   */
  TransceiverBuffer* active;
  TransceiverBuffer* next;  //!< The next buffer ready to be transmitted

  TransceiverBuffer* free_list[NUMBER_OF_BUFFERS];
  uint8_t free_size;  //!< The number of buffers in the free list, may be 0.
} TransceiverData;

typedef struct {
  // Timing params
  uint16_t break_time;
  uint16_t break_ticks;
  uint16_t mark_time;
  uint16_t mark_ticks;
  uint16_t rdm_broadcast_timeout;
  uint16_t rdm_response_timeout;
  uint16_t rdm_dub_response_limit;
  uint16_t rdm_responder_delay;
  uint16_t rdm_responder_jitter;
} TimingSettings;

// The TX / RX buffers
static TransceiverBuffer buffers[NUMBER_OF_BUFFERS];

// The transceiver state
TransceiverData g_transceiver;

// The hardware settings
static TransceiverHardwareSettings g_hw_settings;

// The timing information for the current operation.
static TransceiverTiming g_timing;

// The event callback, or NULL if there isn't one.
static TransceiverEventCallback g_tx_callback = NULL;
static TransceiverEventCallback g_rx_callback = NULL;

// The timing settings
static TimingSettings g_timing_settings;

// Timer Functions
// ----------------------------------------------------------------------------
/*
 * @brief Convert microseconds to ticks.
 */
static inline uint16_t MicroSecondsToTicks(uint16_t micro_seconds) {
  return micro_seconds * (SYS_CLK_FREQ / 1000000u);
}

/*
 * @brief Rebase the timer to the last input change event.
 *
 * This is used to set the timer value such that the timer would have started
 * when the last event occurred. We use this to time packets, since often we
 * don't know what's a break until after the event.
 */
static inline void RebaseTimer(uint16_t last_event) {
  PLIB_TMR_Counter16BitSet(TMR_ID_3,
                           PLIB_TMR_Counter16BitGet(TMR_ID_3) - last_event);
}

// I/O Functions
// ----------------------------------------------------------------------------

/*
 * @brief Switch the transceiver to TX mode.
 */
static inline void EnableTX() {
  PLIB_PORTS_PinSet(PORTS_ID_0,
                    g_hw_settings.port,
                    g_hw_settings.tx_enable_bit);
  PLIB_PORTS_PinSet(PORTS_ID_0,
                    g_hw_settings.port,
                    g_hw_settings.rx_enable_bit);
}

/*
 * @brief Switch the transceiver to RX mode.
 */
static inline void EnableRX() {
  PLIB_PORTS_PinClear(PORTS_ID_0,
                      g_hw_settings.port,
                      g_hw_settings.rx_enable_bit);
  PLIB_PORTS_PinClear(PORTS_ID_0,
                      g_hw_settings.port,
                      g_hw_settings.tx_enable_bit);
}

/*
 * @brief Set the line to a break.
 */
static inline void SetBreak() {
  PLIB_PORTS_PinClear(PORTS_ID_0,
                      g_hw_settings.port,
                      g_hw_settings.break_bit);
}

/*
 * @brief Set the line to a mark.
 */
static inline void SetMark() {
  PLIB_PORTS_PinSet(PORTS_ID_0,
                    g_hw_settings.port,
                    g_hw_settings.break_bit);
}

/*
 * @brief Put us into a MARK state
 */
static inline void ResetToMark() {
  SetMark();
  EnableTX();
}

// UART Helpers
// ----------------------------------------------------------------------------
/*
 * @brief Push data into the UART TX queue.
 */
static void UART_TXBytes() {
  while (!PLIB_USART_TransmitterBufferIsFull(g_hw_settings.usart) &&
         g_transceiver.data_index != g_transceiver.active->size) {
    PLIB_USART_TransmitterByteSend(
        g_hw_settings.usart,
        g_transceiver.active->data[g_transceiver.data_index]);
    g_transceiver.data_index++;
  }
}

void UART_FlushRX() {
  while (PLIB_USART_ReceiverDataIsAvailable(g_hw_settings.usart)) {
    PLIB_USART_ReceiverByteReceive(g_hw_settings.usart);
  }
}

/*
 * @brief Pull data out of the UART RX queue.
 * @returns true if the RX buffer is now full.
 */
bool UART_RXBytes() {
  while (PLIB_USART_ReceiverDataIsAvailable(g_hw_settings.usart) &&
         g_transceiver.data_index != BUFFER_SIZE) {
    g_transceiver.active->data[g_transceiver.data_index] =
        PLIB_USART_ReceiverByteReceive(g_hw_settings.usart);
    g_transceiver.data_index++;
  }
  if (g_transceiver.active->op == OP_RDM_WITH_RESPONSE ||
      g_transceiver.active->op == OP_RDM_BROADCAST) {
    if (g_transceiver.found_expected_length) {
      if (g_transceiver.data_index == g_transceiver.expected_length) {
        // We've got enough data to move on
        PLIB_USART_ReceiverDisable(g_hw_settings.usart);
        ResetToMark();
        g_transceiver.state = STATE_C_COMPLETE;
      }
    } else {
      if (g_transceiver.data_index >= 3u) {
        if (g_transceiver.active->data[0] == RDM_START_CODE &&
            g_transceiver.active->data[1] == RDM_SUB_START_CODE) {
          g_transceiver.found_expected_length = true;
          // Add two bytes for the checksum
          g_transceiver.expected_length = g_transceiver.active->data[2] + 2;
        }
      }
    }
  }
  g_transceiver.last_byte = PLIB_TMR_Counter16BitGet(TMR_ID_3);
  g_transceiver.last_byte_coarse = CoarseTimer_GetTime();
  return g_transceiver.data_index >= BUFFER_SIZE;
}

// Memory Buffer Management
// ----------------------------------------------------------------------------

/*
 * @brief Setup the transceiver buffers.
 */
static void InitializeBuffers() {
  g_transceiver.active = NULL;
  g_transceiver.next = NULL;

  unsigned int i = 0u;
  for (; i < NUMBER_OF_BUFFERS; i++) {
    g_transceiver.free_list[i] = &buffers[i];
  }
  g_transceiver.free_size = NUMBER_OF_BUFFERS;
}

/*
 * @brief Return the active buffer to the free list.
 */
static void FreeActiveBuffer() {
  if (g_transceiver.active) {
    g_transceiver.free_list[g_transceiver.free_size] = g_transceiver.active;
    g_transceiver.free_size++;
    g_transceiver.active = NULL;
  }
}

/*
 * @brief Move the next buffer to the active buffer.
 */
static void TakeNextBuffer() {
  if (g_transceiver.active) {
    g_transceiver.free_list[g_transceiver.free_size] = g_transceiver.active;
    g_transceiver.free_size++;
  }
  g_transceiver.active = g_transceiver.next;
  g_transceiver.next = NULL;
  g_transceiver.data_index = 0u;
}

// ----------------------------------------------------------------------------
static inline void PrepareRDMResponse() {
  // Rebase the timer to when the last byte was received
  RebaseTimer(g_transceiver.last_byte);

  g_transceiver.state = STATE_R_TX_WAITING;
  PLIB_USART_ReceiverDisable(g_hw_settings.usart);
  PLIB_USART_TransmitterInterruptModeSelect(g_hw_settings.usart,
                                            USART_TRANSMIT_FIFO_EMPTY);

  TakeNextBuffer();

  // Enable the timer to trigger when we send the RDM response.
  unsigned int jitter = 0u;
  if (g_timing_settings.rdm_responder_jitter) {
    jitter = Random_PseudoGet() % g_timing_settings.rdm_responder_jitter;
  }
  PLIB_TMR_Period16BitSet(
      TMR_ID_3,
      g_timing_settings.rdm_responder_delay - RESPONSE_FUDGE_FACTOR + jitter);
  SYS_INT_SourceStatusClear(INT_SOURCE_TIMER_3);
  SYS_INT_SourceEnable(INT_SOURCE_TIMER_3);
}

static inline void StartSendingRDMResponse() {
  PLIB_USART_TransmitterEnable(g_hw_settings.usart);
  if (!PLIB_USART_TransmitterBufferIsFull(g_hw_settings.usart) &&
       g_transceiver.data_index != g_transceiver.active->size) {
    PLIB_USART_TransmitterByteSend(
        g_hw_settings.usart,
        g_transceiver.active->data[g_transceiver.data_index]);
    g_transceiver.data_index++;
  }
  g_transceiver.state = STATE_R_TX_DATA;

  SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_TRANSMIT);
  SYS_INT_SourceEnable(INT_SOURCE_USART_1_TRANSMIT);
}

static inline void LogStateChange() {
  static TransceiverState last_state = STATE_RESET;

  if (g_transceiver.state != last_state) {
    SysLog_Print(SYSLOG_DEBUG, "Changed to %d", g_transceiver.state);
    last_state = g_transceiver.state;
  }
}

/*
 * @brief Run the completion callback.
 */
static inline void FrameComplete() {
  const uint8_t* data = NULL;
  unsigned int length = 0u;
  if (g_transceiver.active->op != OP_TX_ONLY &&
      g_transceiver.data_index != 0u) {
    // We actually got some data.
    data = g_transceiver.active->data;
    length = g_transceiver.data_index;
    g_transceiver.result = T_RESULT_RX_DATA;
  }

  TransceiverEvent event = {
    g_transceiver.active->token,
    (TransceiverOperation) g_transceiver.active->op,
    g_transceiver.result,
    data,
    length,
    &g_timing
  };

#ifdef PIPELINE_TRANSCEIVER_TX_EVENT
  PIPELINE_TRANSCEIVER_TX_EVENT(&event);
#else
  if (g_tx_callback) {
    g_tx_callback(&event);
  }
#endif
}

/*
 * @brief Run the RX callback.
 */
static inline void RXFrameEvent() {
  TransceiverEvent event = {
    0u,
    T_OP_RX,
    g_transceiver.event_index == 0u ? T_RESULT_RX_START_FRAME :
        T_RESULT_RX_CONTINUE_FRAME,
    g_transceiver.active->data,
    g_transceiver.data_index,
    &g_timing
  };

#ifdef PIPELINE_TRANSCEIVER_RX_EVENT
  PIPELINE_TRANSCEIVER_RX_EVENT(&event);
#else
  if (g_rx_callback) {
    g_rx_callback(&event);
  }
#endif
}

/*
 * @brief Run the RX callback with an end-of-frame event.
 */
static inline void RXEndFrameEvent() {
  TransceiverEvent event = {
    0u,
    T_OP_RX,
    T_RESULT_RX_FRAME_TIMEOUT,
    NULL,
    0u,
    &g_timing
  };

#ifdef PIPELINE_TRANSCEIVER_RX_EVENT
  PIPELINE_TRANSCEIVER_RX_EVENT(&event);
#else
  if (g_rx_callback) {
    g_rx_callback(&event);
  }
#endif
}

/*
 * @brief Reset the settings to their default values.
 */
static void ResetTimingSettings() {
  Transceiver_SetBreakTime(DEFAULT_BREAK_TIME);
  Transceiver_SetMarkTime(DEFAULT_MARK_TIME);
  Transceiver_SetRDMBroadcastTimeout(DEFAULT_RDM_BROADCAST_TIMEOUT);
  Transceiver_SetRDMResponseTimeout(DEFAULT_RDM_RESPONSE_TIMEOUT);
  Transceiver_SetRDMDUBResponseLimit(DEFAULT_RDM_DUB_RESPONSE_LIMIT);
  Transceiver_SetRDMResponderDelay(DEFAULT_RDM_RESPONDER_DELAY);
  Transceiver_SetRDMResponderJitter(0u);
}

// Interrupt Handlers
// ----------------------------------------------------------------------------
/*
 * @brief Called when an input capture event occurs.
 */
void __ISR(_INPUT_CAPTURE_2_VECTOR, ipl6) InputCaptureEvent(void) {
  while (!PLIB_IC_BufferIsEmpty(IC_ID_2)) {
    uint16_t value = PLIB_IC_Buffer16BitGet(IC_ID_2);
    switch (g_transceiver.state) {
      case STATE_C_RX_WAIT_FOR_DUB:
        g_timing.dub_response.start = value;
        g_transceiver.state = STATE_C_RX_IN_DUB;
        break;
      case STATE_C_RX_IN_DUB:
        g_timing.dub_response.end = value;
        break;
      case STATE_C_RX_WAIT_FOR_BREAK:
        g_timing.get_set_response.break_start = value;
        g_transceiver.state = STATE_C_RX_WAIT_FOR_MARK;
        break;
      case STATE_C_RX_WAIT_FOR_MARK:
        if ((uint16_t) (value - g_timing.get_set_response.break_start) <
            CONTROLLER_RX_BREAK_TIME_MIN) {
          // The break was too short, keep looking for a break
          g_timing.get_set_response.break_start = value;
          g_transceiver.state = STATE_C_RX_WAIT_FOR_BREAK;
        } else {
          g_timing.get_set_response.mark_start = value;
          // Break was good, enable UART
          SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_RECEIVE);
          SYS_INT_SourceEnable(INT_SOURCE_USART_1_RECEIVE);
          SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_ERROR);
          SYS_INT_SourceEnable(INT_SOURCE_USART_1_ERROR);
          PLIB_USART_ReceiverEnable(g_hw_settings.usart);
          g_transceiver.state = STATE_C_RX_DATA;
        }
        break;
      case STATE_C_RX_DATA:
        g_timing.get_set_response.mark_end = value;
        SYS_INT_SourceDisable(INT_SOURCE_INPUT_CAPTURE_2);
        PLIB_IC_Disable(INPUT_CAPTURE_MODULE);
        break;

      case STATE_R_RX_MBB:
        // Rebase the timer to when the falling edge occured.
        RebaseTimer(value);
        g_transceiver.state = STATE_R_RX_BREAK;
        break;
      case STATE_R_RX_BREAK:
        if (value >= RESPONDER_RX_BREAK_TIME_MIN &&
            value <= RESPONDER_RX_BREAK_TIME_MAX) {
          // Break was good, enable UART
          g_timing.request.break_time = value;
          SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_RECEIVE);
          SYS_INT_SourceEnable(INT_SOURCE_USART_1_RECEIVE);
          PLIB_USART_ReceiverEnable(g_hw_settings.usart);
          g_transceiver.state = STATE_R_RX_MARK;
        } else {
          // Break was out of range.
          g_transceiver.state = STATE_R_RX_MBB;
        }
        break;
      case STATE_R_RX_MARK:
        if ((uint16_t) (value - g_timing.request.break_time) <
              RESPONDER_RX_MARK_TIME_MIN ||
            (uint16_t) (value - g_timing.request.break_time) >
              RESPONDER_RX_MARK_TIME_MAX) {
          // Mark was out of range, rebase timer & switch back to BREAK
          RebaseTimer(value);

          // Disable UART
          PLIB_USART_ReceiverDisable(g_hw_settings.usart);
          SYS_INT_SourceDisable(INT_SOURCE_USART_1_RECEIVE);
          SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_RECEIVE);
          g_transceiver.state = STATE_R_RX_BREAK;
        } else {
          g_timing.request.mark_time = value - g_timing.request.break_time;
          g_transceiver.state = STATE_R_RX_DATA;
        }
        break;

      case STATE_R_RX_DATA:
        g_transceiver.last_change = value;
        break;

      case STATE_C_INITIALIZE:
      case STATE_C_TX_READY:
      case STATE_C_IN_BREAK:
      case STATE_C_IN_MARK:
      case STATE_C_TX_DATA:
      case STATE_C_TX_DRAIN:
      case STATE_C_RX_TIMEOUT:
      case STATE_C_COMPLETE:
      case STATE_C_BACKOFF:
      case STATE_R_INITIALIZE:
      case STATE_R_RX_PREPARE:
      case STATE_R_TX_WAITING:
      case STATE_R_TX_BREAK:
      case STATE_R_TX_MARK:
      case STATE_R_TX_DATA:
      case STATE_R_TX_DRAIN:
      case STATE_R_TX_COMPLETE:
      case STATE_ERROR:
      case STATE_RESET:
        // Should never happen.
        {};
    }
  }
  SYS_INT_SourceStatusClear(INT_SOURCE_INPUT_CAPTURE_2);
}

/*
 * @brief Called when the timer expires.
 */
void __ISR(_TIMER_3_VECTOR, ipl6) Transceiver_TimerEvent() {
  switch (g_transceiver.state) {
    case STATE_C_IN_BREAK:
    case STATE_R_TX_BREAK:
      // Transition to MAB.
      SetMark();
      g_transceiver.state = g_transceiver.state == STATE_C_IN_BREAK ?
          STATE_C_IN_MARK : STATE_R_TX_MARK;
      PLIB_TMR_Counter16BitClear(TMR_ID_3);
      PLIB_TMR_Period16BitSet(TMR_ID_3, g_timing_settings.mark_ticks);
      break;
    case STATE_C_IN_MARK:
      // Stop the timer.
      SYS_INT_SourceDisable(INT_SOURCE_TIMER_3);
      PLIB_TMR_Stop(TMR_ID_3);

      // Transition to sending the data.
      // Only push a single byte into the TX queue at the begining, otherwise
      // we blow our timing budget.
      if (!PLIB_USART_TransmitterBufferIsFull(g_hw_settings.usart) &&
          g_transceiver.data_index != g_transceiver.active->size) {
        PLIB_USART_TransmitterByteSend(
            g_hw_settings.usart,
            g_transceiver.active->data[g_transceiver.data_index]);
        g_transceiver.data_index++;
      }
      PLIB_USART_Enable(g_hw_settings.usart);
      PLIB_USART_TransmitterEnable(g_hw_settings.usart);
      g_transceiver.state = STATE_C_TX_DATA;
      SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_TRANSMIT);
      SYS_INT_SourceEnable(INT_SOURCE_USART_1_TRANSMIT);
      break;
    case STATE_R_TX_WAITING:
      EnableTX();

      if (g_transceiver.active->op == OP_RDM_WITH_RESPONSE) {
        SetBreak();
        PLIB_TMR_PrescaleSelect(TMR_ID_3, TMR_PRESCALE_VALUE_1);
        PLIB_TMR_Counter16BitClear(TMR_ID_3);
        PLIB_TMR_Period16BitSet(TMR_ID_3, g_timing_settings.break_ticks);
        g_transceiver.state = STATE_R_TX_BREAK;
      } else {
        SYS_INT_SourceDisable(INT_SOURCE_TIMER_3);
        StartSendingRDMResponse();
      }
      break;
    case STATE_R_TX_MARK:
      SYS_INT_SourceDisable(INT_SOURCE_TIMER_3);
      PLIB_TMR_PrescaleSelect(TMR_ID_3, TMR_PRESCALE_VALUE_8);

      StartSendingRDMResponse();
      break;
    case STATE_C_INITIALIZE:
    case STATE_C_TX_READY:
    case STATE_C_TX_DATA:
    case STATE_C_TX_DRAIN:
    case STATE_C_RX_WAIT_FOR_BREAK:
    case STATE_C_RX_WAIT_FOR_MARK:
    case STATE_C_RX_DATA:
    case STATE_C_RX_WAIT_FOR_DUB:
    case STATE_C_RX_IN_DUB:
    case STATE_C_RX_TIMEOUT:
    case STATE_C_COMPLETE:
    case STATE_C_BACKOFF:
    case STATE_R_INITIALIZE:
    case STATE_R_RX_PREPARE:
    case STATE_R_RX_BREAK:
    case STATE_R_RX_MARK:
    case STATE_R_RX_MBB:
    case STATE_R_RX_DATA:
    case STATE_R_TX_DATA:
    case STATE_R_TX_DRAIN:
    case STATE_R_TX_COMPLETE:
    case STATE_ERROR:
    case STATE_RESET:
      // Should never happen
      {}
  }
  SYS_INT_SourceStatusClear(INT_SOURCE_TIMER_3);
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
    if (g_transceiver.state == STATE_C_TX_DATA) {
      UART_TXBytes();
      if (g_transceiver.data_index == g_transceiver.active->size) {
        PLIB_USART_TransmitterInterruptModeSelect(
            g_hw_settings.usart, USART_TRANSMIT_FIFO_IDLE);
        g_transceiver.state = STATE_C_TX_DRAIN;
      }
    } else if (g_transceiver.state == STATE_C_TX_DRAIN) {
      // The last byte has been transmitted
      PLIB_TMR_Counter16BitClear(TMR_ID_3);
      PLIB_TMR_Period16BitSet(TMR_ID_3, 65535u);  // 6.5 ms until overflow.
      PLIB_TMR_PrescaleSelect(TMR_ID_3, TMR_PRESCALE_VALUE_8);
      PLIB_TMR_Start(TMR_ID_3);

      g_transceiver.tx_frame_end = CoarseTimer_GetTime();
      SYS_INT_SourceDisable(INT_SOURCE_USART_1_TRANSMIT);
      PLIB_USART_TransmitterDisable(g_hw_settings.usart);

      if (g_transceiver.active->op == OP_TX_ONLY) {
        PLIB_USART_Disable(g_hw_settings.usart);
        SetMark();
        PLIB_TMR_Stop(TMR_ID_3);
        g_transceiver.state = STATE_C_COMPLETE;
      } else {
        // Switch to RX Mode.
        if (g_transceiver.active->op == OP_RDM_DUB) {
          g_transceiver.state = STATE_C_RX_WAIT_FOR_DUB;
          g_transceiver.data_index = 0u;

          // Turn around the line
          EnableRX();
          UART_FlushRX();

          PLIB_IC_FirstCaptureEdgeSelect(INPUT_CAPTURE_MODULE, IC_EDGE_FALLING);
          PLIB_IC_Enable(INPUT_CAPTURE_MODULE);
          SYS_INT_SourceStatusClear(INT_SOURCE_INPUT_CAPTURE_2);
          SYS_INT_SourceEnable(INT_SOURCE_INPUT_CAPTURE_2);

          // TODO(simon) I think we can remove this because its done in the IC
          // ISR
          PLIB_USART_ReceiverEnable(g_hw_settings.usart);
          SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_RECEIVE);
          SYS_INT_SourceEnable(INT_SOURCE_USART_1_RECEIVE);
          SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_ERROR);
          SYS_INT_SourceEnable(INT_SOURCE_USART_1_ERROR);

        } else if (g_transceiver.active->op == OP_RDM_BROADCAST &&
                   g_timing_settings.rdm_broadcast_timeout == 0u) {
          // Go directly to the complete state.
          PLIB_TMR_Stop(TMR_ID_3);
          g_transceiver.state = STATE_C_COMPLETE;
        } else {
          // Either T_OP_RDM_WITH_RESPONSE or a non-0 broadcast listen time.
          g_transceiver.rdm_response_timeout = (
              g_transceiver.active->op == OP_RDM_BROADCAST ?
              g_timing_settings.rdm_broadcast_timeout :
              g_timing_settings.rdm_response_timeout);
          g_transceiver.state = STATE_C_RX_WAIT_FOR_BREAK;
          g_transceiver.data_index = 0u;

          EnableRX();
          UART_FlushRX();

          PLIB_IC_FirstCaptureEdgeSelect(INPUT_CAPTURE_MODULE, IC_EDGE_FALLING);
          PLIB_IC_Enable(INPUT_CAPTURE_MODULE);
          SYS_INT_SourceStatusClear(INT_SOURCE_INPUT_CAPTURE_2);
          SYS_INT_SourceEnable(INT_SOURCE_INPUT_CAPTURE_2);
        }
      }
    } else if (g_transceiver.state == STATE_R_TX_DATA) {
      UART_TXBytes();
      if (g_transceiver.data_index == g_transceiver.active->size) {
        PLIB_USART_TransmitterInterruptModeSelect(
            g_hw_settings.usart, USART_TRANSMIT_FIFO_IDLE);
        g_transceiver.state = STATE_R_TX_DRAIN;
      }
    } else if (g_transceiver.state == STATE_R_TX_DRAIN) {
      EnableRX();
      SYS_INT_SourceDisable(INT_SOURCE_USART_1_TRANSMIT);
      PLIB_USART_TransmitterDisable(g_hw_settings.usart);
      g_transceiver.state = STATE_R_TX_COMPLETE;
    }
    SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_TRANSMIT);
  } else if (SYS_INT_SourceStatusGet(INT_SOURCE_USART_1_RECEIVE)) {
    if (g_transceiver.state == STATE_C_RX_IN_DUB ||
        g_transceiver.state == STATE_C_RX_DATA) {
      if (UART_RXBytes()) {
        // RX buffer is full.
        PLIB_TMR_Stop(TMR_ID_3);
        SYS_INT_SourceDisable(INT_SOURCE_USART_1_RECEIVE);
        SYS_INT_SourceDisable(INT_SOURCE_USART_1_ERROR);
        PLIB_USART_ReceiverDisable(g_hw_settings.usart);
        ResetToMark();
        g_transceiver.result = T_RESULT_RX_INVALID;
        g_transceiver.state = STATE_C_COMPLETE;
      }
    } else if (g_transceiver.state == STATE_R_RX_DATA) {
      if (PLIB_USART_ErrorsGet(g_hw_settings.usart) & USART_ERROR_FRAMING) {
        // A framing error indicates a possible break.
        // Switch out of RX mode and back into the break state.
        SYS_INT_SourceDisable(INT_SOURCE_USART_1_RECEIVE);
        UART_FlushRX();
        PLIB_USART_ReceiverDisable(g_hw_settings.usart);

        // TODO(simon): how to handle this?
        // We need to make sure the last byte was delivered.
        RebaseTimer(g_transceiver.last_change);
        g_transceiver.data_index = 0u;
        g_transceiver.event_index = 0u;
        g_transceiver.state = STATE_R_RX_BREAK;
      } else if (UART_RXBytes()) {
        // RX buffer is full.
        // TODO(simon): What should we do here?
        SYS_INT_SourceDisable(INT_SOURCE_USART_1_RECEIVE);
        SYS_INT_SourceDisable(INT_SOURCE_USART_1_ERROR);
        PLIB_USART_ReceiverDisable(g_hw_settings.usart);

        g_transceiver.state = STATE_R_TX_COMPLETE;
      }
    }
    SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_RECEIVE);
  } else if (SYS_INT_SourceStatusGet(INT_SOURCE_USART_1_ERROR)) {
    switch (g_transceiver.state) {
      case STATE_C_RX_IN_DUB:
        SYS_INT_SourceDisable(INT_SOURCE_INPUT_CAPTURE_2);
        PLIB_IC_Disable(INPUT_CAPTURE_MODULE);
        // Fall through
      case STATE_C_RX_DATA:
        PLIB_TMR_Stop(TMR_ID_3);
        SYS_INT_SourceDisable(INT_SOURCE_USART_1_RECEIVE);
        SYS_INT_SourceDisable(INT_SOURCE_USART_1_ERROR);
        PLIB_USART_ReceiverDisable(g_hw_settings.usart);
        ResetToMark();
        g_transceiver.state = STATE_C_COMPLETE;
        break;
      case STATE_C_INITIALIZE:
      case STATE_C_TX_READY:
      case STATE_C_IN_BREAK:
      case STATE_C_IN_MARK:
      case STATE_C_TX_DATA:
      case STATE_C_TX_DRAIN:
      case STATE_C_RX_WAIT_FOR_BREAK:
      case STATE_C_RX_WAIT_FOR_MARK:
      case STATE_C_RX_WAIT_FOR_DUB:
      case STATE_C_RX_TIMEOUT:
      case STATE_C_COMPLETE:
      case STATE_C_BACKOFF:
      case STATE_R_INITIALIZE:
      case STATE_R_RX_PREPARE:
      case STATE_R_RX_BREAK:
      case STATE_R_RX_MARK:
      case STATE_R_RX_DATA:
      case STATE_R_RX_MBB:
      case STATE_R_TX_WAITING:
      case STATE_R_TX_BREAK:
      case STATE_R_TX_MARK:
      case STATE_R_TX_DATA:
      case STATE_R_TX_DRAIN:
      case STATE_R_TX_COMPLETE:
      case STATE_ERROR:
      case STATE_RESET:
        // Should never happen.
        {}
    }
    SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_ERROR);
  }
}

// Public API Functions
// ----------------------------------------------------------------------------
void Transceiver_Initialize(const TransceiverHardwareSettings* settings,
                            TransceiverEventCallback tx_callback,
                            TransceiverEventCallback rx_callback) {
  g_hw_settings = *settings;
  g_tx_callback = tx_callback;
  g_rx_callback = rx_callback;

  g_transceiver.state = STATE_R_INITIALIZE;
  g_transceiver.mode = T_MODE_RESPONDER;
  g_transceiver.desired_mode = T_MODE_RESPONDER;
  g_transceiver.data_index = 0u;

  InitializeBuffers();
  ResetTimingSettings();

  // Setup the Break, TX Enable & RX Enable I/O Pins
  PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0,
                                   g_hw_settings.port,
                                   g_hw_settings.break_bit);
  PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0,
                                   g_hw_settings.port,
                                   g_hw_settings.tx_enable_bit);
  PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0,
                                   g_hw_settings.port,
                                   g_hw_settings.rx_enable_bit);

  // Setup the timer
  PLIB_TMR_ClockSourceSelect(TMR_ID_3, TMR_CLOCK_SOURCE_PERIPHERAL_CLOCK);
  PLIB_TMR_PrescaleSelect(TMR_ID_3, TMR_PRESCALE_VALUE_1);
  PLIB_TMR_Mode16BitEnable(TMR_ID_3);
  SYS_INT_VectorPrioritySet(INT_VECTOR_T3, INT_PRIORITY_LEVEL1);
  SYS_INT_VectorSubprioritySet(INT_VECTOR_T3, INT_SUBPRIORITY_LEVEL0);

  // Setup the UART
  PLIB_USART_BaudRateSet(g_hw_settings.usart,
                         SYS_CLK_PeripheralFrequencyGet(CLK_BUS_PERIPHERAL_1),
                         DMX_BAUD);
  PLIB_USART_HandshakeModeSelect(g_hw_settings.usart,
                                 USART_HANDSHAKE_MODE_SIMPLEX);
  PLIB_USART_OperationModeSelect(g_hw_settings.usart,
                                 USART_ENABLE_TX_RX_USED);
  PLIB_USART_LineControlModeSelect(g_hw_settings.usart, USART_8N2);
  PLIB_USART_SyncModeSelect(g_hw_settings.usart, USART_ASYNC_MODE);
  PLIB_USART_TransmitterInterruptModeSelect(g_hw_settings.usart,
                                            USART_TRANSMIT_FIFO_EMPTY);

  SYS_INT_VectorPrioritySet(INT_VECTOR_UART1, INT_PRIORITY_LEVEL6);
  SYS_INT_VectorSubprioritySet(INT_VECTOR_UART1, INT_SUBPRIORITY_LEVEL0);
  SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_TRANSMIT);

  // Setup input capture
  PLIB_IC_Disable(INPUT_CAPTURE_MODULE);
  PLIB_IC_ModeSelect(INPUT_CAPTURE_MODULE, IC_INPUT_CAPTURE_EVERY_EDGE_MODE);
  PLIB_IC_FirstCaptureEdgeSelect(INPUT_CAPTURE_MODULE, IC_EDGE_RISING);
  PLIB_IC_TimerSelect(INPUT_CAPTURE_MODULE, IC_TIMER_TMR3);
  PLIB_IC_BufferSizeSelect(INPUT_CAPTURE_MODULE, IC_BUFFER_SIZE_16BIT);
  PLIB_IC_EventsPerInterruptSelect(INPUT_CAPTURE_MODULE,
                                   IC_INTERRUPT_ON_EVERY_CAPTURE_EVENT);

  SYS_INT_VectorPrioritySet(INT_VECTOR_IC2, INT_PRIORITY_LEVEL6);
  SYS_INT_VectorSubprioritySet(INT_VECTOR_IC2, INT_SUBPRIORITY_LEVEL0);
}

void Transceiver_SetMode(TransceiverMode mode) {
  if (mode == T_MODE_CONTROLLER) {
    SysLog_Print(SYSLOG_INFO, "Switching to Controller mode");
  } else {
    SysLog_Print(SYSLOG_INFO, "Switching to Responder mode");
  }

  g_transceiver.desired_mode = mode;
}

TransceiverMode Transceiver_GetMode() {
  return g_transceiver.mode;
}

void Transceiver_Tasks() {
  bool ok;
  LogStateChange();

  switch (g_transceiver.state) {
    case STATE_C_INITIALIZE:
      PLIB_TMR_Stop(TMR_ID_3);
      PLIB_USART_ReceiverDisable(g_hw_settings.usart);
      PLIB_USART_TransmitterDisable(g_hw_settings.usart);
      PLIB_USART_Disable(g_hw_settings.usart);
      PLIB_IC_Disable(INPUT_CAPTURE_MODULE);
      ResetToMark();
      g_transceiver.state = STATE_C_TX_READY;
      // Fall through
    case STATE_C_TX_READY:
      if (g_transceiver.desired_mode != T_MODE_CONTROLLER) {
        TakeNextBuffer();
        FreeActiveBuffer();
        SysLog_Print(SYSLOG_INFO, "Switched to responder mode");
        g_transceiver.mode = g_transceiver.desired_mode;
        g_transceiver.state = STATE_R_INITIALIZE;
        break;
      }

      if (!g_transceiver.next) {
        return;
      }
      // @pre Timer is not running.
      // @pre UART is disabled
      // @pre TX is enabled.
      // @pre RX is disabled.
      // @pre RX InputCapture is disabled.
      // @pre line in marking state

      TakeNextBuffer();

      // Reset state
      g_transceiver.found_expected_length = false;
      g_transceiver.expected_length = 0u;
      g_transceiver.result = T_RESULT_TX_OK;
      memset(&g_timing, 0, sizeof(g_timing));

      // Prepare the UART
      // Set UART Interrupts when the buffer is empty.
      PLIB_USART_TransmitterInterruptModeSelect(g_hw_settings.usart,
                                                USART_TRANSMIT_FIFO_EMPTY);

      // Set break and start timer.
      g_transceiver.state = STATE_C_IN_BREAK;
      PLIB_TMR_PrescaleSelect(TMR_ID_3 , TMR_PRESCALE_VALUE_1);
      g_transceiver.tx_frame_start = CoarseTimer_GetTime();
      PLIB_TMR_Counter16BitClear(TMR_ID_3);
      PLIB_TMR_Period16BitSet(TMR_ID_3, g_timing_settings.break_ticks);
      SYS_INT_SourceStatusClear(INT_SOURCE_TIMER_3);
      SYS_INT_SourceEnable(INT_SOURCE_TIMER_3);
      SetBreak();
      PLIB_TMR_Start(TMR_ID_3);

    case STATE_C_IN_BREAK:
    case STATE_C_IN_MARK:
      // Noop, wait for timer event
      break;
    case STATE_C_TX_DATA:
    case STATE_C_TX_DRAIN:
      // Noop, wait TX to complete.
      break;

    case STATE_C_RX_WAIT_FOR_BREAK:
      if (CoarseTimer_HasElapsed(g_transceiver.tx_frame_end,
                                 g_transceiver.rdm_response_timeout)) {
        SYS_INT_SourceDisable(INT_SOURCE_INPUT_CAPTURE_2);
        // Note: the IC ISR may have run between the case check and the
        // SourceDisable and switched us to STATE_C_RX_WAIT_FOR_MARK.
        SYS_INT_SourceDisable(INT_SOURCE_USART_1_RECEIVE);
        SYS_INT_SourceDisable(INT_SOURCE_USART_1_ERROR);
        PLIB_IC_Disable(INPUT_CAPTURE_MODULE);
        PLIB_TMR_Stop(TMR_ID_3);
        PLIB_USART_ReceiverDisable(g_hw_settings.usart);
        ResetToMark();
        g_transceiver.state = STATE_C_RX_TIMEOUT;
      }
      break;

    case STATE_C_RX_WAIT_FOR_MARK:
      // Disable interupts so we don't race
      SYS_INT_SourceDisable(INT_SOURCE_INPUT_CAPTURE_2);
      if (g_transceiver.state == STATE_C_RX_WAIT_FOR_MARK &&
          ((uint16_t) (PLIB_TMR_Counter16BitGet(TMR_ID_3) -
            g_timing.get_set_response.break_start) >
            CONTROLLER_RX_BREAK_TIME_MAX)) {
        // Break was too long
        g_transceiver.result = T_RESULT_RX_INVALID;
        PLIB_TMR_Stop(TMR_ID_3);
        ResetToMark();
        g_transceiver.state = STATE_C_COMPLETE;
      } else {
        SYS_INT_SourceEnable(INT_SOURCE_INPUT_CAPTURE_2);
      }
      break;

    case STATE_C_RX_DATA:
      // TODO(simon): handle the timeout case here.
      // It's not a static timeout, rather it varies with the slot count.
      // PLIB_TMR_Stop(TMR_ID_3);
      break;

    case STATE_C_RX_WAIT_FOR_DUB:
      if (CoarseTimer_HasElapsed(g_transceiver.tx_frame_end,
                                 g_timing_settings.rdm_response_timeout)) {
        SYS_INT_SourceDisable(INT_SOURCE_INPUT_CAPTURE_2);
        // Note: the IC ISR may have run between the case check and the
        // SourceDisable and switched us to STATE_C_RX_IN_DUB.
        SYS_INT_SourceDisable(INT_SOURCE_USART_1_RECEIVE);
        SYS_INT_SourceDisable(INT_SOURCE_USART_1_ERROR);
        PLIB_IC_Disable(INPUT_CAPTURE_MODULE);
        PLIB_USART_ReceiverDisable(g_hw_settings.usart);
        PLIB_TMR_Stop(TMR_ID_3);
        ResetToMark();
        g_transceiver.state = STATE_C_RX_TIMEOUT;
      }
      break;
    case STATE_C_RX_IN_DUB:
      if ((uint16_t) (PLIB_TMR_Counter16BitGet(TMR_ID_3) -
                      g_timing.dub_response.start) >
           g_timing_settings.rdm_dub_response_limit) {
        // The UART Error interupt may have fired, putting us into
        // STATE_C_COMPLETE, already.
        SYS_INT_SourceDisable(INT_SOURCE_INPUT_CAPTURE_2);
        SYS_INT_SourceDisable(INT_SOURCE_USART_1_RECEIVE);
        SYS_INT_SourceDisable(INT_SOURCE_USART_1_ERROR);
        PLIB_IC_Disable(INPUT_CAPTURE_MODULE);
        PLIB_USART_ReceiverDisable(g_hw_settings.usart);
        PLIB_TMR_Stop(TMR_ID_3);
        ResetToMark();
        // We got at least a falling edge, so this should probably be
        // considered a collision, rather than a timeout.
        g_transceiver.state = STATE_C_COMPLETE;
      }
      break;

    case STATE_C_RX_TIMEOUT:
      SysLog_Message(SYSLOG_INFO, "RX timeout");
      g_transceiver.state = STATE_C_COMPLETE;
      g_transceiver.result = T_RESULT_RX_TIMEOUT;
      break;
    case STATE_C_COMPLETE:
      if (g_transceiver.active->op == OP_RDM_DUB) {
        SysLog_Print(SYSLOG_INFO, "First DUB: %d", g_timing.dub_response.start);
        SysLog_Print(SYSLOG_INFO, "Last DUB: %d", g_timing.dub_response.end);
      }
      if (g_transceiver.active->op == OP_RDM_WITH_RESPONSE) {
        SysLog_Print(SYSLOG_INFO, "break: %d",
                     g_timing.get_set_response.break_start);
        SysLog_Print(SYSLOG_INFO, "mark start: %d, end: %d",
                     g_timing.get_set_response.mark_start,
                     g_timing.get_set_response.mark_end);
        SysLog_Print(SYSLOG_INFO, "Break: %d, Mark: %d",
                     (uint16_t) (g_timing.get_set_response.mark_start -
                      g_timing.get_set_response.break_start),
                     (uint16_t) (g_timing.get_set_response.mark_end -
                      g_timing.get_set_response.mark_start));
      }
      FrameComplete();
      g_transceiver.state = STATE_C_BACKOFF;
      // Fall through
    case STATE_C_BACKOFF:
      // From E1.11, the min break-to-break time is 1.204ms.
      //
      // From E1.20:
      //  - If DUB, the min EOF to break is 5.8ms
      //  - If bcast, the min EOF to break is 0.176ms
      //  - If lost response, the min EOF to break is 3.0ms
      //  - Any other packet, min EOF to break is 176uS.
      ok = CoarseTimer_HasElapsed(g_transceiver.tx_frame_start,
                                  CONTROLLER_MIN_BREAK_TO_BREAK);

      switch (g_transceiver.active->op) {
        case OP_TX_ONLY:
          // 176uS min, rounds to 0.2ms.
          ok &= CoarseTimer_HasElapsed(g_transceiver.tx_frame_end,
                                       CONTROLLER_NON_RDM_BACKOFF);
          break;
        case OP_RDM_DUB:
          // It would be nice to be able to reduce this if we didn't get a
          // response, but the standard doesn't allow this.
          ok &= CoarseTimer_HasElapsed(g_transceiver.tx_frame_end,
                                       CONTROLLER_DUB_BACKOFF);
          break;
        case OP_RDM_BROADCAST:
          ok &= CoarseTimer_HasElapsed(g_transceiver.tx_frame_end,
                                       CONTROLLER_BROADCAST_BACKOFF);
          break;
        case OP_RDM_WITH_RESPONSE:
          // TODO(simon):
          // We can probably make this faster, since the 3ms only
          // applies for no responses. If we do get a response, then it's only
          // a 0.176ms delay, from the end of the response frame.
          ok &= CoarseTimer_HasElapsed(g_transceiver.tx_frame_end,
                                       CONTROLLER_MISSING_RESPONSE_BACKOFF);
          break;
        case OP_RDM_DUB_RESPONSE:
        case OP_RDM_RESEPONSE:
        case OP_RX:
          // Noop
          {}
      }

      if (ok) {
        FreeActiveBuffer();
        g_transceiver.state = STATE_C_TX_READY;
      }
      break;
    case STATE_R_INITIALIZE:
      // This is done once when we switch to Responder mode
      // Reset the UART
      PLIB_USART_ReceiverDisable(g_hw_settings.usart);
      PLIB_USART_TransmitterDisable(g_hw_settings.usart);
      PLIB_USART_Enable(g_hw_settings.usart);
      UART_FlushRX();

      // Put us into RX mode
      EnableRX();

      // Setup the timer
      PLIB_TMR_Counter16BitClear(TMR_ID_3);
      PLIB_TMR_Period16BitSet(TMR_ID_3, 65535);  // 6.5 ms until overflow.
      PLIB_TMR_PrescaleSelect(TMR_ID_3, TMR_PRESCALE_VALUE_8);
      PLIB_TMR_Start(TMR_ID_3);

      // Fall through
    case STATE_R_RX_PREPARE:
      // Setup RX buffer
      if (!g_transceiver.active) {
        if (g_transceiver.free_size == 0u) {
          SysLog_Message(SYSLOG_INFO, "Lost buffers!");
          g_transceiver.state = STATE_ERROR;
          return;
        }

        g_transceiver.free_size--;
        g_transceiver.active = g_transceiver.free_list[g_transceiver.free_size];
      }

      // Reset state variables.
      g_timing.request.break_time = 0u;
      g_timing.request.mark_time = 0u;
      g_transceiver.data_index = 0u;
      g_transceiver.event_index = 0u;
      g_transceiver.active->op = OP_RX;

      g_transceiver.state = STATE_R_RX_MBB;

      // Catch the next falling edge.
      SYS_INT_SourceDisable(INT_SOURCE_INPUT_CAPTURE_2);
      SYS_INT_SourceStatusClear(INT_SOURCE_INPUT_CAPTURE_2);
      PLIB_IC_Disable(INPUT_CAPTURE_MODULE);
      PLIB_IC_FirstCaptureEdgeSelect(INPUT_CAPTURE_MODULE, IC_EDGE_FALLING);
      PLIB_IC_Enable(INPUT_CAPTURE_MODULE);
      SYS_INT_SourceEnable(INT_SOURCE_INPUT_CAPTURE_2);

      // Fall through
    case STATE_R_RX_MBB:
      // noop, waiting for IC event

      SYS_INT_SourceDisable(INT_SOURCE_INPUT_CAPTURE_2);
      if (g_transceiver.desired_mode != T_MODE_RESPONDER) {
        g_transceiver.mode = g_transceiver.desired_mode;
        PLIB_IC_Disable(INPUT_CAPTURE_MODULE);
        PLIB_TMR_Stop(TMR_ID_3);
        FreeActiveBuffer();
        SysLog_Print(SYSLOG_INFO, "Switched to controller mode");
        g_transceiver.state = STATE_C_INITIALIZE;
        break;
      }
      SYS_INT_SourceEnable(INT_SOURCE_INPUT_CAPTURE_2);
      break;

    case STATE_R_RX_BREAK:
      // noop, waiting for IC event
      break;
    case STATE_R_RX_MARK:
      // noop, waiting for IC event
      break;

    case STATE_R_RX_DATA:
      SYS_INT_SourceDisable(INT_SOURCE_USART_1_RECEIVE);

      if (g_transceiver.data_index != 0u) {
        // Got at least one byte, so we have the start code.
        // Check the time since the last byte.
        if ((g_transceiver.active->data[0] == RDM_START_CODE &&
             CoarseTimer_HasElapsed(g_transceiver.last_byte_coarse,
                                    RESPONDER_RDM_INTERSLOT_TIMEOUT)) ||
            CoarseTimer_HasElapsed(g_transceiver.last_byte_coarse,
                                   RESPONDER_DMX_INTERSLOT_TIMEOUT)) {
          // RDM inter-slot timeout
          RXEndFrameEvent();
          PLIB_USART_ReceiverDisable(g_hw_settings.usart);
          g_transceiver.state = STATE_R_RX_PREPARE;
          break;
        }
      }

      if (g_transceiver.event_index != g_transceiver.data_index) {
        RXFrameEvent();
        g_transceiver.event_index = g_transceiver.data_index;
      }

      if (g_transceiver.next) {
        // Update the seed with the value from the coarse timer. This is a
        // useful source of entropy.
        Random_SetSeed(CoarseTimer_GetTime());
        PrepareRDMResponse();
      } else {
        // Continue receiving
        SYS_INT_SourceEnable(INT_SOURCE_USART_1_RECEIVE);
      }
      break;
    case STATE_R_TX_WAITING:
    case STATE_R_TX_BREAK:
    case STATE_R_TX_MARK:
      // noop, waiting for timer event.
      break;
    case STATE_R_TX_DATA:
      // noop
      break;
    case STATE_R_TX_DRAIN:
      FreeActiveBuffer();
      break;
    case STATE_R_TX_COMPLETE:
      PLIB_TMR_Period16BitSet(TMR_ID_3, 65535u);
      g_transceiver.data_index = 0u;
      g_transceiver.state = STATE_R_RX_PREPARE;
      break;
    case STATE_RESET:
      g_transceiver.mode = g_transceiver.desired_mode;
      g_transceiver.state = (g_transceiver.mode == T_MODE_RESPONDER ?
          STATE_R_INITIALIZE : STATE_C_INITIALIZE);
      break;
    case STATE_ERROR:
      break;
  }
}

/*
 * Queue an operation.
 * @param token The token for this operation.
 * @param start_code The start code for the outgoing frame.
 * @param op The type of operation.
 * @param data The frame's slot data.
 * @param size The number of slots.
 * @returns true if the operation was queued, false if the buffer was full.
 */
bool Transceiver_QueueFrame(uint8_t token, uint8_t start_code,
                            InternalOperation op, const uint8_t* data,
                            unsigned int size) {
  if (g_transceiver.mode == T_MODE_RESPONDER || g_transceiver.free_size == 0u) {
    return false;
  }

  g_transceiver.free_size--;
  g_transceiver.next = g_transceiver.free_list[g_transceiver.free_size];

  if (size > DMX_FRAME_SIZE) {
    size = DMX_FRAME_SIZE;
  }
  g_transceiver.next->size = size + 1u;  // include start code.
  g_transceiver.next->op = op;
  g_transceiver.next->token = token;
  g_transceiver.next->data[0] = start_code;
  SysLog_Print(SYSLOG_INFO, "Start code %d", start_code);
  memcpy(&g_transceiver.next->data[1], data, size);
  return true;
}

bool Transceiver_QueueDMX(uint8_t token, const uint8_t* data,
                          unsigned int size) {
  return Transceiver_QueueFrame(
      token, NULL_START_CODE, OP_TX_ONLY, data, size);
}

bool Transceiver_QueueASC(uint8_t token, uint8_t start_code,
                          const uint8_t* data, unsigned int size) {
  return Transceiver_QueueFrame(
      token, start_code, OP_TX_ONLY, data, size);
}

bool Transceiver_QueueRDMDUB(uint8_t token, const uint8_t* data,
                             unsigned int size) {
  return Transceiver_QueueFrame(
      token, RDM_START_CODE, OP_RDM_DUB,
      data, size);
}

bool Transceiver_QueueRDMRequest(uint8_t token, const uint8_t* data,
                                 unsigned int size, bool is_broadcast) {
  return Transceiver_QueueFrame(
      token, RDM_START_CODE,
      is_broadcast ? OP_RDM_BROADCAST : OP_RDM_WITH_RESPONSE,
      data, size);
}

bool Transceiver_QueueRDMResponse(bool include_break,
                                  const IOVec* data,
                                  unsigned int iov_count) {
  if (g_transceiver.free_size == 0u) {
    return false;
  }

  g_transceiver.free_size--;
  g_transceiver.next = g_transceiver.free_list[g_transceiver.free_size];

  unsigned int i = 0u;
  uint16_t offset = 0u;
  for (; i != iov_count; i++) {
    if (offset + data[i].length > BUFFER_SIZE) {
      memcpy(g_transceiver.next->data + offset, data[i].base,
             BUFFER_SIZE - offset);
      offset = BUFFER_SIZE;
      SysLog_Message(SYSLOG_ERROR, "Truncated RDM response");
      break;
    } else {
      memcpy(g_transceiver.next->data + offset, data[i].base, data[i].length);
      offset += data[i].length;
    }
  }
  g_transceiver.next->size = offset;
  g_transceiver.next->op = include_break ? OP_RDM_WITH_RESPONSE :
                           OP_RDM_DUB_RESPONSE;
  return true;
}

/*
 *  This is called by the MessageHandler, so we know we're not in _Tasks or an
 *  ISR.
 */
void Transceiver_Reset() {
  // Disable & clear all interrupts.
  SYS_INT_SourceDisable(INT_SOURCE_USART_1_TRANSMIT);
  SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_TRANSMIT);
  SYS_INT_SourceDisable(INT_SOURCE_USART_1_RECEIVE);
  SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_RECEIVE);
  SYS_INT_SourceDisable(INT_SOURCE_USART_1_ERROR);
  SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_ERROR);

  InitializeBuffers();

  // Reset Timer
  SYS_INT_SourceDisable(INT_SOURCE_TIMER_3);
  SYS_INT_SourceStatusClear(INT_SOURCE_TIMER_3);
  PLIB_TMR_Stop(TMR_ID_3);

  // Reset IC
  SYS_INT_SourceDisable(INT_SOURCE_INPUT_CAPTURE_2);
  SYS_INT_SourceStatusClear(INT_SOURCE_INPUT_CAPTURE_2);
  PLIB_IC_Disable(INPUT_CAPTURE_MODULE);

  // Reset UART
  PLIB_USART_ReceiverDisable(g_hw_settings.usart);
  PLIB_USART_TransmitterDisable(g_hw_settings.usart);
  PLIB_USART_Disable(g_hw_settings.usart);

  // Reset buffers in case we got into a weird state.
  InitializeBuffers();

  // Reset all timing configuration.
  ResetTimingSettings();

  // Set us back into the TX Mark state.
  ResetToMark();

  g_transceiver.state = STATE_RESET;
}

bool Transceiver_SetBreakTime(uint16_t break_time_us) {
  if (break_time_us < MINIMUM_TX_BREAK_TIME ||
      break_time_us > MAXIMUM_TX_BREAK_TIME) {
    return false;
  }
  g_timing_settings.break_time = break_time_us;
  uint16_t ticks = MicroSecondsToTicks(break_time_us);
  g_timing_settings.break_ticks = ticks - BREAK_FUDGE_FACTOR;
  SysLog_Print(SYSLOG_INFO, "Break ticks is %d", ticks);
  return true;
}

uint16_t Transceiver_GetBreakTime() {
  return g_timing_settings.break_time;
}

bool Transceiver_SetMarkTime(uint16_t mark_time_us) {
  if (mark_time_us < MINIMUM_TX_MARK_TIME ||
      mark_time_us > MAXIMUM_TX_MARK_TIME) {
    return false;
  }
  g_timing_settings.mark_time = mark_time_us;
  uint16_t ticks = MicroSecondsToTicks(mark_time_us);
  g_timing_settings.mark_ticks = ticks - MARK_FUDGE_FACTOR;
  SysLog_Print(SYSLOG_INFO, "MAB ticks is %d", ticks);
  return true;
}

uint16_t Transceiver_GetMarkTime() {
  return g_timing_settings.mark_time;
}

bool Transceiver_SetRDMBroadcastTimeout(uint16_t delay) {
  if (delay > 50u) {
    return false;
  }
  g_timing_settings.rdm_broadcast_timeout = delay;
  SysLog_Print(SYSLOG_INFO, "Bcast timeout: %d",
               g_timing_settings.rdm_broadcast_timeout);
  return true;
}

uint16_t Transceiver_GetRDMBroadcastTimeout() {
  return g_timing_settings.rdm_broadcast_timeout;
}

bool Transceiver_SetRDMResponseTimeout(uint16_t delay) {
  if (delay < 10u || delay > 50u) {
    return false;
  }
  g_timing_settings.rdm_response_timeout = delay;
  return true;
}

uint16_t Transceiver_GetRDMResponseTimeout() {
  return g_timing_settings.rdm_response_timeout;
}

bool Transceiver_SetRDMDUBResponseLimit(uint16_t limit) {
  if (limit < 10000u || limit > 35000u) {
    return false;
  }
  g_timing_settings.rdm_dub_response_limit = limit;
  return true;
}

uint16_t Transceiver_GetRDMDUBResponseLimit() {
  return g_timing_settings.rdm_dub_response_limit;
}

bool Transceiver_SetRDMResponderDelay(uint16_t delay) {
  if (delay < MINIMUM_RESPONDER_DELAY || delay > MAXIMUM_RESPONDER_DELAY) {
    return false;
  }
  g_timing_settings.rdm_responder_delay = delay;
  uint16_t max_jitter = MAXIMUM_RESPONDER_DELAY - delay;
  g_timing_settings.rdm_responder_jitter = (
    g_timing_settings.rdm_responder_jitter < max_jitter ?
    g_timing_settings.rdm_responder_jitter : max_jitter);
  return true;
}

uint16_t Transceiver_GetRDMResponderDelay() {
  return g_timing_settings.rdm_responder_delay;
}

bool Transceiver_SetRDMResponderJitter(uint16_t max_jitter) {
  if ((uint32_t) max_jitter + g_timing_settings.rdm_responder_delay >
      MAXIMUM_RESPONDER_DELAY) {
    return false;
  }
  g_timing_settings.rdm_responder_jitter = max_jitter;
  return true;
}

uint16_t Transceiver_GetRDMResponderJitter() {
  return g_timing_settings.rdm_responder_jitter;
}
