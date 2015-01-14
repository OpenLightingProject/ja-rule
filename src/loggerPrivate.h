/* 
 * File:   loggerPrivate..h
 * Author: Simon Newton
 */

#ifndef SRC_LOGGERPRIVATE_H_
#define SRC_LOGGERPRIVATE_H_

#include "constants.h"

#ifdef __cplusplus
extern "C" {
#endif

// Since we may have data in two regions of the buffer, we need up to 2 IOVecs
#define IOVEC_ARRAY_SIZE 2

typedef struct {
  TXFunction tx_cb;
  int16_t head;  // next index to read from. -1 if there is no data.
  int16_t tail;  // next index to write to.
  uint8_t enabled : 1;
  uint8_t overflow : 1;
  IOVec iovec[IOVEC_ARRAY_SIZE];
  // The circular buffer for log data
  char log_buffer[LOG_BUFFER_SIZE];
} LoggerData;

#ifdef __cplusplus
}
#endif

#endif  // SRC_LOGGERPRIVATE_H_
