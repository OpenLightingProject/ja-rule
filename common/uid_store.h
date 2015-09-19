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
 * uid_store.h
 * Copyright (C) 2015 Simon Newton
 */

#ifndef COMMON_UID_STORE_H_
#define COMMON_UID_STORE_H_

#include <stdint.h>
#include "uid.h"

/**
 * @{
 * @file uid_store.h
 * @brief The API to retrieve the device's UID.
 *
 * For now, we store the UID in program flash.
 */

/**
 * @brief Get the device's UID.
 * @returns A pointer to the device's UID
 */
const uint8_t* UIDStore_GetUID();

/**
 * @brief Convert the UID to a string representation, e.g. abcd:01020304
 * @param output The location to store the string in, must be at least
 *   UID_LENGTH * 2 + 1 characters.
 */
void UIDStore_AsAsciiString(char *output);

/**
 * @brief Convert the UID to a string representation, e.g. abcd:01020304
 * @param output The location to store the string in, must be at least
 *   UID_LENGTH * 2 + 1 characters.
 */
void UIDStore_AsUnicodeString(uint16_t *output);

#endif  // COMMON_UID_STORE_H_

/**
 * @}
 */
