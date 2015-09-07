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
 * launcher.h
 * Copyright (C) 2015 Simon Newton
 */

#ifndef BOOTLOADER_FIRMWARE_SRC_LAUNCHER_H_
#define BOOTLOADER_FIRMWARE_SRC_LAUNCHER_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @{
 * @file launcher.h
 * @brief Jump to the main application.
 */

/**
 * @brief Jump to the main application at the specified address.
 * @param address The address of the first instruction to execute.
 *
 * This function will never return.
 */
void Launcher_RunApp(uint32_t address);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // BOOTLOADER_FIRMWARE_SRC_FLASH_H_
