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

#include <nrfx.h>

#if NRFX_CHECK(NRFX_RTC_ENABLED)

#if !(NRFX_CHECK(NRFX_RTC0_ENABLED))
#error "No enabled RTC instances. Check <nrfx_config.h>."
#endif

#include <xincx_rtc.h>
#include "bsp_clk.h"

#define NRFX_LOG_MODULE RTC
#include <nrfx_log.h>

/**@brief RTC driver instance control block structure. */
typedef struct
{
    nrfx_drv_state_t state;        /**< Instance state. */
    bool             reliable;     /**< Reliable mode flag. */
    uint8_t          tick_latency; /**< Maximum length of interrupt handler in ticks (max 7.7 ms). */
} xincx_rtc_cb_t;

// User callbacks local storage.
static xincx_rtc_handler_t m_handlers[NRFX_RTC_ENABLED_COUNT];
static xincx_rtc_handler_t m_AoTimehandlers[NRFX_RTC_ENABLED_COUNT];
static xincx_rtc_cb_t      m_cb[NRFX_RTC_ENABLED_COUNT];
static volatile uint8_t 						calibration_flag;

nrfx_err_t xincx_rtc_init(xincx_rtc_t const * const  p_instance,
                         xincx_rtc_config_t const * p_config,
                         xincx_rtc_handler_t        handler)
{

   
	
    NRFX_ASSERT(p_config);
    NRFX_ASSERT(handler);
    nrfx_err_t err_code = NRFX_SUCCESS;
		uint32_t reg;
	  float freq = 0.0;
		if(p_config->type == NRF_RTC_TYPE_AOTIME)
		{
						 
			  m_AoTimehandlers[p_instance->instance_id] = handler;
				xc_rtc_clk_init();
				NRFX_IRQ_ENABLE(RTC_IRQn);
			
				return err_code;
	
		} 

    if (m_cb[p_instance->instance_id].state != NRFX_DRV_STATE_UNINITIALIZED)
    {
        err_code = NRFX_ERROR_INVALID_STATE;
				printf("Function: %s, error code",__func__);
			
				if(m_AoTimehandlers[p_instance->instance_id] != NULL)
				{
					err_code = NRFX_SUCCESS;
				}

        NRFX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         NRFX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
	
		xc_rtc_clk_init();
		p_instance->p_reg->ICR = 0x00;
		m_handlers[p_instance->instance_id] = handler;
		calibration_flag = 0;
		p_instance->p_reg->AO_TIMER_CTL = 0;
		p_instance->p_reg->AO_TIMER_CTL = 32;
		p_instance->p_reg->AO_TIMER_CTL |= (0x01 << 18);
		reg = p_instance->p_reg->ALL_INTR_AO;
		
		//  NRFX_IRQ_PRIORITY_SET(p_instance->irq, p_config->interrupt_priority);
		NRFX_IRQ_ENABLE(RTC_IRQn);
		while(!calibration_flag)
		{
			__nop();	
		}
		reg = p_instance->p_reg->FREQ_32K_TIMER_VAL;
		p_instance->p_reg->AO_TIMER_CTL = (0X01 << 16);
		printf("FREQ_32K_TIMER_VALreg: %d\r\n",reg);
		
		freq = (float)(15625.0 / (float)reg) * 32768.0;
		
		printf("freq: %f\r\n",freq);
 

		uint8_t data_limit[3];

//	
//	
		data_limit[0] = p_config->hour_limit;
		data_limit[1] = p_config->min_limit;
		data_limit[2] = p_config->sec_limit;
	//	
		if(p_config->type != 0)
		{
			nrf_rtc_date_set(p_instance->p_reg, p_config->date);

			nrf_rtc_datelimit_set(p_instance->p_reg, data_limit);

			nrf_rtc_freq_set(p_instance->p_reg, (uint32_t)freq);
	}

		

    m_cb[p_instance->instance_id].reliable     = p_config->reliable;
    m_cb[p_instance->instance_id].state        = NRFX_DRV_STATE_INITIALIZED;

    err_code = NRFX_SUCCESS;
    NRFX_LOG_INFO("Function: %s, error code: %s.", __func__, NRFX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}


void xincx_rtc_uninit(xincx_rtc_t const * const p_instance)
{
	m_cb[p_instance->instance_id].state        = NRFX_DRV_STATE_UNINITIALIZED;
}


void xincx_rtc_enable(xincx_rtc_t const * const p_instance)
{
	NRFX_ASSERT(m_cb[p_instance->instance_id].state == NRFX_DRV_STATE_INITIALIZED);

	p_instance->p_reg->ICR |= 0x100;
}

void xincx_rtc_disable(xincx_rtc_t const * const p_instance)
{
	p_instance->p_reg->ICR &= ~0x100;
}


void xincx_rtc_sec_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq)
{
   
    uint32_t mask = NRF_RTC_INT_SEC_MASK;
    if (enable_irq)
    {
        nrf_rtc_int_enable(p_instance->p_reg, mask);
    }
}

void xincx_rtc_sec_int_disable(xincx_rtc_t const * const p_instance)
{
    uint32_t mask = NRF_RTC_INT_SEC_MASK;
    nrf_rtc_int_disable(p_instance->p_reg, mask);
}

void xincx_rtc_min_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq)
{
   
    uint32_t mask = NRF_RTC_INT_MIN_MASK;
    if (enable_irq)
    {
        nrf_rtc_int_enable(p_instance->p_reg, mask);
    }
}

void xincx_rtc_min_int_disable(xincx_rtc_t const * const p_instance)
{
    uint32_t mask = NRF_RTC_INT_MIN_MASK;
    nrf_rtc_int_disable(p_instance->p_reg, mask);
}

void xincx_rtc_hour_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq)
{
   
    uint32_t mask = NRF_RTC_INT_HOUR_MASK;
    if (enable_irq)
    {
        nrf_rtc_int_enable(p_instance->p_reg, mask);
    }
}

void xincx_rtc_hour_int_disable(xincx_rtc_t const * const p_instance)
{
    uint32_t mask = NRF_RTC_INT_HOUR_MASK;
    nrf_rtc_int_disable(p_instance->p_reg, mask);
}

void xincx_rtc_day_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq)
{
   
    uint32_t mask = NRF_RTC_INT_DAY_MASK;
    if (enable_irq)
    {
        nrf_rtc_int_enable(p_instance->p_reg, mask);
    }
}

void xincx_rtc_day_int_disable(xincx_rtc_t const * const p_instance)
{
    uint32_t mask = NRF_RTC_INT_DAY_MASK;
    nrf_rtc_int_disable(p_instance->p_reg, mask);
}

void xincx_rtc_time1_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq)
{
   
    uint32_t mask = NRF_RTC_INT_TIME1_MASK;
    if (enable_irq)
    {
        nrf_rtc_int_enable(p_instance->p_reg, mask);
    }
}

void xincx_rtc_time1_int_disable(xincx_rtc_t const * const p_instance)
{
    uint32_t mask = NRF_RTC_INT_TIME1_MASK;
    nrf_rtc_int_disable(p_instance->p_reg, mask);
}

void xincx_rtc_time2_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq)
{
   
    uint32_t mask = NRF_RTC_INT_TIME2_MASK;
    if (enable_irq)
    {
        nrf_rtc_int_enable(p_instance->p_reg, mask);
    }
}

void xincx_rtc_time2_int_disable(xincx_rtc_t const * const p_instance)
{
    uint32_t mask = NRF_RTC_INT_TIME2_MASK;
    nrf_rtc_int_disable(p_instance->p_reg, mask);
}

void xincx_rtc_time3_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq)
{
   
    uint32_t mask = NRF_RTC_INT_TIME3_MASK;
    if (enable_irq)
    {
        nrf_rtc_int_enable(p_instance->p_reg, mask);
    }
}

void xincx_rtc_time3_int_disable(xincx_rtc_t const * const p_instance)
{
    uint32_t mask = NRF_RTC_INT_TIME3_MASK;
    nrf_rtc_int_disable(p_instance->p_reg, mask);
}

void xincx_rtc_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq)
{
   
    uint32_t mask = NRF_RTC_INT_ALL_MASK;
    if (enable_irq)
    {
        nrf_rtc_int_disable(p_instance->p_reg, mask);
    }
}

void xincx_rtc_int_disable(xincx_rtc_t const * const p_instance)
{
    uint32_t mask = NRF_RTC_INT_ALL_MASK;
    nrf_rtc_int_enable(p_instance->p_reg, mask);
}

nrfx_err_t xincx_rtc_time_set(xincx_rtc_t const * const p_instance,
                           xincx_rtc_match_timer_ch_t                 channel,
                           xincx_rtc_match_config_t  config ,
                           bool                     enable_irq)
{
		nrfx_err_t err_code = NRFX_SUCCESS;
		__IOM uint32_t *reg = NULL;
		uint8_t irq_idx;
		switch(channel)
		{
			case NRFX_RTC_MATCH_TIME_1:
			{
				reg = &(p_instance->p_reg->CMR_ONE);
				irq_idx = 5;
			}break;
			
			case NRFX_RTC_MATCH_TIME_2:
			{
				reg = &(p_instance->p_reg->CMR_TWO);
				irq_idx = 4;
			}break;
			
			case NRFX_RTC_MATCH_TIME_3:
			{
				reg = &(p_instance->p_reg->CMR_THREE);
				irq_idx = 6;
			}break;
			
			default:
					err_code = NRFX_ERROR_INVALID_PARAM;
			break;
		}
		
		if(reg != NULL)
		{
			*reg = config.value;
			printf("config.value:%x,ONE:%x\r\n",config.value,p_instance->p_reg->CMR_ONE);
			if(enable_irq)
			{
				nrf_rtc_int_enable(p_instance->p_reg, 0x01 << irq_idx);
			}
			 printf("ICR:0x%x\r\n",p_instance->p_reg->ICR);
		}
		
	return err_code ;
}

nrfx_err_t xincx_rtc_time_disable(xincx_rtc_t const * const p_instance, xincx_rtc_match_timer_ch_t channel)
{
	uint8_t irq_idx;
	nrfx_err_t err_code = NRFX_SUCCESS;
	switch(channel)
		{
			case NRFX_RTC_MATCH_TIME_1:
			{
				irq_idx = 5;
			}break;
			
			case NRFX_RTC_MATCH_TIME_2:
			{
		
				irq_idx = 4;
			}break;
			
			case NRFX_RTC_MATCH_TIME_3:
			{
				irq_idx = 6;
			}break;
			
			default:
				err_code = NRFX_ERROR_INVALID_PARAM;
			break;
		}
		nrf_rtc_int_disable(p_instance->p_reg, 0x01 << irq_idx);
		
		return err_code ;
}

void xincx_rtc_AOtime_set(xincx_rtc_t const * const p_instance,                         
                           uint32_t  tick)
{

		p_instance->p_reg->AO_TIMER_CTL = 0;
		p_instance->p_reg->AO_TIMER_CTL = tick;
		p_instance->p_reg->AO_TIMER_CTL |= (0x01 << 17);
}

void xincx_rtc_date_set(xincx_rtc_t const * const p_instance,                         
                           nrf_rtc_time_t  date)
{
		nrf_rtc_date_set(p_instance->p_reg,date);

}

void xincx_rtc_date_get(xincx_rtc_t const * const p_instance,                         
                           nrf_rtc_time_t  *date)
{
		nrf_rtc_date_get(p_instance->p_reg,date);

}

static void irq_handler(NRF_RTC_Type * p_reg,
                        uint32_t       instance_id)
{
	uint32_t reg ;
	reg = p_reg->ALL_INTR_AO;
//	printf("RTC irq_handler:%x\r\n",reg);
	if(reg)
	{
		reg = p_reg->AO_TIMER_CTL;
		reg |= 0x01 << 16;
		p_reg->AO_TIMER_CTL = reg;
		calibration_flag = 1;
//		if(m_cb[instance_id].state == NRFX_DRV_STATE_UNINITIALIZED)
//		{
//			NRFX_LOG_INFO("RTC 32K  calibration ok.\r\n");
//			calibration_flag = 1;
//			printf("RTC 32K calibration ok.\r\n");
//		}
		//else
		{
			if(m_AoTimehandlers[instance_id])
			{
				m_AoTimehandlers[instance_id](NRFX_RTC_INT_AOTIME);
			}
			
		}
	}
	
	reg = p_reg->ISR_EOI;
	if(reg)
	{
		p_reg->ISR_EOI = reg;
		uint32_t rtc_val[5];
		if(m_cb[instance_id].state == NRFX_DRV_STATE_INITIALIZED)
		{
			if((reg & NRF_RTC_INT_SEC_MASK) == NRF_RTC_INT_SEC_MASK)
			{
			//	printf("sec add\n");	
				m_handlers[instance_id](NRFX_RTC_INT_SEC);
			}
			
			if((reg & NRF_RTC_INT_TIME1_MASK) == NRF_RTC_INT_TIME1_MASK)
			{
			//	printf("sec add\n");	
				m_handlers[instance_id](NRFX_RTC_INT_TIME1);
			}
		}
	}
	
			
}

#if NRFX_CHECK(NRFX_RTC0_ENABLED)
void xincx_rtc_0_irq_handler(void)
{
    irq_handler(NRF_RTC0, NRFX_RTC0_INST_IDX);
}
#endif

#if NRFX_CHECK(NRFX_RTC_ENABLED)
void RTC_Handler(void)
{
			
		xincx_rtc_0_irq_handler();
}

#endif



#endif // NRFX_CHECK(NRFX_RTC_ENABLED)
