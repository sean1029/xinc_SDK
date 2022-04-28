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
#include "app_uart.h"

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

void uart_event_handle(app_uart_evt_t * p_event)
{
    static uint8_t data_array[20];
    static uint16_t index = 0;
    uint32_t ret_val;
    uint32_t err_code;

    switch (p_event->evt_type)
    {
       

        /**@snippet [Handling data from UART] */
        case APP_UART_COMMUNICATION_ERROR:
            
            break;

        case APP_UART_FIFO_ERROR:
            
            break;

        default:
            break;
    }
    (void)ret_val;
}
#define UART_TX_BUF_SIZE        64                                     /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE        64                                     /**< UART RX buffer size. */
void uart_loop_test(void)
{

    ret_code_t err_code;

    //定义串口通讯参数配置结构体并初始化
    app_uart_comm_params_t const comm_params =
    {
        .uart_inst_idx = XINCX_APP_UART1_INST_IDX,
        .rx_pin_no    = APP_UART_RX_PIN_NUMBER,//定义 uart 接收引脚
        .tx_pin_no    = APP_UART_TX_PIN_NUMBER,//定义 uart 发送引脚x
        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,//关闭 uart 硬件流控
        .use_parity   = false,//禁止奇偶检验
        .data_bits = 3,
        .stop_bits = 0,
        .baud_rate    = UART_BAUDRATE_BAUDRATE_Baud115200//uart 波特率设置为 115200bps
    };
    //初始化串口，注册串口事件回调函数
    APP_UART_FIFO_INIT(&comm_params,
                        UART_RX_BUF_SIZE,
                        UART_TX_BUF_SIZE,
                        uart_event_handle,
                        APP_IRQ_PRIORITY_LOWEST,
                        err_code);

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
  //  SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;

    
    uart_loop_test();

    while(1) {
    
       app_sched_execute();

       uint8_t cr;
        //查询是否接收到数据
        if (app_uart_get(XINCX_APP_UART1_INST_IDX,&cr) == XINC_SUCCESS)
        {
            //将接收的数据原样发回
            app_uart_put(XINCX_APP_UART1_INST_IDX,cr);

        }
        			
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



