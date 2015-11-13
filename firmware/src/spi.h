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
 * spi.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @defgroup spi SPI
 * @brief SPI Driver
 *
 * This driver allows multiple clients to share the SPI bus. This assumes that
 * all clients use the same SPI configuration. If that isn't the case we'll
 * need to introduce client handles or something.
 *
 * Clients can queue an SPI transfer with the SPI_QueueTransfer() method. The
 * callback argument can be used to specify a callback to be run before and
 * after the transfer is performed. This callback can be used to set the
 * relevant chip-enable line.
 *
 * @addtogroup spi
 * @{
 * @file spi.h
 * @brief SPI Driver
 */

#ifndef FIRMWARE_SRC_SPI_H_
#define FIRMWARE_SRC_SPI_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief SPI Event types.
 */
typedef enum {
  SPI_BEGIN_TRANSFER,
  SPI_COMPLETE_TRANSFER
} SPIEventType;

/**
 * @brief The callback run before or after the SPI transfer begins.
 * @param event The type of SPI event that occurs.
 */
typedef void (*SPI_Callback)(SPIEventType event);

/**
 * @brief Queue an SPI transfer.
 * @param output The output buffer to send. If null 0s will be sent.
 * @param output_length The size of the output buffer.
 * @param input The location to store received data, may be NULL.
 * @param input_length The length of the input data buffer.
 * @param callback The callback run prior and post this transfer.
 * @returns True if the transfer was scheduled, false if the queue was full.
 *
 * The number of bytes sent will be the maximum of (output_length,
 * input_length). If output_length < input_length, the output data will be
 * padded with 0s.
 */
bool SPI_QueueTransfer(const uint8_t *output,
                       unsigned int output_length,
                       uint8_t *input,
                       unsigned int input_length,
                       SPI_Callback callback);
/**
 * @brief Initialize the SPI driver.
 */
void SPI_Initialize();

/**
 * @brief The tasks function, this should be called from the main event loop.
 */
void SPI_Tasks();

#ifdef __cplusplus
}
#endif

#endif  // FIRMWARE_SRC_SPI_H_

/**
 * @}
 */
