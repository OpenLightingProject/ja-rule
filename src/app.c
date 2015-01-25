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
  Flags_Initialize();
  Logging_Initialize(NULL, PAYLOAD_SIZE);
  MessageHandler_Initialize(NULL);
  StreamDecoder_Initialize(NULL);
  Logging_SetState(true);
  USBTransport_Initialize(NULL);
}

void APP_Tasks(void) {
  USBTransport_Tasks();
  DMX_Tasks();
}
