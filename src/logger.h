/*
 * File:   logger.h
 * Author: Simon Newton
 */

/**
 * @defgroup logger Logger
 * @brief The Logging Subsystem.
 *
 * The Logger uses a ring buffer to store log messages. The messages can then be
 * retrieved by the host system with a GET_LOG comand, which would then call
 * Logging_SendResponse().
 *
 * An overflow occurs if there is more data than what would fit in the ring
 * buffer. In this case, as much data as possible is saved and
 * Logging_HasOverflowed() will return true. When the next call to
 * Logging_SendResponse() is made the overflow flag will be reset.
 *
 * @addtogroup logger
 * @{
 * @file logger.h
 * @brief The Logging Subsystem.
 */

#ifndef SRC_LOGGER_H_
#define SRC_LOGGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "transport.h"
#include "loggerPrivate.h"

/// @cond INTERNAL
extern LoggerData g_logger;
/// @endcond

/**
 * @brief Initialize the Logging sub-system.
 * @param tx_cb The callback to use for sending messages when
 *   Logging_SendResponse() is called. This can be overridden, see the note
 *   below.
 * @param max_payload_size The maximum size of the payload to be passed to the
 *   TXFunction. Must be at least 2.
 *
 * A Logger starts off in the disabled state.
 *
 * If PIPELINE_TRANSPORT_TX is defined in system_pipeline.h, the macro
 * will override the tx_cb argument.
 */
void Logging_Initialize(TXFunction tx_cb,
                        uint16_t max_payload_size);

/**
 * @brief Change the state of the logger.
 * @param enabled The new mode.
 *
 * When disabled, all log output will be discarded. Disabling an enabled logger
 * will reset the pending and overflow flags.
 */
void Logging_SetState(bool enabled);

/**
 * @brief Check if the logger is enabled.
 * @returns true if the logger is enabled, false otherwise.
 */
extern inline bool Logging_IsEnabled() {
  return g_logger.enabled;
}

/**
 * @brief Check if there is log data pending.
 * @returns true if there is log data pending, false otherwise.
 */
extern inline bool Logging_DataPending() {
  return g_logger.read != -1;
}

/**
 * @brief Check if the log buffer has overflowed
 * @returns true if the buffer has overflowed, false otherwise.
 */
extern inline bool Logging_HasOverflowed() {
  return g_logger.overflow;
}

/**
 * @brief Log a message.
 * @param str The string to log.
 *
 * @note It's not safe to call Logging_Log from an ISR.
 */
void Logging_Log(const char* str);

/**
 * @brief Send a Log message.
 *
 * This uses the TXFunction passed in Logging_Initialize() to transmit the
 * frame.
 */
void Logging_SendResponse();

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // SRC_LOGGER_H_

