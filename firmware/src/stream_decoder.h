/*
 * File:   stream_decoder.h
 * Author: Simon N
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
  uint16_t command;  //!< The Command
  uint16_t length;   //!< The payload length
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
