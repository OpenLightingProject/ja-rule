/*
 * File:   system_pipeline.h
 * Author: simonn
 */

#ifndef SRC_SYSTEM_CONFIG_MX_795_512L_SYSTEM_PIPELINE_H_
#define SRC_SYSTEM_CONFIG_MX_795_512L_SYSTEM_PIPELINE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Controls which function is used to transmit messages to the host.
 *
 * This should either call a function of type TXFunction or be undefined.
 */
#define PIPELINE_TRANSPORT_TX(command, rc, iov, iov_count) \
  USBTransport_SendResponse(command, rc, iov, iov_count);

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
  HandleMessage(message);

#ifdef __cplusplus
}
#endif

#endif  // SRC_SYSTEM_CONFIG_MX_795_512L_SYSTEM_PIPELINE_H_

