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

#ifndef BOARDCFG_TEMPLATE_BOOTLOADER_SETTINGS_H_
#define BOARDCFG_TEMPLATE_BOOTLOADER_SETTINGS_H_

#include "bootloader.h"
#include "common_settings.h"

/**
 * @file bootloader_settings.h
 * @brief Configuration settings for the bootloader.
 */

/*
 * @brief The reset address of the application.
 */
#define APPLICATION_RESET_ADDRESS 0x9d008000

/*
 * @brief The size of a flash page
 */
#define FLASH_PAGE_SIZE 0x1000

/**
 * @brief The size of the words used for flash programming.
 */
#define FLASH_WORD_SIZE 4

/**
 * @brief The port channel of the switch that controls bootloader mode.
 */
#define SWITCH_PORT_CHANNEL PORT_CHANNEL_D

/**
 * @brief The port pin of the switch that controls bootloader mode.
 */
#define SWITCH_PORT_BIT PORTS_BIT_POS_7

/**
 * @brief True if the switch is active high, false if active low.
 */
#define SWITCH_ACTIVE_HIGH false

/**
 * @brief The LEDS to flash in bootloader mode.
 */
const Bootloader_LEDs BOOTLOADER_LEDS;

#endif  // BOARDCFG_TEMPLATE_BOOTLOADER_SETTINGS_H_
