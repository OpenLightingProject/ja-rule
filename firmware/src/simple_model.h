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
 * simple_model.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @defgroup simple_model Simple RDM Model
 * @brief A simple RDM Model for controlling RGB pixels.
 *
 * @addtogroup simple_model
 * @{
 * @file simple_model.h
 * @brief A simple RDM Model for controlling RGB pixels.
 */

#ifndef FIRMWARE_SRC_SIMPLE_MODEL_H_
#define FIRMWARE_SRC_SIMPLE_MODEL_H_

#include <stdbool.h>
#include <stdint.h>

#include "system_config.h"

#include "peripheral/ports/plib_ports.h"
#include "rdm.h"
#include "rdm_model.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The ModelEntry for this model.
 */
extern const ModelEntry SIMPLE_MODEL_ENTRY;

/**
 * @brief The settings to use for the Simple RDM Model.
 */
typedef struct {
  PORTS_CHANNEL identify_port;  //!< The port to use for the identify signal.
  PORTS_BIT_POS identify_bit;  //!< The port bit to use for the identify signal.
  PORTS_CHANNEL mute_port;  //!< The port to use to indicate mute state.
  PORTS_BIT_POS mute_bit;  //!< The port bit used to indicate mute state.
  uint8_t uid[UID_LENGTH];  //!< The responder's UID.
} SimpleModelSettings;

/**
 * @brief Initialize the simple model.
 * @param settings The settings for this model.
 */
void SimpleModel_Initialize(const SimpleModelSettings *settings);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_SIMPLE_MODEL_H_
