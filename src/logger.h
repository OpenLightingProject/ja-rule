/* 
 * File:   logger.h
 * Author: Simon Newton
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

/**
 * @private
 */
extern LoggerData g_logger;

/**
 * @brief Initialize the Logging sub-system.
 * @param tx_cb The callback to use for sending messages when
 *   Logging_SendResponse is called.
 * @param enabled true to enable the logger, false to disable.
 * @param max_payload_size The maximum size of the payload to be passed to the
 *   TXFunction. Must be at least 2.
 */
void Logging_Initialize(TXFunction tx_cb, bool enabled,
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
 * It's not safe to call Logging_Log from an ISR.
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

#endif  // SRC_LOGGER_H_

