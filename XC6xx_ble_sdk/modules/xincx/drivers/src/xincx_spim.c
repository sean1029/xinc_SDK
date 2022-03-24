/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#include <xincx.h>
#include "bsp_register_macro.h"
#include "bsp_clk.h"
#if XINCX_CHECK(XINCX_SPIM_ENABLED)

#if !(XINCX_CHECK(XINCX_SPIM0_ENABLED) || XINCX_CHECK(XINCX_SPIM1_ENABLED) || \
      XINCX_CHECK(XINCX_SPIM2_ENABLED) || XINCX_CHECK(XINCX_SPIM3_ENABLED))
#error "No enabled SPIM instances. Check <xincx_config.h>."
#endif

#if XINCX_CHECK(XINCX_SPIM_EXTENDED_ENABLED) && !XINCX_CHECK(XINCX_SPIM3_ENABLED)
#error "Extended options are available only in SPIM3 on the nRF52840 SoC."
#endif

#include <xincx_spim.h>
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



#define SPIM_LENGTH_VALIDATE(drv_inst_idx, rx_len, tx_len)  \
    (SPIM0_LENGTH_VALIDATE(drv_inst_idx, rx_len, tx_len) || \
     SPIM1_LENGTH_VALIDATE(drv_inst_idx, rx_len, tx_len))

#if defined(XINC6206_XXAA) && (XINCX_CHECK(XINCX_SPIM3_ENABLED))
// Enable workaround for nRF52840 anomaly 195 (SPIM3 continues to draw current after disable).
#define USE_WORKAROUND_FOR_ANOMALY_195
#endif

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
    uint8_t         ss_pin;
    uint8_t         miso_pin;
    uint8_t         orc;

    uint8_t tx_dma_ch;
    uint8_t rx_dma_ch;
} spim_control_block_t;

static spim_control_block_t m_cb[XINCX_SPIM_ENABLED_COUNT];


static void xincx_spim_clk_init(xincx_spim_t const * const  p_instance,
                         xincx_spim_config_t const * p_config)
{
    uint32_t    val;
    uint8_t ch = p_instance->id;
    val = p_instance->p_cpr->SSI_CTRL;
    if(ch == 0)
    {
        // *SSI_MCLK_CTL_Base = 0x110010;//1��Ƶ			//- spi(x)_mclk = 32Mhz(When TXCO=32Mhz).
        p_instance->p_cpr->SSI0_MCLK_CTL = ((0UL << CPR_SSI_MCLK_CTL_SSI_MCLK_DIV_Pos) | CPR_SSI_MCLK_CTL_SSI_MCLK_DIV_WE) | 
                                             ((CPR_SSI_MCLK_CTL_SSI_MCLK_EN_Enable << CPR_SSI_MCLK_CTL_SSI_MCLK_EN_Pos) | CPR_SSI_MCLK_CTL_SSI_MCLK_EN_WE);

        val |= (CPR_SSI_CTRL_SSI0_PROTOCOL_SPI << CPR_SSI_CTRL_SSI0_PROTOCOL_Pos);

    }else
    {
        p_instance->p_cpr->SSI1_MCLK_CTL = ((0UL << CPR_SSI_MCLK_CTL_SSI_MCLK_DIV_Pos) | CPR_SSI_MCLK_CTL_SSI_MCLK_DIV_WE) | 
                                            ((CPR_SSI_MCLK_CTL_SSI_MCLK_EN_Enable << CPR_SSI_MCLK_CTL_SSI_MCLK_EN_Pos) | CPR_SSI_MCLK_CTL_SSI_MCLK_EN_WE);

        val |= (CPR_SSI_CTRL_SSI1_PROTOCOL_SPI << CPR_SSI_CTRL_SSI1_PROTOCOL_Pos) | (CPR_SSI_CTRL_SSI1_MASTER_EN_Master << CPR_SSI_CTRL_SSI1_MASTER_EN_Pos);

    }


    //(0x1000100 << ch);         
    p_instance->p_cpr->CTLAPBCLKEN_GRCTL = ((CPR_CTLAPBCLKEN_GRCTL_SSI0_PCLK_EN_Enable << CPR_CTLAPBCLKEN_GRCTL_SSI0_PCLK_EN_Pos)|
                                        (CPR_CTLAPBCLKEN_GRCTL_SSI0_PCLK_EN_Msk  << CPR_CTLAPBCLKEN_GRCTL_MASK_OFFSET)) <<ch; 
    p_instance->p_cpr->SSI_CTRL = val;




   
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
            err_code = xinc_gpio_secfun_config(mosi_pin,XINC_GPIO_PIN_SSI1_TX);
        
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
            err_code = xinc_gpio_secfun_config(miso_pin,XINC_GPIO_PIN_SSI1_RX);
        
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
            err_code = xinc_gpio_secfun_config(p_config->ss_pin,XINC_GPIO_PIN_SSI1_SSN);
        
            if(err_code != XINCX_SUCCESS)
            {
                return err_code;
            }
        }
        
        if (p_config->sck_pin != XINCX_SPIM_PIN_NOT_USED)
        {
            err_code = xinc_gpio_secfun_config(p_config->sck_pin,XINC_GPIO_PIN_SSI1_CLK);
        
            if(err_code != XINCX_SUCCESS)
            {
                return err_code;
            }
        }
    }

    miso_pin = p_config->miso_pin;
    mosi_pin = p_config->mosi_pin;
    p_cb->miso_pin = p_config->miso_pin;
    // - Slave Select (optional) - output with initial value 1 (inactive).

    // 'p_cb->ss_pin' variable is used during transfers to check if SS pin should be toggled,
    // so this field needs to be initialized even if the pin is not used.
    p_cb->ss_pin = p_config->ss_pin;

    printf("miso_pin:%d\n",miso_pin); 
    printf("mosi_pin:%d\n",mosi_pin); 
    printf("sck_pin:%d\n",p_config->sck_pin);
    printf("ss_pin:%d\n",p_config->ss_pin);		


	
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
        p_cb->rx_dma_ch = 10;
        p_cb->tx_dma_ch = 2;
    }
    
    if(ch == 1)
    {
        p_cb->rx_dma_ch = 11;
        p_cb->tx_dma_ch = 3;
    }
		
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
        XINCX_IRQ_ENABLE((IRQn_Type)(SPI0_IRQn + ch));
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
        XINCX_IRQ_DISABLE((IRQn_Type)(SPI0_IRQn + p_instance->drv_inst_idx));
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
    __write_hw_reg32(DMAS_CHx_SAR(p_cb->tx_dma_ch) , (uint32_t)p_xfer_desc->p_tx_buffer);
    __write_hw_reg32(DMAS_CHx_DAR(p_cb->tx_dma_ch) , (uint32_t)&(p_spim->DATA));
    __write_hw_reg32(DMAS_CHx_CTL1(p_cb->tx_dma_ch) ,((2 << 8)|  0));
    __write_hw_reg32(DMAS_CHx_CTL0(p_cb->tx_dma_ch) ,p_xfer_desc->tx_length);//


        //- RX Channel
    __write_hw_reg32(DMAS_CHx_SAR(p_cb->rx_dma_ch) , (uint32_t)&(p_spim->DATA));//
    __write_hw_reg32(DMAS_CHx_DAR(p_cb->rx_dma_ch) , (uint32_t)p_xfer_desc->p_rx_buffer);
    __write_hw_reg32(DMAS_CHx_CTL1(p_cb->rx_dma_ch) ,((2 << 8)|  0));
    __write_hw_reg32(DMAS_CHx_CTL0(p_cb->rx_dma_ch) ,p_xfer_desc->rx_length);


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
        __write_hw_reg32(DMAS_EN , p_cb->rx_dma_ch);
        __write_hw_reg32(DMAS_EN , p_cb->tx_dma_ch);
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

    {

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
#endif

#if XINCX_CHECK(XINCX_SPIM1_ENABLED)
void xincx_spim_1_irq_handler(void)
{
    irq_handler(XINC_SPIM1, &m_cb[XINCX_SPIM1_INST_IDX]);
}
#endif

void SPI0_Handler()
{
	uint32_t iWK;
	uint32_t iWK1;
	uint32_t STS;
	__read_hw_reg32(SSIx_IS(0) , iWK);
	__read_hw_reg32(SSIx_RIS(0) , iWK1);
	__read_hw_reg32(SSIx_STS(0) , STS);
	
	if(iWK && (STS & 0x4))
	{
        do{
            __read_hw_reg32(DMAS_INT_RAW , iWK);
        }while((iWK&0x404) != 0x404);
				
        __write_hw_reg32(DMAS_INT_RAW, 0x404);
        __write_hw_reg32(DMAS_CLR , 10);
        __write_hw_reg32(DMAS_CLR , 2);

        xincx_spim_0_irq_handler();
    }  
    (void)iWK1;	
}

void SPI1_Handler()
{
	uint32_t iWK;
	uint32_t iWK1;
	uint32_t STS;
	__read_hw_reg32(SSIx_IS(1) , iWK);
	__read_hw_reg32(SSIx_RIS(1) , iWK1);
	__read_hw_reg32(SSIx_STS(1) , STS);
//	printf("SPI1_Handler iWK :%x,iWK1:%x,STS:%x\n",iWK,iWK1,STS);  
	if(iWK && (STS & 0x4))
	{
        do{
            __read_hw_reg32(DMAS_INT_RAW , iWK);
        }while((iWK&0x808) != 0x808);

        __write_hw_reg32(DMAS_INT_RAW, 0x808);
        __write_hw_reg32(DMAS_CLR , 11);
        __write_hw_reg32(DMAS_CLR , 3);

		xincx_spim_1_irq_handler();
	}
	(void)iWK1;
}


#endif // XINCX_CHECK(XINCX_SPIM_ENABLED)
