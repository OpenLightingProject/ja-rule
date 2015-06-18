/*
 * File:   system_settings.h
 * Author: Simon Newton
 */

/**
 * @{
 * @file system_settings.h
 * @brief The compile time settings for the device.
 */

#ifndef FIRMWARE_SRC_SYSTEM_CONFIG_MX_795_512L_SYSTEM_SETTINGS_H_
#define FIRMWARE_SRC_SYSTEM_CONFIG_MX_795_512L_SYSTEM_SETTINGS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The port used used by the RDM Responder.
 */
#define RDM_RESPONDER_PORT

/**
 * @brief The pin used for IDENTIFY_DEVICE
 */
#define RDM_RESPONDER_IDENTIFY_PORT_BIT

/**
 * @brief The pin used to indicate mute status.
 */
#define RDM_RESPONDER_MUTE_PORT_BIT

/**
 * @brief The USART to use for the Transceiver.
 */
#define TRANSCEIVER_TX_UART

/**
 * @brief The port used to create the breaks.
 *
 * See TRANSCEIVER_PORT_BIT.
 */
#define TRANSCEIVER_PORT

/**
 * @brief The pin used to create the breaks.
 *
 * TRANSCEIVER_PORT and TRANSCEIVER_PORT_BIT should be chosen so that they
 * share a pin with the USART TX.
 */
#define TRANSCEIVER_PORT_BIT

/**
 * @brief The pin used to enable transmitting.
 */
#define TRANSCEIVER_TX_ENABLE

/**
 * @brief The pin used to enable receiving.
 *
 * This pin is pulled low to enable RX, since the MAX485 (and apparently other
 * drivers) all have an inverter on the R/E pin.
 */
#define TRANSCEIVER_RX_ENABLE

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_SYSTEM_CONFIG_MX_795_512L_SYSTEM_SETTINGS_H_

