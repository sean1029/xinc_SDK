/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#include <xincx.h>

#if XINCX_CHECK(XINCX_GPIO_ENABLED)

#include <xincx_gpio.h>
#include "xinc_bitmask.h"
#include <string.h>


#define XINCX_LOG_MODULE GPIO
#include <xincx_log.h>

#if (GPIO_COUNT == 1)
#define MAX_PIN_NUMBER 32
#elif (GPIO_COUNT == 2)
#define MAX_PIN_NUMBER (32 + P1_PIN_NUM)
#else
#error "Not supported."
#endif

#define FORBIDDEN_HANDLER_ADDRESS ((xincx_gpio_evt_handler_t)UINT32_MAX)
#define PIN_NOT_USED              (-1)
#define PIN_USED                  (-2)
#define NO_HANDLERS_ID               (-1)
#define POLARITY_FIELD_POS        (6)
#define POLARITY_FIELD_MASK       (0xC0)


/*lint -save -e571*/ /* Suppress "Warning 571: Suspicious cast" */
typedef struct
{
    xincx_gpio_evt_handler_t handlers[MAX_PIN_NUMBER];
    int8_t                    pin_assignments[MAX_PIN_NUMBER];
    uint8_t                   configured_pins[((MAX_PIN_NUMBER)+7) / 8];
    xincx_drv_state_t          state;
} gpiote_control_block_t;

static gpiote_control_block_t m_cb;

__STATIC_INLINE bool pin_in_use(uint32_t pin)
{
    return (m_cb.pin_assignments[pin] != PIN_NOT_USED);
}


__STATIC_INLINE bool pin_in_use_by_gpio_handler(uint32_t pin)
{
    return (m_cb.pin_assignments[pin] >= 0);
}


__STATIC_INLINE void pin_in_use_by_handler_set(uint32_t                  pin,
                                        uint32_t                  handler_id,
                                        xincx_gpio_evt_handler_t handler)
{
    m_cb.pin_assignments[pin] = handler_id;
    m_cb.handlers[handler_id] = handler;
}


__STATIC_INLINE void pin_in_use_set(uint32_t pin)
{
    printf("pin_in_use_set:%d\r\n",pin);
    m_cb.pin_assignments[pin] = PIN_USED;
}


__STATIC_INLINE void pin_in_use_clear(uint32_t pin)
{
    m_cb.pin_assignments[pin] = PIN_NOT_USED;
}


__STATIC_INLINE void pin_configured_set(uint32_t pin)
{
    xinc_bitmask_bit_set(pin, m_cb.configured_pins);
}

__STATIC_INLINE void pin_configured_clear(uint32_t pin)
{
    xinc_bitmask_bit_clear(pin, m_cb.configured_pins);
}

__STATIC_INLINE bool pin_configured_check(uint32_t pin)
{
    return 0 != xinc_bitmask_bit_is_set(pin, m_cb.configured_pins);
}

__STATIC_INLINE int8_t pin_handler_id_get(uint32_t pin)
{
    return m_cb.pin_assignments[pin];
}


__STATIC_INLINE xincx_gpio_evt_handler_t pin_handler_get(uint32_t channel)
{
    return m_cb.handlers[channel];
}


static int8_t pin_handler_use_alloc(uint32_t pin, xincx_gpio_evt_handler_t handler)
{
    int8_t   handler_id = NO_HANDLERS_ID;
    uint32_t i;

    uint32_t start_idx = 0;
    uint32_t end_idx   = MAX_PIN_NUMBER;

    // critical section

    for (i = start_idx; i < end_idx; i++)
    {
        if (m_cb.handlers[i] == FORBIDDEN_HANDLER_ADDRESS)
        {
            pin_in_use_by_handler_set(pin, i, handler);
            handler_id = i;
            break;
        }
    }
  //  printf("pin_handler_use_alloc :%d\r\n",handler_id);
    // critical section
    return handler_id;
}


static void pin_handler_free(uint8_t handler_id)
{
    m_cb.handlers[handler_id] = FORBIDDEN_HANDLER_ADDRESS;
}

#include "xinc_delay.h"
xincx_err_t xincx_gpio_init(void)
{
    xincx_err_t err_code;

    if (m_cb.state != XINCX_DRV_STATE_UNINITIALIZED)
    {
        err_code = XINCX_ERROR_INVALID_STATE;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         XINCX_LOG_ERROR_STRING_GET(err_code));
        printf("Function: %s,error code:0x%x\r\n ", __func__,err_code);
        return err_code;
    }
    XINC_CPR_CTL_Type * p_cpr            = XINC_CPR;
   // CPR_RSTCTL_CTLAPB_SW_GPIO_RSTN_Enable
    p_cpr->RSTCTL_CTLAPB_SW = (CPR_RSTCTL_CTLAPB_SW_GPIO_RSTN_Enable << CPR_RSTCTL_CTLAPB_SW_GPIO_RSTN_Pos) |
                               (CPR_RSTCTL_CTLAPB_SW_GPIO_RSTN_Msk << CPR_RSTCTL_CTLAPB_SW_MASK_OFFSET);
    
    p_cpr->RSTCTL_CTLAPB_SW = (CPR_RSTCTL_CTLAPB_SW_GPIO_RSTN_Disable << CPR_RSTCTL_CTLAPB_SW_GPIO_RSTN_Pos) |
                               (CPR_RSTCTL_CTLAPB_SW_GPIO_RSTN_Msk << CPR_RSTCTL_CTLAPB_SW_MASK_OFFSET);
    
    xinc_delay_ms(100);
  
    printf("RSTCTL_CTLAPB_SW: 0x%x\r\n ",p_cpr->RSTCTL_CTLAPB_SW);
    p_cpr->CTLAPBCLKEN_GRCTL =  (CPR_CTLAPBCLKEN_GRCTL_GPIO_PCLK_EN_Enable <<  CPR_CTLAPBCLKEN_GRCTL_GPIO_PCLK_EN_Pos) | 
                                (CPR_CTLAPBCLKEN_GRCTL_GPIO_PCLK_EN_Msk << CPR_CTLAPBCLKEN_GRCTL_MASK_OFFSET);
    
    p_cpr->OTHERCLKEN_GRCTL =   (CPR_OTHERCLKEN_GRCTL_GPIO_CLK_EN_Enable <<  CPR_OTHERCLKEN_GRCTL_GPIO_CLK_EN_Pos) | 
                                (CPR_OTHERCLKEN_GRCTL_GPIO_CLK_EN_Msk << CPR_OTHERCLKEN_GRCTL_MASK_OFFSET);

    uint8_t i;

    for (i = 0; i < MAX_PIN_NUMBER; i++)
    {
        if (xinc_gpio_pin_present_check(i))
        {
            pin_in_use_clear(i);
        }
        pin_handler_free(i);
    }
 
    memset(m_cb.configured_pins, 0, sizeof(m_cb.configured_pins));

    NVIC_EnableIRQ(GPIO_IRQn);
    NVIC_EnableIRQ(PendSV_IRQn);

    m_cb.state = XINCX_DRV_STATE_INITIALIZED;

    err_code = XINCX_SUCCESS;
    XINCX_LOG_INFO("Function: %s, error code: %s.", __func__, XINCX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}


bool xincx_gpio_is_init(void)
{
    return (m_cb.state != XINCX_DRV_STATE_UNINITIALIZED) ? true : false;
}


void xincx_gpio_uninit(void)
{
    XINCX_ASSERT(m_cb.state != XINCX_DRV_STATE_UNINITIALIZED);

    uint32_t i;

    for (i = 0; i < MAX_PIN_NUMBER; i++)
    {   
        if (xinc_gpio_pin_present_check(i))
        {
           
            if (pin_in_use_by_gpio_handler(i))
            {
                /* Disable gpiote_in is having the same effect on out pin as gpiote_out_uninit on
                 * so it can be called on all pins used by GPIOTE.
                 */
                xincx_gpio_in_uninit(i);
            }
        }
    }
    m_cb.state = XINCX_DRV_STATE_UNINITIALIZED;
    XINCX_LOG_INFO("Uninitialized.");
}


xincx_err_t xincx_gpio_out_init(xincx_gpio_pin_t                pin,
                                xincx_gpio_out_config_t const * p_config)
{
    XINCX_ASSERT(xinc_gpio_pin_present_check(pin));
    XINCX_ASSERT(m_cb.state == XINCX_DRV_STATE_INITIALIZED);
    XINCX_ASSERT(p_config);

    xincx_err_t err_code = XINCX_SUCCESS;

    if (pin_in_use(pin))
    {
        err_code = XINCX_ERROR_INVALID_STATE;
    }
    else
    {
           
        pin_in_use_set(pin);
        
        if (err_code == XINCX_SUCCESS)
        {
            xinc_gpio_cfg_output(pin);
            pin_configured_set(pin);
            if (p_config->init_state == XINC_GPIO_INITIAL_VALUE_HIGH)
            {
                xinc_gpio_pin_set(pin);
            }
            else
            {
                xinc_gpio_pin_clear(pin);
            }     
            
        }
    }

    XINCX_LOG_INFO("Function: %s, error code: %s.", __func__, XINCX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}


void xincx_gpio_out_uninit(xincx_gpio_pin_t pin)
{
    XINCX_ASSERT(xinc_gpio_pin_present_check(pin));
    XINCX_ASSERT(pin_in_use(pin));

    pin_in_use_clear(pin);

    if (pin_configured_check(pin))
    {
        xinc_gpio_cfg_default(pin);
        pin_configured_clear(pin);
    }
}


void xincx_gpio_out_set(xincx_gpio_pin_t pin)
{
    XINCX_ASSERT(xinc_gpio_pin_present_check(pin));
    XINCX_ASSERT(pin_in_use(pin));

    xinc_gpio_pin_set(pin);
}


void xincx_gpio_out_clear(xincx_gpio_pin_t pin)
{
    XINCX_ASSERT(xinc_gpio_pin_present_check(pin));
    XINCX_ASSERT(pin_in_use(pin));

    xinc_gpio_pin_clear(pin);
}


void xincx_gpio_out_toggle(xincx_gpio_pin_t pin)
{
    XINCX_ASSERT(xinc_gpio_pin_present_check(pin));
    XINCX_ASSERT(pin_in_use(pin));

    xinc_gpio_pin_toggle(pin);
}


xincx_err_t xincx_gpio_in_init(xincx_gpio_pin_t               pin,
                               xincx_gpio_in_config_t const * p_config,
                               xincx_gpio_evt_handler_t       evt_handler)
{
    XINCX_ASSERT(xinc_gpio_pin_present_check(pin));
    xincx_err_t err_code = XINCX_SUCCESS;
    if (pin_in_use_by_gpio_handler(pin))
    {
        err_code = XINCX_ERROR_INVALID_STATE;
    }
    else
    {
        
        int8_t handler_id = pin_handler_use_alloc(pin, evt_handler);
        if (handler_id != NO_HANDLERS_ID)
        {     
            xinc_gpio_cfg_input(pin, p_config->input_config);

            pin_configured_set(pin);  
             
            
        }
        else
        {
            err_code = XINCX_ERROR_NO_MEM;
        }

    }

    XINCX_LOG_INFO("Function: %s, error code: %s.", __func__, XINCX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}



void xincx_gpio_in_uninit(xincx_gpio_pin_t pin)
{
    XINCX_ASSERT(xinc_gpio_pin_present_check(pin));
    XINCX_ASSERT(pin_in_use_by_gpio_handler(pin));
   
    if (pin_configured_check(pin))
    {
        xinc_gpio_cfg_default(pin);
        pin_configured_clear(pin);
    }
    {
        pin_handler_free((uint8_t)pin_handler_id_get(pin));
    }
    pin_in_use_clear(pin);
}


bool xincx_gpio_in_is_set(xincx_gpio_pin_t pin)
{
    XINCX_ASSERT(xinc_gpio_pin_present_check(pin));
    return xinc_gpio_pin_read(pin) ? true : false;
}


ret_code_t xinc_gpio_secfun_config(uint32_t pin,xinc_gpio_pin_fun_sel_t fun)
{
    ret_code_t err_code = XINC_SUCCESS; 

    if (pin_in_use(pin))
    {
        err_code = XINCX_ERROR_INVALID_STATE;
    }
    else
    {
        if(pin > XINC_GPIO_MAX)
        {
            err_code = XINC_ERROR_INVALID_PARAM;
        }
        else if((XINC_GPIO_PIN_PWM2 == fun) && (XINC_GPIO_0 == pin))
        {
            xinc_gpio_mux_ctl(pin,2);		     
        }
        else if((XINC_GPIO_PIN_PWM3 == fun) && (XINC_GPIO_1 == pin))
        {
            xinc_gpio_mux_ctl(pin,2);        
        }
        else if((XINC_GPIO_PIN_PWM4 == fun) && (XINC_GPIO_12 == pin))
        {
            xinc_gpio_mux_ctl(pin,3);
        }
        else if((XINC_GPIO_PIN_PWM5 == fun) && (XINC_GPIO_13 == pin))
        {                       
            xinc_gpio_mux_ctl(pin,3);
        }
        #if defined (XC66XX_M4)
        else if((XINC_GPIO_PIN_SSI2_CLK == fun) && (XINC_GPIO_14 == pin))
        {                       
            xinc_gpio_mux_ctl(pin,2);		
        }
        else if((XINC_GPIO_PIN_SSI2_SSN == fun) && (XINC_GPIO_15 == pin))
        {                       
            xinc_gpio_mux_ctl(pin,2);
        }
         else if((XINC_GPIO_PIN_SSI2_D0 == fun) && (XINC_GPIO_16 == pin))
        {                       
            xinc_gpio_mux_ctl(pin,2);
        }
        else if((XINC_GPIO_PIN_SSI2_D1 == fun) && (XINC_GPIO_17 == pin))
        {                       
            xinc_gpio_mux_ctl(pin,2);
        }
        else if((XINC_GPIO_PIN_SSI2_D2 == fun) && (XINC_GPIO_26 == pin))
        {                       
            xinc_gpio_mux_ctl(pin,2);
        }
        else if((XINC_GPIO_PIN_SSI2_D3 == fun) && (XINC_GPIO_27 == pin))
        {                       
            xinc_gpio_mux_ctl(pin,2);	
        }
        else if((XINC_GPIO_PIN_UART1_TX == fun) && (XINC_GPIO_31 == pin))
        {                       
            xinc_gpio_mux_ctl(pin,1);		
        }
        else if((XINC_GPIO_PIN_UART2_TX == fun) && (XINC_GPIO_32 == pin))
        {                       
            xinc_gpio_mux_ctl(pin,1);		
        }
        else if((XINC_GPIO_PIN_AUDIO_MIC_P == fun) && (XINC_GPIO_18 == pin))
        {                       
            xinc_gpio_mux_ctl(pin,0);
            xinc_gpio_fun_sel(pin,XINC_GPIO_PIN_GPIODx);
            xinc_gpio_inter_sel(pin,XINC_GPIO_PIN_INPUT_NOINT);
            xinc_gpio_pin_dir_set(pin,XINC_GPIO_PIN_DIR_INPUT);
            xinc_gpio_pull_sel(pin,XINC_GPIO_PIN_NOPULL);
                    
        }
        else if((XINC_GPIO_PIN_AUDIO_MIC_N == fun) && (XINC_GPIO_0 == pin))
        {                       
            xinc_gpio_mux_ctl(pin,0);
            xinc_gpio_fun_sel(pin,XINC_GPIO_PIN_GPIODx);
            xinc_gpio_inter_sel(pin,XINC_GPIO_PIN_INPUT_NOINT);
            xinc_gpio_pin_dir_set(pin,XINC_GPIO_PIN_DIR_INPUT);
            xinc_gpio_pull_sel(pin,XINC_GPIO_PIN_NOPULL);		
        }
        else if((XINC_GPIO_PIN_AUDIO_MIC_BIAS == fun) && (XINC_GPIO_19 == pin))
        {                       
            xinc_gpio_mux_ctl(pin,0);
            xinc_gpio_fun_sel(pin,XINC_GPIO_PIN_GPIODx);
            xinc_gpio_inter_sel(pin,XINC_GPIO_PIN_INPUT_NOINT);
            xinc_gpio_pin_dir_set(pin,XINC_GPIO_PIN_DIR_INPUT);
            xinc_gpio_pull_sel(pin,XINC_GPIO_PIN_NOPULL);	
        }
        #endif
        switch(pin)
        {
            case XINC_GPIO_11:
            case XINC_GPIO_12:
            case XINC_GPIO_13:	
            { 
                if(fun > XINC_GPIO_PIN_PWM1_INV)
                {
                    err_code = XINCX_ERROR_INVALID_PARAM;
                }else
                {
                    xinc_gpio_mux_ctl(pin,1);
                    xinc_gpio_fun_sel(pin,fun);
                }                
            }break;
            
            default:
            {                
                if(fun > XINC_GPIO_PIN_PWM1_INV)
                {
                    err_code = XINCX_ERROR_INVALID_PARAM;
                }else
                {
                   
                    xinc_gpio_mux_ctl(pin,0);
                    xinc_gpio_fun_sel(pin,fun);
                }
            }
            break;	
        
        }	
    }

    if(err_code == XINC_SUCCESS)
    {
        pin_in_use_set(pin);
    }

    return err_code;
}

void xincx_gpio_irq_handler(void)
{
    uint32_t            status[GPIO_COUNT]    = {0};
    uint32_t            i;
    uint32_t            mask  = (uint32_t)0x01;
    
    XINC_GPIO_Type * gpio_regs[GPIO_COUNT] = GPIO_REG_LIST;

    status[0] = gpio_regs[0]->INTR_STATUS_C0[0];

    gpio_regs[0]->INTR_CLR[0] = status[0];
		
        
    /* collect status of all GPIO pin events. Processing is done once all are collected and cleared.*/
        /* Process pin events. */
    if (status[0])
    {
        for (i = 0; i < MAX_PIN_NUMBER; i++)
        {
            if (mask & status[0])
            {
                //	printf("xincx_gpio_irq_handler:%x,i:%d\n",status,i);
                xincx_gpio_evt_handler_t handler =
                pin_handler_get((uint32_t)pin_handler_id_get(i));
                if (handler)
                {
                    handler(i, XINC_GPIO_POLARITY_LOTOHI);
                }
              //  status[0] |= mask;
            }
            mask <<= 1;
            /* Incrementing to next event, utilizing the fact that events are grouped together
            * in ascending order. */
        }
    }
		
}

// void GPIO_Handler(void)
//{
//    xincx_gpio_irq_handler();
//}

/*lint -restore*/
#endif // XINCX_CHECK(XINCX_GPIO_ENABLED)
