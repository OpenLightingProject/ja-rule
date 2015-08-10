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
 * app.c
 * Copyright (C) 2015 Simon Newton
 */

#include "app.h"
#include "constants.h"
#include "dfu_constants.h"
#include "flash.h"
#include "macros.h"
#include "peripheral/nvm/plib_nvm.h"

#include "system_config.h"

/*
 * @brief The base address to write the firmware to.
 */
enum { APP_BASE_ADDRESS = 0x9d006000 };

/**
 * @brief The final address of the user firmware.
 */
enum { APP_END_ADDRESS = 0x9d07ffff };

/*
 * @brief The reset address of the application.
 */
enum { APP_RESET_ADDRESS = 0x9d007000 };

/*
 * @brief The size of a flash page
 */
enum { FLASH_PAGE_SIZE = 0x1000 };

/**
 * @brief The size of the words used for flash programming.
 */
enum { FLASH_WORD_SIZE = 4 };

typedef enum {
  APP_STATE_INIT,
  APP_STATE_WAIT_FOR_USB_CONFIGURATION,
  APP_STATE_DFU,
  APP_STATE_BOOT
} AppState;

typedef struct {
  USB_DEVICE_HANDLE usb_device;  //!< The USB Device layer handle.
  AppState state;
  DFUState dfu_state;
  DFUStatus dfu_status;  //!< The current DFU state.
  void *write_address;  //!< The address to write the chunk of data to.
  uint16_t next_block;  //!< The expected index of the next block received.
  uint16_t data_length;  //!< The length of the data in DATA_BUFFER.
  bool is_configured;  //!< Keep track of whether the device is configured.
  bool erase_flash;  //!< True if we should start a flash erase cycle
  bool has_new_firmware;  //!< True if there is new firmware ready.
} AppData;

static AppData g_app;

/**
 * @brief The buffer that holds the DFU Status response.
 */
static uint8_t g_status_response[GET_STATUS_RESPONSE_SIZE];

/*
 * @brief The buffer into which we receive DFU data.
 *
 * The minimum flash program size is a word (32-bits). I can't see anything in
 * the DFU standard that requires the transfer size to be a multiple of 4. This
 * means we could end up with 1-3 bytes that we can't write immediately.
 *
 * The solution is to make the incoming DFU buffer slightly bigger so we can
 * accomodate this, and write them out once we receive the remaining data.
 */
static uint8_t DATA_BUFFER[DFU_BLOCK_SIZE + FLASH_WORD_SIZE - 1];

// Helper functions
// ----------------------------------------------------------------------------
static inline uint32_t ExtractUInt32(const uint8_t *ptr) {
  return (ptr[0] << 24) + (ptr[1] << 16) + (ptr[2] << 8) + ptr[3];
}

/*
 * @brief Switch to the error state and stall the control pipe.
 * @param status The DFU status code.
 *
 * Stalling the pipe should cause the host to send a DFU_GETSTATUS command.
 */
static void StallAndError(DFUStatus status) {
  g_app.dfu_state = DFU_STATE_ERROR;
  g_app.dfu_status = status;
  USB_DEVICE_ControlStatus(g_app.usb_device, USB_DEVICE_CONTROL_STATUS_ERROR);
}

/*
 * @brief Erase the application program flash.
 * @returns true if the flash was erased, false if an erase error occurred.
 */
static bool EraseFlash() {
  unsigned int pages = (
      (APP_END_ADDRESS - APP_BASE_ADDRESS + 1u) / FLASH_PAGE_SIZE);
  unsigned int i = 0u;
  for (; i < pages; i++) {
    void *address = (void*) APP_BASE_ADDRESS + (i * FLASH_PAGE_SIZE);
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
static bool WriteAndVerify(void *address, uint32_t data) {
  if (!Flash_WriteWord(address, data)) {
    // write failed
    g_app.dfu_status = DFU_STATUS_ERR_PROG;
    return false;
  }

  // Verify the data
  if (PLIB_NVM_FlashRead(NVM_ID_0, address) != data) {
    g_app.dfu_status = DFU_STATUS_ERR_VERIFY;
    return false;
  }
  return true;
}


/*
 * @brief Write as much of the firmware buffer to flash as we can.
 * @returns true if data was written, false if there was an error.
 *
 * If an error occurs, g_app.dfu_status is set appropriately and
 * g_app.data_length is reset to 0.
 *
 * This may leave up to FLASH_WORD_SIZE - 1 bytes remaining in the data buffer.
 */
static bool ProgramFlash(bool include_all) {
  unsigned int i = 0u;
  while (i + FLASH_WORD_SIZE <= g_app.data_length) {
    uint32_t data = ExtractUInt32(DATA_BUFFER + i);

    if (!WriteAndVerify(g_app.write_address, data)) {
      g_app.data_length = 0u;
      return false;
    }
    i += FLASH_WORD_SIZE;
    g_app.write_address += FLASH_WORD_SIZE;
  }

  // Move any remaining bytes to the start of the buffer.
  unsigned int bytes_remaining = 0u;
  while (i != g_app.data_length) {
    DATA_BUFFER[bytes_remaining++] = DATA_BUFFER[i++];
  }

  if (include_all && bytes_remaining) {
    // pad the remaining bytes with 0xff
    unsigned int i = bytes_remaining;
    for (; i != FLASH_WORD_SIZE; i++) {
      DATA_BUFFER[i] = 0xff;
    }

    uint32_t data = ExtractUInt32(DATA_BUFFER);
    g_app.data_length = 0u;
    return WriteAndVerify(g_app.write_address, data);
  }

  g_app.data_length = bytes_remaining;
  return true;
}

// DFU Handlers
// ----------------------------------------------------------------------------
static inline void DFUDownload(USB_SETUP_PACKET *packet) {
  if (g_app.dfu_state != DFU_STATE_IDLE &&
      g_app.dfu_state != DFU_STATE_DNLOAD_IDLE) {
    StallAndError(DFU_STATUS_ERR_STALLED_PKT);
    return;
  }

  if (g_app.dfu_state == DFU_STATE_IDLE && packet->wLength == 0u) {
    StallAndError(DFU_STATUS_ERR_STALLED_PKT);
    return;
  }

  if (packet->wLength > DFU_BLOCK_SIZE) {
    StallAndError(DFU_STATUS_ERR_STALLED_PKT);
    return;
  }

  if (g_app.dfu_state == DFU_STATE_IDLE) {
    g_app.next_block = 0u;
    g_app.write_address = (void*) APP_BASE_ADDRESS;
  } else {
    g_app.next_block++;
  }

  if (g_app.next_block != packet->wValue) {
    StallAndError(DFU_STATUS_ERR_STALLED_PKT);
  }

  if (packet->wLength) {
    g_app.data_length += packet->wLength;
    USB_DEVICE_ControlReceive(g_app.usb_device, DATA_BUFFER, packet->wLength);
  } else {
    g_app.dfu_state = DFU_STATE_MANIFEST_SYNC;
    g_app.has_new_firmware = true;
    USB_DEVICE_ControlStatus(g_app.usb_device, USB_DEVICE_CONTROL_STATUS_OK);
  }
}

static inline void DFUGetStatus() {
  // Some Get Status messages trigger a state change.
  // The status response always contains the *next* state, so figure that out
  // first.
  if (g_app.dfu_state == DFU_STATE_DNLOAD_SYNC) {
    g_app.dfu_state = DFU_STATE_DNLOAD_IDLE;
  } else if (g_app.dfu_state == DFU_STATE_MANIFEST_SYNC) {
    if (g_app.has_new_firmware) {
      g_app.dfu_state = DFU_STATE_MANIFEST;
    } else {
      g_app.dfu_state = DFU_STATE_IDLE;
    }
  }

  g_status_response[0] = g_app.dfu_status;
  g_status_response[1] = 0u;
  g_status_response[2] = 0u;
  g_status_response[3] = 0u;
  g_status_response[4] = g_app.dfu_state;
  g_status_response[5] = 0u;

  USB_DEVICE_ControlSend(g_app.usb_device, g_status_response,
                         GET_STATUS_RESPONSE_SIZE);
}

static inline void DFUClearStatus() {
  if (g_app.dfu_state == DFU_STATE_ERROR) {
    g_app.dfu_state = DFU_STATE_IDLE;
    g_app.dfu_status = DFU_STATUS_OK;
    USB_DEVICE_ControlStatus(g_app.usb_device, USB_DEVICE_CONTROL_STATUS_OK);
  } else {
    StallAndError(DFU_STATUS_ERR_STALLED_PKT);
  }
}

static inline void DFUGetState() {
  switch (g_app.dfu_state) {
    case APP_STATE_IDLE:
    case APP_STATE_DETACH:
    case DFU_STATE_IDLE:
    case DFU_STATE_DNLOAD_SYNC:
    case DFU_STATE_DNLOAD_IDLE:
    case DFU_STATE_MANIFEST_SYNC:
    case DFU_STATE_UPLOAD_IDLE:
    case DFU_STATE_ERROR:
      USB_DEVICE_ControlSend(g_app.usb_device, &g_app.dfu_state, 1);
      break;
    case DFU_STATE_DNBUSY:
    case DFU_STATE_MANIFEST:
    case DFU_STATE_MANIFEST_WAIT_RESET:
    default:
      StallAndError(DFU_STATUS_ERR_STALLED_PKT);
  }
}

static inline void DFUAbort() {
  switch (g_app.dfu_state) {
    case DFU_STATE_IDLE:
    case DFU_STATE_DNLOAD_SYNC:
    case DFU_STATE_DNLOAD_IDLE:
    case DFU_STATE_MANIFEST_SYNC:
    case DFU_STATE_UPLOAD_IDLE:
      g_app.dfu_state = DFU_STATE_IDLE;
      USB_DEVICE_ControlStatus(g_app.usb_device, USB_DEVICE_CONTROL_STATUS_OK);
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
  if (g_app.dfu_state != DFU_STATE_IDLE &&
      g_app.dfu_state != DFU_STATE_DNLOAD_IDLE) {
    StallAndError(DFU_STATUS_ERR_STALLED_PKT);
  }

  // If we're in idle state, we need to erase the flash.
  if (g_app.dfu_state == DFU_STATE_IDLE) {
    g_app.erase_flash = true;
  }

  g_app.dfu_state = DFU_STATE_DNBUSY;
  USB_DEVICE_ControlStatus(g_app.usb_device, USB_DEVICE_CONTROL_STATUS_OK);
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
  uint8_t alternate_setting;
  uint8_t* configurationValue;
  USB_SETUP_PACKET* setupPacket;

  switch (event) {
    case USB_DEVICE_EVENT_RESET:
    case USB_DEVICE_EVENT_DECONFIGURED:
      g_app.is_configured = false;
      break;

    case USB_DEVICE_EVENT_CONFIGURED:
      // Check the configuration
      configurationValue = (uint8_t*) event_data;
      if (*configurationValue == 1) {
        // Reset endpoint data send & receive flag
        g_app.is_configured = true;
      }
      break;

    case USB_DEVICE_EVENT_SUSPENDED:
      /* Device is suspended. Update LED indication */
      break;

    case USB_DEVICE_EVENT_POWER_DETECTED:
      /* VBUS is detected. Attach the device */
      USB_DEVICE_Attach(g_app.usb_device);
      break;

    case USB_DEVICE_EVENT_POWER_REMOVED:
      /* VBUS is removed. Detach the device */
      USB_DEVICE_Detach(g_app.usb_device);
      break;

    case USB_DEVICE_EVENT_CONTROL_TRANSFER_SETUP_REQUEST:
      setupPacket = (USB_SETUP_PACKET*) event_data;
      if (setupPacket->RequestType == USB_SETUP_REQUEST_TYPE_CLASS &&
          setupPacket->Recipient == USB_SETUP_REQUEST_RECIPIENT_INTERFACE &&
          setupPacket->wIndex == USB_DFU_INTERFACE_INDEX) {
        HandleDFUEvent(setupPacket);
      } else if (setupPacket->bRequest == USB_REQUEST_SET_INTERFACE) {
        // Just ACK, there are no alternate_settings.
        USB_DEVICE_ControlStatus(g_app.usb_device,
                                 USB_DEVICE_CONTROL_STATUS_OK);
      } else if (setupPacket->bRequest == USB_REQUEST_GET_INTERFACE) {
        // No alternate settings
        alternate_setting = 0u;
        USB_DEVICE_ControlSend(g_app.usb_device, &alternate_setting,
                               sizeof(alternate_setting));
      } else {
        // We have received a request that we cannot handle, stall the pipe.
        USB_DEVICE_ControlStatus(g_app.usb_device,
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


void APP_Initialize(void) {
  // TODO(simon):
  // If we're in bootloader mode, or some switch is pressed...
  g_app.usb_device = USB_DEVICE_HANDLE_INVALID;
  g_app.state = APP_STATE_INIT;
  g_app.dfu_state = DFU_STATE_IDLE;
  g_app.dfu_status = DFU_STATUS_OK;
  g_app.write_address = NULL;
  g_app.data_length = 0u;
  g_app.is_configured = false;
  g_app.next_block = 0u;
  g_app.has_new_firmware = false;
  g_app.erase_flash = false;
}

void APP_Tasks(void) {
  static unsigned int count = 0;

  switch (g_app.state) {
    case APP_STATE_INIT:
      g_app.usb_device = USB_DEVICE_Open(USB_DEVICE_INDEX_0,
                                         DRV_IO_INTENT_READWRITE);
      if (g_app.usb_device != USB_DEVICE_HANDLE_INVALID) {
        // Register a callback with device layer to get event notification
        // for endpoint 0.
        USB_DEVICE_EventHandlerSet(g_app.usb_device, USBEventHandler, NULL);
        g_app.state = APP_STATE_WAIT_FOR_USB_CONFIGURATION;
      }
      break;

    case APP_STATE_WAIT_FOR_USB_CONFIGURATION:
      if (g_app.is_configured) {
        g_app.state = APP_STATE_DFU;
      }
      break;

    case APP_STATE_DFU:
      if (!g_app.is_configured) {
        // This means the device was deconfigured, change back to waiting for
        // USB config and reset the DFU state.
        g_app.state = APP_STATE_WAIT_FOR_USB_CONFIGURATION;
        g_app.dfu_state = DFU_STATE_IDLE;
      }

      if (g_app.erase_flash) {
        if (!EraseFlash()) {
          g_app.dfu_state = DFU_STATE_ERROR;
          g_app.dfu_status = DFU_STATUS_ERR_ERASE;
        }
        g_app.erase_flash = false;
      }

      if (g_app.dfu_state == DFU_STATE_DNBUSY &&
          g_app.data_length >= FLASH_WORD_SIZE) {
        if (ProgramFlash(false)) {
          g_app.dfu_state = DFU_STATE_DNLOAD_SYNC;
        } else {
          g_app.dfu_state = DFU_STATE_ERROR;
        }
      }

      if (g_app.dfu_state == DFU_STATE_MANIFEST) {
        // The firmware may not be a multiple of 4, so write any remaining
        // bytes now
        if (!ProgramFlash(true)) {
          g_app.dfu_state = DFU_STATE_ERROR;
        } else {
          // We're done, switch back to DFU_STATE_MANIFEST_SYNC.
          g_app.has_new_firmware = false;
          g_app.dfu_state = DFU_STATE_MANIFEST_SYNC;
        }
      }

      if (++count > 50000) {
        BSP_LEDToggle(BSP_LED_1);
        count = 0;
      }
      break;
    case APP_STATE_BOOT:
      // TODO(simon): Do boot
    default:
      break;
  }
}
