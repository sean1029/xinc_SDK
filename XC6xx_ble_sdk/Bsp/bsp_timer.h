#ifndef _BSP_TIMER_H_
#define _BSP_TIMER_H_
#include <stdint.h>

typedef enum
{
	TIMER_CH0 = 0,
	TIMER_CH1 = 1,
	TIMER_CH2 = 2,
	TIMER_CH3 = 3,
	
}timer_ch_t;

void xc_timer_init(timer_ch_t timer_ch,uint32_t us);
extern	void	Timer_disable(uint32_t ch);
#endif
