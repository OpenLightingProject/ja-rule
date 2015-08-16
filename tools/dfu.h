/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Copyright (C) 2015 Simon Newton.
 */

#ifndef TOOLS_DFU_H_
#define TOOLS_DFU_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Options used for constructing the firmware blob.
 */
typedef struct {
  uint16_t vendor_id;  //!< The vendor ID to use in the DFU suffix
  uint16_t product_id;  //!< The product ID to use in the DFU suffix.
  bool invert_bytes;  //!< True to invert the  TODO(simon):....
} FirmwareOptions;

/**
 * @brief Write data to a DFU file.
 * @param options The firmware options.
 * @param data The contents of the DFU file
 * @param size The size of the data.
 * @param file The file to write to.
 *
 * This prepends a custom header (not part of the DFU spec) containing the
 * length of the DFU data and a CRC and appends the DFU
 * suffix.
 *
 * The reason for the prefix & suffix is that the DFU suffix is used by the
 * host side tool and not sent to the device. The prefix is a manufacturer
 * defined format, and we use it to pass the length & CRC to the device.
 */
bool WriteDFUFile(const FirmwareOptions *options,
                  const uint8_t *data,
                  unsigned int size,
                  const char *file);

#endif  // TOOLS_DFU_H_
