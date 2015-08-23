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
 * bootloader_options.c
 * Copyright (C) 2015 Simon Newton
 */

#include "bootloader_options.h"

#include <stdbool.h>
#include <peripheral/reset/plib_reset.h>

/*
 * @brief The magic value that triggers the bootloader.
 */
enum { MAGIC_BOOTLOADER_TOKEN = 0xb11dfe53 };

/*
 * @brief The location of the bootloader token.
 */
extern uint32_t __attribute__((space(prog))) _boot_option;

BootOption BootloaderOptions_GetBootOption() {
  BootOption option = BOOT_PRIMARY_APPLICATION;
  if (PLIB_RESET_ReasonGet(RESET_ID_0) & RESET_REASON_SOFTWARE) {
    switch (_boot_option) {
      case MAGIC_BOOTLOADER_TOKEN:
        option = BOOT_BOOTLOADER;
        break;
      default:
        ;
    }
  }
  return option;
}

void BootloaderOptions_SetBootOption(BootOption option) {
  switch (option) {
    case BOOT_BOOTLOADER:
      _boot_option = MAGIC_BOOTLOADER_TOKEN;
      break;
    default:
      _boot_option = 0u;
  }
}
