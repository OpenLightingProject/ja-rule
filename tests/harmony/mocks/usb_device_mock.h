#ifndef TESTS_HARMONY_MOCKS_USB_DEVICE_MOCK_H_
#define TESTS_HARMONY_MOCKS_USB_DEVICE_MOCK_H_

#include <gmock/gmock.h>
#include "usb/usb_device.h"

class USBDeviceInterface {
 public:
  virtual ~USBDeviceInterface() {}

  virtual void Attach(USB_DEVICE_HANDLE usbDeviceHandle) = 0;
  virtual void Detach(USB_DEVICE_HANDLE usbDeviceHandle) = 0;

  virtual USB_DEVICE_HANDLE Open(const uint16_t instanceIndex,
                                 const DRV_IO_INTENT intent) = 0;
  virtual void EventHandlerSet(USB_DEVICE_HANDLE usbDeviceHandle,
                               const USB_DEVICE_EVENT_HANDLER callBackFunc,
                               uintptr_t context) = 0;

  virtual USB_DEVICE_CONTROL_TRANSFER_RESULT ControlStatus(
      USB_DEVICE_HANDLE usbDeviceHandle,
      USB_DEVICE_CONTROL_STATUS status) = 0;
  virtual USB_DEVICE_CONTROL_TRANSFER_RESULT ControlSend(
      USB_DEVICE_HANDLE usbDeviceHandle,
      void *data,
      size_t length) = 0;
  virtual USB_DEVICE_CONTROL_TRANSFER_RESULT ControlReceive(
      USB_DEVICE_HANDLE usbDeviceHandle,
      void* data,
      size_t length) = 0;
  virtual USB_SPEED ActiveSpeedGet(USB_DEVICE_HANDLE usbDeviceHandle) = 0;
  virtual bool EndpointIsEnabled(USB_DEVICE_HANDLE usbDeviceHandle,
                                 USB_ENDPOINT_ADDRESS endpoint) = 0;
  virtual USB_DEVICE_RESULT EndpointEnable(
      USB_DEVICE_HANDLE usbDeviceHandle,
      uint8_t interface,
      USB_ENDPOINT_ADDRESS endpoint,
      USB_TRANSFER_TYPE transferType,
      size_t size) = 0;
  virtual USB_DEVICE_RESULT EndpointDisable(USB_DEVICE_HANDLE usbDeviceHandle,
                                            USB_ENDPOINT_ADDRESS endpoint) = 0;
  virtual void EndpointStall(USB_DEVICE_HANDLE usbDeviceHandle,
                             USB_ENDPOINT_ADDRESS endpoint) = 0;
  virtual USB_DEVICE_RESULT EndpointRead(
      USB_DEVICE_HANDLE usbDeviceHandle,
      USB_DEVICE_TRANSFER_HANDLE * transferHandle,
      USB_ENDPOINT_ADDRESS endpoint,
      void* buffer,
      size_t bufferSize) = 0;
  virtual USB_DEVICE_RESULT EndpointWrite(
      USB_DEVICE_HANDLE usbDeviceHandle,
      USB_DEVICE_TRANSFER_HANDLE * transferHandle,
      USB_ENDPOINT_ADDRESS endpoint,
      const void* data,
      size_t size,
      USB_DEVICE_TRANSFER_FLAGS flags) = 0;
  virtual USB_DEVICE_RESULT EndpointTransferCancel(
      USB_DEVICE_HANDLE usbDeviceHandle,
      USB_ENDPOINT_ADDRESS endpoint,
      USB_DEVICE_TRANSFER_HANDLE transferHandle) = 0;
};

class MockUSBDevice : public USBDeviceInterface {
 public:
  MOCK_METHOD1(Attach, void(USB_DEVICE_HANDLE usbDeviceHandle));
  MOCK_METHOD1(Detach, void(USB_DEVICE_HANDLE usbDeviceHandle));
  MOCK_METHOD2(Open,
               USB_DEVICE_HANDLE(
                   const uint16_t instanceIndex,
                   const DRV_IO_INTENT intent));
  MOCK_METHOD3(EventHandlerSet,
               void(USB_DEVICE_HANDLE usbDeviceHandle,
                    const USB_DEVICE_EVENT_HANDLER callBackFunc,
                    uintptr_t context));

  MOCK_METHOD2(ControlStatus,
               USB_DEVICE_CONTROL_TRANSFER_RESULT(
                   USB_DEVICE_HANDLE usbDeviceHandle,
                   USB_DEVICE_CONTROL_STATUS status));
  MOCK_METHOD3(ControlSend,
               USB_DEVICE_CONTROL_TRANSFER_RESULT(
                  USB_DEVICE_HANDLE usbDeviceHandle,
                  void *data,
                  size_t length));
  MOCK_METHOD3(ControlReceive,
               USB_DEVICE_CONTROL_TRANSFER_RESULT(
                   USB_DEVICE_HANDLE usbDeviceHandle,
                   void* data,
                   size_t length));

  MOCK_METHOD1(ActiveSpeedGet, USB_SPEED(USB_DEVICE_HANDLE usbDeviceHandle));
  MOCK_METHOD2(EndpointIsEnabled,
               bool(USB_DEVICE_HANDLE usbDeviceHandle,
                    USB_ENDPOINT_ADDRESS endpoint));
  MOCK_METHOD5(EndpointEnable,
               USB_DEVICE_RESULT(USB_DEVICE_HANDLE usbDeviceHandle,
                                 uint8_t interface,
                                 USB_ENDPOINT_ADDRESS endpoint,
                                 USB_TRANSFER_TYPE transferType,
                                 size_t size));
  MOCK_METHOD2(EndpointDisable,
               USB_DEVICE_RESULT(USB_DEVICE_HANDLE usbDeviceHandle,
                                 USB_ENDPOINT_ADDRESS endpoint));
  MOCK_METHOD2(EndpointStall,
               void(USB_DEVICE_HANDLE usbDeviceHandle,
                    USB_ENDPOINT_ADDRESS endpoint));

  MOCK_METHOD5(EndpointRead,
               USB_DEVICE_RESULT(USB_DEVICE_HANDLE usbDeviceHandle,
                                 USB_DEVICE_TRANSFER_HANDLE * transferHandle,
                                 USB_ENDPOINT_ADDRESS endpoint,
                                 void* buffer,
                                 size_t bufferSize));
  MOCK_METHOD6(EndpointWrite,
               USB_DEVICE_RESULT(USB_DEVICE_HANDLE usbDeviceHandle,
                                 USB_DEVICE_TRANSFER_HANDLE * transferHandle,
                                 USB_ENDPOINT_ADDRESS endpoint,
                                 const void* data,
                                 size_t size,
                                 USB_DEVICE_TRANSFER_FLAGS flags));
  MOCK_METHOD3(EndpointTransferCancel,
               USB_DEVICE_RESULT(USB_DEVICE_HANDLE usbDeviceHandle,
                                 USB_ENDPOINT_ADDRESS endpoint,
                                 USB_DEVICE_TRANSFER_HANDLE transferHandle));
};

void USBDevice_SetMock(USBDeviceInterface* mock);

#endif  // TESTS_HARMONY_MOCKS_USB_DEVICE_MOCK_H_
