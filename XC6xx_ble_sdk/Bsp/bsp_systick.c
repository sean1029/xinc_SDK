
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
	GulSystickCount++;//����һ��10ms
}


uint32_t SysTick_get(void)
{    
		return GulSystickCount;//
}
 
void SysTick_set(uint32_t sys_ticks)
{    
    GulSystickCount = sys_ticks;//
}
 


//��ʱnus
//nus:Ҫ��ʱ��us��.	
//nus:0~204522252(���ֵ��(2^32)/32)	    								   
void delay_us(u32 nus)
{		
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;				//LOAD��ֵ	    	 
	ticks=nus*32; 						    //��Ҫ�Ľ�����  32M�ٶ� 1us��Ҫ32��ʱ������
	told=SysTick->VAL;        				//�ս���ʱ�ļ�����ֵ
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	//����ע��һ��SYSTICK��һ���ݼ��ļ������Ϳ�����.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			//ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳�.
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

