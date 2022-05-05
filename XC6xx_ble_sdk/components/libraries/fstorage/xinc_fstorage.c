/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#include "sdk_config.h"

#include "xinc_fstorage.h"
#include <stddef.h>
#include <stdint.h>
#include "sdk_errors.h"
#include "sdk_macros.h"
#include "xinc_section.h"
#include <stdio.h>


/* Create the section "fs_data". */
XINC_SECTION_DEF(fs_data, xinc_fstorage_t);


/**@brief   Macro to handle user input validation.
 *
 * If @p _cond evaluates to true, does nothing. Otherwise,
 * if the XINC_FSTORAGE_PARAM_CHECK_DISABLED is not set, logs an error message and returns @p _err.
 * If the XINC_FSTORAGE_PARAM_CHECK_DISABLED is set, behaves like the @ref ASSERT macro.
 *
 * Parameter checking implemented using this macro can be optionally turned off for release code.
 * Only disable runtime parameter checks if size if a major concern.
 *
 * @param _cond     The condition to be evaluated.
 * @param _err      The error code to be returned.
 */
#define XINC_FSTORAGE_PARAM_CHECK(_cond, _err)                                                       \
    XINC_PARAM_CHECK(XINC_FSTORAGE, _cond, _err, XINC_LOG_ERROR)
		
		
static bool addr_is_aligned32(uint32_t addr);
static bool addr_is_page_aligned(xinc_fstorage_t const * p_fs, uint32_t addr);
static bool addr_is_within_bounds(xinc_fstorage_t const * p_fs, uint32_t addr, uint32_t len);

ret_code_t xinc_fstorage_init(xinc_fstorage_t     * p_fs,
                             xinc_fstorage_api_t * p_api,
                             void               * p_param)
{
    XINC_FSTORAGE_PARAM_CHECK(p_fs,  XINC_ERROR_NULL);
    XINC_FSTORAGE_PARAM_CHECK(p_api, XINC_ERROR_NULL);

    p_fs->p_api = p_api;

    return (p_fs->p_api)->init(p_fs, p_param);
}


ret_code_t xinc_fstorage_uninit(xinc_fstorage_t * p_fs,
                               void           * p_param)
{
    ret_code_t rc;

    XINC_FSTORAGE_PARAM_CHECK(p_fs,        XINC_ERROR_NULL);
    XINC_FSTORAGE_PARAM_CHECK(p_fs->p_api, XINC_ERROR_INVALID_STATE);

    rc = (p_fs->p_api)->uninit(p_fs, p_param);

    /* Uninitialize the API. */
    p_fs->p_api        = NULL;
    p_fs->p_flash_info = NULL;

    return rc;
}

ret_code_t xinc_fstorage_read(xinc_fstorage_t const * p_fs,
                             uint32_t               src,
                             void                 * p_dest,
                             uint32_t               len)
{
    XINC_FSTORAGE_PARAM_CHECK(p_fs,        XINC_ERROR_NULL);
    XINC_FSTORAGE_PARAM_CHECK(p_dest,      XINC_ERROR_NULL);
    XINC_FSTORAGE_PARAM_CHECK(p_fs->p_api, XINC_ERROR_INVALID_STATE);
    XINC_FSTORAGE_PARAM_CHECK(len,         XINC_ERROR_INVALID_LENGTH);

    /* Source addres must be word-aligned. */
    XINC_FSTORAGE_PARAM_CHECK(addr_is_aligned32(src),                XINC_ERROR_INVALID_ADDR);
    XINC_FSTORAGE_PARAM_CHECK(addr_is_within_bounds(p_fs, src, len), XINC_ERROR_INVALID_ADDR);
    
    return (p_fs->p_api)->read(p_fs, src, p_dest, len);
}


ret_code_t xinc_fstorage_write(xinc_fstorage_t const * p_fs,
                              uint32_t               dest,
                              void           const * p_src,
                              uint32_t               len,
                              void                 * p_context)
{
    XINC_FSTORAGE_PARAM_CHECK(p_fs,        XINC_ERROR_NULL);
    XINC_FSTORAGE_PARAM_CHECK(p_src,       XINC_ERROR_NULL);
    XINC_FSTORAGE_PARAM_CHECK(p_fs->p_api, XINC_ERROR_INVALID_STATE);
    XINC_FSTORAGE_PARAM_CHECK(len,         XINC_ERROR_INVALID_LENGTH);

    /* Length must be a multiple of the program unit. */
    XINC_FSTORAGE_PARAM_CHECK(!(len % p_fs->p_flash_info->program_unit), XINC_ERROR_INVALID_LENGTH);

    /* Source and destination addresses must be word-aligned. */
    XINC_FSTORAGE_PARAM_CHECK(addr_is_aligned32(dest),                XINC_ERROR_INVALID_ADDR);
    XINC_FSTORAGE_PARAM_CHECK(addr_is_aligned32((uint32_t)p_src),     XINC_ERROR_INVALID_ADDR);
    XINC_FSTORAGE_PARAM_CHECK(addr_is_within_bounds(p_fs, dest, len), XINC_ERROR_INVALID_ADDR);

    return (p_fs->p_api)->write(p_fs, dest, p_src, len, p_context);
}


ret_code_t xinc_fstorage_erase(xinc_fstorage_t const * p_fs,
                              uint32_t               page_addr,
                              uint32_t               len,
                              void                 * p_context)
{
    XINC_FSTORAGE_PARAM_CHECK(p_fs,        XINC_ERROR_NULL);
    XINC_FSTORAGE_PARAM_CHECK(p_fs->p_api, XINC_ERROR_INVALID_STATE);
    XINC_FSTORAGE_PARAM_CHECK(len,         XINC_ERROR_INVALID_LENGTH);

    /* Address must be aligned to a page boundary. */
    XINC_FSTORAGE_PARAM_CHECK(addr_is_page_aligned(p_fs, page_addr), XINC_ERROR_INVALID_ADDR);

    XINC_FSTORAGE_PARAM_CHECK(
        addr_is_within_bounds(p_fs, page_addr, (len * p_fs->p_flash_info->erase_unit)),
        XINC_ERROR_INVALID_ADDR
    );

    return (p_fs->p_api)->erase(p_fs, page_addr, len, p_context);
}

ret_code_t xinc_fstorage_space_init(xinc_fstorage_t const * p_fs,
                              uint32_t               page_addr,
                              uint32_t               len,
                              void                 * p_context)
{
    XINC_FSTORAGE_PARAM_CHECK(p_fs,        XINC_ERROR_NULL);
    XINC_FSTORAGE_PARAM_CHECK(p_fs->p_api, XINC_ERROR_INVALID_STATE);
    XINC_FSTORAGE_PARAM_CHECK(len,         XINC_ERROR_INVALID_LENGTH);

    /* Address must be aligned to a page boundary. */
    XINC_FSTORAGE_PARAM_CHECK(addr_is_page_aligned(p_fs, page_addr), XINC_ERROR_INVALID_ADDR);

    XINC_FSTORAGE_PARAM_CHECK(
        addr_is_within_bounds(p_fs, page_addr, (len * p_fs->p_flash_info->erase_unit)),
        XINC_ERROR_INVALID_ADDR
    );

    return (p_fs->p_api)->space_init(p_fs, page_addr, len, p_context);
}


uint8_t const * xinc_fstorage_rmap(xinc_fstorage_t const * p_fs, uint32_t addr)
{
    if ((p_fs == NULL) || (p_fs->p_api == NULL))
    {
        return NULL;
    }

    return (p_fs->p_api)->rmap(p_fs, addr);
}


uint8_t * xinc_fstorage_wmap(xinc_fstorage_t const * p_fs, uint32_t addr)
{
    if ((p_fs == NULL) || (p_fs->p_api == NULL))
    {
        return NULL;
    }

    return (p_fs->p_api)->wmap(p_fs, addr);
}

bool xinc_fstorage_is_busy(xinc_fstorage_t const * p_fs)
{
    /* If a NULL instance is provided, return true if any instance is busy.
     * Uninitialized instances are considered not busy. */
    if ((p_fs == NULL) || (p_fs->p_api == NULL))
    {
        for (uint32_t i = 0; i < XINC_FSTORAGE_INSTANCE_CNT; i++)
        {
            p_fs = XINC_FSTORAGE_INSTANCE_GET(i);    /* cannot be NULL. */
            if (p_fs->p_api != NULL)
            {
                /* p_api->is_busy() cannot be NULL. */
                if (p_fs->p_api->is_busy(p_fs))
                {
                    return true;
                }
            }
        }

        return false;
    }

    return p_fs->p_api->is_busy(p_fs);
}


static bool addr_is_within_bounds(xinc_fstorage_t const * p_fs,
                                  uint32_t               addr,
                                  uint32_t               len)
{
    return (   (addr           >= p_fs->start_addr)
            && (addr + len - 1 <= p_fs->end_addr));
}


static bool addr_is_aligned32(uint32_t addr)
{
    return !(addr & 0x03);
}


static bool addr_is_page_aligned(xinc_fstorage_t const * p_fs,
                                 uint32_t               addr)
{
    return (addr & (p_fs->p_flash_info->erase_unit - 1)) == 0;
}
