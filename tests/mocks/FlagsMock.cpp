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
 * FlagsMock.cpp
 * A mock flags module.
 * Copyright (C) 2015 Simon Newton
 */

#include "FlagsMock.h"

namespace {
MockFlags *g_flags_mock = NULL;
}

FlagsData g_flags;

void Flags_SetMock(MockFlags* mock) {
  g_flags_mock = mock;
}

void Flags_Initialize(TransportTXFunction tx_cb) {
  if (g_flags_mock) {
    g_flags_mock->Initialize(tx_cb);
  }
}

void Flags_SendResponse() {
  if (g_flags_mock) {
    g_flags_mock->SendResponse();
  }
}

