/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#ifndef XINC_CLI_UART_H__
#define XINC_CLI_UART_H__

#include "xinc_cli.h"
#include "xinc_drv_uart.h"
#include "xinc_ringbuf.h"
#include "app_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**@file
 *
 * @defgroup xinc_cli_uart UART command line interface transport layer
 * @ingroup xinc_cli
 *
 * @{
 *
 */

/**
 * @brief Command line interface transport.
 */
extern const xinc_cli_transport_api_t xinc_cli_uart_transport_api;

typedef struct xinc_cli_uart_internal_s xinc_cli_uart_internal_t;

typedef struct {
    xinc_cli_transport_handler_t   handler;
    void *                        p_context;
    xinc_drv_uart_config_t         uart_config;
    bool                          timer_created;
    bool                          blocking;
} xinc_cli_uart_internal_cb_t;

struct xinc_cli_uart_internal_s {
    xinc_cli_transport_t          transport;
    xinc_cli_uart_internal_cb_t * p_cb;
    app_timer_id_t const *       p_timer;
    xinc_ringbuf_t const *        p_rx_ringbuf;
    xinc_ringbuf_t const *        p_tx_ringbuf;
    xinc_drv_uart_t const *       p_uart;
};

typedef xinc_drv_uart_config_t xinc_cli_uart_config_t;

/**@brief CLI UART transport definition.
 *
 * @param _name      Name of the instance.
 * @param _uart_id   UART instance ID.
 * @param _tx_buf_sz Size of TX ring buffer.
 * @param _rx_buf_sz Size of RX ring buffer.
 */
#define XINC_CLI_UART_DEF(_name, _uart_id, _tx_buf_sz, _rx_buf_sz)   \
    APP_TIMER_DEF(CONCAT_2(_name, _timer));                         \
    XINC_RINGBUF_DEF(CONCAT_2(_name,_tx_ringbuf), _tx_buf_sz);       \
    XINC_RINGBUF_DEF(CONCAT_2(_name,_rx_ringbuf), _rx_buf_sz);       \
    static const xinc_drv_uart_t CONCAT_2(_name,_uart) =             \
                                 XINC_DRV_UART_INSTANCE(_uart_id);   \
    static xinc_cli_uart_internal_cb_t CONCAT_2(_name, _cb);         \
    static const xinc_cli_uart_internal_t _name = {                  \
        .transport = {.p_api = &xinc_cli_uart_transport_api},        \
        .p_cb = &CONCAT_2(_name, _cb),                              \
        .p_timer = &CONCAT_2(_name, _timer),                        \
        .p_rx_ringbuf = &CONCAT_2(_name,_rx_ringbuf),               \
        .p_tx_ringbuf = &CONCAT_2(_name,_tx_ringbuf),               \
        .p_uart       = &CONCAT_2(_name,_uart),                     \
    }
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* XINC_CLI_UART_H__ */
