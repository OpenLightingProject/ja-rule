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
 * dmx_spec.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @{
 * @file dmx_spec.h
 * @brief Constants from the DMX (E1.11) specification.
 */

#ifndef FIRMWARE_SRC_DMX_SPEC_H_
#define FIRMWARE_SRC_DMX_SPEC_H_

/**
 * @brief The maximum size of a DMX frame, excluding the start code.
 */
#define DMX_FRAME_SIZE 512u

/**
 * @brief The Null Start Code (NSC).
 */
#define NULL_START_CODE 0x00u

/**
 * @brief The Baud rate for DMX / RDM.
 */
#define DMX_BAUD 250000u  // 250kHz

#endif  // FIRMWARE_SRC_DMX_SPEC_H_

/**
 * @}
 */
