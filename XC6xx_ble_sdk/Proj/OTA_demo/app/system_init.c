
#include	"stdio.h"
void SystemInit (void)
{
	//此寄存器设置0x2d，有助于提升adc采集的稳定性，建议必须采用
	//此寄存器的默认值是0x2c，设为0x2d是为了方便在低功耗和非低功耗中折中同时使用；
	//如果不使用低功耗不需要考虑功耗那么可以直接设置为0x2e;
	*((volatile unsigned *)(0x40002400 + 0x20))  =0x2d; 
	
    
    
}
 


struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;


int fputc(int ch, FILE *f) {
 	return 0;
}


int ferror(FILE *f) {
  /* Your implementation of ferror */
  return EOF;
}


void _ttywrch(int ch) {
   
}

void _sys_exit(int return_code) {
label:  goto label;  /* endless loop */
}

