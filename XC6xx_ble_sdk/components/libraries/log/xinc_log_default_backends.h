/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#ifndef XINC_LOG_DEFAULT_BACKENDS_H__
#define XINC_LOG_DEFAULT_BACKENDS_H__

/**@file
 * @addtogroup xinc_log Logger module
 * @ingroup    app_common
 *
 * @defgroup xinc_log_default_backends Functions for initializing and adding default backends
 * @{
 * @ingroup  xinc_log
 * @brief    The xinc_log default backends.
 */

#include "sdk_config.h"
#include "sdk_errors.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def XINC_LOG_DEFAULT_BACKENDS_INIT
 * @brief Macro for initializing default backends.
 *
 * Each backend enabled in configuration is initialized and added as a backend to the logger.
 */
#if XINC_LOG_ENABLED
#define XINC_LOG_DEFAULT_BACKENDS_INIT() xinc_log_default_backends_init()
#else
#define XINC_LOG_DEFAULT_BACKENDS_INIT()
#endif

void xinc_log_default_backends_init(void);

#ifdef __cplusplus
}
#endif

/** @} */

#endif // XINC_LOG_DEFAULT_BACKENDS_H__
