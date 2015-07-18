#ifndef TESTS_HARMONY_MOCKS_PLIB_SPI_MOCK_H_
#define TESTS_HARMONY_MOCKS_PLIB_SPI_MOCK_H_

#include <gmock/gmock.h>
#include "peripheral/spi/plib_spi.h"

class MockPeripheralSPI {
 public:
  MOCK_METHOD1(Enable, void(SPI_MODULE_ID index));
  MOCK_METHOD1(TransmitBufferIsFull, bool(SPI_MODULE_ID index));
  MOCK_METHOD2(CommunicationWidthSelect,
               void(SPI_MODULE_ID index, SPI_COMMUNICATION_WIDTH width));
  MOCK_METHOD2(ClockPolaritySelect,
               void(SPI_MODULE_ID index, SPI_CLOCK_POLARITY polarity));
  MOCK_METHOD1(MasterEnable, void(SPI_MODULE_ID index));
  MOCK_METHOD3(BaudRateSet, void(SPI_MODULE_ID index, uint32_t clockFrequency,
                                 uint32_t baudRate));
  MOCK_METHOD1(IsBusy, bool(SPI_MODULE_ID index));
  MOCK_METHOD1(FIFOEnable, void(SPI_MODULE_ID index));
  MOCK_METHOD2(BufferWrite, void(SPI_MODULE_ID index, uint8_t data));
  MOCK_METHOD1(SlaveSelectDisable, void(SPI_MODULE_ID index));
  MOCK_METHOD2(PinDisable, void(SPI_MODULE_ID index, SPI_PIN pin));
};

void PLIB_SPI_SetMock(MockPeripheralSPI* mock);

#endif  // TESTS_HARMONY_MOCKS_PLIB_SPI_MOCK_H_
