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

typedef enum
{	
	XINC_AUDIO_ADC_CHANNEL_REFVOL_2_47=0,
	XINC_AUDIO_ADC_CHANNEL_REFVOL_3_3=1,
} xinc_audio_adc_refvol_t;




#define XINC_AUDIO_ADC_GPADC_MAIN_CTL_ALL_CLOSE     AUDIO_ADC_GPADC_MAIN_CTL_ALL_CLOSE


/** @brief Analog-to-digital converter channel mode. */
typedef enum
{
    XINC_AUDIO_ADC_MODE_SINGLE_ENDED = 0,  ///< Single-ended mode. PSELN will be ignored, negative input to ADC shorted to GND.
    XINC_AUDIO_ADC_MODE_DIFFERENTIAL = 1 ///< Differential mode.
} xinc_audio_adc_mode_t;


/** @brief Analog-to-digital converter value limit type. */
typedef enum
{
    XINC_AUDIO_ADC_LIMIT_LOW  = 0, ///< Low limit type.
    XINC_AUDIO_ADC_LIMIT_HIGH = 1  ///< High limit type.
} xinc_audio_adc_limit_t;

/** @brief Type of a single ADC conversion result. */
typedef int32_t xinc_audio_adc_value_t;


/** @brief Analog-to-digital converter channel configuration structure. */
typedef struct
{     
    xinc_audio_adc_mode_t      mode;///< AUDIO_ADC mode. Single-ended or differential.
    uint8_t        	      adc_fifo_len; 
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
    if(config->mode == XINC_AUDIO_ADC_MODE_SINGLE_ENDED)
    {
        
        switch(channel)
        {
            case 0:
            case 1:
            case 2:
            case 3:
            {
                cfg_ch = 21 - channel;
                xinc_gpio_mux_ctl(cfg_ch,0);
                xinc_gpio_fun_sel(cfg_ch,XINC_GPIO_PIN_GPIODx);
                xinc_gpio_inter_sel(cfg_ch,XINC_GPIO_PIN_INPUT_NOINT);
                xinc_gpio_pin_dir_set(cfg_ch,XINC_GPIO_PIN_DIR_INPUT);
                xinc_gpio_pull_sel(cfg_ch,XINC_GPIO_PIN_PULLUP);
            }break;
            
            
            case 4:
            {
                xinc_gpio_mux_ctl(0,0);
                xinc_gpio_fun_sel(0,XINC_GPIO_PIN_GPIODx);
                xinc_gpio_inter_sel(0,XINC_GPIO_PIN_INPUT_NOINT);
                xinc_gpio_pin_dir_set(0,XINC_GPIO_PIN_DIR_INPUT);
                xinc_gpio_pull_sel(0,XINC_GPIO_PIN_PULLUP);
            }break;
            case 5:
            {
                xinc_gpio_mux_ctl(1,0);
                xinc_gpio_fun_sel(1,XINC_GPIO_PIN_GPIODx);
                xinc_gpio_inter_sel(1,XINC_GPIO_PIN_INPUT_NOINT);
                xinc_gpio_pin_dir_set(1,XINC_GPIO_PIN_DIR_INPUT);
                xinc_gpio_pull_sel(1,XINC_GPIO_PIN_PULLUP);

            }break;
            case 6:
            {	
                xinc_gpio_mux_ctl(4,0);
                xinc_gpio_fun_sel(4,XINC_GPIO_PIN_GPIODx);
                xinc_gpio_inter_sel(4,XINC_GPIO_PIN_INPUT_NOINT);
                xinc_gpio_pin_dir_set(4,XINC_GPIO_PIN_DIR_INPUT);
                xinc_gpio_pull_sel(4,XINC_GPIO_PIN_PULLUP);
            }break;
            
            case 7:
            {
                xinc_gpio_mux_ctl(5,0);
                xinc_gpio_fun_sel(5,XINC_GPIO_PIN_GPIODx);
                xinc_gpio_inter_sel(5,XINC_GPIO_PIN_INPUT_NOINT);
                xinc_gpio_pin_dir_set(5,XINC_GPIO_PIN_DIR_INPUT);
                xinc_gpio_pull_sel(5,XINC_GPIO_PIN_PULLUP);
            }break;
            default:break;
        }
    }else
    {
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
            case 2:
            case 3:
            {
                xinc_gpio_mux_ctl(18,0);
                xinc_gpio_fun_sel(18,XINC_GPIO_PIN_GPIODx);
                xinc_gpio_inter_sel(18,XINC_GPIO_PIN_INPUT_NOINT);
                xinc_gpio_pin_dir_set(18,XINC_GPIO_PIN_DIR_INPUT);
                xinc_gpio_pull_sel(18,XINC_GPIO_PIN_PULLUP);

                xinc_gpio_mux_ctl(19,0);
                xinc_gpio_fun_sel(19,XINC_GPIO_PIN_GPIODx);
                xinc_gpio_inter_sel(19,XINC_GPIO_PIN_INPUT_NOINT);
                xinc_gpio_pin_dir_set(19,XINC_GPIO_PIN_DIR_INPUT);
                xinc_gpio_pull_sel(19,XINC_GPIO_PIN_PULLUP);
            }break;
            case 4:
            case 5:
            {
                xinc_gpio_mux_ctl(0,0);
                xinc_gpio_fun_sel(0,XINC_GPIO_PIN_GPIODx);
                xinc_gpio_inter_sel(0,XINC_GPIO_PIN_INPUT_NOINT);
                xinc_gpio_pin_dir_set(0,XINC_GPIO_PIN_DIR_INPUT);
                xinc_gpio_pull_sel(0,XINC_GPIO_PIN_PULLUP);

                xinc_gpio_mux_ctl(1,0);
                xinc_gpio_fun_sel(1,XINC_GPIO_PIN_GPIODx);
                xinc_gpio_inter_sel(1,XINC_GPIO_PIN_INPUT_NOINT);
                xinc_gpio_pin_dir_set(1,XINC_GPIO_PIN_DIR_INPUT);
                xinc_gpio_pull_sel(1,XINC_GPIO_PIN_PULLUP);
            }break;
            case 6:
            case 7:
            {	
                xinc_gpio_mux_ctl(4,0);
                xinc_gpio_fun_sel(4,XINC_GPIO_PIN_GPIODx);
                xinc_gpio_inter_sel(4,XINC_GPIO_PIN_INPUT_NOINT);
                xinc_gpio_pin_dir_set(4,XINC_GPIO_PIN_DIR_INPUT);
                xinc_gpio_pull_sel(4,XINC_GPIO_PIN_PULLUP);

                xinc_gpio_mux_ctl(5,0);
                xinc_gpio_fun_sel(5,XINC_GPIO_PIN_GPIODx);
                xinc_gpio_inter_sel(5,XINC_GPIO_PIN_INPUT_NOINT);
                xinc_gpio_pin_dir_set(5,XINC_GPIO_PIN_DIR_INPUT);
                xinc_gpio_pull_sel(5,XINC_GPIO_PIN_PULLUP);
            }break;
            default:break;
    
        }
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
