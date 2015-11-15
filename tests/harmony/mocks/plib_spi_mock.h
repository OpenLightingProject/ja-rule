#ifndef TESTS_HARMONY_MOCKS_PLIB_SPI_MOCK_H_
#define TESTS_HARMONY_MOCKS_PLIB_SPI_MOCK_H_

#include <gmock/gmock.h>
#include "peripheral/spi/plib_spi.h"

class PeripheralSPIInterface {
 public:
  virtual ~PeripheralSPIInterface() {}

  virtual void Enable(SPI_MODULE_ID index) = 0;
  virtual void Disable(SPI_MODULE_ID index) = 0;
  virtual bool TransmitBufferIsFull(SPI_MODULE_ID index) = 0;
  virtual void CommunicationWidthSelect(SPI_MODULE_ID index,
                                        SPI_COMMUNICATION_WIDTH width) = 0;
  virtual void ClockPolaritySelect(SPI_MODULE_ID index,
                                   SPI_CLOCK_POLARITY polarity) = 0;
  virtual void MasterEnable(SPI_MODULE_ID index) = 0;
  virtual void FIFOInterruptModeSelect(SPI_MODULE_ID index,
                                       SPI_FIFO_INTERRUPT mode) = 0;
  virtual void BaudRateSet(SPI_MODULE_ID index, uint32_t clockFrequency,
                           uint32_t baudRate) = 0;
  virtual bool IsBusy(SPI_MODULE_ID index) = 0;
  virtual void FIFOEnable(SPI_MODULE_ID index) = 0;
  virtual bool ReceiverFIFOIsEmpty(SPI_MODULE_ID index) = 0;
  virtual void BufferWrite(SPI_MODULE_ID index, uint8_t data) = 0;
  virtual void BufferClear(SPI_MODULE_ID index) = 0;
  virtual uint8_t BufferRead(SPI_MODULE_ID index) = 0;
  virtual void SlaveSelectDisable(SPI_MODULE_ID index) = 0;
  virtual void PinDisable(SPI_MODULE_ID index, SPI_PIN pin) = 0;
};

class MockPeripheralSPI : public PeripheralSPIInterface {
 public:
  MOCK_METHOD1(Enable, void(SPI_MODULE_ID index));
  MOCK_METHOD1(Disable, void(SPI_MODULE_ID index));
  MOCK_METHOD1(TransmitBufferIsFull, bool(SPI_MODULE_ID index));
  MOCK_METHOD2(CommunicationWidthSelect,
               void(SPI_MODULE_ID index, SPI_COMMUNICATION_WIDTH width));
  MOCK_METHOD2(ClockPolaritySelect,
               void(SPI_MODULE_ID index, SPI_CLOCK_POLARITY polarity));
  MOCK_METHOD1(MasterEnable, void(SPI_MODULE_ID index));
  MOCK_METHOD2(FIFOInterruptModeSelect,
               void(SPI_MODULE_ID index, SPI_FIFO_INTERRUPT mode));
  MOCK_METHOD3(BaudRateSet, void(SPI_MODULE_ID index, uint32_t clockFrequency,
                                 uint32_t baudRate));
  MOCK_METHOD1(IsBusy, bool(SPI_MODULE_ID index));
  MOCK_METHOD1(FIFOEnable, void(SPI_MODULE_ID index));
  MOCK_METHOD1(ReceiverFIFOIsEmpty, bool(SPI_MODULE_ID index));
  MOCK_METHOD2(BufferWrite, void(SPI_MODULE_ID index, uint8_t data));
  MOCK_METHOD1(BufferClear, void(SPI_MODULE_ID index));
  MOCK_METHOD1(BufferRead, uint8_t(SPI_MODULE_ID index));
  MOCK_METHOD1(SlaveSelectDisable, void(SPI_MODULE_ID index));
  MOCK_METHOD2(PinDisable, void(SPI_MODULE_ID index, SPI_PIN pin));
};

void PLIB_SPI_SetMock(PeripheralSPIInterface* spi);

#endif  // TESTS_HARMONY_MOCKS_PLIB_SPI_MOCK_H_
