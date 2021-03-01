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
 * usb_console.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @addtogroup logging
 * @{
 * @file usb_console.h
 * @brief A logging transport that uses a USB serial console.
 *
 * This module is an implementation of the logging transport, that uses a CDC
 * USB device (serial console). This has the advantage of allowing users to
 * monitor the device's logs without requiring custom software. This worked
 * with both minicom and Hyperterminal.
 *
 * The USB Console uses a statically allocated circular buffer for the logs. If
 * the buffer overflows, the most recent logs are discarded.
 */

#ifndef FIRMWARE_SRC_USB_CONSOLE_H_
#define FIRMWARE_SRC_USB_CONSOLE_H_


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the USB Console module.
 */
void USBConsole_Initialize();

/**
 * @brief Write a message to the console.
 * @param message a NULL terminated string.
 *
 * We may not buffer the entire message if the log buffer is full.
 * Since the messages are sent over a serial console, the NULL characters are
 * replaced with \\n characters.
 */
void USBConsole_Log(const char* message);

/**
 * @brief Perform the housekeeping tasks for the USB Console.
 */
void USBConsole_Tasks();

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_USB_CONSOLE_H_
