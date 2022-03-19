/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINCX_WDT_H__
#define XINCX_WDT_H__

#include <xincx.h>
#include <hal/xinc_wdt.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xincx_wdt WDT driver
 * @{
 * @ingroup xinc_wdt
 * @brief   Watchdog Timer (WDT) peripheral driver.
 */

#if !XINCX_CHECK(XINCX_WDT_CONFIG_NO_IRQ) || defined(__XINCX_DOXYGEN__)
/** @brief WDT instance interrupt priority configuration. */
    #define XINCX_WDT_IRQ_CONFIG .interrupt_priority = XINCX_WDT_CONFIG_IRQ_PRIORITY
#else
    #define XINCX_WDT_IRQ_CONFIG
#endif

/**@brief Struct for WDT initialization. */
typedef struct
{
    uint32_t               reload_value;       /**< WDT reload value in ms. */
#if !XINCX_CHECK(XINCX_WDT_CONFIG_NO_IRQ) || defined(__XINCX_DOXYGEN__)
    uint8_t                interrupt_priority; /**< WDT interrupt priority */
#endif
} xincx_wdt_config_t;

/** @brief WDT event handler function type. */
typedef void (*xincx_wdt_event_handler_t)(void);


/** @brief WDT driver default configuration. */
#define XINCX_WDT_DEAFULT_CONFIG                                               \
{                                                                         \
    .reload_value       = XINCX_WDT_CONFIG_RELOAD_VALUE,                   \
    XINCX_WDT_IRQ_CONFIG                                                   \
}
/**
 * @brief This function initializes the watchdog.
 *
 * @param[in] p_config          Pointer to the structure with the initial configuration.
 * @param[in] wdt_event_handler Event handler provided by the user. Ignored when
 *                              @ref XINCX_WDT_CONFIG_NO_IRQ option is enabled.
 *
 * @return XINCX_SUCCESS on success, otherwise an error code.
 */
xincx_err_t xincx_wdt_init(xincx_wdt_config_t const * p_config,
                         xincx_wdt_event_handler_t  wdt_event_handler);


/**
 * @brief Function for starting the watchdog.
 *
 * @note After calling this function the watchdog is started, so the user needs to feed all allocated
 *       watchdog channels to avoid reset. At least one watchdog channel must be allocated.
 */
void xincx_wdt_enable(void);

/**
 * @brief Function for feeding the watchdog.
 *
 * @details Function feeds all allocated watchdog channels.
 */
void xincx_wdt_feed(void);


/** @} */


void xincx_wdt_irq_handler(void);


#ifdef __cplusplus
}
#endif

#endif

