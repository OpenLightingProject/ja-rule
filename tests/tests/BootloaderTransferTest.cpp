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
 * BootloaderTransferTest.cpp
 * Tests for the Bootloader code that perform a series of image transfers.
 * Copyright (C) 2015 Simon Newton
 */

#include <gtest/gtest.h>

#include <algorithm>

#include "Array.h"
#include "FlashMock.h"
#include "Matchers.h"
#include "bootloader.h"
#include "macros.h"
#include "dfu_properties.h"
#include "dfu_spec.h"
#include "usb/usb_device.h"
#include "usb_device_mock.h"
#include "plib_ports_mock.h"
#include "BootloaderTestHelper.h"

using std::min;
using testing::DoAll;
using testing::NotNull;
using testing::Mock;
using testing::Return;
using testing::SaveArg;
using testing::WithArgs;
using testing::_;

/*
 * Represents a flash chip. This will store data written to flash in memory so
 * we can confirm the data is correct.
 */
class FlashChip : public FlashInterface {
 public:
  class Options {
   public:
    Options(uint32_t address, uint32_t total_size)
        : address(address),
          total_size(total_size),
          page_size(0x1000),
          fail_erase(false),
          fail_write(false),
          corrupt_data(false) {
    }

    uint32_t address;
    uint32_t total_size;
    uint32_t page_size;
    bool fail_erase;
    bool fail_write;
    bool corrupt_data;
  };

  explicit FlashChip(const Options &options)
      : m_page_size(options.page_size),
        m_lower(options.address),
        m_upper(options.address + options.total_size),
        m_fail_erase(options.fail_erase),
        m_fail_write(options.fail_write),
        m_corrupt_data(options.corrupt_data),
        m_was_erased(false) {
    m_data = new uint8_t[options.total_size];
    memset(m_data, 0, options.total_size);
  }

  ~FlashChip() {
    delete[] m_data;
  }

  void SetFailOnErase(bool enabled) {
    m_fail_erase = enabled;
  }

  void SetFailOnWrite(bool enabled) {
    m_fail_write = enabled;
  }

  void SetCorruptData(bool enabled) {
    m_corrupt_data = enabled;
  }

  bool WasErased() const {
    return m_was_erased;
  }

  bool ErasePage(uint32_t address) {
    if (address < m_lower || address + m_page_size > m_upper + 1 ||
        m_fail_erase) {
      return false;
    }
    memset(m_data + NormalizeAddress(address), 0xff, m_page_size);
    m_was_erased = true;
    return true;
  }

  bool WriteWord(uint32_t address, uint32_t data) {
    if (address < m_lower || address + sizeof(uint32_t) > m_upper ||
        m_fail_write) {
      return false;
    }
    memcpy(m_data + NormalizeAddress(address), &data, sizeof(uint32_t));
    return true;
  }

  uint32_t ReadWord(uint32_t address) {
    if (address < m_lower || address + sizeof(uint32_t) > m_upper) {
      return 0;
    }
    uint32_t value;
    memcpy(&value, m_data + NormalizeAddress(address), sizeof(value));
    if (m_corrupt_data) {
      value++;
    }
    return value;
  }

  bool ReadData(uint32_t address, uint8_t *data, unsigned int size) {
    if (address < m_lower || address + size > m_upper) {
      return false;
    }
    memcpy(data, m_data + NormalizeAddress(address), size);
    return true;
  }

 private:
  const uint32_t m_page_size;
  const uint32_t m_lower;
  const uint32_t m_upper;
  bool m_fail_erase;
  bool m_fail_write;
  bool m_corrupt_data;
  bool m_was_erased;
  uint8_t *m_data;

  uint32_t NormalizeAddress(uint32_t user_address) {
    return user_address - m_lower;
  }
};

class DFUClient {
 public:
  struct Options {
    Options()
        : block_size(DFU_BLOCK_SIZE),
          out_of_order_blocks(false),
          last_block_stalls(false),
          abort_dfu_transfer(false),
          abort_control_transfer(false),
          leave_in_manifest_state(false) {
    }

    uint16_t block_size;
    bool out_of_order_blocks;
    bool last_block_stalls;
    bool abort_dfu_transfer;
    bool abort_control_transfer;
    bool leave_in_manifest_state;
  };

  DFUClient(USBHost *host, const uint8_t *data, unsigned int size)
      : m_host(host),
        m_data(data),
        m_size(size) {
  }

  /*
   * Try to perform a download & manifest sequence.
   *
   * This tries to act like a DFU Client. If it hits an unknown code path,
   * it'll log an error and return false.
   */
  bool Download(const Options &options) {
    RunDownload(options);
    return !::testing::Test::HasNonfatalFailure();
  }

 private:
  /*
   * Download and check the status of a block.
   */
  DFUState DownloadBlock(USBHost::DownloadOutcome outcome,
                         uint16_t block_index,
                         const uint8_t *data,
                         unsigned int length) {
    m_host->DFUDownload(outcome, block_index, data, length);
    if (::testing::Test::HasNonfatalFailure()) {
      // We can return anything here, the caller should check for an error
      return DFU_STATE_ERROR;
    }
    Bootloader_Tasks();
    Bootloader_Tasks();

    DFUState state;
    DFUStatus status;
    m_host->GetDFUStatus(&state, &status);
    return state;
  }

  void RunDownload(const Options &options) {
    DFUState state;
    unsigned int offset = 0;
    uint16_t block_index = 0;

    while (offset < m_size) {
      unsigned int len = min(static_cast<unsigned int>(options.block_size),
                             m_size - offset);
      USBHost::DownloadOutcome outcome = USBHost::DOWNLOAD_OUTCOME_RECEIVE;
      if (block_index && options.out_of_order_blocks) {
        block_index++;
        outcome = USBHost::DOWNLOAD_OUTCOME_STALL;
      }
      if (options.block_size > DFU_BLOCK_SIZE) {
        outcome = USBHost::DOWNLOAD_OUTCOME_STALL;
      }

      if (options.abort_control_transfer) {
        m_host->DFUDownloadAndAbort(block_index++, len);
        return;
      }

      state = DownloadBlock(outcome, block_index++, m_data + offset, len);
      if (::testing::Test::HasNonfatalFailure() ||
          state != DFU_STATE_DNLOAD_IDLE) {
        return;
      }

      if (options.abort_dfu_transfer) {
        m_host->DFUAbort(USBHost::OUTCOME_OK);
        return;
      }

      offset += len;
    }

    // Now send a final DNLOAD message with length 0
    USBHost::DownloadOutcome outcome = options.last_block_stalls ?
        USBHost::DOWNLOAD_OUTCOME_STALL : USBHost::DOWNLOAD_OUTCOME_OK;
    state = DownloadBlock(outcome, block_index++, nullptr, 0);
    if (::testing::Test::HasNonfatalFailure() || state != DFU_STATE_MANIFEST) {
      return;
    }

    if (!options.leave_in_manifest_state) {
      Bootloader_Tasks();

      DFUStatus status;
      m_host->GetDFUStatus(&state, &status);
    }
  }

  USBHost *m_host;
  const uint8_t * m_data;
  const unsigned int m_size;
};

class TransferTest : public testing::Test {
 public:
  TransferTest()
      : m_host(&m_usb_mock),
        m_flash(FlashChip::Options(FLASH_BASE_ADDRESS, FLASH_SIZE)) {
  }

  void SetUp() {
    USBDevice_SetMock(&m_usb_mock);
    Flash_SetMock(&m_flash);
    PLIB_PORTS_SetMock(&m_ports);

    // This is inverted for some odd reason
    ON_CALL(m_ports, PinGet(PORTS_ID_0, PORT_CHANNEL_D, PORTS_BIT_POS_7))
        .WillByDefault(Return(false));

    m_host.InitDevice();
    m_host.SetAlternateInterface(1);
  }

  void TearDown() {
    USBDevice_SetMock(nullptr);
    Flash_SetMock(nullptr);
    PLIB_PORTS_SetMock(nullptr);
  }

 protected:
  testing::StrictMock<MockUSBDevice> m_usb_mock;
  USBHost m_host;
  FlashChip m_flash;
  testing::NiceMock<MockPeripheralPorts> m_ports;

  static const uint8_t FW_IMAGE[];
  static const uint8_t UID_IMAGE[];
  enum { IMAGE_HEADER_SIZE = 20 };
};

const uint8_t TransferTest::FW_IMAGE[] = {
  0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x10,
  0x00, 0x03, 0x00, 0x00, 0x6a, 0x51, 0xa0, 0xa2,
  0x00, 0x00, 0x00, 0x00,
  1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
};

const uint8_t TransferTest::UID_IMAGE[] = {
  0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06,
  0x00, 0x00, 0x00, 0x00, 0x6a, 0x51, 0xa0, 0xa2,
  0x00, 0x00, 0x00, 0x00, 0x7a, 0x70, 0x00, 0x00, 0x00, 0x01
};

TEST_F(TransferTest, simpleFWTransfer) {
  m_host.SetAlternateInterface(0);

  DFUClient client(&m_host, FW_IMAGE, arraysize(FW_IMAGE));
  ASSERT_TRUE(client.Download(DFUClient::Options()));
  EXPECT_EQ(DFU_STATE_IDLE, Bootloader_GetState());
  EXPECT_EQ(DFU_STATUS_OK, Bootloader_GetStatus());

  uint8_t flash_data[arraysize(FW_IMAGE) - IMAGE_HEADER_SIZE];
  m_flash.ReadData(FW_BASE_ADDRESS, flash_data, arraysize(flash_data));
  EXPECT_THAT(ArrayTuple(flash_data, arraysize(flash_data)),
              DataIs(FW_IMAGE + IMAGE_HEADER_SIZE, arraysize(flash_data)));
}

TEST_F(TransferTest, incorrectHardwareModel) {
  m_host.SetAlternateInterface(0);

  uint8_t fw_image[arraysize(FW_IMAGE)];
  memcpy(fw_image, FW_IMAGE, arraysize(FW_IMAGE));
  fw_image[9] = 2;  // Not the Ethernet SK II

  DFUClient client(&m_host, fw_image, arraysize(fw_image));
  ASSERT_TRUE(client.Download(DFUClient::Options()));
  EXPECT_EQ(DFU_STATE_ERROR, Bootloader_GetState());
  EXPECT_EQ(DFU_STATUS_ERR_TARGET, Bootloader_GetStatus());
  EXPECT_FALSE(m_flash.WasErased());
}

TEST_F(TransferTest, simpleUIDTransfer) {
  DFUClient client(&m_host, UID_IMAGE, arraysize(UID_IMAGE));
  ASSERT_TRUE(client.Download(DFUClient::Options()));
  EXPECT_EQ(DFU_STATE_IDLE, Bootloader_GetState());
  EXPECT_EQ(DFU_STATUS_OK, Bootloader_GetStatus());

  uint8_t flash_data[arraysize(UID_IMAGE) - IMAGE_HEADER_SIZE];
  m_flash.ReadData(UID_BASE_ADDRESS, flash_data, arraysize(flash_data));
  EXPECT_THAT(ArrayTuple(flash_data, arraysize(flash_data)),
              DataIs(UID_IMAGE + IMAGE_HEADER_SIZE, arraysize(flash_data)));
}

TEST_F(TransferTest, oddSizeBlockTransfer) {
  DFUClient::Options options;
  options.block_size = 7;
  DFUClient client(&m_host, UID_IMAGE, arraysize(UID_IMAGE));
  ASSERT_TRUE(client.Download(options));

  EXPECT_EQ(DFU_STATE_IDLE, Bootloader_GetState());
  EXPECT_EQ(DFU_STATUS_OK, Bootloader_GetStatus());

  uint8_t flash_data[6];
  m_flash.ReadData(UID_BASE_ADDRESS, flash_data, arraysize(flash_data));
  EXPECT_THAT(ArrayTuple(flash_data, arraysize(flash_data)),
              DataIs(UID_IMAGE + IMAGE_HEADER_SIZE, arraysize(flash_data)));
}

TEST_F(TransferTest, flashEraseError) {
  m_flash.SetFailOnErase(true);

  DFUClient client(&m_host, UID_IMAGE, arraysize(UID_IMAGE));
  ASSERT_TRUE(client.Download(DFUClient::Options()));
  EXPECT_EQ(DFU_STATE_ERROR, Bootloader_GetState());
  EXPECT_EQ(DFU_STATUS_ERR_ERASE, Bootloader_GetStatus());
}

TEST_F(TransferTest, flashWriteError) {
  m_flash.SetFailOnWrite(true);

  DFUClient client(&m_host, UID_IMAGE, arraysize(UID_IMAGE));
  ASSERT_TRUE(client.Download(DFUClient::Options()));
  EXPECT_EQ(DFU_STATE_ERROR, Bootloader_GetState());
  EXPECT_EQ(DFU_STATUS_ERR_PROG, Bootloader_GetStatus());
}

TEST_F(TransferTest, flashCorruptData) {
  m_flash.SetCorruptData(true);

  DFUClient client(&m_host, UID_IMAGE, arraysize(UID_IMAGE));
  ASSERT_TRUE(client.Download(DFUClient::Options()));
  EXPECT_EQ(DFU_STATE_ERROR, Bootloader_GetState());
  EXPECT_EQ(DFU_STATUS_ERR_VERIFY, Bootloader_GetStatus());
}

TEST_F(TransferTest, missingBlock) {
  DFUClient::Options options;
  options.block_size = 12;  // send multiple blocks to trigger the error
  options.out_of_order_blocks = true;
  DFUClient client(&m_host, UID_IMAGE, arraysize(UID_IMAGE));

  ASSERT_TRUE(client.Download(options));
  EXPECT_EQ(DFU_STATE_ERROR, Bootloader_GetState());
  EXPECT_EQ(DFU_STATUS_ERR_STALLED_PKT, Bootloader_GetStatus());
}

TEST_F(TransferTest, incompleteImage) {
  DFUClient::Options options;
  options.last_block_stalls = true;
  DFUClient client(&m_host, UID_IMAGE, arraysize(UID_IMAGE) - 1);
  ASSERT_TRUE(client.Download(options));
  EXPECT_EQ(DFU_STATE_ERROR, Bootloader_GetState());
  EXPECT_EQ(DFU_STATUS_ERR_NOT_DONE, Bootloader_GetStatus());
}

TEST_F(TransferTest, oversizedImage) {
  const unsigned int image_size = 1 + UID_END_ADDRESS - UID_BASE_ADDRESS;
  uint8_t *uid_image = new uint8_t[image_size];
  memset(uid_image, 0x00, image_size);
  uid_image[3] = 1;
  uint32_t *firmware_size = reinterpret_cast<uint32_t*>(&uid_image[4]);
  *firmware_size = image_size;

  DFUClient client(&m_host, uid_image, image_size);
  ASSERT_TRUE(client.Download(DFUClient::Options()));
  EXPECT_EQ(DFU_STATE_ERROR, Bootloader_GetState());
  EXPECT_EQ(DFU_STATUS_ERR_ADDRESS, Bootloader_GetStatus());
  EXPECT_FALSE(m_flash.WasErased());

  delete[] uid_image;
}

TEST_F(TransferTest, wrongVersion) {
  uint8_t uid_image[arraysize(UID_IMAGE)];
  uint32_t *version = reinterpret_cast<uint32_t*>(uid_image);
  *version = 2;

  DFUClient client(&m_host, uid_image, arraysize(UID_IMAGE));
  ASSERT_TRUE(client.Download(DFUClient::Options()));
  EXPECT_EQ(DFU_STATE_ERROR, Bootloader_GetState());
  EXPECT_EQ(DFU_STATUS_ERR_TARGET, Bootloader_GetStatus());
  EXPECT_FALSE(m_flash.WasErased());
}

TEST_F(TransferTest, abortTransfer) {
  DFUClient::Options options;
  options.block_size = 7;
  options.abort_dfu_transfer = true;
  DFUClient client(&m_host, UID_IMAGE, arraysize(UID_IMAGE));
  ASSERT_TRUE(client.Download(options));

  EXPECT_EQ(DFU_STATE_IDLE, Bootloader_GetState());
  EXPECT_EQ(DFU_STATUS_OK, Bootloader_GetStatus());
  EXPECT_FALSE(m_flash.WasErased());
}

TEST_F(TransferTest, zeroLengthDownload) {
  DFUClient client(&m_host, UID_IMAGE, 0);

  DFUClient::Options options;
  options.last_block_stalls = true;
  ASSERT_TRUE(client.Download(options));

  EXPECT_EQ(DFU_STATE_ERROR, Bootloader_GetState());
  EXPECT_EQ(DFU_STATUS_ERR_STALLED_PKT, Bootloader_GetStatus());
  EXPECT_FALSE(m_flash.WasErased());
}

TEST_F(TransferTest, largeBlockSize) {
  unsigned int image_size = UID_END_ADDRESS - UID_BASE_ADDRESS;
  uint8_t *uid_image = new uint8_t[image_size];
  memset(uid_image, 0x00, image_size);
  uid_image[3] = 1;
  uint32_t *firmware_size = reinterpret_cast<uint32_t*>(&uid_image[4]);
  *firmware_size = image_size;

  DFUClient client(&m_host, uid_image, image_size);

  DFUClient::Options options;
  options.block_size = DFU_BLOCK_SIZE + 1;
  ASSERT_TRUE(client.Download(options));

  EXPECT_EQ(DFU_STATE_ERROR, Bootloader_GetState());
  EXPECT_EQ(DFU_STATUS_ERR_STALLED_PKT, Bootloader_GetStatus());
  EXPECT_FALSE(m_flash.WasErased());

  delete[] uid_image;
}

TEST_F(TransferTest, abortControlTransfer) {
  DFUClient::Options options;
  options.abort_control_transfer = true;
  DFUClient client(&m_host, UID_IMAGE, arraysize(UID_IMAGE));
  ASSERT_TRUE(client.Download(options));
  EXPECT_EQ(DFU_STATE_ERROR, Bootloader_GetState());
  EXPECT_EQ(DFU_STATUS_ERR_STALLED_PKT, Bootloader_GetStatus());
  EXPECT_FALSE(m_flash.WasErased());
}

TEST_F(TransferTest, failAndRetry) {
  // The first attempt fails due to a flash erase error.
  m_flash.SetFailOnErase(true);

  DFUClient client(&m_host, UID_IMAGE, arraysize(UID_IMAGE));
  ASSERT_TRUE(client.Download(DFUClient::Options()));
  EXPECT_EQ(DFU_STATE_ERROR, Bootloader_GetState());
  EXPECT_EQ(DFU_STATUS_ERR_ERASE, Bootloader_GetStatus());

  // Clear the status
  m_host.DFUClearStatus();
  EXPECT_EQ(DFU_STATE_IDLE, Bootloader_GetState());
  EXPECT_EQ(DFU_STATUS_OK, Bootloader_GetStatus());

  // Now try again
  m_flash.SetFailOnErase(false);
  ASSERT_TRUE(client.Download(DFUClient::Options()));
  EXPECT_EQ(DFU_STATE_IDLE, Bootloader_GetState());
  EXPECT_EQ(DFU_STATUS_OK, Bootloader_GetStatus());
}

TEST_F(TransferTest, manifestStall) {
  DFUClient client(&m_host, UID_IMAGE, arraysize(UID_IMAGE));
  DFUClient::Options options;
  options.leave_in_manifest_state = true;
  ASSERT_TRUE(client.Download(options));
  EXPECT_EQ(DFU_STATE_MANIFEST, Bootloader_GetState());
  EXPECT_EQ(DFU_STATUS_OK, Bootloader_GetStatus());

  uint8_t state;
  m_host.GetDFUState(USBHost::OUTCOME_STALL, &state);
}

TEST_F(TransferTest, abortDuringManifest) {
  DFUClient client(&m_host, UID_IMAGE, arraysize(UID_IMAGE));
  DFUClient::Options options;
  options.leave_in_manifest_state = true;
  ASSERT_TRUE(client.Download(options));
  EXPECT_EQ(DFU_STATE_MANIFEST, Bootloader_GetState());
  EXPECT_EQ(DFU_STATUS_OK, Bootloader_GetStatus());

  m_host.DFUAbort(USBHost::OUTCOME_STALL);
}
