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
 * receiver_counters.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @addtogroup responder
 * @{
 * @file receiver_counters.h
 * @brief Counters for the DMX/RDM receiver.
 */

#ifndef FIRMWARE_SRC_RECEIVIER_COUNTERS_H_
#define FIRMWARE_SRC_RECEIVIER_COUNTERS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// @cond INTERNAL
typedef struct {
  uint32_t dmx_frames;
  uint32_t asc_frames;
  uint32_t rdm_frames;
  uint32_t rdm_short_frame;
  uint32_t rdm_length_mismatch;
  uint32_t rdm_sub_start_code_invalid;
  uint32_t rdm_msg_len_invalid;
  uint32_t rdm_param_data_len_invalid;
  uint32_t rdm_checksum_invalid;
  uint8_t dmx_last_checksum;
  uint16_t dmx_last_slot_count;
  uint16_t dmx_min_slot_count;
  uint16_t dmx_max_slot_count;
} ReceiverCounters;

// @endcond

/**
 * @brief The counters for the receiver.
 */
extern ReceiverCounters g_responder_counters;

/**
 * @brief Reset the counters.
 */
void ReceiverCounters_ResetCounters();

/**
 * @brief Reset the COMMS_STATUS counters.
 */
void ReceiverCounters_ResetCommsStatusCounters();

/**
 * @brief The number of DMX512 frames received.
 */
static inline uint32_t ReceiverCounters_DMXFrames() {
  return g_responder_counters.dmx_frames;
}

/**
 * @brief The number of ASC frames received.
 */
static inline uint32_t ReceiverCounters_ASCFrames() {
  return g_responder_counters.asc_frames;
}

/**
 * @brief The number of RDM frames received.
 */
static inline uint32_t ReceiverCounters_RDMFrames() {
  return g_responder_counters.rdm_frames;
}

/**
 * @brief The number of RDM frames that were too short.
 *
 * See COMMS_STATUS from E1.20 for a description
 */
static inline uint32_t ReceiverCounters_RDMShortFrame() {
  return g_responder_counters.rdm_short_frame;
}

/**
 * @brief The number of RDM frames that had a length mismatch.
 *
 * See COMMS_STATUS from E1.20 for a description
 */
static inline uint32_t ReceiverCounters_RDMLengthMismatch() {
  return g_responder_counters.rdm_length_mismatch;
}

/**
 * @brief The number of RDM frames received where the sub-start-code was
 * incorrect.
 */
static inline uint32_t ReceiverCounters_RDMSubStartCodeInvalidCounter() {
  return g_responder_counters.rdm_sub_start_code_invalid;
}

/**
 * @brief The number of RDM frames received where the message length was
 * incorrect.
 */
static inline uint32_t ReceiverCounters_RDMMessageLengthInvalidCounter() {
  return g_responder_counters.rdm_msg_len_invalid;
}

/**
 * @brief The number of RDM frames received where the param data length was
 * incorrect.
 */
static inline uint32_t ReceiverCounters_RDMParamDataLenInvalidCounter() {
  return g_responder_counters.rdm_param_data_len_invalid;
}

/**
 * @brief The number of RDM frames received where the checksum was incorrect.
 */
static inline uint32_t ReceiverCounters_RDMChecksumInvalidCounter() {
  return g_responder_counters.rdm_checksum_invalid;
}

/**
 * @brief The additive checksum of the last DMX frame.
 *
 * If no DMX frames have been received, 0xff is reported.
 */
static inline uint32_t ReceiverCounters_DMXLastChecksum() {
  return g_responder_counters.dmx_last_checksum;
}

/**
 * @brief The number of slots in the most recent DMX frame.
 *
 * If no DMX frames have been received, 0xffff is reported.
 */
static inline uint32_t ReceiverCounters_DMXLastSlotCount() {
  return g_responder_counters.dmx_last_slot_count;
}

/**
 * @brief The smallest DMX frame seen.
 *
 * If no DMX frames have been received, 0xffff is reported. This is only
 * updated when the start of the next frame is received.
 */
static inline uint32_t ReceiverCounters_DMXMinimumSlotCount() {
  return g_responder_counters.dmx_min_slot_count;
}

/**
 * @brief The largest DMX frame seen.
 *
 * If no DMX frames have been received, 0xffff is reported.
 */
static inline uint32_t ReceiverCounters_DMXMaximumSlotCount() {
  return g_responder_counters.dmx_max_slot_count;
}

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_RECEIVIER_COUNTERS_H_
