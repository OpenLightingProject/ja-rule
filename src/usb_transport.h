/*
 * File:   usb_transport.h
 * Author: Simon Newton
 */

/**
 * @defgroup usb_transport USB Transport
 * @brief A USB Transport.
 *
 * A implementation of the generic transport that uses USB. The PIC acts as a
 * custome USB device.
 *
 * @addtogroup usb_transport
 * @{
 * @file usb_transport.h
 * @brief A USB Transport
 */

#ifndef SRC_USB_TRANSPORT_H_
#define SRC_USB_TRANSPORT_H_

#include <stdbool.h>
#include <stdint.h>

#include "constants.h"
#include "transport.h"

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
void USBTransport_Initialize(RXFunction rx_cb);

/**
 * @brief Perform the periodic USB layer tasks.
 *
 * This must be called within the main event loop.
 */
void USBTransport_Tasks();

/**
 * @brief Send a response to the Host.
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
bool USBTransport_SendResponse(Command command, uint8_t rc, const IOVec* data,
                               unsigned int iov_count);

/**
 * @brief Check if there is a write in progress
 */
bool USBTransport_WritePending();

#ifdef __cplusplus
}
#endif

#endif  // SRC_USB_TRANSPORT_H_

/**
 * @}
 */
