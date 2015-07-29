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
 * responder.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @defgroup responder Responder
 * @brief The Responder Subsystem.
 *
 * The responder receives data from the transceiver module and de-mulitplexes
 * based on start code.
 *
 * @addtogroup responder
 * @{
 * @file responder.h
 * @brief The Responder Subsystem.
 */

#ifndef FIRMWARE_SRC_RESPONDER_H_
#define FIRMWARE_SRC_RESPONDER_H_

#include <stdbool.h>
#include <stdint.h>
#include "transceiver.h"

#ifdef __cplusplus
extern "C" {
#endif

// @cond INTERNAL
typedef struct {
  uint32_t dmx_frames;
  uint32_t asc_frames;
  uint32_t rdm_frames;
  uint32_t rdm_sub_start_code_invalid;
  uint32_t rdm_msg_len_invalid;
  uint32_t rdm_param_data_len_invalid;
  uint32_t rdm_checksum_invalid;
  uint8_t dmx_last_checksum;
  uint16_t dmx_last_slot_count;
  uint16_t dmx_min_slot_count;
  uint16_t dmx_max_slot_count;
} ResponderCounters;

extern ResponderCounters g_responder_counters;
// @endcond


/**
 * @brief Initialize the Responder sub-system.
 */
void Responder_Initialize();

/**
 * @brief Reset the counters.
 */
void Responder_ResetCounters();

/**
 * @brief Called when data is received.
 * @param event The transceiver event.
 */
void Responder_Receive(const TransceiverEvent *event);

/**
 * @brief The number of DMX512 frames received.
 */
static inline uint32_t Responder_DMXFrames() {
  return g_responder_counters.dmx_frames;
}

/**
 * @brief The number of ASC frames received.
 */
static inline uint32_t Responder_ASCFrames() {
  return g_responder_counters.asc_frames;
}

/**
 * @brief The number of RDM frames received.
 */
static inline uint32_t Responder_RDMFrames() {
  return g_responder_counters.rdm_frames;
}

/**
 * @brief The number of RDM frames received where the sub-start-code was
 * incorrect.
 */
static inline uint32_t Responder_RDMSubStartCodeInvalidCounter() {
  return g_responder_counters.rdm_sub_start_code_invalid;
}

/**
 * @brief The number of RDM frames received where the message length was
 * incorrect.
 */
static inline uint32_t Responder_RDMMessageLengthInvalidCounter() {
  return g_responder_counters.rdm_msg_len_invalid;
}

/**
 * @brief The number of RDM frames received where the param data length was
 * incorrect.
 */
static inline uint32_t Responder_RDMParamDataLenInvalidCounter() {
  return g_responder_counters.rdm_param_data_len_invalid;
}

/**
 * @brief The number of RDM frames received where the checksum was incorrect.
 */
static inline uint32_t Responder_RDMChecksumInvalidCounter() {
  return g_responder_counters.rdm_checksum_invalid;
}

/**
 * @brief The additive checksum of the last DMX frame.
 *
 * If no DMX frames have been received, 0xff is reported.
 */
static inline uint32_t Responder_DMXLastChecksum() {
  return g_responder_counters.dmx_last_checksum;
}

/**
 * @brief The number of slots in the most recent DMX frame.
 *
 * If no DMX frames have been received, 0xffff is reported.
 */
static inline uint32_t Responder_DMXLastSlotCount() {
  return g_responder_counters.dmx_last_slot_count;
}

/**
 * @brief The smallest DMX frame seen.
 *
 * If no DMX frames have been received, 0xffff is reported. This is only
 * updated when the start of the next frame is received.
 */
static inline uint32_t Responder_DMXMinimumSlotCount() {
  return g_responder_counters.dmx_min_slot_count;
}

/**
 * @brief The largest DMX frame seen.
 *
 * If no DMX frames have been received, 0xffff is reported.
 */
static inline uint32_t Responder_DMXMaximumSlotCount() {
  return g_responder_counters.dmx_max_slot_count;
}

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_RESPONDER_H_
