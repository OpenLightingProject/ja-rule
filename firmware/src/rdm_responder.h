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
 * rdm_responder.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @defgroup rdm_responder RDM Responder
 * @brief The RDM Responder Subsystem.
 *
 * @addtogroup rdm_responder
 * @{
 * @file rdm_responder.h
 * @brief The RDM Responder Subsystem.
 */

#ifndef FIRMWARE_SRC_RDM_RESPONDER_H_
#define FIRMWARE_SRC_RDM_RESPONDER_H_

#include <stdbool.h>
#include <stdint.h>
#include "iovec.h"
#include "rdm_frame.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The callback used to send RDM responses.
 * @param include_break true if a break should be used for the response
 * @param data the raw data to send in the response
 * @param iov_count the number of IOVec that make up the response
 */
typedef void (*RDMResponderSendCallback)(bool include_break,
                                         const IOVec* data,
                                         unsigned int iov_count);



/**
 * @brief Initialize the RDM Responder sub-system.
 * @param uid The UID to use for this responder.
 * @param send_callback The callback to use for sending responses.
 *
 * If PIPELINE_RDMRESPONDER_SEND is defined this will override the
 * send_callback.
 */
void RDMResponder_Initialize(const uint8_t uid[UID_LENGTH],
                             RDMResponderSendCallback send_callback);

/**
 * @brief Check if a destination UID requires us to take action.
 * @returns True if a command sent to this UID is addressed to this responder.
 */
bool RDMResponder_UIDRequiresAction(const uint8_t uid[UID_LENGTH]);

/**
 * @brief Validate the checksum for an RDM frame.
 * @param frame The frame data, starting with the start code.
 * @param size The length of the frame data.
 * @returns True if the checksum was valid, false otherwise.
 */
bool RDMResponder_VerifyChecksum(const uint8_t *frame, unsigned int size);

/**
 * @brief Handle a RDM Request.
 * @pre Sub-Start-Code is SUB_START_CODE.
 * @pre message_length is valid.
 * @pre RDMResponder_UIDRequiresAction() returned true.
 * @pre The checksum of the command is correct
 * @param header The RDM command header.
 * @param param_data the parameter data
 * @param length The length of parameter data.
 */
void RDMResponder_HandleRequest(const RDMHeader *header,
                                const uint8_t *param_data,
                                unsigned int length);


/**
 * @brief Check if the responder is muted.
 * @returns true if this responder is currently muted.
 */
bool RDMResponder_IsMuted();


#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_RDM_RESPONDER_H_
