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
 * rdm_handler.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @defgroup rdm_handler RDM Request Handler
 * @brief The handler for RDM requests.
 *
 * The RDM Handler manages dispatching the RDM requests to the correct model.
 * This allows the physical device to simulate different types of RDM devices
 * from LED drivers and moving lights to sensor-only devices and fog machines.
 *
 * Only a single model can be active at once, the PID_DEVICE_MODEL parameter
 * can be used to change models at run time. Note that PID_DEVICE_MODEL and
 * PID_DEVICE_MODEL_LIST aren't included in SUPPORTED_PARAMETERS so there are
 * 'hidden' PIDs.
 *
 * @addtogroup rdm_handler
 * @{
 * @file rdm_handler.h
 * @brief The handler for RDM requests.
 */

#ifndef FIRMWARE_SRC_RDM_HANDLER_H_
#define FIRMWARE_SRC_RDM_HANDLER_H_

#include <stdbool.h>
#include <stdint.h>
#include "iovec.h"
#include "rdm_frame.h"
#include "rdm_model.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The callback used to send RDM responses.
 * @param include_break true if a break should be used for the response
 * @param data the raw data to send in the response
 * @param iov_count the number of IOVec that make up the response
 */
typedef void (*RDMHandlerSendCallback)(bool include_break,
                                       const IOVec* data,
                                       unsigned int iov_count);

/**
 * @brief The settings to use for the RDM Handler.
 */
typedef struct {
  uint16_t default_model;  //!< The default model to use

  /**
   * @brief The callback to use for sending.
   *
   * If PIPELINE_RDMRESPONDER_SEND is defined this will override the
   * send_callback.
   */
  RDMHandlerSendCallback send_callback;
} RDMHandlerSettings;

/**
 * @brief Initialize the RDM Handler sub-system.
 * @param settings The settings for the handler.
 */
void RDMHandler_Initialize(const RDMHandlerSettings *settings);

/**
 * @brief Add a model to the list of available models.
 * @brief model_entry The ModelEntry to add.
 * @returns true if the model was added, false if there weren't enough slots or
 * the model already existed.
 *
 * This should be called after to RDMHandler_Initialize().
 */
bool RDMHandler_AddModel(const ModelEntry *entry);

/**
 * @brief Change the active model of responder.
 * @param model_id the new model to use.
 * @returns true if the new model was set, false if the new model doesn't exist.
 */
bool RDMHandler_SetActiveModel(uint16_t model_id);

/**
 * @brief Get the active model id.
 * @returns the active model id.
 */
uint16_t RDMHandler_ActiveModel();

/**
 * @brief Handle a RDM Request.
 * @pre Sub-Start-Code is SUB_START_CODE.
 * @pre message_length is valid.
 * @pre The checksum of the command is correct
 * @param header The RDM command header.
 * @param param_data the parameter data
 */
void RDMHandler_HandleRequest(const RDMHeader *header,
                              const uint8_t *param_data);

/**
 * @brief Get the UID of the responder.
 * @param[out] uid A pointer to copy the UID to; should be at least UID_LENGTH.
 *
 * If no model is active, the UID will be 0000:00000000.
 */
void RDMHandler_GetUID(uint8_t *uid);

/**
 * @brief Perform the periodic RDM Handler tasks.
 *
 * This should be called in the main event loop, it delegates to calling the
 * tasks_fn on the active model.
 */
void RDMHandler_Tasks();

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_RDM_HANDLER_H_
