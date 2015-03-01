/*
 * File:   app.c
 * Author: Simon Newton
 */
#include "app.h"

#include "logger.h"
#include "message_handler.h"
#include "stream_decoder.h"
#include "syslog.h"
#include "system_definitions.h"
#include "system_settings.h"
#include "transceiver.h"
#include "usb_transport.h"

void APP_Initialize(void) {
  // Initialize the Logging system, bottom up
  USBTransport_Initialize(NULL);
  USBConsole_Initialize();
  SysLog_Initialize(NULL);

  // Initialize the DMX / RDM Transceiver
  Transceiver_Settings transceiver_settings = {
    .usart = TRANSCEIVER_TX_UART,
    .port = TRANSCEIVER_PORT,
    .break_bit = TRANSCEIVER_PORT_BIT,
    .rx_enable_bit = TRANSCEIVER_TX_ENABLE,
    .tx_enable_bit = TRANSCEIVER_RX_ENABLE,
  };
  Transceiver_Initialize(&transceiver_settings);

  // Initialize the Host message layers.
  MessageHandler_Initialize(NULL);
  StreamDecoder_Initialize(NULL);

  Flags_Initialize();
  // TODO: simon: remove this.
  Logger_Initialize(NULL, PAYLOAD_SIZE);
  Logger_SetState(true);
}

void APP_Tasks(void) {
  USBTransport_Tasks();
  Transceiver_Tasks();
  USBConsole_Tasks();
}

void APP_Reset() {
  Transceiver_Reset();
  SysLog_Message(SYSLOG_INFO, "Reset Device");
}
