#include <stdio.h>
#include <string.h>
#include "Includes.h"
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


#define APP_UART_USE_INST_IDX   XINCX_APP_UART1_INST_IDX


 uint8_t rxd[500];
 uint16_t rxLen = 0;
 uint16_t rxReady = 0;
 uint16_t rxDone = 0;
void uart_event_handle(app_uart_evt_t * p_event)
{
    static uint8_t data_array[20];
    static uint8_t *p_data ;
    static uint16_t index = 0;
    uint32_t ret_val;
    uint32_t err_code;
    uint8_t  req_len = 255;

    switch (p_event->evt_type)
    {
       
        case APP_UART_DATA_READY:
          //  UNUSED_VARIABLE(app_uart_get(&data_array[0]));  
            req_len = 255;
            err_code = app_uart_gets(APP_UART_USE_INST_IDX,&rxd[rxLen],&req_len);
            rxLen+= req_len;
            rxReady++;
           
            bsp_board_led_invert(bsp_board_pin_to_led_idx(LED_1));
         //   printf("%c \r\n",data_array[0]);	
            break;
        
        case APP_UART_DATA_DONE:
          //  UNUSED_VARIABLE(app_uart_get(&data_array[0]));
            req_len = 255;
            err_code = app_uart_gets(APP_UART_USE_INST_IDX,&rxd[rxLen],&req_len);
            rxLen+=req_len;
            rxDone++;
            bsp_board_led_invert(bsp_board_pin_to_led_idx(LED_1));


            break;
        /**@snippet [Handling data from UART] */
        case APP_UART_COMMUNICATION_ERROR:
            
            break;

        case APP_UART_TX_EMPTY:
            //翻转指示灯 D2 状态，指示串口发送完成事件
            bsp_board_led_invert(bsp_board_pin_to_led_idx(LED_2));
            break;
        
        case APP_UART_DATA:
            app_uart_get(APP_UART_USE_INST_IDX,&data_array[0]);
            bsp_board_led_invert(bsp_board_pin_to_led_idx(LED_1));
           
            app_uart_put(APP_UART_USE_INST_IDX,data_array[0]);
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
#define S1BUTTON_BUTTON_PIN            BSP_BUTTON_0   


#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(10) 

static uint8_t buff[52];
static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{

 //   printf("button_event_handler,pin_no:%d\n",pin_no);
    switch (pin_no)
    {
        case S1BUTTON_BUTTON_PIN:
        {
            bsp_board_led_invert(bsp_board_pin_to_led_idx(LED_1));
           if(button_action == APP_BUTTON_PUSH)
            {
                
                for(int j = 0 ; j < 26;j++)
                {
                    buff[j] = 'a' + j;
                   
                }
                for(int j = 26 ; j < 52;j++)
                {
                    buff[j] = 'A' + j - 26;
                   
                }
                
                
                for(int j = 0 ; j < 2;j++)
                {
                    //通过串口发送数据
                    app_uart_puts(APP_UART_USE_INST_IDX,buff,30);
                   
                }
                
                
 
            }
            
        }break;
        
        default:break;
        
    }
}
void uart_handler_test(void)
{

    ret_code_t err_code = XINC_SUCCESS;


    
     //The array must be static because a pointer to it will be saved in the button handler module.
    static app_button_cfg_t buttons[] =
    {
        {S1BUTTON_BUTTON_PIN, false, BUTTON_PULL, button_event_handler},//BUTTON_PULLDOWN

    };
    

    //定义串口通讯参数配置结构体并初始化
    app_uart_comm_params_t const comm_params =
    {
        .uart_inst_idx = APP_UART_USE_INST_IDX,
        .rx_pin_no    = APP_UART_RX_PIN_NUMBER,//定义 uart 接收引脚
        .tx_pin_no    = APP_UART_TX_PIN_NUMBER,//定义 uart 发送引脚x
        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,//关闭 uart 硬件流控
        .use_parity   = false,//禁止奇偶检验
        .data_bits = 3,
        .stop_bits = 0,
        .baud_rate    = UART_BAUDRATE_BAUDRATE_Baud115200//uart 波特率设置为 115200bps
    };


    APP_ERROR_CHECK(err_code);
    
        //初始化 LED,用来指示uart 发送/接收状态
    bsp_board_init(BSP_INIT_LEDS);
 
        //初始化 按键,用来触发执行put 操作
    err_code = app_button_init(buttons, ARRAY_SIZE(buttons),
                               BUTTON_DETECTION_DELAY);
    rxLen =0;
        //初始化串口，注册串口事件回调函数
    APP_UART_FIFO_INIT(&comm_params,
                        UART_RX_BUF_SIZE,
                        UART_TX_BUF_SIZE,
                        uart_event_handle,
                        APP_IRQ_PRIORITY_LOWEST,
                        err_code);
 }





int	main(void)
{

    key_init();
    xincx_gpio_init();
    scheduler_init();
    app_timer_init();

    SysTick_Config(32000000/100);
  //  SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;

    
    uart_handler_test();




    while(1) {
        
       app_sched_execute();
        			
       if(LastTimeGulSystickCount!=GulSystickCount)//10msִ��һ��
	   {		   

		   LastTimeGulSystickCount=GulSystickCount;
           if(LastTimeGulSystickCount % 500 == 0)
           {
               printf("SystickCount:%d\n",LastTimeGulSystickCount);
            if(rxLen)
            {
                printf("rx_len:%d,rxDone:%d,rxReady:%d\r\n",rxLen,rxDone,rxReady);
                for(int i = 0;i < rxLen;i++)
                {
                    printf("rx_byte:%c\r\n",rxd[i]);
                }
                rxLen = 0; 
                rxReady = 0;
                rxDone = 0;
            }
               
           }
  
			 
	   }		   


    }
}



