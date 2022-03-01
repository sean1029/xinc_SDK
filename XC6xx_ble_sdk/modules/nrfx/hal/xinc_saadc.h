/**
 * Copyright (c) 2015 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef XINC_SAADC_H_
#define XINC_SAADC_H_

#include <nrfx.h>
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
#define NRF_SAADC_CHANNEL_COUNT 8


/** @brief Resolution of the analog-to-digital converter. */
typedef enum
{	
	NRF_SAADC_FREQ_8M=1,
	NRF_SAADC_FREQ_4M=2,
	NRF_SAADC_FREQ_2M=4,
	NRF_SAADC_FREQ_1M=8,
	NRF_SAADC_FREQ_500K=16,
} xinc_saadc_freq_t;

typedef enum
{	
	NRF_SAADC_CHANNEL_REFVOL_2_47=0,
	NRF_SAADC_CHANNEL_REFVOL_3_3=1,
} xinc_saadc_refvol_t;


/** @brief Input selection for the analog-to-digital converter. */
typedef enum
{
    NRF_SAADC_INPUT_DISABLED = SAADC_CH_PSELP_PSELP_NC,           ///< Not connected.
    NRF_SAADC_INPUT_AIN0     = SAADC_CH_PSELP_PSELP_AnalogInput0, ///< Analog input 0 (AIN0).
    NRF_SAADC_INPUT_AIN1     = SAADC_CH_PSELP_PSELP_AnalogInput1, ///< Analog input 1 (AIN1).
    NRF_SAADC_INPUT_AIN2     = SAADC_CH_PSELP_PSELP_AnalogInput2, ///< Analog input 2 (AIN2).
    NRF_SAADC_INPUT_AIN3     = SAADC_CH_PSELP_PSELP_AnalogInput3, ///< Analog input 3 (AIN3).
    NRF_SAADC_INPUT_AIN4     = SAADC_CH_PSELP_PSELP_AnalogInput4, ///< Analog input 4 (AIN4).
    NRF_SAADC_INPUT_AIN5     = SAADC_CH_PSELP_PSELP_AnalogInput5, ///< Analog input 5 (AIN5).
    NRF_SAADC_INPUT_AIN6     = SAADC_CH_PSELP_PSELP_AnalogInput6, ///< Analog input 6 (AIN6).
    NRF_SAADC_INPUT_AIN7     = SAADC_CH_PSELP_PSELP_AnalogInput7, ///< Analog input 7 (AIN7).
    NRF_SAADC_INPUT_VDD      = SAADC_CH_PSELP_PSELP_VDD,          ///< VDD as input.
#if defined(SAADC_CH_PSELP_PSELP_VDDHDIV5) || defined(__NRFX_DOXYGEN__)
    NRF_SAADC_INPUT_VDDHDIV5 = SAADC_CH_PSELP_PSELP_VDDHDIV5      ///< VDDH/5 as input.
#endif
} xinc_saadc_input_t;





/** @brief Reference selection for the analog-to-digital converter. */
typedef enum
{
    NRF_SAADC_REFERENCE_INTERNAL = SAADC_CH_CONFIG_REFSEL_Internal, ///< Internal reference (0.6 V).
    NRF_SAADC_REFERENCE_VDD4     = SAADC_CH_CONFIG_REFSEL_VDD1_4    ///< VDD/4 as reference.
} xinc_saadc_reference_t;





/** @brief Analog-to-digital converter value limit type. */
typedef enum
{
    NRF_SAADC_LIMIT_LOW  = 0, ///< Low limit type.
    NRF_SAADC_LIMIT_HIGH = 1  ///< High limit type.
} xinc_saadc_limit_t;

/** @brief Type of a single ADC conversion result. */
typedef int16_t xinc_saadc_value_t;


/** @brief Analog-to-digital converter channel configuration structure. */
typedef struct
{
    xinc_saadc_refvol_t    refvol;  ///< Reference control value.
    xinc_saadc_freq_t      freq;     
    uint32_t        waite_time;       ///< SAADC mode. Single-ended or differential.
		uint8_t        	adc_fifo_len; 
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
	uint32_t reg_val;
		
	
	XINC_SAADC->FIFO_CTL = 0x10;
	XINC_SAADC->FIFO_CTL = 0x00;
	
	XINC_SAADC->FIFO_CTL = config->adc_fifo_len;
	reg_val = XINC_SAADC->RF_CTL;
	
	if(config->refvol ==  NRF_SAADC_CHANNEL_REFVOL_2_47)
	{
		reg_val = (config->freq << 8) | 0x10;
	}
	else 
	{
		reg_val = (config->freq << 8) | 0x12;
	}
			
  XINC_SAADC->RF_CTL = reg_val;
	
	XINC_SAADC->CHAN_CTL = channel;
	
	
	XINC_SAADC->TIMER0 = config->waite_time;
	
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
	

}


__STATIC_INLINE void xinc_saadc_enable(void)
{
	uint32_t val;
	
	
	val = XINC_SAADC->MAIN_CTL;
	val |= (SAADC_ENABLE_ENABLE_Enabled << SAADC_ENABLE_ENABLE_Pos);
  XINC_SAADC->MAIN_CTL = val;
	printf("MAIN_CTL=%x\n",XINC_SAADC->MAIN_CTL);
}


__STATIC_INLINE void xinc_saadc_fifo_clear(void)
{

  XINC_SAADC->FIFO_CTL |= 0x01 << 4;
	XINC_SAADC->FIFO_CTL &= ~(0x01 << 4);
}


#endif // SUPPRESS_INLINE_IMPLEMENTATION

/** @} */

#ifdef __cplusplus
}
#endif

#endif // NRF_SAADC_H_
