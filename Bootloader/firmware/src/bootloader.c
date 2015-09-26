/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * bootloader.c
 * Copyright (C) 2015 Simon Newton
 */

#include "bootloader.h"

#if HAVE_CONFIG_H
// We're in the test environment
#include <config.h>
#else
#include <machine/endian.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include "system_config.h"

#include "bootloader_options.h"
#include "crc.h"
#include "dfu_properties.h"
#include "dfu_spec.h"
#include "flash.h"
#include "launcher.h"
#include "macros.h"
#include "peripheral/nvm/plib_nvm.h"
#include "peripheral/ports/plib_ports.h"
#include "reset.h"
#include "usb/usb_device.h"

// The settings for the bootloader
#include "bootloader_settings.h"

/**
 * @brief The size of the words used for flash programming.
 */
static const uint32_t FIRMWARE_HEADER_SIZE = 20u;

/**
 * @brief The version of the firmware header to use.
 */
static const uint32_t FIRMWARE_HEADER_VERSION = 1u;

/**
 * @brief The default value of 4 bytes of erased flash.
 */
static const uint32_t ERASED_FLASH_VALUE = 0xffffffff;

/**
 * @brief The initial CRC value
 */
static const uint32_t INITIAL_CRC = 0xffffffff;

/*
 * @brief The memory addresses associated with a DFU interface.
 *
 * Each interface represents a different region of memory. This allows us to
 * program the firmware independently from the UID.
 *
 * Be careful with the ranges, they need to be a multiple of the page size
 * (4k).
 */
typedef struct {
  uint32_t start_address;  //!< The first address in the region.
  uint32_t end_address;  //!< The last address in the region.
} DFUConfiguration;

DFUConfiguration DFU_CONFIGURATION[2] = {
  // The firmware, 484kB
  {
    .start_address = 0x9d007000,
    .end_address = 0x9d07ffff
  },
  // The page containing the UID, right now we only use 6 bytes, but we allow
  // the full 4kB in case we want to store something else here.
  {
    .start_address = 0x9d006000,
    .end_address = 0x9d006fff
  }
};


/**
 * @brief The top level state machine.
 *
 * This tracks when the USB device is plugged in & configured. Once the USB
 * stack is initialized we remain in BOOTLOADER_STATE_DFU until a USB reset or
 * the USB cable is unplugged.
 */
typedef enum {
  BOOTLOADER_STATE_INIT,  //!< Waiting to initialize USB
  BOOTLOADER_STATE_WAIT_FOR_POWER,  //!< Waiting for power on the USB bus
  BOOTLOADER_STATE_WAIT_FOR_USB_CONFIGURATION,  //!< Waiting to config callback.
  BOOTLOADER_STATE_DFU,  //!< Running the DFU state machine.
} AppState;

/**
 * @brief The various states of a DFU transfer.
 *
 * These are independent from the DFU states, since they handle the
 * manufacturer specific logic like verification of the firmware header.
 */
typedef enum {
  TRANSFER_BEGIN,  //!<  Received intent to transfer, waiting to check header
  TRANSFER_WRITE,  //!< Receiving data chunks
  TRANSFER_LAST_BLOCK_RECEIVED,  //!<  We've received the last data
  TRANSFER_WRITE_COMPLETE,  //!< All data has been written to flash
  TRANSFER_MANIFEST_COMPLETE  //!< manifestation is complete
} TransferState;

typedef struct {
  USB_DEVICE_HANDLE usb_device;  //!< The USB Device layer handle.
  AppState state;
  DFUState dfu_state;  //!< The DFU state machine.
  DFUStatus dfu_status;  //!< The current DFU status code.
  uint32_t expected_crc;  //!< The CRC we expect
  uint32_t crc;  //!< The CRC for data received so far.
  uint8_t active_interface;  //!< The index of the active interface.
} BootloaderData;

static BootloaderData g_bootloader;

/**
 * @brief The state associated with a DFU transfer.
 */
typedef struct {
  TransferState transfer_state;
  uint32_t total_size;  //!< The total size of the transfer, ex. the header.
  uint32_t current_size;  //!< The amount of data received & written so far.
  uint32_t write_address;  //!< The address to write the next block of data to.
  uint16_t next_block;  //!< The expected index of the next block to receive.
  uint16_t block_size;  //!< The length of the data in g_data_buffer.
} TransferData;

static TransferData g_transfer;

/**
 * @brief The buffer that holds the DFU Status response.
 */
static uint8_t g_status_response[GET_STATUS_RESPONSE_SIZE];

/**
 * @brief The buffer into which we receive DFU data.
 *
 * The minimum flash program size is a word (32-bits). I can't see anything in
 * the DFU standard that requires the transfer size to be a multiple of 4. This
 * means we could end up with 1-3 bytes that we can't write immediately.
 *
 * The solution is to make the incoming DFU buffer slightly bigger so we can
 * accomodate this, and write them out once we receive the remaining data.
 */
static uint8_t g_data_buffer[DFU_BLOCK_SIZE + FLASH_WORD_SIZE - 1];

// Helper functions
// ----------------------------------------------------------------------------

static inline bool SwitchPressed() {
  return SWITCH_ACTIVE_HIGH == PLIB_PORTS_PinGet(
      PORTS_ID_0, SWITCH_PORT_CHANNEL, SWITCH_PORT_BIT);
}

static inline uint32_t ExtractUInt32(const uint8_t *ptr) {
  return (ptr[0] << 24) + (ptr[1] << 16) + (ptr[2] << 8) + ptr[3];
}

static inline uint32_t ExtractUInt16(const uint8_t *ptr) {
  return (ptr[0] << 8) + ptr[1];
}

/*
 * @brief Switch to the error state.
 * @param status The DFU status code.
 */
static void SetError(DFUStatus status) {
  g_bootloader.dfu_state = DFU_STATE_ERROR;
  g_bootloader.dfu_status = status;
}

/*
 * @brief Switch to the error state and stall the control pipe.
 * @param status The DFU status code.
 *
 * Stalling the pipe should cause the host to send a DFU_GETSTATUS command.
 */
static void StallAndError(DFUStatus status) {
  SetError(status);
  USB_DEVICE_ControlStatus(g_bootloader.usb_device,
                           USB_DEVICE_CONTROL_STATUS_ERROR);
}

/*
 * @brief Erase the application program flash.
 * @returns true if the flash was erased, false if an erase error occurred.
 */
static bool EraseFlash() {
  const DFUConfiguration *config =
      &DFU_CONFIGURATION[g_bootloader.active_interface];
  unsigned int pages = (
      (config->end_address - config->start_address + 1u) / FLASH_PAGE_SIZE);
  unsigned int i = 0u;
  for (; i < pages; i++) {
    uint32_t address = config->start_address + (i * FLASH_PAGE_SIZE);
    if (!Flash_ErasePage(address)) {
      return false;
    }
  }
  return true;
}

/*
 * @brief Write a word and verify the flash was updated.
 * @returns true if data was written & verified, false if there was an error.
 */
static bool WriteAndVerify(uint32_t address, uint32_t data) {
  // Convert to little endian.
  data = ntohl(data);
  if (!Flash_WriteWord(address, data)) {
    // write failed
    SetError(DFU_STATUS_ERR_PROG);
    return false;
  }

  // Verify the data
  if (Flash_ReadWord(address) != data) {
    SetError(DFU_STATUS_ERR_VERIFY);
    return false;
  }
  return true;
}

/*
 * @brief Write as much of the firmware buffer to flash as we can.
 * @returns true if data was written, false if there was an error.
 *
 * If an error occurs, g_bootloader.dfu_status is set appropriately and
 * g_bootloader.block_size is reset to 0.
 *
 * This may leave up to FLASH_WORD_SIZE - 1 bytes remaining in the data buffer.
 */
static bool ProgramFlash(bool include_all, unsigned int offset) {
  if (offset >= g_transfer.block_size) {
    return true;
  }

  unsigned int i = offset;
  while (i + FLASH_WORD_SIZE <= g_transfer.block_size) {
    uint32_t data = ExtractUInt32(g_data_buffer + i);
    g_bootloader.crc = CalculateCRC(g_bootloader.crc, g_data_buffer + i,
                                    FLASH_WORD_SIZE);

    if (!WriteAndVerify(g_transfer.write_address, data)) {
      g_transfer.block_size = 0u;
      return false;
    }
    i += FLASH_WORD_SIZE;
    g_transfer.write_address += FLASH_WORD_SIZE;
  }

  g_transfer.current_size += (i - offset);

  // Move any remaining bytes to the start of the buffer.
  unsigned int bytes_remaining = 0u;
  while (i != g_transfer.block_size) {
    g_data_buffer[bytes_remaining++] = g_data_buffer[i++];
  }

  if (include_all && bytes_remaining) {
    // pad the remaining bytes with 0xff
    unsigned int i = bytes_remaining;
    for (; i != FLASH_WORD_SIZE; i++) {
      g_data_buffer[i] = 0xff;
    }

    g_bootloader.crc = CalculateCRC(g_bootloader.crc, g_data_buffer,
                                    bytes_remaining);

    uint32_t data = ExtractUInt32(g_data_buffer);
    g_transfer.block_size = 0u;
    return WriteAndVerify(g_transfer.write_address, data);
  }

  g_transfer.block_size = bytes_remaining;
  return true;
}

// DFU Handlers
// ----------------------------------------------------------------------------
static inline void DFUDownload(USB_SETUP_PACKET *packet) {
  if (g_bootloader.dfu_state != DFU_STATE_IDLE &&
      g_bootloader.dfu_state != DFU_STATE_DNLOAD_IDLE) {
    StallAndError(DFU_STATUS_ERR_STALLED_PKT);
    return;
  }

  if (g_bootloader.dfu_state == DFU_STATE_IDLE && packet->wLength == 0u) {
    StallAndError(DFU_STATUS_ERR_STALLED_PKT);
    return;
  }

  if (packet->wLength > DFU_BLOCK_SIZE) {
    StallAndError(DFU_STATUS_ERR_STALLED_PKT);
    return;
  }

  if (g_bootloader.dfu_state == DFU_STATE_IDLE) {
    // First message in a DFU transfer
    g_transfer.transfer_state = TRANSFER_BEGIN;
    g_transfer.total_size = 0u;
    g_transfer.current_size = 0u;
    g_transfer.write_address =
        DFU_CONFIGURATION[g_bootloader.active_interface].start_address;
    g_transfer.next_block = 0u;
    g_transfer.block_size = 0u;
  } else {
    g_transfer.next_block++;
  }

  if (g_transfer.next_block != packet->wValue) {
    StallAndError(DFU_STATUS_ERR_STALLED_PKT);
    return;
  }

  if (packet->wLength) {
    unsigned int offset = g_transfer.block_size;
    g_transfer.block_size += packet->wLength;
    USB_DEVICE_ControlReceive(g_bootloader.usb_device,
                              g_data_buffer + offset,
                              packet->wLength);
  } else {
    // A length of 0 means the transfer is complete.
    if (g_transfer.current_size + g_transfer.block_size !=
        g_transfer.total_size) {
      StallAndError(DFU_STATUS_ERR_NOT_DONE);
    } else {
      g_bootloader.dfu_state = DFU_STATE_MANIFEST_SYNC;
      g_transfer.transfer_state = TRANSFER_LAST_BLOCK_RECEIVED;
      USB_DEVICE_ControlStatus(g_bootloader.usb_device,
                               USB_DEVICE_CONTROL_STATUS_OK);
    }
  }
}

static inline void DFUGetStatus() {
  // Some Get Status messages trigger a state change.
  // The status response always contains the *next* state, so figure that out
  // first.
  if (g_bootloader.dfu_state == DFU_STATE_DNLOAD_SYNC) {
    g_bootloader.dfu_state = DFU_STATE_DNLOAD_IDLE;
  } else if (g_bootloader.dfu_state == DFU_STATE_MANIFEST_SYNC) {
    if (g_transfer.transfer_state == TRANSFER_WRITE_COMPLETE) {
      g_bootloader.dfu_state = DFU_STATE_MANIFEST;
    } else if (g_transfer.transfer_state == TRANSFER_MANIFEST_COMPLETE) {
      g_bootloader.dfu_state = DFU_STATE_IDLE;
    }
  }

  g_status_response[0] = g_bootloader.dfu_status;
  g_status_response[1] = 0u;
  g_status_response[2] = 0u;
  g_status_response[3] = 0u;
  g_status_response[4] = g_bootloader.dfu_state;
  g_status_response[5] = 0u;

  USB_DEVICE_ControlSend(g_bootloader.usb_device, g_status_response,
                         GET_STATUS_RESPONSE_SIZE);
}

static inline void DFUClearStatus() {
  if (g_bootloader.dfu_state == DFU_STATE_ERROR) {
    g_bootloader.dfu_state = DFU_STATE_IDLE;
    g_bootloader.dfu_status = DFU_STATUS_OK;
    USB_DEVICE_ControlStatus(g_bootloader.usb_device,
                             USB_DEVICE_CONTROL_STATUS_OK);
  } else {
    StallAndError(DFU_STATUS_ERR_STALLED_PKT);
  }
}

static inline void DFUGetState() {
  switch (g_bootloader.dfu_state) {
    case APP_STATE_IDLE:
    case APP_STATE_DETACH:
    case DFU_STATE_IDLE:
    case DFU_STATE_DNLOAD_SYNC:
    case DFU_STATE_DNLOAD_IDLE:
    case DFU_STATE_MANIFEST_SYNC:
    case DFU_STATE_UPLOAD_IDLE:
    case DFU_STATE_ERROR:
      USB_DEVICE_ControlSend(g_bootloader.usb_device, &g_bootloader.dfu_state,
                             1);
      break;
    case DFU_STATE_DNBUSY:
    case DFU_STATE_MANIFEST:
    case DFU_STATE_MANIFEST_WAIT_RESET:
    default:
      StallAndError(DFU_STATUS_ERR_STALLED_PKT);
  }
}

static inline void DFUAbort() {
  switch (g_bootloader.dfu_state) {
    case DFU_STATE_IDLE:
    case DFU_STATE_DNLOAD_SYNC:
    case DFU_STATE_DNLOAD_IDLE:
    case DFU_STATE_MANIFEST_SYNC:
    case DFU_STATE_UPLOAD_IDLE:
      g_bootloader.dfu_state = DFU_STATE_IDLE;
      USB_DEVICE_ControlStatus(g_bootloader.usb_device,
                               USB_DEVICE_CONTROL_STATUS_OK);
      break;
    case APP_STATE_IDLE:
    case APP_STATE_DETACH:
    case DFU_STATE_DNBUSY:
    case DFU_STATE_MANIFEST:
    case DFU_STATE_MANIFEST_WAIT_RESET:
    case DFU_STATE_ERROR:
    default:
      StallAndError(DFU_STATUS_ERR_STALLED_PKT);
  }
}

static void HandleDFUEvent(USB_SETUP_PACKET *packet) {
  if (packet->DataDir == USB_SETUP_REQUEST_DIRECTION_DEVICE_TO_HOST) {
    // Device to Host.
    switch (packet->bRequest) {
      case DFU_GETSTATUS:
        DFUGetStatus();
        break;
      case DFU_GETSTATE:
        DFUGetState();
        break;
      default:
        // Unknown command, stall the pipe
        StallAndError(DFU_STATUS_ERR_STALLED_PKT);
    }
    return;
  } else {
    // Host to Device.
    switch (packet->bRequest) {
      case DFU_DNLOAD:
        DFUDownload(packet);
        break;
      case DFU_CLRSTATUS:
        DFUClearStatus();
        break;
      case DFU_ABORT:
        DFUAbort();
        break;
      default:
        // Unknown command, stall the pipe
        StallAndError(DFU_STATUS_ERR_STALLED_PKT);
    }
  }
}

static void DFUTransferComplete() {
  if (g_bootloader.dfu_state != DFU_STATE_IDLE &&
      g_bootloader.dfu_state != DFU_STATE_DNLOAD_IDLE) {
    StallAndError(DFU_STATUS_ERR_STALLED_PKT);
  }

  g_bootloader.dfu_state = DFU_STATE_DNBUSY;
  USB_DEVICE_ControlStatus(g_bootloader.usb_device,
                           USB_DEVICE_CONTROL_STATUS_OK);
}

/*
 * @brief The host aborted a control transfer.
 *
 * This is different from sending a DFU_ABORT command.
 */
static void DFUTransferAborted() {
  StallAndError(DFU_STATUS_ERR_STALLED_PKT);
}

/**
 * @brief Called when USB events occur.
 * @param event The type of event
 * @param event_data Data associated with the event
 * @param context The context (unused).
 *
 * This is called from the main event loop, since we're using polled mode USB.
 */
static void USBEventHandler(USB_DEVICE_EVENT event,
                            void* event_data,
                            UNUSED uintptr_t context) {
  uint8_t *configuration_value;
  USB_SETUP_PACKET *setup_packet;

  switch (event) {
    case USB_DEVICE_EVENT_POWER_DETECTED:
      /* VBUS is detected. Attach the device */
      g_bootloader.state = BOOTLOADER_STATE_WAIT_FOR_USB_CONFIGURATION;
      USB_DEVICE_Attach(g_bootloader.usb_device);
      break;

    case USB_DEVICE_EVENT_POWER_REMOVED:
      /* VBUS is removed. Detach the device */
      g_bootloader.state = BOOTLOADER_STATE_WAIT_FOR_POWER;
      g_bootloader.dfu_state = DFU_STATE_IDLE;
      USB_DEVICE_Detach(g_bootloader.usb_device);
      break;

    case USB_DEVICE_EVENT_RESET:
      if (g_bootloader.state == BOOTLOADER_STATE_DFU) {
        // A reset while configured is a signal to reboot into the application.
        BootloaderOptions_SetBootOption(BOOT_PRIMARY_APPLICATION);
        Reset_SoftReset();
      }
      break;

    case USB_DEVICE_EVENT_CONFIGURED:
      // Check the configuration
      configuration_value = (uint8_t*) event_data;
      if (*configuration_value == 1) {
        g_bootloader.state = BOOTLOADER_STATE_DFU;
      }
      break;

    case USB_DEVICE_EVENT_DECONFIGURED:
      g_bootloader.state = BOOTLOADER_STATE_WAIT_FOR_USB_CONFIGURATION;
      g_bootloader.dfu_state = DFU_STATE_IDLE;
      break;

    case USB_DEVICE_EVENT_SUSPENDED:
      break;

    case USB_DEVICE_EVENT_CONTROL_TRANSFER_SETUP_REQUEST:
      setup_packet = (USB_SETUP_PACKET*) event_data;
      if (setup_packet->RequestType == USB_SETUP_REQUEST_TYPE_CLASS &&
          setup_packet->Recipient == USB_SETUP_REQUEST_RECIPIENT_INTERFACE &&
          setup_packet->wIndex == DFU_MODE_DFU_INTERFACE_INDEX) {
        HandleDFUEvent(setup_packet);
      } else if (setup_packet->bRequest == USB_REQUEST_SET_INTERFACE) {
        if (setup_packet->wValue > DFU_ALT_INTERFACE_UID) {
          USB_DEVICE_ControlStatus(g_bootloader.usb_device,
                                   USB_DEVICE_CONTROL_STATUS_ERROR);
        } else {
          g_bootloader.active_interface = setup_packet->wValue;
          USB_DEVICE_ControlStatus(g_bootloader.usb_device,
                                   USB_DEVICE_CONTROL_STATUS_OK);
        }
      } else if (setup_packet->bRequest == USB_REQUEST_GET_INTERFACE) {
        USB_DEVICE_ControlSend(g_bootloader.usb_device,
                               &g_bootloader.active_interface,
                               sizeof(g_bootloader.active_interface));
      } else {
        // We have received a request that we cannot handle, stall the pipe.
        USB_DEVICE_ControlStatus(g_bootloader.usb_device,
                                 USB_DEVICE_CONTROL_STATUS_ERROR);
      }
      break;

    case USB_DEVICE_EVENT_CONTROL_TRANSFER_DATA_RECEIVED:
      DFUTransferComplete();
      break;
    case USB_DEVICE_EVENT_CONTROL_TRANSFER_DATA_SENT:
      // The Harmony examples show a call to USB_DEVICE_ControlStatus() here,
      // but that doesn't make sense, since for an IN transfer the host side
      // ACKs. Adding the call causes everything to break, so I think the
      // example is wrong.
      break;
    case USB_DEVICE_EVENT_CONTROL_TRANSFER_ABORTED:
      DFUTransferAborted();
      break;
    // These events are not used.
    case USB_DEVICE_EVENT_ENDPOINT_READ_COMPLETE:
    case USB_DEVICE_EVENT_ENDPOINT_WRITE_COMPLETE:
    case USB_DEVICE_EVENT_RESUMED:
    case USB_DEVICE_EVENT_ERROR:
    default:
      break;
  }
}

void ProcessDownload() {
  unsigned int offset = 0u;
  if (g_transfer.transfer_state == TRANSFER_BEGIN) {
    if (g_transfer.block_size >= FIRMWARE_HEADER_SIZE) {
      uint32_t version = ExtractUInt32(g_data_buffer);
      if (version != FIRMWARE_HEADER_VERSION) {
        SetError(DFU_STATUS_ERR_TARGET);
        return;
      }
      uint32_t total_size = ExtractUInt32(g_data_buffer + sizeof(uint32_t));
      const DFUConfiguration *config =
          &DFU_CONFIGURATION[g_bootloader.active_interface];
      if (total_size > config->end_address - config->start_address + 1u) {
        SetError(DFU_STATUS_ERR_ADDRESS);
        return;
      }
      g_transfer.total_size = total_size;

      uint16_t model_id = ExtractUInt16(g_data_buffer + 2 * sizeof(uint32_t));
      if (model_id != MODEL_UNDEFINED && model_id != HARDWARE_MODEL) {
        // Firmware model mismatch
        SetError(DFU_STATUS_ERR_TARGET);
        return;
      }

      g_bootloader.expected_crc = ExtractUInt32(
          g_data_buffer + 3 * sizeof(uint32_t));
      g_bootloader.crc = INITIAL_CRC;

      // At this point we've checked as much as we can, go ahead and erase the
      // flash.
      if (!EraseFlash()) {
        SetError(DFU_STATUS_ERR_ERASE);
        return;
      }

      offset = FIRMWARE_HEADER_SIZE;
      g_transfer.transfer_state = TRANSFER_WRITE;
    } else {
      // Wait for more data
      g_bootloader.dfu_state = DFU_STATE_DNLOAD_SYNC;
      return;
    }
  }

  if (ProgramFlash(false, offset)) {
    g_bootloader.dfu_state = DFU_STATE_DNLOAD_SYNC;
  }
}

void Bootloader_Initialize() {
  PLIB_PORTS_PinDirectionInputSet(PORTS_ID_0, SWITCH_PORT_CHANNEL,
                                  SWITCH_PORT_BIT);

  bool run_bootloader = (
    BootloaderOptions_GetBootOption() == BOOT_BOOTLOADER ||
    SwitchPressed() ||
    Flash_ReadWord(APPLICATION_RESET_ADDRESS) == ERASED_FLASH_VALUE);

  if (!run_bootloader) {
    Launcher_RunApp(APPLICATION_RESET_ADDRESS);
  }

  g_bootloader.usb_device = USB_DEVICE_HANDLE_INVALID;
  g_bootloader.state = BOOTLOADER_STATE_INIT;
  g_bootloader.dfu_state = DFU_STATE_IDLE;
  g_bootloader.dfu_status = DFU_STATUS_OK;
  g_bootloader.active_interface = DFU_ALT_INTERFACE_FIRMWARE;

  unsigned int i = 0u;
  for (; i < BOOTLOADER_LEDS.count; i++) {
    PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0,
                                     BOOTLOADER_LEDS.leds[i].port_channel,
                                     BOOTLOADER_LEDS.leds[i].port_bit);
    PLIB_PORTS_PinClear(PORTS_ID_0,
                        BOOTLOADER_LEDS.leds[i].port_channel,
                        BOOTLOADER_LEDS.leds[i].port_bit);
  }
}

void Bootloader_Tasks() {
  // Flash the LED to indicate we're in bootloader mode.
  static unsigned int count = 0u;
  if (++count > 50000u) {
    unsigned int i = 0u;
    for (; i < BOOTLOADER_LEDS.count; i++) {
      PLIB_PORTS_PinToggle(PORTS_ID_0,
                           BOOTLOADER_LEDS.leds[i].port_channel,
                           BOOTLOADER_LEDS.leds[i].port_bit);
    }
    count = 0;
  }

  switch (g_bootloader.state) {
    case BOOTLOADER_STATE_INIT:
      g_bootloader.usb_device = USB_DEVICE_Open(USB_DEVICE_INDEX_0,
                                                DRV_IO_INTENT_READWRITE);
      if (g_bootloader.usb_device != USB_DEVICE_HANDLE_INVALID) {
        // Register a callback with the device layer to receive USB events.
        g_bootloader.state = BOOTLOADER_STATE_WAIT_FOR_POWER;
        USB_DEVICE_EventHandlerSet(g_bootloader.usb_device, USBEventHandler,
                                   0u);
      }
      break;
    case BOOTLOADER_STATE_WAIT_FOR_POWER:
    case BOOTLOADER_STATE_WAIT_FOR_USB_CONFIGURATION:
      // no op, waiting for USBEventHandler to change state.
      break;

    case BOOTLOADER_STATE_DFU:
      if (g_bootloader.dfu_state == DFU_STATE_DNBUSY) {
        ProcessDownload();
      } else if (g_bootloader.dfu_state == DFU_STATE_MANIFEST_SYNC &&
                 g_transfer.transfer_state == TRANSFER_LAST_BLOCK_RECEIVED) {
        // The firmware size may not be a multiple of 4, so write any remaining
        // bytes now.
        if (ProgramFlash(true, 0u)) {
          g_transfer.transfer_state = TRANSFER_WRITE_COMPLETE;
        }
        // Check the CRC matches now
        if (g_bootloader.expected_crc &&
            g_bootloader.crc != g_bootloader.expected_crc) {
          SetError(DFU_STATUS_ERR_FIRMWARE);
        }
      } else if (g_bootloader.dfu_state == DFU_STATE_MANIFEST) {
        // Nothing to do during the manifest stage, reset the variables
        g_transfer.transfer_state = TRANSFER_MANIFEST_COMPLETE;
        g_bootloader.dfu_state = DFU_STATE_MANIFEST_SYNC;
      }
      break;
    default:
      break;
  }
}

bool Bootloader_USBActive() {
  return g_bootloader.state == BOOTLOADER_STATE_DFU;
}

DFUState Bootloader_GetState() {
  return g_bootloader.dfu_state;
}

DFUStatus Bootloader_GetStatus() {
  return g_bootloader.dfu_status;
}
