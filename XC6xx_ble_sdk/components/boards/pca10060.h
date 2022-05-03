/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#ifndef PCA10060_H
#define PCA10060_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xinc_gpio.h"

// LEDs definitions for PCA10060
#define LEDS_NUMBER    3

#define LED_START      4
#define LED_1          29
#define LED_2          30
#define LED_3          5
#define LED_4          31
#define LED_STOP       31

#define LEDS_ACTIVE_STATE 0

#define LEDS_INV_MASK  LEDS_MASK

#define LEDS_LIST { LED_1, LED_2, LED_3}//, LED_4 }
//#define LEDS_LIST { LED_1, LED_2 }

#define BSP_LED_0      LED_1
#define BSP_LED_1      LED_2
#define BSP_LED_2      LED_3
#define BSP_LED_3      LED_4

#define BUTTONS_NUMBER 3

#define BUTTON_START   8
#define BUTTON_1       8
#define BUTTON_2       9
#define BUTTON_3       10
#define BUTTON_4       28
#define BUTTON_STOP    28
#define BUTTON_PULL    XINC_GPIO_PIN_PULLUP
#define BUTTON_PULLDOWN XINC_GPIO_PIN_PULLDOWN

#define BUTTONS_ACTIVE_STATE 0

#define BUTTONS_LIST { /*BUTTON_1,*/ BUTTON_2, BUTTON_3, BUTTON_4 }

#define BSP_BUTTON_0   BUTTON_1
#define BSP_BUTTON_1   BUTTON_2
#define BSP_BUTTON_2   BUTTON_3
#define BSP_BUTTON_3   BUTTON_4



#define KBS_ROW_BUTTONS_NUMBER 4
#define KBS_ROW_BUTTON_1       2
#define KBS_ROW_BUTTON_2       3
#define KBS_ROW_BUTTON_3       6
#define KBS_ROW_BUTTON_4       7
#define KBS_ROW_BUTTONS_LIST { KBS_ROW_BUTTON_1, KBS_ROW_BUTTON_2,KBS_ROW_BUTTON_3,KBS_ROW_BUTTON_3}

#define BSP_KBS_ROW_BUTTON_1    KBS_ROW_BUTTON_1       
#define BSP_KBS_ROW_BUTTON_2    KBS_ROW_BUTTON_2  
#define BSP_KBS_ROW_BUTTON_3    KBS_ROW_BUTTON_3       
#define BSP_KBS_ROW_BUTTON_4    KBS_ROW_BUTTON_4  

#define KBS_COL_BUTTONS_NUMBER 4
#define KBS_COL_BUTTON_1       22
#define KBS_COL_BUTTON_2       23
#define KBS_COL_BUTTON_3       24
#define KBS_COL_BUTTON_4       25
#define KBS_COL_BUTTONS_LIST { KBS_COL_BUTTON_1, KBS_COL_BUTTON_2,KBS_COL_BUTTON_3, KBS_COL_BUTTON_4}

#define BSP_KBS_COL_BUTTON_1    KBS_COL_BUTTON_1       
#define BSP_KBS_COL_BUTTON_2    KBS_COL_BUTTON_2  
#define BSP_KBS_COL_BUTTON_3    KBS_COL_BUTTON_3       
#define BSP_KBS_COL_BUTTON_4    KBS_COL_BUTTON_4 


#define APP_UART_TX_PIN_NUMBER  2
#define APP_UART_RX_PIN_NUMBER  3
#define CTS_PIN_NUMBER 7
#define RTS_PIN_NUMBER 5
#define HWFC           false
    
#define APP_UART2_TX_PIN_NUMBER  32
#define APP_UART2_RX_PIN_NUMBER  31
	

#define CLI_RX_PIN_NUMBER  3
#define CLI_TX_PIN_NUMBER  2

#define CLI2_RX_PIN_NUMBER  31
#define CLI2_TX_PIN_NUMBER  32

#define SPIM0_SCK_PIN   29  // SPI clock GPIO pin number.
#define SPIM0_MOSI_PIN  25  // SPI Master Out Slave In GPIO pin number.
#define SPIM0_MISO_PIN  28  // SPI Master In Slave Out GPIO pin number.
#define SPIM0_SS_PIN    12  // SPI Slave Select GPIO pin number.

#define SPIM1_SCK_PIN   2   // SPI clock GPIO pin number.
#define SPIM1_MOSI_PIN  7   // SPI Master Out Slave In GPIO pin number.
#define SPIM1_MISO_PIN  6   // SPI Master In Slave Out GPIO pin number.
#define SPIM1_SS_PIN    3   // SPI Slave Select GPIO pin number.



#ifdef __cplusplus
}
#endif

#endif // PCA10060_H
