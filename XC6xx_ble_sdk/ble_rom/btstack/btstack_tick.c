
#include	"Includes.h"
#include    "patch_function_id.h"
#define		CORE_CLK			    32000000ul      /* Set Processor frequency */
#define     TICKS_PER_SEC           100ul           /* Set the number of ticks in one second  */
#define     TICK_PERIOD_IN_MS       (1000ul/TICKS_PER_SEC)


void dummy_handler_no_param(void){}
extern  void (*tick_handler)(void) ;


void hal_tick_init(void)
{
	if(PATCH_FUN[HAL_TICK_INIT_ID]){
         ((void (*)(void))PATCH_FUN[HAL_TICK_INIT_ID])();
         return ;
    }

    SysTick_Config(CORE_CLK/TICKS_PER_SEC);
}

int  hal_tick_get_tick_period_in_ms(void){
    return  TICK_PERIOD_IN_MS;
}

void hal_tick_set_handler(void (*handler)(void)){
	
	if(PATCH_FUN[HAL_TICK_SET_HANDLER_ID]){
         ((void (*)(void))PATCH_FUN[HAL_TICK_SET_HANDLER_ID])();
         return ;
    }

    if (handler == 0){
        tick_handler = &dummy_handler_no_param;
        return;
    }
    tick_handler = handler;
}

void ble_tick_handler(void){
	(*tick_handler)();
}

