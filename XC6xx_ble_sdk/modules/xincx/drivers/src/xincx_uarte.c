/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#include <xincx.h>

#if XINCX_CHECK(XINCX_UARTE_ENABLED)

#if !(XINCX_CHECK(XINCX_UARTE0_ENABLED) || \
      XINCX_CHECK(XINCX_UARTE1_ENABLED) || \
      XINCX_CHECK(XINCX_UARTE2_ENABLED) || \
      XINCX_CHECK(XINCX_UARTE3_ENABLED))
#error "No enabled UARTE instances. Check <xincx_config.h>."
#endif

#include <xincx_uarte.h>
#include "prs/xincx_prs.h"
#include <hal/nrf_gpio.h>

#define XINCX_LOG_MODULE UARTE
#include <xincx_log.h>

#define EVT_TO_STR(event) \
    (event == XINC_UARTE_EVENT_ERROR ? "XINC_UARTE_EVENT_ERROR" : \
                                      "UNKNOWN EVENT")

#define UARTEX_LENGTH_VALIDATE(peripheral, drv_inst_idx, len1, len2)     \
    (((drv_inst_idx) == XINCX_CONCAT_3(XINCX_, peripheral, _INST_IDX)) && \
     XINCX_EASYDMA_LENGTH_VALIDATE(peripheral, len1, len2))

#if XINCX_CHECK(XINCX_UARTE0_ENABLED)
#define UARTE0_LENGTH_VALIDATE(...)  UARTEX_LENGTH_VALIDATE(UARTE0, __VA_ARGS__)
#else
#define UARTE0_LENGTH_VALIDATE(...)  0
#endif

#if XINCX_CHECK(XINCX_UARTE1_ENABLED)
#define UARTE1_LENGTH_VALIDATE(...)  UARTEX_LENGTH_VALIDATE(UARTE1, __VA_ARGS__)
#else
#define UARTE1_LENGTH_VALIDATE(...)  0
#endif

#if XINCX_CHECK(XINCX_UARTE2_ENABLED)
#define UARTE2_LENGTH_VALIDATE(...)  UARTEX_LENGTH_VALIDATE(UARTE2, __VA_ARGS__)
#else
#define UARTE2_LENGTH_VALIDATE(...)  0
#endif

#if XINCX_CHECK(XINCX_UARTE3_ENABLED)
#define UARTE3_LENGTH_VALIDATE(...)  UARTEX_LENGTH_VALIDATE(UARTE3, __VA_ARGS__)
#else
#define UARTE3_LENGTH_VALIDATE(...)  0
#endif

#define UARTE_LENGTH_VALIDATE(drv_inst_idx, length)     \
    (UARTE0_LENGTH_VALIDATE(drv_inst_idx, length, 0) || \
     UARTE1_LENGTH_VALIDATE(drv_inst_idx, length, 0) || \
     UARTE2_LENGTH_VALIDATE(drv_inst_idx, length, 0) || \
     UARTE3_LENGTH_VALIDATE(drv_inst_idx, length, 0))

typedef struct
{
    void                     * p_context;
    xincx_uarte_event_handler_t handler;
    uint8_t            const * p_tx_buffer;
    uint8_t                  * p_rx_buffer;
    uint8_t                  * p_rx_secondary_buffer;
    volatile size_t            tx_buffer_length;
    size_t                     rx_buffer_length;
    size_t                     rx_secondary_buffer_length;
    xincx_drv_state_t           state;
} uarte_control_block_t;
static uarte_control_block_t m_cb[XINCX_UARTE_ENABLED_COUNT];

static void apply_config(xincx_uarte_t        const * p_instance,
                         xincx_uarte_config_t const * p_config)
{
    if (p_config->pseltxd != XINC_UARTE_PSEL_DISCONNECTED)
    {
        nrf_gpio_pin_set(p_config->pseltxd);
        nrf_gpio_cfg_output(p_config->pseltxd);
    }
    if (p_config->pselrxd != XINC_UARTE_PSEL_DISCONNECTED)
    {
        nrf_gpio_cfg_input(p_config->pselrxd, XINC_GPIO_PIN_NOPULL);
    }

    nrf_uarte_baudrate_set(p_instance->p_reg, p_config->baudrate);
    nrf_uarte_configure(p_instance->p_reg, p_config->parity, p_config->hwfc);
    nrf_uarte_txrx_pins_set(p_instance->p_reg, p_config->pseltxd, p_config->pselrxd);
    if (p_config->hwfc == XINC_UARTE_HWFC_ENABLED)
    {
        if (p_config->pselcts != XINC_UARTE_PSEL_DISCONNECTED)
        {
            nrf_gpio_cfg_input(p_config->pselcts, XINC_GPIO_PIN_NOPULL);
        }
        if (p_config->pselrts != XINC_UARTE_PSEL_DISCONNECTED)
        {
            nrf_gpio_pin_set(p_config->pselrts);
            nrf_gpio_cfg_output(p_config->pselrts);
        }
        nrf_uarte_hwfc_pins_set(p_instance->p_reg, p_config->pselrts, p_config->pselcts);
    }
}

static void interrupts_enable(xincx_uarte_t const * p_instance,
                              uint8_t              interrupt_priority)
{
    nrf_uarte_event_clear(p_instance->p_reg, XINC_UARTE_EVENT_ENDRX);
    nrf_uarte_event_clear(p_instance->p_reg, XINC_UARTE_EVENT_ENDTX);
    nrf_uarte_event_clear(p_instance->p_reg, XINC_UARTE_EVENT_ERROR);
    nrf_uarte_event_clear(p_instance->p_reg, XINC_UARTE_EVENT_RXTO);
    nrf_uarte_event_clear(p_instance->p_reg, XINC_UARTE_EVENT_TXSTOPPED);
    nrf_uarte_int_enable(p_instance->p_reg, XINC_UARTE_INT_ENDRX_MASK |
                                            XINC_UARTE_INT_ENDTX_MASK |
                                            XINC_UARTE_INT_ERROR_MASK |
                                            XINC_UARTE_INT_RXTO_MASK  |
                                            XINC_UARTE_INT_TXSTOPPED_MASK);
    XINCX_IRQ_PRIORITY_SET(xincx_get_irq_number((void *)p_instance->p_reg),
                          interrupt_priority);
    XINCX_IRQ_ENABLE(xincx_get_irq_number((void *)p_instance->p_reg));
}

static void interrupts_disable(xincx_uarte_t const * p_instance)
{
    nrf_uarte_int_disable(p_instance->p_reg, XINC_UARTE_INT_ENDRX_MASK |
                                             XINC_UARTE_INT_ENDTX_MASK |
                                             XINC_UARTE_INT_ERROR_MASK |
                                             XINC_UARTE_INT_RXTO_MASK  |
                                             XINC_UARTE_INT_TXSTOPPED_MASK);
    XINCX_IRQ_DISABLE(xincx_get_irq_number((void *)p_instance->p_reg));
}

static void pins_to_default(xincx_uarte_t const * p_instance)
{
    /* Reset pins to default states */
    uint32_t txd;
    uint32_t rxd;
    uint32_t rts;
    uint32_t cts;

    txd = nrf_uarte_tx_pin_get(p_instance->p_reg);
    rxd = nrf_uarte_rx_pin_get(p_instance->p_reg);
    rts = nrf_uarte_rts_pin_get(p_instance->p_reg);
    cts = nrf_uarte_cts_pin_get(p_instance->p_reg);
    nrf_uarte_txrx_pins_disconnect(p_instance->p_reg);
    nrf_uarte_hwfc_pins_disconnect(p_instance->p_reg);

    if (txd != XINC_UARTE_PSEL_DISCONNECTED)
    {
        nrf_gpio_cfg_default(txd);
    }
    if (rxd != XINC_UARTE_PSEL_DISCONNECTED)
    {
        nrf_gpio_cfg_default(rxd);
    }
    if (cts != XINC_UARTE_PSEL_DISCONNECTED)
    {
        nrf_gpio_cfg_default(cts);
    }
    if (rts != XINC_UARTE_PSEL_DISCONNECTED)
    {
        nrf_gpio_cfg_default(rts);
    }
}

xincx_err_t xincx_uarte_init(xincx_uarte_t const *        p_instance,
                           xincx_uarte_config_t const * p_config,
                           xincx_uarte_event_handler_t  event_handler)
{
    XINCX_ASSERT(p_config);
    uarte_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    xincx_err_t err_code = XINCX_SUCCESS;

    if (p_cb->state != XINCX_DRV_STATE_UNINITIALIZED)
    {
        err_code = XINCX_ERROR_INVALID_STATE;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }

#if XINCX_CHECK(XINCX_PRS_ENABLED)
    static xincx_irq_handler_t const irq_handlers[XINCX_UARTE_ENABLED_COUNT] = {
        #if XINCX_CHECK(XINCX_UARTE0_ENABLED)
        xincx_uarte_0_irq_handler,
        #endif
        #if XINCX_CHECK(XINCX_UARTE1_ENABLED)
        xincx_uarte_1_irq_handler,
        #endif
        #if XINCX_CHECK(XINCX_UARTE2_ENABLED)
        xincx_uarte_2_irq_handler,
        #endif
        #if XINCX_CHECK(XINCX_UARTE3_ENABLED)
        xincx_uarte_3_irq_handler,
        #endif
    };
    if (xincx_prs_acquire(p_instance->p_reg,
            irq_handlers[p_instance->drv_inst_idx]) != XINCX_SUCCESS)
    {
        err_code = XINCX_ERROR_BUSY;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
#endif // XINCX_CHECK(XINCX_PRS_ENABLED)

    apply_config(p_instance, p_config);

#if defined(XINC5340_XXAA_APPLICATION) || defined(XINC5340_XXAA_NETWORK) || defined(XINC9160_XXAA)
    // Apply workaround for anomalies:
    // - nRF9160 - anomaly 23
    // - nRF5340 - anomaly 44
    volatile uint32_t const * rxenable_reg =
        (volatile uint32_t *)(((uint32_t)p_instance->p_reg) + 0x564);
    volatile uint32_t const * txenable_reg =
        (volatile uint32_t *)(((uint32_t)p_instance->p_reg) + 0x568);

    if (*txenable_reg == 1)
    {
        nrf_uarte_task_trigger(p_instance->p_reg, XINC_UARTE_TASK_STOPTX);
    }

    if (*rxenable_reg == 1)
    {
        nrf_uarte_enable(p_instance->p_reg);
        nrf_uarte_task_trigger(p_instance->p_reg, XINC_UARTE_TASK_STOPRX);

        while (*rxenable_reg)
        {}

        (void)nrf_uarte_errorsrc_get_and_clear(p_instance->p_reg);
        nrf_uarte_disable(p_instance->p_reg);
    }
#endif // defined(XINC5340_XXAA_APPLICATION) || defined(XINC5340_XXAA_NETWORK) || defined(XINC9160_XXAA)

    p_cb->handler   = event_handler;
    p_cb->p_context = p_config->p_context;

    if (p_cb->handler)
    {
        interrupts_enable(p_instance, p_config->interrupt_priority);
    }

    nrf_uarte_enable(p_instance->p_reg);
    p_cb->rx_buffer_length           = 0;
    p_cb->rx_secondary_buffer_length = 0;
    p_cb->tx_buffer_length           = 0;
    p_cb->state                      = XINCX_DRV_STATE_INITIALIZED;
    XINCX_LOG_WARNING("Function: %s, error code: %s.",
                     __func__,
                     XINCX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}

void xincx_uarte_uninit(xincx_uarte_t const * p_instance)
{
    uarte_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    XINC_UARTE_Type * p_reg = p_instance->p_reg;

    if (p_cb->handler)
    {
        interrupts_disable(p_instance);
    }
    // Make sure all transfers are finished before UARTE is disabled
    // to achieve the lowest power consumption.
    nrf_uarte_shorts_disable(p_reg, XINC_UARTE_SHORT_ENDRX_STARTRX);

    // Check if there is any ongoing reception.
    if (p_cb->rx_buffer_length)
    {
        nrf_uarte_event_clear(p_reg, XINC_UARTE_EVENT_RXTO);
        nrf_uarte_task_trigger(p_reg, XINC_UARTE_TASK_STOPRX);
    }

    nrf_uarte_event_clear(p_reg, XINC_UARTE_EVENT_TXSTOPPED);
    nrf_uarte_task_trigger(p_reg, XINC_UARTE_TASK_STOPTX);

    // Wait for TXSTOPPED event and for RXTO event, provided that there was ongoing reception.
    while (!nrf_uarte_event_check(p_reg, XINC_UARTE_EVENT_TXSTOPPED) ||
           (p_cb->rx_buffer_length && !nrf_uarte_event_check(p_reg, XINC_UARTE_EVENT_RXTO)))
    {}

    nrf_uarte_disable(p_reg);
    pins_to_default(p_instance);

#if XINCX_CHECK(XINCX_PRS_ENABLED)
    xincx_prs_release(p_reg);
#endif

    p_cb->state   = XINCX_DRV_STATE_UNINITIALIZED;
    p_cb->handler = NULL;
    XINCX_LOG_INFO("Instance uninitialized: %d.", p_instance->drv_inst_idx);
}

xincx_err_t xincx_uarte_tx(xincx_uarte_t const * p_instance,
                         uint8_t const *      p_data,
                         size_t               length)
{
    uarte_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    XINCX_ASSERT(p_cb->state == XINCX_DRV_STATE_INITIALIZED);
    XINCX_ASSERT(p_data);
    XINCX_ASSERT(length > 0);
    XINCX_ASSERT(UARTE_LENGTH_VALIDATE(p_instance->drv_inst_idx, length));

    xincx_err_t err_code;

    // EasyDMA requires that transfer buffers are placed in DataRAM,
    // signal error if the are not.
    if (!xincx_is_in_ram(p_data))
    {
        err_code = XINCX_ERROR_INVALID_ADDR;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }

    if (xincx_uarte_tx_in_progress(p_instance))
    {
        err_code = XINCX_ERROR_BUSY;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
    p_cb->tx_buffer_length = length;
    p_cb->p_tx_buffer      = p_data;

    XINCX_LOG_INFO("Transfer tx_len: %d.", p_cb->tx_buffer_length);
    XINCX_LOG_DEBUG("Tx data:");
    XINCX_LOG_HEXDUMP_DEBUG(p_cb->p_tx_buffer,
                           p_cb->tx_buffer_length * sizeof(p_cb->p_tx_buffer[0]));

    err_code = XINCX_SUCCESS;

    nrf_uarte_event_clear(p_instance->p_reg, XINC_UARTE_EVENT_ENDTX);
    nrf_uarte_event_clear(p_instance->p_reg, XINC_UARTE_EVENT_TXSTOPPED);
    nrf_uarte_tx_buffer_set(p_instance->p_reg, p_cb->p_tx_buffer, p_cb->tx_buffer_length);
    nrf_uarte_task_trigger(p_instance->p_reg, XINC_UARTE_TASK_STARTTX);

    if (p_cb->handler == NULL)
    {
        bool endtx;
        bool txstopped;
        do
        {
            endtx     = nrf_uarte_event_check(p_instance->p_reg, XINC_UARTE_EVENT_ENDTX);
            txstopped = nrf_uarte_event_check(p_instance->p_reg, XINC_UARTE_EVENT_TXSTOPPED);
        }
        while ((!endtx) && (!txstopped));

        if (txstopped)
        {
            err_code = XINCX_ERROR_FORBIDDEN;
        }
        else
        {
            // Transmitter has to be stopped by triggering the STOPTX task to achieve
            // the lowest possible level of the UARTE power consumption.
            nrf_uarte_task_trigger(p_instance->p_reg, XINC_UARTE_TASK_STOPTX);

            while (!nrf_uarte_event_check(p_instance->p_reg, XINC_UARTE_EVENT_TXSTOPPED))
            {}
        }
        p_cb->tx_buffer_length = 0;
    }

    XINCX_LOG_INFO("Function: %s, error code: %s.", __func__, XINCX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}

bool xincx_uarte_tx_in_progress(xincx_uarte_t const * p_instance)
{
    return (m_cb[p_instance->drv_inst_idx].tx_buffer_length != 0);
}

xincx_err_t xincx_uarte_rx(xincx_uarte_t const * p_instance,
                         uint8_t *            p_data,
                         size_t               length)
{
    uarte_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];

    XINCX_ASSERT(m_cb[p_instance->drv_inst_idx].state == XINCX_DRV_STATE_INITIALIZED);
    XINCX_ASSERT(p_data);
    XINCX_ASSERT(length > 0);
    XINCX_ASSERT(UARTE_LENGTH_VALIDATE(p_instance->drv_inst_idx, length));

    xincx_err_t err_code;

    // EasyDMA requires that transfer buffers are placed in DataRAM,
    // signal error if the are not.
    if (!xincx_is_in_ram(p_data))
    {
        err_code = XINCX_ERROR_INVALID_ADDR;
        XINCX_LOG_WARNING("Function: %s, error code: %s.",
                         __func__,
                         XINCX_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }

    bool second_buffer = false;

    if (p_cb->handler)
    {
        nrf_uarte_int_disable(p_instance->p_reg, XINC_UARTE_INT_ERROR_MASK |
                                                 XINC_UARTE_INT_ENDRX_MASK);
    }
    if (p_cb->rx_buffer_length != 0)
    {
        if (p_cb->rx_secondary_buffer_length != 0)
        {
            if (p_cb->handler)
            {
                nrf_uarte_int_enable(p_instance->p_reg, XINC_UARTE_INT_ERROR_MASK |
                                                        XINC_UARTE_INT_ENDRX_MASK);
            }
            err_code = XINCX_ERROR_BUSY;
            XINCX_LOG_WARNING("Function: %s, error code: %s.",
                             __func__,
                             XINCX_LOG_ERROR_STRING_GET(err_code));
            return err_code;
        }
        second_buffer = true;
    }

    if (!second_buffer)
    {
        p_cb->rx_buffer_length = length;
        p_cb->p_rx_buffer      = p_data;
        p_cb->rx_secondary_buffer_length = 0;
    }
    else
    {
        p_cb->p_rx_secondary_buffer = p_data;
        p_cb->rx_secondary_buffer_length = length;
    }

    XINCX_LOG_INFO("Transfer rx_len: %d.", length);

    err_code = XINCX_SUCCESS;

    nrf_uarte_event_clear(p_instance->p_reg, XINC_UARTE_EVENT_ENDRX);
    nrf_uarte_event_clear(p_instance->p_reg, XINC_UARTE_EVENT_RXTO);
    nrf_uarte_rx_buffer_set(p_instance->p_reg, p_data, length);
    if (!second_buffer)
    {
        nrf_uarte_task_trigger(p_instance->p_reg, XINC_UARTE_TASK_STARTRX);
    }
    else
    {
        nrf_uarte_shorts_enable(p_instance->p_reg, XINC_UARTE_SHORT_ENDRX_STARTRX);
    }

    if (m_cb[p_instance->drv_inst_idx].handler == NULL)
    {
        bool endrx;
        bool rxto;
        bool error;
        do {
            endrx  = nrf_uarte_event_check(p_instance->p_reg, XINC_UARTE_EVENT_ENDRX);
            rxto   = nrf_uarte_event_check(p_instance->p_reg, XINC_UARTE_EVENT_RXTO);
            error  = nrf_uarte_event_check(p_instance->p_reg, XINC_UARTE_EVENT_ERROR);
        } while ((!endrx) && (!rxto) && (!error));

        m_cb[p_instance->drv_inst_idx].rx_buffer_length = 0;

        if (error)
        {
            err_code = XINCX_ERROR_INTERNAL;
        }

        if (rxto)
        {
            err_code = XINCX_ERROR_FORBIDDEN;
        }
    }
    else
    {
        nrf_uarte_int_enable(p_instance->p_reg, XINC_UARTE_INT_ERROR_MASK |
                                                XINC_UARTE_INT_ENDRX_MASK);
    }
    XINCX_LOG_INFO("Function: %s, error code: %s.", __func__, XINCX_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}

bool xincx_uarte_rx_ready(xincx_uarte_t const * p_instance)
{
    return nrf_uarte_event_check(p_instance->p_reg, XINC_UARTE_EVENT_ENDRX);
}

uint32_t xincx_uarte_errorsrc_get(xincx_uarte_t const * p_instance)
{
    nrf_uarte_event_clear(p_instance->p_reg, XINC_UARTE_EVENT_ERROR);
    return nrf_uarte_errorsrc_get_and_clear(p_instance->p_reg);
}

static void rx_done_event(uarte_control_block_t * p_cb,
                          size_t                  bytes,
                          uint8_t *               p_data)
{
    xincx_uarte_event_t event;

    event.type             = XINCX_UARTE_EVT_RX_DONE;
    event.data.rxtx.bytes  = bytes;
    event.data.rxtx.p_data = p_data;

    p_cb->handler(&event, p_cb->p_context);
}

static void tx_done_event(uarte_control_block_t * p_cb,
                          size_t                  bytes)
{
    xincx_uarte_event_t event;

    event.type             = XINCX_UARTE_EVT_TX_DONE;
    event.data.rxtx.bytes  = bytes;
    event.data.rxtx.p_data = (uint8_t *)p_cb->p_tx_buffer;

    p_cb->tx_buffer_length = 0;

    p_cb->handler(&event, p_cb->p_context);
}

void xincx_uarte_tx_abort(xincx_uarte_t const * p_instance)
{
    uarte_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];

    nrf_uarte_event_clear(p_instance->p_reg, XINC_UARTE_EVENT_TXSTOPPED);
    nrf_uarte_task_trigger(p_instance->p_reg, XINC_UARTE_TASK_STOPTX);
    if (p_cb->handler == NULL)
    {
        while (!nrf_uarte_event_check(p_instance->p_reg, XINC_UARTE_EVENT_TXSTOPPED))
        {}
    }
    XINCX_LOG_INFO("TX transaction aborted.");
}

void xincx_uarte_rx_abort(xincx_uarte_t const * p_instance)
{
    uarte_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];

    // Short between ENDRX event and STARTRX task must be disabled before
    // aborting transmission.
    if (p_cb->rx_secondary_buffer_length != 0)
    {
        nrf_uarte_shorts_disable(p_instance->p_reg, XINC_UARTE_SHORT_ENDRX_STARTRX);
    }
    nrf_uarte_task_trigger(p_instance->p_reg, XINC_UARTE_TASK_STOPRX);
    XINCX_LOG_INFO("RX transaction aborted.");
}

static void uarte_irq_handler(XINC_UARTE_Type *        p_uarte,
                              uarte_control_block_t * p_cb)
{
    if (nrf_uarte_event_check(p_uarte, XINC_UARTE_EVENT_ERROR))
    {
        xincx_uarte_event_t event;

        nrf_uarte_event_clear(p_uarte, XINC_UARTE_EVENT_ERROR);

        event.type                   = XINCX_UARTE_EVT_ERROR;
        event.data.error.error_mask  = nrf_uarte_errorsrc_get_and_clear(p_uarte);
        event.data.error.rxtx.bytes  = nrf_uarte_rx_amount_get(p_uarte);
        event.data.error.rxtx.p_data = p_cb->p_rx_buffer;

        // Abort transfer.
        p_cb->rx_buffer_length = 0;
        p_cb->rx_secondary_buffer_length = 0;

        p_cb->handler(&event, p_cb->p_context);
    }
    else if (nrf_uarte_event_check(p_uarte, XINC_UARTE_EVENT_ENDRX))
    {
        nrf_uarte_event_clear(p_uarte, XINC_UARTE_EVENT_ENDRX);
        size_t amount = nrf_uarte_rx_amount_get(p_uarte);
        // If the transfer was stopped before completion, amount of transfered bytes
        // will not be equal to the buffer length. Interrupted transfer is ignored.
        if (amount == p_cb->rx_buffer_length)
        {
            if (p_cb->rx_secondary_buffer_length != 0)
            {
                uint8_t * p_data = p_cb->p_rx_buffer;
                nrf_uarte_shorts_disable(p_uarte, XINC_UARTE_SHORT_ENDRX_STARTRX);
                p_cb->rx_buffer_length = p_cb->rx_secondary_buffer_length;
                p_cb->p_rx_buffer = p_cb->p_rx_secondary_buffer;
                p_cb->rx_secondary_buffer_length = 0;
                rx_done_event(p_cb, amount, p_data);
            }
            else
            {
                p_cb->rx_buffer_length = 0;
                rx_done_event(p_cb, amount, p_cb->p_rx_buffer);
            }
        }
    }

    if (nrf_uarte_event_check(p_uarte, XINC_UARTE_EVENT_RXTO))
    {
        nrf_uarte_event_clear(p_uarte, XINC_UARTE_EVENT_RXTO);

        if (p_cb->rx_buffer_length != 0)
        {
            p_cb->rx_buffer_length = 0;
            // In case of using double-buffered reception both variables storing buffer length
            // have to be cleared to prevent incorrect behaviour of the driver.
            p_cb->rx_secondary_buffer_length = 0;
            rx_done_event(p_cb, nrf_uarte_rx_amount_get(p_uarte), p_cb->p_rx_buffer);
        }
    }

    if (nrf_uarte_event_check(p_uarte, XINC_UARTE_EVENT_ENDTX))
    {
        nrf_uarte_event_clear(p_uarte, XINC_UARTE_EVENT_ENDTX);

        // Transmitter has to be stopped by triggering STOPTX task to achieve
        // the lowest possible level of the UARTE power consumption.
        nrf_uarte_task_trigger(p_uarte, XINC_UARTE_TASK_STOPTX);

        if (p_cb->tx_buffer_length != 0)
        {
            tx_done_event(p_cb, nrf_uarte_tx_amount_get(p_uarte));
        }
    }

    if (nrf_uarte_event_check(p_uarte, XINC_UARTE_EVENT_TXSTOPPED))
    {
        nrf_uarte_event_clear(p_uarte, XINC_UARTE_EVENT_TXSTOPPED);
        if (p_cb->tx_buffer_length != 0)
        {
            tx_done_event(p_cb, nrf_uarte_tx_amount_get(p_uarte));
        }
    }
}

#if XINCX_CHECK(XINCX_UARTE0_ENABLED)
void xincx_uarte_0_irq_handler(void)
{
    uarte_irq_handler(XINC_UARTE0, &m_cb[XINCX_UARTE0_INST_IDX]);
}
#endif

#if XINCX_CHECK(XINCX_UARTE1_ENABLED)
void xincx_uarte_1_irq_handler(void)
{
    uarte_irq_handler(XINC_UARTE1, &m_cb[XINCX_UARTE1_INST_IDX]);
}
#endif

#if XINCX_CHECK(XINCX_UARTE2_ENABLED)
void xincx_uarte_2_irq_handler(void)
{
    uarte_irq_handler(XINC_UARTE2, &m_cb[XINCX_UARTE2_INST_IDX]);
}
#endif

#if XINCX_CHECK(XINCX_UARTE3_ENABLED)
void xincx_uarte_3_irq_handler(void)
{
    uarte_irq_handler(XINC_UARTE3, &m_cb[XINCX_UARTE3_INST_IDX]);
}
#endif

#endif // XINCX_CHECK(XINCX_UARTE_ENABLED)
