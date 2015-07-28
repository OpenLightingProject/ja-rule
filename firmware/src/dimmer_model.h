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
 * dimmer_model.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @addtogroup rdm_models
 * @{
 * @file dimmer_model.h
 * @brief A dimmer-only RDM Model.
 *
 * This model simulates a dimmer with different subdevices, one per dimmer
 * module. Each sub-device consumes one slot of DMX data. The model implements
 * many of the PIDs from E1.37-1.
 */

#ifndef FIRMWARE_SRC_DIMMER_MODEL_H_
#define FIRMWARE_SRC_DIMMER_MODEL_H_

#include "rdm_model.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The ModelEntry for the Dimmer-only model.
 */
extern const ModelEntry DIMMER_MODEL_ENTRY;

/**
 * @brief Initialize the dimmer model.
 */
void DimmerModel_Initialize();

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_DIMMER_MODEL_H_
