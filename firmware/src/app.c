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
#include "syslog.h"
#include "system_definitions.h"

void APP_Initialize(void) {
  DMX_Initialize();
  Flags_Initialize();
  Logger_Initialize(NULL, PAYLOAD_SIZE);
  SysLog_Initialize(NULL);
  MessageHandler_Initialize(NULL);
  StreamDecoder_Initialize(NULL);
  Logger_SetState(true);
  USBTransport_Initialize(NULL);
  USBConsole_Initialize();
}

void APP_Tasks(void) {
  USBTransport_Tasks();
  DMX_Tasks();
  USBConsole_Tasks();
}
