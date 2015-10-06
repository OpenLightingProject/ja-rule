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
 * bootloader_settings.h
 * Copyright (C) 2015 Simon Newton
 */

#ifndef TESTS_SYSTEM_CONFIG_BOOTLOADER_SETTINGS_H_
#define TESTS_SYSTEM_CONFIG_BOOTLOADER_SETTINGS_H_

#include <stdbool.h>
#include "bootloader.h"
#include "common/config_options.h"

/*
 * @brief The reset address of the application.
 */
enum { APPLICATION_RESET_ADDRESS = 0x9d008000 };

/*
 * @brief The size of a flash page
 */
enum { FLASH_PAGE_SIZE = 0x1000 };

/**
 * @brief The size of the words used for flash programming.
 */
enum { FLASH_WORD_SIZE = 4 };

/**
 * @brief Enables updating the flash based UID using DFU.
 */
#define CFG_ALLOW_DFU_UID_UPDATES

/**
 * @brief The port channel of the switch that controls bootloader mode.
 */
const PORTS_CHANNEL SWITCH_PORT_CHANNEL = PORT_CHANNEL_D;

/**
 * @brief The port pin of the switch that controls bootloader mode.
 */
const PORTS_BIT_POS SWITCH_PORT_BIT = PORTS_BIT_POS_7;

/**
 * @brief True if the switch is active high, false if active low.
 */
const bool SWITCH_ACTIVE_HIGH = false;

/**
 * @brief The hardware model.
 */
#define HARDWARE_MODEL MODEL_ETHERNET_SK2

/**
 * @brief The LEDS to flash in bootloader mode.
 */
const Bootloader_LEDs BOOTLOADER_LEDS = {
  .count = 1,
  .leds = {
    {
      .port_channel = PORT_CHANNEL_D,
      .port_bit = PORTS_BIT_POS_0
    },
  }
};

#endif  // TESTS_SYSTEM_CONFIG_BOOTLOADER_SETTINGS_H_
