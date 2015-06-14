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
 * ResponderTest.cpp
 * Tests for the Responder code.
 * Copyright (C) 2015 Simon Newton
 */

#include <gtest/gtest.h>

#include <memory>

#include "responder.h"
#include "Array.h"
#include "Matchers.h"
#include "RDMResponderMock.h"

using ::testing::StrictMock;
using ::testing::Return;
using ::testing::_;

class ResponderTest : public testing::Test {
 public:
  void SetUp() {
    RDMResponder_SetMock(&rdm_mock);
    Responder_Initialize();
  }

  void TearDown() {
    RDMResponder_SetMock(NULL);
  }

  void SendFrame(const uint8_t *frame, unsigned int size,
                 unsigned int chunk_size = 1) {
    TransceiverEvent event;
    event.token = 0;
    event.op = T_OP_RX;
    event.data = frame;
    event.timing = NULL;

    unsigned int i = 0;
    while (i < size) {
      event.result = i ? T_RESULT_RX_CONTINUE_FRAME : T_RESULT_RX_START_FRAME;
      event.length = std::min(i + chunk_size, size);
      Responder_Receive(&event);
      i += chunk_size;
    }
  }

 protected:
  StrictMock<MockRDMResponder> rdm_mock;

  static const uint8_t ASC_FRAME[];
  static const uint8_t DMX_FRAME[];
  static const uint8_t RDM_FRAME[];
};

const uint8_t ResponderTest::ASC_FRAME[] = {
  99,
  1, 2, 3, 4, 5, 6, 7, 8, 9, 10
};

const uint8_t ResponderTest::DMX_FRAME[] = {
  0,
  1, 2, 3, 4, 5, 6, 7, 8, 9, 10
};

const uint8_t ResponderTest::RDM_FRAME[] = {
  0xcc, 0x01, 0x18, 0x7a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x70, 0x12, 0x34,
  0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x02, 0x00, 0x03, 0xdf
};

TEST_F(ResponderTest, rxSequence) {
  // The important bit here is that by interleaving different frames, the RDM
  // handler continues to be called when appropriate.
  EXPECT_CALL(rdm_mock, UIDRequiresAction(&RDM_FRAME[3]))
    .WillRepeatedly(Return(true));
  EXPECT_CALL(rdm_mock, VerifyChecksum(RDM_FRAME, arraysize(RDM_FRAME)))
    .WillRepeatedly(Return(true));
  EXPECT_CALL(rdm_mock, HandleRequest(
        reinterpret_cast<const RDMHeader*>(RDM_FRAME), NULL, 0))
    .Times(4);

  EXPECT_EQ(0, Responder_DMXFrames());
  EXPECT_EQ(0, Responder_ASCFrames());
  EXPECT_EQ(0, Responder_RDMFrames());

  SendFrame(DMX_FRAME, arraysize(DMX_FRAME));

  EXPECT_EQ(1, Responder_DMXFrames());
  EXPECT_EQ(0, Responder_ASCFrames());
  EXPECT_EQ(0, Responder_RDMFrames());

  SendFrame(RDM_FRAME, arraysize(RDM_FRAME));

  EXPECT_EQ(1, Responder_DMXFrames());
  EXPECT_EQ(0, Responder_ASCFrames());
  EXPECT_EQ(1, Responder_RDMFrames());

  SendFrame(ASC_FRAME, arraysize(ASC_FRAME));

  EXPECT_EQ(1, Responder_DMXFrames());
  EXPECT_EQ(1, Responder_ASCFrames());
  EXPECT_EQ(1, Responder_RDMFrames());

  SendFrame(RDM_FRAME, arraysize(RDM_FRAME));

  EXPECT_EQ(1, Responder_DMXFrames());
  EXPECT_EQ(1, Responder_ASCFrames());
  EXPECT_EQ(2, Responder_RDMFrames());

  // 'empty' DMX frame
  SendFrame(DMX_FRAME, 1);

  EXPECT_EQ(2, Responder_DMXFrames());
  EXPECT_EQ(1, Responder_ASCFrames());
  EXPECT_EQ(2, Responder_RDMFrames());

  SendFrame(RDM_FRAME, arraysize(RDM_FRAME));

  EXPECT_EQ(2, Responder_DMXFrames());
  EXPECT_EQ(1, Responder_ASCFrames());
  EXPECT_EQ(3, Responder_RDMFrames());

  // Frames that arrive in 2 byte chunks
  SendFrame(DMX_FRAME, arraysize(DMX_FRAME), 2);

  EXPECT_EQ(3, Responder_DMXFrames());
  EXPECT_EQ(1, Responder_ASCFrames());
  EXPECT_EQ(3, Responder_RDMFrames());

  SendFrame(RDM_FRAME, arraysize(RDM_FRAME), 2);

  EXPECT_EQ(3, Responder_DMXFrames());
  EXPECT_EQ(1, Responder_ASCFrames());
  EXPECT_EQ(4, Responder_RDMFrames());
}

TEST_F(ResponderTest, rdmNotForUs) {
  EXPECT_CALL(rdm_mock, UIDRequiresAction(&RDM_FRAME[3]))
    .WillOnce(Return(false));
  SendFrame(RDM_FRAME, arraysize(RDM_FRAME));
}

TEST_F(ResponderTest, rdmChecksumMismatch) {
  EXPECT_CALL(rdm_mock, UIDRequiresAction(&RDM_FRAME[3]))
    .WillOnce(Return(true));
  EXPECT_CALL(rdm_mock, VerifyChecksum(RDM_FRAME, arraysize(RDM_FRAME)))
    .WillOnce(Return(false));
  SendFrame(RDM_FRAME, arraysize(RDM_FRAME));

  EXPECT_EQ(1, Responder_RDMChecksumInvalidCounter());
}

TEST_F(ResponderTest, badSubStartCode) {
  const uint8_t frame[] = {
    0xcc, 0x02, 0x18, 0x7a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x70, 0x12,
    0x34, 0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x02, 0x00,
    0x03, 0xe0
  };
  SendFrame(frame, arraysize(frame));
  EXPECT_EQ(1, Responder_RDMSubStartCodeInvalidCounter());
}

TEST_F(ResponderTest, msgLenTooShort) {
  const uint8_t frame[] = {
    0xcc, 0x01, 0x17, 0x7a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x70, 0x12,
    0x34, 0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x02, 0x00,
    0x03, 0xe0
  };
  SendFrame(frame, arraysize(frame));
  EXPECT_EQ(1, Responder_RDMMessageLengthInvalidCounter());
}

TEST_F(ResponderTest, paramDataLenMismatch) {
  const uint8_t frame[] = {
    0xcc, 0x01, 0x18, 0x7a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x70, 0x12,
    0x34, 0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x02, 0x01,
    0x01, 0x03, 0xe2
  };

  EXPECT_CALL(rdm_mock, UIDRequiresAction(&frame[3]))
    .WillOnce(Return(true));

  SendFrame(frame, arraysize(frame));
  EXPECT_EQ(1, Responder_RDMParamDataLenInvalidCounter());
}

// Send an RDM frame that's too short, that also contains a valid checksum

TEST_F(ResponderTest, nonRxOp) {
  TransceiverEvent event;
  event.token = 0;
  event.op = T_OP_TX_ONLY;
  event.data = NULL;
  event.length = 0;
  event.timing = NULL;
  event.result = T_RESULT_RX_CONTINUE_FRAME;
  Responder_Receive(&event);
}
