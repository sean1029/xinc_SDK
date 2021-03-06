/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#ifndef XINC_LOG_CTRL_H
#define XINC_LOG_CTRL_H

/**@file
 * @addtogroup xinc_log Logger module
 * @ingroup    app_common
 *
 * @defgroup xinc_log_ctrl Functions for controlling xinc_log
 * @{
 * @ingroup  xinc_log
 * @brief    The xinc_log control interface.
 */

#include "sdk_config.h"
#include "sdk_errors.h"
#include <stdint.h>
#include <stdbool.h>
#include "xinc_log_types.h"
#include "xinc_log_ctrl_internal.h"
#include "xinc_log_backend_interface.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Timestamp function prototype.
 *
 * @return Timestamp value.
 */
typedef uint32_t (*xinc_log_timestamp_func_t)(void);


/**@brief Macro for initializing the logs.
 *
 * Macro has one or two parameters. First parameter (obligatory) is the timestamp function (@ref xinc_log_timestamp_func_t).
 * Additionally, as the second parameter timestamp frequency in Hz can be provided. If not provided then default
 * frequency is used (@ref  XINC_LOG_TIMESTAMP_DEFAULT_FREQUENCY). Frequency is used to format timestamp prefix if
 * @ref XINC_LOG_STR_FORMATTER_TIMESTAMP_FORMAT_ENABLED is set.
 *
 * @return  XINC_SUCCESS after successful initialization, otherwise an error code.
 */
#define XINC_LOG_INIT(...) XINC_LOG_INTERNAL_INIT(__VA_ARGS__)


/**@brief Macro for processing a single log entry from a queue of deferred logs.
 *
 * You can call this macro from the main context or from the error handler to process
 * log entries one by one.
 *
 * @note If logs are not deferred, this call has no use and is defined as 'false'.
 *
 * @retval true    There are more logs to process in the buffer.
 * @retval false   No more logs in the buffer.
 */
#define XINC_LOG_PROCESS()    XINC_LOG_INTERNAL_PROCESS()

/** @brief Macro for processing all log entries from the buffer.
 * It blocks until all buffered entries are processed by the backend.
 *
 * @note If logs are not deferred, this call has no use and is empty.
 */
#define XINC_LOG_FLUSH()      XINC_LOG_INTERNAL_FLUSH()

/** @brief Macro for flushing log data before reset.
 *
 * @note If logs are not deferred, this call has no use and is empty.
 *
 * @note If RTT is used, then a breakpoint is hit once flushed.
 */
#define XINC_LOG_FINAL_FLUSH() XINC_LOG_INTERNAL_FINAL_FLUSH()

/**
 * @brief Function for initializing the frontend and the default backend.
 *
 * @ref XINC_LOG_INIT calls this function to initialize the frontend and the backend.
 * If custom backend is used, then @ref XINC_LOG_INIT should not be called.
 * Instead, frontend and user backend should be verbosely initialized.
 *
 * @param timestamp_func Function for getting a 32-bit timestamp.
 * @param timestamp_freq Frequency of the timestamp.
 *
 * @return Error status.
 *
 */
ret_code_t xinc_log_init(xinc_log_timestamp_func_t timestamp_func, uint32_t timestamp_freq);

/**
 * @brief Function for adding new backend interface to the logger.
 *
 * @param p_backend Pointer to the backend interface.
 * @param severity  Initial value of severity level for each module forwarded to the backend. This
 *                  option is only applicable if @ref XINC_LOG_FILTERS_ENABLED is set.
 * @return -1 if backend cannot be added or positive number (backend ID).
 */
int32_t xinc_log_backend_add(xinc_log_backend_t const * p_backend, xinc_log_severity_t severity);

/**
 * @brief Function for removing backend from the logger.
 *
 * @param p_backend Pointer to the backend interface.
 *
 */
void xinc_log_backend_remove(xinc_log_backend_t const * p_backend);

/**
 * @brief Function for setting logger backends into panic mode.
 *
 * When this function is called all attached backends are informed about panic state of the system.
 * It is up to the backend to react properly (hold or process logs in blocking mode, etc.)
 */
void xinc_log_panic(void);

/**
 * @brief Function for handling a single log entry.
 *
 * Use this function only if the logs are buffered. It takes a single entry from the
 * buffer and attempts to process it.
 *
 * @retval true  If there are more entries to process.
 * @retval false If there are no more entries to process.
 */
bool xinc_log_frontend_dequeue(void);

/**
 * @brief Function for getting number of independent log modules registered into the logger.
 *
 * @return Number of registered modules.
 */
uint32_t xinc_log_module_cnt_get(void);

/**
 * @brief Function for getting module name.
 *
 * @param module_id      Module ID.
 * @param is_ordered_idx Module ID is given is index in alphabetically sorted list of modules.
 * @return Pointer to string with module name.
 */
const char * xinc_log_module_name_get(uint32_t module_id, bool is_ordered_idx);

/**
 * @brief Function for getting coloring of specific logs.
 *
 * @param module_id Module ID.
 * @param severity  Log severity.
 *
 * @return ID of the color.
 */
uint8_t xinc_log_color_id_get(uint32_t module_id, xinc_log_severity_t severity);

/**
 * @brief Function for configuring filtering ofs logs in the module.
 *
 * Filtering of logs in modules is independent for each backend.
 *
 * @param backend_id Backend ID which want to chenge its configuration.
 * @param module_id  Module ID which logs will be reconfigured.
 * @param severity   New severity filter.
 */
void xinc_log_module_filter_set(uint32_t backend_id,
                               uint32_t module_id,
                               xinc_log_severity_t severity);

/**
 * @brief Function for getting module severity level.
 *
 * @param backend_id     Backend ID.
 * @param module_id      Module ID.
 * @param is_ordered_idx Module ID is given is index in alphabetically sorted list of modules.
 * @param dynamic        It true current filter for given backend is returned. If false then
 *                       compiled-in level is returned (maximum available). If this parameter is
 *                       false then backend_id parameter is not used.
 *
 * @return Severity.
 */
xinc_log_severity_t xinc_log_module_filter_get(uint32_t backend_id,
                                             uint32_t module_id,
                                             bool     is_ordered_idx,
                                             bool     dynamic);

/**
 * @brief Function stores current filtering configuration into non-volatile memory using @ref fds module.
 *
 * @return XINC_SUCCESS or @ref fds error code.
 */
ret_code_t xinc_log_config_store(void);

/**
 * @brief Function loads configuration from non-volatile memory using @ref fds module.
 *
 * @retval XINC_SUCCESS         On successful loading.
 * @retval XINC_ERROR_NOT_FOUND Configuration file not found.
 * @retval XINC_ERROR_INTERNAL  Other @ref fds error on reading configuration file.
 */
ret_code_t xinc_log_config_load(void);

#ifdef __cplusplus
}
#endif

#endif // XINC_LOG_CTRL_H

/**
 *@}
 **/
