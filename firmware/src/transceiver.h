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
 * The transceiver can be in either controller or responder mode.
 *
 * @par Controller Mode
 *
 * In controller mode, operations can be triggered by calling one of:
 *  - Transceiver_QueueDMX();
 *  - Transceiver_QueueASC();
 *  - Transceiver_QueueRDMDUB();
 *  - Transceiver_QueueRDMRequest();
 *
 * When the operation completes, the TransceiverEventCallback will be run, with
 * the result of the operation. See @ref controller_sm
 * "Controller State Machine".
 *
 * @par Responder Mode
 *
 * In responder mode, the TransceiverEventCallback will be run when a frame is
 *   received. See @ref responder_sm "Responder State Machine".
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

#include "system_config.h"
#include "peripheral/ports/plib_ports.h"
#include "peripheral/usart/plib_usart.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Identifies the type of transceiver operation.
 *
 * Certain start-codes such as RDM may result in bi-directional communication.
 * There is also a difference between DUB response and normal GET/SET responses
 * as the latter require a break.
 */
typedef enum {
  T_OP_TX_ONLY,  //!< No response (DMX512) or ASC.
  T_OP_RDM_DUB,  //!< An RDM Discovery Unique Branch
  T_OP_RDM_BROADCAST,  //!< A broadcast Get / Set Request.
  T_OP_RDM_WITH_RESPONSE,  //!< A RDM Get / Set Request.
  T_OP_RX  //!< Receive mode.
} TransceiverOperation;

/**
 * @brief The result of an operation.
 */
typedef enum {
  /**
   * @brief The frame was sent sucessfully and no response was expected.
   */
  T_RESULT_TX_OK,
  T_RESULT_TX_ERROR,  //!< A TX error occurred.
  T_RESULT_RX_DATA,  //!< Data was received.
  T_RESULT_RX_TIMEOUT,  //!< No response was received within the RDM wait time.
  T_RESULT_RX_INVALID,  //!< Invalid data received.

  T_RESULT_RX_START_FRAME,  //!< A frame was received
  T_RESULT_RX_CONTINUE_FRAME  //!< A frame was received
} TransceiverOperationResult;

/**
 * @brief The timing measurements for an operation.
 */
typedef union {
  /**
   * @brief The timing measurments for a DUB transaction.
   *
   * All times are measured in 10ths of a microsecond from the end of the DUB
   * frame.
   */
  struct {
    uint16_t start;  //!< The start of the discovery response.
    uint16_t end;  //!< The end of the discovery response.
  } dub_response;

  /**
   * @brief The timing measurments for a Get / Set transaction.
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
 * @brief A transceiver event.
 *
 * In controller mode an event occurs when:
 *  - A DMX frame has been completely sent.
 *  - A TX error occured.
 *  - A RDM frame has been broadcast.
 *  - A RDM response (either DUB or Get/Set) has been received.
 *  - A RDM timeout has occured.
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
  uint8_t token;

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
 */
typedef struct {
  USART_MODULE_ID usart;  //!< The USART module to use
  PORTS_CHANNEL port;  //!< The port to use for control signals.
  PORTS_BIT_POS break_bit;  //!< The port bit to use to generate breaks.
  PORTS_BIT_POS rx_enable_bit;  //!< The RX Enable bit.
  PORTS_BIT_POS tx_enable_bit;  //!< The TX Enable bit.
} TransceiverHardwareSettings;

/**
 * @brief Initialize the transceiver.
 * @param settings The settings to use for the transceiver.
 * @param callback The callback to run when a transceiver event occurs.
 *
 * If PIPELINE_TRANSCEIVER_EVENT is defined in system_pipeline.h, the macro
 * will override this value.
 */
void Transceiver_Initialize(const TransceiverHardwareSettings *settings,
                            TransceiverEventCallback callback);

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
bool Transceiver_QueueDMX(uint8_t token, const uint8_t* data,
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
bool Transceiver_QueueASC(uint8_t token, uint8_t start_code,
                          const uint8_t* data, unsigned int size);

/**
 * @brief Queue an RDM DUB operation.
 * @param token The token for this operation.
 * @param data The RDM DUB data, excluding the start code.
 * @param size The size of the RDM DUB data, excluding the start code.
 * @returns true if the frame was accepted and buffered, false if the transmit
 *   buffer is full.
 */
bool Transceiver_QueueRDMDUB(uint8_t token, const uint8_t* data,
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
bool Transceiver_QueueRDMRequest(uint8_t token, const uint8_t* data,
                                 unsigned int size, bool is_broadcast);

/**
 * @brief Reset the transceiver state.
 *
 * This can be used to recover from an error. The line will be placed back into
 * a MARK state.
 */
void Transceiver_Reset();

/**
 * @brief Set the break (space) time.
 * @param break_time_us the break time in micro-seconds, values are 44 to 800
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
 * @returns The break time in micro-seconds.
 */
uint16_t Transceiver_GetBreakTime();

/**
 * @brief Set the mark-after-break (MAB) time.
 * @param mark_time_us the mark time in micro-seconds, values are 4 to 800
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
 * @returns The MAB time in micro-seconds.
 */
uint16_t Transceiver_GetMarkTime();

/**
 * @brief Set the time to wait after RDM broadcasts.
 * @param delay the broadcast listen delay, in 10ths of a millisecond. Valid
 *   values are 0 to 50 (0 to 5ms).
 * @returns true if the broadcast listen delay was updated, false if the value
 *   was out of range.
 *
 * With the exception of a DUB, an RDM controller usually doesn't listen to the
 * line after sending a broadcast command. However for testing purposes we
 * want to be able to listen for responders that incorrectly reply.
 */
bool Transceiver_SetRDMBroadcastListen(uint16_t delay);

/**
 * @brief Return the current configured RDM broadcast listen time.
 * @returns The RDM broadcast listen time, in 10ths of a millisecond.
 */
uint16_t Transceiver_GetRDMBroadcastListen();

/**
 * @brief Set the time to wait for a response before considering a RDM response
 *   lost.
 * @param wait_time the time to wait in 10ths of a millisecond. Valid values
 *   are 10 - 50 (1 - 5ms). Values < 28 are outside the specification but may
 *   be used for testing.
 * @returns true if time was updated, false if the value was out of range.
 *
 * This is used for both DISCOVERY and GET/SET commands. However the limits for
 * broadcast commands is controlled with Transceiver_SetRDMBroadcastListen().
 *
 * The default value is 28 (2.8mS), see Lines 1 & 3, Table 3-2, E1.20.
 *
 * By setting the value less than 28, we can cause responders that are at the
 * limits of the specification to fail. By setting the value more than 28, we
 * can support responders that are out-of-spec.
 */
bool Transceiver_SetRDMWaitTime(uint16_t wait_time);

/**
 * @brief Return the RDM wait time.
 * @returns The RDM wait time in 10ths of a millisecond.
 */
uint16_t Transceiver_GetRDMWaitTime();

/**
 * @brief Configure the maximum time that a RDM DUB response packet can take.
 * @param wait_time the time to wait from the start of the DUB response until
 * the end, in 10ths of a microseconds. Valid values are 10000 - 35000
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
bool Transceiver_SetRDMDUBResponseTime(uint16_t wait_time);

/**
 * @brief Return the RDM RX packet
 * @returns The RDM wait time in 10ths of a microsecond.
 */
uint16_t Transceiver_GetRDMDUBResponseTime();

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_TRANSCEIVER_H_
