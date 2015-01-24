/* 
 * File:   usb.h
 * Author: Simon Newton
 */

#ifndef SRC_USB_H_
#define SRC_USB_H_

#include <stdint.h>
#include "constants.h"

/**
 * @brief Initialize the USB layer.
 */
void USB_Initialize(void);

/**
 * @brief Perform the periodic USB layer tasks.
 */
void USB_Tasks(void);

/**
 * @brief Send a response.
 * @param command The command class of the response.
 * @param rc The return code of the response.
 * @param data The payload data.
 * @param data_length The length of the payload data.
 */
void SendResponse(Command command, uint8_t rc, const uint8_t* data,
                  unsigned int data_length);

#endif  // SRC_USB_H_

