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

#include "system_config.h"
#include "peripheral/usart/plib_usart.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The type of frame transaction.
 *
 * Certain start-codes such as RDM may result in bi-directional communication.
 */
typedef enum {
  TRANSCEIVER_NO_RESPONSE,  //!< No response (DMX512), ot
  RDM_DUB,  //!< An RDM Discovery Unique Branch
  RDM_WITH_RESPONSE  //!< A RDM Request
} TransceiverFrameType;

typedef enum {
  COMPLETED_OK,
  TX_ERROR,
  RX_TIMEOUT
} TransceiverResult;

/**
 * @brief The callback run when a frame transaction completes.
 * @param frame_type The type of frame that was sent.
 * @param result The result of the operation.
 * @param data The received data, may be NULL if there was no response.
 * @param length The size of the received data.
 */
typedef bool (*TransceiverCallback)(TransceiverFrameType, TransceiverResult,
                                    const uint8_t*, unsigned int);


/**
 * @brief The hardware settings to use for the Transceiver.
 */
typedef struct {
  USART_MODULE_ID usart;  //!< The USART module to use
  PORTS_CHANNEL port;  //!< The port to use for control signals.
  PORTS_BIT_POS break_bit;  //!< The bit to use to generate breaks.
  PORTS_BIT_POS rx_enable_bit;  //!< The RX Enable bit.
  PORTS_BIT_POS tx_enable_bit;  //!< The TX Enable bit.

  /**
   * @brief The callback to run when each frame is transmitted.
   *
   * If 
   */
  TransceiverCallback callback;
} Transceiver_Settings;


/**
 * @brief Initialize the DMX layer.
 * @param settings The settings to use for the transceiver.
 */
void Transceiver_Initialize(const Transceiver_Settings* settings);

/**
 * @brief Perform the periodic DMX layer tasks
 */
void Transceiver_Tasks();

/**
 * @brief Queue a DMX frame for transmission.
 * @param data The DMX data in the frame. May be partial data.
 * @param size The size of the DMX data.
 * @returns true if the frame was accepted and buffered, false if the transmit
 *   buffer is full.
 */
bool Transceiver_QueueDMX(uint8_t token, const uint8_t* data,
                          unsigned int size);

bool Transceiver_QueueDUB(uint8_t token, const uint8_t* data,
                          unsigned int size);

bool Transceiver_QueueRDMRequest(uint8_t token, const uint8_t* data,
                                 unsigned int size);

/**
 * @brief Reset the transceiver state.
 *
 * This can be used to recover from an error.
 */
void Transceiver_Reset();


/**
 * @brief Set the break (space) time.
 * @param break_time_us. The break time in micro-seconds, values are 44 to 800.
 * @returns true if the break time was updated, false if the value was out of
 *   range.
 *
 * The default time is 176 uS. Table 6 in E1.11 lists the minimum break time as
 * 92uS but the 1990 standard allows 88uS. We go down to 44 uS for testing
 * purposes.
 */
bool Transceiver_SetBreakTime(uint16_t break_time_us);

/**
 * @brief Return the break time.
 * @returns The break time in micro-seconds.
 */
uint16_t Transceiver_GetBreakTime();

/**
 * @brief Set the mark-after-break time.
 * @param mark_time_us. The mark time in micro-seconds, values are 4 to 800
 * @returns true if the mark time was updated, false if the value was out of
 *   range.
 *
 * The default is 12uS. Table 6 in E1.11 allows 12uS to 1s. We go down to 4 uS
 * for testing purposes.
 */
bool Transceiver_SetMarkTime(uint16_t mark_time_us);

/**
 * @brief Return the MAB time.
 * @returns The MAB time in micro-seconds.
 */
uint16_t Transceiver_GetMarkTime();

/**
 * @brief 
 */
bool Transceiver_SetRDMBroadcastListen(uint16_t delay);
uint16_t Transceiver_GetRDMBroadcastListen();

/**
 * @brief Configured the time to wait before considering a RDM response lost.
 * @param The time to wait in 10ths of a millisecond. Valid values are 10 -
 *   50. Values < 28 are outside the specification but may be used for
 *   testing.
 * @returns true if time was updated, false if the value was out of range.
 *
 * The default value is 28 (2.8mS), see Line 3, Table 3-2, E1.20.
 *
 * By setting the value less than 28, we can cause responsers that are at the
 * limits of the specification to fail. By setting the value more than 28, we
 * can support responders that are out-of-spec.
 */
bool Transceiver_SetRDMWaitTime(uint16_t wait_time);

/**
 * @brief Return the RDM wait time.
 * @returns The RDM wait time in 10ths of a milli-second.
 */
uint16_t Transceiver_GetRDMWaitTime();


#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_TRANSCEIVER_H_
