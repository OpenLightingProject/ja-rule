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
 * USBTransportTest.cpp
 * Tests for the USB Transport code.
 * Copyright (C) 2015 Simon Newton
 */

#include <string.h>

#include "system_definitions.h"

#include "Array.h"
#include "CMockaWrapper.h"
#include "LoggerMock.h"
#include "USBMock.h"
#include "flags.h"
#include "usb_transport.h"

/*
 * @brief Put the USB Device into configured mode.
 * @param event_handler, the captured USBEventHandler that can later be used to
 *   trigger events.
 */
void ConfigureDevice(USBEventHandler *event_handler) {
  // The value here doesn't matter, we just need a pointer to represent the
  // device.
  uint32_t dummy_device = 0;

  expect_any(USB_DEVICE_Open, index);
  expect_any(USB_DEVICE_Open, intent);
  will_return(USB_DEVICE_Open, &dummy_device);

  expect_value(USB_DEVICE_EventHandlerSet, usb_device, &dummy_device);
  expect_any(USB_DEVICE_EventHandlerSet, cb);
  expect_value(USB_DEVICE_EventHandlerSet, context, 0);

  will_return(USB_DEVICE_EventHandlerSet, event_handler);

  USBTransport_Tasks();

  uint8_t configurationValue = 1;
  (*event_handler)(USB_DEVICE_EVENT_CONFIGURED,
                   reinterpret_cast<void*>(&configurationValue), 0);
}

/*
 * @brief Trigger a write-complete event.
 * @param event_handler The USBEventHandler to use to trigger the event.
 */
void CompleteWrite(USBEventHandler event_handler) {
  uint8_t configurationValue = 0;
  event_handler(USB_DEVICE_EVENT_ENDPOINT_WRITE_COMPLETE,
                reinterpret_cast<void*>(&configurationValue), 0);
}

/*
 * Check an uninitialized device doesn't send anything.
 */
void testUninitializedDevice(void **state) {
  USBTransport_Initialize(nullptr);
  USBTransport_SendResponse(ECHO, RC_OK, NULL, 0);
  (void) state;
}

/*
 * Check sending messages to the Host works.
 */
void testSendResponse(void **state) {
  USBTransport_Initialize(nullptr);

  // Try with a unconfigured device.
  assert_false(USBTransport_SendResponse(ECHO, RC_OK, NULL, 0));

  // Now configure the device and clear the logging bit.
  USBEventHandler event_handler;
  ConfigureDevice(&event_handler);
  Logging_SetDataPendingFlag(false);

  // Test a message with no data.
  const uint8_t expected_message[] = {
    0x5a, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa5
  };

  expect_any(USB_DEVICE_EndpointWrite, usb_device);
  expect_any(USB_DEVICE_EndpointWrite, transfer);
  expect_value(USB_DEVICE_EndpointWrite, endpoint, 0x81);
  expect_memory(USB_DEVICE_EndpointWrite, data, expected_message,
                arraysize(expected_message));
  expect_value(USB_DEVICE_EndpointWrite, size, 8);
  expect_value(USB_DEVICE_EndpointWrite, flags,
               USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE);
  will_return(USB_DEVICE_EndpointWrite, USB_DEVICE_RESULT_OK);

  assert_true(USBTransport_SendResponse(ECHO, RC_OK, NULL, 0));

  // Try to send a second message while the first is pending.
  assert_false(USBTransport_SendResponse(ECHO, RC_OK, NULL, 0));

  // Now mark the Write as completed
  CompleteWrite(event_handler);

  // Send another message, this time with the logging flag set and some payload
  // data.
  Logging_SetDataPendingFlag(true);
  const uint8_t chunk1[] = {1, 2};
  const uint8_t chunk2[] = {3, 4};

  IOVec iovec[2] = {
      { reinterpret_cast<const void*>(&chunk1), arraysize(chunk1) },
      { reinterpret_cast<const void*>(&chunk2), arraysize(chunk2) }
  };

  const uint8_t expected_message2[] = {
    0x5a, 0x80, 0x00, 0x04, 0x00, 0x00, 0x01,
    1, 2, 3, 4, 0xa5
  };

  expect_any(USB_DEVICE_EndpointWrite, usb_device);
  expect_any(USB_DEVICE_EndpointWrite, transfer);
  expect_value(USB_DEVICE_EndpointWrite, endpoint, 0x81);
  expect_memory(USB_DEVICE_EndpointWrite, data, expected_message2,
                arraysize(expected_message2));
  expect_value(USB_DEVICE_EndpointWrite, size, 12);
  expect_value(USB_DEVICE_EndpointWrite, flags,
               USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE);
  will_return(USB_DEVICE_EndpointWrite, USB_DEVICE_RESULT_OK);

  assert_true(USBTransport_SendResponse(ECHO, RC_OK, iovec, arraysize(iovec)));

  CompleteWrite(event_handler);
  Logging_SetDataPendingFlag(false);

  // Perform a send where USB_DEVICE_EndpointWrite returns an error
  expect_any(USB_DEVICE_EndpointWrite, usb_device);
  expect_any(USB_DEVICE_EndpointWrite, transfer);
  expect_value(USB_DEVICE_EndpointWrite, endpoint, 0x81);
  expect_memory(USB_DEVICE_EndpointWrite, data, expected_message,
                arraysize(expected_message));
  expect_value(USB_DEVICE_EndpointWrite, size, 8);
  expect_value(USB_DEVICE_EndpointWrite, flags,
               USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE);
  will_return(USB_DEVICE_EndpointWrite, USB_DEVICE_RESULT_ERROR);

  assert_false(USBTransport_SendResponse(ECHO, RC_OK, NULL, 0));

  // Send a lot of data, and make sure we set the truncated bit.
  const unsigned int big_payload_size = PAYLOAD_SIZE + 100;
  uint8_t large_payload[big_payload_size];
  memset(large_payload, 0, arraysize(large_payload));
  iovec[0].base = large_payload;
  iovec[0].length = big_payload_size;

  uint8_t expected_message3[7 + PAYLOAD_SIZE + 1];
  memset(expected_message3, 0, arraysize(expected_message3));
  expected_message3[0] = 0x5a;
  expected_message3[1] = 0x80;
  expected_message3[2] = 0x0;
  expected_message3[3] = 0x01;
  expected_message3[4] = 0x02;
  expected_message3[5] = RC_OK;
  expected_message3[6] = 0x04;  // flags, truncated.
  expected_message3[7 + PAYLOAD_SIZE] = 0xa5;

  expect_any(USB_DEVICE_EndpointWrite, usb_device);
  expect_any(USB_DEVICE_EndpointWrite, transfer);
  expect_value(USB_DEVICE_EndpointWrite, endpoint, 0x81);
  expect_memory(USB_DEVICE_EndpointWrite, data, expected_message3,
                arraysize(expected_message3));
  expect_value(USB_DEVICE_EndpointWrite, size, 8 + PAYLOAD_SIZE);
  expect_value(USB_DEVICE_EndpointWrite, flags,
               USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE);
  will_return(USB_DEVICE_EndpointWrite, USB_DEVICE_RESULT_OK);

  assert_true(USBTransport_SendResponse(ECHO, RC_OK, iovec, 1));
  CompleteWrite(event_handler);

  // Now try a test where there is a flag change pending
  Flags_SetTXDrop();
  assert_true(Flags_HasChanged());

  const uint8_t expected_message4[] = {
    0x5a, 0x80, 0x00, 0x00, 0x00, 0x00, 0x02, 0xa5
  };

  expect_any(USB_DEVICE_EndpointWrite, usb_device);
  expect_any(USB_DEVICE_EndpointWrite, transfer);
  expect_value(USB_DEVICE_EndpointWrite, endpoint, 0x81);
  expect_memory(USB_DEVICE_EndpointWrite, data, expected_message4,
                arraysize(expected_message4));
  expect_value(USB_DEVICE_EndpointWrite, size, 8);
  expect_value(USB_DEVICE_EndpointWrite, flags,
               USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE);
  will_return(USB_DEVICE_EndpointWrite, USB_DEVICE_RESULT_OK);

  assert_true(USBTransport_SendResponse(ECHO, RC_OK, NULL, 0));

  (void) state;
}

int main(void) {
  const UnitTest tests[] = {
    unit_test(testUninitializedDevice),
    unit_test(testSendResponse),
  };

  return run_tests(tests);
}
