#include    <stdio.h>
#include    <string.h>
#include	"Includes.h"

 

#include  "bsp_spi_flash.h"
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
#include "xincx_kbs.h"
#include "xinc_delay.h"
#include "mem_manager.h"
#include "xinc_pwr_mgmt.h"
#include "xinc_log_ctrl.h"
#include "xinc_spi_flash.h"
#include "xinc_drv_timer.h"




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


/**@brief Function for handling bsp events.
 */
void bsp_evt_handler(bsp_event_t evt)
{
    switch (evt)
    {
        // 按键S1 注册的按下事件的回调
        case BSP_EVENT_LED1_ON:
        {
            //点亮 LED 指示灯 D1
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_1));
            printf("EVENT_LED1_ON\r\n");
            
        } break;
        // 按键S1 注册的释放事件的回调
        case BSP_EVENT_LED1_OFF:
        {
            //熄灭 LED 指示灯 D1
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_1));
            printf("EVENT_LED1_OFF\r\n");
        } break;
        // 按键S2 注册的长按事件的回调
        case BSP_EVENT_LED2_ON:
        {
            //点亮 LED 指示灯 D2
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_2));
            printf("EVENT_LED2_ON\r\n");
        } break;
        // 按键S2 注册的释放事件的回调
        case BSP_EVENT_LED2_OFF:
        {
            //熄灭 LED 指示灯 D2
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_2));
            printf("EVENT_LED2_OFF\r\n");
        } break;
        
        case BSP_EVENT_KEY_0:
        {
            printf("BSP_EVENT_KEY_0 \r\n");
        } break;
        
        case BSP_EVENT_KEY_1:
        {
 
            printf("BSP_EVENT_KEY_1\r\n");
        } break;
        
        
           

        default:
            break; // No implementation needed
    }
}


void kbs_mtxkey_bsp_test()
{
    ret_code_t err_code = 0;
    
    bsp_board_init(BSP_INIT_LEDS);
    
    err_code = bsp_event_to_mtxkey_action_assign(2, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_LED1_ON ));
    err_code = bsp_event_to_mtxkey_action_assign(2, BSP_BUTTON_ACTION_RELEASE, (bsp_event_t)(BSP_EVENT_LED1_OFF ));

    err_code = bsp_event_to_mtxkey_action_assign(3, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_LED2_ON ));
    err_code = bsp_event_to_mtxkey_action_assign(3, BSP_BUTTON_ACTION_RELEASE, (bsp_event_t)(BSP_EVENT_LED2_OFF ));

    err_code = bsp_event_to_mtxkey_action_assign(1, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_KEY_1 ));
    err_code = bsp_event_to_mtxkey_action_assign(0, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_KEY_0 ));

    
    err_code = bsp_init(BSP_INIT_MTXKEY,bsp_evt_handler);

  
    printf("xincx_kbs_init err_code:0x%x\n",err_code);
    APP_ERROR_CHECK(err_code);
}



static void mtxkey_event_handler(int16_t mtxkey_idx,uint16_t key_val,uint8_t row_pin,uint8_t col_pin,uint8_t button_action);
static xincx_kbs_mtxkey_cfg_t mtxkeys[] =
{
  [2] = {12,KBS_ROW_BUTTON_2, KBS_COL_BUTTON_1, mtxkey_event_handler},//BUTTON_PULLDOWN
  [1] = {10,KBS_ROW_BUTTON_1, KBS_COL_BUTTON_1, mtxkey_event_handler},
  [3] = {11,KBS_ROW_BUTTON_1, KBS_COL_BUTTON_2, mtxkey_event_handler},

  
  [0] = {13,KBS_ROW_BUTTON_2, KBS_COL_BUTTON_2, mtxkey_event_handler},

};
            
static void mtxkey_event_handler(int16_t mtxkey_idx,uint16_t key_val,uint8_t row_pin,uint8_t col_pin,uint8_t button_action)
{

    switch (button_action)
    {
        case KBS_MTXKEY_PUSH:
        {     
            if(mtxkey_idx == 2)
            {
                bsp_board_led_on(bsp_board_pin_to_led_idx(LED_1));
                bsp_board_led_off(bsp_board_pin_to_led_idx(LED_2));
            }
            
        }break;

        case KBS_MTXKEY_RELEASE:
        {     
            if(mtxkey_idx == 2)
            {
                bsp_board_led_on(bsp_board_pin_to_led_idx(LED_2));
                bsp_board_led_off(bsp_board_pin_to_led_idx(LED_1));
            }
        }break;

        case KBS_MTXKEY_LONG_PUSH:
        {     
            if(mtxkey_idx == 2)
            {
                bsp_board_led_on(bsp_board_pin_to_led_idx(LED_2));
                bsp_board_led_on(bsp_board_pin_to_led_idx(LED_1));
            }
            
        }break;

    }
  
}

void kbs_mtxkey_drv_test()
{
    ret_code_t err_code = 0;
    
    bsp_board_init(BSP_INIT_LEDS);
    
    err_code = xincx_kbs_init(mtxkeys, ARRAY_SIZE(mtxkeys), NULL); 

  
    printf("xincx_kbs_init err_code:0x%x\n",err_code);
    APP_ERROR_CHECK(err_code);
}



int	main(void)
{

    key_init();
    xincx_gpio_init();
    
    xinc_mem_init();   
	
    scheduler_init();
    app_timer_init();
    
    SysTick_Config(32000000/100);


    kbs_mtxkey_bsp_test();
    

    while(1) {
 
       app_sched_execute();
	      
       if(LastTimeGulSystickCount!=GulSystickCount)//10msִ
	   {	
           if(LastTimeGulSystickCount % 200 == 0)
           {
               printf("LastTimeGulSystickCount:%d\n",LastTimeGulSystickCount/200);            
           }           

		   LastTimeGulSystickCount=GulSystickCount;
          
			 
	   }


    }
}




