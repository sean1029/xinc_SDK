
#include	"stdio.h"
void SystemInit (void)
{
	//�˼Ĵ�������0x2d������������adc�ɼ����ȶ��ԣ�����������
	//�˼Ĵ�����Ĭ��ֵ��0x2c����Ϊ0x2d��Ϊ�˷����ڵ͹��ĺͷǵ͹���������ͬʱʹ�ã�
	//�����ʹ�õ͹��Ĳ���Ҫ���ǹ�����ô����ֱ������Ϊ0x2e;
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

