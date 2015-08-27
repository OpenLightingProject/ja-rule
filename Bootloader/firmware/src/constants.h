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
 * constants.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @{
 * @file constants.h
 * @brief Various constants.
 *
 * This file defines constants that are not expected to change.
 */

#ifndef BOOTLOADER_FIRMWARE_SRC_CONSTANTS_H_
#define BOOTLOADER_FIRMWARE_SRC_CONSTANTS_H_

// *****************************************************************************
// USB specific constants
// *****************************************************************************

/**
 * @brief The USB Vendor ID.
 *
 * See http://pid.codes/1209/ACEE/
 */
enum { USB_DEVICE_VENDOR_ID = 0x1209 };

/**
 * @brief The Ja Rule Bootloaded USB Product ID.
 *
 * See http://pid.codes/1209/ACEE/
 */
enum { USB_DEVICE_PRODUCT_ID = 0xacee };

/**
 * @brief The DFU interface index.
 *
 * 64 bytes is the highest value a full speed, bulk endpoint can use.
 */
enum { USB_DFU_INTERFACE_INDEX = 0 };

/**
 * @brief By using alternate interfaces we can control which memory region we
 * write to.
 */
typedef enum {
  DFU_ALT_INTERFACE_FIRMWARE = 0,  //!< The main firmware
  DFU_ALT_INTERFACE_UID = 1,  //!< The UID region
} AlternateInterfaces;

#endif  // BOOTLOADER_FIRMWARE_SRC_CONSTANTS_H_

/**
 * @}
 */
