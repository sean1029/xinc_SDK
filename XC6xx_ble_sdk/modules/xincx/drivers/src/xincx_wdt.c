/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#include <xincx.h>

#if XINCX_CHECK(XINCX_WDT_ENABLED)
#include <xincx_wdt.h>

#define XINCX_LOG_MODULE WDT
#include <xincx_log.h>


/**@brief WDT state. */
static xincx_drv_state_t m_state;
static xinc_wdt_mode_t m_mode = XINC_WDT_MODE_RUN_0;
/**@brief WDT alloc table. */
static uint8_t m_alloc_index;

#if !XINCX_CHECK(XINCX_WDT_CONFIG_NO_IRQ)
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

    if(((stat & WDT_STAT_STAT_Msk )) >> WDT_STAT_STAT_Pos == WDT_STAT_STAT_Generated)
    {
        if(m_wdt_event_handler)
        {
            m_wdt_event_handler();
        }
    }
}
#endif


xincx_err_t xincx_wdt_init(xincx_wdt_config_t const * p_config,
                         xincx_wdt_event_handler_t  wdt_event_handler)
{

    printf("Function: %s\n",__func__);
    XINC_CPR->CTLAPBCLKEN_GRCTL = 0x10001;////WDT_PCLK 使能

    XINC_CPR->RSTCTL_CTLAPB_SW = 0x40000;//WDT 复位 

    XINC_CPR->RSTCTL_CTLAPB_SW = 0x40004;

    XINC_CPR->RSTCTL_WDTRST_MASK = 0x02;//WDT ����� M0 ������λ,������ WDT �����ϵͳ������λ
    
    XINCX_ASSERT(p_config);
    xincx_err_t err_code;

#if !XINCX_CHECK(XINCX_WDT_CONFIG_NO_IRQ)
    XINCX_ASSERT(wdt_event_handler != NULL);
    m_wdt_event_handler = wdt_event_handler;
#else
    XINCX_ASSERT(wdt_event_handler == NULL);
    (void)wdt_event_handler;
#endif
    if (m_state == XINCX_DRV_STATE_UNINITIALIZED)
    {
        m_state = XINCX_DRV_STATE_INITIALIZED;
    }
    else
    {
        err_code = XINCX_ERROR_INVALID_STATE;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }


#if !XINCX_CHECK(XINCX_WDT_CONFIG_NO_IRQ)
    XINCX_IRQ_PRIORITY_SET(WDT_IRQn, p_config->interrupt_priority);
    XINCX_IRQ_ENABLE(WDT_IRQn);
    m_mode = XINC_WDT_MODE_RUN_1;
#endif
    xinc_wdt_mode_set(m_mode);
    xinc_wdt_reload_value_set((uint32_t) p_config->reload_value);
    
    err_code = XINCX_SUCCESS;

    XINCX_LOG_INFO("Function: %s, error code: %s.", __func__, XINCX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}


void xincx_wdt_enable(void)
{
	uint8_t val = (WDT_CR_RPL_4pclk << WDT_CR_RPL_Pos) | WDT_CR_WDT_EN_Msk;
		
    XINCX_ASSERT(m_alloc_index != 0);
    XINCX_ASSERT(m_state == XINCX_DRV_STATE_INITIALIZED);
#if !XINCX_CHECK(XINCX_WDT_CONFIG_NO_IRQ)
    xinc_wdt_enable(val);
	#else
    xinc_wdt_enable(val);
#endif
    m_state = XINCX_DRV_STATE_POWERED_ON;
    XINCX_LOG_INFO("Enabled.");
}


void xincx_wdt_feed(void)
{
    XINCX_ASSERT(m_state == XINCX_DRV_STATE_POWERED_ON);

    xinc_wdt_reload_request_set(WDT_CRR_CRR_Enable);
}


#endif // XINCX_CHECK(XINCX_WDT_ENABLED)
