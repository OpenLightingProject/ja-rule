/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * usb_descriptors.c
 * Copyright (C) 2015 Simon Newton
 */

#include "usb_descriptors.h"

#include <stdint.h>

#include "app_settings.h"
#include "constants.h"
#include "dfu_spec.h"
#include "dfu_properties.h"
#include "uid.h"
#include "usb/usb_device_cdc.h"
#include "usb_properties.h"

// CDC Function Driver Init Data
// ----------------------------------------------------------------------------
static const USB_DEVICE_CDC_INIT g_cdc_init0 = {
  .queueSizeRead = 1,
  .queueSizeWrite = 1,
  .queueSizeSerialStateNotification = 1
};

// USB Device Layer Function Driver Registration Table
// ----------------------------------------------------------------------------
static const USB_DEVICE_FUNCTION_REGISTRATION_TABLE g_func_table[3] = {
  /* Function 1 - CDC (serial port) */
  {
    .configurationValue = 1,
    .interfaceNumber = 0,
    .numberOfInterfaces = 2,
    .speed = USB_SPEED_FULL,
    .funcDriverIndex = 0,
    .driver = (void*) USB_DEVICE_CDC_FUNCTION_DRIVER,
    .funcDriverInit = (void*) &g_cdc_init0
  },
  /* Function 2 - The Ja Rule Interface */
  {
    .configurationValue = 1,
    .interfaceNumber = 2,
    .numberOfInterfaces = 1,
    .speed = USB_SPEED_FULL,
    .funcDriverIndex = 0,
    .driver = NULL,  // No function driver
    .funcDriverInit = NULL
  },
  /* Function 3 - The DFU Interface */
  {
    .configurationValue = 1,
    .interfaceNumber = RUNTIME_MODE_DFU_INTERFACE_INDEX,
    .numberOfInterfaces = 1,
    .speed = USB_SPEED_FULL,
    .funcDriverIndex = 0,
    .driver = NULL,  // No function driver
    .funcDriverInit = NULL
  },
};

// USB Device Layer Descriptors
// ----------------------------------------------------------------------------
static const USB_DEVICE_DESCRIPTOR g_device_descriptor = {
  0x12,  // Size of this descriptor in bytes
  USB_DESCRIPTOR_DEVICE,  // DEVICE descriptor type
  0x0200,  // USB Spec Release Number in BCD format
  0x00,  // Class Code
  0x00,  // Subclass code
  0x00,  // Protocol code
  USB_DEVICE_EP0_BUFFER_SIZE,  // Max packet size for EP0, see usb_config.h
  USB_DEVICE_VENDOR_ID,
  USB_DEVICE_MAIN_PRODUCT_ID,
  0x0000,  // Device release number in BCD format
  0x01,  // Manufacturer string index
  0x02,  // Product string index
  0x03,  // Device serial number string index
  0x01  // Number of possible configurations
};

// Device Configuration Decriptor
// ----------------------------------------------------------------------------
static const uint8_t g_config_descriptor[] = {
  // Configuration Descriptor Header
  0x09,  // Size of this descriptor
  USB_DESCRIPTOR_CONFIGURATION,  // Descriptor type
  0x6c, 0x00,  // Total length of data for this cfg
  4,  // Number of interfaces in this cfg
  1,  // Index value of this configuration
  0,  // Configuration string index
  USB_ATTRIBUTE_DEFAULT | USB_ATTRIBUTE_SELF_POWERED,  // Attributes
  USB_POWER_CONSUMPTION,  // Max power consumption

  // First CDC Interface Descriptor
  0x09,  // Size of this descriptor in bytes
  USB_DESCRIPTOR_INTERFACE,  // Descriptor type
  0x00,  // Interface Number
  0x00,  // Alternate Setting Number
  0x01,  // Number of endpoints in this intf
  USB_CDC_COMMUNICATIONS_INTERFACE_CLASS_CODE,  // Class code
  USB_CDC_SUBCLASS_ABSTRACT_CONTROL_MODEL,  // Subclass code
  USB_CDC_PROTOCOL_AT_V250,  // Protocol code
  0x00,  // Interface string index

  /* CDC Class-Specific Descriptors */
  sizeof(USB_CDC_HEADER_FUNCTIONAL_DESCRIPTOR),  // Size of the descriptor
  USB_CDC_DESC_CS_INTERFACE,  // CS_INTERFACE
  USB_CDC_FUNCTIONAL_HEADER,  // Type of functional descriptor
  0x20, 0x01,  // CDC spec version

  sizeof(USB_CDC_ACM_FUNCTIONAL_DESCRIPTOR),  // Size of the descriptor
  USB_CDC_DESC_CS_INTERFACE,  // CS_INTERFACE
  USB_CDC_FUNCTIONAL_ABSTRACT_CONTROL_MANAGEMENT,  // Descriptor type
  USB_CDC_ACM_SUPPORT_LINE_CODING_LINE_STATE_AND_NOTIFICATION,  // Capabilities

  sizeof(USB_CDC_UNION_FUNCTIONAL_DESCRIPTOR_HEADER) + 1,  // Size
  USB_CDC_DESC_CS_INTERFACE,  // CS_INTERFACE
  USB_CDC_FUNCTIONAL_UNION,  // Type of functional descriptor
  0x00,  // Communication interface number
  0x01,  // Data Interface Number

  sizeof(USB_CDC_CALL_MANAGEMENT_DESCRIPTOR),  // Size of the descriptor
  USB_CDC_DESC_CS_INTERFACE,  // CS_INTERFACE
  USB_CDC_FUNCTIONAL_CALL_MANAGEMENT,  // Type of functional descriptor
  0x00,  // bmCapabilities of CallManagement
  0x01,  // Data interface number

  // CDC Interrupt Endpoint (IN) Descriptor
  0x07,  // Size of this descriptor
  USB_DESCRIPTOR_ENDPOINT,  // Endpoint Descriptor
  0x2 | USB_EP_DIRECTION_IN,  // EndpointAddress ( EP2 IN INTERRUPT)
  USB_TRANSFER_TYPE_INTERRUPT,  // Attributes type of EP (INTERRUPT)
  0x0A, 0x00,  // Max packet size of this EP
  0x02,  // Poll interval (in ms)

  // Second CDC Interface Descriptor
  0x09,  // Size of this descriptor in bytes
  USB_DESCRIPTOR_INTERFACE,  // Descriptor type
  0x01,  // Interface Number
  0x00,  // Alternate Setting Number
  0x02,
  USB_CDC_DATA_INTERFACE_CLASS_CODE,  // Class code
  0x00,  // Subclass code
  USB_CDC_PROTOCOL_NO_CLASS_SPECIFIC,  // Protocol code
  0x00,  // Interface string index

  // CDC Interrupt Endpoint (IN) Descriptor
  0x07,  // Size of this descriptor
  USB_DESCRIPTOR_ENDPOINT,  // Descriptor type
  0x3 | USB_EP_DIRECTION_OUT,  // EndpointAddress ( EP3 OUT BULK)
  USB_TRANSFER_TYPE_BULK,  // Attributes type of EP (BULK)
  0x40, 0x00,  // Max packet size of this EP
  0x00,  // Interval (in ms)

  // CDC Interrupt Endpoint (OUT) Descriptor
  0x07,  // Size of this descriptor
  USB_DESCRIPTOR_ENDPOINT,  // Descriptor type
  0x3 | USB_EP_DIRECTION_IN,  // EndpointAddress ( EP3 IN )
  USB_TRANSFER_TYPE_BULK,  // Attributes type of EP (BULK)
  0x40, 0x00,  // Max packet size of this EP
  0x00,  // Interval (in ms)

  // Ja Rule Interface Descriptor
  0x09,  // Size of this descriptor in bytes
  USB_DESCRIPTOR_INTERFACE,  // Descriptor type
  2,  // Interface Number
  0,  // Alternate Setting Number
  2,  // Number of endpoints in this intf
  0xFF,  // Class code
  0xFF,  // Subclass code
  0xFF,  // Protocol code
  0,  // Interface string index

  // Ja Rule Bulk Endpoint (OUT) Descriptor
  0x07,  // Size of this descriptor in bytes
  USB_DESCRIPTOR_ENDPOINT,  // Descriptor type
  0x1 | USB_EP_DIRECTION_OUT,  // EndpointAddress
  USB_TRANSFER_TYPE_BULK,  // Attributes
  USB_MAX_PACKET_SIZE, 0x00,  // Size
  USB_POLLING_INTERVAL,  // Interval

  // Ja Rule Bulk Endpoint (IN) Descriptor
  0x07,  // Size of this descriptor in bytes
  USB_DESCRIPTOR_ENDPOINT,  // Descriptor type
  0x1 | USB_EP_DIRECTION_IN,  // EndpointAddress
  USB_TRANSFER_TYPE_BULK,  // Attributes
  USB_MAX_PACKET_SIZE, 0x00,  // Size
  USB_POLLING_INTERVAL,  // Interval

  // DFU Interface Descriptor
  0x09,  // Size of this descriptor in bytes
  USB_DESCRIPTOR_INTERFACE,  // Descriptor Type
  RUNTIME_MODE_DFU_INTERFACE_INDEX,  // Interface Number
  0x00,  // Alternate Setting Number
  0x00,  // Number of endpoints in this intf
  0xfe,  // Class code
  0x01,  // Subclass code
  0x01,  // Protocol code
  0x00,  // Interface string index

  // DFU functional descriptor
  0x09,  // size
  0x21,  // DFU functional descriptor
  DFU_WILL_DETACH | DFU_MANIFESTATION_TOLERANT | DFU_CAN_DOWNLOAD,
  0x00, 0x00,  // detatch timeout
  DFU_BLOCK_SIZE, 0x00,  // transfer size
  0x01, 0x10   // Rev 1.1
};

//  String descriptors.
// ----------------------------------------------------------------------------

// Language code string descriptor [0]
static const struct {
  uint8_t bLength;
  uint8_t bDscType;
  uint16_t string;
} g_language_string_descriptor = {
  sizeof(g_language_string_descriptor),
  USB_DESCRIPTOR_STRING,
  0x0409
};

// Manufacturer string descriptor [1]
static const struct {
  uint8_t bLength;
  uint8_t bDscType;
  uint16_t string[21];
}

g_manufacturer_string_descriptor = {
  sizeof(g_manufacturer_string_descriptor),
  USB_DESCRIPTOR_STRING, {
    'O', 'p', 'e', 'n', ' ', 'L', 'i', 'g', 'h', 't', 'i', 'n', 'g', ' ',
    'P', 'r', 'o', 'j', 'e', 'c', 't'
  }
};

// Product string descriptor [2]
static const struct {
  uint8_t bLength;
  uint8_t bDscType;
  uint16_t string[7];
} g_product_string_descriptor = {
  sizeof(g_product_string_descriptor),
  USB_DESCRIPTOR_STRING, {
    'J', 'a', ' ', 'R', 'u', 'l', 'e'
  }
};

/*
 * Serial number string descriptor [3]
 * This is populated from the UID in flash memory so it can't be const.
 */
static struct {
  uint8_t bLength;
  uint8_t bDscType;
  uint16_t string[UID_LENGTH * 2 + 1];
} g_serial_number_string_descriptor = {
  sizeof(g_serial_number_string_descriptor),
  USB_DESCRIPTOR_STRING, {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  }
};

// Array of string descriptors
static const USB_DEVICE_STRING_DESCRIPTORS_TABLE g_string_descriptors[] = {
  (const uint8_t* const) &g_language_string_descriptor,
  (const uint8_t* const) &g_manufacturer_string_descriptor,
  (const uint8_t* const) &g_product_string_descriptor,
  (const uint8_t* const) &g_serial_number_string_descriptor,
};

// Array of full speed config descriptors
// ----------------------------------------------------------------------------
static const USB_DEVICE_CONFIGURATION_DESCRIPTORS_TABLE g_config_desc_set[1] = {
  g_config_descriptor
};

// USB Device Layer Master Descriptor Table
// ----------------------------------------------------------------------------
static const USB_DEVICE_MASTER_DESCRIPTOR g_usb_master_descriptor = {
  &g_device_descriptor,
  1,  // Total number of full speed configurations available.
  &g_config_desc_set[0],

  NULL,  // High speed device is not supported.
  0,  // Total number of high speed configurations available.
  NULL,  // Pointer to array of high speed configurations descriptors.

  4,  // Total number of string descriptors available.
  g_string_descriptors,  // Pointer to array of string descriptors

  NULL,  // Pointer to full speed dev qualifier.
  NULL,  // Pointer to high speed dev qualifier.
};

// Endpoint Table needed by the Device Layer.
// ----------------------------------------------------------------------------
static uint8_t __attribute__((aligned(512)))
    g_endpoint_table[USB_DEVICE_ENDPOINT_TABLE_SIZE];

// USB Device Layer Initialization Data
// ----------------------------------------------------------------------------
static const USB_DEVICE_INIT g_usb_device_config = {
  .moduleInit = {SYS_MODULE_POWER_RUN_FULL},
  .usbID = USB_ID_1,
  .stopInIdle = false,
  .suspendInSleep = false,
  .interruptSource = INT_SOURCE_USB_1,
  .endpointTable = g_endpoint_table,
  .registeredFuncCount = 3,  // Must match the size of the g_func_table
  .registeredFunctions = (USB_DEVICE_FUNCTION_REGISTRATION_TABLE*) g_func_table,
  .usbMasterDescriptor =
      (USB_DEVICE_MASTER_DESCRIPTOR*) &g_usb_master_descriptor,
  .deviceSpeed = USB_SPEED_FULL,
  .queueSizeEndpointRead = 1,
  .queueSizeEndpointWrite = 1,
};


uint16_t* USBDescriptor_UnicodeUID() {
  return g_serial_number_string_descriptor.string;
}

const USB_DEVICE_INIT* USBDescriptor_GetDeviceConfig() {
  return &g_usb_device_config;
}
