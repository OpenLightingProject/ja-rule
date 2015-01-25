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
 * Mock of the Logger.
 * Copyright (C) 2015 Simon Newton
 */

#include "CMockaWrapper.h"
#include "logger.h"

LoggerData g_logger;

void Logging_Initialize(TXFunction tx_cb, uint16_t max_payload_size) {
  check_expected(tx_cb);
  check_expected(max_payload_size);
}

void Logging_SetState(bool enabled) {
  check_expected(enabled);
}

void Logging_Log(const char* str) {
  check_expected(str);
}

void Logging_SendResponse() {}

void Logging_SetDataPendingFlag(bool flag) {
  g_logger.read = flag ? 0 : - 1;
}

void Logging_SetOverflowFlag(bool flag) {
  g_logger.overflow = flag;
}
