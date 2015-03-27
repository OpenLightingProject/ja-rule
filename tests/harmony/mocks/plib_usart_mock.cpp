#include <gmock/gmock.h>
#include "plib_usart_mock.h"

namespace {
  MockPeripheralUSART *g_plib_usart_mock = NULL;
}

void PLIB_USART_SetMock(MockPeripheralUSART* mock) {
  g_plib_usart_mock = mock;
}

void PLIB_USART_Enable(USART_MODULE_ID index) {
  if (g_plib_usart_mock) {
    g_plib_usart_mock->Enable(index);
  }
}

void PLIB_USART_Disable(USART_MODULE_ID index) {
  if (g_plib_usart_mock) {
    g_plib_usart_mock->Disable(index);
  }
}

void PLIB_USART_TransmitterEnable(USART_MODULE_ID index) {
  if (g_plib_usart_mock) {
    g_plib_usart_mock->TransmitterEnable(index);
  }
}

void PLIB_USART_TransmitterDisable(USART_MODULE_ID index) {
  if (g_plib_usart_mock) {
    g_plib_usart_mock->TransmitterDisable(index);
  }
}

void PLIB_USART_BaudRateSet(USART_MODULE_ID index, uint32_t clockFrequency,
                            uint32_t baudRate) {
  if (g_plib_usart_mock) {
    g_plib_usart_mock->BaudRateSet(index, clockFrequency, baudRate);
  }
}

void PLIB_USART_TransmitterByteSend(USART_MODULE_ID index, int8_t data) {
  if (g_plib_usart_mock) {
    g_plib_usart_mock->TransmitterByteSend(index, data);
  }
}

int8_t PLIB_USART_ReceiverByteReceive(USART_MODULE_ID index) {
  if (g_plib_usart_mock) {
    return g_plib_usart_mock->ReceiverByteReceive(index);
  }
  return 0;
}

bool PLIB_USART_ReceiverDataIsAvailable(USART_MODULE_ID index) {
  if (g_plib_usart_mock) {
    return g_plib_usart_mock->ReceiverDataIsAvailable(index);
  }
  return false;
}

bool PLIB_USART_TransmitterBufferIsFull(USART_MODULE_ID index) {
  if (g_plib_usart_mock) {
    return g_plib_usart_mock->TransmitterBufferIsFull(index);
  }
  return false;
}

void PLIB_USART_ReceiverEnable(USART_MODULE_ID index) {
  if (g_plib_usart_mock) {
    g_plib_usart_mock->ReceiverEnable(index);
  }
}

void PLIB_USART_ReceiverDisable(USART_MODULE_ID index) {
  if (g_plib_usart_mock) {
    g_plib_usart_mock->ReceiverDisable(index);
  }
}

void PLIB_USART_TransmitterInterruptModeSelect(
    USART_MODULE_ID index,
    USART_TRANSMIT_INTR_MODE fifolevel) {
  if (g_plib_usart_mock) {
    g_plib_usart_mock->TransmitterInterruptModeSelect(index, fifolevel);
  }
}

void PLIB_USART_HandshakeModeSelect(USART_MODULE_ID index,
                                    USART_HANDSHAKE_MODE handshakeConfig) {
  if (g_plib_usart_mock) {
    g_plib_usart_mock->HandshakeModeSelect(index, handshakeConfig);
  }
}

void PLIB_USART_OperationModeSelect(USART_MODULE_ID index,
                                    USART_OPERATION_MODE operationmode) {
  if (g_plib_usart_mock) {
    g_plib_usart_mock->OperationModeSelect(index, operationmode);
  }
}

void PLIB_USART_SyncModeSelect(USART_MODULE_ID index, USART_SYNC_MODES mode) {
  if (g_plib_usart_mock) {
    g_plib_usart_mock->SyncModeSelect(index, mode);
  }
}

void PLIB_USART_LineControlModeSelect(USART_MODULE_ID index,
                                      USART_LINECONTROL_MODE dataFlowConfig) {
  if (g_plib_usart_mock) {
    g_plib_usart_mock->LineControlModeSelect(index, dataFlowConfig);
  }
}
