/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include "nrf_ringbuf.h"
#include "app_util_platform.h"
#include "nrf_assert.h"

#define WR_OFFSET 0
#define RD_OFFSET 1

void nrf_ringbuf_init(nrf_ringbuf_t const * p_ringbuf)
{
    p_ringbuf->p_cb->wr_idx = 0;
    p_ringbuf->p_cb->rd_idx = 0;
    p_ringbuf->p_cb->tmp_rd_idx = 0;
    p_ringbuf->p_cb->tmp_wr_idx = 0;
    p_ringbuf->p_cb->rd_flag   = 0;
    p_ringbuf->p_cb->wr_flag   = 0;
}

ret_code_t nrf_ringbuf_alloc(nrf_ringbuf_t const * p_ringbuf, uint8_t * * pp_data, size_t * p_length, bool start)
{
    ASSERT(pp_data);
    ASSERT(p_length);

    if (start)
    {
        if (nrf_atomic_flag_set_fetch(&p_ringbuf->p_cb->wr_flag))
        {
            return XINC_ERROR_BUSY;
        }
    }

    if (p_ringbuf->p_cb->tmp_wr_idx - p_ringbuf->p_cb->rd_idx == p_ringbuf->bufsize_mask + 1)
    {
        *p_length = 0;
				printf("ringbuf_alloc err\r\n");
        if (start)
        {
            UNUSED_RETURN_VALUE(nrf_atomic_flag_clear(&p_ringbuf->p_cb->wr_flag));
        }
        return XINC_SUCCESS;
    }

    uint32_t wr_idx = p_ringbuf->p_cb->tmp_wr_idx & p_ringbuf->bufsize_mask;
    uint32_t rd_idx = p_ringbuf->p_cb->rd_idx & p_ringbuf->bufsize_mask;
    uint32_t available = (wr_idx >= rd_idx) ? p_ringbuf->bufsize_mask + 1 - wr_idx :
            p_ringbuf->p_cb->rd_idx -  (p_ringbuf->p_cb->tmp_wr_idx - (p_ringbuf->bufsize_mask + 1));
    *p_length = *p_length < available ? *p_length : available;
    *pp_data = &p_ringbuf->p_buffer[wr_idx];
    p_ringbuf->p_cb->tmp_wr_idx += *p_length;

    return XINC_SUCCESS;
}

ret_code_t nrf_ringbuf_put(nrf_ringbuf_t const * p_ringbuf, size_t length)
{
    uint32_t available = p_ringbuf->bufsize_mask + 1 -
            (p_ringbuf->p_cb->wr_idx -  p_ringbuf->p_cb->rd_idx);
    if (length > available)
    {
        return XINC_ERROR_NO_MEM;
    }

    p_ringbuf->p_cb->wr_idx    += length;
    p_ringbuf->p_cb->tmp_wr_idx = p_ringbuf->p_cb->wr_idx;
    if (nrf_atomic_flag_clear_fetch(&p_ringbuf->p_cb->wr_flag) == 0)
    {
        /* Flag was already cleared. Suggests misuse. */
        return XINC_ERROR_INVALID_STATE;
    }
    return XINC_SUCCESS;
}

ret_code_t nrf_ringbuf_cpy_put(nrf_ringbuf_t const * p_ringbuf,
                               uint8_t const * p_data,
                               size_t * p_length)
{
    ASSERT(p_data);
    ASSERT(p_length);

    if (nrf_atomic_flag_set_fetch(&p_ringbuf->p_cb->wr_flag))
    {
        return XINC_ERROR_BUSY;
    }

    uint32_t available = p_ringbuf->bufsize_mask + 1 -
                                (p_ringbuf->p_cb->wr_idx -  p_ringbuf->p_cb->rd_idx);
    *p_length = available > *p_length ? *p_length : available;
    size_t   length        = *p_length;
    uint32_t masked_wr_idx = (p_ringbuf->p_cb->wr_idx & p_ringbuf->bufsize_mask);
    uint32_t trail         = p_ringbuf->bufsize_mask + 1 - masked_wr_idx;

    if (length > trail)
    {
        memcpy(&p_ringbuf->p_buffer[masked_wr_idx], p_data, trail);
        length -= trail;
        masked_wr_idx = 0;
        p_data += trail;
    }
    memcpy(&p_ringbuf->p_buffer[masked_wr_idx], p_data, length);
    p_ringbuf->p_cb->wr_idx += *p_length;
    p_ringbuf->p_cb->tmp_wr_idx = p_ringbuf->p_cb->wr_idx;

    UNUSED_RETURN_VALUE(nrf_atomic_flag_clear(&p_ringbuf->p_cb->wr_flag));

    return XINC_SUCCESS;
}

ret_code_t nrf_ringbuf_get(nrf_ringbuf_t const * p_ringbuf, uint8_t * * pp_data, size_t * p_length, bool start)
{
    ASSERT(pp_data);
    ASSERT(p_length);

    if (start)
    {
        if (nrf_atomic_flag_set_fetch(&p_ringbuf->p_cb->rd_flag))
        {
            return XINC_ERROR_BUSY;
        }
    }

    uint32_t available = p_ringbuf->p_cb->wr_idx - p_ringbuf->p_cb->tmp_rd_idx;
    if (available == 0)
    {
        *p_length = 0;
        if (start)
        {
            UNUSED_RETURN_VALUE(nrf_atomic_flag_clear(&p_ringbuf->p_cb->rd_flag));
        }
        return XINC_SUCCESS;
    }

    uint32_t masked_tmp_rd_idx = p_ringbuf->p_cb->tmp_rd_idx & p_ringbuf->bufsize_mask;
    uint32_t masked_wr_idx     = p_ringbuf->p_cb->wr_idx & p_ringbuf->bufsize_mask;

    if ((masked_wr_idx > masked_tmp_rd_idx) && (available < *p_length))
    {
        *p_length = available;
    }
    else if (masked_wr_idx <= masked_tmp_rd_idx)
    {
        uint32_t trail = p_ringbuf->bufsize_mask + 1 - masked_tmp_rd_idx;
        if (*p_length > trail)
        {
            *p_length = trail;
        }
    }
    *pp_data = &p_ringbuf->p_buffer[masked_tmp_rd_idx];
    p_ringbuf->p_cb->tmp_rd_idx += *p_length;

    return XINC_SUCCESS;
}

ret_code_t nrf_ringbuf_cpy_get(nrf_ringbuf_t const * p_ringbuf,
                               uint8_t * p_data,
                               size_t * p_length)
{
    ASSERT(p_data);
    ASSERT(p_length);

    if (nrf_atomic_flag_set_fetch(&p_ringbuf->p_cb->rd_flag))
    {
       return XINC_ERROR_BUSY;
    }

    uint32_t available = p_ringbuf->p_cb->wr_idx -  p_ringbuf->p_cb->rd_idx;
    *p_length = available > *p_length ? *p_length : available;
    size_t   length        = *p_length;
    uint32_t masked_rd_idx = (p_ringbuf->p_cb->rd_idx & p_ringbuf->bufsize_mask);
    uint32_t masked_wr_idx = (p_ringbuf->p_cb->wr_idx & p_ringbuf->bufsize_mask);
    uint32_t trail         = (masked_wr_idx > masked_rd_idx) ? masked_wr_idx - masked_rd_idx :
                                                      p_ringbuf->bufsize_mask + 1 - masked_rd_idx;

    if (length > trail)
    {
        memcpy(p_data, &p_ringbuf->p_buffer[masked_rd_idx], trail);
        length -= trail;
        masked_rd_idx = 0;
        p_data += trail;
    }
    memcpy(p_data, &p_ringbuf->p_buffer[masked_rd_idx], length);
    p_ringbuf->p_cb->rd_idx += *p_length;
    p_ringbuf->p_cb->tmp_rd_idx = p_ringbuf->p_cb->rd_idx;

    UNUSED_RETURN_VALUE(nrf_atomic_flag_clear(&p_ringbuf->p_cb->rd_flag));

    return XINC_SUCCESS;
}

ret_code_t nrf_ringbuf_free(nrf_ringbuf_t const * p_ringbuf, size_t length)
{
    uint32_t available = (p_ringbuf->p_cb->wr_idx -  p_ringbuf->p_cb->rd_idx);
    if (length > available)
    {
        return XINC_ERROR_NO_MEM;
    }

    p_ringbuf->p_cb->rd_idx    += length;
    p_ringbuf->p_cb->tmp_rd_idx = p_ringbuf->p_cb->rd_idx;
    UNUSED_RETURN_VALUE(nrf_atomic_flag_clear(&p_ringbuf->p_cb->rd_flag));

    return XINC_SUCCESS;
}
