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
 * ethernet_sk2/bootloader_settings.c
 * Copyright (C) 2015 Simon Newton
 */

#include "bootloader_settings.h"

const Bootloader_LEDs BOOTLOADER_LEDS = {
  .count = 3,
  .leds = {
    {
      .port_channel = PORT_CHANNEL_D,
      .port_bit = PORTS_BIT_POS_0
    },
    {
      .port_channel = PORT_CHANNEL_D,
      .port_bit = PORTS_BIT_POS_1
    },
    {
      .port_channel = PORT_CHANNEL_D,
      .port_bit = PORTS_BIT_POS_2
    },
  }
};
