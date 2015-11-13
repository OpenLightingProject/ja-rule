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
 * PeripheralSPI.h
 * The SPI module used with the simulator.
 * Copyright (C) 2015 Simon Newton
 */

#ifndef TESTS_SIM_PERIPHERALSPI_H_
#define TESTS_SIM_PERIPHERALSPI_H_

#include <deque>
#include <memory>
#include <vector>

#include "plib_spi_mock.h"

#include "InterruptController.h"
#include "Simulator.h"
#include "ola/Callback.h"

class PeripheralSPI : public PeripheralSPIInterface {
 public:
  // Ownership is not transferred.
  PeripheralSPI(Simulator *simulator,
                InterruptController *interrupt_controller);
  ~PeripheralSPI();

  // Queue rx data
  void QueueResponseByte(SPI_MODULE_ID index, uint8_t data);

  std::vector<uint8_t> SentBytes(SPI_MODULE_ID index);

  void Tick();

  void Enable(SPI_MODULE_ID index);
  void Disable(SPI_MODULE_ID index);
  bool TransmitBufferIsFull(SPI_MODULE_ID index);
  void CommunicationWidthSelect(SPI_MODULE_ID index,
                                       SPI_COMMUNICATION_WIDTH width);
  void ClockPolaritySelect(SPI_MODULE_ID index,
                                  SPI_CLOCK_POLARITY polarity);
  void MasterEnable(SPI_MODULE_ID index);
  void FIFOInterruptModeSelect(SPI_MODULE_ID index,
                                      SPI_FIFO_INTERRUPT mode);
  void BaudRateSet(SPI_MODULE_ID index, uint32_t clockFrequency,
                          uint32_t baudRate);
  bool IsBusy(SPI_MODULE_ID index);
  void FIFOEnable(SPI_MODULE_ID index);
  bool ReceiverFIFOIsEmpty(SPI_MODULE_ID index);
  void BufferWrite(SPI_MODULE_ID index, uint8_t data);
  void BufferClear(SPI_MODULE_ID index);
  uint8_t BufferRead(SPI_MODULE_ID index);
  void SlaveSelectDisable(SPI_MODULE_ID index);
  void PinDisable(SPI_MODULE_ID index, SPI_PIN pin);

 private:
  typedef std::vector<uint8_t> ByteVector;

  Simulator *m_simulator;
  InterruptController *m_interrupt_controller;
  std::unique_ptr<ola::Callback0<void>> m_callback;

  struct SPI {
   public:
    explicit SPI(INT_SOURCE source);

    bool enabled;
    const INT_SOURCE interrupt_source;
    uint8_t fifo_size;
    uint32_t ticks_per_byte;
    uint32_t counter;
    bool in_transfer;
    bool has_overflowed;
    SPI_FIFO_INTERRUPT rx_interrupt_mode;
    SPI_FIFO_INTERRUPT tx_interrupt_mode;

    // The queue for outgoing bytes.
    std::deque<uint32_t> tx_queue;
    // The queue for incoming bytes.
    std::deque<uint32_t> rx_queue;

    // All outgoing bytes.
    ByteVector sent_bytes;
    // Incoming bytes to return.
    ByteVector incoming_bytes;
    ByteVector::iterator rx_iter;

    static const uint8_t ENHANCED_BUFFER_SIZE = 8;
  };

  std::vector<SPI> m_spi;
};

#endif  // TESTS_SIM_PERIPHERALSPI_H_
