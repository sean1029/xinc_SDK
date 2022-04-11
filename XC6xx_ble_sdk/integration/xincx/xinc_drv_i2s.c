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
#include "sdk_macros.h"
#include <xinc_delay.h>
#include <hal/xinc_gpio.h>
#include "xincx_dmas.h"
#ifdef I2SM_PRESENT
#define INSTANCE_COUNT   I2SM_COUNT
#else
#define INSTANCE_COUNT   I2S_COUNT
#endif


static xinc_drv_i2s_evt_handler_t m_handlers[INSTANCE_COUNT];
static void *                    m_contexts[INSTANCE_COUNT];


#ifdef I2SM_PRESENT
static void i2sm_evt_handler(xincx_i2sm_buffers_t const * p_released,
                                        uint32_t                   status,
                                        void *                 p_context)
{
    uint32_t inst_idx = (uint32_t)p_context;
    xinc_drv_i2s_data_handler_t const event =
    {
        .p_released = p_released,
        .status = status,
    };

    m_handlers[inst_idx](&event, m_contexts[inst_idx]);
}
#endif // I2SM_PRESENT

#ifdef I2S_PRESENT
static void i2s_evt_handler(xincx_i2s_buffers_t const * p_released,
                                        uint32_t                   status,
                                        void *                 p_context)
{
    uint32_t inst_idx = (uint32_t)p_context;
    xinc_drv_i2s_data_handler_t const event =
    {
        .p_released = p_released,
        .status = status, 
    };
    m_handlers[inst_idx](&event, m_contexts[inst_idx]);
}
#endif // I2S_PRESENT

ret_code_t xinc_drv_i2s_init(xinc_drv_i2s_t const *        p_instance,
                            xinc_drv_i2s_config_t const * p_config,
                            xinc_drv_i2s_evt_handler_t    handler,
                            void *                       p_context)
{
    uint32_t inst_idx = p_instance->inst_idx;
    m_handlers[inst_idx] = handler;
    m_contexts[inst_idx] = p_context;

    ret_code_t result = XINC_SUCCESS;

    if (XINC_DRV_I2S_USE_I2SM)
    {
        if (p_config == NULL)
        {
            static xincx_i2sm_config_t const default_config = XINCX_I2SM_DEFAULT_CONFIG;
            p_config = &default_config;
        }

        if(!xincx_dmas_is_init())
        {
            result = xincx_dmas_init(NULL,NULL,NULL);
            VERIFY_SUCCESS(result);
            
        }

        result = xincx_i2sm_init(&p_instance->u.i2sm,
                               (xincx_i2sm_config_t const *)p_config,
                               handler ? i2sm_evt_handler : NULL,
                               (void *)inst_idx);
    }  
#ifdef I2S_PRESENT    
    else if (XINC_DRV_I2S_USE_I2S)
    {
        if (p_config == NULL)
        {
            static xincx_i2s_config_t const default_config = XINCX_I2S_DEFAULT_CONFIG;
            p_config = &default1_config;
        }
        
        result = xincx_i2s_init(&p_instance->u.i2s,
                               (xincx_i2s_config_t const *)p_config,
                               event_handler ? i2s_evt_handler : NULL,
                               (void *)inst_idx);
    }
#endif //
    return result;
}

#endif //

