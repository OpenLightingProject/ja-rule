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
 * BootloaderTestHelper.h
 * Helpers for the DFU bootloader tests.
 * Copyright (C) 2015 Simon Newton
 */

#ifndef TESTS_TESTS_BOOTLOADERTESTHELPER_H_
#define TESTS_TESTS_BOOTLOADERTESTHELPER_H_

#include "dfu_constants.h"
#include "usb/usb_device.h"
#include "usb_device_mock.h"

static const uint32_t FLASH_BASE_ADDRESS = 0x9d000000;
static const uint32_t FLASH_SIZE = 0x7ffff;
static const uint32_t UID_BASE_ADDRESS = 0x9d006000;
static const uint32_t UID_END_ADDRESS = 0x9d006fff;
static const uint32_t FW_BASE_ADDRESS = 0x9d007000;

::std::ostream& operator<<(::std::ostream& os, const DFUState& state);
::std::ostream& operator<<(::std::ostream& os, const DFUStatus& status);

/*
 * Represents the host side, this communicates with the USB device.
 */
class USBHost {
 public:
  /*
   * Allow either a stall or an 'ok'.
   */
  typedef enum {
    OUTCOME_STALL,
    OUTCOME_OK,
  } Outcome;

  /*
   * The DNLOAD command can go a couple of ways. This controls what sequence of
   * events we expect.
   */
  typedef enum {
    DOWNLOAD_OUTCOME_STALL,
    DOWNLOAD_OUTCOME_OK,
    DOWNLOAD_OUTCOME_RECEIVE
  } DownloadOutcome;

  explicit USBHost(MockUSBDevice *mock_usb);

  void InitDevice();
  void SendUSBReset();
  void SendDeconfigure();
  void SendPowerLoss();
  void SetAlternateInterface(uint16_t alt_setting);
  void GetDFUState(Outcome outcome, uint8_t *state);
  void GetDFUStatus(DFUState *state, DFUStatus *status);
  void DFUDownload(DownloadOutcome outcome, uint16_t block_index,
                   const uint8_t *data, uint16_t size);
  void DFUDownloadAndAbort(uint16_t block_index, uint16_t size);
  void DFUAbort(Outcome outcome);
  void DFUClearStatus();

  void SetupRequest(void *data, unsigned int size);

 private:
  MockUSBDevice *m_usb_mock;
  USB_DEVICE_HANDLE m_usb_handle;
  USB_DEVICE_EVENT_HANDLER m_event_handler;

  static const uint16_t INTERFACE = 0u;
};

#endif  // TESTS_TESTS_BOOTLOADERTESTHELPER_H_
