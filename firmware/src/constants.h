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
  ECHO = 0x80,
  TX_DMX = 0x81,
  GET_LOG = 0x82,
  GET_FLAGS = 0x83,
  WRITE_LOG = 0x84,
  COMMAND_RESET_DEVICE = 0x85,
  COMMAND_RDM_DUB_REQUEST = 0x86,
  COMMAND_RDM_REQUEST = 0x87,
  SET_BREAK_TIME = 0x88,
  GET_BREAK_TIME = 0x89,
  SET_MAB_TIME = 0x90,
  GET_MAB_TIME = 0x91,
  SET_RDM_BROADCAST_LISTEN = 0x92,
  GET_RDM_BROADCAST_LISTEN = 0x93,
  SET_RDM_WAIT_TIME = 0x94,
  GET_RDM_WAIT_TIME = 0x95,
  COMMAND_RDM_BROADCAST_REQUEST = 0x96
} Command;

/**
 * @brief JaRule message return codes.
 */
typedef enum {
  RC_OK,
  RC_UNKNOWN,
  RC_BUFFER_FULL,
  RC_BAD_PARAM,
  RC_TX_ERROR,
  RC_RX_TIMEOUT,
  RC_RX_BCAST_RESPONSE,
  RC_RX_INVALID_RESPONSE,
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
#define DEFAULT_RDM_BROADCAST_LISTEN 28

/**
 * @brief The default time to wait for a response after sending an RDM message.
 *
 * Measured is in 10ths of a millisecond, from Line 1 & 3, Table 3-2, E1.20.
 * Responders have 2ms (see Table 3-4), and then in line proxies can introduce
 * up to 704uS of delay. This rounds to 2.8 ms.
 */
#define DEFAULT_RDM_WAIT_TIME 28

/**
 * @brief The default time a RDM DUB response can take.
 *
 * Measured in in 10ths of a microsecond. From Line 3, Table 3-3, E1.20.
 */
#define DEFAULT_RDM_DUB_RESPONSE_TIME 29000

/**
 * @brief The minimum break time for controllers to receive.
 *
 * Measured in 10ths of a microsecond. The value is from line 2 of Table 3-1
 * in E1.20.
 */
#define CONTROLLER_RX_BREAK_TIME_MIN 880

/**
 * @brief The maximum break time for controllers to receive.
 *
 * Measured in 10ths of a microsecond. The value is from line 2 of Table 3-1
 * in E1.20.
 */
#define CONTROLLER_RX_BREAK_TIME_MAX 3520

#endif  // FIRMWARE_SRC_CONSTANTS_H_

/**
 * @}
 */
