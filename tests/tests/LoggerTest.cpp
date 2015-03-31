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
 * LoggerTest.cpp
 * Tests for the Logger code.
 * Copyright (C) 2015 Simon Newton
 */

#include <gtest/gtest.h>
#include <string>

#include "logger.h"
#include "Array.h"
#include "Matchers.h"
#include "TransportMock.h"

using std::string;
using ::testing::Args;
using ::testing::StrictMock;
using ::testing::Return;
using ::testing::_;

class LoggerTest : public testing::Test {
 public:
  void SetUp() {
    Transport_SetMock(&transport_mock);
    Logger_Initialize(Transport_Send, PAYLOAD_SIZE);
  }

  void TearDown() {
    Transport_SetMock(nullptr);
  }

  StrictMock<MockTransport> transport_mock;
};

/*
 * Confirm when the logger is disabled, no writes occur.
 */
TEST_F(LoggerTest, disabled) {
  EXPECT_FALSE(Logger_IsEnabled());

  string test("This is a test");
  Logger_Log(test.c_str());

  EXPECT_FALSE(Logger_DataPending());
  EXPECT_FALSE(Logger_HasOverflowed());

  // Even when the logger is disabled, Logger_SendResponse() should still
  // transmit an (empty) message.
  const uint8_t payload[] = {0};
  EXPECT_CALL(transport_mock, Send(GET_LOG, RC_OK, _, 1))
      .With(Args<2, 3>(PayloadIs(payload, arraysize(payload))))
      .WillOnce(Return(true));

  Logger_SendResponse();
}

/*
 * Confirm passing a nullptr callback doesn't crash.
 */
TEST_F(LoggerTest, nullCallback) {
  Logger_Initialize(nullptr, PAYLOAD_SIZE);
  Logger_SetState(true);

  EXPECT_TRUE(Logger_IsEnabled());

  string test("This is a test");
  Logger_Log(test.c_str());

  EXPECT_TRUE(Logger_DataPending());
  EXPECT_FALSE(Logger_HasOverflowed());

  // A nullptr callback means no messages.
  Logger_SendResponse();
}

/*
 * Confirm resetting the Logger causes the flags to be reset.
 */
TEST_F(LoggerTest, reset) {
  Logger_SetState(true);
  EXPECT_TRUE(Logger_IsEnabled());

  string test(1000, 'x');
  Logger_Log(test.c_str());

  EXPECT_TRUE(Logger_DataPending());
  EXPECT_TRUE(Logger_HasOverflowed());

  // Now reset
  Logger_SetState(false);
  EXPECT_FALSE(Logger_IsEnabled());
  EXPECT_FALSE(Logger_DataPending());
  EXPECT_FALSE(Logger_HasOverflowed());

  // Re-enable
  Logger_SetState(true);
  EXPECT_TRUE(Logger_IsEnabled());
  EXPECT_FALSE(Logger_DataPending());
  EXPECT_FALSE(Logger_HasOverflowed());

  string test2(10, 'x');
  Logger_Log(test2.c_str());

  EXPECT_TRUE(Logger_DataPending());

  uint8_t payload[] = {0, 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 'x', 0};
  EXPECT_CALL(transport_mock, Send(GET_LOG, RC_OK, _, _))
      .With(Args<2, 3>(PayloadIs(payload, arraysize(payload))))
      .WillOnce(Return(true));

  Logger_SendResponse();
}

/*
 * Check messages are correctly formed.
 */
TEST_F(LoggerTest, logAndFetch) {
  Logger_Initialize(Transport_Send, 100);
  Logger_SetState(true);
  Logger_Log(string(200, 'x').c_str());

  EXPECT_TRUE(Logger_DataPending());

  uint8_t payload1[100] = {0};
  memset(payload1 + 1, 'x', 99);

  uint8_t payload2[100] = {1};
  memset(payload2 + 1, 'x', 99);

  uint8_t payload3[100] = {0};
  memset(payload3 + 1, 'x', 2);
  payload3[3] = 0;
  memset(payload3 + 4, 'y', 96);

  uint8_t payload4[59] = {0};
  memset(payload4 + 1, 'y', 58);
  payload4[58] = 0;

  uint8_t payload5[] = {0};

  testing::InSequence seq;
  EXPECT_CALL(transport_mock, Send(GET_LOG, RC_OK, _, _))
      .With(Args<2, 3>(PayloadIs(payload1, arraysize(payload1))))
      .WillOnce(Return(true));
  EXPECT_CALL(transport_mock, Send(GET_LOG, RC_OK, _, _))
      .With(Args<2, 3>(PayloadIs(payload2, arraysize(payload2))))
      .WillOnce(Return(true));
  EXPECT_CALL(transport_mock, Send(GET_LOG, RC_OK, _, _))
      .With(Args<2, 3>(PayloadIs(payload3, arraysize(payload3))))
      .WillOnce(Return(true));
  EXPECT_CALL(transport_mock, Send(GET_LOG, RC_OK, _, _))
      .With(Args<2, 3>(PayloadIs(payload4, arraysize(payload4))))
      .WillOnce(Return(true));
  EXPECT_CALL(transport_mock, Send(GET_LOG, RC_OK, _, _))
      .With(Args<2, 3>(PayloadIs(payload5, arraysize(payload5))))
      .WillOnce(Return(true));

  Logger_SendResponse();

  // Now write some more data
  Logger_Log(string(200, 'y').c_str());

  EXPECT_TRUE(Logger_DataPending());
  EXPECT_TRUE(Logger_HasOverflowed());

  Logger_SendResponse();  // 99 'x'
  Logger_SendResponse();  // 2 'x', 97 'y'
  Logger_SendResponse();  // 58 'y'
  Logger_SendResponse();  // Empty
}

/*
 * Confirm the overflow flag is set correctly.
 */
TEST_F(LoggerTest, overflow) {
  Logger_SetState(true);
  Logger_Log(string(1000, 'x').c_str());

  EXPECT_TRUE(Logger_HasOverflowed());

  uint8_t payload1[257] = {1};
  memset(payload1 + 1, 'x', 255);
  payload1[256] = 0;

  uint8_t payload2[] = {0};

  testing::InSequence seq;
  EXPECT_CALL(transport_mock, Send(GET_LOG, RC_OK, _, _))
      .With(Args<2, 3>(PayloadIs(payload1, arraysize(payload1))))
      .WillOnce(Return(true));
  EXPECT_CALL(transport_mock, Send(GET_LOG, RC_OK, _, _))
      .With(Args<2, 3>(PayloadIs(payload2, arraysize(payload2))))
      .WillOnce(Return(true));

  Logger_SendResponse();
  EXPECT_FALSE(Logger_DataPending());

  // Now fetch the next message, the overflow flag must clear.
  Logger_SendResponse();
}

/*
 * Confirm writing raw data works.
 */
TEST_F(LoggerTest, write) {
  Logger_SetState(true);

  const string data1("test 1");
  const string data2("test 2");

  Logger_Write(reinterpret_cast<const uint8_t*>(data1.c_str()),
               data1.size() + 1);
  Logger_Write(reinterpret_cast<const uint8_t*>(data2.c_str()),
               data2.size() + 1);

  EXPECT_FALSE(Logger_HasOverflowed());

  uint8_t payload1[] = {
    0,
    't', 'e', 's', 't', ' ', '1', 0,
    't', 'e', 's', 't', ' ', '2', 0,
  };

  EXPECT_CALL(transport_mock, Send(GET_LOG, RC_OK, _, _))
      .With(Args<2, 3>(PayloadIs(payload1, arraysize(payload1))))
      .WillOnce(Return(true));

  Logger_SendResponse();
}
