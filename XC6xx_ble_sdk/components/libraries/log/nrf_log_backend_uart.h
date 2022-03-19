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
 * @defgroup nrf_log_backend_uart Log UART backend
 * @{
 * @ingroup  nrf_log
 * @brief Log UART backend.
 */

#ifndef NRF_LOG_BACKEND_UART_H
#define NRF_LOG_BACKEND_UART_H

#include "nrf_log_backend_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const nrf_log_backend_api_t nrf_log_backend_uart_api;

typedef struct {
    nrf_log_backend_t               backend;
} nrf_log_backend_uart_t;

#define NRF_LOG_BACKEND_UART_DEF(_name)                         \
    NRF_LOG_BACKEND_DEF(_name, nrf_log_backend_uart_api, NULL)

void nrf_log_backend_uart_init(void);

#ifdef __cplusplus
}
#endif

#endif //NRF_LOG_BACKEND_UART_H

/** @} */
