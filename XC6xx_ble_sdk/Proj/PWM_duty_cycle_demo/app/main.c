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

static xinc_drv_pwm_t m_pwm0 = XINC_DRV_PWM_INSTANCE(0);
static xinc_drv_pwm_t m_pwm1 = XINC_DRV_PWM_INSTANCE(1);
static int8_t duty;
static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{

    switch (pin_no)
    {
        case S1BUTTON_BUTTON_PIN:
        {
             //检测按键 S1 是否按下
            if(button_action == APP_BUTTON_PUSH)
            {
                //减少 pwm 的占空比
                duty-= 5;
                if(duty <= 0)
                {
                    duty = 0;
                }
                printf("pwm_duty_cycle_update0 :%d\r\n",duty);
                xinc_drv_pwm_duty_cycle_update(&m_pwm0,duty);

            }    

            
        }break;
          
        case S2BUTTON_BUTTON_PIN:
        {
             //检测按键 S2 是否按下
            if(button_action == APP_BUTTON_PUSH)
            {
                //增加 pwm 的占空比
                duty+= 5;
                if(duty >= 100)
                {
                    duty = 100;
                }
                printf("pwm_duty_cycle_update1 :%d\r\n",duty);
                xinc_drv_pwm_duty_cycle_update(&m_pwm0,duty);

            }    
   
        }break;
          
                
        default:
            APP_ERROR_HANDLER(pin_no);
            break;
    }
}

	

void pwm_duty_test()
{
    ret_code_t err_code;

    //The array must be static because a pointer to it will be saved in the button handler module.
    static app_button_cfg_t buttons[] =
    {
        {S1BUTTON_BUTTON_PIN, false, BUTTON_PULL, button_event_handler},//BUTTON_PULLDOWN
        {S2BUTTON_BUTTON_PIN, false, BUTTON_PULL, button_event_handler},
    };


    err_code = app_button_init(buttons, ARRAY_SIZE(buttons),
                               BUTTON_DETECTION_DELAY);
//    
    APP_ERROR_CHECK(err_code);
    xinc_drv_pwm_config_t const config =
    {
        .clk_src = XINC_PWM_CLK_SRC_32M_DIV,//使用32M 时钟源
        .ref_clk   = XINC_PWM_REF_CLK_2MHzOr2K,//分频采用2M时钟作为pwm的参考时钟
        .frequency       = 1000,//设定pwm脉冲的频率
        .duty_cycle   = 2,//设置pwm 脉冲的占空比
        .start = true,//初始化完成后自动启动
        .inv_enable = true,//打开反向输出
        .inv_delay = 3,
        .mode = XINC_PWM_MODE_ACC_100
    };
    
    duty = config.duty_cycle;
    

    //该函数用来检测设定的 pwm 频率是否在对应的时钟下能够产生
    xinc_drv_pwm_freq_valid_range_check(config.clk_src,config.ref_clk,config.frequency);
    
    APP_ERROR_CHECK(xinc_drv_pwm_init(&m_pwm0, &config, NULL));
    
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
  //  SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;

    pwm_duty_test();

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



