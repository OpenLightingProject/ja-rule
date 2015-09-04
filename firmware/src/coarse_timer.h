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
 * coarse_timer.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @defgroup timer Coarse Timer
 * @brief A coarse global timer that can be used to track time intervals.
 *
 * The timer is accurate to 10ths of a millisecond.
 *
 * @addtogroup timer
 * @{
 * @file coarse_timer.h
 * @brief Provides a coarse timer for determining the elapsed time between two
 *   events.
 */

#ifndef FIRMWARE_SRC_COARSE_TIMER_H_
#define FIRMWARE_SRC_COARSE_TIMER_H_

#include <stdbool.h>
#include <stdint.h>

#include "system_config.h"
#include "peripheral/tmr/plib_tmr.h"
#include "system/int/sys_int.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Settings for the CoarseTimer module.
 */
typedef struct {
  TMR_MODULE_ID timer_id;  //!< The timer module to use.
  INT_SOURCE interrupt_source;  //!< The interrupt source to use.
} CoarseTimer_Settings;

/**
 * @brief An opaque type used to represent a time stamp.
 */
typedef uint32_t CoarseTimer_Value;

/**
 * @brief Initialize the timer.
 * @param settings The timer settings.
 *
 * The settings should match the interrupt vector used to call
 * CoarseTimer_TimerEvent().
 *
 * @examplepara
 * ~~~~~~~~~~~~~~~~~~~~~
 * CoarseTimer_Settings timer_settings = {
 *   .timer_id = TMR_ID_2,
 *   .interrupt_source = INT_SOURCE_TIMER_2
 * };
 *
 * CoarseTimer_Initialize(&timer_settings);
 * ~~~~~~~~~~~~~~~~~~~~~
 */
void CoarseTimer_Initialize(const CoarseTimer_Settings *settings);

/**
 * @brief Update the timer.
 *
 * This should be called from within an ISR.
 *
 * @examplepara
 * ~~~~~~~~~~~~~~~~~~~~~
 * void __ISR(_TIMER_2_VECTOR, ipl6) TimerEvent() {
 *  CoarseTimer_TimerEvent();
 * }
 * ~~~~~~~~~~~~~~~~~~~~~
 *
 * The interrupt vector should match what was supplied to
 * CoarseTimer_Initialize().
 */
void CoarseTimer_TimerEvent();

/**
 * @brief Get the current value of the timer.
 * @returns The current value of the timer counter.
 *
 * The value returned can be later passed to CoarseTimer_HasElapsed() and
 * CoarseTimer_ElapsedTime().
 */
CoarseTimer_Value CoarseTimer_GetTime();

/**
 * @brief Return the interval since the start_time.
 * @param start_time The time to measure from.
 * @returns The time since the start_time, measured in 10ths of a millisecond.
 *
 * Accuracy is to within 10ths of a millisecond. Be careful if using this to
 * trigger events. as the events may then trigger up to 0.1ms before they were
 * supposed to.
 */
uint32_t CoarseTimer_ElapsedTime(CoarseTimer_Value start_time);

/**
 * @brief Return the interval between two times.
 * @param start_time The time to measure from.
 * @param end_time The time to measure to.
 * @returns The time between the start and end time, measured in 10ths of a
 *   millisecond.
 *
 * Accuracy is to within 10ths of a millisecond. Be careful if using this to
 * trigger events. as the events may then trigger up to 0.1ms before they were
 * supposed to.
 */
uint32_t CoarseTimer_Delta(CoarseTimer_Value start_time,
                           CoarseTimer_Value end_time);

/**
 * @brief Check if a time interval has passed.
 * @param start_time The time to measure from.
 * @param interval The time interval in 10ths of a millisecond.
 * @returns true if the interval has elapsed since the start_time.
 */
bool CoarseTimer_HasElapsed(CoarseTimer_Value start_time, uint32_t interval);

/**
 * @brief Manually set the internal counter.
 * @param count the new value of the internal counter.
 * @note This function should be used for testing only.
 */
void CoarseTimer_SetCounter(uint32_t count);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_COARSE_TIMER_H_
