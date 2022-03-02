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

#ifndef NRF_RTC_H
#define NRF_RTC_H

#include <nrfx.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup nrf_rtc_hal RTC HAL
 * @{
 * @ingroup nrf_rtc
 * @brief   Hardware access layer for managing the Real Time Counter (RTC) peripheral.
 */

/** @brief Macro for getting the number of compare channels available in a given RTC instance. */
#define NRF_RTC_CC_CHANNEL_COUNT(id)  NRFX_CONCAT_3(RTC, id, _CC_NUM)

/** @brief Input frequency of the RTC instance. */
#define RTC_INPUT_FREQ 32768

/** @brief Macro for converting expected frequency to prescaler setting. */
#define RTC_FREQ_TO_PRESCALER(FREQ) (uint16_t)(((RTC_INPUT_FREQ) / (FREQ)) - 1)

/** @brief Macro for trimming values to the RTC bit width. */
#define RTC_WRAP(val) ((val) & RTC_COUNTER_COUNTER_Msk)

/** @brief Macro for creating the interrupt bitmask for the specified compare channel. */
#define RTC_CHANNEL_INT_MASK(ch)    ((uint32_t)(NRF_RTC_INT_COMPARE0_MASK) << (ch))



/** @brief RTC interrupts. */
typedef enum
{
    NRF_RTC_INT_DAY_MASK     = (0x01 << 0),     /**< RTC interrupt from tick event. */
    NRF_RTC_INT_HOUR_MASK = (0x01 << 1),   /**< RTC interrupt from overflow event. */
    NRF_RTC_INT_MIN_MASK = (0x01 << 2), /**< RTC interrupt from compare event on channel 0. */
    NRF_RTC_INT_SEC_MASK = (0x01 << 3), /**< RTC interrupt from compare event on channel 1. */
    NRF_RTC_INT_TIME2_MASK = (0x01 << 4), /**< RTC interrupt from compare event on channel 2. */
    NRF_RTC_INT_TIME1_MASK = (0x01 << 5),  /**< RTC interrupt from compare event on channel 3. *///RTC_INTENSET_COMPARE3_Msk
		NRF_RTC_INT_TIME3_MASK = (0x01 << 6) ,
	  NRF_RTC_INT_ALL_MASK = (0x01 << 7) 
} nrf_rtc_int_t;

/** @brief RTC driver instance structure. */
typedef struct
{
  uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t         week; 
  uint16_t         day;      
} nrf_rtc_time_t;

/**
 * @brief Function for enabling interrupts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Interrupt mask to be enabled.
 */
__STATIC_INLINE void nrf_rtc_int_enable(NRF_RTC_Type * p_reg, uint32_t mask);

/**
 * @brief Function for disabling interrupts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Interrupt mask to be disabled.
 */
__STATIC_INLINE void nrf_rtc_int_disable(NRF_RTC_Type * p_reg, uint32_t mask);

/**
 * @brief Function for checking if interrupts are enabled.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of interrupt flags to be checked.
 *
 * @return Mask with enabled interrupts.
 */
__STATIC_INLINE uint32_t nrf_rtc_int_is_enabled(NRF_RTC_Type * p_reg, uint32_t mask);

/**
 * @brief Function for returning the status of currently enabled interrupts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return Value in INTEN register.
 */
__STATIC_INLINE uint32_t nrf_rtc_int_get(NRF_RTC_Type * p_reg);



/**
 * @brief Function for returning a counter value.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return Counter value.
 */
__STATIC_INLINE uint32_t nrf_rtc_counter_get(NRF_RTC_Type * p_reg);

/**
 * @brief Function for setting a prescaler value.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] val   Value to set the prescaler to.
 */
__STATIC_INLINE void nrf_rtc_prescaler_set(NRF_RTC_Type * p_reg, uint32_t val);




#ifndef SUPPRESS_INLINE_IMPLEMENTATION

__STATIC_INLINE  void nrf_rtc_cc_set(NRF_RTC_Type * p_reg, uint32_t ch, uint32_t cc_val)
{
    //p_reg->CC[ch] = cc_val;
}

__STATIC_INLINE  uint32_t nrf_rtc_cc_get(NRF_RTC_Type * p_reg, uint32_t ch)
{
    //return p_reg->CC[ch];
}

__STATIC_INLINE void nrf_rtc_int_enable(NRF_RTC_Type * p_reg, uint32_t mask)
{
    p_reg->ICR |= mask;
}

__STATIC_INLINE void nrf_rtc_int_disable(NRF_RTC_Type * p_reg, uint32_t mask)
{
    p_reg->ICR &= ~(mask);
}

__STATIC_INLINE uint32_t nrf_rtc_int_is_enabled(NRF_RTC_Type * p_reg, uint32_t mask)
{
    //return (p_reg->INTENSET & mask);
}

__STATIC_INLINE uint32_t nrf_rtc_int_get(NRF_RTC_Type * p_reg)
{
    //return p_reg->INTENSET;
}



__STATIC_INLINE uint32_t nrf_rtc_counter_get(NRF_RTC_Type * p_reg)
{
     //return p_reg->COUNTER;
}

__STATIC_INLINE void nrf_rtc_prescaler_set(NRF_RTC_Type * p_reg, uint32_t val)
{
    NRFX_ASSERT(val <= (RTC_PRESCALER_PRESCALER_Msk >> RTC_PRESCALER_PRESCALER_Pos));
    //p_reg->PRESCALER = val;
}
__STATIC_INLINE uint32_t rtc_prescaler_get(NRF_RTC_Type * p_reg)
{
    //return p_reg->PRESCALER;
}

__STATIC_INLINE void nrf_rtc_date_set(NRF_RTC_Type * p_reg, uint8_t* val)
{	
	uint32_t reg_val = p_reg->CLR;

	reg_val = ( val[0]<<17 | val[1]<<12 | val[2]<<6 | val[3] );
	p_reg->CLR = reg_val;
	reg_val = p_reg->CLR;
}

__STATIC_INLINE void nrf_rtc_datelimit_set(NRF_RTC_Type * p_reg, uint8_t* val)
{
 
	uint32_t reg_val;
		
	reg_val = p_reg->HOUR_LIMIT;
	reg_val |= val[0] ;
	p_reg->HOUR_LIMIT = reg_val;
	
	reg_val = p_reg->MINUTE_LIMIT;
	reg_val |= val[1] ;
	p_reg->MINUTE_LIMIT = reg_val;
	
	reg_val = p_reg->SECOND_LIMIT;
	reg_val |= val[2] ;
	p_reg->SECOND_LIMIT = reg_val;
	
	printf("datelimit=[hour:%d,min:%d,sec:%d]\n",p_reg->HOUR_LIMIT, p_reg->MINUTE_LIMIT, p_reg->SECOND_LIMIT);	
}

__STATIC_INLINE void nrf_rtc_freq_set(NRF_RTC_Type * p_reg, uint16_t val)
{
	p_reg->RAW_LIMIT = val;
}

__STATIC_INLINE void nrf_rtc_val_get(NRF_RTC_Type * p_reg, nrf_rtc_time_t *time)
{
	uint32_t tmp_CCVR,tmp_WVR;
	tmp_CCVR = p_reg->CCVR;
	tmp_WVR = p_reg->WVR;

	time->sec = (tmp_CCVR)  &0x3F;

	time->min =  (tmp_CCVR >> 6) &0x3F;
	time->hour =  (tmp_CCVR >>12) &0x1f;
	time->day =  (tmp_CCVR >>17) &0x7fff;
	time->week =  tmp_WVR &0x07;
}



#endif

/** @} */

#ifdef __cplusplus
}
#endif

#endif  /* NRF_RTC_H */
