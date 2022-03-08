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

#ifndef NRFX_PWM_H__
#define NRFX_PWM_H__

#include <nrfx.h>
#include <hal/nrf_pwm.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup nrfx_pwm PWM driver
 * @{
 * @ingroup nrf_pwm
 * @brief   Pulse Width Modulation (PWM) peripheral driver.
 */

/** @brief PWM driver instance data structure. */
typedef struct
{
    NRF_PWM_Type * p_reg;  ///< Pointer to the structure with PWM peripheral instance registers.
		XINC_CPR_CTL_Type * p_cpr; 
		uint8_t          id;
    uint8_t        drv_inst_idx; ///< Index of the driver instance. For internal use only.
} nrfx_pwm_t;

/** @brief Macro for creating a PWM driver instance. */
#define NRFX_PWM_INSTANCE(Id)                               \
{                                                           \
    .p_reg  = NRFX_CONCAT_2(NRF_PWM, Id),             \
		.p_cpr            = XINC_CPR,											\
		.id 		= Id,																				\
    .drv_inst_idx = NRFX_CONCAT_3(NRFX_PWM, Id, _INST_IDX), \
}

#ifndef __NRFX_DOXYGEN__
enum {
#if NRFX_CHECK(NRFX_PWM0_ENABLED)
    NRFX_PWM0_INST_IDX,
#endif
#if NRFX_CHECK(NRFX_PWM1_ENABLED)
    NRFX_PWM1_INST_IDX,
#endif
#if NRFX_CHECK(NRFX_PWM2_ENABLED)
    NRFX_PWM2_INST_IDX,
#endif
#if NRFX_CHECK(NRFX_PWM3_ENABLED)
    NRFX_PWM3_INST_IDX,
#endif
#if NRFX_CHECK(NRFX_PWM4_ENABLED)
    NRFX_PWM4_INST_IDX,
#endif
#if NRFX_CHECK(NRFX_PWM5_ENABLED)
    NRFX_PWM5_INST_IDX,
#endif
    NRFX_PWM_ENABLED_COUNT
};
#endif

/**
 * @brief This value can be provided instead of a pin number for any channel
 *        to specify that its output is not used and therefore does not need
 *        to be connected to a pin.
 */
#define NRFX_PWM_PIN_NOT_USED    0xFF

/** @brief This value can be added to a pin number to invert its polarity (set idle state = 1). */
#define NRFX_PWM_PIN_INVERTED    0x80

/** @brief PWM driver configuration structure. */
typedef struct
{
    uint8_t output_pins; ///< Pin numbers for individual output channels (optional).
                                                /**< Use @ref NRFX_PWM_PIN_NOT_USED
                                                 *   if a given output channel is not needed. */
    uint8_t            irq_priority; ///< Interrupt priority.
		xinc_pwm_clk_src_t clk_src;          ///< Bit width.
    nrf_pwm_ref_clk_t  ref_clk;   ///< Base clock frequency.

		uint32_t            frequency;
		uint16_t           	duty_cycle;
	
		bool           			start;
		
} nrfx_pwm_config_t;

/** @brief PWM driver default configuration. */
#define NRFX_PWM_DEFAULT_CONFIG                                             \
{                                                                           \
    .output_pins  =  NRFX_PWM_DEFAULT_CONFIG_OUT0_PIN,                      \
    .irq_priority =  NRFX_PWM_DEFAULT_CONFIG_IRQ_PRIORITY,                  \
    .ref_clk   = (nrf_pwm_ref_clk_t)NRFX_PWM_DEFAULT_CONFIG_REF_CLOCK,      \
		.clk_src   = 		(xinc_pwm_clk_src_t)XINCX_PWM_DEFAULT_CONFIG_CLK_SRC,      \
		.frequency       =  1000,                                                     \
		.duty_cycle   =  50,                                                    \
}


/** @brief PWM flags that provide additional playback options. */
typedef enum
{
    NRFX_PWM_FLAG_STOP = 0x01, /**< When the requested playback is finished,
                                    the peripheral will be stopped.
                                    @note The STOP task is triggered when
                                    the last value of the final sequence is
                                    loaded from RAM, and the peripheral stops
                                    at the end of the current PWM period.
                                    For sequences with configured repeating
                                    of duty cycle values, this might result in
                                    less than the requested number of repeats
                                    of the last value. */
    NRFX_PWM_FLAG_LOOP = 0x02, /**< When the requested playback is finished,
                                    it will be started from the beginning.
                                    This flag is ignored if used together
                                    with @ref NRFX_PWM_FLAG_STOP.
                                    @note The playback restart is done via a
                                    shortcut configured in the PWM peripheral.
                                    This shortcut triggers the proper starting
                                    task when the final value of previous
                                    playback is read from RAM and applied to
                                    the pulse generator counter.
                                    When this mechanism is used together with
                                    the @ref NRF_PWM_STEP_TRIGGERED mode,
                                    the playback restart will occur right
                                    after switching to the final value (this
                                    final value will be played only once). */
    NRFX_PWM_FLAG_SIGNAL_END_SEQ0 = 0x04, /**< The event handler is to be
                                               called when the last value
                                               from sequence 0 is loaded. */
    NRFX_PWM_FLAG_SIGNAL_END_SEQ1 = 0x08, /**< The event handler is to be
                                               called when the last value
                                               from sequence 1 is loaded. */
    NRFX_PWM_FLAG_NO_EVT_FINISHED = 0x10, /**< The playback finished event
                                               (enabled by default) is to be
                                               suppressed. */
    NRFX_PWM_FLAG_START_VIA_TASK = 0x80, /**< The playback must not be
                                              started directly by the called
                                              function. Instead, the function
                                              must only prepare it and
                                              return the address of the task
                                              to be triggered to start the
                                              playback. */
} nrfx_pwm_flag_t;

/** @brief PWM driver event type. */
typedef enum
{
    NRFX_PWM_EVT_FINISHED, ///< Sequence playback finished.
    NRFX_PWM_EVT_END_SEQ0, /**< End of sequence 0 reached. Its data can be
                                safely modified now. */
    NRFX_PWM_EVT_END_SEQ1, /**< End of sequence 1 reached. Its data can be
                                safely modified now. */
    NRFX_PWM_EVT_STOPPED,  ///< The PWM peripheral has been stopped.
} nrfx_pwm_evt_type_t;

/** @brief PWM driver event handler type. */
typedef void (* nrfx_pwm_handler_t)(nrfx_pwm_evt_type_t event_type);

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
nrfx_err_t nrfx_pwm_init(nrfx_pwm_t const * const  p_instance,
                         nrfx_pwm_config_t const * p_config,
                         nrfx_pwm_handler_t        handler);

/**
 * @brief Function for uninitializing the PWM driver.
 *
 * If any sequence playback is in progress, it is stopped immediately.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void nrfx_pwm_uninit(nrfx_pwm_t const * const p_instance);


bool nrfx_pwm_start(nrfx_pwm_t const * const p_instance);
bool nrfx_pwm_stop(nrfx_pwm_t const * const p_instance);

/**
 * @brief Function for checking the status of the PWM peripheral.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 *
 * @retval true  The PWM peripheral is stopped.
 * @retval false The PWM peripheral is not stopped.
 */
bool nrfx_pwm_is_stopped(nrfx_pwm_t const * const p_instance);

nrfx_err_t nrfx_pwm_freq_duty_cycl_update(nrfx_pwm_t const * const p_instance,uint32_t new_freq,uint8_t new_duty);

nrfx_err_t nrfx_pwm_duty_cycle_update(nrfx_pwm_t const * const p_instance,uint8_t new_duty);

nrfx_err_t nrfx_pwm_freq_update(nrfx_pwm_t const * const p_instance,uint32_t new_freq);

bool nrfx_pwm_freq_valid_range_check(uint8_t clk_src,uint8_t ref_clk, uint32_t set_freq);


#ifndef SUPPRESS_INLINE_IMPLEMENTATION



#endif // SUPPRESS_INLINE_IMPLEMENTATION

/** @} */



#ifdef __cplusplus
}
#endif

#endif // NRFX_PWM_H__
