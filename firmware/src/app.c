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
  // Initialize the Logging system, bottom up
  USBTransport_Initialize(NULL);
  USBConsole_Initialize();
  SysLog_Initialize(NULL);

  // Initialize the DMX System
  DMX_Initialize();
  MessageHandler_Initialize(NULL);
  StreamDecoder_Initialize(NULL);

  Flags_Initialize();
  // TODO: simon: remove this.
  Logger_Initialize(NULL, PAYLOAD_SIZE);
  Logger_SetState(true);
}

void APP_Tasks(void) {
  USBTransport_Tasks();
  DMX_Tasks();
  USBConsole_Tasks();
}

void APP_Reset() {
  DMX_Reset();
  SysLog_Message(SYSLOG_INFO, "Reset Device");
}