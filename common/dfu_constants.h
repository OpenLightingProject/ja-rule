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
 * dfu_constants.h
 * Copyright (C) 2015 Simon Newton
 */

#ifndef COMMON_DFU_CONSTANTS_H_
#define COMMON_DFU_CONSTANTS_H_

/**
 * @file dfu_constants.h
 * @brief Constants for Device Firmware Uploads
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief DFU message types
 */
enum {
  DFU_DETATCH = 0,
  DFU_DNLOAD = 1,
  DFU_UPLOAD = 2,
  DFU_GETSTATUS = 3,
  DFU_CLRSTATUS = 4,
  DFU_GETSTATE = 5,
  DFU_ABORT = 6
};

/**
 * @brief DFU States
 */
typedef enum {
  APP_STATE_IDLE = 0,
  APP_STATE_DETACH = 1,
  DFU_STATE_IDLE = 2,
  DFU_STATE_DNLOAD_SYNC = 3,
  DFU_STATE_DNBUSY = 4,
  DFU_STATE_DNLOAD_IDLE = 5,
  DFU_STATE_MANIFEST_SYNC = 6,
  DFU_STATE_MANIFEST = 7,
  DFU_STATE_MANIFEST_WAIT_RESET = 8,
  DFU_STATE_UPLOAD_IDLE = 9,
  DFU_STATE_ERROR = 10
} DFUState;

/**
 * @brief DFU status codes
 */
typedef enum {
  DFU_STATUS_OK = 0x00,
  DFU_STATUS_ERR_TARGET = 0x01,
  DFU_STATUS_ERR_FILE = 0x02,
  DFU_STATUS_ERR_WRITE = 0x03,
  DFU_STATUS_ERR_ERASE = 0x04,
  DFU_STATUS_ERR_CHECK_ERASED = 0x05,
  DFU_STATUS_ERR_PROG = 0x06,
  DFU_STATUS_ERR_VERIFY = 0x07,
  DFU_STATUS_ERR_ADDRESS = 0x08,
  DFU_STATUS_ERR_NOT_DONE = 0x09,
  DFU_STATUS_ERR_FIRMWARE = 0x0a,
  DFU_STATUS_ERR_VENDOR = 0x0b,
  DFU_STATUS_ERR_USBR = 0x0c,
  DFU_STATUS_ERR_POR = 0x0d,
  DFU_STATUS_ERR_UNKNOWN = 0x0e,
  DFU_STATUS_ERR_STALLED_PKT = 0x0f
} DFUStatus;

/**
 * @brief DFU device attributes
 */
enum {
  DFU_CAN_DOWNLOAD = 0x01,  //!< Download supported
  DFU_CAN_UPLOAD = 0x02,  //!< Upload supported
  DFU_MANIFESTATION_TOLERANT = 0x04,  //!< Manifestation tolerant
  DFU_WILL_DETACH = 0x08  //!< Will detach without timer
};

/**
 * @brief The size of a GET STATUS response.
 */
enum { GET_STATUS_RESPONSE_SIZE = 6 };

/**
 * @brief The maximum size of the firmware blocks.
 *
 * Per the USB spec, this should be 8, 16, 32 or 64 bytes.
 */
enum { DFU_BLOCK_SIZE = 64 };

#ifdef __cplusplus
}
#endif

#endif  // COMMON_DFU_CONSTANTS_H_
