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
  CPPUNIT_TEST(test);
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
  void test();

  void Tx(Command command, uint8_t return_code, const IOVec* data) {
    Message message = {
      .command = command,
      .return_code = return_code
    };
    while (data != nullptr) {
      message.data.append(reinterpret_cast<char*>(data->base), data->length);
      data++;
    }
    received_messages.push_back(message);
  }

 private:
  struct Message {
    Command command;
    uint8_t return_code;
    string data;
  };

  std::vector<Message> received_messages;
};

CPPUNIT_TEST_SUITE_REGISTRATION(LoggerTest);

/*
 * Called by the Logger code under test.
 */
void TxFunction(Command command, uint8_t return_code, const IOVec* data) {
  if (logger_test) {
    logger_test->Tx(command, return_code, data);
  }
}

/*
 * Confirm when the logger is disabled, no writes occur.
 */
void LoggerTest::testDisabled() {
  Logging_Initialize(TxFunction, false);

  string test("This is a test");
  Logging_Log(test.c_str());

  ASSERT_FALSE(Logging_DataPending());
  ASSERT_FALSE(Logging_HasOverflowed());

  // Even when the logger is disabled, Logging_SendResponse() should still
  // transmit an (empty) message.
  Logging_SendResponse();
  ASSERT_NOT_EMPTY(received_messages);

  const Message &message = received_messages[0];
  ASSERT_EQ(GET_LOG, message.command);
  ASSERT_EQ(static_cast<uint8_t>(0), message.return_code);
  ASSERT_EQ(string(""), message.data);
}

/*
 * Confirm passing a nullptr callback doesn't crash.
 */
void LoggerTest::testNullCallback() {
  Logging_Initialize(nullptr, true);

  string test("This is a test");
  Logging_Log(test.c_str());

  ASSERT_TRUE(Logging_DataPending());
  ASSERT_FALSE(Logging_HasOverflowed());

  // A nullptr callback means no messages.
  Logging_SendResponse();
  ASSERT_EMPTY(received_messages);
}

/*
 * Confirm resetting the Logger causes the flags to be reset.
 */
void LoggerTest::testReset() {
  Logging_Initialize(TxFunction, true);

  string test(1000, 'x');
  Logging_Log(test.c_str());

  ASSERT_TRUE(Logging_DataPending());
  ASSERT_TRUE(Logging_HasOverflowed());
  ASSERT_EMPTY(received_messages);

  // Now reset
  Logging_SetState(false);
  ASSERT_FALSE(Logging_DataPending());
  ASSERT_FALSE(Logging_HasOverflowed());
  ASSERT_EMPTY(received_messages);

  // Re-enable
  Logging_SetState(true);
  ASSERT_FALSE(Logging_DataPending());
  ASSERT_FALSE(Logging_HasOverflowed());

  string test2(10, 'x');
  Logging_Log(test2.c_str());

  ASSERT_TRUE(Logging_DataPending());

  Logging_SendResponse();
  ASSERT_NOT_EMPTY(received_messages);
  const Message &message = received_messages[0];
  ASSERT_EQ(GET_LOG, message.command);
  ASSERT_EQ(static_cast<uint8_t>(0), message.return_code);
  ASSERT_EQ(test2, message.data);
}

/*
 * 
 */
void LoggerTest::test() {
  Logging_Initialize(TxFunction, true);
  string test("This is a test");
  Logging_Log(test.c_str());

  ASSERT_TRUE(Logging_DataPending());

  Logging_SendResponse();
  ASSERT_NOT_EMPTY(received_messages);
  const Message &message = received_messages[0];
  ASSERT_EQ(GET_LOG, message.command);
  ASSERT_EQ(static_cast<uint8_t>(0), message.return_code);
  ASSERT_EQ(test, message.data);
}
