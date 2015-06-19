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


void Transceiver_Initialize(const TransceiverHardwareSettings* settings,
                            TransceiverEventCallback tx_callback,
                            TransceiverEventCallback rx_callback) {
  if (g_transceiver_mock) {
    return g_transceiver_mock->Initialize(settings, tx_callback, rx_callback);
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

bool Transceiver_SetRDMBroadcastListen(uint16_t delay) {
  if (g_transceiver_mock) {
    return g_transceiver_mock->SetRDMBroadcastListen(delay);
  }
  return true;
}

uint16_t Transceiver_GetRDMBroadcastListen() {
  if (g_transceiver_mock) {
    return g_transceiver_mock->GetRDMBroadcastListen();
  }
  return 0;
}

bool Transceiver_SetRDMWaitTime(uint16_t wait_time) {
  if (g_transceiver_mock) {
    return g_transceiver_mock->SetRDMWaitTime(wait_time);
  }
  return true;
}

uint16_t Transceiver_GetRDMWaitTime() {
  if (g_transceiver_mock) {
    return g_transceiver_mock->GetRDMWaitTime();
  }
  return 28;
}
