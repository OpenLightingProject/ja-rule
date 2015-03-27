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
 * SysLogMock.cpp
 * A mock SysLog module.
 * Copyright (C) 2015 Simon Newton
 */

#include "SysLogMock.h"

namespace {
MockSysLog *g_syslog_mock = NULL;
static const char DUMMY_LEVEL[] = "";
}

void SysLog_SetMock(MockSysLog* mock) {
  g_syslog_mock = mock;
}

void SysLog_Initialize(SysLogWriteFn write_fn) {
  if (g_syslog_mock) {
    g_syslog_mock->Initialize(write_fn);
  }
}

void SysLog_Message(SysLogLevel level, const char* msg) {
  if (g_syslog_mock) {
    g_syslog_mock->Message(level, msg);
  }
}

void SysLog_Print(SysLogLevel level, const char* format, ...) {
  // Noop
  (void) level;
  (void) format;
}

SysLogLevel SysLog_GetLevel() {
  if (g_syslog_mock) {
    return g_syslog_mock->GetLevel();
  }
  return SYSLOG_WARN;
}

void SysLog_SetLevel(SysLogLevel level) {
  if (g_syslog_mock) {
    return g_syslog_mock->SetLevel(level);
  }
}

void SysLog_Increment() {
  if (g_syslog_mock) {
    g_syslog_mock->Increment();
  }
}

void SysLog_Decrement() {
  if (g_syslog_mock) {
    g_syslog_mock->Decrement();
  }
}

const char* SysLog_LevelToString(SysLogLevel level) {
  if (g_syslog_mock) {
    return g_syslog_mock->LevelToString(level);
  }
  return DUMMY_LEVEL;
}
