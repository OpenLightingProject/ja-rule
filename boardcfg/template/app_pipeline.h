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
 * template/app_pipeline.h
 * Copyright (C) 2015 Simon Newton
 */


/**
 * @file app_pipeline.h
 * @brief Controls how the application modules are combined.
 *
 * These \#defines decouple modules from each other and enable us to unit test
 * each module in isolation.
 */
#ifndef BOARDCFG_TEMPLATE_APP_PIPELINE_H_
#define BOARDCFG_TEMPLATE_APP_PIPELINE_H_

#define PIPELINE_TRANSPORT_TX(token, command, rc, iov, iov_count) \
  USBTransport_SendResponse(token, command, rc, iov, iov_count);

#define PIPELINE_TRANSPORT_RX(data, size) \
  StreamDecoder_Process(data, size);

#define PIPELINE_HANDLE_MESSAGE(message) \
  MessageHandler_HandleMessage(message);

#define PIPELINE_LOG_WRITE(message) \
  USBConsole_Log(message);

#define PIPELINE_TRANSCEIVER_TX_EVENT(event) \
  MessageHandler_TransceiverEvent(event);

#define PIPELINE_TRANSCEIVER_RX_EVENT(event) \
  Responder_Receive(event);

#define PIPELINE_RDMRESPONDER_SEND(include_break, iov, iov_len) \
  Transceiver_QueueRDMResponse(include_break, iov, iov_len);

#endif  // BOARDCFG_TEMPLATE_APP_PIPELINE_H_
