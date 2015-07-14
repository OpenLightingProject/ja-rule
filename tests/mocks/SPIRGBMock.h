/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * SPIRGBMock.h
 * A mock coarse timer module.
 * Copyright (C) 2015 Simon Newton
 */

#ifndef TESTS_MOCKS_SPIRGBMOCK_H_
#define TESTS_MOCKS_SPIRGBMOCK_H_

#include <gmock/gmock.h>
#include "spi_rgb.h"

class MockSPIRGB {
 public:
  MOCK_METHOD1(Init, void(const SPIRGBConfiguration *config));
  MOCK_METHOD0(BeginUpdate, void());
  MOCK_METHOD3(SetPixel, void(uint16_t index, RGB_Color color, uint8_t value));
  MOCK_METHOD0(CompleteUpdate, void());
  MOCK_METHOD0(Tasks, void());
};

void SPIRGB_SetMock(MockSPIRGB* mock);

#endif  // TESTS_MOCKS_SPIRGBMOCK_H_
