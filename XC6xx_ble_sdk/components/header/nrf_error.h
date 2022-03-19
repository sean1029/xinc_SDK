/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

 /**
  @defgroup nrf_error SoftDevice Global Error Codes
  @{

  @brief Global Error definitions
*/

/* Header guard */
#ifndef XINC_ERROR_H__
#define XINC_ERROR_H__

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup XINC_ERRORS_BASE Error Codes Base number definitions
 * @{ */
#define XINC_ERROR_BASE_NUM      (0x0)       ///< Global error base
#define XINC_ERROR_SDM_BASE_NUM  (0x1000)    ///< SDM error base
#define XINC_ERROR_SOC_BASE_NUM  (0x2000)    ///< SoC error base
#define XINC_ERROR_STK_BASE_NUM  (0x3000)    ///< STK error base
/** @} */

#define XINC_SUCCESS                           (XINC_ERROR_BASE_NUM + 0)  ///< Successful command
#define XINC_ERROR_SVC_HANDLER_MISSING         (XINC_ERROR_BASE_NUM + 1)  ///< SVC handler is missing
#define XINC_ERROR_SOFTDEVICE_NOT_ENABLED      (XINC_ERROR_BASE_NUM + 2)  ///< SoftDevice has not been enabled
#define XINC_ERROR_INTERNAL                    (XINC_ERROR_BASE_NUM + 3)  ///< Internal Error
#define XINC_ERROR_NO_MEM                      (XINC_ERROR_BASE_NUM + 4)  ///< No Memory for operation
#define XINC_ERROR_NOT_FOUND                   (XINC_ERROR_BASE_NUM + 5)  ///< Not found
#define XINC_ERROR_NOT_SUPPORTED               (XINC_ERROR_BASE_NUM + 6)  ///< Not supported
#define XINC_ERROR_INVALID_PARAM               (XINC_ERROR_BASE_NUM + 7)  ///< Invalid Parameter
#define XINC_ERROR_INVALID_STATE               (XINC_ERROR_BASE_NUM + 8)  ///< Invalid state, operation disallowed in this state
#define XINC_ERROR_INVALID_LENGTH              (XINC_ERROR_BASE_NUM + 9)  ///< Invalid Length
#define XINC_ERROR_INVALID_FLAGS               (XINC_ERROR_BASE_NUM + 10) ///< Invalid Flags
#define XINC_ERROR_INVALID_DATA                (XINC_ERROR_BASE_NUM + 11) ///< Invalid Data
#define XINC_ERROR_DATA_SIZE                   (XINC_ERROR_BASE_NUM + 12) ///< Invalid Data size
#define XINC_ERROR_TIMEOUT                     (XINC_ERROR_BASE_NUM + 13) ///< Operation timed out
#define XINC_ERROR_NULL                        (XINC_ERROR_BASE_NUM + 14) ///< Null Pointer
#define XINC_ERROR_FORBIDDEN                   (XINC_ERROR_BASE_NUM + 15) ///< Forbidden Operation
#define XINC_ERROR_INVALID_ADDR                (XINC_ERROR_BASE_NUM + 16) ///< Bad Memory Address
#define XINC_ERROR_BUSY                        (XINC_ERROR_BASE_NUM + 17) ///< Busy
#define XINC_ERROR_CONN_COUNT                  (XINC_ERROR_BASE_NUM + 18) ///< Maximum connection count exceeded.
#define XINC_ERROR_RESOURCES                   (XINC_ERROR_BASE_NUM + 19) ///< Not enough resources for operation

#ifdef __cplusplus
}
#endif
#endif // XINC_ERROR_H__

/**
  @}
*/
