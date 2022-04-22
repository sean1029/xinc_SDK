/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#include "xinc_drv_uart.h"

#ifdef UART_PRESENT
#define INSTANCE_COUNT   UART_COUNT
#endif

static xinc_uart_event_handler_t m_handlers[INSTANCE_COUNT];
static void *                   m_contexts[INSTANCE_COUNT];


#if defined(XINC_DRV_UART_WITH_UART)
static void uart_evt_handler(xincx_uart_event_t const * p_event,
                             void *                    p_context)
{
    uint32_t inst_idx = (uint32_t)p_context;;
 //   printf("uart_evt_handler inst_idx:%d\r\n",inst_idx);
    xinc_drv_uart_event_t event =
    {
        .type = (xinc_drv_uart_evt_type_t)p_event->type,
        .data =
        {
            .error =
            {
                .rxtx =
                {
                    .p_data = p_event->data.error.rxtx.p_data,
                    .bytes  = p_event->data.error.rxtx.bytes,
                },
                .error_mask = p_event->data.error.error_mask,
            }
        }
    };
    m_handlers[inst_idx](&event, m_contexts[inst_idx]);
}
#endif //defined(XINC_DRV_UART_WITH_UART)

#if defined(XINC_DRV_UART_WITH_UARTE)
static void uarte_evt_handler(xincx_uarte_event_t const * p_event,
                             void *                    p_context)
{
    uint32_t inst_idx = (uint32_t)p_context;
    xinc_drv_uart_event_t event =
    {
        .type = (xinc_drv_uart_evt_type_t)p_event->type,
        .data =
        {
            .error =
            {
                .rxtx =
                {
                    .p_data = p_event->data.error.rxtx.p_data,
                    .bytes  = p_event->data.error.rxtx.bytes,
                },
                .error_mask = p_event->data.error.error_mask,
            }
        }
    };
    m_handlers[inst_idx](&event, m_contexts[inst_idx]);
}
#endif //defined(XINC_DRV_UART_WITH_UARTE)

ret_code_t xinc_drv_uart_init(xinc_drv_uart_t const *        p_instance,
                             xinc_drv_uart_config_t const * p_config,
                             xinc_uart_event_handler_t      event_handler)
{
    uint32_t inst_idx = p_instance->inst_idx;
    m_handlers[inst_idx] = event_handler;
    m_contexts[inst_idx] = p_config->p_context;


    xinc_drv_uart_config_t config = *p_config;

    printf("__func__=%s, p_config baudrate:%d\r\n",__func__,p_config->baudrate);
    printf("__func__=%s, config baudrate:%d\r\n",__func__,config.baudrate);
    
    printf("__func__=%s, m_contexts[inst_idx]:%d\r\n",__func__,(uint32_t )m_contexts[inst_idx]);
    printf("__func__=%s, m_handlers[inst_idx]:0x%p\r\n",__func__,m_handlers[inst_idx] );

    

    config.p_context = (void *)inst_idx;
    ret_code_t result = 0;
	
    if (XINC_DRV_UART_USE_UARTE)       
    {
        if(p_instance->use_easy_dma)
        {
            result = xincx_uarte_init(&p_instance->uarte,
                                (xincx_uarte_config_t const *)&config,
                                event_handler ? uarte_evt_handler : NULL);
                                                          
                                
        }else
        {
            result = xincx_uart_init(&p_instance->uart,
                                (xincx_uart_config_t const *)&config,
                                event_handler ? uart_evt_handler : NULL);   
        }

    }
    else if(XINC_DRV_UART_USE_UART)
    {
        result = xincx_uart_init(&p_instance->uart,
                            (xincx_uart_config_t const *)&config,
                            event_handler ? uart_evt_handler : NULL);   
    }


    return result;
}
