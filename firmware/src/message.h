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
 * message.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @addtogroup stream
 * @{
 * @file message.h
 * @brief The host to device message data structure.
 */

#ifndef FIRMWARE_SRC_MESSAGE_H_
#define FIRMWARE_SRC_MESSAGE_H_

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

#endif  // FIRMWARE_SRC_MESSAGE_H_

/**
 * @}
 */
