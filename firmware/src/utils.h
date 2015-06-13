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
 * utils.h
 * Copyright (C) 2015 Simon Newton
 */

#ifndef FIRMWARE_SRC_UTILS_H_
#define FIRMWARE_SRC_UTILS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Return the LSB from a 16bit value.
 * @param s The 16bit value to split.
 * @returns The lower 8 bits of the uint16_t.
 */
static inline uint8_t ShortLSB(uint16_t s) {
  return s & 0xff;
}

/**
 * @brief Return the MSB from a 16bit value.
 * @param s The 16bit value to split.
 * @returns The upper 8 bits of the uint16_t.
 */
static inline uint8_t ShortMSB(uint16_t s) {
  return s >> 8;
}

#ifdef __cplusplus
}
#endif

#endif  // FIRMWARE_SRC_UTILS_H_
