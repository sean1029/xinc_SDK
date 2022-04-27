/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include <xincx.h>
#if XINCX_CHECK(XINCX_AUDIO_ADC_ENABLED)


#if !defined (XC66XX_M4)
#error "AUDIO_ADC only in chip XC66XX Check <xincx_config.h>."
#endif

#include <xincx_audio_adc.h>
#include <xincx_dmas.h>

#include <hal/xinc_gpio.h>
#include "xinc_delay.h"

//#define XINCX_LOG_MODULE AUDIO_ADC
#include <xincx_log.h>





#define EVT_TO_STR(event)                                                       \
    (event == XINC_AUDIO_ADC_EVENT_STARTED       ? "XINC_AUDIO_ADC_EVENT_STARTED"       : \
    (event == XINC_AUDIO_ADC_EVENT_END           ? "XINC_AUDIO_ADC_EVENT_END"           : \
    (event == XINC_AUDIO_ADC_EVENT_DONE          ? "XINC_AUDIO_ADC_EVENT_DONE"          : \
    (event == XINC_AUDIO_ADC_EVENT_RESULTDONE    ? "XINC_AUDIO_ADC_EVENT_RESULTDONE"    : \
    (event == XINC_AUDIO_ADC_EVENT_CALIBRATEDONE ? "XINC_AUDIO_ADC_EVENT_CALIBRATEDONE" : \
    (event == XINC_AUDIO_ADC_EVENT_STOPPED       ? "XINC_AUDIO_ADC_EVENT_STOPPED"       : \
                                              "UNKNOWN EVENT"))))))


typedef enum
{
    XINC_AUDIO_ADC_STATE_IDLE        = 0,
    XINC_AUDIO_ADC_STATE_BUSY        = 1,
    XINC_AUDIO_ADC_STATE_CONVERT = 2 
} xinc_audio_adc_state_t;



/** @brief AUDIO_ADC control block.*/
typedef struct
{
    xincx_audio_adc_evt_handler_t    event_handler;                 ///< Event handler function pointer.
    void *                  p_context;
    volatile xinc_audio_adc_value_t  * p_buffer;                      ///< Sample buffer.
    volatile uint16_t             buffer_size;                   ///< Size of the sample buffer.
    volatile xinc_audio_adc_value_t  * p_secondary_buffer;            ///< Secondary sample buffer.
    volatile xinc_audio_adc_state_t    adc_state;                     ///< State of the AUDIO_ADC.
    uint16_t                      secondary_buffer_size;         ///< Size of the secondary buffer.
    xincx_drv_state_t              state;                         ///< Driver initialization state.
    uint8_t                       active_channels;               ///< Number of enabled AUDIO_ADC channels.
    xinc_dma_ch_t                 rx_dma_ch;
    uint8_t                       rx_fifo_lvl;
    uint8_t                       channel_state[XINCX_AUDIO_ADC_CH_COUNT];                  ///< Indicates if channel is active.
    bool                          conversions_end;               
    xinc_audio_adc_channel_config_t   channel_config[XINCX_AUDIO_ADC_CH_COUNT];
} xincx_audio_adc_cb_t;



static xincx_audio_adc_cb_t m_cb[XINCX_AUDIO_ADC_ENABLED_COUNT];

static uint32_t audio_adc_buff[32];

static void audio_adc_irq_dma_ch_handler(xincx_dmas_ch_evt_t const * p_event,
                                        void *                 p_context);

#define HW_TIMEOUT 1000000



static void xincx_audio_adc_clk_init(xincx_audio_adc_t const * const  p_instance,
                         xincx_audio_adc_config_t const * p_config)
{
    printf("%s\r\n",__func__);
    XINC_CDC_Type*  p_reg = p_instance->p_reg;
    
    XINC_CPR_CTL_Type*  p_cpr  = p_instance->p_cpr; 
    
    XINC_CPR_AO_CTL_Type*  p_cprAO = p_instance->p_cprAO; 
    
    uint32_t reg_val = p_cprAO->AUDIO_ADC_CTRL[0];
    
    reg_val &= ~(CPR_AO_AUDIO_ADC_CTRL0_PDVBIAS_Msk | CPR_AO_AUDIO_ADC_CTRL0_PDSDMR_Msk |
               CPR_AO_AUDIO_ADC_CTRL0_PDPGAR_Msk  | CPR_AO_AUDIO_ADC_CTRL0_PDMIC_Msk  |
               CPR_AO_AUDIO_ADC_CTRL0_PDBIAS_Msk) ; //-config electret mic (פ���� mic)
    p_cprAO->AUDIO_ADC_CTRL[0] = reg_val;
    
    
    p_cpr->CTLAPBCLKEN_GRCTL = ( ( CPR_CTLAPBCLKEN_GRCTL_BT_PCLK_EN_Enable << CPR_CTLAPBCLKEN_GRCTL_BT_PCLK_EN_Pos ) |
                                 ( 0x01UL << CPR_CTLAPBCLKEN_GRCTL_MASK_OFFSET ) ); //- bt_pclk 
    
  //  *((uint32_t volatile*)0x40002450)|= 0x1;                  //- open rf digital
    
    p_cprAO->RESERVED03[0] |= 0x1; //- open rf digital
    
     *((volatile unsigned *)(0X4002F000+ 0x068)) |=0xF000;     //- open bbpll 64M 
     
     
     //reset audio adc
     p_cpr->M4_NEW_REG1 &= ~CPR_OTHER_CLK_CDC_RSTN_REG_Msk;
     xinc_delay_ms(2);
     p_cpr->M4_NEW_REG1 |= CPR_OTHER_CLK_CDC_RSTN_REG_Msk;
     
     //audio ADC decimal mul div    (mul/div)=(2/8)=1/4
     p_cpr->M4_NEW_REG2 = (0x02UL << CPR_OTHER_CLK_CDC_MCLK_MUL_Pos) | (0x08UL << CPR_OTHER_CLK_CDC_MCLK_DIV_Pos);
     
     //audio ADC mclk primary frequency division set 2[FPGA no active]
     reg_val = p_cpr->M4_NEW_REG1;
     reg_val &= ~CPR_OTHER_CLK_CDC_MCLK_GR_Msk;
     reg_val |= ( 0x02UL << CPR_OTHER_CLK_CDC_MCLK_GR_Pos);
     p_cpr->M4_NEW_REG1 = reg_val;
     
     //audio frequency division update 
     reg_val = p_cpr->M4_NEW_REG1;
     reg_val |= (CPR_OTHER_CLK_CDC_MCLK_GR_UPD_Enable << CPR_OTHER_CLK_CDC_MCLK_GR_UPD_Pos);
     p_cpr->M4_NEW_REG1 = reg_val;
     
     //audio hclk enable
     reg_val = p_cpr->M4_NEW_REG1;
     reg_val |= (CPR_OTHER_CLK_CDC_HCLK_EN_Enable << CPR_OTHER_CLK_CDC_HCLK_EN_Pos);
     p_cpr->M4_NEW_REG1 = reg_val;
     
    printf("reg_val:%08x\r\n",reg_val);
 

}

static void xincx_audio_adc_dma_ch_set(xincx_audio_adc_t const * const  p_instance )
{
    uint8_t id = p_instance->id;
    xincx_audio_adc_cb_t * p_cb = &m_cb[p_instance->drv_inst_idx];
  
    p_cb->rx_dma_ch = DMAS_CH_RCV_AUDIO_ADC;
    xincx_dmas_ch_handler_register(p_cb->rx_dma_ch,audio_adc_irq_dma_ch_handler);  
}

xincx_err_t xincx_audio_adc_init(xincx_audio_adc_t const * const p_instance,
                            xincx_audio_adc_config_t const * p_config,
                           xincx_audio_adc_evt_handler_t  event_handler,
                             void                     * p_context)
{


    
    xincx_err_t err_code = XINCX_SUCCESS;
    xincx_audio_adc_cb_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    XINC_CDC_Type     *p_reg = p_instance->p_reg;

     printf("%s\r\n",__func__);
    if (p_cb->state != XINCX_DRV_STATE_UNINITIALIZED)
    {
        err_code = XINCX_ERROR_INVALID_STATE;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                                            __func__,
                                            XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }

    xincx_audio_adc_clk_init(p_instance,p_config);  

    p_cb->event_handler = event_handler;
    
    p_cb->p_context = p_context;
    
    xincx_audio_adc_dma_ch_set(p_instance);

    p_cb->state                = XINCX_DRV_STATE_INITIALIZED;
    p_cb->adc_state            = XINC_AUDIO_ADC_STATE_IDLE;

    //audio adc 采用DMA方式,因此会调用dma 的中断处理函数。

    xincx_audio_adc_config_set(p_instance,p_config);

    return err_code;
	
}


void xincx_audio_adc_uninit(xincx_audio_adc_t const * const p_instance)
{
    XINCX_ASSERT(m_cb[p_instance->drv_inst_idx].state != XINCX_DRV_STATE_UNINITIALIZED);
    
    m_cb[p_instance->drv_inst_idx].state = XINCX_DRV_STATE_UNINITIALIZED;
    for(uint8_t i = 0; i < XINC_AUDIO_ADC_CHANNEL_COUNT;i ++)
    {
        m_cb[p_instance->drv_inst_idx].channel_state[i] = XINCX_DRV_STATE_UNINITIALIZED;
    }   
}

void xincx_audio_adc_enable(xincx_audio_adc_t const * const p_instance)
{
    xincx_audio_adc_cb_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    
    XINC_CDC_Type *       p_reg = p_instance->p_reg;
    
    XINCX_ASSERT(p_cb->state == XINCX_DRV_STATE_INITIALIZED);
    
    
    xinc_audio_adc_enable(p_instance->p_reg);
    xincx_dmas_ch_set_t set;

    set.ch = p_cb->rx_dma_ch;
    set.src_addr = (uint32_t)&(p_reg->RXFIFO_DATA);
    set.dst_addr = (uint32_t)p_cb->p_buffer;
    set.ctl0 = p_cb->rx_fifo_lvl;
    set.ctl1 = ((2 << 8)|  2 | 0xfff << 16);
    xincx_dmas_ch_param_set(set);
    
    p_cb->adc_state            = XINC_AUDIO_ADC_STATE_CONVERT;
    p_cb->state = XINCX_DRV_STATE_POWERED_ON;
    
}

void xincx_audio_adc_disable(xincx_audio_adc_t const * const p_instance)
{
    xincx_audio_adc_cb_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    XINCX_ASSERT(p_cb->state == XINCX_DRV_STATE_POWERED_ON);
    
    xinc_audio_adc_disable(p_instance->p_reg);
    
    p_cb->adc_state            = XINC_AUDIO_ADC_STATE_IDLE;
    p_cb->state = XINCX_DRV_STATE_INITIALIZED;
    
}
//    uint32_t                mic_p;                 ///< mic_p pin number.
//    uint32_t                mic_n;                 ///< mic_n pin number.
//    uint32_t                mic_bias;                 ///< mic_bias pin number.
//    xinc_audio_adc_freq_t   frequency;           ///< audio adc frequency.
//    uint8_t                 interrupt_priority;  ///< Interrupt priority.
//    xinc_audio_adc_channel_config_t ch_config;
/*
audio_adc_clk)输出的频率=bbpll*(gr/8)*(mul/div)*(1/2);
audio 一级分频得到 freq1=audio_hclk/(audio ADC mclk primary frequency division)=16M/2=8M
audio 二级分频得到 freq2=freq1*(mul/div)=8*1/4=2M 
audio 采样率 =freq2/128=2M/128=16K <-->
(16K的采样率就代表1s采集16K的数据，一个数据占两个自己)
*/
void xincx_audio_adc_config_set(xincx_audio_adc_t const * const p_instance,                                
                                   xincx_audio_adc_config_t const * p_config)

{
    uint32_t reg_val;
    xincx_err_t err_code = XINCX_SUCCESS;
    XINC_CDC_Type*  p_reg = p_instance->p_reg;
     xincx_audio_adc_cb_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    if (p_config->mic_p != XINCX_AUDIO_ADC_PIN_NOT_USED)
    {
                     
         err_code = xinc_gpio_secfun_config(p_config->mic_p,XINC_GPIO_PIN_AUDIO_MIC_P);

    }
    
    if (p_config->mic_n != XINCX_AUDIO_ADC_PIN_NOT_USED)
    {
                     
         err_code = xinc_gpio_secfun_config(p_config->mic_p,XINC_GPIO_PIN_AUDIO_MIC_N);

    }
    
    if (p_config->mic_bias != XINCX_AUDIO_ADC_PIN_NOT_USED)
    {
                     
         err_code = xinc_gpio_secfun_config(p_config->mic_p,XINC_GPIO_PIN_AUDIO_MIC_BIAS);

    }
    reg_val = p_reg->ANA_REG0;
    
    reg_val &= ~(CDC_CDC_ANA_REG0_REG00_D_VOL_Msk | CDC_CDC_ANA_REG0_REG00_HPF_EN_Msk | CDC_CDC_ANA_REG0_REG00_HPF_BYPASS_Msk);
    
    reg_val |=  ((p_config->ch_config.reg0.bits.adc_d_vol << CDC_CDC_ANA_REG0_REG00_D_VOL_Pos) & CDC_CDC_ANA_REG0_REG00_D_VOL_Msk);
    
    reg_val |=  ((p_config->ch_config.reg0.bits.hpf_en << CDC_CDC_ANA_REG0_REG00_HPF_EN_Pos ) & CDC_CDC_ANA_REG0_REG00_HPF_EN_Msk);

    reg_val |=  ((p_config->ch_config.reg0.bits.hpf_bypass << CDC_CDC_ANA_REG0_REG00_HPF_BYPASS_Pos ) & CDC_CDC_ANA_REG0_REG00_HPF_BYPASS_Msk);

    p_reg->ANA_REG0 = reg_val;
    
    p_reg->DMA_CONFIG = (p_config->ch_config.adc_fifo_len << CDC_CDC_DMA_CONFIG_DMA_RXLVL_Pos) & CDC_CDC_DMA_CONFIG_DMA_RXLVL_Msk;
    p_cb->rx_fifo_lvl = p_config->ch_config.adc_fifo_len ;

}




xincx_err_t xincx_audio_adc_buffer_convert(xincx_audio_adc_t const * const p_instance,xinc_audio_adc_value_t * p_buffer, uint16_t size)
{
	xincx_err_t err_code;
    xincx_audio_adc_cb_t * p_cb = &m_cb[p_instance->drv_inst_idx];
	if (p_cb->adc_state == XINC_AUDIO_ADC_STATE_BUSY)
    {
        if ( p_cb->p_secondary_buffer)
        {
            err_code = XINCX_ERROR_BUSY;
            XINCX_LOG_WARNING("Function: %s, error code: %s.",
                             __func__,
                             XINCX_LOG_ERROR_STRING_GET(err_code));
            return err_code;
        }
        else
        {
            p_cb->p_secondary_buffer    = p_buffer;
            p_cb->secondary_buffer_size = size;
            
            err_code = XINCX_SUCCESS;
            XINCX_LOG_WARNING("Function: %s, error code: %s.",
                             __func__,
                             XINCX_LOG_ERROR_STRING_GET(err_code));
            return err_code;
        }
  }
		
//	p_cb->adc_state = XINC_AUDIO_ADC_STATE_BUSY;

	p_cb->p_buffer           = p_buffer;
	p_cb->buffer_size        = size;
	p_cb->p_secondary_buffer = NULL;

	err_code = XINCX_SUCCESS;
	
	return err_code;
}


bool xincx_audio_adc_is_busy(xincx_audio_adc_t const * const p_instance)
{
    xincx_audio_adc_cb_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    return (p_cb->adc_state != XINC_AUDIO_ADC_STATE_IDLE);
}


static void rx_data_event(xincx_audio_adc_cb_t * p_cb,
                          size_t                 data_len,
                          volatile xinc_audio_adc_value_t *  p_data)
{
    p_cb->adc_state            = XINC_AUDIO_ADC_STATE_IDLE;

    xincx_audio_adc_evt_t evt;

    evt.type = XINCX_AUDIO_ADC_EVT_DONE;
    
    evt.data.done.size = data_len;
    evt.data.done.p_buffer = p_data;
  
    p_cb->event_handler(&evt,p_cb->p_context);
}

static void xincx_audio_adc_irq_handler(XINC_CDC_Type * p_reg,xincx_audio_adc_cb_t * p_cb)
{
    //没有使用audio adc 中断方式

}

void AUDIAO_ADC_Handler(void)
{
    xincx_audio_adc_irq_handler(XINC_AUDIO_ADC0, &m_cb[XINCX_AUDIO_ADC0_INST_IDX]);
}


static void audio_adc_irq_dma_rx_handler(XINC_CDC_Type *        p_reg,
                             xincx_audio_adc_cb_t * p_cb)
{
   
    uint32_t RD_ADDR;
    uint32_t rx_len;
   
    xincx_dmas_ch_ca_get(p_cb->rx_dma_ch,&RD_ADDR);
    rx_len = RD_ADDR - (uint32_t)p_cb->p_buffer;
    
    rx_data_event(p_cb, rx_len, p_cb->p_buffer);
    
    if(p_cb->adc_state == XINC_AUDIO_ADC_STATE_CONVERT)
    {
        xincx_dmas_ch_set_t set;
        set.ch = p_cb->rx_dma_ch;
        set.src_addr = (uint32_t)&(p_reg->RXFIFO_DATA);
        set.dst_addr = (uint32_t)p_cb->p_buffer;
        set.ctl0 = p_cb->rx_fifo_lvl;
        set.ctl1 = ((2 << 8)|  2 | 0xfff << 16);
        xincx_dmas_ch_param_set(set);
    }
        
}

static void audio_adc_irq_dma_ch_handler(xincx_dmas_ch_evt_t const * p_event,
                                        void *                 p_context)
{   
    xincx_audio_adc_cb_t * p_cb = &m_cb[XINCX_AUDIO_ADC0_INST_IDX];
    if(p_event->dmas_ch == p_cb->rx_dma_ch)
    {
        audio_adc_irq_dma_rx_handler(XINC_AUDIO_ADC0, &m_cb[XINCX_AUDIO_ADC0_INST_IDX]);
    }
}




#endif // XINCX_CHECK(XINCX_AUDIO_ADC_ENABLED)
