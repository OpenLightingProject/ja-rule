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
 * UtilsTest.cpp
 * Tests for the Utils code.
 * Copyright (C) 2015 Simon Newton
 */

#include <gtest/gtest.h>

#include "Array.h"
#include "Matchers.h"
#include "utils.h"

TEST(UtilsTest, testExtractUInt16) {
  uint8_t ptr[] = {0x12, 0x34};
  EXPECT_EQ(0x1234, ExtractUInt16(ptr));
}

TEST(UtilsTest, testExtractUInt32) {
  uint8_t ptr[] = {0x12, 0x34, 0x56, 0x78};
  EXPECT_EQ(0x12345678, ExtractUInt32(ptr));
}

TEST(UtilsTest, testPushUInt16) {
  uint8_t ptr[2] = {0, 0};
  uint8_t *result = PushUInt16(ptr, 0x1234);
  EXPECT_EQ(ptr + arraysize(ptr), result);

  const uint8_t expected[] = {0x12, 0x34};
  EXPECT_THAT(ArrayTuple(ptr, 2), DataIs(expected, arraysize(expected)));
}

TEST(UtilsTest, testPushUInt32) {
  uint8_t ptr[4] = {0, 0, 0, 0};
  uint8_t *result = PushUInt32(ptr, 0x12345678);
  EXPECT_EQ(ptr + arraysize(ptr), result);

  const uint8_t expected[] = {0x12, 0x34, 0x56, 0x78};
  EXPECT_THAT(ArrayTuple(ptr, 4), DataIs(expected, arraysize(expected)));
}
