#include "message_handler.h"

#include "constants.h"
#include "system_definitions.h"


void HandleMessage(const Message *message) {
  BSP_LEDToggle(BSP_LED_1);
  switch (message->command) {
    case ECHO:
      SendResponse(ECHO, RC_OK, message->payload, message->length);
      break;
    case TX_DMX:
      DMX_BeginFrame(message->payload, message->length);
      DMX_FinalizeFrame();
      SendResponse(TX_DMX, RC_OK, NULL, 0);
      break;
    case GET_LOG:
      Logging_SendResponse();
      break;
    default:
      BSP_LEDToggle(BSP_LED_2);
  }
}
