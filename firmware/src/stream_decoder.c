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
 * stream_decoder.c
 * Copyright (C) 2015 Simon Newton
 */

#include "stream_decoder.h"

#include <stdlib.h>
#include <string.h>

#include "app_pipeline.h"
#include "constants.h"

// Microchip defines this macro in stdlib.h but it's non standard.
// We define it here so that the unit tests work.
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

// The state indicates the next byte we expect

typedef enum {
  START_OF_MESSAGE,
  TOKEN,
  COMMAND_LOW,
  COMMAND_HIGH,
  LENGTH_LOW,
  LENGTH_HIGH,
  PAYLOAD,
  END_OF_MESSAGE,
} StreamDecoderState;

typedef struct {
  StreamDecoderState state;
#ifndef PIPELINE_HANDLE_MESSAGE
  MessageHandler handler;
#endif
  Message message;
  unsigned int fragment_offset;
  uint8_t fragmented_buffer[PAYLOAD_SIZE];
  uint8_t fragmented_frame : 1;  // true if we've received a fragmented frame
} StreamDecoderData;

StreamDecoderData g_stream_data;

// Public Functions
// ----------------------------------------------------------------------------
void StreamDecoder_Initialize(MessageHandler handler) {
  g_stream_data.state = START_OF_MESSAGE;
#ifndef PIPELINE_HANDLE_MESSAGE
  g_stream_data.handler = handler;
#endif
  g_stream_data.message.token = 0u;
  g_stream_data.message.length = 0u;
  g_stream_data.message.command = 0u;
  g_stream_data.message.payload = NULL;
  g_stream_data.fragment_offset = 0u;
  g_stream_data.fragmented_frame = false;
}

bool StreamDecoder_GetFragmentedFrameFlag() {
  return g_stream_data.fragmented_frame;
}

void StreamDecoder_ClearFragmentedFrameFlag() {
  g_stream_data.fragmented_frame = false;
}

void StreamDecoder_Process(const uint8_t* data, unsigned int size) {
#ifndef PIPELINE_HANDLE_MESSAGE
  if (!g_stream_data.handler) {
    return;
  }
#endif

  const uint8_t *end = data + size;
  uint32_t payload_size;

  while (data < end) {
    switch (g_stream_data.state) {
      case START_OF_MESSAGE:
        for (; data < end; data++) {
          if (*data == START_OF_MESSAGE_ID) {
            g_stream_data.state = TOKEN;
            break;
          }
        }
        break;
      case TOKEN:
        g_stream_data.message.token = *data;
        g_stream_data.state = COMMAND_LOW;
        break;
      case COMMAND_LOW:
        g_stream_data.message.command = (*data & 0xff);
        g_stream_data.state = COMMAND_HIGH;
        break;
      case COMMAND_HIGH:
        g_stream_data.message.command |= (*data << 8);
        g_stream_data.state = LENGTH_LOW;
        break;
      case LENGTH_LOW:
        g_stream_data.message.length = (*data & 0xff);
        g_stream_data.state = LENGTH_HIGH;
        break;
      case LENGTH_HIGH:
        g_stream_data.message.length |= (*data << 8);
        if (g_stream_data.message.length > 0u) {
          g_stream_data.state = PAYLOAD;
        } else {
          g_stream_data.state = END_OF_MESSAGE;
        }
        g_stream_data.fragment_offset = 0u;
        break;
      case PAYLOAD:
        // This frame is fragmented, which means we need to reassemble in the
        // fragment buffer. Fragmentation is expensive.
        payload_size = end - data;
        if (payload_size < g_stream_data.message.length + 1u ||
            g_stream_data.fragment_offset != 0u) {
          g_stream_data.fragmented_frame = true;
          payload_size = min(
              payload_size,
              g_stream_data.message.length - g_stream_data.fragment_offset);
          memcpy(
              g_stream_data.fragmented_buffer + g_stream_data.fragment_offset,
              data,
              payload_size);
          g_stream_data.fragment_offset += payload_size;
          data += payload_size;
          if (g_stream_data.fragment_offset == g_stream_data.message.length) {
            g_stream_data.state = END_OF_MESSAGE;
            g_stream_data.message.payload = g_stream_data.fragmented_buffer;
          }
        } else {
          g_stream_data.message.payload = data;
          g_stream_data.state = END_OF_MESSAGE;
          data += g_stream_data.message.length;
        }
        data--;
        break;
      case END_OF_MESSAGE:
        if (*data == END_OF_MESSAGE_ID) {
#ifdef PIPELINE_HANDLE_MESSAGE
          PIPELINE_HANDLE_MESSAGE(&g_stream_data.message);
#else
          g_stream_data.handler(&g_stream_data.message);
#endif
        }
        g_stream_data.fragment_offset = 0u;
        g_stream_data.state = START_OF_MESSAGE;
    }
    data++;
  }
}
