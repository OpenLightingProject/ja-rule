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
 * RDMUtilTest.cpp
 * Tests for the RDMUtil code.
 * Copyright (C) 2015 Simon Newton
 */

#include <gtest/gtest.h>

#include "rdm_util.h"
#include "rdm_responder.h"
#include "Array.h"
#include "Matchers.h"

const uint8_t SAMPLE_MESSAGE[] = {
  0xcc, 0x01, 0x18, 0x7a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x70, 0x12, 0x34,
  0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x02, 0x00, 0x03, 0xdf
};

class RDMUtilTest : public testing::Test {
 protected:
  static const uint8_t OUR_UID[];
  static const uint8_t VENDORCAST_UID[];
  static const uint8_t BROADCAST_UID[];
  static const uint8_t OTHER_VENDORCAST_UID[];
  static const uint8_t OTHER_UID[];
};

const uint8_t RDMUtilTest::OUR_UID[] = {0x7a, 0x70, 1, 2, 3, 4};

const uint8_t RDMUtilTest::VENDORCAST_UID[] = {
  0x7a, 0x70, 0xff, 0xff, 0xff, 0xff
};
const uint8_t RDMUtilTest::BROADCAST_UID[] = {
  0x7a, 0x70, 0xff, 0xff, 0xff, 0xff
};
const uint8_t RDMUtilTest::OTHER_VENDORCAST_UID[] = {
  0x4a, 0x80, 0xff, 0xff, 0xff, 0xff
};
const uint8_t RDMUtilTest::OTHER_UID[] = {0x7a, 0x70, 1, 2, 3, 99};

TEST_F(RDMUtilTest, testUIDCompare) {
  EXPECT_EQ(0, RDMUtil_UIDCompare(OUR_UID, OUR_UID));
  EXPECT_LT(0, RDMUtil_UIDCompare(OTHER_UID, OUR_UID));
  EXPECT_GT(0, RDMUtil_UIDCompare(OUR_UID, OTHER_UID));
}

TEST_F(RDMUtilTest, testRequiresAction) {
  EXPECT_TRUE(RDMUtil_RequiresAction(OUR_UID, OUR_UID));
  EXPECT_TRUE(RDMUtil_RequiresAction(OUR_UID, VENDORCAST_UID));
  EXPECT_TRUE(RDMUtil_RequiresAction(OUR_UID, BROADCAST_UID));
  EXPECT_FALSE(RDMUtil_RequiresAction(OUR_UID, OTHER_VENDORCAST_UID));
  EXPECT_FALSE(RDMUtil_RequiresAction(OUR_UID, OTHER_UID));
}

TEST_F(RDMUtilTest, testRequiresResponse) {
  RDMResponder responder;
  memcpy(responder.uid, OUR_UID, UID_LENGTH);

  EXPECT_TRUE(RDMUtil_RequiresResponse(&responder, OUR_UID));
  EXPECT_FALSE(RDMUtil_RequiresResponse(&responder, VENDORCAST_UID));
  EXPECT_FALSE(RDMUtil_RequiresResponse(&responder, BROADCAST_UID));
  EXPECT_FALSE(RDMUtil_RequiresResponse(&responder, OTHER_VENDORCAST_UID));
  EXPECT_FALSE(RDMUtil_RequiresResponse(&responder, OTHER_UID));
}

TEST_F(RDMUtilTest, testAppendChecksum) {
  uint8_t bad_packet[arraysize(SAMPLE_MESSAGE)];
  memcpy(bad_packet, SAMPLE_MESSAGE, arraysize(SAMPLE_MESSAGE));
  // zero checksum
  bad_packet[24] = 0;
  bad_packet[25] = 0;

  EXPECT_EQ(26, RDMUtil_AppendChecksum(bad_packet));
  EXPECT_EQ(0x03, bad_packet[24]);
  EXPECT_EQ(0xdf, bad_packet[25]);
}

TEST_F(RDMUtilTest, StringCopy) {
  const unsigned int DEST_SIZE = 10;
  char dest[DEST_SIZE];

  const char short_string[] = "foobar";

  // A non NULL terminated string, shorter than the dest.
  memset(dest, 0xff, DEST_SIZE);
  const char expected1[] = "foo\0\xff\xff\xff\xff\xff\xff";
  EXPECT_EQ(3, RDMUtil_StringCopy(dest, DEST_SIZE, short_string, 3));
  EXPECT_THAT(ArrayTuple(dest, DEST_SIZE), StringIs(expected1, DEST_SIZE));

  // A NULL terminated string, shorter than the dest.
  memset(dest, 0xff, DEST_SIZE);
  const char expected2[] = "foobar\0\xff\xff\xff";
  EXPECT_EQ(6, RDMUtil_StringCopy(dest, DEST_SIZE, short_string, 6));
  EXPECT_THAT(ArrayTuple(dest, DEST_SIZE), StringIs(expected2, DEST_SIZE));

  // A non-null terminated string, equal in size to DEST
  memset(dest, 0xff, DEST_SIZE);
  const char equal_string[] = "0123456789";
  const char expected3[] = "0123456789";
  EXPECT_EQ(DEST_SIZE, RDMUtil_StringCopy(dest, DEST_SIZE, equal_string,
                                          strlen(equal_string)));
  EXPECT_THAT(ArrayTuple(dest, DEST_SIZE), StringIs(expected3, DEST_SIZE));

  // A null terminated string, equal in size to DEST
  const char equal_with_null[] = "012345678";
  memset(dest, 0xff, DEST_SIZE);
  const char expected4[] = "012345678";
  EXPECT_EQ(9, RDMUtil_StringCopy(dest, DEST_SIZE, equal_with_null,
                                  arraysize(equal_with_null)));
  EXPECT_THAT(ArrayTuple(dest, DEST_SIZE), StringIs(expected4, DEST_SIZE));

  // A non-NULL terminated string, longer than dest
  const char long_string[] = "this is a test";
  memset(dest, 0xff, DEST_SIZE);
  const char expected5[] = "this is a ";
  EXPECT_EQ(DEST_SIZE,
            RDMUtil_StringCopy(dest, DEST_SIZE, long_string,
                               strlen(long_string)));
  EXPECT_THAT(ArrayTuple(dest, DEST_SIZE), StringIs(expected5, DEST_SIZE));

  // A non-NULL terminated string, longer than dest
  memset(dest, 0xff, DEST_SIZE);
  EXPECT_EQ(DEST_SIZE,
            RDMUtil_StringCopy(dest, DEST_SIZE, long_string,
                               arraysize(long_string)));
  EXPECT_THAT(ArrayTuple(dest, DEST_SIZE), StringIs(expected5, DEST_SIZE));
}

TEST_F(RDMUtilTest, testSafeStringLength) {
  const char test_string[] = "this is a test";
  EXPECT_EQ(4u, RDMUtil_SafeStringLength(test_string, 4));
  EXPECT_EQ(14u, RDMUtil_SafeStringLength(test_string, arraysize(test_string)));
}

TEST_F(RDMUtilTest, testUpdateSensor) {
  SensorData sensor = {
    .present_value = 14,
    .lowest_value = SENSOR_VALUE_UNSUPPORTED,
    .highest_value = SENSOR_VALUE_UNSUPPORTED
  };

  int16_t new_value = 99;
  // A sensor that doesn't support recording
  RDMUtil_UpdateSensor(&sensor, 0, new_value);
  EXPECT_EQ(new_value, sensor.present_value);
  EXPECT_EQ(0, sensor.lowest_value);
  EXPECT_EQ(0, sensor.highest_value);

  // A sensor that supports recording
  new_value = 52;
  sensor.lowest_value = sensor.present_value;
  sensor.highest_value = sensor.present_value;
  RDMUtil_UpdateSensor(&sensor, SENSOR_SUPPORTS_LOWEST_HIGHEST_MASK, new_value);
  EXPECT_EQ(new_value, sensor.present_value);
  EXPECT_EQ(52, sensor.lowest_value);
  EXPECT_EQ(99, sensor.highest_value);

  new_value = 434;
  RDMUtil_UpdateSensor(&sensor, SENSOR_SUPPORTS_LOWEST_HIGHEST_MASK, new_value);
  EXPECT_EQ(new_value, sensor.present_value);
  EXPECT_EQ(52, sensor.lowest_value);
  EXPECT_EQ(434, sensor.highest_value);

  new_value = 7;
  RDMUtil_UpdateSensor(&sensor, SENSOR_SUPPORTS_LOWEST_HIGHEST_MASK, new_value);
  EXPECT_EQ(new_value, sensor.present_value);
  EXPECT_EQ(7, sensor.lowest_value);
  EXPECT_EQ(434, sensor.highest_value);
}

// Tests for RDMUtil_VerifyChecksum
//-----------------------------------------------------------------------------
class ChecksumTest : public ::testing::TestWithParam<uint32_t> {};


TEST_P(ChecksumTest, checksumFails) {
  EXPECT_FALSE(RDMUtil_VerifyChecksum(SAMPLE_MESSAGE, GetParam()));
}

INSTANTIATE_TEST_CASE_P(
    sizeTooSmall,
    ChecksumTest,
    ::testing::Range(
        0u, static_cast<unsigned int>(arraysize(SAMPLE_MESSAGE) - 1))
);

TEST_F(ChecksumTest, checksumPasses) {
  EXPECT_TRUE(RDMUtil_VerifyChecksum(SAMPLE_MESSAGE,
                                     arraysize(SAMPLE_MESSAGE)));
}

TEST_F(ChecksumTest, checksumMismatch) {
  uint8_t bad_packet[arraysize(SAMPLE_MESSAGE)];
  memcpy(bad_packet, SAMPLE_MESSAGE, arraysize(SAMPLE_MESSAGE));
  bad_packet[arraysize(SAMPLE_MESSAGE) - 1]++;

  EXPECT_FALSE(RDMUtil_VerifyChecksum(bad_packet, arraysize(bad_packet)));
}
