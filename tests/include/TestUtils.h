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
 * TestUtils.h
 * Assertion Macros.
 * Copyright (C) 2012 Simon Newton
 */

#ifndef TESTS_INCLUDE_TESTUTILS_H_
#define TESTS_INCLUDE_TESTUTILS_H_

#include <cppunit/Asserter.h>
#include <cppunit/SourceLine.h>
#include <cppunit/extensions/HelperMacros.h>

#include "TestUtilsPrivate.h"

// Useful macros. This allows us to switch between unit testing frameworks in
// the future.
#define ASSERT(condition)  \
  CPPUNIT_ASSERT(condition)

#define ASSERT_TRUE(condition)  \
  CPPUNIT_ASSERT(condition)

#define ASSERT_FALSE(condition)  \
  CPPUNIT_ASSERT(!(condition))

#define ASSERT_EQ(expected, output)  \
  CPPUNIT_ASSERT_EQUAL(expected, output)

#define ASSERT_NE(expected, output)  \
  CPPUNIT_ASSERT((expected) != (output))

#define ASSERT_LT(expected, output)  \
  CPPUNIT_ASSERT((expected) < (output))

#define ASSERT_LTE(expected, output)  \
  CPPUNIT_ASSERT((expected) <= (output))

#define ASSERT_GT(expected, output)  \
  CPPUNIT_ASSERT((expected) > (output))

#define ASSERT_GTE(expected, output)  \
  CPPUNIT_ASSERT((expected) >= (output))

#define ASSERT_DATA_EQUALS(expected, expected_length, actual, \
                               actual_length)  \
  ola::testing::_AssertDataEquals(\
      CPPUNIT_SOURCELINE(), (expected), (expected_length), \
      (actual), (actual_length))

#define ASSERT_VECTOR_EQ(expected, output)  \
  ola::testing::_AssertVectorEq(CPPUNIT_SOURCELINE(), (expected), (output))

#define ASSERT_SET_EQ(expected, output)  \
  ola::testing::_AssertSetEq(CPPUNIT_SOURCELINE(), (expected), (output))

#define ASSERT_NULL(value) \
  CPPUNIT_NS::Asserter::failIf( \
    NULL != value, \
    CPPUNIT_NS::Message("Expression: " #value " != NULL"), \
    CPPUNIT_SOURCELINE())

#define ASSERT_NOT_NULL(value) \
  CPPUNIT_NS::Asserter::failIf( \
    NULL == value, \
    CPPUNIT_NS::Message("Expression: " #value " == NULL"), \
    CPPUNIT_SOURCELINE())

#define ASSERT_EMPTY(container) \
  CPPUNIT_NS::Asserter::failIf( \
    !container.empty(), \
    CPPUNIT_NS::Message("Expression: " #container " is empty"), \
    CPPUNIT_SOURCELINE())

#define ASSERT_NOT_EMPTY(container) \
  CPPUNIT_NS::Asserter::failIf( \
    container.empty(), \
    CPPUNIT_NS::Message("Expression: " #container " is empty"), \
    CPPUNIT_SOURCELINE())

#define FAIL(reason)  CPPUNIT_FAIL(reason)
#endif  // TESTS_INCLUDE_TESTUTILS_H_
