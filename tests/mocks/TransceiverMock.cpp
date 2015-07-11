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
 * TransceiverMock.cpp
 * The Mock Transceiver module.
 * Copyright (C) 2015 Simon Newton
 */

#include "TransceiverMock.h"

namespace {
MockTransceiver *g_transceiver_mock = NULL;
}

void Transceiver_SetMock(MockTransceiver* mock) {
  g_transceiver_mock = mock;
}


void Transceiver_Initialize(const TransceiverHardwareSettings* settings) {
  if (g_transceiver_mock) {
    return g_transceiver_mock->Initialize(settings);
  }
}

void Transceiver_Tasks() {
  if (g_transceiver_mock) {
    return g_transceiver_mock->Tasks();
  }
}

bool Transceiver_QueueDMX(uint8_t token, const uint8_t* data,
                          unsigned int size) {
  if (g_transceiver_mock) {
    return g_transceiver_mock->QueueDMX(token, data, size);
  }
  return true;
}

bool Transceiver_QueueASC(uint8_t token, uint8_t start_code,
                          const uint8_t* data, unsigned int size) {
  if (g_transceiver_mock) {
    return g_transceiver_mock->QueueASC(token, start_code, data, size);
  }
  return true;
}

bool Transceiver_QueueRDMDUB(uint8_t token, const uint8_t* data,
                             unsigned int size) {
  if (g_transceiver_mock) {
    return g_transceiver_mock->QueueRDMDUB(token, data, size);
  }
  return true;
}

bool Transceiver_QueueRDMRequest(uint8_t token, const uint8_t* data,
                                 unsigned int size, bool is_broadcast) {
  if (g_transceiver_mock) {
    return g_transceiver_mock->QueueRDMRequest(token, data, size, is_broadcast);
  }
  return true;
}

bool Transceiver_SetBreakTime(uint16_t mark_time_us) {
  if (g_transceiver_mock) {
    return g_transceiver_mock->SetBreakTime(mark_time_us);
  }
  return true;
}

uint16_t Transceiver_GetBreakTime() {
  if (g_transceiver_mock) {
    return g_transceiver_mock->GetBreakTime();
  }
  return 176;
}

bool Transceiver_SetMarkTime(uint16_t mark_time_us) {
  if (g_transceiver_mock) {
    return g_transceiver_mock->SetMarkTime(mark_time_us);
  }
  return true;
}

uint16_t Transceiver_GetMarkTime() {
  if (g_transceiver_mock) {
    return g_transceiver_mock->GetMarkTime();
  }
  return 12;
}

bool Transceiver_SetRDMBroadcastTimeout(uint16_t delay) {
  if (g_transceiver_mock) {
    return g_transceiver_mock->SetRDMBroadcastTimeout(delay);
  }
  return true;
}

uint16_t Transceiver_GetRDMBroadcastTimeout() {
  if (g_transceiver_mock) {
    return g_transceiver_mock->GetRDMBroadcastTimeout();
  }
  return 0;
}

bool Transceiver_SetRDMResponseTimeout(uint16_t wait_time) {
  if (g_transceiver_mock) {
    return g_transceiver_mock->SetRDMResponseTimeout(wait_time);
  }
  return true;
}

uint16_t Transceiver_GetRDMResponseTimeout() {
  if (g_transceiver_mock) {
    return g_transceiver_mock->GetRDMResponseTimeout();
  }
  return 28;
}

bool Transceiver_SetRDMDUBResponseLimit(uint16_t limit) {
  if (g_transceiver_mock) {
    return g_transceiver_mock->SetRDMDUBResponseLimit(limit);
  }
  return true;
}

uint16_t Transceiver_GetRDMDUBResponseLimit() {
  if (g_transceiver_mock) {
    return g_transceiver_mock->GetRDMDUBResponseLimit();
  }
  return 28;
}

bool Transceiver_SetRDMResponderDelay(uint16_t delay) {
  if (g_transceiver_mock) {
    return g_transceiver_mock->SetRDMResponderDelay(delay);
  }
  return true;
}

uint16_t Transceiver_GetRDMResponderDelay() {
  if (g_transceiver_mock) {
    return g_transceiver_mock->GetRDMResponderDelay();
  }
  return 1760;
}
