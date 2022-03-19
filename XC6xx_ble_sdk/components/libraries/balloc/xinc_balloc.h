/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
/**
  * @defgroup xinc_balloc Block memory allocator
  * @{
  * @ingroup app_common
  * @brief This module handles block memory allocator features.
  */


#ifndef XINC_BALLOC_H__
#define XINC_BALLOC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "sdk_errors.h"
#include "sdk_config.h"
#include "app_util_platform.h"
#include "app_util.h"
#include "xinc_log_instance.h"
#include "xinc_section.h"

/** @brief Name of the module used for logger messaging.
 */
#define XINC_BALLOC_LOG_NAME balloc

#if XINC_BALLOC_CONFIG_DEBUG_ENABLED || XINC_BALLOC_CLI_CMDS
#define XINC_BALLOC_HAS_NAME 1
#else
#define XINC_BALLOC_HAS_NAME 0
#endif

/**@defgroup XINC_BALLOC_DEBUG Macros for preparing debug flags for block allocator module.
 * @{ */
#define XINC_BALLOC_DEBUG_HEAD_GUARD_WORDS_SET(words)        (((words) & 0xFF) << 0)
#define XINC_BALLOC_DEBUG_HEAD_GUARD_WORDS_GET(flags)        (((flags) >> 0) & 0xFF)
#define XINC_BALLOC_DEBUG_TAIL_GUARD_WORDS_SET(words)        (((words) & 0xFF) << 8)
#define XINC_BALLOC_DEBUG_TAIL_GUARD_WORDS_GET(flags)        (((flags) >> 8) & 0xFF)

#define XINC_BALLOC_DEBUG_BASIC_CHECKS_SET(enable)           (!!(enable) << 16)
#define XINC_BALLOC_DEBUG_BASIC_CHECKS_GET(flags)            (flags & (1 << 16))
#define XINC_BALLOC_DEBUG_DOUBLE_FREE_CHECK_SET(enable)      (!!(enable) << 17)
#define XINC_BALLOC_DEBUG_DOUBLE_FREE_CHECK_GET(flags)       (flags & (1 << 17))
#define XINC_BALLOC_DEBUG_DATA_TRASHING_CHECK_SET(enable)    (!!(enable) << 18)
#define XINC_BALLOC_DEBUG_DATA_TRASHING_CHECK_GET(flags)     (flags & (1 << 18))
/**@} */

/**@brief Default debug flags for @ref xinc_balloc. This is used by the @ref XINC_BALLOC_DEF macro.
 *        Flags can be changed in @ref sdk_config.
 */
#if XINC_BALLOC_CONFIG_DEBUG_ENABLED
    #define XINC_BALLOC_DEFAULT_DEBUG_FLAGS                                                      \
    (                                                                                           \
        XINC_BALLOC_DEBUG_HEAD_GUARD_WORDS_SET(XINC_BALLOC_CONFIG_HEAD_GUARD_WORDS)           |   \
        XINC_BALLOC_DEBUG_TAIL_GUARD_WORDS_SET(XINC_BALLOC_CONFIG_TAIL_GUARD_WORDS)           |   \
        XINC_BALLOC_DEBUG_BASIC_CHECKS_SET(XINC_BALLOC_CONFIG_BASIC_CHECKS_ENABLED)           |   \
        XINC_BALLOC_DEBUG_DOUBLE_FREE_CHECK_SET(XINC_BALLOC_CONFIG_DOUBLE_FREE_CHECK_ENABLED) |   \
        XINC_BALLOC_DEBUG_DATA_TRASHING_CHECK_SET(XINC_BALLOC_CONFIG_DATA_TRASHING_CHECK_ENABLED) \
    )
#else
    #define XINC_BALLOC_DEFAULT_DEBUG_FLAGS   0
#endif // XINC_BALLOC_CONFIG_DEBUG_ENABLED

/**@brief Block memory allocator control block.*/
typedef struct
{
    uint8_t * p_stack_pointer;          //!< Current allocation stack pointer.
    uint8_t   max_utilization;          //!< Maximum utilization of the memory pool.
} xinc_balloc_cb_t;

/**@brief Block memory allocator pool instance. The pool is made of elements of the same size. */
typedef struct
{
    xinc_balloc_cb_t * p_cb;             //!< Pointer to the instance control block.
    uint8_t         * p_stack_base;     //!< Base of the allocation stack.
                                        /**<
                                         * Stack is used to store handlers to not allocated elements.
                                         */
    uint8_t         * p_stack_limit;    //!< Maximum possible value of the allocation stack pointer.
    void            * p_memory_begin;   //!< Pointer to the start of the memory pool.
                                        /**<
                                         * Memory is used as a heap for blocks.
                                         */
    XINC_LOG_INSTANCE_PTR_DECLARE(p_log) //!< Pointer to instance of the logger object (Conditionally compiled).
#if XINC_BALLOC_HAS_NAME
    const char      * p_name;           //!< Pointer to string with pool name.
#endif
#if XINC_BALLOC_CONFIG_DEBUG_ENABLED
    uint32_t          debug_flags;      //!< Debugging settings.
                                        /**<
                                         * Debug flag should be created by @ref XINC_BALLOC_DEBUG.
                                         */
#endif // XINC_BALLOC_CONFIG_DEBUG_ENABLED
    uint16_t          block_size;       //!< Size of the allocated block (including debug overhead).
                                        /**<
                                         * Single block contains user element with header and tail
                                         * words.
                                         */
} xinc_balloc_t;

/**@brief Get total memory consumed by single block (element size with overhead caused by debug
 *        flags).
 *
 * @param[in]   _element_size    Size of an element.
 * @param[in]   _debug_flags     Debug flags.
 */
#if XINC_BALLOC_CONFIG_DEBUG_ENABLED
    #define XINC_BALLOC_BLOCK_SIZE(_element_size, _debug_flags)                      \
    (                                                                               \
       (sizeof(uint32_t) * XINC_BALLOC_DEBUG_HEAD_GUARD_WORDS_GET(_debug_flags)) +   \
       ALIGN_NUM(sizeof(uint32_t), (_element_size)) +                               \
       (sizeof(uint32_t) * XINC_BALLOC_DEBUG_TAIL_GUARD_WORDS_GET(_debug_flags))     \
    )
#else
    #define XINC_BALLOC_BLOCK_SIZE(_element_size, _debug_flags)  \
                ALIGN_NUM(sizeof(uint32_t), (_element_size))
#endif // XINC_BALLOC_CONFIG_DEBUG_ENABLED


/**@brief Get element size ( excluding debugging overhead is present)
 *        flags).
 *
 * @param[in]   _p_balloc   Pointer to balloc instance.
 */
#if XINC_BALLOC_CONFIG_DEBUG_ENABLED
#define XINC_BALLOC_ELEMENT_SIZE(_p_balloc) \
           (ALIGN_NUM(sizeof(uint32_t), (_p_balloc)->block_size) -                                 \
           ((sizeof(uint32_t) * XINC_BALLOC_DEBUG_HEAD_GUARD_WORDS_GET((_p_balloc)->debug_flags)) + \
           (sizeof(uint32_t) * XINC_BALLOC_DEBUG_TAIL_GUARD_WORDS_GET((_p_balloc)->debug_flags))))
#else
#define XINC_BALLOC_ELEMENT_SIZE(_p_balloc) \
           (_p_balloc)->block_size
#endif // XINC_BALLOC_CONFIG_DEBUG_ENABLED

#if XINC_BALLOC_CONFIG_DEBUG_ENABLED
#define __XINC_BALLOC_ASSIGN_DEBUG_FLAGS(_debug_flags)   .debug_flags = (_debug_flags),
#else
#define __XINC_BALLOC_ASSIGN_DEBUG_FLAGS(_debug_flags)
#endif

#if XINC_BALLOC_HAS_NAME
#define __XINC_BALLOC_ASSIGN_POOL_NAME(_name)            .p_name = STRINGIFY(_name),
#else
#define __XINC_BALLOC_ASSIGN_POOL_NAME(_name)
#endif


/**@brief Create a block allocator instance with custom debug flags.
 *
 * @note  This macro reserves memory for the given block allocator instance.
 *
 * @param[in]   _name           Name of the allocator.
 * @param[in]   _element_size   Size of one element.
 * @param[in]   _pool_size      Size of the pool.
 * @param[in]   _debug_flags    Debug flags (@ref XINC_BALLOC_DEBUG).
 */
#define XINC_BALLOC_DBG_DEF(_name, _element_size, _pool_size, _debug_flags)                      \
    STATIC_ASSERT((_pool_size) <= UINT8_MAX);                                                   \
    static uint8_t              CONCAT_2(_name, _xinc_balloc_pool_stack)[(_pool_size)];          \
    static uint32_t             CONCAT_2(_name,_xinc_balloc_pool_mem)                            \
        [XINC_BALLOC_BLOCK_SIZE(_element_size, _debug_flags) * (_pool_size) / sizeof(uint32_t)]; \
    static xinc_balloc_cb_t      CONCAT_2(_name,_xinc_balloc_cb);                                 \
    XINC_LOG_INSTANCE_REGISTER(XINC_BALLOC_LOG_NAME, _name,                                       \
                              XINC_BALLOC_CONFIG_INFO_COLOR,                                     \
                              XINC_BALLOC_CONFIG_DEBUG_COLOR,                                    \
                              XINC_BALLOC_CONFIG_INITIAL_LOG_LEVEL,                              \
                              XINC_BALLOC_CONFIG_LOG_ENABLED ?                                   \
                                      XINC_BALLOC_CONFIG_LOG_LEVEL : XINC_LOG_SEVERITY_NONE);     \
    XINC_SECTION_ITEM_REGISTER(xinc_balloc, const xinc_balloc_t  _name) =                          \
        {                                                                                       \
            .p_cb           = &CONCAT_2(_name,_xinc_balloc_cb),                                  \
            .p_stack_base   = CONCAT_2(_name,_xinc_balloc_pool_stack),                           \
            .p_stack_limit  = CONCAT_2(_name,_xinc_balloc_pool_stack) + (_pool_size),            \
            .p_memory_begin = CONCAT_2(_name,_xinc_balloc_pool_mem),                             \
            .block_size     = XINC_BALLOC_BLOCK_SIZE(_element_size, _debug_flags),               \
                                                                                                \
            XINC_LOG_INSTANCE_PTR_INIT(p_log, XINC_BALLOC_LOG_NAME, _name)                        \
            __XINC_BALLOC_ASSIGN_POOL_NAME(_name)                                                \
            __XINC_BALLOC_ASSIGN_DEBUG_FLAGS(_debug_flags)                                       \
        }

/**@brief Create a block allocator instance.
 *
 * @note  This macro reserves memory for the given block allocator instance.
 *
 * @param[in]   _name           Name of the allocator.
 * @param[in]   _element_size   Size of one element.
 * @param[in]   _pool_size      Size of the pool.
 */
#define XINC_BALLOC_DEF(_name, _element_size, _pool_size)                                           \
            XINC_BALLOC_DBG_DEF(_name, _element_size, _pool_size, XINC_BALLOC_DEFAULT_DEBUG_FLAGS)

/**@brief Create a block allocator interface.
 *
 * @param[in]   _type    Type which is allocated.
 * @param[in]   _name    Name of the allocator.
 */
#define XINC_BALLOC_INTERFACE_DEC(_type, _name)    \
    _type * CONCAT_2(_name,_alloc)(void);                  \
    void    CONCAT_2(_name,_free)(_type * p_element)

/**@brief Define a custom block allocator interface.
 *
 * @param[in]   _attr    Function attribute that will be added to allocator function definition.
 * @param[in]   _type    Type which is allocated.
 * @param[in]   _name    Name of the allocator.
 * @param[in]   _p_pool  Pool from which data will be allocated.
 */
#define XINC_BALLOC_INTERFACE_CUSTOM_DEF(_attr, _type, _name, _p_pool)           \
    _attr _type * CONCAT_2(_name,_alloc)(void)                                  \
    {                                                                           \
        GCC_PRAGMA("GCC diagnostic push")                                       \
        GCC_PRAGMA("GCC diagnostic ignored \"-Waddress\"")                      \
        ASSERT((_p_pool) != NULL);                                              \
        ASSERT((_p_pool)->block_size >=                                         \
               XINC_BALLOC_BLOCK_SIZE(sizeof(_type), (_p_pool)->debug_flags));   \
        GCC_PRAGMA("GCC diagnostic pop")                                        \
        return (_type *)(xinc_balloc_alloc(_p_pool));                            \
    }                                                                           \
                                                                                \
    _attr void CONCAT_2(_name,_free)(_type * p_element)                         \
    {                                                                           \
        GCC_PRAGMA("GCC diagnostic push")                                       \
        GCC_PRAGMA("GCC diagnostic ignored \"-Waddress\"")                      \
        ASSERT((_p_pool) != NULL);                                              \
        ASSERT((_p_pool)->block_size >=                                         \
               XINC_BALLOC_BLOCK_SIZE(sizeof(_type), (_p_pool)->debug_flags));   \
        GCC_PRAGMA("GCC diagnostic pop")                                        \
        xinc_balloc_free((_p_pool), p_element);                                  \
    }

/**@brief Define block allocator interface.
 *
 * @param[in]   _type    Type which is allocated.
 * @param[in]   _name    Name of the allocator.
 * @param[in]   _p_pool  Pool from which data will be allocated.
 */
#define XINC_BALLOC_INTERFACE_DEF(_type, _name, _p_pool)        \
        XINC_BALLOC_INTERFACE_CUSTOM_DEF(/* empty */, _type, _name, _p_pool)

/**@brief Define a local block allocator interface.
 *
 * @param[in]   _type    Type which is allocated.
 * @param[in]   _name    Name of the allocator.
 * @param[in]   _p_pool  Pool from which data will be allocated.
 */
#define XINC_BALLOC_INTERFACE_LOCAL_DEF(_type, _name, _p_pool)  \
        XINC_BALLOC_INTERFACE_CUSTOM_DEF(static, _type, _name, _p_pool)

/**@brief Function for initializing a block memory allocator pool.
 *
 * @param[out]  p_pool          Pointer to the pool that is to be initialized.
 *
 * @return  XINC_SUCCESS on success, otherwise error code.
 */
ret_code_t xinc_balloc_init(xinc_balloc_t const * p_pool);

/**@brief Function for allocating an element from the pool.
 *
 * @note    This module guarantees that the returned memory is aligned to 4.
 *
 * @param[in]   p_pool  Pointer to the memory pool from which the element will be allocated.
 *
 * @return      Allocated element or NULL if the specified pool is empty.
 */
void * xinc_balloc_alloc(xinc_balloc_t const * p_pool);

/**@brief Function for freeing an element back to the pool.
 *
 * @param[in]   p_pool      Pointer to the memory pool.
 * @param[in]   p_element   Element to be freed.
 */
void xinc_balloc_free(xinc_balloc_t const * p_pool, void * p_element);

/**@brief Function for getting maximum memory pool utilization.
 *
 * @param[in]   p_pool Pointer to the memory pool instance.
 *
 * @return Maximum number of elements allocated from the pool.
 */
__STATIC_INLINE uint8_t xinc_balloc_max_utilization_get(xinc_balloc_t const * p_pool);

#ifndef SUPPRESS_INLINE_IMPLEMENTATION
__STATIC_INLINE uint8_t xinc_balloc_max_utilization_get(xinc_balloc_t const * p_pool)
{
    ASSERT(p_pool != NULL);
    return p_pool->p_cb->max_utilization;
}
#endif //SUPPRESS_INLINE_IMPLEMENTATION

/**@brief Function for getting current memory pool utilization.
 *
 * @param[in]   p_pool Pointer to the memory pool instance.
 *
 * @return Maximum number of elements allocated from the pool.
 */
__STATIC_INLINE uint8_t xinc_balloc_utilization_get(xinc_balloc_t const * p_pool);

#ifndef SUPPRESS_INLINE_IMPLEMENTATION
__STATIC_INLINE uint8_t xinc_balloc_utilization_get(xinc_balloc_t const * p_pool)
{
    ASSERT(p_pool != NULL);
    return (p_pool->p_stack_limit - p_pool->p_cb->p_stack_pointer);
}
#endif //SUPPRESS_INLINE_IMPLEMENTATION

#ifdef __cplusplus
}
#endif

#endif // XINC_BALLOC_H__
/** @} */
