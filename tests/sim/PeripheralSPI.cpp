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
 * PeripheralSPI.cpp
 * The SPI module used with the simulator.
 * Copyright (C) 2015 Simon Newton
 */

#include "PeripheralSPI.h"

#include <gtest/gtest.h>

#include <vector>

#include "macros.h"
#include "Simulator.h"
#include "ola/Callback.h"

using std::vector;

PeripheralSPI::SPI::SPI(INT_SOURCE source)
    : enabled(false),
      interrupt_source(source),
      fifo_size(1),
      ticks_per_byte(0),
      counter(0),
      in_transfer(false),
      has_overflowed(false),
      rx_interrupt_mode(SPI_FIFO_INTERRUPT_WHEN_RECEIVE_BUFFER_IS_FULL),
      tx_interrupt_mode(SPI_FIFO_INTERRUPT_WHEN_TRANSMIT_BUFFER_IS_NOT_FULL),
      rx_iter(incoming_bytes.begin()) {
}


PeripheralSPI::PeripheralSPI(
    Simulator *simulator,
    InterruptController *interrupt_controller)
    : m_simulator(simulator),
      m_interrupt_controller(interrupt_controller),
      m_callback(ola::NewCallback(this, &PeripheralSPI::Tick)) {
  m_simulator->AddTask(m_callback.get());
  std::vector<INT_SOURCE> spi_sources = {
    INT_SOURCE_SPI_1_ERROR,
    INT_SOURCE_SPI_2_ERROR,
    INT_SOURCE_SPI_3_ERROR,
    INT_SOURCE_SPI_4_ERROR,
  };
  for (const auto &source : spi_sources) {
    m_spi.push_back(SPI(source));
  }
}

PeripheralSPI::~PeripheralSPI() {
  m_simulator->RemoveTask(m_callback.get());
}

void PeripheralSPI::QueueResponseByte(SPI_MODULE_ID index, uint8_t data) {
  if (index >= m_spi.size()) {
    ADD_FAILURE() << "Invalid SPI " << index;
    return;
  }

  SPI *spi = &m_spi[index];
  bool reset_iter = spi->rx_iter == spi->incoming_bytes.end();
  spi->incoming_bytes.push_back(data);
  if (reset_iter) {
    spi->rx_iter = spi->incoming_bytes.end();
    spi->rx_iter--;
  }
}

vector<uint8_t> PeripheralSPI::SentBytes(SPI_MODULE_ID index) {
  if (index >= m_spi.size()) {
    ADD_FAILURE() << "Invalid SPI " << index;
    return vector<uint8_t>();
  }

  SPI *spi = &m_spi[index];
  return spi->sent_bytes;
}

void PeripheralSPI::Tick() {
  for (auto &spi : m_spi) {
    if (!spi.enabled) {
      continue;
    }

    bool run_tx_isr = false;
    if (spi.in_transfer) {
      spi.counter++;
      if (spi.counter == spi.ticks_per_byte) {
        // transfer is complete.
        uint8_t tx_data = spi.tx_queue.front();
        spi.tx_queue.pop_front();
        spi.sent_bytes.push_back(tx_data);

        uint8_t rx_data = 0;
        if (spi.rx_iter != spi.incoming_bytes.end()) {
          rx_data = *spi.rx_iter;
          spi.rx_iter++;
        }
        if (spi.rx_queue.size() < spi.fifo_size) {
          spi.rx_queue.push_back(rx_data);
        } else {
          spi.has_overflowed = true;
        }
        spi.in_transfer = false;

        if (spi.tx_interrupt_mode ==
            SPI_FIFO_INTERRUPT_WHEN_TRANSMISSION_IS_COMPLETE &&
            spi.tx_queue.empty()) {
          // We sent the last byte.
          run_tx_isr = true;
        }
      }
    }

    if (!spi.in_transfer) {
      // Start the next byte.
      spi.in_transfer = true;
      spi.counter = 0;
    }

    switch (spi.tx_interrupt_mode) {
      case SPI_FIFO_INTERRUPT_WHEN_TRANSMIT_BUFFER_IS_NOT_FULL:
        run_tx_isr = spi.tx_queue.size() != spi.fifo_size;
        break;
      case SPI_FIFO_INTERRUPT_WHEN_TRANSMIT_BUFFER_IS_1HALF_EMPTY_OR_MORE:
        run_tx_isr = spi.tx_queue.size() < spi.fifo_size / 2;
        break;
      case SPI_FIFO_INTERRUPT_WHEN_TRANSMIT_BUFFER_IS_COMPLETELY_EMPTY:
        run_tx_isr = spi.tx_queue.empty();
        break;
      default:
        {}
    }

    if (run_tx_isr) {
      m_interrupt_controller->RaiseInterrupt(
          static_cast<INT_SOURCE>(spi.interrupt_source + 2));
    }

    bool run_rx_isr = false;
    switch (spi.rx_interrupt_mode) {
      case SPI_FIFO_INTERRUPT_WHEN_RECEIVE_BUFFER_IS_FULL:
        run_rx_isr = spi.rx_queue.size() == spi.fifo_size;
        break;
      case SPI_FIFO_INTERRUPT_WHEN_RECEIVE_BUFFER_IS_1HALF_FULL_OR_MORE:
        run_tx_isr = spi.rx_queue.size() >= spi.fifo_size / 2;
        break;
      case SPI_FIFO_INTERRUPT_WHEN_RECEIVE_BUFFER_IS_NOT_EMPTY:
        run_tx_isr = !spi.rx_queue.empty();
        break;
      case SPI_FIFO_INTERRUPT_WHEN_BUFFER_IS_EMPTY:
        run_tx_isr = spi.rx_queue.empty();
        break;
      default:
        {}
    }

    if (run_rx_isr) {
      m_interrupt_controller->RaiseInterrupt(
          static_cast<INT_SOURCE>(spi.interrupt_source + 1));
    }
  }
}


void PeripheralSPI::Enable(SPI_MODULE_ID index) {
  if (index >= m_spi.size()) {
    ADD_FAILURE() << "Invalid SPI " << index;
  }
  m_spi[index].enabled = true;
}

void PeripheralSPI::Disable(SPI_MODULE_ID index) {
  if (index >= m_spi.size()) {
    ADD_FAILURE() << "Invalid SPI " << index;
  }
  SPI *spi = &m_spi[index];
  spi->enabled = false;
  spi->has_overflowed = false;
}

bool PeripheralSPI::TransmitBufferIsFull(SPI_MODULE_ID index) {
  if (index >= m_spi.size()) {
    ADD_FAILURE() << "invalid SPI " << index;
  }
  return m_spi[index].tx_queue.size() == m_spi[index].fifo_size;
}

void PeripheralSPI::CommunicationWidthSelect(
    SPI_MODULE_ID index,
    UNUSED SPI_COMMUNICATION_WIDTH width) {
  if (index >= m_spi.size()) {
    ADD_FAILURE() << "Invalid SPI " << index;
  }
}

void PeripheralSPI::ClockPolaritySelect(SPI_MODULE_ID index,
                                        UNUSED SPI_CLOCK_POLARITY polarity) {
  if (index >= m_spi.size()) {
    ADD_FAILURE() << "Invalid SPI " << index;
  }
}

void PeripheralSPI::MasterEnable(SPI_MODULE_ID index) {
  if (index >= m_spi.size()) {
    ADD_FAILURE() << "Invalid SPI " << index;
  }
}

void PeripheralSPI::FIFOInterruptModeSelect(SPI_MODULE_ID index,
                                            SPI_FIFO_INTERRUPT mode) {
  if (index >= m_spi.size()) {
    ADD_FAILURE() << "invalid spi " << index;
    return;
  }
  SPI *spi = &m_spi[index];
  if (mode >= SPI_FIFO_INTERRUPT_WHEN_RECEIVE_BUFFER_IS_FULL) {
    spi->rx_interrupt_mode = mode;
  } else {
    spi->tx_interrupt_mode = mode;
  }
}

void PeripheralSPI::BaudRateSet(SPI_MODULE_ID index,
                                uint32_t clockFrequency,
                                uint32_t baudRate) {
  if (index >= m_spi.size()) {
    ADD_FAILURE() << "Invalid SPI " << index;
    return;
  }
  m_spi[index].ticks_per_byte = 8 * clockFrequency / baudRate;
}

bool PeripheralSPI::IsBusy(SPI_MODULE_ID index) {
  if (index >= m_spi.size()) {
    ADD_FAILURE() << "Invalid SPI " << index;
    return true;
  }
  return !m_spi[index].tx_queue.empty();
}

void PeripheralSPI::FIFOEnable(SPI_MODULE_ID index) {
  if (index >= m_spi.size()) {
    ADD_FAILURE() << "Invalid SPI " << index;
    return;
  }
  m_spi[index].fifo_size = SPI::ENHANCED_BUFFER_SIZE;
}

bool PeripheralSPI::ReceiverFIFOIsEmpty(SPI_MODULE_ID index) {
  if (index >= m_spi.size()) {
    ADD_FAILURE() << "Invalid SPI " << index;
    return true;
  }
  return m_spi[index].rx_queue.empty();
}

void PeripheralSPI::BufferWrite(SPI_MODULE_ID index, uint8_t data) {
  if (index >= m_spi.size()) {
    ADD_FAILURE() << "Invalid SPI " << index;
  }
  SPI *spi = &m_spi[index];
  if (spi->tx_queue.size() > spi->fifo_size) {
    ADD_FAILURE() << "TX queue overflow for SPI " << index;
    return;
  }
  spi->tx_queue.push_back(data);
}

void PeripheralSPI::BufferClear(SPI_MODULE_ID index) {
  if (index >= m_spi.size()) {
    ADD_FAILURE() << "Invalid SPI " << index;
  }
  m_spi[index].tx_queue.clear();
  m_spi[index].rx_queue.clear();
}

uint8_t PeripheralSPI::BufferRead(SPI_MODULE_ID index) {
  if (index >= m_spi.size()) {
    ADD_FAILURE() << "Invalid SPI " << index;
  }
  SPI *spi = &m_spi[index];
  if (spi->rx_queue.empty()) {
    ADD_FAILURE() << "SPI buffer underrun, index " << index;
    return 0;
  }
  uint8_t data = spi->rx_queue.front();
  spi->rx_queue.pop_front();
  return data;
}

void PeripheralSPI::SlaveSelectDisable(SPI_MODULE_ID index) {
  if (index >= m_spi.size()) {
    ADD_FAILURE() << "Invalid SPI " << index;
  }
}

void PeripheralSPI::PinDisable(SPI_MODULE_ID index, UNUSED SPI_PIN pin) {
  if (index >= m_spi.size()) {
    ADD_FAILURE() << "Invalid SPI " << index;
  }
}
