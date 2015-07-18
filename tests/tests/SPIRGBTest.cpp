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
 * SPIRGBTest.cpp
 * Tests for the SPI RGB code.
 * Copyright (C) 2015 Simon Newton
 */

#include <gtest/gtest.h>
#include <vector>

#include "spi_rgb.h"
#include "Array.h"
#include "Matchers.h"
#include "plib_spi_mock.h"

using ::testing::ElementsAreArray;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::StrictMock;
using ::testing::WithArgs;
using ::testing::_;

class SPIRGBTest : public testing::Test {
 public:
  void SetUp() {
    PLIB_SPI_SetMock(&spi_mock);
  }

  void TearDown() {
    PLIB_SPI_SetMock(NULL);
  }

  void AppendByte(uint8_t byte) {
    m_spi_data.push_back(byte);
  }

 protected:
  StrictMock<MockPeripheralSPI> spi_mock;
  std::vector<uint8_t> m_spi_data;
};

TEST_F(SPIRGBTest, testSimpleMode) {
  SPIRGBConfiguration config;
  config.module_id = SPI_ID_1;
  config.baud_rate = 2000000;
  config.use_enhanced_buffering = false;

  EXPECT_CALL(spi_mock, BaudRateSet(SPI_ID_1, _, 2000000))
    .Times(1);
  EXPECT_CALL(spi_mock,
              CommunicationWidthSelect(SPI_ID_1, SPI_COMMUNICATION_WIDTH_8BITS))
    .Times(1);
  EXPECT_CALL(spi_mock,
              ClockPolaritySelect(SPI_ID_1, SPI_CLOCK_POLARITY_IDLE_HIGH))
    .Times(1);
  EXPECT_CALL(spi_mock, SlaveSelectDisable(SPI_ID_1))
    .Times(1);
  EXPECT_CALL(spi_mock, PinDisable(SPI_ID_1, SPI_PIN_SLAVE_SELECT))
    .Times(1);
  EXPECT_CALL(spi_mock, Enable(SPI_ID_1))
    .Times(1);
  EXPECT_CALL(spi_mock, MasterEnable(SPI_ID_1))
    .Times(1);
  EXPECT_CALL(spi_mock, IsBusy(SPI_ID_1))
    .WillRepeatedly(Return(false));
  EXPECT_CALL(spi_mock, BufferWrite(SPI_ID_1, _))
    .WillRepeatedly(WithArgs<1>(Invoke(this, &SPIRGBTest::AppendByte)));

  EXPECT_CALL(spi_mock, IsBusy(SPI_ID_1))
    .WillRepeatedly(Return(false));

  SPIRGB_Init(&config);

  SPIRGB_BeginUpdate();
  SPIRGB_CompleteUpdate();
  SPIRGB_Tasks();

  const uint8_t expected[] = {
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0
  };
  EXPECT_THAT(m_spi_data, ElementsAreArray(expected));
  m_spi_data.clear();

  // Now set some pixels
  SPIRGB_SetPixel(0, RED, 255);

  // Now actually start the update
  SPIRGB_BeginUpdate();
  SPIRGB_SetPixel(0, BLUE, 255);
  SPIRGB_SetPixel(1, GREEN, 128);

  SPIRGB_CompleteUpdate();
  SPIRGB_Tasks();

  const uint8_t expected2[] = {
    0x80, 0x80, 0xff, 0xc0, 0x80, 0x80, 0
  };
  EXPECT_THAT(m_spi_data, ElementsAreArray(expected2));
}

TEST_F(SPIRGBTest, testEnhancedMode) {
  SPIRGBConfiguration config;
  config.module_id = SPI_ID_1;
  config.baud_rate = 4000000;
  config.use_enhanced_buffering = true;

  EXPECT_CALL(spi_mock, BaudRateSet(SPI_ID_1, _, 4000000))
    .Times(1);
  EXPECT_CALL(spi_mock,
              CommunicationWidthSelect(SPI_ID_1, SPI_COMMUNICATION_WIDTH_8BITS))
    .Times(1);
  EXPECT_CALL(spi_mock,
              ClockPolaritySelect(SPI_ID_1, SPI_CLOCK_POLARITY_IDLE_HIGH))
    .Times(1);
  EXPECT_CALL(spi_mock, FIFOEnable(SPI_ID_1))
    .Times(1);
  EXPECT_CALL(spi_mock, SlaveSelectDisable(SPI_ID_1))
    .Times(1);
  EXPECT_CALL(spi_mock, PinDisable(SPI_ID_1, SPI_PIN_SLAVE_SELECT))
    .Times(1);
  EXPECT_CALL(spi_mock, Enable(SPI_ID_1))
    .Times(1);
  EXPECT_CALL(spi_mock, MasterEnable(SPI_ID_1))
    .Times(1);
  EXPECT_CALL(spi_mock, IsBusy(SPI_ID_1))
    .WillRepeatedly(Return(false));
  EXPECT_CALL(spi_mock, BufferWrite(SPI_ID_1, _))
    .WillRepeatedly(WithArgs<1>(Invoke(this, &SPIRGBTest::AppendByte)));

  EXPECT_CALL(spi_mock, TransmitBufferIsFull(SPI_ID_1))
    .WillRepeatedly(Return(false));

  SPIRGB_Init(&config);

  SPIRGB_BeginUpdate();
  SPIRGB_CompleteUpdate();
  SPIRGB_Tasks();

  const uint8_t expected[] = {
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0
  };
  EXPECT_THAT(m_spi_data, ElementsAreArray(expected));
  m_spi_data.clear();

  // Now set some pixels
  SPIRGB_SetPixel(0, RED, 255);

  // Now actually start the update
  SPIRGB_BeginUpdate();
  SPIRGB_SetPixel(0, BLUE, 255);
  SPIRGB_SetPixel(1, GREEN, 128);

  SPIRGB_CompleteUpdate();
  SPIRGB_Tasks();

  const uint8_t expected2[] = {
    0x80, 0x80, 0xff, 0xc0, 0x80, 0x80, 0
  };
  EXPECT_THAT(m_spi_data, ElementsAreArray(expected2));
}
