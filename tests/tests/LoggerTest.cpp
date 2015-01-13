
#include <string>
#include <vector>

#include "logger.h"
#include "TestUtils.h"

#include <cppunit/extensions/HelperMacros.h>

using std::string;

class LoggerTest *logger_test = NULL;

extern void _mon_putc(char c);

class LoggerTest: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(LoggerTest);
  CPPUNIT_TEST(testDisabled);
  CPPUNIT_TEST(test);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp() {
    logger_test = this;
  }

  void tearDown() {
    logger_test = NULL;
  }

  void testDisabled();
  void test();

  void Tx(Command command, uint8_t return_code, const IOVec* data) {
    Message message = {
      .command = command,
      .return_code = return_code
    };
    while (data != NULL) {
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
  Logging_Initialize(TxFunction, LOGGING_DISABLE);

  string test("This is a test");
  Logging_Log(test.c_str());

  // Check empty bit

  Logging_SendResponse();
  ASSERT_NOT_EMPTY(received_messages);

  const Message &message = received_messages[0];
  ASSERT_EQ(GET_LOG, message.command);
  ASSERT_EQ(static_cast<uint8_t>(0), message.return_code);
  ASSERT_EQ(string(""), message.data);
}

/*
 * 
 */
void LoggerTest::test() {
  Logging_Initialize(TxFunction, LOGGING_ENABLE);
  string test("This is a test");
  Logging_Log(test.c_str());

  Logging_SendResponse();
  ASSERT_NOT_EMPTY(received_messages);
  const Message &message = received_messages[0];
  ASSERT_EQ(GET_LOG, message.command);
  ASSERT_EQ(static_cast<uint8_t>(0), message.return_code);
  ASSERT_EQ(test, message.data);
}
