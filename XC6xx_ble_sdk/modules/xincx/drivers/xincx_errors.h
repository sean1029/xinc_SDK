/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINCX_ERRORS_H__
#define XINCX_ERRORS_H__

#if !XINCX_CHECK(XINCX_CUSTOM_ERROR_CODES)

/**
 * @defgroup xincx_error_codes Global Error Codes
 * @{
 * @ingroup xincx
 *
 * @brief Global error code definitions.
 */

/** @brief Base number of error codes. */
#define XINCX_ERROR_BASE_NUM         0x0BAD0000

/** @brief Base number of driver error codes. */
#define XINCX_ERROR_DRIVERS_BASE_NUM (XINCX_ERROR_BASE_NUM + 0x10000)

/** @brief Enumerated type for error codes. */
typedef enum {
    XINCX_SUCCESS                    = (XINCX_ERROR_BASE_NUM + 0),  ///< Operation performed successfully.
    XINCX_ERROR_INTERNAL             = (XINCX_ERROR_BASE_NUM + 1),  ///< Internal error.
    XINCX_ERROR_NO_MEM               = (XINCX_ERROR_BASE_NUM + 2),  ///< No memory for operation.
    XINCX_ERROR_NOT_SUPPORTED        = (XINCX_ERROR_BASE_NUM + 3),  ///< Not supported.
    XINCX_ERROR_INVALID_PARAM        = (XINCX_ERROR_BASE_NUM + 4),  ///< Invalid parameter.
    XINCX_ERROR_INVALID_STATE        = (XINCX_ERROR_BASE_NUM + 5),  ///< Invalid state, operation disallowed in this state.
    XINCX_ERROR_INVALID_LENGTH       = (XINCX_ERROR_BASE_NUM + 6),  ///< Invalid length.
    XINCX_ERROR_TIMEOUT              = (XINCX_ERROR_BASE_NUM + 7),  ///< Operation timed out.
    XINCX_ERROR_FORBIDDEN            = (XINCX_ERROR_BASE_NUM + 8),  ///< Operation is forbidden.
    XINCX_ERROR_NULL                 = (XINCX_ERROR_BASE_NUM + 9),  ///< Null pointer.
    XINCX_ERROR_INVALID_ADDR         = (XINCX_ERROR_BASE_NUM + 10), ///< Bad memory address.
    XINCX_ERROR_BUSY                 = (XINCX_ERROR_BASE_NUM + 11), ///< Busy.
    XINCX_ERROR_ALREADY_INITIALIZED  = (XINCX_ERROR_BASE_NUM + 12), ///< Module already initialized.

    XINCX_ERROR_DRV_I2C_ERR_OVERRUN  = (XINCX_ERROR_DRIVERS_BASE_NUM + 0), ///< I2C error: Overrun.
    XINCX_ERROR_DRV_I2C_ERR_ANACK    = (XINCX_ERROR_DRIVERS_BASE_NUM + 1), ///< I2C error: Address not acknowledged.
    XINCX_ERROR_DRV_I2C_ERR_DNACK    = (XINCX_ERROR_DRIVERS_BASE_NUM + 2)  ///< I2C error: Data not acknowledged.
} xincx_err_t;

/** @} */

#endif // !XINCX_CHECK(XINCX_CUSTOM_ERROR_CODES)

#endif // XINCX_ERRORS_H__
