/*
 * File:   app.c
 * Author: Simon Newton
 */
#include "app.h"

#include "dmx.h"
#include "logger.h"
#include "usb_transport.h"
#include "stream_decoder.h"
#include "message_handler.h"

void APP_Initialize(void) {
  DMX_Initialize();
  StreamDecoder_Initialize(NULL);

  USBTransport_Initialize(NULL);

  Logging_Initialize(NULL, PAYLOAD_SIZE);
  Logging_SetState(true);
}


void APP_Tasks(void) {
  USBTransport_Tasks();
  DMX_Tasks();
}
