
#ifndef TESTS_TESTS_USBMOCK_H_
#define TESTS_TESTS_USBMOCK_H_

#include "usb_stub.h"

typedef void (*USBEventHandler)(USB_DEVICE_EVENT, void*, uintptr_t);

#endif  // TESTS_TESTS_USBMOCK_H_
