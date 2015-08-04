/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * app.c
 * Copyright (C) 2015 Simon Newton
 */

#include "app.h"

#include "sys/attribs.h"

#include "coarse_timer.h"
#include "dimmer_model.h"
#include "led_model.h"
#include "message_handler.h"
#include "moving_light.h"
#include "network_model.h"
#include "proxy_model.h"
#include "rdm.h"
#include "rdm_handler.h"
#include "rdm_responder.h"
#include "receiver_counters.h"
#include "sensor_model.h"
#include "spi_rgb.h"
#include "stream_decoder.h"
#include "syslog.h"
#include "system_definitions.h"
#include "system_settings.h"
#include "transceiver.h"
#include "usb_transport.h"

// TODO(simon): figure out how to allocate UIDs.
// The last byte must be 0 so that we have room for the child responders.
const uint8_t OUR_UID[UID_LENGTH] = {0x7a, 0x70, 0xff, 0xff, 0xfe, 0};

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
  Transceiver_Initialize(&transceiver_settings, NULL, NULL);

  // Base RDM Responder
  RDMResponderSettings responder_settings = {
    .identify_port = RDM_RESPONDER_PORT,
    .identify_bit = RDM_RESPONDER_IDENTIFY_PORT_BIT,
    .mute_port = RDM_RESPONDER_PORT,
    .mute_bit = RDM_RESPONDER_MUTE_PORT_BIT,
  };
  memcpy(responder_settings.uid, OUR_UID, UID_LENGTH);
  RDMResponder_Initialize(&responder_settings);
  ReceiverCounters_ResetCounters();

  // RDM Handler
  RDMHandlerSettings rdm_handler_settings = {
    .default_model = LED_MODEL_ID,
    .send_callback = NULL
  };
  RDMHandler_Initialize(&rdm_handler_settings);

  // Initialize RDM Models, keep these in Model ID order.
  LEDModel_Initialize();
  RDMHandler_AddModel(&LED_MODEL_ENTRY);

  ProxyModel_Initialize();
  RDMHandler_AddModel(&PROXY_MODEL_ENTRY);

  MovingLightModel_Initialize();
  RDMHandler_AddModel(&MOVING_LIGHT_MODEL_ENTRY);

  SensorModel_Initialize();
  RDMHandler_AddModel(&SENSOR_MODEL_ENTRY);

  NetworkModel_Initialize();
  RDMHandler_AddModel(&NETWORK_MODEL_ENTRY);

  DimmerModel_Initialize();
  RDMHandler_AddModel(&DIMMER_MODEL_ENTRY);

  // Initialize the Host message layers.
  MessageHandler_Initialize(NULL);
  StreamDecoder_Initialize(NULL);

  Flags_Initialize();

  // SPI DMX Output
  SPIRGBConfiguration spi_config;
  spi_config.module_id = SPI_ID_1;
  spi_config.baud_rate = 1000000u;
  spi_config.use_enhanced_buffering = true;
  SPIRGB_Init(&spi_config);

  // Send a frame with all pixels set to 0.
  SPIRGB_BeginUpdate();
  SPIRGB_CompleteUpdate();
}

void APP_Tasks(void) {
  USBTransport_Tasks();
  Transceiver_Tasks();
  USBConsole_Tasks();
  RDMResponder_Tasks();

  if (Transceiver_GetMode() == T_MODE_RESPONDER) {
    RDMHandler_Tasks();
    SPIRGB_Tasks();
  }
}

void APP_Reset() {
  Transceiver_Reset();
  SysLog_Message(SYSLOG_INFO, "Reset Device");
  USBTransport_SoftReset();
}
