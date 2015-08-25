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

#include <algorithm>
#include <memory>

#include "responder.h"
#include "receiver_counters.h"
#include "Array.h"
#include "Matchers.h"
#include "RDMHandlerMock.h"
#include "SPIRGBMock.h"

using ::testing::IgnoreResult;
using ::testing::Return;
using ::testing::StrictMock;
using ::testing::WithArgs;
using ::testing::_;

class ResponderTest : public testing::Test {
 public:
  void SetUp() {
    RDMHandler_SetMock(&handler_mock);
    SPIRGB_SetMock(&spi_mock);
    Responder_Initialize();
    ReceiverCounters_ResetCounters();
  }

  void TearDown() {
    RDMHandler_SetMock(nullptr);
    SPIRGB_SetMock(nullptr);
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
  StrictMock<MockRDMHandler> handler_mock;
  MockSPIRGB spi_mock;

  static const uint8_t TEST_UID[];
  static const uint8_t ASC_FRAME[];
  static const uint8_t DMX_FRAME[];
  static const uint8_t RDM_FRAME[];
  static const uint8_t SHORT_DMX_FRAME[];
  static const uint8_t LONG_DMX_FRAME[];
};

const uint8_t ResponderTest::TEST_UID[] = {0x7a, 0x70, 0, 0, 0, 1};

const uint8_t ResponderTest::ASC_FRAME[] = {
  99,
  1, 2, 3, 4, 5, 6, 7, 8, 9, 10
};

const uint8_t ResponderTest::DMX_FRAME[] = {
  0,
  1, 2, 3, 4, 5, 6, 7, 8, 9, 10
};

const uint8_t ResponderTest::LONG_DMX_FRAME[] = {
  0,
  1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
  22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45
};

const uint8_t ResponderTest::SHORT_DMX_FRAME[] = {
  0, 1, 2
};

const uint8_t ResponderTest::RDM_FRAME[] = {
  0xcc, 0x01, 0x18, 0x7a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x70, 0x12, 0x34,
  0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x02, 0x00, 0x03, 0xdf
};

TEST_F(ResponderTest, rxSequence) {
  // The important bit here is that by interleaving different frames, the RDM
  // handler continues to be called when appropriate.
  EXPECT_CALL(handler_mock, HandleRequest(
        reinterpret_cast<const RDMHeader*>(RDM_FRAME), NULL))
    .Times(4);

  EXPECT_EQ(0, ReceiverCounters_DMXFrames());
  EXPECT_EQ(0, ReceiverCounters_ASCFrames());
  EXPECT_EQ(0, ReceiverCounters_RDMFrames());

  SendFrame(DMX_FRAME, arraysize(DMX_FRAME));

  EXPECT_EQ(1, ReceiverCounters_DMXFrames());
  EXPECT_EQ(0, ReceiverCounters_ASCFrames());
  EXPECT_EQ(0, ReceiverCounters_RDMFrames());

  SendFrame(RDM_FRAME, arraysize(RDM_FRAME));

  EXPECT_EQ(1, ReceiverCounters_DMXFrames());
  EXPECT_EQ(0, ReceiverCounters_ASCFrames());
  EXPECT_EQ(1, ReceiverCounters_RDMFrames());

  SendFrame(ASC_FRAME, arraysize(ASC_FRAME));

  EXPECT_EQ(1, ReceiverCounters_DMXFrames());
  EXPECT_EQ(1, ReceiverCounters_ASCFrames());
  EXPECT_EQ(1, ReceiverCounters_RDMFrames());

  SendFrame(RDM_FRAME, arraysize(RDM_FRAME));

  EXPECT_EQ(1, ReceiverCounters_DMXFrames());
  EXPECT_EQ(1, ReceiverCounters_ASCFrames());
  EXPECT_EQ(2, ReceiverCounters_RDMFrames());

  // 'empty' DMX frame
  SendFrame(DMX_FRAME, 1);

  EXPECT_EQ(2, ReceiverCounters_DMXFrames());
  EXPECT_EQ(1, ReceiverCounters_ASCFrames());
  EXPECT_EQ(2, ReceiverCounters_RDMFrames());

  SendFrame(RDM_FRAME, arraysize(RDM_FRAME));

  EXPECT_EQ(2, ReceiverCounters_DMXFrames());
  EXPECT_EQ(1, ReceiverCounters_ASCFrames());
  EXPECT_EQ(3, ReceiverCounters_RDMFrames());

  // Frames that arrive in 2 byte chunks
  SendFrame(DMX_FRAME, arraysize(DMX_FRAME), 2);

  EXPECT_EQ(3, ReceiverCounters_DMXFrames());
  EXPECT_EQ(1, ReceiverCounters_ASCFrames());
  EXPECT_EQ(3, ReceiverCounters_RDMFrames());

  SendFrame(RDM_FRAME, arraysize(RDM_FRAME), 2);

  EXPECT_EQ(3, ReceiverCounters_DMXFrames());
  EXPECT_EQ(1, ReceiverCounters_ASCFrames());
  EXPECT_EQ(4, ReceiverCounters_RDMFrames());

  // Confirm counters
  EXPECT_EQ(55, ReceiverCounters_DMXLastChecksum());
  EXPECT_EQ(10, ReceiverCounters_DMXLastSlotCount());
  EXPECT_EQ(0, ReceiverCounters_DMXMinimumSlotCount());
  EXPECT_EQ(10, ReceiverCounters_DMXMaximumSlotCount());
}

TEST_F(ResponderTest, rdmChecksumMismatch) {
  EXPECT_CALL(handler_mock, GetUID(_))
    .WillOnce(WithArgs<0>(IgnoreResult(CopyUID(TEST_UID))));

  const uint8_t bad_frame[] = {
    0xcc, 0x01, 0x18, 0x7a, 0x70, 0xff, 0xff, 0xff, 0xff, 0x7a, 0x70, 0x12,
    0x34, 0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x02, 0x00,
    0xAB, 0xCD
  };
  SendFrame(bad_frame, arraysize(bad_frame));

  EXPECT_EQ(1, ReceiverCounters_RDMChecksumInvalidCounter());
}

TEST_F(ResponderTest, badSubStartCode) {
  const uint8_t frame[] = {
    0xcc, 0x02, 0x18, 0x7a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x70, 0x12,
    0x34, 0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x02, 0x00,
    0x03, 0xe0
  };
  SendFrame(frame, arraysize(frame));
  EXPECT_EQ(1, ReceiverCounters_RDMSubStartCodeInvalidCounter());
}

TEST_F(ResponderTest, msgLenTooShort) {
  const uint8_t frame[] = {
    0xcc, 0x01, 0x17, 0x7a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x70, 0x12,
    0x34, 0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x02, 0x00,
    0x03, 0xe0
  };
  SendFrame(frame, arraysize(frame));
  EXPECT_EQ(1, ReceiverCounters_RDMMessageLengthInvalidCounter());
}

TEST_F(ResponderTest, paramDataLenMismatch) {
  const uint8_t frame[] = {
    0xcc, 0x01, 0x18, 0x7a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x70, 0x12,
    0x34, 0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x02, 0x01,
    0x01, 0x03, 0xe2
  };

  SendFrame(frame, arraysize(frame));
  EXPECT_EQ(1, ReceiverCounters_RDMParamDataLenInvalidCounter());

  const uint8_t frame2[] = {
    0xcc, 0x01, 0x18, 0x7a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x70, 0x12,
    0x34, 0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x02, 0x01,
    0x03, 0xdf
  };

  SendFrame(frame2, arraysize(frame2));
  EXPECT_EQ(2, ReceiverCounters_RDMParamDataLenInvalidCounter());
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

TEST_F(ResponderTest, dmxCounters) {
  EXPECT_EQ(0xff, ReceiverCounters_DMXLastChecksum());
  EXPECT_EQ(0xffff, ReceiverCounters_DMXLastSlotCount());
  EXPECT_EQ(0xffff, ReceiverCounters_DMXMinimumSlotCount());
  EXPECT_EQ(0xffff, ReceiverCounters_DMXMaximumSlotCount());

  SendFrame(DMX_FRAME, arraysize(DMX_FRAME));

  EXPECT_EQ(1, ReceiverCounters_DMXFrames());
  EXPECT_EQ(55, ReceiverCounters_DMXLastChecksum());
  EXPECT_EQ(10, ReceiverCounters_DMXLastSlotCount());
  EXPECT_EQ(0xffff, ReceiverCounters_DMXMinimumSlotCount());
  EXPECT_EQ(10, ReceiverCounters_DMXMaximumSlotCount());

  SendFrame(DMX_FRAME, arraysize(DMX_FRAME));

  EXPECT_EQ(2, ReceiverCounters_DMXFrames());
  EXPECT_EQ(55, ReceiverCounters_DMXLastChecksum());
  EXPECT_EQ(10, ReceiverCounters_DMXLastSlotCount());
  EXPECT_EQ(10, ReceiverCounters_DMXMinimumSlotCount());
  EXPECT_EQ(10, ReceiverCounters_DMXMaximumSlotCount());

  SendFrame(SHORT_DMX_FRAME, arraysize(SHORT_DMX_FRAME));
  SendFrame(SHORT_DMX_FRAME, arraysize(SHORT_DMX_FRAME));

  EXPECT_EQ(4, ReceiverCounters_DMXFrames());
  EXPECT_EQ(3, ReceiverCounters_DMXLastChecksum());
  EXPECT_EQ(2, ReceiverCounters_DMXLastSlotCount());
  EXPECT_EQ(2, ReceiverCounters_DMXMinimumSlotCount());
  EXPECT_EQ(10, ReceiverCounters_DMXMaximumSlotCount());

  SendFrame(LONG_DMX_FRAME, arraysize(LONG_DMX_FRAME));

  EXPECT_EQ(5, ReceiverCounters_DMXFrames());
  EXPECT_EQ(0x0b, ReceiverCounters_DMXLastChecksum());
  EXPECT_EQ(45, ReceiverCounters_DMXLastSlotCount());
  EXPECT_EQ(2, ReceiverCounters_DMXMinimumSlotCount());
  EXPECT_EQ(45, ReceiverCounters_DMXMaximumSlotCount());
}

TEST_F(ResponderTest, SPIOutput) {
  SPIRGBConfiguration spi_config;
  spi_config.module_id = SPI_ID_1;
  spi_config.baud_rate = 2000000;
  spi_config.use_enhanced_buffering = false;
  SPIRGB_Init(&spi_config);

  EXPECT_CALL(spi_mock, BeginUpdate())
    .Times(1);
  EXPECT_CALL(spi_mock, SetPixel(0, RED, 1)).Times(1);
  EXPECT_CALL(spi_mock, SetPixel(0, GREEN, 2)).Times(1);
  EXPECT_CALL(spi_mock, SetPixel(0, BLUE, 3)).Times(1);
  EXPECT_CALL(spi_mock, SetPixel(1, RED, 4)).Times(1);
  EXPECT_CALL(spi_mock, SetPixel(1, GREEN, 5)).Times(1);
  EXPECT_CALL(spi_mock, SetPixel(1, BLUE, 6)).Times(1);
  EXPECT_CALL(spi_mock, CompleteUpdate())
    .Times(1);

  SendFrame(DMX_FRAME, arraysize(DMX_FRAME));
}
