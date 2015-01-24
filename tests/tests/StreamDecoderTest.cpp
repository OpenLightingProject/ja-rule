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
 * StreamDecoderTest.cpp
 * Tests for the StreamDecoder code.
 * Copyright (C) 2015 Simon Newton
 */

#include <cppunit/extensions/HelperMacros.h>
#include <string>
#include <vector>

#include "stream_decoder.h"
#include "Array.h"
#include "TestUtils.h"

using std::string;

class StreamDecoderTest *test = nullptr;

class StreamDecoderTest: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(StreamDecoderTest);
  CPPUNIT_TEST(testEmptyHandler);
  CPPUNIT_TEST(testSimpleMessages);
  CPPUNIT_TEST(testFragmentedMessage);
  CPPUNIT_TEST(testSingleByteRx);
  CPPUNIT_TEST(testNoise);
  CPPUNIT_TEST(testMissingEOM);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp() {
    test = this;
  }

  void tearDown() {
    test = nullptr;
  }

  void testEmptyHandler();
  void testSimpleMessages();
  void testFragmentedMessage();
  void testSingleByteRx();
  void testNoise();
  void testMissingEOM();

  void HandleMessage(const Message *message) {
    received_messages.push_back(*message);
  }

 private:
  std::vector<Message> received_messages;

  static const uint8_t empty_msg1[];
  static const uint8_t message1[];

  static const uint8_t PAYLOAD_OFFSET = 5;
  // Size of the payload for message1
  static const uint8_t MSG1_PAYLOAD_SIZE = 5;
};

CPPUNIT_TEST_SUITE_REGISTRATION(StreamDecoderTest);

/*
 * Called by the StreamDecoder code under test.
 */
void HandleMessage(const Message *message) {
  if (test) {
    test->HandleMessage(message);
  }
}

const uint8_t StreamDecoderTest::empty_msg1[] = {
  0x5a, 0x01, 0x02, 0x00, 0x00, 0xa5};

const uint8_t StreamDecoderTest::message1[] = {
  0x5a, 0x02, 0x02, 0x05, 0x00, 1, 2, 3, 4, 5, 0xa5};

/*
 * Check nothing happens if the handler is null.
 */
void StreamDecoderTest::testEmptyHandler() {
  StreamDecoder_Initialize(nullptr);
  StreamDecoder_Process(empty_msg1, arraysize(empty_msg1));
  ASSERT_EMPTY(received_messages);
}

/*
 * Check that simple messages can be decoded.
 */
void StreamDecoderTest::testSimpleMessages() {
  StreamDecoder_Initialize(::HandleMessage);
  ASSERT_FALSE(StreamDecoder_GetFragmentedFrameFlag());

  StreamDecoder_Process(empty_msg1, arraysize(empty_msg1));

  ASSERT_NOT_EMPTY(received_messages);
  ASSERT_FALSE(StreamDecoder_GetFragmentedFrameFlag());

  {
    const Message &message = received_messages[0];
    ASSERT_EQ(static_cast<uint16_t>(0x0201), message.command);
    ASSERT_EQ(static_cast<uint16_t>(0), message.length);
    ASSERT_NULL(message.payload);
    received_messages.clear();
  }

  StreamDecoder_Process(message1, arraysize(message1));
  ASSERT_NOT_EMPTY(received_messages);

  {
    const Message &message = received_messages[0];
    ASSERT_EQ(static_cast<uint16_t>(0x0202), message.command);
    ASSERT_EQ(static_cast<uint16_t>(5), message.length);
    ASSERT_NOT_NULL(message.payload);
    ASSERT_ARRAY_EQ(message1 + 5, 5, message.payload, message.length);
  }
}

/*
 * Check that fragmentation is correctly handled.
 */
void StreamDecoderTest::testFragmentedMessage() {
  StreamDecoder_Initialize(::HandleMessage);

  // Split the calls to StreamDecoder_Process in the middle of the payload data.
  unsigned int split_index = PAYLOAD_OFFSET + MSG1_PAYLOAD_SIZE / 2;
  StreamDecoder_Process(message1, split_index);
  StreamDecoder_Process(message1 + split_index,
                       arraysize(message1) - split_index);

  ASSERT_NOT_EMPTY(received_messages);

  {
    const Message &message = received_messages[0];
    ASSERT_EQ(static_cast<uint16_t>(0x0202), message.command);
    ASSERT_EQ(static_cast<uint16_t>(MSG1_PAYLOAD_SIZE), message.length);
    ASSERT_NOT_NULL(message.payload);
    ASSERT_ARRAY_EQ(message1 + PAYLOAD_OFFSET, MSG1_PAYLOAD_SIZE,
                    message.payload, message.length);
    ASSERT_TRUE(StreamDecoder_GetFragmentedFrameFlag());
    StreamDecoder_ClearFragmentedFrameFlag();
    ASSERT_FALSE(StreamDecoder_GetFragmentedFrameFlag());
    received_messages.clear();
  }

  // Try another fragmented frame
  split_index = PAYLOAD_OFFSET + 1;
  StreamDecoder_Process(message1, split_index);
  StreamDecoder_Process(message1 + split_index,
                       arraysize(message1) - split_index);

  {
    const Message &message = received_messages[0];
    ASSERT_EQ(static_cast<uint16_t>(0x0202), message.command);
    ASSERT_EQ(static_cast<uint16_t>(MSG1_PAYLOAD_SIZE), message.length);
    ASSERT_NOT_NULL(message.payload);
    ASSERT_ARRAY_EQ(message1 + PAYLOAD_OFFSET, MSG1_PAYLOAD_SIZE,
                    message.payload, message.length);
    ASSERT_TRUE(StreamDecoder_GetFragmentedFrameFlag());
    StreamDecoder_ClearFragmentedFrameFlag();
    ASSERT_FALSE(StreamDecoder_GetFragmentedFrameFlag());
    received_messages.clear();
  }

  // And one more
  split_index = PAYLOAD_OFFSET + MSG1_PAYLOAD_SIZE - 1;
  StreamDecoder_Process(message1, split_index);
  StreamDecoder_Process(message1 + split_index,
                       arraysize(message1) - split_index);

  {
    const Message &message = received_messages[0];
    ASSERT_EQ(static_cast<uint16_t>(0x0202), message.command);
    ASSERT_EQ(static_cast<uint16_t>(MSG1_PAYLOAD_SIZE), message.length);
    ASSERT_NOT_NULL(message.payload);
    ASSERT_ARRAY_EQ(message1 + PAYLOAD_OFFSET, MSG1_PAYLOAD_SIZE,
                    message.payload, message.length);
    ASSERT_TRUE(StreamDecoder_GetFragmentedFrameFlag());
    StreamDecoder_ClearFragmentedFrameFlag();
    ASSERT_FALSE(StreamDecoder_GetFragmentedFrameFlag());
    received_messages.clear();
  }
}

void StreamDecoderTest::testSingleByteRx() {
  StreamDecoder_Initialize(::HandleMessage);

  for (unsigned int i = 0; i < arraysize(empty_msg1); i++) {
    // Send s byte at a time.
    StreamDecoder_Process(empty_msg1 + i, 1);
  }

  ASSERT_NOT_EMPTY(received_messages);
  ASSERT_FALSE(StreamDecoder_GetFragmentedFrameFlag());

  {
    const Message &message = received_messages[0];
    ASSERT_EQ(static_cast<uint16_t>(0x0201), message.command);
    ASSERT_EQ(static_cast<uint16_t>(0), message.length);
    ASSERT_NULL(message.payload);
    // Because the payload is empty, this is not considered fragmented.
    ASSERT_FALSE(StreamDecoder_GetFragmentedFrameFlag());
    received_messages.clear();
  }

  for (unsigned int i = 0; i < arraysize(message1); i++) {
    // Send s byte at a time.
    StreamDecoder_Process(message1 + i, 1);
  }

  ASSERT_NOT_EMPTY(received_messages);

  {
    const Message &message = received_messages[0];
    ASSERT_EQ(static_cast<uint16_t>(0x0202), message.command);
    ASSERT_EQ(static_cast<uint16_t>(MSG1_PAYLOAD_SIZE), message.length);
    ASSERT_NOT_NULL(message.payload);
    ASSERT_ARRAY_EQ(message1 + PAYLOAD_OFFSET, MSG1_PAYLOAD_SIZE,
                    message.payload, message.length);
    ASSERT_TRUE(StreamDecoder_GetFragmentedFrameFlag());
    StreamDecoder_ClearFragmentedFrameFlag();
    ASSERT_FALSE(StreamDecoder_GetFragmentedFrameFlag());
  }
}

/**
 * Check we don't begin a message until a valid SOM / EOM is found
 */
void StreamDecoderTest::testNoise() {
  StreamDecoder_Initialize(::HandleMessage);

  for (uint8_t i = 0; i < 50; i++) {
    StreamDecoder_Process(&i, 1);  // noise
  }
  const uint8_t more_noise[] = {'n', 'o', 'i', 's', 'e'};
  StreamDecoder_Process(more_noise, arraysize(more_noise));

  StreamDecoder_Process(empty_msg1, arraysize(empty_msg1));
  ASSERT_NOT_EMPTY(received_messages);

  const Message &message = received_messages[0];
  ASSERT_EQ(static_cast<uint16_t>(0x0201), message.command);
  ASSERT_EQ(static_cast<uint16_t>(0), message.length);
  ASSERT_NULL(message.payload);
}

/*
 * Check that messages without a valid EOM do not result in the handler being
 * called.
 */
void StreamDecoderTest::testMissingEOM() {
  StreamDecoder_Initialize(::HandleMessage);
  StreamDecoder_Process(empty_msg1, arraysize(empty_msg1) - 1);
  uint8_t not_eom = 0;
  StreamDecoder_Process(&not_eom, 1);  // not an EOM marker
  ASSERT_EMPTY(received_messages);
}
