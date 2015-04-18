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
 * stream_decoder.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @defgroup stream Stream Decoder
 * @brief Unpack messages from the host.
 *
 * @addtogroup stream
 * @{
 * @file stream_decoder.h
 * @brief Unpack messages from the host.
 */

#ifndef FIRMWARE_SRC_STREAM_DECODER_H_
#define FIRMWARE_SRC_STREAM_DECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
  * @brief A de-serialized message.
  */
typedef struct {
  uint8_t token;  //!< The token associated with this message.
  uint16_t command;  //!< The Command
  uint16_t length;   //!< The length of the message's payload
  const uint8_t* payload;  //!< A pointer to the payload data.
} Message;


/**
 * @brief A function pointer used to handle new messages.
 * @param message The message to handle.
 */
typedef void (*MessageHandler)(const Message*);


/**
 * @brief Initialize the Stream Decoder.
 * @param handler The MessageHandler to call with each new message.
 *
 * If PIPELINE_HANDLE_MESSAGE is defined in system_pipeline.h, the macro
 * will override the handler argument.
 */
void StreamDecoder_Initialize(MessageHandler handler);

/**
 * @brief Get the value of the fragmented frame flag.
 *
 * This indicates if a fragmented frame has been received. Fragmentation is
 * expensive as it incurs an extra copy.
 */
bool StreamDecoder_GetFragmentedFrameFlag();

/**
 * @brief Clear the fragmented frame flag.
 */
void StreamDecoder_ClearFragmentedFrameFlag();

/**
 * @brief Decode data from an input stream.
 * @param data A pointer to the incoming data.
 * @param size The size of the incommig data buffer.
 *
 * Since this may result in a response being sent, this should only be called
 * if there is space available in the Host TX buffer.
 */
void StreamDecoder_Process(const uint8_t* data, unsigned int size);

#ifdef __cplusplus
}
#endif

#endif  // FIRMWARE_SRC_STREAM_DECODER_H_

/**
 * @}
 */
