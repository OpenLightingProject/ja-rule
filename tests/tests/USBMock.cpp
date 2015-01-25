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

#include "CMockaWrapper.h"
#include "usb_stub.h"

void USB_DEVICE_Attach(USB_DEVICE_HANDLE usb_device) {
  check_expected(usb_device);
}

void USB_DEVICE_Detach(USB_DEVICE_HANDLE usb_device) {
  check_expected(usb_device);
}

USB_DEVICE_CONTROL_TRANSFER_RESULT USB_DEVICE_ControlStatus(
    USB_DEVICE_HANDLE usb_device,
    USB_DEVICE_CONTROL_STATUS status) {
  check_expected(usb_device);
  check_expected(status);
  return (USB_DEVICE_CONTROL_TRANSFER_RESULT) mock();
}

USB_DEVICE_CONTROL_TRANSFER_RESULT USB_DEVICE_ControlSend(
    USB_DEVICE_HANDLE usb_device,
    void* data,
    size_t length) {
  check_expected(usb_device);
  check_expected(data);
  check_expected(length);
  return (USB_DEVICE_CONTROL_TRANSFER_RESULT) mock();
}

USB_DEVICE_HANDLE USB_DEVICE_Open(
    const SYS_MODULE_INDEX index,
    const DRV_IO_INTENT intent) {
  check_expected(index);
  check_expected(intent);
  return (USB_DEVICE_HANDLE) mock();
}

bool USB_DEVICE_EndpointIsEnabled(
    USB_DEVICE_HANDLE usb_device,
    USB_ENDPOINT_ADDRESS endpoint) {
  check_expected(usb_device);
  check_expected(endpoint);
  return mock_type(bool);
}

USB_SPEED USB_DEVICE_ActiveSpeedGet(USB_DEVICE_HANDLE usb_device) {
  check_expected(usb_device);
  return (USB_SPEED) mock();
}

void USB_DEVICE_EventHandlerSet(
    USB_DEVICE_HANDLE usb_device,
    const USB_DEVICE_EVENT_HANDLER cb,
    uintptr_t context) {
  check_expected(usb_device);
  check_expected(cb);
  check_expected(context);

  USBEventHandler* cb_fn = mock_ptr_type(USBEventHandler*);
  *cb_fn = cb;
}

USB_DEVICE_RESULT USB_DEVICE_EndpointEnable(
    USB_DEVICE_HANDLE usb_device,
    uint8_t interface,
    USB_ENDPOINT_ADDRESS endpoint,
    USB_TRANSFER_TYPE transfer_type,
    size_t size) {
  check_expected(usb_device);
  check_expected(interface);
  check_expected(endpoint);
  check_expected(transfer_type);
  check_expected(size);
  return (USB_DEVICE_RESULT) mock();
}

USB_DEVICE_RESULT USB_DEVICE_EndpointDisable(
    USB_DEVICE_HANDLE usb_device,
    USB_ENDPOINT_ADDRESS endpoint) {
  check_expected(usb_device);
  check_expected(endpoint);
  return (USB_DEVICE_RESULT) mock();
}

void USB_DEVICE_EndpointStall(
    USB_DEVICE_HANDLE usb_device,
    USB_ENDPOINT_ADDRESS endpoint) {
  check_expected(usb_device);
  check_expected(endpoint);
}

USB_DEVICE_RESULT USB_DEVICE_EndpointRead(
    USB_DEVICE_HANDLE usb_device,
    USB_DEVICE_TRANSFER_HANDLE* transfer,
    USB_ENDPOINT_ADDRESS endpoint,
    void* buffer,
    size_t bufferSize) {
  check_expected(usb_device);
  check_expected(transfer);
  check_expected(endpoint);
  check_expected(buffer);
  check_expected(bufferSize);
  return (USB_DEVICE_RESULT) mock();
}

USB_DEVICE_RESULT USB_DEVICE_EndpointWrite(
    USB_DEVICE_HANDLE usb_device,
    USB_DEVICE_TRANSFER_HANDLE* transfer,
    USB_ENDPOINT_ADDRESS endpoint,
    const void* data,
    size_t size,
    USB_DEVICE_TRANSFER_FLAGS flags) {
  check_expected(usb_device);
  check_expected(transfer);
  check_expected(endpoint);
  check_expected(data);
  check_expected(size);
  check_expected(flags);
  return (USB_DEVICE_RESULT) mock();
}

