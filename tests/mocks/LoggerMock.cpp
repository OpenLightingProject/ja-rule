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
 * LoggerMock.cpp
 * Mock Logger.
 * Copyright (C) 2015 Simon Newton
 */

#include "LoggerMock.h"
#include "transport.h"
#include "logger.h"

LoggerData g_logger;

namespace {
MockLogger *g_logger_mock = NULL;
}

void Logger_SetMock(MockLogger* mock) {
  g_logger_mock = mock;
}

void Logger_Initialize(TransportTXFunction tx_cb, uint16_t max_payload_size) {
  if (g_logger_mock) {
    g_logger_mock->Initialize(tx_cb, max_payload_size);
  }
}

void Logger_SetState(bool enabled) {
  if (g_logger_mock) {
    g_logger_mock->SetState(enabled);
  }
}

void Logger_Log(const char* str) {
  if (g_logger_mock) {
    g_logger_mock->Log(str);
  }
}

void Logger_Write(const uint8_t* data, unsigned int length) {
  if (g_logger_mock) {
    g_logger_mock->Write(data, length);
  }
}

void Logger_SendResponse(uint8_t token) {
  if (g_logger_mock) {
    g_logger_mock->SendResponse(token);
  }
}

void Logger_SetDataPendingFlag(bool flag) {
  g_logger.read = flag ? 0 : - 1;
}

void Logger_SetOverflowFlag(bool flag) {
  g_logger.overflow = flag;
}
