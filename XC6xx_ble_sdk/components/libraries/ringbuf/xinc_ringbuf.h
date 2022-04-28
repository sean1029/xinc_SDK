/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#ifndef XINC_RINGBUF_H
#define XINC_RINGBUF_H

/**
* @defgroup xinc_ringbuf Ring buffer
* @{
* @ingroup app_common
* @brief Functions for controlling the ring buffer.
*/

#include <stdint.h>
#include "xinc_atomic.h"
#include "sdk_errors.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Ring buffer instance control block.
 * */
typedef struct
{
    xinc_atomic_flag_t   wr_flag;   //!< Protection flag.
    xinc_atomic_flag_t   rd_flag;   //!< Protection flag.
    uint32_t            wr_idx;     //!< Write index (updated when putting).
    uint32_t            tmp_wr_idx; //!< Temporary write index (updated when allocating).
    uint32_t            rd_idx;     //!< Read index (updated when freeing).
    uint32_t            tmp_rd_idx; //!< Temporary read index (updated when getting).
} xinc_ringbuf_cb_t;

/**
 * @brief Ring buffer instance structure.
 * */
typedef struct
{
    uint8_t           * p_buffer;     //!< Pointer to the memory used by the ring buffer.
    uint32_t            bufsize_mask; //!< Buffer size mask (buffer size must be a power of 2).
    xinc_ringbuf_cb_t  * p_cb;         //!< Pointer to the instance control block.
} xinc_ringbuf_t;

/**
 * @brief Macro for defining a ring buffer instance.
 *
 * @param _name Instance name.
 * @param _size Size of the ring buffer (must be a power of 2).
 * */
#ifdef XC60XX_M0
#define XINC_RINGBUF_DEF(_name, _size)                                         \
    STATIC_ASSERT(IS_POWER_OF_TWO(_size));                                    \
    __ALIGN(4) static uint8_t __attribute__((section("DMA_BUFF"))) CONCAT_2(_name,_buf)[_size];   \
    static xinc_ringbuf_cb_t CONCAT_2(_name,_cb);                              \
    static const xinc_ringbuf_t _name = {                                      \
            .p_buffer = CONCAT_2(_name,_buf),                                 \
            .bufsize_mask = _size - 1,                                        \
            .p_cb         = &CONCAT_2(_name,_cb),                             \
    }
#else

#define XINC_RINGBUF_DEF(_name, _size)                                         \
    STATIC_ASSERT(IS_POWER_OF_TWO(_size));                                    \
    static uint8_t CONCAT_2(_name,_buf)[_size];                               \
    static xinc_ringbuf_cb_t CONCAT_2(_name,_cb);                              \
    static const xinc_ringbuf_t _name = {                                      \
            .p_buffer = CONCAT_2(_name,_buf),                                 \
            .bufsize_mask = _size - 1,                                        \
            .p_cb         = &CONCAT_2(_name,_cb),                             \
    }
#endif 



/**
 * @brief Function for initializing a ring buffer instance.
 *
 * @param p_ringbuf             Pointer to the ring buffer instance.
 *
 * */
void xinc_ringbuf_init(xinc_ringbuf_t const * p_ringbuf);

/**
 * @brief Function for allocating memory from a ring buffer.
 *
 * This function attempts to allocate the amount of memory requested by the user, or a smaller amount if
 * the requested amount is not available. If a start flag is set, then exclusive access to allocation
 * is established. @ref xinc_ringbuf_put frees access to allocation. If a start flag is not set, then
 * exclusive access check is omitted.
 *
 * @param[in] p_ringbuf      Pointer to the ring buffer instance.
 * @param[in] pp_data        Pointer to the pointer to the allocated buffer.
 * @param[in, out] p_length  Pointer to length. Length is set to the requested amount and filled
 *                           by the function with actually allocated amount.
 * @param[in] start          Set to true if exclusive access should be controlled.
 *
 * @retval XINC_SUCCESS       Successful allocation (can be smaller amount than requested).
 *         XINC_ERROR_BUSY    Ring buffer allocation process (alloc-put) is ongoing.
 *
 * */
ret_code_t xinc_ringbuf_alloc(xinc_ringbuf_t const * p_ringbuf, uint8_t * * pp_data, size_t * p_length, bool start);

/**
 * @brief Function for commiting data to a ring buffer.
 *
 * When an allocated buffer (see @ref xinc_ringbuf_alloc) has been filled with data, it must be committed
 * to make it available for @ref xinc_ringbuf_get and @ref xinc_ringbuf_cpy_get. This function commits
 * the data (can be smaller amount than allocated).
 *
 * @param[in] p_ringbuf          Pointer to the ring buffer instance.
 * @param[in] length             Amount of bytes to put.

 * @return  XINC_SUCCESS on successful put or error.
 * */
ret_code_t xinc_ringbuf_put(xinc_ringbuf_t const * p_ringbuf, size_t length);

/**
 * @brief Function for copying data directly into the ring buffer.
 *
 * This function copies a user buffer to the ring buffer.
 *
 * @param[in] p_ringbuf       Pointer to the ring buffer instance.
 * @param[in] p_data          Pointer to the input buffer.
 * @param[in, out] p_length   Amount of bytes to copy. Amount of bytes copied.

 * @return  XINC_SUCCESS on successful put or error.
 * */
ret_code_t xinc_ringbuf_cpy_put(xinc_ringbuf_t const * p_ringbuf,
                               uint8_t const* p_data,
                               size_t * p_length);


/**
 * Function for getting data from the ring buffer.
 *
 * This function attempts to get the requested amount of data from the ring buffer. If a start flag is set, then
 * exclusive access to getting data from the ring buffer is established. @ref xinc_ringbuf_free frees
 * access to getting data from the ring buffer. If a start flag is not set, then
 * exclusive access check is omitted.
 *
 * @param[in] p_ringbuf     Pointer to the ring buffer instance.
 * @param[in] pp_data       Pointer to the pointer to the buffer with data.
 * @param[in, out] p_length Pointer to length. Length is set to the requested amount and filled
 *                          by the function with the actual amount.
 * @param[in] start         Set to true if exclusive access should be controlled.
 *
 * @retval XINC_SUCCESS            Successful getting (can be smaller amount than requested).
 *         XINC_ERROR_BUSY         Ring buffer getting process (get-free) is ongoing.
 */
ret_code_t xinc_ringbuf_get(xinc_ringbuf_t const * p_ringbuf, uint8_t * * pp_data, size_t * p_length, bool start);

/**
 * @brief Function for freeing a buffer back to the ring buffer.
 *
 * When a buffer with data taken from the ring buffer (see @ref xinc_ringbuf_get) has been processed, it
 * must be freed to make it available for further use. This function frees the buffer (can be smaller
 * amount than get).
 *
 * @param[in] p_ringbuf          Pointer to the ring buffer instance.
 * @param[in] length             Amount of bytes to free.

 * @return  XINC_SUCCESS on successful put or error.
 * */
ret_code_t xinc_ringbuf_free(xinc_ringbuf_t const * p_ringbuf, size_t length);

/**
 * @brief Function for copying data directly out of the ring buffer.
 *
 * This function copies available data from the ring buffer to a user buffer.
 *
 * @param[in]      p_ringbuf    Pointer to the ring buffer instance.
 * @param[in]      p_data       Pointer to the input buffer.
 * @param[in, out] p_length     Amount of bytes to copy. Amount of bytes copied.

 * @return  XINC_SUCCESS on successful put or error.
 * */
ret_code_t xinc_ringbuf_cpy_get(xinc_ringbuf_t const * p_ringbuf,
                               uint8_t * p_data,
                               size_t * p_length);
#ifdef __cplusplus
}
#endif

#endif //XINC_RINGBUF_H
/** @} */
