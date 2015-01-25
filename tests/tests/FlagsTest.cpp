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
 * FlagsTest.cpp
 * Tests for the Flags code.
 * Copyright (C) 2015 Simon Newton
 */

#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <vector>

#include "flags.h"
#include "TestUtils.h"

using std::string;

class FlagsTest *flags_test = nullptr;

class FlagsTest: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(FlagsTest);
  CPPUNIT_TEST(testFlags);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp() {
    flags_test = this;
  }

  void tearDown() {
    flags_test = nullptr;
  }

  void testFlags();

  bool Tx(Command command, uint8_t return_code, const IOVec* iov,
          unsigned int iov_count) {
    string payload;
    for (unsigned int i = 0; i != iov_count; i++) {
      payload.append(reinterpret_cast<const char*>(iov[i].base), iov[i].length);
    }

    Message message = {
      .command = command,
      .return_code = return_code,
      .data = payload
    };
    received_messages.push_back(message);
    return true;
  }

 private:
  struct Message {
    Command command;
    uint8_t return_code;
    string data;
  };

  std::vector<Message> received_messages;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FlagsTest);

/*
 * Called by the Flags code under test.
 */
bool TxFunction(Command command, uint8_t return_code, const IOVec* iov,
                unsigned int iov_count) {
  if (flags_test) {
    return flags_test->Tx(command, return_code, iov, iov_count);
  }
  return true;
}

/**
 * Confirm the flags work as intended.
 */
void FlagsTest::testFlags() {
  Flags_Initialize(TxFunction);
  ASSERT_FALSE(Flags_HasChanged());

  Flags_SendResponse();
  ASSERT_NOT_EMPTY(received_messages);
  {
    const Message &message = received_messages[0];
    ASSERT_EQ(GET_FLAGS, message.command);
    ASSERT_EQ(static_cast<uint8_t>(0), message.return_code);
    ASSERT_EQ((size_t)1, message.data.size());
    ASSERT_EQ(string(1, 0), message.data);
    received_messages.clear();
  }

  // Set the 'Log overflow' flag.
  Flags_SetLogOverflow();
  ASSERT_TRUE(Flags_HasChanged());

  Flags_SendResponse();
  ASSERT_NOT_EMPTY(received_messages);
  {
    const Message &message = received_messages[0];
    ASSERT_EQ(GET_FLAGS, message.command);
    ASSERT_EQ(static_cast<uint8_t>(RC_OK), message.return_code);
    ASSERT_EQ(string(1, 1), message.data);
    received_messages.clear();
  }

  ASSERT_FALSE(Flags_HasChanged());

  // Set the 'TX drop' flag.
  Flags_SetTXDrop();
  ASSERT_TRUE(Flags_HasChanged());

  Flags_SendResponse();
  ASSERT_NOT_EMPTY(received_messages);
  {
    const Message &message = received_messages[0];
    ASSERT_EQ(GET_FLAGS, message.command);
    ASSERT_EQ(static_cast<uint8_t>(RC_OK), message.return_code);
    ASSERT_EQ(string(1, 2), message.data);
    received_messages.clear();
  }

  // Set the 'TX error' flag.
  Flags_SetTXError();
  ASSERT_TRUE(Flags_HasChanged());

  Flags_SendResponse();
  ASSERT_NOT_EMPTY(received_messages);
  {
    const Message &message = received_messages[0];
    ASSERT_EQ(GET_FLAGS, message.command);
    ASSERT_EQ(static_cast<uint8_t>(RC_OK), message.return_code);
    ASSERT_EQ(string(1, 4), message.data);
    received_messages.clear();
  }

  // TODO(simon): Add a test to confirm the flags aren't reset if the TX
  // function fails. We need a decent mocking infrastructure setup before we
  // can do this.
}
