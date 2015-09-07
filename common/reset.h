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
 * reset.h
 * Copyright (C) 2015 Simon Newton
 */

#ifndef COMMON_RESET_H_
#define COMMON_RESET_H_

/**
 * @{
 * @file reset.h
 * @brief API to reset the device.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Reset the device.
 *
 * This function never returns.
 */
void Reset_SoftReset();

#ifdef __cplusplus
}
#endif

#endif  // COMMON_RESET_H_

/**
 * @}
 */
