/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINC_SAADC_H_
#define XINC_SAADC_H_

#include <xincx.h>
#include "xinc_gpio.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xinc_saadc_hal SAADC HAL
 * @{
 * @ingroup xinc_saadc
 * @brief   Hardware access layer for managing the SAADC peripheral.
 */

/** @brief Number of available SAADC channels. */
#define XINC_SAADC_CHANNEL_COUNT 9

#define XINCX_SAADC_FIFOLEN          SAADC_GPADC_FIFO_FIFO_DOUT_LEN


/** @brief Resolution of the analog-to-digital converter. */
typedef enum
{	
	XINC_SAADC_FREQ_8M=1,
	XINC_SAADC_FREQ_4M=2,
	XINC_SAADC_FREQ_2M=4,
	XINC_SAADC_FREQ_1M=8,
	XINC_SAADC_FREQ_500K=16,
} xinc_saadc_freq_t;

typedef enum
{	
	XINC_SAADC_CHANNEL_REFVOL_2_47=0,
	XINC_SAADC_CHANNEL_REFVOL_3_3=1,
} xinc_saadc_refvol_t;




#define XINC_SAADC_GPADC_MAIN_CTL_ALL_CLOSE     SAADC_GPADC_MAIN_CTL_ALL_CLOSE


/** @brief Analog-to-digital converter channel mode. */
typedef enum
{
    XINC_SAADC_MODE_SINGLE_ENDED = SAADC_CH_CONFIG_MODE_SE,  ///< Single-ended mode. PSELN will be ignored, negative input to ADC shorted to GND.
    XINC_SAADC_MODE_DIFFERENTIAL = SAADC_CH_CONFIG_MODE_Diff ///< Differential mode.
} xinc_saadc_mode_t;


/** @brief Analog-to-digital converter value limit type. */
typedef enum
{
    XINC_SAADC_LIMIT_LOW  = 0, ///< Low limit type.
    XINC_SAADC_LIMIT_HIGH = 1  ///< High limit type.
} xinc_saadc_limit_t;

/** @brief Type of a single ADC conversion result. */
typedef int16_t xinc_saadc_value_t;


/** @brief Analog-to-digital converter channel configuration structure. */
typedef struct
{     
    xinc_saadc_mode_t      mode;///< SAADC mode. Single-ended or differential.
    uint8_t        	      adc_fifo_len; 
} xinc_saadc_channel_config_t;



/**
 * @brief Function for initializing the SAADC channel.
 *
 * @param[in] channel Channel number.
 * @param[in] config  Pointer to the channel configuration structure.
 */
__STATIC_INLINE void xinc_saadc_channel_init(uint8_t                                  channel,
                                            xinc_saadc_channel_config_t const * const config);




#ifndef SUPPRESS_INLINE_IMPLEMENTATION

 __STATIC_INLINE void xinc_saadc_channel_init(uint8_t                                  channel,
                                            xinc_saadc_channel_config_t const * const config)
{
    uint8_t cfg_ch;
    if(config->mode == XINC_SAADC_MODE_SINGLE_ENDED)
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


__STATIC_INLINE void xinc_saadc_enable(XINC_SAADC_Type * p_reg)
{

}

__STATIC_INLINE void xinc_saadc_fifo_clear(XINC_SAADC_Type * p_reg)
{

    p_reg->FIFO_CTL |= SAADC_GPADC_FIFO_CTL_FIFO_FLUSH_Msk;
	p_reg->FIFO_CTL &= ~(SAADC_GPADC_FIFO_CTL_FIFO_FLUSH_Msk);

}


#endif // SUPPRESS_INLINE_IMPLEMENTATION

/** @} */

#ifdef __cplusplus
}
#endif

#endif // XINC_SAADC_H_
