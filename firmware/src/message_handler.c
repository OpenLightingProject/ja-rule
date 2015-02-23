/*
 * File:   message_handler.c
 * Author: Simon Newton
 */

#include "message_handler.h"

#include "constants.h"
#include "dmx.h"
#include "flags.h"
#include "logger.h"
#include "system_definitions.h"
#include "system_pipeline.h"
#include "syslog.h"

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

void MessageHandler_HandleMessage(const Message *message) {
  switch (message->command) {
    case ECHO:
      MessageHandler_Echo(message);
      break;
    case TX_DMX:
      DMX_QueueDMX(message->payload, message->length);
      SendMessage(TX_DMX, RC_OK, NULL, 0);
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
      SendMessage(COMMAND_RESET_DEVICE, RC_OK, NULL, 0);
      break;
    case COMMAND_RDM_DUB_REQUEST:
      DMX_QueueDUB(message->payload, message->length);
      // TODO(simon): Send the actual response here
      SendMessage(COMMAND_RDM_DUB_REQUEST, RC_OK, NULL, 0);
      break;
    case COMMANE_RDM_REQUEST:
      DMX_QueueRDMRequest(message->payload, message->length);
      // TODO(simon): Send the actual response here
      SendMessage(COMMANE_RDM_REQUEST, RC_OK, NULL, 0);
      break;
    default:
      // Just echo the command code back if we don't understand it.
      SendMessage(message->command, RC_UNKNOWN, NULL, 0);
  }
}
