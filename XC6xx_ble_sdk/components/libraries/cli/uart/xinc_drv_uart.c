/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#include "xinc_drv_uart.h"

#ifdef UARTE_PRESENT
#define INSTANCE_COUNT   UARTE_COUNT
#else
#define INSTANCE_COUNT   UART_COUNT
#endif

static xinc_uart_event_handler_t m_handlers[INSTANCE_COUNT];
static void *                   m_contexts[INSTANCE_COUNT];

#if defined(UARTE_PRESENT) && defined(UART_PRESENT)
uint8_t xinc_drv_uart_use_easy_dma[INSTANCE_COUNT];
#endif

#if defined(XINC_DRV_UART_WITH_UARTE)
static void uarte_evt_handler(xincx_uarte_event_t const * p_event,
                              void *                     p_context)
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
#endif // defined(XINC_DRV_UART_WITH_UARTE)

#if defined(XINC_DRV_UART_WITH_UART)
static void uart_evt_handler(xincx_uart_event_t const * p_event,
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
#endif // defined(XINC_DRV_UART_WITH_UART)

ret_code_t xinc_drv_uart_init(xinc_drv_uart_t const *        p_instance,
                             xinc_drv_uart_config_t const * p_config,
                             xinc_uart_event_handler_t      event_handler)
{
    uint32_t inst_idx = p_instance->inst_idx;
    m_handlers[inst_idx] = event_handler;
    m_contexts[inst_idx] = p_config->p_context;

#if defined(XINC_DRV_UART_WITH_UARTE) && defined(XINC_DRV_UART_WITH_UART)
#ifdef XINC6206_XXAA
    if (inst_idx == 1)
    {
        ASSERT(p_config->use_easy_dma);
    }
#endif
    xinc_drv_uart_use_easy_dma[inst_idx] = p_config->use_easy_dma;
#endif

    xinc_drv_uart_config_t config = *p_config;
    printf("uart_init p_config baudrate:%d\r\n",p_config->baudrate);
    printf("uart_init config baudrate:%d\r\n",config.baudrate);
    config.p_context = (void *)inst_idx;

    ret_code_t result = 0;
	
		printf("%s,USE_UARTE:%d\r\n",__func__,XINC_DRV_UART_USE_UARTE);
    if (XINC_DRV_UART_USE_UARTE)
    {
        result = xincx_uarte_init(&p_instance->uarte,
                                 (xincx_uarte_config_t const *)&config,
                                 event_handler ? uarte_evt_handler : NULL);
    }
    else if (XINC_DRV_UART_USE_UART)
    {
        result = xincx_uart_init(&p_instance->uart,
                                (xincx_uart_config_t const *)&config,
                                event_handler ? uart_evt_handler : NULL);
    }
    return result;
}
