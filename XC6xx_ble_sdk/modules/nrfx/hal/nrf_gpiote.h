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

#ifndef NRF_GPIOTE_H__
#define NRF_GPIOTE_H__

#include <nrfx.h>
#include "nrf52840.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
* @defgroup nrf_gpiote_hal GPIOTE HAL
* @{
* @ingroup nrf_gpiote
* @brief   Hardware access layer (HAL) for managing the GPIOTE peripheral.
*/


 /** @brief Polarity for the GPIO channel. */
typedef enum
{
  NRF_GPIOTE_POLARITY_LOTOHI = GPIOTE_CONFIG_POLARITY_LoToHi,       ///<  Low to high.
  NRF_GPIOTE_POLARITY_HITOLO = GPIOTE_CONFIG_POLARITY_HiToLo,       ///<  High to low.
  NRF_GPIOTE_POLARITY_TOGGLE = GPIOTE_CONFIG_POLARITY_Toggle        ///<  Toggle.
} nrf_gpiote_polarity_t;

/** @brief Initial output value for the GPIO channel. */
typedef enum
{
  NRF_GPIOTE_INITIAL_VALUE_LOW  = GPIOTE_CONFIG_OUTINIT_Low,       ///<  Low to high.
  NRF_GPIOTE_INITIAL_VALUE_HIGH = GPIOTE_CONFIG_OUTINIT_High       ///<  High to low.
} nrf_gpiote_outinit_t;


/** @brief GPIOTE interrupts. */

/**
 * @brief Function for enabling interrupts.
 *
 * @param[in] mask Mask of interrupts to be enabled.
 */
__STATIC_INLINE void nrf_gpiote_int_enable(uint32_t mask);

/**
 * @brief Function for disabling interrupts.
 *
 * @param[in] mask Mask of interrupts to be disabled.
 */
__STATIC_INLINE void nrf_gpiote_int_disable(uint32_t mask);

/**
 * @brief Function for checking if interrupts are enabled.
 *
 * @param[in] mask Mask of interrupts to be checked.
 *
 * @return Mask with enabled interrupts.
 */
__STATIC_INLINE uint32_t nrf_gpiote_int_is_enabled(uint32_t mask);



/** @} */

#ifdef __cplusplus
}
#endif

#endif
