/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINC_AUDIO_ADC_H_
#define XINC_AUDIO_ADC_H_

#include <xincx.h>
#include "xinc_gpio.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xinc_audio_adc_hal AUDIO_ADC HAL
 * @{
 * @ingroup xinc_audio_adc
 * @brief   Hardware access layer for managing the AUDIO_ADC peripheral.
 */

/** @brief Number of available AUDIO_ADC channels. */
#define XINC_AUDIO_ADC_CHANNEL_COUNT 1

#define XINCX_AUDIO_ADC_FIFOLEN          AUDIO_ADC_GPADC_FIFO_FIFO_DOUT_LEN


/** @brief Resolution of the analog-to-digital converter. */
typedef enum
{	
	XINC_AUDIO_ADC_FREQ_16K=0,
    XINC_AUDIO_ADC_FREQ_32K=1,
} xinc_audio_adc_freq_t;





#define XINC_AUDIO_ADC_GPADC_MAIN_CTL_ALL_CLOSE     AUDIO_ADC_GPADC_MAIN_CTL_ALL_CLOSE



/** @brief Analog-to-digital converter value limit type. */
typedef enum
{
    XINC_AUDIO_ADC_LIMIT_LOW  = 0, ///< Low limit type.
    XINC_AUDIO_ADC_LIMIT_HIGH = 1  ///< High limit type.
} xinc_audio_adc_limit_t;

/** @brief Type of a single ADC conversion result. */
typedef int32_t xinc_audio_adc_value_t;


typedef	union		cdc_ana_reg0 {
        uint8_t     REG00;
        struct      bit0 {
            uint8_t     adc_d_vol  :6;     
            uint8_t     hpf_en  :1;
            uint8_t     hpf_bypass  :1;
        }bits;
}cdc_ana_reg00_t;


/** @brief Analog-to-digital converter channel configuration structure. */
typedef struct
{     
    uint8_t        	      adc_fifo_len; 
    cdc_ana_reg00_t       reg0;
    uint8_t        	      reg1; 
} xinc_audio_adc_channel_config_t;



#ifndef SUPPRESS_INLINE_IMPLEMENTATION



__STATIC_INLINE void xinc_audio_adc_enable(XINC_CDC_Type * p_reg)
{
    uint32_t reg_val = p_reg->ANA_REG0;
    reg_val |= CDC_CDC_ANA_REG0_REG00_HPF_EN_Msk;
    reg_val &= ~CDC_CDC_ANA_REG0_REG00_HPF_BYPASS_Msk;
    p_reg->ANA_REG0 = reg_val;
}

__STATIC_INLINE void xinc_audio_adc_disable(XINC_CDC_Type * p_reg)
{
    uint32_t reg_val = p_reg->ANA_REG0;
    reg_val &= ~CDC_CDC_ANA_REG0_REG00_HPF_EN_Msk;
    reg_val |= CDC_CDC_ANA_REG0_REG00_HPF_BYPASS_Msk;
    p_reg->ANA_REG0 = reg_val;
}


__STATIC_INLINE void xinc_audio_adc_fifo_clear(XINC_CDC_Type * p_reg)
{
    p_reg->FIFO_STATUS |= CDC_CDC_RXFIFO_STATUS_RXFIFO_FLUSH_Msk;
	p_reg->FIFO_STATUS &= ~(CDC_CDC_RXFIFO_STATUS_RXFIFO_FLUSH_Msk);
}


#endif // SUPPRESS_INLINE_IMPLEMENTATION

/** @} */

#ifdef __cplusplus
}
#endif

#endif // XINC_AUDIO_ADC_H_
