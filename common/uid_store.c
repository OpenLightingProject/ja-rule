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
 * uid_store.c
 * Copyright (C) 2015 Simon Newton
 */

#include "common_settings.h"
#include "config_options.h"
#include "uid_store.h"

#include <stdint.h>

#ifndef CFG_UID_SOURCE
#error "CFG_UID_SOURCE is not set"
#endif

#if CFG_UID_SOURCE == UID_FROM_MAC
#include "peripheral/eth/plib_eth.h"

static uint8_t kUIDArray[UID_LENGTH];

inline uint8_t ShiftRight(uint8_t b) {
  return (b >> 4) & 0x0f;
}

inline uint8_t ShiftLeft(uint8_t b) {
  return (b << 4) & 0xf0;
}

void UIDStore_Init() {
  // The UID is derived from the RDM manufacturer ID & the MAC address.
  // The first 3 bytes of the MAC address is the Microchip OID 00:1e:c0 which
  // is constant. The bottom 3 bytes contain the unique serial number.
  //
  // To support more than one responder per device, we set the lower 4 bits of
  // the UID to 0 so we have 16 responders per device. These means the complete
  // UID takes the form:
  //   MMMM:0AAAAAA0
  // Where M is the PLASA manufacturer ID and A are the values from the MAC
  // address.
  kUIDArray[0] = CFG_MANUFACTURER_ID >> 8;
  kUIDArray[1] = CFG_MANUFACTURER_ID & 0xff;
  kUIDArray[2] = ShiftRight(PLIB_ETH_StationAddressGet(ETH_ID_0, 4));
  kUIDArray[3] = ShiftLeft(PLIB_ETH_StationAddressGet(ETH_ID_0, 4)) +
                 ShiftRight(PLIB_ETH_StationAddressGet(ETH_ID_0, 5));
  kUIDArray[4] = ShiftLeft(PLIB_ETH_StationAddressGet(ETH_ID_0, 5)) +
                 ShiftRight(PLIB_ETH_StationAddressGet(ETH_ID_0, 6));
  kUIDArray[5] = ShiftLeft(PLIB_ETH_StationAddressGet(ETH_ID_0, 6));
}

const uint8_t* const kUID = &kUIDArray[0];

#elif CFG_UID_SOURCE == UID_FROM_FLASH
// Read from the magic linker symbol.
extern uint8_t __attribute__((space(prog))) _uid;
const uint8_t* const kUID = &_uid;

void UIDStore_Init() {}

#elif CFG_UID_SOURCE == UID_TEST_UID

static const uint8_t kDevUID[UID_LENGTH] = {
  CFG_MANUFACTURER_ID >> 8,
  CFG_MANUFACTURER_ID & 0xff,
  0xff, 0xff, 0xfe, 0x00
};
const uint8_t *kUID = &kDevUID[0];

void UIDStore_Init() {}

#else
#error "Invalid value for CFG_UID_SOURCE"
#endif

static char LowerToHex(uint8_t byte) {
  if (byte < 10u) {
    return '0' + byte;
  } else if (byte < 16u) {
    return 'a' + (byte - 10u);
  }
  return '?';
}

const uint8_t* UIDStore_GetUID() {
  return kUID;
}

void UIDStore_AsAsciiString(char *output) {
  unsigned int i = 0u;
  unsigned int offset = 0u;
  for (; i < UID_LENGTH; i++) {
    output[offset++] = LowerToHex((kUID[i] >> 4) & 0x0f);
    output[offset++] = LowerToHex(kUID[i] & 0x0f);
    if (offset == 4) {
      output[offset++] = ':';
    }
  }
}

void UIDStore_AsUnicodeString(uint16_t *output) {
  unsigned int i = 0u;
  unsigned int offset = 0u;
  for (; i < UID_LENGTH; i++) {
    output[offset++] = LowerToHex((kUID[i] >> 4) & 0x0f);
    output[offset++] = LowerToHex(kUID[i] & 0x0f);
    if (offset == 4) {
      output[offset++] = ':';
    }
  }
}
