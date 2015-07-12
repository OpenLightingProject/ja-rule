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
 * constants.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @{
 * @file constants.h
 * @brief Various constants.
 *
 * This file defines constants that are not expected to change. Constants that
 * are board specific should be placed in the system_config/ directory.
 */

#ifndef FIRMWARE_SRC_CONSTANTS_H_
#define FIRMWARE_SRC_CONSTANTS_H_

// *****************************************************************************
// USB specific constants
// *****************************************************************************

/**
 * @brief The USB Vendor ID.
 */
#define USB_DEVICE_VENDOR_ID 0x04D8

/**
 * @brief The USB Product ID.
 *
 * @todo Apply for a product ID.
 */
#define USB_DEVICE_PRODUCT_ID 0x0053

/**
 * @brief The maximum size of a USB packet to / from the bulk endpoint.
 *
 * 64 bytes is the highest value a full speed, bulk endpoint can use.
 */
#define USB_MAX_PACKET_SIZE 64

/**
 * @brief The maximum transfer size of a Ja Rule USB command.
 *
 * This should be a multiple of USB_MAX_PACKET_SIZE.
 */
#define USB_READ_BUFFER_SIZE 576

/**
 * @brief The polling interval for the bulk endpoint in milliseconds.
 *
 * 1ms is the shortest polling interval USB allows.
 */
#define USB_POLLING_INTERVAL 1

// *****************************************************************************
// DMX512 specific constants
// *****************************************************************************

/**
 * @brief The maximum size of a DMX frame, excluding the start code.
 */
#define DMX_FRAME_SIZE 512

/**
 * @brief The Null Start Code (NSC).
 */
#define NULL_START_CODE 0x00

/**
 * @brief The Baud rate for DMX / RDM.
 */
#define DMX_BAUD 250000  // 250kHz

// *****************************************************************************
// RDM specific constants
// *****************************************************************************

/**
 * @brief The RDM Start Code.
 */
#define RDM_START_CODE 0xcc

/**
 * @brief The RDM Sub-start Code.
 */
#define RDM_SUB_START_CODE 0x01

// *****************************************************************************
// Protocol specific constants
// *****************************************************************************

/**
 * @brief The Ja Rule message commands.
 */
typedef enum {
  // Base commands
  /**
   * @brief Reset the device.
   * See @ref message_cmd_reset.
   */
  COMMAND_RESET_DEVICE = 0x00,

  /**
   * @brief Change the operating mode of the device.
   * @sa @ref message_cmd_setmode.
   */
  COMMAND_SET_MODE = 0x01,

  // User Configuration
  /**
   * @brief Set the break time of the transceiver.
   * See @ref message_cmd_setbreak
   */
  COMMAND_SET_BREAK_TIME = 0x10,

  /**
   * @brief Fetch the current transceiver break time.
   * See @ref message_cmd_getbreak
   */
  COMMAND_GET_BREAK_TIME = 0x11,

  /**
   * @brief Set the mark-after-break time of the transceiver.
   * See @ref message_cmd_setmark
   */
  COMMAND_SET_MARK_TIME = 0x12,

  /**
   * @brief Fetch the current transceiver mark-after-break time.
   * See @ref message_cmd_getmark
   */
  COMMAND_GET_MARK_TIME = 0x13,

  // Advanced Configuration
  /**
   * @brief Set the RDM Broadcast timeout.
   * See @ref message_cmd_setrdmbcasttimeout.
   */
  COMMAND_SET_RDM_BROADCAST_TIMEOUT = 0x20,

  /**
   * @brief Get the RDM Broadcast timeout.
   * See @ref message_cmd_getrdmbcasttimeout.
   */
  COMMAND_GET_RDM_BROADCAST_TIMEOUT = 0x21,

  /**
   * @brief Set the RDM Response timeout.
   * See @ref message_cmd_setrdmresponsetimeout.
   */
  COMMAND_SET_RDM_RESPONSE_TIMEOUT = 0x22,

  /**
   * @brief Get the RDM Response timeout.
   * See @ref message_cmd_getrdmresponsetimeout.
   */
  COMMAND_GET_RDM_RESPONSE_TIMEOUT = 0x23,

  /**
   * @brief Set the RDM DUB Response limit.
   * See @ref message_cmd_setrdmdubresponselimit.
   */
  COMMAND_SET_RDM_DUB_RESPONSE_LIMIT = 0x24,

  /**
   * @brief Get the RDM Response limit.
   * See @ref message_cmd_getrdmdubresponselimit.
   */
  COMMAND_GET_RDM_DUB_RESPONSE_LIMIT = 0x25,

  /**
   * @brief Set the RDM responder delay.
   * See @ref message_cmd_setrdmresponderdelay.
   */
  COMMAND_SET_RDM_RESPONDER_DELAY = 0x26,

  /**
   * @brief Get the RDM responder delay.
   * See @ref message_cmd_getrdmresponderdelay.
   */
  COMMAND_GET_RDM_RESPONDER_DELAY = 0x27,

  /**
   * @brief Set the RDM responder jitter.
   * See @ref message_cmd_setrdmresponderjitter.
   */
  COMMAND_SET_RDM_RESPONDER_JITTER = 0x28,

  /**
   * @brief Get the RDM responder jitter.
   * See @ref message_cmd_getrdmresponderjitter.
   */
  COMMAND_GET_RDM_RESPONDER_JITTER = 0x29,

  // DMX
  TX_DMX = 0x30,  //!< Transmit a DMX frame. See @ref message_cmd_txdmx.

  // RDM
  /**
   * @brief Send an RDM Discovery Unique Branch and wait for a response.
   * See @ref message_cmd_rdmdub.
   */
  COMMAND_RDM_DUB_REQUEST = 0x40,

  /**
   * @brief Send an RDM Get / Set command.
   * See @ref message_cmd_rdm
   */
  COMMAND_RDM_REQUEST = 0x41,

  /**
   * @brief Send a broadcast RDM command.
   * See @ref message_cmd_rdmbcast.
   */
  COMMAND_RDM_BROADCAST_REQUEST = 0x42,

  // Experimental / testing
  ECHO = 0xf0,  //!< Echo the data back. See @ref message_cmd_echo
  GET_LOG = 0xf1,  //!< Fetch more log data
  GET_FLAGS = 0xf2,  //!< Get the flags state
  WRITE_LOG = 0xf3
} Command;

/**
 * @brief JaRule command return codes.
 */
typedef enum {
  RC_OK,  //!< The command completed successfully.
  RC_UNKNOWN,  //!< Unknown command
  /**
   * @brief The command could not be completed due to a full memory buffer
   */
  RC_BUFFER_FULL,
  RC_BAD_PARAM,  //!< The command was malformed.
  RC_TX_ERROR,  //!< There was an error during transceiver transmit.
  RC_RDM_TIMEOUT,  //!< No response was received.

  /**
   * @brief Data was received in response to a broadcast RDM command.
   *
   * This usually indicates a broken responder.
   */
  RC_RDM_BCAST_RESPONSE,
  RC_RDM_INVALID_RESPONSE,  //!< An invalid RDM response was received.
} ReturnCode;

/**
 * @brief The Start of Message identifier.
 */
#define START_OF_MESSAGE_ID 0x5a

/**
 * @brief The End of Message Identifier.
 */
#define END_OF_MESSAGE_ID 0xa5

/**
 * @brief The maximum payload size in a message.
 */
#define PAYLOAD_SIZE 513

/**
 * @brief The break time in microseconds.
 */
#define DEFAULT_BREAK_TIME 176

/**
 * @brief The mark time in microseconds.
 */
#define DEFAULT_MARK_TIME 12

/**
 * @brief The time to listen for a response after sending an RDM broadcast.
 *
 * Measured in 10ths of a millisecond. This can be 0, since we don't expect
 * responses from broadcast messages, however by waiting we can detect bad
 * responders, so we set this the same as DEFAULT_RDM_WAIT_TIME.
 */
#define DEFAULT_RDM_BROADCAST_TIMEOUT 28

/**
 * @brief The default RDM response timeout for a controller.
 *
 * Measured is in 10ths of a millisecond, from Line 1 & 3, Table 3-2, E1.20.
 * Responders have 2ms (see Table 3-4), and then in line proxies can introduce
 * up to 704uS of delay. This rounds to 2.8 ms.
 */
#define DEFAULT_RDM_RESPONSE_TIMEOUT 28

/**
 * @brief The default maximum time an RDM DUB response can take.
 *
 * Measured in in 10ths of a microsecond. From Line 3, Table 3-3, E1.20.
 */
#define DEFAULT_RDM_DUB_RESPONSE_LIMIT 29000

/**
 * @brief The default time to wait before sending an RDM response.
 * @sa Transceiver_SetRDMResponderDelay.
 *
 * Measured in in 10ths of a microsecond. From Table 3-4, E1.20.
 */
#define DEFAULT_RDM_RESPONDER_DELAY 1760

#endif  // FIRMWARE_SRC_CONSTANTS_H_

/**
 * @}
 */
