#include <gmock/gmock.h>
#include "plib_spi_mock.h"

namespace {
  MockPeripheralSPI *g_plib_spi_mock = NULL;
}

void PLIB_SPI_SetMock(MockPeripheralSPI* mock) {
  g_plib_spi_mock = mock;
}

void PLIB_SPI_Enable(SPI_MODULE_ID index) {
  if (g_plib_spi_mock) {
    g_plib_spi_mock->Enable(index);
  }
}

bool PLIB_SPI_TransmitBufferIsFull(SPI_MODULE_ID index) {
  if (g_plib_spi_mock) {
    return g_plib_spi_mock->TransmitBufferIsFull(index);
  }
  return false;
}

void PLIB_SPI_CommunicationWidthSelect(SPI_MODULE_ID index,
                                       SPI_COMMUNICATION_WIDTH width) {
  if (g_plib_spi_mock) {
    g_plib_spi_mock->CommunicationWidthSelect(index, width);
  }
}

void PLIB_SPI_ClockPolaritySelect(SPI_MODULE_ID index,
                                  SPI_CLOCK_POLARITY polarity) {
  if (g_plib_spi_mock) {
    g_plib_spi_mock->ClockPolaritySelect(index, polarity);
  }
}

void PLIB_SPI_MasterEnable(SPI_MODULE_ID index) {
  if (g_plib_spi_mock) {
    g_plib_spi_mock->MasterEnable(index);
  }
}

void PLIB_SPI_BaudRateSet(SPI_MODULE_ID index, uint32_t clockFrequency,
                          uint32_t baudRate) {
  if (g_plib_spi_mock) {
    g_plib_spi_mock->BaudRateSet(index, clockFrequency, baudRate);
  }
}

bool PLIB_SPI_IsBusy(SPI_MODULE_ID index) {
  if (g_plib_spi_mock) {
    return g_plib_spi_mock->IsBusy(index);
  }
  return false;
}

void PLIB_SPI_FIFOEnable(SPI_MODULE_ID index) {
  if (g_plib_spi_mock) {
    g_plib_spi_mock->FIFOEnable(index);
  }
}

void PLIB_SPI_BufferWrite(SPI_MODULE_ID index, uint8_t data) {
  if (g_plib_spi_mock) {
    g_plib_spi_mock->BufferWrite(index, data);
  }
}

void PLIB_SPI_SlaveSelectDisable(SPI_MODULE_ID index) {
  if (g_plib_spi_mock) {
    g_plib_spi_mock->SlaveSelectDisable(index);
  }
}

void PLIB_SPI_PinDisable(SPI_MODULE_ID index, SPI_PIN pin) {
  if (g_plib_spi_mock) {
    g_plib_spi_mock->PinDisable(index, pin);
  }
}
