/*
 * File:   message_handler.c
 * Author: Simon Newton
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

static inline void SendMessage(Command command, uint8_t rc, const IOVec* iov,
                               unsigned int iov_size) {
#ifdef PIPELINE_TRANSPORT_TX
  PIPELINE_TRANSPORT_TX(command, rc, iov, iov_size);
#else
  g_message_tx_cb(command, rc, iov, iov_size);
#endif
}

void MessageHandler_Echo(const Message *message) {
  IOVec iovec;
  iovec.base = message->payload;
  iovec.length = message->length;
  SendMessage(ECHO, RC_OK, &iovec, 1);
}

void MessageHandler_WriteLog(const Message* message) {
  Logger_Write(message->payload, message->length);
  if (message->payload[message->length - 1]) {
    // NULL terminate.
    Logger_Log("");
  }
}

static inline void MessageHandler_SetBreakTime(const uint8_t* payload,
                                               unsigned int length) {
  uint16_t break_time;
  if (length != sizeof(break_time)) {
    SendMessage(SET_BREAK_TIME, RC_BAD_PARAM, NULL, 0);
    return;
  }

  break_time = payload[0] + (payload[1] << 8);
  bool ok = Transceiver_SetBreakTime(break_time);
  SendMessage(SET_BREAK_TIME, ok ? RC_OK : RC_BAD_PARAM, NULL, 0);
}

static inline void MessageHandler_ReturnBreakTime() {
  uint16_t break_time = Transceiver_GetBreakTime();
  IOVec iovec;
  iovec.base = (uint8_t*) &break_time;
  iovec.length = sizeof(break_time);
  SendMessage(GET_BREAK_TIME, RC_OK, &iovec, 1);
}

static inline void MessageHandler_SetMarkTime(const uint8_t* payload,
                                              unsigned int length) {
  uint16_t mark_time;
  if (length != sizeof(mark_time)) {
    SendMessage(SET_MAB_TIME, RC_BAD_PARAM, NULL, 0);
    return;
  }

  mark_time = payload[0] + (payload[1] << 8);
  bool ok = Transceiver_SetMarkTime(mark_time);
  SendMessage(SET_MAB_TIME, ok ? RC_OK : RC_BAD_PARAM, NULL, 0);
}

static inline void MessageHandler_ReturnMABTime() {
  uint16_t mab_time = Transceiver_GetMarkTime();
  IOVec iovec;
  iovec.base = (uint8_t*) &mab_time;
  iovec.length = sizeof(mab_time);
  SendMessage(GET_MAB_TIME, RC_OK, &iovec, 1);
}

static inline void MessageHandler_SetRDMBroadcastListen(const uint8_t* payload,
                                                        unsigned int length) {
  uint16_t time;
  if (length != sizeof(time)) {
    SendMessage(SET_RDM_BROADCAST_LISTEN, RC_BAD_PARAM, NULL, 0);
    return;
  }

  time = payload[0] + (payload[1] << 8);
  bool ok = Transceiver_SetRDMBroadcastListen(time);
  SendMessage(SET_RDM_BROADCAST_LISTEN, ok ? RC_OK : RC_BAD_PARAM, NULL, 0);
}

static inline void MessageHandler_ReturnRDMBroadcastListen() {
  uint16_t time = Transceiver_GetRDMBroadcastListen();
  IOVec iovec;
  iovec.base = (uint8_t*) &time;
  iovec.length = sizeof(time);
  SendMessage(GET_RDM_BROADCAST_LISTEN, RC_OK, &iovec, 1);
}

static inline void MessageHandler_SetRDMWaitTime(const uint8_t* payload,
                                                 unsigned int length) {
  uint16_t wait_time;
  if (length != sizeof(wait_time)) {
    SendMessage(SET_RDM_WAIT_TIME, RC_BAD_PARAM, NULL, 0);
    return;
  }

  wait_time = payload[0] + (payload[1] << 8);
  bool ok = Transceiver_SetRDMWaitTime(wait_time);
  SendMessage(SET_RDM_WAIT_TIME, ok ? RC_OK : RC_BAD_PARAM, NULL, 0);
}

static inline void MessageHandler_ReturnRDMWaitTime() {
  uint16_t wait_time = Transceiver_GetRDMWaitTime();
  IOVec iovec;
  iovec.base = (uint8_t*) &wait_time;
  iovec.length = sizeof(wait_time);
  SendMessage(GET_RDM_WAIT_TIME, RC_OK, &iovec, 1);
}

void MessageHandler_HandleMessage(const Message *message) {
  switch (message->command) {
    case ECHO:
      MessageHandler_Echo(message);
      break;
    case TX_DMX:
      if (!Transceiver_QueueDMX(0, message->payload, message->length)) {
        SendMessage(message->command, RC_BUFFER_FULL, NULL, 0);
      }
      break;
    case GET_LOG:
      Logger_SendResponse();
      break;
    case GET_FLAGS:
      Flags_SendResponse();
      break;
    case WRITE_LOG:
      MessageHandler_WriteLog(message);
      SendMessage(WRITE_LOG, RC_OK, NULL, 0);
      break;
    case COMMAND_RESET_DEVICE:
      APP_Reset();
      SendMessage(message->command, RC_OK, NULL, 0);
      break;
    case COMMAND_RDM_DUB_REQUEST:
      if (!Transceiver_QueueRDMDUB(0, message->payload, message->length)) {
        SendMessage(message->command, RC_BUFFER_FULL, NULL, 0);
      }
      break;
    case COMMAND_RDM_REQUEST:
      if (!Transceiver_QueueRDMRequest(0, message->payload, message->length)) {
        SendMessage(message->command, RC_BUFFER_FULL, NULL, 0);
      }
      break;
    case SET_BREAK_TIME:
      MessageHandler_SetBreakTime(message->payload, message->length);
      break;
    case GET_BREAK_TIME:
      MessageHandler_ReturnBreakTime();
      break;
    case SET_MAB_TIME:
      MessageHandler_SetMarkTime(message->payload, message->length);
      break;
    case GET_MAB_TIME:
      MessageHandler_ReturnMABTime();
      break;
    case SET_RDM_BROADCAST_LISTEN:
      MessageHandler_SetRDMBroadcastListen(message->payload, message->length);
      break;
    case GET_RDM_BROADCAST_LISTEN:
      MessageHandler_ReturnRDMBroadcastListen();
      break;
    case SET_RDM_WAIT_TIME:
      MessageHandler_SetRDMWaitTime(message->payload, message->length);
      break;
    case GET_RDM_WAIT_TIME:
      MessageHandler_ReturnRDMWaitTime();
      break;

    default:
      // Just echo the command code back if we don't understand it.
      SendMessage(message->command, RC_UNKNOWN, NULL, 0);
  }
}

void MessageHandler_TransceiverEvent(uint8_t token,
                                     TransceiverOperation type,
                                     TransceiverResult result,
                                     const uint8_t* data,
                                     unsigned int length) {
  SysLog_Print(SYSLOG_INFO, "Result was %d, size %d", result, length);
  uint8_t vector_size = 1;
  IOVec iovec[2];
  iovec[0].base = &token;
  iovec[0].length = sizeof(token);

  if (data && length > 0) {
    iovec[1].base = data;
    iovec[1].length = length;
    vector_size++;
  }

  Command command;
  ReturnCode rc;
  switch (result) {
    case T_RC_COMPLETED_OK:
      rc = RC_OK;
      break;
    case T_RC_TX_ERROR:
      rc = RC_TX_ERROR;
      break;
    case T_RC_RX_TIMEOUT:
      rc = RC_RX_TIMEOUT;
      break;
    default:
      rc = RC_UNKNOWN;
  }

  switch (type) {
    case T_OP_TRANSCEIVER_NO_RESPONSE:
      command = TX_DMX;
      break;
    case T_OP_RDM_DUB:
      command = COMMAND_RDM_DUB_REQUEST;
      break;
    case T_OP_RDM_WITH_RESPONSE:
      command = COMMAND_RDM_REQUEST;
      break;
    default:
      SysLog_Print(SYSLOG_INFO, "Unknown Transceiver event %d", type);
      return;
  }
  SendMessage(command, rc, (IOVec*) &iovec, vector_size);
}
