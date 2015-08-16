/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * CRC polynomial 0xedb88320 – Contributed unknowingly by Gary S. Brown.
 * "Copyright (C) 1986 Gary S. Brown. You may use this program, or code or
 * tables extracted from it, as desired without restriction."
 *
 * The updcrc macro (referred to here as CalculateCRC) is derived from an
 * article Copyright 1986 by Stephen Satchell.
 *
 * Remainder portions of the code are Copyright (C) 2015 Simon Newton.
 */

#ifndef TOOLS_UTILS_H_
#define TOOLS_UTILS_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Convert a string to a uint16_t.
 * @returns true if the input was within range, false otherwise.
 *
 * The string can either be decimal or hex (prefixed with 0x).
 */
bool StringToUInt16(const char *input, uint16_t *output);

/**
 * @brief Convert a string to a uint32_t.
 * @returns true if the input was within range, false otherwise.
 *
 * The string can either be decimal or hex (prefixed with 0x).
 */
bool StringToUInt32(const char *input, uint32_t *output);

#endif  // TOOLS_UTILS_H_
