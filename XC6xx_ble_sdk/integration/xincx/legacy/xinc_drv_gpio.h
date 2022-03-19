/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#ifndef XINC_DRV_GPIOTE_H__
#define XINC_DRV_GPIOTE_H__

#include <xincx_gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xinc_drv_gpio GPIOTE driver - legacy layer
 * @{
 * @ingroup xinc_gpio
 * @brief Layer providing compatibility with the former API.
 */

/** @brief Type definition for forwarding the new implementation. */
typedef xincx_gpio_in_config_t xinc_drv_gpio_in_config_t;
/** @brief Type definition for forwarding the new implementation. */
typedef xincx_gpio_pin_t xinc_drv_gpio_pin_t;
/** @brief Type definition for forwarding the new implementation. */
typedef xincx_gpio_out_config_t xinc_drv_gpio_out_config_t;
/** @brief Type definition for forwarding the new implementation. */
typedef xincx_gpio_evt_handler_t xinc_drv_gpio_evt_handler_t;

/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_gpio_init               xincx_gpio_init
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_gpio_is_init            xincx_gpio_is_init
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_gpio_uninit             xincx_gpio_uninit
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_gpio_out_init           xincx_gpio_out_init
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_gpio_out_uninit         xincx_gpio_out_uninit
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_gpio_out_set            xincx_gpio_out_set
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_gpio_out_clear          xincx_gpio_out_clear
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_gpio_out_toggle         xincx_gpio_out_toggle
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_gpio_in_init            xincx_gpio_in_init
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_gpio_in_uninit          xincx_gpio_in_uninit
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_gpio_in_is_set          xincx_gpio_in_is_set


/** @} */

#ifdef __cplusplus
}
#endif

#endif //XINC_DRV_GPIOTE_H__
