/*
 * This is the stub for plib_usart.h used for the tests. It contains the bare
 * minimum required to implement the mock USART symbols.
 */

#ifndef TESTS_HARMONY_INCLUDE_PERIPHERAL_USART_PLIB_USART_H_
#define TESTS_HARMONY_INCLUDE_PERIPHERAL_USART_PLIB_USART_H_

#ifdef  __cplusplus
extern "C" {
#endif

typedef enum {
  USART_ID_1 = 0,
  USART_ID_3,
  USART_ID_2,
  USART_ID_4,
  USART_ID_6,
  USART_ID_5,
  USART_NUMBER_OF_MODULES
} USART_MODULE_ID;

typedef enum {
  USART_TRANSMIT_FIFO_NOT_FULL = 0x00,
  USART_TRANSMIT_FIFO_IDLE = 0x01,
  USART_TRANSMIT_FIFO_EMPTY = 0x02
} USART_TRANSMIT_INTR_MODE;

typedef enum {
  USART_HANDSHAKE_MODE_FLOW_CONTROL = 0x00,
  USART_HANDSHAKE_MODE_SIMPLEX = 0x01
} USART_HANDSHAKE_MODE;

typedef enum {
  USART_ENABLE_TX_RX_BCLK_USED = 0x03,
  USART_ENABLE_TX_RX_CTS_RTS_USED = 0x02,
  USART_ENABLE_TX_RX_RTS_USED = 0x01,
  USART_ENABLE_TX_RX_USED = 0x00
} USART_OPERATION_MODE;

typedef enum {
  USART_ASYNC_MODE = 0x00,
  USART_SYNC_MODE = 0x01
} USART_SYNC_MODES;

typedef enum {
  USART_8N1 = 0x00,
  USART_8E1 = 0x01,
  USART_8O1 = 0x02,
  USART_9N1 = 0x03,
  USART_8N2 = 0x04,
  USART_8E2 = 0x05,
  USART_8O2 = 0x06,
  USART_9N2 = 0x07
} USART_LINECONTROL_MODE;

typedef enum {
  USART_ERROR_NONE = 0x00,
  USART_ERROR_RECEIVER_OVERRUN = 0x01,
  USART_ERROR_FRAMING = 0x02,
  USART_ERROR_PARITY = 0x04
} USART_ERROR;

void PLIB_USART_Enable(USART_MODULE_ID index);

void PLIB_USART_Disable(USART_MODULE_ID index);

void PLIB_USART_TransmitterEnable(USART_MODULE_ID index);

void PLIB_USART_TransmitterDisable(USART_MODULE_ID index);

void PLIB_USART_BaudRateSet(USART_MODULE_ID index, uint32_t clockFrequency,
    uint32_t baudRate);

void PLIB_USART_TransmitterByteSend(USART_MODULE_ID index, int8_t data);

int8_t PLIB_USART_ReceiverByteReceive(USART_MODULE_ID index);

bool PLIB_USART_ReceiverDataIsAvailable(USART_MODULE_ID index);

bool PLIB_USART_TransmitterBufferIsFull(USART_MODULE_ID index);

void PLIB_USART_ReceiverEnable(USART_MODULE_ID index);

void PLIB_USART_ReceiverDisable(USART_MODULE_ID index);

void PLIB_USART_TransmitterInterruptModeSelect(
    USART_MODULE_ID index,
    USART_TRANSMIT_INTR_MODE fifolevel);

void PLIB_USART_HandshakeModeSelect(USART_MODULE_ID index,
                                    USART_HANDSHAKE_MODE handshakeConfig);

void PLIB_USART_OperationModeSelect(USART_MODULE_ID index,
                                    USART_OPERATION_MODE operationmode);

void PLIB_USART_LineControlModeSelect(USART_MODULE_ID index,
                                      USART_LINECONTROL_MODE dataFlowConfig);

USART_ERROR PLIB_USART_ErrorsGet(USART_MODULE_ID index);

#ifdef  __cplusplus
}
#endif

#endif  // TESTS_HARMONY_INCLUDE_PERIPHERAL_USART_PLIB_USART_H_
