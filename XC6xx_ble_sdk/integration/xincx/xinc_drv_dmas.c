/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#include "xinc_drv_dmas.h"

#if XINCX_CHECK(XINCX_DMAS_ENABLED)
#include <hal/xinc_dmas.h>

#ifdef DMAS_PRESENT
#define INSTANCE_COUNT   DMAS_COUNT
#define INSTANCE_CH_COUNT   DMAS_CH_COUNT
#else
#define INSTANCE_COUNT   0
#endif


static xinc_drv_dmas_evt_handler_t m_handlers[DMAS_COUNT];
static xinc_drv_dmas_ch_evt_handler_t m_ch_handlers[DMAS_COUNT];
static void *                      m_contexts[DMAS_COUNT];



static void dmas_evt_handler(xincx_dmas_evt_t const * p_event,
                            void *                 p_context)
{
    uint32_t inst_idx = (uint32_t)p_context;
    xinc_drv_dmas_evt_t const event =
    {
       
    };
    printf("%s\r\n",__func__);
    m_handlers[inst_idx](&event, m_contexts[inst_idx]);
}


ret_code_t xinc_drv_dmas_init(xinc_drv_dmas_t const *        p_instance,
                            xinc_drv_dmas_config_t const * p_config,
                            xinc_drv_dmas_evt_handler_t    event_handler,
                            void *                       p_context)
{
    uint32_t inst_idx = p_instance->inst_idx;
    m_handlers[inst_idx] = event_handler;
    m_contexts[inst_idx] = p_context;

    ret_code_t result = 0;

    
    result = xincx_dmas_init(&p_instance->dmas,
                               (xincx_dmas_config_t const *)p_config,
                               event_handler ? dmas_evt_handler : NULL,
                               (void *)inst_idx);
    
    return result;
}

ret_code_t xinc_drv_dmas_ch_handler_register(xinc_drv_dmas_t const *p_instance ,uint8_t ch,xinc_drv_dmas_ch_evt_handler_t handler)
{
    ret_code_t result = 0;
    
    m_ch_handlers[ch] = handler;
    
    return result;
}


#endif // 