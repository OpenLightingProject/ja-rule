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
 * Matchers.cpp
 * Useful matchers.
 * Copyright (C) 2015 Simon Newton
 */

#include "Matchers.h"

#include <gmock/gmock.h>

// DataMatcher
// ----------------------------------------------------------------------------

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

bool DataMatcher::MatchAndExplain(
    ::testing::tuple<const void*, unsigned int> args,
    testing::MatchResultListener* listener) const {
  ::testing::tuple<const uint8_t*, unsigned int> new_args(
      reinterpret_cast<const uint8_t*>(::testing::get<0>(args)),
      ::testing::get<1>(args));
  return InternalMatchAndExplain(new_args, listener);
}

bool DataMatcher::InternalMatchAndExplain(
    const ::testing::tuple<const uint8_t*, unsigned int>& args,
    testing::MatchResultListener* listener) const {
  const uint8_t* data = ::testing::get<0>(args);
  unsigned int size = ::testing::get<1>(args);

  if (size != m_expected_size) {
    *listener << "data size was " << size;
    return false;
  }

  if (data == nullptr && m_expected_data == nullptr) {
    return true;
  }

  if (data == nullptr || m_expected_data == nullptr) {
    *listener << "the data was NULL";
    return false;
  }

  bool matched = true;
  if (listener->IsInterested()) {
    std::ios::fmtflags ostream_flags(listener->stream()->flags());
    for (unsigned int i = 0; i < m_expected_size; i++) {
      uint8_t actual = data[i];
      uint8_t expected = reinterpret_cast<const uint8_t*>(m_expected_data)[i];

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

      matched &= (expected == actual);
    }
    listener->stream()->flags(ostream_flags);
  }
  return matched;
}

testing::Matcher< ::testing::tuple<const void*, unsigned int> > DataIs(
    const uint8_t* expected_data,
    unsigned int expected_size) {
  return testing::MakeMatcher(new DataMatcher(expected_data, expected_size));
}


// PayloadMatcher
// ----------------------------------------------------------------------------

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


bool PayloadMatcher::MatchAndExplain(
    IOVecTuple args,
    testing::MatchResultListener* listener) const {
  const IOVec* iovec = ::testing::get<0>(args);
  unsigned int iov_count = ::testing::get<1>(args);

  unsigned int data_size = 0;
  for (unsigned int i = 0; i < iov_count; i++) {
    data_size += iovec[i].length;
  }
  if (data_size != m_expected_size) {
    *listener << "payload size was " << data_size;
    return false;
  }

  if (m_expected_data == NULL) {
    return true;
  }

  bool matched = true;
  unsigned int block_offset = 0;
  std::ios::fmtflags ostream_flags;
  if (listener->IsInterested()) {
    ostream_flags = listener->stream()->flags();
  }

  for (unsigned int i = 0; i < m_expected_size; i++) {
    uint8_t actual = reinterpret_cast<const uint8_t*>(
        iovec->base)[block_offset];
    uint8_t expected = m_expected_data[i];

    if (listener->IsInterested()) {
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
    }

    matched &= (expected == actual);
    block_offset++;
    if (block_offset >= iovec->length) {
      block_offset = 0;
      iovec++;
    }
  }
  if (listener->IsInterested()) {
    listener->stream()->flags(ostream_flags);
  }
  return matched;
}

testing::Matcher<IOVecTuple> PayloadIs(const uint8_t* expected_data,
                                       unsigned int expected_size) {
  return testing::MakeMatcher(new PayloadMatcher(expected_data, expected_size));
}

testing::Matcher<IOVecTuple> EmptyPayload() {
  return testing::MakeMatcher(new PayloadMatcher(nullptr, 0));
}
