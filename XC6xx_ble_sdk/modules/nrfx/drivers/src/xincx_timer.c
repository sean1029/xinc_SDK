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

#if NRFX_CHECK(XINCX_TIMER_ENABLED)

#if !(NRFX_CHECK(XINCX_TIMER0_ENABLED) || NRFX_CHECK(XINCX_TIMER1_ENABLED) || \
      NRFX_CHECK(XINCX_TIMER2_ENABLED) || NRFX_CHECK(XINCX_TIMER3_ENABLED))
#error "No enabled TIMER instances. Check <nrfx_config.h>."
#endif

#include <xincx_timer.h>

#define NRFX_LOG_MODULE TIMER
#include <nrfx_log.h>

/**@brief Timer control block. */
typedef struct
{
    xincx_timer_event_handler_t handler;
    void *                     context;
    nrfx_drv_state_t           state;
} timer_control_block_t;

static timer_control_block_t m_cb[XINCX_TIMER_ENABLED_COUNT];


nrfx_err_t xincx_timer_init(xincx_timer_t const * const  p_instance,
                           xincx_timer_config_t const * p_config,
                           xincx_timer_event_handler_t  timer_event_handler)
{

	nrfx_err_t err_code;

	timer_control_block_t * p_cb = &m_cb[p_instance->instance_idx];
	
#ifdef SOFTDEVICE_PRESENT
    NRFX_ASSERT(p_instance->p_reg != XINC_TIMER0);
#endif
    NRFX_ASSERT(p_config);
    NRFX_ASSERT(timer_event_handler);

    
	if (p_cb->state != NRFX_DRV_STATE_UNINITIALIZED)
	{
			err_code = NRFX_ERROR_INVALID_STATE;
			NRFX_LOG_WARNING("Function: %s, error code: %s.",
											 __func__,
											 NRFX_LOG_ERROR_STRING_GET(err_code));
			return err_code;
	}
	printf("p_config->frequency val:%d,p_config->clk_src:%d\r\n",p_config->frequency,p_config->clk_src);
	if(p_config->frequency > XINC_TIMER_FREQ_32000Hz)
	{
			err_code = NRFX_ERROR_INVALID_PARAM;
			NRFX_LOG_WARNING("Function: %s, error code: %s.",
											 __func__,
											 NRFX_LOG_ERROR_STRING_GET(err_code));
			
			return err_code;
	}
	
	if((p_config->frequency ==  XINC_TIMER_FREQ_32000Hz)&& (p_config->clk_src != XINC_TIMER_CLK_SRC_32K))
	{
		err_code = NRFX_ERROR_INVALID_PARAM;
		
		NRFX_LOG_WARNING("Function: %s, error code: %s.",
											 __func__,
											 NRFX_LOG_ERROR_STRING_GET(err_code));
		return err_code;
	}
	
	if((p_config->frequency < XINC_TIMER_FREQ_32000Hz)&& (p_config->clk_src == XINC_TIMER_CLK_SRC_32K))
	{
		err_code = NRFX_ERROR_INVALID_PARAM;
		
		NRFX_LOG_WARNING("Function: %s, error code: %s.",
											 __func__,
											 NRFX_LOG_ERROR_STRING_GET(err_code));
		return err_code;
	}
		
	p_instance->p_cpr->CTLAPBCLKEN_GRCTL = ((0x01 << 3) | (0x01 << 19));//TIMER_PCLK 时钟使能
 
	xinc_timer_frequency_div_set(p_instance->p_cpr,p_instance->id,p_config->clk_src,p_config->frequency); //TIMERx_CLK 时钟控制寄存器 mclk_in(32MHz)/2x(0x0f + 0x1)=1M
	printf("frequency val:0x%08x\r\n",p_config->frequency);
	xinc_timer_mode_set(p_instance->p_reg, p_config->mode);
	
	p_cb->handler = timer_event_handler;
  p_cb->context = p_config->p_context;

	// clear timer interrupt
  xinc_timer_int_clear(p_instance->p_reg,1);
	
	
	NRFX_IRQ_PRIORITY_SET(TIMER0_IRQn + p_instance->id,
        p_config->interrupt_priority);
  NRFX_IRQ_ENABLE(TIMER0_IRQn + p_instance->id);
	

	p_cb->state = NRFX_DRV_STATE_INITIALIZED;

	err_code = NRFX_SUCCESS;
	NRFX_LOG_INFO("Function: %s, error code: %s.",
								__func__,
								NRFX_LOG_ERROR_STRING_GET(err_code));
	return err_code;
	

}

void xincx_timer_uninit(xincx_timer_t const * const p_instance)
{
    NRFX_IRQ_DISABLE(nrfx_get_irq_number(p_instance->p_reg));

    #define DISABLE_ALL UINT32_MAX
    xinc_timer_int_disable(p_instance->p_reg, DISABLE_ALL);
    #undef DISABLE_ALL

    xincx_timer_disable(p_instance);

    m_cb[p_instance->instance_idx].state = NRFX_DRV_STATE_UNINITIALIZED;
    NRFX_LOG_INFO("Uninitialized instance: %d.", p_instance->instance_idx);
}

void xincx_timer_enable(xincx_timer_t const * const p_instance)
{
    NRFX_ASSERT(m_cb[p_instance->instance_idx].state == NRFX_DRV_STATE_INITIALIZED);
		p_instance->p_reg->TCR |= (0x01 << 0);
    m_cb[p_instance->instance_idx].state = NRFX_DRV_STATE_POWERED_ON;
    NRFX_LOG_INFO("Enabled instance: %d.", p_instance->instance_idx);
}

void xincx_timer_disable(xincx_timer_t const * const p_instance)
{
		NRFX_ASSERT(m_cb[p_instance->instance_idx].state != NRFX_DRV_STATE_UNINITIALIZED);

		p_instance->p_reg->TCR &= ~(0x01 << 0);
		printf("timer TCR=[%x]\n", p_instance->p_reg->TCR);
		
    m_cb[p_instance->instance_idx].state = NRFX_DRV_STATE_INITIALIZED;
    NRFX_LOG_INFO("Disabled instance: %d.", p_instance->instance_idx);
}

bool xincx_timer_is_enabled(xincx_timer_t const * const p_instance)
{
    NRFX_ASSERT(m_cb[p_instance->instance_idx].state != NRFX_DRV_STATE_UNINITIALIZED);
    return (m_cb[p_instance->instance_idx].state == NRFX_DRV_STATE_POWERED_ON);
}




void xincx_timer_compare(xincx_timer_t const * const p_instance,               
                        uint32_t                   cc_value,
                        bool                       enable_int)
{
    xinc_timer_int_mask_t timer_int = 0x01 << 2;

    if (enable_int)
    {
        xinc_timer_int_clear(p_instance->p_reg,1);
        xinc_timer_int_enable(p_instance->p_reg, timer_int);
    }
    else
    {
        xinc_timer_int_disable(p_instance->p_reg, timer_int);
    }

    xinc_timer_cc_write(p_instance->p_reg, cc_value);
    NRFX_LOG_INFO("Timer id: %d, capture value set: %lu.",
                  p_instance->instance_idx,
                  cc_value);
}


void xincx_timer_compare_int_enable(xincx_timer_t const * const p_instance,
                                   uint32_t                   channel)
{
    NRFX_ASSERT(m_cb[p_instance->instance_idx].state != NRFX_DRV_STATE_UNINITIALIZED);
    NRFX_ASSERT(channel < p_instance->cc_channel);

    xinc_timer_int_clear(p_instance->p_reg,XINC_TIMER_EVENT_TIMEOUT);
   
    xinc_timer_int_enable(p_instance->p_reg,0x01 << 2);
}

void xincx_timer_compare_int_disable(xincx_timer_t const * const p_instance,
                                    uint32_t                   channel)
{
    NRFX_ASSERT(m_cb[p_instance->instance_idx].state != NRFX_DRV_STATE_UNINITIALIZED);
    NRFX_ASSERT(channel < p_instance->cc_channel);

    xinc_timer_int_disable(p_instance->p_reg,0x01 << 2);
}

static void irq_handler(XINC_TIMER_Type        * p_reg,
                        timer_control_block_t * p_cb,
                        uint8_t                 channel)
{
    uint8_t i;
   
    {
        xinc_timer_int_event_t event = XINC_TIMER_EVENT_TIMEOUT;
        xinc_timer_int_mask_t int_mask = 0x01 << 2;
        if (xinc_timer_int_check(p_reg, event) &&
            xinc_timer_int_enable_check(p_reg, int_mask))
        {           
						if(p_cb->handler)
						{
							p_cb->handler(event,channel, p_cb->context);
						}
						 
        }
				xinc_timer_int_clear(p_reg, event);
    }
}

#if NRFX_CHECK(XINCX_TIMER0_ENABLED)
//void TIMER0_Handler(void)
//{
//	uint32_t val=0;
//	val = XINC_TIMER0->TIC;
//	printf("TIMER0_Handler\n");

//}
void xincx_timer_0_irq_handler(void)
{
    irq_handler(XINC_TIMER0, &m_cb[XINCX_TIMER0_INST_IDX],
        0);
}
#endif

#if NRFX_CHECK(XINCX_TIMER1_ENABLED)
//void TIMER1_Handler(void)
//{

//	xincx_timer_1_irq_handler();

//}
void xincx_timer_1_irq_handler(void)
{
    irq_handler(XINC_TIMER1, &m_cb[XINCX_TIMER1_INST_IDX],
        1);
}
#endif

#if NRFX_CHECK(XINCX_TIMER2_ENABLED)
void TIMER2_Handler(void)
{

	xincx_timer_2_irq_handler();

}
void xincx_timer_2_irq_handler(void)
{
    irq_handler(XINC_TIMER2, &m_cb[XINCX_TIMER2_INST_IDX],
        2);
}
#endif

#if NRFX_CHECK(XINCX_TIMER3_ENABLED)
void TIMER3_Handler(void)
{

	xincx_timer_3_irq_handler();

}
void xincx_timer_3_irq_handler(void)
{
    irq_handler(XINC_TIMER3, &m_cb[XINCX_TIMER3_INST_IDX],
        3);
}
#endif


#endif // NRFX_CHECK(XINCX_TIMER_ENABLED)
