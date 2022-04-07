/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include "bsp.h"
#include "xinc_drv_spi.h"
#if XINCX_CHECK(XINCX_SPIM_ENABLED)
#ifdef SPIM_PRESENT
#define INSTANCE_COUNT   SPIM_COUNT
#endif

static xinc_drv_spi_evt_handler_t m_handlers[INSTANCE_COUNT];
static void *                    m_contexts[INSTANCE_COUNT];

#ifdef SPIM_PRESENT
static void spim_evt_handler(xincx_spim_evt_t   const * p_event,
                             void *                  p_context)
{
    uint32_t inst_idx = (uint32_t)p_context;
    xinc_drv_spi_evt_t event =
    {
        .type = (xinc_drv_spi_evt_type_t)p_event->type,
        .data =
        {
            .done =
            {
                .p_tx_buffer = p_event->xfer_desc.p_tx_buffer,
                .tx_length   = p_event->xfer_desc.tx_length,
                .p_rx_buffer = p_event->xfer_desc.p_rx_buffer,
                .rx_length   = p_event->xfer_desc.rx_length,
            }
        }
    };
    m_handlers[inst_idx](&event, m_contexts[inst_idx]);
}
#endif // SPIM_PRESENT


ret_code_t xinc_drv_spi_init(xinc_drv_spi_t const * const p_instance,
                            xinc_drv_spi_config_t const * p_config,
                            xinc_drv_spi_evt_handler_t    handler,
                            void *                       p_context)
{
    uint32_t inst_idx = p_instance->inst_idx;
    m_handlers[inst_idx] = handler;
    m_contexts[inst_idx] = p_context;

    ret_code_t result = 0;
    if (XINC_DRV_SPI_USE_SPIM)
    {
#ifdef SPIM_PRESENT
        xincx_spim_config_t config_spim = XINCX_SPIM_DEFAULT_CONFIG;
        config_spim.sck_pin        = p_config->sck_pin;
        config_spim.mosi_pin       = p_config->mosi_pin;
        config_spim.miso_pin       = p_config->miso_pin;
        config_spim.ss_pin         = p_config->ss_pin;
        #if defined (XC66XX_M4) && XINCX_CHECK(XINCX_SPIM2_ENABLED)
        config_spim.d2_pin             = p_config->d2_pin;
        config_spim.d2_pin             = p_config->d3_pin;
        #endif
        config_spim.irq_priority   = p_config->irq_priority;
        config_spim.orc            = p_config->orc;
        config_spim.frequency      = (xinc_spim_frequency_t)p_config->frequency;
        config_spim.mode           = (xinc_spim_mode_t)p_config->mode;
        config_spim.bit_order      = (xinc_spim_bit_order_t)p_config->bit_order;
        result = xincx_spim_init(&p_instance->u.spim,
                                &config_spim,
                                handler ? spim_evt_handler : NULL,//spim_evt_handler
                                (void *)inst_idx);
#endif
    }
    return result;
}
#endif //

