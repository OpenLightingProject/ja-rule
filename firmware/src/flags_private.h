/*
 * File:   flags_private.h
 * Author: Simon Newton
 */

#ifndef SRC_FLAGS_PRIVATE_H_
#define SRC_FLAGS_PRIVATE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// See flags.h for what each flag tracks.
typedef struct {
  uint8_t log_overflow : 1;
  uint8_t tx_drop : 1;
  uint8_t tx_error : 1;
} FlagsState;

typedef struct {
  bool has_changed;
  FlagsState flags;
} FlagsData;

#ifdef __cplusplus
}
#endif

#endif  // SRC_FLAGS_PRIVATE_H_
