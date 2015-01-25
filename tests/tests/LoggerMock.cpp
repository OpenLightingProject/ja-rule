#include "CMockaWrapper.h"
#include "logger.h"

LoggerData g_logger;

void Logging_Initialize(TXFunction tx_cb, uint16_t max_payload_size) {
  check_expected(tx_cb);
  check_expected(max_payload_size);
}

void Logging_SetState(bool enabled) {
  check_expected(enabled);
}

void Logging_Log(const char* str) {
  check_expected(str);
}

void Logging_SendResponse() {}

void Logging_SetDataPendingFlag(bool flag) {
  g_logger.read = flag ? 0 : - 1;
}

void Logging_SetOverflowFlag(bool flag) {
  g_logger.overflow = flag;
}
