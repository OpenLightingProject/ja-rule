/*
 * File:   usb.h
 * Author: Simon Newton
 *
 * Created on January 3, 2015, 1:16 PM
 */

#include "usb.h"

USB_DATA usbData;

/* Receive data buffer */
uint8_t receivedDataBuffer[USB_READ_BUFFER_SIZE] USB_MAKE_BUFFER_DMA_READY;

/* Transmit data buffer */
uint8_t transmitDataBuffer[USB_READ_BUFFER_SIZE] USB_MAKE_BUFFER_DMA_READY;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/*********************************************
 * Application USB Device Layer Event Handler
 *********************************************/

void APP_USBDeviceEventHandler(USB_DEVICE_EVENT event, void * eventData, uintptr_t context) {
  uint8_t * configurationValue;
  USB_SETUP_PACKET * setupPacket;
  switch (event) {
    case USB_DEVICE_EVENT_RESET:
    case USB_DEVICE_EVENT_DECONFIGURED:
      usbData.deviceIsConfigured = false;
      break;

    case USB_DEVICE_EVENT_CONFIGURED:
      // Check the configuration
      configurationValue = (uint8_t *) eventData;
      if (*configurationValue == 1) {
        // Reset endpoint data send & receive flag
        usbData.deviceIsConfigured = true;
      }
      break;

    case USB_DEVICE_EVENT_SUSPENDED:
      /* Device is suspended. Update LED indication */
      break;

    case USB_DEVICE_EVENT_POWER_DETECTED:
      /* VBUS is detected. Attach the device */
      USB_DEVICE_Attach(usbData.usbDevHandle);
      break;

    case USB_DEVICE_EVENT_POWER_REMOVED:
      /* VBUS is removed. Detach the device */
      USB_DEVICE_Detach(usbData.usbDevHandle);
      break;

    case USB_DEVICE_EVENT_CONTROL_TRANSFER_SETUP_REQUEST:
      /* This means we have received a setup packet */
      setupPacket = (USB_SETUP_PACKET *) eventData;
      if (setupPacket->bRequest == USB_REQUEST_SET_INTERFACE) {
        /* If we have got the SET_INTERFACE request, we just acknowledge
         for now. This demo has only one alternate setting which is already
         active. */
        USB_DEVICE_ControlStatus(usbData.usbDevHandle, USB_DEVICE_CONTROL_STATUS_OK);
      } else if (setupPacket->bRequest == USB_REQUEST_GET_INTERFACE) {
        /* We have only one alternate setting and this setting 0. So
         * we send this information to the host. */

        USB_DEVICE_ControlSend(usbData.usbDevHandle, &usbData.altSetting, 1);
      } else {
        /* We have received a request that we cannot handle. Stall it*/
        USB_DEVICE_ControlStatus(usbData.usbDevHandle, USB_DEVICE_CONTROL_STATUS_ERROR);
      }
      break;

    case USB_DEVICE_EVENT_ENDPOINT_READ_COMPLETE:
      /* Endpoint read is complete */
      usbData.epDataReadPending = false;
      break;

    case USB_DEVICE_EVENT_ENDPOINT_WRITE_COMPLETE:
      /* Endpoint write is complete */
      usbData.epDataWritePending = false;
      break;

      /* These events are not used in this demo. */
    case USB_DEVICE_EVENT_RESUMED:
    case USB_DEVICE_EVENT_ERROR:
    default:
      break;
  }
}

/*******************************************************************************
  Function:
    void USB_Initialize ( void )

  Remarks:
    See prototype in usb.h.
 */

void USB_Initialize(void) {
  usbData.state = USB_STATE_INIT;
  usbData.usbDevHandle = USB_DEVICE_HANDLE_INVALID;
  usbData.deviceIsConfigured = false;
  usbData.endpointRx = 0x01;
  usbData.endpointTx = 0x81;
  usbData.epDataReadPending = false;
  usbData.epDataWritePending = false;
  usbData.altSetting = 0;
}

/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void USB_Tasks(void) {
  uint16_t endpointSize;
  switch (usbData.state) {
    case USB_STATE_INIT:
      /* Open the device layer */
      usbData.usbDevHandle = USB_DEVICE_Open(USB_DEVICE_INDEX_0,
                                             DRV_IO_INTENT_READWRITE);

      if (usbData.usbDevHandle != USB_DEVICE_HANDLE_INVALID) {
        /* Register a callback with device layer to get event notification (for end point 0) */
        USB_DEVICE_EventHandlerSet(usbData.usbDevHandle, APP_USBDeviceEventHandler, 0);

        usbData.state = USB_STATE_WAIT_FOR_CONFIGURATION;
      } else {
        /* The Device Layer is not ready to be opened. We should try
         * again later. */
      }

      break;

    case USB_STATE_WAIT_FOR_CONFIGURATION:

      /* Check if the device is configured */
      if (usbData.deviceIsConfigured == true) {
        if (USB_DEVICE_ActiveSpeedGet(usbData.usbDevHandle) == USB_SPEED_FULL) {
          endpointSize = 64;
        } else if (USB_DEVICE_ActiveSpeedGet(usbData.usbDevHandle) == USB_SPEED_HIGH) {
          endpointSize = 512;
        }
        if (USB_DEVICE_EndpointIsEnabled(usbData.usbDevHandle, usbData.endpointRx) == false) {
          /* Enable Read Endpoint */
          USB_DEVICE_EndpointEnable(usbData.usbDevHandle, 0, usbData.endpointRx,
                                    USB_TRANSFER_TYPE_BULK, endpointSize);
        }
        if (USB_DEVICE_EndpointIsEnabled(usbData.usbDevHandle, usbData.endpointTx) == false) {
          /* Enable Write Endpoint */
          USB_DEVICE_EndpointEnable(usbData.usbDevHandle, 0, usbData.endpointTx,
                                    USB_TRANSFER_TYPE_BULK, endpointSize);
        }
        /* Indicate that we are waiting for read */
        usbData.epDataReadPending = true;

        /* Place a new read request. */
        USB_DEVICE_EndpointRead(usbData.usbDevHandle, &usbData.readTranferHandle,
                                usbData.endpointRx, &receivedDataBuffer[0], sizeof (receivedDataBuffer));

        /* Device is ready to run the main task */
        usbData.state = USB_STATE_MAIN_TASK;
      }
      break;

    case USB_STATE_MAIN_TASK:
      if (!usbData.deviceIsConfigured) {
        /* This means the device got deconfigured. Change the
         * application state back to waiting for configuration. */
        usbData.state = USB_STATE_WAIT_FOR_CONFIGURATION;

        /* Disable the endpoint*/
        USB_DEVICE_EndpointDisable(usbData.usbDevHandle, usbData.endpointRx);
        USB_DEVICE_EndpointDisable(usbData.usbDevHandle, usbData.endpointTx);
        usbData.epDataReadPending = false;
        usbData.epDataWritePending = false;
      } else if (usbData.epDataReadPending == false) {
        /* Look at the data the host sent, to see what kind of
         * application specific command it sent. */

        switch (receivedDataBuffer[0]) {
          case 0x80:
            break;

          case 0x81:
            if (usbData.epDataWritePending == false) {
              /* Echo back to the host PC the command we are fulfilling
               * in the first byte.  In this case, the Get Pushbutton
               * State command. */

              transmitDataBuffer[0] = 0x81;
              transmitDataBuffer[1] = 0x00;

              /* Send the data to the host */

              usbData.epDataWritePending = true;

              USB_DEVICE_EndpointWrite(usbData.usbDevHandle, &usbData.writeTranferHandle,
                                       usbData.endpointTx, &transmitDataBuffer[0],
                                       sizeof (transmitDataBuffer),
                                       USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE);
            }
            break;
          default:
            break;
        }

        usbData.epDataReadPending = true;

        /* Place a new read request. */
        USB_DEVICE_EndpointRead(usbData.usbDevHandle, &usbData.readTranferHandle,
                                usbData.endpointRx, &receivedDataBuffer[0], sizeof (receivedDataBuffer));
      }
      break;

    case USB_STATE_ERROR:
      break;

    default:
      break;
  }
}