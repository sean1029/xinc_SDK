/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINC_TIMER_H__
#define XINC_TIMER_H__

#include <xincx.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xinc_timer_hal TIMER HAL
 * @{
 * @ingroup xinc_timer
 * @brief   Hardware access layer for managing the TIMER peripheral.
 */

/**
 * @brief Macro for checking correctness of bit width configuration for the specified timer.
 *
 * @param[in] p_reg     Timer instance register.
 * @param[in] bit_width Bit width resolution value to be checked.
 *
 * @retval true  Timer instance supports the specified bit width resolution value.
 * @retval false Timer instance does not support the specified bit width resolution value.
 */


#define TIMERx_MIN_TICKS    TIMERx_TLC_TLC_MIN_VAL                     

/** @brief Timer events. */
typedef enum
{
	XINC_TIMER_EVENT_TIMEOUT = TIMERx_TIS_TIS_Msk,

} xinc_timer_int_event_t;

/** @brief Timer modes. */
typedef enum
{
    XINC_TIMER_MODE_AUTO_TIMER             = TIMERx_TCR_TES_MODE_AUTO,//TIMER_MODE_MODE_Timer,           ///< Timer mode: timer.
    XINC_TIMER_MODE_USER_COUNTER           =  TIMERx_TCR_TES_MODE_USER_COUNTER,//TIMER_MODE_MODE_Counter,         ///< Timer mode: counter.
} xinc_timer_mode_t;

/** @brief Timer bit width. */
typedef enum
{
	XINC_TIMER_CLK_SRC_32M_DIV = 0, ///< Timer CLK SRC 32MHz div.
	XINC_TIMER_CLK_SRC_32K_DIV = 1, ///< Timer CLK SRC 32kHz div.
	XINC_TIMER_CLK_SRC_32K = 4, ///< TimerCLK SRC 32kHz.
} xinc_timer_clk_src_t;

/** @brief Timer prescalers. */
typedef enum XINC_PWM_REF_CLK_16MHzOr16K
{
    XINC_TIMER_REF_CLK_16MHzOr16K = 0, 		///< Timer ref clk 16 MHz or 16KHz
    XINC_TIMER_REF_CLK_8MHzOr8K = 1,      ///< Timer ref clk 8 MHz or 8KHz
    XINC_TIMER_REF_CLK_4MHzOr4K = 2,      ///< Timer ref clk 4 MHz or 4KHz.
    XINC_TIMER_REF_CLK_2MHzOr2K = 3,      ///< Timer ref clk 2 MHz or 2KHz.
    XINC_TIMER_REF_CLK_1MHzOr1K = 4,      ///< Timer ref clk 1 MHz or 1KHz.
    XINC_TIMER_REF_CLK_500kHzOr500 = 5,    ///< Timer ref clk 500 kHz or 500Hz.
    XINC_TIMER_REF_CLK_250kHzOr250 = 6,    ///< Timer ref clk 250 kHz or 250Hz.
    XINC_TIMER_REF_CLK_125kHzOr125 = 7,    ///< Timer ref clk 125 kHz or 125Hz.
    XINC_TIMER_REF_CLK_62500HzOr62_5 = 8,   ///< Timer ref clk 62500 Hz or 62.5Hz.
    XINC_TIMER_REF_CLK_32000Hz = 9    ///< Timer ref clk 32000 Hz.
} xinc_timer_ref_clk_t;


/** @brief Timer interrupts. */
typedef enum
{
    XINC_TIMER_EN_MASK = TIMERx_TCR_TES_Msk, ///< Timer interrupt from compare event on channel 0.
    XINC_TIMER_MODE_MASK = TIMERx_TCR_TMS_Msk, ///< Timer interrupt from compare event on channel 1.
    XINC_TIMER_INT_MASK = TIMERx_TCR_TIM_Msk, ///< Timer interrupt from compare event on channel 2.
   
} xinc_timer_ctr_mask_t;




/**
 * @brief Function for clearing the specified timer event.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] event Event to clear.
 */
__STATIC_INLINE void xinc_timer_int_clear(XINC_TIMER_Type *  p_reg,
                                           xinc_timer_int_event_t event);

/**
 * @brief Function for retrieving the state of the TIMER event.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] event Event to be checked.
 *
 * @retval true  The event has been generated.
 * @retval false The event has not been generated.
 */
__STATIC_INLINE bool xinc_timer_int_check(XINC_TIMER_Type *  p_reg,
                                           xinc_timer_int_event_t event);

#endif

/**
 * @brief Function for enabling the specified .
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void xinc_timer_enable(XINC_TIMER_Type * p_reg);



/**
 * @brief Function for disabling the specified.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void xinc_timer_disable(XINC_TIMER_Type * p_reg);
/**
 * @brief Function for enabling the specified interrupts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of interrupts to be enabled.
 */
__STATIC_INLINE void xinc_timer_int_enable(XINC_TIMER_Type * p_reg,
                                          uint32_t         mask);

/**
 * @brief Function for disabling the specified interrupts.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mask  Mask of interrupts to be disabled.
 */
__STATIC_INLINE void xinc_timer_int_disable(XINC_TIMER_Type * p_reg,
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
__STATIC_INLINE bool xinc_timer_int_enable_check(XINC_TIMER_Type * p_reg,
                                                uint32_t         timer_int);

/**
 * @brief Function for setting the timer mode.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 * @param[in] mode  Timer mode.
 */
__STATIC_INLINE void xinc_timer_mode_set(XINC_TIMER_Type * p_reg,
                                        xinc_timer_mode_t mode);

/**
 * @brief Function for retrieving the timer mode.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return Timer mode.
 */
__STATIC_INLINE xinc_timer_mode_t xinc_timer_mode_get(XINC_TIMER_Type * p_reg);



/**
 * @brief Function for setting the timer frequency.
 *
 * @param[in] p_reg     Pointer to the structure of registers of the peripheral.
 * @param[in] frequency Timer frequency.
 */
__STATIC_INLINE void xinc_timer_clk_div_set(XINC_CPR_CTL_Type *      p_reg,uint8_t id,xinc_timer_clk_src_t clk_src,
                                             xinc_timer_ref_clk_t ref_clk);

/**
 * @brief Function for retrieving the timer frequency.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 *
 * @return Timer frequency.
 */
__STATIC_INLINE uint32_t xinc_timer_clk_div_get(XINC_CPR_CTL_Type * p_reg,uint8_t id);

/**
 * @brief Function for writing the capture/compare register for the specified channel.
 *
 * @param[in] p_reg      Pointer to the structure of registers of the peripheral.
 * @param[in] cc_value   Value to write to the capture/compare register.
 */
__STATIC_INLINE void xinc_timer_cc_write(XINC_TIMER_Type *       p_reg,
                                        uint32_t               cc_value);

/**
 * @brief Function for retrieving the capture/compare value for a specified channel.
 *
 * @param[in] p_reg      Pointer to the structure of registers of the peripheral.
 *
 * @return Value from the specified capture/compare register.
 */
__STATIC_INLINE uint32_t xinc_timer_cc_read(XINC_TIMER_Type *       p_reg);


/**
 * @brief Function for calculating the number of timer ticks for a given time
 *        (in microseconds) and timer frequency.
 *
 * @param[in] time_us   Time in microseconds.
 * @param[in] frequency Timer frequency.
 *
 * @return Number of timer ticks.
 */
__STATIC_INLINE uint32_t xinc_timer_us_to_ticks(uint32_t              time_us,
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
__STATIC_INLINE uint32_t xinc_timer_ms_to_ticks(uint32_t              time_ms,
                                               uint32_t frequency);


#ifndef SUPPRESS_INLINE_IMPLEMENTATION



__STATIC_INLINE void xinc_timer_int_clear(XINC_TIMER_Type * p_reg,xinc_timer_int_event_t event)
{
	uint32_t reg = p_reg->TIC;
}

__STATIC_INLINE bool xinc_timer_int_check(XINC_TIMER_Type * p_reg,
                                           xinc_timer_int_event_t event)
{
	return (bool)(p_reg->TIS & event);
}

__STATIC_INLINE void xinc_timer_enable(XINC_TIMER_Type * p_reg)
{
     p_reg->TCR = (p_reg->TCR & ~TIMERx_TCR_TES_Msk) |
                        ((TIMERx_TCR_TES_Enable << TIMERx_TCR_TES_Pos) & TIMERx_TCR_TES_Msk);
}

__STATIC_INLINE void xinc_timer_disable(XINC_TIMER_Type * p_reg)
{
   p_reg->TCR = (p_reg->TCR & ~TIMERx_TCR_TES_Msk) |
                        ((TIMERx_TCR_TES_Disable << TIMERx_TCR_TES_Pos) & TIMERx_TCR_TES_Msk);
}


__STATIC_INLINE void xinc_timer_int_enable(XINC_TIMER_Type * p_reg,
                                          uint32_t mask)
{
    p_reg->TCR &= ~(mask);
}

__STATIC_INLINE void xinc_timer_int_disable(XINC_TIMER_Type * p_reg,
                                           uint32_t mask)
{
    p_reg->TCR |= (mask);

}

__STATIC_INLINE bool xinc_timer_int_enable_check(XINC_TIMER_Type * p_reg,
                                                uint32_t timer_int)
{
    return (bool)!(p_reg->TCR & (timer_int));
}


__STATIC_INLINE void xinc_timer_mode_set(XINC_TIMER_Type * p_reg,
                                        xinc_timer_mode_t mode)
{
    p_reg->TCR = (p_reg->TCR & ~TIMERx_TCR_TMS_Msk) |
                        ((mode << TIMERx_TCR_TMS_Pos) & TIMERx_TCR_TMS_Msk);
//    uint32_t reg  = p_reg->TCR;
//    reg &= ~(TIMERx_TCR_TMS_Msk);
//    reg |= (mode << TIMERx_TCR_TMS_Pos);
//    p_reg->TCR = reg;
}

__STATIC_INLINE xinc_timer_mode_t xinc_timer_mode_get(XINC_TIMER_Type * p_reg)
{
  return (xinc_timer_mode_t)((p_reg->TCR & TIMERx_TCR_TMS_Msk) >> TIMERx_TCR_TMS_Pos);
}



__STATIC_INLINE void xinc_timer_clk_div_set(XINC_CPR_CTL_Type * p_reg,uint8_t id,xinc_timer_clk_src_t clk_src,
                                             xinc_timer_ref_clk_t ref_clk)
{
	volatile uint32_t *Timer0CtlBaseAddr = (uint32_t*)&(p_reg->TIMER0_CLK_CTL);

    Timer0CtlBaseAddr+= id;
    switch(clk_src)
    {
        case XINC_TIMER_CLK_SRC_32M_DIV:
        {
            *Timer0CtlBaseAddr = (1 << ref_clk) - 1 ;
        }break;
        
        case XINC_TIMER_CLK_SRC_32K_DIV:
        {
            *Timer0CtlBaseAddr = ((1 << ref_clk) - 1) | (0x01 << 28) ;
        }break;
        
        case XINC_TIMER_CLK_SRC_32K:
        {
            *Timer0CtlBaseAddr = (0x04 << 28);
        }break;
        
        default:
            break;

    }
	
    printf("timer clock id:%d, addr=[%p][%p],val=[%d],freq=[%d]\n",id,&p_reg->TIMER0_CLK_CTL,Timer0CtlBaseAddr,*Timer0CtlBaseAddr,ref_clk);
}

__STATIC_INLINE uint32_t xinc_timer_clk_div_get(XINC_CPR_CTL_Type * p_reg,uint8_t id)
{

    uint32_t clk_div = 0;
    volatile uint32_t *Timer0CtlBaseAddr = (uint32_t*)&(p_reg->TIMER0_CLK_CTL);

    Timer0CtlBaseAddr+= id;

    clk_div = (*Timer0CtlBaseAddr);

    return (uint32_t)clk_div;
	
}

__STATIC_INLINE void xinc_timer_cc_write(XINC_TIMER_Type * p_reg,
                                        uint32_t               cc_value)
{
    p_reg->TLC = cc_value;
    printf("%s,p_reg->TLC:%d,ticks=[%d],\n",__FUNCTION__ ,p_reg->TLC,cc_value);
}

__STATIC_INLINE uint32_t xinc_timer_cc_read(XINC_TIMER_Type * p_reg )
{
	return p_reg->TLC;
}



__STATIC_INLINE uint32_t xinc_timer_us_to_ticks(uint32_t              time_us,
                                               uint32_t frequency)
{
    // The "frequency" parameter here is actually the prescaler value, and the
    // timer runs at the following frequency: f = 16 MHz / 2^prescaler.
    xinc_timer_clk_src_t clk_src  = (xinc_timer_clk_src_t)((frequency >> 28) & 0x07);
    uint32_t prescaler;
    uint32_t clk_base;
    uint32_t ticks;
    switch(clk_src)
    {
        case XINC_TIMER_CLK_SRC_32M_DIV:
        {
            prescaler = frequency & 0xFF;
            clk_base = 16ULL;
            ticks = ((time_us * clk_base) /(prescaler + 1));
        }break;

        case XINC_TIMER_CLK_SRC_32K_DIV:
        {
            prescaler = (frequency >> 8 )& 0xFF;
            clk_base = 16ULL;
            ticks = ((time_us * clk_base) /(prescaler + 1)) / 1000;
        }break;

        case XINC_TIMER_CLK_SRC_32K:
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

__STATIC_INLINE uint32_t xinc_timer_ms_to_ticks(uint32_t              time_ms,
                                               uint32_t frequency)
{
    // The "frequency" parameter here is actually the prescaler value, and the
    // timer runs at the following frequency: f = 16000 kHz / 2^prescaler.
	// xinchip f=32000kHz/(2*(prescaler+1))
    xinc_timer_clk_src_t clk_src  = (xinc_timer_clk_src_t)((frequency >> 28) & 0x07);
    uint32_t prescaler;
    uint32_t clk_base;
    switch(clk_src)
    {
        case XINC_TIMER_CLK_SRC_32M_DIV:
        {
            prescaler = frequency & 0xFF;
            clk_base = 16000ULL;
        }break;
        
        case XINC_TIMER_CLK_SRC_32K_DIV:
        {
            prescaler = (frequency >> 8 )& 0xFF;
            clk_base = 16ULL;
        }break;
        
        case XINC_TIMER_CLK_SRC_32K:
        {
            prescaler = 0;
            clk_base = 32ULL;
        }break;
        
        default:
            break;

    }

    uint32_t ticks = ((time_ms * clk_base) /(prescaler + 1));
    
    if(ticks < TIMERx_MIN_TICKS) ticks = TIMERx_MIN_TICKS;

    printf("%s,frequency:%x,clk_src:%d,time_ms:%d,ticks=[%d],presc=[%d]\n",__FUNCTION__ ,frequency,clk_src,time_ms,ticks ,prescaler);
    NRFX_ASSERT(ticks <= UINT32_MAX);
    return (uint32_t)ticks;
}

#endif // SUPPRESS_INLINE_IMPLEMENTATION

/** @} */

#ifdef __cplusplus
}

#endif // XINC_TIMER_H__
