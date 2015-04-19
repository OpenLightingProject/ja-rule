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
 * iovec.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @addtogroup transport
 * @{
 * @file iovec.h
 * @brief Vector I/O.
 */

#ifndef FIRMWARE_SRC_IOVEC_H_
#define FIRMWARE_SRC_IOVEC_H_

#include <stdint.h>

/**
 * @brief An IOVector, with a pointer to memory and a length attribute.
 */
typedef struct {
  const void* base;   //!< A pointer to the data
  unsigned int length;  //!< The size of the data
} IOVec;

#endif  // FIRMWARE_SRC_IOVEC_H_

/**
 * @}
 */
