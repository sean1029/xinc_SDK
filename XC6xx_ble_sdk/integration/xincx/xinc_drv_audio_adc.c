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

#include "sdk_macros.h"
#include <hal/xinc_gpio.h>
#include "xincx_dmas.h"

#ifdef AUDIO_ADC_PRESENT
#define INSTANCE_COUNT   AUDIO_ADC_COUNT
#else
#define INSTANCE_COUNT   AUDIO_ADC_COUNT
#endif

static xinc_drv_audio_adc_evt_handler_t m_handlers[INSTANCE_COUNT];
static void *                    m_contexts[INSTANCE_COUNT];


#ifdef AUDIO_ADC_PRESENT
static void audio_adc_evt_handler(xincx_audio_adc_evt_t const * p_event,
                            void *                 p_context)
{
	printf("__func__=%s\n",__func__);
    uint32_t inst_idx = (uint32_t)p_context;
    xinc_drv_audio_adc_evt_t const event =
    {
        .type = p_event->type,
        .data.done.size = p_event->data.done.size,
        .data.done.p_buffer = p_event->data.done.p_buffer,
        .data.done.channel = p_event->data.done.channel
    };
    
    if(m_handlers[inst_idx])
    {
        m_handlers[inst_idx](&event, m_contexts[inst_idx]);
    } 
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

    ret_code_t result = XINC_SUCCESS;
    
    if (p_config == NULL)
    {
        static const xincx_audio_adc_config_t default_config = XINCX_AUDIO_ADC_DEFAULT_CONFIG;
        p_config = &default_config;
    }
    
    if(!xincx_dmas_is_init())
    {
        result = xincx_dmas_init(NULL,NULL,NULL);
        VERIFY_SUCCESS(result);
        
    }
    
    result = xincx_audio_adc_init(&p_instance->audio_adc,
                            (xincx_audio_adc_config_t const *)p_config,
                             audio_adc_evt_handler,(void *)inst_idx);
    
   
    return result;
}
#endif //

