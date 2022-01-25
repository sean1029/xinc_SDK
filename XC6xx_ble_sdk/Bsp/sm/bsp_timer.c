
#include    "bsp_timer.h"
#include    "xinc_m0.h"


#define     	__write_hw_reg32(reg,val)  ((*reg) = (val))
#define     	__read_hw_reg32(reg, val)  ((val) = (*reg))

#define		    CPR_BASE                0x40000000
#define		    TIMER_BASE              0x40003000
/*----------------------------------------------------------------------------------------------------
	INCLUDE HEADE FILES
----------------------------------------------------------------------------------------------------*/
#define         CPR_CTLAPBCLKEN_GRCTL   ((volatile unsigned *)(CPR_BASE + 0x070))
#define         CPR_LP_CTL              ((volatile unsigned *)(CPR_BASE + 0x118))
//- TIMER
#define     	CPR_TIMER_CLK_CTL(a)	((volatile unsigned *)(CPR_BASE + 0x58 + (a * 0x04)))
#define     	TIMERx_TLC(a)     		((volatile unsigned *)(TIMER_BASE + 0x00 + (a * 0x14)))
#define     	TIMERx_TCV(a)     		((volatile unsigned *)(TIMER_BASE + 0x04 + (a * 0x14)))
#define     	TIMERx_TCR(a)     		((volatile unsigned *)(TIMER_BASE + 0x08 + (a * 0x14)))
#define     	TIMERx_TIC(a)     		((volatile unsigned *)(TIMER_BASE + 0x0C + (a * 0x14)))
#define     	TIMERx_TIS(a)      		((volatile unsigned *)(TIMER_BASE + 0x10 + (a * 0x14)))
