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
 * bootloader_options.h
 * Copyright (C) 2015 Simon Newton
 */

#ifndef COMMON_BOOTLOADER_OPTIONS_H_
#define COMMON_BOOTLOADER_OPTIONS_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @{
 * @file bootloader_options.h
 * @brief The API to share information between the bootloader and the main
 * application.
 *
 * We set aside a small region of RAM at the end of the address space and use
 * this to pass information between the application and the bootloader.
 *
 * During the initial boot process, if the cause of the reset was a software
 * reset, we check the boot token to see if the application wants us to enter
 * bootloader mode.
 *
 * The application can enter the bootloader mode by calling
 * BootloaderOptions_SetNextBoot(BOOT_BOOTLOADER) and then forcing a s/w reset.
 */

/**
 * @brief The different boot options.
 */
typedef enum {
  BOOT_BOOTLOADER,  //!< Start the bootloader
  BOOT_PRIMARY_APPLICATION  //!< Start the primary application
} BootOption;

/**
 * @brief Check what code we should run.
 * @returns The BootOption of the code to run.
 *
 * This will only return BOOT_BOOTLOADER after a s/w reset.
 */
BootOption BootloaderOptions_GetBootOption();

/**
 * @brief Control what we boot after the next s/w reset.
 * @param option The code to boot.
 */
void BootloaderOptions_SetBootOption(BootOption option);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // COMMON_BOOTLOADER_OPTIONS_H_

