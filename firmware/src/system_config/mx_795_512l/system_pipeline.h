/*
 * File:   system_pipeline.h
 * Author: simonn
 */

#ifndef SRC_SYSTEM_CONFIG_MX_795_512L_SYSTEM_PIPELINE_H_
#define SRC_SYSTEM_CONFIG_MX_795_512L_SYSTEM_PIPELINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define PIPELINE_TRANSPORT_TX(command, rc, iov, iov_count) \
  USBTransport_SendResponse(command, rc, iov, iov_count);

#define PIPELINE_TRANSPORT_RX(data, size) \
  StreamDecoder_Process(data, size);

#define PIPELINE_HANDLE_MESSAGE(message) \
  MessageHandler_HandleMessage(message);

#define PIPELINE_LOG_WRITE(message) \
  USBConsole_Log(message);

#define PIPELINE_TRANSCEIVER_EVENT(operation, rc, data, length) \
  MessageHandler_TransceiverEvent(operation, rc, data, length);

#ifdef __cplusplus
}
#endif

#endif  // SRC_SYSTEM_CONFIG_MX_795_512L_SYSTEM_PIPELINE_H_

