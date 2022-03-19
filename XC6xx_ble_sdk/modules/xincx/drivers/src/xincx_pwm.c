/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include <xincx.h>

#if XINCX_CHECK(XINCX_PWM_ENABLED)

#if !(XINCX_CHECK(XINCX_PWM0_ENABLED) || XINCX_CHECK(XINCX_PWM1_ENABLED) || \
      XINCX_CHECK(XINCX_PWM2_ENABLED) || XINCX_CHECK(XINCX_PWM3_ENABLED) || \
			XINCX_CHECK(XINCX_PWM4_ENABLED) || XINCX_CHECK(XINCX_PWM5_ENABLED))
#error "No enabled PWM instances. Check <xincx_config.h>."
#endif

#include <xincx_pwm.h>
#include <hal/xinc_gpio.h>

#define XINCX_LOG_MODULE PWM
#include <xincx_log.h>


// Control block - driver instance local data.
typedef struct
{
    xincx_drv_state_t volatile state;
    uint8_t                   flags;
    xinc_pwm_clk_src_t clk_src;          ///< Bit width.
    xinc_pwm_ref_clk_t  ref_clk;   ///< Base clock frequency.
    uint8_t period; //set pwm out freq
    uint8_t ocpy;// duty cycle
 
} pwm_control_block_t;
static pwm_control_block_t m_cb[XINCX_PWM_ENABLED_COUNT];

static uint16_t xincx_pwm_freq_to_period(uint8_t inst_idx,uint32_t freq);

static xincx_err_t configure_pins(xincx_pwm_t const * const p_instance,xincx_pwm_config_t const * p_config)
{
    xincx_err_t err_code = XINCX_SUCCESS;
    switch(p_instance->id)
    {
        case XINC_PWM_ID_0:      
        case XINC_PWM_ID_1:            
        {   
            err_code = xinc_gpio_secfun_config(p_instance->output_pin,(xinc_gpio_pin_fun_sel_t)(XINC_GPIO_PIN_PWM0 + p_instance->id));
            if(err_code != XINCX_SUCCESS)
            {
                return err_code;
            }
            #if (XINCX_CHECK(XINCX_PWM0_ENABLED))
            if(p_config->inv_enable)
            {
                err_code = xinc_gpio_secfun_config(p_instance->output_inv_pin,(xinc_gpio_pin_fun_sel_t)(XINC_GPIO_PIN_PWM0_INV + p_instance->id));
                if(err_code != XINCX_SUCCESS)
                {
                    return err_code;
                }
            }
            #endif
        }break;
        
        case XINC_PWM_ID_2:
        {
            if(p_instance->output_pin == XINC_GPIO_0)
            {
                err_code = xinc_gpio_secfun_config(p_instance->output_pin,(xinc_gpio_pin_fun_sel_t)(XINC_GPIO_PIN_PWM2));
                if(err_code != XINCX_SUCCESS)
                {
                    return err_code;
                }
            }					
        }break;
        
        case XINC_PWM_ID_3:
        {
            if(p_instance->output_pin == XINC_GPIO_1)
            {
                err_code = xinc_gpio_secfun_config(p_instance->output_pin,(xinc_gpio_pin_fun_sel_t)(XINC_GPIO_PIN_PWM3));
                if(err_code != XINCX_SUCCESS)
                {
                    return err_code;
                }
            }
        }break;
        
        case XINC_PWM_ID_4:
        {
            if(p_instance->output_pin == XINC_GPIO_12)
            {
                err_code = xinc_gpio_secfun_config(p_instance->output_pin,(xinc_gpio_pin_fun_sel_t)(XINC_GPIO_PIN_PWM4));
                if(err_code != XINCX_SUCCESS)
                {
                    return err_code;
                }
            }
        }break;
        
        case XINC_PWM_ID_5:
        {
            if(p_instance->output_pin == XINC_GPIO_13)
            {
                err_code = xinc_gpio_secfun_config(p_instance->output_pin,(xinc_gpio_pin_fun_sel_t)(XINC_GPIO_PIN_PWM5));
                if(err_code != XINCX_SUCCESS)
                {
                    return err_code;
                }
            }
        }break;
            
        default:
        {
            err_code = XINCX_ERROR_NOT_SUPPORTED;
        }break;
              
    }

    return err_code;
}
                          

xincx_err_t xincx_pwm_init(xincx_pwm_t const * const p_instance,
                         xincx_pwm_config_t const * p_config,
                         xincx_pwm_handler_t        handler)
{
    XINCX_ASSERT(p_config);

    xincx_err_t err_code = XINCX_SUCCESS;

    pwm_control_block_t * p_cb  = &m_cb[p_instance->drv_inst_idx];

    if (p_cb->state != XINCX_DRV_STATE_UNINITIALIZED)
    {
        err_code = XINCX_ERROR_INVALID_STATE;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                            __func__,
                            XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }

    err_code = configure_pins(p_instance,p_config);
    if(err_code != XINCX_SUCCESS)
    {
        return err_code;
    }
        
    printf("p_instance->id:%d,p_instance->drv_inst_idx:%d,XINCX_PWM_ENABLED_COUNT:%d \r\n",p_instance->id,p_instance->drv_inst_idx,XINCX_PWM_ENABLED_COUNT);

    for(uint8_t i = 0;i < XINCX_PWM_ENABLED_COUNT;i ++)
    {
        pwm_control_block_t * p_cb1  = &m_cb[i];
        if (p_cb1->state == XINCX_DRV_STATE_INITIALIZED)
        {			
                
            if(p_config->clk_src != p_cb1->clk_src)
            {
                err_code = XINCX_ERROR_INVALID_PARAM;
            }
            if(p_config->clk_src != XINC_PWM_CLK_SRC_32K)
            {
                if(p_config->ref_clk != p_cb1->ref_clk)
                {
                    err_code = XINCX_ERROR_INVALID_PARAM;
                }	
            }
						
            if(err_code != XINCX_SUCCESS)
            {
                XINCX_LOG_WARNING("Function: %s, error code: %s.",
                                            __func__,
                                            XINCX_LOG_ERROR_STRING_GET(err_code));
                printf("Function: %s, error code: %s.\r\n",
                                            __func__,
                                            XINCX_LOG_ERROR_STRING_GET(err_code));
                printf("err i:%d,clk_src:%d,ref_clk:%d,clk_src:%d,ref_clk:%d \r\n",i,p_cb1->clk_src,p_cb1->ref_clk,p_config->clk_src,p_config->ref_clk);
        
                return err_code;
            }	
							
        }
    }
		
    if(p_config->ref_clk > XINC_PWM_REF_CLK_32000Hz)
    {
        err_code = XINCX_ERROR_INVALID_PARAM;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                                            __func__,
                                            XINCX_LOG_ERROR_STRING_GET(err_code));
        
        return err_code;
    }
	
    p_instance->p_cpr->CTLAPBCLKEN_GRCTL = 0x10001000;//PWM_PCLK ʱ��ʹ��

    xinc_pwm_disable(p_instance->p_reg);

    configure_pins(p_instance, p_config);
        
    p_cb->clk_src = p_config->clk_src;
    p_cb->ref_clk = p_config->ref_clk;

    printf("p_cb->clk_src=%d,p_cb->ref_clk:%d\r\n", p_cb->clk_src,p_cb->ref_clk);
    xinc_pwm_clk_div_set(p_instance->p_cpr,p_instance->id,p_config->clk_src,p_config->ref_clk);

    uint16_t period;
    period = xincx_pwm_freq_to_period(p_instance->drv_inst_idx,p_config->frequency);
    if(period == 0xFFFF)
    {	
        xincx_pwm_freq_valid_range_check(p_cb->clk_src,p_cb->ref_clk,p_config->frequency);
        err_code = XINCX_ERROR_INVALID_PARAM;
        
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                                                __func__,
                                                XINCX_LOG_ERROR_STRING_GET(err_code));
        
        printf("Function: %s,period:%x error code: %s.\r\n",
                                                __func__,period,
                                                XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
    xinc_pwm_configure(p_instance->p_reg,period, p_config->duty_cycle);
    p_cb->period = period;
    p_cb->ocpy =  p_config->duty_cycle;

    #if (XINCX_CHECK(XINCX_PWM0_ENABLED) || XINCX_CHECK(XINCX_PWM1_ENABLED))
    if(p_config->inv_enable && ((p_instance->id == XINC_PWM_ID_0) ||(p_instance->id == XINC_PWM_ID_1) ))
    {
        p_instance->p_reg->PWMCOMPTIME = (p_config->inv_delay << PWM_COMP_TIME_PWMCOMPTIME_Pos)& PWM_COMP_TIME_PWMCOMPTIME_Pos_Msk;
        p_instance->p_reg->PWMCOMPEN = (PWM_COMP_EN_PWMCOMPEN_Enable << PWM_COMP_EN_PWMCOMPEN_Pos) & PWM_COMP_EN_PWMCOMPEN_Msk;
    }
    #endif

    if(p_config->start)
    {
        xinc_pwm_enable(p_instance->p_reg);
    }

    p_cb->state = XINCX_DRV_STATE_INITIALIZED;

    err_code = XINCX_SUCCESS;
    XINCX_LOG_INFO("Function: %s, error code: %s.", __func__, XINCX_LOG_ERROR_STRING_GET(err_code));
    printf("Function: %s, error code: %d.\r\n", __func__, (err_code));
    return err_code;
}


void xincx_pwm_uninit(xincx_pwm_t const * const p_instance)
{
    pwm_control_block_t * p_cb  = &m_cb[p_instance->drv_inst_idx];
    XINCX_ASSERT(p_cb->state != XINCX_DRV_STATE_UNINITIALIZED);

    xinc_pwm_disable(p_instance->p_reg);

    p_cb->state = XINCX_DRV_STATE_UNINITIALIZED;
}


bool xincx_pwm_start(xincx_pwm_t const * const p_instance)
{
    XINCX_ASSERT(m_cb[p_instance->drv_inst_idx].state != XINCX_DRV_STATE_UNINITIALIZED);

    if(m_cb[p_instance->drv_inst_idx].state != XINCX_DRV_STATE_INITIALIZED)
    {
        return false;
    }
    xinc_pwm_enable(p_instance->p_reg);

    return true;
}

bool xincx_pwm_stop(xincx_pwm_t const * const p_instance)
{
    XINCX_ASSERT(m_cb[p_instance->drv_inst_idx].state != XINCX_DRV_STATE_UNINITIALIZED);

    if(m_cb[p_instance->drv_inst_idx].state != XINCX_DRV_STATE_INITIALIZED)
    {
        return false;
    }
    xinc_pwm_disable(p_instance->p_reg);

    return true;
}




xincx_err_t xincx_pwm_freq_update(xincx_pwm_t const * const p_instance,uint32_t new_freq)
{
    xincx_err_t err_code = XINCX_SUCCESS;
    uint16_t period;
    pwm_control_block_t * p_cb  = &m_cb[p_instance->drv_inst_idx];
    period = xincx_pwm_freq_to_period(p_instance->drv_inst_idx,new_freq);
    if(period == 0xFFFF)
    {
        err_code = XINCX_ERROR_INVALID_PARAM;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                                                __func__,
                                                XINCX_LOG_ERROR_STRING_GET(err_code));
        
        printf("Function: %s,period:%x error code: %s.\r\n",
                                                __func__,period,
                                                XINCX_LOG_ERROR_STRING_GET(err_code));
        
        return err_code;
    }
    p_cb->period = period;
    xinc_pwm_configure(p_instance->p_reg,period,p_cb->ocpy);

    return err_code;
}


xincx_err_t xincx_pwm_duty_cycle_update(xincx_pwm_t const * const p_instance,uint8_t new_duty)
{
    xincx_err_t err_code = XINCX_SUCCESS;
    XINCX_ASSERT(new_duty <= 100);
    pwm_control_block_t * p_cb  = &m_cb[p_instance->drv_inst_idx];

    if(new_duty >= 100)
    {
        err_code = XINCX_ERROR_INVALID_PARAM;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                                                __func__,
                                                XINCX_LOG_ERROR_STRING_GET(err_code));
        
        printf("Function: %s,new_duty:%x error code: %s.\r\n",
                                                __func__,new_duty,
                                                XINCX_LOG_ERROR_STRING_GET(err_code));
        
        return err_code;
    }
    p_cb->ocpy = new_duty;
    xinc_pwm_configure(p_instance->p_reg,p_cb->period,new_duty);

    return err_code;

}
xincx_err_t xincx_pwm_freq_duty_cycl_update(xincx_pwm_t const * const p_instance,uint32_t new_freq,uint8_t new_duty)
{
    xincx_err_t err_code = XINCX_SUCCESS;
    XINCX_ASSERT(new_duty <= 100);
    uint16_t period;
    pwm_control_block_t * p_cb  = &m_cb[p_instance->drv_inst_idx];
    period = xincx_pwm_freq_to_period(p_instance->drv_inst_idx,new_freq);
    if((period == 0xFFFF) || (new_duty >= 100))
    {
        err_code = XINCX_ERROR_INVALID_PARAM;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                                                __func__,
                                                XINCX_LOG_ERROR_STRING_GET(err_code));
        
        printf("Function: %s,period:%x error code: %s.\r\n",
                                                __func__,period,
                                                XINCX_LOG_ERROR_STRING_GET(err_code));
        
        return err_code;
    }
    p_cb->period = period;
    p_cb->ocpy = new_duty;
    xinc_pwm_configure(p_instance->p_reg,period,new_duty);

    return err_code;
}



static uint16_t xincx_pwm_freq_to_period(uint8_t inst_idx,uint32_t freq)
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

bool xincx_pwm_freq_valid_range_check(uint8_t clk_src,uint8_t ref_clk, uint32_t set_freq)
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
    XINCX_LOG_INFO("Function: %s, valid_freq_min : %d. valid_freq_max : %d\r\n",
                                                __func__,
                                                valid_freq_min,valid_freq_max);
    printf("freq_valid_range min:%d,max:%d\r\n",(uint32_t)valid_freq_min,(uint32_t)valid_freq_max);
    if((set_freq >= valid_freq_min) && (set_freq <= valid_freq_max))
    {
        return true;
    }
    return false;
}

#endif // XINCX_CHECK(XINCX_PWM_ENABLED)
