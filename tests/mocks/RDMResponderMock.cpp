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
 * RDMResponderMock.cpp
 * A mock RDMResponder module.
 * Copyright (C) 2015 Simon Newton
 */

#include "RDMResponderMock.h"

namespace {
static MockRDMResponder *g_rdmresponder_mock = NULL;
}

void RDMResponder_SetMock(MockRDMResponder* mock) {
  g_rdmresponder_mock = mock;
}

void RDMResponder_Initialize(const uint8_t uid[UID_LENGTH],
                             RDMResponderSendCallback send_callback) {
  if (g_rdmresponder_mock) {
    return g_rdmresponder_mock->Initialize(uid, send_callback);
  }
}

bool RDMResponder_UIDRequiresAction(const uint8_t uid[UID_LENGTH]) {
  if (g_rdmresponder_mock) {
    return g_rdmresponder_mock->UIDRequiresAction(uid);
  }
  return false;
}

bool RDMResponder_VerifyChecksum(const uint8_t *frame, unsigned int size) {
  if (g_rdmresponder_mock) {
    return g_rdmresponder_mock->VerifyChecksum(frame, size);
  }
  return false;
}

void RDMResponder_HandleRequest(const RDMHeader *header,
                                const uint8_t *param_data,
                                unsigned int length) {
  if (g_rdmresponder_mock) {
    g_rdmresponder_mock->HandleRequest(header, param_data, length);
  }
}

bool RDMResponder_IsMuted() {
  if (g_rdmresponder_mock) {
    return g_rdmresponder_mock->IsMuted();
  }
  return false;
}
