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
 * rdm_util.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @defgroup rdm_util RDM Helper functions
 * @brief RDM Helper Functions
 *
 * @addtogroup rdm_util
 * @{
 * @file rdm_util.h
 * @brief RDM Helper functions.
 */

#ifndef FIRMWARE_SRC_RDM_UTIL_H_
#define FIRMWARE_SRC_RDM_UTIL_H_

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "rdm.h"
#include "rdm_responder.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Compare two UIDs.
 * @param uid1 The first uid.
 * @param uid2 The second uid.
 * @returns < 0 if uid1 < uid2, 0 if the UIDs are the same, > 0 if uid1 > uid2.
 */
static inline int RDMUtil_UIDCompare(const uint8_t *uid1, const uint8_t *uid2) {
  return memcmp(uid1, uid2, UID_LENGTH);
}

/**
 * @brief Check if a RDM request sent to a UID requires us to take action.
 * @param our_uid The UID of the responder to check
 * @param uid The UID to check.
 * @returns true if we should process the RDM request, false otherwise.
 *
 * Action is required if any of the following is true:
 *  - The UID exactly matches our UID.
 *  - The UID is the broadcast UID (ffff:ffffffff)
 *  - The UID is a vendorcast UID, and the manufacturer ID matches ours.
 */
bool RDMUtil_RequiresAction(const uint8_t our_uid[UID_LENGTH],
                            const uint8_t uid[UID_LENGTH]);

/*
 * @brief Check if we should respond to an RDM request.
 * @param responder The responder information
 * @param uid The destination UID to check.
 * @returns true if the UIDs match, false otherwise.
 * @pre This assumes that RDMUtil_RequiresAction already returned true.
 *
 * A response is only required if the last four bytes of the UID are not
 * 0xffffffff.
 */
bool RDMUtil_RequiresResponse(const uint8_t uid[UID_LENGTH]);

/**
 * @brief Verify the checksum of an RDM frame.
 * @param frame The frame data, begining with the start code.
 * @param size The size of the frame data.
 * @returns True if the checksum was correct, false otherwise.
 *
 * Frame sizes less than the minimum RDM frame size (26 bytes) will always
 * return false.
 */
bool RDMUtil_VerifyChecksum(const uint8_t *frame, unsigned int size);

/**
 * @brief Append the RDM checksum for a frame.
 * @param frame The RDM frame.
 * @returns The size of the complete RDM frame.
 * @pre The frame points to a well formed RDM message.
 */
int RDMUtil_AppendChecksum(uint8_t *frame);

/**
 * @brief Copy a string from one location to another.
 * @param dst The location to copy to.
 * @param dest_size The maximum number of bytes to copy to dst.
 * @param src The location to copy from.
 * @param max_size The maximum number of characters to copy.
 * @returns The size of the final string, excluding any NULL if there is one.
 *
 * RDM strings are not required to have a NULL terminator, which means we can't
 * use the usual string functions like strncpy(). This function should
 * be used when copying strings.
 */
unsigned int RDMUtil_StringCopy(char *dst, unsigned int dest_size,
                                const char *src, unsigned int max_size);

/**
 * @brief Calculate the size of the string, but never scan beyond max_size.
 * @param str The string
 * @param max_size The max size of the string.
 * @returns The size of the string, at most max_size.
 *
 * Since RDM strings may be missing the NULL terminator, this should be used
 * when determining string lengths.
 */
unsigned int RDMUtil_SafeStringLength(const char *str, unsigned int max_size);

/**
 * @brief Update the value of a sensor, setting the lowest / highest values if
 * appropriate.
 * @param sensor The sensor to update
 * @param recorded_value_support The bitfield to check if the sensor supports
 *   recording.
 * @param new_value The new value of the sensor.
 */
void RDMUtil_UpdateSensor(SensorData *sensor, uint8_t recorded_value_support,
                          int16_t new_value);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_RDM_UTIL_H_
