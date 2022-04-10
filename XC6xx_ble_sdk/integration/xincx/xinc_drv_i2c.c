/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#include "xinc_drv_i2c.h"

#if XINCX_CHECK(XINCX_I2C_ENABLED)
#include <xinc_delay.h>
#include <hal/xinc_gpio.h>

#ifdef I2CM_PRESENT
#define INSTANCE_COUNT   I2CM_COUNT
#else
#define INSTANCE_COUNT   I2C_COUNT
#endif


static xinc_drv_i2c_evt_handler_t m_handlers[INSTANCE_COUNT];
static void *                    m_contexts[INSTANCE_COUNT];

static void i2c_clear_bus(xinc_drv_i2c_config_t const * p_config)
{

}

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

#ifdef I2C_PRESENT
static void i2c_evt_handler(xincx_i2c_evt_t const * p_event,
                            void *                 p_context)
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
#endif // I2C_PRESENT

ret_code_t xinc_drv_i2c_init(xinc_drv_i2c_t const *        p_instance,
                            xinc_drv_i2c_config_t const * p_config,
                            xinc_drv_i2c_evt_handler_t    event_handler,
                            void *                       p_context)
{
    uint32_t inst_idx = p_instance->inst_idx;
    m_handlers[inst_idx] = event_handler;
    m_contexts[inst_idx] = p_context;

    if(p_config->clear_bus_init)
    {
        /* Send clocks (max 9) until slave device back from stuck mode */
        i2c_clear_bus(p_config);
    }

    ret_code_t result = 0;
    if (XINC_DRV_I2C_USE_I2CM)
    {
        result = xincx_i2cm_init(&p_instance->u.i2cm,
                                (xincx_i2cm_config_t const *)p_config,
                                event_handler ? i2cm_evt_handler : NULL,
                                (void *)inst_idx);
    }
    else if (XINC_DRV_I2C_USE_I2C)
    {
        result = xincx_i2c_init(&p_instance->u.i2c,
                               (xincx_i2c_config_t const *)p_config,
                               event_handler ? i2c_evt_handler : NULL,
                               (void *)inst_idx);
    }
    return result;
}
#endif //

