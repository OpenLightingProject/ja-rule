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
 * In combination with the RDMHandler, the Model API provides a flexible method
 * of supporting multiple RDM models on a single physical device. Only one model
 * may be active at once.
 *
 * Each model provides a ModelEntry struct, which contains function
 * pointers for the model's implementation. To add a new model, implement the
 * functions within ModelEntry and then add the ModelEntry to the
 * RDMHandler by calling RDMHandler_AddModel().
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
  NULL_MODEL_ID = 0x0000,  //!< No model set

  /**
   * @brief A simple RDM responder.
   */
  BASIC_RESPONDER_MODEL_ID = 0x0100,

  /**
   * @brief A responder that acts as a proxy.
   */
  PROXY_MODEL_ID = 0x0101,

  /**
   * @brief A responder that acts a moving light
   */
  MOVING_LIGHT_MODEL_ID = 0x0102,

  /**
   * @brief A responder that contains only sensors.
   */
  SENSOR_MODEL_ID = 0x0103,

  /**
   * @brief A responder that presents network interfaces.
   */
  NETWORK_MODEL_ID = 0x0104,

  /**
   * @brief A responder that acts as a dimmer with sub-devices.
   */
  DIMMER_MODEL_ID = 0x0105
} ResponderModel;

/**
 * @brief Model ioctl enums.
 *
 * Ioctls are the generic 'catch-all' operations. We use them so we can reduce
 * the number of functions we need to implement when adding a new model.
 */
typedef enum {
  /**
   * @brief Copies the model's UID into the data pointer.
   * @param data, a memory location to copy the UID to.
   * @param length should be set to UID_LENGTH.
   * @returns Returns 1 on success or 0 if length didn't match UID_LENGTH.
   */
  IOCTL_GET_UID,
} ModelIoctl;

/**
 * @brief The function table entry for a particular responder model.
 *
 * This can be registered with the RDMHandler by calling RDMHandler_AddModel().
 */
typedef struct {
  uint16_t model_id;  //!< The model ID.

  /**
   * @brief Activate function.
   *
   * Called when the model is activated via RDMHandler_SetActiveModel().
   */
  void (*activate_fn)();  //!< Activate function.

  /**
   * @brief The function used to deactivate an RDM Model.
   *
   * The deactivate function is called if this modle is active, and another
   * model becomes active via a call to RDMHandler_SetActiveModel().
   */
  void (*deactivate_fn)();

  /**
   * @brief The generic catch-all function.
   * @param command The ioctl command to run.
   * @param data arbitary data, depends on the ModelIoctl.
   * @param length the size of the data.
   * @returns An int, the meaning of which depends on the ModelIoctl.
   * @sa ModelIoctl for a list of commands.
   */
  int (*ioctl_fn)(ModelIoctl command, uint8_t *data, unsigned int length);

  /**
   * @brief The function used to handle an RDM request.
   * @param header The RDM request header.
   * @param param_data The RDM parameter data, or NULL if there wasn't any.
   * @returns The size of the RDM response, in g_rdm_buffer. The sign is
   * used to indicate if a break should be sent or not. Negative means no break,
   * positive will send a break. 0 means no response will be sent.
   */
  int (*request_fn)(const RDMHeader *header, const uint8_t *param_data);

  /**
   * @brief The tasks function.
   *
   * This is called periodically by RDMHandler_Tasks().
   */
  void (*tasks_fn)();
} ModelEntry;

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_RDM_MODEL_H_
