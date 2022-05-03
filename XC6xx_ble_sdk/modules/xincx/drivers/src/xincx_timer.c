/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#include <xincx.h>

#if XINCX_CHECK(XINCX_TIMER_ENABLED)

#if !(XINCX_CHECK(XINCX_TIMER0_ENABLED) || XINCX_CHECK(XINCX_TIMER1_ENABLED) || \
      XINCX_CHECK(XINCX_TIMER2_ENABLED) || XINCX_CHECK(XINCX_TIMER3_ENABLED))
#error "No enabled TIMER instances. Check <xincx_config.h>."
#endif

#include <xincx_timer.h>

#define XINCX_LOG_MODULE TIMER
#include <xincx_log.h>

/**@brief Timer control block. */
typedef struct
{
    xincx_timer_event_handler_t handler;
    void *                     context;
    xincx_drv_state_t           state;
} timer_control_block_t;

static timer_control_block_t m_cb[XINCX_TIMER_ENABLED_COUNT];



static void xincx_timer_clk_init(xincx_timer_t const * const  p_instance,
                         xincx_timer_config_t const * p_config)
{
    XINC_CPR_CTL_Type * p_cpr = (XINC_CPR_CTL_Type *)p_instance->p_cpr;
    
    //TIMER_PCLK en
    p_cpr->CTLAPBCLKEN_GRCTL = ((CPR_CTLAPBCLKEN_GRCTL_TIMER_PCLK_EN_Enable << CPR_CTLAPBCLKEN_GRCTL_TIMER_PCLK_EN_Pos) |
                                (CPR_CTLAPBCLKEN_GRCTL_TIMER_PCLK_EN_Msk << CPR_CTLAPBCLKEN_GRCTL_MASK_OFFSET));
    



}
xincx_err_t xincx_timer_init(xincx_timer_t const * const  p_instance,
                           xincx_timer_config_t const * p_config,
                           xincx_timer_event_handler_t  timer_event_handler)
{

    xincx_err_t err_code;

    timer_control_block_t * p_cb = &m_cb[p_instance->instance_idx];

    #ifdef SOFTDEVICE_PRESENT
    XINCX_ASSERT(p_instance->p_reg != XINC_TIMER0);
    #endif
    XINCX_ASSERT(p_config);
    XINCX_ASSERT(timer_event_handler);


    if (p_cb->state != XINCX_DRV_STATE_UNINITIALIZED)
    {
        err_code = XINCX_ERROR_INVALID_STATE;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                                            __func__,
                                            XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
    printf("p_config->ref_clk val:%d,p_config->clk_src:%d\r\n",p_config->ref_clk,p_config->clk_src);
    if(p_config->ref_clk > XINC_TIMER_REF_CLK_32000Hz)
    {
        err_code = XINCX_ERROR_INVALID_PARAM;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                                            __func__,
                                            XINCX_LOG_ERROR_STRING_GET(err_code));
        
        return err_code;
    }

    xincx_timer_clk_init(p_instance,p_config);
    
      
  //  p_instance->p_cpr->CTLAPBCLKEN_GRCTL = ((0x01 << 3) | (0x01 << 19));//TIMER_PCLK ʱ��ʹ��
    printf("frequency val:0x%08x\r\n",p_config->ref_clk);
    xinc_timer_clk_div_set(p_instance->p_cpr,p_instance->id,p_config->clk_src,p_config->ref_clk); //TIMERx_CLK ʱ�ӿ��ƼĴ��� mclk_in(32MHz)/2x(0x0f + 0x1)=1M
    printf("mode_set val:0x%08x\r\n",p_config->mode);
    xinc_timer_mode_set(p_instance->p_reg, p_config->mode);

    p_instance->p_cpr->LP_CTL |= (0x01 << 2);
    
    printf("p_cpr->LP_CTL:%p,0x%x\r\n",&p_instance->p_cpr->LP_CTL,p_instance->p_cpr->LP_CTL);
    
    p_cb->handler = timer_event_handler;
    p_cb->context = p_config->p_context;

    // clear timer interrupt
    xinc_timer_int_clear(p_instance->p_reg,XINC_TIMER_EVENT_TIMEOUT);


    XINCX_IRQ_PRIORITY_SET((IRQn_Type)(TIMER0_IRQn + p_instance->id),
        p_config->interrupt_priority);
    XINCX_IRQ_ENABLE((IRQn_Type)(TIMER0_IRQn + p_instance->id));


    p_cb->state = XINCX_DRV_STATE_INITIALIZED;

    err_code = XINCX_SUCCESS;
    XINCX_LOG_INFO("Function: %s, error code: %s.",
                                __func__,
                                XINCX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
	
}

void xincx_timer_uninit(xincx_timer_t const * const p_instance)
{
    XINCX_IRQ_DISABLE(xincx_get_irq_number(p_instance->p_reg));

    #define DISABLE_ALL UINT32_MAX
    xinc_timer_int_disable(p_instance->p_reg, DISABLE_ALL);
    #undef DISABLE_ALL

    xincx_timer_disable(p_instance);

    m_cb[p_instance->instance_idx].state = XINCX_DRV_STATE_UNINITIALIZED;
    XINCX_LOG_INFO("Uninitialized instance: %d.", p_instance->instance_idx);
}

void xincx_timer_enable(xincx_timer_t const * const p_instance)
{
    XINCX_ASSERT(m_cb[p_instance->instance_idx].state == XINCX_DRV_STATE_INITIALIZED);
    xinc_timer_enable(p_instance->p_reg);
    m_cb[p_instance->instance_idx].state = XINCX_DRV_STATE_POWERED_ON;
    XINCX_LOG_INFO("Enabled instance: %d.", p_instance->instance_idx);
}

void xincx_timer_disable(xincx_timer_t const * const p_instance)
{
    XINCX_ASSERT(m_cb[p_instance->instance_idx].state != XINCX_DRV_STATE_UNINITIALIZED);

    xinc_timer_disable(p_instance->p_reg);
  //  printf("timer TCR=[%x]\n", p_instance->p_reg->TCR);

    m_cb[p_instance->instance_idx].state = XINCX_DRV_STATE_INITIALIZED;
    XINCX_LOG_INFO("Disabled instance: %d.", p_instance->instance_idx);
}

bool xincx_timer_is_enabled(xincx_timer_t const * const p_instance)
{
    XINCX_ASSERT(m_cb[p_instance->instance_idx].state != XINCX_DRV_STATE_UNINITIALIZED);
    return (m_cb[p_instance->instance_idx].state == XINCX_DRV_STATE_POWERED_ON);
}


uint32_t xincx_timer_cnt_get(xincx_timer_t const * const p_instance)
{
    return xinc_timer_cc_read(p_instance->p_reg);
}

void xincx_timer_compare(xincx_timer_t const * const p_instance,               
                        uint32_t                   cc_value,
                        xinc_timer_mode_t                       mode,bool enable)
{
    xinc_timer_ctr_mask_t timer_int = XINC_TIMER_INT_MASK;
    xincx_timer_disable(p_instance);

    xinc_timer_mode_set(p_instance->p_reg,mode);
    xinc_timer_int_clear(p_instance->p_reg,XINC_TIMER_EVENT_TIMEOUT);
    xinc_timer_int_enable(p_instance->p_reg, timer_int);

    xinc_timer_cc_write(p_instance->p_reg, cc_value);
    if(enable)
    {
          xincx_timer_enable(p_instance);
    }
  
    XINCX_LOG_INFO("Timer id: %d, capture value set: %lu.",
                    p_instance->instance_idx,
                    cc_value);
}


void xincx_timer_compare_int_enable(xincx_timer_t const * const p_instance,
                                   uint32_t                   channel)
{
    XINCX_ASSERT(m_cb[p_instance->instance_idx].state != XINCX_DRV_STATE_UNINITIALIZED);

    xinc_timer_int_clear(p_instance->p_reg,XINC_TIMER_EVENT_TIMEOUT);

    xinc_timer_int_enable(p_instance->p_reg,TIMERx_TCR_TIM_Msk);
}

void xincx_timer_compare_int_disable(xincx_timer_t const * const p_instance,
                                    uint32_t                   channel)
{
    XINCX_ASSERT(m_cb[p_instance->instance_idx].state != XINCX_DRV_STATE_UNINITIALIZED);

    xinc_timer_int_disable(p_instance->p_reg,TIMERx_TCR_TIM_Msk);
}

static void irq_handler(XINC_TIMER_Type        * p_reg,
                        timer_control_block_t * p_cb,
                        uint8_t                 channel)
{
//    printf("Tim_irq_handler:%d\r\n",channel);

    {
        xinc_timer_int_event_t event = XINC_TIMER_EVENT_TIMEOUT;
        xinc_timer_ctr_mask_t int_mask = XINC_TIMER_INT_MASK;
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

#if XINCX_CHECK(XINCX_TIMER0_ENABLED)
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

#if XINCX_CHECK(XINCX_TIMER1_ENABLED)
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

#if XINCX_CHECK(XINCX_TIMER2_ENABLED)
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

#if XINCX_CHECK(XINCX_TIMER3_ENABLED)
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


#endif // XINCX_CHECK(XINCX_TIMER_ENABLED)
