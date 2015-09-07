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
 * spi_rgb.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @defgroup spi_dmx SPI Pixel Controller
 * @brief Control RGB Pixels using SPI
 *
 * This only supports the LPD8806 chip for now. We're happy to accept pull
 * requests adding support for different pixel types.
 *
 * @addtogroup spi_dmx
 * @{
 * @file spi_rgb.h
 * @brief Control RGB Pixels using SPI
 *
 */

#ifndef FIRMWARE_SRC_SPI_RGB_H_
#define FIRMWARE_SRC_SPI_RGB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "system_config.h"
#include "peripheral/spi/plib_spi.h"

/**
 * @brief RGB color values.
 */
typedef enum {
  RED = 0,
  GREEN = 1,
  BLUE = 2
} RGB_Color;

/**
 * @brief SPI RGB Module configuration
 */
typedef struct {
  SPI_MODULE_ID module_id;  //!< The SPI module to use
  uint32_t baud_rate;  //!< The Baud rate

  /**
   * @brief Use enhanced buffer mode, not all chips support this.
   *
   * Enhanced mode allows us to queue up multiple bytes of SPI data at once. In
   * normal mode there may be delays between bytes.
   */
  bool use_enhanced_buffering;
} SPIRGBConfiguration;

/**
 * @brief Initialize the SPI RGB module.
 * @param config The configuration to use.
 */
void SPIRGB_Init(const SPIRGBConfiguration *config);

/**
 * @brief Begin a frame update.
 *
 * This pauses the SPI sending task.
 */
void SPIRGB_BeginUpdate();

/**
 * @brief Set the value of a pixel
 * @param index The pixel offset
 * @param color The pixel color
 * @param value The new value for the pixel
 */
void SPIRGB_SetPixel(uint16_t index, RGB_Color color, uint8_t value);

/**
 * @brief Complete a frame update.
 *
 * This unpauses the SPI sending task. The frame will be sent on the next call
 * to SPIRGB_Tasks().
 */
void SPIRGB_CompleteUpdate();

/**
 * @brief Perform the periodic SPI RGB tasks.
 *
 * This should be called in the main event loop.
 */
void SPIRGB_Tasks();

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_SPI_RGB_H_
