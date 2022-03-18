/**
 * Copyright (c) 2015 - 2020, Nordic Semiconductor ASA
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

#ifndef XINCX_GPIO_H__
#define XINCX_GPIO_H__

#if 1
//#define  GPIOTE_CH_NUM 8 //nrf52840_peripherals.h
#include <nrfx.h>
#include <hal/xinc_gpio.h>
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xincx_gpio GPIO driver
 * @{
 * @ingroup xinc_gpio
 * @brief   GPIO  peripheral driver.
 */

/** @brief Input pin configuration. */
typedef struct
{
  //  xinc_gpio_polarity_t sense;               /**< Transition that triggers the interrupt. */
    xinc_gpio_pin_pull_t   pull;                /**< Pulling mode. */
    bool                  is_watcher      : 1; /**< True when the input pin is tracking an output pin. */
    bool                  hi_accuracy     : 1; /**< True when high accuracy (IN_EVENT) is used. */
    bool                  skip_gpio_setup : 1; /**< Do not change GPIO configuration */
} xincx_gpio_in_config_t;

/**
 * @brief Macro for configuring a pin to use a GPIO IN or PORT EVENT to detect low-to-high transition.
 * @details Set hi_accu to true to use IN_EVENT.
 */


/** @brief Output pin configuration. */
typedef struct
{
//    xinc_gpio_polarity_t action;     /**< Configuration of the pin task. */
    xinc_gpio_outinit_t  init_state; /**< Initial state of the output pin. */
    bool                  task_pin;   /**< True if the pin is controlled by a GPIOTE task. */
} xincx_gpio_out_config_t;


/** @brief Pin. */
typedef uint32_t xincx_gpio_pin_t;

/**
 * @brief Pin event handler prototype.
 *
 * @param[in] pin    Pin that triggered this event.
 * @param[in] action Action that led to triggering this event.
 */
typedef void (*xincx_gpio_evt_handler_t)(xincx_gpio_pin_t pin, xinc_gpio_polarity_t action);

/**
 * @brief Function for initializing the GPIOTE module.
 *
 * @details Only static configuration is supported to prevent the shared
 * resource being customized by the initiator.
 *
 * @retval NRFX_SUCCESS             Initialization was successful.
 * @retval NRFX_ERROR_INVALID_STATE The driver was already initialized.
 */
nrfx_err_t xincx_gpio_init(void);

/**
 * @brief Function for checking if the GPIOTE module is initialized.
 *
 * @details The GPIOTE module is a shared module. Therefore, check if
 * the module is already initialized and skip initialization if it is.
 *
 * @retval true  The module is already initialized.
 * @retval false The module is not initialized.
 */
bool xincx_gpio_is_init(void);

/** @brief Function for uninitializing the GPIOTE module. */
void xincx_gpio_uninit(void);

/**
 * @brief Function for initializing a GPIOTE output pin.
 * @details The output pin can be controlled by the CPU or by PPI. The initial
 * configuration specifies which mode is used. If PPI mode is used, the driver
 * attempts to allocate one of the available GPIOTE channels. If no channel is
 * available, an error is returned.
 *
 * @param[in] pin      Pin.
 * @param[in] p_config Initial configuration.
 *
 * @retval NRFX_SUCCESS             Initialization was successful.
 * @retval NRFX_ERROR_INVALID_STATE The driver is not initialized or the pin is already used.
 * @retval NRFX_ERROR_NO_MEM        No GPIOTE channel is available.
 */
nrfx_err_t xincx_gpio_out_init(xincx_gpio_pin_t                pin,
                                xincx_gpio_out_config_t const * p_config);

/**
 * @brief Function for uninitializing a GPIOTE output pin.
 * @details The driver frees the GPIOTE channel if the output pin was using one.
 *
 * @param[in] pin Pin.
 */
void xincx_gpio_out_uninit(xincx_gpio_pin_t pin);

/**
 * @brief Function for setting a GPIOTE output pin.
 *
 * @param[in] pin Pin.
 */
void xincx_gpio_out_set(xincx_gpio_pin_t pin);

/**
 * @brief Function for clearing a GPIOTE output pin.
 *
 * @param[in] pin Pin.
 */
void xincx_gpio_out_clear(xincx_gpio_pin_t pin);

/**
 * @brief Function for toggling a GPIOTE output pin.
 *
 * @param[in] pin Pin.
 */
void xincx_gpio_out_toggle(xincx_gpio_pin_t pin);


/**
 * @brief Function for initializing a GPIOTE input pin.
 * @details The input pin can act in two ways:
 * - lower accuracy but low power (high frequency clock not needed)
 * - higher accuracy (high frequency clock required)
 *
 * The initial configuration specifies which mode is used.
 * If high-accuracy mode is used, the driver attempts to allocate one
 * of the available GPIOTE channels. If no channel is
 * available, an error is returned.
 * In low accuracy mode SENSE feature is used. In this case, only one active pin
 * can be detected at a time. It can be worked around by setting all of the used
 * low accuracy pins to toggle mode.
 * For more information about SENSE functionality, refer to Product Specification.
 *
 * @param[in] pin         Pin.
 * @param[in] p_config    Initial configuration.
 * @param[in] evt_handler User function to be called when the configured transition occurs.
 *
 * @retval NRFX_SUCCESS             Initialization was successful.
 * @retval NRFX_ERROR_INVALID_STATE The driver is not initialized or the pin is already used.
 * @retval NRFX_ERROR_NO_MEM        No GPIOTE channel is available.
 */
nrfx_err_t xincx_gpio_in_init(xincx_gpio_pin_t               pin,
                               xincx_gpio_in_config_t const * p_config,
                               xincx_gpio_evt_handler_t       evt_handler);

/**
 * @brief Function for uninitializing a GPIOTE input pin.
 * @details The driver frees the GPIOTE channel if the input pin was using one.
 *
 * @param[in] pin Pin.
 */
void xincx_gpio_in_uninit(xincx_gpio_pin_t pin);


/**
 * @brief Function for checking if a GPIOTE input pin is set.
 *
 * @param[in] pin Pin.
 *
 * @retval true  The input pin is set.
 * @retval false The input pin is not set.
 */
bool xincx_gpio_in_is_set(xincx_gpio_pin_t pin);




/** @} */


void xincx_gpio_irq_handler(void);


#ifdef __cplusplus
}
#endif
#endif

#endif // XINCX_GPIO_H__
