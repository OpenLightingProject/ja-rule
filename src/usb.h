/* 
 * File:   usb.h
 * Author: Simon Newton
 *
 * Created on January 3, 2015, 1:16 PM
 */

#ifndef USB_H
#define	USB_H

#include "system_config.h"
#include "system_definitions.h"

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

} USB_DATA;

void USB_Initialize(void);

void USB_Tasks(void);


#endif	/* USB_H */

