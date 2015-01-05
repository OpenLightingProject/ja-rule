/*
 * File:   app.h
 * Author: Simon Newton
 */

#ifndef SRC_APP_H_
#define SRC_APP_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "system_config.h"
#include "system_definitions.h"

/**
 * @brief Initialize the Application.
 */
void APP_Initialize(void);

/**
 * @brief Perform the periodic Application tasks.
 */
void APP_Tasks(void);

#endif  // SRC_APP_H_

