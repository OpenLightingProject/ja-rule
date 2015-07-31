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
 * responder.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @defgroup responder Responder
 * @brief The Responder Subsystem.
 *
 * The responder receives data from the transceiver module and de-mulitplexes
 * based on start code.
 *
 * @addtogroup responder
 * @{
 * @file responder.h
 * @brief The Responder Subsystem.
 */

#ifndef FIRMWARE_SRC_RESPONDER_H_
#define FIRMWARE_SRC_RESPONDER_H_

#include "transceiver.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Responder sub-system.
 */
void Responder_Initialize();

/**
 * @brief Called when data is received.
 * @param event The transceiver event.
 */
void Responder_Receive(const TransceiverEvent *event);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_RESPONDER_H_
