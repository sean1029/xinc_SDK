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

#ifndef XINCX_PWM_H__
#define XINCX_PWM_H__

#include <nrfx.h>
#include <hal/xinc_pwm.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xincx_pwm PWM driver
 * @{
 * @ingroup xinc_pwm
 * @brief   Pulse Width Modulation (PWM) peripheral driver.
 */

/** @brief PWM driver instance data structure. */
typedef struct
{
    XINC_PWM_Type       *p_reg;  ///< Pointer to the structure with PWM peripheral instance registers.
    XINC_CPR_CTL_Type   *p_cpr; 
    uint8_t             id;
    uint8_t             drv_inst_idx; ///< Index of the driver instance. For internal use only.
    uint8_t             output_pin; ///< Pin numbers for individual output ./**< Use @ref XINCX_PWM_PIN_NOT_USED */
    uint8_t             output_inv_pin;					///< Pin numbers for individual output(optional )
} xincx_pwm_t;

//#if NRFX_CHECK(XINCX_PWM2_ENABLED)
//#define    XINCX_PWM_DEFAULT_CONFIG_OUT2_INV_PIN (0xFF)
//#endif
//#if NRFX_CHECK(XINCX_PWM3_ENABLED)
//#define    XINCX_PWM_DEFAULT_CONFIG_OUT3_INV_PIN (0xFF)
//#endif
//#if NRFX_CHECK(XINCX_PWM4_ENABLED)
//#define    XINCX_PWM_DEFAULT_CONFIG_OUT4_INV_PIN (0xFF)
//#endif
//#if NRFX_CHECK(XINCX_PWM5_ENABLED)
//#define    XINCX_PWM_DEFAULT_CONFIG_OUT5_INV_PIN (0xFF)
//#endif

/** @brief Macro for creating a PWM driver instance. */
#define XINCX_PWM_INSTANCE(Id)                               \
{                                                           \
    .p_reg  = NRFX_CONCAT_2(XINC_PWM, Id),             \
    .p_cpr            = XINC_CPR,											\
    .id 		= Id,																				\
    .drv_inst_idx = NRFX_CONCAT_3(XINCX_PWM, Id, _INST_IDX), \
    .output_pin  = NRFX_CONCAT_3(XINCX_PWM_DEFAULT_CONFIG_OUT,Id,_PIN),                   \
    .output_inv_pin  = NRFX_CONCAT_3(XINCX_PWM_DEFAULT_CONFIG_OUT,Id,_INV_PIN),                   \
}

#ifndef __NRFX_DOXYGEN__
enum {
#if NRFX_CHECK(XINCX_PWM0_ENABLED)
    XINCX_PWM0_INST_IDX,
#endif
#if NRFX_CHECK(XINCX_PWM1_ENABLED)
    XINCX_PWM1_INST_IDX,
#endif
#if NRFX_CHECK(XINCX_PWM2_ENABLED)
    XINCX_PWM2_INST_IDX,
#endif
#if NRFX_CHECK(XINCX_PWM3_ENABLED)
    XINCX_PWM3_INST_IDX,
#endif
#if NRFX_CHECK(XINCX_PWM4_ENABLED)
    XINCX_PWM4_INST_IDX,
#endif
#if NRFX_CHECK(XINCX_PWM5_ENABLED)
    XINCX_PWM5_INST_IDX,
#endif
    XINCX_PWM_ENABLED_COUNT
};
#endif

/**
 * @brief This value can be provided instead of a pin number for any channel
 *        to specify that its output is not used and therefore does not need
 *        to be connected to a pin.
 */
#define XINCX_PWM_PIN_NOT_USED    0xFF

/** @brief This value can be added to a pin number to invert its polarity (set idle state = 1). */
#define XINCX_PWM_PIN_INVERTED    0x80

/** @brief PWM driver configuration structure. */
typedef struct
{
																		
	xinc_pwm_clk_src_t clk_src;          ///< Bit width.
	xinc_pwm_ref_clk_t  ref_clk;   ///< Base clock frequency.

	uint32_t            frequency;
	uint16_t           	duty_cycle;
	uint8_t           	inv_delay;
	bool           			start;
	bool           			inv_enable;
		
} xincx_pwm_config_t;

/** @brief PWM driver default configuration. */
#define XINCX_PWM_DEFAULT_CONFIG                                             \
{                                                                           \
	.ref_clk   = (xinc_pwm_ref_clk_t)XINCX_PWM_DEFAULT_CONFIG_REF_CLOCK,      \
	.clk_src   = 		(xinc_pwm_clk_src_t)XINCX_PWM_DEFAULT_CONFIG_CLK_SRC,      \
	.frequency       =  1000,                                                     \
	.duty_cycle   =  50,   	                                                 \
}



/** @brief PWM driver event handler type. */
typedef void (* xincx_pwm_handler_t)();

/**
 * @brief Function for initializing the PWM driver.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] p_config   Pointer to the structure with the initial configuration.
 * @param[in] handler    Event handler provided by the user. If NULL is passed
 *                       instead, event notifications are not done and PWM
 *                       interrupts are disabled.
 *
 * @retval NRFX_SUCCESS             Initialization was successful.
 * @retval NRFX_ERROR_INVALID_STATE The driver was already initialized.
 */
nrfx_err_t xincx_pwm_init(xincx_pwm_t const * const  p_instance,
                         xincx_pwm_config_t const * p_config,
                         xincx_pwm_handler_t        handler);

/**
 * @brief Function for uninitializing the PWM driver.
 *
 * If any sequence playback is in progress, it is stopped immediately.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_pwm_uninit(xincx_pwm_t const * const p_instance);


bool xincx_pwm_start(xincx_pwm_t const * const p_instance);
bool xincx_pwm_stop(xincx_pwm_t const * const p_instance);


nrfx_err_t xincx_pwm_freq_duty_cycl_update(xincx_pwm_t const * const p_instance,uint32_t new_freq,uint8_t new_duty);

nrfx_err_t xincx_pwm_duty_cycle_update(xincx_pwm_t const * const p_instance,uint8_t new_duty);

nrfx_err_t xincx_pwm_freq_update(xincx_pwm_t const * const p_instance,uint32_t new_freq);

bool xincx_pwm_freq_valid_range_check(uint8_t clk_src,uint8_t ref_clk, uint32_t set_freq);


#ifndef SUPPRESS_INLINE_IMPLEMENTATION



#endif // SUPPRESS_INLINE_IMPLEMENTATION

/** @} */



#ifdef __cplusplus
}
#endif

#endif // XINCX_PWM_H__
