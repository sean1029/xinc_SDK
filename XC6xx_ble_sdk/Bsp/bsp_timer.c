#include 	"Platform.h"
#include  "bsp_register_macro.h"
#include "bsp_timer.h"
 
#if 0
/* ---------------------------------------------------------------------------------------------------
- 函数名称: xc_timer_init
- 函数功能: 定时器初始化
- 函数形参：定时器号，定时多少us
- 创建日期: 2019-04-22
- 创 建 人：陈俊伟
----------------------------------------------------------------------------------------------------*/
void xc_timer_init1(timer_ch_t timer_ch,uint32_t us)
{
	__write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL,0x80008); //TIMER_PCLK 时钟使能
	__write_hw_reg32(CPR_TIMER_CLK_CTL(timer_ch),0x0F);//TIMERx_CLK 时钟控制寄存器 mclk_in(32MHz)/2*(0x0F + 0x1)
	__write_hw_reg32(TIMERx_TCR(timer_ch),0x0);//不屏蔽定时器中断，不使能定时器timer_num，
	__write_hw_reg32(TIMERx_TCR(timer_ch),0x0);//设置定时器工作在自运行计数模式
	//	__write_hw_reg32(TIMERx_TCR(timer_ch),0x2);//设置定时器工作在用户定义计数模式
	__write_hw_reg32(TIMERx_TLC(timer_ch),us);//载入计数器计数初值(32bits),该值应大于等于 0x4
    __write_hw_reg32(TIMERx_TCR(timer_ch),0x0);//使能定时器timer_num
	NVIC_EnableIRQ((IRQn_Type)(TIMER0_IRQn+timer_ch));
}

void xc_timer_init(timer_ch_t timer_ch,uint32_t us)
{
	uint32_t clk =0;
	clk = 0xf << 0;
	clk |= 0xf << 8;
	clk |= 0x01 << 28;
	__write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL,0x80008); //TIMER_PCLK 时钟使能
	__write_hw_reg32(CPR_TIMER_CLK_CTL(timer_ch),clk);//TIMERx_CLK 时钟控制寄存器 mclk_in(32MHz)/2*(0x0F + 0x1)
	__write_hw_reg32(TIMERx_TCR(timer_ch),0x0);//不屏蔽定时器中断，不使能定时器timer_num，
	__write_hw_reg32(TIMERx_TCR(timer_ch),0x0);//设置定时器工作在自运行计数模式
	//	__write_hw_reg32(TIMERx_TCR(timer_ch),0x2);//设置定时器工作在用户定义计数模式
	__write_hw_reg32(TIMERx_TLC(timer_ch),us);//载入计数器计数初值(32bits),该值应大于等于 0x4
    __write_hw_reg32(TIMERx_TCR(timer_ch),0x0);//使能定时器timer_num
	NVIC_EnableIRQ((IRQn_Type)(TIMER0_IRQn+timer_ch));
}

void xc_timer_start(timer_ch_t timer_ch,uint32_t us)
{
	//printf("xc_timer_start:%d \r\n",us);
	 __write_hw_reg32(TIMERx_TCR(timer_ch),0x0);//使能定时器timer_num
	__write_hw_reg32(TIMERx_TLC(timer_ch),us);
   __write_hw_reg32(TIMERx_TCR(timer_ch),0x1);//使能定时器timer_num
}
void xc_timer_restart(timer_ch_t timer_ch,uint32_t us)
{
	 __write_hw_reg32(TIMERx_TCR(timer_ch),0x0);//使能定时器timer_num
	__write_hw_reg32(TIMERx_TLC(timer_ch),us);
   __write_hw_reg32(TIMERx_TCR(timer_ch),0x1);//使能定时器timer_num
}

uint32_t  xc_timer_cnt_get(timer_ch_t timer_ch)
{
	uint32_t cur_val;
   __read_hw_reg32(TIMERx_TCV(timer_ch),cur_val);//
	return cur_val;
}

void xc_timer_stop(timer_ch_t timer_ch)
{	
	//printf("xc_timer_stop \r\n");
   __write_hw_reg32(TIMERx_TCR(timer_ch),0x0);//使能定时器timer_num

}
/* ---------------------------------------------------------------------------------------------------
- 函数名称: TIMER2_Handler
- 函数功能: 定时器2服务函数
- 函数形参：无
- 创建日期: 2019-04-22
- 创 建 人：陈俊伟
----------------------------------------------------------------------------------------------------*/


//void TIMER2_Handler(void)
//{
//	uint32_t val=0;
//	__read_hw_reg32(TIMER2_TIC , val);
//	//需要处理的定时器任务
//	printf("TIMER2_Handler\n");
//	
//}
/* ---------------------------------------------------------------------------------------------------
- 函数名称: TIMER3_Handler
- 函数功能: 定时器3服务函数
- 函数形参：无
- 创建日期: 2019-04-22
- 创 建 人：陈俊伟
----------------------------------------------------------------------------------------------------*/
void TIMER3_Handler(void)
{
	uint32_t val=0;
	__read_hw_reg32(TIMER3_TIC , val);
	GPIO_OUTPUT_LOW(4);
	GPIO_OUTPUT_HIGH(4);
	GPIO_OUTPUT_LOW(4);
	
	GPIO_OUTPUT_LOW(4);
	GPIO_OUTPUT_HIGH(4);
	GPIO_OUTPUT_LOW(4);
	
	GPIO_OUTPUT_LOW(4);
	GPIO_OUTPUT_HIGH(4);
	GPIO_OUTPUT_LOW(4);
	
	//需要处理的定时器任务
	printf("TIMER3_Handler\n");
	
}
#endif