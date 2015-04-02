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
 * USBMock.h
 * Mock of the Harmony USB module.
 * Copyright (C) 2015 Simon Newton
 */

#ifndef TESTS_MOCKS_USBMOCK_H_
#define TESTS_MOCKS_USBMOCK_H_

#include <gmock/gmock.h>
#include "usb_stub.h"

class MockUSB {
 public:
  MOCK_METHOD1(Attach, void(USB_DEVICE_HANDLE usb_device));
  MOCK_METHOD1(Detach, void(USB_DEVICE_HANDLE usb_device));
  MOCK_METHOD2(
      ControlStatus,
      USB_DEVICE_CONTROL_TRANSFER_RESULT(USB_DEVICE_HANDLE usb_device,
                                         USB_DEVICE_CONTROL_STATUS status));
  MOCK_METHOD3(
      ControlSend,
      USB_DEVICE_CONTROL_TRANSFER_RESULT(USB_DEVICE_HANDLE usb_device,
                                         void* data,
                                         size_t length));
  MOCK_METHOD2(Open, USB_DEVICE_HANDLE(const SYS_MODULE_INDEX index,
                                       const DRV_IO_INTENT intent));
  MOCK_METHOD2(EndpointIsEnabled, bool(USB_DEVICE_HANDLE usb_device,
                                       USB_ENDPOINT_ADDRESS endpoint));
  MOCK_METHOD1(ActiveSpeedGet, USB_SPEED(USB_DEVICE_HANDLE usb_device));
  MOCK_METHOD3(EventHandlerSet, void(USB_DEVICE_HANDLE usb_device,
                                     const USB_DEVICE_EVENT_HANDLER cb,
                                     uintptr_t context));
  MOCK_METHOD5(EndpointEnable,
               USB_DEVICE_RESULT(USB_DEVICE_HANDLE usb_device,
                                 uint8_t interface,
                                 USB_ENDPOINT_ADDRESS endpoint,
                                 USB_TRANSFER_TYPE transfer_type,
                                 size_t size));
  MOCK_METHOD2(EndpointDisable,
               USB_DEVICE_RESULT(USB_DEVICE_HANDLE usb_device,
                                 USB_ENDPOINT_ADDRESS endpoint));
  MOCK_METHOD2(EndpointStall, void(USB_DEVICE_HANDLE usb_device,
                                   USB_ENDPOINT_ADDRESS endpoint));
  MOCK_METHOD5(EndpointRead,
               USB_DEVICE_RESULT(USB_DEVICE_HANDLE usb_device,
                                 USB_DEVICE_TRANSFER_HANDLE* transfer,
                                 USB_ENDPOINT_ADDRESS endpoint,
                                 void* buffer,
                                 size_t bufferSize));
  MOCK_METHOD6(EndpointWrite,
               USB_DEVICE_RESULT(USB_DEVICE_HANDLE usb_device,
                                 USB_DEVICE_TRANSFER_HANDLE* transfer,
                                 USB_ENDPOINT_ADDRESS endpoint,
                                 const void* data,
                                 size_t size,
                                 USB_DEVICE_TRANSFER_FLAGS flags));

  MOCK_METHOD3(EndpointTransferCancel,
               USB_DEVICE_RESULT(USB_DEVICE_HANDLE usb_device,
                                 USB_ENDPOINT_ADDRESS endpoint,
                                 USB_DEVICE_TRANSFER_HANDLE transfer));
};

void USB_SetMock(MockUSB* mock);

#endif  // TESTS_MOCKS_USBMOCK_H_
