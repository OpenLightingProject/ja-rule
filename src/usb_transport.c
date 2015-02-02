/*
 * File:   usb_transport.c
 * Author: Simon Newton
 */

#include "usb_transport.h"

#include <string.h>

#include "constants.h"
#include "flags.h"
#include "logger.h"
#include "stream_decoder.h"
#include "system_config.h"
#include "system_definitions.h"
#include "system_pipeline.h"
#include "transport.h"

typedef enum {
  USB_STATE_INIT = 0,
  USB_STATE_WAIT_FOR_CONFIGURATION,
  USB_STATE_MAIN_TASK,
  USB_STATE_ERROR
} USBTransportState;

typedef struct {
  TransportRxFunction rx_cb;
  USB_DEVICE_HANDLE usb_device;  //!< The USB Device layer handle.
  USBTransportState state;
  bool is_configured;  //!< Keep track of if the device is configured.

  bool tx_in_progress;  //!< True if there is a TX in progress
  bool rx_in_progress;  //!< True if there is a RX in progress.

  USB_DEVICE_TRANSFER_HANDLE write_transfer;
  USB_DEVICE_TRANSFER_HANDLE read_transfer;

  /* The transmit endpoint address */
  USB_ENDPOINT_ADDRESS endpointTx;

  /* The receive endpoint address */
  USB_ENDPOINT_ADDRESS endpointRx;

  /* Tracks the alternate setting */
  uint8_t altSetting;

  int rx_data_size;
} USBTransportData;

USBTransportData g_usb_transport_data;

/* Receive data buffer */
uint8_t receivedDataBuffer[USB_READ_BUFFER_SIZE] USB_MAKE_BUFFER_DMA_READY;

/* Transmit data buffer */
uint8_t transmitDataBuffer[USB_READ_BUFFER_SIZE] USB_MAKE_BUFFER_DMA_READY;

/**
 * @brief Called when device events occur.
 * @param event The type of event
 * @param event_data Data associated with the event
 * @param context The context (unused).
 */
void USBTransport_EventHandler(USB_DEVICE_EVENT event, void* event_data,
                               uintptr_t context) {
  uint8_t* configurationValue;
  USB_SETUP_PACKET* setupPacket;

  switch (event) {
    case USB_DEVICE_EVENT_RESET:
    case USB_DEVICE_EVENT_DECONFIGURED:
      g_usb_transport_data.is_configured = false;
      break;

    case USB_DEVICE_EVENT_CONFIGURED:
      // Check the configuration
      configurationValue = (uint8_t*) event_data;
      if (*configurationValue == 1) {
        // Reset endpoint data send & receive flag
        g_usb_transport_data.is_configured = true;
      }
      break;

    case USB_DEVICE_EVENT_SUSPENDED:
      /* Device is suspended. Update LED indication */
      break;

    case USB_DEVICE_EVENT_POWER_DETECTED:
      /* VBUS is detected. Attach the device */
      USB_DEVICE_Attach(g_usb_transport_data.usb_device);
      break;

    case USB_DEVICE_EVENT_POWER_REMOVED:
      /* VBUS is removed. Detach the device */
      USB_DEVICE_Detach(g_usb_transport_data.usb_device);
      break;

    case USB_DEVICE_EVENT_CONTROL_TRANSFER_SETUP_REQUEST:
      /* This means we have received a setup packet */
      setupPacket = (USB_SETUP_PACKET*) event_data;
      if (setupPacket->bRequest == USB_REQUEST_SET_INTERFACE) {
        /* If we have got the SET_INTERFACE request, we just acknowledge
         for now. This demo has only one alternate setting which is already
         active. */
        USB_DEVICE_ControlStatus(g_usb_transport_data.usb_device, USB_DEVICE_CONTROL_STATUS_OK);
      } else if (setupPacket->bRequest == USB_REQUEST_GET_INTERFACE) {
        /* We have only one alternate setting and this setting 0. So
         * we send this information to the host. */

        USB_DEVICE_ControlSend(g_usb_transport_data.usb_device, &g_usb_transport_data.altSetting, 1);
      } else {
        /* We have received a request that we cannot handle. Stall it*/
        USB_DEVICE_ControlStatus(g_usb_transport_data.usb_device, USB_DEVICE_CONTROL_STATUS_ERROR);
      }
      break;

    case USB_DEVICE_EVENT_ENDPOINT_READ_COMPLETE:
      /* Endpoint read is complete */
      g_usb_transport_data.rx_in_progress = false;
      g_usb_transport_data.rx_data_size = ((USB_DEVICE_EVENT_DATA_ENDPOINT_WRITE_COMPLETE*) event_data)->length;
      break;

    case USB_DEVICE_EVENT_ENDPOINT_WRITE_COMPLETE:
      /* Endpoint write is complete */
      g_usb_transport_data.tx_in_progress = false;
      break;

      /* These events are not used in this demo. */
    case USB_DEVICE_EVENT_RESUMED:
    case USB_DEVICE_EVENT_ERROR:
    default:
      break;
  }
  (void) context;
}

void USBTransport_Initialize(TransportRxFunction rx_cb) {
  g_usb_transport_data.rx_cb = rx_cb;
  g_usb_transport_data.state = USB_STATE_INIT;
  g_usb_transport_data.usb_device = USB_DEVICE_HANDLE_INVALID;
  g_usb_transport_data.is_configured = false;
  g_usb_transport_data.endpointRx = 0x01;
  g_usb_transport_data.endpointTx = 0x81;
  g_usb_transport_data.rx_in_progress = false;
  g_usb_transport_data.tx_in_progress = false;
  g_usb_transport_data.altSetting = 0;
  g_usb_transport_data.rx_data_size = 0;
}

void USBTransport_Tasks() {
  uint16_t endpointSize = 64;
  switch (g_usb_transport_data.state) {
    case USB_STATE_INIT:
      // Try to open the device layer.
      g_usb_transport_data.usb_device = USB_DEVICE_Open(
          USB_DEVICE_INDEX_0, DRV_IO_INTENT_READWRITE);
      if (g_usb_transport_data.usb_device != USB_DEVICE_HANDLE_INVALID) {
        // Register a callback with device layer to get event notification
        // for end point 0.
        USB_DEVICE_EventHandlerSet(g_usb_transport_data.usb_device,
                                   USBTransport_EventHandler, 0);

        g_usb_transport_data.state = USB_STATE_WAIT_FOR_CONFIGURATION;
      } else {
        // The Device Layer is not ready yet. Try again later.
      }
      break;

    case USB_STATE_WAIT_FOR_CONFIGURATION:
      if (g_usb_transport_data.is_configured == true) {
        if (USB_DEVICE_ActiveSpeedGet(g_usb_transport_data.usb_device) == USB_SPEED_FULL) {
          endpointSize = 64;
        } else if (USB_DEVICE_ActiveSpeedGet(g_usb_transport_data.usb_device) == USB_SPEED_HIGH) {
          endpointSize = 512;
        }
        if (USB_DEVICE_EndpointIsEnabled(g_usb_transport_data.usb_device, g_usb_transport_data.endpointRx) == false) {
          /* Enable Read Endpoint */
          USB_DEVICE_EndpointEnable(g_usb_transport_data.usb_device, 0, g_usb_transport_data.endpointRx,
                                    USB_TRANSFER_TYPE_BULK, endpointSize);
        }
        if (USB_DEVICE_EndpointIsEnabled(g_usb_transport_data.usb_device, g_usb_transport_data.endpointTx) == false) {
          /* Enable Write Endpoint */
          USB_DEVICE_EndpointEnable(g_usb_transport_data.usb_device, 0, g_usb_transport_data.endpointTx,
                                    USB_TRANSFER_TYPE_BULK, endpointSize);
        }
        /* Indicate that we are waiting for read */
        g_usb_transport_data.rx_in_progress = true;

        /* Place a new read request. */
        USB_DEVICE_RESULT result = USB_DEVICE_EndpointRead(
                                                           g_usb_transport_data.usb_device, &g_usb_transport_data.read_transfer,
                                                           g_usb_transport_data.endpointRx, &receivedDataBuffer[0], sizeof (receivedDataBuffer));

        (void) result;

        /* Device is ready to run the main task */
        g_usb_transport_data.state = USB_STATE_MAIN_TASK;
      }
      break;

    case USB_STATE_MAIN_TASK:
      if (!g_usb_transport_data.is_configured) {
        /* This means the device got deconfigured. Change the
         * application state back to waiting for configuration. */
        g_usb_transport_data.state = USB_STATE_WAIT_FOR_CONFIGURATION;

        // Disable the endpoints
        USB_DEVICE_EndpointDisable(g_usb_transport_data.usb_device,
                                   g_usb_transport_data.endpointRx);
        USB_DEVICE_EndpointDisable(g_usb_transport_data.usb_device,
                                   g_usb_transport_data.endpointTx);
        g_usb_transport_data.rx_in_progress = false;
        g_usb_transport_data.tx_in_progress = false;
      } else if (g_usb_transport_data.rx_in_progress == false) {
        /* Look at the data the host sent, to see what kind of
         * application specific command it sent. */


        if (g_usb_transport_data.tx_in_progress == false) {
          // we only go ahead and process the data if we can respond.
#ifdef PIPELINE_TRANSPORT_TX
          PIPELINE_TRANSPORT_RX(receivedDataBuffer,
                                g_usb_transport_data.rx_data_size);
#else
          g_usb_transport_data.rx_cb(receivedDataBuffer,
                                     g_usb_transport_data.rx_data_size);
#endif

          //USB_DEVICE_EndpointStall(g_usb_transport_data.usb_device,
          //                         g_usb_transport_data.endpointRx);
          // schedule the next read
          g_usb_transport_data.rx_in_progress = true;
          USB_DEVICE_EndpointRead(g_usb_transport_data.usb_device,
                                  &g_usb_transport_data.read_transfer,
                                  g_usb_transport_data.endpointRx,
                                  &receivedDataBuffer[0],
                                  sizeof (receivedDataBuffer));
        }
      }
      break;

    case USB_STATE_ERROR:
      break;
    default:
      break;
  }
}

bool USBTransport_SendResponse(Command command, uint8_t rc, const IOVec* data,
                               unsigned int iov_count) {
  if (g_usb_transport_data.tx_in_progress ||
      !g_usb_transport_data.is_configured) {
    return false;
  }

  transmitDataBuffer[0] = START_OF_MESSAGE_ID;
  transmitDataBuffer[1] = command & 0xff;
  transmitDataBuffer[2] = command >> 8;
  // 3 & 4 are the length.
  transmitDataBuffer[5] = rc;

  // Set appropriate flags.
  transmitDataBuffer[6] = 0;
  if (Logger_DataPending()) {
    transmitDataBuffer[6] |= TRANSPORT_LOGS_PENDING;
  }
  if (Flags_HasChanged()) {
    transmitDataBuffer[6] |= TRANSPORT_FLAGS_CHANGED;
  }

  unsigned int i = 0;
  uint16_t offset = 0;
  for (; i != iov_count; i++) {
    if (offset + data[i].length > PAYLOAD_SIZE) {
      memcpy(transmitDataBuffer + offset + 7, data[i].base,
             PAYLOAD_SIZE - offset);
      offset = PAYLOAD_SIZE;
      transmitDataBuffer[6] |= TRANSPORT_MSG_TRUNCATED;
      break;
    } else {
      memcpy(transmitDataBuffer + offset + 7, data[i].base, data[i].length);
      offset += data[i].length;
    }
  }

  transmitDataBuffer[3] = offset & 0xf;
  transmitDataBuffer[4] = offset >> 8;
  transmitDataBuffer[7 + offset] = END_OF_MESSAGE_ID;

  g_usb_transport_data.tx_in_progress = true;

  USB_DEVICE_RESULT result = USB_DEVICE_EndpointWrite(
      g_usb_transport_data.usb_device,
      &g_usb_transport_data.write_transfer,
      g_usb_transport_data.endpointTx, transmitDataBuffer,
      offset + 8,
      USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE);
  if (result != USB_DEVICE_RESULT_OK) {
    g_usb_transport_data.tx_in_progress = false;
  }
  return result == USB_DEVICE_RESULT_OK;
}

bool USBTransport_WritePending() {
  return g_usb_transport_data.tx_in_progress;
}
