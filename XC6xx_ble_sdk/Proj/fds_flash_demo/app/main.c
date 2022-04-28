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
#include "xinc_spi_flash.h"
#include "app_uart.h"

#include "xinc_cli_uart.h"
#include "xinc_cli.h"
#include "es_flash.h"


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


ret_code_t es_flash_test(void)
{
    ret_code_t         err_code = XINC_SUCCESS;

    uint8_t lock_key[SIZE_OF_LOCK_KEY];

    uint8_t pub_key[SIZE_OF_PUB_KEY];
    for(int i = 0; i < SIZE_OF_LOCK_KEY;i ++)
    {
        lock_key[i] = 'b' + i;
    }
    for(int i = 0; i < SIZE_OF_PUB_KEY;i ++)
    {
        pub_key[i] = '0' + i;
    }
    
    err_code = es_flash_access_lock_key(lock_key,ES_FLASH_ACCESS_READ); 
    
    if(err_code != XINC_SUCCESS)
    {
        err_code = es_flash_access_lock_key(lock_key,ES_FLASH_ACCESS_WRITE);
        if(err_code != XINC_SUCCESS)
        {
            printf("access_lock_key WRITE fail\r\n");
            return err_code;
        }
    }
    
    err_code = es_flash_access_pub_key(pub_key,ES_FLASH_ACCESS_READ);
    
    if(err_code != XINC_SUCCESS)
    {
        err_code = es_flash_access_pub_key(pub_key,ES_FLASH_ACCESS_WRITE);
        if(err_code != XINC_SUCCESS)
        {
            printf("access_pub_key write fail\r\n");
            return err_code;
        }
    }
 
    
    for(int i = 0; i < SIZE_OF_LOCK_KEY;i ++)
    {
        lock_key[i] = 0;
    }
    for(int i = 0; i < SIZE_OF_PUB_KEY;i ++)
    {
        pub_key[i] = 0;
    }
    
    printf("access_lock_key start read\r\n");
    err_code = es_flash_access_lock_key(lock_key,ES_FLASH_ACCESS_READ);

    if(err_code != XINC_SUCCESS)
    {
        printf("access_lock_key read fail\r\n");
        return err_code;
    }
    printf("access_lock_key read data:");
    for(int i = 0; i < SIZE_OF_LOCK_KEY;i ++)
    {
        printf("%c ",lock_key[i]);
    }printf("\r\n");

    err_code = es_flash_access_pub_key(pub_key,ES_FLASH_ACCESS_READ);

    if(err_code != XINC_SUCCESS)
    {
        printf("access_pub_key read fail\r\n");
        return err_code;
    }

    printf("access_pub_key read data:");
    for(int i = 0; i < SIZE_OF_PUB_KEY;i ++)
    {
        printf("%c ",pub_key[i]);
    }printf("\r\n");


    return err_code;
}

int	main(void)
{
    key_init();
    xincx_gpio_init();
    scheduler_init();
    printf("scheduler_init\n");
    app_timer_init();
    
    es_flash_init();
    es_flash_test();

    SysTick_Config(32000000/100);
  //  SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;

    

    while(1) {

       app_sched_execute();
                      
       if(LastTimeGulSystickCount!=GulSystickCount)//10msִ��һ��
	   {		   

		   LastTimeGulSystickCount=GulSystickCount;
            if(LastTimeGulSystickCount % 300 == 0)
            {
                printf("sched  cnt:%d\n",LastTimeGulSystickCount/300);
            }
			 
	   }		   


    }
}



