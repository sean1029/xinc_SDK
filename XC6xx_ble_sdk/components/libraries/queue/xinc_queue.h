/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
/**
* @defgroup xinc_queue Queue module
* @{
* @ingroup app_common
* @brief Functions that handle the queue instances.
*/

#ifndef XINC_QUEUE_H__
#define XINC_QUEUE_H__

#include <stdint.h>
#include <stdint.h>
#include <string.h>
#include "xinc_assert.h"
#include "sdk_errors.h"
#include "app_util.h"
#include "app_util_platform.h"
#include "xinc_log_instance.h"
#include "xinc_section.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Name of the module used for logger messaging.
 */
#define XINC_QUEUE_LOG_NAME queue

/**@brief Queue control block. */
typedef struct
{
    volatile size_t front;          //!< Queue front index.
    volatile size_t back;           //!< Queue back index.
    size_t max_utilization;         //!< Maximum utilization of the queue.
} xinc_queue_cb_t;

/**@brief Supported queue modes. */
typedef enum
{
    XINC_QUEUE_MODE_OVERFLOW,        //!< If the queue is full, new element will overwrite the oldest.
    XINC_QUEUE_MODE_NO_OVERFLOW,     //!< If the queue is full, new element will not be accepted.
} xinc_queue_mode_t;

/**@brief Instance of the queue. */
typedef struct
{
    xinc_queue_cb_t * p_cb;              //!< Pointer to the instance control block.
    void           * p_buffer;          //!< Pointer to the memory that is used as storage.
    size_t           size;              //!< Size of the queue.
    size_t           element_size;      //!< Size of one element.
    xinc_queue_mode_t mode;              //!< Mode of the queue.
#if XINC_QUEUE_CLI_CMDS
    const char      * p_name;           //!< Pointer to string with queue name.
#endif
    XINC_LOG_INSTANCE_PTR_DECLARE(p_log) //!< Pointer to instance of the logger object (Conditionally compiled).
} xinc_queue_t;

#if XINC_QUEUE_CLI_CMDS
#define __XINC_QUEUE_ASSIGN_POOL_NAME(_name)            .p_name = STRINGIFY(_name),
#else
#define __XINC_QUEUE_ASSIGN_POOL_NAME(_name)
#endif
/**@brief Create a queue instance.
 *
 * @note  This macro reserves memory for the given queue instance.
 *
 * @param[in]   _type       Type which is stored.
 * @param[in]   _name       Name of the queue.
 * @param[in]   _size       Size of the queue.
 * @param[in]   _mode       Mode of the queue.
 */
#define XINC_QUEUE_DEF(_type, _name, _size, _mode)                                        \
    static _type             CONCAT_2(_name, _xinc_queue_buffer[(_size) + 1]);            \
    static xinc_queue_cb_t    CONCAT_2(_name, _xinc_queue_cb);                             \
    XINC_LOG_INSTANCE_REGISTER(XINC_QUEUE_LOG_NAME, _name,                                 \
                                  XINC_QUEUE_CONFIG_INFO_COLOR,                           \
                                  XINC_QUEUE_CONFIG_DEBUG_COLOR,                          \
                                  XINC_QUEUE_CONFIG_LOG_INIT_FILTER_LEVEL,                \
                                  XINC_QUEUE_CONFIG_LOG_ENABLED ?                         \
                                    XINC_QUEUE_CONFIG_LOG_LEVEL : XINC_LOG_SEVERITY_NONE); \
     XINC_SECTION_ITEM_REGISTER(xinc_queue, const xinc_queue_t  _name) =                    \
        {                                                                                \
            .p_cb           = &CONCAT_2(_name, _xinc_queue_cb),                           \
            .p_buffer       = CONCAT_2(_name,_xinc_queue_buffer),                         \
            .size           = (_size),                                                   \
            .element_size   = sizeof(_type),                                             \
            .mode           = _mode,                                                     \
            __XINC_QUEUE_ASSIGN_POOL_NAME(_name)                                          \
            XINC_LOG_INSTANCE_PTR_INIT(p_log, XINC_QUEUE_LOG_NAME, _name)                  \
        }

#if !(defined(__LINT__))
/**@brief Create multiple queue instances.
 *
 * @note  This macro reserves memory for array of queue instances.
 *
 * @param[in]   _type       Type which is stored.
 * @param[in]   _name       Name of the array with queue instances.
 * @param[in]   _size       Size of single queue instance.
 * @param[in]   _mode       Mode of single queue instance.
 * @param[in]   _num        Number of queue instances within array.
 */
#define XINC_QUEUE_ARRAY_DEF(_type, _name, _size, _mode, _num)                                 \
    MACRO_REPEAT_FOR(_num, XINC_QUEUE_ARRAY_INSTANCE_ELEMS_DEC, _type, _name, _size, _mode)    \
    static const xinc_queue_t _name[] =                                                        \
        {                                                                                     \
            MACRO_REPEAT_FOR(_num, XINC_QUEUE_ARRAY_INSTANCE_INIT, _type, _name, _size, _mode) \
        };                                                                                    \
    STATIC_ASSERT(ARRAY_SIZE(_name) == _num)
#else
#define XINC_QUEUE_ARRAY_DEF(_type, _name, _size, _mode, _num) \
    static const xinc_queue_t _name[_num];
#endif // !(defined(__LINT__))

/**@brief Helping macro used to declare elements for xinc_queue_t instance.
 *        Used in @ref XINC_QUEUE_ARRAY_DEF.
 */
#define XINC_QUEUE_ARRAY_INSTANCE_ELEMS_DEC(_num, _type, _name, _size, _mode)     \
    static _type          CONCAT_3(_name, _num, _xinc_queue_buffer[(_size) + 1]); \
    static xinc_queue_cb_t CONCAT_3(_name, _num, _xinc_queue_cb);

/**@brief Helping macro used to initialize xinc_queue_t instance in an array fashion.
 *        Used in @ref XINC_QUEUE_ARRAY_DEF.
 */
#define XINC_QUEUE_ARRAY_INSTANCE_INIT(_num, _type, _name, _size, _mode) \
    {                                                                   \
        .p_cb           = &CONCAT_3(_name, _num, _xinc_queue_cb),        \
        .p_buffer       = CONCAT_3(_name, _num, _xinc_queue_buffer),     \
        .size           = (_size),                                      \
        .element_size   = sizeof(_type),                                \
        .mode           = _mode,                                        \
    },

/**@brief Declare a queue interface.
 *
 * @param[in]   _type    Type which is stored.
 * @param[in]   _name    Name of the queue.
 */
#define XINC_QUEUE_INTERFACE_DEC(_type, _name)               \
    ret_code_t  _name##_push(_type const * p_element);      \
    ret_code_t  _name##_pop(_type * p_element);             \
    ret_code_t  _name##_peek(_type * p_element);            \
    ret_code_t  _name##_write(_type const * p_data,         \
                              size_t        element_count); \
    ret_code_t  _name##_read(_type * p_data,                \
                             size_t  element_count);        \
    size_t      _name##_out(_type * p_data,                 \
                            size_t  element_count);         \
    size_t      _name##_in(_type const * p_data,            \
                            size_t element_count);          \
    bool        _name##_is_full(void);                      \
    bool        _name##_is_empty(void);                     \
    size_t      _name##_utilization_get(void);              \
    size_t      _name##_available_get(void);                \
    size_t      _name##_max_utilization_get(void);          \
    void        _name##_reset(void)

/**@brief Define a queue interface.
 *
 * @param[in]   _type    Type which is stored.
 * @param[in]   _name    Name of the queue.
 * @param[in]   _p_queue Queue instance.
 */
#define XINC_QUEUE_INTERFACE_DEF(_type, _name, _p_queue)                 \
    ret_code_t _name##_push(_type const * p_element)                    \
    {                                                                   \
        GCC_PRAGMA("GCC diagnostic push")                               \
        GCC_PRAGMA("GCC diagnostic ignored \"-Waddress\"")              \
        ASSERT((_p_queue) != NULL);                                     \
        ASSERT((_p_queue)->element_size == sizeof(_type));              \
        GCC_PRAGMA("GCC diagnostic pop")                                \
        return xinc_queue_push((_p_queue), p_element);                   \
    }                                                                   \
    ret_code_t _name##_pop(_type * p_element)                           \
    {                                                                   \
        GCC_PRAGMA("GCC diagnostic push")                               \
        GCC_PRAGMA("GCC diagnostic ignored \"-Waddress\"")              \
        ASSERT((_p_queue) != NULL);                                     \
        ASSERT((_p_queue)->element_size == sizeof(_type));              \
        GCC_PRAGMA("GCC diagnostic pop")                                \
        return xinc_queue_pop((_p_queue), p_element);                    \
    }                                                                   \
    ret_code_t _name##_peek(_type * p_element)                          \
    {                                                                   \
        GCC_PRAGMA("GCC diagnostic push")                               \
        GCC_PRAGMA("GCC diagnostic ignored \"-Waddress\"")              \
        ASSERT((_p_queue) != NULL);                                     \
        ASSERT((_p_queue)->element_size == sizeof(_type));              \
        GCC_PRAGMA("GCC diagnostic pop")                                \
        return xinc_queue_peek((_p_queue), p_element);                   \
    }                                                                   \
    ret_code_t _name##_write(_type const * p_data,                      \
                             size_t        element_count)               \
    {                                                                   \
        GCC_PRAGMA("GCC diagnostic push")                               \
        GCC_PRAGMA("GCC diagnostic ignored \"-Waddress\"")              \
        ASSERT((_p_queue) != NULL);                                     \
        ASSERT((_p_queue)->element_size == sizeof(_type));              \
        GCC_PRAGMA("GCC diagnostic pop")                                \
        return xinc_queue_write((_p_queue), p_data, element_count);      \
    }                                                                   \
    ret_code_t _name##_read(_type * p_data,                             \
                            size_t  element_count)                      \
    {                                                                   \
        GCC_PRAGMA("GCC diagnostic push")                               \
        GCC_PRAGMA("GCC diagnostic ignored \"-Waddress\"")              \
        ASSERT((_p_queue) != NULL);                                     \
        ASSERT((_p_queue)->element_size == sizeof(_type));              \
        GCC_PRAGMA("GCC diagnostic pop")                                \
        return xinc_queue_read((_p_queue), p_data, element_count);       \
    }                                                                   \
    size_t _name##_in(_type const * p_data,                             \
                      size_t  element_count)                            \
    {                                                                   \
        GCC_PRAGMA("GCC diagnostic push")                               \
        GCC_PRAGMA("GCC diagnostic ignored \"-Waddress\"")              \
        ASSERT((_p_queue) != NULL);                                     \
        ASSERT((_p_queue)->element_size == sizeof(_type));              \
        GCC_PRAGMA("GCC diagnostic pop")                                \
        return xinc_queue_in((_p_queue), p_data, element_count);         \
    }                                                                   \
    size_t _name##_out(_type * p_data,                                  \
                       size_t  element_count)                           \
    {                                                                   \
        GCC_PRAGMA("GCC diagnostic push")                               \
        GCC_PRAGMA("GCC diagnostic ignored \"-Waddress\"")              \
        ASSERT((_p_queue) != NULL);                                     \
        ASSERT((_p_queue)->element_size == sizeof(_type));              \
        GCC_PRAGMA("GCC diagnostic pop")                                \
        return xinc_queue_out((_p_queue), p_data, element_count);        \
    }                                                                   \
    bool _name##_is_full(void)                                          \
    {                                                                   \
        GCC_PRAGMA("GCC diagnostic push")                               \
        GCC_PRAGMA("GCC diagnostic ignored \"-Waddress\"")              \
        ASSERT((_p_queue) != NULL);                                     \
        return xinc_queue_is_full(_p_queue);                             \
        GCC_PRAGMA("GCC diagnostic pop")                                \
    }                                                                   \
    bool _name##_is_empty(void)                                         \
    {                                                                   \
        GCC_PRAGMA("GCC diagnostic push")                               \
        GCC_PRAGMA("GCC diagnostic ignored \"-Waddress\"")              \
        ASSERT((_p_queue) != NULL);                                     \
        GCC_PRAGMA("GCC diagnostic pop")                                \
        return xinc_queue_is_empty(_p_queue);                            \
    }                                                                   \
    size_t _name##_utilization_get(void)                                \
    {                                                                   \
        GCC_PRAGMA("GCC diagnostic push")                               \
        GCC_PRAGMA("GCC diagnostic ignored \"-Waddress\"")              \
        ASSERT((_p_queue) != NULL);                                     \
        GCC_PRAGMA("GCC diagnostic pop")                                \
        return xinc_queue_utilization_get(_p_queue);                     \
    }                                                                   \
    size_t _name##_available_get(void)                                  \
    {                                                                   \
        GCC_PRAGMA("GCC diagnostic push")                               \
        GCC_PRAGMA("GCC diagnostic ignored \"-Waddress\"")              \
        ASSERT((_p_queue) != NULL);                                     \
        GCC_PRAGMA("GCC diagnostic pop")                                \
        return xinc_queue_available_get(_p_queue);                       \
    }                                                                   \
    size_t _name##_max_utilization_get(void)                            \
    {                                                                   \
        GCC_PRAGMA("GCC diagnostic push")                               \
        GCC_PRAGMA("GCC diagnostic ignored \"-Waddress\"")              \
        ASSERT((_p_queue) != NULL);                                     \
        GCC_PRAGMA("GCC diagnostic pop")                                \
        return xinc_queue_max_utilization_get(_p_queue);                 \
    }                                                                   \
    void _name##_reset(void)                                            \
    {                                                                   \
        GCC_PRAGMA("GCC diagnostic push")                               \
        GCC_PRAGMA("GCC diagnostic ignored \"-Waddress\"")              \
        ASSERT((_p_queue) != NULL);                                     \
        GCC_PRAGMA("GCC diagnostic pop")                                \
        xinc_queue_reset(_p_queue);                                      \
    }

/**@brief Function for pushing an element to the end of queue.
 *
 * @param[in]   p_queue             Pointer to the xinc_queue_t instance.
 * @param[in]   p_element           Pointer to the element that will be stored in the queue.
 *
 * @return      XINC_SUCCESS         If an element has been successfully added.
 * @return      XINC_ERROR_NO_MEM    If the queue is full (only in @ref XINC_QUEUE_MODE_NO_OVERFLOW).
 */
ret_code_t xinc_queue_push(xinc_queue_t const * p_queue, void const * p_element);

/**@brief Generic pop implementation.
 *
 * @param[in]   p_queue             Pointer to the xinc_queue_t instance.
 * @param[out]  p_element           Pointer where the element will be copied.
 * @param[out]  just_peek           If true, the returned element will not be removed from queue.
 *
 * @return      XINC_SUCCESS         If an element was returned.
 * @return      XINC_ERROR_NOT_FOUND If there are no more elements in the queue.
 */
ret_code_t xinc_queue_generic_pop(xinc_queue_t const * p_queue,
                                 void              * p_element,
                                 bool                just_peek);

/**@brief Pop element from the front of the queue.
 *
 * @param[in]   _p_queue            Pointer to the xinc_queue_t instance.
 * @param[out]  _p_element          Pointer where the element will be copied.
 *
 * @return      XINC_SUCCESS         If an element was returned.
 * @return      XINC_ERROR_NOT_FOUND If there are no more elements in the queue.
 */
#define xinc_queue_pop(_p_queue, _p_element) xinc_queue_generic_pop((_p_queue), (_p_element), false)

/**@brief Peek element from the front of the queue.
 *
 * @param[in]   _p_queue            Pointer to the xinc_queue_t instance.
 * @param[out]  _p_element          Pointer where the element will be copied.
 *
 * @return      XINC_SUCCESS         If an element was returned.
 * @return      XINC_ERROR_NOT_FOUND If there are no more elements in the queue.
 */
#define xinc_queue_peek(_p_queue, _p_element) xinc_queue_generic_pop((_p_queue), (_p_element), true)

/**@brief Function for writing elements to the queue.
 *
 * @param[in]   p_queue             Pointer to the xinc_queue_t instance.
 * @param[in]   p_data              Pointer to the buffer with elements to write.
 * @param[in]   element_count       Number of elements to write.
 *
 * @return      XINC_SUCCESS         If an element was written.
 * @return      XINC_ERROR_NO_MEM    There is not enough space in the queue. No element was written.
 */
ret_code_t xinc_queue_write(xinc_queue_t const * p_queue,
                           void const        * p_data,
                           size_t              element_count);

/**@brief Function for writing a portion of elements to the queue.
 *
 * @param[in]   p_queue             Pointer to the xinc_queue_t instance.
 * @param[in]   p_data              Pointer to the buffer with elements to write.
 * @param[in]   element_count       Number of elements to write.
 *
 * @return      The number of added elements.
 */
size_t xinc_queue_in(xinc_queue_t const * p_queue,
                    void const        * p_data,
                    size_t              element_count);

/**@brief Function for reading elements from the queue.
 *
 * @param[in]   p_queue             Pointer to the xinc_queue_t instance.
 * @param[out]  p_data              Pointer to the buffer where elements will be copied.
 * @param[in]   element_count       Number of elements to read.
 *
 * @return      XINC_SUCCESS         If an element was returned.
 * @return      XINC_ERROR_NOT_FOUND There is not enough elements in the queue.
 */
ret_code_t xinc_queue_read(xinc_queue_t const * p_queue,
                          void              * p_data,
                          size_t              element_count);

/**@brief Function for reading a portion of elements from the queue.
 *
 * @param[in]   p_queue             Pointer to the xinc_queue_t instance.
 * @param[out]  p_data              Pointer to the buffer where elements will be copied.
 * @param[in]   element_count       Number of elements to read.
 *
 * @return      The number of read elements.
 */
size_t xinc_queue_out(xinc_queue_t const * p_queue,
                    void               * p_data,
                    size_t               element_count);

/**@brief Function for checking if the queue is full.
 *
 * @param[in]   p_queue     Pointer to the queue instance.
 *
 * @return      True if the queue is full.
 */
bool xinc_queue_is_full(xinc_queue_t const * p_queue);

/**@brief Function for checking if the queue is empty.
 *
 * @param[in]   p_queue     Pointer to the queue instance.
 *
 * @return      True if the queue is empty.
 */
bool xinc_queue_is_empty(xinc_queue_t const * p_queue);

/**@brief Function for getting the current queue utilization.
 *
 * @param[in]   p_queue     Pointer to the queue instance.
 *
 * @return      Current queue utilization.
 */
size_t xinc_queue_utilization_get(xinc_queue_t const * p_queue);

/**@brief Function for getting the size of available space.
 *
 * @param[in]   p_queue     Pointer to the queue instance.
 *
 * @return      Size of available space.
 */
size_t xinc_queue_available_get(xinc_queue_t const * p_queue);

/**@brief Function for getting the maximal queue utilization.
 *
 * @param[in]   p_queue     Pointer to the queue instance.
 *
 * @return      Maximal queue utilization.
 */
size_t xinc_queue_max_utilization_get(xinc_queue_t const * p_queue);

/**@brief Function for resetting the maximal queue utilization.
 *
 * @param[in]   p_queue     Pointer to the queue instance.
 *
 */
void xinc_queue_max_utilization_reset(xinc_queue_t const * p_queue);

/**@brief Function for resetting the queue state.
 *
 * @param[in]   p_queue     Pointer to the queue instance.
 */
void xinc_queue_reset(xinc_queue_t const * p_queue);

#ifdef __cplusplus
}
#endif

#endif // XINC_QUEUE_H__
/** @} */
