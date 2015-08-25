#ifndef TESTS_HARMONY_MOCKS_PLIB_PORTS_MOCK_H_
#define TESTS_HARMONY_MOCKS_PLIB_PORTS_MOCK_H_

#include <gmock/gmock.h>
#include "peripheral/ports/plib_ports.h"

class MockPeripheralPorts {
 public:
  MOCK_METHOD3(PinDirectionOutputSet,
               void(PORTS_MODULE_ID index,
                    PORTS_CHANNEL channel,
                    PORTS_BIT_POS bitPos));
  MOCK_METHOD3(PinGet,
               bool(PORTS_MODULE_ID index,
                    PORTS_CHANNEL channel,
                    PORTS_BIT_POS bitPos));
  MOCK_METHOD3(PinSet,
               void(PORTS_MODULE_ID index,
                    PORTS_CHANNEL channel,
                    PORTS_BIT_POS bitPos));
  MOCK_METHOD3(PinClear,
               void(PORTS_MODULE_ID index,
                    PORTS_CHANNEL channel,
                    PORTS_BIT_POS bitPos));

  MOCK_METHOD3(PinToggle,
               void(PORTS_MODULE_ID index,
                    PORTS_CHANNEL channel,
                    PORTS_BIT_POS bitPos));
};

void PLIB_PORTS_SetMock(MockPeripheralPorts* mock);

#endif  // TESTS_HARMONY_MOCKS_PLIB_PORTS_MOCK_H_
