/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#ifndef XINC_LOG_BACKEND_SERIAL_H
#define XINC_LOG_BACKEND_SERIAL_H
/**@file
 * @addtogroup nrf_log Logger module
 * @ingroup    app_common
 *
 * @defgroup nrf_log_backend_serial Common part of serial backends
 * @{
 * @ingroup  nrf_log
 * @brief    The nrf_log serial backend common put function.
 */


#include "nrf_log_backend_interface.h"
#include "nrf_fprintf.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A function for processing logger entry with simple serial interface as output.
 *
 *
 */
void nrf_log_backend_serial_put(nrf_log_backend_t const * p_backend,
                               nrf_log_entry_t * p_msg,
                               uint8_t * p_buffer,
                               uint32_t  length,
                               nrf_fprintf_fwrite tx_func);

#endif //XINC_LOG_BACKEND_SERIAL_H

#ifdef __cplusplus
}
#endif

/** @} */
