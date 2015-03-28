/*
 * File:   app.h
 * Author: Simon Newton
 */

#ifndef FIRMWARE_SRC_APP_H_
#define FIRMWARE_SRC_APP_H_

#include "system_config.h"
#include "system_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Application.
 */
void APP_Initialize(void);

/**
 * @brief Perform the periodic Application tasks.
 */
void APP_Tasks(void);

/**
 * @brief Reset the application.
 */
void APP_Reset(void);

#ifdef __cplusplus
}
#endif

#endif  // FIRMWARE_SRC_APP_H_
