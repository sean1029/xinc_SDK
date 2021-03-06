/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#ifndef APP_ERROR_WEAK_H__
#define APP_ERROR_WEAK_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @file
 *
 * @defgroup app_error Common application error handler
 * @{
 * @ingroup app_common
 *
 * @brief Common application error handler.
 */

/**@brief       Callback function for errors, asserts, and faults.
 *
 * @details     This function is called every time an error is raised in app_error, xinc_assert, or
 *              in the SoftDevice. Information about the error can be found in the @p info
 *              parameter.
 *
 *              See also @ref xinc_fault_handler_t for more details.
 *
 * @note        The function is implemented as weak so that it can be redefined by a custom error
 *              handler when needed.
 *
 * @param[in] id    Fault identifier. See @ref XINC_FAULT_IDS.
 * @param[in] pc    The program counter of the instruction that triggered the fault, or 0 if
 *                  unavailable.
 * @param[in] info  Optional additional information regarding the fault. The value of the @p id
 *                  parameter dictates how to interpret this parameter. Refer to the documentation
 *                  for each fault identifier (@ref XINC_FAULT_IDS and @ref APP_ERROR_FAULT_IDS) for
 *                  details about interpreting @p info.
 */
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info);


/** @} */


#ifdef __cplusplus
}
#endif

#endif // APP_ERROR_WEAK_H__
