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

#ifndef FIRMWARE_SRC_DFU_CONSTANTS_H_
#define FIRMWARE_SRC_DFU_CONSTANTS_H_

/**
 * @file dfu_constants.h
 * @brief Constants for Device Firmware Uploads
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
  DFU_DETATCH = 0,
  DFU_DNLOAD = 1,
  DFU_UPLOAD = 2,
  DFU_GETSTATUS = 3,
  DFU_CLRSTATUS = 4,
  DFU_GETSTATE = 5,
  DFU_ABORT = 6
};

typedef enum {
  APP_IDLE = 0,
  APP_DETACH = 1,
  DFU_IDLE = 2,
  DFU_DNLOAD_SYNC = 3,
  DFU_DNBUSY = 4,
  DFU_DNLOAD_IDLE = 5,
  DFU_MANUFEST_SYNC = 6,
  DFU_MANIFEST = 7,
  DFU_MANIFEST_WAIT_RESET = 8,
  DFU_UPLOAD_IDLE = 9,
  DFU_ERROR = 10
} DFUState;

typedef enum {
  DFU_STATUS_OK = 0x00,
  DFU_STATUS_ERR_TARGET = 0x01,
  DFU_STATUS_ERR_FILE = 0x02,
  DFU_STATUS_ERR_WROTE = 0x03,
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

#ifdef __cplusplus
}
#endif

#endif  // FIRMWARE_SRC_DFU_CONSTANTS_H_
