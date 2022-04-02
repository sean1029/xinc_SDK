/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#include <xincx.h>

#if XINCX_CHECK(XINCX_SPIM_ENABLED)

#if defined(XINC6206_XXAA)

#if !(XINCX_CHECK(XINCX_SPIM0_ENABLED) || XINCX_CHECK(XINCX_SPIM1_ENABLED))
#error "No enabled SPIM 0/1 instances. Check <xincx_config.h>."
#endif //

#elif defined (XINC628_XXAA)
#if !(XINCX_CHECK(XINCX_SPIM0_ENABLED) || XINCX_CHECK(XINCX_SPIM1_ENABLED) || XINCX_CHECK(XINCX_SPIM2_ENABLED))
#error "No enabled SPIM instances. Check <xincx_config.h>."
#endif

#endif // defined(XINC6206_XXAA)

#if !XINCX_CHECK(XINCX_DMAS_ENABLED)
#error "No enabled DMAS instances. Check <xincx_config.h>."
#endif

#include <xincx_spim.h>
#include <xincx_dmas.h>
#include <hal/xinc_gpio.h>

#define XINCX_LOG_MODULE SPIM
#include <xincx_log.h>

#define SPIMX_LENGTH_VALIDATE(peripheral, drv_inst_idx, rx_len, tx_len) \
    (((drv_inst_idx) == XINCX_CONCAT_3(XINCX_, peripheral, _INST_IDX)) && \
     XINCX_EASYDMA_LENGTH_VALIDATE(peripheral, rx_len, tx_len))

#if XINCX_CHECK(XINCX_SPIM0_ENABLED)
#define SPIM0_LENGTH_VALIDATE(...)  SPIMX_LENGTH_VALIDATE(SPIM0, __VA_ARGS__)
#else
#define SPIM0_LENGTH_VALIDATE(...)  0
#endif

#if XINCX_CHECK(XINCX_SPIM1_ENABLED)
#define SPIM1_LENGTH_VALIDATE(...)  SPIMX_LENGTH_VALIDATE(SPIM1, __VA_ARGS__)
#else
#define SPIM1_LENGTH_VALIDATE(...)  0
#endif

#if XINCX_CHECK(XINCX_SPIM2_ENABLED)
#define SPIM2_LENGTH_VALIDATE(...)  SPIMX_LENGTH_VALIDATE(SPIM2, __VA_ARGS__)
#else
#define SPIM2_LENGTH_VALIDATE(...)  0
#endif

#define SPIM_LENGTH_VALIDATE(drv_inst_idx, rx_len, tx_len)  \
    (SPIM0_LENGTH_VALIDATE(drv_inst_idx, rx_len, tx_len) || \
     SPIM1_LENGTH_VALIDATE(drv_inst_idx, rx_len, tx_len))



// Control block - driver instance local data.
typedef struct
{
    xincx_spim_evt_handler_t handler;
    void *                  p_context;
    xincx_spim_evt_t         evt;  // Keep the struct that is ready for event handler. Less memcpy.
    xincx_drv_state_t        state;
    volatile bool           transfer_in_progress;

#if XINCX_CHECK(XINCX_SPIM_EXTENDED_ENABLED)
    bool                    use_hw_ss;
#endif

    // [no need for 'volatile' attribute for the following members, as they
    //  are not concurrently used in IRQ handlers and main line code]
    bool            ss_active_high;
    uint8_t         sck_pin;
    uint8_t         ss_pin;
    uint8_t         miso_pin;
    uint8_t         mosi_pin;
    #if defined (XINC628_XXAA) && XINCX_CHECK(XINCX_SPIM2_ENABLED)
    uint8_t         d2_pin;
    uint8_t         d3_pin;
    #endif
    uint8_t         orc;

    xinc_dma_ch_t   tx_dma_ch;
    xinc_dma_ch_t   rx_dma_ch;
} spim_control_block_t;

static spim_control_block_t m_cb[XINCX_SPIM_ENABLED_COUNT];


static void xincx_spim_clk_init(xincx_spim_t const * const  p_instance,
                         xincx_spim_config_t const * p_config)
{
    uint32_t    ssi_ctrl_val;
    uint8_t ch = p_instance->id;
    ssi_ctrl_val = p_instance->p_cpr->SSI_CTRL;
    #if (defined (XINC6206_XXAA) || defined (XINC628_XXAA))&& XINCX_CHECK(XINCX_SPIM0_ENABLED)
    if(ch == 0)
    {
        p_instance->p_cpr->RSTCTL_SUBRST_SW = (CPR_RSTCTL_SUBRST_SW_SSI0_RSTN_Enable << CPR_RSTCTL_SUBRST_SW_SSI0_RSTN_Pos)|
                                               (CPR_RSTCTL_SUBRST_SW_SSI0_RSTN_Msk << CPR_RSTCTL_SUBRST_SW_MASK_OFFSET);
        // *SSI_MCLK_CTL_Base = 0x110010;//1��Ƶ			//- spi(x)_mclk = 32Mhz(When TXCO=32Mhz).
        p_instance->p_cpr->SSI0_MCLK_CTL = ((0UL << CPR_SSI_MCLK_CTL_SSI_MCLK_DIV_Pos) | CPR_SSI_MCLK_CTL_SSI_MCLK_DIV_WE) | 
                                           ((CPR_SSI_MCLK_CTL_SSI_MCLK_EN_Enable << CPR_SSI_MCLK_CTL_SSI_MCLK_EN_Pos) | 
                                            CPR_SSI_MCLK_CTL_SSI_MCLK_EN_WE);

        ssi_ctrl_val |= (CPR_SSI_CTRL_SSI0_PROTOCOL_SPI << CPR_SSI_CTRL_SSI0_PROTOCOL_Pos);
            //(0x1000100 << ch);         
        p_instance->p_cpr->CTLAPBCLKEN_GRCTL = ((CPR_CTLAPBCLKEN_GRCTL_SSI0_PCLK_EN_Enable << CPR_CTLAPBCLKEN_GRCTL_SSI0_PCLK_EN_Pos)|
                                        (CPR_CTLAPBCLKEN_GRCTL_SSI0_PCLK_EN_Msk  << CPR_CTLAPBCLKEN_GRCTL_MASK_OFFSET)) << ch; 

    }
    #endif 
    #if (defined (XINC6206_XXAA) || defined (XINC628_XXAA))&& XINCX_CHECK(XINCX_SPIM1_ENABLED)
    if(ch == 1)
    {
        p_instance->p_cpr->RSTCTL_SUBRST_SW = (CPR_RSTCTL_SUBRST_SW_SSI1_RSTN_Enable << CPR_RSTCTL_SUBRST_SW_SSI1_RSTN_Pos)|
                                               (CPR_RSTCTL_SUBRST_SW_SSI1_RSTN_Msk << CPR_RSTCTL_SUBRST_SW_MASK_OFFSET);
        
        p_instance->p_cpr->SSI1_MCLK_CTL = ((0UL << CPR_SSI_MCLK_CTL_SSI_MCLK_DIV_Pos) | CPR_SSI_MCLK_CTL_SSI_MCLK_DIV_WE) | 
                                            ((CPR_SSI_MCLK_CTL_SSI_MCLK_EN_Enable << CPR_SSI_MCLK_CTL_SSI_MCLK_EN_Pos) | 
                                            CPR_SSI_MCLK_CTL_SSI_MCLK_EN_WE);

                    //(0x1000100 << ch);         
        p_instance->p_cpr->CTLAPBCLKEN_GRCTL = ((CPR_CTLAPBCLKEN_GRCTL_SSI0_PCLK_EN_Enable << CPR_CTLAPBCLKEN_GRCTL_SSI0_PCLK_EN_Pos)|
                                        (CPR_CTLAPBCLKEN_GRCTL_SSI0_PCLK_EN_Msk  << CPR_CTLAPBCLKEN_GRCTL_MASK_OFFSET)) << ch; 
        
        ssi_ctrl_val |= (CPR_SSI_CTRL_SSI1_PROTOCOL_SPI << CPR_SSI_CTRL_SSI1_PROTOCOL_Pos) |
                        (CPR_SSI_CTRL_SSI1_MASTER_EN_Master << CPR_SSI_CTRL_SSI1_MASTER_EN_Pos);

    }
    #endif
    #if defined (XINC628_XXAA) && XINCX_CHECK(XINCX_SPIM2_ENABLED)
    if(ch == 2)
    {
        p_instance->p_cpr->RSTCTL_SUBRST_SW = (CPR_RSTCTL_SUBRST_SW_SSI2_RSTN_Enable << CPR_RSTCTL_SUBRST_SW_SSI2_RSTN_Pos)|
                                       (CPR_RSTCTL_SUBRST_SW_SSI2_RSTN_Msk << CPR_RSTCTL_SUBRST_SW_MASK_OFFSET);
 
        p_instance->p_cpr->SSI1_MCLK_CTL = ((0UL << CPR_SSI_MCLK_CTL_SSI2_MCLK_DIV_Pos) | CPR_SSI_MCLK_CTL_SSI2_MCLK_DIV_WE) | 
                                             ((CPR_SSI_MCLK_CTL_SSI2_MCLK_EN_Enable << CPR_SSI_MCLK_CTL_SSI2_MCLK_EN_Pos) |
                                             CPR_SSI_MCLK_CTL_SSI2_MCLK_EN_WE);

        p_instance->p_cpr->CTLAPBCLKEN_GRCTL = ((CPR_CTLAPBCLKEN_GRCTL_SSI2_PCLK_EN_Enable << CPR_CTLAPBCLKEN_GRCTL_SSI2_PCLK_EN_Pos)|
                                        (CPR_CTLAPBCLKEN_GRCTL_SSI2_PCLK_EN_Msk  << CPR_CTLAPBCLKEN_GRCTL_MASK_OFFSET)); 
        
        ssi_ctrl_val |= (CPR_SSI_CTRL_SSI2_PROTOCOL_SPI << CPR_SSI_CTRL_SSI2_PROTOCOL_Pos);
     
     }
    #endif



    p_instance->p_cpr->SSI_CTRL = ssi_ctrl_val;




   
}

xincx_err_t xincx_spim_init(xincx_spim_t  const * const p_instance,
                          xincx_spim_config_t const * p_config,
                          xincx_spim_evt_handler_t    handler,
                          void                     * p_context)
{
    XINCX_ASSERT(p_config);
    spim_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    xincx_err_t err_code = XINCX_SUCCESS;

    if (p_cb->state != XINCX_DRV_STATE_UNINITIALIZED)
    {
        err_code = XINCX_ERROR_INVALID_STATE;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                            __func__,
                            XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }


    XINC_SPIM_Type * p_spim = (XINC_SPIM_Type *)p_instance->p_reg;


    p_cb->handler = handler;
    p_cb->p_context = p_context;

    uint32_t mosi_pin;
    uint32_t miso_pin;

    // Configure pins used by the peripheral:
    // - SCK - output with initial value corresponding with the SPI mode used:
    //   0 - for modes 0 and 1 (CPOL = 0), 1 - for modes 2 and 3 (CPOL = 1);
    //   according to the reference manual guidelines this pin and its input
    //   buffer must always be connected for the SPI to work.
   
    if(p_instance->id != 0)
    {
        if (p_config->mosi_pin != XINCX_SPIM_PIN_NOT_USED)
        {
            mosi_pin = p_config->mosi_pin;

            #if (defined (XINC6206_XXAA) || defined (XINC628_XXAA)) && XINCX_CHECK(XINCX_SPIM1_ENABLED)
            if(p_instance->id == 1UL)
            {
                err_code = xinc_gpio_secfun_config(mosi_pin,XINC_GPIO_PIN_SSI1_TX);
            }
            #endif
            #if defined (XINC628_XXAA) && XINCX_CHECK(XINCX_SPIM2_ENABLED)
            if(p_instance->id == 2UL)
            {
                err_code = xinc_gpio_secfun_config(mosi_pin,XINC_GPIO_PIN_SSI2_D0);
            }
            #endif
           
            if(err_code != XINCX_SUCCESS)
            {
                return err_code;
            }
        }
        else
        {
            mosi_pin = XINC_SPIM_PIN_NOT_CONNECTED;
            err_code = XINCX_ERROR_INVALID_PARAM;
            XINCX_LOG_WARNING("Function: %s, error code: %s.",
                                                __func__,
                                                XINCX_LOG_ERROR_STRING_GET(err_code));
            return err_code;
        }
        // - MISO (optional) - input,
        if (p_config->miso_pin != XINCX_SPIM_PIN_NOT_USED)
        {
            miso_pin = p_config->miso_pin;
            #if (defined (XINC6206_XXAA) || defined (XINC628_XXAA)) && XINCX_CHECK(XINCX_SPIM1_ENABLED)
            if(p_instance->id == 1UL)
            {
                err_code = xinc_gpio_secfun_config(miso_pin,XINC_GPIO_PIN_SSI1_RX);
            }
            #endif
            #if defined (XINC628_XXAA) && XINCX_CHECK(XINCX_SPIM2_ENABLED)
            if(p_instance->id == 2UL)
            {
                err_code = xinc_gpio_secfun_config(mosi_pin,XINC_GPIO_PIN_SSI2_D1);
            }
            #endif
            if(err_code != XINCX_SUCCESS)
            {
                return err_code;
            }
        }
        else
        {
            miso_pin = XINC_SPIM_PIN_NOT_CONNECTED;
            err_code = XINCX_ERROR_INVALID_PARAM;
            XINCX_LOG_WARNING("Function: %s, error code: %s.",
                                                __func__,
                                                XINCX_LOG_ERROR_STRING_GET(err_code));
            return err_code;
        }
        
        if (p_config->ss_pin != XINCX_SPIM_PIN_NOT_USED)
        {
            p_cb->ss_active_high = p_config->ss_active_high;
            #if (defined (XINC6206_XXAA) || defined (XINC628_XXAA)) && XINCX_CHECK(XINCX_SPIM1_ENABLED)
            if(p_instance->id == 1UL)
            {
                err_code = xinc_gpio_secfun_config(p_config->ss_pin,XINC_GPIO_PIN_SSI1_SSN);
            }
            #endif
            #if defined (XINC628_XXAA) && XINCX_CHECK(XINCX_SPIM2_ENABLED)
            if(p_instance->id == 2UL)
            {
                err_code = xinc_gpio_secfun_config(mosi_pin,XINC_GPIO_PIN_SSI2_SSN);
            }
            #endif
            if(err_code != XINCX_SUCCESS)
            {
                return err_code;
            }
        }
        
        if (p_config->sck_pin != XINCX_SPIM_PIN_NOT_USED)
        {
            #if (defined (XINC6206_XXAA) || defined (XINC628_XXAA)) && XINCX_CHECK(XINCX_SPIM1_ENABLED)
            if(p_instance->id == 1UL)
            {
                err_code = xinc_gpio_secfun_config(p_config->sck_pin,XINC_GPIO_PIN_SSI1_CLK);
            }
            #endif
            #if defined (XINC628_XXAA) && XINCX_CHECK(XINCX_SPIM2_ENABLED)
            if(p_instance->id == 2UL)
            {
                err_code = xinc_gpio_secfun_config(mosi_pin,XINC_GPIO_PIN_SSI2_CLK);
            }
            #endif
            
            if(err_code != XINCX_SUCCESS)
            {
                return err_code;
            }
        }
        #if defined (XINC628_XXAA) && XINCX_CHECK(XINCX_SPIM2_ENABLED)
               
        if (p_config->d2_pin != XINCX_SPIM_PIN_NOT_USED)
        {
            if(p_instance->id == 2UL)
            {
                err_code = xinc_gpio_secfun_config(p_config->d2_pin,XINC_GPIO_PIN_SSI2_D2);
            }                   
            if(err_code != XINCX_SUCCESS)
            {
                return err_code;
            }
        }
        if (p_config->d3_pin != XINCX_SPIM_PIN_NOT_USED)
        {
            if(p_instance->id == 2UL)
            {
                err_code = xinc_gpio_secfun_config(p_config->d3_pin,XINC_GPIO_PIN_SSI2_D3);
            }                   
            if(err_code != XINCX_SUCCESS)
            {
                return err_code;
            }
        }
        p_cb->d2_pin = p_config->d2_pin;
        p_cb->d3_pin  = p_config->d3_pin;
        
        #endif                  
    }

    miso_pin = p_config->miso_pin;
    mosi_pin = p_config->mosi_pin;
    p_cb->miso_pin = p_config->miso_pin;
    p_cb->mosi_pin = p_config->mosi_pin;

    p_cb->ss_pin = p_config->ss_pin;
    p_cb->sck_pin = p_config->sck_pin;

    printf("miso_pin:%d\n",miso_pin); 
    printf("mosi_pin:%d\n",mosi_pin); 
    printf("sck_pin:%d\n",p_config->sck_pin);
    printf("ss_pin:%d\n",p_config->ss_pin);		
   
    #if defined (XINC628_XXAA) && XINCX_CHECK(XINCX_SPIM2_ENABLED)
    printf("d2_pin:%d\n",p_config->d2_pin);
    printf("d3_pin:%d\n",p_config->d3_pin);	
    #endif

	
    // uint32_t    val;
    
    // volatile uint32_t *Spim0CtlBaseAddr = (uint32_t*)&(p_instance->p_cpr->SSI0_MCLK_CTL);
    // Spim0CtlBaseAddr+=ch;
    
    // *Spim0CtlBaseAddr = 0x110010;//1��Ƶ			//- spi(x)_mclk = 32Mhz(When TXCO=32Mhz).
    // p_instance->p_cpr->CTLAPBCLKEN_GRCTL = (0x1000100 << ch);
        
    // val = p_instance->p_cpr->SSI_CTRL;
    // val |= (ch==0)? 0x01: 0x30;
    // p_instance->p_cpr->SSI_CTRL = val;

    xincx_spim_clk_init(p_instance,p_config);
    
     uint8_t ch = p_instance->id;
    // DMA 通道和外设使用是固定绑定关系的，因此此处不能做修改，对应中断处理里也就固定其检测值
    if(ch == 0)
    {
        p_cb->rx_dma_ch = DMAS_CH_RCV_SSI0;
        p_cb->tx_dma_ch = DMAS_CH_SEND_SSI0;
    }
    
    else if(ch == 1)
    {
        p_cb->rx_dma_ch = DMAS_CH_RCV_SSI1;
        p_cb->tx_dma_ch = DMAS_CH_SEND_SSI1;
    }
    #if defined (XINC628_XXAA) && XINCX_CHECK(XINCX_SPIM2_ENABLED)
    else if(ch == 2)
    {
        p_cb->rx_dma_ch = DMAS_CH_RCV_SSI2;
        p_cb->tx_dma_ch = DMAS_CH_RCV_SSI2;
    }	
    #endif
		
    xinc_spim_disable(p_spim);
    printf("p_spim:%p\n",p_spim); 

    printf("CTRL0:%p,0x%x\n",&(p_spim->CTRL0),p_spim->CTRL0); 
    printf("xincx_spim_init ch :%d,mode:%d\n",ch,p_config->mode); 


    xinc_spim_configure(p_spim, p_config->mode, p_config->bit_order);

    printf("EN:%p\n",&(p_spim->EN)); 

    p_spim->SE = SSI_SSI_SE_SS0_Select << SSI_SSI_SE_SS0_Pos;
    printf("SE:%p\n",&(p_spim->SE)); 

    xinc_spim_frequency_set(p_spim,p_config->frequency);

    printf("BAUD:%p\n",&(p_spim->BAUD)); 

    p_spim->RXFLT = 0x00;
    p_spim->TXFLT = 0x00;
    printf("RXFLT:%p\n",&(p_spim->RXFLT)); 
    printf("TXFLT:%p\n",&(p_spim->TXFLT)); 
    printf("frequency:%d\n",p_config->frequency); 


    if (p_cb->handler)
    {
        xinc_spim_int_enable(p_spim,XINC_SPIM_INT_TXEIE_MASK);
        #if (defined (XINC6206_XXAA) || defined (XINC628_XXAA)) && (XINCX_CHECK(XINCX_SPIM0_ENABLED) || XINCX_CHECK(XINCX_SPIM1_ENABLED))
        if((ch == 0 )||( ch == 1))
        {
            XINCX_IRQ_ENABLE((IRQn_Type)(SPI0_IRQn + ch));
        }
        #endif
        #if defined (XINC628_XXAA) && XINCX_CHECK(XINCX_SPIM2_ENABLED)
        if(ch == 2)
        {
            XINCX_IRQ_ENABLE((IRQn_Type)(SSI2_IRQn));
        }	
        #endif

    }

    p_cb->transfer_in_progress = false;
    p_cb->state = XINCX_DRV_STATE_INITIALIZED;

    err_code = XINCX_SUCCESS;
    XINCX_LOG_INFO("Function: %s, error code: %s.", __func__, XINCX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}

void xincx_spim_uninit(xincx_spim_t const * const p_instance)
{
    spim_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    XINCX_ASSERT(p_cb->state != XINCX_DRV_STATE_UNINITIALIZED);

    if (p_cb->handler)
    {      
        #if (defined (XINC6206_XXAA) || defined (XINC628_XXAA)) && (XINCX_CHECK(XINCX_SPIM0_ENABLED) || XINCX_CHECK(XINCX_SPIM1_ENABLED))
        if((p_instance->id == 0 )||( p_instance->id == 1))
        {
            XINCX_IRQ_DISABLE((IRQn_Type)(SPI0_IRQn + p_instance->id));
        }
        #endif
        #if defined (XINC628_XXAA)  && XINCX_CHECK(XINCX_SPIM2_ENABLED)
        if(p_instance->id == 2)
        {
            XINCX_IRQ_DISABLE((IRQn_Type)(SSI2_IRQn));
        }	
        #endif
    }

    XINC_SPIM_Type * p_spim = (XINC_SPIM_Type *)p_instance->p_reg;
    if (p_cb->handler)
    {
        if (p_cb->transfer_in_progress)
        {
            // Ensure that SPI is not performing any transfer.
          //  while (!xinc_spim_event_check(p_spim, XINC_SPIM_EVENT_STOPPED))
            {}
            p_cb->transfer_in_progress = false;
        }
    }

    if (p_cb->miso_pin != XINCX_SPIM_PIN_NOT_USED)
    {

    }

    xinc_spim_int_disable(p_spim,XINC_SPIM_INT_DISALL_MASK);


#ifdef USE_WORKAROUND_FOR_ANOMALY_195
    if (p_spim == XINC_SPIM3)
    {
        *(volatile uint32_t *)0x4002F004 = 1;
    }
#endif

    p_cb->state = XINCX_DRV_STATE_UNINITIALIZED;
}


static void finish_transfer(spim_control_block_t * p_cb)
{
    // If Slave Select signal is used, this is the time to deactivate it.
    if (p_cb->ss_pin != XINCX_SPIM_PIN_NOT_USED)
    {
#if XINCX_CHECK(XINCX_SPIM_EXTENDED_ENABLED)
        if (!p_cb->use_hw_ss)
#endif
        {
            if (p_cb->ss_active_high)
            {
             //   xinc_gpio_pin_clear(p_cb->ss_pin);
            }
            else
            {
             //   xinc_gpio_pin_set(p_cb->ss_pin);
            }
        }
    }

    // By clearing this flag before calling the handler we allow subsequent
    // transfers to be started directly from the handler function.
    p_cb->transfer_in_progress = false;

    p_cb->evt.type = XINCX_SPIM_EVENT_DONE;
    p_cb->handler(&p_cb->evt, p_cb->p_context);
}


static xincx_err_t spim_xfer(XINC_SPIM_Type               * p_spim,
                            spim_control_block_t        * p_cb,
                            xincx_spim_xfer_desc_t  * p_xfer_desc,
                            uint32_t                      flags)
{
    xincx_err_t err_code;
    // EasyDMA requires that transfer buffers are placed in Data RAM region;
    // signal error if they are not.
    if ((p_xfer_desc->p_tx_buffer != NULL && !xincx_is_in_ram(p_xfer_desc->p_tx_buffer)) ||
        (p_xfer_desc->p_rx_buffer != NULL && !xincx_is_in_ram(p_xfer_desc->p_rx_buffer)))
    {
        p_cb->transfer_in_progress = false;
        err_code = XINCX_ERROR_INVALID_ADDR;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         XINCX_LOG_ERROR_STRING_GET(err_code));
        printf("spim_xfer error:%d\r\n",err_code);
        return err_code;
    }
		


    xinc_spim_disable(p_spim);
    p_spim->DMAS = (SSI_SSI_DMAS_RDMAE_Enable << SSI_SSI_DMAS_RDMAE_Pos) | (SSI_SSI_DMAS_TDMAE_Enable << SSI_SSI_DMAS_TDMAE_Pos);
    p_spim->DMATDL = 0x04;
    p_spim->DMARDL = 0x04;
    

            //- TX Channel
    xincx_dmas_ch_set_t set;

    set.ch = p_cb->tx_dma_ch;
    set.src_addr = (uint32_t)p_xfer_desc->p_tx_buffer;
    set.dst_addr = (uint32_t)&(p_spim->DATA);
    set.ctl0 = p_xfer_desc->tx_length;
    set.ctl1 = (2 << 8);
    xincx_dmas_ch_param_set(set);


        //- RX Channel

    set.ch = p_cb->rx_dma_ch;
    set.src_addr = (uint32_t)&(p_spim->DATA);
    set.dst_addr = (uint32_t)p_xfer_desc->p_rx_buffer;
    set.ctl0 = p_xfer_desc->rx_length;
    set.ctl1 = (2 << 8);
    xincx_dmas_ch_param_set(set);


#if XINCX_CHECK(XINCX_SPIM3_XINC52840_ANOMALY_198_WORKAROUND_ENABLED)
    if (p_spim == XINC_SPIM3)
    {
        anomaly_198_enable(p_xfer_desc->p_tx_buffer, p_xfer_desc->tx_length);
    }
#endif



    if (!p_cb->handler)
    {
        if (p_cb->ss_pin != XINCX_SPIM_PIN_NOT_USED)
        {
#if XINCX_CHECK(XINCX_SPIM_EXTENDED_ENABLED)
            if (!p_cb->use_hw_ss)
#endif
            {
                if (p_cb->ss_active_high)
                {
                  //  xinc_gpio_pin_clear(p_cb->ss_pin);
                }
                else
                {
                  //  xinc_gpio_pin_set(p_cb->ss_pin);
                }
            }
        }
    }
    else
    {
        xincx_dmas_ch_enable(p_cb->rx_dma_ch);
        xincx_dmas_ch_enable(p_cb->tx_dma_ch);
        xinc_spim_enable(p_spim);
			
    }
    err_code = XINCX_SUCCESS;
    XINCX_LOG_INFO("Function: %s, error code: %s.", __func__, XINCX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}

xincx_err_t xincx_spim_xfer(xincx_spim_t     const * const p_instance,
                          xincx_spim_xfer_desc_t  * p_xfer_desc,
                          uint32_t                      flags)
{
    spim_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    XINCX_ASSERT(p_cb->state != XINCX_DRV_STATE_UNINITIALIZED);
    XINCX_ASSERT(p_xfer_desc->p_tx_buffer != NULL || p_xfer_desc->tx_length == 0);
    XINCX_ASSERT(p_xfer_desc->p_rx_buffer != NULL || p_xfer_desc->rx_length == 0);
    XINCX_ASSERT(SPIM_LENGTH_VALIDATE(p_instance->drv_inst_idx,
                                     p_xfer_desc->rx_length,
                                     p_xfer_desc->tx_length));

    xincx_err_t err_code = XINCX_SUCCESS;

    if (p_cb->transfer_in_progress)
    {
        err_code = XINCX_ERROR_BUSY;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
    else
    {
        if (p_cb->handler && !(flags & (XINCX_SPIM_FLAG_REPEATED_XFER |
                                        XINCX_SPIM_FLAG_NO_XFER_EVT_HANDLER)))
        {
            p_cb->transfer_in_progress = true;
        }
    }

    p_cb->evt.xfer_desc = *p_xfer_desc;

    if (p_cb->ss_pin != XINCX_SPIM_PIN_NOT_USED)
    {
#if XINCX_CHECK(XINCX_SPIM_EXTENDED_ENABLED)
        if (!p_cb->use_hw_ss)
#endif
        {
            if (p_cb->ss_active_high)
            {
              //  xinc_gpio_pin_set(p_cb->ss_pin);
            }
            else
            {
              //  xinc_gpio_pin_clear(p_cb->ss_pin);
            }
        }
    }

    return spim_xfer(p_instance->p_reg, p_cb,  p_xfer_desc, flags);
}

void xincx_spim_abort(xincx_spim_t const * p_instance)
{
    spim_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    XINCX_ASSERT(p_cb->state != XINCX_DRV_STATE_UNINITIALIZED);

    p_cb->transfer_in_progress = false;
}



static void irq_handler(XINC_SPIM_Type * p_spim, spim_control_block_t * p_cb)
{
	uint32_t intSta;
	uint32_t rowIntSta;
	uint32_t state;
    intSta = p_spim->IS;
    rowIntSta = p_spim->RIS;
    state = p_spim->STS;
    
    uint32_t mask = (0x01 << p_cb->rx_dma_ch) | (0x01 << p_cb->tx_dma_ch);
    
    if(intSta && (state & 0x4))
	{
        do{
           // __read_hw_reg32(DMAS_INT_RAW , iWK);
            rowIntSta = xincx_dmas_int_raw_stat_get();
        }while((rowIntSta & mask) != mask);
				
       // __write_hw_reg32(DMAS_INT_RAW, 0x404);
      //  __write_hw_reg32(DMAS_CLR , 10);
        xincx_dmas_int_sta_clear(p_cb->rx_dma_ch);
        xincx_dmas_int_sta_clear(p_cb->tx_dma_ch);
        
        XINCX_ASSERT(p_cb->handler);
        XINCX_LOG_DEBUG("Event: XINC_SPIM_EVENT_END.");
        xinc_spim_disable(p_spim);
        finish_transfer(p_cb);
    
    }
        
    
}

#if XINCX_CHECK(XINCX_SPIM0_ENABLED)
void xincx_spim_0_irq_handler(void)
{
    irq_handler(XINC_SPIM0, &m_cb[XINCX_SPIM0_INST_IDX]);
}
void SPI0_Handler()
{
    xincx_spim_0_irq_handler();
}
#endif

#if XINCX_CHECK(XINCX_SPIM1_ENABLED)
void xincx_spim_1_irq_handler(void)
{
    irq_handler(XINC_SPIM1, &m_cb[XINCX_SPIM1_INST_IDX]);
}
void SPI1_Handler()
{
    xincx_spim_1_irq_handler();
}

#endif





#endif // XINCX_CHECK(XINCX_SPIM_ENABLED)
