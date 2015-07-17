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
 * rdm_buffer.c
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @addtogroup rdm
 * @{
 * @file rdm_buffer.h
 * @brief The memory buffer used to construct the RDM response.
 */

#ifndef FIRMWARE_SRC_RDM_BUFFER_H_
#define FIRMWARE_SRC_RDM_BUFFER_H_

#include "rdm_buffer.h"
#include "rdm.h"

#ifdef __cplusplus
extern "C" {
#endif

static uint8_t RDM_BUFFER[RDM_FRAME_SIZE];

uint8_t *g_rdm_buffer = RDM_BUFFER;

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_RDM_BUFFER_H_
