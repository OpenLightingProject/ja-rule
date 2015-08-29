/*
 * File:   system_pipeline.h
 * Author: simonn
 */

#ifndef FIRMWARE_SRC_SYSTEM_CONFIG_MX_795_512L_SYSTEM_PIPELINE_H_
#define FIRMWARE_SRC_SYSTEM_CONFIG_MX_795_512L_SYSTEM_PIPELINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define PIPELINE_TRANSPORT_TX(token, command, rc, iov, iov_count) \
  USBTransport_SendResponse(token, command, rc, iov, iov_count);

#define PIPELINE_TRANSPORT_RX(data, size) \
  StreamDecoder_Process(data, size);

#define PIPELINE_HANDLE_MESSAGE(message) \
  MessageHandler_HandleMessage(message);

#define PIPELINE_LOG_WRITE(message) \
  USBConsole_Log(message);

#define PIPELINE_TRANSCEIVER_TX_EVENT(event) \
  MessageHandler_TransceiverEvent(event);

#define PIPELINE_TRANSCEIVER_RX_EVENT(event) \
  Responder_Receive(event);

#define PIPELINE_RDMRESPONDER_SEND(include_break, iov, iov_len) \
  Transceiver_QueueRDMResponse(include_break, iov, iov_len);

#ifdef __cplusplus
}
#endif

#endif  // FIRMWARE_SRC_SYSTEM_CONFIG_MX_795_512L_SYSTEM_PIPELINE_H_
