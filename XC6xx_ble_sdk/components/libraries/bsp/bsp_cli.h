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
 * @defgroup bsp_cli BSP over CLI Module
 * @{
 * @ingroup bsp
 *
 * @brief Module for sending BSP events over CLI.
 *
 * @details The module uses Command Line Interface and enables user to send events
 * to BSP. They are later handled by the event handler provided.
 * Available commands:
 * - bsp btn X (where X is button number) - sends BSP_EVENT_KEY_X
 * - bsp evt X (where X is event number) - sends BSP event with X id
 */

#ifndef BSP_CLI_H__
#define BSP_CLI_H__

#include <stdint.h>
#include "xinc_cli.h"
#include "bsp.h"

#ifdef __cplusplus
extern "C" {
#endif

/**@brief Function for initializing the BSP over CLI Module.
 *
 * Before calling this function, the BSP module must be initialized.
 *
 * @param[in]  callback           Function to be called when event is recevied.
 *
 * @retval XINC_SUCCESS  If initialization was successful.
 */

ret_code_t bsp_cli_init(bsp_event_callback_t callback);

#ifdef __cplusplus
}
#endif

#endif /* BSP_CLI_H__ */

/** @} */
