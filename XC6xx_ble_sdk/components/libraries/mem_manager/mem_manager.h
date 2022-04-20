/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
/** @file
 *
 * @defgroup mem_manager Memory Manager
 * @{
 * @ingroup app_common
 * @brief Memory Manager for the @xcXX SDK
 *
 * @details This module allows for dynamic use of memory. Currently,
 * this module can be used only to allocate and free memory in the RAM.
 *
 * The Memory Manager manages static memory blocks of fixed sizes. These blocks can be requested for
 * usage, and freed when the application no longer needs them. A maximum of seven block categories
 * can be managed by the module. These block categories are identified by xxsmall, xmall, small,
 * medium, large, xlarge, and xxlarge. They are ordered in increasing block sizes.
 * The size and the count of each of the block categories can be configured based on the application
 * requirements in the configuration file @c sdk_config.h.
 * To use fewer than seven buffer pools, do not define the count for the unwanted block
 * or explicitly set it to zero. At least one block category must be configured
 * for this module to function as expected.
 */

#ifndef MEM_MANAGER_H__
#define MEM_MANAGER_H__

#include "sdk_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MEM_MANAGER_ENABLE_DIAGNOSTICS 1
/**@brief Initializes Memory Manager.
 *
 * @details API to initialize the Memory Manager. Always call this API before using any of the other
 *          APIs of the module. This API should be called only once.
 *
 * @retval XINC_SUCCESS If initialization was successful.
 *         Otherwise, an error code that indicates the reason for the failure is returned.
 *
 * @warning   If this API fails, the application shall not proceed with using other APIs of this
 *            module.
 */
uint32_t xinc_mem_init(void);


/**@brief Reserves a block of memory for the application.
 *
 * @details API to request a contiguous memory block of the given length. If
 *          the memory allocation succeeds, pp_buffer points to the memory block. If
 *          the memory allocation fails, pp_buffer points to NULL and the return value of
 *          the API indicates the reason for the failure. The memory block reserved using this API can
 *          be freed using the @ref xinc_free function.
 *
 * @param[out]   pp_buffer             Pointer to the allocated memory block if memory allocation
 *                                     succeeds; otherwise points to NULL.
 * @param[inout] p_size                Requested memory size. This parameter returns the actual size
 *                                     allocated. If the procedure was successful, the actual size
 *                                     returned is always greater than or equal to requested size,
 *                                     never less.
 *
 * @retval     XINC_SUCCESS             If memory was successfully allocated.
 *                                     Otherwise, an error code indicating the reason for failure.
 * @retval     XINC_ERROR_INVALID_PARAM If the requested memory size is zero or greater than the
 *                                     largest memory block that the module is configured to
 *                                     support.
 * @retval     XINC_ERROR_NO_MEM        If there is no memory available of the requested size.
 */
uint32_t xinc_mem_reserve(uint8_t ** pp_buffer, uint32_t * p_size);


/**@brief 'malloc' styled memory allocation function.
 *
 * @details API to allocate memory, same as xinc_mem_reserve but uses malloc signature.
 *
 * @param[in]  size  Requested memory size.
 *
 * @retval     Valid memory location if the procedure was successful, else, NULL.
 */
void * xinc_malloc(uint32_t size);


/**@brief 'calloc' styled memory allocation function.
 *
 * @details API to allocate zero-initialized memory of size count*size.
 *
 * @param[in]  nmemb  Number of elements of 'size' bytes.
 * @param[in]  size   Size of each element allocated.
 *
 * @retval     Valid, zero-initialized memory location if the procedure was successful, else, NULL.
 */
void * xinc_calloc(uint32_t nmemb, uint32_t size);


/**@brief Free allocated memory - standard 'free' styles API.
 *
 * @details API to resubmit memory allocated, same in functionality xinc_free.
 *
 * @param[out] p_buffer   Pointer to the memory block that is being freed.
 */
void xinc_free(void * p_buffer);


/**@brief Memory reallocation (trim) function.
 *
 * @details API to reallocate memory or to trim it. Trim is mentioned here to avoid use of API to
 *          request memory size larger than original memory allocated.
 *
 * @param[in] p_buffer   Pointer to the memory block that needs to be trimmed.
 * @param[in] size       Size of memory at the beginning of the buffer to be left untrimmed.
 *
 * @retval    Pointer to memory location with trimmed size, else, NULL.
 */
void * xinc_realloc(void *p_buffer, uint32_t size);

#if defined(MEM_MANAGER_ENABLE_DIAGNOSTICS) && (MEM_MANAGER_ENABLE_DIAGNOSTICS == 1)

/**@brief Function to print statistics related to memory blocks managed by memory manager.
 *
 * @details This API prints information with respects to each block function, including size, total
 *          block count, number of blocks in use at the time of printing, smallest memory size
 *          allocated in the block and the largest one. This  API is intended to help developers
 *          tune the block sizes to make optimal use of memory for the application.
 *          This functionality is never needed in final application and therefore, is disabled by
 *          default.
 */
void xinc_mem_diagnose(void);


/**@brief Function to reset memory block statistics.
 *
 * @details This API resets the statistics of the memory manager.
 *
 */
void xinc_mem_diagnose_reset(void);


/**@brief Macro that will log the mem_manager statistics if MEM_MANAGER_ENABLE_DIAGNOSTICS
 * is enabled
 */
#define XINC_MEM_MANAGER_DIAGNOSE xinc_mem_diagnose()


/*@brief Macro that will reset the mem_manager statisitics if MEM_MANAGER_ENABLE_DIAGNOSTICS
* is enabled
*/
#define XINC_MEM_MANAGER_DIAGNOSE_RESET xinc_mem_diagnose_reset()

#else

/**@brief Macro that will log the mem_manager statistics if MEM_MANAGER_ENABLE_DIAGNOSTICS
 * is enabled
 */
#define XINC_MEM_MANAGER_DIAGNOSE


/*@brief Macro that will reset the mem_manager statisitics if MEM_MANAGER_ENABLE_DIAGNOSTICS
* is enabled
*/
#define XINC_MEM_MANAGER_DIAGNOSE_RESET

#endif // MEM_MANAGER_ENABLE_DIAGNOSTICS

#ifdef __cplusplus
}
#endif

#endif // MEM_MANAGER_H__
/** @} */
