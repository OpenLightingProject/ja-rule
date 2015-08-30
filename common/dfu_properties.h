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
 * dfu_properties.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @{
 * @file dfu_properties.h
 * @brief Properties of our DFU implementation.
 *
 * The constants in this file are not expected to change.
 */

#ifndef COMMON_DFU_PROPERTIES_H_
#define COMMON_DFU_PROPERTIES_H_

/**
 * @brief The DFU interface index during DFU mode.
 *
 * This is always 0, per the spec.
 */
enum { DFU_MODE_DFU_INTERFACE_INDEX = 0 };

/**
 * @brief The DFU interface index during runtime mode.
 *
 * This is typically the last interface for a configuration.
 */
enum { RUNTIME_MODE_DFU_INTERFACE_INDEX = 3 };

/**
 * @brief Alternate settings for the DFU interface.
 *
 * By using alternate interfaces we can control which memory region we write to.
 */
typedef enum {
  DFU_ALT_INTERFACE_FIRMWARE = 0,  //!< The main application firmware
  DFU_ALT_INTERFACE_UID = 1,  //!< The UID region
} DFUInterfaceAlternateSetting;

/**
 * @brief The maximum size of the firmware blocks.
 *
 * Per the USB spec, this should be 8, 16, 32 or 64 bytes.
 */
enum { DFU_BLOCK_SIZE = 64 };

#endif  // COMMON_DFU_PROPERTIES_H_

/**
 * @}
 */
