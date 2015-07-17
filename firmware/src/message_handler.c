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
#include "rdm_frame.h"
#include "rdm_handler.h"
#include "syslog.h"
#include "system_definitions.h"
#include "system_pipeline.h"
#include "transceiver.h"

#ifndef PIPELINE_TRANSPORT_TX
static TransportTXFunction g_message_tx_cb;
#endif

static inline uint16_t JoinUInt16(uint8_t upper, uint8_t lower) {
  return (upper << 8) + lower;
}

static inline void SendMessage(uint8_t token, Command command, uint8_t rc,
                               const IOVec* iov, unsigned int iov_size) {
#ifdef PIPELINE_TRANSPORT_TX
  PIPELINE_TRANSPORT_TX(token, command, rc, iov, iov_size);
#else
  g_message_tx_cb(token, command, rc, iov, iov_size);
#endif
}

static void Echo(const Message *message) {
  IOVec iovec;
  iovec.base = message->payload;
  iovec.length = message->length;
  SendMessage(message->token, ECHO, RC_OK, &iovec, 1);
}

static void WriteLog(const Message* message) {
  Logger_Write(message->payload, message->length);
  if (message->payload[message->length - 1]) {
    // NULL terminate.
    Logger_Log("");
  }
}

static void SetMode(uint8_t token,
                    const uint8_t* payload,
                    unsigned int length) {
  uint8_t mode;
  if (length != sizeof(mode)) {
    SendMessage(token, COMMAND_SET_MODE, RC_BAD_PARAM, NULL, 0);
    return;
  }
  mode = payload[0];
  Transceiver_SetMode(mode ? T_MODE_RESPONDER : T_MODE_CONTROLLER);
  SendMessage(token, COMMAND_SET_MODE, RC_OK, NULL, 0);
}

static void GetUID(uint8_t token, unsigned int length) {
  if (length) {
    SendMessage(token, COMMAND_GET_UID, RC_BAD_PARAM, NULL, 0);
    return;
  }
  uint8_t uid[UID_LENGTH];
  RDMHandler_GetUID(uid);
  IOVec iovec;
  iovec.base = (uint8_t*) uid;
  iovec.length = UID_LENGTH;
  SendMessage(token, COMMAND_GET_UID, RC_OK, &iovec, 1);
}

static void SetBreakTime(uint8_t token,
                         const uint8_t* payload,
                         unsigned int length) {
  uint16_t break_time;
  if (length != sizeof(break_time)) {
    SendMessage(token, COMMAND_SET_BREAK_TIME, RC_BAD_PARAM, NULL, 0);
    return;
  }

  break_time = JoinUInt16(payload[1], payload[0]);
  bool ok = Transceiver_SetBreakTime(break_time);
  SendMessage(token, COMMAND_SET_BREAK_TIME, ok ? RC_OK : RC_BAD_PARAM,
              NULL, 0);
}

static void ReturnBreakTime(uint8_t token, unsigned int length) {
  if (length) {
    SendMessage(token, COMMAND_GET_BREAK_TIME, RC_BAD_PARAM, NULL, 0);
    return;
  }

  uint16_t break_time = Transceiver_GetBreakTime();
  IOVec iovec;
  iovec.base = (uint8_t*) &break_time;
  iovec.length = sizeof(break_time);
  SendMessage(token, COMMAND_GET_BREAK_TIME, RC_OK, &iovec, 1);
}

static void SetMarkTime(uint8_t token,
                        const uint8_t* payload,
                        unsigned int length) {
  uint16_t mark_time;
  if (length != sizeof(mark_time)) {
    SendMessage(token, COMMAND_SET_MARK_TIME, RC_BAD_PARAM, NULL, 0);
    return;
  }

  mark_time = JoinUInt16(payload[1], payload[0]);
  bool ok = Transceiver_SetMarkTime(mark_time);
  SendMessage(token, COMMAND_SET_MARK_TIME, ok ? RC_OK : RC_BAD_PARAM, NULL, 0);
}

static void ReturnMarkTime(uint8_t token, unsigned int length) {
  if (length) {
    SendMessage(token, COMMAND_GET_MARK_TIME, RC_BAD_PARAM, NULL, 0);
    return;
  }

  uint16_t mab_time = Transceiver_GetMarkTime();
  IOVec iovec;
  iovec.base = (uint8_t*) &mab_time;
  iovec.length = sizeof(mab_time);
  SendMessage(token, COMMAND_GET_MARK_TIME, RC_OK, &iovec, 1);
}

static void SetRDMBroadcastTimeout(uint8_t token,
                                   const uint8_t* payload,
                                   unsigned int length) {
  uint16_t time;
  if (length != sizeof(time)) {
    SendMessage(token, COMMAND_SET_RDM_BROADCAST_TIMEOUT, RC_BAD_PARAM, NULL,
                0);
    return;
  }

  time = JoinUInt16(payload[1], payload[0]);
  bool ok = Transceiver_SetRDMBroadcastTimeout(time);
  SendMessage(token, COMMAND_SET_RDM_BROADCAST_TIMEOUT,
              ok ? RC_OK : RC_BAD_PARAM, NULL, 0);
}

static void ReturnRDMBroadcastTimeout(uint8_t token, unsigned int length) {
  if (length) {
    SendMessage(token, COMMAND_GET_RDM_BROADCAST_TIMEOUT, RC_BAD_PARAM, NULL,
                0);
    return;
  }
  uint16_t time = Transceiver_GetRDMBroadcastTimeout();
  IOVec iovec;
  iovec.base = (uint8_t*) &time;
  iovec.length = sizeof(time);
  SendMessage(token, COMMAND_GET_RDM_BROADCAST_TIMEOUT, RC_OK, &iovec, 1);
}

static void SetRDMResponseTimeout(
    uint8_t token,
    const uint8_t* payload,
    unsigned int length) {
  uint16_t timeout;
  if (length != sizeof(timeout)) {
    SendMessage(token, COMMAND_SET_RDM_RESPONSE_TIMEOUT, RC_BAD_PARAM, NULL, 0);
    return;
  }

  timeout = JoinUInt16(payload[1], payload[0]);
  bool ok = Transceiver_SetRDMResponseTimeout(timeout);
  SendMessage(token, COMMAND_SET_RDM_RESPONSE_TIMEOUT,
              ok ? RC_OK : RC_BAD_PARAM, NULL, 0);
}

static void ReturnRDMResponseTimeout(
    uint8_t token,
    unsigned int length) {
  if (length) {
    SendMessage(token, COMMAND_GET_RDM_RESPONSE_TIMEOUT, RC_BAD_PARAM, NULL, 0);
    return;
  }
  uint16_t timeout = Transceiver_GetRDMResponseTimeout();
  IOVec iovec;
  iovec.base = (uint8_t*) &timeout;
  iovec.length = sizeof(timeout);
  SendMessage(token, COMMAND_GET_RDM_RESPONSE_TIMEOUT, RC_OK, &iovec, 1);
}

static void SetRDMDUBResponseLimit(uint8_t token,
                                   const uint8_t* payload,
                                   unsigned int length) {
  uint16_t limit;
  if (length != sizeof(limit)) {
    SendMessage(token, COMMAND_SET_RDM_DUB_RESPONSE_LIMIT, RC_BAD_PARAM, NULL,
                0);
    return;
  }

  limit = JoinUInt16(payload[1], payload[0]);
  bool ok = Transceiver_SetRDMDUBResponseLimit(limit);
  SendMessage(token, COMMAND_SET_RDM_DUB_RESPONSE_LIMIT,
              ok ? RC_OK : RC_BAD_PARAM, NULL, 0);
}

static void ReturnRDMDUBResponseLimit(uint8_t token, unsigned int length) {
  if (length) {
    SendMessage(token, COMMAND_GET_RDM_DUB_RESPONSE_LIMIT, RC_BAD_PARAM,
                NULL, 0);
    return;
  }
  uint16_t limit = Transceiver_GetRDMDUBResponseLimit();
  IOVec iovec;
  iovec.base = (uint8_t*) &limit;
  iovec.length = sizeof(limit);
  SendMessage(token, COMMAND_GET_RDM_DUB_RESPONSE_LIMIT, RC_OK, &iovec, 1);
}

static void SetRDMResponderDelay(uint8_t token,
                                 const uint8_t* payload,
                                 unsigned int length) {
  uint16_t delay;
  if (length != sizeof(delay)) {
    SendMessage(token, COMMAND_SET_RDM_RESPONDER_DELAY, RC_BAD_PARAM, NULL, 0);
    return;
  }

  delay = JoinUInt16(payload[1], payload[0]);
  bool ok = Transceiver_SetRDMResponderDelay(delay);
  SendMessage(token, COMMAND_SET_RDM_RESPONDER_DELAY,
              ok ? RC_OK : RC_BAD_PARAM, NULL, 0);
}

static void ReturnRDMResponderDelay(uint8_t token, unsigned int length) {
  if (length) {
    SendMessage(token, COMMAND_GET_RDM_RESPONDER_DELAY, RC_BAD_PARAM,
                NULL, 0);
    return;
  }
  uint16_t delay = Transceiver_GetRDMResponderDelay();
  IOVec iovec;
  iovec.base = (uint8_t*) &delay;
  iovec.length = sizeof(delay);
  SendMessage(token, COMMAND_GET_RDM_RESPONDER_DELAY, RC_OK, &iovec, 1);
}

static void SetRDMResponderJitter(uint8_t token,
                                  const uint8_t* payload,
                                  unsigned int length) {
  uint16_t jitter;
  if (length != sizeof(jitter)) {
    SendMessage(token, COMMAND_SET_RDM_RESPONDER_JITTER, RC_BAD_PARAM, NULL, 0);
    return;
  }

  jitter = JoinUInt16(payload[1], payload[0]);
  bool ok = Transceiver_SetRDMResponderJitter(jitter);
  SendMessage(token, COMMAND_SET_RDM_RESPONDER_JITTER,
              ok ? RC_OK : RC_BAD_PARAM, NULL, 0);
}

static void ReturnRDMResponderJitter(uint8_t token, unsigned int length) {
  if (length) {
    SendMessage(token, COMMAND_GET_RDM_RESPONDER_JITTER, RC_BAD_PARAM,
                NULL, 0);
    return;
  }
  uint16_t jitter = Transceiver_GetRDMResponderJitter();
  IOVec iovec;
  iovec.base = (uint8_t*) &jitter;
  iovec.length = sizeof(jitter);
  SendMessage(token, COMMAND_GET_RDM_RESPONDER_JITTER, RC_OK, &iovec, 1);
}

// Public Functions
// ----------------------------------------------------------------------------
void MessageHandler_Initialize(TransportTXFunction tx_cb) {
#ifndef PIPELINE_TRANSPORT_TX
  g_message_tx_cb = tx_cb;
#endif
}

void MessageHandler_HandleMessage(const Message *message) {
  switch (message->command) {
    case ECHO:
      Echo(message);
      break;
    case TX_DMX:
      if (!Transceiver_QueueDMX(message->token, message->payload,
                                message->length)) {
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
      WriteLog(message);
      SendMessage(message->token, message->command, RC_OK, NULL, 0);
      break;
    case COMMAND_RESET_DEVICE:
      APP_Reset();
      SendMessage(message->token, message->command, RC_OK, NULL, 0);
      break;
    case COMMAND_SET_MODE:
      SetMode(message->token, message->payload, message->length);
      break;
    case COMMAND_GET_UID:
      GetUID(message->token, message->length);
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
    case COMMAND_SET_BREAK_TIME:
      SetBreakTime(message->token, message->payload, message->length);
      break;
    case COMMAND_GET_BREAK_TIME:
      ReturnBreakTime(message->token, message->length);
      break;
    case COMMAND_SET_MARK_TIME:
      SetMarkTime(message->token, message->payload, message->length);
      break;
    case COMMAND_GET_MARK_TIME:
      ReturnMarkTime(message->token, message->length);
      break;
    case COMMAND_SET_RDM_BROADCAST_TIMEOUT:
      SetRDMBroadcastTimeout(message->token, message->payload, message->length);
      break;
    case COMMAND_GET_RDM_BROADCAST_TIMEOUT:
      ReturnRDMBroadcastTimeout(message->token, message->length);
      break;
    case COMMAND_SET_RDM_RESPONSE_TIMEOUT:
      SetRDMResponseTimeout(message->token, message->payload, message->length);
      break;
    case COMMAND_GET_RDM_RESPONSE_TIMEOUT:
      ReturnRDMResponseTimeout(message->token, message->length);
      break;
    case COMMAND_SET_RDM_DUB_RESPONSE_LIMIT:
      SetRDMDUBResponseLimit(message->token, message->payload, message->length);
      break;
    case COMMAND_GET_RDM_DUB_RESPONSE_LIMIT:
      ReturnRDMDUBResponseLimit(message->token, message->length);
      break;
    case COMMAND_SET_RDM_RESPONDER_DELAY:
      SetRDMResponderDelay(message->token, message->payload, message->length);
      break;
    case COMMAND_GET_RDM_RESPONDER_DELAY:
      ReturnRDMResponderDelay(message->token, message->length);
      break;
    case COMMAND_SET_RDM_RESPONDER_JITTER:
      SetRDMResponderJitter(message->token, message->payload, message->length);
      break;
    case COMMAND_GET_RDM_RESPONDER_JITTER:
      ReturnRDMResponderJitter(message->token, message->length);
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
