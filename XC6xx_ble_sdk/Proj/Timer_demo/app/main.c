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
#include "app_scheduler.h"
#include "xinc_delay.h"


extern uint32_t  GulSystickCount;
uint32_t  LastTimeGulSystickCount=0xFF;




void key_init(void)
{
	Init_gpio();

}

#define SCHED_MAX_EVENT_DATA_SIZE           APP_TIMER_SCHED_EVENT_DATA_SIZE             //!< Maximum size of the scheduler event data.
#define SCHED_QUEUE_SIZE                    10                                          //!< Size of the scheduler queue.

static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}


const xinc_drv_timer_t TIMER_LED = XINC_DRV_TIMER_INSTANCE(2);

void timer_led_event_handler(xinc_timer_int_event_t event_type,uint8_t channel, void* p_context)
{

   printf("timer_led_event_handler event_type:[%d],channel:%d\n",event_type,channel);
 
    switch (event_type)
    {
        case XINC_TIMER_EVENT_TIMEOUT:
        {				
           bsp_board_led_invert(bsp_board_pin_to_led_idx(BSP_LED_0));
        }break;

		default:
            //Do nothing.
            break;
    }
}

void timer_test()
{
    ret_code_t err_code;


    bsp_board_init(BSP_INIT_LEDS);
    
    uint32_t time_ms = 500; //Time(in miliseconds) between consecutive compare events.
    uint32_t time_ticks;

    //Configure TIMER_LED for generating simple light effect - leds on board will invert his state one after the other.
    xinc_drv_timer_config_t timer_cfg = XINC_DRV_TIMER_DEFAULT_CONFIG;
    
    
    err_code = xinc_drv_timer_init(&TIMER_LED, &timer_cfg, timer_led_event_handler);
    APP_ERROR_CHECK(err_code);

 //   time_ticks = xinc_drv_timer_us_to_ticks(&TIMER_LED, time_ms);
 //   printf("time_ticks = [%d]\n",time_ticks);
    time_ticks = xinc_drv_timer_ms_to_ticks(&TIMER_LED, time_ms);
    printf("time_ticks = [%d]\n",time_ticks);


    xinc_drv_timer_compare(&TIMER_LED, time_ticks,XINC_TIMER_MODE_USER_COUNTER ,true);//  // XINC_TIMER_MODE_USER_COUNTER //XINC_TIMER_MODE_AUTO_TIMER

    
}


int	main(void)
{

    key_init();
    xincx_gpio_init();
    scheduler_init();
    app_timer_init();

    SysTick_Config(32000000/100);
    SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;
    
    timer_test();

    while(1) {

       app_sched_execute();
			
       if(LastTimeGulSystickCount!=GulSystickCount)//10msִ��һ��
	   {		   

           if(LastTimeGulSystickCount % 500 == 0)
           {
               printf("LastTimeGulSystickCount:%d\n",LastTimeGulSystickCount/500);            
           }           

		   LastTimeGulSystickCount=GulSystickCount;
			 
	   }		   


    }
}



