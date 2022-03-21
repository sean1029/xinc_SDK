/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#include <xincx.h>

#if XINCX_CHECK(XINCX_RTC_ENABLED)

#if !(XINCX_CHECK(XINCX_RTC0_ENABLED))
#error "No enabled RTC instances. Check <xincx_config.h>."
#endif

#include <xincx_rtc.h>
#include "bsp_clk.h"

#define XINCX_LOG_MODULE RTC
#include <xincx_log.h>


#define HW_RTC_TIMEOUT      100000

/**@brief RTC driver instance control block structure. */
typedef struct
{
    xincx_drv_state_t state;        /**< Instance state. */
    bool             reliable;     /**< Reliable mode flag. */
    uint8_t          tick_latency; /**< Maximum length of interrupt handler in ticks (max 7.7 ms). */
} xincx_rtc_cb_t;

// User callbacks local storage.
static xincx_rtc_handler_t m_handlers[XINCX_RTC_ENABLED_COUNT];
static xincx_rtc_handler_t m_AoTimehandlers[XINCX_RTC_ENABLED_COUNT];
static xincx_rtc_cb_t      m_cb[XINCX_RTC_ENABLED_COUNT];
static volatile uint8_t 						calibration_flag;

static void xincx_rtc_clk_init(xincx_rtc_t const * const  p_instance,
                         xincx_rtc_config_t const * p_config)
{
   //   	__write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL, 0x20002);   // RTC_PCLK 时钟使能：
	//	__write_hw_reg32(CPR_AOCLKEN_GRCTL, 0x20002);       // RTC_CLK 时钟使能：
    
    p_instance->p_cpr->CTLAPBCLKEN_GRCTL = 0x20002;
 //   p_instance->p_cpr->AOCLK
}

xincx_err_t xincx_rtc_init(xincx_rtc_t const * const  p_instance,
                         xincx_rtc_config_t const * p_config,
                         xincx_rtc_handler_t        handler)
{	
    XINCX_ASSERT(p_config);
    XINCX_ASSERT(handler);
    xincx_err_t err_code = XINCX_SUCCESS;
    uint32_t reg;
    float freq = 0.0;
    volatile int32_t hw_timeout = HW_RTC_TIMEOUT;
    if(p_config->type == XINC_RTC_TYPE_AOTIME)
    {              
        m_AoTimehandlers[p_instance->instance_id] = handler;
        xc_rtc_clk_init();
        XINCX_IRQ_ENABLE(RTC_IRQn);

        return err_code;
    } 

    if (m_cb[p_instance->instance_id].state != XINCX_DRV_STATE_UNINITIALIZED)
    {
        err_code = XINCX_ERROR_INVALID_STATE;
        printf("Function: %s, error code",__func__);

        if(m_AoTimehandlers[p_instance->instance_id] != NULL)
        {
            err_code = XINCX_SUCCESS;
        }

        XINCX_LOG_WARNING("Function: %s, error code: %s.",
        __func__,
        XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
	
    xc_rtc_clk_init();
    uint32_t icr_en;
    icr_en =    (RTC_ICR_DaE_Disable << RTC_ICR_DaE_Pos)|
                (RTC_ICR_HoE_Disable << RTC_ICR_HoE_Pos)|
                (RTC_ICR_MiE_Disable << RTC_ICR_MiE_Pos)|
                (RTC_ICR_SeE_Disable << RTC_ICR_SeE_Pos)|
                (RTC_ICR_T2E_Disable << RTC_ICR_T2E_Pos)|
                (RTC_ICR_T1E_Disable << RTC_ICR_T1E_Pos)|
                (RTC_ICR_T3E_Disable << RTC_ICR_T3E_Pos)|
                (RTC_ICR_MASK_ALL_Disable << RTC_ICR_MASK_ALL_Pos)|
                (RTC_ICR_CntE_Disable << RTC_ICR_CntE_Pos);
    
    
    p_instance->p_reg->ICR = icr_en;
    m_handlers[p_instance->instance_id] = handler;
    calibration_flag = 0;
    p_instance->p_reg->AO_TIMER_CTL = 0;
    p_instance->p_reg->AO_TIMER_CTL |= ((32 << RTC_AO_TIMER_CTL_AO_TIMER_VALUE_Pos) & (RTC_AO_TIMER_CTL_AO_TIMER_VALUE_Msk));
    p_instance->p_reg->AO_TIMER_CTL |= ((RTC_AO_TIMER_CTL_FREQ_TIMER_EN_Enable << RTC_AO_TIMER_CTL_FREQ_TIMER_EN_Pos) & RTC_AO_TIMER_CTL_FREQ_TIMER_EN_Msk);
    
    reg = p_instance->p_reg->ALL_INTR_AO;

    //  XINCX_IRQ_PRIORITY_SET(p_instance->irq, p_config->interrupt_priority);
    XINCX_IRQ_ENABLE(RTC_IRQn);
    while((!calibration_flag) && (hw_timeout > 0))//
    {
        __nop();	
        hw_timeout--;
    }
    if(hw_timeout < 0)
    {
        err_code = XINCX_ERROR_TIMEOUT;
        return err_code;
    }
    reg = p_instance->p_reg->FREQ_32K_TIMER_VAL;
    p_instance->p_reg->AO_TIMER_CTL = (RTC_AO_TIMER_CTL_AO_TIMER_EN_Disable << RTC_AO_TIMER_CTL_AO_TIMER_EN_Pos) | (RTC_AO_TIMER_CTL_AO_TIMER_CLR_Clear << RTC_AO_TIMER_CTL_AO_TIMER_CLR_Pos);
    printf("FREQ_32K_TIMER_VA L reg: %d,hw_timeout:%d\r\n",reg,hw_timeout);
    
    freq = (float)(15625.0 / (float)reg) * 32768.0;
    
    printf("freq: %f\r\n",freq);


    uint8_t data_limit[3];
	
    data_limit[0] = p_config->hour_limit;
    data_limit[1] = p_config->min_limit;
    data_limit[2] = p_config->sec_limit;
	
    if(p_config->type != 0)
    {
        xinc_rtc_date_set(p_instance->p_reg, p_config->date);

        xinc_rtc_datelimit_set(p_instance->p_reg, data_limit);

        xinc_rtc_freq_set(p_instance->p_reg, (uint32_t)freq);
    }

        

    m_cb[p_instance->instance_id].reliable     = p_config->reliable;
    m_cb[p_instance->instance_id].state        = XINCX_DRV_STATE_INITIALIZED;

    err_code = XINCX_SUCCESS;
    XINCX_LOG_INFO("Function: %s, error code: %s.", __func__, XINCX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}


void xincx_rtc_uninit(xincx_rtc_t const * const p_instance)
{
    m_cb[p_instance->instance_id].state        = XINCX_DRV_STATE_UNINITIALIZED;
}


void xincx_rtc_enable(xincx_rtc_t const * const p_instance)
{
    XINCX_ASSERT(m_cb[p_instance->instance_id].state == XINCX_DRV_STATE_INITIALIZED);

    p_instance->p_reg->ICR |= RTC_ICR_CntE_Msk;
}

void xincx_rtc_disable(xincx_rtc_t const * const p_instance)
{
    p_instance->p_reg->ICR &= ~RTC_ICR_CntE_Msk;
}


void xincx_rtc_sec_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq)
{ 
    uint32_t mask = XINC_RTC_INT_SEC_MASK;
    if (enable_irq)
    {
        xinc_rtc_int_enable(p_instance->p_reg, mask);
    }
}

void xincx_rtc_sec_int_disable(xincx_rtc_t const * const p_instance)
{
    uint32_t mask = XINC_RTC_INT_SEC_MASK;
    xinc_rtc_int_disable(p_instance->p_reg, mask);
}

void xincx_rtc_min_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq)
{
    uint32_t mask = XINC_RTC_INT_MIN_MASK;
    if (enable_irq)
    {
        xinc_rtc_int_enable(p_instance->p_reg, mask);
    }
}

void xincx_rtc_min_int_disable(xincx_rtc_t const * const p_instance)
{
    uint32_t mask = XINC_RTC_INT_MIN_MASK;
    xinc_rtc_int_disable(p_instance->p_reg, mask);
}

void xincx_rtc_hour_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq)
{  
    uint32_t mask = XINC_RTC_INT_HOUR_MASK;
    if (enable_irq)
    {
        xinc_rtc_int_enable(p_instance->p_reg, mask);
    }
}

void xincx_rtc_hour_int_disable(xincx_rtc_t const * const p_instance)
{
    uint32_t mask = XINC_RTC_INT_HOUR_MASK;
    xinc_rtc_int_disable(p_instance->p_reg, mask);
}

void xincx_rtc_day_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq)
{
    uint32_t mask = XINC_RTC_INT_DAY_MASK;
    if (enable_irq)
    {
        xinc_rtc_int_enable(p_instance->p_reg, mask);
    }
}

void xincx_rtc_day_int_disable(xincx_rtc_t const * const p_instance)
{
    uint32_t mask = XINC_RTC_INT_DAY_MASK;
    xinc_rtc_int_disable(p_instance->p_reg, mask);
}

void xincx_rtc_time1_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq)
{  
    uint32_t mask = XINC_RTC_INT_TIME1_MASK;
    if (enable_irq)
    {
        xinc_rtc_int_enable(p_instance->p_reg, mask);
    }
}

void xincx_rtc_time1_int_disable(xincx_rtc_t const * const p_instance)
{
    uint32_t mask = XINC_RTC_INT_TIME1_MASK;
    xinc_rtc_int_disable(p_instance->p_reg, mask);
}

void xincx_rtc_time2_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq)
{  
    uint32_t mask = XINC_RTC_INT_TIME2_MASK;
    if (enable_irq)
    {
        xinc_rtc_int_enable(p_instance->p_reg, mask);
    }
}

void xincx_rtc_time2_int_disable(xincx_rtc_t const * const p_instance)
{
    uint32_t mask = XINC_RTC_INT_TIME2_MASK;
    xinc_rtc_int_disable(p_instance->p_reg, mask);
}

void xincx_rtc_time3_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq)
{ 
    uint32_t mask = XINC_RTC_INT_TIME3_MASK;
    if (enable_irq)
    {
        xinc_rtc_int_enable(p_instance->p_reg, mask);
    }
}

void xincx_rtc_time3_int_disable(xincx_rtc_t const * const p_instance)
{
    uint32_t mask = XINC_RTC_INT_TIME3_MASK;
    xinc_rtc_int_disable(p_instance->p_reg, mask);
}

void xincx_rtc_int_enable(xincx_rtc_t const * const p_instance, bool enable_irq)
{   
    uint32_t mask = XINC_RTC_INT_ALL_MASK;
    if (enable_irq)
    {
        xinc_rtc_int_disable(p_instance->p_reg, mask);
    }
}

void xincx_rtc_int_disable(xincx_rtc_t const * const p_instance)
{
    uint32_t mask = XINC_RTC_INT_ALL_MASK;
    xinc_rtc_int_enable(p_instance->p_reg, mask);
}

xincx_err_t xincx_rtc_time_set(xincx_rtc_t const * const p_instance,
                           xincx_rtc_match_timer_ch_t                 channel,
                           xincx_rtc_match_config_t  config ,
                           bool                     enable_irq)
{
    xincx_err_t err_code = XINCX_SUCCESS;
    __IOM uint32_t *reg = NULL;
    uint8_t irq_idx;
    switch(channel)
    {
        case XINCX_RTC_MATCH_TIME_1:
        {
            reg = &(p_instance->p_reg->CMR_ONE);
            irq_idx = RTC_ICR_T1E_Pos;
        }break;
        
        case XINCX_RTC_MATCH_TIME_2:
        {
            reg = &(p_instance->p_reg->CMR_TWO);
            irq_idx = RTC_ICR_T2E_Pos;
        }break;
        
        case XINCX_RTC_MATCH_TIME_3:
        {
            reg = &(p_instance->p_reg->CMR_THREE);
            irq_idx = RTC_ICR_T3E_Pos;
        }break;
        
        default:
                err_code = XINCX_ERROR_INVALID_PARAM;
        break;
    }
		
		if(reg != NULL)
		{
			*reg = config.value;
			printf("config.value:%x,ONE:%x\r\n",config.value,p_instance->p_reg->CMR_ONE);
			if(enable_irq)
			{
				xinc_rtc_int_enable(p_instance->p_reg, 0x01 << irq_idx);
			}
			 printf("ICR:0x%x\r\n",p_instance->p_reg->ICR);
		}
		
	return err_code ;
}

xincx_err_t xincx_rtc_time_disable(xincx_rtc_t const * const p_instance, xincx_rtc_match_timer_ch_t channel)
{
    uint8_t irq_idx;
    xincx_err_t err_code = XINCX_SUCCESS;
    switch(channel)
    {
        case XINCX_RTC_MATCH_TIME_1:
        {
            irq_idx = RTC_ICR_T1E_Pos;
        }break;
        
        case XINCX_RTC_MATCH_TIME_2:
        {
    
            irq_idx = RTC_ICR_T2E_Pos;
        }break;
        
        case XINCX_RTC_MATCH_TIME_3:
        {
            irq_idx = RTC_ICR_T3E_Pos;
        }break;
        
        default:
            err_code = XINCX_ERROR_INVALID_PARAM;
        break;
    }
    xinc_rtc_int_disable(p_instance->p_reg, 0x01 << irq_idx);
    
    return err_code ;
}

void xincx_rtc_AOtime_set(xincx_rtc_t const * const p_instance,                         
                           uint32_t  tick)
{
    p_instance->p_reg->AO_TIMER_CTL = 0;
    p_instance->p_reg->AO_TIMER_CTL |= (tick << RTC_AO_TIMER_CTL_AO_TIMER_VALUE_Pos) & RTC_AO_TIMER_CTL_AO_TIMER_VALUE_Msk;
    p_instance->p_reg->AO_TIMER_CTL |= (RTC_AO_TIMER_CTL_AO_TIMER_EN_Enable << RTC_AO_TIMER_CTL_AO_TIMER_EN_Pos);
}

void xincx_rtc_date_set(xincx_rtc_t const * const p_instance,                         
                           xinc_rtc_time_t  date)
{
    xinc_rtc_date_set(p_instance->p_reg,date);

}

void xincx_rtc_date_get(xincx_rtc_t const * const p_instance,                         
                           xinc_rtc_time_t  *date)
{
    xinc_rtc_date_get(p_instance->p_reg,date);
}

static void irq_handler(XINC_RTC_Type * p_reg,
                        uint32_t       instance_id)
{
    uint32_t reg,clr_reg;
    reg = p_reg->ALL_INTR_AO;
    //	printf("RTC irq_handler:%x\r\n",reg);
    if(reg & (RTC_AO_ALL_INTR_AO_TIMER_INTR_Msk))
    {
        clr_reg = p_reg->AO_TIMER_CTL;
        clr_reg |= RTC_AO_TIMER_CTL_AO_TIMER_CLR_Clear << RTC_AO_TIMER_CTL_AO_TIMER_CLR_Pos;
        clr_reg &= ~(RTC_AO_TIMER_CTL_AO_TIMER_EN_Enable << RTC_AO_TIMER_CTL_AO_TIMER_EN_Pos);
        p_reg->AO_TIMER_CTL = clr_reg;
        
        calibration_flag = 1;
    //		if(m_cb[instance_id].state == XINCX_DRV_STATE_UNINITIALIZED)
    //		{
    //			XINCX_LOG_INFO("RTC 32K  calibration ok.\r\n");
    //			calibration_flag = 1;
    //			printf("RTC 32K calibration ok.\r\n");
    //		}
        //else
        {
            if(m_AoTimehandlers[instance_id])
            {
                m_AoTimehandlers[instance_id](XINCX_RTC_INT_AOTIME);
            }
            
        }
    }

    reg = p_reg->ISR_EOI;
    if(reg)
    {
        p_reg->ISR_EOI = reg;
        if(m_cb[instance_id].state == XINCX_DRV_STATE_INITIALIZED)
        {
            if((reg & XINC_RTC_INT_DAY_MASK) == XINC_RTC_INT_DAY_MASK)
            {
            //	printf("sec add\n");	
                m_handlers[instance_id](XINCX_RTC_INT_DAY);
            }
            
            if((reg & XINC_RTC_INT_HOUR_MASK) == XINC_RTC_INT_HOUR_MASK)
            {
            //	printf("sec add\n");	
                m_handlers[instance_id](XINCX_RTC_INT_HOUR);
            }
            
            if((reg & XINC_RTC_INT_MIN_MASK) == XINC_RTC_INT_MIN_MASK)
            {
            //	printf("sec add\n");	
                m_handlers[instance_id](XINCX_RTC_INT_MIN);
            }
            
            if((reg & XINC_RTC_INT_SEC_MASK) == XINC_RTC_INT_SEC_MASK)
            {
            //	printf("sec add\n");	
                m_handlers[instance_id](XINCX_RTC_INT_SEC);
            }
            
            if((reg & XINC_RTC_INT_TIME1_MASK) == XINC_RTC_INT_TIME1_MASK)
            {
            //	printf("sec add\n");	
                m_handlers[instance_id](XINCX_RTC_INT_TIME1);
            }
           
            if((reg & XINC_RTC_INT_TIME2_MASK) == XINC_RTC_INT_TIME2_MASK)
            {
            //	printf("sec add\n");	
                m_handlers[instance_id](XINCX_RTC_INT_TIME2);
            }
            
            if((reg & XINC_RTC_INT_TIME3_MASK) == XINC_RTC_INT_TIME3_MASK)
            {
            //	printf("sec add\n");	
                m_handlers[instance_id](XINCX_RTC_INT_TIME3);
            }
                 
        }
    }

}

#if XINCX_CHECK(XINCX_RTC0_ENABLED)
void xincx_rtc_0_irq_handler(void)
{
    irq_handler(XINC_RTC0, XINCX_RTC0_INST_IDX);
}
#endif

#if XINCX_CHECK(XINCX_RTC_ENABLED)
void RTC_Handler(void)
{		
    xincx_rtc_0_irq_handler();
}

#endif



#endif // XINCX_CHECK(XINCX_RTC_ENABLED)
