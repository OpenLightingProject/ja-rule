/* 
 * File:   message_handler.h
 * Author: Simon Newton
 *
 * Created on January 16, 2015, 8:21 AM
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

#ifndef MESSAGE_HANDLER_H
#define	MESSAGE_HANDLER_H

#include "stream_decoder.h"

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * @brief Handle messages from the Host System
 * @param message The message to handle, ownership is not transferred.
 *   Invalidated once the call completes.
 */
void HandleMessage(const Message *message);

#ifdef	__cplusplus
}
#endif

#endif	/* MESSAGE_HANDLER_H */

/**
 * @}
 */