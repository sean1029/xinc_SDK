/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#include "sdk_common.h"
#if XINC_MODULE_ENABLED(XINC_STRERROR)
#include "nrf_strerror.h"

/**
 * @brief Macro for adding an entity to the description array.
 *
 * Macro that helps to create a single entity in the description array.
 */
#define XINC_STRERROR_ENTITY(mnemonic) {.code = mnemonic, .name = #mnemonic}

/**
 * @brief Array entity element that describes an error.
 */
typedef struct
{
    ret_code_t   code; /**< Error code. */
    char const * name; /**< Descriptive name (the same as the internal error mnemonic). */
}nrf_strerror_desc_t;

/**
 * @brief Unknown error code.
 *
 * The constant string used by @ref nrf_strerror_get when the error description was not found.
 */
static char const m_unknown_str[] = "Unknown error code";

/**
 * @brief Array with error codes.
 *
 * Array that describes error codes.
 *
 * @note It is required for this array to have error codes placed in ascending order.
 *       This condition is checked in automatic unit test before the release.
 */
static nrf_strerror_desc_t const nrf_strerror_array[] =
{
    XINC_STRERROR_ENTITY(XINC_SUCCESS),
    XINC_STRERROR_ENTITY(XINC_ERROR_SVC_HANDLER_MISSING),
    XINC_STRERROR_ENTITY(XINC_ERROR_SOFTDEVICE_NOT_ENABLED),
    XINC_STRERROR_ENTITY(XINC_ERROR_INTERNAL),
    XINC_STRERROR_ENTITY(XINC_ERROR_NO_MEM),
    XINC_STRERROR_ENTITY(XINC_ERROR_NOT_FOUND),
    XINC_STRERROR_ENTITY(XINC_ERROR_NOT_SUPPORTED),
    XINC_STRERROR_ENTITY(XINC_ERROR_INVALID_PARAM),
    XINC_STRERROR_ENTITY(XINC_ERROR_INVALID_STATE),
    XINC_STRERROR_ENTITY(XINC_ERROR_INVALID_LENGTH),
    XINC_STRERROR_ENTITY(XINC_ERROR_INVALID_FLAGS),
    XINC_STRERROR_ENTITY(XINC_ERROR_INVALID_DATA),
    XINC_STRERROR_ENTITY(XINC_ERROR_DATA_SIZE),
    XINC_STRERROR_ENTITY(XINC_ERROR_TIMEOUT),
    XINC_STRERROR_ENTITY(XINC_ERROR_NULL),
    XINC_STRERROR_ENTITY(XINC_ERROR_FORBIDDEN),
    XINC_STRERROR_ENTITY(XINC_ERROR_INVALID_ADDR),
    XINC_STRERROR_ENTITY(XINC_ERROR_BUSY),
#ifdef XINC_ERROR_CONN_COUNT
    XINC_STRERROR_ENTITY(XINC_ERROR_CONN_COUNT),
#endif
#ifdef XINC_ERROR_RESOURCES
    XINC_STRERROR_ENTITY(XINC_ERROR_RESOURCES),
#endif

    /* SDK Common errors */
    XINC_STRERROR_ENTITY(XINC_ERROR_MODULE_NOT_INITIALIZED),
    XINC_STRERROR_ENTITY(XINC_ERROR_MUTEX_INIT_FAILED),
    XINC_STRERROR_ENTITY(XINC_ERROR_MUTEX_LOCK_FAILED),
    XINC_STRERROR_ENTITY(XINC_ERROR_MUTEX_UNLOCK_FAILED),
    XINC_STRERROR_ENTITY(XINC_ERROR_MUTEX_COND_INIT_FAILED),
    XINC_STRERROR_ENTITY(XINC_ERROR_MODULE_ALREADY_INITIALIZED),
    XINC_STRERROR_ENTITY(XINC_ERROR_STORAGE_FULL),
    XINC_STRERROR_ENTITY(XINC_ERROR_API_NOT_IMPLEMENTED),
    XINC_STRERROR_ENTITY(XINC_ERROR_FEATURE_NOT_ENABLED),
    XINC_STRERROR_ENTITY(XINC_ERROR_IO_PENDING),

    /* TWI error codes */
//    XINC_STRERROR_ENTITY(XINC_ERROR_DRV_TWI_ERR_OVERRUN),
//    XINC_STRERROR_ENTITY(XINC_ERROR_DRV_TWI_ERR_ANACK),
//    XINC_STRERROR_ENTITY(XINC_ERROR_DRV_TWI_ERR_DNACK),

    /* IPSP error codes */
    XINC_STRERROR_ENTITY(XINC_ERROR_BLE_IPSP_RX_PKT_TRUNCATED),
    XINC_STRERROR_ENTITY(XINC_ERROR_BLE_IPSP_CHANNEL_ALREADY_EXISTS),
    XINC_STRERROR_ENTITY(XINC_ERROR_BLE_IPSP_LINK_DISCONNECTED),
    XINC_STRERROR_ENTITY(XINC_ERROR_BLE_IPSP_PEER_REJECTED)
};


char const * nrf_strerror_get(ret_code_t code)
{
    char const * p_ret = nrf_strerror_find(code);
    return (p_ret == NULL) ? m_unknown_str : p_ret;
}

char const * nrf_strerror_find(ret_code_t code)
{
    nrf_strerror_desc_t const * p_start;
    nrf_strerror_desc_t const * p_end;
    p_start = nrf_strerror_array;
    p_end   = nrf_strerror_array + ARRAY_SIZE(nrf_strerror_array);

    while (p_start < p_end)
    {
        nrf_strerror_desc_t const * p_mid = p_start + ((p_end - p_start) / 2);
        ret_code_t mid_c = p_mid->code;
        if (mid_c > code)
        {
            p_end = p_mid;
        }
        else if (mid_c < code)
        {
            p_start = p_mid + 1;
        }
        else
        {
            return p_mid->name;
        }
    }
    return NULL;
}

#endif /* XINC_STRERROR enabled */
