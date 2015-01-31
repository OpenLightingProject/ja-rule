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
 * MessageHandlerMock.h
 * A mock message handler.
 * Copyright (C) 2015 Simon Newton
 */

#ifndef TESTS_MOCKS_MESSAGEHANDLERMOCK_H_
#define TESTS_MOCKS_MESSAGEHANDLERMOCK_H_

#include <gmock/gmock.h>
#include <stdint.h>

#include "message_handler.h"

class MessageMatcher : public testing::MatcherInterface<const Message*> {
 public:
  MessageMatcher(uint16_t command, const uint8_t* payload,
                 unsigned int payload_size)
      : m_command(command),
        m_payload(payload),
        m_payload_size(payload_size) {
  }

  virtual bool MatchAndExplain(const Message* message,
                               testing::MatchResultListener* listener) const;

  virtual void DescribeTo(::std::ostream* os) const {
    *os << "Message is command " << static_cast<int>(m_command) << " with size "
        << m_payload_size;
  }

  virtual void DescribeNegationTo(::std::ostream* os) const {
    *os << "Message is not command " << static_cast<int>(m_command)
        << " with size " << m_payload_size;
  }

 private:
  uint16_t m_command;
  const uint8_t* m_payload;
  unsigned int m_payload_size;
};

/*
 * Check that a message has the given command, and that the payload matches.
 */
inline testing::Matcher<const Message*> MessageIs(uint16_t command,
                                                  const uint8_t* payload,
                                                  unsigned int payload_size) {
  return testing::MakeMatcher(
      new MessageMatcher(command, payload, payload_size));
}

class MockMessageHandler {
 public:
  MOCK_METHOD1(Initialize, void(TXFunction tx_cb));
  MOCK_METHOD1(HandleMessage, void(const Message* message));
};

void MessageHandler_SetMock(MockMessageHandler* mock);

#endif  // TESTS_MOCKS_MESSAGEHANDLERMOCK_H_
