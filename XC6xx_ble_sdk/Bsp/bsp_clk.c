
/*----------------------------------------------------------------------------------------------------
	INCLUDE HEADE FILES
----------------------------------------------------------------------------------------------------*/
#include "bsp_register_macro.h"
#include "xinc_reg.h"
#include  "bsp_clk.h"



void xc_uart_clk_init(uint8_t ch,uint32_t baud)
{
				uint32_t	val;		
			__write_hw_reg32(CPR_RSTCTL_SUBRST_SW , (1<<(16+ch)));	//复位uart模块
			__write_hw_reg32(CPR_RSTCTL_SUBRST_SW , ((1<<(16+ch))|(1<<ch)));//解复位uart模块		
	
			__read_hw_reg32(CPR_LP_CTL , val);
			val &= ~(1<<(1-ch));
			__write_hw_reg32(CPR_LP_CTL , val);						//关闭uartx时钟保护
			
			val = (1<<(16+ch+4)) | (1<<(ch+4));
			__write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL , val); 			//打开uartx pclk
			__write_hw_reg32(CPR_UARTx_CLK_GRCTL(ch), 0x110018);
			__write_hw_reg32(CPR_UARTx_CLK_CTL(ch), baud>>4);	
	
}


void xc_gpio_clk_init(void)
{
	__write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL , 0x40004); /*gpio_pclk enable*/
	__write_hw_reg32(CPR_OTHERCLKEN_GRCTL , 0x10001);  /*gpio_clk  enable*/  
	
}

void xc_spi_clk_init(uint8_t ch)
{
    uint32_t    val;
    
    __write_hw_reg32(CPR_SPIx_MCLK_CTL(ch), 0x110010);//1分频			//- spi(x)_mclk = 32Mhz(When TXCO=32Mhz).
    __write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL , (0x1000100<<ch)); 	//- 打开spi(x) pclk.
    __read_hw_reg32(CPR_SSI_CTRL, val);
    val |= (ch==0)? 0x01: 0x30;
    __write_hw_reg32(CPR_SSI_CTRL, val); 
	
}

void xc_i2c_clk_init(void)
{
  __write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL, 0x8000800);

	__write_hw_reg32(CPR_RSTCTL_SUBRST_SW, 0x400000);
	__write_hw_reg32(CPR_RSTCTL_SUBRST_SW, 0x400040);

	__write_hw_reg32(CPR_I2C_CLK_CTL, 0x110011);                //-> i2c_mclk = 16mhz.
	
}

void xc_saadc_clk_init(void)
{
  __write_hw_reg32(CPR_RSTCTL_CTLAPB_SW , 0x10000000); /*先 使GPADC_RSTN=0，再使 GPADC_RSTN=1，软复位 GPADC 模块*/
	__write_hw_reg32(CPR_RSTCTL_CTLAPB_SW , 0x10001000); /*先 使GPADC_RSTN=0，再使 GPADC_RSTN=1，软复位 GPADC 模块*/
  __write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL , 0x20002000);//使能GPADC_PCLK_EN 的GPADC_PCLK时钟
	
	
}

void xc_rtc_clk_init(void)
{
  	__write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL, 0x20002);   // RTC_PCLK 时钟使能：
		__write_hw_reg32(CPR_AOCLKEN_GRCTL, 0x20002);       // RTC_CLK 时钟使能：
	
	
}

