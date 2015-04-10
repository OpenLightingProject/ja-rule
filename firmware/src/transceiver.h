/*
 * File:   transceiver.h
 * Author: Simon Newton
 */

/**
 * @defgroup transceiver Transceiver
 * @brief The DMX512 / RDM Transceiver
 *
 * This module handles communications on the RS485 line.
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
 * Also there is a difference between DUB response and normal GET/SET responses
 * and the latter require a break.
 */
typedef enum {
  T_OP_TRANSCEIVER_NO_RESPONSE,  //!< No response (DMX512) or ASC.
  T_OP_RDM_DUB,  //!< An RDM Discovery Unique Branch
  T_OP_RDM_BROADCAST,  //!< A broadcast Get / Set Request.
  T_OP_RDM_WITH_RESPONSE  //!< A RDM Get / Set Request.
} TransceiverOperation;

/**
 * @brief The result of the transceiver operation.
 */
typedef enum {
  T_RC_COMPLETED_OK,  //!< The operation completed sucessfully.
  T_RC_TX_ERROR,  //!< A TX error occurred.
  T_RC_RX_TIMEOUT  //!< Non response was received within the RDM wait time.
} TransceiverResult;

/**
 * @brief The callback run when a frame transaction completes.
 * @param token The token associated with this operation. If the operation was
 *   not initiated by this device (i.e. we're operating as a receiver), the
 *   token will be 0.
 * @param operation The type of frame that was sent.
 * @param result The result of the operation.
 * @param data The received data, may be NULL if there was no response.
 * @param length The size of the received data.
 */
typedef bool (*TransceiverEventCallback)(uint8_t token,
                                         TransceiverOperation,
                                         TransceiverResult,
                                         const uint8_t*,
                                         unsigned int);

/**
 * @brief The hardware settings to use for the Transceiver.
 */
typedef struct {
  USART_MODULE_ID usart;  //!< The USART module to use
  PORTS_CHANNEL port;  //!< The port to use for control signals.
  PORTS_BIT_POS break_bit;  //!< The port bit to use to generate breaks.
  PORTS_BIT_POS rx_enable_bit;  //!< The RX Enable bit.
  PORTS_BIT_POS tx_enable_bit;  //!< The TX Enable bit.

  /**
   * @brief The callback to run when a transceiver event occurs.
   *
   * If PIPELINE_TRANSCEIVER_EVENT is defined in system_pipeline.h, the macro
   * will override this value.
   */
  TransceiverEventCallback callback;
} Transceiver_Settings;


/**
 * @brief Initialize the transceiver.
 * @param settings The settings to use for the transceiver.
 */
void Transceiver_Initialize(const Transceiver_Settings* settings);

/**
 * @brief Perform the periodic transceiver tasks.
 *
 * This should be called periodically.
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
 * @brief Queue a alternate start code (ASC) frame for transmission.
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
 * @brief Set the Break (space) time.
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
 * @brief Return the current configured Break time.
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
 * By setting the value less than 28, we can cause responsers that are at the
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
 * the end, in 10ths of a millisecond. Valid values are 10 - 50 (1 - 5ms).
 * Values < 28 are outside the specification but may be used for testing.
 * @returns true if time was updated, false if the value was out of range.
 *
 * The default value is 29 (2.9mS), see Line 3, Table 3-3, E1.20.
 *
 * By setting the value less than 28, we can cause responsers that are at the
 * limits of the specification to fail. By setting the value more than 28, we
 * can support responders that are out-of-spec.
 */
bool Transceiver_SetRDMDUBResponseTime(uint16_t wait_time);

/**
 * @brief Return the RDM RX packet
 * @returns The RDM wait time in 10ths of a millisecond.
 */
uint16_t Transceiver_GetRDMDUBResponseTime();

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_TRANSCEIVER_H_
