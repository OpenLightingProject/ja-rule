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
 * receiver_counters.c
 * Copyright (C) 2015 Simon Newton
 */

#include "receiver_counters.h"

static const uint16_t UNINITIALIZED_COUNTER = 0xffff;
static const uint8_t UNINITIALIZED_CHECKSUM = 0xff;

/*
 * @brief The counters.
 */
ReceiverCounters g_responder_counters;

// Public Functions
// ----------------------------------------------------------------------------
void ReceiverCounters_ResetCounters() {
  ReceiverCounters_ResetCommsStatusCounters();

  g_responder_counters.dmx_frames = 0u;
  g_responder_counters.asc_frames = 0u;
  g_responder_counters.rdm_frames = 0u;
  g_responder_counters.rdm_sub_start_code_invalid = 0u;
  g_responder_counters.rdm_msg_len_invalid = 0u;
  g_responder_counters.rdm_param_data_len_invalid = 0u;
  // The initial values are from E1.37-5 (draft).
  g_responder_counters.dmx_last_checksum = UNINITIALIZED_CHECKSUM;
  g_responder_counters.dmx_last_slot_count = UNINITIALIZED_COUNTER;
  g_responder_counters.dmx_min_slot_count = UNINITIALIZED_COUNTER;
  g_responder_counters.dmx_max_slot_count = UNINITIALIZED_COUNTER;
}

void ReceiverCounters_ResetCommsStatusCounters() {
  g_responder_counters.rdm_short_frame = 0u;
  g_responder_counters.rdm_length_mismatch = 0u;
  g_responder_counters.rdm_checksum_invalid = 0u;
}
