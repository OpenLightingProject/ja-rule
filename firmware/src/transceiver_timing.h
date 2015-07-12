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
 * transceiver_timing.h
 * Copyright (C) 2015 Simon Newton
 */

#ifndef FIRMWARE_SRC_TRANSCEIVER_TIMING_H_
#define FIRMWARE_SRC_TRANSCEIVER_TIMING_H_

/**
 * @addtogroup transceiver
 * @{
 * @file transceiver_timing.h
 */

// Common params
// ----------------------------------------------------------------------------

/**
 * @brief The minimum break time the user can configure, in microseconds.
 *
 * DMX1990 was 88uS and later versions raised this to 92uS. We allow down to
 * 44uS because Sean says it's useful for testing.
 */
#define MINIMUM_TX_BREAK_TIME 44

/**
 * @brief The maximum break time the user can configure, in microseconds.
 *
 * This needs to be kept within what a 16-bit timer can support. We'll probably
 * need to change this if the clock speed increases.
 */
#define MAXIMUM_TX_BREAK_TIME 800

/**
 * @brief The minimum mark time the user can configure, in microseconds.
 *
 * DMX1986 apparently allows a 4uS mark time.
 */
#define MINIMUM_TX_MARK_TIME 4

/**
 * @brief The maximum mark time the user can configure, in microseconds.
 *
 * This needs to be kept within what a 16-bit timer can support. We'll probably
 * need to change this if the clock speed increases.
 */
#define MAXIMUM_TX_MARK_TIME 800

// Controller params
// ----------------------------------------------------------------------------

/**
 * @brief The minimum break time for controllers to receive.
 *
 * Measured in 10ths of a microsecond. The value is from line 2 of Table 3-1
 * in E1.20.
 *
 * Note that some responders, like say the Robin 600 Spot, have a very short low
 * before the actual break. This is probably the transceiver turning around and
 * lasts on the order of 200nS.
 */
#define CONTROLLER_RX_BREAK_TIME_MIN 880

/**
 * @brief The maximum break time for controllers to receive.
 *
 * Measured in 10ths of a microsecond. The value is from line 2 of Table 3-1
 * in E1.20.
 */
#define CONTROLLER_RX_BREAK_TIME_MAX 3520

/**
 * @brief The minimum break-to-break time at a controller.
 *
 * Measured in 10ths of a millisecond. The value is from Table 6
 * in E1.11 (2008). In this case we round 1.204ms to 1.3 ms.
 */
#define CONTROLLER_MIN_BREAK_TO_BREAK 13

/**
 * @brief The back off time for a DUB command
 *
 * Measured in 10ths of a millisecond. The value is from line 2 of Table 3-2
 * in E1.20.
 */
#define CONTROLLER_DUB_BACKOFF 58

/**
 * @brief The back off time for a broadcast command.
 *
 * Measured in 10ths of a millisecond. The value is from line 6 of Table 3-2
 * in E1.20. In this case we round 176uS to 0.2 ms.
 */
#define CONTROLLER_BROADCAST_BACKOFF 2

/**
 * @brief The back off time for a missing response.
 *
 * Measured in 10ths of a millisecond. The value is from line 5 of Table 3-2
 * in E1.20.
 */
#define CONTROLLER_MISSING_RESPONSE_BACKOFF 30

/**
 * @brief The back off time for a non-RDM command
 *
 * Measured in 10ths of a millisecond. The value is from line 7 of Table 3-2
 * in E1.20. In this case we round 176uS to 0.2 ms.
 */
#define CONTROLLER_NON_RDM_BACKOFF 2

// Responder params
// ----------------------------------------------------------------------------

/**
 * @brief The minimum break time for responders to receive.
 *
 * Measured in 10ths of a microsecond. The value is from line 1 of Table 3-3
 * in E1.20.
 */
#define RESPONDER_RX_BREAK_TIME_MIN  880

/**
 * @brief The maximum break time for responders to receive.
 *
 * Measured in 10ths of a millisecond. The value is from line 1 of Table 3-3
 * in E1.20.
 */
#define RESPONDER_RX_BREAK_TIME_MAX  10000

/**
 * @brief The minimum RDM responder delay in 10ths of a microsecond, Table 3-4,
 * E1.20
 */
#define MINIMUM_RESPONDER_DELAY 1760

/**
 * @brief The maximum RDM responder delay in 10ths of a microsecond. Table 3-4,
 * E1.20
 */
#define MAXIMUM_RESPONDER_DELAY 20000

/**
 * @brief The minimum mark time for responders to receive
 *
 * Measured in 10ths of a microsecond. The value is from line 1 of Table 3-3
 * in E1.20.
 */
#define RESPONDER_RX_MARK_TIME_MIN  80

/**
 * @brief The maximum mark time for responders to receive.
 *
 * Measured in 10ths of a millisecond. The value is from line 1 of Table 3-3
 * in E1.20.
 */
#define RESPONDER_RX_MARK_TIME_MAX  10000

/**
 * @brief The inter-slot timeout for RDM frames.
 *
 * Measured in 10ths of a millisecond. The value is from line 1 of Table 3-3
 * in E1.20.
 */
#define RESPONDER_RDM_INTERSLOT_TIMEOUT 21

/**
 * @brief The inter-slot timeout for DMX and other ASC frames.
 *
 * Measured in 10ths of a millisecond. The value is from Table 6 of E1.11-2008
 */
#define RESPONDER_DMX_INTERSLOT_TIMEOUT 10000


/**
 * @}
 */

#endif   // FIRMWARE_SRC_TRANSCEIVER_TIMING_H_
