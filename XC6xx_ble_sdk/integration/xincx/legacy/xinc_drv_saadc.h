/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINC_DRV_SAADC_H__
#define XINC_DRV_SAADC_H__

#include <xincx_saadc.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup nrf_drv_saadc SAADC driver - legacy layer
 * @{
 * @ingroup  xinc_saadc
 *
 * @brief    @tagAPI52 Layer providing compatibility with the former API.
 */
/** @brief Type definition for forwarding the new implementation. */
typedef xincx_saadc_t        xinc_drv_saadc_t;
/** @brief Type definition for forwarding the new implementation. */
typedef xincx_saadc_config_t xinc_drv_saadc_config_t;

/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_SAADC_INSTANCE          XINCX_SAADC_INSTANCE

/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_SAADC_EVT_DONE          XINCX_SAADC_EVT_DONE
/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_SAADC_EVT_LIMIT         XINCX_SAADC_EVT_LIMIT
/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_SAADC_EVT_CALIBRATEDONE XINCX_SAADC_EVT_CALIBRATEDONE
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_saadc_evt_type_t        xincx_saadc_evt_type_t
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_saadc_done_evt_t        xincx_saadc_done_evt_t
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_saadc_limit_evt_t       xincx_saadc_limit_evt_t
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_saadc_evt_t             xincx_saadc_evt_t
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_saadc_event_handler_t   xincx_saadc_event_handler_t

/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_SAADC_DEFAULT_CONFIG    XINCX_SAADC_DEFAULT_CONFIG
/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE \
        XINCX_SAADC_DEFAULT_CHANNEL_CONFIG

/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_saadc_uninit            xincx_saadc_uninit
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_saadc_channel_init      xincx_saadc_channel_init
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_saadc_channel_uninit    xincx_saadc_channel_uninit
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_saadc_sample            xincx_saadc_sample
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_saadc_sample_convert    xincx_saadc_sample_convert
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_saadc_buffer_convert    xincx_saadc_buffer_convert
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_saadc_is_busy           xincx_saadc_is_busy



/**
 * @brief Function for initializing the SAADC.
 *
 * @param[in] p_config      Pointer to the structure with initial configuration.
 *                          If NULL, the default one is used.
 * @param[in] event_handler Event handler provided by the user.
 *
 * @retval NRF_SUCCESS If initialization was successful.
 * @retval NRF_ERROR_INVALID_STATE If the driver is already initialized.
 * @retval NRF_ERROR_INVALID_PARAM If event_handler is NULL.
 */
__STATIC_INLINE ret_code_t xinc_drv_saadc_init(xincx_saadc_t const * const  p_instance,xinc_drv_saadc_config_t const * p_config,
                                              xinc_drv_saadc_event_handler_t  event_handler)
{
    if (p_config == NULL)
    {
        static const xincx_saadc_config_t default_config = XINCX_SAADC_DEFAULT_CONFIG;
        p_config = &default_config;
    }
    return xincx_saadc_init(p_instance,p_config, event_handler);
}

/** @} */

#ifdef __cplusplus
}
#endif

#endif // XINC_DRV_SAADC_H__
