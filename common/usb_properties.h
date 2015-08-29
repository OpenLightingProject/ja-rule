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
 * usb_properties.h
 * Copyright (C) 2015 Simon Newton
 */

#ifndef COMMON_USB_PROPERTIES_H_
#define COMMON_USB_PROPERTIES_H_

/**
 * @file usb_properties.h
 * @brief USB Device specific constants.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The USB Vendor ID.
 *
 * See http://pid.codes/1209/ACEE/
 */
enum { USB_DEVICE_VENDOR_ID = 0x1209 };

/**
 * @brief The Ja Rule USB Product ID.
 *
 * See http://pid.codes/1209/ACED/
 */
enum { USB_DEVICE_MAIN_PRODUCT_ID = 0xaced };

/**
 * @brief The Ja Rule Bootloader USB Product ID.
 *
 * See http://pid.codes/1209/ACEE/
 */
enum { USB_DEVICE_BOOTLOADER_PRODUCT_ID = 0xacee };

#ifdef __cplusplus
}
#endif

#endif  // COMMON_USB_PROPERTIES_H_
