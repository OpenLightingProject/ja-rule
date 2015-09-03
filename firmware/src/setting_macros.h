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
 * setting_macros.h
 * Copyright (C) 2015 Simon Newton
 */

#ifndef FIRMWARE_SRC_SETTING_MACROS_H_
#define FIRMWARE_SRC_SETTING_MACROS_H_

/**
 * @{
 * @file setting_macros.h
 * @brief Expand configuration settings to symbols.
 *
 * These macros can be used to convert integers to module IDs, interrupt sources
 * & interrupt vectors. This allows us to specify the peripheral index in a
 * single location, see system_settings.h
 */

/// @cond INTERNAL
#define _CAT2(x, y) x ## y
#define _CAT3(x, y, z) x ## y ## z
/// @endcond

/**
 * @def AS_TIMER_ID
 * @brief Expands to a TMR_MODULE_ID.
 * @param id The timer module id.
 * @returns The corresponding TMR_MODULE_ID.
 */
#define AS_TIMER_ID(id) _CAT2(TMR_ID_, id)

/**
 * @def AS_TIMER_INTERRUPT_SOURCE
 * @brief Expands to a INT_SOURCE.
 * @param id The timer module id.
 * @returns The corresponding INT_SOURCE.
 */
#define AS_TIMER_INTERRUPT_SOURCE(id) _CAT2(INT_SOURCE_TIMER_, id)

/**
 * @def AS_TIMER_INTERRUPT_VECTOR
 * @brief Expands to an INT_VECTOR.
 * @param id The timer module id.
 * @returns The corresponding vector
 */
#define AS_TIMER_INTERRUPT_VECTOR(id) _CAT2(INT_VECTOR_T, id)

/**
 * @def AS_TIMER_ISR_VECTOR
 * @brief Expands to an ISR vector number.
 * @param id The timer module id.
 * @returns The corresponding ISR vector
 */
#define AS_TIMER_ISR_VECTOR(id) _CAT3(_TIMER_, id, _VECTOR)

/**
 * @def AS_USART_ID
 * @brief Expands to a USART_MODULE_ID.
 * @param id The USART module id.
 * @returns The corresponding USART_MODULE_ID.
 */
#define AS_USART_ID(id) _CAT2(USART_ID_, id)

/**
 * @def AS_USART_ISR_VECTOR
 * @brief Expands to an ISR vector number.
 * @param id The USART module id.
 * @returns The corresponding ISR vector
 */
#define AS_USART_ISR_VECTOR(id) _CAT3(_UART_, id, _VECTOR)

/**
 * @def AS_USART_INTERRUPT_VECTOR
 * @brief Expands to an INT_VECTOR.
 * @param id The USART module id.
 * @returns The corresponding vector
 */
#define AS_USART_INTERRUPT_VECTOR(id) _CAT2(INT_VECTOR_UART, id)

/**
 * @def AS_USART_INTERRUPT_TX_SOURCE
 * @brief Expands to an INT_SOURCE.
 * @param id The USART module id.
 * @returns The corresponding TX source
 */
#define AS_USART_INTERRUPT_TX_SOURCE(id) _CAT3(INT_SOURCE_USART_, id, _TRANSMIT)

/**
 * @def AS_USART_INTERRUPT_RX_SOURCE
 * @brief Expands to an INT_SOURCE.
 * @param id The USART module id.
 * @returns The corresponding RX source
 */
#define AS_USART_INTERRUPT_RX_SOURCE(id) _CAT3(INT_SOURCE_USART_, id, _RECEIVE)

/**
 * @def AS_USART_INTERRUPT_ERROR_SOURCE
 * @brief Expands to an INT_SOURCE.
 * @param id The USART module id.
 * @returns The corresponding error source
 */
#define AS_USART_INTERRUPT_ERROR_SOURCE(id) _CAT3(INT_SOURCE_USART_, id, _ERROR)

/**
 * @def AS_IC_ID
 * @brief Expands to a IC_MODULE_ID.
 * @param id The Input Capture module id.
 * @returns The corresponding IC_MODULE_ID.
 */
#define AS_IC_ID(id) _CAT2(IC_ID_, id)

/**
 * @def AS_IC_ISR_VECTOR
 * @brief Expands to an ISR vector number.
 * @param id The IC module id.
 * @returns The corresponding ISR vector
 */
#define AS_IC_ISR_VECTOR(id) _CAT3(_INPUT_CAPTURE_, id, _VECTOR)

/**
 * @def AS_IC_INTERRUPT_SOURCE
 * @brief Expands to a INT_SOURCE.
 * @param id The IC module id.
 * @returns The corresponding INT_SOURCE.
 */
#define AS_IC_INTERRUPT_SOURCE(id) _CAT2(INT_SOURCE_INPUT_CAPTURE_, id)

/**
 * @def AS_IC_INTERRUPT_VECTOR
 * @brief Expands to an INT_VECTOR.
 * @param id The IC module id.
 * @returns The corresponding vector
 */
#define AS_IC_INTERRUPT_VECTOR(id) _CAT2(INT_VECTOR_IC, id)

/**
 * @def AS_IC_TMR_ID
 * @brief Expands to an IC_TIMERS.
 * @param id The timer module id.
 * @returns The corresponding IC timer id
 */
#define AS_IC_TMR_ID(id) _CAT2(IC_TIMER_TMR, id)

/**
 * @}
 */

#endif  // FIRMWARE_SRC_SETTING_MACROS_H_
