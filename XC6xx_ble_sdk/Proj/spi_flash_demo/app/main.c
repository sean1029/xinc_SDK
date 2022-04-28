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
#include "xinc_spi_flash.h"


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


/**@brief Function for handling events from the button handler module.
 *
 * @param[in] pin_no        The pin that the event applies to.
 * @param[in] button_action The button action (press/release).
 */
#define S1BUTTON_BUTTON_PIN            BSP_BUTTON_0   
#define S2BUTTON_BUTTON_PIN            BSP_BUTTON_1   
#define S3BUTTON_BUTTON_PIN            BSP_BUTTON_2 

#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(10) 
#define SAMPLES_IN_BUFFER 32

#define TEST_FLASH_ADDR   (132* 1024) 
static	uint8_t w_buf[20];
static	uint8_t r_buf[20];
static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    ret_code_t err_code;

    switch (pin_no)
    {
//        case S3BUTTON_BUTTON_PIN:
//        {
//             //检测按键 S1 是否按下
//            if(button_action == APP_BUTTON_PUSH)
//            {
//                //擦除 falsh sector
//                printf("sector_erase addr:0x%x\r\n",TEST_FLASH_ADDR);
//                //点亮 LED 指示灯 D1
//                bsp_board_led_on(bsp_board_pin_to_led_idx(LED_1));
//                
//             //   spim_flash_sector_erase(TEST_FLASH_ADDR);
// 
//            }else
//            {
//                //熄灭LED 指示灯 D1
//                bsp_board_led_off(bsp_board_pin_to_led_idx(LED_1));
//            }            

//            
//        }break;
          
        case S2BUTTON_BUTTON_PIN:
        {
             //检测按键 S2 是否按下
            if(button_action == APP_BUTTON_PUSH)
            {
                 printf("write data:\r\n");
                //点亮 LED 指示灯 D2
                bsp_board_led_on(bsp_board_pin_to_led_idx(LED_2));
               
                for(int i = 0;i < 20;i ++)
                {
                    w_buf[i] = 'a' + i;
                    printf("%c:0x%x ",w_buf[i],w_buf[i]);
                }printf("\r\n");
                //向指定地址写入特定长度的数据
                spim_flash_write(TEST_FLASH_ADDR,w_buf,20);
		
	
            }else
            {
                //熄灭LED 指示灯 D1
                bsp_board_led_off(bsp_board_pin_to_led_idx(LED_2));
            }            


        
        }break;
        
        case S1BUTTON_BUTTON_PIN:
        {
             //检测按键 S1 是否按下
            if(button_action == APP_BUTTON_PUSH)
            {
                //点亮 LED 指示灯 D2
                bsp_board_led_on(bsp_board_pin_to_led_idx(LED_1));
              
                 //向指定地址读特定长度的数据
                spim_flash_read(TEST_FLASH_ADDR,r_buf,20);
                printf("read_data char[] hex[]:\r\n");
                for(int i = 0;i < 20;i++)
                {
                    printf("%c:0x%x ",r_buf[i],r_buf[i]);
                    if(i % 16 == 15)
                    {
                        printf("\r\n");
                    }
                }
                printf("\r\n");
	
            }else
            {
                //熄灭LED 指示灯 D1
                bsp_board_led_off(bsp_board_pin_to_led_idx(LED_1));
            }            


        
        }break;
          
       
            

        default:
            APP_ERROR_HANDLER(pin_no);
            break;
    }
}

void spi_flash_test()
{
    ret_code_t err_code;

    //The array must be static because a pointer to it will be saved in the button handler module.
    static app_button_cfg_t buttons[] =
    {
        {S1BUTTON_BUTTON_PIN, false, BUTTON_PULL, button_event_handler},//BUTTON_PULLDOWN
        {S2BUTTON_BUTTON_PIN, false, BUTTON_PULL, button_event_handler},
     //   {S3BUTTON_BUTTON_PIN, false, BUTTON_PULL, button_event_handler},
    };
    //初始化 LED,用来指示按键按下状态
    bsp_board_init(BSP_INIT_LEDS);

    //初始化 按键,用来触发执行flash操作
    err_code = app_button_init(buttons, ARRAY_SIZE(buttons),
                               BUTTON_DETECTION_DELAY);
    
    printf("app_button_init err_code:%x\n",err_code);
    APP_ERROR_CHECK(err_code);

        
    spim_flash_init();

    APP_ERROR_CHECK(err_code);
 
}




#include "xinc_assert.h"

int	main(void)
{
    key_init();
    xincx_gpio_init();
    printf("scheduler_init\n");
    scheduler_init();
    printf("app_timer_init\n");
    app_timer_init();
    
    SysTick_Config(32000000/100);
   // SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;
    
    spi_flash_test();

    while(1) {

       app_sched_execute();
			
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



