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
 * rdm_model.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @addtogroup rdm
 * @{
 * @file rdm_model.h
 * @brief The API for an RDM Model implementation.
 *
 * To add support for a new device model, implement the functions within
 * ModelEntry and then add the ModelEntry to the RDMHandler by calling
 * RDMHandler_AddModel().
 */

#ifndef FIRMWARE_SRC_RDM_MODEL_H_
#define FIRMWARE_SRC_RDM_MODEL_H_

#include <stdbool.h>
#include <stdint.h>
#include "rdm_frame.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The Model IDs.
 * @sa https://wiki.openlighting.org/index.php/Open_Lighting_Allocations.
 */
typedef enum {
  NULL_MODEL = 0x0000,  //!< No model set

  /**
   * @brief A simple RDM responder.
   */
  BASIC_RESPONDER = 0x0100,

  /**
   * @brief A responder that acts as a proxy.
   */
  PROXY_RESPONDER = 0x0101
} ResponderModel;

/**
 * @brief The function used to activate an RDM Model.
 *
 * The activate function is called when the model becomes active via a call to
 * RDMHandler_SetActiveModel().
 */
typedef void (*RDMModelActivateFunction)();

/**
 * @brief The function used to deactivate a RDM Model.
 *
 * The deactivate function is called if this modle is active, and another model
 * becomes active via a call to RDMHandler_SetActiveModel().
 */
typedef void (*RDMModelDeactivateFunction) ();

/**
 * @brief The function used to handle an RDM request.
 * @param header The RDM request header.
 * @param param_data The RDM parameter data, or NULL if there wasn't any.
 * @returns The size of the RDM response, in g_responder.buffer. The sign is
 * used to indicate if a break should be sent or not. Negative means no break,
 * positive will send a break. 0 means no response will be sent.
 */
typedef int (*RDMModelRequestFunction) (const RDMHeader *header,
                                        const uint8_t *param_data);

/**
 * @brief The tasks function.
 *
 * This is called periodically by RDMHandler_Tasks().
 */
typedef void (*RDMModelTasksFunction) ();

/**
 * @brief The function table entry for a particular responder model.
 */
typedef struct {
  uint16_t model_id;  //!< The model ID.
  RDMModelActivateFunction  activate_fn;  //!< Activate function.
  RDMModelDeactivateFunction deactivate_fn;  //!< Deactivate function.
  RDMModelRequestFunction request_fn;  //!< RDM Request function.
  RDMModelTasksFunction tasks_fn;  //!< Tasks function.
} ModelEntry;

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_RDM_MODEL_H_
