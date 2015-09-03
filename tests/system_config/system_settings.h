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
 * system_settings.h
 * Copyright (C) 2015 Simon Newton
 */

#ifndef FIRMWARE_SRC_SYSTEM_CONFIG_MX_795_512L_SYSTEM_SETTINGS_H_
#define FIRMWARE_SRC_SYSTEM_CONFIG_MX_795_512L_SYSTEM_SETTINGS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file system_settings.h
 * @brief Configuration settings for the main application.
 *
 * These will need to be adjusted to suit the particular processor / board
 * used.
 *
 * @name Coarse Timer
 * Settings for the @ref timer. These are used to initialize
 * CoarseTimer_Settings.
 * @{
 */

/**
 * @brief The timer to use for the coarse timer.
 */
#define COARSE_TIMER_ID 2

/**
 * @}
 *
 * @name Transceiver
 * Settings for the @ref transceiver. These are used to initialize
 * TransceiverHardwareSettings.
 * @{
 */

/**
 * @brief The USART to use for the DMX/RDM transceiver.
 */
#define TRANSCEIVER_UART 1

/**
 * @brief The Timer module id to use for the DMX/RDM transceiver.
 */
#define TRANSCEIVER_TIMER 3

/**
 * @brief The input capture module id to use for the DMX/RDM transceiver.
 */
#define TRANSCEIVER_IC 2

/**
 * @brief The port to use for the direction & break pins.
 */
#define TRANSCEIVER_PORT PORT_CHANNEL_F

/**
 * @brief The bit position of the I/O pin used to create the break.
 */
#define TRANSCEIVER_PORT_BIT PORTS_BIT_POS_8

/**
 * @brief The bit position of the TX enable pin.
 */
#define TRANSCEIVER_TX_ENABLE_PORT_BIT PORTS_BIT_POS_0

/**
 * @brief The bit position of the RX enable pin.
 */
#define TRANSCEIVER_RX_ENABLE_PORT_BIT PORTS_BIT_POS_1

/**
 * @}
 *
 * @name RDM Responder
 * Settings for the @ref rdm_responder.h "RDM Responder". These are used to
 * initialize RDMResponderSettings.
 * @{
 */

/**
 * @brief The port containing the RDM identify LED.
 */
#define RDM_RESPONDER_IDENTIFY_PORT PORT_CHANNEL_D

/**
 * @brief The bit position of the RDM identify LED.
 */
#define RDM_RESPONDER_IDENTIFY_PORT_BIT PORTS_BIT_POS_3

/**
 * @brief The port containing the RDM mute status LED.
 */
#define RDM_RESPONDER_MUTE_PORT PORT_CHANNEL_D

/**
 * @brief The bit position of the RDM mute status LED.
 */
#define RDM_RESPONDER_MUTE_PORT_BIT PORTS_BIT_POS_12

/**
 * @}
 *
 * @name SPI DMX
 * Settings for the @ref spi_dmx. These are used to initialize
 * SPIRGBConfiguration.
 * @{
 */

/**
 * @brief The SPI module to use for output.
 */
#define SPI_MODULE_ID SPI_ID_1

/**
 * @brief The baud rate of the SPI output.
 */
#define SPI_BAUD_RATE 1000000u

/**
 * @brief Use enhanced buffering.
 */
#define SPI_USE_ENHANCED_BUFFERING true

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_SYSTEM_CONFIG_MX_795_512L_SYSTEM_SETTINGS_H_
