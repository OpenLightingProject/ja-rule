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
 * flags.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @defgroup flags Flags
 * @brief Flags track the occurence of abnormal events.
 *
 * A flag is a single-bit variable which can be set when an abnormal event
 * occurs. The next response frame sent to the Host will have the flags-changed
 * bit set, which informs the host it should send a GET_FLAGS command.
 *
 * On receiving a GET_FLAGS command, the Device sends the packed flags
 * structure as the message payload.
 *
 * @addtogroup flags
 * @{
 * @file flags.h
 * @brief The Flags Module.
 */

#ifndef FIRMWARE_SRC_FLAGS_H_
#define FIRMWARE_SRC_FLAGS_H_

#include "flags_private.h"
#include "transport.h"

#ifdef __cplusplus
extern "C" {
#endif

/// @cond INTERNAL
extern FlagsData g_flags;
/// @endcond

/**
 * @brief Initialize the Flags sub-system.
 * @param tx_cb The callback to use for sending messages when
 *   Flags_SendResponse() is called. This can be overridden, see the note
 *   below.
 *
 * If PIPELINE_TRANSPORT_TX is defined in app_pipeline.h, the macro
 * will override the tx_cb argument.
 */
void Flags_Initialize(TransportTXFunction tx_cb);

/**
 * @brief Check if the flags have changed since the last GET_FLAGS message.
 * @returns true if the flags have changed, false otherwise.
 */
static inline bool Flags_HasChanged() {
  return g_flags.has_changed;
}

/**
 * @brief Set the log overflow flag.
 *
 * This indicates the Logger buffer overflowed and some messages were dropped.
 */
static inline void Flags_SetLogOverflow() {
  g_flags.flags.log_overflow = true;
  g_flags.has_changed = true;
}

/**
 * @brief Set the TX Drop flag.
 *
 * This indicates we tried to send a message to the host before the previous
 * message completed sending.
 */
static inline void Flags_SetTXDrop() {
  g_flags.flags.tx_drop = true;
  g_flags.has_changed = true;
}

/**
 * @brief Set the TX Error flag.
 *
 * This indicates USB_DEVICE_EndpointWrite returned an error.
 */
static inline void Flags_SetTXError() {
  g_flags.flags.tx_error = true;
  g_flags.has_changed = true;
}

/**
 * @brief Send a flags message.
 * @param token The token to include in the response.
 *
 * This uses the TransportTXFunction passed in Flags_Initialize() to transmit
 * the frame.
 */
void Flags_SendResponse(uint8_t token);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_FLAGS_H_
