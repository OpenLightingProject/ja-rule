/*
 * File:   stream_decode.h
 * Author: Simon N
 */

#include "stream_decode.h"
#include <stdlib.h>

#include "system_definitions.h"
#include "constants.h"
#include "usb.h"

// The state indicates the next byte we expect

typedef enum {
  START_OF_MESSAGE,
  COMMAND_LOW,
  COMMAND_HIGH,
  LENGTH_LOW,
  LENGTH_HIGH,
  PAYLOAD,
  END_OF_MESSAGE,
  PADDING
} StreamDecodeState;

typedef struct {
  StreamDecodeState state;
  uint16_t length;
  uint16_t command;
  const uint8_t* payload;
  unsigned int payload_offset;
} StreamDecodeData;

StreamDecodeData g_stream_data;

void StreamDecode_Initialize(void) {
  g_stream_data.state = START_OF_MESSAGE;
  g_stream_data.length = 0;
  g_stream_data.command = 0;
  g_stream_data.payload = NULL;
  g_stream_data.payload_offset = 0;
}

void HandleMessage() {
  BSP_LEDToggle(BSP_LED_1);
  switch (g_stream_data.command) {
    case ECHO:
      SendResponse(ECHO, RC_OK, g_stream_data.payload, g_stream_data.length);
      break;
    case TX_DMX:
      DMX_BeginFrame(g_stream_data.payload, g_stream_data.length);
      DMX_FinalizeFrame();
      SendResponse(TX_DMX, RC_OK, NULL, 0);
      break;
    default:
      BSP_LEDToggle(BSP_LED_2);
  }
}

void StreamDecode_Process(const uint8_t* data, unsigned int size) {
  const uint8_t *end = data + size;
  const uint8_t *start = data;

  while (data < end) {
    switch (g_stream_data.state) {
      case START_OF_MESSAGE:
        for (; data < end && *data != START_OF_MESSAGE_ID; data++);
        if (*data == START_OF_MESSAGE_ID) {
          g_stream_data.state = COMMAND_LOW;
        }
        break;
      case COMMAND_LOW:
        g_stream_data.command = (*data & 0xff);
        g_stream_data.state = COMMAND_HIGH;
        break;
      case COMMAND_HIGH:
        g_stream_data.command |= (*data << 8);
        g_stream_data.state = LENGTH_LOW;
        break;
      case LENGTH_LOW:
        g_stream_data.length = (*data & 0xff);
        g_stream_data.state = LENGTH_HIGH;
        break;
      case LENGTH_HIGH:
        g_stream_data.length |= (*data << 8);
        if (g_stream_data.length > 0) {
          g_stream_data.state = PAYLOAD;
        } else {
          g_stream_data.state = END_OF_MESSAGE;
        }
        break;
      case PAYLOAD:
        // This is where it gets tricky.
        if (end - data < g_stream_data.length + 1) {
          BSP_LEDToggle(BSP_LED_2);
        } else {
          g_stream_data.payload = data;
          g_stream_data.state = END_OF_MESSAGE;
        }
      case END_OF_MESSAGE:
        if (*data == END_OF_MESSAGE_ID) {
          HandleMessage();
        }
    }
    data++;
  }
}
