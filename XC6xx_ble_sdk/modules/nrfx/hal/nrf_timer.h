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

#ifndef NRF_TIMER_H__
#define NRF_TIMER_H__

#include <nrfx.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup nrf_timer_hal TIMER HAL
 * @{
 * @ingroup nrf_timer
 * @brief   Hardware access layer for managing the TIMER peripheral.
 */

/**
 * @brief Macro for getting the maximum bit resolution of the specified timer instance.
 *
 * @param[in] id Index of the specified timer instance.
 *
 * @retval Maximum bit resolution of the specified timer instance.
 */
#define TIMER_MAX_SIZE(id)  NRFX_CONCAT_3(TIMER, id, _MAX_SIZE)

/**
 * @brief Macro for validating the correctness of the bit width resolution setting.
 *
 * @param[in] id        Index of the specified timer instance.
 * @param[in] bit_width Bit width resolution value to be checked.
 *
 * @retval true  Timer instance supports the specified bit width resolution value.
 * @retval false Timer instance does not support the specified bit width resolution value.
 */
#define TIMER_BIT_WIDTH_MAX(id, bit_width) \
    (TIMER_MAX_SIZE(id) == 8   ? (bit_width == NRF_TIMER_BIT_WIDTH_8)  :  \
    (TIMER_MAX_SIZE(id) == 16  ? (bit_width == NRF_TIMER_BIT_WIDTH_8)  || \
                                 (bit_width == NRF_TIMER_BIT_WIDTH_16)  : \
    (TIMER_MAX_SIZE(id) == 24  ? (bit_width == NRF_TIMER_BIT_WIDTH_8)  || \
                                 (bit_width == NRF_TIMER_BIT_WIDTH_16) || \
                                 (bit_width == NRF_TIMER_BIT_WIDTH_24) :  \
    (TIMER_MAX_SIZE(id) == 32  ? (bit_width == NRF_TIMER_BIT_WIDTH_8)  || \
                                 (bit_width == NRF_TIMER_BIT_WIDTH_16) || \
                                 (bit_width == NRF_TIMER_BIT_WIDTH_24) || \
                                 (bit_width == NRF_TIMER_BIT_WIDTH_32) :  \
    false))))

/**
 * @brief Macro for checking correctness of bit width configuration for the specified timer.
 *
 * @param[in] p_reg     Timer instance register.
 * @param[in] bit_width Bit width resolution value to be checked.
 *
 * @retval true  Timer instance supports the specified bit width resolution value.
 * @retval false Timer instance does not support the specified bit width resolution value.
 */
#if (TIMER_COUNT == 3) || defined(__NRFX_DOXYGEN__)
    #define NRF_TIMER_IS_BIT_WIDTH_VALID(p_reg, bit_width) (              \
           ((p_reg == NRF_TIMER0) && TIMER_BIT_WIDTH_MAX(0, bit_width))   \
        || ((p_reg == NRF_TIMER1) && TIMER_BIT_WIDTH_MAX(1, bit_width))   \
        || ((p_reg == NRF_TIMER2) && TIMER_BIT_WIDTH_MAX(2, bit_width)))
#elif (TIMER_COUNT == 4)
    #define NRF_TIMER_IS_BIT_WIDTH_VALID(p_reg, bit_width) (              \
           ((p_reg == NRF_TIMER0) && TIMER_BIT_WIDTH_MAX(0, bit_width))   \
        || ((p_reg == NRF_TIMER1) && TIMER_BIT_WIDTH_MAX(1, bit_width))   \
        || ((p_reg == NRF_TIMER2) && TIMER_BIT_WIDTH_MAX(2, bit_width))   \
        || ((p_reg == NRF_TIMER3) && TIMER_BIT_WIDTH_MAX(3, bit_width)))
#elif (TIMER_COUNT == 5)
    #define NRF_TIMER_IS_BIT_WIDTH_VALID(p_reg, bit_width) (              \
           ((p_reg == NRF_TIMER0) && TIMER_BIT_WIDTH_MAX(0, bit_width))   \
        || ((p_reg == NRF_TIMER1) && TIMER_BIT_WIDTH_MAX(1, bit_width))   \
        || ((p_reg == NRF_TIMER2) && TIMER_BIT_WIDTH_MAX(2, bit_width))   \
        || ((p_reg == NRF_TIMER3) && TIMER_BIT_WIDTH_MAX(3, bit_width))   \
        || ((p_reg == NRF_TIMER4) && TIMER_BIT_WIDTH_MAX(4, bit_width)))
#else
    #error "Not supported timer count"
#endif



/** @brief Timer events. */
typedef enum
{
	NRF_TIMER_EVENT_TIMEOUT = 0x01 << 0,

} nrf_timer_int_event_t;

/** @brief Timer modes. */
typedef enum
{
    NRF_TIMER_MODE_AUTO_TIMER             = 0,//TIMER_MODE_MODE_Timer,           ///< Timer mode: timer.
    NRF_TIMER_MODE_USER_COUNTER           =  1,//TIMER_MODE_MODE_Counter,         ///< Timer mode: counter.
} nrf_timer_mode_t;

/** @brief Timer bit width. */
typedef enum
{
    NRF_TIMER_CLK_SRC_32M_DIV = 0, ///< Timer CLK SRC 32MHz div.
    NRF_TIMER_CLK_SRC_32K_DIV = 1, ///< Timer CLK SRC 32kHz div.
	  NRF_TIMER_CLK_SRC_32K = 4, ///< TimerCLK SRC 32kHz.
} nrf_timer_clk_src_t;

/** @brief Timer prescalers. */
typedef enum
{
    NRF_TIMER_FREQ_16MHz = 0, 		///< Timer frequency 16 MHz or 16KHz
    NRF_TIMER_FREQ_8MHz = 1,      ///< Timer frequency 8 MHz or 8KHz
    NRF_TIMER_FREQ_4MHz = 2,      ///< Timer frequency 4 MHz or 4KHz.
    NRF_TIMER_FREQ_2MHz = 3,      ///< Timer frequency 2 MHz or 2KHz.
    NRF_TIMER_FREQ_1MHz = 4,      ///< Timer frequency 1 MHz or 1KHz.
    NRF_TIMER_FREQ_500kHz = 5,    ///< Timer frequency 500 kHz or 500Hz.
    NRF_TIMER_FREQ_250kHz = 6,    ///< Timer frequency 250 kHz or 250Hz.
    NRF_TIMER_FREQ_125kHz = 7,    ///< Timer frequency 125 kHz or 125Hz.
    NRF_TIMER_FREQ_62500Hz = 8,   ///< Timer frequency 62500 Hz or 62.5Hz.
    NRF_TIMER_FREQ_32000Hz = 9    ///< Timer frequency 32000 Hz.
} nrf_timer_frequency_t;


/** @brief Timer interrupts. */
typedef enum
{
    NRF_TIMER_INT_COMPARE0_MASK = TIMER_INTENSET_COMPARE0_Msk, ///< Timer interrupt from compare event on channel 0.
    NRF_TIMER_INT_COMPARE1_MASK = TIMER_INTENSET_COMPARE1_Msk, ///< Timer interrupt from compare event on channel 1.
    NRF_TIMER_INT_COMPARE2_MASK = TIMER_INTENSET_COMPARE2_Msk, ///< Timer interrupt from compare event on channel 2.
    NRF_TIMER_INT_COMPARE3_MASK = TIMER_INTENSET_COMPARE3_Msk, ///< Timer interrupt from compare event on channel 3.
#if defined(TIMER_INTENSET_COMPARE4_Msk) || defined(__NRFX_DOXYGEN__)
    NRF_TIMER_INT_COMPARE4_MASK = TIMER_INTENSET_COMPARE4_Msk, ///< Timer interrupt from compare event on channel 4.
#endif
#if defined(TIMER_INTENSET_COMPARE5_Msk) || defined(__NRFX_DOXYGEN__)
    NRF_TIMER_INT_COMPARE5_MASK = TIMER_INTENSET_COMPARE5_Msk, ///< Timer interrupt from compare event on channel 5.
#endif
} nrf_timer_int_mask_t;




/**
 * @brief Function for clearing the specified timer event.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] event Event to clear.
 */
__STATIC_INLINE void nrf_timer_int_clear(NRF_TIMER_Type *  p_reg,
                                           nrf_timer_int_event_t event);

/**
 * @brief Function for retrieving the state of the TIMER event.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] event Event to be checked.
 *
 * @retval true  The event has been generated.
 * @retval false The event has not been generated.
 */
__STATIC_INLINE bool nrf_timer_int_check(NRF_TIMER_Type *  p_reg,
                                           nrf_timer_int_event_t event);

#endif


/**
 * @brief Function for enabling the specified interrupts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of interrupts to be enabled.
 */
__STATIC_INLINE void nrf_timer_int_enable(NRF_TIMER_Type * p_reg,
                                          uint32_t         mask);

/**
 * @brief Function for disabling the specified interrupts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of interrupts to be disabled.
 */
__STATIC_INLINE void nrf_timer_int_disable(NRF_TIMER_Type * p_reg,
                                           uint32_t         mask);

/**
 * @brief Function for retrieving the state of a given interrupt.
 *
 * @param[in] p_reg     Pointer to the structure of registers of the peripheral.
 * @param[in] timer_int Interrupt to be checked.
 *
 * @retval true  The interrupt is enabled.
 * @retval false The interrupt is not enabled.
 */
__STATIC_INLINE bool nrf_timer_int_enable_check(NRF_TIMER_Type * p_reg,
                                                uint32_t         timer_int);

/**
 * @brief Function for setting the timer mode.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mode  Timer mode.
 */
__STATIC_INLINE void nrf_timer_mode_set(NRF_TIMER_Type * p_reg,
                                        nrf_timer_mode_t mode);

/**
 * @brief Function for retrieving the timer mode.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return Timer mode.
 */
__STATIC_INLINE nrf_timer_mode_t nrf_timer_mode_get(NRF_TIMER_Type * p_reg);



/**
 * @brief Function for setting the timer frequency.
 *
 * @param[in] p_reg     Pointer to the structure of registers of the peripheral.
 * @param[in] frequency Timer frequency.
 */
__STATIC_INLINE void nrf_timer_frequency_div_set(XINC_CPR_CTL_Type *      p_reg,uint8_t id,nrf_timer_clk_src_t clk_src,
                                             nrf_timer_frequency_t frequency);

/**
 * @brief Function for retrieving the timer frequency.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return Timer frequency.
 */
__STATIC_INLINE uint32_t nrf_timer_frequency_div_get(XINC_CPR_CTL_Type * p_reg,uint8_t id);

/**
 * @brief Function for writing the capture/compare register for the specified channel.
 *
 * @param[in] p_reg      Pointer to the structure of registers of the peripheral.
 * @param[in] cc_channel The specified capture/compare channel.
 * @param[in] cc_value   Value to write to the capture/compare register.
 */
__STATIC_INLINE void nrf_timer_cc_write(NRF_TIMER_Type *       p_reg,
                                        uint32_t               cc_value);

/**
 * @brief Function for retrieving the capture/compare value for a specified channel.
 *
 * @param[in] p_reg      Pointer to the structure of registers of the peripheral.
 * @param[in] cc_channel The specified capture/compare channel.
 *
 * @return Value from the specified capture/compare register.
 */
__STATIC_INLINE uint32_t nrf_timer_cc_read(NRF_TIMER_Type *       p_reg);


/**
 * @brief Function for calculating the number of timer ticks for a given time
 *        (in microseconds) and timer frequency.
 *
 * @param[in] time_us   Time in microseconds.
 * @param[in] frequency Timer frequency.
 *
 * @return Number of timer ticks.
 */
__STATIC_INLINE uint32_t nrf_timer_us_to_ticks(uint32_t              time_us,
                                               uint32_t frequency);

/**
 * @brief Function for calculating the number of timer ticks for a given time
 *        (in milliseconds) and timer frequency.
 *
 * @param[in] time_ms   Time in milliseconds.
 * @param[in] frequency Timer frequency.
 *
 * @return Number of timer ticks.
 */
__STATIC_INLINE uint32_t nrf_timer_ms_to_ticks(uint32_t              time_ms,
                                               uint32_t frequency);


#ifndef SUPPRESS_INLINE_IMPLEMENTATION



__STATIC_INLINE void nrf_timer_int_clear(NRF_TIMER_Type * p_reg,nrf_timer_int_event_t event)
{
   event = p_reg->TIC;

}

__STATIC_INLINE bool nrf_timer_int_check(NRF_TIMER_Type * p_reg,
                                           nrf_timer_int_event_t event)
{
	  return (bool)(p_reg->TIS & event);
}


__STATIC_INLINE void nrf_timer_int_enable(NRF_TIMER_Type * p_reg,
                                          uint32_t mask)
{
	p_reg->TCR &= ~(mask);
}

__STATIC_INLINE void nrf_timer_int_disable(NRF_TIMER_Type * p_reg,
                                           uint32_t mask)
{
	p_reg->TCR |= (mask);

}

__STATIC_INLINE bool nrf_timer_int_enable_check(NRF_TIMER_Type * p_reg,
                                                uint32_t timer_int)
{
    return (bool)!(p_reg->TCR & (timer_int));
}


__STATIC_INLINE void nrf_timer_mode_set(NRF_TIMER_Type * p_reg,
                                        nrf_timer_mode_t mode)
{
//    p_reg->TCR = (p_reg->TCR & ~TIMER_MODE_MODE_Msk) |
//                    ((mode << TIMER_MODE_MODE_Pos) & TIMER_MODE_MODE_Msk);
	uint32_t reg  = p_reg->TCR;
	reg &= ~(0x01 << 1);
	reg |= (mode << 1);
	p_reg->TCR = reg;
}

__STATIC_INLINE nrf_timer_mode_t nrf_timer_mode_get(NRF_TIMER_Type * p_reg)
{
  return (nrf_timer_mode_t)(p_reg->TCR & 0x02) >> 1;
}



__STATIC_INLINE void nrf_timer_frequency_div_set(XINC_CPR_CTL_Type * p_reg,uint8_t id,nrf_timer_clk_src_t clk_src,
                                             nrf_timer_frequency_t frequency)
{
	volatile uint32_t *Timer0CtlBaseAddr = (uint32_t*)&(p_reg->TIMER0_CLK_CTL);
	
	Timer0CtlBaseAddr+= id;
	switch(clk_src)
	{
		case NRF_TIMER_CLK_SRC_32M_DIV:
		{
			*Timer0CtlBaseAddr = (1 << frequency) - 1 ;
		}break;
		
		case NRF_TIMER_CLK_SRC_32K_DIV:
		{
			*Timer0CtlBaseAddr = ((1 << frequency) - 1) | (0x01 << 28) ;
		}break;
		
		case NRF_TIMER_CLK_SRC_32K:
		{
			*Timer0CtlBaseAddr = (0x04 << 28);
		}break;
		
		default:
			break;
	
	}
	
	printf("timer clock id:%d, addr=[%p][%p],val=[%d],freq=[%d]\n",id,&p_reg->TIMER0_CLK_CTL,Timer0CtlBaseAddr,*Timer0CtlBaseAddr,frequency);
}

__STATIC_INLINE uint32_t nrf_timer_frequency_div_get(XINC_CPR_CTL_Type * p_reg,uint8_t id)
{

	uint32_t clk_div = 0;
	nrf_timer_clk_src_t clk_src;
	volatile uint32_t *Timer0CtlBaseAddr = (uint32_t*)&(p_reg->TIMER0_CLK_CTL);
	
	Timer0CtlBaseAddr+= id;
	
	clk_div = (*Timer0CtlBaseAddr);
//	switch(clk_src)
//	{
//		case NRF_TIMER_CLK_SRC_32M_DIV:
//		{
//			clk_div = *Timer0CtlBaseAddr & 0xff;
//		}break;
//		
//		case NRF_TIMER_CLK_SRC_32K_DIV:
//		{
//			clk_div = (*Timer0CtlBaseAddr & 0xff00) >> 8;
//		}break;
//		
//		case NRF_TIMER_CLK_SRC_32K:
//		{
//			clk_div = 0;
//		}break;
//		
//		default:
//			break;
//	
//	}
//	
	
	return (uint32_t)clk_div;
	
}

__STATIC_INLINE void nrf_timer_cc_write(NRF_TIMER_Type * p_reg,
                                        uint32_t               cc_value)
{
	p_reg->TLC = cc_value;
	printf("%s,p_reg->TLC:%d,ticks=[%d],\n",__FUNCTION__ ,p_reg->TLC,cc_value);
}

__STATIC_INLINE uint32_t nrf_timer_cc_read(NRF_TIMER_Type * p_reg )
{
	return p_reg->TLC;
}



__STATIC_INLINE uint32_t nrf_timer_us_to_ticks(uint32_t              time_us,
                                               uint32_t frequency)
{
    // The "frequency" parameter here is actually the prescaler value, and the
    // timer runs at the following frequency: f = 16 MHz / 2^prescaler.
    nrf_timer_clk_src_t clk_src  = (frequency >> 28) & 0x07;
		uint32_t prescaler;
		uint32_t clk_base;
		uint32_t ticks;
		switch(clk_src)
		{
		case NRF_TIMER_CLK_SRC_32M_DIV:
		{
			prescaler = frequency & 0xFF;
			clk_base = 16ULL;
			ticks = ((time_us * clk_base) /(prescaler + 1));
		}break;
		
		case NRF_TIMER_CLK_SRC_32K_DIV:
		{
			prescaler = (frequency >> 8 )& 0xFF;
			clk_base = 16ULL;
			ticks = ((time_us * clk_base) /(prescaler + 1)) / 1000;
		}break;
		
		case NRF_TIMER_CLK_SRC_32K:
		{
			prescaler = 0;
			clk_base = 32ULL;
			ticks = ((time_us * clk_base) /(prescaler + 1)) / 1000;
		}break;
		
		default:
			break;
	
		} 
		if(ticks < 4) ticks = 4;
		printf("%s,frequency:%x,clk_src:%d,time_us:%d,ticks=[%d],presc=[%d]\n",__FUNCTION__ ,frequency,clk_src,time_us,ticks ,prescaler);
    NRFX_ASSERT(ticks <= UINT32_MAX);
    return (uint32_t)ticks;
}

__STATIC_INLINE uint32_t nrf_timer_ms_to_ticks(uint32_t              time_ms,
                                               uint32_t frequency)
{
    // The "frequency" parameter here is actually the prescaler value, and the
    // timer runs at the following frequency: f = 16000 kHz / 2^prescaler.
	// xinchip f=32000kHz/(2*(prescaler+1))
		nrf_timer_clk_src_t clk_src  = (frequency >> 28) & 0x07;
		uint32_t prescaler;
		uint32_t clk_base;
		switch(clk_src)
		{
		case NRF_TIMER_CLK_SRC_32M_DIV:
		{
			prescaler = frequency & 0xFF;
			clk_base = 16000ULL;
		}break;
		
		case NRF_TIMER_CLK_SRC_32K_DIV:
		{
			prescaler = (frequency >> 8 )& 0xFF;
			clk_base = 16ULL;
		}break;
		
		case NRF_TIMER_CLK_SRC_32K:
		{
			prescaler = 0;
			clk_base = 32ULL;
		}break;
		
		default:
			break;
	
		}
    uint32_t ticks = ((time_ms * clk_base) /(prescaler + 1));
		if(ticks < 4) ticks = 4;
		printf("%s,frequency:%x,clk_src:%d,time_ms:%d,ticks=[%d],presc=[%d]\n",__FUNCTION__ ,frequency,clk_src,time_ms,ticks ,prescaler);
    NRFX_ASSERT(ticks <= UINT32_MAX);
    return (uint32_t)ticks;
}

#endif // SUPPRESS_INLINE_IMPLEMENTATION

/** @} */

#ifdef __cplusplus
}

#endif // NRF_TIMER_H__
