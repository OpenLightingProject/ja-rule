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
 * USBTransportTest.cpp
 * Tests for the USB Transport code.
 * Copyright (C) 2015 Simon Newton
 */

#include <gtest/gtest.h>

#include <string.h>

#include "system_definitions.h"

#include "Array.h"
#include "LoggerMock.h"
#include "Matchers.h"
#include "StreamDecoderMock.h"
#include "USBMock.h"
#include "flags.h"
#include "usb_transport.h"

using ::testing::Args;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::StrictMock;
using ::testing::NotNull;
using ::testing::_;

typedef void (*USBEventHandler)(USB_DEVICE_EVENT, void*, uintptr_t);

class USBTransportTest : public testing::Test {
 public:
  void SetUp() {
    USB_SetMock(&usb_mock);
    StreamDecoder_SetMock(&stream_decoder_mock);
    // Logger_Initialize(Transport_Send, PAYLOAD_SIZE);
  }

  void TearDown() {
    USB_SetMock(nullptr);
    StreamDecoder_SetMock(nullptr);
  }

  void ConfigureDevice();
  void CompleteWrite();

  StrictMock<MockUSB> usb_mock;
  StrictMock<MockStreamDecoder> stream_decoder_mock;

  // The value here doesn't matter, we just need a pointer to represent the
  // device.
  USB_DEVICE_HANDLE dummy_device = 0;

  // Holds a pointer to the USBTransport_EventHandler function once
  // ConfigureDevice() has run
  USBEventHandler m_event_handler_fn = nullptr;
};

/*
 * @brief Put the USB Device into configured mode.
 * @param event_handler, the captured USBEventHandler that can later be used to
 *   trigger events.
 */
void USBTransportTest::ConfigureDevice() {
  EXPECT_CALL(usb_mock, Open(_, DRV_IO_INTENT_READWRITE))
      .WillOnce(Return(dummy_device));
  EXPECT_CALL(usb_mock,
      EventHandlerSet(dummy_device, _, 0))
      .WillOnce(SaveArg<1>(&m_event_handler_fn));

  USBTransport_Tasks();
  ASSERT_THAT(m_event_handler_fn, NotNull());

  // Send a USB_DEVICE_EVENT_CONFIGURED event.
  uint8_t configurationValue = 1;
  m_event_handler_fn(USB_DEVICE_EVENT_CONFIGURED,
                     reinterpret_cast<void*>(&configurationValue), 0);
}

/*
 * @brief Trigger a write-complete event.
 * @param event_handler The USBEventHandler to use to trigger the event.
 */
void USBTransportTest::CompleteWrite() {
  ASSERT_THAT(m_event_handler_fn, NotNull());
  uint8_t configurationValue = 0;
  m_event_handler_fn(USB_DEVICE_EVENT_ENDPOINT_WRITE_COMPLETE,
                     reinterpret_cast<void*>(&configurationValue), 0);
}

/*
 * Check an uninitialized transport doesn't send anything.
 */
TEST_F(USBTransportTest, uninitialized) {
  // Even though we ca;;t USBTransport_Initialize() here, since we haven't
  // called USBTransport_Tasks() the transport remains in an uninitialized
  // state.
  USBTransport_Initialize(nullptr);
  EXPECT_FALSE(USBTransport_SendResponse(ECHO, RC_OK, NULL, 0));
}

/*
 * Check sending messages to the Host works.
 */
TEST_F(USBTransportTest, sendResponse) {
  USBTransport_Initialize(StreamDecoder_Process);

  // Try with a unconfigured transport.
  EXPECT_FALSE(USBTransport_SendResponse(ECHO, RC_OK, NULL, 0));

  // Now configure the device and clear the logging bit.
  ConfigureDevice();
  Logger_SetDataPendingFlag(false);

  // Test a message with no data.
  const uint8_t expected_message[] = {
    0x5a, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa5
  };

  EXPECT_CALL(
      usb_mock,
      EndpointWrite(dummy_device, _, 0x81, _, _,
                    USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE))
      .With(Args<3, 4>(DataIs(expected_message, arraysize(expected_message))))
      .WillOnce(Return(USB_DEVICE_RESULT_OK));

  EXPECT_TRUE(USBTransport_SendResponse(ECHO, RC_OK, NULL, 0));
  EXPECT_TRUE(USBTransport_WritePending());

  CompleteWrite();
  EXPECT_FALSE(USBTransport_WritePending());
}

TEST_F(USBTransportTest, doubleSendResponse) {
  USBTransport_Initialize(StreamDecoder_Process);
  ConfigureDevice();
  Logger_SetDataPendingFlag(false);

  const uint8_t expected_message[] = {
    0x5a, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa5
  };

  EXPECT_CALL(
      usb_mock,
      EndpointWrite(dummy_device, _, 0x81, _, _,
                    USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE))
      .With(Args<3, 4>(DataIs(expected_message, arraysize(expected_message))))
      .WillOnce(Return(USB_DEVICE_RESULT_OK));

  EXPECT_TRUE(USBTransport_SendResponse(ECHO, RC_OK, NULL, 0));
  // Try to send a second message while the first is pending.
  EXPECT_FALSE(USBTransport_SendResponse(ECHO, RC_OK, NULL, 0));
  EXPECT_TRUE(USBTransport_WritePending());

  CompleteWrite();
  EXPECT_FALSE(USBTransport_WritePending());
}

TEST_F(USBTransportTest, sendResponseWithData) {
  USBTransport_Initialize(StreamDecoder_Process);
  ConfigureDevice();
  Logger_SetDataPendingFlag(true);

  const uint8_t chunk1[] = {1, 2};
  const uint8_t chunk2[] = {3, 4};

  IOVec iovec[2] = {
      { reinterpret_cast<const void*>(&chunk1), arraysize(chunk1) },
      { reinterpret_cast<const void*>(&chunk2), arraysize(chunk2) }
  };

  const uint8_t expected_message[] = {
    0x5a, 0x80, 0x00, 0x04, 0x00, 0x00, 0x01,
    1, 2, 3, 4, 0xa5
  };

  EXPECT_CALL(
      usb_mock,
      EndpointWrite(dummy_device, _, 0x81, _, _,
                    USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE))
      .With(Args<3, 4>(DataIs(expected_message, arraysize(expected_message))))
      .WillOnce(Return(USB_DEVICE_RESULT_OK));

  EXPECT_TRUE(USBTransport_SendResponse(ECHO, RC_OK, iovec, arraysize(iovec)));
  EXPECT_TRUE(USBTransport_WritePending());

  CompleteWrite();
  EXPECT_FALSE(USBTransport_WritePending());
}

TEST_F(USBTransportTest, sendError) {
  USBTransport_Initialize(StreamDecoder_Process);
  ConfigureDevice();
  Logger_SetDataPendingFlag(false);

  EXPECT_CALL(
      usb_mock,
      EndpointWrite(dummy_device, _, 0x81, _, _,
                    USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE))
      .WillOnce(Return(USB_DEVICE_RESULT_ERROR));

  EXPECT_FALSE(USBTransport_SendResponse(ECHO, RC_OK, NULL, 0));
  EXPECT_FALSE(USBTransport_WritePending());
}

TEST_F(USBTransportTest, truncateResponse) {
  USBTransport_Initialize(StreamDecoder_Process);
  ConfigureDevice();
  Logger_SetDataPendingFlag(false);

  // Send a lot of data, and make sure we set the truncated bit.
  const unsigned int big_payload_size = PAYLOAD_SIZE + 100;
  uint8_t large_payload[big_payload_size];  // NOLINT(runtime/arrays)
  memset(large_payload, 0, arraysize(large_payload));

  IOVec iovec { large_payload, big_payload_size};

  uint8_t expected_message[7 + PAYLOAD_SIZE + 1];
  memset(expected_message, 0, arraysize(expected_message));
  expected_message[0] = 0x5a;
  expected_message[1] = 0x80;
  expected_message[2] = 0x0;
  expected_message[3] = 0x01;
  expected_message[4] = 0x02;
  expected_message[5] = RC_OK;
  expected_message[6] = 0x04;  // flags, truncated.
  expected_message[7 + PAYLOAD_SIZE] = 0xa5;

  EXPECT_CALL(
      usb_mock,
      EndpointWrite(dummy_device, _, 0x81, _, _,
                    USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE))
      .With(Args<3, 4>(DataIs(expected_message, arraysize(expected_message))))
      .WillOnce(Return(USB_DEVICE_RESULT_OK));

  EXPECT_TRUE(USBTransport_SendResponse(ECHO, RC_OK, &iovec, 1));
  EXPECT_TRUE(USBTransport_WritePending());

  CompleteWrite();
  EXPECT_FALSE(USBTransport_WritePending());
}

TEST_F(USBTransportTest, pendingFlags) {
  USBTransport_Initialize(StreamDecoder_Process);
  ConfigureDevice();
  Logger_SetDataPendingFlag(false);

  Flags_SetTXDrop();
  EXPECT_TRUE(Flags_HasChanged());

  const uint8_t expected_message[] = {
    0x5a, 0x80, 0x00, 0x00, 0x00, 0x00, 0x02, 0xa5
  };

  EXPECT_CALL(
      usb_mock,
      EndpointWrite(dummy_device, _, 0x81, _, _,
                    USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE))
      .With(Args<3, 4>(DataIs(expected_message, arraysize(expected_message))))
      .WillOnce(Return(USB_DEVICE_RESULT_OK));

  EXPECT_TRUE(USBTransport_SendResponse(ECHO, RC_OK, NULL, 0));
  EXPECT_TRUE(USBTransport_WritePending());
  CompleteWrite();
  EXPECT_FALSE(USBTransport_WritePending());
}
