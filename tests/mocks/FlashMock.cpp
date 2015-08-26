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
 * FlashMock.cpp
 * A mock flash module.
 * Copyright (C) 2015 Simon Newton
 */

#include "FlashMock.h"

namespace {
FlashInterface *g_flash_mock = NULL;
}

void Flash_SetMock(FlashInterface* mock) {
  g_flash_mock = mock;
}

bool Flash_ErasePage(uint32_t address) {
  if (g_flash_mock) {
    return g_flash_mock->ErasePage(address);
  }
  return true;
}

bool Flash_WriteWord(uint32_t address, uint32_t data) {
  if (g_flash_mock) {
    return g_flash_mock->WriteWord(address, data);
  }
  return true;
}

uint32_t Flash_ReadWord(uint32_t address) {
  if (g_flash_mock) {
    return g_flash_mock->ReadWord(address);
  }
  return 0;
}
