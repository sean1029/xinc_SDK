#ifndef 	__PLATFORM_H
#define 	__PLATFORM_H

/*
*********************************************************************************************************
*            HARDWARE PLATFORM
*********************************************************************************************************
*/
#include "xinc.h"
#if defined(XC60XX_M0)
  #include    "xinc_m0.h"
#elif defined (XC66XX_M4)
  #include    "xinc_m4.h"
#else
    #error "Device must be defined. See xinc.h."
#endif

#include    "bsp_register_macro.h"
#include    "bsp_com_spi.h"
#include    "bsp_uart.h"
#include    "bsp_pwm.h"
#include    "bsp_gpio.h"
//#include    "bsp_spi_master.h"
#include	"stdio.h"
#include    "bsp_wdog.h"
#include    "test_config.h"
#include "bsp_clk.h"

/*
*********************************************************************************************************
*            PLATFORM DEFINES
*********************************************************************************************************
*/
#define     CORE_CLK            32000000ul      /* Set Processor frequency */
#define     TICKS_PER_SEC       100ul           /* Set the number of ticks in one second  */ 

/*
*********************************************************************************************************
*         	GLOBAL FUNCTION
*********************************************************************************************************
*/
#define     AT_USE_UART             1
#define     AT_BUFF_LEN			    100

#define PIN_32_16



#define     __ASSERT(a)    while(!(a))


void delay_us(uint32_t  nus);
void delay_ms(uint32_t nms);


#endif

