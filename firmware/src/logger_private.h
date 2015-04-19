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
 * logger_private.h
 * Copyright (C) 2015 Simon Newton
 */

#ifndef FIRMWARE_SRC_LOGGER_PRIVATE_H_
#define FIRMWARE_SRC_LOGGER_PRIVATE_H_

#include "constants.h"
#include "system_config.h"
#include "transport.h"

#ifdef __cplusplus
extern "C" {
#endif

// Since we may have data in two regions of the buffer and we need to prepend
// the flags byte, we may need up to 3 IOVecs
#define IOVEC_ARRAY_SIZE 3

typedef struct {
  TransportTXFunction tx_cb;
  int16_t read;  // next index to read from. Range -1 to LOG_BUFFER_SIZE -1
  int16_t write;  // next index to write to. Range 0 to LOG_BUFFER_SIZE - 1
  uint8_t enabled : 1;  // true if logging is enabled
  uint8_t overflow : 1;  // true if an overflow has occurred.
  uint16_t max_payload_size;  // the maximum size for a payload.
  IOVec iovec[IOVEC_ARRAY_SIZE];
  // The circular buffer for log data
  char log_buffer[LOG_BUFFER_SIZE];
} LoggerData;

#ifdef __cplusplus
}
#endif

#endif  // FIRMWARE_SRC_LOGGER_PRIVATE_H_
