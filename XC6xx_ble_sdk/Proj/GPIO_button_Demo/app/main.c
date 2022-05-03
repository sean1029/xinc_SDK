#include    <stdio.h>
#include    <string.h>
#include	"Includes.h"
#include "xinc_gpio.h"
#include "xincx_gpio.h"
#include "xinc_drv_spi.h"
#include "AT24C02.h"
#include "xinc_drv_saadc.h"
#include "xinc_drv_rtc.h"
#include "bsp_clk.h"
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



//#define    GPIO_BUTTON_TEST_1
//#define    GPIO_BUTTON_TEST_2 
#define    GPIO_BUTTON_TEST_3 

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

#if defined GPIO_BUTTON_TEST_1
void gpio_buttun_test1()
{
    bsp_board_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS);
    while(true)
    {
        //检测按键 S1 是否按下
        if(xinc_gpio_pin_read(BUTTON_1) == 0)
        {
            //点亮 LED 指示灯 D1
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_1));
            while(xinc_gpio_pin_read(BUTTON_1) == 0);//等待按键释放
            //熄灭 LED 指示灯 D1
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_1));
        }
        
        if(xinc_gpio_pin_read(BUTTON_2) == 0)
        {
            //点亮 LED 指示灯 D2
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_2));
            while(xinc_gpio_pin_read(BUTTON_2) == 0);//等待按键释放
            //熄灭 LED 指示灯 D2
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_2));
        }
#if defined (BOARD_PCA10060)
        if(xinc_gpio_pin_read(BUTTON_3) == 0)
        {
            //点亮 LED 指示灯 D2
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_3));
            while(xinc_gpio_pin_read(BUTTON_3) == 0);//等待按键释放
            //熄灭 LED 指示灯 D2
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_3));
        }
        
        if(xinc_gpio_pin_read(BUTTON_4) == 0)
        {
            //点亮 LED 指示灯 D2
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_4));
            while(xinc_gpio_pin_read(BUTTON_4) == 0);//等待按键释放
            //熄灭 LED 指示灯 D2
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_4));
        }
#endif
    }

}
#endif // GPIO_BUTTON_TEST_1


#if defined GPIO_BUTTON_TEST_2
/**@brief Function for handling events from the button handler module.
 *
 * @param[in] pin_no        The pin that the event applies to.
 * @param[in] button_action The button action (press/release).
 */
#define S1BUTTON_BUTTON_PIN            BSP_BUTTON_0   
#define S2BUTTON_BUTTON_PIN            BSP_BUTTON_1 
#if defined (BOARD_PCA10060)
#define S3BUTTON_BUTTON_PIN            BSP_BUTTON_2   
#define S4BUTTON_BUTTON_PIN            BSP_BUTTON_3
#endif

#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(10) 

static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    ret_code_t err_code;
    uint8_t rxbuff[AT24Cxx_PAGESIZE];
    uint8_t txbuff[AT24Cxx_PAGESIZE];
    uint16_t addr;
    switch (pin_no)
    {
        case S1BUTTON_BUTTON_PIN:
        {
             //检测按键 S1 是否按下
            if(button_action == APP_BUTTON_PUSH)
            {
                //点亮 LED 指示灯 D1
               bsp_board_led_on(bsp_board_pin_to_led_idx(LED_1));
            }    
            else//S1 按键释放
            {
                //熄灭 LED 指示灯 D1
                bsp_board_led_off(bsp_board_pin_to_led_idx(LED_1));
            }

            
        }break;
          
        case S2BUTTON_BUTTON_PIN:
        {
             //检测按键 S2 是否按下
            if(button_action == APP_BUTTON_PUSH)
            {
                //点亮 LED 指示灯 D2
                bsp_board_led_on(bsp_board_pin_to_led_idx(LED_2));
            }    
            else//S2 按键释放
            {
                //熄灭 LED 指示灯 D2
                bsp_board_led_off(bsp_board_pin_to_led_idx(LED_2));
            }

        
        }break;
        
        #if defined (BOARD_PCA10060)
        case S3BUTTON_BUTTON_PIN:
        {
             //检测按键 S3 是否按下
            if(button_action == APP_BUTTON_PUSH)
            {
                //点亮 LED 指示灯 D3
                bsp_board_led_on(bsp_board_pin_to_led_idx(LED_3));
            }    
            else//S2 按键释放
            {
                //熄灭 LED 指示灯 D3
                bsp_board_led_off(bsp_board_pin_to_led_idx(LED_3));
            }     
        }break;
        
        case S4BUTTON_BUTTON_PIN:
        {
             //检测按键 S3 是否按下
            if(button_action == APP_BUTTON_PUSH)
            {
                //点亮 LED 指示灯 D4
                bsp_board_led_on(bsp_board_pin_to_led_idx(LED_4));
            }    
            else//S2 按键释放
            {
                //熄灭 LED 指示灯 D4
                bsp_board_led_off(bsp_board_pin_to_led_idx(LED_4));
            }

        
        }break;
        
        
        #endif 
          
       
            

        default:
            APP_ERROR_HANDLER(pin_no);
            break;
    }
}

void gpio_buttun_test2()
{
    ret_code_t err_code;

    //The array must be static because a pointer to it will be saved in the button handler module.
    static app_button_cfg_t buttons[] =
    {
        {S1BUTTON_BUTTON_PIN, false, BUTTON_PULL, button_event_handler},//BUTTON_PULLDOWN
        {S2BUTTON_BUTTON_PIN, false, BUTTON_PULL, button_event_handler},
        #if defined (BOARD_PCA10060)
        {S3BUTTON_BUTTON_PIN, false, BUTTON_PULL, button_event_handler},//BUTTON_PULLDOWN
        {S4BUTTON_BUTTON_PIN, false, BUTTON_PULL, button_event_handler},
        #endif
    };

    bsp_board_init(BSP_INIT_LEDS);
    err_code = app_button_init(buttons, ARRAY_SIZE(buttons),
                               BUTTON_DETECTION_DELAY);
    APP_ERROR_CHECK(err_code);
}
#endif // #if defined GPIO_BUTTON_TEST_2



#if defined GPIO_BUTTON_TEST_3
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
            
        } break;
        // 按键S1 注册的释放事件的回调
        case BSP_EVENT_LED1_OFF:
        {
            //熄灭 LED 指示灯 D1
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_1));
        } break;
        // 按键S2 注册的长按事件的回调
        case BSP_EVENT_LED2_ON:
        {
            //点亮 LED 指示灯 D2
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_2));
        } break;
        // 按键S2 注册的释放事件的回调
        case BSP_EVENT_LED2_OFF:
        {
            //熄灭 LED 指示灯 D2
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_2));
        } break;
        
        #if defined (BOARD_PCA10060)
        case BSP_EVENT_LED3_ON:
        {
            //点亮 LED 指示灯 D3
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_3));
        } break;
        // 按键S2 注册的释放事件的回调
        case BSP_EVENT_LED3_OFF:
        {
            //熄灭 LED 指示灯 D3
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_3));
        } break;
        
        case BSP_EVENT_LED4_ON:
        {
            //点亮 LED 指示灯 D4
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_4));
        } break;
        // 按键S2 注册的释放事件的回调
        case BSP_EVENT_LED4_OFF:
        {
            //熄灭 LED 指示灯 D4
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_4));
        } break;
        
        #endif
           

        default:
            break; // No implementation needed
    }
}
void gpio_buttun_test3()
{
    ret_code_t err_code;
    //注册按键S1 按下时候的事件
    err_code = bsp_event_to_button_action_assign(0, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_LED1_ON ));
    //注册按键S1 释放时候的事件
    err_code = bsp_event_to_button_action_assign(0, BSP_BUTTON_ACTION_RELEASE, (bsp_event_t)(BSP_EVENT_LED1_OFF));
    //注册按键S2 长按下时候的事件
    err_code = bsp_event_to_button_action_assign(1, BSP_BUTTON_ACTION_LONG_PUSH, (bsp_event_t)(BSP_EVENT_LED2_ON ));
    //注册按键S2 释放时候的事件
    err_code = bsp_event_to_button_action_assign(1, BSP_BUTTON_ACTION_RELEASE, (bsp_event_t)(BSP_EVENT_LED2_OFF));

    //注册按键S3 按下时候的事件
    err_code = bsp_event_to_button_action_assign(2, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_LED3_ON ));
    //注册按键S3 释放时候的事件
    err_code = bsp_event_to_button_action_assign(2, BSP_BUTTON_ACTION_RELEASE, (bsp_event_t)(BSP_EVENT_LED3_OFF));
    
        //注册按键S4 按下时候的事件
    err_code = bsp_event_to_button_action_assign(3, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_LED4_ON ));
    //注册按键S4 释放时候的事件
    err_code = bsp_event_to_button_action_assign(3, BSP_BUTTON_ACTION_RELEASE, (bsp_event_t)(BSP_EVENT_LED4_OFF));
    
    err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS,bsp_evt_handler);

    APP_ERROR_CHECK(err_code);
}
#endif // #if defined GPIO_BUTTON_TEST_3


int	main(void)
{

    key_init();
    xincx_gpio_init();

    scheduler_init();
    app_timer_init();

    SysTick_Config(32000000/100);
    //  SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;
#if defined GPIO_BUTTON_TEST_1

    gpio_buttun_test1();
    
#elif defined GPIO_BUTTON_TEST_2
    
    gpio_buttun_test2();
    
#elif defined GPIO_BUTTON_TEST_3
    
    gpio_buttun_test3();
    
#endif 

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



