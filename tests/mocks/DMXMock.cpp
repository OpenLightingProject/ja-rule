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
 * DMXMock.cpp
 * Mock DMX Engine.
 * Copyright (C) 2015 Simon Newton
 */

#include "DMXMock.h"

namespace {
MockDMX *g_dmx_mock = NULL;
}

void DMX_SetMock(MockDMX* mock) {
  g_dmx_mock = mock;
}

void DMX_Initialize() {
  if (g_dmx_mock) {
    g_dmx_mock->Initialize();
  }
}

void DMX_Tasks() {
  if (g_dmx_mock) {
    g_dmx_mock->Tasks();
  }
}

void DMX_BeginFrame(uint8_t start_code, const uint8_t* data,
                    unsigned int size) {
  if (g_dmx_mock) {
    g_dmx_mock->BeginFrame(start_code, data, size);
  }
}

void DMX_FinalizeFrame() {
  if (g_dmx_mock) {
    g_dmx_mock->FinalizeFrame();
  }
}
