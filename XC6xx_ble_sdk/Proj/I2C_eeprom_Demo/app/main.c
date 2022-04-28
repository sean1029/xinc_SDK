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

extern uint32_t  GulSystickCount;
uint32_t  LastTimeGulSystickCount=0xFF;


void key_init(void)
{
	Init_gpio();

}

/**@brief Function for handling events from the button handler module.
 *
 * @param[in] pin_no        The pin that the event applies to.
 * @param[in] button_action The button action (press/release).
 */
#define S1BUTTON_BUTTON_PIN            BSP_BUTTON_0   
#define S2BUTTON_BUTTON_PIN            BSP_BUTTON_1   
#define S3BUTTON_BUTTON_PIN            BSP_BUTTON_2   

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
            if(button_action == APP_BUTTON_RELEASE)
            {
                addr = 0x00;
                txbuff[0] = 0x55;
                printf("write one Byte:0x%x at addr :0x%x\r\n",txbuff[0],addr);
                AT24Cxx_write_byte(addr,txbuff[0]);
                AT24Cxx_read_buf(0x00,rxbuff,1);
                printf("read one Byte data at addr:0x%x,data:0x%x\r\n",addr,rxbuff[0]);
            }

            
        }break;
          
        case S2BUTTON_BUTTON_PIN:
        {
            if(button_action == APP_BUTTON_RELEASE)
            {              
                addr = 0x00;

                for(int i = 0 ;i < AT24Cxx_PAGESIZE;i++)
                {
                    txbuff[i] = 0x1 + i;
                }
                
                
                AT24Cxx_write_page(1,txbuff,AT24Cxx_PAGESIZE);
                printf("write one page data at addr :0x%x\r\n",addr);
                AT24Cxx_read_buf(0x00,rxbuff,AT24Cxx_PAGESIZE);
                printf("read page data at addr:0x%x,data \r\n",addr);
                
                for(int i = 0 ;i < AT24Cxx_PAGESIZE;i++)
                {
                    printf(":0x%x",rxbuff[i]);
                }
                printf("\r\n");
            }
        
        }break;

            
#ifdef BSP_BUTTON_2 //       
        case S3BUTTON_BUTTON_PIN:
        {
            if(button_action == APP_BUTTON_RELEASE)
            {
                addr = 0x07;

                for(int i = 0 ;i < 20;i++)
                {
                    txbuff[i] = 0x1 + i;
                }
                
                
                AT24Cxx_write_buf(addr,txbuff,20);
                printf("write 20 Byte data at addr :0x%x\r\n",addr);
                AT24Cxx_read_buf(addr,rxbuff,20);
                printf("read 20 Byte data at addr:0x%x,data \r\n",addr);
                
                for(int i = 0 ;i < 20;i++)
                {
                    printf(":0x%x",rxbuff[i]);
                }
                printf("\r\n");
            }
        
        }break;

#endif            

        default:
            APP_ERROR_HANDLER(pin_no);
            break;
    }
}


static void buttons_init(void)
{
    ret_code_t err_code;

    //The array must be static because a pointer to it will be saved in the button handler module.
    static app_button_cfg_t buttons[] =
    {
        {S1BUTTON_BUTTON_PIN, false, BUTTON_PULL, button_event_handler},
        {S2BUTTON_BUTTON_PIN, false, BUTTON_PULL, button_event_handler},
        #ifdef BSP_BUTTON_2 //
        {S3BUTTON_BUTTON_PIN, false, BUTTON_PULL, button_event_handler}
        #endif
    };

    err_code = app_button_init(buttons, ARRAY_SIZE(buttons),
                               BUTTON_DETECTION_DELAY);
    APP_ERROR_CHECK(err_code);
}


/** @brief Function configuring gpio for pin toggling.
 */
static void buton_config(void)
{
    buttons_init();
   
}
static void AT24C02_init(void)
{
    i2c_at24c02_init();
   
}

#define SCHED_MAX_EVENT_DATA_SIZE           APP_TIMER_SCHED_EVENT_DATA_SIZE             //!< Maximum size of the scheduler event data.
#define SCHED_QUEUE_SIZE                    10                                          //!< Size of the scheduler queue.

static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}

     
int	main(void)
{

    key_init();
    xincx_gpio_init();
    scheduler_init();
    app_timer_init();

    SysTick_Config(32000000/100);
    // SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;

	printf("sbc_init_msbc\n");
    
    buton_config();
    AT24C02_init();

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



