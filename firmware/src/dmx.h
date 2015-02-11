/* 
 * File:   dmx.h
 * Author: Simon Newton
 */

#ifndef SRC_DMX_H_
#define SRC_DMX_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the DMX layer.
 */
void DMX_Initialize();

/**
 * @brief Perform the periodic DMX layer tasks
 */
void DMX_Tasks();

/**
 * @brief Indicate there is a new frame to send.
 * @param start_code The frame's start code.
 * @param data The DMX data in the frame. May be partial data.
 * @param size The size o
 */
void DMX_BeginFrame(uint8_t start_code, const uint8_t* data, unsigned int size);

/**
 * @brief
 */
void DMX_FinalizeFrame();

#ifdef __cplusplus
}
#endif

#endif  // SRC_DMX_H_

