/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef __XINC66_xxxx_CPR_BITS_H
#define __XINC66_xxxx_CPR_BITS_H

/*lint ++flb "Enter library region" */


/* Peripheral: CPR PD */
/* Description: CPR PD Clock control */

/* CPR PD BASE ADDR:0x40000000 */


#define CPR_CLK_SRC_32MHZ_DIV    (0UL) ///< CLK SRC 32MHz div.
#define CPR_CLK_SRC_32KHZ_DIV    (1UL) ///<  CLK SRC 32kHz div.
#define CPR_CLK_SRC_32KHZ    (4UL) ///<  CLK SRC 32kHz.

/* Register: CPR_UART0_CLK_GRCTL */
/* Description :UART0_CLK 时 钟 GR 控制寄存器 对应UART 0 
* Offset      : 0x30
    BIT 16 写屏蔽 BIT[3:0]，BIT 20 写屏蔽 BIT[7:4]，BIT 24 写屏蔽 BIT[11:8]，BIT 28
写屏蔽 BIT[15:12]*/
#define CPR_UART_CLK_GRCTL_MASK_OFFSET (16UL)

/* Bit 0..3: UART0_CLK_GR */
/* Description: uart0_clk 相对于 32MHz 时
钟的一级分频比为:UART0_CLK_GR/8
*/
#define CPR_UART_CLK_GRCTL_UART0_CLK_GR_Pos (0UL) // /*!< Position of UART0_CLK_GR field. */
#define CPR_UART_CLK_GRCTL_UART0_CLK_GR_Msk (0xFUL << CPR_UART_CLK_GRCTL_UART0_CLK_GR_Pos) 
#define CPR_UART_CLK_GRCTL_UART0_CLK_GR_WE  ((1UL << CPR_UART_CLK_GRCTL_UART0_CLK_GR_Pos) << CPR_UART_CLK_GRCTL_MASK_OFFSET)

/* Bit 4: UART0_CLK_GR_UPD */
/* Description: uart0_clk GR 参数更新寄存器
自清 0
1 有效
*/
#define CPR_UART_CLK_GRCTL_UART0_CLK_GR_UPD_Pos (4UL) // /*!< Position of UART0_CLK_GR_UPD field. */
#define CPR_UART_CLK_GRCTL_UART0_CLK_GR_UPD_Msk (0x1UL << CPR_UART_CLK_GRCTL_UART0_CLK_GR_UPD_Pos) 
#define CPR_UART_CLK_GRCTL_UART0_CLK_GR_UPD_Disable (0UL) /*!< 自清 */
#define CPR_UART_CLK_GRCTL_UART0_CLK_GR_UPD_Enable (1UL) /*!< 有效*/

#define CPR_UART_CLK_GRCTL_UART0_CLK_GR_UPD_WE  ((1UL << CPR_UART_CLK_GRCTL_UART0_CLK_GR_UPD_Pos) << CPR_UART_CLK_GRCTL_MASK_OFFSET)



/* Register: CPR_UART0_CLK_CTL */
/* Description :UART0_CLK 时钟控制寄存器 对应UART 0 1*/
/* Offset      : 0x34 */

/* Bit 0..15: UART0_CLK_DIV */
/* Description: uart0_clk 相对于 32MHz 时钟的二级分频系数 DIV
*/
#define CPR_UART_CLK_CTL_UART0_CLK_DIV_Pos (0UL) // /*!< Position of UART0_CLK_DIV field. */
#define CPR_UART_CLK_CTL_UART0_CLK_DIV_Msk (0xFFFFUL << CPR_UART_CLK_CTL_UART0_CLK_DIV_Pos) 

/* Bit 16..31: UART0_CLK_MUL */
/* Description: uart0_clk 相对于 32MHz 时钟的二级分频系数 MUL
*/
#define CPR_UART_CLK_CTL_UART0_CLK_MUL_Pos (16UL) // /*!< Position of UART0_CLK_MUL field. */
#define CPR_UART_CLK_CTL_UART0_CLK_MUL_Msk (0xFFFFUL << CPR_UART_CLK_CTL_UART0_CLK_MUL_Pos) 




/* Register: CPR_UART1_CLK_GRCTL */
/* Description :UART1_CLK 时 钟 GR 控制寄存器 对应UART 1
* Offset      : 0x38
    BIT 16 写屏蔽 BIT[3:0]，BIT 20 写屏蔽 BIT[7:4]，BIT 24 写屏蔽 BIT[11:8]，BIT 28
写屏蔽 BIT[15:12]*/

/* Bit 0..3: UART1_CLK_GR */
/* Description: uart1_clk 相对于 32MHz 时
钟的一级分频比为:UART1_CLK_GR/8
0：不复位
1：复位
*/
#define CPR_UART_CLK_GRCTL_UART1_CLK_GR_Pos (0UL) // /*!< Position of UART1_CLK_GR field. */
#define CPR_UART_CLK_GRCTL_UART1_CLK_GR_Msk (0xFUL << CPR_UART_CLK_GRCTL_UART1_CLK_GR_Pos) 
#define CPR_UART_CLK_GRCTL_UART1_CLK_GR_WE  ((1UL << CPR_UART_CLK_GRCTL_UART1_CLK_GR_Pos) << CPR_UART_CLK_GRCTL_MASK_OFFSET)

/* Bit 4: UART1_CLK_GR_UPD */
/* Description: uart1_clk GR 参数更新寄存器
自清 0
1 有效
*/
#define CPR_UART_CLK_GRCTL_UART1_CLK_GR_UPD_Pos (4UL) // /*!< Position of UART_CLK_GR_UPD field. */
#define CPR_UART_CLK_GRCTL_UART1_CLK_GR_UPD_Msk (0x1UL << CPR_UART_CLK_GRCTL_UART1_CLK_GR_UPD_Pos) 
#define CPR_UART_CLK_GRCTL_UART1_CLK_GR_UPD_Disable (0UL) /*!< 自清 */
#define CPR_UART_CLK_GRCTL_UART1_CLK_GR_UPD_Enable (1UL) /*!< 有效*/

#define CPR_UART_CLK_GRCTL_UART1_CLK_GR_UPD_WE  ((1UL << CPR_UART_CLK_GRCTL_UART1_CLK_GR_UPD_Pos) << CPR_UART_CLK_GRCTL_MASK_OFFSET)



/* Register: CPR_UART1_CLK_CTL */
/* Description :UART0_CLK 时钟控制寄存器 对应UART  1*/
/* Offset      : 0x3c */

/* Bit 0..15: UART1_CLK_DIV */
/* Description: uart1_clk 相对于 32MHz 时钟的二级分频系数 DIV
*/
#define CPR_UART_CLK_CTL_UART1_CLK_DIV_Pos (0UL) // /*!< Position of UART1_CLK_DIV field. */
#define CPR_UART_CLK_CTL_UART1_CLK_DIV_Msk (0xFFFFUL << CPR_UART_CLK_CTL_UART1_CLK_DIV_Pos) 

/* Bit 16..31: UART1_CLK_MUL */
/* Description: uart1_clk 相对于 32MHz 时钟的二级分频系数 MUL
*/
#define CPR_UART_CLK_CTL_UART1_CLK_MUL_Pos (16UL) // /*!< Position of UART1_CLK_MUL field. */
#define CPR_UART_CLK_CTL_UART1_CLK_MUL_Msk (0xFFFFUL << CPR_UART_CLK_CTL_UART1_CLK_MUL_Pos) 






/* Register: CPR_SSI_MCLK_CTL */
/* Offset: 0x50 -0x54 */
/* RegCount: 2 */
/* Description :SSI_MCLK 时钟控制寄存器
    BIT 16 写屏蔽 BIT[3:0]，BIT 20 写屏蔽 BIT[7:4]，BIT 24 写屏蔽 BIT[11:8]，BIT 28
写屏蔽 BIT[15:12]*/
#define CPR_SSI_MCLK_CTL_MASK_OFFSET (16UL)

/* Bit 0..3: SSI_MCLK_DIV */
/* Description: SSI_MCLK 时钟分频系数：
0：不复位
1：复位
*/
#define CPR_SSI_MCLK_CTL_SSI_MCLK_DIV_Pos (0UL) // /*!< Position of SSI_MCLK_DIV field. */
#define CPR_SSI_MCLK_CTL_SSI_MCLK_DIV_Msk (0xFUL << CPR_SSI_MCLK_CTL_SSI_MCLK_DIV_Pos) 
#define CPR_SSI_MCLK_CTL_SSI_MCLK_DIV_WE  ((1UL << CPR_SSI_MCLK_CTL_MASK_OFFSET) << CPR_SSI_MCLK_CTL_SSI_MCLK_DIV_Pos)

/* Bit 4: SSI_MCLK_EN */
/* Description: SSI_MCLK 时钟使能
0：不复位
1：复位
*/
#define CPR_SSI_MCLK_CTL_SSI_MCLK_EN_Pos (4UL) // /*!< Position of SSI_MCLK_EN field. */
#define CPR_SSI_MCLK_CTL_SSI_MCLK_EN_Msk (0x1UL << CPR_SSI_MCLK_CTL_SSI_MCLK_EN_Pos) 
#define CPR_SSI_MCLK_CTL_SSI_MCLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_SSI_MCLK_CTL_SSI_MCLK_EN_Enable (1UL) /*!< 1：使能时钟*/
#define CPR_SSI_MCLK_CTL_SSI_MCLK_EN_WE  ((1UL << CPR_SSI_MCLK_CTL_MASK_OFFSET) << CPR_SSI_MCLK_CTL_SSI_MCLK_EN_Pos)


/*******************************************************
* SSI2_MCLK_DIV & SSI2_MCLK_EN AT Reg Offset 0x54 CPR_SSI1_MCLK_CTL
********************************************************/
/* Bit 8..11: SSI2_MCLK_DIV */
/* Description: SSI2_MCLK 时钟分频系数：
0：不复位
1：复位
*/
#define CPR_SSI_MCLK_CTL_SSI2_MCLK_DIV_Pos (8UL) // /*!< Position of SSI2_MCLK_DIV field. */
#define CPR_SSI_MCLK_CTL_SSI2_MCLK_DIV_Msk (0xFUL << CPR_SSI_MCLK_CTL_SSI2_MCLK_DIV_Pos) 
#define CPR_SSI_MCLK_CTL_SSI2_MCLK_DIV_WE  ((1UL << CPR_SSI_MCLK_CTL_MASK_OFFSET) << CPR_SSI_MCLK_CTL_SSI2_MCLK_DIV_Pos)

/* Bit 12: SSI2_MCLK_EN */
/* Description: SSI2_MCLK 时钟使能
0：不复位
1：复位
*/
#define CPR_SSI_MCLK_CTL_SSI2_MCLK_EN_Pos (12UL) // /*!< Position of SSI_MCLK_EN field. */
#define CPR_SSI_MCLK_CTL_SSI2_MCLK_EN_Msk (0x1UL << CPR_SSI_MCLK_CTL_SSI2_MCLK_EN_Pos) 
#define CPR_SSI_MCLK_CTL_SSI2_MCLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_SSI_MCLK_CTL_SSI2_MCLK_EN_Enable (1UL) /*!< 1：使能时钟*/
#define CPR_SSI_MCLK_CTL_SSI2_MCLK_EN_WE  ((1UL << CPR_SSI_MCLK_CTL_MASK_OFFSET) << CPR_SSI_MCLK_CTL_SSI2_MCLK_EN_Pos)


/* Register: CPR_TIMER_CLK_CTL */
/* Offset: 0x54 -0x64 */
/* Description :TIMER_CLK 时 钟 控 制 寄 存 器
*/

/* Bit 0..7: TIMER_CLK0_DIV */
/* Description: 控制 timer0_clk 相对于 mclk_in(32MHz)时钟的分频比为 2*(TIMER_CLK0_DIV+ 1)
0：不复位
1：复位
*/
#define CPR_TIMER_CLK_CTL_TIMER_CLK0_DIV_Pos (0UL) // /*!< Position of TIMER_CLK0_DIV field. */
#define CPR_TIMER_CLK_CTL_TIMER_CLK0_DIV_Msk (0xFFUL << CPR_TIMER_CLK_CTL_TIMER_CLK0_DIV_Pos) 

/* Bit 8..15: TIMER_CLK1_DIV */
/* Description: 控制 timer0_clk 相对于 32KHz时钟的分频比为 2*(TIMER_CLK1_DIV+ 1)
0：不复位
1：复位
*/
#define CPR_TIMER_CLK_CTL_TIMER_CLK1_DIV_Pos (8UL) // /*!< Position of TIMER_CLK1_DIV field. */
#define CPR_TIMER_CLK_CTL_TIMER_CLK1_DIV_Msk (0xFFUL << CPR_TIMER_CLK_CTL_TIMER_CLK1_DIV_Pos) 


/* Bit 28..30: TIMER_CLKSEL */
/* Description: 控制 timer0_clk 相对于 32KHz时钟的分频比为 2*(TIMER_CLK1_DIV+ 1)
0：不复位
1：复位
*/
#define CPR_TIMER_CLK_CTL_TIMER_CLKSEL_Pos (28UL) // /*!< Position of TIMER_CLKSEL field. */
#define CPR_TIMER_CLK_CTL_TIMER_CLKSEL_Msk (0x7UL << CPR_TIMER_CLK_CTL_TIMER_CLKSEL_Pos) 
#define CPR_TIMER_CLK_CTL_TIMER_CLKSEL_32M_DIV      CPR_CLK_SRC_32MHZ_DIV ///< CLK SRC 32MHz div.
#define CPR_TIMER_CLK_CTL_TIMER_CLKSEL_32K_DIV      CPR_CLK_SRC_32KHZ_DIV   ///<  CLK SRC 32kHz div.
#define CPR_TIMER_CLK_CTL_TIMER_CLKSEL_32K          CPR_CLK_SRC_32KHZ    ///<  CLK SRC 32kHz.






/* Register: CPR_PWM_CLK_CTL */
/* Offset: 0x68 */
/* Description :PWM_CLK 时 钟 控 制 寄 存 器
*/

/* Bit 0..7: PWM_CLK0_DIV */
/* Description: 控制 pwm_clk 相对于 mclk_in(32MHz)时钟的分频比为 2*(PWM_CLK0_DIV+ 1)
0：不复位
1：复位
*/
#define CPR_PWM_CLK_CTL_PWM_CLK0_DIV_Pos (0UL) // /*!< Position of PWM_CLK0_DIV field. */
#define CPR_PWM_CLK_CTL_PWM_CLK0_DIV_Msk (0xFFUL << CPR_PWM_CLK_CTL_PWM_CLK0_DIV_Pos) 

/* Bit 8..15: PWM_CLK1_DIV */
/* Description: 控制 pwm_clk 相对于 32KHz时钟的分频比为 2*(PWM_CLK1_DIV+ 1)
0：不复位
1：复位
*/
#define CPR_PWM_CLK_CTL_PWM_CLK1_DIV_Pos (8UL) // /*!< Position of PWM_CLK1_DIV field. */
#define CPR_PWM_CLK_CTL_PWM_CLK1_DIV_Msk (0xFFUL << CPR_PWM_CLK_CTL_PWM_CLK1_DIV_Pos) 


/* Bit 28..30: PWM_CLKSEL */
/* Description: 控制 timer0_clk 相对于 32KHz时钟的分频比为 2*(TIMER_CLK1_DIV+ 1)
0：不复位
1：复位
*/
#define CPR_PWM_CLK_CTL_PWM_CLKSEL_Pos (28UL) // /*!< Position of TIMER_CLKSEL field. */
#define CPR_PWM_CLK_CTL_PWM_CLKSEL_Msk (0x7UL << CPR_PWM_CLK_CTL_PWM_CLKSEL_Pos) 
#define CPR_PWM_CLK_CTL_PWM_CLKSEL_32M_DIV      CPR_CLK_SRC_32MHZ_DIV ///< CLK SRC 32MHz div.
#define CPR_PWM_CLK_CTL_PWM_CLKSEL_32K_DIV      CPR_CLK_SRC_32KHZ_DIV   ///<  CLK SRC 32kHz div.
#define CPR_PWM_CLK_CTL_PWM_CLKSEL_32K          CPR_CLK_SRC_32KHZ    ///<  CLK SRC 32kHz.

/* Bit 31: PWM_CLK_EN */
/* Description: PWM_CLK 时钟使能
0：不使能时钟
1：使能时钟
*/
#define CPR_PWM_CLK_CTL_PWM_CLK_EN_Pos (31UL) // /*!< Position of PWM_CLK_EN field. */
#define CPR_PWM_CLK_CTL_PWM_CLK_EN_Msk (0x1UL << CPR_PWM_CLK_CTL_PWM_CLK_EN_Pos)
#define CPR_PWM_CLK_CTL_PWM_CLK_EN_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_PWM_CLK_CTL_PWM_CLK_EN_Enable (1UL) /*!< 1：使能时钟*/





/* Register: CPR_AHBCLKEN_GRCTL */
/* Offset: 0x6c */
/* Description:AHB 总线时钟使能寄存器 
    高 16BITS 写屏蔽低 16BITS*/
#define CPR_AHBCLKEN_GRCTL_MASK_OFFSET (16UL)

/* Bit 0: DMAS_HCLK_EN */
/* Description: DMAS_HCLK 时钟使能
0：不使能时钟
1：使能时钟
*/
#define CPR_AHBCLKEN_GRCTL_DMAS_HCLK_EN_Pos (0UL) // /*!< Position of DMAS_HCLK_EN field. */
#define CPR_AHBCLKEN_GRCTL_DMAS_HCLK_EN_Msk (0x1UL << CPR_AHBCLKEN_GRCTL_DMAS_HCLK_EN_Pos) 
#define CPR_AHBCLKEN_GRCTL_DMAS_HCLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_AHBCLKEN_GRCTL_DMAS_HCLK_EN_Enable (1UL) /*!< 1：使能时钟*/

/* Bit 1: SHRAM0_HCLK_EN */
/* Description: SHRAM0_HCLK 时钟使能
0：不使能时钟
1：使能时钟
*/
#define CPR_AHBCLKEN_GRCTL_SHRAM0_HCLK_EN_Pos (1UL) // /*!< Position of SHRAM0_HCLK_EN field. */
#define CPR_AHBCLKEN_GRCTL_SHRAM0_HCLK_EN_Msk (0x1UL << CPR_AHBCLKEN_GRCTL_SHRAM0_HCLK_EN_Pos)
#define CPR_AHBCLKEN_GRCTL_SHRAM0_HCLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_AHBCLKEN_GRCTL_SHRAM0_HCLK_EN_Enable (1UL) /*!< 1：使能时钟*/

/* Bit 2: AES_HCLK_EN */
/* Description: AES_HCLK 时钟使能
0：不使能时钟
1：使能时钟
*/
#define CPR_AHBCLKEN_GRCTL_AES_HCLK_EN_Pos (2UL) // /*!< Position of AES_HCLK_EN field. */
#define CPR_AHBCLKEN_GRCTL_AES_HCLK_EN_Msk (0x1UL << CPR_AHBCLKEN_GRCTL_AES_HCLK_EN_Pos)
#define CPR_AHBCLKEN_GRCTL_AES_HCLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_AHBCLKEN_GRCTL_AES_HCLK_EN_Enable (1UL) /*!< 1：使能时钟*/

/* Bit 3: M0_RAM_HCLK_EN */
/* Description: M0_RAM_HCLK 时钟使能
0：不使能时钟
1：使能时钟
*/
#define CPR_AHBCLKEN_GRCTL_M0_RAM_HCLK_EN_Pos (3UL) // /*!< Position of M0_RAM_HCLK_EN field. */
#define CPR_AHBCLKEN_GRCTL_M0_RAM_HCLK_EN_Msk (0x1UL << CPR_AHBCLKEN_GRCTL_M0_RAM_HCLK_EN_Pos)
#define CPR_AHBCLKEN_GRCTL_M0_RAM_HCLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_AHBCLKEN_GRCTL_M0_RAM_HCLK_EN_Enable (1UL) /*!< 1：使能时钟*/

/* Bit 4: ROM_HCLK_EN */
/* Description: ROM_HCLK 时钟使能
0：不使能时钟
1：使能时钟
*/
#define CPR_AHBCLKEN_GRCTL_ROM_HCLK_EN_Pos (3UL) // /*!< Position of ROM_HCLK_EN field. */
#define CPR_AHBCLKEN_GRCTL_ROM_HCLK_EN_Msk (0x1UL << CPR_AHBCLKEN_GRCTL_ROM_HCLK_EN_Pos)
#define CPR_AHBCLKEN_GRCTL_ROM_HCLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_AHBCLKEN_GRCTL_ROM_HCLK_EN_Enable (1UL) /*!< 1：使能时钟*/




/* Register: CPR_CTLAPBCLKEN_GRCTL */
/* Offset: 0x70 */
/* Description: CTL_APB 总 线 时 钟 使 能 寄 存 器 
    高 16BITS 写屏蔽低 16BITS*/
#define CPR_CTLAPBCLKEN_GRCTL_MASK_OFFSET (16UL)

/* Bit 0: WDT_PCLK_EN */
/* Description: WDT_PCLK 时钟使能：
0：不使能时钟
1：使能时钟
*/
#define CPR_CTLAPBCLKEN_GRCTL_WDT_PCLK_EN_Pos (0UL) // /*!< Position of WDT_PCLK_EN field. */
#define CPR_CTLAPBCLKEN_GRCTL_WDT_PCLK_EN_Msk (0x1UL << CPR_CTLAPBCLKEN_GRCTL_WDT_PCLK_EN_Pos) 
#define CPR_CTLAPBCLKEN_GRCTL_WDT_PCLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_CTLAPBCLKEN_GRCTL_WDT_PCLK_EN_Enable (1UL) /*!< 1：使能时钟*/

/* Bit 1: RTC_PCLK_EN */
/* Description: RTC_PCLK 时钟使能
0：不使能时钟
1：使能时钟
*/
#define CPR_CTLAPBCLKEN_GRCTL_RTC_PCLK_EN_Pos (1UL) // /*!< Position of WDT_PCLK_EN field. */
#define CPR_CTLAPBCLKEN_GRCTL_RTC_PCLK_EN_Msk (0x1UL << CPR_CTLAPBCLKEN_GRCTL_RTC_PCLK_EN_Pos) 
#define CPR_CTLAPBCLKEN_GRCTL_RTC_PCLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_CTLAPBCLKEN_GRCTL_RTC_PCLK_EN_Enable (1UL) /*!< 1：使能时钟*/

/* Bit 2: GPIO_PCLK_EN */
/* Description: GPIO_PCLK 时钟使能：
0：不使能时钟
1：使能时钟
*/
#define CPR_CTLAPBCLKEN_GRCTL_GPIO_PCLK_EN_Pos (2UL) // /*!< Position of GPIO_PCLK_EN field. */
#define CPR_CTLAPBCLKEN_GRCTL_GPIO_PCLK_EN_Msk (0x1UL << CPR_CTLAPBCLKEN_GRCTL_GPIO_PCLK_EN_Pos) 
#define CPR_CTLAPBCLKEN_GRCTL_GPIO_PCLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_CTLAPBCLKEN_GRCTL_GPIO_PCLK_EN_Enable (1UL) /*!< 1：使能时钟*/

/* Bit 3: TIMER_PCLK_EN */
/* Description: TIMER_PCLK 时钟使能：
0：不使能时钟
1：使能时钟
*/
#define CPR_CTLAPBCLKEN_GRCTL_TIMER_PCLK_EN_Pos (3UL) // /*!< Position of TIMER_PCLK_EN field. */
#define CPR_CTLAPBCLKEN_GRCTL_TIMER_PCLK_EN_Msk (0x1UL << CPR_CTLAPBCLKEN_GRCTL_TIMER_PCLK_EN_Pos) 
#define CPR_CTLAPBCLKEN_GRCTL_TIMER_PCLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_CTLAPBCLKEN_GRCTL_TIMER_PCLK_EN_Enable (1UL) /*!< 1：使能时钟*/

/* Bit 4: UART0_PCLK_EN */
/* Description: UART0_PCLK 时钟使能：
0：不使能时钟
1：使能时钟
*/
#define CPR_CTLAPBCLKEN_GRCTL_UART0_PCLK_EN_Pos (4UL) // /*!< Position of UART0_PCLK_EN field. */
#define CPR_CTLAPBCLKEN_GRCTL_UART0_PCLK_EN_Msk (0x1UL << CPR_CTLAPBCLKEN_GRCTL_UART0_PCLK_EN_Pos) 
#define CPR_CTLAPBCLKEN_GRCTL_UART0_PCLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_CTLAPBCLKEN_GRCTL_UART0_PCLK_EN_Enable (1UL) /*!< 1：使能时钟*/

/* Bit 5: UART1_PCLK_EN */
/* Description: UART1_PCLK 时钟使能：
0：不使能时钟
1：使能时钟
*/
#define CPR_CTLAPBCLKEN_GRCTL_UART1_PCLK_EN_Pos (5UL) // /*!< Position of UART1_PCLK_EN field. */
#define CPR_CTLAPBCLKEN_GRCTL_UART1_PCLK_EN_Msk (0x1UL << CPR_CTLAPBCLKEN_GRCTL_UART1_PCLK_EN_Pos) 
#define CPR_CTLAPBCLKEN_GRCTL_UART1_PCLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_CTLAPBCLKEN_GRCTL_UART1_PCLK_EN_Enable (1UL) /*!< 1：使能时钟*/

/* Bit 6: QDEC_PCLK_EN */
/* Description: QDEC_PCLK 时钟使能：
0：不使能时钟
1：使能时钟
*/
#define CPR_CTLAPBCLKEN_GRCTL_QDEC_PCLK_EN_Pos (6UL) // /*!< Position of QDEC_PCLK_EN field. */
#define CPR_CTLAPBCLKEN_GRCTL_QDEC_PCLK_EN_Msk (0x1UL << CPR_CTLAPBCLKEN_GRCTL_QDEC_PCLK_EN_Pos) 
#define CPR_CTLAPBCLKEN_GRCTL_QDEC_PCLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_CTLAPBCLKEN_GRCTL_QDEC_PCLK_EN_Enable (1UL) /*!< 1：使能时钟*/

/* Bit 7: KBS_PCLK_EN */
/* Description: KBS_PCLK 时钟使能：
0：不使能时钟
1：使能时钟
*/
#define CPR_CTLAPBCLKEN_GRCTL_KBS_PCLK_EN_Pos (7UL) // /*!< Position of KBS_PCLK_EN field. */
#define CPR_CTLAPBCLKEN_GRCTL_KBS_PCLK_EN_Msk (0x1UL << CPR_CTLAPBCLKEN_GRCTL_KBS_PCLK_EN_Pos) 
#define CPR_CTLAPBCLKEN_GRCTL_KBS_PCLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_CTLAPBCLKEN_GRCTL_KBS_PCLK_EN_Enable (1UL) /*!< 1：使能时钟*/

/* Bit 8: SSI0_PCLK_EN */
/* Description: SSI0_PCLK 时钟使能：
0：不使能时钟
1：使能时钟
*/
#define CPR_CTLAPBCLKEN_GRCTL_SSI0_PCLK_EN_Pos (8UL) // /*!< Position of SSI0_PCLK_EN field. */
#define CPR_CTLAPBCLKEN_GRCTL_SSI0_PCLK_EN_Msk (0x1UL << CPR_CTLAPBCLKEN_GRCTL_SSI0_PCLK_EN_Pos) 
#define CPR_CTLAPBCLKEN_GRCTL_SSI0_PCLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_CTLAPBCLKEN_GRCTL_SSI0_PCLK_EN_Enable (1UL) /*!< 1：使能时钟*/

/* Bit 9: SSI1_PCLK_EN */
/* Description: SSI1_PCLK 时钟使能：
0：不使能时钟
1：使能时钟
*/
#define CPR_CTLAPBCLKEN_GRCTL_SSI1_PCLK_EN_Pos (9UL) // /*!< Position of SSI0_PCLK_EN field. */
#define CPR_CTLAPBCLKEN_GRCTL_SSI1_PCLK_EN_Msk (0x1UL << CPR_CTLAPBCLKEN_GRCTL_SSI1_PCLK_EN_Pos) 
#define CPR_CTLAPBCLKEN_GRCTL_SSI1_PCLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_CTLAPBCLKEN_GRCTL_SSI1_PCLK_EN_Enable (1UL) /*!< 1：使能时钟*/

/* Bit 10: BT_PCLK_EN */
/* Description: BT_PCLK 时钟使能：
0：不使能时钟
1：使能时钟
*/
#define CPR_CTLAPBCLKEN_GRCTL_BT_PCLK_EN_Pos (10UL) // /*!< Position of BT_PCLK_EN field. */
#define CPR_CTLAPBCLKEN_GRCTL_BT_PCLK_EN_Msk (0x1UL << CPR_CTLAPBCLKEN_GRCTL_BT_PCLK_EN_Pos) 
#define CPR_CTLAPBCLKEN_GRCTL_BT_PCLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_CTLAPBCLKEN_GRCTL_BT_PCLK_EN_Enable (1UL) /*!< 1：使能时钟*/

/* Bit 11: I2C_PCLK_EN */
/* Description: I2C_PCLK 时钟使能：
0：不使能时钟
1：使能时钟
*/
#define CPR_CTLAPBCLKEN_GRCTL_I2C_PCLK_EN_Pos (11UL) // /*!< Position of I2C_PCLK_EN field. */
#define CPR_CTLAPBCLKEN_GRCTL_I2C_PCLK_EN_Msk (0x1UL << CPR_CTLAPBCLKEN_GRCTL_I2C_PCLK_EN_Pos) 
#define CPR_CTLAPBCLKEN_GRCTL_I2C_PCLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_CTLAPBCLKEN_GRCTL_I2C_PCLK_EN_Enable (1UL) /*!< 1：使能时钟*/

/* Bit 12: PWM_PCLK_EN */
/* Description: PWM_PCLK 时钟使能：
0：不使能时钟
1：使能时钟
*/
#define CPR_CTLAPBCLKEN_GRCTL_PWM_PCLK_EN_Pos (12UL) // /*!< Position of PWM_PCLK_EN field. */
#define CPR_CTLAPBCLKEN_GRCTL_PWM_PCLK_EN_Msk (0x1UL << CPR_CTLAPBCLKEN_GRCTL_PWM_PCLK_EN_Pos) 
#define CPR_CTLAPBCLKEN_GRCTL_PWM_PCLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_CTLAPBCLKEN_GRCTL_PWM_PCLK_EN_Enable (1UL) /*!< 1：使能时钟*/

/* Bit 13: GPADC_PCLK_EN */
/* Description: GPADC_PCLK 时钟使能：
0：不使能时钟
1：使能时钟
*/
#define CPR_CTLAPBCLKEN_GRCTL_GPADC_PCLK_EN_Pos (13UL) // /*!< Position of GPADC_PCLK_EN field. */
#define CPR_CTLAPBCLKEN_GRCTL_GPADC_PCLK_EN_Msk (0x1UL << CPR_CTLAPBCLKEN_GRCTL_GPADC_PCLK_EN_Pos) 
#define CPR_CTLAPBCLKEN_GRCTL_GPADC_PCLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_CTLAPBCLKEN_GRCTL_GPADC_PCLK_EN_Enable (1UL) /*!< 1：使能时钟*/

/* Bit 14: SIM_PCLK_EN */
/* Description: SIM_PCLK 时钟使能：
0：不使能时钟
1：使能时钟
*/
#define CPR_CTLAPBCLKEN_GRCTL_SIM_PCLK_EN_Pos (14UL) // /*!< Position of SIM_PCLK_EN field. */
#define CPR_CTLAPBCLKEN_GRCTL_SIM_PCLK_EN_Msk (0x1UL << CPR_CTLAPBCLKEN_GRCTL_SIM_PCLK_EN_Pos) 
#define CPR_CTLAPBCLKEN_GRCTL_SIM_PCLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_CTLAPBCLKEN_GRCTL_SIM_PCLK_EN_Enable (1UL) /*!< 1：使能时钟*/

/* Bit 15: SSI2_PCLK_EN */
/* Description: SSI2_PCLK 时钟使能：
0：不使能时钟
1：使能时钟
*/
#define CPR_CTLAPBCLKEN_GRCTL_SSI2_PCLK_EN_Pos (15UL) // /*!< Position of SSI2_PCLK_EN field. */
#define CPR_CTLAPBCLKEN_GRCTL_SSI2_PCLK_EN_Msk (0x1UL << CPR_CTLAPBCLKEN_GRCTL_SSI2_PCLK_EN_Pos) 
#define CPR_CTLAPBCLKEN_GRCTL_SSI2_PCLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_CTLAPBCLKEN_GRCTL_SSI2_PCLK_EN_Enable (1UL) /*!< 1：使能时钟*/


/* Register: CPR_OTHERCLKEN_GRCTL */
/* Offset: 0x74 */
/* Description :CTL_APB 总 线 时 钟 使 能 寄 存 器 
    高 16BITS 写屏蔽低 16BITS*/
#define CPR_OTHERCLKEN_GRCTL_MASK_OFFSET (16UL)

/* Bit 0: GPIO_CLK_EN */
/* Description: GPIO_CLK 时钟使能：
0：不使能时钟
1：使能时钟
*/
#define CPR_OTHERCLKEN_GRCTL_GPIO_CLK_EN_Pos (0UL) // /*!< Position of GPIO_CLK_EN field. */
#define CPR_OTHERCLKEN_GRCTL_GPIO_CLK_EN_Msk (0x1UL << CPR_OTHERCLKEN_GRCTL_GPIO_CLK_EN_Pos) 
#define CPR_OTHERCLKEN_GRCTL_GPIO_CLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_OTHERCLKEN_GRCTL_GPIO_CLK_EN_Enable (1UL) /*!< 1：使能时钟*/


/* Bit 2: BT32K_CLK_EN */
/* Description: BT32K_CLK 时钟使能：
0：不使能时钟
1：使能时钟
*/
#define CPR_OTHERCLKEN_GRCTL_BT32K_CLK_EN_Pos (2UL) // /*!< Position of BT32K_CLK_EN field. */
#define CPR_OTHERCLKEN_GRCTL_BT32K_CLK_EN_Msk (0x1UL << CPR_OTHERCLKEN_GRCTL_BT32K_CLK_EN_Pos) 
#define CPR_OTHERCLKEN_GRCTL_BT32K_CLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_OTHERCLKEN_GRCTL_BT32K_CLK_EN_Enable (1UL) /*!< 1：使能时钟*/

/* Bit 3: KBS_CLK_EN */
/* Description: KBS_CLK 时钟使能：
0：不使能时钟
1：使能时钟
*/
#define CPR_OTHERCLKEN_GRCTL_KBS_CLK_EN_Pos (3UL) // /*!< Position of KBS_CLK_EN field. */
#define CPR_OTHERCLKEN_GRCTL_KBS_CLK_EN_Msk (0x1UL << CPR_OTHERCLKEN_GRCTL_KBS_CLK_EN_Pos) 
#define CPR_OTHERCLKEN_GRCTL_KBS_CLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_OTHERCLKEN_GRCTL_KBS_CLK_EN_Enable (1UL) /*!< 1：使能时钟*/

/* Register: CPR_I2C_CLK_CTL */
/* Offset: 0x78 */
/* Description :I2C_CLK 时钟控制寄存器
 * Offset : 0x     
    BIT 16 写屏蔽 BIT[3:0]，BIT 20 写屏蔽 BIT[7:4]，BIT 24 写屏蔽 BIT[11:8]，BIT 28
写屏蔽 BIT[15:12]*/
#define CPR_I2C_CLK_CTL_MASK_OFFSET (16UL)

/* Bit 0..3: I2C_CLK_DIV */
/* Description: I2C_CLK 时钟分频系数：
0：不复位
1：复位
*/
#define CPR_I2C_CLK_CTL_I2C_CLK_DIV_Pos (0UL) // /*!< Position of I2C_CLK_DIV field. */
#define CPR_I2C_CLK_CTL_I2C_CLK_DIV_Msk (0xFUL << CPR_I2C_CLK_CTL_I2C_CLK_DIV_Pos) 


/* Bit 4: I2C_CLK_EN */
/* Description: I2C_CLK 时钟分频系数：
0：不复位
1：复位
*/
#define CPR_I2C_CLK_CTL_I2C_CLK_EN_Pos (4UL) // /*!< Position of I2C_CLK_EN field. */
#define CPR_I2C_CLK_CTL_I2C_CLK_EN_Msk (0xFUL << CPR_I2C_CLK_CTL_I2C_CLK_EN_Pos) 
#define CPR_I2C_CLK_CTL_I2C_CLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_I2C_CLK_CTL_I2C_CLK_EN_Enable (1UL) /*!< 1：使能时钟*/





/* Register: CPR_RSTCTL_CTLAPB_SW */
/* Offset: 0x100 */
/* Description :CTLAPB 模 块 软 复 位 寄 存 器
    高 16BITS 写屏蔽低 16BITS*/
#define CPR_RSTCTL_CTLAPB_SW_MASK_OFFSET (16UL)



/* Bit 1: GPIO_RSTN */
/* Description: GPIO 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_CTLAPB_SW_GPIO_RSTN_Pos (1UL) // /*!< Position of GPIO_RSTN field. */
#define CPR_RSTCTL_CTLAPB_SW_GPIO_RSTN_Msk (0x1UL << CPR_RSTCTL_CTLAPB_SW_GPIO_RSTN_Pos) 
#define CPR_RSTCTL_CTLAPB_SW_GPIO_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_CTLAPB_SW_GPIO_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/

/* Bit 2: WDT_RSTN */
/* Description: WDT 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_CTLAPB_SW_WDT_RSTN_Pos (2UL) // /*!< Position of WDT_RSTN field. */
#define CPR_RSTCTL_CTLAPB_SW_WDT_RSTN_Msk (0x1UL << CPR_RSTCTL_CTLAPB_SW_WDT_RSTN_Pos) 
#define CPR_RSTCTL_CTLAPB_SW_WDT_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_CTLAPB_SW_WDT_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/

/* Bit 3: TIMER0_RSTN */
/* Description: TIMER0 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_CTLAPB_SW_TIMER0_RSTN_Pos (3UL) // /*!< Position of TIMER0_RSTN field. */
#define CPR_RSTCTL_CTLAPB_SW_TIMER0_RSTN_Msk (0x1UL << CPR_RSTCTL_CTLAPB_SW_TIMER0_RSTN_Pos) 
#define CPR_RSTCTL_CTLAPB_SW_TIMER0_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_CTLAPB_SW_TIMER0_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/

/* Bit 4: TIMER1_RSTN */
/* Description: TIMER1 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_CTLAPB_SW_TIMER1_RSTN_Pos (4UL) // /*!< Position of TIMER1_RSTN field. */
#define CPR_RSTCTL_CTLAPB_SW_TIMER1_RSTN_Msk (0x1UL << CPR_RSTCTL_CTLAPB_SW_TIMER1_RSTN_Pos) 
#define CPR_RSTCTL_CTLAPB_SW_TIMER1_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_CTLAPB_SW_TIMER1_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/

/* Bit 5: TIMER2_RSTN */
/* Description: TIMER2 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_CTLAPB_SW_TIMER2_RSTN_Pos (5UL) // /*!< Position of TIMER2_RSTN field. */
#define CPR_RSTCTL_CTLAPB_SW_TIMER2_RSTN_Msk (0x1UL << CPR_RSTCTL_CTLAPB_SW_TIMER2_RSTN_Pos) 
#define CPR_RSTCTL_CTLAPB_SW_TIMER2_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_CTLAPB_SW_TIMER2_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/

/* Bit 6: TIMER3_RSTN */
/* Description: TIMER3 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_CTLAPB_SW_TIMER3_RSTN_Pos (6UL) // /*!< Position of TIMER3_RSTN field. */
#define CPR_RSTCTL_CTLAPB_SW_TIMER3_RSTN_Msk (0x1UL << CPR_RSTCTL_CTLAPB_SW_TIMER3_RSTN_Pos) 
#define CPR_RSTCTL_CTLAPB_SW_TIMER3_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_CTLAPB_SW_TIMER3_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/


/* Bit 7: TIMER_PRESETN */
/* Description: TIMER 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_CTLAPB_SW_TIMER_PRESETN_Pos (7UL) // /*!< Position of TIMER_RSTN field. */
#define CPR_RSTCTL_CTLAPB_SW_TIMER_PRESETN_Msk (0x1UL << CPR_RSTCTL_CTLAPB_SW_TIMER_RSTN_Pos) 
#define CPR_RSTCTL_CTLAPB_SW_TIMER_PRESETN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_CTLAPB_SW_TIMER_PRESETN_Enable (RSTN_VALID) /*!< 0：复位*/

/* Bit 8: CTL_APB_RSTN */
/* Description: CTL_APB 桥软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_CTLAPB_SW_CTL_APB_RSTN_Pos (8UL) // /*!< Position of CTL_APB_RSTN field. */
#define CPR_RSTCTL_CTLAPB_SW_CTL_APB_RSTN_Msk (0x1UL << CPR_RSTCTL_CTLAPB_SW_CTL_APB_RSTN_Pos) 
#define CPR_RSTCTL_CTLAPB_SW_CTL_APB_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_CTLAPB_SW_CTL_APB_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/


/* Bit 9: KBS_RSTN */
/* Description: KBS 模块软复位
1：不复位
0：复位
*/
#define CPR_RSTCTL_CTLAPB_SW_KBS_RSTN_Pos (9UL) // /*!< Position of KBS_RSTN field. */
#define CPR_RSTCTL_CTLAPB_SW_KBS_RSTN_Msk (0x1UL << CPR_RSTCTL_CTLAPB_SW_KBS_RSTN_Pos) 
#define CPR_RSTCTL_CTLAPB_SW_KBS_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_CTLAPB_SW_KBS_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/

/* Bit 10: QDEC_RSTN */
/* Description: QDEC 模块软复位
1：不复位
0：复位
*/
#define CPR_RSTCTL_CTLAPB_SW_QDEC_RSTN_Pos (10UL) // /*!< Position of QDEC_RSTN field. */
#define CPR_RSTCTL_CTLAPB_SW_QDEC_RSTN_Msk (0x1UL << CPR_RSTCTL_CTLAPB_SW_QDEC_RSTN_Pos) 
#define CPR_RSTCTL_CTLAPB_SW_QDEC_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_CTLAPB_SW_QDEC_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/

/* Bit 11: PWM_RSTN */
/* Description: PWM 模块软复位
1：不复位
0：复位
*/
#define CPR_RSTCTL_CTLAPB_SW_PWM_RSTN_Pos (11UL) // /*!< Position of PWM_RSTN field. */
#define CPR_RSTCTL_CTLAPB_SW_PWM_RSTN_Msk (0x1UL << CPR_RSTCTL_CTLAPB_SW_PWM_RSTN_Pos) 
#define CPR_RSTCTL_CTLAPB_SW_PWM_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_CTLAPB_SW_PWM_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/

/* Bit 12: GPADC_RSTN */
/* Description: GPADC 模块软复位
1：不复位
0：复位
*/
#define CPR_RSTCTL_CTLAPB_SW_GPADC_RSTN_Pos (12UL) // /*!< Position of GPADC_RSTN field. */
#define CPR_RSTCTL_CTLAPB_SW_GPADC_RSTN_Msk (0x1UL << CPR_RSTCTL_CTLAPB_SW_GPADC_RSTN_Pos) 
#define CPR_RSTCTL_CTLAPB_SW_GPADC_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_CTLAPB_SW_GPADC_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/


/* Register: CPR_RSTCTL_SUBRST_SW */
/* Offset: 0x104 */
/* Description :SUB 模块软复位寄存器
    高 16BITS 写屏蔽低 16BITS*/
#define CPR_RSTCTL_SUBRST_SW_MASK_OFFSET (16UL)

/* Bit 0: UART0_RSTN */
/* Description: UART0 模块软复位：
1：不复位
1：复位
*/
#define CPR_RSTCTL_SUBRST_SW_UART0_RSTN_Pos (0UL) // /*!< Position of UART0_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_UART0_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_UART0_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_UART0_RSTN_Disable (RSTN_INVALID) /*!< 0：不复位 */
#define CPR_RSTCTL_SUBRST_SW_UART0_RSTN_Enable (RSTN_VALID) /*!< 1：复位*/

/* Bit 1: UART1_RSTN */
/* Description: UART1 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_UART1_RSTN_Pos (1UL) // /*!< Position of UART1_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_UART1_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_UART1_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_UART1_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_UART1_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/


/* Bit 2: SSI0_RSTN */
/* Description: SS0 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_SSI0_RSTN_Pos (2UL) // /*!< Position of SSI0_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_SSI0_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_SSI0_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_SSI0_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_SSI0_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/

/* Bit 3: SSI1_RSTN */
/* Description: SSI1 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_SSI1_RSTN_Pos (3UL) // /*!< Position of SSI1_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_SSI1_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_SSI1_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_SSI1_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_SSI1_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/

/* Bit 4: DMAS_RSTN */
/* Description: DMAS 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_DMAS_RSTN_Pos (4UL) // /*!< Position of DMAS_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_DMAS_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_DMAS_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_DMAS_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_DMAS_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/

/* Bit 5: BT_RSTN */
/* Description: BT 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_BT_RSTN_Pos (5UL) // /*!< Position of BT_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_BT_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_BT_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_BT_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_BT_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/

/* Bit 6: I2C_RSTN */
/* Description: I2C 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_I2C_RSTN_Pos (6UL) // /*!< Position of I2C_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_I2C_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_I2C_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_I2C_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_I2C_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/

/* Bit 7: SHRAM0_RSTN */
/* Description: SHRAM0 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_SHRAM0_RSTN_Pos (7UL) // /*!< Position of SHRAM0_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_SHRAM0_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_SHRAM0_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_SHRAM0_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_SHRAM0_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/

/* Bit 8: AES_RSTN */
/* Description: AES 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_AES_RSTN_Pos (8UL) // /*!< Position of AES_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_AES_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_AES_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_AES_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_AES_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/

/* Bit 9: SIM_RSTN */
/* Description: SIM 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_SIM_RSTN_Pos (9UL) // /*!< Position of SIM_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_SIM_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_SIM_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_SIM_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_SIM_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/


/* Bit 10: SSI2_RSTN */
/* Description: SSI2 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_SSI2_RSTN_Pos (10UL) // /*!< Position of SSI2_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_SSI2_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_SSI2_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_SSI2_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_SSI2_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/

/* Bit 11: UART2_RSTN */
/* Description: UART2 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_UART2_RSTN_Pos (11UL) // /*!< Position of UART2_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_UART2_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_UART2_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_UART2_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_UART2_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/

/* Bit 12: I2S_RSTN */
/* Description: I2S 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_I2S_RSTN_Pos (12UL) // /*!< Position of I2S_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_I2S_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_I2S_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_I2S_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_I2S_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/


/* Bit 13: CAN_RSTN */
/* Description: CAN 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_CAN_RSTN_Pos (13UL) // /*!< Position of CAN_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_CAN_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_CAN_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_CAN_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_CAN_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/




/* Register: CPR_RSTCTL_WDTRST_MASK */
/* Offset: 0x114 */
/* Description :CTLAPB 模 块 软 复 位 寄 存 器
*/

/* Bit 0: WDT_SYS_RSTN_MASK */
/* Description: 屏蔽 WDT 引起的系统的
0：不屏蔽
1：屏蔽
*/
#define CPR_RSTCTL_WDTRST_MASK_WDT_SYS_RSTN_MASK_Pos (0UL) // /*!< Position of WDT_SYS_RSTN_MASK field. */
#define CPR_RSTCTL_WDTRST_MASK_WDT_SYS_RSTN_MASK_Msk (0x1UL << CPR_RSTCTL_WDTRST_MASK_WDT_SYS_RSTN_MASK_Pos) 
#define CPR_RSTCTL_WDTRST_MASK_WDT_SYS_RSTN_MASK_Disable (0UL) /*!< 0：不屏蔽 */
#define CPR_RSTCTL_WDTRST_MASK_WDT_SYS_RSTN_MASK_Enable (1UL) /*!< 1：屏蔽*/

/* Bit 1: WDT_M0_RSTN_MASK */
/* Description: 屏蔽 WDT 引起的系统的
0：不屏蔽
1：屏蔽
*/
#define CPR_RSTCTL_WDTRST_MASK_WDT_M0_RSTN_MASK_Pos (1UL) // /*!< Position of WDT_M0_RSTN_MASK field. */
#define CPR_RSTCTL_WDTRST_MASK_WDT_M0_RSTN_MASK_Msk (0x1UL << CPR_RSTCTL_WDTRST_MASK_WDT_M0_RSTN_MASK_Pos) 
#define CPR_RSTCTL_WDTRST_MASK_WDT_M0_RSTN_MASK_Disable (0UL) /*!< 0：不屏蔽 */
#define CPR_RSTCTL_WDTRST_MASK_WDT_M0_RSTN_MASK_Enable (1UL) /*!< 1：屏蔽*/



/* Register: CPR_LP_CTL */
/* Offset: 0x118 */
/* Description :低功耗使能寄存器
*/

/* Bit 0: UART1_CLK_OFF_PROTECT_EN */
/* Description: UART1 时钟保护功能使能
0：不使能
1：使能
*/
#define CPR_LP_CTL_UART1_CLK_OFF_PROTECT_EN_Pos (0UL) // /*!< Position of UART1_CLK_OFF_PROTECT_EN field. */
#define CPR_LP_CTL_UART1_CLK_OFF_PROTECT_EN_Msk (0x1UL << CPR_LP_CTL_UART1_CLK_OFF_PROTECT_EN_Pos) 
#define CPR_LP_CTL_UART1_CLK_OFF_PROTECT_EN_Disable (0UL) /*!< 0：不使能 */
#define CPR_LP_CTL_UART1_CLK_OFF_PROTECT_EN_Enable (1UL) /*!< 1：使能*/



/* Bit 1: UART0_CLK_OFF_PROTECT_EN */
/* Description: UART0 时钟保护功能使能
0：不使能
1：使能
*/
#define CPR_LP_CTL_UART0_CLK_OFF_PROTECT_EN_Pos (1UL) // /*!< Position of UART0_CLK_OFF_PROTECT_EN field. */
#define CPR_LP_CTL_UART0_CLK_OFF_PROTECT_EN_Msk (0x1UL << CPR_LP_CTL_UART0_CLK_OFF_PROTECT_EN_Pos) 
#define CPR_LP_CTL_UART0_CLK_OFF_PROTECT_EN_Disable (0UL) /*!< 0：不使能 */
#define CPR_LP_CTL_UART0_CLK_OFF_PROTECT_EN_Enable (1UL) /*!< 1：使能*/



/* Register: CPR_SSI_CTRL */
/* Offse: 0x160 */
/* Description :SSI 控制寄存器
*/

/* Bit 0: SSI0_PROTOCOL */
/* Description: SSI0 支持协议选择
0：SSP
1：SPI
*/
#define CPR_SSI_CTRL_SSI0_PROTOCOL_Pos (0UL) // /*!< Position of SSI0_PROTOCOL field. */
#define CPR_SSI_CTRL_SSI0_PROTOCOL_Msk (0x1UL << CPR_SSI_CTRL_SSI0_PROTOCOL_Pos) 
#define CPR_SSI_CTRL_SSI0_PROTOCOL_SSP (0UL) /*!< 0：SSP */
#define CPR_SSI_CTRL_SSI0_PROTOCOL_SPI (1UL) /*!< 1：SPI*/

/* Bit 1: SSI2_PROTOCOL */
/* Description: SSI2 支持协议选择
0：SSP
1：SPI
*/
#define CPR_SSI_CTRL_SSI2_PROTOCOL_Pos (1UL) // /*!< Position of SSI2_PROTOCOL field. */
#define CPR_SSI_CTRL_SSI2_PROTOCOL_Msk (0x1UL << CPR_SSI_CTRL_SSI02_PROTOCOL_Pos) 
#define CPR_SSI_CTRL_SSI2_PROTOCOL_SSP (0UL) /*!< 0：SSP */
#define CPR_SSI_CTRL_SSI2_PROTOCOL_SPI (1UL) /*!< 1：SPI*/


/* Bit 4: SSI1_PROTOCOL */
/* Description: SSI1 支持协议选择
0：SSP
1：SPI
*/
#define CPR_SSI_CTRL_SSI1_PROTOCOL_Pos (4UL) // /*!< Position of SSI1_PROTOCOL field. */
#define CPR_SSI_CTRL_SSI1_PROTOCOL_Msk (0x1UL << CPR_SSI_CTRL_SSI1_PROTOCOL_Pos) 
#define CPR_SSI_CTRL_SSI1_PROTOCOL_SSP (0UL) /*!< 0：0：SSP */
#define CPR_SSI_CTRL_SSI1_PROTOCOL_SPI (1UL) /*!< 1：SPI*/

/* Bit 5: SSI1_MASTER_EN */
/* Description: SS1 的主从配置：
0：Slave
1：Master
*/
#define CPR_SSI_CTRL_SSI1_MASTER_EN_Pos (5UL) // /*!< Position of SSI1_MASTER_EN field. */
#define CPR_SSI_CTRL_SSI1_MASTER_EN_Msk (0x1UL << CPR_SSI_CTRL_SSI1_MASTER_EN_Pos) 
#define CPR_SSI_CTRL_SSI1_MASTER_EN_Slave (0UL) /*!< 0：Slave */
#define CPR_SSI_CTRL_SSI1_MASTER_EN_Master (1UL) /*!< 1：Master*/


/* Register: CPR_M4_NEW_REG1 */
/* Offse: 0x1B4 */
/* Description :USB AUDIO 2.4G 相关控制寄存器
*/

/* Bit 0..3: USB_MCLK_DIV */
/* Description: usb mclk分频比
分频值=寄存器值+1
*/
#define CPR_OTHER_CLK_USB_MCLK_DIV_Pos (0UL) // /*!< Position of USB_MCLK_DIV field. */
#define CPR_OTHER_CLK_USB_MCLK_DIV_Msk (0xFUL << CPR_OTHER_CLK_USB_MCLK_DIV_Pos) 

/* Bit 4..7: RF24G_MCLK_DIV */
/* Description: RF24G mclk分频比
分频值=寄存器值+1
*/
#define CPR_OTHER_CLK_RF24G_MCLK_DIV_Pos (4UL) // /*!< Position of RF24G_MCLK_DIV field. */
#define CPR_OTHER_CLK_RF24G_MCLK_DIV_Msk (0xFUL << CPR_OTHER_CLK_RF24G_MCLK_DIV_Pos) 

/* Bit 8..11: CDC_MCLK_GR */
/* Description: audio ADC mclk一级分频
*/
#define CPR_OTHER_CLK_CDC_MCLK_GR_Pos (8UL) // /*!< Position of CDC_MCLK_GR field. */
#define CPR_OTHER_CLK_CDC_MCLK_GR_Msk (0xFUL << CPR_OTHER_CLK_CDC_MCLK_GR_Pos) 

/* Bit 12: USB_HCLK_EN */
/* Description: usb hclk使能
 * 0：Disable
 * 1：Enable
*/
#define CPR_OTHER_CLK_USB_HCLK_EN_Pos (12UL) // /*!< Position of USB_HCLK_EN field. */
#define CPR_OTHER_CLK_USB_HCLK_EN_Msk (0x1UL << CPR_OTHER_CLK_USB_HCLK_EN_Pos) 
#define CPR_OTHER_CLK_USB_HCLK_EN_Disable (0UL) /*!< 0：不使能 */
#define CPR_OTHER_CLK_USB_HCLK_EN_Enable (1UL) /*!< 1：使能*/

/* Bit 13: USB_MCLK_EN */
/* Description: usb mclk使能
 * 0：Disable
 * 1：Enable
*/
#define CPR_OTHER_CLK_USB_MCLK_EN_Pos (13UL) // /*!< Position of USB_MCLK_EN field. */
#define CPR_OTHER_CLK_USB_MCLK_EN_Msk (0x1UL << CPR_OTHER_CLK_USB_MCLK_EN_Pos) 
#define CPR_OTHER_CLK_USB_MCLK_EN_Disable (0UL) /*!< 0：不使能 */
#define CPR_OTHER_CLK_USB_MCLK_EN_Enable (1UL) /*!< 1：使能*/

/* Bit 14: RF24G_HCLK_EN */
/* Description: RF24G hclk使能
 * 0：Disable
 * 1：Enable
*/
#define CPR_OTHER_CLK_RF24G_HCLK_EN_Pos (14UL) // /*!< Position of RF24G_HCLK_EN field. */
#define CPR_OTHER_CLK_RF24G_HCLK_EN_Msk (0x1UL << CPR_OTHER_CLK_RF24G_HCLK_EN_Pos) 
#define CPR_OTHER_CLK_RF24G_HCLK_EN_Disable (0UL) /*!< 0：不使能 */
#define CPR_OTHER_CLK_RF24G_HCLK_EN_Enable (1UL) /*!< 1：使能*/

/* Bit 15: RF24G_MCLK_EN */
/* Description: RF24G mclk使能
 * 0：Disable
 * 1：Enable
*/
#define CPR_OTHER_CLK_RF24G_MCLK_EN_Pos (15UL) // /*!< Position of RF24G_MCLK_EN field. */
#define CPR_OTHER_CLK_RF24G_MCLK_EN_Msk (0x1UL << CPR_OTHER_CLK_RF24G_MCLK_EN_Pos) 
#define CPR_OTHER_CLK_RF24G_MCLK_EN_Disable (0UL) /*!< 0：不使能 */
#define CPR_OTHER_CLK_RF24G_MCLK_EN_Enable (1UL) /*!< 1：使能*/

/* Bit 16: CDC_HCLK_EN */
/* Description: CDC hclk使能
 * 0：Disable
 * 1：Enable
*/
#define CPR_OTHER_CLK_CDC_HCLK_EN_Pos (16UL) // /*!< Position of CDC_HCLK_EN field. */
#define CPR_OTHER_CLK_CDC_HCLK_EN_Msk (0x1UL << CPR_OTHER_CLK_CDC_HCLK_EN_Pos) 
#define CPR_OTHER_CLK_CDC_HCLK_EN_Disable (0UL) /*!< 0：不使能 */
#define CPR_OTHER_CLK_CDC_HCLK_EN_Enable (1UL) /*!< 1：使能*/

/* Bit 17: CDC_MCLK_GR_UPD */
/* Description:  audio ADC分频寄存器更新，写1自清0
 * 0：Disable
 * 1：Enable
*/
#define CPR_OTHER_CLK_CDC_MCLK_GR_UPD_Pos (17UL) // /*!< Position of CDC_MCLK_GR_UPD field. */
#define CPR_OTHER_CLK_CDC_MCLK_GR_UPD_Msk (0x1UL << CPR_OTHER_CLK_CDC_MCLK_GR_UPD_Pos) 
#define CPR_OTHER_CLK_CDC_MCLK_GR_UPD_Disable (0UL) /*!< 0：不使能 */
#define CPR_OTHER_CLK_CDC_MCLK_GR_UPD_Enable (1UL) /*!< 1：使能*/

/* Bit 20: USB_RSTN_REG */
/* Description:  usb软复位，0：复位有效
 * 1：Invalid
 * 0：valid
*/
#define CPR_OTHER_CLK_USB_RSTN_REG_Pos (17UL) // /*!< Position of USB_RSTN_REG field. */
#define CPR_OTHER_CLK_USB_RSTN_REG_Msk (0x1UL << CPR_OTHER_CLK_USB_RSTN_REG_Pos) 
#define CPR_OTHER_CLK_USB_RSTN_REG_Disable (RSTN_INVALID) /*!< 1：不使能 */
#define CPR_OTHER_CLK_USB_RSTN_REG_Enable (RSTN_VALID) /*!< 0：使能*/


/* Bit 21: RF24G_RSTN_REG */
/* Description:  RF24G软复位，0：复位有效
 * 1：Invalid
 * 0：valid
*/
#define CPR_OTHER_CLK_RF24G_RSTN_REG_Pos (21UL) // /*!< Position of RF24G_RSTN_REG field. */
#define CPR_OTHER_CLK_RF24G_RSTN_REG_Msk (0x1UL << CPR_OTHER_CLK_RF24G_RSTN_REG_Pos) 
#define CPR_OTHER_CLK_RF24G_RSTN_REG_Disable (RSTN_INVALID) /*!< 1：不使能 */
#define CPR_OTHER_CLK_RF24G_RSTN_REG_Enable (RSTN_VALID) /*!< 0：使能*/

/* Bit 22: CDC_RSTN_REG */
/* Description:  CDC 软复位，0：复位有效
 * 1：Invalid
 * 0：valid
*/
#define CPR_OTHER_CLK_CDC_RSTN_REG_Pos (22UL) // /*!< Position of CDC_RSTN_REG field. */
#define CPR_OTHER_CLK_CDC_RSTN_REG_Msk (0x1UL << CPR_OTHER_CLK_CDC_RSTN_REG_Pos) 
#define CPR_OTHER_CLK_CDC_RSTN_REG_Disable (RSTN_INVALID) /*!< 1：不使能 */
#define CPR_OTHER_CLK_CDC_RSTN_REG_Enable (RSTN_VALID) /*!< 0：使能*/

/* Bit 23: CDC_CLK_INV_EN */
/* Description:  audio ADC模拟时钟反向：1：反向
 * 0：Disable
 * 1：Enable
*/
#define CPR_OTHER_CLK_CDC_CLK_INV_EN_Pos (23UL) // /*!< Position of CDC_CLK_INV_EN field. */
#define CPR_OTHER_CLK_CDC_CLK_INV_EN_Msk (0x1UL << CPR_OTHER_CLK_CDC_CLK_INV_EN_Pos) 
#define CPR_OTHER_CLK_CDC_CLK_INV_EN_Disable (0UL) /*!< 0：不使能 */
#define CPR_OTHER_CLK_CDC_CLK_INV_EN_Enable (1UL) /*!< 1：使能*/

/* Bit 24..27: USB_UTMI_CLK_DIV */
/* Description:  usb的utmi时钟分频比，分频值=寄存器值+1
*/
#define CPR_OTHER_CLK_USB_UTMI_CLK_DIV_Pos (24UL) // /*!< Position of CDC_CLK_INV_EN field. */
#define CPR_OTHER_CLK_USB_UTMI_CLK_DIV_Msk (0xFUL << CPR_OTHER_CLK_USB_UTMI_CLK_DIV_Pos) 

/* Bit 28: USB_UTMI_CLK_EN */
/* Description:  usb的utmi时钟时钟，1使能
 * 0：Disable
 * 1：Enable
*/
#define CPR_OTHER_CLK_USB_UTMI_CLK_EN_Pos (28UL) // /*!< Position of USB_UTMI_CLK_EN field. */
#define CPR_OTHER_CLK_USB_UTMI_CLK_EN_Msk (0x1UL << CPR_OTHER_CLK_USB_UTMI_CLK_EN_Pos) 
#define CPR_OTHER_CLK_USB_UTMI_CLK_EN_Disable (0UL) /*!< 0：不使能 */
#define CPR_OTHER_CLK_USB_UTMI_CLK_EN_Enable (1UL) /*!< 1：使能*/


/* Register: CPR_M4_NEW_REG2 */
/* Offse: 0x1B8 */
/* Description :AUDIO  CLK 相关控制寄存器
*/

/* Bit 0..15: CDC_MCLK_DIV */
/* Description: audio ADC小数分频DIV值
*/
#define CPR_OTHER_CLK_CDC_MCLK_DIV_Pos (0UL) // /*!< Position of CPR_OTHER_CLK_CDC_MCLK_DIV_Pos field. */
#define CPR_OTHER_CLK_CDC_MCLK_DIV_Msk (0xFFFFUL << CPR_OTHER_CLK_CDC_MCLK_DIV_Pos) 

/* Bit 16..31: CDC_MCLK_MUL */
/* Description: audio ADC小数分频MUL值
*/
#define CPR_OTHER_CLK_CDC_MCLK_MUL_Pos (16UL) // /*!< Position of CDC_MCLK_MUL field. */
#define CPR_OTHER_CLK_CDC_MCLK_MUL_Msk (0xFFFFUL << CPR_OTHER_CLK_CDC_MCLK_MUL_Pos) 



/* Register: CPR_UART2_CLK_GRCTL */
/* Description :UART2_CLK 时 钟 GR 控制寄存器 对应UART2
* Offset      : 0x250
    BIT 16 写屏蔽 BIT[3:0]，BIT 20 写屏蔽 BIT[7:4]，BIT 24 写屏蔽 BIT[11:8]，BIT 28
写屏蔽 BIT[15:12]*/
/* Bit 0..3: UART2_CLK_GR */
/* Description: uart2_clk 相对于 32MHz 时
钟的一级分频比为:UART2_CLK_GR/8

*/
#define CPR_UART_CLK_GRCTL_UART2_CLK_GR_Pos (0UL) // /*!< Position of UART2_CLK_GR field. */
#define CPR_UART_CLK_GRCTL_UART2_CLK_GR_Msk (0xFUL << CPR_UART_CLK_GRCTL_UART2_CLK_GR_Pos) 
#define CPR_UART_CLK_GRCTL_UART2_CLK_GR_WE  ((1UL << CPR_UART_CLK_GRCTL_UART2_CLK_GR_Pos) << CPR_UART_CLK_GRCTL_MASK_OFFSET)

/* Bit 4: UART2_CLK_GR_UPD */
/* Description: uart2_clk GR 参数更新寄存器
自清 0
1 有效
*/
#define CPR_UART_CLK_GRCTL_UART2_CLK_GR_UPD_Pos (4UL) // /*!< Position of UART2_CLK_GR_UPD field. */
#define CPR_UART_CLK_GRCTL_UART2_CLK_GR_UPD_Msk (0x1UL << CPR_UART_CLK_GRCTL_UART2_CLK_GR_UPD_Pos) 
#define CPR_UART_CLK_GRCTL_UART2_CLK_GR_UPD_Disable (0UL) /*!< 自清 */
#define CPR_UART_CLK_GRCTL_UART2_CLK_GR_UPD_Enable (1UL) /*!< 有效*/

#define CPR_UART_CLK_GRCTL_UART2_CLK_GR_UPD_WE  ((1UL << CPR_UART_CLK_GRCTL_UART2_CLK_GR_UPD_Pos) << CPR_UART_CLK_GRCTL_MASK_OFFSET)

/* Bit 5: UART2_PCLK_EN */
/* Description: UART2_PCLK 时钟使能
0：不使能时钟
1：使能时钟
*/
#define CPR_UART_CLK_GRCTL_UART2_PCLK_EN_Pos (5UL) // /*!< Position of UART2_PCLK_EN field. */
#define CPR_UART_CLK_GRCTL_UART2_PCLK_EN_Msk (0x1UL << CPR_UART_CLK_GRCTL_UART2_PCLK_EN_Pos) 
#define CPR_UART_CLK_GRCTL_UART2_PCLK_EN_Disable (0UL) /*!< 不使能时钟 */
#define CPR_UART_CLK_GRCTL_UART2_PCLK_EN_Enable (1UL) /*!< 使能时钟*/

#define CPR_UART_CLK_GRCTL_UART2_PCLK_EN_WE  CPR_UART_CLK_GRCTL_UART2_CLK_GR_UPD_WE





/* Register: CPR_I2S_CLK_CTL */
/* Offse: 0x258 */
/* Description : I2S CLK 相关控制寄存器
*/

/* Bit 0..11: i2s_mclk_div */
/* Description: 
*/
#define CPR_I2S_CLK_CTL_I2S_MCLK_DIV_Pos (0UL) // /*!< Position of i2s_mclk_div field. */
#define CPR_I2S_CLK_CTL_I2S_MCLK_DIV_Msk (0xFFFUL << CPR_I2S_CLK_CTL_I2S_MCLK_DIV_Pos) 

/* Bit 12: i2s_mclk_en */
/* Description: 
*/
#define CPR_I2S_CLK_CTL_I2S_MCLK_EN_Pos (12UL) // /*!< Position of i2s_mclk_en field. */
#define CPR_I2S_CLK_CTL_I2S_MCLK_EN_Msk (0x1UL << CPR_I2S_CLK_CTL_I2S_MCLK_EN_Pos) 
#define CPR_I2S_CLK_CTL_I2S_MCLK_EN_Disable (0UL) /*!< 不使能时钟 */
#define CPR_I2S_CLK_CTL_I2S_MCLK_EN_Enable (1UL) /*!< 使能时钟*/

/* Bit 13: i2s_pclk_en */
/* Description: 
*/
#define CPR_I2S_CLK_CTL_I2S_PCLK_EN_Pos (13UL) // /*!< Position of i2s_pclk_en field. */
#define CPR_I2S_CLK_CTL_I2S_PCLK_EN_Msk (0x1UL << CPR_I2S_CLK_CTL_I2S_PCLK_EN_Pos) 
#define CPR_I2S_CLK_CTL_I2S_PCLK_EN_Disable (0UL) /*!< 不使能时钟 */
#define CPR_I2S_CLK_CTL_I2S_PCLK_EN_Enable (1UL) /*!< 使能时钟*/


/* Register: CPR_CAN_CLK_CTL */
/* Offse: 0x25C */
/* Description : CAN CLK 相关控制寄存器
*/

/* Bit 0..11: can_mclk_div */
/* Description: 
*/
#define CPR_CAN_CLK_CTL_CAN_MCLK_DIV_Pos (0UL) // /*!< Position of can_mclk_div field. */
#define CPR_CAN_CLK_CTL_CAN_MCLK_DIV_Msk (0xFFFUL << CPR_CAN_CLK_CTL_CAN_MCLK_DIV_Pos) 

/* Bit 12: can_mclk_en */
/* Description: 
*/
#define CPR_CAN_CLK_CTL_CAN_MCLK_EN_Pos (12UL) // /*!< Position of can_mclk_en field. */
#define CPR_CAN_CLK_CTL_CAN_MCLK_EN_Msk (0x1UL << CPR_CAN_CLK_CTL_CAN_MCLK_EN_Pos) 
#define CPR_CAN_CLK_CTL_CAN_MCLK_EN_Disable (0UL) /*!< 不使能时钟 */
#define CPR_CAN_CLK_CTL_CAN_MCLK_EN_Enable (1UL) /*!< 使能时钟*/

/* Bit 13: can_pclk_en */
/* Description: 
*/
#define CPR_CAN_CLK_CTL_CAN_PCLK_EN_Pos (13UL) // /*!< Position of can_pclk_en field. */
#define CPR_CAN_CLK_CTL_CAN_PCLK_EN_Msk (0x1UL << CPR_CAN_CLK_CTL_CAN_PCLK_EN_Pos) 
#define CPR_CAN_CLK_CTL_CAN_PCLK_EN_Disable (0UL) /*!< 不使能时钟 */
#define CPR_CAN_CLK_CTL_CAN_PCLK_EN_Enable (1UL) /*!< 使能时钟*/



/*lint --flb "Leave library region" */
#endif
