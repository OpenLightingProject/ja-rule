/*
 * File:   app.c
 * Author: Simon Newton
 */
#include "app.h"

#include "dmx.h"
#include "usb.h"


void APP_Initialize(void) {
  DMX_Initialize();
  StreamDecode_Initialize();

  // TODO(simon): pass handler function here;
  USB_Initialize();
}

void APP_Tasks(void) {
  USB_Tasks();
  DMX_Tasks();
}
