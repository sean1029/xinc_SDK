/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#ifndef NRF_LOG_TYPES_H
#define NRF_LOG_TYPES_H

#include <stdint.h>

/**
 * @brief Logger severity levels.
 */
typedef enum
{
    NRF_LOG_SEVERITY_NONE,
    NRF_LOG_SEVERITY_ERROR,
    NRF_LOG_SEVERITY_WARNING,
    NRF_LOG_SEVERITY_INFO,
    NRF_LOG_SEVERITY_DEBUG,
    NRF_LOG_SEVERITY_INFO_RAW, /* Artificial level to pass information about skipping string postprocessing.*/
} nrf_log_severity_t;

/**
 * @brief Structure holding dynamic data associated with a module.
 *
 * See @ref NRF_LOG_MODULE_REGISTER and @ref NRF_LOG_INSTANCE_REGISTER.
 */
typedef struct
{
    uint16_t     order_idx;     ///< Ordered index of the module (used for auto-completion).
    uint16_t     filter;        ///< Current highest severity level accepted (redundant to @ref nrf_log_module_filter_data_t::filter_lvls, used for optimization)
} nrf_log_module_dynamic_data_t;

/**
 * @brief Structure holding dynamic filters associated with a module or instance if filtering is enabled (@ref NRF_LOG_FILTERS_ENABLED).
 *
 * @note Backend filters logically are part of @ref nrf_log_module_dynamic_data_t but they are kept separated to enable storing them in non-volatile memory.
 */
typedef struct
{
    uint32_t     filter_lvls;   ///< Current severity levels for each backend (3 bits per backend).
} nrf_log_module_filter_data_t;

/**
 * @brief Structure holding constant data associated with a module or instance.
 *
 * See @ref NRF_LOG_MODULE_REGISTER and @ref NRF_LOG_INSTANCE_REGISTER.
 */
typedef struct
{
    const char *       p_module_name;    ///< Module or instance name.
    uint8_t            info_color_id;    ///< Color code of info messages.
    uint8_t            debug_color_id;   ///< Color code of debug messages.
    nrf_log_severity_t compiled_lvl;     ///< Compiled highest severity level.
    nrf_log_severity_t initial_lvl;      ///< Severity level for given module or instance set on backend initialization.
} nrf_log_module_const_data_t;

#endif //NRF_LOG_TYPES_H
