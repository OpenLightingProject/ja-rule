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
 * flags_private.h
 * Copyright (C) 2015 Simon Newton
 */

#ifndef FIRMWARE_SRC_FLAGS_PRIVATE_H_
#define FIRMWARE_SRC_FLAGS_PRIVATE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// See flags.h for what each flag tracks.
typedef struct {
  uint8_t log_overflow : 1;
  uint8_t tx_drop : 1;
  uint8_t tx_error : 1;
} FlagsState;

typedef struct {
  bool has_changed;
  FlagsState flags;
} FlagsData;

#ifdef __cplusplus
}
#endif

#endif  // FIRMWARE_SRC_FLAGS_PRIVATE_H_
