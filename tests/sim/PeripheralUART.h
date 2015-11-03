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
 * PeripheralUART.h
 * The UART used with the simulator.
 * Copyright (C) 2015 Simon Newton
 */

#ifndef TESTS_SIM_PERIPHERALUART_H_
#define TESTS_SIM_PERIPHERALUART_H_

#include <memory>
#include <queue>
#include <vector>

#include "plib_usart_mock.h"

#include "InterruptController.h"
#include "Simulator.h"
#include "ola/Callback.h"

class PeripheralUART : public PeripheralUSARTInterface {
 public:
  // Invoked when a byte is transmitted.
  typedef ola::Callback2<void, USART_MODULE_ID, uint8_t> TXCallback;

  // Ownership of arguments is not transferred.
  PeripheralUART(Simulator *simulator,
                 InterruptController *interrupt_controller,
                 TXCallback *tx_callback);
  ~PeripheralUART();

  void Tick();

  // Used to push a byte of data to the receiver.
  void ReceiveByte(USART_MODULE_ID index, uint8_t byte);

  // Signal a framing error has occured.
  void SignalFramingError(USART_MODULE_ID index, uint8_t byte);

  void Enable(USART_MODULE_ID index);
  void Disable(USART_MODULE_ID index);
  void TransmitterEnable(USART_MODULE_ID index);
  void TransmitterDisable(USART_MODULE_ID index);
  void BaudRateSet(USART_MODULE_ID index, uint32_t clockFrequency,
                   uint32_t baudRate);
  void TransmitterByteSend(USART_MODULE_ID index, int8_t data);
  int8_t ReceiverByteReceive(USART_MODULE_ID index);
  bool ReceiverDataIsAvailable(USART_MODULE_ID index);
  bool TransmitterBufferIsFull(USART_MODULE_ID index);
  void ReceiverEnable(USART_MODULE_ID index);
  void ReceiverDisable(USART_MODULE_ID index);
  void TransmitterInterruptModeSelect(
      USART_MODULE_ID index, USART_TRANSMIT_INTR_MODE fifolevel);
  void HandshakeModeSelect(USART_MODULE_ID index,
                           USART_HANDSHAKE_MODE handshakeConfig);
  void OperationModeSelect(USART_MODULE_ID index,
                           USART_OPERATION_MODE operationMode);
  void LineControlModeSelect(USART_MODULE_ID index,
                             USART_LINECONTROL_MODE dataFlowConfig);
  USART_ERROR ErrorsGet(USART_MODULE_ID index);

 private:
  Simulator *m_simulator;
  InterruptController *m_interrupt_controller;
  TXCallback *m_tx_callback;
  std::unique_ptr<ola::Callback0<void>> m_callback;

  enum UARTState {
    IDLE,
    START_BIT,
    BIT_0,
    BIT_1,
    BIT_2,
    BIT_3,
    BIT_4,
    BIT_5,
    BIT_6,
    BIT_7,
    STOP_BIT_1,
    STOP_BIT_2,
  };

  struct UART {
   public:
    explicit UART(INT_SOURCE source);

    const INT_SOURCE interrupt_source;
    bool enabled;
    bool tx_enable;
    bool rx_enable;
    USART_TRANSMIT_INTR_MODE int_mode;

    std::queue<uint8_t> tx_buffer;
    // The MSB bit holds the state of the framing error
    std::queue<uint16_t> rx_buffer;
    uint8_t tx_byte;
    uint8_t errors;

    uint32_t ticks_per_bit;
    uint32_t tx_counter;
    UARTState tx_state;

    static const uint16_t FRAMING_ERROR_FLAG = 0x8000;
  };

  std::vector<UART> m_uarts;
  static const uint8_t TX_FIFO_SIZE = 8;
  static const uint8_t RX_FIFO_SIZE = 8;
};


#endif  // TESTS_SIM_PERIPHERALUART_H_
