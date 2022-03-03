#ifndef		__BSP_CLK_H_
#define		__BSP_CLK_H_

void xc_uart_clk_init(uint8_t ch,uint32_t baud);
void xc_gpio_clk_init(void);

void xc_spi_clk_init(uint8_t ch);
void xc_i2c_clk_init(void);
void xc_saadc_clk_init(void);
void xc_rtc_clk_init(void);
#endif

