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

#if NRFX_CHECK(XINCX_WDT_ENABLED)
#include <xincx_wdt.h>

#define NRFX_LOG_MODULE WDT
#include <nrfx_log.h>


/**@brief WDT state. */
static nrfx_drv_state_t m_state;
static xinc_wdt_mode_t m_mode = XINC_WDT_MODE_RUN_0;
/**@brief WDT alloc table. */
static uint8_t m_alloc_index;

#if !NRFX_CHECK(XINCX_WDT_CONFIG_NO_IRQ)
/**@brief WDT event handler. */
static xincx_wdt_event_handler_t m_wdt_event_handler = NULL;


void WDT_Handler()
{
	xincx_wdt_irq_handler();
}
/**@brief WDT interrupt handler. */
void xincx_wdt_irq_handler(void)
{
		uint32_t stat = XINC_WDT->STAT;
		uint32_t icr = XINC_WDT->ICR;  
    {
				if(m_wdt_event_handler)
				{
					 m_wdt_event_handler();
				}
       
        
    }
}
#endif


nrfx_err_t xincx_wdt_init(xincx_wdt_config_t const * p_config,
                         xincx_wdt_event_handler_t  wdt_event_handler)
{

		printf("Function: %s\n",__func__);
		XINC_CPR->CTLAPBCLKEN_GRCTL = 0x10001;////WDT_PCLK 时钟使能

		XINC_CPR->RSTCTL_CTLAPB_SW = 0x40000;//WDT 模块软复位  

		XINC_CPR->RSTCTL_CTLAPB_SW = 0x40004;

		XINC_CPR->RSTCTL_WDTRST_MASK = 0x02;//屏蔽 WDT 引起的 M0 的软复位,不屏蔽 WDT 引起的系统的软复位
		
		NRFX_ASSERT(p_config);
		nrfx_err_t err_code;

#if !NRFX_CHECK(XINCX_WDT_CONFIG_NO_IRQ)
    NRFX_ASSERT(wdt_event_handler != NULL);
    m_wdt_event_handler = wdt_event_handler;
#else
    NRFX_ASSERT(wdt_event_handler == NULL);
    (void)wdt_event_handler;
#endif
    if (m_state == NRFX_DRV_STATE_UNINITIALIZED)
    {
        m_state = NRFX_DRV_STATE_INITIALIZED;
    }
    else
    {
        err_code = NRFX_ERROR_INVALID_STATE;
        NRFX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         NRFX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }


#if !NRFX_CHECK(XINCX_WDT_CONFIG_NO_IRQ)
    NRFX_IRQ_PRIORITY_SET(WDT_IRQn, p_config->interrupt_priority);
    NRFX_IRQ_ENABLE(WDT_IRQn);
		m_mode = XINC_WDT_MODE_RUN_1;
#endif
		xinc_wdt_mode_set(m_mode);
		xinc_wdt_reload_value_set((uint32_t) p_config->reload_value);
		
    err_code = NRFX_SUCCESS;
	

    NRFX_LOG_INFO("Function: %s, error code: %s.", __func__, NRFX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}


void xincx_wdt_enable(void)
{
		uint8_t val = 5;
		
    NRFX_ASSERT(m_alloc_index != 0);
    NRFX_ASSERT(m_state == NRFX_DRV_STATE_INITIALIZED);
#if !NRFX_CHECK(XINCX_WDT_CONFIG_NO_IRQ)
    xinc_wdt_int_enable(val);
	#else
		xinc_wdt_int_enable(val);
#endif
    m_state = NRFX_DRV_STATE_POWERED_ON;
    NRFX_LOG_INFO("Enabled.");
}


void xincx_wdt_feed(void)
{
  NRFX_ASSERT(m_state == NRFX_DRV_STATE_POWERED_ON);

	xinc_wdt_reload_request_set(0x76);
}


#endif // NRFX_CHECK(XINCX_WDT_ENABLED)
