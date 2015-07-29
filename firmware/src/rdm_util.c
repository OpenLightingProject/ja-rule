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
 * rdm_util.c
 * Copyright (C) 2015 Simon Newton
 */
#include "rdm_util.h"

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include "system_pipeline.h"
#include "constants.h"
#include "utils.h"

static uint16_t Checksum(const uint8_t *data, unsigned int length) {
  uint16_t checksum = 0u;
  unsigned int i;
  for (i = 0u; i < length; i++) {
    checksum += data[i];
  }
  return checksum;
}

bool RDMUtil_RequiresAction(const uint8_t our_uid[UID_LENGTH],
                            const uint8_t uid[UID_LENGTH]) {
  if (RDMUtil_UIDCompare(our_uid, uid) == 0) {
    return true;
  }

  if (uid[2] != 0xff || uid[3] != 0xff || uid[4] != 0xff || uid[5] != 0xff) {
    // definitely not a broadcast
    return false;
  }

  return (uid[0] == our_uid[0] && uid[1] == our_uid[1]) ||
         (uid[0] == 0xff && uid[1] == 0xff);
}

bool RDMUtil_RequiresResponse(const uint8_t uid[UID_LENGTH]) {
  return !(uid[2] == 0xff && uid[3] == 0xff && uid[4] == 0xff &&
           uid[5] == 0xff);
}

bool RDMUtil_VerifyChecksum(const uint8_t *frame, unsigned int size) {
  if (size < sizeof(RDMHeader) + (unsigned int) RDM_CHECKSUM_LENGTH ||
      frame[2] + (unsigned int) RDM_CHECKSUM_LENGTH != size) {
    return false;
  }

  uint8_t message_length = frame[MESSAGE_LENGTH_OFFSET];
  uint16_t checksum = Checksum(frame, message_length);
  return (ShortMSB(checksum) == frame[message_length] &&
          ShortLSB(checksum) == frame[message_length + 1]);
}

int RDMUtil_AppendChecksum(uint8_t *frame) {
  uint8_t message_length = frame[MESSAGE_LENGTH_OFFSET];
  uint16_t checksum = Checksum(frame, message_length);
  frame[message_length] = ShortMSB(checksum);
  frame[message_length + 1] = ShortLSB(checksum);
  return message_length + RDM_CHECKSUM_LENGTH;
}

unsigned int RDMUtil_StringCopy(char *dst, unsigned int dest_size,
                                const char *src, unsigned int src_size) {
  unsigned int size = 0u;
  while (size < src_size && size < dest_size && (*dst++ = *src++)) {
    size++;
  }
  if (size < dest_size - 1) {
    *dst = 0;
  }
  return size;
}

unsigned int RDMUtil_SafeStringLength(const char *str, unsigned int max_size) {
  unsigned int size = 0u;
  while (*str++ && size < max_size) {
    size++;
  }
  return size;
}

void RDMUtil_UpdateSensor(SensorData *sensor, uint8_t recorded_value_support,
                          int16_t new_value) {
  sensor->present_value = new_value;
  if (recorded_value_support & SENSOR_SUPPORTS_LOWEST_HIGHEST_MASK) {
    if (new_value < sensor->lowest_value) {
      sensor->lowest_value = new_value;
    }
    if (new_value > sensor->highest_value) {
      sensor->highest_value = new_value;
    }
  }
}
