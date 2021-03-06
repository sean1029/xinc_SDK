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
    DMAS_CH_SEND_UART0 = 0UL,
    DMAS_CH_SEND_UART1 = 1UL,
    DMAS_CH_SEND_SSI0 = 2UL,
    DMAS_CH_SEND_SSI1 = 3UL,
    #if defined (XC66XX_M4) 
    DMAS_CH_SEND_SSI2 = 4UL,
    DMAS_CH_SEND_I2S = 5UL,
    #endif
    DMAS_CH_RCV_UART0 = 8UL,
    DMAS_CH_RCV_UART1 = 9UL,
    DMAS_CH_RCV_SSI0 = 10UL,
    DMAS_CH_RCV_SSI1 = 11UL,
    DMAS_CH_RCV_GPADC = 12UL,
    #if defined (XC66XX_M4) 
    DMAS_CH_RCV_AUDIO_ADC = 13UL,
    DMAS_CH_RCV_SSI2 = 14UL,
    DMAS_CH_RCV_I2S = 15UL,
    #endif
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

__STATIC_INLINE void xinc_dmas_intv_unit_set(XINC_DMAS_Type * p_reg,uint32_t intv_unit)
{
     p_reg->DMAs_INTV_UNIT = intv_unit;
}

__STATIC_INLINE XINC_DMAS_Type * xinc_dmas_reg_decode(void)
{

    return XINC_DMAS0;
}

#endif //XINC_DMAS_H__
