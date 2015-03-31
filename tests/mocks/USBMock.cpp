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
 * USBMock.cpp
 * Mock of the Harmony USB module.
 * Copyright (C) 2015 Simon Newton
 */

#include "USBMock.h"

#include <gmock/gmock.h>

namespace {
MockUSB *g_usb_mock = NULL;
}

void USB_DEVICE_Attach(USB_DEVICE_HANDLE usb_device) {
  if (g_usb_mock) {
    g_usb_mock->Attach(usb_device);
  }
}

void USB_DEVICE_Detach(USB_DEVICE_HANDLE usb_device) {
  if (g_usb_mock) {
    g_usb_mock->Detach(usb_device);
  }
}

USB_DEVICE_CONTROL_TRANSFER_RESULT USB_DEVICE_ControlStatus(
    USB_DEVICE_HANDLE usb_device,
    USB_DEVICE_CONTROL_STATUS status) {
  if (g_usb_mock) {
    return g_usb_mock->ControlStatus(usb_device, status);
  }
  return USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS;
}

USB_DEVICE_CONTROL_TRANSFER_RESULT USB_DEVICE_ControlSend(
    USB_DEVICE_HANDLE usb_device,
    void* data,
    size_t length) {
  if (g_usb_mock) {
    return g_usb_mock->ControlSend(usb_device, data, length);
  }
  return USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS;
}

USB_DEVICE_HANDLE USB_DEVICE_Open(
    const SYS_MODULE_INDEX index,
    const DRV_IO_INTENT intent) {
  if (g_usb_mock) {
    return g_usb_mock->Open(index, intent);
  }
  return 0;
}

bool USB_DEVICE_EndpointIsEnabled(
    USB_DEVICE_HANDLE usb_device,
    USB_ENDPOINT_ADDRESS endpoint) {
  if (g_usb_mock) {
    return g_usb_mock->EndpointIsEnabled(usb_device, endpoint);
  }
  return true;
}

USB_SPEED USB_DEVICE_ActiveSpeedGet(USB_DEVICE_HANDLE usb_device) {
  if (g_usb_mock) {
    return g_usb_mock->ActiveSpeedGet(usb_device);
  }
  return USB_SPEED_ERROR;
}

void USB_DEVICE_EventHandlerSet(
    USB_DEVICE_HANDLE usb_device,
    const USB_DEVICE_EVENT_HANDLER cb,
    uintptr_t context) {
  if (g_usb_mock) {
    g_usb_mock->EventHandlerSet(usb_device, cb, context);
  }
}

USB_DEVICE_RESULT USB_DEVICE_EndpointEnable(
    USB_DEVICE_HANDLE usb_device,
    uint8_t interface,
    USB_ENDPOINT_ADDRESS endpoint,
    USB_TRANSFER_TYPE transfer_type,
    size_t size) {
  if (g_usb_mock) {
    return g_usb_mock->EndpointEnable(usb_device, interface, endpoint,
                                      transfer_type, size);
  }
  return USB_DEVICE_RESULT_ERROR;
}

USB_DEVICE_RESULT USB_DEVICE_EndpointDisable(
    USB_DEVICE_HANDLE usb_device,
    USB_ENDPOINT_ADDRESS endpoint) {
  if (g_usb_mock) {
    return g_usb_mock->EndpointDisable(usb_device, endpoint);
  }
  return USB_DEVICE_RESULT_ERROR;
}

void USB_DEVICE_EndpointStall(
    USB_DEVICE_HANDLE usb_device,
    USB_ENDPOINT_ADDRESS endpoint) {
  if (g_usb_mock) {
    g_usb_mock->EndpointStall(usb_device, endpoint);
  }
}

USB_DEVICE_RESULT USB_DEVICE_EndpointRead(
    USB_DEVICE_HANDLE usb_device,
    USB_DEVICE_TRANSFER_HANDLE* transfer,
    USB_ENDPOINT_ADDRESS endpoint,
    void* buffer,
    size_t bufferSize) {
  if (g_usb_mock) {
    return g_usb_mock->EndpointRead(usb_device, transfer, endpoint, buffer,
                                    bufferSize);
  }
  return USB_DEVICE_RESULT_ERROR;
}

USB_DEVICE_RESULT USB_DEVICE_EndpointWrite(
    USB_DEVICE_HANDLE usb_device,
    USB_DEVICE_TRANSFER_HANDLE* transfer,
    USB_ENDPOINT_ADDRESS endpoint,
    const void* data,
    size_t size,
    USB_DEVICE_TRANSFER_FLAGS flags) {
  if (g_usb_mock) {
    return g_usb_mock->EndpointWrite(usb_device, transfer, endpoint, data,
                                     size, flags);
  }
  return USB_DEVICE_RESULT_ERROR;
}

USB_DEVICE_RESULT USB_DEVICE_EndpointTransferCancel(
    USB_DEVICE_HANDLE usbDeviceHandle,
    USB_ENDPOINT_ADDRESS endpoint,
    USB_DEVICE_TRANSFER_HANDLE transferHandle) {
  if (g_usb_mock) {
    return g_usb_mock->EndpointTransferCancel(usbDeviceHandle, endpoint,
                                              transferHandle);
  }
  return USB_DEVICE_RESULT_ERROR;
}

void USB_SetMock(MockUSB* mock) {
  g_usb_mock = mock;
}
