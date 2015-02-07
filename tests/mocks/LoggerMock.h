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
 * LoggerMock.h
 * Mock Logger.
 * Copyright (C) 2015 Simon Newton
 */

#ifndef TESTS_MOCKS_LOGGERMOCK_H_
#define TESTS_MOCKS_LOGGERMOCK_H_

#include <gmock/gmock.h>

#include "transport.h"

class MockLogger {
 public:
  MOCK_METHOD2(Initialize,
               void(TransportTxFunction tx_cb, uint16_t max_payload_size));
  MOCK_METHOD1(SetState, void(bool enabled));
  MOCK_METHOD1(Log, void(const char* str));
  MOCK_METHOD0(SendResponse, void());
};

void Logger_SetMock(MockLogger* mock);

void Logger_SetDataPendingFlag(bool flag);

void Logger_SetOverflowFlag(bool flag);

#endif  // TESTS_MOCKS_LOGGERMOCK_H_
