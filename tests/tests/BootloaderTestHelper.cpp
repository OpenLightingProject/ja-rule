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
 * BootloaderTestHelper.cpp
 * Helpers for the DFU bootloader tests.
 * Copyright (C) 2015 Simon Newton
 */

#include "BootloaderTestHelper.h"

#include <gtest/gtest.h>

#include "Matchers.h"
#include "bootloader.h"
#include "dfu_spec.h"
#include "usb/usb_device.h"
#include "usb_device_mock.h"

using testing::DoAll;
using testing::NotNull;
using testing::Mock;
using testing::Return;
using testing::SaveArg;
using testing::WithArgs;
using testing::_;

::std::ostream& operator<<(::std::ostream& os, const DFUState& state) {
  switch (state) {
    case APP_STATE_IDLE:
      return os << "APP_STATE_IDLE(0)";
    case APP_STATE_DETACH:
      return os << "APP_STATE_DETACH(1)";
    case DFU_STATE_IDLE:
      return os << "DFU_STATE_IDLE(2)";
    case DFU_STATE_DNLOAD_SYNC:
      return os << "DFU_STATE_DNLOAD_SYNC(3)";
    case DFU_STATE_DNBUSY:
      return os << "DFU_STATE_DNBUSY(4)";
    case DFU_STATE_DNLOAD_IDLE:
      return os << "DFU_STATE_DNLOAD_IDLE(5)";
    case DFU_STATE_MANIFEST_SYNC:
      return os << "DFU_STATE_MANIFEST_SYNC(6)";
    case DFU_STATE_MANIFEST:
      return os << "DFU_STATE_MANIFEST(7)";
    case DFU_STATE_MANIFEST_WAIT_RESET:
      return os << "DFU_STATE_MANIFEST_WAIT_RESET(8)";
    case DFU_STATE_UPLOAD_IDLE:
      return os << "DFU_STATE_UPLOAD_IDLE(9)";
    case DFU_STATE_ERROR:
      return os << "DFU_STATE_ERROR(10)";
    default:
      return os << "UNKNOWN(" << state << ")";
  }
}

::std::ostream& operator<<(::std::ostream& os, const DFUStatus& status) {
  switch (status) {
    case DFU_STATUS_OK:
      return os << "DFU_STATUS_OK(0x00)";
    case DFU_STATUS_ERR_TARGET:
      return os << "DFU_STATUS_ERR_TARGET(0x01)";
    case DFU_STATUS_ERR_FILE:
      return os << "DFU_STATUS_ERR_FILE(0x02)";
    case DFU_STATUS_ERR_WRITE:
      return os << "DFU_STATUS_ERR_WRITE(0x03)";
    case DFU_STATUS_ERR_ERASE:
      return os << "DFU_STATUS_ERR_ERASE(0x04)";
    case DFU_STATUS_ERR_CHECK_ERASED:
      return os << "DFU_STATUS_ERR_CHECK_ERASED(0x05)";
    case DFU_STATUS_ERR_PROG:
      return os << "DFU_STATUS_ERR_PROG(0x06)";
    case DFU_STATUS_ERR_VERIFY:
      return os << "DFU_STATUS_ERR_VERIFY(0x07)";
    case DFU_STATUS_ERR_ADDRESS:
      return os << "DFU_STATUS_ERR_ADDRESS(0x08)";
    case DFU_STATUS_ERR_NOT_DONE:
      return os << "DFU_STATUS_ERR_NOT_DONE(0x09)";
    case DFU_STATUS_ERR_FIRMWARE:
      return os << "DFU_STATUS_ERR_FIRMWARE(0x0a)";
    case DFU_STATUS_ERR_VENDOR:
      return os << "DFU_STATUS_ERR_VENDOR(0x0b)";
    case DFU_STATUS_ERR_USBR:
      return os << "DFU_STATUS_ERR_USBR(0x0c)";
    case DFU_STATUS_ERR_POR:
      return os << "DFU_STATUS_ERR_POR(0x0d)";
    case DFU_STATUS_ERR_UNKNOWN:
      return os << "DFU_STATUS_ERR_UNKNOWN(0x0e)";
    case DFU_STATUS_ERR_STALLED_PKT:
      return os << "DFU_STATUS_ERR_STALLED_PKT(0x0f)";
    default:
      return os << "UNKNOWN(" << status << ")";
  }
}

USBHost::USBHost(MockUSBDevice *mock_usb)
    : m_usb_mock(mock_usb),
      m_usb_handle(0x123456),
      m_event_handler(nullptr) {
}

void USBHost::InitDevice() {
  Mock::VerifyAndClearExpectations(m_usb_mock);

  EXPECT_CALL(*m_usb_mock, Open(USB_DEVICE_INDEX_0, DRV_IO_INTENT_READWRITE))
    .WillOnce(Return(m_usb_handle));
  EXPECT_CALL(*m_usb_mock, EventHandlerSet(m_usb_handle, _, 0u))
    .WillOnce(SaveArg<1>(&m_event_handler));
  EXPECT_CALL(*m_usb_mock, Attach(m_usb_handle));

  Bootloader_Initialize();
  Bootloader_Tasks();

  ASSERT_THAT(m_event_handler, NotNull());

  m_event_handler(USB_DEVICE_EVENT_POWER_DETECTED, nullptr, 0u);
  uint8_t configuration = 1u;
  m_event_handler(USB_DEVICE_EVENT_CONFIGURED, &configuration, 0u);
  Bootloader_Tasks();

  Mock::VerifyAndClearExpectations(m_usb_mock);
}

void USBHost::SendUSBReset() {
  m_event_handler(USB_DEVICE_EVENT_RESET, nullptr, 0u);
}

void USBHost::SendDeconfigure() {
  m_event_handler(USB_DEVICE_EVENT_DECONFIGURED, nullptr, 0u);
}

void USBHost::SendPowerLoss() {
  m_event_handler(USB_DEVICE_EVENT_POWER_REMOVED, nullptr, 0u);
}

void USBHost::SetAlternateInterface(uint16_t alt_setting) {
  Mock::VerifyAndClearExpectations(m_usb_mock);

  EXPECT_CALL(*m_usb_mock,
              ControlStatus(m_usb_handle, USB_DEVICE_CONTROL_STATUS_OK))
    .WillOnce(Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS));

  USB_SETUP_PACKET set_interface_request;
  set_interface_request.bRequest = USB_REQUEST_SET_INTERFACE;
  set_interface_request.wValue = alt_setting;

  SetupRequest(&set_interface_request, sizeof(set_interface_request));
  Mock::VerifyAndClearExpectations(m_usb_mock);
}

void USBHost::GetDFUState(Outcome outcome, uint8_t *state) {
  Mock::VerifyAndClearExpectations(m_usb_mock);

  if (outcome == OUTCOME_OK) {
    EXPECT_CALL(*m_usb_mock,
                ControlSend(m_usb_handle, _, sizeof(uint8_t)))
        .WillOnce(DoAll(
              WithArgs<1, 2>(CopyDataTo(state, sizeof(uint8_t))),
              Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS)));
  } else {
    EXPECT_CALL(*m_usb_mock,
                ControlStatus(m_usb_handle, USB_DEVICE_CONTROL_STATUS_ERROR))
      .WillOnce(Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS));
  }

  USB_SETUP_PACKET packet;
  packet.bmRequestType = 0xa1;
  packet.bRequest = DFU_GETSTATE;
  packet.wValue = 0;
  packet.wIndex = INTERFACE;
  packet.wLength = 1;

  SetupRequest(&packet, sizeof(packet));
  Mock::VerifyAndClearExpectations(m_usb_mock);
}

void USBHost::GetDFUStatus(DFUState *state, DFUStatus *status) {
  Mock::VerifyAndClearExpectations(m_usb_mock);

  uint8_t status_response[GET_STATUS_RESPONSE_SIZE];

  EXPECT_CALL(*m_usb_mock,
              ControlSend(m_usb_handle, _, GET_STATUS_RESPONSE_SIZE))
      .WillOnce(DoAll(
            WithArgs<1, 2>(CopyDataTo(status_response,
                           GET_STATUS_RESPONSE_SIZE)),
            Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS)));

  USB_SETUP_PACKET packet;
  packet.bmRequestType = 0xa1;
  packet.bRequest = DFU_GETSTATUS;
  packet.wValue = 0;
  packet.wIndex = INTERFACE;
  packet.wLength = GET_STATUS_RESPONSE_SIZE;
  SetupRequest(&packet, sizeof(packet));

  Mock::VerifyAndClearExpectations(m_usb_mock);
  *state = static_cast<DFUState>(status_response[4]);
  *status = static_cast<DFUStatus>(status_response[0]);
}

void USBHost::DFUDownload(DownloadOutcome outcome,
                          uint16_t block_index,
                          const uint8_t *data, uint16_t size) {
  Mock::VerifyAndClearExpectations(m_usb_mock);

  switch (outcome) {
    case DOWNLOAD_OUTCOME_STALL:
      EXPECT_CALL(*m_usb_mock, ControlStatus(m_usb_handle,
                                             USB_DEVICE_CONTROL_STATUS_ERROR))
        .WillOnce(Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS));
    break;
    case DOWNLOAD_OUTCOME_OK:
      EXPECT_CALL(*m_usb_mock,
                  ControlStatus(m_usb_handle, USB_DEVICE_CONTROL_STATUS_OK))
        .WillOnce(Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS));
      break;
    case DOWNLOAD_OUTCOME_RECEIVE:
      EXPECT_CALL(*m_usb_mock,
                  ControlReceive(m_usb_handle, _, size))
        .WillOnce(DoAll(
            WithArgs<1, 2>(CopyDataFrom(data, size)),
            Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS)));
      EXPECT_CALL(*m_usb_mock,
                  ControlStatus(m_usb_handle, USB_DEVICE_CONTROL_STATUS_OK))
        .WillOnce(Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS));
      break;
    default:
      {}
  }

  USB_SETUP_PACKET packet;
  packet.bmRequestType = 0x21;
  packet.bRequest = DFU_DNLOAD;
  packet.wValue = block_index;
  packet.wIndex = INTERFACE;
  packet.wLength = size;
  SetupRequest(&packet, sizeof(packet));
  if (outcome == DOWNLOAD_OUTCOME_RECEIVE) {
    m_event_handler(USB_DEVICE_EVENT_CONTROL_TRANSFER_DATA_RECEIVED,
                    nullptr, 0);
  }

  Mock::VerifyAndClearExpectations(m_usb_mock);
}

void USBHost::DFUDownloadAndAbort(uint16_t block_index, uint16_t size) {
  Mock::VerifyAndClearExpectations(m_usb_mock);

  EXPECT_CALL(*m_usb_mock,
              ControlReceive(m_usb_handle, _, size))
    .WillOnce(Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS));
  EXPECT_CALL(*m_usb_mock,
              ControlStatus(m_usb_handle, USB_DEVICE_CONTROL_STATUS_ERROR))
    .WillOnce(Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS));

  USB_SETUP_PACKET packet;
  packet.bmRequestType = 0x21;
  packet.bRequest = DFU_DNLOAD;
  packet.wValue = block_index;
  packet.wIndex = INTERFACE;
  packet.wLength = size;
  SetupRequest(&packet, sizeof(packet));
  m_event_handler(USB_DEVICE_EVENT_CONTROL_TRANSFER_ABORTED, nullptr, 0);

  Mock::VerifyAndClearExpectations(m_usb_mock);
}

void USBHost::DFUAbort(Outcome outcome) {
  Mock::VerifyAndClearExpectations(m_usb_mock);

  if (outcome == OUTCOME_OK) {
    EXPECT_CALL(*m_usb_mock,
                ControlStatus(m_usb_handle, USB_DEVICE_CONTROL_STATUS_OK))
      .WillOnce(Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS));
  } else {
    EXPECT_CALL(*m_usb_mock,
                ControlStatus(m_usb_handle, USB_DEVICE_CONTROL_STATUS_ERROR))
      .WillOnce(Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS));
  }

  USB_SETUP_PACKET packet;
  packet.bmRequestType = 0x21;
  packet.bRequest = DFU_ABORT;
  packet.wValue = 0;
  packet.wIndex = INTERFACE;
  packet.wLength = 0;

  SetupRequest(&packet, sizeof(packet));
  Mock::VerifyAndClearExpectations(m_usb_mock);
}

void USBHost::DFUClearStatus() {
  Mock::VerifyAndClearExpectations(m_usb_mock);

  EXPECT_CALL(*m_usb_mock,
              ControlStatus(m_usb_handle, USB_DEVICE_CONTROL_STATUS_OK))
    .WillOnce(Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS));

  USB_SETUP_PACKET packet;
  packet.bmRequestType = 0x21;
  packet.bRequest = DFU_CLRSTATUS;
  packet.wValue = 0;
  packet.wIndex = INTERFACE;
  packet.wLength = 0;

  SetupRequest(&packet, sizeof(packet));
  Mock::VerifyAndClearExpectations(m_usb_mock);
}

void USBHost::SetupRequest(void *data, unsigned int size) {
  m_event_handler(USB_DEVICE_EVENT_CONTROL_TRANSFER_SETUP_REQUEST,
                  reinterpret_cast<void*>(data), size);
}
