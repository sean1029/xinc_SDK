/**
 * Copyright (c) 2015 - 2021, Nordic Semiconductor ASA
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
#ifndef NRF_DRV_GPIOTE_H__
#define NRF_DRV_GPIOTE_H__

#include <xincx_gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xinc_drv_gpio GPIOTE driver - legacy layer
 * @{
 * @ingroup xinc_gpio
 * @brief Layer providing compatibility with the former API.
 */

/** @brief Type definition for forwarding the new implementation. */
typedef xincx_gpio_in_config_t xinc_drv_gpio_in_config_t;
/** @brief Type definition for forwarding the new implementation. */
typedef xincx_gpio_pin_t xinc_drv_gpio_pin_t;
/** @brief Type definition for forwarding the new implementation. */
typedef xincx_gpio_out_config_t xinc_drv_gpio_out_config_t;
/** @brief Type definition for forwarding the new implementation. */
typedef xincx_gpio_evt_handler_t xinc_drv_gpio_evt_handler_t;

/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_gpio_init               xincx_gpio_init
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_gpio_is_init            xincx_gpio_is_init
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_gpio_uninit             xincx_gpio_uninit
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_gpio_out_init           xincx_gpio_out_init
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_gpio_out_uninit         xincx_gpio_out_uninit
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_gpio_out_set            xincx_gpio_out_set
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_gpio_out_clear          xincx_gpio_out_clear
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_gpio_out_toggle         xincx_gpio_out_toggle
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_gpio_in_init            xincx_gpio_in_init
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_gpio_in_uninit          xincx_gpio_in_uninit
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_gpio_in_is_set          xincx_gpio_in_is_set


/** @} */

#ifdef __cplusplus
}
#endif

#endif //NRF_DRV_GPIOTE_H__
