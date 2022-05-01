#include "xinc.h"
#if defined(XC60XX_M0)
  #include    "xinc_m0.h"
#elif defined (XC66XX_M4)
  #include    "xinc_m4.h"
#else
    #error "Device must be defined. See xinc.h."
#endif
#include    "le_Power.h"

extern	void	ble_linklayer_init(void);
extern	void	ble_linklayer_schedule(void);
extern  void    ble_stack_init(void *init);
extern  void    ble_stack_schedule(void);
extern  void    hal_tick_init(void);
extern	uint32_t btstack_run_loop_embedded_get_ticks(void);
extern	uint32_t btstack_run_loop_embedded_ticks_for_ms(uint32_t time_in_ms);
extern	void 	btstack_run_loop_embedded_set_ticks(uint32_t ticks);

uint32_t TEST1 ;
/* ---------------------------------------------------------------------------------------------------
- 函数名称: ble_init
- 函数功能: ble初始化
----------------------------------------------------------------------------------------------------*/
void	ble_init(void *init)
{
	
	TEST1 = (uint32_t)ble_linklayer_init;
	printf("TEST1 = 0X%x\n",TEST1);
	ble_linklayer_init();
    ble_stack_init(init);
    /*---------------------------------------------------
    - Cortex-M0 Interrupt Priority Registers.
    ---------------------------------------------------*/
	(*((volatile unsigned *)(0xE000E400))) = 0xFFFFFF7F;
	(*((volatile unsigned *)(0xE000E404))) = 0xFFFFFFFF;
	(*((volatile unsigned *)(0xE000E408))) = 0xFFFFFFFF;
	(*((volatile unsigned *)(0xE000E40C))) = 0xFFFFFFFF;
	(*((volatile unsigned *)(0xE000E410))) = 0xFFFFFFFF;            
	NVIC_SetPriority(SysTick_IRQn, 2);
	NVIC_SetPriority(TIMER0_IRQn, 1);
	NVIC_SetPriority(TIMER1_IRQn, 0);	
}


/* ---------------------------------------------------------------------------------------------------
- 函数名称: ble_mainloop
- 函数功能: ble主循环
----------------------------------------------------------------------------------------------------*/
void	ble_mainloop(void)
{
	ble_linklayer_schedule();
    ble_stack_schedule();
}

typedef struct {
    unsigned int    version_main;
    unsigned int    version_no;
    unsigned char   version_date[12];    
    unsigned char   version_time[9];
}firmware_version_t;

const __attribute__((used)) firmware_version_t firmware_version = {
    .version_main = 0x1234A55A,
    .version_no   = 0x00000001,
    .version_date = __DATE__,
    .version_time = __TIME__,   
};
/* ---------------------------------------------------------------------------------------------------
- 函数名称: ble_version
- 函数功能: 记录软件版本
----------------------------------------------------------------------------------------------------*/
void *ble_version(void)
{
    return (void *)&firmware_version;
}

/* ---------------------------------------------------------------------------------------------------
- 函数名称: ble_lowpower_remaining_clock
- 函数功能: 基带低功耗剩下的时钟数
----------------------------------------------------------------------------------------------------*/
uint32_t	ble_lowpower_remaining_clock(uint32_t stat)
{
	return ble_lowpower_remaining_clk(stat);
}

/* ---------------------------------------------------------------------------------------------------
- 函数名称: ble_enter_lowpower
- 函数功能: 基带进入低功耗
----------------------------------------------------------------------------------------------------*/
void	ble_enter_lowpower(void)
{
	SysTick->CTRL  = 0;						//- 关闭SysTick中断.	
	NVIC_DisableIRQ(BLUETOOTH_IRQn);		//- 关闭BLE中断	
}

/* ---------------------------------------------------------------------------------------------------
- 函数名称: ble_exit_lowpower
- 函数功能: 基带退出低功耗
- 输入参数: 休眠过去的时间(单位:32K 对应的计数个数)
----------------------------------------------------------------------------------------------------*/
void	ble_exit_lowpower(uint32_t  t)
{
    uint32_t    VAL;
    VAL = btstack_run_loop_embedded_get_ticks();
    t >>= 5;                                //- (t*1000)/32000 = t/32
    VAL += btstack_run_loop_embedded_ticks_for_ms(t);
    btstack_run_loop_embedded_set_ticks(VAL);
    
	hal_tick_init();						//- 重新初始化SysTick,打开SysTick中断.
	NVIC_EnableIRQ(BLUETOOTH_IRQn);
}
