/**
 * Copyright (c) 2014 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef XINC_DRV_WDT_H__
#define XINC_DRV_WDT_H__

#include <xincx_wdt.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xinc_drv_wdt WDT driver - legacy layer
 * @{
 * @ingroup  xinc_wdt
 *
 * @brief    A layer providing compatibility with former API.
 */

/** @brief Type definition for forwarding the new implementation. */
typedef xincx_wdt_config_t xinc_drv_wdt_config_t;

/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_WDT_DEAFULT_CONFIG  XINCX_WDT_DEAFULT_CONFIG

/** @brief Macro for forwarding the new implementation. */
#define xinc_wdt_event_handler_t     xincx_wdt_event_handler_t
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_wdt_channel_id      xincx_wdt_channel_id

/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_wdt_channel_alloc   xincx_wdt_channel_alloc
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_wdt_enable          xincx_wdt_enable
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_wdt_feed            xincx_wdt_feed

/**
 * @brief This function initializes watchdog.
 *
 * @param[in] p_config          Pointer to the structure with initial configuration. Default
 *                              configuration used if NULL.
 * @param[in] wdt_event_handler Specifies event handler provided by user.
 *
 * @note Function asserts if wdt_event_handler is NULL.
 *
 * @return NRF_SUCCESS on success, otherwise an error code.
 */
__STATIC_INLINE ret_code_t xinc_drv_wdt_init(xinc_drv_wdt_config_t const * p_config,
                                            xinc_wdt_event_handler_t      wdt_event_handler)
{
    if (p_config == NULL)
    {
        static const xincx_wdt_config_t default_config = XINCX_WDT_DEAFULT_CONFIG;
        p_config = &default_config;
    }
    return xincx_wdt_init(p_config, wdt_event_handler);
}

/** @} */

#ifdef __cplusplus
}
#endif

#endif // XINC_DRV_WDT_H__
