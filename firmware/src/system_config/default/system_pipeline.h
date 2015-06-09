/*
 * File:   system_pipeline.h
 * Author: Simon Newton
 */

/**
 * @{
 * @file system_pipeline.h
 * @brief The compile time pipeline for the device.
 */

#ifndef FIRMWARE_SRC_SYSTEM_CONFIG_DEFAULT_SYSTEM_PIPELINE_H_
#define FIRMWARE_SRC_SYSTEM_CONFIG_DEFAULT_SYSTEM_PIPELINE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Controls which function is used to transmit messages to the host.
 *
 * This should either call a function of type TXFunction or be undefined.
 */
#define PIPELINE_TRANSPORT_TX(token, command, rc, iov, iov_count) \
  USBTransport_SendResponse(token, command, rc, iov, iov_count);

/**
 * @brief Controls which function is called when data is received from the
 * host.
 *
 * This should either call a function of type RXFunction or be undefined.
 */
#define PIPELINE_TRANSPORT_RX(data, size) \
  StreamDecoder_Process(data, size);

/**
 * @brief Controls which function is used to handle messages from the host.
 *
 * This should either call a function of type MessageHandler or be undefined.
 */
#define PIPELINE_HANDLE_MESSAGE(message) \
  MessageHandler_HandleMessage(message);

/**
 * @brief Specifies the function to call to log messages.
 */
#define PIPELINE_LOG_WRITE(message) \
  USBConsole_Log(message);

/**
 * @brief Specifies the function to call when a transceiver event occurs.
 * @sa TransceiverEventCallback.
 *
 * This should either call a function of type TransceiverEventCallback or
 * undefined.
 */
#define PIPELINE_TRANSCEIVER_EVENT(event) \
  MessageHandler_TransceiverEvent(event);

/**
 * @brief Specifies the function to call when the RDMResponder needs to send a
 * frame.
 * @sa RDMResponderSendCallback.
 *
 * This should either call a function of type RDMResponderSendCallback or
 * undefined.
 */
//#define PIPELINE_RDMRESPONDER_SEND(include_break, iov, iov_len) \

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_SYSTEM_CONFIG_DEFAULT_SYSTEM_PIPELINE_H_
