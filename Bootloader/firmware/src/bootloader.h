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
 * bootloader.h
 * Copyright (C) 2015 Simon Newton
 */

#ifndef BOOTLOADER_FIRMWARE_SRC_BOOTLOADER_H_
#define BOOTLOADER_FIRMWARE_SRC_BOOTLOADER_H_

#include <stddef.h>
#include <stdbool.h>
#include "dfu_spec.h"
#include "system_config.h"
#include "peripheral/ports/plib_ports.h"

/**
 * @{
 * @file bootloader.h
 * @brief The DFU bootloader.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Specifies an LED to flash during bootloader mode.
 */
typedef struct {
  PORTS_CHANNEL port_channel;
  PORTS_BIT_POS port_bit;
} Bootloader_LED;

/**
 * @brief The array of LEDs to flash when we're in bootloader mode.
 *
 * This is populated in bootloader_settings.h.
 */
typedef struct {
  uint8_t count;
  Bootloader_LED leds[];
} Bootloader_LEDs;

/**
 * @brief Initialize the boot loader
 */
void Bootloader_Initialize();

/**
 * @brief The bootloader event loop.
 */
void Bootloader_Tasks();

/**
 * @brief Check if USB is active & configured.
 * @returns true if USB is active & configured.
 */
bool Bootloader_USBActive();

/**
 * @brief Fetch the DFU state of the bootloader.
 * @returns The DFU state.
 */
DFUState Bootloader_GetState();

/**
 * @brief Fetch the DFU status of the bootloader.
 * @returns The DFU Status.
 */
DFUStatus Bootloader_GetStatus();

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // BOOTLOADER_FIRMWARE_SRC_BOOTLOADER_H_
