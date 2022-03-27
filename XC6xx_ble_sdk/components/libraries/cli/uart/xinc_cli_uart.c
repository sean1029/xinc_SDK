/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include "sdk_common.h"
#if XINC_MODULE_ENABLED(XINC_CLI_UART)
#include "xinc_cli_uart.h"
#include "xinc_drv_uart.h"
#include "xinc_assert.h"

#define XINC_LOG_MODULE_NAME cli_uart

#define XINC_LOG_LEVEL       (XINC_CLI_UART_CONFIG_LOG_ENABLED ? XINC_CLI_UART_CONFIG_LOG_LEVEL : 0)
#define XINC_LOG_INFO_COLOR  XINC_CLI_UART_CONFIG_INFO_COLOR
#define XINC_LOG_DEBUG_COLOR XINC_CLI_UART_CONFIG_DEBUG_COLOR

#include "xinc_log.h"
XINC_LOG_MODULE_REGISTER();

#define CLI_UART_RX_TIMEOUT 100

static ret_code_t rx_try(xinc_cli_uart_internal_t * p_internal)
{
    ret_code_t err_code;
    size_t     len = 255;
    uint8_t *  p_data;
//		printf("rx_try\r\n ");
    err_code = xinc_ringbuf_alloc(p_internal->p_rx_ringbuf, &p_data, &len, true);
    
//    printf("rx_try ringbuf_alloc err_code:%d;len:%d\r\n",err_code,len);

    ASSERT(err_code == XINC_SUCCESS);
		
    if ((err_code == XINC_SUCCESS) && len)
    {
        err_code = xinc_drv_uart_rx(p_internal->p_uart, p_data, len);
			
        if (err_code == XINC_SUCCESS)
        {
            err_code = app_timer_start(*p_internal->p_timer,
                                        APP_TIMER_TICKS(CLI_UART_RX_TIMEOUT),
                                        p_internal);
        }
    }

    return err_code;
}

static void uart_event_handler(xinc_drv_uart_event_t * p_event, void * p_context)
{
    xinc_cli_uart_internal_t * p_internal = (xinc_cli_uart_internal_t *)p_context;
    ret_code_t err_code = XINC_SUCCESS;
    UNUSED_VARIABLE(err_code);
    uint8_t * p_data;
    size_t len = 255;
	//	printf("uart_event_handler type:%d\r\n ",p_event->type);
    switch (p_event->type)
    {
        case XINC_DRV_UART_EVT_ERROR:
            XINC_LOG_WARNING("id:%d, evt: ERROR:%d",
                            p_internal->p_uart->inst_idx,
                            p_event->data.error.error_mask);
            err_code = xinc_ringbuf_put(p_internal->p_rx_ringbuf, p_event->data.error.rxtx.bytes);
            ASSERT((err_code == XINC_SUCCESS) || (err_code == XINC_ERROR_NO_MEM));
            err_code = rx_try(p_internal);
            ASSERT(err_code == XINC_SUCCESS);

            break;

        case XINC_DRV_UART_EVT_RX_READY:
        case XINC_DRV_UART_EVT_RX_DONE:
//						printf("R:%d\r\n",p_event->data.rxtx.bytes);
            err_code = xinc_ringbuf_put(p_internal->p_rx_ringbuf, p_event->data.rxtx.bytes);
            ASSERT((err_code == XINC_SUCCESS) || (err_code == XINC_ERROR_NO_MEM));

            if (p_event->data.rxtx.bytes)
            {
//                XINC_LOG_INFO("id:%d, evt: RXRDY len:%d",
//                             p_internal->p_uart->inst_idx,
//                             p_event->data.rxtx.bytes);
            //    XINC_LOG_HEXDUMP_DEBUG(p_event->data.rxtx.p_data, p_event->data.rxtx.bytes);
                p_internal->p_cb->handler(XINC_CLI_TRANSPORT_EVT_RX_RDY,
                                          p_internal->p_cb->p_context);
            }
            err_code = rx_try(p_internal);
            ASSERT(err_code == XINC_SUCCESS);

            break;

        case XINC_DRV_UART_EVT_TX_DONE:
            err_code = xinc_ringbuf_free(p_internal->p_tx_ringbuf, p_event->data.rxtx.bytes);
            ASSERT(err_code == XINC_SUCCESS);
            len = 255;
            err_code = xinc_ringbuf_get(p_internal->p_tx_ringbuf, &p_data, &len, true);
            ASSERT(err_code == XINC_SUCCESS);
            if (len)
            {
            //    XINC_LOG_INFO("id:%d, evt uart_tx, len:%d", p_internal->p_uart->inst_idx, len);
                err_code = xinc_drv_uart_tx(p_internal->p_uart, p_data, len);
            ASSERT(err_code == XINC_SUCCESS);
            }
            p_internal->p_cb->handler(XINC_CLI_TRANSPORT_EVT_TX_RDY, p_internal->p_cb->p_context);
//            XINC_LOG_INFO("id:%d, evt: TXRDY, len:%d",
//                         p_internal->p_uart->inst_idx,
//                         p_event->data.rxtx.bytes);
            break;

        default:
        //    XINC_LOG_ERROR("Unknown event");
            ASSERT(false);
    }
}

static void timer_handler(void * p_context)
{
    xinc_cli_uart_internal_t * p_internal = (xinc_cli_uart_internal_t *)p_context;
  //  XINC_LOG_DEBUG("id:%d, evt: Timeout", p_internal->p_uart->inst_idx);
    xinc_drv_uart_rx_abort(p_internal->p_uart);
}

static ret_code_t cli_uart_init(xinc_cli_transport_t const * p_transport,
                                void const *                p_config,
                                xinc_cli_transport_handler_t evt_handler,
                                void *                      p_context)
{
	
	printf("%s\r\n",__func__);
	XINC_LOG_INFO("cli_uart_init");
    xinc_cli_uart_internal_t * p_internal =
                                       CONTAINER_OF(p_transport,
                                                    xinc_cli_uart_internal_t,
                                                    transport);
    p_internal->p_cb->handler = evt_handler;
    p_internal->p_cb->p_context = p_context;
    p_internal->p_cb->timer_created = false;
    p_internal->p_cb->blocking      = false;

    xinc_drv_uart_config_t * p_uart_config = (xinc_drv_uart_config_t *)p_config;
    memcpy(&p_internal->p_cb->uart_config, p_uart_config, sizeof(xinc_drv_uart_config_t));
    p_uart_config->p_context = (void *)p_internal;
    ret_code_t err_code = xinc_drv_uart_init(p_internal->p_uart,
                                            p_uart_config,
                                            uart_event_handler);
    if (err_code == XINC_SUCCESS)
    {
        xinc_ringbuf_init(p_internal->p_rx_ringbuf);
        xinc_ringbuf_init(p_internal->p_tx_ringbuf);
    }
    return err_code;
}

static ret_code_t cli_uart_uninit(xinc_cli_transport_t const * p_transport)
{
    xinc_cli_uart_internal_t * p_internal =
                                       CONTAINER_OF(p_transport,
                                                    xinc_cli_uart_internal_t,
                                                    transport);

    xinc_drv_uart_uninit(p_internal->p_uart);

    return app_timer_stop(*p_internal->p_timer);
}

static ret_code_t cli_uart_enable(xinc_cli_transport_t const * p_transport,
                                 bool                         blocking)
{
    xinc_cli_uart_internal_t * p_internal =
                                       CONTAINER_OF(p_transport,
                                                    xinc_cli_uart_internal_t,
                                                    transport);
    ret_code_t err_code = XINC_SUCCESS;

    if (p_internal->p_cb->timer_created)
    {
        err_code = app_timer_stop(*p_internal->p_timer); //Timer may be running or inactive
        if ((err_code != XINC_SUCCESS) && (err_code != XINC_ERROR_INVALID_STATE))
        {
            return err_code;
        }
        else
        {
            err_code = XINC_SUCCESS;
        }
    }

    if (blocking)
    {
        xinc_drv_uart_uninit(p_internal->p_uart);
        err_code = xinc_drv_uart_init(p_internal->p_uart, &p_internal->p_cb->uart_config, NULL);
        if (err_code == XINC_SUCCESS)
        {
            p_internal->p_cb->blocking = true;
            return XINC_SUCCESS;
        }
        else
        {
            return XINC_ERROR_NOT_SUPPORTED;
        }
    }
    else
    {
        if (!p_internal->p_cb->timer_created)
        {
            err_code = app_timer_create(p_internal->p_timer,
                                        APP_TIMER_MODE_SINGLE_SHOT,
                                        timer_handler);
            p_internal->p_cb->timer_created = true;
					printf("app_timer_create:%d\r\n",err_code);
        }
        if (err_code == XINC_SUCCESS)
        {
            err_code = rx_try(p_internal);
        }
    }
    return err_code;
}

static ret_code_t cli_uart_read(xinc_cli_transport_t const * p_transport,
                                void *                      p_data,
                                size_t                      length,
                                size_t *                    p_cnt)
{
    ASSERT(p_cnt);
    xinc_cli_uart_internal_t * p_instance =
                                 CONTAINER_OF(p_transport, xinc_cli_uart_internal_t, transport);

    *p_cnt = length;
    ret_code_t err_code = xinc_ringbuf_cpy_get(p_instance->p_rx_ringbuf, p_data, p_cnt);
    
   // printf("cli_uart_read err_code:0x%x\r\n",err_code);

    if (*p_cnt)
    {
     //   XINC_LOG_INFO("id:%d, read:%d", p_instance->p_uart->inst_idx, *p_cnt);
			//	printf("id:%d, read:%d\r\n", p_instance->p_uart->inst_idx, *p_cnt);
    }

    return err_code;
}

static ret_code_t cli_uart_write(xinc_cli_transport_t const * p_transport,
                                 void const *                p_data,
                                 size_t                      length,
                                 size_t *                    p_cnt)
{
//	printf("cli_uart_write\r\n");
    ASSERT(p_cnt);
    xinc_cli_uart_internal_t * p_instance =
                                     CONTAINER_OF(p_transport, xinc_cli_uart_internal_t, transport);
    ret_code_t err_code;
    *p_cnt = length;
    err_code = xinc_ringbuf_cpy_put(p_instance->p_tx_ringbuf, p_data, p_cnt);
    if (err_code == XINC_SUCCESS)
    {
//        XINC_LOG_INFO("id:%d, write, req:%d, done:%d",
//                     p_instance->p_uart->inst_idx,
//                     length,
//                     *p_cnt);

        if (!xinc_drv_uart_tx_in_progress(p_instance->p_uart))
        {
            uint8_t * p_buf;
            size_t len = 255;
            if (xinc_ringbuf_get(p_instance->p_tx_ringbuf, &p_buf, &len, true) == XINC_SUCCESS)
            {
            //    XINC_LOG_INFO("id:%d, uart_tx, len:%d", p_instance->p_uart->inst_idx, len);

                err_code = xinc_drv_uart_tx(p_instance->p_uart, p_buf, len);
                if (p_instance->p_cb->blocking && (err_code == XINC_SUCCESS))
                {
                    (void)xinc_ringbuf_free(p_instance->p_tx_ringbuf, len);
                }
            }
        }
    }
    return err_code;
}

const xinc_cli_transport_api_t xinc_cli_uart_transport_api = {
        .init = cli_uart_init,
        .uninit = cli_uart_uninit,
        .enable = cli_uart_enable,
        .read = cli_uart_read,
        .write = cli_uart_write,
};

#endif
