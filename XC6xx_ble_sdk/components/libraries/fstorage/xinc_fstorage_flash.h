/**
 * @file
 *
 * @defgroup xinc_fstorage_flash FLASH implementation
 * @ingroup xinc_fstorage
 * @{
 *
 * @brief API implementation of fstorage that uses the non-volatile memory controller (Flash).
*/

#ifndef XINC_FSTORAGE_FLASH_H__
#define XINC_FSTORAGE_FLASH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "xinc_fstorage.h"

/**@brief   API implementation that uses the non-volatile memory controller.
 *
 * @details An fstorage instance with this API implementation can be initialized by providing
 *          this structure as a parameter to @ref xinc_fstorage_init.
 *          The structure is defined in @c xinc_fstorage_flash.c.
 */
extern nrf_fstorage_api_t nrf_fstorage_nvmc;
	
#ifdef __cplusplus
}
#endif

#endif //XINC_FSTORAGE_FLASH_H__

