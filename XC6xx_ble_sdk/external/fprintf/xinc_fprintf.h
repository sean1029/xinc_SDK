/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#ifndef XINC_FPRINTF_H__
#define XINC_FPRINTF_H__

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (* xinc_fprintf_fwrite)(void const * p_user_ctx, char const * p_str, size_t length);

/**
 * @brief fprintf context
 */
typedef struct xinc_fprintf_ctx
{
    char * const p_io_buffer;       ///< Pointer to IO buffer.
    size_t const io_buffer_size;    ///< IO buffer size.
    size_t io_buffer_cnt;           ///< IO buffer usage.
    bool auto_flush;                ///< Auto flush configurator.

    void const * const p_user_ctx;  ///< Pointer to user data to be passed to the fwrite funciton.

    xinc_fprintf_fwrite fwrite;      ///< Pointer to function sending data stream.
} xinc_fprintf_ctx_t;


/**
 * @brief Macro for defining xinc_fprintf instance.
 *
 * @param name              Instance name.
 * @param _p_user_ctx       Pointer to user data.
 * @param _p_io_buffer      Pointer to IO buffer
 * @param _io_buffer_size   Size of IO buffer.
 * @param _auto_flush       Indicator if IO buffer shall be automatically flush.
 * @param _fwrite           Pointer to function sending data stream.
 * */
#define XINC_FPRINTF_DEF(name, _p_user_ctx, _p_io_buffer, _io_buffer_size, _auto_flush, _fwrite) \
    static xinc_fprintf_ctx_t name =                                                             \
    {                                                                                           \
        .p_io_buffer = _p_io_buffer,                                                            \
        .io_buffer_size = _io_buffer_size,                                                      \
        .io_buffer_cnt = 0,                                                                     \
        .auto_flush = _auto_flush,                                                              \
        .p_user_ctx = _p_user_ctx,                                                              \
        .fwrite = _fwrite                                                                       \
    }

/**
 * @brief fprintf like function which send formated data stream to output specified by user
 * @ref xinc_fprintf_ctx_t
 *
 * @param p_ctx     fprintf context.
 * @param p_fmt     Format string.
 * @param ...       List of parameters to print.
 * */
void xinc_fprintf(xinc_fprintf_ctx_t * const p_ctx,
                 char const *              p_fmt,
                                           ...);

/**
 * @brief function flushing data stored in io_buffer @ref xinc_fprintf_ctx_t
 *
 * @param p_ctx fprintf context
 */
void xinc_fprintf_buffer_flush(xinc_fprintf_ctx_t * const p_ctx);


#ifdef __cplusplus
}
#endif

#endif /* XINC_FPRINTF_H__ */

