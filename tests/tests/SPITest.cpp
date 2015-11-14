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
 * SPITest.cpp
 * Tests for the SPI code.
 * Copyright (C) 2015 Simon Newton
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

#include "Array.h"
#include "Matchers.h"
#include "constants.h"
#include "dmx_spec.h"
#include "setting_macros.h"
#include "spi.h"

#include "tests/sim/InterruptController.h"
#include "tests/sim/PeripheralSPI.h"
#include "tests/sim/Simulator.h"

using ::testing::ElementsAreArray;
using ::testing::InSequence;;
using ::testing::InvokeWithoutArgs;
using ::testing::IsEmpty;;
using ::testing::Return;
using ::testing::StrictMock;
using ::testing::_;
using ola::NewCallback;
using std::vector;

#ifdef __cplusplus
extern "C" {
#endif

// Declare the ISR symbols.
void SPI_Event(void);

#ifdef __cplusplus
}
#endif

// This mock is used to capture SPI events.
class MockEventHandler {
 public:
  MOCK_METHOD1(Run, void(SPIEventType event));
};

MockEventHandler *g_event_handler = nullptr;

void EventHandler(SPIEventType event) {
  if (g_event_handler) {
    g_event_handler->Run(event);
  }
}

class SPITest : public testing::Test {
 public:
  SPITest()
      : m_callback(ola::NewCallback(&SPI_Tasks)),
        m_simulator(kClockSpeed),
        m_spi(&m_simulator, &m_interrupt_controller) {
  }

  void SetUp() {
    m_simulator.SetClockLimit(1000000, true);  // default to 1s
    g_event_handler = &m_event_handler;
    PLIB_SPI_SetMock(&m_spi);
    SYS_INT_SetMock(&m_interrupt_controller);

    m_interrupt_controller.RegisterISR(INT_SOURCE_SPI_2_RECEIVE,
        NewCallback(&SPI_Event));
    m_interrupt_controller.RegisterISR(INT_SOURCE_SPI_2_TRANSMIT,
        NewCallback(&SPI_Event));

    m_simulator.AddTask(m_callback.get());

    SPI_Initialize();
  }

  void TearDown() {
    g_event_handler = nullptr;
    PLIB_SPI_SetMock(nullptr);
    SYS_INT_SetMock(nullptr);

    m_simulator.RemoveTask(m_callback.get());
  }

 protected:
  std::unique_ptr<ola::Callback0<void>> m_callback;

  Simulator m_simulator;
  InterruptController m_interrupt_controller;
  PeripheralSPI m_spi;

  StrictMock<MockEventHandler> m_event_handler;

  void AddInputBytes(const uint8_t *data, unsigned int size) {
    for (unsigned int i = 0; i < size; i++) {
      m_spi.QueueResponseByte(SPI_ID_2, data[i]);
    }
  }

  static const uint32_t kClockSpeed = 80000000;
  static const uint32_t kBaudRate = 250000;
};

TEST_F(SPITest, testOutput) {
  uint8_t output[] = {1, 2, 3};
  EXPECT_TRUE(SPI_QueueTransfer(
      output, arraysize(output), nullptr, 0, &EventHandler));

  EXPECT_CALL(m_event_handler, Run(SPI_BEGIN_TRANSFER)).Times(1);
  EXPECT_CALL(m_event_handler, Run(SPI_COMPLETE_TRANSFER))
    .WillOnce(InvokeWithoutArgs(&m_simulator, &Simulator::Stop));

  m_simulator.Run();
  EXPECT_THAT(m_spi.SentBytes(SPI_ID_2), ElementsAreArray(output));
}

TEST_F(SPITest, testInput) {
  const uint8_t data[] = {4, 5, 6};
  AddInputBytes(data, arraysize(data));

  uint8_t input[3];
  EXPECT_TRUE(SPI_QueueTransfer(
      nullptr, 0, input, arraysize(input), &EventHandler));

  EXPECT_CALL(m_event_handler, Run(SPI_BEGIN_TRANSFER)).Times(1);
  EXPECT_CALL(m_event_handler, Run(SPI_COMPLETE_TRANSFER))
    .WillOnce(InvokeWithoutArgs(&m_simulator, &Simulator::Stop));

  m_simulator.Run();

  ArrayTuple received_bytes(input, arraysize(input));
  EXPECT_THAT(received_bytes, DataIs(data, arraysize(data)));

  // Check we only sent 0s
  const uint8_t sent_bytes[] = {0, 0, 0};
  EXPECT_THAT(m_spi.SentBytes(SPI_ID_2), ElementsAreArray(sent_bytes));
}

TEST_F(SPITest, nullTransfer) {
  EXPECT_TRUE(SPI_QueueTransfer(
      nullptr, 0, nullptr, 0, &EventHandler));

  EXPECT_CALL(m_event_handler, Run(SPI_COMPLETE_TRANSFER))
    .WillOnce(InvokeWithoutArgs(&m_simulator, &Simulator::Stop));

  m_simulator.Run();

  EXPECT_THAT(m_spi.SentBytes(SPI_ID_2), IsEmpty());
}

TEST_F(SPITest, bigTransfer) {
  // larger than the enhanced buffer size.
  uint8_t output[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  uint8_t input[10];
  EXPECT_TRUE(SPI_QueueTransfer(
      output, arraysize(output), input, arraysize(input), &EventHandler));

  EXPECT_CALL(m_event_handler, Run(SPI_BEGIN_TRANSFER)).Times(1);
  EXPECT_CALL(m_event_handler, Run(SPI_COMPLETE_TRANSFER))
    .WillOnce(InvokeWithoutArgs(&m_simulator, &Simulator::Stop));

  m_simulator.Run();
  EXPECT_THAT(m_spi.SentBytes(SPI_ID_2), ElementsAreArray(output));
}

TEST_F(SPITest, testDoubleTransfer) {
  uint8_t output1[] = {1, 2, 3};
  uint8_t output2[] = {4, 5, 6};
  uint8_t output3[] = {7, 8, 9};
  EXPECT_TRUE(SPI_QueueTransfer(
      output1, arraysize(output1), nullptr, 0, &EventHandler));
  EXPECT_TRUE(SPI_QueueTransfer(
      output2, arraysize(output2), nullptr, 0, &EventHandler));
  EXPECT_FALSE(SPI_QueueTransfer(
      output3, arraysize(output3), nullptr, 0, &EventHandler));

  InSequence seq;
  EXPECT_CALL(m_event_handler, Run(SPI_BEGIN_TRANSFER)).Times(1);
  EXPECT_CALL(m_event_handler, Run(SPI_COMPLETE_TRANSFER))
    .WillOnce(InvokeWithoutArgs(&m_simulator, &Simulator::Stop));
  EXPECT_CALL(m_event_handler, Run(SPI_BEGIN_TRANSFER)).Times(1);
  EXPECT_CALL(m_event_handler, Run(SPI_COMPLETE_TRANSFER))
    .WillOnce(InvokeWithoutArgs(&m_simulator, &Simulator::Stop));

  m_simulator.Run();

  EXPECT_THAT(m_spi.SentBytes(SPI_ID_2), ElementsAreArray(output1));

  // Now continue.
  m_simulator.Run();

  const uint8_t expected[] = {1, 2, 3, 4, 5, 6};
  EXPECT_THAT(m_spi.SentBytes(SPI_ID_2), ElementsAreArray(expected));
}
