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
#include "BootloaderOptionsMock.h"
#include "Matchers.h"
#include "ResetMock.h"
#include "StreamDecoderMock.h"
#include "flags.h"
#include "usb_device_mock.h"
#include "usb_transport.h"

using ::testing::Args;
using ::testing::InSequence;
using ::testing::Mock;
using ::testing::NotNull;
using ::testing::Pointee;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::StrictMock;
using ::testing::_;

typedef void (*USBEventHandler)(USB_DEVICE_EVENT, void*, uintptr_t);

class USBTransportTest : public testing::Test {
 public:
  void SetUp() {
    USBDevice_SetMock(&m_usb_mock);
    StreamDecoder_SetMock(&m_stream_decoder_mock);
    BootloaderOptions_SetMock(&m_bootloader_options_mock);
    Reset_SetMock(&m_reset_mock);
  }

  void TearDown() {
    USBDevice_SetMock(nullptr);
    StreamDecoder_SetMock(nullptr);
    BootloaderOptions_SetMock(nullptr);
    Reset_SetMock(nullptr);
  }

  void ConfigureDevice();
  void CompleteWrite();

 protected:
  StrictMock<MockUSBDevice> m_usb_mock;
  StrictMock<MockStreamDecoder> m_stream_decoder_mock;
  StrictMock<MockBootloaderOptions> m_bootloader_options_mock;
  StrictMock<MockReset> m_reset_mock;

  // The value here doesn't matter, we just need a pointer to represent the
  // device.
  USB_DEVICE_HANDLE m_usb_handle = 0;

  // Holds a pointer to the USBTransport_EventHandler function once
  // ConfigureDevice() has run
  USBEventHandler m_event_handler = nullptr;

  static const uint8_t kToken = 99;
};

/*
 * @brief Put the USB Device into configured mode.
 * @param event_handler, the captured USBEventHandler that can later be used to
 *   trigger events.
 */
void USBTransportTest::ConfigureDevice() {
  EXPECT_CALL(m_usb_mock, Open(_, DRV_IO_INTENT_READWRITE))
      .WillOnce(Return(m_usb_handle));
  EXPECT_CALL(m_usb_mock,
      EventHandlerSet(m_usb_handle, _, 0))
      .WillOnce(SaveArg<1>(&m_event_handler));
  EXPECT_CALL(m_usb_mock, Attach(m_usb_handle)).Times(1);
  EXPECT_CALL(m_usb_mock, ActiveSpeedGet(m_usb_handle))
    .WillOnce(Return(USB_SPEED_FULL));
  EXPECT_CALL(m_usb_mock, EndpointIsEnabled(m_usb_handle, 1))
    .WillOnce(Return(false));
  EXPECT_CALL(m_usb_mock,
              EndpointEnable(m_usb_handle, 0, 1, USB_TRANSFER_TYPE_BULK, 64))
    .WillOnce(Return(USB_DEVICE_RESULT_OK));
  EXPECT_CALL(m_usb_mock, EndpointIsEnabled(m_usb_handle, 0x81))
    .WillOnce(Return(false));
  EXPECT_CALL(m_usb_mock,
              EndpointEnable(m_usb_handle, 0, 0x81, USB_TRANSFER_TYPE_BULK, 64))
    .WillOnce(Return(USB_DEVICE_RESULT_OK));
  EXPECT_CALL(m_usb_mock, EndpointRead(m_usb_handle, _, 1, _, _))
    .WillOnce(Return(USB_DEVICE_RESULT_OK));

  USBTransport_Tasks();
  ASSERT_THAT(m_event_handler, NotNull());

  m_event_handler(USB_DEVICE_EVENT_POWER_DETECTED, nullptr, 0u);

  // Send a USB_DEVICE_EVENT_CONFIGURED event.
  uint8_t configurationValue = 1;
  m_event_handler(USB_DEVICE_EVENT_CONFIGURED,
                  reinterpret_cast<void*>(&configurationValue), 0);

  USBTransport_Tasks();

  Mock::VerifyAndClearExpectations(&m_usb_mock);
}

/*
 * @brief Trigger a write-complete event.
 * @param event_handler The USBEventHandler to use to trigger the event.
 */
void USBTransportTest::CompleteWrite() {
  ASSERT_THAT(m_event_handler, NotNull());
  uint8_t configurationValue = 0;
  m_event_handler(USB_DEVICE_EVENT_ENDPOINT_WRITE_COMPLETE,
                  reinterpret_cast<void*>(&configurationValue), 0);
}

/*
 * Check an uninitialized transport doesn't send anything.
 */
TEST_F(USBTransportTest, uninitialized) {
  // Even though we call USBTransport_Initialize() here, since we haven't
  // called USBTransport_Tasks() the transport remains in an uninitialized
  // state.
  USBTransport_Initialize(nullptr);
  EXPECT_FALSE(USBTransport_SendResponse(kToken, COMMAND_ECHO, RC_OK, NULL, 0));
}


TEST_F(USBTransportTest, usbLifecycle) {
  USB_DEVICE_EVENT_HANDLER event_handler;

  InSequence seq;
  EXPECT_CALL(m_usb_mock, Open(USB_DEVICE_INDEX_0, DRV_IO_INTENT_READWRITE))
    .WillOnce(Return(USB_DEVICE_HANDLE_INVALID));
  EXPECT_CALL(m_usb_mock, Open(USB_DEVICE_INDEX_0, DRV_IO_INTENT_READWRITE))
    .WillOnce(Return(m_usb_handle));
  EXPECT_CALL(m_usb_mock, EventHandlerSet(m_usb_handle, _, 0u))
    .WillOnce(SaveArg<1>(&event_handler));
  EXPECT_CALL(m_usb_mock, Attach(m_usb_handle)).Times(1);
  EXPECT_CALL(m_usb_mock, ActiveSpeedGet(m_usb_handle))
    .WillOnce(Return(USB_SPEED_FULL));
  EXPECT_CALL(m_usb_mock, EndpointIsEnabled(m_usb_handle, 1))
    .WillOnce(Return(false));
  EXPECT_CALL(m_usb_mock,
              EndpointEnable(m_usb_handle, 0, 1, USB_TRANSFER_TYPE_BULK, 64))
    .WillOnce(Return(USB_DEVICE_RESULT_OK));
  EXPECT_CALL(m_usb_mock, EndpointIsEnabled(m_usb_handle, 0x81))
    .WillOnce(Return(false));
  EXPECT_CALL(m_usb_mock,
              EndpointEnable(m_usb_handle, 0, 0x81, USB_TRANSFER_TYPE_BULK, 64))
    .WillOnce(Return(USB_DEVICE_RESULT_OK));
  EXPECT_CALL(m_usb_mock, EndpointRead(m_usb_handle, _, 1, _, _))
    .WillOnce(Return(USB_DEVICE_RESULT_OK));

  USBTransport_Initialize(nullptr);
  EXPECT_FALSE(USBTransport_IsConfigured());

  // First call the USB stack isn't ready yet
  USBTransport_Tasks();
  EXPECT_FALSE(USBTransport_IsConfigured());

  // Now it's ready
  USBTransport_Tasks();
  EXPECT_FALSE(USBTransport_IsConfigured());

  // Power event, this causes the attach
  event_handler(USB_DEVICE_EVENT_POWER_DETECTED, nullptr, 0u);
  EXPECT_FALSE(USBTransport_IsConfigured());
  USBTransport_Tasks();
  EXPECT_FALSE(USBTransport_IsConfigured());

  // Device configured
  uint8_t configuration = 1u;
  event_handler(USB_DEVICE_EVENT_CONFIGURED, &configuration, 0u);

  USBTransport_Tasks();
  EXPECT_TRUE(USBTransport_IsConfigured());

  // Check the handler matches
  EXPECT_EQ(m_usb_handle, USBTransport_GetHandle());

  // Loss of power event
  EXPECT_CALL(m_usb_mock, Detach(m_usb_handle))
    .Times(1);
  EXPECT_CALL(m_usb_mock, EndpointIsEnabled(m_usb_handle, 0x81))
    .WillOnce(Return(true));
  EXPECT_CALL(m_usb_mock, EndpointDisable(m_usb_handle,  0x81))
    .WillOnce(Return(USB_DEVICE_RESULT_OK));
  EXPECT_CALL(m_usb_mock, EndpointIsEnabled(m_usb_handle, 1))
    .WillOnce(Return(true));
  EXPECT_CALL(m_usb_mock, EndpointDisable(m_usb_handle, 1))
    .WillOnce(Return(USB_DEVICE_RESULT_OK));

  event_handler(USB_DEVICE_EVENT_POWER_REMOVED, nullptr, 0u);
  USBTransport_Tasks();
  EXPECT_FALSE(USBTransport_IsConfigured());
}

TEST_F(USBTransportTest, alternateSettings) {
  USBTransport_Initialize(nullptr);
  ConfigureDevice();

  // Get alt settings
  const uint8_t alt_interface[] = { 0 };
  EXPECT_CALL(m_usb_mock, ControlSend(m_usb_handle, _, _))
    .With(Args<1, 2>(DataIs(alt_interface, arraysize(alt_interface))))
    .WillOnce(Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS));

  USB_SETUP_PACKET get_interface_request;
  get_interface_request.bRequest = USB_REQUEST_GET_INTERFACE;
  m_event_handler(USB_DEVICE_EVENT_CONTROL_TRANSFER_SETUP_REQUEST,
                  reinterpret_cast<void*>(&get_interface_request),
                  sizeof(get_interface_request));

  // Try to set an invalid settings
  EXPECT_CALL(m_usb_mock,
              ControlStatus(m_usb_handle, USB_DEVICE_CONTROL_STATUS_ERROR))
    .WillOnce(Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS));

  USB_SETUP_PACKET set_interface_request;
  set_interface_request.bRequest = USB_REQUEST_SET_INTERFACE;
  set_interface_request.wValue = 1u;
  m_event_handler(USB_DEVICE_EVENT_CONTROL_TRANSFER_SETUP_REQUEST,
                  reinterpret_cast<void*>(&set_interface_request),
                  sizeof(set_interface_request));

  // Try to set the correct setting
  EXPECT_CALL(m_usb_mock,
              ControlStatus(m_usb_handle, USB_DEVICE_CONTROL_STATUS_OK))
    .WillOnce(Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS));

  set_interface_request.wValue = 0u;
  m_event_handler(USB_DEVICE_EVENT_CONTROL_TRANSFER_SETUP_REQUEST,
                  reinterpret_cast<void*>(&set_interface_request),
                  sizeof(set_interface_request));
}

TEST_F(USBTransportTest, dfuGetStatus) {
  USBTransport_Initialize(nullptr);
  ConfigureDevice();

  // Response is all 0s.
  const uint8_t get_status_res[6] = {};

  EXPECT_CALL(m_usb_mock, ControlSend(m_usb_handle, _, _))
    .With(Args<1, 2>(DataIs(get_status_res, arraysize(get_status_res))))
    .WillOnce(Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS));

  USB_SETUP_PACKET get_status_req;
  get_status_req.bmRequestType = 0xa1;
  get_status_req.bRequest = 3u;  // DFU_GET_STATUS
  get_status_req.wValue = 0;
  get_status_req.wIndex = 3u;  // dfu interface
  get_status_req.wLength = 6u;  // expected length

  m_event_handler(USB_DEVICE_EVENT_CONTROL_TRANSFER_SETUP_REQUEST,
                  reinterpret_cast<void*>(&get_status_req),
                  sizeof(get_status_req));
}

TEST_F(USBTransportTest, dfuDetach) {
  USBTransport_Initialize(nullptr);
  ConfigureDevice();

  EXPECT_CALL(m_usb_mock,
              ControlStatus(m_usb_handle, USB_DEVICE_CONTROL_STATUS_OK))
    .WillOnce(Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS));
  EXPECT_CALL(m_bootloader_options_mock, SetBootOption(BOOT_BOOTLOADER))
    .Times(1);
  EXPECT_CALL(m_reset_mock, SoftReset()).Times(1);

  USB_SETUP_PACKET get_status_req;
  get_status_req.bmRequestType = 0x21;
  get_status_req.bRequest = 0u;  // DFU_DETACH
  get_status_req.wValue = 0;
  get_status_req.wIndex = 3u;  // dfu interface
  get_status_req.wLength = 0u;  // expected length

  m_event_handler(USB_DEVICE_EVENT_CONTROL_TRANSFER_SETUP_REQUEST,
                  reinterpret_cast<void*>(&get_status_req),
                  sizeof(get_status_req));

  USBTransport_Tasks();
}

/*
 * Check sending messages to the Host works.
 */
TEST_F(USBTransportTest, sendResponse) {
  USBTransport_Initialize(StreamDecoder_Process);

  // Try with a unconfigured transport.
  EXPECT_FALSE(USBTransport_SendResponse(kToken, COMMAND_ECHO, RC_OK, NULL, 0));

  // Now configure the device and clear the logging bit.
  ConfigureDevice();

  // Test a message with no data.
  const uint8_t expected_message[] = {
    0x5a, kToken, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa5
  };

  EXPECT_CALL(
      m_usb_mock,
      EndpointWrite(m_usb_handle, _, 0x81, _, _,
                    USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE))
      .With(Args<3, 4>(DataIs(expected_message, arraysize(expected_message))))
      .WillOnce(Return(USB_DEVICE_RESULT_OK));

  EXPECT_TRUE(USBTransport_SendResponse(kToken, COMMAND_ECHO, RC_OK, NULL, 0));
  EXPECT_TRUE(USBTransport_WritePending());

  CompleteWrite();
  EXPECT_FALSE(USBTransport_WritePending());
}

TEST_F(USBTransportTest, doubleSendResponse) {
  USBTransport_Initialize(StreamDecoder_Process);
  ConfigureDevice();

  const uint8_t expected_message[] = {
    0x5a, kToken, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa5
  };

  EXPECT_CALL(
      m_usb_mock,
      EndpointWrite(m_usb_handle, _, 0x81, _, _,
                    USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE))
      .With(Args<3, 4>(DataIs(expected_message, arraysize(expected_message))))
      .WillOnce(Return(USB_DEVICE_RESULT_OK));

  EXPECT_TRUE(USBTransport_SendResponse(kToken, COMMAND_ECHO, RC_OK, NULL, 0));
  // Try to send a second message while the first is pending.
  EXPECT_FALSE(
      USBTransport_SendResponse(kToken + 1, COMMAND_ECHO, RC_OK, NULL, 0));
  EXPECT_TRUE(USBTransport_WritePending());

  CompleteWrite();
  EXPECT_FALSE(USBTransport_WritePending());
}

TEST_F(USBTransportTest, sendResponseWithData) {
  USBTransport_Initialize(StreamDecoder_Process);
  ConfigureDevice();

  const uint8_t chunk1[] = {1, 2, 3, 4, 5, 6, 7, 8};
  const uint8_t chunk2[] = {9, 0, 1, 2, 3, 4, 5, 6, 7, 8};

  IOVec iovec[2] = {
      { reinterpret_cast<const void*>(&chunk1), arraysize(chunk1) },
      { reinterpret_cast<const void*>(&chunk2), arraysize(chunk2) }
  };

  const uint8_t expected_message[] = {
    0x5a, kToken, 0xf0, 0x00, 0x12, 0x00, 0x00, 0x00,
    1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8,
    0xa5
  };

  EXPECT_CALL(
      m_usb_mock,
      EndpointWrite(m_usb_handle, _, 0x81, _, _,
                    USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE))
      .With(Args<3, 4>(DataIs(expected_message, arraysize(expected_message))))
      .WillOnce(Return(USB_DEVICE_RESULT_OK));

  EXPECT_TRUE(USBTransport_SendResponse(kToken, COMMAND_ECHO, RC_OK, iovec,
                                        arraysize(iovec)));
  EXPECT_TRUE(USBTransport_WritePending());

  CompleteWrite();
  EXPECT_FALSE(USBTransport_WritePending());
}

TEST_F(USBTransportTest, sendError) {
  USBTransport_Initialize(StreamDecoder_Process);
  ConfigureDevice();

  EXPECT_CALL(
      m_usb_mock,
      EndpointWrite(m_usb_handle, _, 0x81, _, _,
                    USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE))
      .WillOnce(Return(USB_DEVICE_RESULT_ERROR));

  EXPECT_FALSE(USBTransport_SendResponse(kToken, COMMAND_ECHO, RC_OK, NULL, 0));
  EXPECT_FALSE(USBTransport_WritePending());
}

TEST_F(USBTransportTest, truncateResponse) {
  USBTransport_Initialize(StreamDecoder_Process);
  ConfigureDevice();

  // Send a lot of data, and make sure we set the truncated bit.
  const unsigned int big_payload_size = PAYLOAD_SIZE + 100;
  uint8_t large_payload[big_payload_size];  // NOLINT(runtime/arrays)
  memset(large_payload, 0, arraysize(large_payload));

  IOVec iovec { large_payload, big_payload_size};

  uint8_t expected_message[8 + PAYLOAD_SIZE + 1];
  memset(expected_message, 0, arraysize(expected_message));
  expected_message[0] = 0x5a;
  expected_message[1] = kToken;
  expected_message[2] = 0xf0;
  expected_message[3] = 0x0;
  expected_message[4] = 0x01;
  expected_message[5] = 0x02;
  expected_message[6] = RC_OK;
  expected_message[7] = 0x04;  // flags, truncated.
  expected_message[8 + PAYLOAD_SIZE] = 0xa5;

  EXPECT_CALL(
      m_usb_mock,
      EndpointWrite(m_usb_handle, _, 0x81, _, _,
                    USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE))
      .With(Args<3, 4>(DataIs(expected_message, arraysize(expected_message))))
      .WillOnce(Return(USB_DEVICE_RESULT_OK));

  EXPECT_TRUE(
      USBTransport_SendResponse(kToken, COMMAND_ECHO, RC_OK, &iovec, 1));
  EXPECT_TRUE(USBTransport_WritePending());

  CompleteWrite();
  EXPECT_FALSE(USBTransport_WritePending());
}

TEST_F(USBTransportTest, pendingFlags) {
  USBTransport_Initialize(StreamDecoder_Process);
  ConfigureDevice();

  Flags_SetTXDrop();
  EXPECT_TRUE(Flags_HasChanged());

  const uint8_t expected_message[] = {
    0x5a, kToken, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x02, 0xa5
  };

  EXPECT_CALL(
      m_usb_mock,
      EndpointWrite(m_usb_handle, _, 0x81, _, _,
                    USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE))
      .With(Args<3, 4>(DataIs(expected_message, arraysize(expected_message))))
      .WillOnce(Return(USB_DEVICE_RESULT_OK));

  EXPECT_TRUE(USBTransport_SendResponse(kToken, COMMAND_ECHO, RC_OK, NULL, 0));
  EXPECT_TRUE(USBTransport_WritePending());
  CompleteWrite();
  EXPECT_FALSE(USBTransport_WritePending());
}
