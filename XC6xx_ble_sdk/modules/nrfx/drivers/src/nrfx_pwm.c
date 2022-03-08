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

#include <nrfx.h>

#if NRFX_CHECK(NRFX_PWM_ENABLED)

#if !(NRFX_CHECK(NRFX_PWM0_ENABLED) || NRFX_CHECK(NRFX_PWM1_ENABLED) || \
      NRFX_CHECK(NRFX_PWM2_ENABLED) || NRFX_CHECK(NRFX_PWM3_ENABLED) || \
			NRFX_CHECK(NRFX_PWM4_ENABLED) || NRFX_CHECK(NRFX_PWM5_ENABLED))
#error "No enabled PWM instances. Check <nrfx_config.h>."
#endif

#include <nrfx_pwm.h>
#include <hal/nrf_gpio.h>

#define NRFX_LOG_MODULE PWM
#include <nrfx_log.h>


// Control block - driver instance local data.
typedef struct
{
    nrfx_drv_state_t volatile state;
    uint8_t                   flags;
		xinc_pwm_clk_src_t clk_src;          ///< Bit width.
		nrf_pwm_ref_clk_t  ref_clk;   ///< Base clock frequency.
		uint8_t period; //set pwm out freq
		uint8_t ocpy;// duty cycle
} pwm_control_block_t;
static pwm_control_block_t m_cb[NRFX_PWM_ENABLED_COUNT];

static uint16_t nrfx_pwm_freq_to_period(uint8_t inst_idx,uint32_t freq);

static void configure_pins(nrfx_pwm_t const * const p_instance,
                           nrfx_pwm_config_t const * p_config)
{
    uint32_t out_pins;

	if(p_instance->id == NRF_PWM_ID_2 || p_instance->id == NRF_PWM_ID_3)
	{
		gpio_mux_ctl(p_config->output_pins,2);
	}
	else if(p_instance->id == NRF_PWM_ID_4 || p_instance->id == NRF_PWM_ID_5)
	{
		gpio_mux_ctl(p_config->output_pins,3);
	}
	else if(p_instance->id == NRF_PWM_ID_0)
	{
		gpio_mux_ctl(p_config->output_pins,0); 
		gpio_fun_sel(p_config->output_pins,12); 
		gpio_fun_inter(p_config->output_pins,0);
		printf("configure_pins p_config->output_pins:%d,p_instance->id :%d\r\n",p_config->output_pins,p_instance->id);
	}
	else if(p_instance->id == NRF_PWM_ID_1)
	{
		gpio_mux_ctl(p_config->output_pins,0); 
		gpio_fun_sel(p_config->output_pins,13); 
		gpio_fun_inter(p_config->output_pins,0);
	}

}


nrfx_err_t nrfx_pwm_init(nrfx_pwm_t const * const p_instance,
                         nrfx_pwm_config_t const * p_config,
                         nrfx_pwm_handler_t        handler)
{
	#include    "bsp_register_macro.h"
	__write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL,0x10001000); //PWM_PCLK Ê±ÖÓÊ¹ÄÜ
//	__write_hw_reg32(CPR_PWM_CLK_CTL,0x80000009); //pwm_clk=mclk_in(32M)/(2*(9+1))=1.6M
    NRFX_ASSERT(p_config);

    nrfx_err_t err_code = NRFX_SUCCESS;

    pwm_control_block_t * p_cb  = &m_cb[p_instance->drv_inst_idx];

    if (p_cb->state != NRFX_DRV_STATE_UNINITIALIZED)
    {
        err_code = NRFX_ERROR_INVALID_STATE;
        NRFX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         NRFX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
		
		printf("p_instance->id:%d,p_instance->drv_inst_idx:%d,NRFX_PWM_ENABLED_COUNT:%d \r\n",p_instance->id,p_instance->drv_inst_idx,NRFX_PWM_ENABLED_COUNT);

		for(uint8_t i = 0;i < NRFX_PWM_ENABLED_COUNT;i ++)
		{
			pwm_control_block_t * p_cb1  = &m_cb[i];
			if (p_cb1->state == NRFX_DRV_STATE_INITIALIZED)
			{			
					
						if(p_config->clk_src != p_cb1->clk_src)
						{
							err_code = NRFX_ERROR_INVALID_PARAM;
						}
						if(p_config->clk_src != XINC_PWM_CLK_SRC_32K)
						{
							if(p_config->ref_clk != p_cb1->ref_clk)
							{
								err_code = NRFX_ERROR_INVALID_PARAM;
							}	
						}
						
						if(err_code != NRFX_SUCCESS)
						{
							NRFX_LOG_WARNING("Function: %s, error code: %s.",
													 __func__,
													 NRFX_LOG_ERROR_STRING_GET(err_code));
							printf("Function: %s, error code: %s.\r\n",
													 __func__,
													 NRFX_LOG_ERROR_STRING_GET(err_code));
							printf("err i:%d,clk_src:%d,ref_clk:%d,clk_src:%d,ref_clk:%d \r\n",i,p_cb1->clk_src,p_cb1->ref_clk,p_config->clk_src,p_config->ref_clk);
					
							return err_code;
						}	
							
			}
		}
		
		if(p_config->ref_clk > XINC_PWM_REF_CLK_32000Hz)
		{
				err_code = NRFX_ERROR_INVALID_PARAM;
				NRFX_LOG_WARNING("Function: %s, error code: %s.",
												 __func__,
												 NRFX_LOG_ERROR_STRING_GET(err_code));
				
				return err_code;
		}
	

	printf("reg addr=%p\r\n", p_instance->p_reg);

	nrf_pwm_disable(p_instance->p_reg);
	
	configure_pins(p_instance, p_config);
	   
	p_cb->clk_src = p_config->clk_src;
	p_cb->ref_clk = p_config->ref_clk;
	
	printf("p_cb->clk_src=%d,p_cb->ref_clk:%d\r\n", p_cb->clk_src,p_cb->ref_clk);
	xinc_pwm_clk_div_set(p_instance->p_cpr,p_instance->id,p_config->clk_src,p_config->ref_clk);

	uint16_t period;
	period = nrfx_pwm_freq_to_period(p_instance->drv_inst_idx,p_config->frequency);
	if(period == 0xFFFF)
	{	
		nrfx_pwm_freq_valid_range_check(p_cb->clk_src,p_cb->ref_clk,p_config->frequency);
		err_code = NRFX_ERROR_INVALID_PARAM;
		
		NRFX_LOG_WARNING("Function: %s, error code: %s.",
											 __func__,
											 NRFX_LOG_ERROR_STRING_GET(err_code));
		
		printf("Function: %s,period:%x error code: %s.\r\n",
											 __func__,period,
											 NRFX_LOG_ERROR_STRING_GET(err_code));
		return err_code;
	}
	nrf_pwm_configure(p_instance->p_reg,period, p_config->duty_cycle);
	p_cb->period = period;
	p_cb->ocpy =  p_config->duty_cycle;
	
	if(p_config->start)
	{
		nrf_pwm_enable(p_instance->p_reg);
	}
	
	p_cb->state = NRFX_DRV_STATE_INITIALIZED;

	err_code = NRFX_SUCCESS;
	NRFX_LOG_INFO("Function: %s, error code: %s.", __func__, NRFX_LOG_ERROR_STRING_GET(err_code));
   return err_code;
}


void nrfx_pwm_uninit(nrfx_pwm_t const * const p_instance)
{
    pwm_control_block_t * p_cb  = &m_cb[p_instance->drv_inst_idx];
    NRFX_ASSERT(p_cb->state != NRFX_DRV_STATE_UNINITIALIZED);



    nrf_pwm_disable(p_instance->p_reg);

    p_cb->state = NRFX_DRV_STATE_UNINITIALIZED;
}


bool nrfx_pwm_start(nrfx_pwm_t const * const p_instance)
{
    NRFX_ASSERT(m_cb[p_instance->drv_inst_idx].state != NRFX_DRV_STATE_UNINITIALIZED);
		
		if(m_cb[p_instance->drv_inst_idx].state != NRFX_DRV_STATE_INITIALIZED)
		{
			return false;
		}
    nrf_pwm_enable(p_instance->p_reg);
		
		return true;
}

bool nrfx_pwm_stop(nrfx_pwm_t const * const p_instance)
{
    NRFX_ASSERT(m_cb[p_instance->drv_inst_idx].state != NRFX_DRV_STATE_UNINITIALIZED);
		
		if(m_cb[p_instance->drv_inst_idx].state != NRFX_DRV_STATE_INITIALIZED)
		{
			return false;
		}
    nrf_pwm_disable(p_instance->p_reg);
		
		return true;
}




nrfx_err_t nrfx_pwm_freq_update(nrfx_pwm_t const * const p_instance,uint32_t new_freq)
{
	nrfx_err_t err_code = NRFX_SUCCESS;
	uint16_t period;
	pwm_control_block_t * p_cb  = &m_cb[p_instance->drv_inst_idx];
	period = nrfx_pwm_freq_to_period(p_instance->drv_inst_idx,new_freq);
	if(period == 0xFFFF)
	{
		err_code = NRFX_ERROR_INVALID_PARAM;
		NRFX_LOG_WARNING("Function: %s, error code: %s.",
											 __func__,
											 NRFX_LOG_ERROR_STRING_GET(err_code));
		
		printf("Function: %s,period:%x error code: %s.\r\n",
											 __func__,period,
											 NRFX_LOG_ERROR_STRING_GET(err_code));
		
		return err_code;
	}
	p_cb->period = period;
	nrf_pwm_configure(p_instance->p_reg,period,p_cb->ocpy);

	return err_code;
}


nrfx_err_t nrfx_pwm_duty_cycle_update(nrfx_pwm_t const * const p_instance,uint8_t new_duty)
{
	nrfx_err_t err_code = NRFX_SUCCESS;
	NRFX_ASSERT(new_duty <= 100);
	pwm_control_block_t * p_cb  = &m_cb[p_instance->drv_inst_idx];

	if(new_duty > 100)
	{
		err_code = NRFX_ERROR_INVALID_PARAM;
		NRFX_LOG_WARNING("Function: %s, error code: %s.",
											 __func__,
											 NRFX_LOG_ERROR_STRING_GET(err_code));
		
		printf("Function: %s,new_duty:%x error code: %s.\r\n",
											 __func__,new_duty,
											 NRFX_LOG_ERROR_STRING_GET(err_code));
		
		return err_code;
	}
	p_cb->ocpy = new_duty;
	nrf_pwm_configure(p_instance->p_reg,p_cb->period,new_duty);
	
	return err_code;

}
nrfx_err_t nrfx_pwm_freq_duty_cycl_update(nrfx_pwm_t const * const p_instance,uint32_t new_freq,uint8_t new_duty)
{
	nrfx_err_t err_code = NRFX_SUCCESS;
	NRFX_ASSERT(new_duty <= 100);
	uint16_t period;
	pwm_control_block_t * p_cb  = &m_cb[p_instance->drv_inst_idx];
	period = nrfx_pwm_freq_to_period(p_instance->drv_inst_idx,new_freq);
	if((period == 0xFFFF) || (new_duty > 100))
	{
		err_code = NRFX_ERROR_INVALID_PARAM;
		NRFX_LOG_WARNING("Function: %s, error code: %s.",
											 __func__,
											 NRFX_LOG_ERROR_STRING_GET(err_code));
		
		printf("Function: %s,period:%x error code: %s.\r\n",
											 __func__,period,
											 NRFX_LOG_ERROR_STRING_GET(err_code));
		
		return err_code;
	}
	p_cb->period = period;
	p_cb->ocpy = new_duty;
	nrf_pwm_configure(p_instance->p_reg,period,new_duty);
	
	return err_code;
}



static uint16_t nrfx_pwm_freq_to_period(uint8_t inst_idx,uint32_t freq)
{
	pwm_control_block_t * p_cb  = &m_cb[inst_idx];
	uint32_t pwm_clk;
	uint16_t period = 0;
	
	//	freq = pwm_clk / ((period + 1) * 100);
	switch(p_cb->clk_src)
	{
		case XINC_PWM_CLK_SRC_32M_DIV:
		{
			pwm_clk = 16000000 / (1 << p_cb->ref_clk);
		
		}break;
		
		case XINC_PWM_CLK_SRC_32K_DIV:
		{
			pwm_clk = 32000 / (1 << p_cb->ref_clk);
		
		}break;
		
		case XINC_PWM_CLK_SRC_32K:
		{
			pwm_clk = 32000;
		}break;
		default:
			break;
	
	}
	period  = ((pwm_clk /  100 ) / freq ) - 1 ;
//	period*=2;
	printf("%s,pwm_clk:%d,period:%x,%d\r\n",__func__,pwm_clk,period,period);
	
	if(period > 0xFF)
	{
		period = 0xFFFF;
	}

	return period;
}

bool nrfx_pwm_freq_valid_range_check(uint8_t clk_src,uint8_t ref_clk, uint32_t set_freq)
{
	uint32_t pwm_clk;
	uint32_t valid_freq_min = 0;
	uint32_t valid_freq_max = 0;
	//	freq = pwm_clk / ((period + 1) * 100);
	switch(clk_src)
	{
		case XINC_PWM_CLK_SRC_32M_DIV:
		{
			pwm_clk = 16000000 / (1 << ref_clk);
		
		}break;
		
		case XINC_PWM_CLK_SRC_32K_DIV:
		{
			pwm_clk = 32000 / (1 << ref_clk);
		
		}break;
		
		case XINC_PWM_CLK_SRC_32K:
		{
			pwm_clk = 32000;
		}break;
		default:
			break;
	
	}
	valid_freq_min  = pwm_clk / ((0xFF + 1) * 100);

	if(((pwm_clk/100) % 256) != 0)
	{
		valid_freq_min++;
	}

	valid_freq_max  = pwm_clk / ((0x0 + 1) * 100);
	NRFX_LOG_INFO("Function: %s, valid_freq_min : %d. valid_freq_max : %d\r\n",
											 __func__,
											 valid_freq_min,valid_freq_max);
	printf("freq_valid_range min:%d,max:%d\r\n",(uint32_t)valid_freq_min,(uint32_t)valid_freq_max);
	if((set_freq >= valid_freq_min) && (set_freq <= valid_freq_max))
	{
		return true;
	}
	return false;
}

#endif // NRFX_CHECK(NRFX_PWM_ENABLED)
