#ifndef _FIX_BSP_GP_ADC_H_
#define _FIX_BSP_GP_ADC_H_
#include    "bsp_register_macro.h"

#define FIFO_DEEP   16

#define GADC_FREQ_500K  16
#define GADC_FREQ_1M    8
#define GADC_FREQ_2M    4
#define GADC_FREQ_4M    2
#define GADC_FREQ_8M    1

#define GADC_REF_2_47V   1
#define GADC_REF_AVDD    2

typedef struct  gadc_cache_struct{
	    unsigned int value_2 : 10;
			unsigned int chanel_2 : 4;
			unsigned int  : 2; 
			unsigned int value_1 : 10;
			unsigned int chanel_1 : 4;
			unsigned int  : 2;
}gadc_cache_t; 


int fix_init_adc(uint8_t freq,uint8_t gadc_ref);
void fix_adc_gpio_config(uint16_t channels);

void test_xc_gpadc(void);

#endif