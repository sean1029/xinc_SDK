/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#include "xinc_drv_i2s.h"

#if XINCX_CHECK(XINCX_I2S_ENABLED)
//#include <xinc_delay.h>
#include <hal/xinc_gpio.h>

#ifdef I2SM_PRESENT
#define INSTANCE_COUNT   I2SM_COUNT
#else
#define INSTANCE_COUNT   I2S_COUNT
#endif


static xinc_drv_i2s_evt_handler_t m_handlers[INSTANCE_COUNT];
static void *                    m_contexts[INSTANCE_COUNT];


#ifdef I2SM_PRESENT
static void i2sm_evt_handler(xincx_i2sm_evt_t const * p_event,
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
#endif // I2SM_PRESENT

#ifdef I2S_PRESENT
static void i2s_evt_handler(xincx_i2s_evt_t const * p_event,
                            void *                 p_context)
{
    uint32_t inst_idx = (uint32_t)p_context;
    xinc_drv_i2s_evt_t const event =
    {
        .type = (xinc_drv_i2s_evt_type_t)p_event->type,
        .xfer_desc =
        {
            .type = (xinc_drv_i2s_xfer_type_t)p_event->xfer_desc.type,
            .address          = p_event->xfer_desc.address,
            .primary_length   = p_event->xfer_desc.primary_length,
            .secondary_length = p_event->xfer_desc.secondary_length,
            .p_primary_buf    = p_event->xfer_desc.p_primary_buf,
            .p_secondary_buf  = p_event->xfer_desc.p_secondary_buf,
        }
    };
    m_handlers[inst_idx](&event, m_contexts[inst_idx]);
}
#endif // I2C_PRESENT

ret_code_t xinc_drv_i2s_init(xinc_drv_i2s_t const *        p_instance,
                            xinc_drv_i2s_config_t const * p_config,
                            xinc_drv_i2s_evt_handler_t    event_handler,
                            void *                       p_context)
{
    uint32_t inst_idx = p_instance->inst_idx;
    m_handlers[inst_idx] = event_handler;
    m_contexts[inst_idx] = p_context;

    ret_code_t result = 0;

    if (XINC_DRV_I2S_USE_I2SM)
    {
        result = xincx_i2sm_init(&p_instance->u.i2sm,
                               (xincx_i2c_config_t const *)p_config,
                               event_handler ? i2sm_evt_handler : NULL,
                               (void *)inst_idx);
    }
    
    else if (XINC_DRV_I2S_USE_I2S)
    {
        result = xincx_i2s_init(&p_instance->u.i2s,
                               (xincx_i2s_config_t const *)p_config,
                               event_handler ? i2s_evt_handler : NULL,
                               (void *)inst_idx);
    }
    return result;
}
#endif //

