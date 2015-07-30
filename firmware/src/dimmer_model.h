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
 * @brief An RDM Model for a dimmer with subdevices.
 *
 * This model simulates a dimmer with different subdevices, one per dimmer
 * module. The model implements all of the PIDs from E1.37-1.
 *
 * ### Sub-devices
 *
 * The model has multiple sub-devices, that each take a single slot of DMX
 * data. The sub-devices are not contiguous.
 *
 * DMX_BLOCK_ADDRESS can be used to set the start address of all sub-devices in
 * a single operation.
 *
 * ### Dimmer Settings
 *
 * Each sub-device implements the PIDs from Section 4 of E1.37-1. To make
 * things interesting, not all sub-devices support all the dimmer curves /
 * modulation frequencies.
 *
 * ### Presets & Scenes.
 *
 * The root device provides 3 scenes. The first scene (index 1) is a factory
 * programed scene, which can't be modified. The 2nd and 3rd scenes can be
 * 'updated' with capture preset.
 *
 * DMX_FAIL_MODE and DMX_STARTUP_MODE can be used to change the on-failure and
 * on-startup scenes.
 *
 * ### Lock PIN
 *
 * The root device implements the LOCK_PIN, LOCK_STATE & LOCK_STATE_DESCRIPTION
 * PIDs. Besides the unlock state (default), there are two custom states. The
 * 1st locks only the sub-devices, the 2nd locks both subdevices and the root
 * device.
 *
 * The default lock pin is 0000.
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
