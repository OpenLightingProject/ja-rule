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
 * message_handler.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @defgroup message_handler Message Handler
 * @brief Handle messages from the Host.
 *
 * @addtogroup message_handler
 * @{
 * @file message_handler.h
 * @brief Handle messages from the Host.
 */

#ifndef FIRMWARE_SRC_MESSAGE_HANDLER_H_
#define FIRMWARE_SRC_MESSAGE_HANDLER_H_

#include "stream_decoder.h"
#include "transceiver.h"
#include "transport.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Message Handler sub-system.
 * @param tx_cb The callback to use for sending messages.
 *
 * If PIPELINE_TRANSPORT_TX is defined in app_pipeline.h, the macro
 * will override the tx_cb argument.
 */
void MessageHandler_Initialize(TransportTXFunction tx_cb);

/**
 * @brief Handle messages from the Host System
 * @param message The message to handle, ownership is not transferred.
 *   Invalidated once the call completes.
 */
void MessageHandler_HandleMessage(const Message* message);

/**
 * @brief Handle notifications when the transceiver operations complete.
 * @param event the TransceiverEvent.
 * @sa TransceiverEventCallback.
 */
void MessageHandler_TransceiverEvent(const TransceiverEvent *event);

#ifdef __cplusplus
}
#endif

#endif  // FIRMWARE_SRC_MESSAGE_HANDLER_H_

/**
 * @}
 */
