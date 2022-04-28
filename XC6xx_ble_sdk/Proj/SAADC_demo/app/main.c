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


/**@brief Function for handling events from the button handler module.
 *
 * @param[in] pin_no        The pin that the event applies to.
 * @param[in] button_action The button action (press/release).
 */
#define S1BUTTON_BUTTON_PIN            BSP_BUTTON_0   
#define S2BUTTON_BUTTON_PIN            BSP_BUTTON_1   

#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(10) 
#define SAMPLES_IN_BUFFER 32

static xinc_drv_saadc_t m_saadc = XINCX_SAADC_INSTANCE(0);

static xinc_saadc_value_t   m_buffer_pool[2][SAMPLES_IN_BUFFER];

xinc_saadc_value_t adc_value;

static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    ret_code_t err_code;

    switch (pin_no)
    {
        case S1BUTTON_BUTTON_PIN:
        {
             //检测按键 S1 是否按下
            if(button_action == APP_BUTTON_PUSH)
            {
                //点亮 LED 指示灯 D1
                bsp_board_led_on(bsp_board_pin_to_led_idx(LED_1));
                //启动通道8，芯片内部adc的采样，使用非阻塞方式，在回调函数中得到采样的结果
                xincx_saadc_sample(&m_saadc,8);  
            }else
            {
                //熄灭LED 指示灯 D1
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
                //启动通道8，芯片内部adc的采样，使用阻塞方式,adc_value保存得到的采样结果
                xincx_saadc_sample_convert(&m_saadc,8,&adc_value);
                printf("sample_convert,value=[%d], before cali Voltage:%f V, after cali Voltage:%f V \r\n",\
                adc_value,((adc_value)*2.47)/(1.0*1024),   ((adc_value)*2.47)/(1.0*1024));		
            }else
            {
                //熄灭LED 指示灯 D1
                bsp_board_led_off(bsp_board_pin_to_led_idx(LED_2));
            }            


        
        }break;
          
       
            

        default:
            APP_ERROR_HANDLER(pin_no);
            break;
    }
}

void saadc_callback(xinc_drv_saadc_evt_t const * p_event)
{
    printf("%s\n",__func__);
    uint32_t val;
    
    if (p_event->type == XINC_DRV_SAADC_EVT_DONE)
    {
        //非阻塞方式得到的结果
        val = p_event->data.done.adc_value;
        printf("1.0v,channel=%d,value=[%d], before cali Voltage:%f V, after cali Voltage:%f V \r\n",\
            p_event->data.done.channel, val,((val)*2.47)/(1.0*1024),   ((val)*2.47)/(1.0*1024));		
    }
}	

void saadc_test()
{
    ret_code_t err_code;

    //The array must be static because a pointer to it will be saved in the button handler module.
    static app_button_cfg_t buttons[] =
    {
        {S1BUTTON_BUTTON_PIN, false, BUTTON_PULL, button_event_handler},//BUTTON_PULLDOWN
        {S2BUTTON_BUTTON_PIN, false, BUTTON_PULL, button_event_handler},
    };
    //初始化 LED,用来指示按键按下状态
    bsp_board_init(BSP_INIT_LEDS);

    //初始化 按键,用来触发adc 采用
    err_code = app_button_init(buttons, ARRAY_SIZE(buttons),
                               BUTTON_DETECTION_DELAY);
    
    APP_ERROR_CHECK(err_code);

        

    xinc_saadc_channel_config_t channel_config =  XINC_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE;

    //初始化saadc，config 设置为NULL，使用default 配置
    err_code = xinc_drv_saadc_init(&m_saadc,NULL, saadc_callback);
        
    APP_ERROR_CHECK(err_code);
    // 初始化 8 通道，只有初始化通道后，后面才能使用对应的采样函数
    err_code = xinc_drv_saadc_channel_init(&m_saadc,8, &channel_config);

    APP_ERROR_CHECK(err_code);

    //设置采样数据的 buffer
    err_code = xinc_drv_saadc_buffer_convert(&m_saadc,m_buffer_pool[0], SAMPLES_IN_BUFFER);

    APP_ERROR_CHECK(err_code);
}






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

    saadc_test();

    while(1) {

       app_sched_execute();
			
	//   ble_system_idle();
       if(LastTimeGulSystickCount!=GulSystickCount)//10msִ��һ��
	   {		   

           if(LastTimeGulSystickCount % 500 == 0)
           {
               printf("LastTimeGulSystickCount:%d\n",LastTimeGulSystickCount/200);            
           } 
           
		   LastTimeGulSystickCount=GulSystickCount;
			 
	   }		   


    }
}



