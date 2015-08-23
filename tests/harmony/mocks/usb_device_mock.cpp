#include <gmock/gmock.h>
#include "usb_device_mock.h"

namespace {
  USBDeviceInterface *g_usb_device_mock = NULL;
}

void USBDevice_SetMock(USBDeviceInterface* mock) {
  g_usb_device_mock = mock;
}

void USB_DEVICE_Attach(USB_DEVICE_HANDLE usbDeviceHandle) {
  if (g_usb_device_mock) {
    g_usb_device_mock->Attach(usbDeviceHandle);
  }
}

void USB_DEVICE_Detach(USB_DEVICE_HANDLE usbDeviceHandle) {
  if (g_usb_device_mock) {
    g_usb_device_mock->Detach(usbDeviceHandle);
  }
}

USB_DEVICE_HANDLE USB_DEVICE_Open(
      const uint16_t instanceIndex,
      const DRV_IO_INTENT intent) {
  if (g_usb_device_mock) {
    return g_usb_device_mock->Open(instanceIndex, intent);
  }
  return USB_DEVICE_HANDLE_INVALID;
}

void USB_DEVICE_EventHandlerSet(
    USB_DEVICE_HANDLE usbDeviceHandle,
    const USB_DEVICE_EVENT_HANDLER callBackFunc,
    uintptr_t context) {
  if (g_usb_device_mock) {
    return g_usb_device_mock->EventHandlerSet(usbDeviceHandle, callBackFunc,
                                              context);
  }
}

USB_DEVICE_CONTROL_TRANSFER_RESULT USB_DEVICE_ControlSend(
    USB_DEVICE_HANDLE usbDeviceHandle,
    void *data,
    size_t length) {
  if (g_usb_device_mock) {
    return g_usb_device_mock->ControlSend(usbDeviceHandle, data, length);
  }
  return USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS;
}

USB_DEVICE_CONTROL_TRANSFER_RESULT USB_DEVICE_ControlReceive(
    USB_DEVICE_HANDLE usbDeviceHandle,
    void* data,
    size_t length) {
  if (g_usb_device_mock) {
    return g_usb_device_mock->ControlReceive(usbDeviceHandle, data, length);
  }
  return USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS;
}

USB_DEVICE_CONTROL_TRANSFER_RESULT USB_DEVICE_ControlStatus(
    USB_DEVICE_HANDLE usbDeviceHandle,
    USB_DEVICE_CONTROL_STATUS status) {
  if (g_usb_device_mock) {
    return g_usb_device_mock->ControlStatus(usbDeviceHandle, status);
  }
  return USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS;
}

USB_SPEED USB_DEVICE_ActiveSpeedGet(USB_DEVICE_HANDLE usbDeviceHandle) {
  if (g_usb_device_mock) {
    return g_usb_device_mock->ActiveSpeedGet(usbDeviceHandle);
  }
  return USB_SPEED_FULL;
}

bool USB_DEVICE_EndpointIsEnabled(USB_DEVICE_HANDLE usbDeviceHandle,
                                  USB_ENDPOINT_ADDRESS endpoint) {
  if (g_usb_device_mock) {
    return g_usb_device_mock->EndpointIsEnabled(usbDeviceHandle, endpoint);
  }
  return true;
}

USB_DEVICE_RESULT USB_DEVICE_EndpointEnable(
    USB_DEVICE_HANDLE usbDeviceHandle,
    uint8_t interface,
    USB_ENDPOINT_ADDRESS endpoint,
    USB_TRANSFER_TYPE transferType,
    size_t size) {
  if (g_usb_device_mock) {
    return g_usb_device_mock->EndpointEnable(usbDeviceHandle, interface,
                                             endpoint, transferType, size);
  }
  return USB_DEVICE_RESULT_OK;
}

USB_DEVICE_RESULT USB_DEVICE_EndpointDisable(
    USB_DEVICE_HANDLE usbDeviceHandle,
    USB_ENDPOINT_ADDRESS endpoint) {
  if (g_usb_device_mock) {
    return g_usb_device_mock->EndpointDisable(usbDeviceHandle, endpoint);
  }
  return USB_DEVICE_RESULT_OK;
}

void USB_DEVICE_EndpointStall(
    USB_DEVICE_HANDLE usbDeviceHandle,
    USB_ENDPOINT_ADDRESS endpoint) {
  if (g_usb_device_mock) {
    g_usb_device_mock->EndpointStall(usbDeviceHandle, endpoint);
  }
}

USB_DEVICE_RESULT USB_DEVICE_EndpointRead(
    USB_DEVICE_HANDLE usbDeviceHandle,
    USB_DEVICE_TRANSFER_HANDLE * transferHandle,
    USB_ENDPOINT_ADDRESS endpoint,
    void* buffer,
    size_t bufferSize) {
  if (g_usb_device_mock) {
    return g_usb_device_mock->EndpointRead(usbDeviceHandle, transferHandle,
                                           endpoint, buffer, bufferSize);
  }
  return USB_DEVICE_RESULT_OK;
}

USB_DEVICE_RESULT USB_DEVICE_EndpointWrite(
    USB_DEVICE_HANDLE usbDeviceHandle,
    USB_DEVICE_TRANSFER_HANDLE * transferHandle,
    USB_ENDPOINT_ADDRESS endpoint,
    const void* data,
    size_t size,
    USB_DEVICE_TRANSFER_FLAGS flags) {
  if (g_usb_device_mock) {
    return g_usb_device_mock->EndpointWrite(usbDeviceHandle, transferHandle,
                                            endpoint, data, size, flags);
  }
  return USB_DEVICE_RESULT_OK;
}

USB_DEVICE_RESULT USB_DEVICE_EndpointTransferCancel(
    USB_DEVICE_HANDLE usbDeviceHandle,
    USB_ENDPOINT_ADDRESS endpoint,
    USB_DEVICE_TRANSFER_HANDLE transferHandle) {
  if (g_usb_device_mock) {
    return g_usb_device_mock->EndpointTransferCancel(usbDeviceHandle,
                                                     endpoint, transferHandle);
  }
  return USB_DEVICE_RESULT_OK;
}
