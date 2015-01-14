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
 * TestUtils.cpp
 * Functions used for unit testing.
 * Copyright (C) 2012 Simon Newton
 */

#include "TestUtils.h"

#include <cppunit/extensions/HelperMacros.h>
#include <string.h>
#include <iostream>

namespace testing {

void _AssertDataEquals(
    const CPPUNIT_NS::SourceLine &source_line,
    const uint8_t *expected,
    unsigned int expected_length,
    const uint8_t *actual,
    unsigned int actual_length) {
  CPPUNIT_NS::assertEquals(expected_length, actual_length, source_line,
                           "Array sizes not equal");

  bool data_matches = 0 == memcmp(expected, actual, expected_length);
  std::ostringstream str;
  if (!data_matches) {
    for (unsigned int i = 0; i < expected_length; ++i) {
      str.str("");
      str << std::dec << i << ": 0x" << std::hex
          << static_cast<int>(expected[i]);
      str << ((expected[i] == actual[i]) ? " == " : " != ");
      str << "0x" << static_cast<int>(actual[i]) << " (";
      str << ((expected[i] >= '!' && expected[i] <= '~') ?
              static_cast<char>(expected[i]) : ' ');
      str << ((expected[i] == actual[i]) ? " == " : " != ");
      str << ((actual[i] >= '!' && actual[i] <= '~') ?
              static_cast<char>(actual[i]) : ' ');
      str << ")";

      if (expected[i] != actual[i]) {
        str << "  ## MISMATCH";
      }
      std::cout << str.str() << std::endl;
    }
  }
  CPPUNIT_NS::Asserter::failIf( \
    !data_matches, \
    CPPUNIT_NS::Message("Data mismatcs"), \
    source_line);
}
}  // namespace testing
