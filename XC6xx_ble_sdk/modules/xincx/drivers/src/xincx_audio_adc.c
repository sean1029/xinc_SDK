/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include <xincx.h>
#include "bsp_gpio.h"
#if XINCX_CHECK(XINCX_AUDIO_ADC_ENABLED)


#if !defined (XINC628_XXAA)
#error "AUDIO_ADC only in chip XINC Check <xincx_config.h>."
#endif


#include <xincx_audio_adc.h>
#include "bsp_register_macro.h"
#include "bsp_clk.h"
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
    XINC_AUDIO_ADC_STATE_CALIBRATION = 2
} xinc_audio_adc_state_t;


typedef struct  gadc_fifo_struct {
    unsigned int value_2 : 10;
    unsigned int chanel_2 : 4;
    unsigned int  : 2;
    unsigned int value_1 : 10;
    unsigned int chanel_1 : 4;
    unsigned int  : 2;
} xinc_audio_adc_fifo_t;

/** @brief AUDIO_ADC control block.*/
typedef struct
{
    xincx_audio_adc_evt_handler_t    event_handler;                 ///< Event handler function pointer.
    volatile xinc_audio_adc_value_t  * p_buffer;                      ///< Sample buffer.
    volatile uint16_t             buffer_size;                   ///< Size of the sample buffer.
    volatile xinc_audio_adc_value_t  * p_secondary_buffer;            ///< Secondary sample buffer.
    volatile xinc_audio_adc_state_t    adc_state;                     ///< State of the AUDIO_ADC.
    uint32_t                      limits_enabled_flags;          ///< Enabled limits flags.
    uint16_t                      secondary_buffer_size;         ///< Size of the secondary buffer.
    uint16_t                      buffer_size_left;              ///< When low power mode is active indicates how many samples left to convert on current buffer.
    xincx_drv_state_t              state;                         ///< Driver initialization state.
    uint8_t                       active_channels;               ///< Number of enabled AUDIO_ADC channels.
    uint8_t                       channel_state[1];                  ///< Indicates if channel is active.
    bool                          conversions_end;               ///
    xinc_audio_adc_channel_config_t   channel_config[1];
} xincx_audio_adc_cb_t;



static xincx_audio_adc_cb_t m_cb[XINCX_AUDIO_ADC_ENABLED_COUNT];



#define HW_TIMEOUT 1000000



static void xincx_audio_adc_irq_handler(XINC_CDC_Type * p_reg,xincx_audio_adc_cb_t * p_cb)
{

    uint32_t INT_reg;
    INT_reg = p_reg->INT;

    
    p_cb->adc_state            = XINC_AUDIO_ADC_STATE_IDLE;

    XINCX_IRQ_DISABLE(GADC_IRQn);

    p_reg->INT = INT_reg;  

    xinc_audio_adc_fifo_t *tmp = NULL;
    uint32_t reg;
    uint32_t val;
    uint16_t gadc_count = 0;
    xincx_audio_adc_evt_t evt;
    reg = p_reg->INT;
    p_reg->INT = reg;
    tmp = (xinc_audio_adc_fifo_t*)&val;
    evt.type = XINCX_AUDIO_ADC_EVT_DONE;
    int  t = 0;

    for( t = 0; t <  p_cb->channel_config[p_cb->active_channels].adc_fifo_len ; t++)
    {
                   
        if(tmp->chanel_1 != tmp->chanel_2)
        {
            evt.type = XINCX_AUDIO_ADC_EVT_ERROR;      
        }
        if(tmp->chanel_1 != p_cb->active_channels)
        {
            evt.type = XINCX_AUDIO_ADC_EVT_ERROR;			
        }

        p_cb->p_buffer[gadc_count] = tmp->value_1;
        
        p_cb->p_buffer[gadc_count] = tmp->value_2;
                    
    }

    evt.data.done.p_buffer = (xinc_audio_adc_value_t *)p_cb->p_buffer;
    evt.data.done.size     = gadc_count;//m_cb.buffer_size;
    evt.data.done.adc_value     = val;
    evt.data.done.channel = p_cb->active_channels;
    p_cb->buffer_size = gadc_count;
    p_cb->event_handler(&evt,&p_cb->active_channels);
}

static void xincx_audio_adc_clk_init(xincx_audio_adc_t const * const  p_instance,
                         xincx_audio_adc_config_t const * p_config)
{
    XINC_CPR_CTL_Type*  p_cpr = p_instance->p_cpr;
    
    p_cpr->RSTCTL_CTLAPB_SW =  (CPR_RSTCTL_CTLAPB_SW_GPADC_RSTN_Enable << CPR_RSTCTL_CTLAPB_SW_GPADC_RSTN_Pos)  |
                                            (CPR_RSTCTL_CTLAPB_SW_GPADC_RSTN_Msk << CPR_RSTCTL_CTLAPB_SW_MASK_OFFSET);                           
    
    p_cpr->RSTCTL_CTLAPB_SW =  (CPR_RSTCTL_CTLAPB_SW_GPADC_RSTN_Disable << CPR_RSTCTL_CTLAPB_SW_GPADC_RSTN_Pos)  |
                                            (CPR_RSTCTL_CTLAPB_SW_GPADC_RSTN_Msk << CPR_RSTCTL_CTLAPB_SW_MASK_OFFSET);                           
        
  
    p_cpr->CTLAPBCLKEN_GRCTL =  (CPR_CTLAPBCLKEN_GRCTL_GPADC_PCLK_EN_Enable <<  CPR_CTLAPBCLKEN_GRCTL_GPADC_PCLK_EN_Pos) | 
                                            (CPR_CTLAPBCLKEN_GRCTL_GPADC_PCLK_EN_Msk << CPR_CTLAPBCLKEN_GRCTL_MASK_OFFSET);
    
 

}

xincx_err_t xincx_audio_adc_init(xincx_audio_adc_t const * const p_instance,
                            xincx_audio_adc_config_t const * p_config,
                           xincx_audio_adc_evt_handler_t  event_handler)
{


    
    xincx_err_t err_code = XINCX_SUCCESS;
    xincx_audio_adc_cb_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    XINC_CDC_Type     *p_reg = p_instance->p_reg;

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

    p_cb->state                = XINCX_DRV_STATE_INITIALIZED;
    p_cb->adc_state            = XINC_AUDIO_ADC_STATE_IDLE;



    xincx_audio_adc_config_set(p_instance,p_config);

    return err_code;
//	
}


void xincx_audio_adc_uninit(xincx_audio_adc_t const * const p_instance)
{
    m_cb[p_instance->drv_inst_idx].state = XINCX_DRV_STATE_UNINITIALIZED;
    for(uint8_t i = 0; i < XINC_AUDIO_ADC_CHANNEL_COUNT;i ++)
    {
        m_cb[p_instance->drv_inst_idx].channel_state[i] = XINCX_DRV_STATE_UNINITIALIZED;
    }   
}

void xincx_audio_adc_config_set(xincx_audio_adc_t const * const p_instance,                                
                                   xincx_audio_adc_config_t const * p_config)

{
    uint32_t reg_val;
    XINC_CDC_Type     *p_reg = p_instance->p_reg;



    if(p_config->refvol ==  XINC_AUDIO_ADC_CHANNEL_REFVOL_2_47)
    {
        reg_val = (p_config->freq << 8) | 0x10;
    }
    else 
    {
        reg_val = (p_config->freq << 8) | 0x12;
    }
            


}

xincx_err_t xincx_audio_adc_channel_init(xincx_audio_adc_t const * const p_instance,
                                    uint8_t channel,
                                   xinc_audio_adc_channel_config_t const * const p_config)
{
    xincx_err_t err_code = XINCX_SUCCESS;
    xincx_audio_adc_cb_t * p_cb = &m_cb[p_instance->drv_inst_idx];

        // A channel can only be initialized if the driver is in the idle state.
    if (p_cb->state != XINCX_DRV_STATE_INITIALIZED)
    {
        err_code = XINCX_ERROR_INVALID_STATE;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                                            __func__,
                                            XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
    if (channel >  XINC_AUDIO_ADC_CHANNEL_COUNT)
    {     
        err_code = XINCX_ERROR_INVALID_PARAM;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                                            __func__,
                                            XINCX_LOG_ERROR_STRING_GET(err_code));
        printf("Function 0: %s, error code: %x.\r\n",
                                            __func__,
                                            (err_code));
        return err_code;
    }

   


    p_cb->channel_config[channel].mode = p_config->mode;
    p_cb->channel_config[channel].adc_fifo_len = p_config->adc_fifo_len;

    printf("adc_fifo_len: %d.\r\n",p_cb->channel_config[channel].adc_fifo_len);


    xinc_audio_adc_channel_init(channel, p_config);
    p_cb->channel_state[channel] = XINCX_DRV_STATE_INITIALIZED;



    XINCX_LOG_INFO("Function: %s, error code: %s.",
                                                __func__,
                                                XINCX_LOG_ERROR_STRING_GET(err_code));
    printf("Function: %s, error code: %x.\r\n",
                                                __func__,
                                                (err_code));
    return XINCX_SUCCESS;
}


xincx_err_t xincx_audio_adc_sample_convert(xincx_audio_adc_t const * const p_instance,uint8_t channel, xinc_audio_adc_value_t * p_value)
{

    xincx_err_t err_code = XINCX_SUCCESS;
    xinc_audio_adc_fifo_t *tmp = NULL;
    uint32_t time_out = HW_TIMEOUT;
    uint16_t gadc_count = 0;
    uint32_t val;
    uint32_t INT_reg;
    XINC_CDC_Type     *p_reg = p_instance->p_reg;
    
    xincx_audio_adc_cb_t * p_cb = &m_cb[p_instance->drv_inst_idx];
     
    if(channel > XINC_AUDIO_ADC_CHANNEL_COUNT)
    {
         err_code = XINCX_ERROR_INVALID_PARAM;
        
         return err_code;
    }
    
    if(p_cb->channel_state[channel] !=  XINCX_DRV_STATE_INITIALIZED)
    {
         err_code = XINCX_ERROR_INVALID_STATE;
        
         return err_code;
    }
    
    if (p_cb->adc_state != XINC_AUDIO_ADC_STATE_IDLE)
	{
		err_code = XINCX_ERROR_INVALID_STATE;
        return err_code;
	}
    p_cb->adc_state = XINC_AUDIO_ADC_STATE_BUSY;
 

    xinc_audio_adc_fifo_clear(p_reg);
    

    


    return err_code;
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


xincx_err_t xincx_audio_adc_sample(xincx_audio_adc_t const * const p_instance,uint8_t channel)
{
    xincx_err_t err_code = XINCX_SUCCESS;
    static uint32_t reg;
    xincx_audio_adc_cb_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    XINC_CDC_Type     *p_reg = p_instance->p_reg; 
    reg = p_reg->INT;

    if (channel > XINC_AUDIO_ADC_CHANNEL_COUNT)
    {
        err_code = XINCX_ERROR_INVALID_PARAM;
        return err_code;
    }


    if(p_cb->channel_state[channel] !=  XINCX_DRV_STATE_INITIALIZED)
    {
            err_code = XINCX_ERROR_INVALID_STATE;
        printf("xincx_audio_adc_sample error :%x\n",err_code);
            return err_code;
    }
        
    if (p_cb->adc_state != XINC_AUDIO_ADC_STATE_IDLE)
    {
        err_code = XINCX_ERROR_INVALID_STATE;
        return err_code;
    }

    p_cb->adc_state = XINC_AUDIO_ADC_STATE_BUSY;

  

    xinc_audio_adc_fifo_clear(p_reg);

    return err_code;
}




bool xincx_audio_adc_is_busy(xincx_audio_adc_t const * const p_instance)
{
    xincx_audio_adc_cb_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    return (p_cb->adc_state != XINC_AUDIO_ADC_STATE_IDLE);
}



void AUDIAO_ADC_Handler(void)
{
  //  xincx_audio_adc_irq_handler(XINC_AUDIO_ADC0, &m_cb[XINCX_AUDIO_ADC0_INST_IDX]);
}




#endif // XINCX_CHECK(XINCX_AUDIO_ADC_ENABLED)
