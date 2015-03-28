#ifndef TESTS_HARMONY_MOCKS_PLIB_USART_MOCK_H_
#define TESTS_HARMONY_MOCKS_PLIB_USART_MOCK_H_

#include <gmock/gmock.h>
#include "peripheral/usart/plib_usart.h"

class MockPeripheralUSART {
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
  MOCK_METHOD2(SyncModeSelect,
               void(USART_MODULE_ID index, USART_SYNC_MODES mode));
  MOCK_METHOD2(LineControlModeSelect,
               void(USART_MODULE_ID index,
                    USART_LINECONTROL_MODE dataFlowConfig));
};

void PLIB_USART_SetMock(MockPeripheralUSART* mock);

#endif  // TESTS_HARMONY_MOCKS_PLIB_USART_MOCK_H_
