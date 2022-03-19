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
 * @addtogroup xinc_log Logger module
 * @ingroup    app_common
 *
 * @defgroup xinc_log_backend_serial Common part of serial backends
 * @{
 * @ingroup  xinc_log
 * @brief    The xinc_log serial backend common put function.
 */


#include "xinc_log_backend_interface.h"
#include "xinc_fprintf.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A function for processing logger entry with simple serial interface as output.
 *
 *
 */
void xinc_log_backend_serial_put(xinc_log_backend_t const * p_backend,
                               xinc_log_entry_t * p_msg,
                               uint8_t * p_buffer,
                               uint32_t  length,
                               xinc_fprintf_fwrite tx_func);

#endif //XINC_LOG_BACKEND_SERIAL_H

#ifdef __cplusplus
}
#endif

/** @} */
