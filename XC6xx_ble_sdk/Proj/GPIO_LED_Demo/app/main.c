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


#define  BLINK_LED_TEST_1
//#define  BLINK_LED_TEST_2
//#define  BLINK_LED_TEST_3
//#define  BLINK_LED_TEST_4
//#define  BLINK_LED_TEST_5



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



void blink_LEDs_test1(void)
{
    /* Configure board. */
    bsp_board_init(BSP_INIT_LEDS);

    /* Toggle LEDs. */
    while (true)
    {
        for (int i = 0; i < LEDS_NUMBER; i++)
        {
            bsp_board_led_invert(i);
            xinc_delay_ms(500);
        }
    }
}

void blink_LEDs_test2(void)
{
   //配置用于驱动 LED 指示灯 D1 的管脚，即配置 P0.29 为输出
   xinc_gpio_cfg_output(LED_1);

   //LED 指示灯 D1 初始状态设置为熄灭，即引脚 P0.29 为输出高电平
   xinc_gpio_pin_set(LED_1);

    
    while (true)
    {
        
#if 0        
        //P0.4 输出高电平，熄灭指示灯 D1
        xinc_gpio_pin_set(LED_1);
        //软件延时 200ms
        xinc_delay_ms(200);
        //P0.4 输出低电平，点亮指示灯 D1
        xinc_gpio_pin_clear(LED_1);
        //软件延时 200ms
        xinc_delay_ms(200);
        
#else
        //以下是用 xinc_gpio_pin_toggle 函数实现驱动 led 闪烁
        //翻转引脚 P0.29 的输出状态，即翻转指示灯 D1 的状态
        xinc_gpio_pin_toggle(LED_1);
        //软件延时 200ms
        xinc_delay_ms(200);
#endif //
    }
}



void blink_LEDs_test3(void)
{
    uint8_t i;

    //配置用于驱动 LED 指示灯 D1 D2 D3 D4 的引脚脚，即配置 为输出
    xinc_gpio_range_cfg_output(LED_START, LED_STOP);
    //4个 LED 初始状态设置为熄灭，即 P0.13~P0.16 输出高电平
    xinc_gpio_pin_set(LED_1);
    xinc_gpio_pin_set(LED_2);
    xinc_gpio_pin_set(LED_3);
    xinc_gpio_pin_set(LED_4);

    while(true)
    {
        for(i=0;i<6;i++)
        {
            //指示灯 D1 状态翻转
            xinc_gpio_pin_toggle(LED_1);
            //软件延时 150ms
            xinc_delay_ms(150);
            xinc_gpio_pin_toggle(LED_2);
            xinc_delay_ms(150);
            xinc_gpio_pin_toggle(LED_3);
            xinc_delay_ms(150);
            xinc_gpio_pin_toggle(LED_4);
            xinc_delay_ms(150);
        }

        //所有 LED 同时闪烁 2 次
        for(i=0;i<2;i++)
        {
            //P0.4 输出高电平，D1 熄灭
            xinc_gpio_pin_set(LED_1);
            xinc_gpio_pin_set(LED_2);
            xinc_gpio_pin_set(LED_3);
            xinc_gpio_pin_set(LED_4);
            //软件延时 150ms
            xinc_delay_ms(150);
            //P0.4 输出低电平，D1 点亮
            xinc_gpio_pin_clear(LED_1);
            xinc_gpio_pin_clear(LED_2);
            xinc_gpio_pin_clear(LED_3);
            xinc_gpio_pin_clear(LED_4);
            xinc_delay_ms(150);
        }
        //熄灭所有 LED
        xinc_gpio_pin_set(LED_1);
        xinc_gpio_pin_set(LED_2);
        xinc_gpio_pin_set(LED_3);
        xinc_gpio_pin_set(LED_4);
        //软件延时 500ms
        xinc_delay_ms(500);
    }
}

void blink_LEDs_test4(void)
{
    uint8_t i;

    //配置用于驱动 LED 指示灯 D1 D2 D3 D4 的引脚脚，即配置 P0.13~P0.16 为输出，并将 LED
    //的初始状态设置为熄灭
    bsp_board_init(BSP_INIT_LEDS);
    while(true)
    {
        //使用 BSP 函数翻转 LED 指示灯状态
        for(i=0;i<12;i++)
        {
            for(int j = 0; j < LEDS_NUMBER; j++)
            {
                //翻转 LED 状态
                bsp_board_led_invert(j);
                //软件延时 150ms
                xinc_delay_ms(150);
            }
        }


        //所有 LED 同时闪烁 2 次
        for(i=0;i<2;i++)
        {
            //P0.4 输出高电平，D1 熄灭
            bsp_board_leds_on();
            //软件延时 150ms
            xinc_delay_ms(150);
            bsp_board_leds_off();
            //软件延时 150ms
            xinc_delay_ms(150);
        }
        //软件延时 500ms
        xinc_delay_ms(500);
    }

}
     
void blink_LEDs_test5(void)
{
    uint8_t i;
    xincx_gpio_init();
    xincx_gpio_out_config_t gpio;
    gpio.init_state = XINC_GPIO_INITIAL_VALUE_HIGH;
    xincx_gpio_out_init(LED_1,&gpio);
    xincx_gpio_out_init(LED_2,&gpio);

    while(true)
    {
        for(i=0;i<6;i++)
        {
            //指示灯 D1 状态翻转
            xincx_gpio_out_toggle(LED_1);
            //软件延时 150ms
            xinc_delay_ms(150);
            xincx_gpio_out_toggle(LED_2);
            xinc_delay_ms(150);
        }

        //所有 LED 同时闪烁 2 次
        for(i=0;i<2;i++)
        {
            //P0.4 输出高电平，D1 熄灭
            xincx_gpio_out_set(LED_1);
            xincx_gpio_out_set(LED_2);
    
            //软件延时 150ms
            xinc_delay_ms(150);
            //P0.4 输出低电平，D1 点亮
            xincx_gpio_out_clear(LED_1);
            xincx_gpio_out_clear(LED_2);
            xinc_delay_ms(150);
        }
        //熄灭所有 LED
        xincx_gpio_out_set(LED_1);
        xincx_gpio_out_set(LED_2);
        //软件延时 500ms
        xinc_delay_ms(500);
    }
    
}
int	main(void)
{


	key_init();
    xincx_gpio_init();
    
    scheduler_init();
    app_timer_init();
    
    SysTick_Config(32000000/100);
    SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;

#ifdef BLINK_LED_TEST_1
    blink_LEDs_test1();
#endif  

#ifdef BLINK_LED_TEST_2
    blink_LEDs_test2();
#endif  
    
#ifdef BLINK_LED_TEST_3
    blink_LEDs_test3();
#endif    
    
#ifdef BLINK_LED_TEST_4
    blink_LEDs_test4();
#endif    
    
#ifdef BLINK_LED_TEST_5
    blink_LEDs_test5();
#endif        


    

    while(1) {

    
       app_sched_execute();
			
	//   ble_system_idle();
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



