/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * TestUtilsPrivate.h
 * Useful functions that improve upon CPPUNIT's test functions
 * Copyright (C) 2012 Simon Newton
 */

#ifndef TESTS_INCLUDE_TESTUTILSPRIVATE_H_
#define TESTS_INCLUDE_TESTUTILSPRIVATE_H_

#include <stdint.h>
#include <cppunit/Asserter.h>
#include <cppunit/SourceLine.h>
#include <cppunit/extensions/HelperMacros.h>
#include <set>
#include <sstream>
#include <vector>

namespace testing {

// Assert that two data blocks are the same.
// Private, use ASSERT_DATA_EQUALS below.
void _AssertDataEquals(
    const CPPUNIT_NS::SourceLine &source_line,
    const uint8_t *expected,
    unsigned int expected_length,
    const uint8_t *actual,
    unsigned int actual_length);

// Private, use ASSERT_VECTOR_EQ below
template <typename T>
void _AssertVectorEq(
    const CPPUNIT_NS::SourceLine &source_line,
                     const std::vector<T> &t1,
                     const std::vector<T> &t2) {
  CPPUNIT_NS::assertEquals(t1.size(), t2.size(), source_line,
                           "Vector sizes not equal");

  typename std::vector<T>::const_iterator iter1 = t1.begin();
  typename std::vector<T>::const_iterator iter2 = t2.begin();
  while (iter1 != t1.end()) {
    CPPUNIT_NS::assertEquals(*iter1++, *iter2++, source_line,
                             "Vector elements not equal");
  }
}

// Private, use ASSERT_SET_EQ below
template <typename T>
void _AssertSetEq(const CPPUNIT_NS::SourceLine &source_line,
                  const std::set<T> &t1,
                  const std::set<T> &t2) {
  CPPUNIT_NS::assertEquals(t1.size(), t2.size(), source_line,
                           "Set sizes not equal");

  typename std::set<T>::const_iterator iter1 = t1.begin();
  typename std::set<T>::const_iterator iter2 = t2.begin();
  while (iter1 != t1.end()) {
    CPPUNIT_NS::assertEquals(*iter1++, *iter2++, source_line,
                             "Set elements not equal");
  }
}
}  // namespace testing
#endif  // TESTS_INCLUDE_TESTUTILSPRIVATE_H_
