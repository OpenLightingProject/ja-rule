/*
 * File:   logger_private..h
 * Author: Simon Newton
 */

#ifndef SRC_LOGGERPRIVATE_H_
#define SRC_LOGGERPRIVATE_H_

#include "constants.h"
#include "transport.h"

#ifdef __cplusplus
extern "C" {
#endif

// Since we may have data in two regions of the buffer and we need to prepend
// the flags byte, we may need up to 3 IOVecs
#define IOVEC_ARRAY_SIZE 3

typedef struct {
  TXFunction tx_cb;
  int16_t read;  // next index to read from. Range -1 to LOG_BUFFER_SIZE -1
  int16_t write;  // next index to write to. Range 0 to LOG_BUFFER_SIZE - 1
  uint8_t enabled : 1;  // true if logging is enabled
  uint8_t overflow : 1;  // true if an overflow has occurred.
  uint16_t max_payload_size;  // the maximum size for a payload.
  IOVec iovec[IOVEC_ARRAY_SIZE];
  // The circular buffer for log data
  char log_buffer[LOG_BUFFER_SIZE];
} LoggerData;

#ifdef __cplusplus
}
#endif

#endif  // SRC_LOGGERPRIVATE_H_
