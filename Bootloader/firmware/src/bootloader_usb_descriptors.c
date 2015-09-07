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
 * bootloader_usb_descriptors.c
 * Copyright (C) 2015 Simon Newton
 */

#include "bootloader_usb_descriptors.h"

#include <stdint.h>

#include "dfu_spec.h"
#include "dfu_properties.h"
#include "uid.h"
#include "usb/usb_device_cdc.h"
#include "usb_properties.h"

// USB Device Layer Function Driver Registration Table
// ----------------------------------------------------------------------------
static const USB_DEVICE_FUNCTION_REGISTRATION_TABLE g_func_table[1] = {
  /* Function 3 - The DFU Interface */
  {
    .configurationValue = 1,
    .interfaceNumber = 0,
    .numberOfInterfaces = 1,
    .speed = USB_SPEED_FULL,
    .funcDriverIndex = 0,
    .driver = NULL,
    .funcDriverInit = NULL  // No function driver
  },
};

// USB Device Layer Descriptors
// ----------------------------------------------------------------------------
static const USB_DEVICE_DESCRIPTOR g_device_descriptor = {
  0x12, // Size of this descriptor in bytes
  USB_DESCRIPTOR_DEVICE, // DEVICE descriptor type
  0x0100, // USB Spec Release Number in BCD format
  0x00, // Class Code
  0x00, // Subclass code
  0x00, // Protocol code
  DFU_BLOCK_SIZE, // Max packet size for EP0
  USB_DEVICE_VENDOR_ID, // Vendor ID.
  USB_DEVICE_BOOTLOADER_PRODUCT_ID, // Product ID.
  0x0000, // Device release number in BCD format
  0x01, // Manufacturer string index
  0x02, // Product string index
  0x03, // Device serial number string index
  0x01 // Number of possible configurations
};

// Device Configuration Decriptor
// ----------------------------------------------------------------------------
static const uint8_t g_config_descriptor[] = {
  // Configuration Descriptor Header
  0x09, // Size of this descriptor in bytes
  USB_DESCRIPTOR_CONFIGURATION, // CONFIGURATION descriptor type
  0x24, 0x00, // Total length of data for this cfg
  1, // Number of interfaces in this cfg
  1, // Index value of this configuration
  0, // Configuration string index
  USB_ATTRIBUTE_DEFAULT | USB_ATTRIBUTE_SELF_POWERED, // Attributes
  50, // Max power consumption (2X mA)

  // DFU Interface Descriptor
  0x09, // Size of this descriptor in bytes
  USB_DESCRIPTOR_INTERFACE, // Descriptor Type
  DFU_MODE_DFU_INTERFACE_INDEX, // Interface Number
  DFU_ALT_INTERFACE_FIRMWARE, // Alternate Setting Number
  0x00, // Number of endpoints in this interface
  0xfe, // Class code
  0x01, // Subclass code
  0x02, // Protocol code
  0x04, // Interface string index

  0x09, // Size of this descriptor in bytes
  USB_DESCRIPTOR_INTERFACE, // Descriptor Type
  DFU_MODE_DFU_INTERFACE_INDEX, // Interface Number
  DFU_ALT_INTERFACE_UID, // Alternate Setting Number
  0x00, // Number of endpoints in this interface
  0xfe, // Class code
  0x01, // Subclass code
  0x02, // Protocol code
  0x05, // Interface string index

  // DFU functional descriptor
  0x09,  // size
  0x21,  // DFU functional descriptor
  DFU_MANIFESTATION_TOLERANT | DFU_CAN_DOWNLOAD,  // download capable
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
  uint16_t string[1];
} g_language_string_descriptor = {
  sizeof(g_language_string_descriptor),
  USB_DESCRIPTOR_STRING, {
    0x0409
  }
};

// Manufacturer string descriptor [1]
static const struct {
  uint8_t bLength;
  uint8_t bDscType;
  uint16_t string[21];
} g_manufacturer_string_descriptor = {
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
  uint16_t string[18];
} g_product_string_descriptor = {
  sizeof(g_product_string_descriptor),
  USB_DESCRIPTOR_STRING, {
    'J', 'a', ' ', 'R', 'u', 'l', 'e', ' ', 'B', 'o', 'o', 't', 'l', 'o', 'a',
    'd', 'e', 'r'
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

// Firmware interface string descriptor [4]
static const struct {
  uint8_t bLength;
  uint8_t bDscType;
  uint16_t string[8];
} g_firmware_interface_descriptor = {
  sizeof(g_firmware_interface_descriptor),
  USB_DESCRIPTOR_STRING, {
    'F','i', 'r', 'm', 'w', 'a', 'r', 'e'
  }
};

// UID interface string descriptor [5]
static const struct {
  uint8_t bLength;
  uint8_t bDscType;
  uint16_t string[3];
} g_uid_interface_descriptor = {
  sizeof(g_uid_interface_descriptor),
  USB_DESCRIPTOR_STRING, {
    'U', 'I', 'D'
  }
};

// Array of string descriptors
static const USB_DEVICE_STRING_DESCRIPTORS_TABLE g_string_descriptors[6] = {
  (const uint8_t * const) &g_language_string_descriptor,
  (const uint8_t * const) &g_manufacturer_string_descriptor,
  (const uint8_t * const) &g_product_string_descriptor,
  (const uint8_t * const) &g_serial_number_string_descriptor,
  (const uint8_t * const) &g_firmware_interface_descriptor,
  (const uint8_t * const) &g_uid_interface_descriptor
};

// Array of full speed config descriptors
// ----------------------------------------------------------------------------
static const USB_DEVICE_CONFIGURATION_DESCRIPTORS_TABLE g_config_desc_set[1] = {
  g_config_descriptor
};

// USB Device Layer Master Descriptor Table
// ----------------------------------------------------------------------------
static const USB_DEVICE_MASTER_DESCRIPTOR usbMasterDescriptor = {
  &g_device_descriptor, // Full Speed Device Descriptor.
  1, // Total number of full speed configurations available.
  &g_config_desc_set[0],

  NULL, // High speed device desc is not supported.
  0, // Total number of high speed configurations available.
  NULL, // Pointer to array of high speed configurations descriptors.

  6, // Total number of string descriptors available.
  g_string_descriptors, // Pointer to array of string descriptors

  NULL, // Pointer to full speed dev qualifier.
  NULL, // Pointer to high speed dev qualifier.
};

// Endpoint Table needed by the Device Layer.
// ----------------------------------------------------------------------------
static uint8_t __attribute__((aligned(512)))
    g_endpoint_table[USB_DEVICE_ENDPOINT_TABLE_SIZE];

// USB Device Layer Initialization Data
// ----------------------------------------------------------------------------
const USB_DEVICE_INIT usbDevInitData = {
  .moduleInit = {SYS_MODULE_POWER_RUN_FULL},
  .stopInIdle = false,
  .suspendInSleep = false,
  .endpointTable= g_endpoint_table,
  .registeredFuncCount = 1,
  .registeredFunctions = (USB_DEVICE_FUNCTION_REGISTRATION_TABLE*) g_func_table,
  .usbMasterDescriptor = (USB_DEVICE_MASTER_DESCRIPTOR*) &usbMasterDescriptor,
  .deviceSpeed = USB_SPEED_FULL,
  .queueSizeEndpointRead = 1,
  .queueSizeEndpointWrite= 1,
};

const USB_DEVICE_INIT* BootloaderUSBDescriptor_GetDeviceConfig() {
  return &usbDevInitData;
}

uint16_t* BootloaderUSBDescriptor_UnicodeUID() {
  return g_serial_number_string_descriptor.string;
}
