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
 * @defgroup xinc_log_str_formatter String formatter for the logger messages
 * @{
 * @ingroup xinc_log
 */

#ifndef XINC_LOG_STR_FORMATTER_H
#define XINC_LOG_STR_FORMATTER_H

#include <stdint.h>
#include "xinc_fprintf.h"
#include "xinc_log_ctrl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint32_t            timestamp;
    uint16_t            module_id;
    uint16_t            dropped;
    xinc_log_severity_t  severity;
    uint8_t             use_colors;
} xinc_log_str_formatter_entry_params_t;


void xinc_log_std_entry_process(char const * p_str,
                               uint32_t const * p_args,
                               uint32_t nargs,
                               xinc_log_str_formatter_entry_params_t * p_params,
                               xinc_fprintf_ctx_t * p_ctx);

void xinc_log_hexdump_entry_process(uint8_t * p_data,
                                   uint32_t data_len,
                                   xinc_log_str_formatter_entry_params_t * p_params,
                                   xinc_fprintf_ctx_t * p_ctx);

void xinc_log_str_formatter_timestamp_freq_set(uint32_t freq);
#ifdef __cplusplus
}
#endif

#endif //XINC_LOG_STR_FORMATTER_H
/** @} */
