/*
 * File:   app.c
 * Author: Simon Newton
 */
#include "app.h"

#include "dmx.h"
#include "logger.h"
#include "usb.h"

void APP_Initialize(void) {

  DMX_Initialize();
  StreamDecode_Initialize();

  // TODO(simon): pass handler function here;
  USB_Initialize();

  Logging_Initialize(NULL, PAYLOAD_SIZE);
  Logging_SetState(true);
}


void APP_Tasks(void) {
  USB_Tasks();
  DMX_Tasks();
}
