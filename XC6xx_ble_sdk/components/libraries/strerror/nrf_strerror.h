/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

/**
 * @defgroup nrf_strerror Error code to string converter
 * @ingroup app_common
 *
 * @brief Module for converting error code into a printable string.
 * @{
 */
#ifndef NRF_STRERROR_H__
#define NRF_STRERROR_H__

#include "sdk_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Function for getting a printable error string.
 *
 * @param code Error code to convert.
 *
 * @note This function cannot fail.
 *       For the function that may fail with error translation, see @ref nrf_strerror_find.
 *
 * @return Pointer to the printable string.
 *         If the string is not found,
 *         it returns a simple string that says that the error is unknown.
 */
char const * nrf_strerror_get(ret_code_t code);

/**
 * @brief Function for finding a printable error string.
 *
 * This function gets the error string in the same way as @ref nrf_strerror_get,
 * but if the string is not found, it returns NULL.
 *
 * @param code  Error code to convert.
 * @return      Pointer to the printable string.
 *              If the string is not found, NULL is returned.
 */
char const * nrf_strerror_find(ret_code_t code);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NRF_STRERROR_H__ */
