/* 
 * File:   transport.h
 * Author: Simon Newton
 */

/**
 * @defgroup transport Transport
 * @brief The generic Host / Device communication transport.
 *
 *
 * @addtogroup transport
 * @{
 * @file transport.h
 * @brief The Host / Device communication transport.
 */

#ifndef SRC_TRANSPORT_H_
#define SRC_TRANSPORT_H_

#include <stdint.h>
#include "constants.h"

/**
 * @brief An IOVector, with a pointer to memory and a length attribute.
 */
typedef struct {
  void* base;   /**< @brief A pointer to the data */
  unsigned int length;  /**< @brief The size of the data */
} IOVec;

/**
 * @brief A function pointer to send a message to the host
 * @param command the Command indentifier to send
 * @param rc The 8-bit return code.
 * @param iov A pointer to an array of IOVec structures. The data will be
 *   copied.
 * @param iov_count The number of IOVec structures in the array.
 */
typedef void (*TXFunction)(Command, uint8_t, const IOVec*,
                           unsigned int);

// typedef void (*RXFunction)(const uint8_t*, unsigned int));

#endif  // SRC_TRANSPORT_H_

/**
 * @}
 */
