#include    "platform.h"
#include    "fix_bsp_gpadc.h"
extern  void   xc_spi0_flash(void);
extern  void   xc_uart1_init(void);
extern  void   xc_uart0_init(void);

void xc_init(void)
{	

		#if (XC_TIMER==1)
		xc_timer();
		#endif
		
		#if (XC_GPIO==1)
		xc_gpio();
		#endif
		
		#if (XC_GPADC==1)	
		test_xc_gpadc();
		#endif    
        while(1);	
}