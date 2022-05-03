#include    <stdio.h>
#include    <string.h>

#include	"Includes.h"

//#include "bsp_gpio.h"


//#include "bsp_timer.h"
//#include "bsp_pwm.h"
//#include "bsp_spi_flash.h"
#include "xinc_gpio.h"
//#include "bsp_uart.h"

#include "xincx_gpio.h"
#include "xinc_drv_spi.h"

#include "xinc_drv_saadc.h"
#include "xinc_drv_rtc.h"
//#include "bsp_clk.h"
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

void uart_event_handle(app_uart_evt_t * p_event)
{
    uint32_t ret_val;

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


#define RXBUF_LEN 10
uint8_t RxCnt = 0; //UART 接收字节数
uint8_t UartRxBuf[RXBUF_LEN]; //UART 接收缓存
uint8_t cr; //定义一个变量保存接收到的数据
void uart_led_test(void)
{

    ret_code_t err_code = XINC_SUCCESS;
    

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
 
        //初始化串口，注册串口事件回调函数
    APP_UART_FIFO_INIT(&comm_params,
                        UART_RX_BUF_SIZE,
                        UART_TX_BUF_SIZE,
                        uart_event_handle,
                        APP_IRQ_PRIORITY_LOWEST,
                        err_code);
    printf("APP_UART_FIFO_INIT:%x\r\n",err_code);
 }

 void uart_data_proess(uint8_t cr)
 {
    if((cr != '#')&&(RxCnt < 3))
    {
        UartRxBuf[RxCnt++] = cr;
    }
    else
    {
        //如果接收数据长度大于 3 个字节，表示接收出错，清零接收计数
        if(RxCnt >= 3)
        {
            RxCnt = 0;
        }
        else
        {
            //检查数据是否合法
            if((UartRxBuf[0] == 'D') || (UartRxBuf[0] == 'd'))
            {
                //减去 48，得到 ASCII 对应的十进制数值
                switch(UartRxBuf[1]-48)
                {
                    case 1:
                        bsp_board_leds_off(); //熄灭所有 LED 指示灯
                        bsp_board_led_on(bsp_board_pin_to_led_idx(LED_1));
                        RxCnt = 0;
                    break;
                    
                    case 2:
                        bsp_board_leds_off(); //熄灭所有 LED 指示灯
                        bsp_board_led_on(bsp_board_pin_to_led_idx(LED_2));
                        RxCnt = 0;
                    break;
   #if defined (BOARD_PCA10060)              
                    case 3:
                        bsp_board_leds_off(); //熄灭所有 LED 指示灯
                        bsp_board_led_on(bsp_board_pin_to_led_idx(LED_3));
                        RxCnt = 0;
                    break;
                    
                    case 4:
                        bsp_board_leds_off();
                        bsp_board_led_on(bsp_board_pin_to_led_idx(LED_4));
                        RxCnt = 0;
                    break;
    #endif             
                    default:
                        bsp_board_leds_off();
                        RxCnt = 0;
                        break;
                }
            }
                
        }
    }
 
 }


extern unsigned int GulSystickCount;
unsigned int LastTimeGulSystickCount;


int	main(void)
{
    key_init();
    
    printf("scheduler_init\n");
    scheduler_init();
    
    SysTick_Config(32000000/100);
  
    printf("app_timer_init\n");
    app_timer_init();
    
    xincx_gpio_init();

    uart_led_test();


    while(1) {

       app_sched_execute();
        if(app_uart_get(APP_UART_USE_INST_IDX,&cr) == XINC_SUCCESS)
        {
            uart_data_proess(cr);
            
        }
        

       if(LastTimeGulSystickCount!=GulSystickCount)//10msִ��һ��
	   {		   

		   LastTimeGulSystickCount=GulSystickCount;
           if(LastTimeGulSystickCount % 200 == 0)
           {
               printf("SystickCount:%d\n",LastTimeGulSystickCount);
              // app_uart_put(XINCX_APP_UART1_INST_IDX,'a');
           }
  
			 
	   }		   


    }
}



