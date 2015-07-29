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
 * flags.c
 * Copyright (C) 2015 Simon Newton
 */

#include "flags.h"

#include <string.h>

#include "constants.h"
#include "system_pipeline.h"

FlagsData g_flags;

#ifndef PIPELINE_TRANSPORT_TX
static TransportTXFunction g_flags_tx_cb;
#endif

void Flags_Initialize(TransportTXFunction tx_cb) {
  g_flags.has_changed = false;
  memset(&g_flags.flags, 0, sizeof(g_flags.flags));
#ifndef PIPELINE_TRANSPORT_TX
  g_flags_tx_cb = tx_cb;
#endif
}

void Flags_SendResponse(uint8_t token) {
#ifndef PIPELINE_TRANSPORT_TX
  if (!g_flags_tx_cb) {
    return;
  }
#endif

  IOVec iovec;
  iovec.base = &g_flags.flags;
  iovec.length = sizeof(g_flags.flags);

#ifdef PIPELINE_TRANSPORT_TX
  bool ok = PIPELINE_TRANSPORT_TX(token, GET_FLAGS, RC_OK, &iovec, 1u);
#else
  bool ok = g_flags_tx_cb(token, GET_FLAGS, RC_OK, &iovec, 1u);
#endif
  if (ok) {
    g_flags.has_changed = false;
    memset(&g_flags.flags, 0, sizeof(g_flags.flags));
  }
}
