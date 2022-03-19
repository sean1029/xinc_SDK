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

#ifndef XINC_WDT_H__
#define XINC_WDT_H__

#include <nrfx.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xinc_wdt_hal WDT HAL
 * @{
 * @ingroup xinc_wdt
 * @brief   Hardware access layer for managing the Watchdog Timer (WDT) peripheral.
 */

/** @brief WDT register reload value. */
#define XINC_WDT_RR_VALUE       0xAUL /* Fixed value; should not be modified. */


/** @brief WDT behavior in the SLEEP or HALT CPU modes. */
typedef enum
{
    XINC_WDT_MODE_RUN_0        = WDT_CR_RMOD_Mode0,                         /**< WDT will run on mode 0. */
    XINC_WDT_MODE_RUN_1         = WDT_CR_RMOD_Mode1,                        /**< WDT will run on mode 1. */                                
} xinc_wdt_mode_t;

/** @brief WDT behavior in the SLEEP or HALT CPU modes. */
enum
{
    XINC_WDT_RPL_CLK2           = WDT_CR_RPL_2pclk,                    /**< 2 pclk 时钟周期 */
    XINC_WDT_RPL_CLK4           = WDT_CR_RPL_4pclk,                    /**< 4 pclk 时钟周期 */   
    XINC_WDT_RPL_CLK8           = WDT_CR_RPL_8pclk,                    /**< 8 pclk 时钟周期 */
    XINC_WDT_RPL_CLK16          = WDT_CR_RPL_16pclk,                   /**< 16 pclk 时钟周期 */  
    XINC_WDT_RPL_CLK32          = WDT_CR_RPL_32pclk,                   /**< 32 pclk 时钟周期 */
    XINC_WDT_RPL_CLK64          = WDT_CR_RPL_64pclk,                    /**< 64 pclk 时钟周期 */  
    XINC_WDT_RPL_CLK128         = WDT_CR_RPL_128pclk,                   /**< 128 pclk 时钟周期 */
    XINC_WDT_RPL_CLK256         = WDT_CR_RPL_256pclk,                    /**< 256 pclk 时钟周期 */      
} ;


/**
 * @brief Function for configuring the watchdog behavior when the CPU is sleeping or halted.
 *
 * @param behaviour Watchdog behavior mode.
 */
__STATIC_INLINE void xinc_wdt_mode_set(xinc_wdt_mode_t mode);

/**
 * @brief Function for enabling the specified interrupt.
 *
 * @param[in] int_mask Interrupt.
 */
__STATIC_INLINE void xinc_wdt_enable(uint32_t en);


/**
 * @brief Function for retrieving the watchdog status.
 *
 * @retval true  The watchdog is started.
 * @retval false The watchdog is not started.
 */
__STATIC_INLINE bool xinc_wdt_started(void);


/**
 * @brief Function for setting the watchdog reload value.
 *
 * @param[in] reload_value Watchdog counter initial value.
 */
__STATIC_INLINE void xinc_wdt_reload_value_set(uint32_t reload_value);

/**
 * @brief Function for retrieving the watchdog reload value.
 *
 * @return Reload value.
 */
__STATIC_INLINE uint32_t xinc_wdt_reload_value_get(void);


/**
 * @brief Function for setting a specific reload request register.
 *
 * @param[in]  rr_register       Reload request register to set.
 */
__STATIC_INLINE void xinc_wdt_reload_request_set(uint32_t value);

#ifndef SUPPRESS_INLINE_IMPLEMENTATION

__STATIC_INLINE void xinc_wdt_mode_set(xinc_wdt_mode_t mode)
{
    uint32_t reg = XINC_WDT->CR;  
    reg &= ~(WDT_CR_RMOD_Msk);
    reg |= (mode << WDT_CR_RMOD_Pos);
    XINC_WDT->CR = reg;
}

__STATIC_INLINE void xinc_wdt_enable(uint32_t en)
{

	XINC_WDT->CR |= en;
	XINC_WDT->CRR = WDT_CRR_CRR_Enable;
}

__STATIC_INLINE bool xinc_wdt_enable_check(uint32_t int_mask)
{
    return (bool)(XINC_WDT->CR & WDT_CR_WDT_EN_Msk);// 0x01 << 0,wdt en
}

__STATIC_INLINE void xinc_wdt_int_disable(uint32_t int_mask)
{

}


__STATIC_INLINE bool xinc_wdt_started(void)
{
    return (bool)(XINC_WDT->CR & WDT_CR_WDT_EN_Msk);
}



__STATIC_INLINE void xinc_wdt_reload_value_set(uint32_t reload_value)
{
    XINC_WDT->TORR = reload_value;
}

__STATIC_INLINE uint32_t xinc_wdt_reload_value_get(void)
{
    return (uint32_t)XINC_WDT->TORR;
}



__STATIC_INLINE void xinc_wdt_reload_request_set(uint32_t value)
{
	XINC_WDT->CRR = value;
}

#endif // SUPPRESS_INLINE_IMPLEMENTATION

/** @} */

#ifdef __cplusplus
}
#endif

#endif // XINC_WDT_H__
