/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef _XINC_DELAY_H
#define _XINC_DELAY_H

#include <xincx.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Function for delaying execution for a number of microseconds.
 *
 * @param us_time Number of microseconds to wait.
 */
#define xinc_delay_us(us_time) XINCX_DELAY_US(us_time)
#define xinc_delay_init(void) XINCX_DELAY_INIT(void)

/**
 * @brief Function for delaying execution for a number of milliseconds.
 *
 * @param ms_time Number of milliseconds to wait.
 */

__STATIC_INLINE void xinc_delay_ms(uint32_t ms_time)
{
    if (ms_time == 0)
    {
        return;
    }

    do {
        xinc_delay_us(1000);
    } while (--ms_time);
}

#ifdef __cplusplus
}
#endif

#endif
