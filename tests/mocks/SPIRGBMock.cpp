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
 * SPIRGBMock.cpp
 * A mock SPI RGB module.
 * Copyright (C) 2015 Simon Newton
 */

#include "SPIRGBMock.h"

namespace {
MockSPIRGB *g_spirgb_mock = NULL;
}

void SPIRGB_SetMock(MockSPIRGB* mock) {
  g_spirgb_mock = mock;
}

void SPIRGB_Init(const SPIRGBConfiguration *config) {
  if (g_spirgb_mock) {
    g_spirgb_mock->Init(config);
  }
}

void SPIRGB_BeginUpdate() {
  if (g_spirgb_mock) {
    g_spirgb_mock->BeginUpdate();
  }
}

void SPIRGB_SetPixel(uint16_t index, RGB_Color color, uint8_t value) {
  if (g_spirgb_mock) {
    g_spirgb_mock->SetPixel(index, color, value);
  }
}

void SPIRGB_CompleteUpdate() {
  if (g_spirgb_mock) {
    g_spirgb_mock->CompleteUpdate();
  }
}

void SPIRGB_Tasks() {
  if (g_spirgb_mock) {
    g_spirgb_mock->Tasks();
  }
}
