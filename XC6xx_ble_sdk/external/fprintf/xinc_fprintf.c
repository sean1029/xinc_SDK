/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include "sdk_common.h"
#if XINC_MODULE_ENABLED(XINC_FPRINTF)
#include <stdarg.h>

#include "xinc_assert.h"
#include "xinc_fprintf_format.h"

void xinc_fprintf_buffer_flush(xinc_fprintf_ctx_t * const p_ctx)
{
    ASSERT(p_ctx != NULL);

    if (p_ctx->io_buffer_cnt == 0)
    {
        return;
    }

    p_ctx->fwrite(p_ctx->p_user_ctx,
                  p_ctx->p_io_buffer,
                  p_ctx->io_buffer_cnt);
    p_ctx->io_buffer_cnt = 0;
}
#include <stdio.h>
void xinc_fprintf(xinc_fprintf_ctx_t * const p_ctx,
                 char const *              p_fmt,
                                           ...)
{
    ASSERT(p_ctx != NULL);
    ASSERT(p_ctx->fwrite != NULL);
    ASSERT(p_ctx->p_io_buffer != NULL);
    ASSERT(p_ctx->io_buffer_size > 0);

    if (p_fmt == NULL)
    {
        return;
    }

    va_list args = {0};

    va_start(args, p_fmt);
			
    xinc_fprintf_fmt(p_ctx, p_fmt, &args);

    va_end(args);
}

#endif // XINC_MODULE_ENABLED(XINC_FPRINTF)

