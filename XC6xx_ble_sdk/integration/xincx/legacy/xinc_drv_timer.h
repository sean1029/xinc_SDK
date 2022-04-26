/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */


#ifndef XINC_DRV_TIMER_H__
#define XINC_DRV_TIMER_H__

#include <xincx_timer.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xinc_drv_timer TIMER driver - legacy layer
 * @{
 * @ingroup  xinc_timer
 *
 * @brief    Layer providing compatibility with the former API.
 */

/** @brief Type definition for forwarding the new implementation. */
typedef xincx_timer_t        xinc_drv_timer_t;
/** @brief Type definition for forwarding the new implementation. */
typedef xincx_timer_config_t xinc_drv_timer_config_t;

/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_TIMER_INSTANCE                   XINCX_TIMER_INSTANCE
/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_TIMER_DEFAULT_CONFIG             XINCX_TIMER_DEFAULT_CONFIG

/** @brief Macro for forwarding the new implementation. */
#define xinc_timer_event_handler_t                xincx_timer_event_handler_t

/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_timer_init                       xincx_timer_init
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_timer_uninit                     xincx_timer_uninit
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_timer_enable                     xincx_timer_enable
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_timer_disable                    xincx_timer_disable
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_timer_is_enabled                 xincx_timer_is_enabled
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_timer_clear                      xincx_timer_clear
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_timer_compare                    xincx_timer_compare
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_timer_us_to_ticks                xincx_timer_us_to_ticks
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_timer_ms_to_ticks                xincx_timer_ms_to_ticks
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_timer_compare_int_enable         xincx_timer_compare_int_enable
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_timer_compare_int_disable        xincx_timer_compare_int_disable

#define xinc_drv_timer_compare_cnt_get              xincx_timer_cnt_get 


/** @} */

#ifdef __cplusplus
}
#endif

#endif // XINC_DRV_TIMER_H__
