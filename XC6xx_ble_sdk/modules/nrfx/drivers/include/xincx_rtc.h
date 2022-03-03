/**
 * Copyright (c) 2017 - 2021, Nordic Semiconductor ASA
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

#ifndef NRFX_RTC_H__
#define NRFX_RTC_H__


#include <nrfx_config.h>
#include <drivers/nrfx_common.h>
#include <nrfx_glue.h>
#include <drivers/nrfx_errors.h>
#include <hal/xinc_rtc.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xincx_rtc RTC driver
 * @{
 * @ingroup nrf_rtc
 * @brief   Real Timer Counter (RTC) peripheral driver.
 */

/** @brief Macro for converting microseconds into ticks. */
#define NRFX_RTC_US_TO_TICKS(us,freq) (((us) * (freq)) / 1000000U)

/** @brief RTC driver interrupt types. */
typedef enum
{
    NRFX_RTC_INT_TIME1 = 0, /**< Interrupt from TIME1 event. */
    NRFX_RTC_INT_TIME2 = 1, /**< Interrupt from TIME2 event. */
    NRFX_RTC_INT_TIME3 = 2, /**< Interrupt from TIME3 event. */
    NRFX_RTC_INT_SEC = 3, /**< Interrupt from SEC event. */
    NRFX_RTC_INT_MIN     = 4, /**< Interrupt from MIN event. */
    NRFX_RTC_INT_HOUR = 5,  /**< Interrupt from HOUR event. */
	  NRFX_RTC_INT_DAY = 6,  /**< Interrupt from DAY event. */
		NRFX_RTC_INT_AOTIME = 7
} xincx_rtc_int_type_t;

/** @brief RTC driver interrupt types. */
enum
{
    NRFX_RTC_WEEK_MATCH_SUNDAY = (0x01 << 0), /**< Interrupt from TIME1 event. */
    NRFX_RTC_WEEK_MATCH_MONDAY = (0x01 << 1), /**< Interrupt from TIME2 event. */
    NRFX_RTC_WEEK_MATCH_TUESDAY = (0x01 << 2), /**< Interrupt from TIME3 event. */
    NRFX_RTC_WEEK_MATCH_WEDNESDAY = (0x01 << 3), /**< Interrupt from SEC event. */
    NRFX_RTC_WEEK_MATCH_THURSDAY     = (0x01 << 4), /**< Interrupt from MIN event. */
    NRFX_RTC_WEEK_MATCH_FRIDAY = (0x01 << 5),  /**< Interrupt from HOUR event. */
	  NRFX_RTC_WEEK_MATCH_SATURDAY = (0x01 << 6)  /**< Interrupt from DAY event. */
};

/** @brief RTC driver interrupt types. */
typedef enum
{
    NRFX_RTC_MATCH_TIME_1 = (0x01 << 5), /**< MATCH_TIME_1 ch. */
    NRFX_RTC_MATCH_TIME_2 = (0x01 << 4), /**< MATCH_TIME_2 ch */
    NRFX_RTC_MATCH_TIME_3 = (0x01 << 6), /**< MATCH_TIME_3 ch */
}xincx_rtc_match_timer_ch_t;

/** @brief RTC driver instance structure. */
typedef struct
{
    NRF_RTC_Type  * p_reg;            /**< Pointer to instance register set. */
    uint8_t         instance_id;      /**< Index of the driver instance. For internal use only. */
    uint8_t         cc_channel_count; /**< Number of capture/compare channels. */
} xincx_rtc_t;



/** @brief Macro for creating an RTC driver instance. */
#define NRFX_RTC_INSTANCE(id)                                   \
{                                                               \
    .p_reg            = NRFX_CONCAT_2(NRF_RTC, id),             \
    .instance_id      = NRFX_CONCAT_3(NRFX_RTC, id, _INST_IDX), \
    .cc_channel_count = NRF_RTC_CC_CHANNEL_COUNT(id),           \
}

#ifndef __NRFX_DOXYGEN__
enum {
#if NRFX_CHECK(NRFX_RTC0_ENABLED)
    NRFX_RTC0_INST_IDX,
#endif
    NRFX_RTC_ENABLED_COUNT
};
#endif


/** @brief RTC driver instance configuration structure. */
typedef struct
{
	nrf_rtc_time_t date;
	uint8_t   hour_limit;            /**< hour limit. */
	uint8_t   min_limit;             /**< minute limit. */
	uint8_t   sec_limit;             /**< second limit. */
	uint16_t  freq;                  /**< freq. */
	uint8_t   interrupt_priority;    /**< Interrupt priority. */
	nrf_rtc_type_t   type;           /**< Rtc type. */
	bool      reliable;              /**< Reliable mode flag. */
	
} xincx_rtc_config_t;

/** @brief RTC driver instance configuration structure. */
typedef union xincx_rtc_match_config
{
	uint32_t    value;
	struct time
	{
			uint32_t  sec:6;                    /**< second. */
			uint32_t  hour:6;                  /**< hour. */
			uint32_t  min:5;                   /**< minute. */
			uint32_t  week:7;                  /**< week. */
	}times;

} xincx_rtc_match_config_t;


/** @brief RTC instance default configuration. */
#define NRFX_RTC_DEFAULT_CONFIG                                                     \
{                                                                                   \
  .date.day                = 0,                                                                             \
	.date.hour               = 0,                                                                              \
	.date.min             = 0,                                                                             \
	.date.sec             = 0,                                                                        \
	.date.week               = 0,                                                        \
	.hour_limit         = 24,                                                        \
	.min_limit          = 60,                                                        \
	.sec_limit          = 60,                                                        \
	.freq               = 29790,                                                        \
  .interrupt_priority = 0,                     \
  .reliable           = 0,                         \
}


/** @brief RTC driver instance handler type. */
typedef void (*xincx_rtc_handler_t)(xincx_rtc_int_type_t int_type);


/**
 * @brief Function for initializing the RTC driver instance.
 *
 * After initialization, the instance is in power off state.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] p_config   Pointer to the structure with the initial configuration.
 * @param[in] handler    Event handler provided by the user.
 *                       Must not be NULL.
 *
 * @retval NRFX_SUCCESS             Successfully initialized.
 * @retval NRFX_ERROR_INVALID_STATE The instance is already initialized.
 */
nrfx_err_t xincx_rtc_init(xincx_rtc_t const * const  p_instance,
                         xincx_rtc_config_t const * p_config,
                         xincx_rtc_handler_t        handler);

/**
 * @brief Function for uninitializing the RTC driver instance.
 *
 * After uninitialization, the instance is in idle state. The hardware should return to the state
 * before initialization.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_rtc_uninit(xincx_rtc_t const * const p_instance);

/**
 * @brief Function for enabling the RTC driver instance.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_rtc_enable(xincx_rtc_t const * const p_instance);	

/**
 * @brief Function for disabling the RTC driver instance.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_rtc_disable(xincx_rtc_t const * const p_instance);

/**
 * @brief Function for setting a compare channel.
 *
 * The function powers on the instance if the instance was in power off state.
 *
 * The driver is not entering a critical section when configuring RTC, which means that it can be
 * preempted for a certain amount of time. When the driver was preempted and the value to be set
 * is short in time, there is a risk that the driver sets a compare value that is
 * behind. In this case, if the reliable mode is enabled for the specified instance,
 * the risk is handled.
 * However, to detect if the requested value is behind, this mode makes the following assumptions:
 *  -  The maximum preemption time in ticks (8-bit value) is known and is less than 7.7 ms
 *   (for prescaler = 0, RTC frequency 32 kHz).
 *  -  The requested absolute compare value is not bigger than (0x00FFFFFF)-tick_latency. It is
 *   the user's responsibility to ensure this.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] channel    One of the channels of the instance.
 * @param[in] val        Absolute value to be set in the compare register.
 * @param[in] enable_irq True to enable the interrupt. False to disable the interrupt.
 *
 * @retval NRFX_SUCCESS       The procedure is successful.
 * @retval NRFX_ERROR_TIMEOUT The compare is not set because the request value is behind the
 *                            current counter value. This error can only be reported
 *                            if the reliable mode is enabled.
 */
nrfx_err_t xincx_rtc_time_set(xincx_rtc_t const * const p_instance,
                           xincx_rtc_match_timer_ch_t                 channel,
                           xincx_rtc_match_config_t  config ,
                           bool                     enable_irq);

/**
 * @brief Function for disabling a channel.
 *
 * This function disables channel  events and channel interrupts.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] channel    One of the channels of the instance.
 *
 * @retval NRFX_SUCCESS       The procedure is successful.
 * @retval NRFX_ERROR_TIMEOUT Interrupt is pending on the requested channel.
 */
nrfx_err_t xincx_rtc_time_disable(xincx_rtc_t const * const p_instance, xincx_rtc_match_timer_ch_t channel);
						 
/**
 * @brief Function for enabling the sec event.
 *
 * This function enables the sec event and optionally the interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] enable_irq True to enable the interrupt. False to disable the interrupt.
 */
void xincx_rtc_sec_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq);						 

/**
 * @brief Function for disabling the sec event.
 *
 * This function disables the sec event and interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_rtc_sec_int_disable(xincx_rtc_t const * const p_instance);

/**
 * @brief Function for enabling the min event.
 *
 * This function enables the min event and optionally the interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] enable_irq True to enable the interrupt. False to disable the interrupt.
 */
void xincx_rtc_min_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq);						 

/**
 * @brief Function for disabling the min event.
 *
 * This function disables the min event and interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_rtc_min_int_disable(xincx_rtc_t const * const p_instance);

/**
 * @brief Function for enabling the hour event.
 *
 * This function enables the hour event and optionally the interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] enable_irq True to enable the interrupt. False to disable the interrupt.
 */
void xincx_rtc_hour_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq);						 

/**
 * @brief Function for disabling the hour event.
 *
 * This function disables the hour event and interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_rtc_hour_int_disable(xincx_rtc_t const * const p_instance);

/**
 * @brief Function for enabling the day event.
 *
 * This function enables the day event and optionally the interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] enable_irq True to enable the interrupt. False to disable the interrupt.
 */
void xincx_rtc_day_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq);						 

/**
 * @brief Function for disabling the day event.
 *
 * This function disables the day event and interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_rtc_day_int_disable(xincx_rtc_t const * const p_instance);

/**
 * @brief Function for enabling the Match Time 1 event.
 *
 * This function enables the Match Time 1 event and optionally the interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] enable_irq True to enable the interrupt. False to disable the interrupt.
 */
void xincx_rtc_time1_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq);						 

/**
 * @brief Function for disabling the Match Time 1 event.
 *
 * This function disables the Match Time 1 event and interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_rtc_time1_int_disable(xincx_rtc_t const * const p_instance);

/**
 * @brief Function for enabling the Match Time 2 event.
 *
 * This function enables the Match Time 2 event and optionally the interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] enable_irq True to enable the interrupt. False to disable the interrupt.
 */
void xincx_rtc_time2_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq);						 

/**
 * @brief Function for disabling the Match Time 2 event.
 *
 * This function disables the Match Time 2 event and interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_rtc_time2_int_disable(xincx_rtc_t const * const p_instance);


/**
 * @brief Function for enabling the Match Time 3 event.
 *
 * This function enables the Match Time 3 event and optionally the interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 * @param[in] enable_irq True to enable the interrupt. False to disable the interrupt.
 */
void xincx_rtc_time3_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq);						 

/**
 * @brief Function for disabling the Match Time 3 event.
 *
 * This function disables the Match Time 3 event and interrupt.
 *
 * @param[in] p_instance Pointer to the driver instance structure.
 */
void xincx_rtc_time3_int_disable(xincx_rtc_t const * const p_instance);


void xincx_rtc_date_set(xincx_rtc_t const * const p_instance,                         
                           nrf_rtc_time_t  date);

void xincx_rtc_date_get(xincx_rtc_t const * const p_instance,                         
                           nrf_rtc_time_t  *date);



void xincx_rtc_AOtime_set(xincx_rtc_t const * const p_instance,                         
                           uint32_t  tick);
#endif // NRFX_H__
