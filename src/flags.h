/*
 * File:   flags.h
 * Author: Simon Newton
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

#ifndef SRC_FLAGS_H_
#define SRC_FLAGS_H_

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
 * If PIPELINE_TRANSPORT_TX is defined in system_pipeline.h, the macro
 * will override the tx_cb argument.
 */
void Flags_Initialize(TXFunction tx_cb);

/**
 * @brief Check if the flags have changed since the last GET_FLAGS message.
 * @returns true if the flags have changed, false otherwise.
 */
static inline bool Flags_HasChanged() {
  return g_flags.has_changed;
}

/**
 * @brief Set the Log overflow flag.
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
 *
 * This uses the TXFunction passed in Flags_Initialize() to transmit the
 * frame.
 */
void Flags_SendResponse();

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // SRC_FLAGS_H_
