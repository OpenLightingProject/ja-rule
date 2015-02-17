/*
 * File:   syslog.c
 * Author: Simon Newton
 */

#include "syslog.h"

#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include "system_pipeline.h"

#define SYSLOG_PRINT_BUFFER_SIZE 256

typedef struct {
  uint8_t log_level;
  SysLogWriteFn write_fn;
  char printf_buffer[SYSLOG_PRINT_BUFFER_SIZE];
} SysLogData;

SysLogData g_syslog;

void SysLog_Initialize(SysLogWriteFn write_fn) {
  g_syslog.log_level = SYSLOG_WARN;
  g_syslog.write_fn = write_fn;
}

static inline void SysLog_Write(const char* msg) {
#ifdef PIPELINE_LOG_WRITE
  PIPELINE_LOG_WRITE(msg);
#else
  g_syslog.write_fn(msg);
#endif
}

void SysLog_Message(SysLogLevel level, const char* msg) {
  if (level >= g_syslog.log_level) {
    SysLog_Write(msg);
  }
}

void SysLog_Print(SysLogLevel level, const char* format, ...) {
  if (level < g_syslog.log_level) {
    return;
  }

  size_t padding = 0;
  va_list args;

  va_start(args, format);
  size_t len = vsnprintf(g_syslog.printf_buffer, SYSLOG_PRINT_BUFFER_SIZE,
                         format, args);
  va_end(args);
  SysLog_Write(g_syslog.printf_buffer);
}

SysLogLevel SysLog_GetLevel() {
  return g_syslog.log_level;
}

void SysLog_SetLevel(SysLogLevel level) {
  g_syslog.log_level = level;
}

void SysLog_Increment() {
  if (g_syslog.log_level != SYSLOG_DEBUG) {
    g_syslog.log_level -= 1;
  }
}

void SysLog_Decrement() {
  if (g_syslog.log_level != SYSLOG_FATAL) {
    g_syslog.log_level += 1;
  }
}

const char* SysLog_LevelToString(SysLogLevel level) {
  switch (level) {
    case SYSLOG_DEBUG:
      return "DEBUG";
    case SYSLOG_INFO:
      return "INFO";
    case SYSLOG_WARN:
      return "WARNING";
    case SYSLOG_ERROR:
      return "ERROR";
    case SYSLOG_FATAL:
      return "FATAL";
    case SYSLOG_ALWAYS:
      return "ALWAYS";
    default:
      return "UNKNOWN";
  }
}
