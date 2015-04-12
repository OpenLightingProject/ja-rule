/*
 * File:   app.c
 * Author: Simon Newton
 */
#include "app.h"

#include "sys/attribs.h"

#include "coarse_timer.h"
#include "logger.h"
#include "message_handler.h"
#include "stream_decoder.h"
#include "syslog.h"
#include "system_definitions.h"
#include "system_settings.h"
#include "transceiver.h"
#include "usb_transport.h"

void __ISR(_TIMER_2_VECTOR, ipl6) TimerEvent() {
  CoarseTimer_TimerEvent();
}

void APP_Initialize(void) {
  CoarseTimer_Settings timer_settings = {
    .timer_id = TMR_ID_2,
    .interrupt_source = INT_SOURCE_TIMER_2
  };
  SYS_INT_VectorPrioritySet(INT_VECTOR_T2, INT_PRIORITY_LEVEL6);
  CoarseTimer_Initialize(&timer_settings);

  // Initialize the Logging system, bottom up
  USBTransport_Initialize(NULL);
  USBConsole_Initialize();
  SysLog_Initialize(NULL);

  // Initialize the DMX / RDM Transceiver
  TransceiverHardwareSettings transceiver_settings = {
    .usart = TRANSCEIVER_UART,
    .port = TRANSCEIVER_PORT,
    .break_bit = TRANSCEIVER_PORT_BIT,
    .rx_enable_bit = TRANSCEIVER_TX_ENABLE,
    .tx_enable_bit = TRANSCEIVER_RX_ENABLE,
  };
  Transceiver_Initialize(&transceiver_settings, NULL);

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
  USBTransport_SoftReset();
}
