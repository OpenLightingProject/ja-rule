/* 
 * File:   stream_decode.h
 * Author: Simon N
 */

#ifndef SRC_STREAM_DECODE_H_
#define SRC_STREAM_DECODE_H_

#include <stdint.h>

/**
 * @brief Initialize the Stream Decoder
 */
void StreamDecode_Initialize(void);

/**
 * @brief Decode data from an input stream.
 * @param data
 * @param size
 *
 * Since this may result in a response being sent, this should only be called if
 */
void StreamDecode_Process(const uint8_t* data, unsigned int size);


#endif  // SRC_STREAM_DECODE_H_

