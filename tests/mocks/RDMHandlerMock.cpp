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
 * RDMHandlerMock.cpp
 * A mock RDMHandler module.
 * Copyright (C) 2015 Simon Newton
 */

#include "RDMHandlerMock.h"

namespace {
static MockRDMHandler *g_rdmhandler_mock = NULL;
}

void RDMHandler_SetMock(MockRDMHandler* mock) {
  g_rdmhandler_mock = mock;
}

void RDMHandler_Initialize(RDMHandlerSettings *settings) {
  if (g_rdmhandler_mock) {
    return g_rdmhandler_mock->Initialize(settings);
  }
}

bool RDMHandler_AddModel(const ModelEntry *entry) {
  if (g_rdmhandler_mock) {
    return g_rdmhandler_mock->AddModel(entry);
  }
  return false;
}

bool RDMHandler_SetActiveModel(uint16_t model_id) {
  if (g_rdmhandler_mock) {
    return g_rdmhandler_mock->SetActiveModel(model_id);
  }
  return false;
}

void RDMHandler_HandleRequest(const RDMHeader *header,
                              const uint8_t *param_data) {
  if (g_rdmhandler_mock) {
    g_rdmhandler_mock->HandleRequest(header, param_data);
  }
}

void RDMHandler_Tasks() {
  if (g_rdmhandler_mock) {
    g_rdmhandler_mock->Tasks();
  }
}
