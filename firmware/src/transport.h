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
 * transport.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @defgroup transport Transport
 * @brief The Host <-> Device communication transport.
 *
 * This contains the types used to transfer messages between the Host
 * (typically a machine running a full OS) and the PIC device.
 *
 * @addtogroup transport
 * @{
 * @file transport.h
 * @brief The Host <-> Device communication transport.
 */

#ifndef FIRMWARE_SRC_TRANSPORT_H_
#define FIRMWARE_SRC_TRANSPORT_H_

#include <stdint.h>
#include "constants.h"
#include "iovec.h"

/**
 * @brief Flags use in a response message.
 */
typedef enum {
  TRANSPORT_FLAGS_CHANGED = 0x02,  //!< Flags have changed
  TRANSPORT_MSG_TRUNCATED = 0x04  //!< The message has been truncated.
} TransportFlags;

/**
 * @brief A function pointer to send a message to the host
 * @param token The frame token, this should match the request.
 * @param command the Command identifier to send
 * @param rc The 8-bit return code.
 * @param iov A pointer to an array of IOVec structures. The data will be
 *   copied.
 * @param iov_count The number of IOVec structures in the array.
 */
typedef bool (*TransportTXFunction)(uint8_t, Command, uint8_t, const IOVec*,
                                    unsigned int);

/**
 * @brief A function pointer to call when data is received from the host.
 * @param data A pointer to the new data.
 * @param size The size of the data received.
 */
typedef void (*TransportRxFunction)(const uint8_t*, unsigned int);

#endif  // FIRMWARE_SRC_TRANSPORT_H_

/**
 * @}
 */
