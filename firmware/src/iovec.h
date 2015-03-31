/*
 * File:   iovec.h
 * Author: Simon Newton
 */

/**
 * @addtogroup transport
 * @{
 * @file iovec.h
 * @brief Vector I/O.
 */

#ifndef FIRMWARE_SRC_IOVEC_H_
#define FIRMWARE_SRC_IOVEC_H_

#include <stdint.h>

/**
 * @brief An IOVector, with a pointer to memory and a length attribute.
 */
typedef struct {
  const void* base;   //!< A pointer to the data
  unsigned int length;  //!< The size of the data
} IOVec;

#endif  // FIRMWARE_SRC_IOVEC_H_

/**
 * @}
 */
