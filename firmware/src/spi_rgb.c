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
 * spi_rgb.c
 * Copyright (C) 2015 Simon Newton
 */

#include "spi_rgb.h"

#include <string.h>

#include "peripheral/spi/plib_spi.h"
#include "syslog.h"

// TODO(simon): move these into the config (and set with RDM?)
static const uint8_t LPD8806_PIXEL_BYTE = 0x80u;

enum { LATCH_BYTES = 1u };
enum { PIXEL_COUNT = 2u };
enum { SLOTS_PER_PIXEL = 3u };

typedef struct {
  SPI_MODULE_ID module_id;
  bool use_enhanced_buffering;
  bool in_update;
  uint32_t tx_index;
  uint8_t pixels[SLOTS_PER_PIXEL * PIXEL_COUNT + LATCH_BYTES];
} SPIState;

static SPIState g_spi;

void SPIRGB_Init(const SPIRGBConfiguration *config) {
  g_spi.module_id = config->module_id;
  g_spi.use_enhanced_buffering = config->use_enhanced_buffering;
  g_spi.in_update = false;
  g_spi.tx_index = 0u;

  memset(g_spi.pixels, LPD8806_PIXEL_BYTE, SLOTS_PER_PIXEL * PIXEL_COUNT);
  memset(&g_spi.pixels[SLOTS_PER_PIXEL * PIXEL_COUNT], 0, LATCH_BYTES);

  // Init the SPI hardware.
  PLIB_SPI_BaudRateSet(g_spi.module_id, SYS_CLK_FREQ, config->baud_rate);
  PLIB_SPI_CommunicationWidthSelect(g_spi.module_id,
                                    SPI_COMMUNICATION_WIDTH_8BITS);
  PLIB_SPI_ClockPolaritySelect(g_spi.module_id, SPI_CLOCK_POLARITY_IDLE_HIGH);
  if (g_spi.use_enhanced_buffering) {
    PLIB_SPI_FIFOEnable(g_spi.module_id);
  }
  PLIB_SPI_SlaveSelectDisable(g_spi.module_id);
  PLIB_SPI_PinDisable(g_spi.module_id, SPI_PIN_SLAVE_SELECT);
  PLIB_SPI_MasterEnable(g_spi.module_id);
  PLIB_SPI_Enable(g_spi.module_id);
}

void SPIRGB_BeginUpdate() {
  g_spi.in_update = true;
}

void SPIRGB_SetPixel(uint16_t index, RGB_Color color, uint8_t value) {
  if (index >= PIXEL_COUNT || !g_spi.in_update) {
    return;
  }
  // Map RGB to GRB
  uint8_t color_offset = 0u;
  switch (color) {
    case RED:
      color_offset = 1u;
      break;
    case GREEN:
      color_offset = 0u;
      break;
    case BLUE:
      color_offset = 2u;
      break;
    }
  g_spi.pixels[index * SLOTS_PER_PIXEL + color_offset] =
      LPD8806_PIXEL_BYTE | value >> 1;
}

void SPIRGB_CompleteUpdate() {
  g_spi.in_update = false;
  g_spi.tx_index = 0u;
}

void SPIRGB_Tasks() {
  if (g_spi.in_update) {
    return;
  }

  while (g_spi.tx_index < PIXEL_COUNT * SLOTS_PER_PIXEL + LATCH_BYTES) {
    if (g_spi.use_enhanced_buffering) {
      if (PLIB_SPI_TransmitBufferIsFull(g_spi.module_id)) {
        return;
      }
    } else if (PLIB_SPI_IsBusy(g_spi.module_id)) {
      return;
    }
    PLIB_SPI_BufferWrite(g_spi.module_id, g_spi.pixels[g_spi.tx_index]);
    g_spi.tx_index++;
  }
}
