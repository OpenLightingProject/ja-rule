#ifndef TESTS_HARMONY_MOCKS_PLIB_USART_MOCK_H_
#define TESTS_HARMONY_MOCKS_PLIB_USART_MOCK_H_

#include <gmock/gmock.h>
#include "peripheral/usart/plib_usart.h"

class PeripheralUSARTInterface {
 public:
  virtual ~PeripheralUSARTInterface() {}

  virtual void Enable(USART_MODULE_ID index) = 0;
  virtual void Disable(USART_MODULE_ID index) = 0;
  virtual void TransmitterEnable(USART_MODULE_ID index) = 0;
  virtual void TransmitterDisable(USART_MODULE_ID index) = 0;
  virtual void BaudRateSet(USART_MODULE_ID index, uint32_t clockFrequency,
                           uint32_t baudRate) = 0;
  virtual void TransmitterByteSend(USART_MODULE_ID index, int8_t data) = 0;
  virtual int8_t ReceiverByteReceive(USART_MODULE_ID index) = 0;
  virtual bool ReceiverDataIsAvailable(USART_MODULE_ID index) = 0;
  virtual bool TransmitterBufferIsFull(USART_MODULE_ID index) = 0;

  virtual void ReceiverEnable(USART_MODULE_ID index) = 0;
  virtual void ReceiverDisable(USART_MODULE_ID index) = 0;
  virtual void TransmitterInterruptModeSelect(
      USART_MODULE_ID index, USART_TRANSMIT_INTR_MODE fifolevel) = 0;
  virtual void HandshakeModeSelect(USART_MODULE_ID index,
                                   USART_HANDSHAKE_MODE handshakeConfig) = 0;
  virtual void OperationModeSelect(USART_MODULE_ID index,
                                   USART_OPERATION_MODE operationmode) = 0;
  virtual void LineControlModeSelect(USART_MODULE_ID index,
                                     USART_LINECONTROL_MODE dataFlowConfig) = 0;
  virtual USART_ERROR ErrorsGet(USART_MODULE_ID index) = 0;
};

class MockPeripheralUSART : public PeripheralUSARTInterface {
 public:
  MOCK_METHOD1(Enable, void(USART_MODULE_ID index));
  MOCK_METHOD1(Disable, void(USART_MODULE_ID index));
  MOCK_METHOD1(TransmitterEnable, void(USART_MODULE_ID index));
  MOCK_METHOD1(TransmitterDisable, void(USART_MODULE_ID index));
  MOCK_METHOD3(BaudRateSet,
               void(USART_MODULE_ID index, uint32_t clockFrequency,
                    uint32_t baudRate));
  MOCK_METHOD2(TransmitterByteSend, void(USART_MODULE_ID index, int8_t data));
  MOCK_METHOD1(ReceiverByteReceive, int8_t(USART_MODULE_ID index));
  MOCK_METHOD1(ReceiverDataIsAvailable, bool(USART_MODULE_ID index));
  MOCK_METHOD1(TransmitterBufferIsFull, bool(USART_MODULE_ID index));

  MOCK_METHOD1(ReceiverEnable, void(USART_MODULE_ID index));
  MOCK_METHOD1(ReceiverDisable, void(USART_MODULE_ID index));
  MOCK_METHOD2(TransmitterInterruptModeSelect,
               void(USART_MODULE_ID index, USART_TRANSMIT_INTR_MODE fifolevel));
  MOCK_METHOD2(HandshakeModeSelect,
               void(USART_MODULE_ID index,
                    USART_HANDSHAKE_MODE handshakeConfig));
  MOCK_METHOD2(OperationModeSelect,
               void(USART_MODULE_ID index, USART_OPERATION_MODE operationmode));
  MOCK_METHOD2(LineControlModeSelect,
               void(USART_MODULE_ID index,
                    USART_LINECONTROL_MODE dataFlowConfig));
  MOCK_METHOD1(ErrorsGet, USART_ERROR(USART_MODULE_ID index));
};

void PLIB_USART_SetMock(PeripheralUSARTInterface* mock);

#endif  // TESTS_HARMONY_MOCKS_PLIB_USART_MOCK_H_
