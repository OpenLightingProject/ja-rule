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
 * transceiver.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @defgroup transceiver Transceiver
 * @brief The DMX512 / RDM Transceiver
 *
 * This module handles communications on the RS485 line.
 *
 * The general model is that a client initiates a transceiver operation, passing
 * in a token (cookie) to identify the operation. When the transceiver
 * operation completes, the appropriate TransceiverEventCallback is run, which
 * receives the same token and any operation-specific data. This allows the
 * client to associate the callback with the original request that generated
 * the event.
 *
 * If a TRANSCEIVER_NO_NOTIFICATION is used for the token, the callback will
 * not be run.
 *
 * The transceiver can operate in various modes:
 *  - DMX / RDM Controller
 *  - DMX / RDM Receiver
 *  - Self Test
 *
 * Since we may be in the middle of performing an operation when the mode
 * change request occurs, the Transceiver_SetMode() function takes a token
 * argument. When the mode change completes, a T_OP_MODE_CHANGE event will
 * occur.
 *
 * @par Controller Mode
 *
 * In controller mode, clients can send E1.11 frames by calling one of:
 *  - Transceiver_QueueDMX();
 *  - Transceiver_QueueASC();
 *  - Transceiver_QueueRDMDUB();
 *  - Transceiver_QueueRDMRequest();
 *
 * See @ref controller-overview "Controller State Machine".
 *
 * @par Responder Mode
 *
 * In responder mode, the TransceiverEventCallback will be run when a frame is
 * received. The handler should call Transceiver_QueueRDMResponse() to send a
 * response frame. See @ref responder-overview "Responder State Machine".
 *
 * @par Self Test Mode
 *
 * This puts the E1.11 driver circuit into loopback mode and allows the client
 * to call Transceiver_QueueSelfTest(). The self test operation will send a
 * single byte which can be used to confirm the driver circuit is working
 * correctly.
 *
 * @addtogroup transceiver
 * @{
 * @file transceiver.h
 * @brief The DMX512 / RDM Transceiver
 */

#ifndef FIRMWARE_SRC_TRANSCEIVER_H_
#define FIRMWARE_SRC_TRANSCEIVER_H_

#include <stdint.h>
#include <stdbool.h>

#include "iovec.h"
#include "system_config.h"
#include "peripheral/ic/plib_ic.h"
#include "peripheral/ports/plib_ports.h"
#include "peripheral/tmr/plib_tmr.h"
#include "peripheral/usart/plib_usart.h"
#include "system/int/sys_int.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Suppress event notifications
 *
 * This value can be used as a token to avoid the event handler notification.
 */
extern const int16_t TRANSCEIVER_NO_NOTIFICATION;

/**
 * @brief The operating modes of the transceiver.
 */
typedef enum {
  T_MODE_CONTROLLER,  //!< An RDM controller and/or source of DMX512
  T_MODE_RESPONDER,  //!< An RDM device and/or receiver of DMX512.
  T_MODE_SELF_TEST,  //!< Self test mode.
  T_MODE_LAST  //!< The first 'undefined' mode
} TransceiverMode;

/**
 * @brief Identifies the various types of transceiver operation.
 */
typedef enum {
  T_OP_TX_ONLY,  //!< No response (DMX512) or ASC.
  T_OP_RDM_DUB,  //!< An RDM Discovery Unique Branch
  T_OP_RDM_BROADCAST,  //!< A broadcast Get / Set Request.
  T_OP_RDM_WITH_RESPONSE,  //!< A RDM Get / Set Request.
  T_OP_RX,  //!< Receive mode.
  T_OP_MODE_CHANGE,  //!< Mode change complete
  T_OP_SELF_TEST  //!< Self test complete
} TransceiverOperation;

/**
 * @brief The result of an operation.
 */
typedef enum {
  T_RESULT_OK,  //!< The operation completed successfully.

  T_RESULT_TX_ERROR,  //!< A TX error occurred.
  T_RESULT_RX_DATA,  //!< Data was received.
  T_RESULT_RX_TIMEOUT,  //!< No response was received within the RDM wait time.
  T_RESULT_RX_INVALID,  //!< Invalid data received.

  T_RESULT_RX_START_FRAME,  //!< A frame was received
  T_RESULT_RX_CONTINUE_FRAME,  //!< A frame was received

  /**
   * @brief The frame timed out (inter-slot delay exceeded)
   */
  T_RESULT_RX_FRAME_TIMEOUT,

  T_RESULT_CANCELLED,  //!< The operation was cancelled
  T_RESULT_SELF_TEST_FAILED  //!< The test failed.
} TransceiverOperationResult;

/**
 * @brief The timing measurements for an operation.
 */
typedef union {
  /**
   * @brief The timing measurements for a DUB transaction.
   *
   * All times are measured in 10ths of a microsecond from the end of the DUB
   * frame.
   */
  struct {
    uint16_t start;  //!< The start of the discovery response.
    uint16_t end;  //!< The end of the discovery response.
  } dub_response;

  /**
   * @brief The timing measurements for a Get / Set transaction.
   *
   * All times are measured in 10ths of a microsecond from the end of the DUB
   * frame.
   */
  struct {
    uint16_t break_start;  //!< The start of the break.
    uint16_t mark_start;  //!< The start of the mark / end of the break.
    uint16_t mark_end;  //!< The end of the mark.
  } get_set_response;

  /**
   * @brief The timing measurements for an incoming frame.
   *
   * This may be a DMX frame, a RDM frame or an ASC frame.
   */
  struct {
    uint16_t break_time;  //!< The break time in 10ths of a uS
    uint16_t mark_time;  //!< The mark time in 10ths of a uS.
  } request;
} TransceiverTiming;

/**
 * @brief Information about a transceiver event.
 *
 * In controller mode an event occurs when:
 *  - A DMX frame has been completely sent.
 *  - A TX error occurred.
 *  - A RDM frame has been broadcast.
 *  - A RDM response (either DUB or Get/Set) has been received.
 *  - A RDM timeout has occurred.
 *
 * In responder mode, events occur when a frame is received.
 */
typedef struct {
  /**
   * @brief The Token associated with the operation.
   *
   * This will match the token passed in to Transceiver_QueueDMX(),
   * Transceiver_QueueASC(), Transceiver_QueueRDMDUB() or
   * Transceiver_QueueRDMRequest().
   *
   * In responder mode, the token will be 0.
   */
  int16_t token;

  /**
   * @brief The type of operation that triggered the event.
   */
  TransceiverOperation op;

  /**
   * @brief The result of the operation.
   */
  TransceiverOperationResult result;

  /**
   * @brief The received data. May be null.
   */
  const uint8_t *data;

  /**
   * @brief The length of the received data.
   */
  unsigned int length;

  /**
   * @brief The timing parameters associated with the operation.
   *
   * This may be NULL, if no timing information was available.
   */
  TransceiverTiming *timing;
} TransceiverEvent;

/**
 * @brief The callback run when a transceiver event occurs.
 *
 * The pointer is valid for the lifetime of the function call.
 */
typedef bool (*TransceiverEventCallback)(const TransceiverEvent *event);

/**
 * @brief The hardware settings to use for the Transceiver.
 *
 * Alas, this doesn't contain all of the settings. The vector numbers used in
 * the ISRs are required at compile time, so we can't control that. We'll need
 * to come up with a better way to make this modular.
 */
typedef struct {
  USART_MODULE_ID usart;  //!< The USART module to use
  INT_VECTOR usart_vector;  //!< The vector to use for the USART
  INT_SOURCE usart_tx_source;  //!< The source of USART TX
  INT_SOURCE usart_rx_source;  //!< The source of USART RX
  INT_SOURCE usart_error_source;  //!< The source of USART errors
  PORTS_CHANNEL port;  //!< The port to use for control signals.
  PORTS_BIT_POS break_bit;  //!< The port bit to use to generate breaks.
  PORTS_BIT_POS tx_enable_bit;  //!< The TX Enable bit.
  PORTS_BIT_POS rx_enable_bit;  //!< The RX Enable bit.
  IC_MODULE_ID input_capture_module;  //!< The IC module to use
  INT_VECTOR input_capture_vector;  //!< The vector to use for IC
  INT_SOURCE input_capture_source;  //!< The source to use for IC
  TMR_MODULE_ID timer_module_id;  //!< The timer to use
  INT_VECTOR timer_vector;  //!< The vector to use for timer
  INT_SOURCE timer_source;  //!< The source to use for timer
  IC_TIMERS input_capture_timer;  //!< The timer to use for IC
} TransceiverHardwareSettings;

/**
 * @brief Initialize the transceiver.
 * @param settings The settings to use for the transceiver.
 * @param tx_callback The callback to run when a transceiver TX event occurs.
 * @param rx_callback The callback to run when a transceiver RX event occurs.
 *
 * If PIPELINE_TRANSCEIVER_TX_EVENT is defined in app_pipeline.h, the macro
 * will override the value of tx_callback.
 * If PIPELINE_TRANSCEIVER_RX_EVENT is defined in app_pipeline.h, the macro
 * will override the value of rx_callback.
 */
void Transceiver_Initialize(const TransceiverHardwareSettings *settings,
                            TransceiverEventCallback tx_callback,
                            TransceiverEventCallback rx_callback);

/**
 * @brief Change the operating mode of the transceiver.
 * @param mode the new operating mode.
 * @param token The token passed to the TransceiverEventCallback when the mode
 *   change completes.
 * @returns True if the mode change is queued, false if there was already
 *   another mode change pending or the device is already in the requested
 *   mode.
 *
 * After any in-progress operation completes, then next call to
 * Transceiver_Tasks() will result in the mode change.
 */
bool Transceiver_SetMode(TransceiverMode mode, int16_t token);

/**
 * @brief The operating mode of the transceiver.
 * @returns the current operating mode.
 */
TransceiverMode Transceiver_GetMode();

/**
 * @brief Perform the periodic transceiver tasks.
 *
 * This should be called in the main event loop.
 */
void Transceiver_Tasks();

/**
 * @brief Queue a DMX frame for transmission.
 * @param token The token for this operation.
 * @param data The DMX data, excluding the start code.
 * @param size The size of the DMX data, excluding the start code.
 * @returns true if the frame was accepted and buffered, false if the transmit
 *   buffer is full.
 */
bool Transceiver_QueueDMX(int16_t token, const uint8_t* data,
                          unsigned int size);

/**
 * @brief Queue an alternate start code (ASC) frame for transmission.
 * @param token The token for this operation.
 * @param start_code the alternate start code.
 * @param data The ASC data, excluding the start code.
 * @param size The size of the data, excluding the start code.
 * @returns true if the frame was accepted and buffered, false if the transmit
 *   buffer is full.
 */
bool Transceiver_QueueASC(int16_t token, uint8_t start_code,
                          const uint8_t* data, unsigned int size);

/**
 * @brief Queue an RDM DUB operation.
 * @param token The token for this operation.
 * @param data The RDM DUB data, excluding the start code.
 * @param size The size of the RDM DUB data, excluding the start code.
 * @returns true if the frame was accepted and buffered, false if the transmit
 *   buffer is full.
 */
bool Transceiver_QueueRDMDUB(int16_t token, const uint8_t* data,
                             unsigned int size);

/**
 * @brief Queue an RDM Get / Set operation.
 * @param token The token for this operation.
 * @param data The RDM data, excluding the start code.
 * @param size The size of the RDM data, excluding the start code.
 * @param is_broadcast True if this is a broadcast request.
 * @returns true if the frame was accepted and buffered, false if the transmit
 *   buffer is full.
 */
bool Transceiver_QueueRDMRequest(int16_t token, const uint8_t* data,
                                 unsigned int size, bool is_broadcast);

/**
 * @brief Queue an RDM Response.
 * @param include_break true if this response requires a break
 * @param iov The data to send in the response
 * @param iov_count The number of IOVecs.
 * @returns true if the frame was accepted and buffered, false if the transmit
 *   buffer is full.
 */
bool Transceiver_QueueRDMResponse(bool include_break,
                                  const IOVec* iov,
                                  unsigned int iov_count);


/**
 * @brief Schedule a loopback self test.
 */
bool Transceiver_QueueSelfTest(int16_t token);

/**
 * @brief Reset the transceiver state.
 *
 * This can be used to recover from an error. The line will be placed back into
 * a MARK state.
 */
void Transceiver_Reset();

/**
 * @brief Set the break (space) time.
 * @param break_time_us the break time in microseconds, values are 44 to 800
 *   inclusive.
 * @returns true if the break time was updated, false if the value was out of
 *   range.
 *
 * The default time is 176 uS. Table 6 in E1.11 lists the minimum break time as
 * 92uS but the 1990 standard allows 88uS. We go down to 44 uS for testing
 * purposes. Table 3-1 in E1.20 lists the minimum break as 176uS and the
 * maximum as 352uS.
 */
bool Transceiver_SetBreakTime(uint16_t break_time_us);

/**
 * @brief Return the current configured break time.
 * @returns The break time in microseconds.
 * @sa Transceiver_SetBreakTime
 */
uint16_t Transceiver_GetBreakTime();

/**
 * @brief Set the mark-after-break (MAB) time.
 * @param mark_time_us the mark time in microseconds, values are 4 to 800
 *   inclusive.
 * @returns true if the mark time was updated, false if the value was out of
 *   range.
 *
 * The default is 12uS. Table 6 in E1.11 allows 12uS to 1s. Table 3-1 in E1.20
 * allows 12 to 88uS. We go down to 4 uS for testing purposes.
 */
bool Transceiver_SetMarkTime(uint16_t mark_time_us);

/**
 * @brief Return the current configured mark-after-break (MAB) time.
 * @returns The MAB time in microseconds.
 * @sa Transceiver_SetMarkTime.
 */
uint16_t Transceiver_GetMarkTime();

/**
 * @brief Set the controller timeout for broadcast RDM commands.
 * @param delay the time to wait for a broadcast response, in 10ths of a
 *   millisecond. Valid values are 0 to 50 (0 to 5ms).
 * @returns true if the broadcast timeout was updated, false if the value
 *   was out of range.
 *
 * With the exception of a DUB, an RDM controller usually doesn't listen for
 * responses after sending a broadcast command. However for testing purposes we
 * want to be able to listen for responders that incorrectly reply to non-DUB
 * broadcasts.
 */
bool Transceiver_SetRDMBroadcastTimeout(uint16_t delay);

/**
 * @brief Return the current controller timeout for broadcast RDM commands.
 * @returns The RDM broadcast listen time, in 10ths of a millisecond.
 */
uint16_t Transceiver_GetRDMBroadcastTimeout();

/**
 * @brief Set the controller's RDM response timeout.
 * @param delay the time to wait in 10ths of a millisecond. Valid values
 *   are 10 - 50 (1 - 5ms). Values < 28 are outside the specification but may
 *   be used for testing.
 * @returns true if time was updated, false if the value was out of range.
 *
 * This response timeout is the time the controller waits for an RDM response
 * before considering the response missing. This is used for both DISCOVERY and
 * GET/SET commands. The limits for broadcast commands is controlled with
 * Transceiver_SetRDMBroadcastTimeout().
 *
 * The default value is 28 (2.8mS), see Lines 1 & 3, Table 3-2, E1.20.
 *
 * By setting the value less than 28, we can cause responders that are at the
 * limits of the specification to fail. By setting the value more than 28, we
 * can accommodate responders that are out-of-spec.
 */
bool Transceiver_SetRDMResponseTimeout(uint16_t delay);

/**
 * @brief Return the controller's RDM response timeout.
 * @returns The controller RDM response timeout, in 10ths of a millisecond.
 * @sa Transceiver_SetRDMResponseTimeout
 */
uint16_t Transceiver_GetRDMResponseTimeout();

/**
 * @brief Set the maximum time allowed for a DUB response.
 * @param limit the maximum time to wait from the start of the DUB response
 * until the end, in 10ths of a microseconds. Valid values are 10000 - 35000
 * (1 - 3.5ms). Values < 28000 are outside the specification but may be used
 * for testing.
 * @returns true if time was updated, false if the value was out of range.
 *
 * The default value is 29000 (2.9mS), see Line 3, Table 3-3, E1.20.
 *
 * By setting the value less than 29000, we can cause responders that are at the
 * limits of the specification to fail. By setting the value to more than 29000,
 * we can support responders that are out-of-spec.
 */
bool Transceiver_SetRDMDUBResponseLimit(uint16_t limit);

/**
 * @brief Return the Controller DUB response timeout.
 * @returns The RDM DUB response limit, in 10ths of a microsecond.
 * @sa Transceiver_SetDUBResponseLimit.
 */
uint16_t Transceiver_GetRDMDUBResponseLimit();

/**
 * @brief Configure the delay after the end of the controller's packet before
 * the responder will transmit the reply.
 * @param delay the delay between the end-of-packet and transmitting the
 * responder, in 10ths of a microseconds. Valid values are 1760 - 20000
 * (0.176 - 2ms).
 * @returns true if time was updated, false if the value was out of range.
 *
 * The default value is 1760 (176uS), see Table 3-4, E1.20.
 */
bool Transceiver_SetRDMResponderDelay(uint16_t delay);

/**
 * @brief Return the RDM responder delay.
 * @returns The RDM responder delay, in 10ths of a microsecond.
 * @sa Transceiver_SetRDMResponderDelay.
 */
uint16_t Transceiver_GetRDMResponderDelay();

/**
 * @brief Configure the jitter added to the responder delay.
 * @param max_jitter the maximum jitter in 10ths of a microsecond. Set to 0 to
 *   disable jitter. Valid values are 0 to (20000 - Responder Delay).
 * @returns true if jitter time was updated, false if the value was out of
 *   range.
 *
 * The default value is 0.
 */
bool Transceiver_SetRDMResponderJitter(uint16_t max_jitter);

/**
 * @brief Return the RDM responder jitter.
 * @returns The RDM responder jitter, in 10ths of a microsecond.
 * @sa Transceiver_SetRDMResponderJitter.
 */
uint16_t Transceiver_GetRDMResponderJitter();

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_TRANSCEIVER_H_
