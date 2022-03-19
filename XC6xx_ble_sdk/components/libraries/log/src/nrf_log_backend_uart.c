/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include "sdk_common.h"
#if NRF_MODULE_ENABLED(NRF_LOG) && NRF_MODULE_ENABLED(NRF_LOG_BACKEND_UART)
#include "nrf_log_backend_uart.h"
#include "nrf_log_backend_serial.h"
#include "nrf_log_internal.h"
#include "xinc_drv_uart.h"
#include "app_error.h"

xinc_drv_uart_t m_uart = NRF_DRV_UART_INSTANCE(1);

static uint8_t m_string_buff[NRF_LOG_BACKEND_UART_TEMP_BUFFER_SIZE];
static volatile bool m_xfer_done;
static bool m_async_mode;
static void uart_evt_handler(xinc_drv_uart_event_t * p_event, void * p_context)
{
    m_xfer_done = true;
}

static void uart_init(bool async_mode)
{
    xinc_drv_uart_config_t config = NRF_DRV_UART_DEFAULT_CONFIG;
    config.pseltxd  = NRF_LOG_BACKEND_UART_TX_PIN;
    config.pselrxd  = NRF_LOG_BACKEND_UART_TX_PIN + 1;
    config.pselcts  = XINC_UART_PSEL_DISCONNECTED;
    config.pselrts  = XINC_UART_PSEL_DISCONNECTED;
		config.hwfc = XINC_UART_HWFC_DISABLED;
    config.baudrate = (xinc_uart_baudrate_t)UART_BAUDRATE_BAUDRATE_Baud115200;
	

    ret_code_t err_code = xinc_drv_uart_init(&m_uart, &config, async_mode ? uart_evt_handler : NULL);
    APP_ERROR_CHECK(err_code);

    m_async_mode = async_mode;
}

void nrf_log_backend_uart_init(void)
{
    bool async_mode = NRF_LOG_DEFERRED ? true : false;
    uart_init(async_mode);
}

static void serial_tx(void const * p_context, char const * p_buffer, size_t len)
{
    uint8_t len8 = (uint8_t)(len & 0x000000FF);
    m_xfer_done = false;
		
    ret_code_t err_code = xinc_drv_uart_tx(&m_uart, (uint8_t *)p_buffer, len8);
		
    APP_ERROR_CHECK(err_code);
    /* wait for completion since buffer is reused*/
    while (m_async_mode && (m_xfer_done == false))
    {

    }

}

static void nrf_log_backend_uart_put(nrf_log_backend_t const * p_backend,
                                     nrf_log_entry_t * p_msg)
{
    nrf_log_backend_serial_put(p_backend, p_msg, m_string_buff,
                               NRF_LOG_BACKEND_UART_TEMP_BUFFER_SIZE, serial_tx);
}

static void nrf_log_backend_uart_flush(nrf_log_backend_t const * p_backend)
{

}

static void nrf_log_backend_uart_panic_set(nrf_log_backend_t const * p_backend)
{
    xinc_drv_uart_uninit(&m_uart);

    uart_init(false);
}

const nrf_log_backend_api_t nrf_log_backend_uart_api = {
        .put       = nrf_log_backend_uart_put,
        .flush     = nrf_log_backend_uart_flush,
        .panic_set = nrf_log_backend_uart_panic_set,
};
#endif //NRF_MODULE_ENABLED(NRF_LOG) && NRF_MODULE_ENABLED(NRF_LOG_BACKEND_UART)
