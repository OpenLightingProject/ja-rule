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
 * sensor_model.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @addtogroup rdm_models
 * @{
 * @file sensor_model.h
 * @brief A sensor-only RDM Model.
 *
 * This model is provided because some RDM controllers have difficulty when an
 * RDM device reports 0 for a DMX512 footprint. The Sensor Model doesn't use
 * DMX at all, instead it just reports back values from various (simulated)
 * sensors.
 */

#ifndef FIRMWARE_SRC_SENSOR_MODEL_H_
#define FIRMWARE_SRC_SENSOR_MODEL_H_

#include "rdm_model.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The ModelEntry for the Sensor-only model.
 */
extern const ModelEntry SENSOR_MODEL_ENTRY;

/**
 * @brief Initialize the sensor model.
 * @param settings The settings for this model.
 */
void SensorModel_Initialize();

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_SENSOR_MODEL_H_
