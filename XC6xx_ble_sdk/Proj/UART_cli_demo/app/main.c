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

#include "xinc_cli_uart.h"
#include "xinc_cli.h"


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

XINC_CLI_UART_DEF(cli_uart, 2, 64, 64);
XINC_CLI_DEF(m_cli_uart, "cli:~$ ", &cli_uart.transport, '\r', 4);

void uart_cli_test(void)
{

    ret_code_t err_code = XINC_SUCCESS;
    
    
    //初始化 LED,用来接收cli 控制指令
    bsp_board_init(BSP_INIT_LEDS);
    
    xinc_drv_uart_config_t uart_config;
    uart_config.pseltxd = CLI2_TX_PIN_NUMBER;
    uart_config.pselrxd = CLI2_RX_PIN_NUMBER;
    uart_config.parity = XINC_UART_PARITY_EXCLUDED;
    uart_config.data_bits = XINC_UART_DATA_8_BITS;
    uart_config.stop_bits = XINC_UART_STOP_1_BITS;
    uart_config.hwfc    = XINC_UART_HWFC_DISABLED;
    uart_config.baudrate = XINC_UART_BAUDRATE_115200;
    uart_config.use_easy_dma = false ;

    err_code = xinc_cli_init(&m_cli_uart, &uart_config, false, false, XINC_LOG_SEVERITY_NONE);
    
    xinc_cli_start(&m_cli_uart);
    APP_ERROR_CHECK(err_code);

 }

void cli_processt(void)
{
    xinc_cli_process(&m_cli_uart);
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

	uart_cli_test();
   
    while(1) {

       app_sched_execute();
        
       cli_processt();
                        
       if(LastTimeGulSystickCount!=GulSystickCount)//10msִ��һ��
	   {		   

		   LastTimeGulSystickCount=GulSystickCount;
            if(LastTimeGulSystickCount % 300 == 0)
            {
                printf("LastTimeGulSystickCount:%d\n",LastTimeGulSystickCount/300);
            }
			 
	   }		   


    }
}



