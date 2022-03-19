/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#ifndef XINC_LOG_CTRL_INTERNAL_H
#define XINC_LOG_CTRL_INTERNAL_H
/**
 * @cond (NODOX)
 * @defgroup xinc_log_ctrl_internal Auxiliary internal types declarations
 * @{
 * @internal
 */

#include "sdk_common.h"
#if XINC_MODULE_ENABLED(XINC_LOG)

#define XINC_LOG_LFCLK_FREQ 32768

#ifdef APP_TIMER_CONFIG_RTC_FREQUENCY
#define LOG_TIMESTAMP_DEFAULT_FREQUENCY ((XINC_LOG_TIMESTAMP_DEFAULT_FREQUENCY == 0) ?              \
                                       (XINC_LOG_LFCLK_FREQ/(APP_TIMER_CONFIG_RTC_FREQUENCY + 1)) : \
                                        XINC_LOG_TIMESTAMP_DEFAULT_FREQUENCY)
#else
#define LOG_TIMESTAMP_DEFAULT_FREQUENCY XINC_LOG_TIMESTAMP_DEFAULT_FREQUENCY
#endif

#define XINC_LOG_INTERNAL_INIT(...)               \
        xinc_log_init(GET_VA_ARG_1(__VA_ARGS__),  \
                     GET_VA_ARG_1(GET_ARGS_AFTER_1(__VA_ARGS__, LOG_TIMESTAMP_DEFAULT_FREQUENCY)))

#define XINC_LOG_INTERNAL_PROCESS() xinc_log_frontend_dequeue()
#define XINC_LOG_INTERNAL_FLUSH()            \
    do {                                    \
        while (XINC_LOG_INTERNAL_PROCESS()); \
    } while (0)

#define XINC_LOG_INTERNAL_FINAL_FLUSH()      \
    do {                                    \
        xinc_log_panic();                    \
        XINC_LOG_INTERNAL_FLUSH();           \
    } while (0)


#else // XINC_MODULE_ENABLED(XINC_LOG)
#define XINC_LOG_INTERNAL_PROCESS()            false
#define XINC_LOG_INTERNAL_FLUSH()
#define XINC_LOG_INTERNAL_INIT(...) XINC_SUCCESS
#define XINC_LOG_INTERNAL_HANDLERS_SET(default_handler, bytes_handler) \
    UNUSED_PARAMETER(default_handler); UNUSED_PARAMETER(bytes_handler)
#define XINC_LOG_INTERNAL_FINAL_FLUSH()
#endif // XINC_MODULE_ENABLED(XINC_LOG)

/** @}
 * @endcond
 */
#endif // XINC_LOG_CTRL_INTERNAL_H
