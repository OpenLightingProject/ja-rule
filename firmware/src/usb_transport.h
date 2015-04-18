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
 * usb_transport.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @defgroup usb_transport USB Transport
 * @brief A USB Transport.
 *
 * A implementation of the generic transport that uses USB. The PIC acts as an
 * custom USB device.
 *
 * @addtogroup usb_transport
 * @{
 * @file usb_transport.h
 * @brief A USB Transport
 */

#ifndef FIRMWARE_SRC_USB_TRANSPORT_H_
#define FIRMWARE_SRC_USB_TRANSPORT_H_

#include <stdbool.h>
#include <stdint.h>

#include "constants.h"
#include "transport.h"
#include "system_definitions.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the USB Transport.
 * @param rx_cb The function to call when data is received from the host. This
 *   can be overridden, see below.
 *
 * If PIPELINE_TRANSPORT_RX is defined in system_pipeline.h, the macro
 * will override the rx_cb argument.
 */
void USBTransport_Initialize(TransportRxFunction rx_cb);

/**
 * @brief Perform the periodic USB layer tasks.
 *
 * This must be called within the main event loop.
 */
void USBTransport_Tasks();

/**
 * @brief Send a response to the Host.
 * @param token The frame token, this should match the request.
 * @param command The command class of the response.
 * @param rc The return code of the response.
 * @param data The iovecs with the payload data.
 * @param iov_count The number of IOVecs.
 * @returns true if the message was queued for sending. False if the device was
 * not yet configured, or is there was already a message queued.
 *
 * Only one message can be sent at a time. Until the send completes, any
 * further messages will be dropped.
 */
bool USBTransport_SendResponse(uint8_t token, Command command, uint8_t rc,
                               const IOVec* data, unsigned int iov_count);

/**
 * @brief Check if there is a write in progress
 */
bool USBTransport_WritePending();

/**
 * @brief Return the USB Device handle.
 * @returns The device handle or USB_DEVICE_HANDLE_INVALID.
 */
USB_DEVICE_HANDLE USBTransport_GetHandle();

/**
 * @brief Check if the USB driver is configured.
 * @returns true if the device if configured, false otherwise.
 */
bool USBTransport_IsConfigured();

/**
 * @brief Perform a soft reset. This aborts any outbound (write) transfers
 */
void USBTransport_SoftReset();

#ifdef __cplusplus
}
#endif

#endif  // FIRMWARE_SRC_USB_TRANSPORT_H_

/**
 * @}
 */
