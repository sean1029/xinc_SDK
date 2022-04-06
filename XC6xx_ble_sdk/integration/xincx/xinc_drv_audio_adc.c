/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include "xinc_drv_audio_adc.h"

#if XINCX_CHECK(XINCX_AUDIO_ADC_ENABLED)

//#include <xinc_delay.h>
#include <hal/xinc_gpio.h>

#ifdef AUDIO_ADC_PRESENT
#define INSTANCE_COUNT   AUDIO_ADC_COUNT
#else
#define INSTANCE_COUNT   AUDIO_ADC_COUNT
#endif

static xinc_drv_audio_adc_evt_handler_t m_handlers[INSTANCE_COUNT];
static void *                    m_contexts[INSTANCE_COUNT];


#ifdef I2CM_PRESENT
static void i2cm_evt_handler(xincx_i2cm_evt_t const * p_event,
                             void *                  p_context)
{
    uint32_t inst_idx = (uint32_t)p_context;
    xinc_drv_i2c_evt_t const event =
    {
        .type = (xinc_drv_i2c_evt_type_t)p_event->type,
        .xfer_desc =
        {
            .type = (xinc_drv_i2c_xfer_type_t)p_event->xfer_desc.type,
            .address          = p_event->xfer_desc.address,
            .primary_length   = p_event->xfer_desc.primary_length,
            .secondary_length = p_event->xfer_desc.secondary_length,
            .p_primary_buf    = p_event->xfer_desc.p_primary_buf,
            .p_secondary_buf  = p_event->xfer_desc.p_secondary_buf,
        }
    };
    m_handlers[inst_idx](&event, m_contexts[inst_idx]);
}
#endif // I2CM_PRESENT

#ifdef AUDIO_ADC_PRESENT
static void audio_adc_evt_handler(xincx_audio_adc_evt_t const * p_event,
                            void *                 p_context)
{
    uint32_t inst_idx = (uint32_t)p_context;
    xinc_drv_audio_adc_evt_t const event =
    {
        
    };
    m_handlers[inst_idx](&event, m_contexts[inst_idx]);
}
#endif // AUDIO_ADC_PRESENT

ret_code_t xinc_drv_audio_adc_init(xinc_drv_audio_adc_t const *        p_instance,
                            xinc_drv_audio_adc_config_t const * p_config,
                            xinc_drv_audio_adc_evt_handler_t    event_handler,
                            void *                       p_context)
{
    uint32_t inst_idx = p_instance->inst_idx;
    m_handlers[inst_idx] = event_handler;
    m_contexts[inst_idx] = p_context;


    ret_code_t result = 0;
  
    {
        result = xincx_audio_adc_init(&p_instance->audio_adc,
                                (xincx_audio_adc_config_t const *)p_config,
                                event_handler ? audio_adc_evt_handler : NULL
                                );
    }
   
    return result;
}
#endif //

