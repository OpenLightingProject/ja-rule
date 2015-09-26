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
 * models.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @{
 * @file models.h
 * @brief The model numbers for different hardware devices.
 */

#ifndef COMMON_MODELS_H_
#define COMMON_MODELS_H_

/**
 * @brief Hardware model IDs.
 *
 * These are used to check we're using the correct firmware for the device.
 */
typedef enum {
  MODEL_UNDEFINED = 0,  //!< Undefined, disables checks
  MODEL_NUMBER1 = 1,  //!< Number 1.
  MODEL_NUMBER8 = 2,  //!< Number 8.
  MODEL_ETHERNET_SK2 = 3,  //!< Ethernet Starter Kit II
} JaRuleModel;

#endif  // COMMON_MODELS_H_

/**
 * @}
 */
