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
 * app.c
 * Copyright (C) 2015 Simon Newton
 */

#include "app.h"
#include "constants.h"
#include "dfu_constants.h"
#include "macros.h"

#include "system_config.h"

typedef enum {
  APP_STATE_INIT = 0,
  APP_STATE_WAIT_FOR_USB_CONFIGURATION,
  APP_STATE_DFU,
  APP_STATE_BOOT
} AppState;

typedef struct {
  USB_DEVICE_HANDLE usb_device;  //!< The USB Device layer handle.
  AppState state;
  DFUState dfu_state;
  DFUStatus dfu_status;
  uint8_t alternate_setting;
  bool is_configured;  //!< Keep track of whether the device is configured.
  bool has_new_firmware;
} AppData;

static uint8_t STATUS_RESPONSE[GET_STATUS_RESPONSE_SIZE];
static AppData g_app;
static uint8_t DATA_BUFFER[DFU_BLOCK_SIZE];

// Helper functions
// ----------------------------------------------------------------------------
/*
 * @brief Switch to the error state and stall the control pipe.
 */
static void StallAndError(DFUStatus status) {
  g_app.dfu_state = DFU_STATE_ERROR;
  g_app.dfu_status = status;
  USB_DEVICE_ControlStatus(g_app.usb_device, USB_DEVICE_CONTROL_STATUS_ERROR);
}

// DFU Handlers
// ----------------------------------------------------------------------------
static inline void DFUDownload(USB_SETUP_PACKET *packet) {
  if (g_app.dfu_state != DFU_STATE_IDLE &&
      g_app.dfu_state != DFU_STATE_DNLOAD_IDLE) {
    StallAndError(DFU_STATUS_ERR_STALLED_PKT);
    return;
  }

  if (g_app.dfu_state == DFU_STATE_IDLE && packet->wLength == 0u) {
    StallAndError(DFU_STATUS_ERR_STALLED_PKT);
    return;
  }

  if (packet->wLength > DFU_BLOCK_SIZE) {
    StallAndError(DFU_STATUS_ERR_STALLED_PKT);
    return;
  }

  if (packet->wLength) {
    USB_DEVICE_ControlReceive(g_app.usb_device, DATA_BUFFER, packet->wLength);
  } else {
    // TODO(simon): Verify image here
    g_app.dfu_state = DFU_STATE_MANIFEST_SYNC;
    g_app.has_new_firmware = true;
    USB_DEVICE_ControlStatus(g_app.usb_device, USB_DEVICE_CONTROL_STATUS_OK);
  }
}

static inline void DFUGetStatus() {
  // Some Get Status messages trigger a state change.
  // The status response always contains the *next* state, so figure that out
  // first.
  if (g_app.dfu_state == DFU_STATE_DNLOAD_SYNC) {
    g_app.dfu_state = DFU_STATE_DNLOAD_IDLE;
  } else if (g_app.dfu_state == DFU_STATE_MANIFEST_SYNC) {
    if (g_app.has_new_firmware) {
      g_app.dfu_state = DFU_STATE_MANIFEST;
    } else {
      g_app.dfu_state = DFU_STATE_IDLE;
    }
  }

  STATUS_RESPONSE[0] = g_app.dfu_status;
  STATUS_RESPONSE[1] = 0u;
  STATUS_RESPONSE[2] = 0u;
  STATUS_RESPONSE[3] = 0u;
  STATUS_RESPONSE[4] = g_app.dfu_state;
  STATUS_RESPONSE[5] = 0u;

  USB_DEVICE_ControlSend(g_app.usb_device, STATUS_RESPONSE,
                         GET_STATUS_RESPONSE_SIZE);
}

static inline void DFUClearStatus() {
  if (g_app.dfu_state == DFU_STATE_ERROR) {
    g_app.dfu_state = DFU_STATE_IDLE;
    g_app.dfu_status = DFU_STATUS_OK;
    USB_DEVICE_ControlStatus(g_app.usb_device, USB_DEVICE_CONTROL_STATUS_OK);
  } else {
    StallAndError(DFU_STATUS_ERR_STALLED_PKT);
  }
}

static inline void DFUGetState() {
  switch (g_app.dfu_state) {
    case APP_STATE_IDLE:
    case APP_STATE_DETACH:
    case DFU_STATE_IDLE:
    case DFU_STATE_DNLOAD_SYNC:
    case DFU_STATE_DNLOAD_IDLE:
    case DFU_STATE_MANIFEST_SYNC:
    case DFU_STATE_UPLOAD_IDLE:
    case DFU_STATE_ERROR:
      // TODO(simon): check the behavior is correct for DFU_STATE_DNLOAD_SYNC &
      // DFU_STATE_DNBUSY
      USB_DEVICE_ControlSend(g_app.usb_device, &g_app.dfu_state, 1);
      break;
    case DFU_STATE_DNBUSY:
    case DFU_STATE_MANIFEST:
    case DFU_STATE_MANIFEST_WAIT_RESET:
    default:
      StallAndError(DFU_STATUS_ERR_STALLED_PKT);
  }
}

static inline void DFUAbort() {
  switch (g_app.dfu_state) {
    case DFU_STATE_IDLE:
    case DFU_STATE_DNLOAD_SYNC:
    case DFU_STATE_DNLOAD_IDLE:
    case DFU_STATE_MANIFEST_SYNC:
    case DFU_STATE_UPLOAD_IDLE:
      g_app.dfu_state = DFU_STATE_IDLE;
      USB_DEVICE_ControlStatus(g_app.usb_device, USB_DEVICE_CONTROL_STATUS_OK);
      break;
    case APP_STATE_IDLE:
    case APP_STATE_DETACH:
    case DFU_STATE_DNBUSY:
    case DFU_STATE_MANIFEST:
    case DFU_STATE_MANIFEST_WAIT_RESET:
    case DFU_STATE_ERROR:
    default:
      StallAndError(DFU_STATUS_ERR_STALLED_PKT);
  }
}

static void HandleDFUEvent(USB_SETUP_PACKET *packet) {
  if (packet->DataDir == USB_SETUP_REQUEST_DIRECTION_DEVICE_TO_HOST) {
    // Device to Host.
    switch (packet->bRequest) {
      case DFU_GETSTATUS:
        DFUGetStatus();
        break;
      case DFU_GETSTATE:
        DFUGetState();
        break;
      default:
        // Unknown command, stall the pipe
        StallAndError(DFU_STATUS_ERR_STALLED_PKT);
    }
    return;
  } else {
    // Host to Device.
    switch (packet->bRequest) {
      case DFU_DNLOAD:
        DFUDownload(packet);
        break;
      case DFU_CLRSTATUS:
        DFUClearStatus();
        break;
      case DFU_ABORT:
        DFUAbort();
        break;
      default:
        // Unknown command, stall the pipe
        StallAndError(DFU_STATUS_ERR_STALLED_PKT);
    }
  }
}

static void DFUTransferComplete() {
  if (g_app.dfu_state == DFU_STATE_IDLE ||
      g_app.dfu_state == DFU_STATE_DNLOAD_IDLE) {
    // TODO(simon) do something with the data, we can switch to
    // DFU_STATE_DNBUSY here if we need time to process it.
    g_app.dfu_state = DFU_STATE_DNLOAD_SYNC;
    USB_DEVICE_ControlStatus(g_app.usb_device, USB_DEVICE_CONTROL_STATUS_OK);
  } else {
    StallAndError(DFU_STATUS_ERR_STALLED_PKT);
  }
}

/*
 * @brief The host aborted a control transfer.
 *
 * This is different from sending a DFU_ABORT command.
 */
static void DFUTransferAborted() {
  StallAndError(DFU_STATUS_ERR_STALLED_PKT);
}

/**
 * @brief Called when USB events occur.
 * @param event The type of event
 * @param event_data Data associated with the event
 * @param context The context (unused).
 *
 * This is called from the main event loop, since we're using polled mode USB.
 */
static void USBEventHandler(USB_DEVICE_EVENT event,
                            void* event_data,
                            UNUSED uintptr_t context) {
  uint8_t* configurationValue;
  USB_SETUP_PACKET* setupPacket;

  switch (event) {
    case USB_DEVICE_EVENT_RESET:
    case USB_DEVICE_EVENT_DECONFIGURED:
      g_app.is_configured = false;
      break;

    case USB_DEVICE_EVENT_CONFIGURED:
      // Check the configuration
      configurationValue = (uint8_t*) event_data;
      if (*configurationValue == 1) {
        // Reset endpoint data send & receive flag
        g_app.is_configured = true;
      }
      break;

    case USB_DEVICE_EVENT_SUSPENDED:
      /* Device is suspended. Update LED indication */
      break;

    case USB_DEVICE_EVENT_POWER_DETECTED:
      /* VBUS is detected. Attach the device */
      USB_DEVICE_Attach(g_app.usb_device);
      break;

    case USB_DEVICE_EVENT_POWER_REMOVED:
      /* VBUS is removed. Detach the device */
      USB_DEVICE_Detach(g_app.usb_device);
      break;

    case USB_DEVICE_EVENT_CONTROL_TRANSFER_SETUP_REQUEST:
      setupPacket = (USB_SETUP_PACKET*) event_data;
      if (setupPacket->RequestType == USB_SETUP_REQUEST_TYPE_CLASS &&
          setupPacket->Recipient == USB_SETUP_REQUEST_RECIPIENT_INTERFACE &&
          setupPacket->wIndex == USB_DFU_INTERFACE_INDEX) {
        HandleDFUEvent(setupPacket);
      } else if (setupPacket->bRequest == USB_REQUEST_SET_INTERFACE) {
        // Just ACK, there are no alternate_settings.
        USB_DEVICE_ControlStatus(g_app.usb_device,
                                 USB_DEVICE_CONTROL_STATUS_OK);
      } else if (setupPacket->bRequest == USB_REQUEST_GET_INTERFACE) {
        USB_DEVICE_ControlSend(g_app.usb_device,
                               &g_app.alternate_setting, 1);
      } else {
        // We have received a request that we cannot handle, stall the pipe.
        USB_DEVICE_ControlStatus(g_app.usb_device,
                                 USB_DEVICE_CONTROL_STATUS_ERROR);
      }
      break;

    case USB_DEVICE_EVENT_CONTROL_TRANSFER_DATA_RECEIVED:
      DFUTransferComplete();
      break;
    case USB_DEVICE_EVENT_CONTROL_TRANSFER_DATA_SENT:
      // The Harmony examples show a call to USB_DEVICE_ControlStatus() here,
      // but that doesn't make sense, since for an IN transfer the host side
      // ACKs. Adding the call causes everything to break, so I think the
      // example is wrong.
      break;
    case USB_DEVICE_EVENT_CONTROL_TRANSFER_ABORTED:
      DFUTransferAborted();
      break;
    // These events are not used.
    case USB_DEVICE_EVENT_ENDPOINT_READ_COMPLETE:
    case USB_DEVICE_EVENT_ENDPOINT_WRITE_COMPLETE:
    case USB_DEVICE_EVENT_RESUMED:
    case USB_DEVICE_EVENT_ERROR:
    default:
      break;
  }
}


void APP_Initialize(void) {
  // TODO(simon):
  // If we're in bootloader mode, or some switch is pressed...
  g_app.usb_device = USB_DEVICE_HANDLE_INVALID;
  g_app.state = APP_STATE_INIT;
  g_app.dfu_state = DFU_STATE_IDLE;
  g_app.dfu_status = DFU_STATUS_OK;
  g_app.is_configured = false;
  g_app.alternate_setting = 0u;
  g_app.has_new_firmware = false;
}

void APP_Tasks(void) {
  switch (g_app.state) {
    case APP_STATE_INIT:
      g_app.usb_device = USB_DEVICE_Open(USB_DEVICE_INDEX_0,
                                         DRV_IO_INTENT_READWRITE);
      if (g_app.usb_device != USB_DEVICE_HANDLE_INVALID) {
        // Register a callback with device layer to get event notification
        // for endpoint 0.
        USB_DEVICE_EventHandlerSet(g_app.usb_device, USBEventHandler, NULL);
        g_app.state = APP_STATE_WAIT_FOR_USB_CONFIGURATION;
      }
      break;

    case APP_STATE_WAIT_FOR_USB_CONFIGURATION:
      if (g_app.is_configured) {
        g_app.state = APP_STATE_DFU;
      }
      break;

    case APP_STATE_DFU:
      if (!g_app.is_configured) {
        // This means the device was deconfigured, change back to waiting for
        // USB config and reset the DFU state.
        g_app.state = APP_STATE_WAIT_FOR_USB_CONFIGURATION;
        g_app.dfu_state = DFU_STATE_IDLE;
      }

      if (g_app.dfu_state == DFU_STATE_MANIFEST) {
        g_app.has_new_firmware = false;
        // pretend we're done and switch back to DFU_STATE_MANIFEST_SYNC;
        g_app.dfu_state = DFU_STATE_MANIFEST_SYNC;
      }
      break;
    case APP_STATE_BOOT:
      // TODO(simon): Do boot
    default:
      break;
  }
}
