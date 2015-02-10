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
 * MessageHandlerMock.cpp
 * A mock message handler.
 * Copyright (C) 2015 Simon Newton
 */

#include "MessageHandlerMock.h"

#include <gmock/gmock.h>

namespace {
MockMessageHandler *g_message_handler_mock = NULL;
}

bool MessageMatcher::MatchAndExplain(
    const Message* message,
    testing::MatchResultListener* listener) const {
  if (message->command != m_command) {
    *listener << "the command is " << message->command;
    return false;
  }

  if (message->length != m_payload_size) {
    *listener << "the payload size is " << message->length;
    return false;
  }

  if (m_payload == nullptr && message->payload == nullptr) {
    return true;
  }

  if (m_payload == nullptr || message->payload == nullptr) {
    *listener << "the payload was NULL";
    return false;
  }

  bool matched = true;
  if (listener->IsInterested()) {
    std::ios::fmtflags ostream_flags(listener->stream()->flags());
    for (unsigned int i = 0; i < m_payload_size; i++) {
      uint8_t actual = message->payload[i];
      uint8_t expected = reinterpret_cast<const uint8_t*>(m_payload)[i];

      *listener
         << "\n" << std::dec << i << ": 0x" << std::hex
         << static_cast<int>(expected)
         << (expected == actual ? " == " : " != ")
         << "0x" << static_cast<int>(actual) << " ("
         << ((expected >= '!' && expected <= '~') ?
             static_cast<char>(expected) : ' ')
         << (expected == actual ? " == " : " != ")
         << (actual >= '!' && actual <= '~' ? static_cast<char>(actual) : ' ')
         << ")";

      matched &= expected == actual;
    }
    listener->stream()->flags(ostream_flags);
  }
  return matched;
}

void MessageHandler_SetMock(MockMessageHandler* mock) {
  g_message_handler_mock = mock;
}

void MessageHandler_HandleMessage(const Message* message) {
  if (g_message_handler_mock) {
    g_message_handler_mock->HandleMessage(message);
  }
}
