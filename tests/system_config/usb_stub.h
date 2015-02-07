/*
 * This is the usb_stub.h used for the tests. It contains the bare
 * minimum required to implement the mock USB backend.
 */

/*******************************************************************************
 * Copyright (c) 2013 released Microchip Technology Inc.  All rights reserved.
 *
 * Microchip licenses to you the right to use, modify, copy and distribute
 * Software only when embedded on a Microchip microcontroller or digital signal
 * controller that is integrated into your product or third party product
 * (pursuant to the sublicense terms in the accompanying license agreement).
 *
 * You should refer to the license agreement accompanying this Software for
 * additional information regarding your rights and obligations.
 *
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
 * MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR
 * PURPOSE.
 * IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
 * CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
 * OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
 * INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
 * SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
 * (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 ******************************************************************************/

#ifndef TESTS_SYSTEM_CONFIG_USB_STUB_H_
#define TESTS_SYSTEM_CONFIG_USB_STUB_H_

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#ifdef  __cplusplus
extern "C" {
#endif

// Non USB bits
typedef uint16_t SYS_MODULE_INDEX;

typedef enum {
  DRV_IO_INTENT_READ                = 1 << 0,
  DRV_IO_INTENT_WRITE               = 1 << 1,
  DRV_IO_INTENT_READWRITE = DRV_IO_INTENT_READ | DRV_IO_INTENT_WRITE,
  DRV_IO_INTENT_BLOCKING            = 0 << 2,
  DRV_IO_INTENT_NONBLOCKING         = 1 << 2,
  DRV_IO_INTENT_EXCLUSIVE           = 1 << 3,
  DRV_IO_INTENT_SHARED              = 0 << 3
} DRV_IO_INTENT;

// Enums
typedef enum {
  USB_TRANSFER_TYPE_CONTROL       = 0x00,
  USB_TRANSFER_TYPE_ISOCHRONOUS   = 0x01,
  USB_TRANSFER_TYPE_BULK          = 0x02,
  USB_TRANSFER_TYPE_INTERRUPT     = 0x03
} USB_TRANSFER_TYPE;

typedef enum {
  USB_SPEED_LOW = 1,
  USB_SPEED_FULL = (USB_SPEED_LOW << 1),
  USB_SPEED_HIGH = (USB_SPEED_FULL << 1),
  USB_SPEED_ERROR = 0
} USB_SPEED;

typedef enum {
  USB_ERROR_IRP_QUEUE_FULL = SCHAR_MIN ,
  USB_ERROR_OSAL_FUNCTION ,
  USB_ERROR_IRP_SIZE_INVALID ,
  USB_ERROR_PARAMETER_INVALID ,
  USB_ERROR_DEVICE_ENDPOINT_INVALID ,
  USB_ERROR_DEVICE_IRP_IN_USE ,
  USB_ERROR_CLIENT_NOT_READY ,
  USB_ERROR_IRP_OBJECTS_UNAVAILABLE ,
  USB_ERROR_DEVICE_FUNCTION_INSTANCE_INVALID,
  USB_ERROR_DEVICE_FUNCTION_NOT_CONFIGURED,
  USB_ERROR_ENDPOINT_NOT_CONFIGURED,
  USB_ERROR_DEVICE_CONTROL_TRANSFER_FAILED,
  USB_ERROR_HOST_DEVICE_INSTANCE_INVALID,
  USB_ERROR_HOST_DRIVER_NOT_READY,
  USB_ERROR_HOST_DRIVER_NOT_FOUND,
  USB_ERROR_HOST_ENDPOINT_INVALID,
  USB_ERROR_HOST_PIPE_INVALID,
  USB_ERROR_HOST_ARGUMENTS_INVALID,
  USB_ERROR_HOST_HEADERSIZE_INVALID,
  USB_ERROR_HOST_MAX_INTERFACES_INVALID,
  USB_ERROR_HOST_ENDPOINT_DESC_SIZE_INVALID,
  USB_ERROR_HOST_DESCRIPTOR_INVALID,
  USB_ERROR_HOST_MAX_ENDPOINT_INVALID,
  USB_ERROR_HOST_ALT_SETTING_INVALID,
  USB_ERROR_HOST_BUSY,
  USB_HOST_OBJ_INVALID,
  USB_ERROR_HOST_POINTER_INVALID,
  USB_ERROR_HOST_ENDPOINT_NOT_FOUND,
  USB_ERROR_HOST_DRIVER_INSTANCE_INVALID,
  USB_ERROR_HOST_INTERFACE_NOT_FOUND,
  USB_ERROR_NONE = 0,
} USB_ERROR;

typedef enum {
  USB_DEVICE_RESULT_ERROR_TRANSFER_QUEUE_FULL = USB_ERROR_DEVICE_IRP_IN_USE,
  USB_DEVICE_RESULT_OK = USB_ERROR_NONE,
  USB_DEVICE_RESULT_ERROR_ENDPOINT_NOT_CONFIGURED =
      USB_ERROR_ENDPOINT_NOT_CONFIGURED,
  USB_DEVICE_RESULT_ERROR_ENDPOINT_INVALID =
      USB_ERROR_DEVICE_ENDPOINT_INVALID,
  USB_DEVICE_RESULT_ERROR_PARAMETER_INVALID = USB_ERROR_PARAMETER_INVALID,
  USB_DEVICE_RESULT_ERROR_DEVICE_HANDLE_INVALID,
  USB_DEVICE_RESULT_ERROR
} USB_DEVICE_RESULT;

typedef enum {
  DRV_USB_EVENT_ERROR = 1,
  DRV_USB_EVENT_RESET_DETECT,
  DRV_USB_EVENT_RESUME_DETECT,
  DRV_USB_EVENT_IDLE_DETECT,
  DRV_USB_EVENT_STALL,
  DRV_USB_EVENT_SOF_DETECT,
  DRV_USB_EVENT_HOST_ATTACH,
  DRV_USB_EVENT_HOST_DETACH,
  DRV_USB_EVENT_DEVICE_SESSION_VALID,
  DRV_USB_EVENT_DEVICE_SESSION_INVALID,
} DRV_USB_EVENT;

typedef enum {
  USB_DEVICE_EVENT_RESET = DRV_USB_EVENT_RESET_DETECT,
  USB_DEVICE_EVENT_SUSPENDED = DRV_USB_EVENT_IDLE_DETECT,
  USB_DEVICE_EVENT_RESUMED = DRV_USB_EVENT_RESUME_DETECT,
  USB_DEVICE_EVENT_ERROR = DRV_USB_EVENT_ERROR,
  USB_DEVICE_EVENT_SOF = DRV_USB_EVENT_SOF_DETECT,
  USB_DEVICE_EVENT_POWER_DETECTED = DRV_USB_EVENT_DEVICE_SESSION_VALID,
  USB_DEVICE_EVENT_POWER_REMOVED = DRV_USB_EVENT_DEVICE_SESSION_INVALID,
  USB_DEVICE_EVENT_CONFIGURED,
  USB_DEVICE_EVENT_DECONFIGURED,
  USB_DEVICE_EVENT_CONTROL_TRANSFER_ABORTED,
  USB_DEVICE_EVENT_CONTROL_TRANSFER_DATA_RECEIVED,
  USB_DEVICE_EVENT_CONTROL_TRANSFER_SETUP_REQUEST,
  USB_DEVICE_EVENT_CONTROL_TRANSFER_DATA_SENT,
  USB_DEVICE_EVENT_ENDPOINT_READ_COMPLETE,
  USB_DEVICE_EVENT_ENDPOINT_WRITE_COMPLETE,
  USB_DEVICE_EVENT_SET_DESCRIPTOR,
  USB_DEVICE_EVENT_SYNCH_FRAME
} USB_DEVICE_EVENT;

typedef enum {
  USB_DEVICE_CONTROL_TRANSFER_RESULT_FAILED =
      USB_ERROR_DEVICE_CONTROL_TRANSFER_FAILED,
  USB_DEVICE_CONTROL_TRANSFER_RESULT_SUCCESS = USB_ERROR_NONE
} USB_DEVICE_CONTROL_TRANSFER_RESULT;

typedef enum {
  USB_DEVICE_CONTROL_STATUS_OK,
  USB_DEVICE_CONTROL_STATUS_ERROR
} USB_DEVICE_CONTROL_STATUS;

typedef enum {
    USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE = (1 << 0),
    USB_DEVICE_TRANSFER_FLAGS_MORE_DATA_PENDING = (1 << 1)
} USB_DEVICE_TRANSFER_FLAGS;

typedef union {
  uint16_t Val;
  uint8_t v[2];
  struct {
    uint8_t LB;
    uint8_t HB;
  } byte;
} USB_WORD_VAL;

typedef union __attribute__((packed)) {
  struct __attribute__((packed)) {
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
  };
  struct __attribute__((packed)) {
    unsigned :8;
    unsigned :8;
    USB_WORD_VAL W_Value;
    USB_WORD_VAL W_Index;
    USB_WORD_VAL W_Length;
  };
  struct __attribute__((packed)) {
    unsigned Recipient:5;
    unsigned RequestType:2;
    unsigned DataDir:1;
    unsigned :8;
    uint8_t bFeature;
    unsigned :8;
    unsigned :8;
    unsigned :8;
    unsigned :8;
    unsigned :8;
  };
  struct __attribute__((packed)) {
    union {
      uint8_t bmRequestType;
      struct {
        uint8_t    recipient:  5;
        uint8_t    type:       2;
        uint8_t    direction:  1;
      };
    } requestInfo;
  };
  struct __attribute__((packed)) {
    unsigned :8;
    unsigned :8;
    uint8_t bDscIndex;
    uint8_t bDescriptorType;
    uint16_t wLangID;
    unsigned :8;
    unsigned :8;
  };
  struct __attribute__((packed)) {
    unsigned :8;
    unsigned :8;
    uint8_t bDevADR;
    uint8_t bDevADRH;
    unsigned :8;
    unsigned :8;
    unsigned :8;
    unsigned :8;
  };
  struct __attribute__((packed)) {
    unsigned :8;
    unsigned :8;
    uint8_t bConfigurationValue;
    uint8_t bCfgRSD;
    unsigned :8;
    unsigned :8;
    unsigned :8;
    unsigned :8;
  };
  struct __attribute__((packed)) {
    unsigned :8;
    unsigned :8;
    uint8_t bAltID;
    uint8_t bAltID_H;
    uint8_t bIntfID;
    uint8_t bIntfID_H;
    unsigned :8;
    unsigned :8;
  };
  struct __attribute__((packed)) {
    unsigned :8;
    unsigned :8;
    unsigned :8;
    unsigned :8;
    uint8_t bEPID;
    uint8_t bEPID_H;
    unsigned :8;
    unsigned :8;
  };
  struct __attribute__((packed)) {
    unsigned :8;
    unsigned :8;
    unsigned :8;
    unsigned :8;
    unsigned EPNum:4;
    unsigned :3;
    unsigned EPDir:1;
    unsigned :8;
    unsigned :8;
    unsigned :8;
  };
} USB_SETUP_PACKET;

// Types
typedef uintptr_t USB_DEVICE_HANDLE;
typedef uintptr_t USB_DEVICE_TRANSFER_HANDLE;
typedef uint8_t USB_ENDPOINT_ADDRESS;
typedef void USB_DEVICE_EVENT_RESPONSE;

typedef struct {
  USB_DEVICE_TRANSFER_HANDLE transferHandle;
  size_t length;
}
USB_DEVICE_EVENT_DATA_ENDPOINT_READ_COMPLETE,
USB_DEVICE_EVENT_DATA_ENDPOINT_WRITE_COMPLETE;

typedef USB_DEVICE_EVENT_RESPONSE (*USB_DEVICE_EVENT_HANDLER) (
  USB_DEVICE_EVENT event,
  void * eventData,
  uintptr_t context
);

// Defines
#define USB_DEVICE_INDEX_0         0
#define USB_DEVICE_HANDLE_INVALID ((USB_DEVICE_HANDLE)(-1))

#define USB_REQUEST_GET_STATUS                  0
#define USB_REQUEST_CLEAR_FEATURE               1
#define USB_REQUEST_SET_FEATURE                 3
#define USB_REQUEST_SET_ADDRESS                 5
#define USB_REQUEST_GET_DESCRIPTOR              6
#define USB_REQUEST_SET_DESCRIPTOR              7
#define USB_REQUEST_GET_CONFIGURATION           8
#define USB_REQUEST_SET_CONFIGURATION           9
#define USB_REQUEST_GET_INTERFACE               10
#define USB_REQUEST_SET_INTERFACE               11
#define USB_REQUEST_SYNCH_FRAME                 12


// Functions
void USB_DEVICE_Attach(USB_DEVICE_HANDLE usbDeviceHandle);
void USB_DEVICE_Detach(USB_DEVICE_HANDLE usbDeviceHandle);

USB_DEVICE_CONTROL_TRANSFER_RESULT USB_DEVICE_ControlStatus(
    USB_DEVICE_HANDLE usbDeviceHandle,
    USB_DEVICE_CONTROL_STATUS status);

USB_DEVICE_CONTROL_TRANSFER_RESULT USB_DEVICE_ControlSend(
    USB_DEVICE_HANDLE usbDeviceHandle,
    void* data,
    size_t length);

USB_DEVICE_HANDLE USB_DEVICE_Open(
    const SYS_MODULE_INDEX instanceIndex,
    const DRV_IO_INTENT intent);

bool USB_DEVICE_EndpointIsEnabled(
    USB_DEVICE_HANDLE usbDeviceHandle,
    USB_ENDPOINT_ADDRESS endpoint);

USB_SPEED USB_DEVICE_ActiveSpeedGet(USB_DEVICE_HANDLE usbDeviceHandle);

void USB_DEVICE_EventHandlerSet(
    USB_DEVICE_HANDLE usbDeviceHandle,
    const USB_DEVICE_EVENT_HANDLER callBackFunc,
    uintptr_t context);

USB_DEVICE_RESULT USB_DEVICE_EndpointEnable(
    USB_DEVICE_HANDLE usbDeviceHandle,
    uint8_t interface,
    USB_ENDPOINT_ADDRESS endpoint,
    USB_TRANSFER_TYPE transferType,
    size_t size);

USB_DEVICE_RESULT USB_DEVICE_EndpointDisable(
    USB_DEVICE_HANDLE usbDeviceHandle,
    USB_ENDPOINT_ADDRESS endpoint);

void USB_DEVICE_EndpointStall(
    USB_DEVICE_HANDLE usbDeviceHandle,
    USB_ENDPOINT_ADDRESS endpoint);

USB_DEVICE_RESULT USB_DEVICE_EndpointRead(
    USB_DEVICE_HANDLE usbDeviceHandle,
    USB_DEVICE_TRANSFER_HANDLE * transferHandle,
    USB_ENDPOINT_ADDRESS endpoint,
    void* buffer,
    size_t bufferSize);

USB_DEVICE_RESULT USB_DEVICE_EndpointWrite(
    USB_DEVICE_HANDLE usbDeviceHandle,
    USB_DEVICE_TRANSFER_HANDLE * transferHandle,
    USB_ENDPOINT_ADDRESS endpoint,
    const void* data,
    size_t size,
    USB_DEVICE_TRANSFER_FLAGS flags);

#ifdef  __cplusplus
}
#endif

#endif  // TESTS_SYSTEM_CONFIG_USB_STUB_H_
