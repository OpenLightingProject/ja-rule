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
#include "dfu_constants.h"
#include "constants.h"

#include "system_config.h"

typedef enum {
  APP_STATE_INIT = 0,
  APP_STATE_WAIT_FOR_CONFIGURATION,
  APP_STATE_MAIN_TASK,
  APP_STATE_ERROR
} AppState;

typedef struct {
  USB_DEVICE_HANDLE usb_device;  //!< The USB Device layer handle.
  AppState state;
  bool is_configured;  //!< Keep track of whether the device is configured.
} AppData;


static uint8_t STATUS_RESPONSE[6];
static AppData g_app;

static void HandleDFUEvent(USB_SETUP_PACKET *setupPacket) {
  BSP_LEDToggle(BSP_LED_1);
  if (setupPacket->Recipient == USB_SETUP_REQUEST_DIRECTION_DEVICE_TO_HOST &&
      setupPacket->bRequest == DFU_GETSTATUS) {
    STATUS_RESPONSE[0] = 0;
    STATUS_RESPONSE[1] = 0;
    STATUS_RESPONSE[2] = 0;
    STATUS_RESPONSE[3] = 0;
    STATUS_RESPONSE[4] = 0;
    STATUS_RESPONSE[5] = 0;
    USB_DEVICE_ControlSend(g_app.usb_device, STATUS_RESPONSE, 6);
  } else {
    USB_DEVICE_ControlStatus(g_app.usb_device,
                             USB_DEVICE_CONTROL_STATUS_ERROR);
  }
}

/**
 * @brief Called when USB events occur.
 * @param event The type of event
 * @param event_data Data associated with the event
 * @param context The context (unused).
 */
static void USBEventHandler(USB_DEVICE_EVENT event,
                            void* event_data,
                            uintptr_t context) {
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
      // This means we have received a setup packet
      BSP_LEDToggle(BSP_LED_2);
      setupPacket = (USB_SETUP_PACKET*) event_data;
      if (setupPacket->RequestType == USB_SETUP_REQUEST_TYPE_CLASS &&
          setupPacket->DataDir == USB_SETUP_REQUEST_RECIPIENT_INTERFACE &&
          setupPacket->wIndex == USB_DFU_INTERFACE_INDEX) {
        HandleDFUEvent(setupPacket);
      } else if (setupPacket->bRequest == USB_REQUEST_SET_INTERFACE) {
        /* If we have got the SET_INTERFACE request, we just acknowledge
         for now. This demo has only one alternate setting which is already
         active. */
        USB_DEVICE_ControlStatus(g_app.usb_device,
                                 USB_DEVICE_CONTROL_STATUS_OK);
        /*
      } else if (setupPacket->bRequest == USB_REQUEST_GET_INTERFACE) {
         We have only one alternate setting and this setting 0. So
         * we send this information to the host. 
        USB_DEVICE_ControlSend(g_app.usb_device,
                               &g_usb_transport_data.altSetting, 1);
                               */
      } else {
        // We have received a request that we cannot handle. Stall it
        USB_DEVICE_ControlStatus(g_app.usb_device,
                                 USB_DEVICE_CONTROL_STATUS_ERROR);
      }
      break;

    // These events are not used.
    case USB_DEVICE_EVENT_ENDPOINT_READ_COMPLETE:
    case USB_DEVICE_EVENT_ENDPOINT_WRITE_COMPLETE:
    case USB_DEVICE_EVENT_RESUMED:
    case USB_DEVICE_EVENT_ERROR:
    default:
      break;
  }
  (void) context;
}


void APP_Initialize(void) {
  // If we're in bootloader mode, or some switch is pressed...
  g_app.usb_device = USB_DEVICE_HANDLE_INVALID;
  g_app.state = APP_STATE_INIT;
  g_app.is_configured = false;
}

void APP_Tasks(void) {
  switch (g_app.state) {
    case APP_STATE_INIT:
      // Try to open the device layer.
      g_app.usb_device = USB_DEVICE_Open(USB_DEVICE_INDEX_0,
                                         DRV_IO_INTENT_READWRITE);
      if (g_app.usb_device != USB_DEVICE_HANDLE_INVALID) {
        // Register a callback with device layer to get event notification
        // for end point 0.
        USB_DEVICE_EventHandlerSet(g_app.usb_device, USBEventHandler, 0);
        g_app.state = APP_STATE_WAIT_FOR_CONFIGURATION;
      } else {
        // The Device Layer is not ready yet. Try again later.
      }
      break;

    case APP_STATE_WAIT_FOR_CONFIGURATION:
      if (g_app.is_configured) {
        BSP_LEDToggle(BSP_LED_3);
        // Ready to run the main task
        g_app.state = APP_STATE_MAIN_TASK;
      }
      break;

    case APP_STATE_MAIN_TASK:
      if (!g_app.is_configured) {
        /* This means the device got deconfigured. Change the
         * application state back to waiting for configuration. */
        g_app.state = APP_STATE_WAIT_FOR_CONFIGURATION;
      } else {
        // Do something
      }
      break;
    case APP_STATE_ERROR:
      break;
    default:
      break;
  }
}
