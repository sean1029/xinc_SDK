/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#ifndef XINC_LOG_BACKEND_INTERFACE_H
#define XINC_LOG_BACKEND_INTERFACE_H

/**@file
 * @addtogroup xinc_log Logger module
 * @ingroup    app_common
 *
 * @defgroup xinc_log_backend_interface Logger backend interface
 * @{
 * @ingroup  xinc_log
 * @brief    The xinc_log backend interface.
 */

#if XINC_MODULE_ENABLED(XINC_LOG)
#include "xinc_memobj.h"
#endif

#include "xinc_section.h"
#include "xinchip_common.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief xinc_log entry.
 */
#if XINC_LOG_ENABLED
typedef xinc_memobj_t xinc_log_entry_t;
#else
typedef void xinc_log_entry_t;
#endif

/* Forward declaration of the xinc_log_backend_t type. */
typedef struct xinc_log_backend_s xinc_log_backend_t;

/**
 * @brief Logger backend API.
 */
typedef struct
{
    /**
     * @brief @ref xinc_log_backend_put
     */
    void (*put)(xinc_log_backend_t const * p_backend, xinc_log_entry_t * p_entry);

    /**
     * @brief @ref xinc_log_backend_panic_set
     */
    void (*panic_set)(xinc_log_backend_t const * p_backend);

    /**
     * @brief @ref xinc_log_backend_flush
     */
    void (*flush)(xinc_log_backend_t const * p_backend);
} xinc_log_backend_api_t;

/**
 * @brief Logger backend control block.
 */
typedef struct
{
    xinc_log_backend_t const * p_next;  //!< Pointer to the next backend in the logger.
    uint8_t                   id;      //!< ID of the backend.
    bool                      enabled; //!< Enable flag.
} xinc_log_backend_cb_t;

/**
 * @brief Logger backend structure.
 */
struct xinc_log_backend_s
{
    xinc_log_backend_api_t const * p_api;  //!< Pointer to interface.
    void *                        p_ctx;  //!< User context.
    char *                        p_name; //!< Name of the backend.
    xinc_log_backend_cb_t *        p_cb;   //!< Pointer to the backend control block.
};


#define XINC_LOG_BACKEND_SECTION_NAME log_backends

#define XINC_LOG_BACKEND_SUBSECTION_NAME(_name) XINC_LOG_BACKEND_SECTION_NAME

/** @brief Invalid ID value indicating that logger backend is not attached to the logger frontend.*/
#define XINC_LOG_BACKEND_INVALID_ID 0xFF

/** @brief Memory section where backends are located. */
XINC_SECTION_DEF(XINC_LOG_BACKEND_SECTION_NAME, xinc_log_backend_t);

/**
 * @brief Macro for creating a logger backend instance.
 *
 * @param _name  Name of the backend instance.
 * @param _api   Logger backend API.
 * @param _p_ctx Pointer to the user context.
 */
#define XINC_LOG_BACKEND_DEF(_name, _api, _p_ctx)                           \
    static xinc_log_backend_cb_t CONCAT_2(log_backend_cb_, _name) = {       \
            .enabled = false,                                              \
            .id      = XINC_LOG_BACKEND_INVALID_ID,                         \
            .p_next  = NULL                                                \
    };                                                                     \
    XINC_SECTION_ITEM_REGISTER(XINC_LOG_BACKEND_SUBSECTION_NAME(_name),      \
          static const xinc_log_backend_t _name) = {                        \
            .p_api = &_api,                                                \
            .p_ctx = _p_ctx,                                               \
            .p_cb = &CONCAT_2(log_backend_cb_, _name),                     \
            .p_name = (char *)STRINGIFY(_name)                             \
   }


/**
 * @brief Function for putting message with log entry to the backend.
 *
 * @param[in] p_backend  Pointer to the backend instance.
 * @param[in] p_msg      Pointer to message with log entry.
 */
__STATIC_INLINE void xinc_log_backend_put(xinc_log_backend_t const * const p_backend,
                                         xinc_log_entry_t * p_msg);

/**
 * @brief Function for reconfiguring backend to panic mode.
 *
 * @param[in] p_backend  Pointer to the backend instance.
 */
__STATIC_INLINE void xinc_log_backend_panic_set(xinc_log_backend_t const * const p_backend);

/**
 * @brief Function for flushing backend.
 *
 *        On flushing request backend should release log message(s).
 *
 * @param[in] p_backend  Pointer to the backend instance.
 */
__STATIC_INLINE void xinc_log_backend_flush(xinc_log_backend_t const * const p_backend);


/**
 * @brief Function for setting backend id.
 *
 * @note It is used internally by the logger.
 *
 * @param[in] p_backend  Pointer to the backend instance.
 * @param[in] id         Id.
 */
__STATIC_INLINE void xinc_log_backend_id_set(xinc_log_backend_t const * const p_backend, uint8_t id);

/**
 * @brief Function for getting backend id.
 *
 * @note It is used internally by the logger.
 *
 * @param[in] p_backend  Pointer to the backend instance.
 * @return    Id.
 */
__STATIC_INLINE uint8_t xinc_log_backend_id_get(xinc_log_backend_t const * const p_backend);

/**
 * @brief Function for enabling backend.
 *
 * @param[in] p_backend  Pointer to the backend instance.
 */
__STATIC_INLINE void xinc_log_backend_enable(xinc_log_backend_t const * const p_backend);

/**
 * @brief Function for disabling backend.
 *
 * @param[in] p_backend  Pointer to the backend instance.
 */
__STATIC_INLINE void xinc_log_backend_disable(xinc_log_backend_t const * const p_backend);

/**
 * @brief Function for checking state of the backend.
 *
 * @param[in] p_backend  Pointer to the backend instance.
 *
 * @return True if backend is enabled, false otherwise.
 */
__STATIC_INLINE bool xinc_log_backend_is_enabled(xinc_log_backend_t const * const p_backend);

#ifndef SUPPRESS_INLINE_IMPLEMENTATION
__STATIC_INLINE void xinc_log_backend_put(xinc_log_backend_t const * const p_backend,
                                         xinc_log_entry_t * p_msg)
{
    p_backend->p_api->put(p_backend, p_msg);
}

__STATIC_INLINE void xinc_log_backend_panic_set(xinc_log_backend_t const * const p_backend)
{
    p_backend->p_api->panic_set(p_backend);
}

__STATIC_INLINE void xinc_log_backend_flush(xinc_log_backend_t const * const p_backend)
{
    p_backend->p_api->flush(p_backend);
}

__STATIC_INLINE void xinc_log_backend_id_set(xinc_log_backend_t const * const p_backend, uint8_t id)
{
    p_backend->p_cb->id = id;
}

__STATIC_INLINE uint8_t xinc_log_backend_id_get(xinc_log_backend_t const * const p_backend)
{
    return p_backend->p_cb->id;
}

__STATIC_INLINE void xinc_log_backend_enable(xinc_log_backend_t const * const p_backend)
{
    p_backend->p_cb->enabled = true;
}

__STATIC_INLINE void xinc_log_backend_disable(xinc_log_backend_t const * const p_backend)
{
    p_backend->p_cb->enabled = false;
}

__STATIC_INLINE bool xinc_log_backend_is_enabled(xinc_log_backend_t const * const p_backend)
{
    return p_backend->p_cb->enabled;
}

#endif // SUPPRESS_INLINE_IMPLEMENTATION

#ifdef __cplusplus
}
#endif

#endif //XINC_LOG_BACKEND_INTERFACE_H

/** @} */
