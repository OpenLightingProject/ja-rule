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
 * random.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @defgroup random Random
 * @brief Pseduo random number generator.
 *
 * @note This should not be used for anything related to security.
 *
 * @addtogroup random
 * @{
 * @file random.h
 * @brief Pseduo random number generator.
 */

#ifndef FIRMWARE_SRC_RANDOM_H_
#define FIRMWARE_SRC_RANDOM_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Set the seed for the random number generator.
 * @param seed the seed.
 */
void Random_SetSeed(unsigned int seed);

/**
 * @brief Return a new pseudo-random number.
 * @returns a pseduo random number.
 */
int Random_PseudoGet();

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_RANDOM_H_
