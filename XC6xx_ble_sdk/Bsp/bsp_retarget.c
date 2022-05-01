#include "xinc.h"
#include	"Includes.h"
#include	"stdio.h"
#include "bsp_gpio.h"
#pragma     import(__use_no_semihosting_swi)

#define	__DEBUG_OUT_PORT        0			//- 0:uart0 , 1:uart1  

int  	sendchar(int c)	;
void 	retarget_init(void)
{  
#if	(__DEBUG_OUT_PORT == 1)
		#define CPR_GPIO_FUN_SEL5        ((volatile unsigned *)(0x40000000 + 0x0d4))
        unsigned	int	val;
		//- uart1 clk	
		*((unsigned	int volatile*)0x40000070) = 0x200020;

        __read_hw_reg32(CPR_GPIO_FUN_SEL5 , val);
        val &= 0xFFFF;
        val |= (8<<16)|(7<<24);									       
        __write_hw_reg32(CPR_GPIO_FUN_SEL5 , val);
    
		(*((volatile unsigned *)(0x40000000	+ 0x38))) = 0x110018;
		(*((volatile unsigned  *)( 0x40000000 + 0x3c ))) = 0x480271;
		(*((volatile unsigned  *)( 0x40011000 + 0x0c ))) = 0x80;
		(*((volatile unsigned  *)( 0x40011000 + 0x00 ))) = 0x01;
		(*((volatile unsigned  *)( 0x40011000 + 0x0c ))) = 0x03;
		(*((volatile unsigned  *)( 0x40011000 + 0x08 ))) = 0xb7;
#else
#define CPR_GPIO_FUN_SEL4        ((volatile unsigned *)(0x40000000 + 0x0d0))
#define	UART0_USR	             ((volatile unsigned *)(0x40010000 + 0x7c)) 
#define CPR_RSTCTL_SUBRST_SW     ((volatile unsigned *)(0x40000000 + 0x104))    
#define CPR_CTLAPBCLKEN_GRCTL    ((volatile unsigned *)(0x40000000 + 0x070))
#define CPR_UART0_CLK_GRCTL      ((volatile unsigned *)(0x40000000 + 0x030))
#define CPR_UART0_CLK_CTL        ((volatile unsigned *)(0x40000000 + 0x034))
#define	UART0_TCR				 ((volatile unsigned *)(0x40010000 + 0x0c))
#define UART0_DLL				 ((volatile unsigned *)(0x40010000 + 0x00))
#define UART0_DLH				 ((volatile unsigned *)(0x40010000 + 0x04)) 
#define	UART0_FCR				 ((volatile unsigned *)(0x40010000 + 0x08))    
        unsigned	int	val;
        __read_hw_reg32(CPR_GPIO_FUN_SEL4 , val);
        val &= 0xFFFF;
        val |= ((1<<16) | (2<<24));									        
        __write_hw_reg32(CPR_GPIO_FUN_SEL4 , val);
        
        do  {
            __read_hw_reg32(UART0_USR, val);
        }while(val&0x01);
		__write_hw_reg32(CPR_RSTCTL_SUBRST_SW , 0x10000);					
		__write_hw_reg32(CPR_RSTCTL_SUBRST_SW , 0x10001);					
		__write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL , 0x100010);					
		__write_hw_reg32(CPR_UART0_CLK_GRCTL, 0x110018);
		__write_hw_reg32(CPR_UART0_CLK_CTL, 0x480271);
		__write_hw_reg32(UART0_TCR , 0x80);                					
		__write_hw_reg32(UART0_DLL , 0x01);
		__write_hw_reg32(UART0_DLH , 0x00);                 		
		__write_hw_reg32(UART0_TCR , 0x03);                		
		__write_hw_reg32(UART0_FCR , 0x37);   
        gpio_direction_input(19,0);
        gpio_direction_input(18,0);
        gpio_direction_output(18);
		
#endif	
        
        char *str = "\n SYSTEM BUILD TIME\n";
        for(int i = 0 ; i < 20;i++)
        {
            sendchar(str[i]);
        }
        
	//	while(1);
         //   printf("\n SYSTEM BUILD TIME: %s %s \n",__DATE__,__TIME__);

}

int  	sendchar(int c)				
{
		unsigned	int	status;
#if	(__DEBUG_OUT_PORT == 1)	
		for(; ;)
		{
			status = (*((volatile unsigned  *)( 0x40011000 + 0x14 )));
			status &= 0x20;
			if(status == 0x20) break;
		}
		(*((volatile unsigned  *)( 0x40011000 + 0x00 ))) = c;
		return (1);	
#else
		for(; ;)
		{
			status = (*((volatile unsigned  *)( 0x40010000 + 0x14 )));
			status &= 0x20;
			if(status == 0x20) break;
		}
		(*((volatile unsigned  *)( 0x40010000 + 0x00 ))) = c;
		return (1);	
#endif	
}

struct __FILE { int handle; /* Add whatever you need here */ };

FILE __stdout;
#if  defined (XC60XX_M0)

int fputc(int ch, FILE *f) {
  return (sendchar(ch));
}


int ferror(FILE *f) {
  /* Your implementation of ferror */
  return EOF;
}


void _ttywrch(int ch) {
  sendchar(ch);
}

void _sys_exit(int return_code) {
label:  goto label;  /* endless loop */
}
#endif 


#if 0

void xinc_trace_u16_print(uint16_t value, uint8_t index, uint16_t level)
{

}

void xinc_trace_u32_print(uint32_t value, uint8_t index) //, uint16_t level
{

}
#else
void xinc_trace_u16_print(uint16_t value, uint8_t index, uint16_t level)
{

}

void xinc_trace_u32_print(uint32_t value, uint8_t index) //, uint16_t level
{
   
}

#endif

