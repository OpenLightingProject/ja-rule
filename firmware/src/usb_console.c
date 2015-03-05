/*
 * File:   usb_console.c
 * Author: Simon Newton
 */

#include "usb_console.h"

#include <stdbool.h>
#include <stdint.h>

#include "syslog.h"
#include "system_definitions.h"

#define USB_CONSOLE_BUFFER_SIZE 1024
//#define USB_CONSOLE_BUFFER_SIZE 70


// USB Device CDC Read Buffer Size. This should be a multiple of the CDC
// Bulk Endpoint size
#define USB_CONSOLE_READ_BUFFER_SIZE 64

// USB Device CDC Write Buffer Size.
#define USB_CONSOLE_WRITE_BUFFER_SIZE 64

typedef enum {
  READ_STATE_WAIT_FOR_CONFIGURATION,
  READ_STATE_SCHEDULE_READ,
  READ_STATE_WAIT_FOR_READ_COMPLETE,
  READ_STATE_READ_COMPLETE,
  READ_STATE_ERROR
} ReadState;

typedef enum {
  WRITE_STATE_WAIT_FOR_CONFIGURATION,
  WRITE_STATE_WAIT_FOR_DATA,
  WRITE_STATE_WAIT_FOR_WRITE_COMPLETE,
  WRITE_STATE_WRITE_COMPLETE,
} WriteState;

typedef struct {
  // The next index to read from. Range -1 to USB_CONSOLE_BUFFER_SIZE -1
  // -1 means the buffer is empty.
  int16_t read;
  // The next index to write to. Range 0 to USB_CONSOLE_BUFFER_SIZE - 1
  int16_t write;
  char buffer[USB_CONSOLE_BUFFER_SIZE];
} CircularBuffer;

typedef struct {
  // Set Line Coding Data
  USB_CDC_LINE_CODING set_line_coding;
  // Get Line Coding Data
  USB_CDC_LINE_CODING line_coding;
  // Control Line State
  USB_CDC_CONTROL_LINE_STATE control_line_state;

  // CDC Read
  ReadState read_state;
  USB_DEVICE_CDC_TRANSFER_HANDLE read_handle;
  char readBuffer[USB_CONSOLE_READ_BUFFER_SIZE];
  // The amount of data read.
  int read_length;

  // CDC Write
  WriteState write_state;
  USB_DEVICE_CDC_TRANSFER_HANDLE write_handle;
  CircularBuffer write;
  // The size of the last CDC write.
  uint8_t write_size;
} USBConsoleData;

USBConsoleData g_usb_console;

/*
 * @brief This is called by the Harmony CDC module when CDC events occur.
 */
USB_DEVICE_CDC_EVENT_RESPONSE USBConsole_CDCEventHandler(
   USB_DEVICE_CDC_INDEX index,
   USB_DEVICE_CDC_EVENT event,
   void* event_data,
   uintptr_t unused_user_data) {
  if (index != USB_DEVICE_CDC_INDEX_0) {
    return USB_DEVICE_CDC_EVENT_RESPONSE_NONE;
  }

  USB_CDC_CONTROL_LINE_STATE * control_line_state;
  switch (event) {
    case USB_DEVICE_CDC_EVENT_GET_LINE_CODING:
      // The host wants to know the current line coding. This is a control
      // transfer request.
      USB_DEVICE_ControlSend(USBTransport_GetHandle(),
                             &g_usb_console.line_coding,
                             sizeof(USB_CDC_LINE_CODING));
      break;

    case USB_DEVICE_CDC_EVENT_SET_LINE_CODING:
      // The host wants to set the line coding. This is a control transfer.
      USB_DEVICE_ControlReceive(USBTransport_GetHandle(),
                                &g_usb_console.set_line_coding,
                                sizeof(USB_CDC_LINE_CODING));
      break;

    case USB_DEVICE_CDC_EVENT_SET_CONTROL_LINE_STATE:
      // This means the host is setting the control line state.
      control_line_state = (USB_CDC_CONTROL_LINE_STATE *) event_data;
      g_usb_console.control_line_state.dtr = control_line_state->dtr;
      g_usb_console.control_line_state.carrier = control_line_state->carrier;
      USB_DEVICE_ControlStatus(USBTransport_GetHandle(),
                               USB_DEVICE_CONTROL_STATUS_OK);
      break;

    case USB_DEVICE_CDC_EVENT_SEND_BREAK:
      // Noop
      break;

    case USB_DEVICE_CDC_EVENT_READ_COMPLETE:
      g_usb_console.read_state = READ_STATE_READ_COMPLETE;
      g_usb_console.read_length =
          ((USB_DEVICE_CDC_EVENT_DATA_READ_COMPLETE *) event_data)->length;
      break;

    case USB_DEVICE_CDC_EVENT_CONTROL_TRANSFER_DATA_RECEIVED:
      // The data stage of the last control transfer is complete. For now we
      // accept all the data.
      USB_DEVICE_ControlStatus(USBTransport_GetHandle(),
                               USB_DEVICE_CONTROL_STATUS_OK);
      break;

    case USB_DEVICE_CDC_EVENT_CONTROL_TRANSFER_DATA_SENT:
      // This means the GET LINE CODING function data is valid.
      break;

    case USB_DEVICE_CDC_EVENT_WRITE_COMPLETE:
      g_usb_console.write_state = WRITE_STATE_WRITE_COMPLETE;
      break;

    default:
      break;
  }
  return USB_DEVICE_CDC_EVENT_RESPONSE_NONE;
}

/*
 * @brief Check if the device was reset.
 * @returns true if the USB device was reset, false otherwise.
 */
bool USBConsole_CheckAndHandleReset() {
  if (USBTransport_IsConfigured() == false) {
    g_usb_console.read_state = READ_STATE_WAIT_FOR_CONFIGURATION;
    g_usb_console.read_handle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
    g_usb_console.write_handle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
    g_usb_console.write_state = WRITE_STATE_WAIT_FOR_CONFIGURATION;
    return true;
  }
  return false;
}

void USBConsole_Initialize() {
  // Dummy line coding params.
  g_usb_console.line_coding.dwDTERate = 9600;
  g_usb_console.line_coding.bParityType = 0;
  g_usb_console.line_coding.bParityType = 0;
  g_usb_console.line_coding.bDataBits = 8;

  g_usb_console.read_state = READ_STATE_WAIT_FOR_CONFIGURATION;
  g_usb_console.read_handle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
  g_usb_console.read_length = 0;

  g_usb_console.write_state = WRITE_STATE_WAIT_FOR_CONFIGURATION;
  g_usb_console.write_handle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
  g_usb_console.write.read = -1;
  g_usb_console.write.write = 0;
  g_usb_console.write_state = 0;

  USB_DEVICE_CDC_EventHandlerSet(USB_DEVICE_CDC_INDEX_0,
                                 USBConsole_CDCEventHandler, NULL);
}

void USBConsole_Log(const char* message) {
  if (!USBTransport_IsConfigured()) {
    return;
  }

  if (g_usb_console.write.write == g_usb_console.write.read) {
    // Buffer is full
    return;
  }
  // Include the NULL since we'll need to replace it with a \n
  unsigned int length = strlen(message) + 1;

  if (g_usb_console.write.read < 0) {
    // If the buffer is empty, set the read index, otherwise don't change it.
    g_usb_console.write.read = 0;
    g_usb_console.write.write = 0;
  }

  unsigned int dest_size = USB_CONSOLE_BUFFER_SIZE - g_usb_console.write.write;
  if (dest_size > length) {
    dest_size = length;
  }
  memcpy(g_usb_console.write.buffer + g_usb_console.write.write,
         message, dest_size);

  if (g_usb_console.write.write + dest_size < USB_CONSOLE_BUFFER_SIZE) {
    g_usb_console.write.write += dest_size;
    g_usb_console.write.buffer[g_usb_console.write.write - 1] = '\n';
    return;
  }

  // We reached the end of the buffer.
  if (g_usb_console.write.read == 0) {
    // No more room, \n terminate.
    g_usb_console.write.buffer[USB_CONSOLE_BUFFER_SIZE - 1] = '\n';
    g_usb_console.write.write = 0;
    return;
  }

  // Write to the start of the buffer.
  unsigned int offset = dest_size;
  dest_size = length - dest_size;
  if (dest_size > g_usb_console.write.read) {
    dest_size = g_usb_console.write.read;
  }
  memcpy(g_usb_console.write.buffer, message + offset, dest_size);
  g_usb_console.write.write = dest_size;
  g_usb_console.write.buffer[g_usb_console.write.write - 1] = '\n';
}

void USBConsole_Tasks() {
  if (USBConsole_CheckAndHandleReset()) {
    return;
  }

  // Writer state machine
  switch (g_usb_console.write_state) {
    case WRITE_STATE_WAIT_FOR_CONFIGURATION:
      if (!USBTransport_IsConfigured()) {
        break;
      }
      g_usb_console.write_state = WRITE_STATE_WAIT_FOR_DATA;
      // Fall through
    case WRITE_STATE_WAIT_FOR_DATA:
      if (g_usb_console.write.read != -1) {
        g_usb_console.write_handle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
        if (g_usb_console.write.read < g_usb_console.write.write) {
          g_usb_console.write_size =
            g_usb_console.write.write - g_usb_console.write.read;
        } else {
          // Buffer has wrapped
          g_usb_console.write_size =
            USB_CONSOLE_BUFFER_SIZE - g_usb_console.write.read;
        }
        if (g_usb_console.write_size > USB_CONSOLE_WRITE_BUFFER_SIZE) {
          g_usb_console.write_size = USB_CONSOLE_WRITE_BUFFER_SIZE;
        }

        USB_DEVICE_CDC_Write(
            USB_DEVICE_CDC_INDEX_0,
            &g_usb_console.write_handle,
            g_usb_console.write.buffer + g_usb_console.write.read,
            g_usb_console.write_size,
            USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE);
        g_usb_console.write_state = WRITE_STATE_WAIT_FOR_WRITE_COMPLETE;
      }
      break;
    case WRITE_STATE_WAIT_FOR_WRITE_COMPLETE:
      // Noop
      break;
    case WRITE_STATE_WRITE_COMPLETE:
      g_usb_console.write.read += g_usb_console.write_size;
      if (g_usb_console.write.read >= USB_CONSOLE_BUFFER_SIZE) {
        g_usb_console.write.read = 0;
      }
      if (g_usb_console.write.read == g_usb_console.write.write) {
        g_usb_console.write.write = 0;
        g_usb_console.write.read = -1;
      }
      g_usb_console.write_state = WRITE_STATE_WAIT_FOR_DATA;
      break;
  }

  // Reader state machine
  switch (g_usb_console.read_state) {
    case READ_STATE_WAIT_FOR_CONFIGURATION:
      if (!USBTransport_IsConfigured()) {
        break;
      }
      g_usb_console.read_state = READ_STATE_SCHEDULE_READ;
      // Fall through
    case READ_STATE_SCHEDULE_READ:
      g_usb_console.read_state = READ_STATE_WAIT_FOR_READ_COMPLETE;
      g_usb_console.read_handle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
      USB_DEVICE_CDC_Read(USB_DEVICE_CDC_INDEX_0,
                          &g_usb_console.read_handle,
                          g_usb_console.readBuffer,
                          USB_CONSOLE_READ_BUFFER_SIZE);

      if (g_usb_console.read_handle == USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID) {
        g_usb_console.read_state = READ_STATE_ERROR;
        break;
      }
      break;
    case READ_STATE_WAIT_FOR_READ_COMPLETE:
      // Noop
      break;
    case READ_STATE_READ_COMPLETE:
      switch (g_usb_console.readBuffer[0]) {
        case '+':
          SysLog_Increment();
          SysLog_Print(SYSLOG_ALWAYS, "Log level: %s",
                       SysLog_LevelToString(SysLog_GetLevel()));
          break;
        case '-':
          SysLog_Decrement();
          SysLog_Print(SYSLOG_ALWAYS, "Log level: %s",
                       SysLog_LevelToString(SysLog_GetLevel()));
          break;
        case 'e':
          SysLog_Message(SYSLOG_ERROR, "error");
          break;
        case 'i':
          SysLog_Message(SYSLOG_INFO, "info");
          break;
        case 'w':
          SysLog_Message(SYSLOG_WARN, "warning");
          break;
        case 'd':
          SysLog_Message(SYSLOG_DEBUG, "debug");
          break;
        case 'f':
          SysLog_Message(SYSLOG_FATAL, "fatal");
          break;
        default:
          if (g_usb_console.read_length == USB_CONSOLE_READ_BUFFER_SIZE) {
            g_usb_console.readBuffer[USB_CONSOLE_READ_BUFFER_SIZE - 1] = 0;
          } else {
            g_usb_console.readBuffer[g_usb_console.read_length] = 0;
          }
          USBConsole_Log(g_usb_console.readBuffer);
      }
      g_usb_console.read_state = READ_STATE_SCHEDULE_READ;
      break;
    case READ_STATE_ERROR:
      break;
  }
}
