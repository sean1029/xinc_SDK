/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINC_DRV_WDT_H__
#define XINC_DRV_WDT_H__

#include <xincx_wdt.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xinc_drv_wdt WDT driver - legacy layer
 * @{
 * @ingroup  xinc_wdt
 *
 * @brief    A layer providing compatibility with former API.
 */

/** @brief Type definition for forwarding the new implementation. */
typedef xincx_wdt_config_t xinc_drv_wdt_config_t;

typedef xincx_wdt_t         xinc_drv_wdt_t;


/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_WDT_INSTANCE                    XINCX_WDT_INSTANCE

/** @brief Macro for forwarding the new implementation. */
#define XINC_DRV_WDT_DEAFULT_CONFIG  XINCX_WDT_DEAFULT_CONFIG

/** @brief Macro for forwarding the new implementation. */
#define xinc_wdt_event_handler_t     xincx_wdt_event_handler_t

/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_wdt_enable          xincx_wdt_enable
/** @brief Macro for forwarding the new implementation. */
#define xinc_drv_wdt_feed            xincx_wdt_feed

/**
 * @brief This function initializes watchdog.
 *
 * @param[in] p_config          Pointer to the structure with initial configuration. Default
 *                              configuration used if NULL.
 * @param[in] wdt_event_handler Specifies event handler provided by user.
 *
 * @note Function asserts if wdt_event_handler is NULL.
 *
 * @return XINC_SUCCESS on success, otherwise an error code.
 */
__STATIC_INLINE ret_code_t xinc_drv_wdt_init(xinc_drv_wdt_t  const * const p_instance,xinc_drv_wdt_config_t const * p_config,
                                            xinc_wdt_event_handler_t      wdt_event_handler)
{
    if (p_config == NULL)
    {
        static const xincx_wdt_config_t default_config = XINCX_WDT_DEAFULT_CONFIG;
        p_config = &default_config;
    }
    return xincx_wdt_init(p_instance,p_config, wdt_event_handler);
}

/** @} */

#ifdef __cplusplus
}
#endif

#endif // XINC_DRV_WDT_H__
