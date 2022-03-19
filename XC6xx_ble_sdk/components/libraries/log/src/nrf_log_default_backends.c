/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#include "sdk_common.h"
#if NRF_MODULE_ENABLED(NRF_LOG)
#include "nrf_log_default_backends.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_internal.h"
#include "nrf_assert.h"

#if defined(NRF_LOG_BACKEND_RTT_ENABLED) && NRF_LOG_BACKEND_RTT_ENABLED
#include "nrf_log_backend_rtt.h"
NRF_LOG_BACKEND_RTT_DEF(rtt_log_backend);
#endif

#if defined(NRF_LOG_BACKEND_UART_ENABLED) && NRF_LOG_BACKEND_UART_ENABLED
#include "nrf_log_backend_uart.h"
NRF_LOG_BACKEND_UART_DEF(uart_log_backend);
#endif

void nrf_log_default_backends_init(void)
{
    int32_t backend_id = -1;
    (void)backend_id;
#if defined(NRF_LOG_BACKEND_RTT_ENABLED) && NRF_LOG_BACKEND_RTT_ENABLED
    nrf_log_backend_rtt_init();
    backend_id = nrf_log_backend_add(&rtt_log_backend, NRF_LOG_SEVERITY_DEBUG);
    ASSERT(backend_id >= 0);
    nrf_log_backend_enable(&rtt_log_backend);
#endif

#if defined(NRF_LOG_BACKEND_UART_ENABLED) && NRF_LOG_BACKEND_UART_ENABLED
    nrf_log_backend_uart_init();
    backend_id = nrf_log_backend_add(&uart_log_backend, NRF_LOG_SEVERITY_DEBUG);
    ASSERT(backend_id >= 0);
    nrf_log_backend_enable(&uart_log_backend);
#endif
}
#endif
