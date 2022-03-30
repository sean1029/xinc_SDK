/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINC_DMAS_H__
#define XINC_DMAS_H__

#include <xincx.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xinc_dmas_hal DMAS HAL
 * @{
 * @ingroup xinc_dmas
 * @brief   Hardware access layer for managing the DMAS peripheral.
 */
typedef enum
{
    DMAS_CH_0 = 0,
    DMAS_CH_1 = 1,
    DMAS_CH_2 = 2,
    DMAS_CH_3 = 3,
    DMAS_CH_8 = 8,
    DMAS_CH_9 = 9,
    DMAS_CH_10 = 10,
    DMAS_CH_11 = 11,
    DMAS_CH_12 = 12,
}xinc_dma_ch_t;


__STATIC_INLINE void xinc_dmas_enable(XINC_DMAS_Type * p_reg,xinc_dma_ch_t ch)
{
    p_reg->DMAs_EN = ch;
}

__STATIC_INLINE void xinc_dmas_clear(XINC_DMAS_Type * p_reg,xinc_dma_ch_t ch)
{
    p_reg->DMAs_CLR = ch;
}


__STATIC_INLINE uint32_t xinc_dmas_stat(XINC_DMAS_Type * p_reg)
{
    return p_reg->DMAs_STA;
}


__STATIC_INLINE uint32_t xinc_dmas_int_set_get(XINC_DMAS_Type * p_reg)
{
    return p_reg->DMAs_INT_EN0;
}

__STATIC_INLINE void xinc_dmas_int_enable(XINC_DMAS_Type * p_reg,uint32_t mask)
{
    
    p_reg->DMAs_INT_EN0  |= mask;
}

__STATIC_INLINE void xinc_dmas_int_disable(XINC_DMAS_Type * p_reg,uint32_t mask)
{
    
    p_reg->DMAs_INT_EN0  &= ~mask;
}

__STATIC_INLINE uint32_t xinc_dmas_int_sta(XINC_DMAS_Type * p_reg)
{
    return p_reg->DMAs_INT0;
}

__STATIC_INLINE uint32_t xinc_dmas_int_raw_sta(XINC_DMAS_Type * p_reg)
{
    return p_reg->DMAs_INT_RAW;
}

__STATIC_INLINE void xinc_dmas_int_sta_clr(XINC_DMAS_Type * p_reg,uint32_t clr_bits)
{
     p_reg->DMAs_INT_CLR = clr_bits;
}

__STATIC_INLINE void xinc_dmas_lp_ctl(XINC_DMAS_Type * p_reg,uint32_t ctl)
{
     p_reg->DMAs_LP_CTL = ctl;
}

#endif //XINC_DMAS_H__
