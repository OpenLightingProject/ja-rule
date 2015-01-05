/* 
 * File:   constants.h
 * Author: Simon Newton
 */

#ifndef SRC_CONSTANTS_H_
#define	SRC_CONSTANTS_H_

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

// *****************************************************************************
// Protocol specific constants
// *****************************************************************************

/**
 * @brief
 */
typedef enum {
  ECHO = 0x80,
  TX_DMX = 0x81
} Command;


// The Start of Message identifier.
#define START_OF_MESSAGE_ID 0x5a

// The End of Message Identifier.
#define END_OF_MESSAGE_ID 0xa5

// The OK return code.
#define RC_OK 0

#endif  // SRC_CONSTANTS_H_

