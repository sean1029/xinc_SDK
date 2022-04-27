#include    "xinc_m4.h"

//extern	void 	debug_uart_Init(void);
extern	void 	retarget_init(void);
void SystemInit (void)
{
	 
    //FPU settings
    SCB->CPACR |=((3UL<<10*2)|(3UL<<11*2));//set cp10 and cp11 Full access
    //M4 VTOR  settings
    SCB->VTOR = 0x10000000U;
	//release BootDone ,config clk source
	*((volatile unsigned *)(0x40000000+ 0x130)) &=(~0x06);   //bit2:1  select clock source
	//*((volatile unsigned *)(0x40000000+ 0x130)) |=0x06;      //[00-OSC32M],[01-bbpll direct output],[10-bbpll div output],[11-rc16M] 
    *((volatile unsigned *)(0x40000000+ 0x1B0)) |=0x10000000;//release pin bootctl1(gpio35)

	
    //此寄存器设置0x2d，有助于提升adc采集的稳定性，建议必须采用
    //此寄存器的默认值是0x2c，设为0x2d是为了方便在低功耗和非低功耗中折中同时使用；
    //如果不使用低功耗不需要考虑功耗那么可以直接设置为0x2e;
    *((volatile unsigned *)(0x40002400 + 0x20))  =0x2d;
    
    //print init
    //debug_uart_Init();
	retarget_init();
}

