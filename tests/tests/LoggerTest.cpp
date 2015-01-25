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

#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <vector>

#include "logger.h"
#include "TestUtils.h"

using std::string;

class LoggerTest *logger_test = nullptr;

class LoggerTest: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(LoggerTest);
  CPPUNIT_TEST(testDisabled);
  CPPUNIT_TEST(testNullCallback);
  CPPUNIT_TEST(testReset);
  CPPUNIT_TEST(testLogAndFetch);
  CPPUNIT_TEST(testOverflow);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp() {
    logger_test = this;
  }

  void tearDown() {
    logger_test = nullptr;
  }

  void testDisabled();
  void testNullCallback();
  void testReset();
  void testLogAndFetch();
  void testOverflow();

  bool Tx(Command command, uint8_t return_code, const IOVec* iov,
          unsigned int iov_count) {
    string payload;
    for (unsigned int i = 0; i != iov_count; i++) {
      payload.append(reinterpret_cast<const char*>(iov[i].base), iov[i].length);
    }

    bool valid = !payload.empty();
    bool overflow = valid ? (payload[0] & 0x01) : false;
    string data = payload.size() > 1 ? payload.substr(1) : "";

    Message message = {
      .command = command,
      .return_code = return_code,
      .valid = valid,
      .overflow = overflow,
      .data = data
    };
    received_messages.push_back(message);
    return true;
  }

 private:
  struct Message {
    Command command;
    uint8_t return_code;
    bool valid;
    bool overflow;
    string data;
  };

  std::vector<Message> received_messages;
};

CPPUNIT_TEST_SUITE_REGISTRATION(LoggerTest);

/*
 * Called by the Logger code under test.
 */
bool TxFunction(Command command, uint8_t return_code, const IOVec* iov,
                unsigned int iov_count) {
  if (logger_test) {
    return logger_test->Tx(command, return_code, iov, iov_count);
  }
  return true;
}

/*
 * Confirm when the logger is disabled, no writes occur.
 */
void LoggerTest::testDisabled() {
  Logger_Initialize(TxFunction, PAYLOAD_SIZE);

  ASSERT_FALSE(Logger_IsEnabled());

  string test("This is a test");
  Logger_Log(test.c_str());

  ASSERT_FALSE(Logger_DataPending());
  ASSERT_FALSE(Logger_HasOverflowed());

  // Even when the logger is disabled, Logger_SendResponse() should still
  // transmit an (empty) message.
  Logger_SendResponse();
  ASSERT_NOT_EMPTY(received_messages);

  const Message &message = received_messages[0];
  ASSERT_TRUE(message.valid);
  ASSERT_FALSE(message.overflow);
  ASSERT_EQ(GET_LOG, message.command);
  ASSERT_EQ(static_cast<uint8_t>(0), message.return_code);
  ASSERT_EQ(string(""), message.data);
}

/*
 * Confirm passing a nullptr callback doesn't crash.
 */
void LoggerTest::testNullCallback() {
  Logger_Initialize(nullptr, PAYLOAD_SIZE);
  Logger_SetState(true);

  ASSERT_TRUE(Logger_IsEnabled());

  string test("This is a test");
  Logger_Log(test.c_str());

  ASSERT_TRUE(Logger_DataPending());
  ASSERT_FALSE(Logger_HasOverflowed());

  // A nullptr callback means no messages.
  Logger_SendResponse();
  ASSERT_EMPTY(received_messages);
}

/*
 * Confirm resetting the Logger causes the flags to be reset.
 */
void LoggerTest::testReset() {
  Logger_Initialize(TxFunction, PAYLOAD_SIZE);
  Logger_SetState(true);
  ASSERT_TRUE(Logger_IsEnabled());

  string test(1000, 'x');
  Logger_Log(test.c_str());

  ASSERT_TRUE(Logger_DataPending());
  ASSERT_TRUE(Logger_HasOverflowed());
  ASSERT_EMPTY(received_messages);

  // Now reset
  Logger_SetState(false);
  ASSERT_FALSE(Logger_IsEnabled());
  ASSERT_FALSE(Logger_DataPending());
  ASSERT_FALSE(Logger_HasOverflowed());
  ASSERT_EMPTY(received_messages);

  // Re-enable
  Logger_SetState(true);
  ASSERT_TRUE(Logger_IsEnabled());
  ASSERT_FALSE(Logger_DataPending());
  ASSERT_FALSE(Logger_HasOverflowed());

  string test2(10, 'x');
  Logger_Log(test2.c_str());

  ASSERT_TRUE(Logger_DataPending());

  Logger_SendResponse();
  ASSERT_NOT_EMPTY(received_messages);
  const Message &message = received_messages[0];
  ASSERT_TRUE(message.valid);
  ASSERT_EQ(GET_LOG, message.command);
  ASSERT_EQ(static_cast<uint8_t>(0), message.return_code);
  ASSERT_FALSE(message.overflow);
  ASSERT_EQ(test2, message.data);
}

/*
 * Check messages are correctly formed.
 */
void LoggerTest::testLogAndFetch() {
  // Set the payload size to something short so we can trigger the wrapping
  // behavior.
  Logger_Initialize(TxFunction, 100);
  Logger_SetState(true);
  Logger_Log(string(200, 'x').c_str());

  ASSERT_TRUE(Logger_DataPending());

  Logger_SendResponse();
  ASSERT_NOT_EMPTY(received_messages);

  {
    const Message &message = received_messages[0];
    ASSERT_TRUE(message.valid);
    ASSERT_EQ(GET_LOG, message.command);
    ASSERT_EQ(static_cast<uint8_t>(0), message.return_code);
    ASSERT_FALSE(message.overflow);
    ASSERT_EQ(string(99, 'x'), message.data);
  }

  received_messages.clear();

  // Now write some more data
  Logger_Log(string(200, 'y').c_str());

  ASSERT_TRUE(Logger_DataPending());
  ASSERT_TRUE(Logger_HasOverflowed());

  Logger_SendResponse();  // 99 'x'
  Logger_SendResponse();  // 2 'x', 97 'y'
  Logger_SendResponse();  // 58 'y'
  Logger_SendResponse();  // Empty
  ASSERT_EQ(static_cast<size_t>(4), received_messages.size());

  {
    const Message &message = received_messages[0];
    ASSERT_TRUE(message.valid);
    ASSERT_EQ(GET_LOG, message.command);
    ASSERT_EQ(static_cast<uint8_t>(0), message.return_code);
    ASSERT_TRUE(message.overflow);
    ASSERT_EQ(string(99, 'x'), message.data);
  }

  {
    const Message &message = received_messages[1];
    ASSERT_TRUE(message.valid);
    ASSERT_EQ(GET_LOG, message.command);
    ASSERT_EQ(static_cast<uint8_t>(0), message.return_code);
    ASSERT_FALSE(message.overflow);

    const string expected = "xx" + string(97, 'y');
    ASSERT_EQ(expected, message.data);
  }

  {
    const Message &message = received_messages[2];
    ASSERT_TRUE(message.valid);
    ASSERT_EQ(GET_LOG, message.command);
    ASSERT_EQ(static_cast<uint8_t>(0), message.return_code);
    ASSERT_FALSE(message.overflow);
    ASSERT_EQ(string(58, 'y'), message.data);
  }

  {
    const Message &message = received_messages[3];
    ASSERT_TRUE(message.valid);
    ASSERT_EQ(GET_LOG, message.command);
    ASSERT_EQ(static_cast<uint8_t>(0), message.return_code);
    ASSERT_FALSE(message.overflow);
    ASSERT_EQ(string(""), message.data);
  }
}

/**
 * Confirm the overflow flag is set correctly.
 */
void LoggerTest::testOverflow() {
  Logger_Initialize(TxFunction, PAYLOAD_SIZE);
  Logger_SetState(true);
  Logger_Log(string(1000, 'x').c_str());

  ASSERT_TRUE(Logger_HasOverflowed());

  Logger_SendResponse();
  ASSERT_NOT_EMPTY(received_messages);
  {
    const Message &message = received_messages[0];
    ASSERT_TRUE(message.valid);
    ASSERT_EQ(GET_LOG, message.command);
    ASSERT_EQ(static_cast<uint8_t>(0), message.return_code);
    ASSERT_TRUE(message.overflow);
    ASSERT_EQ(string(256, 'x'), message.data);
  }

  received_messages.clear();

  // Now fetch the next message, the overflow flag must clear.
  Logger_SendResponse();
  ASSERT_NOT_EMPTY(received_messages);
  {
    const Message &message = received_messages[0];
    ASSERT_TRUE(message.valid);
    ASSERT_EQ(GET_LOG, message.command);
    ASSERT_EQ(static_cast<uint8_t>(0), message.return_code);
    ASSERT_FALSE(message.overflow);
    ASSERT_EQ(string(""), message.data);
  }
}
