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
 * BootloaderTest.cpp
 * Tests for the Bootloader code.
 * Copyright (C) 2015 Simon Newton
 */

#include <gtest/gtest.h>

#include "Array.h"
#include "BootloaderOptionsMock.h"
#include "BootloaderTestHelper.h"
#include "FlashMock.h"
#include "LauncherMock.h"
#include "Matchers.h"
#include "ResetMock.h"
#include "bootloader.h"
#include "dfu_constants.h"
#include "macros.h"
#include "plib_ports_mock.h"
#include "usb/usb_device.h"
#include "usb_device_mock.h"

using testing::Args;
using testing::AtLeast;
using testing::InSequence;
using testing::Return;
using testing::SaveArg;
using testing::_;

class BaseBootloaderTest : public testing::Test {
 public:
  void SetUp() {
    USBDevice_SetMock(&m_usb_mock);
    Flash_SetMock(&m_flash_mock);
    PLIB_PORTS_SetMock(&m_ports);
    Launcher_SetMock(&m_launcher);
    BootloaderOptions_SetMock(&m_bootload_options);
    Reset_SetMock(&m_reset);
  }

  void TearDown() {
    USBDevice_SetMock(nullptr);
    Flash_SetMock(nullptr);
    PLIB_PORTS_SetMock(nullptr);
    Launcher_SetMock(nullptr);
    BootloaderOptions_SetMock(nullptr);
    Reset_SetMock(nullptr);
  }

 protected:
  testing::StrictMock<MockUSBDevice> m_usb_mock;
  MockFlash m_flash_mock;
  testing::NiceMock<MockPeripheralPorts> m_ports;
  testing::NiceMock<MockLauncher> m_launcher;
  testing::NiceMock<MockBootloaderOptions> m_bootload_options;
  testing::NiceMock<MockReset> m_reset;;
};

TEST_F(BaseBootloaderTest, USBLifecycle) {
  USB_DEVICE_HANDLE usb_handle = 0x12345678;
  USB_DEVICE_EVENT_HANDLER event_handler;
  ON_CALL(m_bootload_options, GetBootOption())
        .WillByDefault(Return(BOOT_BOOTLOADER));

  InSequence seq;
  EXPECT_CALL(m_usb_mock, Open(USB_DEVICE_INDEX_0, DRV_IO_INTENT_READWRITE))
    .WillOnce(Return(USB_DEVICE_HANDLE_INVALID));
  EXPECT_CALL(m_usb_mock, Open(USB_DEVICE_INDEX_0, DRV_IO_INTENT_READWRITE))
    .WillOnce(Return(usb_handle));
  EXPECT_CALL(m_usb_mock, EventHandlerSet(usb_handle, _, 0u))
    .WillOnce(SaveArg<1>(&event_handler));
  EXPECT_CALL(m_usb_mock, Attach(usb_handle)).Times(1);

  Bootloader_Initialize();
  EXPECT_FALSE(Bootloader_USBActive());
  Bootloader_Tasks();
  Bootloader_Tasks();
  EXPECT_FALSE(Bootloader_USBActive());

  // Power event
  event_handler(USB_DEVICE_EVENT_POWER_DETECTED, nullptr, 0u);
  EXPECT_FALSE(Bootloader_USBActive());
  Bootloader_Tasks();
  EXPECT_FALSE(Bootloader_USBActive());

  // Device configured
  uint8_t configuration = 1u;
  event_handler(USB_DEVICE_EVENT_CONFIGURED, &configuration, 0u);
  EXPECT_TRUE(Bootloader_USBActive());

  // Deconfigured event
  event_handler(USB_DEVICE_EVENT_DECONFIGURED, nullptr, 0u);
  Bootloader_Tasks();
  EXPECT_FALSE(Bootloader_USBActive());

  // Power removed
  EXPECT_CALL(m_usb_mock, Detach(usb_handle));
  event_handler(USB_DEVICE_EVENT_POWER_REMOVED, nullptr, 0u);
  Bootloader_Tasks();

  // Power applied
  EXPECT_CALL(m_usb_mock, Attach(usb_handle));
  event_handler(USB_DEVICE_EVENT_POWER_DETECTED, nullptr, 0u);
  Bootloader_Tasks();
}

TEST_F(BaseBootloaderTest, launchApp) {
  EXPECT_CALL(m_bootload_options, GetBootOption())
        .WillOnce(Return(BOOT_PRIMARY_APPLICATION));
  EXPECT_CALL(m_ports, PinGet(PORTS_ID_0, _, _))
        .WillOnce(Return(true));
  EXPECT_CALL(m_flash_mock, ReadWord(_))
        .WillOnce(Return(0));

  EXPECT_CALL(m_launcher, RunApp(_))
    .Times(1);

  Bootloader_Initialize();
}

TEST_F(BaseBootloaderTest, enterBootloaderFromReset) {
  EXPECT_CALL(m_bootload_options, GetBootOption())
        .WillOnce(Return(BOOT_PRIMARY_APPLICATION));
  EXPECT_CALL(m_launcher, RunApp(_))
    .Times(0);

  Bootloader_Initialize();
}

TEST_F(BaseBootloaderTest, enterBootloaderFromSwitch) {
  EXPECT_CALL(m_bootload_options, GetBootOption())
        .WillOnce(Return(BOOT_PRIMARY_APPLICATION));
  EXPECT_CALL(m_ports, PinGet(PORTS_ID_0, _, _))
        .WillOnce(Return(false));
  EXPECT_CALL(m_launcher, RunApp(_))
    .Times(0);

  Bootloader_Initialize();
}

TEST_F(BaseBootloaderTest, enterBootloaderFromBadFirmware) {
  EXPECT_CALL(m_bootload_options, GetBootOption())
        .WillOnce(Return(BOOT_PRIMARY_APPLICATION));
  EXPECT_CALL(m_ports, PinGet(PORTS_ID_0, _, _))
        .WillOnce(Return(true));
  EXPECT_CALL(m_flash_mock, ReadWord(_))
        .WillOnce(Return(0xffffffff));
  EXPECT_CALL(m_launcher, RunApp(_))
    .Times(0);

  Bootloader_Initialize();
}

class BootloaderTest : public BaseBootloaderTest {
 public:
  BootloaderTest() : m_host(&m_usb_mock) {}

  void SetUp() {
    BaseBootloaderTest::SetUp();

    ON_CALL(m_bootload_options, GetBootOption())
          .WillByDefault(Return(BOOT_BOOTLOADER));

    m_host.InitDevice();
  }

 protected:
  USBHost m_host;

  static const uint8_t UID_IMAGE[];
};

const uint8_t BootloaderTest::UID_IMAGE[] = {
  0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x6a, 0x51, 0xa0, 0xa2,
  0x00, 0x00, 0x00, 0x00, 0x7a, 0x70, 0x00, 0x00, 0x00, 0x01
};

TEST_F(BootloaderTest, getStatusAndState) {
  uint8_t state;
  DFUState dfu_state;
  DFUStatus dfu_status;

  m_host.GetDFUState(USBHost::OUTCOME_OK, &state);

  m_host.GetDFUStatus(&dfu_state, &dfu_status);

  EXPECT_EQ(DFU_STATE_IDLE, state);
  EXPECT_EQ(DFU_STATE_IDLE, dfu_state);
  EXPECT_EQ(DFU_STATUS_OK, dfu_status);
}

TEST_F(BootloaderTest, getSetInterface) {
  const uint8_t response1[] = { 0 };
  const uint8_t response2[] = { 1 };

  InSequence seq;
  EXPECT_CALL(m_usb_mock, ControlSend(_, _, _))
      .With(Args<1, 2>(DataIs(response1, arraysize(response1))))
      .WillOnce(Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS));
  EXPECT_CALL(m_usb_mock,
              ControlStatus(_, USB_DEVICE_CONTROL_STATUS_OK))
      .WillOnce(Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS));
  EXPECT_CALL(m_usb_mock, ControlSend(_, _, _))
      .With(Args<1, 2>(DataIs(response2, arraysize(response2))))
      .WillOnce(Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS));
  EXPECT_CALL(m_usb_mock,
              ControlStatus(_, USB_DEVICE_CONTROL_STATUS_ERROR))
      .WillOnce(Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS));

  USB_SETUP_PACKET get_interface_request;
  get_interface_request.bRequest = USB_REQUEST_GET_INTERFACE;
  m_host.SetupRequest(&get_interface_request, sizeof(get_interface_request));

  // Set to 1 and check with a GET_INTERFACE
  USB_SETUP_PACKET set_interface_request;
  set_interface_request.bRequest = USB_REQUEST_SET_INTERFACE;
  set_interface_request.wValue = 1;
  m_host.SetupRequest(&set_interface_request, sizeof(set_interface_request));
  m_host.SetupRequest(&get_interface_request, sizeof(get_interface_request));

  // Try to set an out-of-range interface
  set_interface_request.wValue = 2;
  m_host.SetupRequest(&set_interface_request, sizeof(set_interface_request));
}

TEST_F(BootloaderTest, doubleDownload) {
  // Send two DNLOAD messages without a GET STATUS in between.
  const unsigned int block_size = 8;
  m_host.DFUDownload(USBHost::DOWNLOAD_OUTCOME_RECEIVE, 0, UID_IMAGE,
                     block_size);
  if (::testing::Test::HasNonfatalFailure()) {
    return;
  }

  Bootloader_Tasks();

  // Now try the second one.
  m_host.DFUDownload(USBHost::DOWNLOAD_OUTCOME_STALL, 0,
                     UID_IMAGE + block_size,
                     block_size);
  Bootloader_Tasks();

  EXPECT_EQ(DFU_STATE_ERROR, Bootloader_GetState());
  EXPECT_EQ(DFU_STATUS_ERR_STALLED_PKT, Bootloader_GetStatus());
}

TEST_F(BootloaderTest, unexpectedClearState) {
  EXPECT_CALL(m_usb_mock,
              ControlStatus(_, USB_DEVICE_CONTROL_STATUS_ERROR))
    .WillOnce(Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS));

  USB_SETUP_PACKET packet;
  packet.bmRequestType = 0x21;
  packet.bRequest = DFU_CLRSTATUS;
  packet.wValue = 0;
  packet.wIndex = 0;
  packet.wLength = 0;

  m_host.SetupRequest(&packet, sizeof(packet));
  EXPECT_EQ(DFU_STATE_ERROR, Bootloader_GetState());
  EXPECT_EQ(DFU_STATUS_ERR_STALLED_PKT, Bootloader_GetStatus());
}

TEST_F(BootloaderTest, unknownDeviceToHostCommand) {
  EXPECT_CALL(m_usb_mock, ControlStatus(_, USB_DEVICE_CONTROL_STATUS_ERROR))
    .WillOnce(Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS));

  USB_SETUP_PACKET packet;
  packet.bmRequestType = 0xa1;
  packet.bRequest = 0x7;  // not a DFU command
  packet.wValue = 0;
  packet.wIndex = 0;
  packet.wLength = 0;
  m_host.SetupRequest(reinterpret_cast<uint8_t*>(&packet), sizeof(packet));
}

TEST_F(BootloaderTest, unknownHostToDeviceCommand) {
  EXPECT_CALL(m_usb_mock, ControlStatus(_, USB_DEVICE_CONTROL_STATUS_ERROR))
    .WillOnce(Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS));

  USB_SETUP_PACKET packet;
  packet.bmRequestType = 0x21;
  packet.bRequest = 0x7;  // not a DFU command
  packet.wValue = 0;
  packet.wIndex = 0;
  packet.wLength = 0;
  m_host.SetupRequest(reinterpret_cast<uint8_t*>(&packet), sizeof(packet));
}

TEST_F(BootloaderTest, unknownSetupPacket) {
  EXPECT_CALL(m_usb_mock, ControlStatus(_, USB_DEVICE_CONTROL_STATUS_ERROR))
    .WillOnce(Return(USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS));

  USB_SETUP_PACKET packet;
  packet.bmRequestType = 0;
  packet.bRequest = 0;
  packet.wValue = 0;
  packet.wIndex = 0;
  packet.wLength = 0;
  m_host.SetupRequest(reinterpret_cast<uint8_t*>(&packet), sizeof(packet));
}

TEST_F(BootloaderTest, abortDuringIdle) {
  m_host.DFUAbort(USBHost::OUTCOME_OK);
}

TEST_F(BootloaderTest, resetWhileIdle) {
  EXPECT_CALL(m_reset, SoftReset()).Times(1);
  m_host.SendUSBReset();
  Bootloader_Tasks();
}

TEST_F(BootloaderTest, deconfiguredWhileIdle) {
  EXPECT_TRUE(Bootloader_USBActive());
  m_host.SendDeconfigure();
  Bootloader_Tasks();
  EXPECT_FALSE(Bootloader_USBActive());
}

TEST_F(BootloaderTest, powerLossWhileIdle) {
  EXPECT_TRUE(Bootloader_USBActive());
  EXPECT_EQ(DFU_STATE_IDLE, Bootloader_GetState());

  EXPECT_CALL(m_usb_mock, Detach(_));
  m_host.SendPowerLoss();
  Bootloader_Tasks();
  EXPECT_FALSE(Bootloader_USBActive());
}

TEST_F(BootloaderTest, flashLED) {
  EXPECT_CALL(m_ports, PinToggle(PORTS_ID_0, _, _))
        .Times(AtLeast(1));

  for (unsigned int i = 0; i < 100000; i++) {
    Bootloader_Tasks();
  }
}
