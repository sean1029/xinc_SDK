/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include <nrfx.h>
#include "bsp_gpio.h"
#if NRFX_CHECK(XINCX_SAADC_ENABLED)
#include <xincx_saadc.h>
#include "bsp_register_macro.h"
#include "bsp_clk.h"
//#define NRFX_LOG_MODULE SAADC
#include <nrfx_log.h>

#if !defined(XINCX_SAADC_API_V2)

#define EVT_TO_STR(event)                                                       \
    (event == NRF_SAADC_EVENT_STARTED       ? "NRF_SAADC_EVENT_STARTED"       : \
    (event == NRF_SAADC_EVENT_END           ? "NRF_SAADC_EVENT_END"           : \
    (event == NRF_SAADC_EVENT_DONE          ? "NRF_SAADC_EVENT_DONE"          : \
    (event == NRF_SAADC_EVENT_RESULTDONE    ? "NRF_SAADC_EVENT_RESULTDONE"    : \
    (event == NRF_SAADC_EVENT_CALIBRATEDONE ? "NRF_SAADC_EVENT_CALIBRATEDONE" : \
    (event == NRF_SAADC_EVENT_STOPPED       ? "NRF_SAADC_EVENT_STOPPED"       : \
                                              "UNKNOWN EVENT"))))))


typedef enum
{
    NRF_SAADC_STATE_IDLE        = 0,
    NRF_SAADC_STATE_BUSY        = 1,
    NRF_SAADC_STATE_CALIBRATION = 2
} xinc_saadc_state_t;


typedef struct  gadc_fifo_struct {
    unsigned int value_2 : 10;
    unsigned int chanel_2 : 4;
    unsigned int  : 2;
    unsigned int value_1 : 10;
    unsigned int chanel_1 : 4;
    unsigned int  : 2;
} xinc_saadc_fifo_t;

/** @brief SAADC control block.*/
typedef struct
{
    xincx_saadc_event_handler_t    event_handler;                 ///< Event handler function pointer.
    volatile xinc_saadc_value_t  * p_buffer;                      ///< Sample buffer.
    volatile uint16_t             buffer_size;                   ///< Size of the sample buffer.
    volatile xinc_saadc_value_t  * p_secondary_buffer;            ///< Secondary sample buffer.
    volatile xinc_saadc_state_t    adc_state;                     ///< State of the SAADC.
    uint32_t                      limits_enabled_flags;          ///< Enabled limits flags.
    uint16_t                      secondary_buffer_size;         ///< Size of the secondary buffer.
    uint16_t                      buffer_size_left;              ///< When low power mode is active indicates how many samples left to convert on current buffer.
    nrfx_drv_state_t              state;                         ///< Driver initialization state.
    uint8_t                       active_channels;               ///< Number of enabled SAADC channels.
    uint8_t                       channel_state[NRF_SAADC_CHANNEL_COUNT];                  ///< Indicates if channel is active.
    bool                          conversions_end;               ///
    xinc_saadc_channel_config_t   channel_config[NRF_SAADC_CHANNEL_COUNT];
} xincx_saadc_cb_t;



static xincx_saadc_cb_t m_cb[XINCX_SAADC_ENABLED_COUNT];

static uint16_t gadc_value[2 * XINCX_SAADC_FIFOLEN] = {0};

#define HW_TIMEOUT 1000000



uint16_t fix_data_average(uint16_t *data,uint16_t len)
{
    uint16_t add_cnt = 0;
    uint32_t sum_data = 0;
    uint8_t  filter_cnt = 0;

    filter_cnt = len/4;

    for(int i=0; i<len-1; i++)
    {
        for(int j=0; j<len-i-1; j++)
        {
            if(data[j]>data[j+1])
            {
                uint16_t temp = data[j];
                data[j] = data[j+1];
                data[j+1] = temp;
            }
        }
    }

    for(int i=filter_cnt; i<len-filter_cnt; i++)
    {
        sum_data += data[i];
        add_cnt++;
    }

    if(0 == add_cnt) return 0xffff;
    else return sum_data/add_cnt;
}

static void xincx_saadc_irq_handler(XINC_SAADC_Type * p_reg,xincx_saadc_cb_t * p_cb)
{

    uint32_t INT_reg;
    INT_reg = p_reg->INT;

    if(!(INT_reg & (SAADC_GPADC_INT_READ_REQ_INT_Msk | SAADC_GPADC_INT_FIFO_ERROR_INT_Msk)))
    {
        return;
    }
    p_cb->adc_state            = NRF_SAADC_STATE_IDLE;

    p_reg->MAIN_CTL = XINC_SAADC_GPADC_MAIN_CTL_ALL_CLOSE;

    p_reg->INT_EN = (SAADC_GPADC_INT_EN_FIFO_ERROR_EN_Disable << SAADC_GPADC_INT_EN_FIFO_ERROR_EN_Pos) 
                    | (SAADC_GPADC_INT_EN_READ_REQ_EN_Disable << SAADC_GPADC_INT_EN_READ_REQ_EN_Pos);

    NRFX_IRQ_DISABLE(GADC_IRQn);

    p_reg->INT = INT_reg;  

    xinc_saadc_fifo_t *tmp = NULL;
    uint32_t reg;
    uint32_t val;
    uint16_t gadc_count = 0;
    xincx_saadc_evt_t evt;
    reg = p_reg->INT;
    p_reg->INT = reg;
    tmp = (xinc_saadc_fifo_t*)&val;
    evt.type = XINCX_SAADC_EVT_DONE;
    int  t = 0;

    for( t = 0; t <  p_cb->channel_config[p_cb->active_channels].adc_fifo_len ; t++)
    {
        val = p_reg->FIFO & SAADC_GPADC_FIFO_FIFO_DOUT_Msk;
        //   printf("tmp->chanel_1=[%d],val_1=[%d],tmp->chanel_2=[%d],val_2=[%d]\n",tmp->chanel_1,tmp->value_1,tmp->chanel_2,tmp->value_2);            
        if(tmp->chanel_1 != tmp->chanel_2)
        {
            evt.type = XINCX_SAADC_EVT_ERROR;      
        }
        if(tmp->chanel_1 != p_cb->active_channels)
        {
            evt.type = XINCX_SAADC_EVT_ERROR;			
        }

        p_cb->p_buffer[gadc_count] = tmp->value_1;
        gadc_value[gadc_count++] = tmp->value_1;
        
        p_cb->p_buffer[gadc_count] = tmp->value_2;
        gadc_value[gadc_count++] = tmp->value_2;              
    }
        
    val = fix_data_average(gadc_value,gadc_count); 

    evt.data.done.p_buffer = (xinc_saadc_value_t *)p_cb->p_buffer;
    evt.data.done.size     = gadc_count;//m_cb.buffer_size;
    evt.data.done.adc_value     = val;
    evt.data.done.channel = p_cb->active_channels;
    p_cb->buffer_size = gadc_count;
    p_cb->event_handler(&evt);
}


nrfx_err_t xincx_saadc_init(xincx_saadc_t const * const p_instance,
                            xincx_saadc_config_t const * p_config,
                           xincx_saadc_event_handler_t  event_handler)
{

    XINC_CPR_CTL_Type   *p_cpr = p_instance->p_cpr;
    p_cpr->RSTCTL_CTLAPB_SW = 0x10000000;
    p_cpr->RSTCTL_CTLAPB_SW = 0x10001000;
    p_cpr->CTLAPBCLKEN_GRCTL = 0x20002000;
    
    nrfx_err_t err_code = NRFX_SUCCESS;
    xincx_saadc_cb_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    XINC_SAADC_Type     *p_reg = p_instance->p_reg;

    if (p_cb->state != NRFX_DRV_STATE_UNINITIALIZED)
    {
        err_code = NRFX_ERROR_INVALID_STATE;
        NRFX_LOG_WARNING("Function: %s, error code: %s.",
                                            __func__,
                                            NRFX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
            

    p_cb->event_handler = event_handler;

    p_cb->state                = NRFX_DRV_STATE_INITIALIZED;
    p_cb->adc_state            = NRF_SAADC_STATE_IDLE;

    p_reg->MAIN_CTL = XINC_SAADC_GPADC_MAIN_CTL_ALL_CLOSE;

    p_reg->INT_EN = (SAADC_GPADC_INT_EN_FIFO_ERROR_EN_Disable << SAADC_GPADC_INT_EN_FIFO_ERROR_EN_Pos) 
                    | (SAADC_GPADC_INT_EN_READ_REQ_EN_Disable << SAADC_GPADC_INT_EN_READ_REQ_EN_Pos);

    
    p_reg->FIFO_CTL = SAADC_GPADC_FIFO_CTL_FIFO_FLUSH_Msk;
    p_reg->FIFO_CTL = SAADC_GPADC_FIFO_CTL_FIFO_FLUSH_Empty;


    xincx_saadc_config_set(p_instance,p_config);

    return err_code;
//	
}


void xincx_saadc_uninit(xincx_saadc_t const * const p_instance)
{
    m_cb[p_instance->drv_inst_idx].state = NRFX_DRV_STATE_UNINITIALIZED;
    for(uint8_t i = 0; i < NRF_SAADC_CHANNEL_COUNT;i ++)
    {
        m_cb[p_instance->drv_inst_idx].channel_state[i] = NRFX_DRV_STATE_UNINITIALIZED;
    }   
}

void xincx_saadc_config_set(xincx_saadc_t const * const p_instance,                                
                                   xincx_saadc_config_t const * p_config)

{
    uint32_t reg_val;
    XINC_SAADC_Type     *p_reg = p_instance->p_reg;

    reg_val = p_reg->RF_CTL;

    if(p_config->refvol ==  NRF_SAADC_CHANNEL_REFVOL_2_47)
    {
        reg_val = (p_config->freq << 8) | 0x10;
    }
    else 
    {
        reg_val = (p_config->freq << 8) | 0x12;
    }
            
    p_reg->RF_CTL = reg_val;
    p_reg->CHAN_CTL = 8;

    p_reg->TIMER0 = p_config->waite_time;

}

nrfx_err_t xincx_saadc_channel_init(xincx_saadc_t const * const p_instance,
                                    uint8_t channel,
                                   xinc_saadc_channel_config_t const * const p_config)
{
    nrfx_err_t err_code = NRFX_SUCCESS;
    xincx_saadc_cb_t * p_cb = &m_cb[p_instance->drv_inst_idx];

        // A channel can only be initialized if the driver is in the idle state.
    if (p_cb->state != NRFX_DRV_STATE_INITIALIZED)
    {
        err_code = NRFX_ERROR_INVALID_STATE;
        NRFX_LOG_WARNING("Function: %s, error code: %s.",
                                            __func__,
                                            NRFX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
    if (channel >  NRF_SAADC_CHANNEL_COUNT)
    {     
        err_code = NRFX_ERROR_INVALID_PARAM;
        NRFX_LOG_WARNING("Function: %s, error code: %s.",
                                            __func__,
                                            NRFX_LOG_ERROR_STRING_GET(err_code));
        printf("Function 0: %s, error code: %x.\r\n",
                                            __func__,
                                            (err_code));
        return err_code;
    }

    if (p_config->adc_fifo_len >  SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_15)
    {     
        err_code = NRFX_ERROR_INVALID_PARAM;
        NRFX_LOG_WARNING("Function: %s, error code: %s.",
                                            __func__,
                                            NRFX_LOG_ERROR_STRING_GET(err_code));
        printf("Function 1: %s, error code: %x.\r\n",
                                            __func__,
                                            (err_code));
        return err_code;
    }


    p_cb->channel_config[channel].mode = p_config->mode;
    p_cb->channel_config[channel].adc_fifo_len = p_config->adc_fifo_len;

    printf("adc_fifo_len: %d.\r\n",p_cb->channel_config[channel].adc_fifo_len);


    xinc_saadc_channel_init(channel, p_config);
    p_cb->channel_state[channel] = NRFX_DRV_STATE_INITIALIZED;



    NRFX_LOG_INFO("Function: %s, error code: %s.",
                                                __func__,
                                                NRFX_LOG_ERROR_STRING_GET(err_code));
    printf("Function: %s, error code: %x.\r\n",
                                                __func__,
                                                (err_code));
    return NRFX_SUCCESS;
}


nrfx_err_t xincx_saadc_sample_convert(xincx_saadc_t const * const p_instance,uint8_t channel, xinc_saadc_value_t * p_value)
{

    nrfx_err_t err_code = NRFX_SUCCESS;
    xinc_saadc_fifo_t *tmp = NULL;
    uint32_t time_out = HW_TIMEOUT;
    uint16_t gadc_count = 0;
    uint32_t val;
    uint32_t INT_reg;
    XINC_SAADC_Type     *p_reg = p_instance->p_reg;
    
    xincx_saadc_cb_t * p_cb = &m_cb[p_instance->drv_inst_idx];
     
    if(channel > NRF_SAADC_CHANNEL_COUNT)
    {
         err_code = NRFX_ERROR_INVALID_PARAM;
        
         return err_code;
    }
    
    if(p_cb->channel_state[channel] !=  NRFX_DRV_STATE_INITIALIZED)
    {
         err_code = NRFX_ERROR_INVALID_STATE;
        
         return err_code;
    }
    
    if (p_cb->adc_state != NRF_SAADC_STATE_IDLE)
	{
		err_code = NRFX_ERROR_INVALID_STATE;
        return err_code;
	}
    p_cb->adc_state = NRF_SAADC_STATE_BUSY;
    tmp = (xinc_saadc_fifo_t*)&val;

    p_reg->MAIN_CTL = XINC_SAADC_GPADC_MAIN_CTL_ALL_CLOSE;
    p_reg->CHAN_CTL &= ~SAADC_GPADC_CHAN_CTL_SELECT_CHAN_Msk;
    p_reg->CHAN_CTL |= ((channel << SAADC_GPADC_CHAN_CTL_SELECT_CHAN_Pos) & SAADC_GPADC_CHAN_CTL_SELECT_CHAN_Msk);

    
    p_cb->active_channels = channel;
    p_instance->p_reg->FIFO_CTL &= ~SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_Msk;
    p_instance->p_reg->FIFO_CTL |= (p_cb->channel_config[channel].adc_fifo_len << SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_Pos) 
                                    & SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_Msk;
    xinc_saadc_fifo_clear(p_reg);
    
    INT_reg = p_reg->INT;
    p_reg->INT = INT_reg;
    p_reg->MAIN_CTL = (SAADC_GPADC_MAIN_CTL_GPADC_EN_Open << SAADC_GPADC_MAIN_CTL_GPADC_EN_Pos) 
                    | (SAADC_GPADC_MAIN_CTL_EDGE_SEL_Rise << SAADC_GPADC_MAIN_CTL_EDGE_SEL_Pos);

    INT_reg = p_reg->INT_RAW;
    

    while(((INT_reg & SAADC_GPADC_INT_READ_REQ_INT_Msk) != SAADC_GPADC_INT_READ_REQ_INT_Msk) && time_out)
    {
        __nop();
       INT_reg = p_reg->INT_RAW;
        time_out--;
    }

    p_cb->adc_state            = NRF_SAADC_STATE_IDLE;

    if(time_out == 0)
    {
        p_reg->INT = INT_reg;
        p_reg->MAIN_CTL = XINC_SAADC_GPADC_MAIN_CTL_ALL_CLOSE;
        err_code = NRFX_ERROR_TIMEOUT;
        NRFX_LOG_ERROR("Function: %s, error code: %s.",
                                                __func__,
                                                NRFX_LOG_ERROR_STRING_GET(err_code)); 
        
        printf("sample_convert error 2 :%x\n",err_code);
        return err_code;
    }

    p_reg->INT = INT_reg;
    p_reg->MAIN_CTL = XINC_SAADC_GPADC_MAIN_CTL_ALL_CLOSE;
	
    for(int t = 0; t <  p_cb->channel_config[p_cb->active_channels].adc_fifo_len ; t++)
    {
        
        val = p_reg->FIFO;
                    
    //	printf("tmp->chanel_1=[%d],val_1=[%d],tmp->chanel_2=[%d],val_2=[%d]\n",tmp->chanel_1,tmp->value_1,tmp->chanel_2,tmp->value_2);
        if(tmp->chanel_1 != tmp->chanel_2)
        {
            printf("channel error 0 t:%d\n",t);
            printf("tmp->chanel_1=[%d],val_1=[%d],tmp->chanel_2=[%d],val_2=[%d]\n",tmp->chanel_1,tmp->value_1,tmp->chanel_2,tmp->value_2);	
            err_code = NRFX_ERROR_INTERNAL;
            NRFX_LOG_ERROR("Function: %s, error code: %s.",
                                                __func__,
                                                NRFX_LOG_ERROR_STRING_GET(err_code));
            return err_code;
        }

        if(tmp->chanel_1 != channel)
        {
            printf("channel error 1,t:%d\n",t);
            printf("tmp->chanel_1=[%d],val_1=[%d],tmp->chanel_2=[%d],val_2=[%d]\n",tmp->chanel_1,tmp->value_1,tmp->chanel_2,tmp->value_2);
            err_code = NRFX_ERROR_INTERNAL;
            NRFX_LOG_ERROR("Function: %s, error code: %s.",
                                                __func__,
                                                NRFX_LOG_ERROR_STRING_GET(err_code));
            return err_code;
            
        }	
        gadc_value[gadc_count++] = tmp->value_1;					
        gadc_value[gadc_count++] = tmp->value_2;
            
    }

    p_reg->MAIN_CTL = XINC_SAADC_GPADC_MAIN_CTL_ALL_CLOSE;

    *p_value = fix_data_average(gadc_value,gadc_count); 

    return err_code;
}


nrfx_err_t xincx_saadc_buffer_convert(xincx_saadc_t const * const p_instance,xinc_saadc_value_t * p_buffer, uint16_t size)
{
	nrfx_err_t err_code;
    xincx_saadc_cb_t * p_cb = &m_cb[p_instance->drv_inst_idx];
	if (p_cb->adc_state == NRF_SAADC_STATE_BUSY)
  {
        if ( p_cb->p_secondary_buffer)
        {
            err_code = NRFX_ERROR_BUSY;
            NRFX_LOG_WARNING("Function: %s, error code: %s.",
                             __func__,
                             NRFX_LOG_ERROR_STRING_GET(err_code));
            return err_code;
        }
        else
        {
            p_cb->p_secondary_buffer    = p_buffer;
            p_cb->secondary_buffer_size = size;
            
            err_code = NRFX_SUCCESS;
            NRFX_LOG_WARNING("Function: %s, error code: %s.",
                             __func__,
                             NRFX_LOG_ERROR_STRING_GET(err_code));
            return err_code;
        }
  }
		
//	p_cb->adc_state = NRF_SAADC_STATE_BUSY;

	p_cb->p_buffer           = p_buffer;
	p_cb->buffer_size        = size;
	p_cb->p_secondary_buffer = NULL;

	err_code = NRFX_SUCCESS;
	
	return err_code;
}


nrfx_err_t xincx_saadc_sample(xincx_saadc_t const * const p_instance,uint8_t channel)
{
    nrfx_err_t err_code = NRFX_SUCCESS;
    static uint32_t reg;
    xincx_saadc_cb_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    XINC_SAADC_Type     *p_reg = p_instance->p_reg; 
    reg = p_reg->INT;

    if (channel > NRF_SAADC_CHANNEL_COUNT)
    {
        err_code = NRFX_ERROR_INVALID_PARAM;
        return err_code;
    }


    if(p_cb->channel_state[channel] !=  NRFX_DRV_STATE_INITIALIZED)
    {
            err_code = NRFX_ERROR_INVALID_STATE;
        printf("xincx_saadc_sample error :%x\n",err_code);
            return err_code;
    }
        
    if (p_cb->adc_state != NRF_SAADC_STATE_IDLE)
    {
        err_code = NRFX_ERROR_INVALID_STATE;
        return err_code;
    }

    p_cb->adc_state = NRF_SAADC_STATE_BUSY;

    p_reg->MAIN_CTL = XINC_SAADC_GPADC_MAIN_CTL_ALL_CLOSE;


    p_reg->CHAN_CTL &= ~SAADC_GPADC_CHAN_CTL_SELECT_CHAN_Msk;
    p_reg->CHAN_CTL |= ((channel << SAADC_GPADC_CHAN_CTL_SELECT_CHAN_Pos) & SAADC_GPADC_CHAN_CTL_SELECT_CHAN_Msk);

    p_cb->active_channels = channel;

    p_reg->FIFO_CTL &= ~SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_Msk;
    p_reg->FIFO_CTL |= (p_cb->channel_config[channel].adc_fifo_len << SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_Pos) & SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_Msk;


    xinc_saadc_fifo_clear(p_reg);

    reg = p_reg->INT;
    p_reg->INT_EN = SAADC_GPADC_INT_EN_FIFO_ERROR_EN_Msk | SAADC_GPADC_INT_EN_READ_REQ_EN_Msk;
    p_reg->INT = reg;

    NRFX_IRQ_ENABLE(GADC_IRQn);
    p_reg->MAIN_CTL = (SAADC_GPADC_MAIN_CTL_GPADC_EN_Open << SAADC_GPADC_MAIN_CTL_GPADC_EN_Pos) 
                    | (SAADC_GPADC_MAIN_CTL_EDGE_SEL_Rise << SAADC_GPADC_MAIN_CTL_EDGE_SEL_Pos);;
    return err_code;
}




bool xincx_saadc_is_busy(xincx_saadc_t const * const p_instance)
{
    xincx_saadc_cb_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    return (p_cb->adc_state != NRF_SAADC_STATE_IDLE);
}



void GADC_Handler(void)
{
    xincx_saadc_irq_handler(XINC_SAADC0, &m_cb[XINCX_SAADC0_INST_IDX]);
}



#endif // !defined(XINCX_SAADC_API_V2)


#endif // NRFX_CHECK(XINCX_SAADC_ENABLED)
