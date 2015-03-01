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
 * @brief The hardware settings to use for the Transceiver.
 */
typedef struct {
  USART_MODULE_ID usart;  //!< The USART module to use
  PORTS_CHANNEL port;  //!< The port to use for control signals.
  PORTS_BIT_POS break_bit;  //!< The bit to use to generate breaks.
  PORTS_BIT_POS rx_enable_bit;  //!< The RX Enable bit.
  PORTS_BIT_POS tx_enable_bit;  //!< The TX Enable bit.
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
 */
void Transceiver_QueueDMX(const uint8_t* data, unsigned int size);

void Transceiver_QueueDUB(const uint8_t* data, unsigned int size);

void Transceiver_QueueRDMRequest(const uint8_t* data, unsigned int size);

/**
 * @brief Reset the transceiver state.
 *
 * This can be used to recover from an error.
 */
void Transceiver_Reset();

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_TRANSCEIVER_H_
