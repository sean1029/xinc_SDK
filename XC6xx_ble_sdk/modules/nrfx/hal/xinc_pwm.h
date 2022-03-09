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

#ifndef XINC_PWM_H__
#define XINC_PWM_H__

#include <nrfx.h>

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
} xinc_pwm_ref_clk_t;


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
    p_reg->EN |= (PWM_ENABLE_ENABLE_Enabled << PWM_ENABLE_ENABLE_Pos);	
}

__STATIC_INLINE void xinc_pwm_disable(XINC_PWM_Type * p_reg)
{	
    p_reg->EN &= ~(PWM_ENABLE_ENABLE_Enabled << PWM_ENABLE_ENABLE_Pos);
}



__STATIC_INLINE void xinc_pwm_configure(XINC_PWM_Type * p_reg,
                                       uint8_t       period,
																			 uint8_t  duty_cycle)
{
    //NRFX_ASSERT(top_value <= PWM_COUNTERTOP_COUNTERTOP_Msk);

	p_reg->OCPY = duty_cycle;
  p_reg->PERIOD = period;
	p_reg->UP = 0x1;
}



__STATIC_INLINE void xinc_pwm_clk_div_set(XINC_CPR_CTL_Type * p_reg,uint8_t id,xinc_pwm_clk_src_t clk_src,
                                             xinc_pwm_ref_clk_t ref_clk)
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
	p_reg->PWM_CLK_CTL |= ((unsigned int)(1 << 31UL));

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

