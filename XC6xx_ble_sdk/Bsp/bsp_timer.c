
#include    "bsp_timer.h"
#include "xinc.h"
/* Device selection for device includes. */
#if  defined (XC60XX_M0)
    #include    "xinc_m0.h"
#elif defined (XC66XX_M4)
    #include    "xinc_m4.h"
#else
    #error "Device must be defined. See xinc.h."
#endif /* */


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

    
extern tHandler_callback   tHandler_Callback[4] ;

/* ---------------------------------------------------------------------------------------------------
- ��������: Init_Timer
- ��������: TIMER��ʼ��
- �������: 0/1/2/3, �����ֵ(Tick����, ��λ: ����, ���ֵΪ 131071 ms)
----------------------------------------------------------------------------------------------------*/
extern	void	Init_Timer(uint32_t ch, uint32_t msTick)
{
		uint32_t	val;
    	
		__write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL , 0x80008);		
		__write_hw_reg32(CPR_TIMER_CLK_CTL(ch) , 0x40000000);	//- TIMERѡ32Khz��Ϊʱ��Դ
	
		__read_hw_reg32(CPR_LP_CTL , val);
		val |= (1<<2);
		__write_hw_reg32(CPR_LP_CTL , val);						//- ˯��ʱ32Khz�л�
	
		__read_hw_reg32(TIMERx_TIC(ch) , val);								
		__write_hw_reg32(TIMERx_TCR(ch) , 0x00);
		//val = (msTick * 32000)/1000;
		val = msTick;
        __write_hw_reg32(TIMERx_TLC(ch) , val);
		__write_hw_reg32(TIMERx_TCR(ch) , 0x03);

		val = TIMER0_IRQn + ch;
		NVIC_EnableIRQ((IRQn_Type)val);							//- ʹ���ж�
}


///* ---------------------------------------------------------------------------------------------------
//- ��������: Timer_Register_Callback
//- ��������: ΪTIMERע���жϴ�����
//----------------------------------------------------------------------------------------------------*/
//extern	void	Timer_Register_Callback(tHandler_callback callback, uint32_t ch)
//{
//        tHandler_Callback[ch] = callback;
//}


/* ---------------------------------------------------------------------------------------------------
- ��������: TIMER0_Handler
- ��������: TIMER0�жϴ�����
----------------------------------------------------------------------------------------------------*/
extern	void	TIMER0_Handler(void)
{
		uint32_t	val;
		__read_hw_reg32(TIMERx_TIC(0) , val);					

        if(tHandler_Callback[0] != (tHandler_callback)0)
            tHandler_Callback[0](0);
}

/* ---------------------------------------------------------------------------------------------------
- ��������: Timer_Disable
- �������: 0/1/2/3
- ��������: �ر�Timer�������ж�
----------------------------------------------------------------------------------------------------*/
extern	void	Timer_disable(uint32_t ch)
{
    uint32_t    val;
    
    __write_hw_reg32(TIMERx_TCR(ch) , 0x00);                    
    
	val = TIMER0_IRQn + ch;
	NVIC_DisableIRQ((IRQn_Type)val);							   
}

void	TIMER1_Handler(void)
{
		uint32_t	val;
		__read_hw_reg32(TIMERx_TIC(1) , val);					//- ����ж�
    
        if(tHandler_Callback[1] != (tHandler_callback)0)
            tHandler_Callback[1](1);
}

uint32_t	timer_current_count(uint32_t ch)
{
	uint32_t	val;
	__read_hw_reg32(TIMERx_TCR(ch) , val);
	if((val & 0x01) == 0) return 0; 
	
	__read_hw_reg32(TIMERx_TCV(ch) , val);
    uint8_t count=0;
    while(val>0x1FFFF)//4096ms
    {    
       if((count++)>50) return 5*32;//5ms           
       __read_hw_reg32(TIMERx_TCV(ch) , val);
       
    }
	return val;
	
}
