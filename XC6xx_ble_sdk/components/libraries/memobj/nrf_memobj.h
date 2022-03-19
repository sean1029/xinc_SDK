/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINC_MEMOBJ_H
#define XINC_MEMOBJ_H

/**
* @defgroup nrf_memobj Memory Object module
* @{
* @ingroup app_common
* @brief Functions for controlling a memory object.
*/
#include <stdint.h>
#include <stdlib.h>
#include "sdk_errors.h"
#include "nrf_balloc.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A memory object can consist of multiple chunks with the same size. Each object has a header part
 * and a data part. The first element in a memory object is memory object head which has a special header.
 * The remaining objects have a header of the same size.
 * 
@verbatim
 _____________________        _____________________          _____________________
|                     |      |                     |        |                     |
|4 B  head header     |  --> |4 B    p_next        |------->|4 B  p_memobj_pool   |
|_____________________|  |   |_____________________|        |_____________________|
|                     |  |   |                     |        |                     |
|4 B    p_next        |--|   |                     |        |                     |
|_____________________|      |                     |  ....  |                     |
|                     |      |        data         |        |        data         |
|                     |      |                     |        |                     |
|       data          |      |                     |        |                     |
|                     |      |                     |        |                     |
|_____________________|      |_____________________|        |_____________________|
         head                      mid_element                  last_element
@endverbatim
 *
 */
#define XINC_MEMOBJ_STD_HEADER_SIZE sizeof(uint32_t)

/**
 * @brief Macro for creating an nrf_memobj pool.
 *
 * This macro declares an nrf_balloc object. The element in the pool contains a user-defined data part and
 * a memory object header.
 *
 * @param _name       Name of the instance.
 * @param _chunk_size Size of a single chunk.
 * @param _pool_size  Number of chunks in the pool.
 */
#define XINC_MEMOBJ_POOL_DEF(_name, _chunk_size, _pool_size)                           \
    STATIC_ASSERT((_chunk_size) > sizeof(uint32_t));                                  \
    XINC_BALLOC_DEF(_name, ((_chunk_size)+XINC_MEMOBJ_STD_HEADER_SIZE), (_pool_size))

/**
 * @brief Pool of memory objects.
 */
typedef nrf_balloc_t nrf_memobj_pool_t;

/**
 * @brief Memory object handle.
 */
typedef void * nrf_memobj_t;

/**
 * @brief Function for initializing the memobj pool instance.
 *
 * This function initializes the pool.
 *
 * @param[in] p_pool     Pointer to the memobj pool instance structure.
 *
 * @return  XINC_SUCCESS on success, otherwise an error code.
 */
ret_code_t nrf_memobj_pool_init(nrf_memobj_pool_t const * p_pool);

/**
 * @brief Function for allocating a memobj with a requested size.
 *
 * Fixed length elements in the pool are linked together to provide the amount of memory requested by
 * the user. If a memory object is successfully allocated, then the users can use the memory.
 * However, it is fragmented into multiple objects so it must be accessed through the API:
 * @ref nrf_memobj_write and @ref nrf_memobj_read.
 * 
 * @param[in] p_pool     Pointer to the memobj pool instance structure.
 * @param[in] size       Data size of requested object.
 *
 * @return  Pointer to a memory object or NULL if the requested size cannot be allocated.
 */
nrf_memobj_t * nrf_memobj_alloc(nrf_memobj_pool_t const * p_pool,
                                size_t size);

/**
 * @brief Function for indicating that a memory object is used and cannot be freed.
 *
 * Memory object can be shared and reused between multiple modules and this mechanism ensures that
 * object is freed when no longer used by any module. Memory object has a counter which is incremented
 * whenever this function is called. @ref nrf_memobj_put function decrements the counter.
 *
 * @param[in] p_obj  Pointer to memory object.
 */
void nrf_memobj_get(nrf_memobj_t const * p_obj);


/**
 * @brief Function for indicated that memory object is no longer used by the module and can be freed
 * if no other module is using it.
 *
 * Memory object is returned to the pool if internal counter reaches 0 after decrementing. It means
 * that no other module is needing it anymore.
 *
 * @note Memory object holds pointer to the pool which was used to allocate it so it does not have
 * to be provided explicitly to this function.
 *
 * @param[in] p_obj  Pointer to memory object.
 */
void nrf_memobj_put(nrf_memobj_t * p_obj);


/**
 * @brief Function for forcing freeing of the memory object.
 *
 * @note This function should be use with caution because it can lead to undefined behavior of the
 * modules since modules using the memory object are not aware that it has been freed.
 *
 * @param[in] p_obj  Pointer to memory object.
 */
void nrf_memobj_free(nrf_memobj_t * p_obj);

/**
 * @brief Function for writing data to the memory object.
 *
 * @param[in]      p_obj  Pointer to memory object.
 * @param[in]      p_data Pointer to data to be written to the memory object.
 * @param[in]      len    Amount of data to be written to the memory object.
 * @param[in]      offset Offset.
 */
void nrf_memobj_write(nrf_memobj_t * p_obj,
                      void *         p_data,
                      size_t         len,
                      size_t         offset);

/**
 * @brief Function for reading data from the memory object.
 *
 * @param[in]     p_obj  Pointer to memory object.
 * @param[in]     p_data Pointer to the destination buffer.
 * @param[in]     len    Amount of data to be read from the memory object.
 * @param[in]     offset Offset.
 */
void nrf_memobj_read(nrf_memobj_t * p_obj,
                     void *         p_data,
                     size_t         len,
                     size_t         offset);

#ifdef __cplusplus
}
#endif

#endif //XINC_MEMOBJ_H

/** @} */
