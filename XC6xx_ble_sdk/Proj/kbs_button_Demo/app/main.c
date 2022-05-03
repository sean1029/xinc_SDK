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
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_1));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_2));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_3));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_4));
        } break;
        
        case BSP_EVENT_KEY_1:
        {

            printf("BSP_EVENT_KEY_1\r\n");         
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_1));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_2));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_3));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_4));
        } break;
        case BSP_EVENT_KEY_2:
        {
            printf("BSP_EVENT_KEY_2 \r\n");
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_1));
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_2));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_3));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_4));
        } break;
        
        case BSP_EVENT_KEY_3:
        {
 
            printf("BSP_EVENT_KEY_3\r\n");
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_1));
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_2));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_3));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_4));
        } break;
        
        #if defined (BOARD_PCA10060)
           case BSP_EVENT_KEY_4:
        {
            printf("BSP_EVENT_KEY_4 \r\n");
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_1));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_2));
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_3));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_4));
        } break;
        
        case BSP_EVENT_KEY_5:
        {
 
            printf("BSP_EVENT_KEY_5\r\n");
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_1));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_2));
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_3));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_4));
        } break;
        case BSP_EVENT_KEY_6:
        {
            printf("BSP_EVENT_KEY_6 \r\n");
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_1));
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_2));
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_3));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_4));
        } break;
        
        case BSP_EVENT_KEY_7:
        {
 
            printf("BSP_EVENT_KEY_7\r\n");
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_1));
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_2));
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_3));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_4));
        } break;
        
        case BSP_EVENT_KEY_8:
        {
            printf("BSP_EVENT_KEY_8 \r\n");
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_1));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_2));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_3));
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_4));
        } break;
        
        case BSP_EVENT_KEY_9:
        {
 
            printf("BSP_EVENT_KEY_9\r\n");
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_1));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_2));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_3));
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_4));
        } break;
        
        case BSP_EVENT_KEY_10:
        {
            printf("BSP_EVENT_KEY_10 \r\n");
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_1));
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_2));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_3));
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_4));
        } break;
        
        case BSP_EVENT_KEY_11:
        {
 
            printf("BSP_EVENT_KEY_11\r\n");
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_1));
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_2));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_3));
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_4));
        } break;
        
         case BSP_EVENT_KEY_12:
        {
            printf("BSP_EVENT_KEY_12 \r\n");
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_1));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_2));
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_3));
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_4));
        } break;
        
        case BSP_EVENT_KEY_13:
        {
 
            printf("BSP_EVENT_KEY_13\r\n");
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_1));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_2));
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_3));
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_4));
        } break;
        
        case BSP_EVENT_KEY_14:
        {
            printf("BSP_EVENT_KEY_14 \r\n");
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_1));
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_2));
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_3));
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_4));
        } break;
        
        case BSP_EVENT_KEY_15:
        {
 
            printf("BSP_EVENT_KEY_15\r\n");
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_1));
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_2));
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_3));
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_4));
        } break;
        case BSP_EVENT_KEY_16:
        {
            printf("BSP_EVENT_KEY_16 \r\n");
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_1));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_2));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_3));
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_4));
        } break;
        
   
    
        #endif
        
           

        default:
            break; // No implementation needed
    }
}
/*********mtxkey_idx 2 key_num**********************
mtxkey_idx  key_num
    0           1
    1           5
    2           2
    3           6
    4           9
    5           16
    6           10
    7           15
    8           3
    9           7
    10          11
    11          14
    12          4
    13          8
    14          12
    15          13
****************************/

void kbs_mtxkey_bsp_test()
{
    ret_code_t err_code = 0;
    
    bsp_board_init(BSP_INIT_LEDS);
    
    err_code = bsp_event_to_mtxkey_action_assign(0, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_KEY_1 ));
    err_code = bsp_event_to_mtxkey_action_assign(1, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_KEY_5 ));
    err_code = bsp_event_to_mtxkey_action_assign(2, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_KEY_2 ));
    err_code = bsp_event_to_mtxkey_action_assign(3, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_KEY_6 ));
    
  #if defined (BOARD_PCA10060)
    
    err_code = bsp_event_to_mtxkey_action_assign(4, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_KEY_9 ));
    err_code = bsp_event_to_mtxkey_action_assign(5, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_KEY_16 ));
    err_code = bsp_event_to_mtxkey_action_assign(6, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_KEY_10 ));
    err_code = bsp_event_to_mtxkey_action_assign(7, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_KEY_15 ));
    
    
    err_code = bsp_event_to_mtxkey_action_assign(8, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_KEY_3 ));
    err_code = bsp_event_to_mtxkey_action_assign(9, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_KEY_7 ));
    err_code = bsp_event_to_mtxkey_action_assign(10, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_KEY_11 ));
    err_code = bsp_event_to_mtxkey_action_assign(11, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_KEY_14 ));
    
    err_code = bsp_event_to_mtxkey_action_assign(12, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_KEY_4 ));
    err_code = bsp_event_to_mtxkey_action_assign(13, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_KEY_8 ));
    err_code = bsp_event_to_mtxkey_action_assign(14, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_KEY_12 ));
    err_code = bsp_event_to_mtxkey_action_assign(15, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_KEY_13 ));
    
    err_code = bsp_event_to_mtxkey_action_assign(15, BSP_BUTTON_ACTION_LONG_PUSH, (bsp_event_t)(BSP_EVENT_KEY_0 ));
    
  #endif
    
    err_code = bsp_init(BSP_INIT_MTXKEY,bsp_evt_handler);

  
    printf("xincx_kbs_init err_code:0x%x\n",err_code);
    APP_ERROR_CHECK(err_code);
}



static void mtxkey_event_handler(int16_t mtxkey_idx,uint16_t key_val,uint8_t row_pin,uint8_t col_pin,uint8_t button_action);
/*********mtxkey_idx 2 key_num**********************
mtxkey_idx  key_num
    0           1
    1           5
    2           2
    3           6
    4           9
    5           16
    6           10
    7           15
    8           3
    9           7
    10          11
    11          14
    12          4
    13          8
    14          12
    15          13
****************************/
static xincx_kbs_mtxkey_cfg_t mtxkeys[] =
{
  [0] = {1,KBS_ROW_BUTTON_1, KBS_COL_BUTTON_1, mtxkey_event_handler},
  [1] = {5,KBS_ROW_BUTTON_1, KBS_COL_BUTTON_2, mtxkey_event_handler},
  [2] = {2,KBS_ROW_BUTTON_2, KBS_COL_BUTTON_1, mtxkey_event_handler},//BUTTON_PULLDOWN
  [3] = {6,KBS_ROW_BUTTON_2, KBS_COL_BUTTON_2, mtxkey_event_handler},
  
  #if defined (BOARD_PCA10060)
  
  [4] = {9,KBS_ROW_BUTTON_1, KBS_COL_BUTTON_3, mtxkey_event_handler},
  [5] = {16,KBS_ROW_BUTTON_1, KBS_COL_BUTTON_4, mtxkey_event_handler},
  [6] = {10,KBS_ROW_BUTTON_2, KBS_COL_BUTTON_3, mtxkey_event_handler},
  [7] = {15,KBS_ROW_BUTTON_2, KBS_COL_BUTTON_4, mtxkey_event_handler},
  
  [8] = {3,KBS_ROW_BUTTON_3, KBS_COL_BUTTON_1, mtxkey_event_handler},
  [9] = {7,KBS_ROW_BUTTON_3, KBS_COL_BUTTON_2, mtxkey_event_handler},
  [10] = {11,KBS_ROW_BUTTON_3, KBS_COL_BUTTON_3, mtxkey_event_handler},
  [11] = {14,KBS_ROW_BUTTON_3, KBS_COL_BUTTON_4, mtxkey_event_handler},
  
  [12] = {4,KBS_ROW_BUTTON_4, KBS_COL_BUTTON_1, mtxkey_event_handler},
  [13] = {8,KBS_ROW_BUTTON_4, KBS_COL_BUTTON_2, mtxkey_event_handler},
  [14] = {12,KBS_ROW_BUTTON_4, KBS_COL_BUTTON_3, mtxkey_event_handler},
  [15] = {13,KBS_ROW_BUTTON_4, KBS_COL_BUTTON_4, mtxkey_event_handler},
  
  
  #endif

  
  

};
            
static void mtxkey_event_handler(int16_t mtxkey_idx,uint16_t key_val,uint8_t row_pin,uint8_t col_pin,uint8_t button_action)
{

    printf("mtxkey_idx:%d,key_val:%d,row_pin:%d,col_pin:%d,action:%d\r\n",mtxkey_idx,key_val,row_pin,col_pin,button_action);
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

   // kbs_mtxkey_drv_test();
    kbs_mtxkey_bsp_test();
    

    while(1) {
 
       app_sched_execute();
	      
       if(LastTimeGulSystickCount!=GulSystickCount)//10msִ
	   {	
           if(LastTimeGulSystickCount % 200 == 0)
           {
             //  printf("LastTimeGulSystickCount:%d\n",LastTimeGulSystickCount/200);            
           }           

		   LastTimeGulSystickCount=GulSystickCount;
          
			 
	   }


    }
}




