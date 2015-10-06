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
 * config_options.h
 * Copyright (C) 2015 Simon Newton
 */

#ifndef COMMON_CONFIG_OPTIONS_H_
#define COMMON_CONFIG_OPTIONS_H_

/**
 * @file config_options.h
 * @brief Settings used for board configuration.
 *
 */

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

/**
 * @brief Controls how the device's UID is derived.
 */
typedef enum {
  UID_FROM_MAC = 0,  //!< Use the MAC address
  UID_FROM_FLASH = 1,  //!< Read from flash
  UID_TEST_UID = 2  //!< Use the test UID
} UIDSource;

#endif  // COMMON_CONFIG_OPTIONS_H_
