/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
/** @file
 *
 * @defgroup app_error Common application error handler
 * @{
 * @ingroup app_common
 *
 * @brief Common application error handler and macros for utilizing a common error handler.
 */

#ifndef APP_ERROR_H__
#define APP_ERROR_H__


#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "nrf.h"
#include "sdk_errors.h"
#include "xinchip_common.h"
#include "app_error_weak.h"
#ifdef ANT_STACK_SUPPORT_REQD
#include "ant_error.h"
#endif // ANT_STACK_SUPPORT_REQD



#ifdef __cplusplus
extern "C" {
#endif

#define XINC_FAULT_ID_SDK_RANGE_START (0x00004000) /**< The start of the range of error IDs defined in the SDK. */

/**@defgroup APP_ERROR_FAULT_IDS Fault ID types
 * @{ */
#define XINC_FAULT_ID_SDK_ERROR       (XINC_FAULT_ID_SDK_RANGE_START + 1) /**< An error stemming from a call to @ref APP_ERROR_CHECK or @ref APP_ERROR_CHECK_BOOL. The info parameter is a pointer to an @ref error_info_t variable. */
#define XINC_FAULT_ID_SDK_ASSERT      (XINC_FAULT_ID_SDK_RANGE_START + 2) /**< An error stemming from a call to ASSERT (nrf_assert.h). The info parameter is a pointer to an @ref assert_info_t variable. */
/**@} */

/**@brief Structure containing info about an error of the type @ref XINC_FAULT_ID_SDK_ERROR.
 */
typedef struct
{
    uint32_t        line_num;    /**< The line number where the error occurred. */
    uint8_t const * p_file_name; /**< The file in which the error occurred. */
    uint32_t        err_code;    /**< The error code representing the error that occurred. */
} error_info_t;

/**@brief Structure containing info about an error of the type @ref XINC_FAULT_ID_SDK_ASSERT.
 */
typedef struct
{
    uint32_t        line_num;    /**< The line number where the error occurred. */
    uint8_t const * p_file_name; /**< The file in which the error occurred. */
} assert_info_t;

/**@brief Defines required by app_error_handler assembler intructions.
 */
#define APP_ERROR_ERROR_INFO_OFFSET_LINE_NUM        (offsetof(error_info_t, line_num))
#define APP_ERROR_ERROR_INFO_OFFSET_P_FILE_NAME     (offsetof(error_info_t, p_file_name))
#define APP_ERROR_ERROR_INFO_OFFSET_ERR_CODE        (offsetof(error_info_t, err_code))
#define APP_ERROR_ERROR_INFO_SIZE                   (sizeof(error_info_t))
#define APP_ERROR_ERROR_INFO_SIZE_ALIGNED_8BYTE \
    ALIGN_NUM(APP_ERROR_ERROR_INFO_SIZE, sizeof(uint64_t))


/**@brief Function for error handling, which is called when an error has occurred.
 *
 * @param[in] error_code  Error code supplied to the handler.
 * @param[in] line_num    Line number where the handler is called.
 * @param[in] p_file_name Pointer to the file name.
 */
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name);

/**@brief Function for error handling, which is called when an error has occurred.
 *
 * @param[in] error_code  Error code supplied to the handler.
 */
void app_error_handler_bare(ret_code_t error_code);

/**@brief       Function for saving the parameters and entering an eternal loop, for debug purposes.
 *
 * @param[in] id    Fault identifier. See @ref XINC_FAULT_IDS.
 * @param[in] pc    The program counter of the instruction that triggered the fault, or 0 if
 *                  unavailable.
 * @param[in] info  Optional additional information regarding the fault. Refer to each fault
 *                  identifier for details.
 */
void app_error_save_and_stop(uint32_t id, uint32_t pc, uint32_t info);

/**@brief       Function for logging details of error and flushing logs.
 *
 * @param[in] id    Fault identifier. See @ref XINC_FAULT_IDS.
 * @param[in] pc    The program counter of the instruction that triggered the fault, or 0 if
 *                  unavailable.
 * @param[in] info  Optional additional information regarding the fault. Refer to each fault
 *                  identifier for details.
 */
void app_error_log_handle(uint32_t id, uint32_t pc, uint32_t info);


/**@brief Macro for calling error handler function.
 *
 * @param[in] ERR_CODE Error code supplied to the error handler.
 */
#ifdef DEBUG
#define APP_ERROR_HANDLER(ERR_CODE)                                    \
    do                                                                 \
    {                                                                  \
        app_error_handler((ERR_CODE), __LINE__, (uint8_t*) __FILE__);  \
    } while (0)
#else
#define APP_ERROR_HANDLER(ERR_CODE)                                    \
    do                                                                 \
    {                                                                  \
        app_error_handler_bare((ERR_CODE));                            \
    } while (0)
#endif
/**@brief Macro for calling error handler function if supplied error code any other than XINC_SUCCESS.
 *
 * @param[in] ERR_CODE Error code supplied to the error handler.
 */
#define APP_ERROR_CHECK(ERR_CODE)                           \
    do                                                      \
    {                                                       \
        const uint32_t LOCAL_ERR_CODE = (ERR_CODE);         \
        if (LOCAL_ERR_CODE != XINC_SUCCESS)                  \
        {                                                   \
            APP_ERROR_HANDLER(LOCAL_ERR_CODE);              \
        }                                                   \
    } while (0)

/**@brief Macro for calling error handler function if supplied boolean value is false.
 *
 * @param[in] BOOLEAN_VALUE Boolean value to be evaluated.
 */
#define APP_ERROR_CHECK_BOOL(BOOLEAN_VALUE)                   \
    do                                                        \
    {                                                         \
        const uint32_t LOCAL_BOOLEAN_VALUE = (BOOLEAN_VALUE); \
        if (!LOCAL_BOOLEAN_VALUE)                             \
        {                                                     \
            APP_ERROR_HANDLER(0);                             \
        }                                                     \
    } while (0)


#ifdef __cplusplus
}
#endif

#endif // APP_ERROR_H__

/** @} */
