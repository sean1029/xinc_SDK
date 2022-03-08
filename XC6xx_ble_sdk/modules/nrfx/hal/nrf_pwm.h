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

#ifndef NRF_PWM_H__
#define NRF_PWM_H__

#include <nrfx.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup nrf_pwm_hal PWM HAL
 * @{
 * @ingroup nrf_pwm
 * @brief   Hardware access layer for managing the Pulse Width Modulation (PWM) peripheral.
 */


/** @brief Number of channels in each PWM instance. */
#define NRF_PWM_CHANNEL_COUNT  6

/**
 * @brief Helper macro for calculating the number of 16-bit values in the specified
 *        array of duty cycle values.
 */
#define NRF_PWM_VALUES_LENGTH(array)  (sizeof(array) / sizeof(uint16_t))

#if 0
/** @brief PWM tasks. */
typedef enum
{
    NRF_PWM_TASK_STOP      = offsetof(NRF_PWM_Type, TASKS_STOP),        ///< Stops PWM pulse generation on all channels at the end of the current PWM period, and stops the sequence playback.
    NRF_PWM_TASK_SEQSTART0 = offsetof(NRF_PWM_Type, TASKS_SEQSTART[0]), ///< Starts playback of sequence 0.
    NRF_PWM_TASK_SEQSTART1 = offsetof(NRF_PWM_Type, TASKS_SEQSTART[1]), ///< Starts playback of sequence 1.
    NRF_PWM_TASK_NEXTSTEP  = offsetof(NRF_PWM_Type, TASKS_NEXTSTEP)     ///< Steps by one value in the current sequence if the decoder is set to @ref NRF_PWM_STEP_TRIGGERED mode.
} nrf_pwm_task_t;

/** @brief PWM events. */
typedef enum
{
    NRF_PWM_EVENT_STOPPED      = offsetof(NRF_PWM_Type, EVENTS_STOPPED),       ///< Response to STOP task, emitted when PWM pulses are no longer generated.
    NRF_PWM_EVENT_SEQSTARTED0  = offsetof(NRF_PWM_Type, EVENTS_SEQSTARTED[0]), ///< First PWM period started on sequence 0.
    NRF_PWM_EVENT_SEQSTARTED1  = offsetof(NRF_PWM_Type, EVENTS_SEQSTARTED[1]), ///< First PWM period started on sequence 1.
    NRF_PWM_EVENT_SEQEND0      = offsetof(NRF_PWM_Type, EVENTS_SEQEND[0]),     ///< Emitted at the end of every sequence 0 when its last value has been read from RAM.
    NRF_PWM_EVENT_SEQEND1      = offsetof(NRF_PWM_Type, EVENTS_SEQEND[1]),     ///< Emitted at the end of every sequence 1 when its last value has been read from RAM.
    NRF_PWM_EVENT_PWMPERIODEND = offsetof(NRF_PWM_Type, EVENTS_PWMPERIODEND),  ///< Emitted at the end of each PWM period.
    NRF_PWM_EVENT_LOOPSDONE    = offsetof(NRF_PWM_Type, EVENTS_LOOPSDONE)      ///< Concatenated sequences have been played the specified number of times.
} nrf_pwm_event_t;
#endif
/** @brief PWM interrupts. */
typedef enum
{
    NRF_PWM_INT_STOPPED_MASK      = PWM_INTENSET_STOPPED_Msk,      ///< Interrupt on STOPPED event.
    NRF_PWM_INT_SEQSTARTED0_MASK  = PWM_INTENSET_SEQSTARTED0_Msk,  ///< Interrupt on SEQSTARTED[0] event.
    NRF_PWM_INT_SEQSTARTED1_MASK  = PWM_INTENSET_SEQSTARTED1_Msk,  ///< Interrupt on SEQSTARTED[1] event.
    NRF_PWM_INT_SEQEND0_MASK      = PWM_INTENSET_SEQEND0_Msk,      ///< Interrupt on SEQEND[0] event.
    NRF_PWM_INT_SEQEND1_MASK      = PWM_INTENSET_SEQEND1_Msk,      ///< Interrupt on SEQEND[1] event.
    NRF_PWM_INT_PWMPERIODEND_MASK = PWM_INTENSET_PWMPERIODEND_Msk, ///< Interrupt on PWMPERIODEND event.
    NRF_PWM_INT_LOOPSDONE_MASK    = PWM_INTENSET_LOOPSDONE_Msk     ///< Interrupt on LOOPSDONE event.
} nrf_pwm_int_mask_t;


/** @brief PWM shortcuts. */
typedef enum
{
    NRF_PWM_SHORT_SEQEND0_STOP_MASK        = PWM_SHORTS_SEQEND0_STOP_Msk,        ///< Shortcut between SEQEND[0] event and STOP task.
    NRF_PWM_SHORT_SEQEND1_STOP_MASK        = PWM_SHORTS_SEQEND1_STOP_Msk,        ///< Shortcut between SEQEND[1] event and STOP task.
    NRF_PWM_SHORT_LOOPSDONE_SEQSTART0_MASK = PWM_SHORTS_LOOPSDONE_SEQSTART0_Msk, ///< Shortcut between LOOPSDONE event and SEQSTART[0] task.
    NRF_PWM_SHORT_LOOPSDONE_SEQSTART1_MASK = PWM_SHORTS_LOOPSDONE_SEQSTART1_Msk, ///< Shortcut between LOOPSDONE event and SEQSTART[1] task.
    NRF_PWM_SHORT_LOOPSDONE_STOP_MASK      = PWM_SHORTS_LOOPSDONE_STOP_Msk       ///< Shortcut between LOOPSDONE event and STOP task.
} nrf_pwm_short_mask_t;

/** @brief PWM modes of operation. */
typedef enum
{
    NRF_PWM_MODE_UP          = PWM_MODE_UPDOWN_Up,        ///< Up counter (edge-aligned PWM duty cycle).
    NRF_PWM_MODE_UP_AND_DOWN = PWM_MODE_UPDOWN_UpAndDown, ///< Up and down counter (center-aligned PWM duty cycle).
} nrf_pwm_mode_t;


/** @brief PWM interrupts. */
enum
{
    NRF_PWM_ID_0 = 0,      ///< NRF_PWM_ID_0
    NRF_PWM_ID_1  = 1,
    NRF_PWM_ID_2 = 2,
    NRF_PWM_ID_3 = 3 ,
    NRF_PWM_ID_4 = 4,     
    NRF_PWM_ID_5 = 5, 
   
} ;




/** @brief Timer bit width. */
typedef enum
{
    XINC_PWM_CLK_SRC_32M_DIV = 0, ///< PWM CLK SRC 32MHz div.
    XINC_PWM_CLK_SRC_32K_DIV = 1, ///< PWM CLK SRC 32kHz div.
	  XINC_PWM_CLK_SRC_32K = 4, ///< PWM CLK SRC 32kHz.
} xinc_pwm_clk_src_t;

/** @brief Timer prescalers. */
/** @brief PWM base clock frequencies. */
typedef enum
{
    XINC_PWM_REF_CLK_16MHzOr16K = 0, 		///< PWM CLK 16 MHz or 16KHz
    XINC_PWM_REF_CLK_8MHzOr8K = 1,      ///< PWM CLK 8 MHz or 8KHz
    XINC_PWM_REF_CLK_4MHzOr4K = 2,      ///< PWM CLK 4 MHz or 4KHz.
    XINC_PWM_REF_CLK_2MHzOr2K = 3,      ///< PWM CLK 2 MHz or 2KHz.
    XINC_PWM_REF_CLK_1MHzOr1K = 4,      ///< PWM CLK 1 MHz or 1KHz.
    XINC_PWM_REF_CLK_500kHzOr500 = 5,    ///< PWM CLK 500 kHz or 500Hz.
    XINC_PWM_REF_CLK_250kHzOr250 = 6,    ///< PWM CLK 250 kHz or 250Hz.
    XINC_PWM_REF_CLK_125kHzOr125 = 7,    ///< PWM CLK 125 kHz or 125Hz.
    XINC_PWM_REF_CLK_62500HzOr62_5 = 8,   ///< PWM CLK 62500 Hz or 62.5Hz.
    XINC_PWM_REF_CLK_32000Hz = 9    ///< PWM CLK 32000 Hz.
} nrf_pwm_ref_clk_t;
/**
 * @brief PWM decoder load modes.
 *
 * The selected mode determines how the sequence data is read from RAM and
 * spread to the compare registers.
 */
typedef enum
{
    NRF_PWM_LOAD_COMMON     = PWM_DECODER_LOAD_Common,     ///< 1st half word (16-bit) used in all PWM channels (0-3).
    NRF_PWM_LOAD_GROUPED    = PWM_DECODER_LOAD_Grouped,    ///< 1st half word (16-bit) used in channels 0 and 1; 2nd word in channels 2 and 3.
    NRF_PWM_LOAD_INDIVIDUAL = PWM_DECODER_LOAD_Individual, ///< 1st half word (16-bit) used in channel 0; 2nd in channel 1; 3rd in channel 2; 4th in channel 3.
    NRF_PWM_LOAD_WAVE_FORM  = PWM_DECODER_LOAD_WaveForm    ///< 1st half word (16-bit) used in channel 0; 2nd in channel 1; ... ; 4th as the top value for the pulse generator counter.
} nrf_pwm_dec_load_t;

/**
 * @brief PWM decoder next step modes.
 *
 * The selected mode determines when the next value from the active sequence
 * is loaded.
 */
typedef enum
{
    NRF_PWM_STEP_AUTO      = PWM_DECODER_MODE_RefreshCount, ///< Automatically after the current value is played and repeated the requested number of times.
    NRF_PWM_STEP_TRIGGERED = PWM_DECODER_MODE_NextStep      ///< When the @ref NRF_PWM_TASK_NEXTSTEP task is triggered.
} nrf_pwm_dec_step_t;

/**
 * @brief Type used for defining duty cycle values for a sequence
 *        loaded in @ref NRF_PWM_LOAD_COMMON mode.
 */
typedef uint16_t nrf_pwm_values_common_t;

/**
 * @brief Structure for defining duty cycle values for a sequence
 *        loaded in @ref NRF_PWM_LOAD_GROUPED mode.
 */
typedef struct {
    uint16_t group_0; ///< Duty cycle value for group 0 (channels 0 and 1).
    uint16_t group_1; ///< Duty cycle value for group 1 (channels 2 and 3).
} nrf_pwm_values_grouped_t;

/**
 * @brief Structure for defining duty cycle values for a sequence
 *        loaded in @ref NRF_PWM_LOAD_INDIVIDUAL mode.
 */
typedef struct
{
    uint16_t channel_0; ///< Duty cycle value for channel 0.
    uint16_t channel_1; ///< Duty cycle value for channel 1.
    uint16_t channel_2; ///< Duty cycle value for channel 2.
    uint16_t channel_3; ///< Duty cycle value for channel 3.
} nrf_pwm_values_individual_t;

/**
 * @brief Structure for defining duty cycle values for a sequence
 *        loaded in @ref NRF_PWM_LOAD_WAVE_FORM mode.
 */
typedef struct {
    uint16_t channel_0;   ///< Duty cycle value for channel 0.
    uint16_t channel_1;   ///< Duty cycle value for channel 1.
    uint16_t channel_2;   ///< Duty cycle value for channel 2.
    uint16_t counter_top; ///< Top value for the pulse generator counter.
} nrf_pwm_values_wave_form_t;

/**
 * @brief Union grouping pointers to arrays of duty cycle values applicable to
 *        various loading modes.
 */
typedef union {
    nrf_pwm_values_common_t     const * p_common;     ///< Pointer to be used in @ref NRF_PWM_LOAD_COMMON mode.
    nrf_pwm_values_grouped_t    const * p_grouped;    ///< Pointer to be used in @ref NRF_PWM_LOAD_GROUPED mode.
    nrf_pwm_values_individual_t const * p_individual; ///< Pointer to be used in @ref NRF_PWM_LOAD_INDIVIDUAL mode.
    nrf_pwm_values_wave_form_t  const * p_wave_form;  ///< Pointer to be used in @ref NRF_PWM_LOAD_WAVE_FORM mode.
    uint16_t                    const * p_raw;        ///< Pointer providing raw access to the values.
} nrf_pwm_values_t;

/**
 * @brief Structure for defining a sequence of PWM duty cycles.
 *
 * When the sequence is set (by a call to @ref nrf_pwm_sequence_set), the
 * provided duty cycle values are not copied. The @p values pointer is stored
 * in the internal register of the peripheral, and the values are loaded from RAM
 * during the sequence playback. Therefore, you must ensure that the values
 * do not change before and during the sequence playback (for example,
 * the values cannot be placed in a local variable that is allocated on stack).
 * If the sequence is played in a loop and the values are to be updated
 * before the next iteration, it is safe to modify them when the corresponding
 * event signaling the end of sequence occurs (@ref NRF_PWM_EVENT_SEQEND0
 * or @ref NRF_PWM_EVENT_SEQEND1, respectively).
 *
 * @note The @p repeats and @p end_delay values (which are written to the
 *       SEQ[n].REFRESH and SEQ[n].ENDDELAY registers in the peripheral,
 *       respectively) are ignored at the end of a complex sequence
 *       playback, indicated by the LOOPSDONE event.
 *       See the @linkProductSpecification52 for more information.
 */
typedef struct
{
    nrf_pwm_values_t values; ///< Pointer to an array with duty cycle values. This array must be in Data RAM.
                             /**< This field is defined as an union of pointers
                              *   to provide a convenient way to define duty
                              *   cycle values in various loading modes
                              *   (see @ref nrf_pwm_dec_load_t).
                              *   In each value, the most significant bit (15)
                              *   determines the polarity of the output and the
                              *   others (14-0) compose the 15-bit value to be
                              *   compared with the pulse generator counter. */
    uint16_t length;    ///< Number of 16-bit values in the array pointed by @p values.
    uint32_t repeats;   ///< Number of times that each duty cycle is to be repeated (after being played once). Ignored in @ref NRF_PWM_STEP_TRIGGERED mode.
    uint32_t end_delay; ///< Additional time (in PWM periods) that the last duty cycle is to be kept after the sequence is played. Ignored in @ref NRF_PWM_STEP_TRIGGERED mode.
} nrf_pwm_sequence_t;

#if 0
/**
 * @brief Function for activating the specified PWM task.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] task  Task to be activated.
 */
__STATIC_INLINE void nrf_pwm_task_trigger(NRF_PWM_Type * p_reg,
                                          nrf_pwm_task_t task);

/**
 * @brief Function for getting the address of the specified PWM task register.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] task  PWM task.
 *
 * @return Address of the specified task register.
 */
__STATIC_INLINE uint32_t nrf_pwm_task_address_get(NRF_PWM_Type const * p_reg,
                                                  nrf_pwm_task_t       task);

/**
 * @brief Function for clearing the specified PWM event.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] event Event to clear.
 */
__STATIC_INLINE void nrf_pwm_event_clear(NRF_PWM_Type *  p_reg,
                                         nrf_pwm_event_t event);

/**
 * @brief Function for retrieving the state of the PWM event.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] event Event to be checked.
 *
 * @retval true  The event has been generated.
 * @retval false The event has not been generated.
 */
__STATIC_INLINE bool nrf_pwm_event_check(NRF_PWM_Type const * p_reg,
                                         nrf_pwm_event_t      event);

/**
 * @brief Function for getting the address of the specified PWM event register.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] event PWM event.
 *
 * @return Address of the specified event register.
 */
__STATIC_INLINE uint32_t nrf_pwm_event_address_get(NRF_PWM_Type const * p_reg,
                                                   nrf_pwm_event_t      event);

/**
 * @brief Function for enabling the specified shortcuts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of shortcuts to be enabled.
 */
__STATIC_INLINE void nrf_pwm_shorts_enable(NRF_PWM_Type * p_reg,
                                           uint32_t       mask);

/**
 * @brief Function for disabling the specified shortcuts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of shortcuts to be disabled.
 */
__STATIC_INLINE void nrf_pwm_shorts_disable(NRF_PWM_Type * p_reg,
                                            uint32_t       mask);

/**
 * @brief Function for setting the configuration of PWM shortcuts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Shortcuts configuration to be set.
 */
__STATIC_INLINE void nrf_pwm_shorts_set(NRF_PWM_Type * p_reg,
                                        uint32_t       mask);
#endif
/**
 * @brief Function for enabling specified interrupts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of interrupts to be enabled.
 */
__STATIC_INLINE void nrf_pwm_int_enable(NRF_PWM_Type * p_reg,
                                        uint32_t       mask);

/**
 * @brief Function for disabling specified interrupts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of interrupts to be disabled.
 */
__STATIC_INLINE void nrf_pwm_int_disable(NRF_PWM_Type * p_reg,
                                         uint32_t       mask);

/**
 * @brief Function for setting the configuration of PWM interrupts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of interrupts to be set.
 */
__STATIC_INLINE void nrf_pwm_int_set(NRF_PWM_Type * p_reg,
                                     uint32_t       mask);

/**
 * @brief Function for retrieving the state of a given interrupt.
 *
 * @param[in] p_reg   Pointer to the structure of registers of the peripheral.
 * @param[in] pwm_int Interrupt to be checked.
 *
 * @retval true  The interrupt is enabled.
 * @retval false The interrupt is not enabled.
 */
__STATIC_INLINE bool nrf_pwm_int_enable_check(NRF_PWM_Type const * p_reg,
                                              nrf_pwm_int_mask_t   pwm_int);

#if defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)
/**
 * @brief Function for setting the subscribe configuration for a given
 *        PWM task.
 *
 * @param[in] p_reg   Pointer to the structure of registers of the peripheral.
 * @param[in] task    Task for which to set the configuration.
 * @param[in] channel Channel through which to subscribe events.
 */
__STATIC_INLINE void nrf_pwm_subscribe_set(NRF_PWM_Type * p_reg,
                                           nrf_pwm_task_t task,
                                           uint8_t        channel);

/**
 * @brief Function for clearing the subscribe configuration for a given
 *        PWM task.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] task  Task for which to clear the configuration.
 */
__STATIC_INLINE void nrf_pwm_subscribe_clear(NRF_PWM_Type * p_reg,
                                             nrf_pwm_task_t task);

/**
 * @brief Function for setting the publish configuration for a given
 *        PWM event.
 *
 * @param[in] p_reg   Pointer to the structure of registers of the peripheral.
 * @param[in] event   Event for which to set the configuration.
 * @param[in] channel Channel through which to publish the event.
 */
__STATIC_INLINE void nrf_pwm_publish_set(NRF_PWM_Type *  p_reg,
                                         nrf_pwm_event_t event,
                                         uint8_t         channel);

/**
 * @brief Function for clearing the publish configuration for a given
 *        PWM event.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] event Event for which to clear the configuration.
 */
__STATIC_INLINE void nrf_pwm_publish_clear(NRF_PWM_Type *  p_reg,
                                           nrf_pwm_event_t event);
#endif // defined(DPPI_PRESENT) || defined(__NRFX_DOXYGEN__)

/**
 * @brief Function for enabling the PWM peripheral.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void nrf_pwm_enable(NRF_PWM_Type * p_reg);

/**
 * @brief Function for disabling the PWM peripheral.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void nrf_pwm_disable(NRF_PWM_Type * p_reg);



/**
 * @brief Function for configuring the PWM peripheral.
 *
 * @param[in] p_reg      Pointer to the structure of registers of the peripheral.
 * @param[in] base_clock Base clock frequency.
 * @param[in] mode       Operating mode of the pulse generator counter.
 * @param[in] top_value  Value up to which the pulse generator counter counts.
 */
__STATIC_INLINE void nrf_pwm_configure(NRF_PWM_Type * p_reg,
                                       uint8_t       period,
									   uint8_t        duty_cycle);





__STATIC_INLINE void xinc_pwm_clk_div_set(XINC_CPR_CTL_Type * p_reg,uint8_t id,xinc_pwm_clk_src_t clk_src,
                                             nrf_pwm_ref_clk_t ref_clk);

__STATIC_INLINE uint32_t xinc_pwm_clk_div_get(XINC_CPR_CTL_Type * p_reg,uint8_t id);


#ifndef SUPPRESS_INLINE_IMPLEMENTATION


__STATIC_INLINE void nrf_pwm_enable(NRF_PWM_Type * p_reg)
{
    p_reg->EN |= (PWM_ENABLE_ENABLE_Enabled << PWM_ENABLE_ENABLE_Pos);	
}

__STATIC_INLINE void nrf_pwm_disable(NRF_PWM_Type * p_reg)
{	
    p_reg->EN &= ~(PWM_ENABLE_ENABLE_Enabled << PWM_ENABLE_ENABLE_Pos);
}



__STATIC_INLINE void nrf_pwm_configure(NRF_PWM_Type * p_reg,
                                       uint8_t       period,
																			 uint8_t  duty_cycle)
{
    //NRFX_ASSERT(top_value <= PWM_COUNTERTOP_COUNTERTOP_Msk);

	p_reg->OCPY = duty_cycle;
  p_reg->PERIOD = period;
	p_reg->UP = 0x1;
}



__STATIC_INLINE void xinc_pwm_clk_div_set(XINC_CPR_CTL_Type * p_reg,uint8_t id,xinc_pwm_clk_src_t clk_src,
                                             nrf_pwm_ref_clk_t ref_clk)
{
	
	switch(clk_src)
	{
		case XINC_PWM_CLK_SRC_32M_DIV:
		{
			p_reg->PWM_CLK_CTL = (1 << ref_clk) - 1 ;
		}break;
		
		case XINC_PWM_CLK_SRC_32K_DIV:
		{
			p_reg->PWM_CLK_CTL = ((1 << ref_clk) - 1) | (0x01 << 28UL) ;
		}break;
		
		case XINC_PWM_CLK_SRC_32K:
		{
			p_reg->PWM_CLK_CTL = (0x04 << 28UL);
		}break;
		
		default:
			break;
	
	}
	p_reg->PWM_CLK_CTL |= (0x01 << 31UL);
	printf("pwm clock id:%d, addr=[%p],val=[0x%x],ref_clk=[%d]\n",id,&p_reg->PWM_CLK_CTL,p_reg->PWM_CLK_CTL,ref_clk);
}

__STATIC_INLINE uint32_t xinc_pwm_clk_div_get(XINC_CPR_CTL_Type * p_reg,uint8_t id)
{

	return (uint32_t)p_reg->PWM_CLK_CTL;
	
}

#endif // SUPPRESS_INLINE_IMPLEMENTATION

/** @} */

#ifdef __cplusplus
}
#endif

#endif // NRF_PWM_H__

