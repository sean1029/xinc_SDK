
#include "xinc.h"
#if defined(XC60XX_M0)
  #include    "xinc_m0.h"
#elif defined (XC66XX_M4)
  #include    "xinc_m4.h"
#else
    #error "Device must be defined. See xinc.h."
#endif

#include "bsp_gpio.h"
#define u32 uint32_t

volatile  unsigned int GulSystickCount=0;
void	SysTick_Handler(void)
{
	GulSystickCount++;//计数一次10ms
}


uint32_t SysTick_get(void)
{    
		return GulSystickCount;//
}
 
void SysTick_set(uint32_t sys_ticks)
{    
    GulSystickCount = sys_ticks;//
}
 


//延时nus
//nus:要延时的us数.	
//nus:0~204522252(最大值即(2^32)/32)	    								   
void delay_us(u32 nus)
{		
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;				//LOAD的值	    	 
	ticks=nus*32; 						    //需要的节拍数  32M速度 1us需要32个时钟周期
	told=SysTick->VAL;        				//刚进入时的计数器值
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	//这里注意一下SYSTICK是一个递减的计数器就可以了.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			//时间超过/等于要延迟的时间,则退出.
		}  
	};		
//    for(int i = 0;i < nus;i++)
//    {
//        for(int j = 0; j < 50;j++)
//        {
//            __nop();
//        }
//    }
}  



void delay_ms(u32 nms)
{	
	for(uint32_t i=0;i<nms;i++) delay_us(1000);
}

