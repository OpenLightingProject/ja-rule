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
 * moving_light.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @addtogroup rdm_models
 * @{
 * @file moving_light.h
 * @brief An RDM Model for a moving light.
 */

#ifndef FIRMWARE_SRC_MOVING_LIGHT_H_
#define FIRMWARE_SRC_MOVING_LIGHT_H_

#include "system_config.h"

#include "rdm_model.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The ModelEntry for a Moving Light.
 */
extern const ModelEntry MOVING_LIGHT_MODEL_ENTRY;

/**
 * @brief Initialize the Moving Light Model.
 */
void MovingLightModel_Initialize();

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_MOVING_LIGHT_H_
