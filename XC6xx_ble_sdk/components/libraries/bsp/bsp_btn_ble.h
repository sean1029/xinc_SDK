/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
/**@file
 *
 * @defgroup bsp_btn_ble BSP: BLE Button Module
 * @{
 * @ingroup bsp
 *
 * @brief Module for controlling BLE behavior through button actions.
 *
 * @details The application must propagate BLE events to the BLE Button Module.
 * Based on these events, the BLE Button Module configures the Board Support Package
 * to generate BSP events for certain button actions. These BSP events should then be
 * handled by the application's BSP event handler.
 *
 */

#ifndef BSP_BTN_BLE_H__
#define BSP_BTN_BLE_H__

#include <stdint.h>
#include "ble.h"
#include "bsp.h"

#ifdef __cplusplus
extern "C" {
#endif

/**@brief BLE Button Module error handler type. */
typedef void (*bsp_btn_ble_error_handler_t) (uint32_t xinc_error);

/**@brief Function for initializing the BLE Button Module.
 *
 * Before calling this function, the BSP module must be initialized with buttons.
 *
 * @param[out] error_handler      Error handler to call in case of internal errors in BLE Button
 *                                Module.
 * @param[out] p_startup_bsp_evt  If not a NULL pointer, the value is filled with an event
 *                                (or BSP_EVENT_NOTHING) derived from the buttons pressed on
 *                                startup. For example, if the bond delete wakeup button was pressed
 *                                to wake up the device, *p_startup_bsp_evt is set to
 *                                @ref BSP_EVENT_CLEAR_BONDING_DATA.
 *
 * @retval XINC_SUCCESS  If initialization was successful. Otherwise, a propagated error code is
 *                      returned.
 */
uint32_t bsp_btn_ble_init(bsp_btn_ble_error_handler_t error_handler, bsp_event_t * p_startup_bsp_evt);

/**@brief Function for setting up wakeup buttons before going into sleep mode.
 *
 * @retval XINC_SUCCESS  If the buttons were prepared successfully. Otherwise, a propagated error
 *                      code is returned.
 */
uint32_t bsp_btn_ble_sleep_mode_prepare(void);


#ifdef __cplusplus
}
#endif

#endif /* BSP_BTN_BLE_H__ */

/** @} */
