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
 * TransportMock.h
 * A mock transport layer.
 * Copyright (C) 2015 Simon Newton
 */

#ifndef TESTS_MOCKS_TRANSPORTMOCK_H_
#define TESTS_MOCKS_TRANSPORTMOCK_H_

#include <gmock/gmock.h>
#include "transport.h"

typedef ::testing::tuple<const IOVec*, unsigned int> IOVecTuple;

// A tuple matcher that checks a message payload.
class PayloadMatcher : public testing::MatcherInterface<IOVecTuple> {
 public:
  PayloadMatcher(const uint8_t* expected_data, unsigned int expected_size)
      : m_expected_data(expected_data),
        m_expected_size(expected_size) {
  }

  virtual bool MatchAndExplain(IOVecTuple args,
                               testing::MatchResultListener* listener) const;

  virtual void DescribeTo(::std::ostream* os) const {
    *os << "matches the payload of size " << m_expected_size;
  }

  virtual void DescribeNegationTo(::std::ostream* os) const {
    *os << "does not match the payload of size " << m_expected_size;
  }

 private:
  const uint8_t* m_expected_data;
  unsigned int m_expected_size;
};

inline testing::Matcher<IOVecTuple> PayloadIs(const uint8_t* expected_data,
                                              unsigned int expected_size) {
  return testing::MakeMatcher(new PayloadMatcher(expected_data, expected_size));
}

inline testing::Matcher<IOVecTuple> EmptyPayload() {
  return testing::MakeMatcher(new PayloadMatcher(nullptr, 0));
}

class MockTransport {
 public:
  MOCK_METHOD4(Send, bool(Command command, uint8_t rc,
                          const IOVec* iovec, unsigned int iovec_count));
};

void Transport_SetMock(MockTransport* mock);

bool Transport_Send(Command command, uint8_t rc, const IOVec* iovec,
                    unsigned int iovec_count);

#endif  // TESTS_MOCKS_TRANSPORTMOCK_H_
