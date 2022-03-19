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

#ifndef XINC_RTC_H
#define XINC_RTC_H

#include <nrfx.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xinc_rtc_hal RTC HAL
 * @{
 * @ingroup xinc_rtc
 * @brief   Hardware access layer for managing the Real Time Counter (RTC) peripheral.
 */

/** @brief Macro for getting the number of compare channels available in a given RTC instance. */
#define XINC_RTC_CC_CHANNEL_COUNT(id)  NRFX_CONCAT_3(RTC, id, _CC_NUM)

/** @brief Input frequency of the RTC instance. */
#define RTC_INPUT_FREQ 32768

/** @brief Macro for converting expected frequency to prescaler setting. */
#define RTC_FREQ_TO_PRESCALER(FREQ) (uint16_t)(((RTC_INPUT_FREQ) / (FREQ)) - 1)

/** @brief Macro for trimming values to the RTC bit width. */
#define RTC_WRAP(val) ((val) & RTC_COUNTER_COUNTER_Msk)



/** @brief RTC interrupts. */
typedef enum
{
    XINC_RTC_INT_DAY_MASK     = RTC_ICR_DaE_Msk,     /**< RTC interrupt from tick event. */
    XINC_RTC_INT_HOUR_MASK = RTC_ICR_HoE_Msk,   /**< RTC interrupt from overflow event. */
    XINC_RTC_INT_MIN_MASK = RTC_ICR_MiE_Msk, /**< RTC interrupt from compare event on channel 0. */
    XINC_RTC_INT_SEC_MASK = RTC_ICR_SeE_Msk, /**< RTC interrupt from compare event on channel 1. */
    XINC_RTC_INT_TIME2_MASK = RTC_ICR_T2E_Msk, /**< RTC interrupt from compare event on channel 2. */
    XINC_RTC_INT_TIME1_MASK = RTC_ICR_T1E_Msk,  /**< RTC interrupt from compare event on channel 3. *///RTC_INTENSET_COMPARE3_Msk
    XINC_RTC_INT_TIME3_MASK = RTC_ICR_T3E_Msk ,
    XINC_RTC_INT_ALL_MASK = RTC_ICR_MASK_ALL_Msk 
} xinc_rtc_int_t;

/** @brief RTC interrupts. */
typedef enum
{
    XINC_RTC_TYPE_RTC     = (0x01 << 0),     
    XINC_RTC_TYPE_AOTIME =  (0x01 << 1),
} xinc_rtc_type_t;

/** @brief RTC driver instance structure. */
typedef struct
{
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t week; 
    uint16_t day;      
} xinc_rtc_time_t;

/**
 * @brief Function for enabling interrupts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Interrupt mask to be enabled.
 */
__STATIC_INLINE void xinc_rtc_int_enable(XINC_RTC_Type * p_reg, uint32_t mask);

/**
 * @brief Function for disabling interrupts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Interrupt mask to be disabled.
 */
__STATIC_INLINE void xinc_rtc_int_disable(XINC_RTC_Type * p_reg, uint32_t mask);

/**
 * @brief Function for checking if interrupts are enabled.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of interrupt flags to be checked.
 *
 * @return Mask with enabled interrupts.
 */
__STATIC_INLINE uint32_t xinc_rtc_int_is_enabled(XINC_RTC_Type * p_reg, uint32_t mask);

/**
 * @brief Function for returning the status of currently enabled interrupts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return Value in INTEN register.
 */
__STATIC_INLINE uint32_t xinc_rtc_int_get(XINC_RTC_Type * p_reg);




#ifndef SUPPRESS_INLINE_IMPLEMENTATION




__STATIC_INLINE void xinc_rtc_int_enable(XINC_RTC_Type * p_reg, uint32_t mask)
{
    p_reg->ICR |= mask;
}

__STATIC_INLINE void xinc_rtc_int_disable(XINC_RTC_Type * p_reg, uint32_t mask)
{
    p_reg->ICR &= ~(mask);
}

__STATIC_INLINE uint32_t xinc_rtc_int_is_enabled(XINC_RTC_Type * p_reg, uint32_t mask)
{
    return (p_reg->ICR & mask);
}

__STATIC_INLINE uint32_t xinc_rtc_int_get(XINC_RTC_Type * p_reg)
{
    return p_reg->ICR;
}



__STATIC_INLINE void xinc_rtc_date_set(XINC_RTC_Type * p_reg, xinc_rtc_time_t data)
{	
    uint32_t reg_val;;
    reg_val = ( ((data.day<< RTC_CLR_DAY_Pos) & RTC_CLR_DAY_Msk)   |
                ((data.hour<<RTC_CLR_HOUR_Pos) & RTC_CLR_HOUR_Msk) |
                ((data.min << RTC_CLR_MIN_Pos) & RTC_CLR_MIN_Msk)  |  
                ((data.sec << RTC_CLR_SEC_Pos)& RTC_CLR_SEC_Msk));
    p_reg->CLR = reg_val;

    reg_val = (data.week << RTC_WLR_Pos) & RTC_WLR_Msk;

    p_reg->WLR = reg_val;
}

__STATIC_INLINE void xinc_rtc_datelimit_set(XINC_RTC_Type * p_reg, uint8_t* val)
{
    uint32_t reg;
    reg = p_reg->HOUR_LIMIT;
    reg &= ~RTC_HOUR_LIMIT_Msk;
    reg |= (val[0] << RTC_HOUR_LIMIT_Pos) & RTC_HOUR_LIMIT_Msk;
    p_reg->HOUR_LIMIT = reg;
    
    reg = p_reg->MINUTE_LIMIT;
    reg &= ~RTC_MINUTE_LIMIT_Msk;
    reg |= (val[1] << RTC_MINUTE_LIMIT_Pos) & RTC_MINUTE_LIMIT_Msk;
    p_reg->MINUTE_LIMIT = reg;
    
    reg = p_reg->SECOND_LIMIT;
    reg &= ~RTC_SECOND_LIMIT_Msk;
    reg |= (val[2] << RTC_SECOND_LIMIT_Pos) & RTC_SECOND_LIMIT_Msk;
    p_reg->SECOND_LIMIT = reg;
    
	printf("datelimit=[hour:%d,min:%d,sec:%d]\n",p_reg->HOUR_LIMIT, p_reg->MINUTE_LIMIT, p_reg->SECOND_LIMIT);	
}

__STATIC_INLINE void xinc_rtc_freq_set(XINC_RTC_Type * p_reg, uint16_t val)
{
    p_reg->RAW_LIMIT = (val << RTC_RAW_LIMIT_Pos) & RTC_RAW_LIMIT_Msk;
}

__STATIC_INLINE void xinc_rtc_date_get(XINC_RTC_Type * p_reg, xinc_rtc_time_t *time)
{
    uint32_t tmp_CCVR,tmp_WVR;
    tmp_CCVR = p_reg->CCVR;
    tmp_WVR = p_reg->WVR;

    time->sec = (tmp_CCVR & RTC_CCVR_SEC_Msk) >> RTC_CCVR_SEC_Pos;

    time->min =  (tmp_CCVR & RTC_CCVR_MIN_Msk) >> RTC_CCVR_MIN_Pos;
    time->hour =  (tmp_CCVR & RTC_CCVR_HOUR_Msk) >> RTC_CCVR_HOUR_Pos;
    time->day =  (tmp_CCVR & RTC_CCVR_DAY_Msk) >> RTC_CCVR_DAY_Pos;
    time->week =  (tmp_WVR & RTC_WVR_Msk) >> RTC_WVR_Pos;
}

#endif

/** @} */

#ifdef __cplusplus
}
#endif

#endif  /* XINC_RTC_H */
