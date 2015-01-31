/*
 * File:   message.h
 * Author: Simon N
 */

/**
 * @addtogroup stream
 * @{
 * @file message.h
 * @brief The host to device message data structure.
 */

#ifndef SRC_MESSAGE_H_
#define SRC_MESSAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
  * @brief A de-serialized message.
  */
typedef struct {
  uint16_t command;  //!< The Command
  uint16_t length;   //!< The payload length
  const uint8_t* payload;  //!< A pointer to the payload data.
} Message;


/**
 * @brief A function pointer used to handle new messages.
 * @param message The message to handle.
 */
typedef void (*MessageHandler)(const Message*);

#ifdef __cplusplus
}
#endif

#endif  // SRC_MESSAGE_H_

/**
 * @}
 */
