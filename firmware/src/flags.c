/*
 * File:   flags.c
 * Author: Simon Newton
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
  bool ok = PIPELINE_TRANSPORT_TX(token, GET_FLAGS, RC_OK, &iovec, 1);
#else
  bool ok = g_flags_tx_cb(token, GET_FLAGS, RC_OK, &iovec, 1);
#endif
  if (ok) {
    g_flags.has_changed = false;
    memset(&g_flags.flags, 0, sizeof(g_flags.flags));
  }
}
