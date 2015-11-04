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
 * PeripheralUART.cpp
 * The UART used with the simulator.
 * Copyright (C) 2015 Simon Newton
 */

#include "PeripheralUART.h"

#include <gtest/gtest.h>
#include <vector>

#include "macros.h"
#include "Simulator.h"
#include "ola/Callback.h"

using std::vector;

PeripheralUART::UART::UART(INT_SOURCE source)
    : interrupt_source(source),
      enabled(false),
      tx_enable(false),
      rx_enable(false),
      int_mode(USART_TRANSMIT_FIFO_NOT_FULL),
      tx_byte(0),
      errors(USART_ERROR_NONE),
      ticks_per_bit(16),
      tx_counter(0),
      tx_state(IDLE) {
}

PeripheralUART::PeripheralUART(Simulator *simulator,
                               InterruptController *interrupt_controller,
                               TXCallback *tx_callback)
    : m_simulator(simulator),
      m_interrupt_controller(interrupt_controller),
      m_tx_callback(tx_callback),
      m_callback(ola::NewCallback(this, &PeripheralUART::Tick)) {
  m_simulator->AddTask(m_callback.get());

  const vector<INT_SOURCE> sources = {
    INT_SOURCE_USART_1_ERROR,
    INT_SOURCE_USART_2_ERROR,
    INT_SOURCE_USART_3_ERROR,
    INT_SOURCE_USART_4_ERROR,
    INT_SOURCE_USART_5_ERROR
  };

  for (const auto &int_source : sources) {
    m_uarts.push_back(UART(int_source));
  }
}

PeripheralUART::~PeripheralUART() {
  m_simulator->RemoveTask(m_callback.get());
}

void PeripheralUART::Tick() {
  for (unsigned int i = 0; i < m_uarts.size(); i++) {
    UART &uart = m_uarts[i];
    if (!uart.enabled) {
      continue;
    }

    if (uart.tx_enable) {
      if (uart.tx_state == IDLE && !uart.tx_buffer.empty()) {
        uart.tx_state = START_BIT;
        uart.tx_byte = uart.tx_buffer.front();
        uart.tx_buffer.pop();
      }
      if (uart.tx_state != IDLE) {
        uart.tx_counter++;
        if (uart.tx_counter == uart.ticks_per_bit) {
          uart.tx_counter = 0;

          if (uart.tx_state == STOP_BIT_2) {
            if (m_tx_callback) {
              m_tx_callback->Run(static_cast<USART_MODULE_ID>(i), uart.tx_byte);
            }
            uart.tx_state = IDLE;
          } else {
            uart.tx_state = static_cast<UARTState>(uart.tx_state + 1);
          }
        }

        bool trigger_tx_isr = false;
        switch (uart.int_mode) {
          case USART_TRANSMIT_FIFO_NOT_FULL:
            trigger_tx_isr = uart.tx_buffer.size() < TX_FIFO_SIZE;
            break;
          case USART_TRANSMIT_FIFO_IDLE:
            trigger_tx_isr = (uart.tx_state == IDLE && uart.tx_buffer.empty());
            break;
          case USART_TRANSMIT_FIFO_EMPTY:
            trigger_tx_isr = uart.tx_buffer.empty();
            break;
        }

        if (trigger_tx_isr) {
          m_interrupt_controller->RaiseInterrupt(
              static_cast<INT_SOURCE>(uart.interrupt_source + 2));
        }
      }
    }

    // Raise RX interrupt if required
    if (uart.rx_enable && !uart.rx_buffer.empty()) {
      m_interrupt_controller->RaiseInterrupt(
          static_cast<INT_SOURCE>(uart.interrupt_source + 1));
    }
  }
}

void PeripheralUART::ReceiveByte(USART_MODULE_ID index, uint8_t byte) {
  if (index >= m_uarts.size()) {
    FAIL() << "Invalid UART " << index;
  }
  UART &uart = m_uarts[index];
  if (uart.rx_enable) {
    uart.rx_buffer.push(byte);
  }
}

void PeripheralUART::SignalFramingError(USART_MODULE_ID index, uint8_t byte) {
  if (index >= m_uarts.size()) {
    FAIL() << "Invalid UART " << index;
  }

  UART &uart = m_uarts[index];
  // The logic here is a bit confusing, it looks like the parity and framing
  // error bits are buffered along with the data byte.
  if (uart.rx_enable) {
    if (uart.rx_buffer.empty()) {
      uart.errors |= USART_ERROR_FRAMING;
      uart.rx_buffer.push(byte);
      m_interrupt_controller->RaiseInterrupt(
        static_cast<INT_SOURCE>(uart.interrupt_source));
    } else {
      uart.rx_buffer.push(UART::FRAMING_ERROR_FLAG | byte);
    }
  }
}

void PeripheralUART::Enable(USART_MODULE_ID index) {
  if (index >= m_uarts.size()) {
    FAIL() << "Invalid UART " << index;
  }
  m_uarts[index].enabled = true;
}

void PeripheralUART::Disable(USART_MODULE_ID index) {
  if (index >= m_uarts.size()) {
    FAIL() << "Invalid UART " << index;
  }
  UART &uart = m_uarts[index];
  uart.enabled = false;

  // 21.4.2
  while (!uart.tx_buffer.empty()) {
    uart.tx_buffer.pop();
  }
  while (!uart.rx_buffer.empty()) {
    uart.rx_buffer.pop();
  }
  uart.tx_counter = 0;
  uart.tx_state = IDLE;
  // TODO(simon): reset flags here
  uart.errors = USART_ERROR_NONE;
}

void PeripheralUART::TransmitterEnable(USART_MODULE_ID index) {
  if (index >= m_uarts.size()) {
    FAIL() << "Invalid UART " << index;
  }
  m_uarts[index].tx_enable = true;
}

void PeripheralUART::TransmitterDisable(USART_MODULE_ID index) {
  if (index >= m_uarts.size()) {
    FAIL() << "Invalid UART " << index;
  }
  m_uarts[index].tx_enable = false;
}

void PeripheralUART::BaudRateSet(USART_MODULE_ID index,
                                 uint32_t clockFrequency,
                                 uint32_t baudRate) {
  if (index >= m_uarts.size()) {
    FAIL() << "Invalid UART " << index;
  }
  m_uarts[index].ticks_per_bit = clockFrequency / baudRate;
}

void PeripheralUART::TransmitterByteSend(USART_MODULE_ID index, int8_t data) {
  if (index >= m_uarts.size()) {
    FAIL() << "Invalid UART " << index;
  }
  UART &uart = m_uarts[index];
  if (uart.tx_buffer.size() < TX_FIFO_SIZE) {
    uart.tx_buffer.push(data);
  }
}

int8_t PeripheralUART::ReceiverByteReceive(USART_MODULE_ID index) {
  if (index >= m_uarts.size()) {
    ADD_FAILURE() << "Invalid UART " << index;
    return 0;
  }
  UART &uart = m_uarts[index];
  uint8_t value = 0;
  if (!uart.rx_buffer.empty()) {
    value = uart.rx_buffer.front();
    uart.rx_buffer.pop();
    // clear the framing error bit
    uart.errors &= !USART_ERROR_FRAMING;
    if (!uart.rx_buffer.empty()) {
      // Set the next framing error bit
      if (uart.rx_buffer.front() & UART::FRAMING_ERROR_FLAG) {
        uart.errors |= USART_ERROR_FRAMING;
      }
    }
  }
  return value;
}

bool PeripheralUART::ReceiverDataIsAvailable(USART_MODULE_ID index) {
  if (index >= m_uarts.size()) {
    ADD_FAILURE() << "Invalid UART " << index;
    return false;
  }
  return !m_uarts[index].rx_buffer.empty();
}

bool PeripheralUART::TransmitterBufferIsFull(USART_MODULE_ID index) {
  if (index >= m_uarts.size()) {
    ADD_FAILURE() << "Invalid UART " << index;
    return true;
  }
  return m_uarts[index].tx_buffer.size() == TX_FIFO_SIZE;
}

void PeripheralUART::ReceiverEnable(USART_MODULE_ID index) {
  if (index >= m_uarts.size()) {
    FAIL() << "Invalid UART " << index;
  }
  m_uarts[index].rx_enable = true;
}

void PeripheralUART::ReceiverDisable(USART_MODULE_ID index) {
  if (index >= m_uarts.size()) {
    FAIL() << "Invalid UART " << index;
  }
  m_uarts[index].rx_enable = false;
}

void PeripheralUART::TransmitterInterruptModeSelect(
    USART_MODULE_ID index,
    USART_TRANSMIT_INTR_MODE fifolevel) {
  if (index >= m_uarts.size()) {
    FAIL() << "Invalid UART " << index;
  }
  m_uarts[index].int_mode = fifolevel;
}

void PeripheralUART::HandshakeModeSelect(
    USART_MODULE_ID index,
    USART_HANDSHAKE_MODE handshakeConfig) {
  if (index >= m_uarts.size()) {
    FAIL() << "Invalid UART " << index;
  }
  if (handshakeConfig != USART_HANDSHAKE_MODE_SIMPLEX) {
    FAIL() << "Unimplemented handshake mode: " << handshakeConfig;
  }
}

void PeripheralUART::OperationModeSelect(
    USART_MODULE_ID index,
    USART_OPERATION_MODE operationMode) {
  if (index >= m_uarts.size()) {
    FAIL() << "Invalid UART " << index;
  }
  if (operationMode != USART_ENABLE_TX_RX_USED) {
    FAIL() << "Unimplemented operation mode: " << operationMode;
  }
}

void PeripheralUART::LineControlModeSelect(
    USART_MODULE_ID index,
    USART_LINECONTROL_MODE dataFlowConfig) {
  if (index >= m_uarts.size()) {
    FAIL() << "Invalid UART " << index;
  }
  if (dataFlowConfig != USART_8N2) {
    FAIL() << "Unimplemented line control mode: " << dataFlowConfig;
  }
}

USART_ERROR PeripheralUART::ErrorsGet(USART_MODULE_ID index) {
  if (index >= m_uarts.size()) {
    ADD_FAILURE() << "Invalid UART " << index;
    return USART_ERROR_NONE;
  }
  // Yuck
  return static_cast<USART_ERROR>(m_uarts[index].errors);
}
