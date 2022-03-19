/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINC_DRV_PWM_H__
#define XINC_DRV_PWM_H__

#include <xincx_pwm.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xinc_drv_pwm PWM driver - legacy layer
 * @{
 * @ingroup  xinc_pwm
 *
 * @brief    @tagAPI52 Layer providing compatibility with the former API.
 */

/** @brief Type definition for forwarding the new implementation. */
typedef xincx_pwm_t          xinc_drv_pwm_t;
/** @brief Type definition for forwarding the new implementation. */
typedef xincx_pwm_config_t   xinc_drv_pwm_config_t;

/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_PWM_INSTANCE                    XINCX_PWM_INSTANCE
/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_PWM_PIN_NOT_USED                XINCX_PWM_PIN_NOT_USED
/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_PWM_PIN_INVERTED                XINCX_PWM_PIN_INVERTED
/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_PWM_DEFAULT_CONFIG              XINCX_PWM_DEFAULT_CONFIG


/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_pwm_init                        xincx_pwm_init
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_pwm_uninit                      xincx_pwm_uninit

/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_pwm_start                        xincx_pwm_start
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_pwm_stop                    			xincx_pwm_stop




/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_pwm_stop                        xincx_pwm_stop
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_pwm_is_stopped                  xincx_pwm_is_stopped
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_pwm_freq_update             xincx_pwm_freq_update
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_pwm_duty_cycle_update      xincx_pwm_duty_cycle_update

/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_pwm_freq_duty_cycle_update      xincx_pwm_freq_duty_cycl_update

/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_pwm_freq_valid_range_check      xincx_pwm_freq_valid_range_check



/** @} */

#ifdef __cplusplus
}
#endif

#endif // XINC_DRV_PWM_H__
