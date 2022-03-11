
#ifndef APP_TEST_H__
#define APP_TEST_H__
#include    <stdio.h>
#include    <string.h>

#include	"Includes.h"

#include "bsp_gpio.h"
#include "sbc.h"
#include "voice_ringbuf.h"
#include "bsp_timer.h"
#include "bsp_pwm.h"
#include "bsp_spi_flash.h"
#include "nrf_gpio.h"
#include "bsp_uart.h"

#include "nrfx_gpiote.h"
#include "xinc_drv_spi.h"
#include "AT24C02.h"
#include "xinc_drv_saadc.h"
#include "xinc_drv_rtc.h"
#include "bsp_clk.h"
#include "xinc_drv_timer.h"
#include "xinc_drv_pwm.h"
#include "xinc_drv_wdt.h"

#include "app_scheduler.h"
#include "app_button.h"
#include "app_error.h"
#include "app_timer.h"
#include "bsp.h"

#define CLI_TEST_EN 0

#define LOG_TEST_EN 0

#define RTC_TEST_EN 0

#define DRV_TIMER_TEST_EN 0


#define DRV_WDT_TEST_EN 0
#define DRV_PWM_TEST_EN 1
#define DRV_SAADC_TEST_EN 0
#define DRV_UART_TEST_EN 1

#define BSP_BUTTON_TEST_EN 0
#define APP_BUTTON_TEST_EN 0

void cli_test(void);
void cli_processt(void);

void log_test(void);
void log_flush(void);

void rtc_test(void);

void drv_timer_test(void);
void drv_wdg_test(void);
void drv_adc_test(void);
void drv_pwm_test(void);
void drv_uart_test(void);

void bsp_button_led_test(void);

void app_button_test(void);





#endif //
