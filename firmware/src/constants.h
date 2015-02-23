/* 
 * File:   constants.h
 * Author: Simon Newton
 */

#ifndef FIRMWARE_SRC_CONSTANTS_H_
#define FIRMWARE_SRC_CONSTANTS_H_

// *****************************************************************************
// USB specific constants
// *****************************************************************************

// The USB Vendor ID.
#define USB_DEVICE_VENDOR_ID 0x04D8

// The USB Product ID.
#define USB_DEVICE_PRODUCT_ID 0x0053

// The maximum size of a USB packet to / from the bulk endpoint.
// 64 is the highest value a full speed, bulk endpoint can use.
#define USB_MAX_PACKET_SIZE 64

// The maximum transfer size of a USB command, this should be a multiple of
// USB_MAX_PACKET_SIZE.
#define USB_READ_BUFFER_SIZE 576

// The polling interval for the bulk endpoint in milliseconds.
// 1ms is the shortest polling interval
#define USB_POLLING_INTERVAL 1

// *****************************************************************************
// DMX512 specific constants
// *****************************************************************************

// The maximum size of a DMX frame, include the start code.
#define DMX_FRAME_SIZE 513

// The Null Start Code (NSC).
#define NULL_START_CODE 0x00

// The Baud rate for DMX / RDM.
#define DMX_BAUD 250000     // 250kHz

// *****************************************************************************
// RDM specific constants
// *****************************************************************************

#define RDM_START_CODE 0xcc

// *****************************************************************************
// Protocol specific constants
// *****************************************************************************

/**
 * @brief
 */
typedef enum {
  ECHO = 0x80,
  TX_DMX = 0x81,
  GET_LOG = 0x82,
  GET_FLAGS = 0x83,
  WRITE_LOG = 0x84,
  COMMAND_RESET_DEVICE = 0x85,
  COMMAND_RDM_DUB_REQUEST = 0x86,
  COMMANE_RDM_REQUEST = 0x87
} Command;

typedef enum {
  RC_OK = 0,
  RC_UNKNOWN = 1,
} ReturnCodes;

// The Start of Message identifier.
#define START_OF_MESSAGE_ID 0x5a

// The End of Message Identifier.
#define END_OF_MESSAGE_ID 0xa5

// The maximum data payload size
#define PAYLOAD_SIZE 513

#endif  // FIRMWARE_SRC_CONSTANTS_H_

