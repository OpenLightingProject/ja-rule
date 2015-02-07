/*
 * This is the system_pipeline.h used for the tests. All macros are undefined,
 * which means we use the function pointers passed to the initialization
 * functions as callbacks.
 */

#ifndef TESTS_SYSTEM_CONFIG_SYSTEM_PIPELINE_H_
#define TESTS_SYSTEM_CONFIG_SYSTEM_PIPELINE_H_
#ifdef  __cplusplus
extern "C" {
#endif

// Undefined. Use the tx_cb arg.
// #define PIPELINE_TRANSPORT_TX(command, rc, iov, iov_count)

// Undefined. Use the rx_cb arg.
// #define PIPELINE_TRANSPORT_RX(data, size);

// Undefined. Use the handler arg instead.
// #define PIPELINE_HANDLE_MESSAGE(message)

#ifdef  __cplusplus
}
#endif

#endif  // TESTS_SYSTEM_CONFIG_SYSTEM_PIPELINE_H_
