/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#ifndef XINC_ATFIFO_H__
#define XINC_ATFIFO_H__

#include <stdint.h>
#include <stdbool.h>
#include "sdk_config.h"
#include "xinchip_common.h"
#include "xinc_assert.h"
#include "sdk_errors.h"
#include "xinc_log_instance.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xinc_atfifo Atomic FIFO
 * @ingroup app_common
 *
 * @brief @tagAPI52 FIFO implementation that allows for making atomic transactions without
 * locking interrupts.
 *
 * @details There are two types of functions to prepare the FIFO writing:
 * - Single function for simple access:
 * @code
 * if (XINC_SUCCESS != xinc_atfifo_simple_put(my_fifo, &data, NULL))
 * {
 *      // Error handling
 * }
 * @endcode
 * - Function pair to limit data copying:
 * @code
 * struct point3d
 * {
 *      int x, y, z;
 * }point3d_t;
 * xinc_atfifo_context_t context;
 * point3d_t * point;
 *
 * if (NULL != (point = xinc_atfifo_item_alloc(my_fifo, &context)))
 * {
 *      point->x = a;
 *      point->y = b;
 *      point->z = c;
 *      if (xinc_atfifo_item_put(my_fifo, &context))
 *      {
 *          // Send information to the rest of the system
 *          // that there is new data in the FIFO available for reading.
 *      }
 * }
 * else
 * {
 *      // Error handling
 * }
 *
 * @endcode
 * @note
 * This atomic FIFO implementation requires that the operation that is
 * opened last is finished (committed/flushed) first.
 * This is typical for operations performed from the interrupt runtime
 * when the other operation is performed from the main thread.
 *
 * This implementation does not support typical multithreading operating system
 * access where operations can be started and finished in totally unrelated order.
 *
 * @{
 */

/**
 * @brief Read and write position structure.
 *
 * A structure that holds the read and write position used by the FIFO head and tail.
 */
typedef struct xinc_atfifo_postag_pos_s
{
    uint16_t wr; //!< First free space to write the data
    uint16_t rd; //!< A place after the last data to read
}xinc_atfifo_postag_pos_t;

/**
 * @brief End data index tag.
 *
 * A tag used to mark the end of data.
 * To properly realize atomic data committing, the whole variable has to be
 * accessed atomically.
 */
typedef union xinc_atfifo_postag_u
{
    uint32_t                tag; //!< Whole tag, used for atomic, 32-bit access
    xinc_atfifo_postag_pos_t pos; //!< Structure that holds reading and writing position separately
}xinc_atfifo_postag_t;

/**
 * @brief The FIFO instance.
 *
 * The instance of atomic FIFO.
 * Used with all FIFO functions.
 */
typedef struct xinc_atfifo_s
{
    void                * p_buf;        //!< Pointer to the data buffer
    xinc_atfifo_postag_t   tail;         //!< Read and write tail position tag
    xinc_atfifo_postag_t   head;         //!< Read and write head position tag
    uint16_t              buf_size;     //!< FIFO size in number of bytes (has to be divisible by @c item_size)
    uint16_t              item_size;    //!< Size of a single FIFO item
    XINC_LOG_INSTANCE_PTR_DECLARE(p_log) //!< Pointer to instance of the logger object (Conditionally compiled).
}xinc_atfifo_t;

/**
 * @brief FIFO write operation item context.
 *
 * Context structure used to mark an allocated space in FIFO that is ready for put.
 * All the data required to properly put allocated and written data.
 */
typedef struct xinc_atfifo_item_put_s
{
    xinc_atfifo_postag_t last_tail; //!< Tail tag value that was here when opening the FIFO to write
}xinc_atfifo_item_put_t;


/**
 * @brief FIFO read operation item context.
 *
 * Context structure used to mark an opened get operation to properly free an item after reading.
 */
typedef struct xinc_atfifo_rcontext_s
{
    xinc_atfifo_postag_t last_head; //!< Head tag value that was here when opening the FIFO to read
}xinc_atfifo_item_get_t;


/** @brief Name of the module used for logger messaging.
 */
#define XINC_ATFIFO_LOG_NAME atfifo

/**
 * @defgroup xinc_atfifo_instmacros FIFO instance macros
 *
 * A group of macros helpful for FIFO instance creation and initialization.
 * They may be used to create and initialize instances for most use cases.
 *
 * FIFO may also be created and initialized directly using
 * @ref xinc_atfifo_init function.
 * @{
 */
    /**
     * @brief Macro for generating the name for a data buffer.
     *
     * The name of the data buffer that would be created by
     * @ref XINC_ATFIFO_DEF macro.
     *
     * @param[in] fifo_id Identifier of the FIFO object.
     *
     * @return Name of the buffer variable.
     *
     * @note This is auxiliary internal macro and in normal usage
     *       it should not be called.
     */
    #define XINC_ATFIFO_BUF_NAME(fifo_id) CONCAT_2(fifo_id, _data)

    /**
     * @brief Macro for generating the name for a FIFO instance.
     *
     * The name of the instance variable that will be created by the
     * @ref XINC_ATFIFO_DEF macro.
     *
     * @param[in] fifo_id Identifier of the FIFO object.
     *
     * @return Name of the instance variable.
     *
     * @note This is auxiliary internal macro and in normal usage
     *       it should not be called.
     */
    #define XINC_ATFIFO_INST_NAME(fifo_id) CONCAT_2(fifo_id, _inst)

    /**
     * @brief Macro for creating an instance.
     *
     * Creates the FIFO object variable itself.
     *
     * Usage example:
     * @code
     * XINC_ATFIFO_DEF(my_fifo, uint16_t, 12);
     * XINC_ATFIFO_INIT(my_fifo);
     *
     * uint16_t some_val = 45;
     * xinc_atfifo_item_put(my_fifo, &some_val, sizeof(some_val), NULL);
     * xinc_atfifo_item_get(my_fifo, &some_val, sizeof(some_val), NULL);
     * @endcode
     *
     * @param[in] fifo_id      Identifier of a FIFO object.
     *                         This identifier will be a pointer to the instance.
     *                         It makes it possible to use this directly for the functions
     *                         that operate on the FIFO.
     *                         Because it is a static const object, it should be optimized by the compiler.
     * @param[in] storage_type Type of data that will be stored in the FIFO.
     * @param[in] item_cnt     Capacity of the created FIFO in maximum number of items that may be stored.
     *                         The phisical size of the buffer will be 1 element bigger.
     */
    #define XINC_ATFIFO_DEF(fifo_id, storage_type, item_cnt)                                     \
        static storage_type XINC_ATFIFO_BUF_NAME(fifo_id)[(item_cnt)+1];                         \
        XINC_LOG_INSTANCE_REGISTER(XINC_ATFIFO_LOG_NAME, fifo_id,                                 \
                                  XINC_ATFIFO_CONFIG_INFO_COLOR,                                 \
                                  XINC_ATFIFO_CONFIG_DEBUG_COLOR,                                \
                                  XINC_ATFIFO_CONFIG_LOG_INIT_FILTER_LEVEL,                      \
                                  XINC_ATFIFO_CONFIG_LOG_ENABLED ?                               \
                                          XINC_ATFIFO_CONFIG_LOG_LEVEL : XINC_LOG_SEVERITY_NONE); \
        static xinc_atfifo_t XINC_ATFIFO_INST_NAME(fifo_id) = {                                   \
                .p_buf = NULL,                                                                  \
                XINC_LOG_INSTANCE_PTR_INIT(p_log, XINC_ATFIFO_LOG_NAME, fifo_id)                  \
        };                                                                                      \
        static xinc_atfifo_t * const fifo_id = &XINC_ATFIFO_INST_NAME(fifo_id)

    /**
     * @brief Macro for initializing the FIFO that was previously declared by the macro.
     *
     * Use this macro to simplify FIFO initialization.
     *
     * @note
     * This macro can be only used on a FIFO object defined by @ref XINC_ATFIFO_DEF macro.
     *
     * @param[in] fifo_id Identifier of the FIFO object.
     *
     * @return Value from the @ref xinc_atfifo_init function.
     */
    #define XINC_ATFIFO_INIT(fifo_id)                \
        xinc_atfifo_init(                            \
            fifo_id,                                \
            XINC_ATFIFO_BUF_NAME(fifo_id),           \
            sizeof(XINC_ATFIFO_BUF_NAME(fifo_id)),   \
            sizeof(XINC_ATFIFO_BUF_NAME(fifo_id)[0]) \
        )

/** @} */

/**
 * @brief Function for initializing the FIFO.
 *
 * Preparing the FIFO instance to work.
 *
 * @param[out]    p_fifo    FIFO object to initialize.
 * @param[in,out] p_buf     FIFO buffer for storing data.
 * @param[in]     buf_size  Total buffer size (has to be divisible by @c item_size).
 * @param[in]     item_size Size of a single item held inside the FIFO.
 *
 * @retval     XINC_SUCCESS              If initialization was successful.
 * @retval     XINC_ERROR_NULL           If a NULL pointer is provided as the buffer.
 * @retval     XINC_ERROR_INVALID_LENGTH If size of the buffer provided is not divisible by @c item_size.
 *
 * @note
 * Buffer size must be able to hold one element more than the designed FIFO capacity.
 * This one, empty element is used for overflow checking.
 */
ret_code_t xinc_atfifo_init(xinc_atfifo_t * const p_fifo, void * p_buf, uint16_t buf_size, uint16_t item_size);

/**
 * @brief Function for clearing the FIFO.
 *
 * Function for clearing the FIFO.
 *
 * If this function is called during an opened and uncommitted write operation,
 * the FIFO is cleared up to the currently ongoing commit.
 * There is no possibility to cancel an ongoing commit.
 *
 * If this function is called during an opened and unflushed read operation,
 * the read position in the head is set, but copying it into the write head position
 * is left to read closing operation.
 *
 * This way, there is no more data to read, but the memory is released
 * in the moment when it is safe.
 *
 * @param[in,out] p_fifo FIFO object.
 *
 * @retval XINC_SUCCESS    FIFO totally cleared.
 * @retval XINC_ERROR_BUSY Function called in the middle of writing or reading operation.
 *                        If it is called in the middle of writing operation,
 *                        FIFO was cleared up to the already started and uncommitted write.
 *                        If it is called in the middle of reading operation,
 *                        write head was only moved. It will be copied into read tail when the reading operation
 *                        is flushed.
 */
ret_code_t xinc_atfifo_clear(xinc_atfifo_t * const p_fifo);

/**
 * @brief Function for atomically putting data into the FIFO.
 *
 * It uses memcpy function inside and in most situations, it is more suitable to
 * use @ref xinc_atfifo_item_alloc, write the data, and @ref xinc_atfifo_item_put to store a new value
 * in a FIFO.
 *
 * @param[in,out] p_fifo    FIFO object.
 * @param[in]     p_var     Variable to copy.
 * @param[in]     size      Size of the variable to copy.
 *                          Can be smaller or equal to the FIFO item size.
 * @param[out]    p_visible See value returned by @ref xinc_atfifo_item_put.
 *                          It may be NULL if the caller does not require the current operation status.
 *
 * @retval XINC_SUCCESS      If an element has been successfully added to the FIFO.
 * @retval XINC_ERROR_NO_MEM If the FIFO is full.
 *
 * @note
 * To avoid data copying, you can use the @ref xinc_atfifo_item_alloc and @ref xinc_atfifo_item_put
 * functions pair.
 */
ret_code_t xinc_atfifo_alloc_put(xinc_atfifo_t * const p_fifo, void const * const p_var, size_t size, bool * const p_visible);

/**
 * @brief Function for opening the FIFO for writing.
 *
 * Function called to start the FIFO write operation and access the given FIFO buffer directly.
 *
 * @param[in,out] p_fifo    FIFO object.
 * @param[out]    p_context Operation context, required by @ref xinc_atfifo_item_put.
 *
 * @return Pointer to the space where variable data can be stored.
 *         NULL if there is no space in the buffer.
 */
void * xinc_atfifo_item_alloc(xinc_atfifo_t * const p_fifo, xinc_atfifo_item_put_t * p_context);

/**
 * @brief Function for closing the writing operation.
 *
 * Puts a previously allocated context into FIFO.
 * This function must be called to commit an opened write operation.
 * It sets all the buffers and marks the data, so that it is visible to read.
 *
 * @param[in,out] p_fifo    FIFO object.
 * @param[in]     p_context Operation context, filled by the @ref xinc_atfifo_item_alloc function.
 *
 * @retval true  Data is currently ready and will be visible to read.
 * @retval false The internal commit was marked, but the writing operation interrupted another writing operation.
 *               The data will be available to read when the interrupted operation is committed.
 */
bool xinc_atfifo_item_put(xinc_atfifo_t * const p_fifo, xinc_atfifo_item_put_t * p_context);

/**
 * @brief Function for getting a single value from the FIFO.
 *
 * This function gets the value from the top of the FIFO.
 * The value is removed from the FIFO memory.
 *
 * @param[in,out] p_fifo     FIFO object.
 * @param[out]    p_var      Pointer to the variable to store the data.
 * @param[in]     size       Size of the data to be loaded.
 * @param[out]    p_released See the values returned by @ref xinc_atfifo_item_free.
 *
 * @retval XINC_SUCCESS         Element was successfully copied from the FIFO memory.
 * @retval XINC_ERROR_NOT_FOUND No data in the FIFO.
 */
ret_code_t xinc_atfifo_get_free(xinc_atfifo_t * const p_fifo, void * const p_var, size_t size, bool * p_released);

/**
 * @brief Function for opening the FIFO for reading.
 *
 * Function called to start the FIFO read operation and access the given FIFO buffer directly.
 *
 * @param[in,out] p_fifo    FIFO object.
 * @param[out]    p_context The operation context, required by @ref xinc_atfifo_item_free
 *
 * @return Pointer to data buffer or NULL if there is no data in the FIFO.
 */
void * xinc_atfifo_item_get(xinc_atfifo_t * const p_fifo, xinc_atfifo_item_get_t * p_context);

/**
 * @brief Function for closing the reading operation.
 *
 * Function used to finish the reading operation.
 * If this reading operation does not interrupt another reading operation, the head write buffer is moved.
 * If this reading operation is placed in the middle of another reading, only the new read pointer is written.
 *
 * @param[in,out] p_fifo    FIFO object.
 * @param[in]     p_context Context of the reading operation to be closed.
 *
 * @retval true  This operation is not generated in the middle of another read operation and the write head will be updated to the read head (space is released).
 * @retval false This operation was performed in the middle of another read operation and the write buffer head was not moved (no space is released).
 */
bool xinc_atfifo_item_free(xinc_atfifo_t * const p_fifo, xinc_atfifo_item_get_t * p_context);


/** @} */

#ifdef __cplusplus
}
#endif

#endif /* XINC_ATFIFO_H__ */
