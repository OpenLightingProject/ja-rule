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

#include <gtest/gtest.h>


#include "stream_decoder.h"
#include "Array.h"
#include "MessageHandlerMock.h"

using ::testing::Args;
using ::testing::StrictMock;
using ::testing::Return;
using ::testing::_;

class StreamDecoderTest : public testing::Test {
 public:
  void SetUp() {
    MessageHandler_SetMock(&message_handler_mock);
  }

  void TearDown() {
    MessageHandler_SetMock(nullptr);
  }

  StrictMock<MockMessageHandler> message_handler_mock;

  static const uint8_t empty_msg1[];
  static const uint8_t message1[];

  static const uint8_t PAYLOAD_OFFSET = 6;
  // Size of the payload for message1
  static const uint8_t MSG1_PAYLOAD_SIZE = 5;
};

const uint8_t StreamDecoderTest::empty_msg1[] = {
  0x5a, 0x44, 0x01, 0x02, 0x00, 0x00, 0xa5};

const uint8_t StreamDecoderTest::message1[] = {
  0x5a, 0x45, 0x02, 0x02, 0x05, 0x00, 1, 2, 3, 4, 5, 0xa5};

/*
 * Check nothing happens if the handler is null.
 */
TEST_F(StreamDecoderTest, emptyHandlerTest) {
  StreamDecoder_Initialize(nullptr);
  StreamDecoder_Process(empty_msg1, arraysize(empty_msg1));
}

/*
 * Check that simple messages can be decoded.
 */
TEST_F(StreamDecoderTest, simpleMessage) {
  StreamDecoder_Initialize(MessageHandler_HandleMessage);
  EXPECT_FALSE(StreamDecoder_GetFragmentedFrameFlag());

  testing::InSequence seq;
  EXPECT_CALL(message_handler_mock,
              HandleMessage(MessageIs(0x44, 0x0201, nullptr, 0u)));
  EXPECT_CALL(message_handler_mock,
              HandleMessage(MessageIs(0x45, 0x0202, message1 + 5, 5u)));

  StreamDecoder_Process(empty_msg1, arraysize(empty_msg1));
  StreamDecoder_Process(message1, arraysize(message1));
}

/*
 * Check that fragmentation is correctly handled.
 */
TEST_F(StreamDecoderTest, fragmentedMessage) {
  StreamDecoder_Initialize(MessageHandler_HandleMessage);
  EXPECT_FALSE(StreamDecoder_GetFragmentedFrameFlag());

  EXPECT_CALL(message_handler_mock,
              HandleMessage(MessageIs(0x45, 0x0202, message1 + PAYLOAD_OFFSET,
                                      MSG1_PAYLOAD_SIZE)))
    .Times(3);

  // Split the calls to StreamDecoder_Process in the middle of the payload data.
  unsigned int split_index = PAYLOAD_OFFSET + MSG1_PAYLOAD_SIZE / 2;
  StreamDecoder_Process(message1, split_index);
  StreamDecoder_Process(message1 + split_index,
                        arraysize(message1) - split_index);

  EXPECT_TRUE(StreamDecoder_GetFragmentedFrameFlag());
  StreamDecoder_ClearFragmentedFrameFlag();
  EXPECT_FALSE(StreamDecoder_GetFragmentedFrameFlag());

  // Try another fragmented frame
  split_index = PAYLOAD_OFFSET + 1;
  StreamDecoder_Process(message1, split_index);
  StreamDecoder_Process(message1 + split_index,
                        arraysize(message1) - split_index);

  EXPECT_TRUE(StreamDecoder_GetFragmentedFrameFlag());
  StreamDecoder_ClearFragmentedFrameFlag();
  EXPECT_FALSE(StreamDecoder_GetFragmentedFrameFlag());

  // And one more
  split_index = PAYLOAD_OFFSET + MSG1_PAYLOAD_SIZE - 1;
  StreamDecoder_Process(message1, split_index);
  StreamDecoder_Process(message1 + split_index,
                       arraysize(message1) - split_index);
  EXPECT_TRUE(StreamDecoder_GetFragmentedFrameFlag());
  StreamDecoder_ClearFragmentedFrameFlag();
  EXPECT_FALSE(StreamDecoder_GetFragmentedFrameFlag());
}

TEST_F(StreamDecoderTest, singleByteRx) {
  StreamDecoder_Initialize(MessageHandler_HandleMessage);

  testing::InSequence seq;
  EXPECT_CALL(message_handler_mock,
              HandleMessage(MessageIs(0x44, 0x0201, nullptr, 0)));
  EXPECT_CALL(message_handler_mock,
              HandleMessage(MessageIs(0x45, 0x0202, message1 + PAYLOAD_OFFSET,
                                      MSG1_PAYLOAD_SIZE)));

  for (unsigned int i = 0; i < arraysize(empty_msg1); i++) {
    // Send s byte at a time.
    StreamDecoder_Process(empty_msg1 + i, 1);
  }

  EXPECT_FALSE(StreamDecoder_GetFragmentedFrameFlag());

  for (unsigned int i = 0; i < arraysize(message1); i++) {
    // Send s byte at a time.
    StreamDecoder_Process(message1 + i, 1);
  }

  EXPECT_TRUE(StreamDecoder_GetFragmentedFrameFlag());
  StreamDecoder_ClearFragmentedFrameFlag();
  EXPECT_FALSE(StreamDecoder_GetFragmentedFrameFlag());
}

/*
 * Check we don't begin a message until a valid SOM / EOM is found
 */
TEST_F(StreamDecoderTest, noise) {
  StreamDecoder_Initialize(MessageHandler_HandleMessage);

  EXPECT_CALL(message_handler_mock,
              HandleMessage(MessageIs(0x44, 0x0201, nullptr, 0)));

  for (uint8_t i = 0; i < 50; i++) {
    StreamDecoder_Process(&i, 1);  // noise
  }

  const uint8_t more_noise[] = {'n', 'o', 'i', 's', 'e'};
  StreamDecoder_Process(more_noise, arraysize(more_noise));

  StreamDecoder_Process(empty_msg1, arraysize(empty_msg1));
}

/*
 * Check that messages without a valid EOM do not result in the handler being
 * called.
 */
TEST_F(StreamDecoderTest, missingEOM) {
  StreamDecoder_Initialize(MessageHandler_HandleMessage);

  StreamDecoder_Process(empty_msg1, arraysize(empty_msg1) - 1);
  uint8_t not_eom = 0;
  StreamDecoder_Process(&not_eom, 1);  // not an EOM marker
}
