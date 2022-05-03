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
 uint8_t rxd[500];
 uint16_t rxLen = 0;

#define APP_UART_USE_INST_IDX   XINCX_APP_UART2_INST_IDX

void uart_event_handle(app_uart_evt_t * p_event)
{
    static uint8_t data_array[20];
    static uint8_t *p_data ;
    static uint16_t index = 0;
    uint32_t ret_val;
    uint32_t err_code;
    uint8_t  req_len = 255;
//    printf("uart_event_handle p_event->evt_type:%d \r\n",p_event->evt_type);
    switch (p_event->evt_type)
    {
       
        case APP_UART_DATA_READY:
          //  UNUSED_VARIABLE(app_uart_get(&data_array[0]));  
            req_len = 255;
            err_code = app_uart_gets(APP_UART_USE_INST_IDX,&rxd[rxLen],&req_len);
            rxLen+= req_len;
           
            bsp_board_led_invert(bsp_board_pin_to_led_idx(LED_1));
         //   printf("%c \r\n",data_array[0]);	
            if(rxLen)
            {
                printf("READY rx_len:%d\r\n",rxLen);
                for(int i = 0;i < rxLen;i++)
                {
                    printf("rx_byte:%c\r\n",rxd[i]);
                }
                rxLen = 0; 
            }
            break;
        
        case APP_UART_DATA_DONE:
          //  UNUSED_VARIABLE(app_uart_get(&data_array[0]));
            req_len = 255;
            err_code = app_uart_gets(APP_UART_USE_INST_IDX,&rxd[rxLen],&req_len);
            rxLen+=req_len;
            bsp_board_led_invert(bsp_board_pin_to_led_idx(LED_1));
            if(rxLen)
            {
                printf("DONE rx_len:%d\r\n",rxLen);
                for(int i = 0;i < rxLen;i++)
                {
                    printf("rx_byte:%c\r\n",rxd[i]);
                }
                rxLen = 0; 
            }

            break;
        /**@snippet [Handling data from UART] */
        case APP_UART_COMMUNICATION_ERROR:
            
            break;

        case APP_UART_TX_EMPTY:
            //翻转指示灯 D2 状态，指示串口发送完成事件
            bsp_board_led_invert(bsp_board_pin_to_led_idx(LED_2));
            printf("TX_EMPTY\r\n");
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
#define S2BUTTON_BUTTON_PIN            BSP_BUTTON_1  

#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(10) 

static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    ret_code_t err_code;
    static uint8_t buff[26];
//    printf("button_event_handler pin_no:%d,button_action:%d\r\n",pin_no,button_action);
    switch (pin_no)
    {
        case S1BUTTON_BUTTON_PIN:
        {
           if(button_action == APP_BUTTON_PUSH)
            {
                
                for(int j = 0 ; j < 26;j++)
                {
                  buff[j] = 'a' + j;
                   
                }
                
                
                for(int j = 0 ; j < 2;j++)
                {
                    //通过串口发送数据
                    app_uart_puts(APP_UART_USE_INST_IDX,buff,26);
                   
                }
             //       app_uart_puts(APP_UART_USE_INST_IDX,buff,1); 
//app_uart_put(APP_UART_USE_INST_IDX,'a');                
            }
            
        }break;
        
        case S2BUTTON_BUTTON_PIN:
        {
            bsp_board_led_invert(bsp_board_pin_to_led_idx(BSP_LED_2));
        }break;
        
        default:break;
        
    }
}
const xinc_drv_timer_t TIMER_LED = XINC_DRV_TIMER_INSTANCE(2);

void timer_led_event_handler(xinc_timer_int_event_t event_type,uint8_t channel, void* p_context)
{

  // printf("timer_led_event_handler event_type:[%d],channel:%d\n",event_type,channel);
 
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
void uart_handler_test(void)
{

    ret_code_t err_code = XINC_SUCCESS;


    
     //The array must be static because a pointer to it will be saved in the button handler module.
    static app_button_cfg_t buttons[] =
    {
        {S1BUTTON_BUTTON_PIN, false, BUTTON_PULL, button_event_handler},//BUTTON_PULLDOWN
        {S2BUTTON_BUTTON_PIN, false, BUTTON_PULL, button_event_handler},//BUTTON_PULLDOWN

    };
    

    //定义串口通讯参数配置结构体并初始化
    app_uart_comm_params_t const comm_params =
    {
        .uart_inst_idx = APP_UART_USE_INST_IDX,
        .tx_pin_no    = APP_UART2_TX_PIN_NUMBER,//定义 uart 发送引脚x
        .rx_pin_no    = APP_UART2_RX_PIN_NUMBER,//定义 uart 接收引脚
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
    
    printf("uart_event_handle:%p \r\n",uart_event_handle);
        //初始化串口，注册串口事件回调函数
    APP_UART_FIFO_INIT(&comm_params,
                        UART_RX_BUF_SIZE,
                        UART_TX_BUF_SIZE,
                        uart_event_handle,
                        2/*APP_IRQ_PRIORITY_LOWEST*/,
                        err_code);
    
      uint32_t time_ms = 500; //Time(in miliseconds) between consecutive compare events.
    uint32_t time_ticks;
    
       //Configure TIMER_LED for generating simple light effect - leds on board will invert his state one after the other.
    xinc_drv_timer_config_t timer_cfg = XINC_DRV_TIMER_DEFAULT_CONFIG;
    
    
    
        err_code = xinc_drv_timer_init(&TIMER_LED, &timer_cfg, timer_led_event_handler);
    APP_ERROR_CHECK(err_code);

    time_ticks = xinc_drv_timer_ms_to_ticks(&TIMER_LED, time_ms);
    printf("time_ticks = [%d]\n",time_ticks);


    xinc_drv_timer_compare(&TIMER_LED, time_ticks,XINC_TIMER_MODE_USER_COUNTER ,true);//  // XINC_TIMER_MODE_USER_COUNTER //XINC_TIMER_MODE_AUTO_TIMER

    
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
    uart_handler_test();
    
    xinc_delay_ms(100);

    while(1) {


       app_sched_execute();
         
        			
       if(LastTimeGulSystickCount!=GulSystickCount)//10msִ��һ��
	   {		   

		   LastTimeGulSystickCount=GulSystickCount;
           if(LastTimeGulSystickCount % 500 == 0)
           {
               printf("SystickCount:%d\n",LastTimeGulSystickCount);              
              
           }
			 
	   }

    }
}



