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
#include "nrf_log_str_formatter.h"
#include "nrf_log_internal.h"
#include "nrf_log_ctrl.h"
#include "nrf_fprintf.h"
#include <ctype.h>

#define XINC_LOG_COLOR_CODE_DEFAULT "\x1B[0m"
#define XINC_LOG_COLOR_CODE_BLACK   "\x1B[1;30m"
#define XINC_LOG_COLOR_CODE_RED     "\x1B[1;31m"
#define XINC_LOG_COLOR_CODE_GREEN   "\x1B[1;32m"
#define XINC_LOG_COLOR_CODE_YELLOW  "\x1B[1;33m"
#define XINC_LOG_COLOR_CODE_BLUE    "\x1B[1;34m"
#define XINC_LOG_COLOR_CODE_MAGENTA "\x1B[1;35m"
#define XINC_LOG_COLOR_CODE_CYAN    "\x1B[1;36m"
#define XINC_LOG_COLOR_CODE_WHITE   "\x1B[1;37m"

#define XINC_LOG_CHAR_CODE_MAX      0x7E

static const char * severity_names[] = {
        NULL,
        "error",
        "warning",
        "info",
        "debug"
};

static const char * m_colors[] = {
        XINC_LOG_COLOR_CODE_DEFAULT,
        XINC_LOG_COLOR_CODE_BLACK,
        XINC_LOG_COLOR_CODE_RED,
        XINC_LOG_COLOR_CODE_GREEN,
        XINC_LOG_COLOR_CODE_YELLOW,
        XINC_LOG_COLOR_CODE_BLUE,
        XINC_LOG_COLOR_CODE_MAGENTA,
        XINC_LOG_COLOR_CODE_CYAN,
        XINC_LOG_COLOR_CODE_WHITE,
};

static uint32_t m_freq;
static uint32_t m_timestamp_div;

static void timestamp_print(nrf_fprintf_ctx_t * p_ctx, uint32_t timestamp)
{
    if (XINC_LOG_USES_TIMESTAMP)
    {
        if (XINC_LOG_STR_FORMATTER_TIMESTAMP_FORMAT_ENABLED)
        {
            timestamp        /= m_timestamp_div;
            uint32_t seconds  = timestamp/m_freq;
            uint32_t hours    = seconds/3600;
            seconds          -= hours * 3600;
            uint32_t mins     = seconds/60;
            seconds          -= mins * 60;

            uint32_t reminder = timestamp % m_freq;
            uint32_t ms       = (reminder * 1000)/m_freq;
            uint32_t us       = (1000*(1000*reminder - (ms * m_freq)))/m_freq;

            nrf_fprintf(p_ctx, "[%02d:%02d:%02d.%03d,%03d] ", hours, mins, seconds, ms, us);
        }
        else
        {
            nrf_fprintf(p_ctx, "[%08lu] ", timestamp);
        }
    }
}
static void prefix_process(nrf_log_str_formatter_entry_params_t * p_params,
                           nrf_fprintf_ctx_t * p_ctx)
{
    if (p_params->dropped)
    {
        nrf_fprintf(p_ctx,
                    "%sLogs dropped (%d)%s\r\n",
                    XINC_LOG_COLOR_CODE_RED,
                    p_params->dropped,
                    XINC_LOG_COLOR_CODE_DEFAULT);
    }

    if (!(p_params->severity == XINC_LOG_SEVERITY_INFO_RAW))
    {
        if (p_params->use_colors)
        {
            nrf_fprintf(p_ctx, "%s",
                          m_colors[nrf_log_color_id_get( p_params->module_id, p_params->severity)]);
        }
        timestamp_print(p_ctx, p_params->timestamp);
		//		printf("<%s> %s: ",severity_names[p_params->severity], nrf_log_module_name_get(p_params->module_id, false));

        nrf_fprintf(p_ctx, "<%s> ",severity_names[p_params->severity]);
    }
}

static void postfix_process(nrf_log_str_formatter_entry_params_t * p_params,
                            nrf_fprintf_ctx_t * p_ctx,
                            bool newline)
{
    if (!(p_params->severity == XINC_LOG_SEVERITY_INFO_RAW))
    {
        if (p_params->use_colors)
        {
            nrf_fprintf(p_ctx, "%s", m_colors[0]);
        }
        nrf_fprintf(p_ctx, "\r\n");
    }
    else if (newline)
    {
        nrf_fprintf(p_ctx, "\r\n");
    }
    nrf_fprintf_buffer_flush(p_ctx);
}

void nrf_log_std_entry_process(char const * p_str,
                                  uint32_t const * p_args,
                                  uint32_t nargs,
                                  nrf_log_str_formatter_entry_params_t * p_params,
                                  nrf_fprintf_ctx_t * p_ctx)
{
    bool auto_flush = p_ctx->auto_flush;
    p_ctx->auto_flush = false;
    prefix_process(p_params, p_ctx);
		
    switch (nargs)
    {
        case 0:
            nrf_fprintf(p_ctx, p_str);
            break;
        case 1:
            nrf_fprintf(p_ctx, p_str, p_args[0]);
            break;
        case 2:
            nrf_fprintf(p_ctx, p_str, p_args[0], p_args[1]);
            break;
        case 3:
            nrf_fprintf(p_ctx, p_str, p_args[0], p_args[1], p_args[2]);
            break;
        case 4:
            nrf_fprintf(p_ctx, p_str, p_args[0], p_args[1], p_args[2], p_args[3]);
            break;
        case 5:
            nrf_fprintf(p_ctx, p_str, p_args[0], p_args[1], p_args[2], p_args[3], p_args[4]);
            break;
        case 6:
            nrf_fprintf(p_ctx, p_str, p_args[0], p_args[1], p_args[2], p_args[3], p_args[4], p_args[5]);
            break;

        default:
            break;
    }

    postfix_process(p_params, p_ctx, false);
    p_ctx->auto_flush = auto_flush;
}

#define HEXDUMP_BYTES_IN_LINE 8

void nrf_log_hexdump_entry_process(uint8_t * p_data,
                                   uint32_t data_len,
                                   nrf_log_str_formatter_entry_params_t * p_params,
                                   nrf_fprintf_ctx_t * p_ctx)
{
    if (data_len > HEXDUMP_BYTES_IN_LINE)
    {
        return;
    }
    bool auto_flush = p_ctx->auto_flush;
    p_ctx->auto_flush = false;

    prefix_process(p_params, p_ctx);

    uint32_t i;

    for (i = 0; i < HEXDUMP_BYTES_IN_LINE; i++)
    {
        if (i < data_len)
        {
            nrf_fprintf(p_ctx, " %02x", p_data[i]);
        }
        else
        {
            nrf_fprintf(p_ctx, "   ");
        }
    }
    nrf_fprintf(p_ctx, "|");

    for (i = 0; i < HEXDUMP_BYTES_IN_LINE; i++)
    {
        if (i < data_len)
        {
            char c = (char)p_data[i];
            nrf_fprintf(p_ctx, "%c", ((c <= XINC_LOG_CHAR_CODE_MAX) && isprint((int)c)) ? c :'.');
        }
        else
        {
            nrf_fprintf(p_ctx, " ");
        }
    }

    postfix_process(p_params, p_ctx, true);

    p_ctx->auto_flush = auto_flush;
}

void nrf_log_str_formatter_timestamp_freq_set(uint32_t freq)
{
    m_timestamp_div = 1;
    /* There is no point to have frequency higher than 1MHz (ns are not printed) and too high
     * frequency leads to overflows in calculations.
     */
    while (freq > 1000000)
    {
        freq /= 2;
        m_timestamp_div *= 2;
    }
    m_freq = freq;
}
#endif //XINC_LOG_ENABLED
