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
 * usb_descriptors.h
 * Copyright (C) 2015 Simon Newton
 */

#ifndef FIRMWARE_SRC_USB_DESCRIPTORS_H_
#define FIRMWARE_SRC_USB_DESCRIPTORS_H_

#include "usb/usb_device.h"

/**
 * @file usb_descriptors.h
 * @brief USB Descriptors
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Fetch a pointer to the USB serial number.
 * @returns a pointer to the unicode string serial number.
 *
 * The length of the buffer will be at least UID_LENGTH * 2 + 1, enough to hold
 * a string representation of a UID.
 */
uint16_t* USBDescriptor_UnicodeUID();

/**
 * @brief Fetch a pointer to the USB device initialization structure.
 * @returns a Pointer to a USB_DEVICE_INIT which contains all the USB
 * descriptors.
 */
const USB_DEVICE_INIT* USBDescriptor_GetDeviceConfig();

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
}
#endif

#endif  // FIRMWARE_SRC_USB_DESCRIPTORS_H_
