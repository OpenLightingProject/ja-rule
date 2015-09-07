/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * BootloaderOptionsMock.cpp
 * A mock bootloader_options module.
 * Copyright (C) 2015 Simon Newton
 */

#include "BootloaderOptionsMock.h"

namespace {
MockBootloaderOptions *g_bootloader_options_mock = NULL;
}

void BootloaderOptions_SetMock(MockBootloaderOptions* mock) {
  g_bootloader_options_mock = mock;
}

BootOption BootloaderOptions_GetBootOption() {
  if (g_bootloader_options_mock) {
    g_bootloader_options_mock->GetBootOption();
  }
  return BOOT_PRIMARY_APPLICATION;
}

void BootloaderOptions_SetBootOption(BootOption option) {
  if (g_bootloader_options_mock) {
    g_bootloader_options_mock->SetBootOption(option);
  }
}
