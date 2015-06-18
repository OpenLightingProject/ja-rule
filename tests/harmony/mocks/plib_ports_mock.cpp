#include <gmock/gmock.h>
#include "plib_ports_mock.h"

namespace {
  MockPeripheralPorts *g_plib_ports_mock = NULL;
}

void PLIB_PORTS_SetMock(MockPeripheralPorts* mock) {
  g_plib_ports_mock = mock;
}

void PLIB_PORTS_PinDirectionOutputSet(PORTS_MODULE_ID index,
                                      PORTS_CHANNEL channel,
                                      PORTS_BIT_POS bitPos) {
  if (g_plib_ports_mock) {
    return g_plib_ports_mock->PinDirectionOutputSet(index, channel, bitPos);
  }
}

void PLIB_PORTS_PinSet(PORTS_MODULE_ID index,
                       PORTS_CHANNEL channel,
                       PORTS_BIT_POS bitPos) {
  if (g_plib_ports_mock) {
    return g_plib_ports_mock->PinSet(index, channel, bitPos);
  }
}

void PLIB_PORTS_PinClear(PORTS_MODULE_ID index,
                         PORTS_CHANNEL channel,
                         PORTS_BIT_POS bitPos) {
  if (g_plib_ports_mock) {
    return g_plib_ports_mock->PinClear(index, channel, bitPos);
  }
}

void PLIB_PORTS_PinToggle(PORTS_MODULE_ID index,
                          PORTS_CHANNEL channel,
                          PORTS_BIT_POS bitPos) {
  if (g_plib_ports_mock) {
    return g_plib_ports_mock->PinToggle(index, channel, bitPos);
  }
}
