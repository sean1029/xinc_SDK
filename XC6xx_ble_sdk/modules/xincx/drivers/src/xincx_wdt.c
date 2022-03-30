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

#if !XINCX_CHECK(XINCX_WDT0_ENABLED)
#error "No enabled WDT instances. Check <xincx_config.h>."
#endif

#define XINCX_LOG_MODULE WDT
#include <xincx_log.h>


// Control block - driver instance local data.
typedef struct
{
    xincx_wdt_event_handler_t  handler;
    void *                  p_context;
    volatile uint32_t       int_mask;
    xinc_wdt_mode_t         mode;
    uint32_t                flags;
    xincx_drv_state_t       state;
    bool                    error;
} wdt_control_block_t;

static wdt_control_block_t m_cb[XINCX_WDT_ENABLED_COUNT];
static xincx_drv_state_t   g_state = XINCX_DRV_STATE_UNINITIALIZED;

/**@brief WDT state. */
//static xincx_drv_state_t m_state;
//static xinc_wdt_mode_t m_mode = XINC_WDT_MODE_RUN_0;


#if !XINCX_CHECK(XINCX_WDT_CONFIG_NO_IRQ)
/**@brief WDT event handler. */
//static xincx_wdt_event_handler_t m_wdt_event_handler = NULL;

static void irq_handler(XINC_WDT_Type * p_reg, wdt_control_block_t * p_cb)
{
    uint32_t stat = p_reg->STAT;
    if(((stat & WDT_STAT_STAT_Msk )) >> WDT_STAT_STAT_Pos == WDT_STAT_STAT_Generated)
    {
        if(p_cb->handler)
        {
            p_cb->handler();
        }
    }
}

#if XINCX_CHECK(XINCX_WDT0_ENABLED)
/**@brief WDT interrupt handler. */
void xincx_wdt0_irq_handler(void)
{
    
    irq_handler(XINC_WDT0, &m_cb[XINCX_WDT0_INST_IDX]);

}
#else
void xincx_wdt0_irq_handler(void)
{

}
#endif

void WDT_Handler()
{
	xincx_wdt0_irq_handler();
}

#endif

static void xincx_wdt_clk_init(xincx_wdt_t  const * const p_instance,xincx_wdt_config_t const * p_config)
{
    XINC_CPR_CTL_Type * p_cpr = (XINC_CPR_CTL_Type *)p_instance->p_cpr;
    
     //WDT_PCLK 使能 ;0x10001 
    p_cpr->CTLAPBCLKEN_GRCTL = (CPR_CTLAPBCLKEN_GRCTL_WDT_PCLK_EN_Enable << CPR_CTLAPBCLKEN_GRCTL_WDT_PCLK_EN_Pos) |
                                (CPR_CTLAPBCLKEN_GRCTL_WDT_PCLK_EN_Msk << CPR_CTLAPBCLKEN_GRCTL_MASK_OFFSET);
   
    //WDT 复位  0x40004;
    p_cpr->RSTCTL_CTLAPB_SW = (CPR_RSTCTL_CTLAPB_SW_WDT_RSTN_Enable << CPR_RSTCTL_CTLAPB_SW_WDT_RSTN_Pos) |
                               (CPR_RSTCTL_CTLAPB_SW_WDT_RSTN_Msk << CPR_RSTCTL_CTLAPB_SW_MASK_OFFSET);

    p_cpr->RSTCTL_WDTRST_MASK = (CPR_RSTCTL_WDTRST_MASK_WDT_M0_RSTN_MASK_Enable << CPR_RSTCTL_WDTRST_MASK_WDT_M0_RSTN_MASK_Pos);
    
    


}

xincx_err_t xincx_wdt_init(xincx_wdt_t  const * const p_instance,xincx_wdt_config_t const * p_config,
                         xincx_wdt_event_handler_t  wdt_event_handler)
{

    printf("Function: %s\n",__func__);

    
    XINCX_ASSERT(p_config);
    xincx_err_t err_code = XINCX_SUCCESS;
    
    wdt_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    
    if (p_cb->state != XINCX_DRV_STATE_UNINITIALIZED)
    {
        err_code = XINCX_ERROR_INVALID_STATE;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                            __func__,
                            XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
    
    XINC_WDT_Type * p_reg = (XINC_WDT_Type *)p_instance->p_reg;
    if(g_state == XINCX_DRV_STATE_UNINITIALIZED)
    {
        xincx_wdt_clk_init(p_instance,p_config);
        g_state = XINCX_DRV_STATE_INITIALIZED;
    }


#if !XINCX_CHECK(XINCX_WDT_CONFIG_NO_IRQ)
    XINCX_ASSERT(wdt_event_handler != NULL);
    p_cb->handler = wdt_event_handler;
#else
    XINCX_ASSERT(wdt_event_handler == NULL);
    (void)wdt_event_handler;
    p_cb->handler = NULL;
#endif
    

#if !XINCX_CHECK(XINCX_WDT_CONFIG_NO_IRQ)
    XINCX_IRQ_PRIORITY_SET((IRQn_Type)(WDT_IRQn + p_instance->id), p_config->interrupt_priority);
    XINCX_IRQ_ENABLE((IRQn_Type)(WDT_IRQn + p_instance->id));
    p_cb->mode = XINC_WDT_MODE_RUN_1;
#endif
    xinc_wdt_mode_set(p_reg,p_cb->mode);
    xinc_wdt_reload_value_set(p_reg,(uint32_t) p_config->reload_value);
    
    p_cb->state = XINCX_DRV_STATE_INITIALIZED;
    err_code = XINCX_SUCCESS;

    XINCX_LOG_INFO("Function: %s, error code: %s.", __func__, XINCX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}


void xincx_wdt_enable(xincx_wdt_t  const * const p_instance)
{
    wdt_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    XINC_WDT_Type * p_reg = (XINC_WDT_Type *)p_instance->p_reg;
    
	uint8_t val = (WDT_CR_RPL_4pclk << WDT_CR_RPL_Pos) | WDT_CR_WDT_EN_Msk;
		
    XINCX_ASSERT(p_cb->state == XINCX_DRV_STATE_INITIALIZED);
#if !XINCX_CHECK(XINCX_WDT_CONFIG_NO_IRQ)
    xinc_wdt_enable(p_reg,val);
	#else
    xinc_wdt_enable(p_reg,val);
#endif
    p_cb->state = XINCX_DRV_STATE_POWERED_ON;
    XINCX_LOG_INFO("Enabled.");
}


void xincx_wdt_feed(xincx_wdt_t  const * const p_instance)
{
    wdt_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    XINC_WDT_Type * p_reg = (XINC_WDT_Type *)p_instance->p_reg;
    
    XINCX_ASSERT(p_cb->state == XINCX_DRV_STATE_POWERED_ON);
    uint32_t icr = p_reg->ICR;  
    xinc_wdt_reload_request_set(p_reg,WDT_CRR_CRR_Enable);
}


#endif // XINCX_CHECK(XINCX_WDT_ENABLED)
