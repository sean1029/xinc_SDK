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

#ifndef NRFX_TIMER_H__
#define NRFX_TIMER_H__

#include <nrfx.h>
#include <hal/nrf_timer.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup nrfx_timer Timer driver
 * @{
 * @ingroup nrf_timer
 * @brief   TIMER peripheral driver.
 */

/**
 * @brief Timer driver instance data structure.
 */
typedef struct
{
    NRF_TIMER_Type * p_reg;            ///< Pointer to the structure with TIMER peripheral instance registers.
	  NRF_TIMER_GLOBAL_Type * p_Greg;
		XINC_CPR_CTL_Type * p_cpr;  
		uint8_t          id;      					///< Index of the driver instance. For internal use only.
    uint8_t          instance_idx;      ///< Index of the driver instance. For internal use only.
    uint8_t          cc_channel; ///< Number of capture/compare channels.
		nrf_timer_frequency_t frequency;          ///< Frequency.
} nrfx_timer_t;

//}			
/** @brief Macro for creating a timer driver instance. */
#define NRFX_TIMER_INSTANCE(Id)                                   \
{                                                                 \
    .p_reg            = NRFX_CONCAT_2(NRF_TIMER, Id),             \
    .p_Greg            = NRF_TIMER_GLOBAL,												\
	  .p_cpr            = XINC_CPR,																	\
		.id            		= Id,																				\
    .instance_idx      = NRFX_CONCAT_3(NRFX_TIMER, Id, _INST_IDX), \
    .cc_channel = 		Id,          																 \
}
          
#ifndef __NRFX_DOXYGEN__
enum {
#if NRFX_CHECK(NRFX_TIMER0_ENABLED)
    NRFX_TIMER0_INST_IDX ,
#endif
#if NRFX_CHECK(NRFX_TIMER1_ENABLED)
    NRFX_TIMER1_INST_IDX,
#endif
#if NRFX_CHECK(NRFX_TIMER2_ENABLED)
    NRFX_TIMER2_INST_IDX ,
#endif
#if NRFX_CHECK(NRFX_TIMER3_ENABLED)
    NRFX_TIMER3_INST_IDX ,
#endif
    NRFX_TIMER_ENABLED_COUNT
};
#endif

/** @brief The configuration structure of the timer driver instance. */
typedef struct
{
    nrf_timer_frequency_t frequency;          ///< Frequency.
    nrf_timer_mode_t      mode;               ///< Mode of operation.
    nrf_timer_clk_src_t 	clk_src;          ///< Bit width.
    uint8_t               interrupt_priority; ///< Interrupt priority.
    void *                p_context;          ///< Context passed to interrupt handler.
} nrfx_timer_config_t;

/** @brief Timer driver instance default configuration. */
#define NRFX_TIMER_DEFAULT_CONFIG                                                    \
{                                                                                    \
    .frequency          = (nrf_timer_frequency_t)NRFX_TIMER_DEFAULT_CONFIG_FREQUENCY,\
    .mode               = (nrf_timer_mode_t)NRFX_TIMER_DEFAULT_CONFIG_MODE,          \
    .clk_src          = 	(nrf_timer_clk_src_t)NRFX_TIMER_DEFAULT_CONFIG_CLK_SRC,\
    .interrupt_priority = NRFX_TIMER_DEFAULT_CONFIG_IRQ_PRIORITY,                    \
    .p_context          = NULL                                                       \
}

/**
 * @brief Timer driver event handler type.
 *
 * @param[in] event_type Timer event.
 * @param[in] p_context  General purpose parameter set during initialization of
 *                       the timer. This parameter can be used to pass
 *                       additional information to the handler function, for
 *                       example, the timer ID.
 */
typedef void (* nrfx_timer_event_handler_t)(nrf_timer_int_event_t event_type,uint8_t channel ,
                                            void            * p_context);

/**
 * @brief Function for initializing the timer.
 *
 * @param[in] p_instance          Pointer to the driver instance structure.
 * @param[in] p_config            Pointer to the structure with the initial configuration.
 * @param[in] timer_event_handler Event handler provided by the user.
 *                                Must not be NULL.
 *
 * @retval NRFX_SUCCESS             Initialization was successful.
 * @retval NRFX_ERROR_INVALID_STATE The instance is already initialized.
 */
nrfx_err_t nrfx_timer_init(nrfx_timer_t const * const  p_instance,
                           nrfx_timer_config_t const * p_config,
                           nrfx_timer_event_handler_t  timer_event_handler);

/**
 * @brief Function for uninitializing the timer.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void nrfx_timer_uninit(nrfx_timer_t const * const p_instance);

/**
 * @brief Function for turning on the timer.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void nrfx_timer_enable(nrfx_timer_t const * const p_instance);

/**
 * @brief Function for turning off the timer.
 *
 * The timer will allow to enter the lowest possible SYSTEM_ON state
 * only after this function is called.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void nrfx_timer_disable(nrfx_timer_t const * const p_instance);

/**
 * @brief Function for checking the timer state.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 *
 * @retval true  Timer is enabled.
 * @retval false Timer is not enabled.
 */
bool nrfx_timer_is_enabled(nrfx_timer_t const * const p_instance);



/**
 * @brief Function for setting the timer channel in compare mode.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] cc_channel Compare channel number.
 * @param[in] cc_value   Compare value.
 * @param[in] enable_int Enable or disable the interrupt for the compare channel.
 */
void nrfx_timer_compare(nrfx_timer_t const * const p_instance,
                        uint32_t                   cc_value,
                        bool                       enable_int);

/**
 * @brief Function for converting time in microseconds to timer ticks.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] time_us    Time in microseconds.
 *
 * @return Number of ticks.
 */
__STATIC_INLINE uint32_t nrfx_timer_us_to_ticks(nrfx_timer_t const * const p_instance,
                                                uint32_t                   time_us);

/**
 * @brief Function for converting time in milliseconds to timer ticks.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] time_ms    Time in milliseconds.
 *
 * @return Number of ticks.
 */
__STATIC_INLINE uint32_t nrfx_timer_ms_to_ticks(nrfx_timer_t const * const p_instance,
                                                uint32_t                   time_ms);

/**
 * @brief Function for enabling timer compare interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] channel    Compare channel.
 */
void nrfx_timer_compare_int_enable(nrfx_timer_t const * const p_instance,
                                   uint32_t                   channel);

/**
 * @brief Function for disabling timer compare interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] channel    Compare channel.
 */
void nrfx_timer_compare_int_disable(nrfx_timer_t const * const p_instance,
                                    uint32_t                   channel);


#ifndef SUPPRESS_INLINE_IMPLEMENTATION


__STATIC_INLINE uint32_t nrfx_timer_us_to_ticks(nrfx_timer_t const * const p_instance,
                                                uint32_t                   timer_us)
{
    return nrf_timer_us_to_ticks(timer_us, nrf_timer_frequency_div_get(p_instance->p_cpr,p_instance->id));
}

__STATIC_INLINE uint32_t nrfx_timer_ms_to_ticks(nrfx_timer_t const * const p_instance,
                                                uint32_t                   timer_ms)
{
    return nrf_timer_ms_to_ticks(timer_ms,nrf_timer_frequency_div_get(p_instance->p_cpr,p_instance->id));
}

#endif // SUPPRESS_INLINE_IMPLEMENTATION

/** @} */


void nrfx_timer_0_irq_handler(void);
void nrfx_timer_1_irq_handler(void);
void nrfx_timer_2_irq_handler(void);
void nrfx_timer_3_irq_handler(void);


#ifdef __cplusplus
}
#endif //__NRFX_DOXYGEN_

#endif // NRFX_TIMER_H__

