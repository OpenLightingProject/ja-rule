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
 * crc.h
 * Copyright (C) 2015 Simon Newton
 */

#ifndef BOOTLOADER_FIRMWARE_SRC_CRC_H_
#define BOOTLOADER_FIRMWARE_SRC_CRC_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @{
 * @file crc.h
 * @brief CRC Calculations.
 */

/**
 * @brief Calculate the CRC of a data block.
 *   complete.
 * @param crc The original CRC.
 * @param data The data
 * @param size The size of the data.
 * @returns The updated CRC.
 */
uint32_t CalculateCRC(uint32_t crc, const uint8_t *data, unsigned int size);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // BOOTLOADER_FIRMWARE_SRC_CRC_H_
