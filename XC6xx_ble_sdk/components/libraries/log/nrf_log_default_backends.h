/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#ifndef NRF_LOG_DEFAULT_BACKENDS_H__
#define NRF_LOG_DEFAULT_BACKENDS_H__

/**@file
 * @addtogroup nrf_log Logger module
 * @ingroup    app_common
 *
 * @defgroup nrf_log_default_backends Functions for initializing and adding default backends
 * @{
 * @ingroup  nrf_log
 * @brief    The nrf_log default backends.
 */

#include "sdk_config.h"
#include "sdk_errors.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def NRF_LOG_DEFAULT_BACKENDS_INIT
 * @brief Macro for initializing default backends.
 *
 * Each backend enabled in configuration is initialized and added as a backend to the logger.
 */
#if NRF_LOG_ENABLED
#define NRF_LOG_DEFAULT_BACKENDS_INIT() nrf_log_default_backends_init()
#else
#define NRF_LOG_DEFAULT_BACKENDS_INIT()
#endif

void nrf_log_default_backends_init(void);

#ifdef __cplusplus
}
#endif

/** @} */

#endif // NRF_LOG_DEFAULT_BACKENDS_H__
