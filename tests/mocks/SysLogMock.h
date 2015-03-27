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
 * SysLogMock.h
 * A mock SysLog module.
 * Copyright (C) 2015 Simon Newton
 */

#ifndef TESTS_MOCKS_SYSLOGMOCK_H_
#define TESTS_MOCKS_SYSLOGMOCK_H_

#include <gmock/gmock.h>

#include "syslog.h"

class MockSysLog {
 public:
  MOCK_METHOD1(Initialize, void(SysLogWriteFn write_fn));
  MOCK_METHOD2(Message, void(SysLogLevel level, const char* msg));
  MOCK_METHOD0(GetLevel, SysLogLevel());
  MOCK_METHOD1(SetLevel, void(SysLogLevel level));
  MOCK_METHOD0(Increment, void());
  MOCK_METHOD0(Decrement, void());
  MOCK_METHOD1(LevelToString, const char*(SysLogLevel level));
};

void SysLog_SetMock(MockSysLog* mock);

#endif  // TESTS_MOCKS_SYSLOGMOCK_H_
