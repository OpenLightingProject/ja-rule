/*
 * File:   usb.h
 * Author: Simon Newton
 */

#include "usb.h"

#include "system_config.h"
#include "system_definitions.h"
#include "constants.h"
#include "stream_decode.h"

typedef enum {
  // USB state machine's initial state.
  USB_STATE_INIT = 0,

  // USB waits for device configuration
  USB_STATE_WAIT_FOR_CONFIGURATION,

  // USB runs the main task
  USB_STATE_MAIN_TASK,

  // USB error occurred
  USB_STATE_ERROR
} USB_STATES;

typedef struct {
  /* Device layer handle returned by device layer open function */
  USB_DEVICE_HANDLE usbDevHandle;

  // USB state
  USB_STATES state;

  /* Track device configuration */
  bool deviceIsConfigured;

  /* Configuration value */
  uint8_t configValue;

  /* speed */
  USB_SPEED speed;

  /* ep data sent */
  bool epDataWritePending;

  /* ep data received */
  bool epDataReadPending;

  /* Transfer handle */
  USB_DEVICE_TRANSFER_HANDLE writeTranferHandle;

  /* Transfer handle */
  USB_DEVICE_TRANSFER_HANDLE readTranferHandle;

  /* The transmit endpoint address */
  USB_ENDPOINT_ADDRESS endpointTx;

  /* The receive endpoint address */
  USB_ENDPOINT_ADDRESS endpointRx;

  /* Tracks the alternate setting */
  uint8_t altSetting;

  int epDataReadSize;

} USB_DATA;


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
      usbData.epDataReadSize = ((USB_DEVICE_EVENT_DATA_ENDPOINT_WRITE_COMPLETE*) eventData)->length;
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
  usbData.epDataReadSize = 0;
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
        USB_DEVICE_RESULT result = USB_DEVICE_EndpointRead(
                                                           usbData.usbDevHandle, &usbData.readTranferHandle,
                                                           usbData.endpointRx, &receivedDataBuffer[0], sizeof (receivedDataBuffer));

        if (result == USB_DEVICE_TRANSFER_HANDLE_INVALID) {
          BSP_LEDToggle(BSP_LED_3);
        }
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


        if (usbData.epDataWritePending == false) {
          // we only go ahead and process the data if we can respond.
          StreamDecode_Process(receivedDataBuffer, usbData.epDataReadSize);

          // schedule the next read
          usbData.epDataReadPending = true;
          USB_DEVICE_EndpointRead(usbData.usbDevHandle,
                                  &usbData.readTranferHandle,
                                  usbData.endpointRx,
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

void SendResponse(Command command, uint8_t rc, const uint8_t* data,
                  unsigned int data_length) {
  if (usbData.epDataWritePending) {
    BSP_LEDToggle(BSP_LED_2);
    return;
  }

  transmitDataBuffer[0] = START_OF_MESSAGE_ID;
  transmitDataBuffer[1] = command & 0xff;
  transmitDataBuffer[2] = command >> 8;
  transmitDataBuffer[3] = data_length & 0xf;
  transmitDataBuffer[4] = data_length >> 8;
  transmitDataBuffer[5] = rc;
  if (data_length > 0) {
    memcpy(transmitDataBuffer + 6, data, data_length);
  }
  transmitDataBuffer[6 + data_length] = END_OF_MESSAGE_ID;

  usbData.epDataWritePending = true;

  USB_DEVICE_EndpointWrite(usbData.usbDevHandle, &usbData.writeTranferHandle,
                           usbData.endpointTx, transmitDataBuffer,
                           data_length + 7,
                           USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE);


}