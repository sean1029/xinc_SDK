
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


typedef struct  gadc_cache_struct {
    unsigned int value_2 : 10;
    unsigned int chanel_2 : 4;
    unsigned int  : 2;
    unsigned int value_1 : 10;
    unsigned int chanel_1 : 4;
    unsigned int  : 2;
} xinc_saadc_cache_t;

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
    uint8_t                       adc_fifo_len;                  ///< Indicates if low power mode is active.
    bool                          conversions_end;               ///< When low power mode is active indicates end of conversions on current buffer.
} xincx_saadc_cb_t;



static xincx_saadc_cb_t m_cb;

static uint16_t gadc_value[2*16] = {0};

#define LOW_LIMIT_TO_FLAG(channel)      ((2 * channel + 1))
#define HIGH_LIMIT_TO_FLAG(channel)     ((2 * channel))
#define FLAG_IDX_TO_EVENT(idx)          ((xinc_saadc_event_t)((uint32_t)NRF_SAADC_EVENT_CH0_LIMITH + \
                                            4 * idx))
#define LIMIT_EVENT_TO_CHANNEL(event)   (uint8_t)(((uint32_t)event - \
                                            (uint32_t)NRF_SAADC_EVENT_CH0_LIMITH) / 8)
#define LIMIT_EVENT_TO_LIMIT_TYPE(event)((((uint32_t)event - (uint32_t)NRF_SAADC_EVENT_CH0_LIMITH) & 4) \
                                            ? NRF_SAADC_LIMIT_LOW : NRF_SAADC_LIMIT_HIGH)
#define HW_TIMEOUT 10000



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

void xincx_saadc_irq_handler(void)
{
	xinc_saadc_cache_t *tmp = NULL;
	uint32_t reg;
	uint32_t val;
	uint16_t gadc_count = 0;
	xincx_saadc_evt_t evt;
	reg = XINC_SAADC->INT;
	XINC_SAADC->INT = reg;
	tmp = (xinc_saadc_cache_t*)&val;
	evt.type = XINCX_SAADC_EVT_DONE;
	int  t=0;
	for( t=0; t <  m_cb.adc_fifo_len ; t++)
	{
			
		__read_hw_reg32(GPADC_FIFO,val);
					
		if(tmp->chanel_1 != tmp->chanel_2)
		{
			evt.type = XINCX_SAADC_EVT_ERROR;
			
		}
		if(tmp->chanel_1 != m_cb.active_channels)
		{
			evt.type = XINCX_SAADC_EVT_ERROR;			
		}

		m_cb.p_buffer[gadc_count] = tmp->value_1;
		gadc_value[gadc_count++] = tmp->value_1;
		
		m_cb.p_buffer[gadc_count] = tmp->value_2;
		gadc_value[gadc_count++] = tmp->value_2;
				
	}
			
	 val = fix_data_average(gadc_value,gadc_count); 
 
	 evt.data.done.p_buffer = (xinc_saadc_value_t *)m_cb.p_buffer;
	 evt.data.done.size     = gadc_count;//m_cb.buffer_size;
	 evt.data.done.adc_value     = val;
	 evt.data.done.channel = m_cb.active_channels;
	 m_cb.buffer_size = gadc_count;
	 m_cb.event_handler(&evt);
	
	
}


nrfx_err_t xincx_saadc_init(xincx_saadc_config_t const * p_config,
                           xincx_saadc_event_handler_t  event_handler)
{

	xc_saadc_clk_init();
	
  nrfx_err_t err_code;
	
	if (m_cb.state != NRFX_DRV_STATE_UNINITIALIZED)
	{
			err_code = NRFX_ERROR_INVALID_STATE;
			NRFX_LOG_WARNING("Function: %s, error code: %s.",
											 __func__,
											 NRFX_LOG_ERROR_STRING_GET(err_code));
			return err_code;
	}
	
	
		
	m_cb.event_handler = event_handler;

	m_cb.state                = NRFX_DRV_STATE_INITIALIZED;
	m_cb.adc_state            = NRF_SAADC_STATE_IDLE;

	XINC_SAADC->INT_EN = 0x00;

	
	err_code = NRFX_SUCCESS;
  
   return err_code;
	
}


void xincx_saadc_uninit(void)
{
	m_cb.state = NRFX_DRV_STATE_UNINITIALIZED;
}


nrfx_err_t xincx_saadc_channel_init(uint8_t                                  channel,
                                   xinc_saadc_channel_config_t const * const p_config)
{
	nrfx_err_t err_code = NRFX_SUCCESS;
	    // A channel can only be initialized if the driver is in the idle state.
	if (m_cb.adc_state != NRF_SAADC_STATE_IDLE)
	{
			err_code = NRFX_ERROR_BUSY;
			NRFX_LOG_WARNING("Function: %s, error code: %s.",
											 __func__,
											 NRFX_LOG_ERROR_STRING_GET(err_code));
			return err_code;
	}
	if (p_config->adc_fifo_len >  0xF)
	{
		
			err_code = NRFX_ERROR_INVALID_PARAM;
			NRFX_LOG_WARNING("Function: %s, error code: %s.",
											 __func__,
											 NRFX_LOG_ERROR_STRING_GET(err_code));
		   printf("Function: %s, error code: %s.\r\n",
											 __func__,
											 NRFX_LOG_ERROR_STRING_GET(err_code));
			return err_code;
	}
	m_cb.adc_fifo_len = p_config->adc_fifo_len;
	m_cb.active_channels = channel;

	xinc_saadc_channel_init(channel, p_config);
	NRFX_LOG_INFO("Function: %s, error code: %s.",
											 __func__,
											 NRFX_LOG_ERROR_STRING_GET(err_code));
	printf("Function: %s, error code: %s.\r\n",
											 __func__,
											 NRFX_LOG_ERROR_STRING_GET(err_code));
	return NRFX_SUCCESS;
}


nrfx_err_t xincx_saadc_sample_convert(uint8_t channel, xinc_saadc_value_t * p_value)
{

	nrfx_err_t err_code = NRFX_SUCCESS;
	xinc_saadc_cache_t *tmp = NULL;
	uint32_t time_out = HW_TIMEOUT;
  uint16_t gadc_count = 0;
  uint32_t val;
	uint32_t INT_reg;
	tmp = (xinc_saadc_cache_t*)&val;
	XINC_SAADC->MAIN_CTL = 0;
	XINC_SAADC->CHAN_CTL = channel;
	m_cb.active_channels = channel;
	xinc_saadc_fifo_clear();
	INT_reg = XINC_SAADC->INT;
	XINC_SAADC->INT = INT_reg;
	XINC_SAADC->MAIN_CTL = 9;
	INT_reg = XINC_SAADC->INT;
	
	while(((INT_reg & 0x01) != 0x1) && time_out)
	{
		__nop();
		INT_reg = XINC_SAADC->INT;
		time_out--;
	}
	if(time_out == 0)
	{
		XINC_SAADC->INT = INT_reg;
		XINC_SAADC->MAIN_CTL = 0;
		err_code = NRFX_ERROR_TIMEOUT;
		NRFX_LOG_ERROR("Function: %s, error code: %s.",
											 __func__,
											 NRFX_LOG_ERROR_STRING_GET(err_code)); 
		return err_code;
	}

	XINC_SAADC->INT = INT_reg;
	XINC_SAADC->MAIN_CTL = 0;

	
		for(int t=0; t <  m_cb.adc_fifo_len ; t++)
		{
			
		__read_hw_reg32(GPADC_FIFO,val);
					
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

	XINC_SAADC->MAIN_CTL = 0;
	
	*p_value = fix_data_average(gadc_value,gadc_count); 
	
	return err_code;
}


nrfx_err_t xincx_saadc_buffer_convert(xinc_saadc_value_t * p_buffer, uint16_t size)
{
	nrfx_err_t err_code;
	if (m_cb.adc_state == NRF_SAADC_STATE_BUSY)
  {
        if ( m_cb.p_secondary_buffer)
        {
            err_code = NRFX_ERROR_BUSY;
            NRFX_LOG_WARNING("Function: %s, error code: %s.",
                             __func__,
                             NRFX_LOG_ERROR_STRING_GET(err_code));
            return err_code;
        }
        else
        {
            m_cb.p_secondary_buffer    = p_buffer;
            m_cb.secondary_buffer_size = size;
            
            err_code = NRFX_SUCCESS;
            NRFX_LOG_WARNING("Function: %s, error code: %s.",
                             __func__,
                             NRFX_LOG_ERROR_STRING_GET(err_code));
            return err_code;
        }
  }
		
	m_cb.adc_state = NRF_SAADC_STATE_BUSY;

	m_cb.p_buffer           = p_buffer;
	m_cb.buffer_size        = size;
	m_cb.p_secondary_buffer = NULL;

	err_code = NRFX_SUCCESS;
	
	return err_code;
}


nrfx_err_t xincx_saadc_sample(uint8_t channel)
{
	nrfx_err_t err_code = NRFX_SUCCESS;
	static uint32_t reg;
	reg = XINC_SAADC->INT;
	if (m_cb.adc_state != NRF_SAADC_STATE_BUSY)
	{
			err_code = NRFX_ERROR_INVALID_STATE;
	}
	
	XINC_SAADC->MAIN_CTL = 0;
	XINC_SAADC->CHAN_CTL = channel;
	m_cb.active_channels = channel;
	xinc_saadc_fifo_clear();
	reg = XINC_SAADC->INT;
	
	XINC_SAADC->INT_EN = 0x03;
	XINC_SAADC->INT = reg;
	
	NRFX_IRQ_ENABLE(GADC_IRQn);
	XINC_SAADC->MAIN_CTL = 0x9;

	return err_code;
}




bool xincx_saadc_is_busy(void)
{
    return (m_cb.adc_state != NRF_SAADC_STATE_IDLE);
}



void GADC_Handler(void)
{
	uint32_t INT_reg;
	INT_reg = XINC_SAADC->INT;
	if(INT_reg == 0)
	{
		return;
	}
	XINC_SAADC->MAIN_CTL = 0;
	XINC_SAADC->INT_EN = 0;
	NRFX_IRQ_DISABLE(GADC_IRQn);
	
	XINC_SAADC->INT = INT_reg;

	xincx_saadc_irq_handler();
}



#endif // !defined(XINCX_SAADC_API_V2)


#endif // NRFX_CHECK(XINCX_SAADC_ENABLED)
