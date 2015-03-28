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
 * Matchers.h
 * Useful matchers.
 * Copyright (C) 2015 Simon Newton
 */

#ifndef TESTS_MOCKS_MATCHERS_H_
#define TESTS_MOCKS_MATCHERS_H_

#include <gmock/gmock.h>
#include <stdint.h>
#include "iovec.h"

typedef ::testing::tuple<const IOVec*, unsigned int> IOVecTuple;

// A tuple matcher that checks a {pointer, length} tuple matches the expected
// data.
class DataMatcher :
    public testing::MatcherInterface< ::testing::tuple<const void*,
                                                       unsigned int> > {
 public:
  DataMatcher(const uint8_t* expected_data, unsigned int expected_size)
      : m_expected_data(expected_data),
        m_expected_size(expected_size) {
  }

  bool MatchAndExplain(
      ::testing::tuple<const void*, unsigned int> args,
      testing::MatchResultListener* listener) const;

  void DescribeTo(::std::ostream* os) const {
    *os << "matches the data of size " << m_expected_size;
  }

  void DescribeNegationTo(::std::ostream* os) const {
    *os << "does not match the data of size " << m_expected_size;
  }

 private:
  const uint8_t* m_expected_data;
  unsigned int m_expected_size;

  bool InternalMatchAndExplain(
      const ::testing::tuple<const uint8_t*, unsigned int>& args,
      testing::MatchResultListener* listener) const;
};

/*
 * Example:
 *   EXPECT_CALL(mock, Send(...))
 *       .With(Args<1, 2>(DataIs(ptr, length));
 */
inline testing::Matcher< ::testing::tuple<const void*, unsigned int> > DataIs(
    const uint8_t* expected_data,
    unsigned int expected_size) {
  return testing::MakeMatcher(new DataMatcher(expected_data, expected_size));
}

// A tuple matcher that checks a {iovec pointer, iovec size} tuple matches the
// expected data.
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

/*
 * Example:
 *   EXPECT_CALL(mock, Send(...))
 *       .With(Args<1, 2>(PayloadIs(&iovec, 1));
 */
inline testing::Matcher<IOVecTuple> PayloadIs(const uint8_t* expected_data,
                                              unsigned int expected_size) {
  return testing::MakeMatcher(new PayloadMatcher(expected_data, expected_size));
}

/*
 * A shortcut that expects an empty payload.
 */
inline testing::Matcher<IOVecTuple> EmptyPayload() {
  return testing::MakeMatcher(new PayloadMatcher(nullptr, 0));
}



#endif  // TESTS_MOCKS_MATCHERS_H_
