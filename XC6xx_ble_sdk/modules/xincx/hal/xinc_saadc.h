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
#include "bsp_gpio.h"
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
} nrf_saadc_mode_t;


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
    nrf_saadc_mode_t      mode;///< SAADC mode. Single-ended or differential.
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
    if(config->mode == XINC_SAADC_MODE_SINGLE_ENDED)
    {
        switch(channel)
        {
            case 0:
            {
                gpio_mux_ctl(21,0);gpio_fun_sel(21,0);gpio_fun_inter(21,0);gpio_direction_input(21,3);
            }break;
            
            case 1:
            {
                gpio_mux_ctl(20,0);gpio_fun_sel(20,0);gpio_fun_inter(20,0);gpio_direction_input(20,3);
            }break;
            case 2:
            {
                gpio_mux_ctl(19,0);gpio_fun_sel(19,0);gpio_fun_inter(19,0);gpio_direction_input(19,3);
            }break;
            case 3:
            {
                gpio_mux_ctl(18,0);gpio_fun_sel(18,0);gpio_fun_inter(18,0);gpio_direction_input(18,3);
            }break;
            case 4:
            {
                gpio_mux_ctl(0,0);gpio_fun_sel(0,0);gpio_fun_inter(0,0);gpio_direction_input(0,3);
            }break;
            case 5:
            {
                gpio_mux_ctl(1,0);gpio_fun_sel(1,0);gpio_fun_inter(1,0);gpio_direction_input(1,3);
            }break;
            case 6:
            {	
                gpio_mux_ctl(4,0);gpio_fun_sel(4,0);gpio_fun_inter(4,0);gpio_direction_input(4,3);
            }break;
            
            case 7:
            {
                gpio_mux_ctl(5,0);gpio_fun_sel(5,0);gpio_fun_inter(5,0);gpio_direction_input(5,3);
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
                gpio_mux_ctl(21,0);gpio_fun_sel(21,0);gpio_fun_inter(21,0);gpio_direction_input(21,3);        
                gpio_mux_ctl(20,0);gpio_fun_sel(20,0);gpio_fun_inter(20,0);gpio_direction_input(20,3);
            }break;
            case 2:
            case 3:
            {
                gpio_mux_ctl(19,0);gpio_fun_sel(19,0);gpio_fun_inter(19,0);gpio_direction_input(19,3);
                gpio_mux_ctl(18,0);gpio_fun_sel(18,0);gpio_fun_inter(18,0);gpio_direction_input(18,3);
            }break;
            case 4:
            case 5:
            {
                gpio_mux_ctl(0,0);gpio_fun_sel(0,0);gpio_fun_inter(0,0);gpio_direction_input(0,3);
                gpio_mux_ctl(1,0);gpio_fun_sel(1,0);gpio_fun_inter(1,0);gpio_direction_input(1,3);
            }break;
            case 6:
            case 7:
            {	
                gpio_mux_ctl(4,0);gpio_fun_sel(4,0);gpio_fun_inter(4,0);gpio_direction_input(4,3);
                gpio_mux_ctl(5,0);gpio_fun_sel(5,0);gpio_fun_inter(5,0);gpio_direction_input(5,3);
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
