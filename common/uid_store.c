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

#include "uid_store.h"

#include <stdint.h>

/*
 * @brief The location of the UID.
 *
 * This is setup in the linker script.
 */
extern uint8_t __attribute__((space(prog))) _uid;

static char LowerToHex(uint8_t byte) {
  if (byte < 10u) {
    return '0' + byte;
  } else if (byte < 16u) {
    return 'a' + (byte - 10u);
  }
  return '?';
}

const uint8_t* UIDStore_GetUID() {
  return &_uid;
}

void UIDStore_AsUnicodeString(uint16_t *output) {
  const uint8_t *uid = &_uid;
  unsigned int i = 0u;
  unsigned int offset = 0u;
  for (; i < UID_LENGTH; i++) {
    output[offset++] = LowerToHex((uid[i] >> 4) & 0x0f);
    output[offset++] = LowerToHex(uid[i] & 0x0f);
    if (offset == 4) {
      output[offset++] = ':';
    }
  }
}
