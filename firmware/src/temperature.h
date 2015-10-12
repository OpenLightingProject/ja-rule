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
 * temperature.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @defgroup temperature Temperature Sensor
 * @brief Reads the temperature sensor(s).
 *
 * @addtogroup temperature
 * @{
 * @file temperature.h
 * @brief Reads the temperature sensor(s).
 */

#ifndef FIRMWARE_SRC_TEMPERATURE_H_
#define FIRMWARE_SRC_TEMPERATURE_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The different types of temperature sensors.
 */
typedef enum {
  TEMPERATURE_BOARD_TEMP  //!< The PCB board temp.
} TemperatureSensor;

/**
 * @brief Initialize the temperature module.
 */
void Temperature_Init();

/**
 * @brief Get the last known value for a sensor.
 * @param sensor The sensor to get the value of
 * @returns The last known value of the sensor, in 10ths of a degree.
 *
 * If the value is known this will return 0.
 */
uint16_t Temperature_GetValue(TemperatureSensor sensor);

/**
 * @brief Perform the periodic tasks.
 */
void Temperature_Tasks();

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_TEMPERATURE_H_
