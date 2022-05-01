/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

/**
 * @defgroup xinc_pwr_mgmt Power management
 * @ingroup app_common
 * @{
 * @brief This module handles power management features.
 *
 */
#ifndef XINC_PWR_MGMT_H__
#define XINC_PWR_MGMT_H__

#include <stdbool.h>
#include <stdint.h>
#include <sdk_errors.h>
#include "xinc_section_iter.h"

#ifdef __cplusplus
extern "C" {
#endif

/**@brief Power management shutdown types. */
typedef enum
{
    XINC_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF,
    //!< Go to System OFF.

    XINC_PWR_MGMT_SHUTDOWN_STAY_IN_SYSOFF,
    //!< Go to System OFF and stay there.
    /**<
     * Useful when battery level is dangerously low, for example.
     */

    XINC_PWR_MGMT_SHUTDOWN_GOTO_DFU,
    //!< Go to DFU mode.

    XINC_PWR_MGMT_SHUTDOWN_RESET,
    //!< Reset chip.

    XINC_PWR_MGMT_SHUTDOWN_CONTINUE
    //!< Continue shutdown.
    /**<
     * This should be used by modules that block the shutdown process, when they become ready for
     * shutdown.
     */
} xinc_pwr_mgmt_shutdown_t;

/**@brief Shutdown event types. */
typedef enum
{
    XINC_PWR_MGMT_EVT_PREPARE_WAKEUP = XINC_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF,
    //!< Application will prepare the wakeup mechanism.

    XINC_PWR_MGMT_EVT_PREPARE_SYSOFF = XINC_PWR_MGMT_SHUTDOWN_STAY_IN_SYSOFF,
    //!< Application will prepare to stay in System OFF state.

    XINC_PWR_MGMT_EVT_PREPARE_DFU    = XINC_PWR_MGMT_SHUTDOWN_GOTO_DFU,
    //!< Application will prepare to enter DFU mode.

    XINC_PWR_MGMT_EVT_PREPARE_RESET  = XINC_PWR_MGMT_SHUTDOWN_RESET,
    //!< Application will prepare to chip reset.
} xinc_pwr_mgmt_evt_t;

/**@brief Shutdown callback.
 * @param[in] event   Type of shutdown process.
 *
 * @retval    true    System OFF / Enter DFU preparation successful. Process will be continued.
 * @retval    false   System OFF / Enter DFU preparation failed. @ref XINC_PWR_MGMT_SHUTDOWN_CONTINUE
 *                    should be used to continue the shutdown process.
 */
typedef bool (*xinc_pwr_mgmt_shutdown_handler_t)(xinc_pwr_mgmt_evt_t event);

/**@brief   Macro for registering a shutdown handler. Modules that want to get events
 *          from this module must register the handler using this macro.
 *
 * @details This macro places the handler in a section named "pwr_mgmt_data".
 *
 * @param[in]   _handler    Event handler (@ref xinc_pwr_mgmt_shutdown_handler_t).
 * @param[in]   _priority   Priority of the given handler.
 */
#define XINC_PWR_MGMT_HANDLER_REGISTER(_handler, _priority)                               \
    STATIC_ASSERT(_priority < XINC_PWR_MGMT_CONFIG_HANDLER_PRIORITY_COUNT);               \
    /*lint -esym(528,*_handler_function) -esym(529,*_handler_function) : Symbol not referenced. */         \
    XINC_SECTION_SET_ITEM_REGISTER(pwr_mgmt_data, _priority,                              \
                                  static xinc_pwr_mgmt_shutdown_handler_t const CONCAT_2(_handler, _handler_function)) = (_handler)

/**@brief   Function for initializing power management.
 *
 * @warning Depending on configuration, this function sets SEVONPEND in System Control Block (SCB).
 *          This operation is unsafe with the SoftDevice from interrupt priority higher than SVC.
 *
 * @retval XINC_SUCCESS
 */
ret_code_t xinc_pwr_mgmt_init(void);

/**@brief Function for running power management. Should run in the main loop.
 */
void xinc_pwr_mgmt_run(void);

/**@brief Function for indicating activity.
 *
 * @details Call this function whenever doing something that constitutes "activity".
 *          For example, whenever sending data, call this function to indicate that the application
 *          is active and should not disconnect any ongoing communication links.
 */
void xinc_pwr_mgmt_feed(void);

/**@brief Function for shutting down the system.
 *
 * @param[in] shutdown_type     Type of operation.
 *
 * @details All callbacks will be executed prior to shutdown.
 */
void xinc_pwr_mgmt_shutdown(xinc_pwr_mgmt_shutdown_t shutdown_type);

#ifdef __cplusplus
}
#endif

#endif // XINC_PWR_MGMT_H__
/** @} */
