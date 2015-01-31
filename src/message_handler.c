#include "message_handler.h"

#include "constants.h"
#include "dmx.h"
#include "flags.h"
#include "logger.h"
#include "system_definitions.h"

#ifndef PIPELINE_TRANSPORT_TX
static TXFunction g_message_tx_cb;
#endif

void MessageHandler_Initialize(TXFunction tx_cb) {
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

void MessageHandler_HandleMessage(const Message *message) {
  switch (message->command) {
    case ECHO:
      MessageHandler_Echo(message);
      break;
    case TX_DMX:
      DMX_BeginFrame(NULL_START_CODE, message->payload, message->length);
      DMX_FinalizeFrame();
      SendMessage(TX_DMX, RC_OK, NULL, 0);
      break;
    case GET_LOG:
      Logger_SendResponse();
      break;
    case GET_FLAGS:
      Flags_SendResponse();
      break;
    default:
      // Just echo the command code back if we don't understand it.
      SendMessage(message->command, RC_UNKNOWN, NULL, 0);
  }
}
