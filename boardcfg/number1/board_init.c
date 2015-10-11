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
 * number1/board_init.c
 * Copyright (C) 2015 Simon Newton
 */

#include "board_init.h"

#include "peripheral/ports/plib_ports.h"

void Number1_PreAppHook() {
  // We need to set the necessary ports to digital mode
  PLIB_PORTS_PinModeSelect(PORTS_ID_0, PORTS_ANALOG_PIN_8,
                           PORTS_PIN_MODE_DIGITAL);  // U5 RX
  PLIB_PORTS_PinModeSelect(PORTS_ID_0, PORTS_ANALOG_PIN_9,
                           PORTS_PIN_MODE_DIGITAL);  // TX En
  PLIB_PORTS_PinModeSelect(PORTS_ID_0, PORTS_ANALOG_PIN_10,
                           PORTS_PIN_MODE_DIGITAL);  // RX En
  PLIB_PORTS_PinModeSelect(PORTS_ID_0, PORTS_ANALOG_PIN_14,
                           PORTS_PIN_MODE_DIGITAL);  // U5 TX
}
