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
 * logger.c
 * Copyright (C) 2015 Simon Newton
 */

#include "logger.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "constants.h"
#include "system_config.h"
#include "system_pipeline.h"

LoggerData g_logger;

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

static inline void SetLoggerIOVec(uint8_t index,
                                  void* base,
                                  unsigned int length,
                                  unsigned int *sum) {
  g_logger.iovec[index].base = base;
  g_logger.iovec[index].length = length;
  *sum += length;
}

void Logger_Initialize(TransportTXFunction tx_cb,
                        uint16_t max_payload_size) {
  g_logger.tx_cb = tx_cb;
  g_logger.read = -1;
  g_logger.write = 0;
  g_logger.enabled = false;
  g_logger.overflow = false;
  g_logger.max_payload_size = max_payload_size;
}

void Logger_SetState(bool enabled) {
  if (g_logger.enabled && !enabled) {
    g_logger.enabled = false;
    g_logger.overflow = false;
    g_logger.read = -1;
    g_logger.write = 0;
  } else if (!g_logger.enabled && enabled) {
    // enable, clear overflow bit and reset indices
    g_logger.enabled = true;
    g_logger.overflow = false;
    g_logger.read = -1;
    g_logger.write = 0;
  }
}

inline void PutChar(char c) {
  if (g_logger.write == g_logger.read) {
    return;
  }
  if (g_logger.read < 0) {
    g_logger.read = g_logger.write;
  }

  int16_t next = (g_logger.write + 1) % LOG_BUFFER_SIZE;
  if (next == g_logger.read) {
    // This is the last byte, NULL terminate & set overflow flag.
    g_logger.overflow = true;
    g_logger.log_buffer[g_logger.write] = 0;
  } else {
    g_logger.log_buffer[g_logger.write] = c;
  }
  g_logger.write = next;
}

void Logger_Log(const char* str) {
  if (!g_logger.enabled) {
    return;
  }
  while (*str) {
    PutChar(*str);
    str++;
  }
  PutChar(0);
}

void Logger_Write(const uint8_t* str, unsigned int length) {
  if (!g_logger.enabled) {
    return;
  }
  unsigned int i;
  for (i = 0; i < length; i++) {
    PutChar(str[i]);
  }
}


void _mon_putc(char c) {
  if (!(g_logger.enabled)) {
    return;
  }
  PutChar(c);
}

void Logger_SendResponse(uint8_t token) {
#ifndef PIPELINE_TRANSPORT_TX
  if (!g_logger.tx_cb) {
    return;
  }
#endif

  uint8_t iovec_index = 0;
  unsigned int payload_size = 0;

  // Populate the first byte (flags)
  uint8_t flags = g_logger.overflow;
  SetLoggerIOVec(iovec_index++, &flags, 1, &payload_size);
  g_logger.overflow = false;

  if (g_logger.read > g_logger.write) {
    // We've wrapped round the ring buffer.
    uint16_t chunk_size = LOG_BUFFER_SIZE - g_logger.read;
    chunk_size = min(chunk_size, g_logger.max_payload_size - payload_size);

    SetLoggerIOVec(iovec_index++,
                   g_logger.log_buffer + g_logger.read,
                   chunk_size, &payload_size);

    g_logger.read += chunk_size;
    if (g_logger.read == LOG_BUFFER_SIZE) {
      g_logger.read = g_logger.write ? 0 : -1;
    }
  }

  // Handle the data at the start of the buffer;
  if (g_logger.read >= 0 && payload_size < g_logger.max_payload_size) {
    uint16_t chunk_size;
    if (g_logger.read == g_logger.write) {
      chunk_size = LOG_BUFFER_SIZE;
    } else {
      chunk_size = g_logger.write - g_logger.read;
    }
    chunk_size = min(chunk_size, g_logger.max_payload_size - payload_size);

    SetLoggerIOVec(iovec_index++,
                   g_logger.log_buffer + g_logger.read,
                   chunk_size, &payload_size);

    g_logger.read = (g_logger.read + chunk_size) % LOG_BUFFER_SIZE;
    if (g_logger.read == g_logger.write) {
      g_logger.read = -1;
      g_logger.write = 0;
    }
  }
#ifdef PIPELINE_TRANSPORT_TX
  PIPELINE_TRANSPORT_TX(token, GET_LOG, RC_OK, g_logger.iovec, iovec_index);
#else
  g_logger.tx_cb(token, GET_LOG, RC_OK, g_logger.iovec, iovec_index);
#endif
}
