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
#include "setting_macros.h"
#include "spi_rgb.h"
#include "stream_decoder.h"
#include "syslog.h"
#include "system_definitions.h"
#include "transceiver.h"
#include "uid_store.h"
#include "usb_descriptors.h"
#include "usb_transport.h"
#include "uid_store.h"

#include "app_settings.h"

void __ISR(AS_TIMER_ISR_VECTOR(COARSE_TIMER_ID), ipl6) TimerEvent() {
  CoarseTimer_TimerEvent();
}

void APP_Initialize(void) {
#ifdef PRE_APP_INIT_HOOK
  PRE_APP_INIT_HOOK();
#endif

  // We can do this after USB_DEVICE_Initialize() has been called since it's
  // not used until we reach the tasks function.
  UIDStore_AsUnicodeString(USBDescriptor_UnicodeUID());

  CoarseTimer_Settings timer_settings = {
    .timer_id = AS_TIMER_ID(COARSE_TIMER_ID),
    .interrupt_source = AS_TIMER_INTERRUPT_SOURCE(COARSE_TIMER_ID)
  };
  SYS_INT_VectorPrioritySet(AS_TIMER_INTERRUPT_VECTOR(COARSE_TIMER_ID),
                            INT_PRIORITY_LEVEL6);
  CoarseTimer_Initialize(&timer_settings);

  // Initialize the Logging system, bottom up
  USBTransport_Initialize(NULL);
  USBConsole_Initialize();
  SysLog_Initialize(NULL);

  // Initialize the DMX / RDM Transceiver
  TransceiverHardwareSettings transceiver_settings = {
    .usart = AS_USART_ID(TRANSCEIVER_UART),
    .usart_vector = AS_USART_INTERRUPT_VECTOR(TRANSCEIVER_UART),
    .usart_tx_source = AS_USART_INTERRUPT_TX_SOURCE(TRANSCEIVER_UART),
    .usart_rx_source = AS_USART_INTERRUPT_RX_SOURCE(TRANSCEIVER_UART),
    .usart_error_source = AS_USART_INTERRUPT_ERROR_SOURCE(TRANSCEIVER_UART),
    .port = TRANSCEIVER_PORT,
    .break_bit = TRANSCEIVER_PORT_BIT,
    .tx_enable_bit = TRANSCEIVER_TX_ENABLE_PORT_BIT,
    .rx_enable_bit = TRANSCEIVER_RX_ENABLE_PORT_BIT,
    .input_capture_module = AS_IC_ID(TRANSCEIVER_IC),
    .input_capture_vector = AS_IC_INTERRUPT_VECTOR(TRANSCEIVER_IC),
    .input_capture_source = AS_IC_INTERRUPT_SOURCE(TRANSCEIVER_IC),
    .timer_module_id = AS_TIMER_ID(TRANSCEIVER_TIMER),
    .timer_vector = AS_TIMER_INTERRUPT_VECTOR(TRANSCEIVER_TIMER),
    .timer_source = AS_TIMER_INTERRUPT_SOURCE(TRANSCEIVER_TIMER),
    .input_capture_timer = AS_IC_TMR_ID(TRANSCEIVER_TIMER),
  };
  Transceiver_Initialize(&transceiver_settings, NULL, NULL);

  // Base RDM Responder
  RDMResponderSettings responder_settings = {
    .identify_port = RDM_RESPONDER_IDENTIFY_PORT,
    .identify_bit = RDM_RESPONDER_IDENTIFY_PORT_BIT,
    .mute_port = RDM_RESPONDER_MUTE_PORT,
    .mute_bit = RDM_RESPONDER_MUTE_PORT_BIT,
  };
  memcpy(responder_settings.uid, UIDStore_GetUID(), UID_LENGTH);
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
  spi_config.module_id = SPI_MODULE_ID;
  spi_config.baud_rate = SPI_BAUD_RATE;
  spi_config.use_enhanced_buffering = SPI_USE_ENHANCED_BUFFERING;
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
