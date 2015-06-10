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
 * message_handler.c
 * Copyright (C) 2015 Simon Newton
 */

#include "message_handler.h"

#include "app.h"
#include "constants.h"
#include "flags.h"
#include "logger.h"
#include "syslog.h"
#include "system_definitions.h"
#include "system_pipeline.h"
#include "transceiver.h"

#ifndef PIPELINE_TRANSPORT_TX
static TransportTXFunction g_message_tx_cb;
#endif

void MessageHandler_Initialize(TransportTXFunction tx_cb) {
#ifndef PIPELINE_TRANSPORT_TX
  g_message_tx_cb = tx_cb;
#endif
}

static inline void SendMessage(uint8_t token, Command command, uint8_t rc,
                               const IOVec* iov, unsigned int iov_size) {
#ifdef PIPELINE_TRANSPORT_TX
  PIPELINE_TRANSPORT_TX(token, command, rc, iov, iov_size);
#else
  g_message_tx_cb(token, command, rc, iov, iov_size);
#endif
}

void MessageHandler_Echo(const Message *message) {
  IOVec iovec;
  iovec.base = message->payload;
  iovec.length = message->length;
  SendMessage(message->token, ECHO, RC_OK, &iovec, 1);
}

void MessageHandler_WriteLog(const Message* message) {
  Logger_Write(message->payload, message->length);
  if (message->payload[message->length - 1]) {
    // NULL terminate.
    Logger_Log("");
  }
}

static inline void MessageHandler_SetBreakTime(uint8_t token,
                                               const uint8_t* payload,
                                               unsigned int length) {
  uint16_t break_time;
  if (length != sizeof(break_time)) {
    SendMessage(token, SET_BREAK_TIME, RC_BAD_PARAM, NULL, 0);
    return;
  }

  break_time = payload[0] + (payload[1] << 8);
  bool ok = Transceiver_SetBreakTime(break_time);
  SendMessage(token, SET_BREAK_TIME, ok ? RC_OK : RC_BAD_PARAM, NULL, 0);
}

static inline void MessageHandler_ReturnBreakTime(uint8_t token) {
  uint16_t break_time = Transceiver_GetBreakTime();
  IOVec iovec;
  iovec.base = (uint8_t*) &break_time;
  iovec.length = sizeof(break_time);
  SendMessage(token, GET_BREAK_TIME, RC_OK, &iovec, 1);
}

static inline void MessageHandler_SetMarkTime(uint8_t token,
                                              const uint8_t* payload,
                                              unsigned int length) {
  uint16_t mark_time;
  if (length != sizeof(mark_time)) {
    SendMessage(token, SET_MAB_TIME, RC_BAD_PARAM, NULL, 0);
    return;
  }

  mark_time = payload[0] + (payload[1] << 8);
  bool ok = Transceiver_SetMarkTime(mark_time);
  SendMessage(token, SET_MAB_TIME, ok ? RC_OK : RC_BAD_PARAM, NULL, 0);
}

static inline void MessageHandler_ReturnMABTime(uint8_t token) {
  uint16_t mab_time = Transceiver_GetMarkTime();
  IOVec iovec;
  iovec.base = (uint8_t*) &mab_time;
  iovec.length = sizeof(mab_time);
  SendMessage(token, GET_MAB_TIME, RC_OK, &iovec, 1);
}

static inline void MessageHandler_SetRDMBroadcastListen(uint8_t token,
                                                        const uint8_t* payload,
                                                        unsigned int length) {
  uint16_t time;
  if (length != sizeof(time)) {
    SendMessage(token, SET_RDM_BROADCAST_LISTEN, RC_BAD_PARAM, NULL, 0);
    return;
  }

  time = payload[0] + (payload[1] << 8);
  bool ok = Transceiver_SetRDMBroadcastListen(time);
  SendMessage(token, SET_RDM_BROADCAST_LISTEN, ok ? RC_OK : RC_BAD_PARAM, NULL,
              0);
}

static inline void MessageHandler_ReturnRDMBroadcastListen(uint8_t token) {
  uint16_t time = Transceiver_GetRDMBroadcastListen();
  IOVec iovec;
  iovec.base = (uint8_t*) &time;
  iovec.length = sizeof(time);
  SendMessage(token, GET_RDM_BROADCAST_LISTEN, RC_OK, &iovec, 1);
}

static inline void MessageHandler_SetRDMWaitTime(uint8_t token,
                                                 const uint8_t* payload,
                                                 unsigned int length) {
  uint16_t wait_time;
  if (length != sizeof(wait_time)) {
    SendMessage(token, SET_RDM_WAIT_TIME, RC_BAD_PARAM, NULL, 0);
    return;
  }

  wait_time = payload[0] + (payload[1] << 8);
  bool ok = Transceiver_SetRDMWaitTime(wait_time);
  SendMessage(token, SET_RDM_WAIT_TIME, ok ? RC_OK : RC_BAD_PARAM, NULL, 0);
}

static inline void MessageHandler_ReturnRDMWaitTime(uint8_t token) {
  uint16_t wait_time = Transceiver_GetRDMWaitTime();
  IOVec iovec;
  iovec.base = (uint8_t*) &wait_time;
  iovec.length = sizeof(wait_time);
  SendMessage(token, GET_RDM_WAIT_TIME, RC_OK, &iovec, 1);
}

void MessageHandler_HandleMessage(const Message *message) {
  switch (message->command) {
    case ECHO:
      MessageHandler_Echo(message);
      break;
    case TX_DMX:
      if (!Transceiver_QueueDMX(0, message->payload, message->length)) {
        SendMessage(message->token, message->command, RC_BUFFER_FULL, NULL, 0);
      }
      break;
    case GET_LOG:
      Logger_SendResponse(message->token);
      break;
    case GET_FLAGS:
      Flags_SendResponse(message->token);
      break;
    case WRITE_LOG:
      MessageHandler_WriteLog(message);
      SendMessage(message->token, message->command, RC_OK, NULL, 0);
      break;
    case COMMAND_RESET_DEVICE:
      APP_Reset();
      SendMessage(message->token, message->command, RC_OK, NULL, 0);
      break;
    case COMMAND_RDM_DUB_REQUEST:
      if (!Transceiver_QueueRDMDUB(message->token, message->payload,
                                   message->length)) {
        SendMessage(message->token, message->command, RC_BUFFER_FULL, NULL, 0);
      }
      break;
    case COMMAND_RDM_REQUEST:
      if (!Transceiver_QueueRDMRequest(message->token, message->payload,
                                       message->length, false)) {
        SendMessage(message->token, message->command, RC_BUFFER_FULL, NULL, 0);
      }
      break;
    case SET_BREAK_TIME:
      MessageHandler_SetBreakTime(message->token, message->payload,
                                  message->length);
      break;
    case GET_BREAK_TIME:
      MessageHandler_ReturnBreakTime(message->token);
      break;
    case SET_MAB_TIME:
      MessageHandler_SetMarkTime(message->token, message->payload,
                                 message->length);
      break;
    case GET_MAB_TIME:
      MessageHandler_ReturnMABTime(message->token);
      break;
    case SET_RDM_BROADCAST_LISTEN:
      MessageHandler_SetRDMBroadcastListen(message->token, message->payload,
                                           message->length);
      break;
    case GET_RDM_BROADCAST_LISTEN:
      MessageHandler_ReturnRDMBroadcastListen(message->token);
      break;
    case SET_RDM_WAIT_TIME:
      MessageHandler_SetRDMWaitTime(message->token, message->payload,
                                    message->length);
      break;
    case GET_RDM_WAIT_TIME:
      MessageHandler_ReturnRDMWaitTime(message->token);
      break;
    case COMMAND_RDM_BROADCAST_REQUEST:
      if (!Transceiver_QueueRDMRequest(message->token, message->payload,
                                       message->length, true)) {
        SendMessage(message->token, message->command, RC_BUFFER_FULL, NULL, 0);
      }
      break;

    default:
      // Just echo the command code back if we don't understand it.
      SendMessage(message->token, message->command, RC_UNKNOWN, NULL, 0);
  }
}

void MessageHandler_TransceiverEvent(const TransceiverEvent *event) {
  uint8_t vector_size = 0;
  IOVec iovec[2];

  Command command;
  ReturnCode rc;
  switch (event->result) {
    case T_RESULT_TX_OK:
      rc = RC_OK;
      break;
    case T_RESULT_TX_ERROR:
      rc = RC_TX_ERROR;
      break;
    case T_RESULT_RX_DATA:
      rc = (event->op == T_OP_RDM_BROADCAST ? RC_RDM_BCAST_RESPONSE : RC_OK);
      break;
    case T_RESULT_RX_TIMEOUT:
      rc = (event->op == T_OP_RDM_BROADCAST ? RC_OK : RC_RDM_TIMEOUT);
      break;
    case T_RESULT_RX_INVALID:
      rc = RC_RDM_INVALID_RESPONSE;
      break;
    default:
      rc = RC_UNKNOWN;
  }

  switch (event->op) {
    case T_OP_TX_ONLY:
      command = TX_DMX;
      break;
    case T_OP_RDM_DUB:
      command = COMMAND_RDM_DUB_REQUEST;
      iovec[vector_size].base = &event->timing->dub_response;
      iovec[vector_size].length = sizeof(event->timing->dub_response);
      vector_size++;
      break;
    case T_OP_RDM_WITH_RESPONSE:
      command = COMMAND_RDM_REQUEST;
      iovec[vector_size].base = &event->timing->get_set_response;
      iovec[vector_size].length = sizeof(event->timing->get_set_response);
      vector_size++;
      break;
    case T_OP_RDM_BROADCAST:
      command = COMMAND_RDM_BROADCAST_REQUEST;
      break;
    default:
      SysLog_Print(SYSLOG_INFO, "Unknown Transceiver event %d", event->op);
      return;
  }

  if (event->data && event->length > 0) {
    iovec[vector_size].base = event->data;
    iovec[vector_size].length = event->length;
    vector_size++;
  }

  SendMessage(event->token, command, rc, (IOVec*) &iovec, vector_size);
  SysLog_Print(SYSLOG_INFO, "Token %d, op %d, result: %d",
               event->token, event->op, event->result);
}
