/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#ifndef NRF_LOG_CTRL_INTERNAL_H
#define NRF_LOG_CTRL_INTERNAL_H
/**
 * @cond (NODOX)
 * @defgroup nrf_log_ctrl_internal Auxiliary internal types declarations
 * @{
 * @internal
 */

#include "sdk_common.h"
#if NRF_MODULE_ENABLED(NRF_LOG)

#define NRF_LOG_LFCLK_FREQ 32768

#ifdef APP_TIMER_CONFIG_RTC_FREQUENCY
#define LOG_TIMESTAMP_DEFAULT_FREQUENCY ((NRF_LOG_TIMESTAMP_DEFAULT_FREQUENCY == 0) ?              \
                                       (NRF_LOG_LFCLK_FREQ/(APP_TIMER_CONFIG_RTC_FREQUENCY + 1)) : \
                                        NRF_LOG_TIMESTAMP_DEFAULT_FREQUENCY)
#else
#define LOG_TIMESTAMP_DEFAULT_FREQUENCY NRF_LOG_TIMESTAMP_DEFAULT_FREQUENCY
#endif

#define NRF_LOG_INTERNAL_INIT(...)               \
        nrf_log_init(GET_VA_ARG_1(__VA_ARGS__),  \
                     GET_VA_ARG_1(GET_ARGS_AFTER_1(__VA_ARGS__, LOG_TIMESTAMP_DEFAULT_FREQUENCY)))

#define NRF_LOG_INTERNAL_PROCESS() nrf_log_frontend_dequeue()
#define NRF_LOG_INTERNAL_FLUSH()            \
    do {                                    \
        while (NRF_LOG_INTERNAL_PROCESS()); \
    } while (0)

#define NRF_LOG_INTERNAL_FINAL_FLUSH()      \
    do {                                    \
        nrf_log_panic();                    \
        NRF_LOG_INTERNAL_FLUSH();           \
    } while (0)


#else // NRF_MODULE_ENABLED(NRF_LOG)
#define NRF_LOG_INTERNAL_PROCESS()            false
#define NRF_LOG_INTERNAL_FLUSH()
#define NRF_LOG_INTERNAL_INIT(...) NRF_SUCCESS
#define NRF_LOG_INTERNAL_HANDLERS_SET(default_handler, bytes_handler) \
    UNUSED_PARAMETER(default_handler); UNUSED_PARAMETER(bytes_handler)
#define NRF_LOG_INTERNAL_FINAL_FLUSH()
#endif // NRF_MODULE_ENABLED(NRF_LOG)

/** @}
 * @endcond
 */
#endif // NRF_LOG_CTRL_INTERNAL_H
