/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#include "sdk_common.h"
#if XINC_MODULE_ENABLED(XINC_LOG)
#include "xinc_log_default_backends.h"
#include "xinc_log_ctrl.h"
#include "xinc_log_internal.h"
#include "xinc_assert.h"

#if defined(XINC_LOG_BACKEND_RTT_ENABLED) && XINC_LOG_BACKEND_RTT_ENABLED
#include "xinc_log_backend_rtt.h"
XINC_LOG_BACKEND_RTT_DEF(rtt_log_backend);
#endif

#if defined(XINC_LOG_BACKEND_UART_ENABLED) && XINC_LOG_BACKEND_UART_ENABLED
#include "xinc_log_backend_uart.h"
XINC_LOG_BACKEND_UART_DEF(uart_log_backend);
#endif

void xinc_log_default_backends_init(void)
{
    int32_t backend_id = -1;
    (void)backend_id;
#if defined(XINC_LOG_BACKEND_RTT_ENABLED) && XINC_LOG_BACKEND_RTT_ENABLED
    xinc_log_backend_rtt_init();
    backend_id = xinc_log_backend_add(&rtt_log_backend, XINC_LOG_SEVERITY_DEBUG);
    ASSERT(backend_id >= 0);
    xinc_log_backend_enable(&rtt_log_backend);
#endif

#if defined(XINC_LOG_BACKEND_UART_ENABLED) && XINC_LOG_BACKEND_UART_ENABLED
    xinc_log_backend_uart_init();
    backend_id = xinc_log_backend_add(&uart_log_backend, XINC_LOG_SEVERITY_DEBUG);
    ASSERT(backend_id >= 0);
    xinc_log_backend_enable(&uart_log_backend);
#endif
}
#endif
