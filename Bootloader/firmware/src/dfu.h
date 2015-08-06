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
 * dfu.h
 * Copyright (C) 2015 Simon Newton
 */

#ifndef FIRMWARE_SRC_DFU_H_
#define FIRMWARE_SRC_DFU_H_

/**
 * @file dfu.h
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

#ifdef __cplusplus
}
#endif

#endif  // FIRMWARE_SRC_DFU_H_
