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
#include "nrf_log_backend_serial.h"
#include "nrf_log_str_formatter.h"
#include "nrf_log_internal.h"

void nrf_log_backend_serial_put(nrf_log_backend_t const * p_backend,
                               nrf_log_entry_t * p_msg,
                               uint8_t * p_buffer,
                               uint32_t  length,
                               nrf_fprintf_fwrite tx_func)
{
    nrf_memobj_get(p_msg);

    nrf_fprintf_ctx_t fprintf_ctx = {
            .p_io_buffer = (char *)p_buffer,
            .io_buffer_size = length,
            .io_buffer_cnt = 0,
            .auto_flush = false,
            .p_user_ctx = NULL,
            .fwrite = tx_func
    };
		

    nrf_log_str_formatter_entry_params_t params;

    nrf_log_header_t header;
    size_t           memobj_offset = 0;

    nrf_memobj_read(p_msg, &header, HEADER_SIZE*sizeof(uint32_t), memobj_offset);
    memobj_offset = HEADER_SIZE*sizeof(uint32_t);

    params.timestamp = header.timestamp;
    params.module_id = header.module_id;
    params.dropped   = header.dropped;
    params.use_colors = XINC_LOG_USES_COLORS;
    /*lint -save -e438*/
    if (header.base.generic.type == HEADER_TYPE_STD)
    {
        char const * p_log_str = (char const *)((uint32_t)header.base.std.addr );
        params.severity  = (nrf_log_severity_t)header.base.std.severity;
        uint32_t nargs = header.base.std.nargs;
        uint32_t args[XINC_LOG_MAX_NUM_OF_ARGS];

        nrf_memobj_read(p_msg, args, nargs*sizeof(uint32_t), memobj_offset);
        memobj_offset += (nargs*sizeof(uint32_t));

        nrf_log_std_entry_process(p_log_str,
                                  args,
                                  nargs,
                                  &params,
                                  &fprintf_ctx);

    }
    else if (header.base.generic.type == HEADER_TYPE_HEXDUMP)
    {
        uint32_t data_len = header.base.hexdump.len;
        params.severity   = (nrf_log_severity_t)header.base.hexdump.severity;
        uint8_t data_buf[8];
        uint32_t chunk_len;
        do
        {
            chunk_len = sizeof(data_buf) > data_len ? data_len : sizeof(data_buf);
            nrf_memobj_read(p_msg, data_buf, chunk_len, memobj_offset);
            memobj_offset += chunk_len;
            data_len -= chunk_len;

            nrf_log_hexdump_entry_process(data_buf,
                                         chunk_len,
                                         &params,
                                         &fprintf_ctx);
        } while (data_len > 0);
    }
    nrf_memobj_put(p_msg);
    /*lint -restore*/
}
#endif //XINC_LOG_ENABLED
