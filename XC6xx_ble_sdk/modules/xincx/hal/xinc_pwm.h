/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINC_PWM_H__
#define XINC_PWM_H__

#include <xincx.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xinc_pwm_hal PWM HAL
 * @{
 * @ingroup xinc_pwm
 * @brief   Hardware access layer for managing the Pulse Width Modulation (PWM) peripheral.
 */


/** @brief Number of channels in each PWM instance. */
#define XINC_PWM_CHANNEL_COUNT  6





/** @brief PWM interrupts. */
enum
{
    XINC_PWM_ID_0 = 0,      ///< XINC_PWM_ID_0
    XINC_PWM_ID_1  = 1,
    XINC_PWM_ID_2 = 2,
    XINC_PWM_ID_3 = 3 ,
    XINC_PWM_ID_4 = 4,     
    XINC_PWM_ID_5 = 5, 
   
} ;




/** @brief Timer bit width. */
typedef enum
{
    XINC_PWM_CLK_SRC_32M_DIV = CPR_PWM_CLK_CTL_PWM_CLKSEL_32M_DIV, ///< PWM CLK SRC 32MHz div.
    XINC_PWM_CLK_SRC_32K_DIV = CPR_PWM_CLK_CTL_PWM_CLKSEL_32K_DIV, ///< PWM CLK SRC 32kHz div.
    XINC_PWM_CLK_SRC_32K = CPR_PWM_CLK_CTL_PWM_CLKSEL_32K, ///< PWM CLK SRC 32kHz.
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
} xinc_pwm_ref_clk_t;

enum
{
    XINC_PWMCOMPTIME_VAL_1clk = PWM_COMP_TIME_PWMCOMPTIME_VAL_1clk, 		///< 死区为 1 个 pwm_clk 时钟周期
    XINC_PWMCOMPTIME_VAL_2clk = PWM_COMP_TIME_PWMCOMPTIME_VAL_2clk,      ///< 死区为 2 个 pwm_clk 时钟周期
    XINC_PWMCOMPTIME_VAL_3clk = PWM_COMP_TIME_PWMCOMPTIME_VAL_3clk,      ///< 死区为 3 个 pwm_clk 时钟周期
    XINC_PWMCOMPTIME_VAL_4clk = PWM_COMP_TIME_PWMCOMPTIME_VAL_4clk,      ///< 死区为 4 个 pwm_clk 时钟周期
    XINC_PWMCOMPTIME_VAL_5clk = PWM_COMP_TIME_PWMCOMPTIME_VAL_5clk,      ///< 死区为 5 个 pwm_clk 时钟周期
    XINC_PWMCOMPTIME_VAL_6clk = PWM_COMP_TIME_PWMCOMPTIME_VAL_6clk,   ///< 死区为 6 个 pwm_clk 时钟周期
    XINC_PWMCOMPTIME_VAL_7clk = PWM_COMP_TIME_PWMCOMPTIME_VAL_7clk,    ///< 死区为 7 个 pwm_clk 时钟周期
    XINC_PWMCOMPTIME_VAL_8clk = PWM_COMP_TIME_PWMCOMPTIME_VAL_8clk,   ///< 死区为 8 个 pwm_clk 时钟周期
};


/**
 * @brief Function for enabling the PWM peripheral.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void xinc_pwm_enable(XINC_PWM_Type * p_reg);

/**
 * @brief Function for disabling the PWM peripheral.
 *
 * @param[in] p_reg Pointer to the structure of registers of the peripheral.
 */
__STATIC_INLINE void xinc_pwm_disable(XINC_PWM_Type * p_reg);



/**
 * @brief Function for configuring the PWM peripheral.
 *
 * @param[in] p_reg      Pointer to the structure of registers of the peripheral.
 * @param[in] base_clock Base clock frequency.
 * @param[in] mode       Operating mode of the pulse generator counter.
 * @param[in] top_value  Value up to which the pulse generator counter counts.
 */
__STATIC_INLINE void xinc_pwm_configure(XINC_PWM_Type * p_reg,
                                       uint8_t       period,
									   uint8_t        duty_cycle);





__STATIC_INLINE void xinc_pwm_clk_div_set(XINC_CPR_CTL_Type * p_reg,uint8_t id,xinc_pwm_clk_src_t clk_src,
                                             xinc_pwm_ref_clk_t ref_clk);

__STATIC_INLINE uint32_t xinc_pwm_clk_div_get(XINC_CPR_CTL_Type * p_reg,uint8_t id);


#ifndef SUPPRESS_INLINE_IMPLEMENTATION


__STATIC_INLINE void xinc_pwm_enable(XINC_PWM_Type * p_reg)
{
    p_reg->EN = (PWM_EN_Enable << PWM_EN_Pos) & PWM_EN_Msk;	
    printf("xinc_pwm_enable EN addr:0x%p,value:0x%x\r\n",&p_reg->EN,p_reg->EN);
}

__STATIC_INLINE void xinc_pwm_disable(XINC_PWM_Type * p_reg)
{	
    p_reg->EN = (PWM_EN_Disable << PWM_EN_Pos)& PWM_EN_Msk;
}



__STATIC_INLINE void xinc_pwm_configure(XINC_PWM_Type * p_reg,
                                       uint8_t       period,
																			 uint8_t  duty_cycle)
{
 //   printf("xinc_pwm_configure period:%d,duty_cycle:%d\r\n",period,duty_cycle);
    //XINCX_ASSERT(top_value <= PWM_COUNTERTOP_COUNTERTOP_Msk);
    p_reg->PERIOD = (period << PWM_P_PERIOD_Pos) & PWM_P_PERIOD_Msk;
    p_reg->OCPY = (duty_cycle << PWM_OCPY_OCPY_RATIO_Pos) & PWM_OCPY_OCPY_RATIO_Msk ;
   
    p_reg->UP = (PWM_UP_UPDATE_Enable << PWM_UP_UPDATE_Pos) & PWM_UP_UPDATE_Msk;
    
  //  printf("xinc_pwm_configure 0x%p,0x%p,0x%p, UP:%d,period:%d,duty_cycle:%d\r\n",&p_reg->UP,&p_reg->PERIOD,&p_reg->OCPY,p_reg->UP,p_reg->PERIOD,p_reg->OCPY);
}



__STATIC_INLINE void xinc_pwm_clk_div_set(XINC_CPR_CTL_Type * p_reg,uint8_t id,xinc_pwm_clk_src_t clk_src,
                                             xinc_pwm_ref_clk_t ref_clk)
{
	switch(clk_src)
	{
		case XINC_PWM_CLK_SRC_32M_DIV:
		{
			p_reg->PWM_CLK_CTL = ((1UL << ref_clk) - 1UL) | (XINC_PWM_CLK_SRC_32M_DIV << CPR_PWM_CLK_CTL_PWM_CLKSEL_Pos) ;
		}break;
		
		case XINC_PWM_CLK_SRC_32K_DIV:
		{
			p_reg->PWM_CLK_CTL = (((1UL << ref_clk) - 1UL) << CPR_PWM_CLK_CTL_PWM_CLK1_DIV_Pos) | (XINC_PWM_CLK_SRC_32K_DIV << CPR_PWM_CLK_CTL_PWM_CLKSEL_Pos) ;
		}break;
		
		case XINC_PWM_CLK_SRC_32K:
		{
			p_reg->PWM_CLK_CTL = (XINC_PWM_CLK_SRC_32K << CPR_PWM_CLK_CTL_PWM_CLKSEL_Pos);
		}break;
		
		default:
			break;
	
	}
	p_reg->PWM_CLK_CTL |= ( CPR_PWM_CLK_CTL_PWM_CLK_EN_Enable << CPR_PWM_CLK_CTL_PWM_CLK_EN_Pos);

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

#endif // XINC_PWM_H__

