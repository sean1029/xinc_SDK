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
#define XINC_AUDIO_ADC_CHANNEL_COUNT 9

#define XINCX_AUDIO_ADC_FIFOLEN          AUDIO_ADC_GPADC_FIFO_FIFO_DOUT_LEN


/** @brief Resolution of the analog-to-digital converter. */
typedef enum
{	
	XINC_AUDIO_ADC_FREQ_8M=1,
	XINC_AUDIO_ADC_FREQ_4M=2,
	XINC_AUDIO_ADC_FREQ_2M=4,
	XINC_AUDIO_ADC_FREQ_1M=8,
	XINC_AUDIO_ADC_FREQ_500K=16,
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



/**
 * @brief Function for initializing the AUDIO_ADC channel.
 *
 * @param[in] channel Channel number.
 * @param[in] config  Pointer to the channel configuration structure.
 */
__STATIC_INLINE void xinc_audio_adc_channel_init(uint8_t                                  channel,
                                            xinc_audio_adc_channel_config_t const * const config);




#ifndef SUPPRESS_INLINE_IMPLEMENTATION

 __STATIC_INLINE void xinc_audio_adc_channel_init(uint8_t                                  channel,
                                            xinc_audio_adc_channel_config_t const * const config)
{
    uint8_t cfg_ch;
    
        
        
           
    
        switch(channel)
        {
            case 0:
            case 1:
            {
                xinc_gpio_mux_ctl(20,0);
                xinc_gpio_fun_sel(20,XINC_GPIO_PIN_GPIODx);
                xinc_gpio_inter_sel(20,XINC_GPIO_PIN_INPUT_NOINT);
                xinc_gpio_pin_dir_set(20,XINC_GPIO_PIN_DIR_INPUT);
                xinc_gpio_pull_sel(20,XINC_GPIO_PIN_PULLUP);

                xinc_gpio_mux_ctl(21,0);
                xinc_gpio_fun_sel(21,XINC_GPIO_PIN_GPIODx);
                xinc_gpio_inter_sel(21,XINC_GPIO_PIN_INPUT_NOINT);
                xinc_gpio_pin_dir_set(21,XINC_GPIO_PIN_DIR_INPUT);
                xinc_gpio_pull_sel(21,XINC_GPIO_PIN_PULLUP);
            }break;


            default:break;
    
        }
    
}


__STATIC_INLINE void xinc_audio_adc_enable(XINC_CDC_Type * p_reg)
{

}

__STATIC_INLINE void xinc_audio_adc_fifo_clear(XINC_CDC_Type * p_reg)
{


}


#endif // SUPPRESS_INLINE_IMPLEMENTATION

/** @} */

#ifdef __cplusplus
}
#endif

#endif // XINC_AUDIO_ADC_H_
