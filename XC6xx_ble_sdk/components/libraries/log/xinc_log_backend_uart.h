/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

 /**@file
 *
 * @defgroup xinc_log_backend_uart Log UART backend
 * @{
 * @ingroup  xinc_log
 * @brief Log UART backend.
 */

#ifndef XINC_LOG_BACKEND_UART_H
#define XINC_LOG_BACKEND_UART_H

#include "xinc_log_backend_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const xinc_log_backend_api_t xinc_log_backend_uart_api;

typedef struct {
    xinc_log_backend_t               backend;
} xinc_log_backend_uart_t;

#define XINC_LOG_BACKEND_UART_DEF(_name)                         \
    XINC_LOG_BACKEND_DEF(_name, xinc_log_backend_uart_api, NULL)

void xinc_log_backend_uart_init(void);

#ifdef __cplusplus
}
#endif

#endif //XINC_LOG_BACKEND_UART_H

/** @} */
