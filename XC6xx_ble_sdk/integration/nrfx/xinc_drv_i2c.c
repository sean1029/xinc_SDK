/**
 * Copyright (c) 2017 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "xinc_drv_i2c.h"

#if NRFX_CHECK(XINCX_I2C_ENABLED)
//#include <nrf_delay.h>
#include <hal/nrf_gpio.h>

#ifdef I2CM_PRESENT
#define INSTANCE_COUNT   I2CM_COUNT
#else
#define INSTANCE_COUNT   I2C_COUNT
#endif

#define SCL_PIN_INIT_CONF                                     \
    ( (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) \
    | (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos) \
    | (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)  \
    | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos) \
    | (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos))

#define SDA_PIN_INIT_CONF        SCL_PIN_INIT_CONF

#define SDA_PIN_UNINIT_CONF                                     \
    ( (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) \
    | (GPIO_PIN_CNF_DRIVE_H0H1       << GPIO_PIN_CNF_DRIVE_Pos) \
    | (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  \
    | (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) \
    | (GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos))

#define SCL_PIN_UNINIT_CONF      SDA_PIN_UNINIT_CONF

#define SCL_PIN_INIT_CONF_CLR                                 \
    ( (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) \
    | (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos) \
    | (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)  \
    | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos) \
    | (GPIO_PIN_CNF_DIR_Output     << GPIO_PIN_CNF_DIR_Pos))

#define SDA_PIN_INIT_CONF_CLR    SCL_PIN_INIT_CONF_CLR

static xinc_drv_i2c_evt_handler_t m_handlers[INSTANCE_COUNT];
static void *                    m_contexts[INSTANCE_COUNT];

static void i2c_clear_bus(xinc_drv_i2c_config_t const * p_config)
{
//    NRF_GPIO->PIN_CNF[p_config->scl] = SCL_PIN_INIT_CONF;
//    NRF_GPIO->PIN_CNF[p_config->sda] = SDA_PIN_INIT_CONF;

//    nrf_gpio_pin_set(p_config->scl);
//    nrf_gpio_pin_set(p_config->sda);

//    NRF_GPIO->PIN_CNF[p_config->scl] = SCL_PIN_INIT_CONF_CLR;
//    NRF_GPIO->PIN_CNF[p_config->sda] = SDA_PIN_INIT_CONF_CLR;

//    nrf_delay_us(4);

//    for (int i = 0; i < 9; i++)
//    {
//        if (nrf_gpio_pin_read(p_config->sda))
//        {
//            if (i == 0)
//            {
//                return;
//            }
//            else
//            {
//                break;
//            }
//        }
//        nrf_gpio_pin_clear(p_config->scl);
//        nrf_delay_us(4);
//        nrf_gpio_pin_set(p_config->scl);
//        nrf_delay_us(4);
//    }
//    nrf_gpio_pin_clear(p_config->sda);
//    nrf_delay_us(4);
//    nrf_gpio_pin_set(p_config->sda);
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
    if (NRF_DRV_I2C_USE_I2CM)
    {
        result = xincx_i2cm_init(&p_instance->u.i2cm,
                                (xincx_i2cm_config_t const *)p_config,
                                event_handler ? i2cm_evt_handler : NULL,
                                (void *)inst_idx);
    }
    else if (NRF_DRV_I2C_USE_I2C)
    {
        result = xincx_i2c_init(&p_instance->u.i2c,
                               (xincx_i2c_config_t const *)p_config,
                               event_handler ? i2c_evt_handler : NULL,
                               (void *)inst_idx);
    }
    return result;
}
#endif //

