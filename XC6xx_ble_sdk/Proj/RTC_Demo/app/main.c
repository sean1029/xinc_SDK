#include    <stdio.h>
#include    <string.h>
#include	"Includes.h"
#include "xinc_gpio.h"
#include "xincx_gpio.h"
#include "xinc_drv_spi.h"
#include "AT24C02.h"
#include "xinc_drv_saadc.h"
#include "xinc_drv_rtc.h"
#include "xinc_drv_timer.h"
#include "xinc_drv_pwm.h"
#include "xinc_drv_wdt.h"
#include "app_button.h"
#include "app_error.h"
#include "app_timer.h"
#include "bsp.h"

extern uint32_t  GulSystickCount;
uint32_t  LastTimeGulSystickCount=0xFF;




void key_init(void)
{
	Init_gpio();

}


const xincx_rtc_t rtc = XINCX_RTC_INSTANCE(0); /**< Declaring an instance of xinc_drv_rtc for RTC0. */

static void rtc_handler(xinc_drv_rtc_int_type_t int_type)
{

    xinc_rtc_time_t rtc_time_val;
    uint8_t static led_state = 0;
    if (int_type == XINCX_RTC_INT_SEC)
    {
        xinc_drv_rtc_date_get(&rtc,&rtc_time_val);
        led_state = bsp_board_led_state_get(bsp_board_pin_to_led_idx(BSP_LED_0));
        printf("led_state:%d\r\n",led_state);
        bsp_board_led_invert(bsp_board_pin_to_led_idx(BSP_LED_0));
            
        printf("SEC:day:%d::week:%d hour:min:sec %d:%d:%d\r\n",rtc_time_val.day,rtc_time_val.week,rtc_time_val.hour,rtc_time_val.min,rtc_time_val.sec);

    }
    else if (int_type == XINCX_RTC_INT_TIME1)
    {
        xinc_drv_rtc_date_get(&rtc,&rtc_time_val);
            
        printf("TIMER1:day:%d::week:%d hour:min:sec %d:%d:%d\r\n",rtc_time_val.day,rtc_time_val.week,rtc_time_val.hour,rtc_time_val.min,rtc_time_val.sec);
    }


}
void rtc_test(void)
{
#if XINCX_CHECK(XINCX_RTC_ENABLED)
    uint32_t err_code;

    //Initialize RTC instance
    xincx_rtc_config_t config = XINCX_RTC_DEFAULT_CONFIG;
    config.freq = 32768;
    config.type = XINC_RTC_TYPE_RTC;
    config.date.week = RTC_WVR_WLR_Monday;
    config.date.day = 4;
    config.date.hour = 18;
    config.date.min = 34;
    config.date.sec = 0;

    err_code = xinc_drv_rtc_init(&rtc, &config, rtc_handler);
    printf("%s,err_code:0x%x ",__func__,err_code);

    xincx_rtc_match_config_t time;
    memset(&time,0,sizeof(time));
    time.times.sec = 20;
    time.times.min = 0;
    time.times.week = XINCX_RTC_WEEK_MATCH_SUNDAY | XINCX_RTC_WEEK_MATCH_MONDAY;
    xinc_drv_rtc_time_set(&rtc,XINCX_RTC_MATCH_TIME_1,time,true);
    //Power on RTC instance
    xincx_rtc_enable(&rtc);
    xinc_drv_rtc_sec_int_enable(&rtc,true);

#endif
	
}


/** @brief Function configuring gpio for pin toggling.
 */
static void leds_config(void)
{
    bsp_board_init(BSP_INIT_LEDS);

}



     
int	main(void)
{



	key_init();
    xincx_gpio_init();   
    leds_config();
    
    SysTick_Config(32000000/100);
    SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;
    rtc_test();

	
    while(1) {

       if(LastTimeGulSystickCount!=GulSystickCount)//10msִ��һ��
	   {		   

           if(LastTimeGulSystickCount % 200 == 0)
           {
               printf("LastTimeGulSystickCount:%d\n",LastTimeGulSystickCount/200);            
           }
		   LastTimeGulSystickCount=GulSystickCount;
			 
	   }		   


    }
}



