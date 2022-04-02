/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef __XINC620610_BITS_H
#define __XINC620610_BITS_H

/*lint ++flb "Enter library region" */


/* Peripheral: CLOCK */
/* Description: Clock control */


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
#define CPR_RSTCTL_CTLAPB_SW_GPIO_RSTN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_CTLAPB_SW_GPIO_RSTN_Enable (0UL) /*!< 0：复位*/

/* Bit 2: WDT_RSTN */
/* Description: WDT 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_CTLAPB_SW_WDT_RSTN_Pos (2UL) // /*!< Position of WDT_RSTN field. */
#define CPR_RSTCTL_CTLAPB_SW_WDT_RSTN_Msk (0x1UL << CPR_RSTCTL_CTLAPB_SW_WDT_RSTN_Pos) 
#define CPR_RSTCTL_CTLAPB_SW_WDT_RSTN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_CTLAPB_SW_WDT_RSTN_Enable (0UL) /*!< 0：复位*/

/* Bit 3: TIMER0_RSTN */
/* Description: TIMER0 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_CTLAPB_SW_TIMER0_RSTN_Pos (3UL) // /*!< Position of TIMER0_RSTN field. */
#define CPR_RSTCTL_CTLAPB_SW_TIMER0_RSTN_Msk (0x1UL << CPR_RSTCTL_CTLAPB_SW_TIMER0_RSTN_Pos) 
#define CPR_RSTCTL_CTLAPB_SW_TIMER0_RSTN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_CTLAPB_SW_TIMER0_RSTN_Enable (0UL) /*!< 0：复位*/

/* Bit 4: TIMER1_RSTN */
/* Description: TIMER1 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_CTLAPB_SW_TIMER1_RSTN_Pos (4UL) // /*!< Position of TIMER1_RSTN field. */
#define CPR_RSTCTL_CTLAPB_SW_TIMER1_RSTN_Msk (0x1UL << CPR_RSTCTL_CTLAPB_SW_TIMER1_RSTN_Pos) 
#define CPR_RSTCTL_CTLAPB_SW_TIMER1_RSTN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_CTLAPB_SW_TIMER1_RSTN_Enable (0UL) /*!< 0：复位*/

/* Bit 5: TIMER2_RSTN */
/* Description: TIMER2 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_CTLAPB_SW_TIMER2_RSTN_Pos (5UL) // /*!< Position of TIMER2_RSTN field. */
#define CPR_RSTCTL_CTLAPB_SW_TIMER2_RSTN_Msk (0x1UL << CPR_RSTCTL_CTLAPB_SW_TIMER2_RSTN_Pos) 
#define CPR_RSTCTL_CTLAPB_SW_TIMER2_RSTN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_CTLAPB_SW_TIMER2_RSTN_Enable (0UL) /*!< 0：复位*/

/* Bit 6: TIMER3_RSTN */
/* Description: TIMER3 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_CTLAPB_SW_TIMER3_RSTN_Pos (6UL) // /*!< Position of TIMER3_RSTN field. */
#define CPR_RSTCTL_CTLAPB_SW_TIMER3_RSTN_Msk (0x1UL << CPR_RSTCTL_CTLAPB_SW_TIMER3_RSTN_Pos) 
#define CPR_RSTCTL_CTLAPB_SW_TIMER3_RSTN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_CTLAPB_SW_TIMER3_RSTN_Enable (0UL) /*!< 0：复位*/


/* Bit 7: TIMER_PRESETN */
/* Description: TIMER3 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_CTLAPB_SW_TIMER_PRESETN_Pos (6UL) // /*!< Position of TIMER3_RSTN field. */
#define CPR_RSTCTL_CTLAPB_SW_TIMER_PRESETN_Msk (0x1UL << CPR_RSTCTL_CTLAPB_SW_TIMER3_RSTN_Pos) 
#define CPR_RSTCTL_CTLAPB_SW_TIMER_PRESETN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_CTLAPB_SW_TIMER_PRESETN_Enable (0UL) /*!< 0：复位*/

/* Bit 8: CTL_APB_RSTN */
/* Description: CTL_APB 桥软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_CTLAPB_SW_CTL_APB_RSTN_Pos (8UL) // /*!< Position of CTL_APB_RSTN field. */
#define CPR_RSTCTL_CTLAPB_SW_CTL_APB_RSTN_Msk (0x1UL << CPR_RSTCTL_CTLAPB_SW_CTL_APB_RSTN_Pos) 
#define CPR_RSTCTL_CTLAPB_SW_CTL_APB_RSTN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_CTLAPB_SW_CTL_APB_RSTN_Enable (0UL) /*!< 0：复位*/


/* Bit 9: KBS_RSTN */
/* Description: KBS 模块软复位
1：不复位
0：复位
*/
#define CPR_RSTCTL_CTLAPB_SW_KBS_RSTN_Pos (9UL) // /*!< Position of KBS_RSTN field. */
#define CPR_RSTCTL_CTLAPB_SW_KBS_RSTN_Msk (0x1UL << CPR_RSTCTL_CTLAPB_SW_KBS_RSTN_Pos) 
#define CPR_RSTCTL_CTLAPB_SW_KBS_RSTN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_CTLAPB_SW_KBS_RSTN_Enable (0UL) /*!< 0：复位*/

/* Bit 10: QDEC_RSTN */
/* Description: QDEC 模块软复位
1：不复位
0：复位
*/
#define CPR_RSTCTL_CTLAPB_SW_QDEC_RSTN_Pos (10UL) // /*!< Position of QDEC_RSTN field. */
#define CPR_RSTCTL_CTLAPB_SW_QDEC_RSTN_Msk (0x1UL << CPR_RSTCTL_CTLAPB_SW_QDEC_RSTN_Pos) 
#define CPR_RSTCTL_CTLAPB_SW_QDEC_RSTN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_CTLAPB_SW_QDEC_RSTN_Enable (0UL) /*!< 0：复位*/

/* Bit 11: PWM_RSTN */
/* Description: PWM 模块软复位
1：不复位
0：复位
*/
#define CPR_RSTCTL_CTLAPB_SW_PWM_RSTN_Pos (11UL) // /*!< Position of PWM_RSTN field. */
#define CPR_RSTCTL_CTLAPB_SW_PWM_RSTN_Msk (0x1UL << CPR_RSTCTL_CTLAPB_SW_PWM_RSTN_Pos) 
#define CPR_RSTCTL_CTLAPB_SW_PWM_RSTN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_CTLAPB_SW_PWM_RSTN_Enable (0UL) /*!< 0：复位*/

/* Bit 12: GPADC_RSTN */
/* Description: GPADC 模块软复位
1：不复位
0：复位
*/
#define CPR_RSTCTL_CTLAPB_SW_GPADC_RSTN_Pos (12UL) // /*!< Position of GPADC_RSTN field. */
#define CPR_RSTCTL_CTLAPB_SW_GPADC_RSTN_Msk (0x1UL << CPR_RSTCTL_CTLAPB_SW_GPADC_RSTN_Pos) 
#define CPR_RSTCTL_CTLAPB_SW_GPADC_RSTN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_CTLAPB_SW_GPADC_RSTN_Enable (0UL) /*!< 0：复位*/


/* Register: CPR_RSTCTL_SUBRST_SW */
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
#define CPR_RSTCTL_SUBRST_SW_UART0_RSTN_Disable (1UL) /*!< 0：不复位 */
#define CPR_RSTCTL_SUBRST_SW_UART0_RSTN_Enable (0UL) /*!< 1：复位*/

/* Bit 1: UART1_RSTN */
/* Description: UART1 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_UART1_RSTN_Pos (1UL) // /*!< Position of UART1_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_UART1_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_UART1_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_UART1_RSTN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_UART1_RSTN_Enable (0UL) /*!< 0：复位*/


/* Bit 2: SSI0_RSTN */
/* Description: SS0 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_SSI0_RSTN_Pos (2UL) // /*!< Position of SSI0_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_SSI0_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_SSI0_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_SSI0_RSTN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_SSI0_RSTN_Enable (0UL) /*!< 0：复位*/

/* Bit 3: SSI1_RSTN */
/* Description: SSI1 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_SSI1_RSTN_Pos (3UL) // /*!< Position of SSI1_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_SSI1_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_SSI1_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_SSI1_RSTN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_SSI1_RSTN_Enable (0UL) /*!< 0：复位*/

/* Bit 4: DMAS_RSTN */
/* Description: DMAS 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_DMAS_RSTN_Pos (4UL) // /*!< Position of DMAS_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_DMAS_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_DMAS_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_DMAS_RSTN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_DMAS_RSTN_Enable (0UL) /*!< 0：复位*/

/* Bit 5: BT_RSTN */
/* Description: BT 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_BT_RSTN_Pos (5UL) // /*!< Position of BT_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_BT_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_BT_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_BT_RSTN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_BT_RSTN_Enable (0UL) /*!< 0：复位*/

/* Bit 6: I2C_RSTN */
/* Description: I2C 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_I2C_RSTN_Pos (6UL) // /*!< Position of I2C_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_I2C_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_I2C_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_I2C_RSTN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_I2C_RSTN_Enable (0UL) /*!< 0：复位*/

/* Bit 7: SHRAM0_RSTN */
/* Description: SHRAM0 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_SHRAM0_RSTN_Pos (7UL) // /*!< Position of SHRAM0_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_SHRAM0_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_SHRAM0_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_SHRAM0_RSTN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_SHRAM0_RSTN_Enable (0UL) /*!< 0：复位*/

/* Bit 8: AES_RSTN */
/* Description: AES 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_AES_RSTN_Pos (8UL) // /*!< Position of AES_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_AES_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_AES_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_AES_RSTN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_AES_RSTN_Enable (0UL) /*!< 0：复位*/

/* Bit 9: SIM_RSTN */
/* Description: SIM 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_SIM_RSTN_Pos (9UL) // /*!< Position of SIM_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_SIM_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_SIM_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_SIM_RSTN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_SIM_RSTN_Enable (0UL) /*!< 0：复位*/


/* Bit 10: SSI2_RSTN */
/* Description: SSI2 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_SSI2_RSTN_Pos (10UL) // /*!< Position of SSI2_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_SSI2_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_SSI2_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_SSI2_RSTN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_SSI2_RSTN_Enable (0UL) /*!< 0：复位*/

/* Bit 11: UART2_RSTN */
/* Description: UART2 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_UART2_RSTN_Pos (11UL) // /*!< Position of UART2_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_UART2_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_UART2_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_UART2_RSTN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_UART2_RSTN_Enable (0UL) /*!< 0：复位*/

/* Bit 12: I2S_RSTN */
/* Description: I2S 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_I2S_RSTN_Pos (12UL) // /*!< Position of I2S_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_I2S_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_I2S_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_I2S_RSTN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_I2S_RSTN_Enable (0UL) /*!< 0：复位*/


/* Bit 13: CAN_RSTN */
/* Description: CAN 模块软复位：
1：不复位
0：复位
*/
#define CPR_RSTCTL_SUBRST_SW_CAN_RSTN_Pos (13UL) // /*!< Position of CAN_RSTN field. */
#define CPR_RSTCTL_SUBRST_SW_CAN_RSTN_Msk (0x1UL << CPR_RSTCTL_SUBRST_SW_CAN_RSTN_Pos) 
#define CPR_RSTCTL_SUBRST_SW_CAN_RSTN_Disable (1UL) /*!< 1：不复位 */
#define CPR_RSTCTL_SUBRST_SW_CAN_RSTN_Enable (0UL) /*!< 0：复位*/




/* Register: CPR_RSTCTL_WDTRST_MASK */
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











/* Register: CPR_AOCLKEN_GRCTL */
/* Description :AO 域时钟使能寄存器
    高 16BITS 写屏蔽低 16BITS*/
#define CPR_AOCLKEN_GRCTL_MASK_OFFSET (16UL)

/* Bit 1: RTC_CLK_EN */
/* Description: RTC_CLK 时钟使能
0：不使能时钟
1：使能时钟
*/
#define CPR_AOCLKEN_GRCTL_RTC_CLK_EN_Pos (1UL) // /*!< Position of RTC_CLK_EN field. */
#define CPR_AOCLKEN_GRCTL_RTC_CLK_EN_Msk (0x1UL << CPR_AOCLKEN_GRCTL_RTC_CLK_EN_Pos) 
#define CPR_AOCLKEN_GRCTL_RTC_CLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_AOCLKEN_GRCTL_RTC_CLK_EN_Enable (1UL) /*!< 1：使能时钟*/



/////////////////////////////////////////////////////////////////////// XINCHIP GPIO   //////////////////////////////////////////////////////



/* Peripheral: GPIO */
/* Description: GPIO Port 0 */


/* Register: GPIO_PORT_DRx */
/* Description:端口数据寄存器 
GPIO Portn 的对应寄存器位为 GPIO_PORT_DRx[y]，其中 x = n / 16，y = n % 16，“/”
为除法运算符，“%”为求余运算符*/

/* Bit 0..15 : DR */
/* Description: 相应端口设置为“输出”模式时，该寄存器
中的值决定 IO 端口的输出。读寄存器，返
回值为上一次写入该寄存器的值。
*/
#define GPIO_GPIO_PORT_DR_Pos (0UL) // /*!< Position of DR field. */
#define GPIO_GPIO_PORT_DR_Msk (0xFFFFUL << GPIO_GPIO_PORT_DR_Pos) // /*!< Position of DR field. */
#define GPIO_GPIO_PORT_DR_Low (0UL) /*!< 0：低电平 */
#define GPIO_GPIO_PORT_DR_High (1UL) /*!< 1：高电平 */

/* Bit 16..31 : DR_WE */
/* Description: DR 写使能位
0：不允许写
1：允许写
DR_WE[n]控制 DR[n]，如 DR_WR[0]控制
DR[0]
*/
#define GPIO_GPIO_PORT_DR_WE_Pos (16UL) // /*!< Position of DR_WE field. */
#define GPIO_GPIO_PORT_DR_WE_Msk (0xFFFFUL << GPIO_GPIO_PORT_DR_WE_Pos) // /*!< Position of DR_WE field. */
#define GPIO_GPIO_PORT_DR_WE_Disable (0UL) /*!< 0：不允许写 */
#define GPIO_GPIO_PORT_DR_WE_Enable (1UL) /*!< 1：允许写 */



/* Register: GPIO_PORT_DDR */
/* Description:端口数据方向寄存器 
GPIO Portn 的对应寄存器位为 GPIO_PORT_DDR[y]，其中 x = n / 16，y = n % 16，“/”
为除法运算符，“%”为求余运算符*/

/* Bit 0..15 : DDR */
/* Description: 相IO 端口的方向。
0：输入方向
1：输出方向
*/
#define GPIO_GPIO_PORT_DDR_Pos (0UL) // /*!< Position of DR field. */
#define GPIO_GPIO_PORT_DDR_Msk (0xFFFFUL << GPIO_GPIO_PORT_DDR_Pos) // /*!< Position of DDR field. */
#define GPIO_GPIO_PORT_DDR_Input (0UL) /*!< 0：输入方向 */
#define GPIO_GPIO_PORT_DDR_Output (1UL) /*!< 1：输出方向 */


/* Bit 16..31 : DDR_WE */
/* Description: DDR 写使能位
0：不允许写
1：允许写
DDR_WE[n]控制 DR[n]，如 DDR_WR[0]控制
DDR[0]
*/
#define GPIO_GPIO_PORT_DDR_WE_Pos (16UL) // /*!< Position of DDR_WE field. */
#define GPIO_GPIO_PORT_DDR_WE_Msk (0xFFFFUL << GPIO_GPIO_PORT_DDR_WE_Pos) // /*!< Position of DDR_WE field. */
#define GPIO_GPIO_PORT_DDR_WE_Disable (0UL) /*!< 0：不允许写 */
#define GPIO_GPIO_PORT_DDR_WE_Enable (1UL) /*!< 1：允许写 */




/* Register: GPIO_EXT_PORT */

/* Description:外部端口输入寄存器 
GPIO Portn 的对应寄存器位为 GPIO_EXT_PORTx[y]，
其中 x = n / 32，y = n % 32， “/”为除法运算符，“%”为求余运算符
*/
/* Bit 0..31 : DIN */
/* Description: 当端口配置为“输入”，读该寄存器的相应
位可以得到对应 IO 端口的值。如果端口配
置为“输出”，读该寄存器得到的是端口数
据寄存器（GPIO_PORT_DRx）的值。
*/
#define GPIO_GPIO_EXT_PORT_DIN_Pos (0UL) // /*!< Position of DIN field. */
#define GPIO_GPIO_EXT_PORT_DIN_Msk (0xFFFFFFFFUL << GPIO_GPIO_EXT_PORT_DIN_Pos) // /*!< Position of DIN field. */
#define GPIO_GPIO_EXT_PORT_DIN_Low (0UL) /*!< 0：low */
#define GPIO_GPIO_EXT_PORT_DIN_High (1UL) /*!< 1:high */

/* Register: GPIO_INTR_CTRL */

/* Description:中断控制寄存器
GPIO Portn 的对应寄存器位为 GPIO_INTR_CTRLx[y*4+3:y*4]，其中 x = n / 4，y = n % 
4，“/”为除法运算符，“%”为求余运算符
*/

/* Bit 0:    MODE_EN */
/* Description: MODE0 [3：1]为 IO 端口中断模式
模式设置
MODE0[0]为 IO 端口中断模式使能
0：不使能中断模式
1：使能中断模式
*/
#define GPIO_GPIO_INTR_CTRL_MODE_EN_Pos (0UL) // /*!< Position of MODE_EN field. */
#define GPIO_GPIO_INTR_CTRL_MODE_EN_Msk (0x1UL << GPIO_GPIO_INTR_CTRL_MODE_EN_Pos) // /*!< Position of MODE_EN field. */
#define GPIO_GPIO_INTR_CTRL_MODE_EN_Disable (0UL) /*!<0：不使能中断模式 */
#define GPIO_GPIO_INTR_CTRL_MODE_EN_Enable (1UL) /*!< 1：使能中断模式 */

/* Bit 1..3:    MODE0 */
/* Description: MODE0 [3：1]为 IO 端口中断模式
模式设置
0：高电平中断
1：低电平中断
2：gpio_clk 上升沿中断
3：gpio_clk 下降沿中断
4：gpio_clk 沿中断
5：pclk 上升沿中断
6：pclk 下降沿中断
7：pclk 沿中断。

*/
#define GPIO_GPIO_INTR_CTRL_MODE0_Pos (1UL) // /*!< Position of MODE0 field. */
#define GPIO_GPIO_INTR_CTRL_MODE0_Msk (0x7UL << GPIO_GPIO_INTR_CTRL_MODE0_Pos) // /*!< Position of MODE0 field. */

#define GPIO_GPIO_INTR_CTRL_MODE1_Pos (5UL) // /*!< Position of MODE1 field. */
#define GPIO_GPIO_INTR_CTRL_MODE1_Msk (0x7UL << GPIO_GPIO_INTR_CTRL_MODE1_Pos) // /*!< Position of MODE1 field. */


#define GPIO_GPIO_INTR_CTRL_MODE2_Pos (9UL) // /*!< Position of MODE2 field. */
#define GPIO_GPIO_INTR_CTRL_MODE2_Msk (0x7UL << GPIO_GPIO_INTR_CTRL_MODE2_Pos) // /*!< Position of MODE2 field. */


#define GPIO_GPIO_INTR_CTRL_MODE3_Pos (13UL) // /*!< Position of MODE3 field. */
#define GPIO_GPIO_INTR_CTRL_MODE3_Msk (0x7UL << GPIO_GPIO_INTR_CTRL_MODE3_Pos) // /*!< Position of MODE3 field. */

#define GPIO_GPIO_INTR_CTRL_MODE_High (0UL) /*!<0：高电平中断 */
#define GPIO_GPIO_INTR_CTRL_MODE_Low (1UL) /*!< 1：低电平中断 */

#define GPIO_GPIO_INTR_CTRL_MODE_gpio_clk_ris_edge (2UL) /*!< 2:gpio_clk 上升沿中断 */
#define GPIO_GPIO_INTR_CTRL_MODE_gpio_clk_fail_edge (3UL) /*!< 3:gpio_clk 下降沿中断 */
#define GPIO_GPIO_INTR_CTRL_MODE_gpio_clk_ris_fail_edge (4UL) /*!< 4:gpio_clk 沿中断 */

#define GPIO_GPIO_INTR_CTRL_MODE_pclk_clk_ris_edge (5UL) /*!< 5:pclk_clk 上升沿中断 */
#define GPIO_GPIO_INTR_CTRL_MODE_pclk_clk_fail_edge (6UL) /*!< 6:pclk_clk 下降沿中断 */
#define GPIO_GPIO_INTR_CTRL_MODE_pclk_clk_ris_fail_edge (7UL) /*!< 7:pclk_clk 沿中断 */


/* Bit 16..19:    MODE_WE */
/* Description: 中断模式写使能位
0：不允许写
1：允许写
*/
#define GPIO_GPIO_INTR_CTRL_MODE_WE_Pos (1UL) // /*!< Position of MODE_WE field. */
#define GPIO_GPIO_INTR_CTRL_MODE_WE_Msk (0x7UL << GPIO_GPIO_INTR_CTRL_MODE_Pos) // /*!< Position of MODE_WE field. */
#define GPIO_GPIO_INTR_CTRL_MODE_WE_Disable (0UL) /*!<0：不允许写 */
#define GPIO_GPIO_INTR_CTRL_MODE_WE_Enable (1UL) /*!< 1：允许写 */



/* Register: GPIO_DEBOUNCE */

/* Description:去毛刺使能寄存器
GPIO Portn 的对应寄存器位为 GPIO_DEBOUNCEx[y]，
其中 x = n / 16，y = n % 16， “/”为除法运算符，“%”为求余运算符
*/
/* Bit 0..15: DEBEN */
/* Description: IO 端口去毛刺操作使能
0：不使能去毛刺操作
1：使能去毛刺操作
*/
#define GPIO_GPIO_DEBOUNCE_DEBEN_Pos (0UL) // /*!< Position of DEBEN field. */
#define GPIO_GPIO_DEBOUNCE_DEBEN_Msk (0xFFFFUL << GPIO_GPIO_DEBOUNCE_DEBEN_Pos) // /*!< Position of DEBEN field. */
#define GPIO_GPIO_DEBOUNCE_DEBEN_Disable (0UL) /*!<0：不使能去毛刺操作 */
#define GPIO_GPIO_DEBOUNCE_DEBEN_Enable (1UL) /*!< 1：使能去毛刺操作 */

/* Bit 16..31: DEBEN_WE */
/* Description: DEBEN 写使能位
0：不允许写
1：允许写
*/
#define GPIO_GPIO_DEBOUNCE_DEBEN_WE_Pos (16UL) // /*!< Position of DEBEN_WE field. */
#define GPIO_GPIO_DEBOUNCE_DEBEN_WE_Msk (0xFFFFUL << GPIO_GPIO_DEBOUNCE_DEBEN_WE_Pos) // /*!< Position of DEBEN_WE field. */
#define GPIO_GPIO_DEBOUNCE_DEBEN_WE_Disable (0UL) /*!< 0：不允许写 */
#define GPIO_GPIO_DEBOUNCE_DEBEN_WE_Enable (1UL) /*!< 1：允许写 */

/* Register: GPIO_INTR_RAW */

/* Description:中断原始状态寄存器
GPIO Portn 的对应寄存器位为 GPIO_INTR_RAWx[y]，
其中 x = n / 32，y = n % 32， “/”为除法运算符，“%”为求余运算符
*/
/* Bit 0..31: RAW */
/* Description: IO 端口中断原始状态
0：没有中断
1：有中断
*/
#define GPIO_GPIO_INTR_RAW_RAW_Pos (0UL) // /*!< Position of RAW field. */
#define GPIO_GPIO_INTR_RAW_RAW_Msk (0xFFFFFFFFUL << GPIO_GPIO_INTR_RAW_RAW_Pos) // /*!< Position of RAW field. */
#define GPIO_GPIO_INTR_RAW_RAW_NotGenerated (0UL) /*!< 0：没有中断 */
#define GPIO_GPIO_INTR_RAW_RAW_Generated (1UL) /*!< 1：有中断 */

/* Register: GPIO_INTR_CLR */

/* Description:中断清除寄存器
GPIO Portn 的对应寄存器位为 GPIO_INTR_CLRx[y]，
其中 x = n / 32，y = n % 32， “/”为除法运算符，“%”为求余运算符
*/
/* Bit 0..31: CLR */
/* Description: IO 端口中断清除
0：不清除中断
1：清除中断
当中断类型为高电平中断或者为低电平中
断时，此寄存器的操作无效
*/
#define GPIO_GPIO_INTR_CLR_CLR_Pos (0UL) // /*!< Position of CLR field. */
#define GPIO_GPIO_INTR_CLR_CLR_Msk (0xFFFFFFFFUL << GPIO_GPIO_INTR_CLR_CLR_Pos) // /*!< Position of CLR field. */
#define GPIO_GPIO_INTR_CLR_CLR_NotClear (0UL) /*!< 0：不清除中断 */
#define GPIO_GPIO_INTR_CLR_CLR_Clear (1UL) /*!< 1：清除中断 */



/* Register: GPIO_INTR_MASK_C0 */

/* Description:中断清除寄存器
GPIO Portn 的对应寄存器位为 GPIO_INTR_CLRx[y]，
其中 x = n / 32，y = n % 32， “/”为除法运算符，“%”为求余运算符
*/
/* Bit 0..15: MASK */
/* Description: IO 端口中断屏蔽
0：不屏蔽中断
1：屏蔽中断
*/
#define GPIO_GPIO_INTR_MASK_C0_MASK_Pos (0UL) // /*!< Position of MASK field. */
#define GPIO_GPIO_INTR_MASK_C0_MASK_Msk (0xFFFFUL << GPIO_GPIO_INTR_MASK_C0_MASK_Pos) // /*!< Position of MASK field. */
#define GPIO_GPIO_INTR_MASK_C0_MASK_Disable (0UL) /*!< 0：不屏蔽中断 */
#define GPIO_GPIO_INTR_MASK_C0_MASK_Enable (1UL) /*!< 1：屏蔽中断 */

/* Bit 16..31: MASK_WE */
/* Description: MASK 写使能位
0：不允许写
1：允许写
*/
#define GPIO_GPIO_INTR_MASK_C0_MASK_WE_Pos (16UL) // /*!< Position of MASK_WE field. */
#define GPIO_GPIO_INTR_MASK_C0_MASK_WE_Msk (0xFFFFUL << GPIO_GPIO_INTR_MASK_C0_MASK_WE_Pos) // /*!< Position of MASK_WE field. */
#define GPIO_GPIO_INTR_MASK_C0_MASK_WE_Disable (0UL) /*!< 0：不允许写 */
#define GPIO_GPIO_INTR_MASK_C0_MASK_WE_Enable (1UL) /*!< 1：允许写 */




/* Register: GPIO_INTR_STATUS_C0 */

/* Description:中断状态寄存器
GPIO Portn 的对应寄存器位为 GPIO_INTR_STATUS_C0x[y]，
其中 x = n / 32，y = n % 
32，“/”为除法运算符，“%”为求余运算符
*/

/* Bit 0..31: STATUS */
/* Description:IO 端口中断状态
0：没有中断
1：有中断 
*/
#define GPIO_GPIO_INTR_STATUS_C0_STATUS_Pos (0UL) // /*!< Position of STATUS field. */
#define GPIO_GPIO_INTR_STATUS_C0_STATUS_Msk (0xFFFFFFFFUL << GPIO_GPIO_INTR_STATUS_C0_STATUS_Pos) // /*!< Position of STATUS field. */
#define GPIO_GPIO_INTR_STATUS_C0_STATUS_Disable (0UL) /*!< 0：不屏蔽中断 */
#define GPIO_GPIO_INTR_STATUS_C0_STATUS_Enable (1UL) /*!< 1：屏蔽中断 */








/////////////////////////////////////////////////////////////////////// XINCHIP SAADC   //////////////////////////////////////////////////////

/* Peripheral: SAADC */
/* Description: TIMERx Interface  */

/* Register:GPADC_MAIN_CTL    RW */
/* Description:主要控制寄存器
 */
/* Bit 0 : SAADC GPADC_EN  RW */
/* Description: GPADC 模块开关：
 */
#define SAADC_GPADC_MAIN_CTL_GPADC_EN_Pos (0UL) /*!< Position of GPADC_EN field. */
#define SAADC_GPADC_MAIN_CTL_GPADC_EN_Msk (0x1UL << SAADC_GPADC_MAIN_CTL_GPADC_EN_Pos) /*!< Bit mask of GPADC_EN field. */
#define SAADC_GPADC_MAIN_CTL_GPADC_EN_Open (0x1) /*!< 1：打开 */
#define SAADC_GPADC_MAIN_CTL_GPADC_EN_Close (0x0) /*!< 0：关闭 */

/* Bit 1 : SAADC DMAS_ON  RW */
/* Description: DMAS_ON 功能开关：
 */
#define SAADC_GPADC_MAIN_CTL_DMAS_ON_Pos (1UL) /*!< Position of DMAS_ON field. */
#define SAADC_GPADC_MAIN_CTL_DMAS_ON_Msk (0x1UL << SAADC_GPADC_MAIN_CTL_DMAS_ON_Pos) /*!< Bit mask of DMAS_ON field. */
#define SAADC_GPADC_MAIN_CTL_DMAS_ON_Open (0x1) /*!< 1：打开 */
#define SAADC_GPADC_MAIN_CTL_DMAS_ON_Close (0x0) /*!< 0：关闭 */

/* Bit 2 : SAADC AUTO_SW  RW */
/* Description: GPADC 输入通道自动切换功能
 */
#define SAADC_GPADC_MAIN_CTL_AUTO_SW_Pos (2UL) /*!< Position of AUTO_SW field. */
#define SAADC_GPADC_MAIN_CTL_AUTO_SW_Msk (0x1UL << SAADC_GPADC_MAIN_CTL_AUTO_SW_Pos) /*!< Bit mask of AUTO_SW field. */
#define SAADC_GPADC_MAIN_CTL_AUTO_SW_Open (0x1) /*!< 1：打开 */
#define SAADC_GPADC_MAIN_CTL_AUTO_SW_Close (0x0) /*!< 0：关闭 */

/* Bit 3 : SAADC EDGE_SEL  RW */
/* Description: 数据采样沿选择：
 */
#define SAADC_GPADC_MAIN_CTL_EDGE_SEL_Pos (3UL) /*!< Position of EDGE_SEL field. */
#define SAADC_GPADC_MAIN_CTL_EDGE_SEL_Msk (0x1UL << SAADC_GPADC_MAIN_CTL_EDGE_SEL_Pos) /*!< Bit mask of EDGE_SEL field. */
#define SAADC_GPADC_MAIN_CTL_EDGE_SEL_Rise (0x1) /*!< 上升沿 */
#define SAADC_GPADC_MAIN_CTL_EDGE_SEL_Fall (0x0) /*!< 下降沿 */

#define SAADC_GPADC_MAIN_CTL_ALL_CLOSE (0UL)


/* Register:GPADC_CHAN_CTL    RW */
/* Description:GPADC 通道控制寄存器
 */
/* Bit 0..3 : SAADC SELECT_CHAN  RW */
/* Description: ADC 输入通道 开关
 */
#define SAADC_GPADC_CHAN_CTL_SELECT_CHAN_Pos (0UL) /*!< Position of SELECT_CHAN field. */
#define SAADC_GPADC_CHAN_CTL_SELECT_CHAN_Msk (0xFUL << SAADC_GPADC_CHAN_CTL_SELECT_CHAN_Pos) /*!< Bit mask of SELECT_CHAN field. */


/* Bit 4..5: SAADC VCM_CHAN_AUTO  RW */
/* Description: 差分输入的自动切换功能使能，bit0 对应通
道 1 和 2 的差分，bit1 对应通道 3 和 4 的差分
 */
#define SAADC_GPADC_CHAN_CTL_VCM_CHAN_AUTO_Pos (4UL) /*!< Position of VCM_CHAN_AUTO field. */
#define SAADC_GPADC_CHAN_CTL_VCM_CHAN_AUTO_Msk (0x3UL << SAADC_GPADC_CHAN_CTL_VCM_CHAN_AUTO_Pos) /*!< Bit mask of VCM_CHAN_AUTO field. */
#define SAADC_GPADC_CHAN_CTL_VCM_CHAN_1_AUTO_Open (0x1) /*!< 1：打开通道 1 和 2 的差分  */
#define SAADC_GPADC_CHAN_CTL_VCM_CHAN_2_AUTO_Open (0x2) /*!< 2：打开通道 3 和 4的差分 */
#define SAADC_GPADC_CHAN_CTL_VCM_CHAN_ALL_AUTO_Close (0x0) /*!< 0：关闭 */

/* Bit 8..11: SAADC CHAN_AUTO  RW */
/* Description: 单端输入时，选择哪个通道参与自动通道切
换功能，每 1bit 对应一个通道
 */
#define SAADC_GPADC_CHAN_CTL_CHAN_AUTO_Pos (8UL) /*!< Position of CHAN_AUTO field. */
#define SAADC_GPADC_CHAN_CTL_CHAN_AUTO_Msk (0xFUL << SAADC_GPADC_CHAN_CTL_CHAN_AUTO_Pos) /*!< Bit mask of CHAN_AUTO field. */
#define SAADC_GPADC_CHAN_CTL_CHAN_1_AUTO_Open (0x1) /*!< 1：打开通道 1 */
#define SAADC_GPADC_CHAN_CTL_CHAN_2_AUTO_Open (0x2) /*!< 1：打开通道 2 */
#define SAADC_GPADC_CHAN_CTL_CHAN_3_AUTO_Open (0x4) /*!< 1：打开通道 3 */
#define SAADC_GPADC_CHAN_CTL_CHAN_4_AUTO_Open (0x8) /*!< 1：打开通道 4 */
#define SAADC_GPADC_CHAN_CTL_CHAN_ALL_AUTO_Close (0x0) /*!< 0：关闭 */


/* Register:GPADC_FIFO_CTL    RW */
/* Description:FIFO 控制寄存器
 */
/* Bit 0..3 : SAADC READ_REQ_THRESH  RW */
/* Description: 读请求中断阈值
 */
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_Pos (0UL) /*!< Position of CHAN_AUTO field. */
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_Msk (0xFUL << SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_Pos) /*!< Bit mask of CHAN_AUTO field. */
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_1 (1UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_2 (2UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_3 (3UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_4 (4UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_5 (5UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_6 (6UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_7 (7UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_8 (8UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_9 (9UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_10 (10UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_11 (11UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_12 (12UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_13 (13UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_14 (14UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_15 (15UL)

/* Bit 4 : SAADC FIFO_FLUSH  RW */
/* Description: 清空 FIFO
 */
#define SAADC_GPADC_FIFO_CTL_FIFO_FLUSH_Pos (4UL) /*!< Position of FIFO_FLUSH field. */
#define SAADC_GPADC_FIFO_CTL_FIFO_FLUSH_Msk (0x1UL << SAADC_GPADC_FIFO_CTL_FIFO_FLUSH_Pos) /*!< Bit mask of FIFO_FLUSH field. */
#define SAADC_GPADC_FIFO_CTL_FIFO_FLUSH_Empty (1UL)/*!< 1：清空 */
#define SAADC_GPADC_FIFO_CTL_FIFO_FLUSH_NotEmpty (0UL)/*!< 0：不清空 */


/* Register:GPADC_TIMER0    RW */
/* Description:通道切换时间计数器
 */
/* Bit 0..7 : SAADC SW_WAIT_CNT  RW */
/* Description: 通 道 切 换 等 待 时 间 = 总 线 时 钟 周 期
*SW_WAIT_CNT
 */
#define SAADC_GPADC_TIMER0_SW_WAIT_CNT_Pos (0UL) /*!< Position of SW_WAIT_CNT field. */
#define SAADC_GPADC_TIMER0_SW_WAIT_CNT_Msk (0xFFUL << SAADC_GPADC_TIMER0_SW_WAIT_CNT_Pos) /*!< Bit mask of SW_WAIT_CNT field. */

/* Register:GPADC_TIMER1    RW */
/* Description:通道自动切换时间间隔计数器
 */
/* Bit 0..15 : SAADC AUTO_SW_CNT  RW */
/* Description: 通 道 切 换 等 待 时 间 = 总 线 时 钟 周 期
*AUTO_SW_CNT
 */
#define SAADC_GPADC_TIMER1_AUTO_SW_CNT_Pos (0UL) /*!< Position of AUTO_SW_CNT field. */
#define SAADC_GPADC_TIMER1_AUTO_SW_CNT_Msk (0xFFFFUL << SAADC_GPADC_TIMER1_AUTO_SW_CNT_Pos) /*!< Bit mask of AUTO_SW_CNT field. */


/* Register:GPADC_INT    RW */
/* Description:中断状态寄存器
 */
/* Bit 0 : SAADC READ_REQ_INT  RW */
/* Description: FIFO 读请求中断
向本比特位写‘1’清除本中断状态位，同
时清除相应的中断原始状态位
 */
#define SAADC_GPADC_INT_READ_REQ_INT_Pos (0UL) /*!< Position of READ_REQ_INT field. */
#define SAADC_GPADC_INT_READ_REQ_INT_Msk (0x1UL << SAADC_GPADC_INT_READ_REQ_INT_Pos) /*!< Bit mask of READ_REQ_INT field. */
#define SAADC_GPADC_INT_READ_REQ_INT_Generated (1UL) /*!< 产生了 FIFO 读请求中断 */
#define SAADC_GPADC_INT_READ_REQ_INT_NotGenerated (0UL) /*!< 没有产生 FIFO 读请求中断 */
#define SAADC_GPADC_INT_READ_REQ_INT_Clear (1UL) /*!< 清除本中断状态位 */



/* Bit 1 : SAADC FIFO_ERROR_INT  RW */
/* Description: FIFO 错误中断
向本比特位写‘1’清除本中断状态位，同
时清除相应的中断原始状态位
 */
#define SAADC_GPADC_INT_FIFO_ERROR_INT_Pos (1UL) /*!< Position of FIFO_ERROR_INT field. */
#define SAADC_GPADC_INT_FIFO_ERROR_INT_Msk (0x1UL << SAADC_GPADC_INT_FIFO_ERROR_INT_Pos) /*!< Bit mask of FIFO_ERROR_INT field. */
#define SAADC_GPADC_INT_FIFO_ERROR_INT_Generated (1UL) /*!< 产生了读空或溢出错误中断 */
#define SAADC_GPADC_INT_FIFO_ERROR_INT_NotGenerated (0UL) /*!< 没有读空或溢出错误中断 */
#define SAADC_GPADC_INT_FIFO_ERROR_INT_Clear (1UL) /*!< 清除本中断状态位 */



/* Register:GPADC_INT_RAW    RW */
/* Description:中断状态寄存器
 */
/* Bit 0 : SAADC READ_REQ_RAW  RW */
/* Description: FIFO 读请求原始中断
 */
#define SAADC_GPADC_INT_RAW_READ_REQ_RAW_Pos (0UL) /*!< Position of READ_REQ_RAW field. */
#define SAADC_GPADC_INT_RAW_READ_REQ_RAW_Msk (0x1UL << SAADC_GPADC_INT_RAW_READ_REQ_RAW_Pos) /*!< Bit mask of READ_REQ_RAW field. */
#define SAADC_GPADC_INT_RAW_READ_REQ_RAW_Generated (1UL) /*!< 产生了 FIFO 读请求中断 */
#define SAADC_GPADC_INT_RAW_READ_REQ_RAW_NotGenerated (0UL) /*!< 没有产生 FIFO 读请求中断 */




/* Bit 1 : SAADC FIFO_ERROR_RAW  RW */
/* Description: FIFO 错误原始中断
 */
#define SAADC_GPADC_INT_RAW_FIFO_ERROR_RAW_Pos (1UL) /*!< Position of FIFO_ERROR_RAW field. */
#define SAADC_GPADC_INT_RAW_FIFO_ERROR_RAW_Msk (0x1UL << SAADC_GPADC_INT_RAW_FIFO_ERROR_RAW_Pos) /*!< Bit mask of FIFO_ERROR_RAW field. */
#define SAADC_GPADC_INT_RAW_FIFO_ERROR_RAW_Generated (1UL) /*!< 产生了读空或溢出错误中断 */
#define SAADC_GPADC_INT_RAW_FIFO_ERROR_RAW_NotGenerated (0UL) /*!< 没有读空或溢出错误中断 */



/* Register:GPADC_INT_EN    RW */
/* Description:中断状态寄存器
 */
/* Bit 0 : SAADC READ_REQ_EN  RW */
/* Description: FIFO 读请求中断使能
 */
#define SAADC_GPADC_INT_EN_READ_REQ_EN_Pos (0UL) /*!< Position of READ_REQ_EN field. */
#define SAADC_GPADC_INT_EN_READ_REQ_EN_Msk (0x1UL << SAADC_GPADC_INT_EN_READ_REQ_EN_Pos) /*!< Bit mask of READ_REQ_EN field. */
#define SAADC_GPADC_INT_EN_READ_REQ_EN_Enable (1UL) /*!< 1：使能 */
#define SAADC_GPADC_INT_EN_READ_REQ_EN_Disable (0UL) /*!< 0：不使能 */



/* Bit 1 : SAADC FIFO_ERROR_EN  RW */
/* Description: FIFO 溢出中断使能
 */
#define SAADC_GPADC_INT_EN_FIFO_ERROR_EN_Pos (1UL) /*!< Position of FIFO_ERROR_EN field. */
#define SAADC_GPADC_INT_EN_FIFO_ERROR_EN_Msk (0x1UL << SAADC_GPADC_INT_EN_FIFO_ERROR_EN_Pos) /*!< Bit mask of FIFO_ERROR_EN field. */
#define SAADC_GPADC_INT_EN_FIFO_ERROR_EN_Enable (1UL) /*!< 1：使能 */
#define SAADC_GPADC_INT_EN_FIFO_ERROR_EN_Disable (0UL) /*!< 0：不使能 */


/* Register:GPADC_FIFO    RW */
/* Description:FIFO 读数据接口
 */
/* Bit 0..31 : SAADC FIFO_DOUT  RW */
/* Description: FIFO 数据输出
 */
#define SAADC_GPADC_FIFO_FIFO_DOUT_Pos (0UL) /*!< Position of FIFO_DOUT field. */
#define SAADC_GPADC_FIFO_FIFO_DOUT_Msk (0xFFFFFFFFUL << SAADC_GPADC_FIFO_FIFO_DOUT_Pos) /*!< Bit mask of FIFO_DOUT field. */
#define SAADC_GPADC_FIFO_FIFO_DOUT_LEN (0x10UL)


/* Register:GPADC_RF_CTL    RW */
/* Description:GPADC 控制寄存器
 */
/* Bit 0 : SAADC pd_gpadc  RW */
/* Description: 射频 GPADC 断电信号
 */
#define SAADC_GPADC_RF_CTL_pd_gpadc_Pos (0UL) /*!< Position of pd_gpadc field. */
#define SAADC_GPADC_RF_CTL_pd_gpadc_Msk (0x1UL << SAADC_GPADC_RF_CTL_pd_gpadc_Pos) /*!< Bit mask of pd_gpadc field. */
#define SAADC_GPADC_RF_CTL_pd_gpadc_PowerOff (1UL)  /*!< 1：断电 */
#define SAADC_GPADC_RF_CTL_pd_gpadc_PowerOn (0UL)  /*!< 0：不断电 */

/* Bit 1 : SAADC pd_vcmgpadc  RW */
/* Description: 射频 GPADC 差分模式断电信号
 */
#define SAADC_GPADC_RF_CTL_pd_vcmgpadc_Pos (1UL) /*!< Position of pd_vcmgpadc field. */
#define SAADC_GPADC_RF_CTL_pd_vcmgpadc_Msk (0x1UL << SAADC_GPADC_RF_CTL_pd_vcmgpadc_Pos) /*!< Bit mask of pd_vcmgpadc field. */
#define SAADC_GPADC_RF_CTL_pd_vcmgpadc_PowerOff (1UL)  /*!< 1：断电 */
#define SAADC_GPADC_RF_CTL_pd_vcmgpadc_PowerOn (0UL)  /*!< 0：不断电 */

/* Bit 2 : SAADC diffsel_gpadc  RW */
/* Description: 射频 GPADC 差分模式选择信号
 */
#define SAADC_GPADC_RF_CTL_diffsel_gpadc_Pos (2UL) /*!< Position of diffsel_gpadc field. */
#define SAADC_GPADC_RF_CTL_diffsel_gpadc_Msk (0x1UL << SAADC_GPADC_RF_CTL_diffsel_gpadc_Pos) /*!< Bit mask of diffsel_gpadc field. */
#define SAADC_GPADC_RF_CTL_diffsel_gpadc_Diff (1UL)  /*!< 差分模式 */
#define SAADC_GPADC_RF_CTL_diffsel_gpadc_Single (0UL)  /*!< 单端模式 */
#define SAADC_CH_CONFIG_MODE_SE (0UL) /*!< Single-ended, PSELN will be ignored, negative input to SAADC shorted to GND */
#define SAADC_CH_CONFIG_MODE_Diff (1UL) /*!< Differential */



/* Bit 3 : SAADC rst_gpadc  RW */
/* Description: 射频 GPADC 复位控制信号
 */
#define SAADC_GPADC_RF_CTL_rst_gpadc_Pos (3UL) /*!< Position of rst_gpadc field. */
#define SAADC_GPADC_RF_CTL_rst_gpadc_Msk (0x1UL << SAADC_GPADC_RF_CTL_rst_gpadc_Pos) /*!< Bit mask of rst_gpadc field. */
#define SAADC_GPADC_RF_CTL_rst_gpadc_Rst (1UL)  /*!< 复位控制 */
#define SAADC_GPADC_RF_CTL_rst_gpadc_NotRst (0UL)  /*!< 不复位控制 */

/* Bit 4 : SAADC gpadc_ctl_mux  RW */
/* Description: 射频 GPADC 复位控制信号
 */
#define SAADC_GPADC_RF_CTL_gpadc_ctl_mux_Pos (4UL) /*!< Position of gpadc_ctl_mux field. */
#define SAADC_GPADC_RF_CTL_gpadc_ctl_mux_Msk (0x1UL << SAADC_GPADC_RF_CTL_rst_gpadc_Pos) /*!< Bit mask of gpadc_ctl_mux field. */
#define SAADC_GPADC_RF_CTL_gpadc_ctl_mux_Dig (1UL)  /*!< 数字 GPADC 控制 */
#define SAADC_GPADC_RF_CTL_gpadc_ctl_mux_Spi (0UL)  /*!< SPI 控制 */

/////////////////////////////////////////////////////////////////////// XINCHIP SAADC END  //////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////// //////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////// XINCHIP TIMER   //////////////////////////////////////////////////////

/* Peripheral: TIMERx */
/* Description: TIMERx Interface  */

/* Register:TIMERx_TLC    RW */
/* Description:计数值载入寄存器
只能在定时器不使能时，对该寄存器做“写”操作。在定时器已经使能后，对该寄存器
进行“写”操作，会得到不可预知的结果。 */

/* Bit 0..31 : TIMERx TLC   RW */
/* Description: 载入计数器计数初值：计数器计数的初
值，该值应大于等于 0x4。 */
#define TIMERx_TLC_TLC_Pos (0UL) /*!< Position of TLC field. */
#define TIMERx_TLC_TLC_Msk (0xFFFFFFFFUL << TIMERx_TLC_TLC_Pos) /*!< Bit mask of TLC field. */
#define TIMERx_TLC_TLC_MIN_VAL (0x4)


/* Register:TIMER_TCV    R */
/* Description:读该寄存器可以得到计数器当前值。
 */
/* Bit 0..31 : TIMER TCV   RW */
/* Description: 当前计数器的值：显示当前计数器的计
数值。*/
#define TIMERx_TCV_TCV_Pos (0UL) /*!< Position of TCV field. */
#define TIMERx_TCV_TCV_Msk (0xFFFFFFFFUL << TIMERx_TCV_TCV_Pos) /*!< Bit mask of TCV field. */



/* Register:TIMERx_TCR    RW */
/* Description:控制寄存器
 */
/* Bit 0 : TIMERx TES   RW */

#define TIMERx_TCR_TES_Pos (0UL) /*!< Position of TES field. */
#define TIMERx_TCR_TES_Msk (0x1UL << TIMERx_TCR_TES_Pos) /*!< Bit mask of TES field. */
#define TIMERx_TCR_TES_Enable (1UL) /*!< 使能定时器 */
#define TIMERx_TCR_TES_Disable (0UL) /*!< 不使能定时器 */

/* Bit 1 : TIMER TMS   RW */
/* Description: 定时器模式选择：
*/
#define TIMERx_TCR_TMS_Pos (1UL) /*!< Position of TMS field. */
#define TIMERx_TCR_TMS_Msk (0x1UL << TIMERx_TCR_TMS_Pos) /*!< Bit mask of TMS field. */
#define TIMERx_TCR_TES_MODE_AUTO (0UL) /*!< 自运行模式 */
#define TIMERx_TCR_TES_MODE_USER_COUNTER (1UL) /*!< 用户定义计数模式 */

/* Bit 2 : TIMERx TIM   RW */
/* Description:定时器中断屏蔽：
*/
#define TIMERx_TCR_TIM_Pos (2UL) /*!< Position of TIM field. */
#define TIMERx_TCR_TIM_Msk (0x1UL << TIMERx_TCR_TIM_Pos) /*!< Bit mask of TIM field. */
#define TIMERx_TCR_TIM_Disable (0UL) /*!< 不屏蔽定时器中断 */
#define TIMERx_TCR_TIM_Enable (1UL) /*!< 屏蔽定时器中断 */



/* Register:TIMERx_TIC    R */
/* Description:中断清除寄存器
 */
/* Bit 0 : TIMERx TIC   R */
/* Description: 中断清除：读该寄存器将清除定时器中断，
因此该寄存器的值也被清 0。
*/
#define TIMERx_TIC_TIC_Pos (0UL) /*!< Position of TIC field. */
#define TIMERx_TIC_TIC_Msk (0x1UL << TIMERx_TIC_TIC_Pos) /*!< Bit mask of TIC field. */


/* Register:TIMERx_TIS    R */
/* Description:中断状态寄存器
 */
/* Bit 0 : TIMERx TIS   R */
/* Description: 中断状态标志：标志定时器的中断状态，若
中断被屏蔽，则该值标志中断屏蔽后的中断
状态。对该寄存器进行读操作不会影响定时
器的中断。
*/
#define TIMERx_TIS_TIS_Pos (0UL) /*!< Position of TIS field. */
#define TIMERx_TIS_TIS_Msk (0x1UL << TIMERx_TIS_TIS_Pos) /*!< Bit mask of TIS field. */
#define TIMERx_TIS_TIS_Generated (1UL) /*!< 定时器发生中断 */
#define TIMERx_TIS_TIS_NotGenerated (0UL) /*!< 定时器没有发生中断 */


/* Register:TIMER_TIS    R */
/* Description:全局中断状态寄存器
标志定时器经过中断屏蔽后的所有的中断状态，对该寄存器进行读操作可以得到 4 个
定时器的中断状态，且不会影响定时器的中断。
 */
/* Bit 0 : TIMER TIS0   R */
/* Description: TIMER0 中断状态标志：标志定时器经过中
断屏蔽后的中断状态，对该位进行读操作不
会影响定时器的中断。
*/
#define TIMER_TIS_TIS0_Pos (0UL) /*!< Position of TIS0 field. */
#define TIMER_TIS_TIS0_Msk (0x1UL << TIMER_TIS_TIS0_Pos) /*!< Bit mask of TIS0 field. */
#define TIMER_TIS_TIS0_Generated (1UL) /*!< 定时器0发生中断 */
#define TIMER_TIS_TIS0_NotGenerated (0UL) /*!< 定时器0没有发生中断 */

/* Bit 1 : TIMER TIS1   R */
/* Description: TIMER1 中断状态标志：标志定时器经过中
断屏蔽后的中断状态，对该位进行读操作不
会影响定时器的中断。
*/
#define TIMER_TIS_TIS1_Pos (1UL) /*!< Position of TIS1 field. */
#define TIMER_TIS_TIS1_Msk (0x1UL << TIMER_TIS_TIS1_Pos) /*!< Bit mask of TIS1 field. */
#define TIMER_TIS_TIS1_Generated (1UL) /*!< 定时器1发生中断 */
#define TIMER_TIS_TIS1_NotGenerated (0UL) /*!< 定时器1没有发生中断 */

/* Bit 0 : TIMER TIS2   R */
/* Description: TIMER2 中断状态标志：标志定时器经过中
断屏蔽后的中断状态，对该位进行读操作不
会影响定时器的中断。
*/
#define TIMER_TIS_TIS2_Pos (2UL) /*!< Position of TIS2 field. */
#define TIMER_TIS_TIS2_Msk (0x1UL << TIMER_TIS_TIS2_Pos) /*!< Bit mask of TIS2 field. */
#define TIMER_TIS_TIS2_Generated (1UL) /*!< 定时器2发生中断 */
#define TIMER_TIS_TIS2_NotGenerated (0UL) /*!< 定时器2没有发生中断 */



/* Register:TIMER_TIC    R */
/* Description:全局中断清除寄存器
对全局中断清除寄存器进行读操作会清除 4 个定时器的全部中断。
 */
/* Bit 0 : TIMER TIC0   R */
/* Description: TIMER0 中断清除：读该位将清除定时器 0
中断，因此该位的值也被清 0。
*/
#define TIMER_TIC_TIC0_Pos (0UL) /*!< Position of TIS field. */
#define TIMER_TIC_TIC0_Msk (0x1UL << TIMER_TIC_TIC0_Pos) /*!< Bit mask of TIC0 field. */
#define TIMER_TIC_TIC0_Generated (1UL) /*!< 定时器0发生中断 */
#define TIMER_TIC_TIC0_NotGenerated (0UL) /*!< 定时器0没有发生中断 */

/* Bit 1 : TIMER TIS1   R */
/* Description: TIMER1 中断清除：读该位将清除定时器 0
中断，因此该位的值也被清 0。
*/
#define TIMER_TIC_TIC1_Pos (1UL) /*!< Position of TIS1 field. */
#define TIMER_TIC_TIC1_Msk (0x1UL << TIMER_TIC_TIC1_Pos) /*!< Bit mask of TIS1 field. */
#define TIMER_TIC_TIC1_Generated (1UL) /*!< 定时器1发生中断 */
#define TIMER_TIC_TIC1_NotGenerated (0UL) /*!< 定时器1没有发生中断 */

/* Bit 0 : TIMER TIC2   R */
/* Description: TIMER2 中断清除：读该位将清除定时器 0
中断，因此该位的值也被清 0。
*/
#define TIMER_TIC_TIC2_Pos (2UL) /*!< Position of TIS2 field. */
#define TIMER_TIC_TIC2_Msk (0x1UL << TIMER_TIC_TIC2_Pos) /*!< Bit mask of TIS2 field. */
#define TIMER_TIC_TIC2_Generated (1UL) /*!< 定时器2发生中断 */
#define TIMER_TIC_TIC2_NotGenerated (0UL) /*!< 定时器2没有发生中断 */



/* Register:TIMER_RTIS    R */
/* Description:全局中断状态寄存器
标志定时器没有经过中断屏蔽的所有的中断状态，对该寄存器进行读操作可以得到 4
个定时器的原始中断状态，且不会影响定时器的中断。
 */
/* Bit 0 : TIMER RTIS0   R */
/* Description: TIMER0 原始中断状态标志：标志定时器经过中
断屏蔽后的中断状态，对该位进行读操作不
会影响定时器的中断。
*/
#define TIMER_RTIS_RTIS0_Pos (0UL) /*!< Position of RTIS0 field. */
#define TIMER_RTIS_TIS0_Msk (0x1UL << TIMER_RTIS_RTIS0_Pos) /*!< Bit mask of RTIS0 field. */
#define TIMER_RTIS_RTIS0_Generated (1UL) /*!< 定时器0发生中断 */
#define TIMER_RTIS_RTIS0_NotGenerated (0UL) /*!< 定时器0没有发生中断 */

/* Bit 1 : TIMER RTIS1   R */
/* Description: TIMER1 原始中断状态标志：标志定时器经过中
断屏蔽后的中断状态，对该位进行读操作不
会影响定时器的中断。
*/
#define TIMER_RTIS_RTIS1_Pos (1UL) /*!< Position of RTIS1 field. */
#define TIMER_RTIS_RTIS1_Msk (0x1UL << TIMER_RTIS_RTIS1_Pos) /*!< Bit mask of RTIS1 field. */
#define TIMER_RTIS_RTIS1_Generated (1UL) /*!< 定时器1发生中断 */
#define TIMER_RTIS_RTIS1_NotGenerated (0UL) /*!< 定时器1没有发生中断 */

/* Bit 0 : TIMER TIS2   R */
/* Description: TIMER2 原始中断状态标志：标志定时器经过中
断屏蔽后的中断状态，对该位进行读操作不
会影响定时器的中断。
*/
#define TIMER_RTIS_RTIS2_Pos (2UL) /*!< Position of RTIS2 field. */
#define TIMER_RTIS_RTIS2_Msk (0x1UL << TIMER_RTIS_RTIS2_Pos) /*!< Bit mask of RTIS2 field. */
#define TIMER_RTIS_RTIS2_Generated (1UL) /*!< 定时器2发生中断 */
#define TIMER_RTIS_RTIS2_NotGenerated (0UL) /*!< 定时器2没有发生中断 */

/////////////////////////////////////////////////////////////////////// XINCHIP TIMER END  //////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////// //////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////// XINCHIP RTC   //////////////////////////////////////////////////////

/* Peripheral: RTC */
/* Description: RTC Interface  */

/* Register:RTC_CCVR    R */
/* Description: RTC 当前计数值寄存器 
 RTC 读该寄存器可以得到计数器当前计数值。 
当前计数值：
用于记录计数器当前计数值。
Bit 31：17：天计数值（0－32767）
Bit 16：12：小时计数值（0－23）
Bit 11：6：分计数值（0－59）
Bit 5：0：秒计数值（0－59） 
*/
/* Bit 0..5 : RTC CCVR_SEC   R */
/* Description: 当前秒计数值 */
#define RTC_CCVR_SEC_Pos (0UL) /*!< Position of CCVR_SEC field. */
#define RTC_CCVR_SEC_Msk (0x3FUL << RTC_CCVR_SEC_Pos) /*!< Bit mask of CCVR_SEC field. */

/* Bit 6..11 : RTC CCVR_MIN   R */
/* Description: 当前秒计数值 */
#define RTC_CCVR_MIN_Pos (6UL) /*!< Position of CCVR_MIN field. */
#define RTC_CCVR_MIN_Msk (0x3FUL << RTC_CCVR_MIN_Pos) /*!< Bit mask of CCVR_MIN field. */

/* Bit 12..16 : RTC CCVR_HOUR   R */
/* Description: 当前秒计数值 */
#define RTC_CCVR_HOUR_Pos (12UL) /*!< Position of CCVR_HOUR field. */
#define RTC_CCVR_HOUR_Msk (0x1FUL << RTC_CCVR_HOUR_Pos) /*!< Bit mask of CCVR_HOUR field. */

/* Bit 17..31: RTC CCVR_DAY   R */
/* Description: 当前秒计数值 */
#define RTC_CCVR_DAY_Pos (17UL) /*!< Position of CCVR_DAY field. */
#define RTC_CCVR_DAY_Msk (0x7FFFUL << RTC_CCVR_DAY_Pos) /*!< Bit mask of CCVR_DAY field. */



/* Register:RTC_CLR  WR*/
/* Description: RTC 计数器初始值设置寄存器 
 RTC 读该寄存器可以得到计数器当前计数值。 */

/* Bit 0..5 : RTC CLR_SEC   R */
/* Description: 当前秒计数值 */
#define RTC_CLR_SEC_Pos (0UL) /*!< Position of CLR_SEC field. */
#define RTC_CLR_SEC_Msk (0x3FUL << RTC_CLR_SEC_Pos) /*!< Bit mask of CLR_SEC field. */

/* Bit 6..11 : RTC CLR_MIN   R */
/* Description: 当前秒计数值 */
#define RTC_CLR_MIN_Pos (6UL) /*!< Position of CLR_MIN field. */
#define RTC_CLR_MIN_Msk (0x3FUL << RTC_CLR_MIN_Pos) /*!< Bit mask of CLR_MIN field. */

/* Bit 12..16 : RTC CLR_HOUR   R */
/* Description: 当前秒计数值 */
#define RTC_CLR_HOUR_Pos (12UL) /*!< Position of CLR_HOUR field. */
#define RTC_CLR_HOUR_Msk (0x1FUL << RTC_CLR_HOUR_Pos) /*!< Bit mask of CLR_HOUR field. */

/* Bit 17..31: RTC CLR_DAY   R */
/* Description: 当前秒计数值 */
#define RTC_CLR_DAY_Pos (17UL) /*!< Position of CLR_DAY field. */
#define RTC_CLR_DAY_Msk (0x7FFFUL << RTC_CLR_DAY_Pos) /*!< Bit mask of CLR_DAY field. */



/* Register:RTC_CMR  WR*/
/* Description: RTC 定时匹配寄存器 
 用于设置定时中断的发生条件。 */

/* Bit 0..5 : RTC Second Match   WR */
/* Description:用于设置中断产生时的秒计数 */
#define RTC_CMR_SECOND_MATCH_Pos (0UL) /*!< Position of Second Match field. */
#define RTC_CMR_SECOND_MATCH_Msk (0x3FUL << RTC_CMR_SECOND_MATCH_Pos) /*!< Bit mask of Second Match field. */

/* Bit 6..11 : RTC Minute Match   WR */
/* Description:用于设置中断产生时的分钟计数 */
#define RTC_CMR_MINUTE_MATCH_Pos (6UL) /*!< Position of Minute Match field. */
#define RTC_CMR_MINUTE_MATCH_Msk (0x3FUL << RTC_CMR_MINUTE_MATCH_Pos) /*!< Bit mask of Minute Match field. */

/* Bit 12..16 : RTC Hour Match   WR */
/* Description:用于设置中断产生时的时计数 */
#define RTC_CMR_HOUR_MATCH_Pos (12UL) /*!< Position of Hour Match field. */
#define RTC_CMR_HOUR_MATCH_Msk (0x1FUL << RTC_CMR_HOUR_MATCH_Pos) /*!< Bit mask of Hour Match field. */


// Monday, Tuesday, Wednesday, Thursday, Friday, Saturday,Sunday
/* Bit 17..23 : RTC Week Match   WR */
/* Description:用于设置中断产生时的时计数 */
#define RTC_CMR_WEEK_MATCH_Pos (17UL) /*!< Position of Week Match field. */
#define RTC_CMR_WEEK_MATCH_Msk (0x3FUL << RTC_CMR_WEEK_MATCH_Pos) /*!< Bit mask of Week Match field. */

#define RTC_CMR_WEEK_MATCH_Sunday_Pos (17UL) /*!< Position of Week Match Sunday field. */
#define RTC_CMR_WEEK_MATCH_Sunday_Msk (0x1UL << RTC_CMR_WEEK_MATCH_Sunday_Pos) /*!< Bit mask of Week  Match Sunday field. */

#define RTC_CMR_WEEK_MATCH_Monday_Pos (18UL) /*!< Position of Week Match Monday field. */
#define RTC_CMR_WEEK_MATCH_Monday_Msk (0x1UL << RTC_CMR_WEEK_MATCH_Monday_Pos) /*!< Bit mask of Week Match Monday field. */

#define RTC_CMR_WEEK_MATCH_Tuesday_Pos (19UL) /*!< Position of Week Match Tuesday field. */
#define RTC_CMR_WEEK_MATCH_Tuesday_Msk (0x1UL << RTC_CMR_WEEK_MATCH_Tuesday_Pos) /*!< Bit mask of Week Match Tuesday field. */

#define RTC_CMR_WEEK_MATCH_Wednesday_Pos (20UL) /*!< Position of Week Match Wednesday field. */
#define RTC_CMR_WEEK_MATCH_Wednesday_Msk (0x1UL << RTC_CMR_WEEK_MATCH_Wednesday_Pos) /*!< Bit mask of Week Match Wednesday field. */

#define RTC_CMR_WEEK_MATCH_Thursday_Pos (21UL) /*!< Position of Week Match Thursday field. */
#define RTC_CMR_WEEK_MATCH_Thursday_Msk (0x1UL << RTC_CMR_WEEK_MATCH_Thursday_Pos) /*!< Bit mask of Week Match Thursday field. */

#define RTC_CMR_WEEK_MATCH_Friday_Pos (22UL) /*!< Position of Week Match Friday field. */
#define RTC_CMR_WEEK_MATCH_Friday_Msk (0x1UL << RTC_CMR_WEEK_MATCH_Friday_Pos) /*!< Bit mask of Week Match Friday field. */

#define RTC_CMR_WEEK_MATCH_Saturday_Pos (23UL) /*!< Position of Week Match Saturday field. */
#define RTC_CMR_WEEK_MATCH_Saturday_Msk (0x1UL << RTC_CMR_WEEK_MATCH_Saturday_Pos) /*!< Bit mask of Week Match Saturday field. */


/* Register:RTC_ICR  WR*/
/* Description: 中断控制寄存器 */

/* Bit 0 : RTC DaE  WR */
/* Description:天中断使能：
设置是否使能天中断 */
#define RTC_ICR_DaE_Pos (0UL) /*!< Position of DaE field. */
#define RTC_ICR_DaE_Msk (0x1UL << RTC_ICR_DaE_Pos) /*!< Bit mask of DaE field. */
#define RTC_ICR_DaE_Enable (1UL) /*!< 使能天中断 */
#define RTC_ICR_DaE_Disable (0UL) /*!< 不使能天中断 */

/* Bit 1 : RTC HoE  WR */
/* Description:小时中断使能：
设置是否使能小时中断 */
#define RTC_ICR_HoE_Pos (1UL) /*!< Position of HoE field. */
#define RTC_ICR_HoE_Msk (0x1UL << RTC_ICR_HoE_Pos) /*!< Bit mask of HoE field. */
#define RTC_ICR_HoE_Enable (1UL) /*!< 使能小时中断 */
#define RTC_ICR_HoE_Disable (0UL) /*!< 不使能小时中断 */

/* Bit 2 : RTC MiE  WR */
/* Description:分中断使能：
设置是否使能分中断 */
#define RTC_ICR_MiE_Pos (2UL) /*!< Position of MiE field. */
#define RTC_ICR_MiE_Msk (0x1UL << RTC_ICR_MiE_Pos) /*!< Bit mask of MiE field. */
#define RTC_ICR_MiE_Enable (1UL) /*!< 使能分中断 */
#define RTC_ICR_MiE_Disable (0UL) /*!< 不使能分中断 */

/* Bit 3 : RTC SeE  WR */
/* Description:秒中断使能：
设置是否使能秒中断 */
#define RTC_ICR_SeE_Pos (3UL) /*!< Position of SeE field. */
#define RTC_ICR_SeE_Msk (0x1UL << RTC_ICR_SeE_Pos) /*!< Bit mask of SeE field. */
#define RTC_ICR_SeE_Enable (1UL) /*!< 使能秒中断 */
#define RTC_ICR_SeE_Disable (0UL) /*!< 不使能秒中断 */



/* Bit 4 : RTC T2E  WR */
/* Description:定时中断 2 使能：
设置是否使能定时中断 2 */
#define RTC_ICR_T2E_Pos (4UL) /*!< Position of T2E field. */
#define RTC_ICR_T2E_Msk (0x1UL << RTC_ICR_T2E_Pos) /*!< Bit mask of T2E field. */
#define RTC_ICR_T2E_Enable (1UL) /*!< 使能定时中断 2 */
#define RTC_ICR_T2E_Disable (0UL) /*!< 不使能定时中断 2 */

/* Bit 5 : RTC T1E  WR */
/* Description:定时中断 1 使能：
设置是否使能定时中断 1 */
#define RTC_ICR_T1E_Pos (5UL) /*!< Position of T2E field. */
#define RTC_ICR_T1E_Msk (0x1UL << RTC_ICR_T1E_Pos) /*!< Bit mask of T2E field. */
#define RTC_ICR_T1E_Enable (1UL) /*!< 使能定时中断 1 */
#define RTC_ICR_T1E_Disable (0UL) /*!< 不使能定时中断 1 */

/* Bit 6 : RTC T3E  WR */
/* Description:定时中断 3 使能：
设置是否使能定时中断 3 */
#define RTC_ICR_T3E_Pos (6UL) /*!< Position of T3E field. */
#define RTC_ICR_T3E_Msk (0x1UL << RTC_ICR_T3E_Pos) /*!< Bit mask of T3E field. */
#define RTC_ICR_T3E_Enable (1UL) /*!< 使能定时中断 3 */
#define RTC_ICR_T3E_Disable (0UL) /*!< 不使能定时中断 3 */


/* Bit 7 : RTC MASK  WR */
/* Description:中断屏蔽：
设置是否屏蔽中断输出 */
#define RTC_ICR_MASK_ALL_Pos (7UL) /*!< Position of MASK field. */
#define RTC_ICR_MASK_ALL_Msk (0x1UL << RTC_ICR_MASK_ALL_Pos) /*!< Bit mask of MASK field. */
#define RTC_ICR_MASK_ALL_Enable (1UL) /*!< 屏蔽所有中断输出 */
#define RTC_ICR_MASK_ALL_Disable (0UL) /*!< 不屏蔽中断输出  */

/* Bit 8 : RTC CntE  WR */
/* Description:计数器使能：
控制计数器是否使能。*/
#define RTC_ICR_CntE_Pos (8UL) /*!< Position of CntE field. */
#define RTC_ICR_CntE_Msk (0x1UL << RTC_ICR_CntE_Pos) /*!< Bit mask of CntE field. */
#define RTC_ICR_CntE_Enable (1UL) /*!< 计数器使能 */
#define RTC_ICR_CntE_Disable (0UL) /*!< 计数器不使能  */



/* Register:RTC_ISR  WR*/
/* Description: 中断状态寄存器 
该寄存器用于显示 RTC 当前中断状态。*/

/* Bit 0 : RTC DaF  WR */
/* Description:中断状态：
该位是天中断的中断状态
*/
#define RTC_ISR_DaF_Pos (0UL) /*!< Position of DaF field. */
#define RTC_ISR_DaF_Msk (0x1UL << RTC_ISR_DaF_Pos) /*!< Bit mask of DaF field. */
#define RTC_ISR_DaF_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_DaF_NotGenerated (0UL) /*!< 没有发生中断  */

/* Bit 1 : RTC  HoF  WR */
/* Description:中断状态：
该位是小时中断的中断状态
*/
#define RTC_ISR_HoF_Pos (1UL) /*!< Position of HoF field. */
#define RTC_ISR_HoF_Msk (0x1UL << RTC_ISR_HoF_Pos) /*!< Bit mask of HoF field. */
#define RTC_ISR_HoF_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_HoF_NotGenerated (0UL) /*!< 没有发生中断  */

/* Bit 2 : RTC  MiF  WR */
/* Description:中断状态：
该位是分中断的中断状态
*/
#define RTC_ISR_MiF_Pos (2UL) /*!< Position of MiF field. */
#define RTC_ISR_MiF_Msk (0x1UL << RTC_ISR_MiF_Pos) /*!< Bit mask of MiF field. */
#define RTC_ISR_MiF_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_MiF_NotGenerated (0UL) /*!< 没有发生中断  */

/* Bit 3 : RTC  SeF  WR */
/* Description:中断状态：
该位是秒中断的中断状态
*/
#define RTC_ISR_SeF_Pos (3UL) /*!< Position of SeF field. */
#define RTC_ISR_SeF_Msk (0x1UL << RTC_ISR_SeF_Pos) /*!< Bit mask of SeF field. */
#define RTC_ISR_SeF_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_SeF_NotGenerated (0UL) /*!< 没有发生中断  */

/* Bit 4 : RTC  T2F  WR */
/* Description:中断状态：
该位是是定时中断 2的中断状态
*/
#define RTC_ISR_T2F_Pos (4UL) /*!< Position of T2F field. */
#define RTC_ISR_T2F_Msk (0x1UL << RTC_ISR_T2F_Pos) /*!< Bit mask of T2F field. */
#define RTC_ISR_T2F_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_T2F_NotGenerated (0UL) /*!< 没有发生中断  */

/* Bit 5 : RTC  T1F  WR */
/* Description:中断状态：
该位是是定时中断 1的中断状态
*/
#define RTC_ISR_T1F_Pos (5UL) /*!< Position of T1F field. */
#define RTC_ISR_T1F_Msk (0x1UL << RTC_ISR_T1F_Pos) /*!< Bit mask of T1F field. */
#define RTC_ISR_T1F_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_T1F_NotGenerated (0UL) /*!< 没有发生中断  */

/* Bit 6 : RTC  T3F  WR */
/* Description:中断状态：
该位是是定时中断 1的中断状态
*/
#define RTC_ISR_T3F_Pos (6UL) /*!< Position of T3F field. */
#define RTC_ISR_T3F_Msk (0x1UL << RTC_ISR_T3F_Pos) /*!< Bit mask of T3F field. */
#define RTC_ISR_T3F_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_T3F_NotGenerated (0UL) /*!< 没有发生中断  */



/* Register:RTC_EOI  WR*/
/* Description: 此寄存器用于清除 RTC 中断。
*/

/* Bit 0 : RTC EOI_da  WR */
/* Description:清天中断：
向寄存器该位写 1 可以清除天中断
*/
#define RTC_EOI_Da_Pos (0UL) /*!< Position of DaF field. */
#define RTC_EOI_Da_Msk (0x1UL << RTC_EOI_Da_Pos) /*!< Bit mask of DaF field. */
#define RTC_EOI_Da_Clear (1UL) /*!< 清天中断： */


/* Bit 1 : RTC  EOI_Ho  WR */
/* Description:中断状态：
该位是小时中断的中断状态
*/
#define RTC_EOI_Ho_Pos (1UL) /*!< Position of EOI_Ho field. */
#define RTC_EOI_Ho_Msk (0x1UL << RTC_EOI_HoF_Pos) /*!< Bit mask of EOI_Ho field. */
#define RTC_EOI_Ho_Clear (1UL) /*!< 清小时中断 */

/* Bit 2 : RTC  EOI_Mi  WR */
/* Description:中断状态：
该位是分中断的中断状态
*/
#define RTC_EOI_Mi_Pos (2UL) /*!< Position of EOI_Mi field. */
#define RTC_EOI_Mi_Msk (0x1UL << RTC_EOI_Mi_Pos) /*!< Bit mask of EOI_Mi field. */
#define RTC_EOI_Mi_Clear (1UL) /*!< 清分中断 */

/* Bit 3 : RTC  EOI_Se  WR */
/* Description:中断状态：
该位是秒中断的中断状态
*/
#define RTC_EOI_Se_Pos (3UL) /*!< Position of EOI_Se field. */
#define RTC_EOI_Se_Msk (0x1UL << RTC_EOI_Se_Pos) /*!< Bit mask of EOI_Se field. */
#define RTC_EOI_Se_Clear (1UL) /*!< 清秒中断 */

/* Bit 4 : RTC  EOI_T2  WR */
/* Description:中断状态：
该位是是定时中断 2的中断状态
*/
#define RTC_EOI_T2_Pos (4UL) /*!< Position of EOI_T2 field. */
#define RTC_EOI_T2_Msk (0x1UL << RTC_EOI_T2_Pos) /*!< Bit mask of EOI_T2 field. */
#define RTC_EOI_T2_Clear (1UL) /*!< 清定时中断 2： */

/* Bit 5 : RTC  EOI_T1  WR */
/* Description:中断状态：
该位是是定时中断 1的中断状态
*/
#define RTC_EOI_T1_Pos (5UL) /*!< Position of EOI_T1 field. */
#define RTC_EOI_T1_Msk (0x1UL << RTC_EOI_T1_Pos) /*!< Bit mask of EOI_T1 field. */
#define RTC_EOI_T1_Clear (1UL) /*!< 清定时中断 1： */

/* Bit 6 : RTC  EOI_T3  WR */
/* Description:中断状态：
该位是是定时中断 1的中断状态
*/
#define RTC_EOI_T3_Pos (6UL) /*!< Position of EOI_T3 field. */
#define RTC_EOI_T3_Msk (0x1UL << RTC_EOI_T3_Pos) /*!< Bit mask of EOI_T3 field. */
#define RTC_EOI_T3_Clear (1UL) /*!< 清定时中断 3： */



/* Register:RTC_WVR  R*/
/* Description: 当前周计数器值寄存器
*/

/* Bit 0..2 : RTC WVR  R */
/* Description:记录当前日期为星期几：
*/
#define RTC_WVR_Pos (0UL) /*!< Position of WVR field. */
#define RTC_WVR_Msk (0x7UL << RTC_WVR_Pos) /*!< Bit mask of WVR field. */

/* Register:RTC_WLR  WR*/
/* Description: 周计数器初值设置寄存器
*/

/* Bit 0..2 : RTC WLR  WR */
/* Description:设置写入 CLR 寄存器的天计数值对应星期几：
(该寄存器和 CLR 有相关性，写 CLR 时注意
WLR 寄存器的值是否匹配)
*/
#define RTC_WLR_Pos (0UL) /*!< Position of WLR field. */
#define RTC_WLR_Msk (0x7UL << RTC_WLR_Pos) /*!< Bit mask of WLR field. */

// Sunday,Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, 

#define RTC_WVR_WLR_Sunday (0UL) /*!< 周日 */
#define RTC_WVR_WLR_Monday (1UL) /*!< 周一 */
#define RTC_WVR_WLR_Tuesday (2UL) /*!< 周二 */
#define RTC_WVR_WLR_Wednesday (3UL) /*!< 周三 */
#define RTC_WVR_WLR_Thursday (4UL) /*!< 周四 */
#define RTC_WVR_WLR_Friday (5UL) /*!< 周五 */
#define RTC_WVR_WLR_Saturday (6UL) /*!< 周六 */



/* Register:RTC_RAW_LIMIT  RW*/
/* Description: 计数频率设置寄存器
*/
/* Bit 0..15 : RTC RAW_LIMIT  WR */
/* Description:RAW_LIMIT 的值为计数 1 秒所需要的
rtc_mclk 时钟周期数。例如，当 rtc_mclk 时
钟频率为 32768Hz 时，计数 1 秒需要 32768
个 rtc_mclk 时钟周期。当计数时钟不等于
32768Hz 时，可以设置 RAW_LIMIT 的值为
实际采用的计数时钟的频率数。
*/
#define RTC_RAW_LIMIT_Pos (0UL) /*!< Position of RAW_LIMIT field. */
#define RTC_RAW_LIMIT_Msk (0xFFFFUL << RTC_RAW_LIMIT_Pos) /*!< Bit mask of RAW_LIMIT field. */

/* Register:RTC_SECOND_LIMIT  RW*/
/* Description: 秒计数上限控制寄存器
*/
/* Bit 0..5 : RTC SECOND_LIMIT  WR */
/* Description:设置秒计数的上限，默认值为 60。主要
用于调试，设置为较小的值，可以减少等
待时间。
*/
#define RTC_SECOND_LIMIT_Pos (0UL) /*!< Position of SECOND_LIMIT field. */
#define RTC_SECOND_LIMIT_Msk (0x1FUL << RTC_SECOND_LIMIT_Pos) /*!< Bit mask of SECOND_LIMIT field. */

/* Register:RTC_MINUTE_LIMIT  RW*/
/* Description: 秒计数上限控制寄存器
*/
/* Bit 0..5 : RTC MINUTE_LIMIT  WR */
/* Description:设置分计数的上限，默认值为 60。主要
用于调试，设置为较小的值，可以减少等
待时间。
*/
#define RTC_MINUTE_LIMIT_Pos (0UL) /*!< Position of MINUTE_LIMIT field. */
#define RTC_MINUTE_LIMIT_Msk (0x1FUL << RTC_MINUTE_LIMIT_Pos) /*!< Bit mask of MINUTE_LIMIT field. */


/* Register:RTC_HOUR_LIMIT  RW*/
/* Description: 秒计数上限控制寄存器
*/
/* Bit 0..4 : RTC HOUR_LIMIT  WR */
/* Description:设置时计数的上限，默认值为 24。主要
用于调试，设置为较小的值，可以减少等
待时间。
*/
#define RTC_HOUR_LIMIT_Pos (0UL) /*!< Position of HOUR_LIMIT field. */
#define RTC_HOUR_LIMIT_Msk (0xFUL << RTC_HOUR_LIMIT_Pos) /*!< Bit mask of HOUR_LIMIT field. */


/* Register:RTC_ISR_RAW  R*/
/* Description: 中断原始状态寄存器
*/
/* Bit 0 : RTC DaF_raw  R */
/* Description:中断原始状态：
该位是天中断的原始中断状态
*/
#define RTC_ISR_RAW_DaF_raw_Pos (0UL) /*!< Position of DaF_raw field. */
#define RTC_ISR_RAW_DaF_raw_Msk (0x1UL << RTC_ISR_RAW_DaF_raw_Pos) /*!< Bit mask of DaF_raw field. */
#define RTC_ISR_RAW_DaF_raw_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_RAW_DaF_raw_NotGenerated (0UL) /*!< 没有发生中断  */

/* Bit 1 : RTC HoF_raw  R */
/* Description:中断原始状态：
该位是小时中断的原始中断状态
*/
#define RTC_ISR_RAW_HoF_raw_Pos (1UL) /*!< Position of HoF_raw field. */
#define RTC_ISR_RAW_HoF_raw_Msk (0x1UL << RTC_ISR_RAW_HoF_raw_Pos) /*!< Bit mask of HoF_raw field. */
#define RTC_ISR_RAW_HoF_raw_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_RAW_HoF_raw_NotGenerated (0UL) /*!< 没有发生中断  */

/* Bit 2 : RTC MiF_raw  R */
/* Description:中断原始状态：
该位是分中断的原始中断状态
*/
#define RTC_ISR_RAW_MiF_raw_Pos (2UL) /*!< Position of MiF_raw field. */
#define RTC_ISR_RAW_MiF_raw_Msk (0x1UL << RTC_ISR_RAW_MiF_raw_Pos) /*!< Bit mask of MiF_raw field. */
#define RTC_ISR_RAW_MiF_raw_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_RAW_MiF_raw_NotGenerated (0UL) /*!< 没有发生中断  */

/* Bit 3 : RTC SeF_raw  R */
/* Description:中断原始状态：
该位是秒中断的原始中断状态
*/
#define RTC_ISR_RAW_SeF_raw_Pos (3UL) /*!< Position of SeF_raw field. */
#define RTC_ISR_RAW_SeF_raw_Msk (0x1UL << RTC_ISR_RAW_SeF_raw_Pos) /*!< Bit mask of SeF_raw field. */
#define RTC_ISR_RAW_SeF_raw_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_RAW_SeF_raw_NotGenerated (0UL) /*!< 没有发生中断  */

/* Bit 4 : RTC T2F_raw  R */
/* Description:中断原始状态：
该位是定时中断 2 的原始中断状态
*/
#define RTC_ISR_RAW_T2F_raw_Pos (4UL) /*!< Position of T2F_raw field. */
#define RTC_ISR_RAW_T2F_raw_Msk (0x1UL << RTC_ISR_RAW_T2F_raw_Pos) /*!< Bit mask of T2F_raw field. */
#define RTC_ISR_RAW_T2F_raw_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_RAW_T2F_raw_NotGenerated (0UL) /*!< 没有发生中断  */

/* Bit 5 : RTC T1F_raw  R */
/* Description:中断原始状态：
该位是定时中断 1 的原始中断状态
*/
#define RTC_ISR_RAW_T1F_raw_Pos (5UL) /*!< Position of T1F_raw field. */
#define RTC_ISR_RAW_T1F_raw_Msk (0x1UL << RTC_ISR_RAW_T2F_raw_Pos) /*!< Bit mask of T1F_raw field. */
#define RTC_ISR_RAW_T1F_raw_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_RAW_T1F_raw_NotGenerated (0UL) /*!< 没有发生中断  */

/* Bit 4 : RTC T3F_raw  R */
/* Description:中断原始状态：
该位是定时中断 3 的原始中断状态
*/
#define RTC_ISR_RAW_T3F_raw_Pos (6UL) /*!< Position of T3F_raw field. */
#define RTC_ISR_RAW_T3F_raw_Msk (0x1UL << RTC_ISR_RAW_T2F_raw_Pos) /*!< Bit mask of T3F_raw field. */
#define RTC_ISR_RAW_T3F_raw_Generated (1UL) /*!< 发生中断 */
#define RTC_ISR_RAW_T3F_raw_NotGenerated (0UL) /*!< 没有发生中断  */


/* Register:RTC_RVR  R*/
/* Description: 当前秒内计数值寄存器
*/
/* Bit 0..15 : RTC RVR  R */
/* Description:当前秒内计数值：
记录 1 秒计数器的计数值。
*/
#define RTC_RVR_Pos (0UL) /*!< Position of RVR field. */
#define RTC_RVR_Msk (0xFFFFUL << RTC_ISR_RAW_T2F_raw_Pos) /*!< Bit mask of RVR field. */


/* Register:AO_TIMER_CTL  WR*/
/* Description: 16 位计数器控制寄存器
*/
/* Bit 0..15 : RTC AO_TIMER_VALUE  WR */
/* Description:计数器目标值，从 0 计数到该值时产生中断
*/
#define RTC_AO_TIMER_CTL_AO_TIMER_VALUE_Pos (0UL) /*!< Position of AO_TIMER_VALUE field. */
#define RTC_AO_TIMER_CTL_AO_TIMER_VALUE_Msk (0xFFFFUL << RTC_AO_TIMER_CTL_AO_TIMER_VALUE_Pos) /*!< Bit mask of AO_TIMER_VALUE field. */

/* Bit 16 : RTC AO_TIMER_CLR  WR */
/* Description:中断清除
*/
#define RTC_AO_TIMER_CTL_AO_TIMER_CLR_Pos (16UL) /*!< Position of AO_TIMER_CLR field. */
#define RTC_AO_TIMER_CTL_AO_TIMER_CLR_Msk (0x1UL << RTC_AO_TIMER_CTL_AO_TIMER_CLR_Pos) /*!< Bit mask of AO_TIMER_CLR field. */
#define RTC_AO_TIMER_CTL_AO_TIMER_CLR_Clear (1UL) /*!< 中断清除 */

/* Bit 17 : RTC AO_TIMER_EN  WR */
/* Description:计数器使能
*/
#define RTC_AO_TIMER_CTL_AO_TIMER_EN_Pos (17UL) /*!< Position of AO_TIMER_EN field. */
#define RTC_AO_TIMER_CTL_AO_TIMER_EN_Msk (0x1UL << RTC_AO_TIMER_CTL_AO_TIMER_EN_Pos) /*!< Bit mask of AO_TIMER_EN field. */
#define RTC_AO_TIMER_CTL_AO_TIMER_EN_Enable (1UL) /*!< 计数器使能 */
#define RTC_AO_TIMER_CTL_AO_TIMER_EN_Disable (0UL) /*!< 计数器不使能 */

/* Bit 18 : RTC FREQ_TIMER_EN  WR */
/* Description:32K 校准计数器使能
*/
#define RTC_AO_TIMER_CTL_FREQ_TIMER_EN_Pos (18UL) /*!< Position of FREQ_TIMER_EN field. */
#define RTC_AO_TIMER_CTL_FREQ_TIMER_EN_Msk (0x1UL << RTC_AO_TIMER_CTL_FREQ_TIMER_EN_Pos) /*!< Bit mask of FREQ_TIMER_EN field. */
#define RTC_AO_TIMER_CTL_FREQ_TIMER_EN_Enable (1UL) /*!< 32K 校准计数器使能 */
#define RTC_AO_TIMER_CTL_FREQ_TIMER_EN_Disable (0UL) /*!< 32K 校准计数器不使能 */



/* Register:AO_ALL_INTR  R*/
/* Description: AO 中断寄存器
*/
/* Bit 0..4 : RTC AOGPIO_INTR  R */
/* Description:gpio0~4 中断标志
*/
#define RTC_AO_ALL_INTR_AOGPIO_INTR_Pos (0UL) /*!< Position of AOGPIO_INTR field. */
#define RTC_AO_ALL_INTR_AOGPIO_INTR_Msk (0x1FUL << RTC_AO_ALL_INTR_AOGPIO_INTR_Pos) /*!< Bit mask of AOGPIO_INTR field. */

/* Bit 5 : RTC AO_TIMER_INTR  R */
/* Description:gpio0~4 中断标志
*/
#define RTC_AO_ALL_INTR_AO_TIMER_INTR_Pos (5UL) /*!< Position of AO_TIMER_INTR field. */
#define RTC_AO_ALL_INTR_AO_TIMER_INTR_Msk (0x1UL << RTC_AO_ALL_INTR_AO_TIMER_INTR_Pos) /*!< Bit mask of AO_TIMER_INTR field. */


/* Register:FREQ_TIMER_VAL  R*/
/* Description: FREQ_TIMER
*/
/* Bit 0..23 : FREQ_TIMER_VALUE  R */
/* Description:32K 校准计数周期
*/
#define RTC_FREQ_TIMER_VAL_Pos (0UL) /*!< Position of FREQ_TIMER_VALUE field. */
#define RTC_FREQ_TIMER_VAL_Msk (0xFFFFFFUL << RTC_FREQ_TIMER_VAL_Pos) /*!< Bit mask of FREQ_TIMER_VALUE field. */




/////////////////////////////////////////////////////////////////////// XINCHIP RTC END  //////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////// //////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////// XINCHIP WDT   //////////////////////////////////////////////////////

/* Peripheral: WDT */
/* Description: WDT Interface  */

/* Register: WDT_CR */
/* Description: WDT 控制寄存器 
该寄存器用于控制 WDT 使能和选择工作模式 */

/* Bit 0 : WDT WDT_EN   */
/* Description: WDT 使能：软件设置 WDT 使能后，任何对该
位写 0 的操作都是无效操作。该位只能被 M0
复位清除。 */
#define WDT_CR_WDT_EN_Pos (0UL) /*!< Position of WDT_EN field. */
#define WDT_CR_WDT_EN_Msk (0x1UL << WDT_CR_WDT_EN_Pos) /*!< Bit mask of WDT_EN field. */
#define WDT_CR_WDT_EN_Enable (1UL) /*!< WDT 不使能 */
#define WDT_CR_WDT_EN_Disable (0UL) /*!< WDT 使能 */

/* Bit 1 : WDT RMOD   */
/* Description: 工作模式   */
#define WDT_CR_RMOD_Pos (1UL) /*!< Position of RMOD field. */
#define WDT_CR_RMOD_Msk (0x1UL << WDT_CR_RMOD_Pos) /*!< Bit mask of RMOD field. */
#define WDT_CR_RMOD_Mode1 (1UL) /*!< 工作模式 1，先产生一个中断，如果在下一个超时来临之前软件没有清除中断，那么会产生复位。 */
#define WDT_CR_RMOD_Mode0 (0UL) /*!< 工作模式 0，直接产生复位信号送给 CPR */

/* Bit 2..4 : WDT RPL   */
/* Description: 复位脉冲宽度：
M0 复位信号，维持多少个 pclk 时钟周期。   */

#define WDT_CR_RPL_Pos (2UL) /*!< Position of RPL field. */
#define WDT_CR_RPL_Msk (0x7UL << WDT_CR_RPL_Pos) /*!< Bit mask of RPL field. */

#define WDT_CR_RPL_2pclk (0UL) /*!< 000：2 pclk 时钟周期 */
#define WDT_CR_RPL_4pclk (1UL) /*!< 001：4 pclk 时钟周期 */
#define WDT_CR_RPL_8pclk (2UL) /*!< 010：8 pclk 时钟周期 */
#define WDT_CR_RPL_16pclk (3UL) /*!< 011：16 pclk 时钟周期 */
#define WDT_CR_RPL_32pclk (4UL) /*!< 100：32 pclk 时钟周期 */
#define WDT_CR_RPL_64pclk (5UL) /*!< 101：64 pclk 时钟周期 */
#define WDT_CR_RPL_128pclk (6UL) /*!< 110：128 pclk 时钟周期 */
#define WDT_CR_RPL_256pclk (7UL) /*!< 111：256 pclk 时钟周期 */



/* Register: WDT_TORR */
/* Description: WDT 超时范围寄存器
该寄存器用于设置超时时间。 */

/* Bit 0..3 : WDT TOP   */
/* Description: 设置超时周期：此字段用于设置 WDT 装载的
计数器值。如果用户想在计数器计数过程中修
改此字段，重启计数器或发生超时，修改后的
值都会被装载入计数器。更详细的描述见 WDT
计数器装载， */
#define WDT_TORR_TOP_Pos (0UL) /*!< Position of WDT_EN field. */
#define WDT_TORR_TOP_Msk (0xFUL << WDT_TORR_TOP_Pos) /*!< Bit mask of WDT_EN field. */
#define WDT_TORR_TOP_VAL_0xFFFF (0UL) /*!< 0000：0xFFFF */
#define WDT_TORR_TOP_VAL_0x1FFFF (1UL) /*!< 0001：0x1FFFF */
#define WDT_TORR_TOP_VAL_0x3FFFF (2UL) /*!< 0010：0x3FFFF */
#define WDT_TORR_TOP_VAL_0x7FFFF (3UL) /*!< 0011：0x7FFFF */
#define WDT_TORR_TOP_VAL_0xFFFFF (4UL) /*!< 0100：0xFFFFF */
#define WDT_TORR_TOP_VAL_0x1FFFFF (5UL) /*!< 0101：0x1FFFFF */
#define WDT_TORR_TOP_VAL_0x3FFFFF (6UL) /*!< 0110：0x3FFFFF */
#define WDT_TORR_TOP_VAL_0x7FFFFF (7UL) /*!< 0111：0x7FFFFF */
#define WDT_TORR_TOP_VAL_0xFFFFFF (8UL) /*!< 1000：0xFFFFFF */
#define WDT_TORR_TOP_VAL_0x1FFFFFF (9UL) /*!< 1001：0x1FFFFFF */
#define WDT_TORR_TOP_VAL_0x3FFFFFF (10UL) /*!< 1010：0x3FFFFFF */
#define WDT_TORR_TOP_VAL_0x7FFFFFF (11UL) /*!< 1011：0x7FFFFFF */
#define WDT_TORR_TOP_VAL_0xFFFFFFF (12UL) /*!< 1100：0xFFFFFFF */
#define WDT_TORR_TOP_VAL_0x1FFFFFFF (13UL) /*!< 1101：0x1FFFFFFF */
#define WDT_TORR_TOP_VAL_0x3FFFFFFF (14UL) /*!< 1110：0x3FFFFFFF */
#define WDT_TORR_TOP_VAL_0x7FFFFFFF (15UL) /*!< 1111：0x7FFFFFFF */



/* Register: WDT_CCVR */
/* Description: 当前计数值寄存器
只读寄存器，用于显示计数器当前计数值。 */

/* Bit 0..31 : WDT CCVR   */
/* Description: 当前计数值：记录当前计数器的计数值。 */
#define WDT_CCVR_CCVR_Pos (0UL) /*!< Position of CCVR field. */
#define WDT_CCVR_CCVR_Msk (0xFFFFFFFUL << WDT_CCVR_CCVR_Pos) /*!< Bit mask of CCVR field. */


/* Register: WDT_CRR */
/* Description:计数器重启寄存器
该寄存器可以设置计数器重启。 */

/* Bit 0..7 : WDT CRR   */
/* Description: 当前计数值：记录当前计数器的计数值。 */
#define WDT_CRR_CRR_Pos (0UL) /*!< Position of CRR field. */
#define WDT_CRR_CRR_Msk (0xFFUL << WDT_CRR_CRR_Pos) /*!< Bit mask of CRR field. */
#define WDT_CRR_CRR_Enable (0x76UL) 
/* !< 重启计数器：用户可向该字段写入 0x76,
计数器将会重启，写其它任何值对该字段没有影响。
用户可以在任何时候对该寄存器进行写操作。
重启计数器会清除当前中断。 */



/* Register: WDT_STAT */
/* Description:中断状态寄存器
该寄存器用于显示 WDT 当前中断状态。 */

/* Bit 0 : WDT STAT   */
/* Description: 当前中断状态：显示 WDT 当前中断状态。 */
#define WDT_STAT_STAT_Pos (0UL) /*!< Position of STAT field. */
#define WDT_STAT_STAT_Msk (0x1UL << WDT_STAT_STAT_Pos) /*!< Bit mask of STAT field. */
#define WDT_STAT_STAT_NotGenerated (0x0UL) /*!< 0：没有发生中断 */
#define WDT_STAT_STAT_Generated (0x1UL) /*!< 1：发生中断 */



/* Register: WDT_ICR */
/* Description:中断清除寄存器
只读寄存器，用于清除 WDT 中断。 */

/* Bit 0 : WDT ICR   */
/* Description: 清除中断：读此寄存器会清除 WDT 当前中断。 */
#define WDT_ICR_ICR_Pos (0UL) /*!< Position of ICR field. */
#define WDT_ICR_ICR_Msk (0x1UL << WDT_ICR_ICR_Pos) /*!< Bit mask of ICR field. */
#define WDT_ICR_ICR_NotGenerated (0x0UL) /*!< 0：没有发生中断 */
#define WDT_ICR_ICR_Generated (0x1UL) /*!< 1：发生中断 */





/////////////////////////////////////////////////////////////////////// XINCHIP WDT END  //////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////// //////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////// XINCHIP PWM   //////////////////////////////////////////////////////
/* Peripheral: PWM */
/* Description: PWM  Interface */

/* Register: PWM_EN */
/* Description: PWM 使能寄存器 
*/

/* Bit 0 : PWM PWM_EN   */
/* Description:PWM 使能控制位。
此寄存器控制 PWM 内部计数
器及输出的工作状态 */
#define PWM_EN_Pos (0UL) /*!< Position of PWM_EN field. */
#define PWM_EN_Msk (0x1UL << PWM_EN_Pos) /*!< Bit mask of PWM_EN field. */
#define PWM_EN_Enable (1UL) /*!< PWM 使能 */
#define PWM_EN_Disable (0UL) /*!< PWM 不使能*/


/* Bit 1..2 : PWM PWM_MODE   */
/* Description:PWM PWM 占空比档位精度。
*/
#define PWM_PWM_MODE_Pos (1UL) /*!< Position of PWM_MODE field. */
#define PWM_PWM_MODE_Msk (0x3UL << PWM_PWM_MODE_Pos) /*!< Bit mask of PWM_EN field. */
#define PWM_PWM_MODE_ACC_100 (0UL) /*!< 0:占空比精度1/100*/
#define PWM_PWM_MODE_ACC_1000 (1UL) /*!<1:占空比精度1/1000*/
#define PWM_PWM_MODE_ACC_4000 (2UL) /*!<2:占空比精度1/4000*/


/* Register: PWM_UP */
/* Description: PWM 参数更新寄存器 
*/

/* Bit 0 : PWM UPDATE   */
/* Description:PWM 参数更新位。
向此位写 1 时，周期设置寄存
器和占空比设置寄存器中的新
值，只有在 PWM 不使能或者一
个完整的 PWM 波形输出后才
真正生效，然后此位自动清 0；
向此位写 0 无效。*/
#define PWM_UP_UPDATE_Pos (0UL) /*!< Position of UPDATE field. */
#define PWM_UP_UPDATE_Msk (0x1UL << PWM_UP_UPDATE_Pos) /*!< Bit mask of UPDATE field. */
#define PWM_UP_UPDATE_Enable (1UL) /*!< PWM UPDATE 使能 */
#define PWM_UP_UPDATE_Disable (0UL) /*!< 无效 */



/* Register: PWM_RST */
/* Description: PWM 复位寄存器 
*/

/* Bit 0 : PWM RESET   */
/* Description:PWM 模块复位
 * 此寄存器控制 PWM 复位操作 
*/
#define PWM_RST_RESET_Pos (0UL) /*!< Position of RESET field. */
#define PWM_RST_RESET_Msk (0x1UL << PWM_RST_RESET_Pos) /*!< Bit mask of RESET field. */
#define PWM_RST_RESET_Disable (0UL) /*!< PWM 模块正常工作 */
#define PWM_RST_RESET_Enable (1UL) /*!< PWM 模块进行复位 */



/* Register: PWM_P */
/* Description: PWM 周期设置寄存器
*/

/* Bit 0..7 : PWM PERIOD   */
/* Description:PWM PERIOD 用于控制周期计数器
的周期，在周期计数器增加到PERIOD 时，周期计数器归 0，
重新开始计数。
*/
#define PWM_P_PERIOD_Pos (0UL) /*!< Position of PERIOD field. */
#define PWM_P_PERIOD_Msk (0xFFUL << PWM_P_PERIOD_Pos) /*!< Bit mask of PERIOD field. */
#define PWM_P_PERIOD_MAX (0xFFUL)

/* Register: PWM_OCPY */
/* Description: PWM 占空比设置寄存器
*/

/* Bit 0..11 : PWM OCPY_RATIO   */
/* Description:OCPY_RATIO 用于控制占空
比计数器的占空比，占空比设
置寄存器用于和占空比计数器
进行比较，相等时，PWM 输出
为低电平；计数器由 99 归 0 时，
PWM 输出为高电平。调节
OCPY_RATIO 的值便可调节
占空比。
*/
#define PWM_OCPY_OCPY_RATIO_Pos (0UL) /*!< Position of OCPY_RATIO field. */
#define PWM_OCPY_OCPY_RATIO_Msk (0xFFFUL << PWM_OCPY_OCPY_RATIO_Pos) /*!< Bit mask of OCPY_RATIO field. */
#define PWM_OCPY_MAX_100 (100UL)
#define PWM_OCPY_MAX_1000 (1000UL)
#define PWM_OCPY_MAX_4000 (4000UL)


/* Register: PWM_COMP_EN */
/* Description: PWM 互补 PWM 输出、死区控制寄存器
*/

/* Bit 0 : PWM PWMCOMPEN   */
/* Description:使能 PWM 的互补信号输出
*/
#define PWM_COMP_EN_PWMCOMPEN_Pos (0UL) /*!< Position of PWMCOMPEN field. */
#define PWM_COMP_EN_PWMCOMPEN_Msk (0x1UL << PWM_COMP_EN_PWMCOMPEN_Pos) /*!< Bit mask of PWMCOMPEN field. */
#define PWM_COMP_EN_PWMCOMPEN_Disable (0UL) /*!< 1 使能 */
#define PWM_COMP_EN_PWMCOMPEN_Enable (1UL) /*!< 0 不使能 */


/* Register: PWM_COMP_TIME */
/* Description: PWM 互补 PWM 输出、死区控制寄存器
*/

/* Bit 0..2 : PWM PWMCOMPTIME   */
/* Description:使能 PWM 的互补信号输出
*/
#define PWM_COMP_TIME_PWMCOMPTIME_Pos (0UL) /*!< Position of PWMCOMPTIME field. */
#define PWM_COMP_TIME_PWMCOMPTIME_Pos_Msk (0xFFUL << PWM_COMP_TIME_PWMCOMPTIME_Pos) /*!< Bit mask of PWMCOMPTIME field. */
#define PWM_COMP_TIME_PWMCOMPTIME_VAL_1clk (0UL) /*!<0：死区为 1 个 pwm_clk 时钟周期 */
#define PWM_COMP_TIME_PWMCOMPTIME_VAL_2clk (1UL) /*!<1：死区为 2个 pwm_clk 时钟周期 */
#define PWM_COMP_TIME_PWMCOMPTIME_VAL_3clk (2UL) /*!<2：死区为 3 个 pwm_clk 时钟周期 */
#define PWM_COMP_TIME_PWMCOMPTIME_VAL_4clk (3UL) /*!<3：死区为 4 个 pwm_clk 时钟周期 */
#define PWM_COMP_TIME_PWMCOMPTIME_VAL_5clk (4UL) /*!<4：死区为 5 个 pwm_clk 时钟周期 */
#define PWM_COMP_TIME_PWMCOMPTIME_VAL_6clk (5UL) /*!<5：死区为 6 个 pwm_clk 时钟周期 */
#define PWM_COMP_TIME_PWMCOMPTIME_VAL_7clk (6UL) /*!<6：死区为 7 个 pwm_clk 时钟周期 */
#define PWM_COMP_TIME_PWMCOMPTIME_VAL_8clk (7UL) /*!<7：死区为 8 个 pwm_clk 时钟周期 */


///////////////////////////////////////////////////////////////////////XINCHIP I2C   //////////////////////////////////////////////////////


/* Peripheral: I2C */
/* Description: I2C compatible Two-Wire Interface 0 */

/* Register: I2C_CON */
/* Description: I2C 控制寄存器 
该寄存器只有在模块不使能时（相对应 I2C_ENABLE 寄存器置0）才能被写入，其余
时刻写入数据无效。*/

/* Bit 0 : I2C MASTER_MODE   */
#define I2C_CON_MASTER_MODE_Pos (0UL) /*!< Position of MASTER_MODE field. */
#define I2C_CON_MASTER_MODE_Msk (0x1UL << I2C_CON_MASTER_MODE_Pos) /*!< Bit mask of MASTER_MODE field. */
#define I2C_CON_MASTER_MODE_Enable (1UL) /*!< Enable Master Mode */
#define I2C_CON_MASTER_MODE_Disable (0UL) /*!< Enable Master Mode */

/* Bit 1..2  I2C SPEED   */
/* Description: I2C SPEED 选择控制位 */
#define I2C_CON_SPEED_Pos (1UL) /*!< Position of SPEED field. */
#define I2C_CON_SPEED_Msk (0x3UL << I2C_CON_SPEED_Pos) /*!< Bit mask of SPEED field. */
#define I2C_CON_SPEED_STANDARD_Mode (1UL) /*!< Enable Speed standard  0 --100 kbit/s*/
#define I2C_CON_SPEED_FAST_Mode (2UL) /*!< Enable Speed Fast Mode < 400 kbit/s*/

/* Bit 3:  I2C 10BITADDR_SLAVE   */ 
/* Description： Slave 模式时可响应的地址模式   */
#define I2C_CON_10BITADDR_SLAVE_Pos (3UL) /*!< Position of 10BITADDR_SLAVE field. */
#define I2C_CON_10BITADDR_SLAVE_Msk (0x1UL << I2C_CON_10BITADDR_SLAVE_Pos) /*!< Bit mask of 10BITADDR_SLAVE field. */
#define I2C_CON_10BITADDR_SLAVE_Mode (1UL) /*!< 10-bit 地址模式*/
#define I2C_CON_7BITADDR_SLAVE_Mode (0UL) /*!< 7-bit 地址模式*/

/* Bit 4:  I2C 10BITADDR_MASTER_R   */ 
/* Description：Master 模式下地址模式的只读标志位*/
#define I2C_CON_10BITADDR_MASTER_R_Pos (4UL) /*!< Position of 10BITADDR_MASTER_R field. */
#define I2C_CON_10BITADDR_MASTER_R_Msk (0x1UL << I2C_CON_10BITADDR_MASTER_R_Pos) /*!< Bit mask of 10BITADDR_SLAVE field. */
#define I2C_CON_10BITADDR_MASTER_Mode (1UL) /*!< 10-bit 地址模式*/
#define I2C_CON_7BITADDR_MASTER_Mode (0UL) /*!< 7-bit 地址模式*/

/* Bit 5:  I2C RESTART_EN   */ 
/* Description：Master 模式下发出 Restart 的使能位
如果禁止 Restart 指示，则无法
实现：
1. 一次传输发送多个字节
2. 一次传输中改变方向
3. 发出 Start Byte
4. 7字节地址和10字节地址下执
行组合格式传输
5. 10 字节地址下执行读操作
*/
#define I2C_CON_RESTART_EN_Pos (5UL) /*!< Position of RESTART_EN field. */
#define I2C_CON_RESTART_EN_Msk (0x1UL << I2C_CON_RESTART_EN_Pos) /*!< Bit mask of RESTART_EN field. */
#define I2C_CON_RESTART_EN_Enable (1UL) /*!< 使能*/
#define I2C_CON_RESTART_EN_Disable (0UL) /*!< 不使能*/

/* Bit 6:  I2C SLAVE_DISABLE   */ 
/* Description：Slave 模式使能控制位   */
#define I2C_CON_SLAVE_DISABLE_Pos (6UL) /*!< Position of 10BITADDR_MASTER_R field. */
#define I2C_CON_SLAVE_DISABLE_Msk (0x1UL << I2C_CON_SLAVE_DISABLE_Pos) /*!< Bit mask of 10BITADDR_SLAVE field. */
#define I2C_CON_SLAVE_DISABLE_Enable (0UL) /*!< Slave 模式使能*/
#define I2C_CON_SLAVE_DISABLE_Disable (1UL) /*!< Slave 模式不使能*/

/* Register: I2C_TAR */
/* Description: I2C 目标地址寄存器 
该寄存器只有在模块不使能时（相对应 I2C_ENABLE 寄存器置0）才能被写入，其余
时刻写入数据无效。
*/

/* Bit 0..9  I2C I2C_TAR   */
/* Description： Master 数据传输的目标地址   */
#define I2C_TAR_I2C_TAR_Pos (0UL) /*!< Position of I2C_TAR field. */
#define I2C_TAR_I2C_TAR_Msk (0x1FFUL << I2C_TAR_I2C_TAR_Pos) /*!< Bit mask of 10BITADDR_SLAVE field. */

/* Bit 10:  I2C GC_OR_START   */ 
/* Description： 通用寻址 OR start   
通用寻址时，只有写命令能够被
执 行 ， 如 果 发 出 读 命 令 ， 则
TX_ABRT 置位。保持通用寻址模
式直到 SPECIAL 复位。
如果 SPECIAL＝0 时：无定义。
*/
#define I2C_TAR_GC_OR_START_Pos (10UL) /*!< Position of GC_OR_START field. */
#define I2C_TAR_GC_OR_START_Msk (0x1UL << I2C_TAR_GC_OR_START_Pos) /*!< Bit mask of GC_OR_START field. */
#define I2C_TAR_GC_OR_START_Gc (0UL) /*!< 通用寻址地址*/
#define I2C_TAR_GC_OR_START_Start (1UL) /*!<Start Byte*/

/* Bit 11:  I2C SPECIAL   */ 
/* Description： 确定软件是否执行通用寻址或者
发送 Start Byte 命令   */
#define I2C_TAR_SPECIAL_Pos (11UL) /*!< Position of SPECIAL field. */
#define I2C_TAR_SPECIA_Msk (0x1UL << I2C_TAR_SPECIAL_Pos) /*!< Bit mask of SPECIAL field. */
#define I2C_TAR_SPECIA_Execute (1UL) /*!<执行 GC_OR_START 所描述的特殊 I2C 命令*/
#define I2C_TAR_SPECIA_Ignore (0UL) /*!<忽略 GC_OR_START，正常使用 I2C_TAR*/

/* Bit 12:  I2C I2C_10BITADDR_MASTER   */ 
/* Description： 确定 Master 模式下传输的地址模式  */
#define I2C_TAR_I2C_10BITADDR_MASTER_Pos (12UL) /*!< Position of I2C_10BITADDR_MASTER field. */
#define I2C_TAR_I2C_10BITADDR_MASTER_Msk (0x1UL << I2C_TAR_I2C_10BITADDR_MASTER_Pos) /*!< Bit mask of I2C_10BITADDR_MASTER field. */
#define I2C_TAR_I2C_10BITADDR_Mode (1UL) /*!<10-bit 地址模式*/
#define I2C_TAR_I2C_7BITADDR_Mode (0UL) /*!<7-bit 地址模式*/



/* Register: I2C_SAR */
/* Description: I2C 从地址控制寄存器 
该寄存器只有在模块不使能时（相对应 I2C_ENABLE 寄存器置0）才能被写入，其余
时刻写入数据无效。*/

/* Bit 0..9: I2C I2C_SAR   */
/* Description： Slave 数据传输的目标地址   */
#define I2C_TAR_I2C_SAR_Pos (0UL) /*!< Position of I2C_TAR field. */
#define I2C_TAR_I2C_SAR_Msk (0x1FFUL << I2C_TAR_I2C_SAR_Pos) /*!< Bit mask of I2C_TAR field. */


/* Register: I2C_DATA_CMD */
/* Description: I2C Rx/Tx 数据缓存和命令寄存器  */

/* Bit 0..7:  I2C DAT   */
/* Description： 包含需要发送或者接收到的 I2C 总线数据   */
#define I2C_DATA_CMD_DAT_Pos (0UL) /*!< Position of DAT field. */
#define I2C_DATA_CMD_DAT_Msk (0xFFUL << I2C_DATA_CMD_DAT_Pos) /*!< Bit mask of DAT field. */

/* Bit 8:  I2C CMD   */
/* Description： 读写控制   */
#define I2C_DATA_CMD_CMD_Pos (8UL) /*!< Position of CMD field. */
#define I2C_DATA_CMD_CMD_Msk (0x1UL << I2C_DATA_CMD_CMD_Pos) /*!< Bit mask of CMD field. */
#define I2C_DATA_CMD_CMD_Read (1UL) /*!<读 操作*/
#define I2C_DATA_CMD_CMD_Write (0UL) /*!<写 操作*/


/* Bit 9:  I2C STOP   */
/* Description： 该位控制在一个字节发送、接收
之后是否产生 STOP 信号   */
#define I2C_DATA_CMD_STOP_Pos (9UL) /*!< Position of STOP field. */
#define I2C_DATA_CMD_STOP_Msk (0x1UL << I2C_DATA_CMD_STOP_Pos) /*!< Bit mask of STOP field. */
#define I2C_DATA_CMD_STOP_Set (1UL) 
/*!不管发送 FIFO 是否为空，当
前字节完成后产生 STOP 信号。
发送 FIFO 非空时 I2C 接口模块
会立即产生 START 信号去获取
总线
*/
#define I2C_DATA_CMD_STOP_Clean (0UL) 
/*!不管发送 FIFO 是否为空，当
前字节完成后不产生 STOP 信
号。发送 FIFO 非空时总线根据
I2C_DATA_CMD.CMD 位 设 置
继续发送/接收字节；发送 FIFO
为空时，I2C 接口模块保持 SCL
低电平直到发送 FIFO 中有新的
可用命令
*/

/* Bit 10:  I2C RESTART   */
/* Description： 该位控制在发送或接收一个字节
之前是否产生 RESTART 信号   */
#define I2C_DATA_CMD_RESTART_Pos (10UL) /*!< Position of RESTART field. */
#define I2C_DATA_CMD_RESTART_Msk (0x1UL << I2C_DATA_CMD_RESTART_Pos) /*!< Bit mask of RESTART field. */
#define I2C_DATA_CMD_RESTART_Set (1UL) 
/*!<： 当
I2C_CON.I2C_RESTART_EN
设置为 1 时，不管传输方向是否
改变，都产生 RESTART 信号；
当
I2C_CON.I2C_RESTART_EN
设置为 0 时，会产生 STOP 和
START 信号代替 RESTART 信
号。
*/
#define I2C_DATA_CMD_RESTART_Clear (0UL) 
/*!： 当
I2C_CON.I2C_RESTART_EN
设置为 1 时，不管传输方向是否
改变，都产生 RESTART 信号；
当
I2C_CON.I2C_RESTART_EN
设置为 0 时，会产生 STOP 和
START 信号代替 RESTART 信
号。
*/


/* Register: I2C_SS_SCL_HCNT */
/* Description: 标准速率 I2C 时 钟 高 电 平 计 数 寄 存 器  
该寄存器只有在模块不使能时（相对应 I2C_ENABLE 寄存器置0）才能被写入，其余
时刻写入数据无效。*/

/* Bit 0..15:  I2C_SS_SCL_HCNT    */
/* Description：为确保正确的 I/O 时序，此寄存
位数 名称 类型 复位值 描述
器需要在任何数据传输之前设
置，设置标准模式下 scl 高电平
计数，最小的合法值为 6。 */
#define I2C_SS_SCL_HCNT_Pos (0UL) /*!< Position of I2C_SS_SCL_HCNT field. */
#define I2C_SS_SCL_HCNT_Msk (0xFFFFUL << I2C_SS_SCL_HCNT_Pos) /*!< Bit mask of I2C_SS_SCL_HCNT field. */


/* Register: I2C_SS_SCL_LCNT */
/* Description: 标准速率 I2C 时 钟 低 电 平 计 数 寄 存 器  
该寄存器只有在模块不使能时（相对应 I2C_ENABLE 寄存器置0）才能被写入，其余
时刻写入数据无效。*/

/* Bit 0..15:  I2C_SS_SCL_LCNT    */
/* Description： 为确保正确的 I/O 时序，此寄存器
需要在任何数据传输之前设置，
设置标准模式下 scl 低电平计数，
最小的合法值为 8。   */
#define I2C_SS_SCL_LCNT_Pos (0UL) /*!< Position of I2C_SS_SCL_LCNT field. */
#define I2C_SS_SCL_LCNT_Msk (0xFFFFUL << I2C_SS_SCL_LCNT_Pos) /*!< Bit mask of I2C_SS_SCL_LCNT field. */


/* Register: I2C_FS_SCL_HCNT */
/* Description: 快速速率 I2C 时 钟 高 电 平 计 数 寄 存 器  
该寄存器只有在模块不使能时（相对应 I2C_ENABLE 寄存器置0）才能被写入，其余
时刻写入数据无效。*/

/* Bit 0..15:  I2C_FS_SCL_HCNT    */
/* Description： 为确保正确的 I/O 时序，此寄存
器需要在任何数据传输之前设
置，设置快速模式下 scl 高电平
计数，最小的合法值为 6。  */
#define I2C_FS_SCL_HCNT_Pos (0UL) /*!< Position of I2C_FS_SCL_HCNT field. */
#define I2C_FS_SCL_HCNT_Msk (0xFFFFUL << I2C_FS_SCL_HCNT_Pos) /*!< Bit mask of I2C_FS_SCL_HCNT field. */


/* Register: I2C_FS_SCL_LCNT */
/* Description: 快速速率 I2C 时 钟 低 电 平 计 数 寄 存 器  
该寄存器只有在模块不使能时（相对应 I2C_ENABLE 寄存器置0）才能被写入，其余
时刻写入数据无效。*/

/* Bit 0..15:  I2C_FS_SCL_LCNT    */
/* Description： 为确保正确的 I/O 时序，此寄存
器需要在任何数据传输之前设
置，设置快速模式下 scl 高电平
计数，最小的合法值为 8。   */
#define I2C_FS_SCL_LCNT_Pos (0UL) /*!< Position of I2C_SS_SCL_LCNT field. */
#define I2C_FS_SCL_LCNT_Msk (0xFFFFUL << I2C_FS_SCL_LCNT_Pos) /*!< Bit mask of I2C_FS_SCL_LCNT field. */



#define I2C_FREQUENCY_FREQUENCY_K100 (0x0063484FUL) /*!< 100 kbps */
#define I2C_FREQUENCY_FREQUENCY_K400 (0x00650C13UL) /*!< 400 kbps */


/* Register: I2C_INTR_STAT */
/* Description: I2C 中断状态寄存器 */

/* Bit 0:  I2C RX_UNDER   */
/* Description： RX FIFO 读空标志   */
#define I2C_INTR_STAT_RX_UNDER_Pos (0UL) /*!< Position of RX_UNDER field. */
#define I2C_INTR_STAT_RX_UNDER_Msk (0x1UL << I2C_INTR_STAT_RX_UNDER_Pos) /*!< Bit mask of RX_UNDER field. */
#define I2C_INTR_STAT_RX_UNDER_NotGenerated (0UL) /*!< 无效. */
#define I2C_INTR_STAT_RX_UNDER_Generated (1UL) /*!< RX_FIFO 读空错误. */

/* Bit 1:  I2C RX_OVER   */
/* Description： RX FIFO 写溢出标志   */
#define I2C_INTR_STAT_RX_OVER_Pos (1UL) /*!< Position of RX_OVER field. */
#define I2C_INTR_STAT_RX_OVER_Msk (0x1UL << I2C_INTR_STAT_RX_OVER_Pos) /*!< Bit mask of RX_OVER field. */
#define I2C_INTR_STAT_RX_OVER_NotGenerated (0UL) /*!< 无效. */
#define I2C_INTR_STAT_RX_OVER_Generated (1UL) /*!< RX_FIFO 写溢出错误. */

/* Bit 2:  I2C RX_FULL   */
/* Description： RX FIFO 满标志   */
#define I2C_INTR_STAT_RX_FULL_Pos (2UL) /*!< Position of RX_FULL field. */
#define I2C_INTR_STAT_RX_FULL_Msk (0x1UL << I2C_INTR_STAT_RX_FULL_Pos) /*!< Bit mask of RX_FULL field. */
#define I2C_INTR_STAT_RX_FULL_NotGenerated (0UL) /*!< 无效. */
#define I2C_INTR_STAT_RX_FULL_Generated (1UL) /*!< RX FIFO 数据大于或者等于RX FIFO 阈值。此比特随 FIFO状态自动更新。. */

/* Bit 3:  I2C TX_OVER   */
/* Description： TX FIFO 写溢出标志   */
#define I2C_INTR_STAT_TX_OVER_Pos (3UL) /*!< Position of TX_OVER field. */
#define I2C_INTR_STAT_TX_OVER_Msk (0x1UL << I2C_INTR_STAT_TX_OVER_Pos) /*!< Bit mask of TX_OVER field. */
#define I2C_INTR_STAT_TX_OVER_NotGenerated (0UL) /*!< 无效. */
#define I2C_INTR_STAT_TX_OVER_Generated (1UL) /*!< TX_FIFO 写溢出错误. */

/* Bit 4:  I2C TX_EMPTY   */
/* Description： TX FIFO 空标志   */
#define I2C_INTR_STAT_TX_EMPTY_Pos (4UL) /*!< Position of TX_EMPTY field. */
#define I2C_INTR_STAT_TX_EMPTY_Msk (0x1UL << I2C_INTR_STAT_TX_EMPTY_Pos) /*!< Bit mask of TX_EMPTY field. */
#define I2C_INTR_STAT_TX_EMPTY_NotGenerated (0UL) /*!< 无效. */
#define I2C_INTR_STAT_TX_EMPTY_Generated (1UL) /*!< TX FIFO 数据小于或者等于TX FIFO 阈值。此比特随 FIFO状态自动更新 */

/* Bit 5:  I2C RD_REQ   */
/* Description：作为 slave，当另外一个 master
尝试从 slave 读数据时产生此中
断。  Slave 保持 I2C 总线为等待
状态（SCL=0）直到中断服务被
响应。这表示 slave 被远端的
master 寻址，并且要求其发送数
据。处理器必须响应这个中断，
并 将 待 发 送 的 数 据 写 入
I2C_DATA_CMD 寄存器。 */
#define I2C_INTR_STAT_RD_REQ_Pos (5UL) /*!< Position of RD_REQ field. */
#define I2C_INTR_STAT_RD_REQ_Msk (0x1UL << I2C_INTR_STAT_RD_REQ_Pos) /*!< Bit mask of RD_REQ field. */
#define I2C_INTR_STAT_RD_REQ_NotGenerated (0UL) /*!< 无效. */
#define I2C_INTR_STAT_RD_REQ_Generated (1UL) /*!< 有效*/

/* Bit 6:  I2C TX_ABRT   */
/* Description：传输异常标志 
作为 master，当接口模块无法完
成处理器的命令时，此位置 1 的
条件包括：
1 地址发出之后无 slave 确认信
号
2 被寻址的 slave 没有发出数据
3 仲裁丢失
4 I2C_RESTART_EN 置 0，但
是处理器发出在没有 restart 条
件就无法完成的命令
5 Start Byte 被确认
6 通用寻址地址无确认
7 读请求中断发生时，TX FIFO
位数 名称 类型 复位值 描述
有 残 留 数 据 ， 或 者 当
I2C_RESTART_EN 不使能时，
在数据传输时丢失总线的控制，
又被当作 I2C slaved 使用
8 通用寻址命令之后发出读命
令
9 响应 RD_REQ 之前，处理器
发出读命令
此位一旦置 1，发送和接收 FIFO
自动清空。*/
#define I2C_INTR_STAT_TX_ABRT_Pos (6UL) /*!< Position of TX_ABRT field. */
#define I2C_INTR_STAT_TX_ABRT_Msk (0x1UL << I2C_INTR_STAT_TX_ABRT_Pos) /*!< Bit mask of TX_ABRT field. */
#define I2C_INTR_STAT_TX_ABRT_NotGenerated  (0UL) /*!< 无效. */
#define I2C_INTR_STAT_TX_ABRT_Generated  (1UL) /*!< 有效 */


/* Bit 7:  I2C RX_DONE   */
/* Description： 作为 slave 发送数据时，如果
master 没有响应的话，在发送最
后一个字节后产生此中断，表示
发送结束  */
#define I2C_INTR_STAT_RX_DONE_Pos (7UL) /*!< Position of RX_DONE field. */
#define I2C_INTR_STAT_RX_DONE_Msk (0x1UL << I2C_INTR_STAT_RX_DONE_Pos) /*!< Bit mask of RX_DONE field. */
#define I2C_INTR_STAT_RX_DONE_NotGenerated (0UL) /*!< 无效. */
#define I2C_INTR_STAT_RX_DONE_Generated (1UL) /*!< 有效 ，表示发送结束 . */

/* Bit 8:  I2C ACTIVITY   */
/* Description： 接口模块活动状态，保持置
位直到软件清除 */
#define I2C_INTR_STAT_ACTIVITY_Pos (8UL) /*!< Position of ACTIVITY field. */
#define I2C_INTR_STAT_ACTIVITY_Msk (0x1UL << I2C_INTR_STAT_ACTIVITY_Pos) /*!< Bit mask of ACTIVITY field. */
#define I2C_INTR_STAT_ACTIVITY_NotGenerated (0UL) /*!< 无效. */
#define I2C_INTR_STAT_ACTIVITY_Generated (1UL) /*!< 有效 ，表示发送结束 . */

/* Bit 9:  I2C STOP_DET   */
/* Description： 结束条件标志 */
#define I2C_INTR_STAT_STOP_DET_Pos (9UL) /*!< Position of STOP_DET field. */
#define I2C_INTR_STAT_STOP_DET_Msk (0x1UL << I2C_INTR_STAT_STOP_DET_Pos) /*!< Bit mask of STOP_DET field. */
#define I2C_INTR_STAT_STOP_DET_NotGenerated (0UL) /*!< 无效. */
#define I2C_INTR_STAT_STOP_DET_Generated (1UL) /*!< 有效, 总线出现结束条件 . */

/* Bit 10:  I2C START_DET   */
/* Description： 结束条件标志 */
#define I2C_INTR_STAT_START_DET_Pos (10UL) /*!< Position of START_DET field. */
#define I2C_INTR_STAT_START_DET_Msk (0x1UL << I2C_INTR_STAT_START_DET_Pos) /*!< Bit mask of START_DET field. */
#define I2C_INTR_STAT_START_DET_NotGenerated (0UL) /*!< 无效. */
#define I2C_INTR_STAT_START_DET_Generated (1UL) /*!< 有效, 总线出现开始条件 . */

/* Bit 11:  I2C GEN_CALL   */
/* Description： 通用寻址请求标志 */
#define I2C_INTR_STAT_GEN_CALL_Pos (11UL) /*!< Position of GEN_CALL field. */
#define I2C_INTR_STAT_GEN_CALL_Msk (0x1UL << I2C_INTR_STAT_START_DET_Pos) /*!< Bit mask of GEN_CALL field. */
#define I2C_INTR_STAT_GEN_CALL_NotGenerated (0UL) /*!< 无效. */
#define I2C_INTR_STAT_GEN_CALL_Generated (1UL) /*!< 有效, 收到通用寻址请求，接口模块将接收到的数据放入 RX 缓存中 . */



/* Register: I2C_INTR_EN */
/* Description: I2C 中断使能寄存器 */

/* Bit 0:  I2C EN_RX_UNDER   */
/* Description： RX FIFO 读空中断使能   */
#define I2C_INTR_EN_RX_UNDER_Pos (0UL) /*!< Position of EN_RX_UNDER field. */
#define I2C_INTR_EN_RX_UNDER_Msk (0x1UL << I2C_INTR_EN_RX_UNDER_Pos) /*!< Bit mask of EN_RX_UNDER field. */
#define I2C_INTR_EN_RX_UNDER_Disable (0UL) /*!< 中断不使能. */
#define I2C_INTR_EN_RX_UNDER_Enable (1UL) /*!< 中断使能. */

/* Bit 1:  I2C EN_RX_OVER   */
/* Description： RX FIFO 写满中断使能   */
#define I2C_INTR_EN_RX_OVER_Pos (1UL) /*!< Position of EN_RX_OVER field. */
#define I2C_INTR_EN_RX_OVER_Msk (0x1UL << I2C_INTR_EN_RX_OVER_Pos) /*!< Bit mask of EN_RX_OVER field. */
#define I2C_INTR_EN_RX_OVER_Disable (0UL) /*!< 中断不使能. */
#define I2C_INTR_EN_RX_OVER_Enable (1UL) /*!< 中断使能. */

/* Bit 2:  I2C EN_RX_FULL   */
/* Description： RX FIFO 满中断使能   */
#define I2C_INTR_EN_RX_FULL_Pos (2UL) /*!< Position of EN_RX_FULL field. */
#define I2C_INTR_EN_RX_FULL_Msk (0x1UL << I2C_INTR_EN_RX_FULL_Pos) /*!< Bit mask of EN_RX_FULL field. */
#define I2C_INTR_EN_RX_FULL_Disable (0UL) /*!< 中断不使能. */
#define I2C_INTR_EN_RX_FULL_Enable (1UL) /*!< 中断使能. */

/* Bit 3:  I2C EN_TX_OVER   */
/* Description： RX FIFO 满中断使能   */
#define I2C_INTR_EN_TX_OVER_Pos (3UL) /*!< Position of EN_TX_OVER field. */
#define I2C_INTR_EN_TX_OVER_Msk (0x1UL << I2C_INTR_EN_TX_OVER_Pos) /*!< Bit mask of EN_TX_OVER field. */
#define I2C_INTR_EN_TX_OVER_Disable (0UL) /*!< 中断不使能. */
#define I2C_INTR_EN_TX_OVER_Enable (1UL) /*!< 中断使能. */

/* Bit 4:  I2C EN_TX_EMPTY   */
/* Description： TX FIFO 空中断使能   */
#define I2C_INTR_EN_TX_EMPTY_Pos (4UL) /*!< Position of EN_TX_EMPTY field. */
#define I2C_INTR_EN_TX_EMPTY_Msk (0x1UL << I2C_INTR_EN_TX_EMPTY_Pos) /*!< Bit mask of EN_TX_EMPTY field. */
#define I2C_INTR_EN_TX_EMPTY_Disable (0UL) /*!< 中断不使能. */
#define I2C_INTR_EN_TX_EMPTY_Enable (1UL) /*!< 中断使能. */

/* Bit 5:  I2C EN_RD_REQ   */
/* Description： Slave 收到发送数据请求中断   */
#define I2C_INTR_EN_RD_REQ_Pos (5UL) /*!< Position of EN_RD_REQ field. */
#define I2C_INTR_EN_RD_REQ_Msk (0x1UL << I2C_INTR_EN_RD_REQ_Pos) /*!< Bit mask of EN_RD_REQ field. */
#define I2C_INTR_EN_RD_REQ_Disable (0UL) /*!< 中断不使能. */
#define I2C_INTR_EN_RD_REQ_Enable (1UL) /*!< 中断使能. */

/* Bit 6:  I2C EN_TX_ABRT   */
/* Description： 传输异常中断使能   */
#define I2C_INTR_EN_TX_ABRT_Pos (6UL) /*!< Position of EN_TX_ABRT field. */
#define I2C_INTR_EN_TX_ABRT_Msk (0x1UL << I2C_INTR_EN_TX_ABRT_Pos) /*!< Bit mask of EN_TX_ABRT field. */
#define I2C_INTR_EN_TX_ABRT_Disable (0UL) /*!< 中断不使能. */
#define I2C_INTR_EN_TX_ABRT_Enable (1UL) /*!< 中断使能. */

/* Bit 7:  I2C EN_RX_DONE   */
/* Description： Slave 发送数据完成中断   */
#define I2C_INTR_EN_RX_DONE_Pos (7UL) /*!< Position of EN_RX_DONE field. */
#define I2C_INTR_EN_RX_DONE_Msk (0x1UL << I2C_INTR_EN_RX_DONE_Pos) /*!< Bit mask of EN_RX_DONE field. */
#define I2C_INTR_EN_RX_DONE_Disable (0UL) /*!< 中断不使能. */
#define I2C_INTR_EN_RX_DONE_Enable (1UL) /*!< 中断使能. */

/* Bit 8:  I2C EN_ACTIVITY   */
/* Description： 活动状态中断使能   */
#define I2C_INTR_EN_ACTIVITY_Pos (8UL) /*!< Position of EN_ACTIVITY field. */
#define I2C_INTR_EN_ACTIVITY_Msk (0x1UL << I2C_INTR_EN_ACTIVITY_Pos) /*!< Bit mask of EN_ACTIVITY field. */
#define I2C_INTR_EN_ACTIVITY_Disable (0UL) /*!< 中断不使能. */
#define I2C_INTR_EN_ACTIVITY_Enable (1UL) /*!< 中断使能. */

/* Bit 9:  I2C EN_STOP_DET   */
/* Description： 结束标志中断使能   */
#define I2C_INTR_EN_STOP_DET_Pos (9UL) /*!< Position of EN_STOP_DET field. */
#define I2C_INTR_EN_STOP_DET_Msk (0x1UL << I2C_INTR_EN_STOP_DET_Pos) /*!< Bit mask of EN_STOP_DET field. */
#define I2C_INTR_EN_STOP_DET_Disable (0UL) /*!< 中断不使能. */
#define I2C_INTR_EN_STOP_DET_Enable (1UL) /*!< 中断使能. */

/* Bit 10:  I2C EN_START_DET   */
/* Description： 开始标志中断使能   */
#define I2C_INTR_EN_START_DET_Pos (10UL) /*!< Position of EN_START_DET field. */
#define I2C_INTR_EN_START_DET_Msk (0x1UL << I2C_INTR_EN_START_DET_Pos) /*!< Bit mask of EN_START_DET field. */
#define I2C_INTR_EN_START_DET_Disable (0UL) /*!< 中断不使能. */
#define I2C_INTR_EN_START_DET_Enable (1UL) /*!< 中断使能. */

/* Bit 11:  I2C EN_GEN_CALL   */
/* Description： 通用寻址中断使能   */
#define I2C_INTR_EN_GEN_CALL_Pos (11UL) /*!< Position of EN_GEN_CALL field. */
#define I2C_INTR_EN_GEN_CALL_Msk (0x1UL << I2C_INTR_EN_GEN_CALL_Pos) /*!< Bit mask of EN_GEN_CALL field. */
#define I2C_INTR_EN_GEN_CALL_Disable (0UL) /*!< 中断不使能. */
#define I2C_INTR_EN_GEN_CALL_Enable (1UL) /*!< 中断使能. */



/* Register: I2C_RAW_INTR_STAT */
/* Description: I2C  中断原始状态寄存器 */

/* Bit 0:  I2C R_RX_UNDER   */
/* Description： 对应中断状态寄存器
RX_UNDER 位 */
#define I2C_RAW_INTR_STAT_RX_UNDER_Pos (0UL) /*!< Position of R_RX_UNDER field. */
#define I2C_RAW_INTR_STAT_RX_UNDER_Msk (0x1UL << I2C_RAW_INTR_STAT_RX_UNDER_Pos) /*!< Bit mask of R_RX_UNDER field. */
#define I2C_RAW_INTR_STAT_RX_UNDER_NotGenerated (0UL) /*!< 无效. */
#define I2C_RAW_INTR_STAT_RX_UNDERL_Generated (1UL) /*!< 有效. */

/* Bit 1:  I2C R_RX_OVER   */
/* Description： 对应中断状态寄存器
RX_OVER 位 */
#define I2C_RAW_INTR_STAT_RX_OVER_Pos (1UL) /*!< Position of R_RX_OVER field. */
#define I2C_RAW_INTR_STAT_RX_OVER_Msk (0x1UL << I2C_RAW_INTR_STAT_RX_OVER_Pos) /*!< Bit mask of R_RX_OVER field. */
#define I2C_RAW_INTR_STAT_RX_OVER_NotGenerated (0UL) /*!< 无效. */
#define I2C_RAW_INTR_STAT_RX_OVER_Generated (1UL) /*!< 有效. */

/* Bit 2:  I2C R_RX_FULL   */
/* Description： 对应中断状态寄存器
RX_FULL 位 */
#define I2C_RAW_INTR_STAT_RX_FULL_Pos (2UL) /*!< Position of R_RX_FULL field. */
#define I2C_RAW_INTR_STAT_RX_FULL_Msk (0x1UL << I2C_RAW_INTR_STAT_RX_FULL_Pos) /*!< Bit mask of R_RX_FULL field. */
#define I2C_RAW_INTR_STAT_RX_FULL_NotGenerated (0UL) /*!< 无效. */
#define I2C_RAW_INTR_STAT_RX_FULL_Generated (1UL) /*!< 有效. */

/* Bit 3:  I2C R_TX_OVER   */
/* Description： 对应中断状态寄存器
TX_OVER 位 */
#define I2C_RAW_INTR_STAT_TX_OVER_Pos (3UL) /*!< Position of R_TX_OVER field. */
#define I2C_RAW_INTR_STAT_TX_OVER_Msk (0x1UL << I2C_RAW_INTR_STAT_TX_OVER_Pos) /*!< Bit mask of R_TX_OVER field. */
#define I2C_RAW_INTR_STAT_TX_OVER_NotGenerated (0UL) /*!< 无效. */
#define I2C_RAW_INTR_STAT_TX_OVER_Generated (1UL) /*!< 有效. */

/* Bit 4:  I2C R_TX_EMPTY   */
/* Description： 对应中断状态寄存器
TX_EMPTY 位 */
#define I2C_RAW_INTR_STAT_TX_EMPTY_Pos (4UL) /*!< Position of R_TX_EMPTY field. */
#define I2C_RAW_INTR_STAT_TX_EMPTY_Msk (0x1UL << I2C_RAW_INTR_STAT_TX_EMPTY_Pos) /*!< Bit mask of R_TX_EMPTY field. */
#define I2C_RAW_INTR_STAT_TX_EMPTY_NotGenerated (0UL) /*!< 无效. */
#define I2C_RAW_INTR_STAT_TX_EMPTY_Generated (1UL) /*!< 有效. */

/* Bit 5:  I2C R_RD_REQ   */
/* Description： 对应中断状态寄存器
RD_REQ 位 */
#define I2C_RAW_INTR_STAT_RD_REQ_Pos (5UL) /*!< Position of R_RD_REQ field. */
#define I2C_RAW_INTR_STAT_RD_REQ_Msk (0x1UL << I2C_RAW_INTR_STAT_RD_REQ_Pos) /*!< Bit mask of R_RD_REQ field. */
#define I2C_RAW_INTR_STAT_RD_REQ_NotGenerated (0UL) /*!< 无效. */
#define I2C_RAW_INTR_STAT_RD_REQ_Generated (1UL) /*!< 有效. */

/* Bit 6:  I2C R_TX_ABRT   */
/* Description： 对应中断状态寄存器
TX_ABRT 位 */
#define I2C_RAW_INTR_STAT_TX_ABRT_Pos (6UL) /*!< Position of R_TX_ABRT field. */
#define I2C_RAW_INTR_STAT_TX_ABRT_Msk (0x1UL << I2C_RAW_INTR_STAT_TX_ABRT_Pos) /*!< Bit mask of R_TX_ABRT field. */
#define I2C_RAW_INTR_STAT_TX_ABRT_NotGenerated (0UL) /*!< 无效. */
#define I2C_RAW_INTR_STAT_TX_ABRT_Generated (1UL) /*!< 有效. */

/* Bit 7:  I2C R_RX_DONE   */
/* Description： 对应中断状态寄存器
RX_DONE 位 */
#define I2C_RAW_INTR_STAT_RX_DONE_Pos (7UL) /*!< Position of R_RX_DONE field. */
#define I2C_RAW_INTR_STAT_RX_DONE_Msk (0x1UL << I2C_RAW_INTR_STAT_RX_DONE_Pos) /*!< Bit mask of R_RX_DONE field. */
#define I2C_RAW_INTR_STAT_RX_DONE_NotGenerated (0UL) /*!< 无效. */
#define I2C_RAW_INTR_STAT_RX_DONE_Generated (1UL) /*!< 有效. */

/* Bit 8:  I2C R_ACTIVITY   */
/* Description： 对应中断状态寄存器
ACTIVITY 位 */
#define I2C_RAW_INTR_STAT_ACTIVITY_Pos (8UL) /*!< Position of R_ACTIVITY field. */
#define I2C_RAW_INTR_STAT_ACTIVITY_Msk (0x1UL << I2C_RAW_INTR_STAT_ACTIVITY_Pos) /*!< Bit mask of R_ACTIVITY field. */
#define I2C_RAW_INTR_STAT_ACTIVITY_NotGenerated (0UL) /*!< 无效. */
#define I2C_RAW_INTR_STAT_ACTIVITY_Generated (1UL) /*!< 有效. */

/* Bit 9:  I2C R_STOP_DET   */
/* Description： 对应中断状态寄存器
STOP_DET 位 */
#define I2C_RAW_INTR_STAT_STOP_DET_Pos (9UL) /*!< Position of R_STOP_DET field. */
#define I2C_RAW_INTR_STAT_STOP_DET_Msk (0x1UL << I2C_RAW_INTR_STAT_STOP_DET_Pos) /*!< Bit mask of R_STOP_DET field. */
#define I2C_RAW_INTR_STAT_STOP_DET_NotGenerated (0UL) /*!< 无效. */
#define I2C_RAW_INTR_STAT_STOP_DET_Generated (1UL) /*!< 有效. */

/* Bit 10:  I2C R_START_DET   */
/* Description： 对应中断状态寄存器
START_DET 位 */
#define I2C_RAW_INTR_STAT_START_DET_Pos (10UL) /*!< Position of R_START_DET field. */
#define I2C_RAW_INTR_STAT_START_DET_Msk (0x1UL << I2C_RAW_INTR_STAT_START_DET_Pos) /*!< Bit mask of R_START_DET field. */
#define I2C_RAW_INTR_STAT_START_DET_NotGenerated (0UL) /*!< 无效. */
#define I2C_RAW_INTR_STAT_START_DET_Generated (1UL) /*!< 有效. */

/* Bit 11:  I2C R_GEN_CALL   */
/* Description： 对应中断状态寄存器
GEN_CALL 位 */
#define I2C_RAW_INTR_STAT_GEN_CALL_Pos (11UL) /*!< Position of R_GEN_CALL field. */
#define I2C_RAW_INTR_STAT_GEN_CALL_Msk (0x1UL << I2C_RAW_INTR_STAT_GEN_CALL_Pos) /*!< Bit mask of R_GEN_CALL field. */
#define I2C_RAW_INTR_STAT_GEN_CALL_NotGenerated (0UL) /*!< 无效. */
#define I2C_RAW_INTR_STAT_GEN_CALL_Generated (1UL) /*!< 有效. */



/* Register: I2C_RX_TL */
/* Description: I2C  接收 FIFO 阈值寄存器 */

/* Bit 0..7:  I2C RX_TL   */
/* Description： RX FIFO 阈值，当 RX FIFO 数
据大于 RX FIFO 阈值时，引发
RX_FULL 中断.
当设置的
值大于 15 时，本寄存器的值均
为 15。
0x0：1 个
0x1：2 个
……
0xF：16 个 */
#define I2C_RX_TL_Pos (0UL) /*!< Position of RX_TL field. */
#define I2C_RX_TL_Msk (0xFFUL << I2C_RX_TL_Pos) /*!< Bit mask of RX_TL field. */

#define I2C_RX_TL_FIFO_1 (0UL)
#define I2C_RX_TL_FIFO_2 (1UL) 
#define I2C_RX_TL_FIFO_3 (2UL) 
#define I2C_RX_TL_FIFO_4 (3UL) 
#define I2C_RX_TL_FIFO_5 (4UL) 
#define I2C_RX_TL_FIFO_6 (5UL) 
#define I2C_RX_TL_FIFO_7 (6UL) 
#define I2C_RX_TL_FIFO_8 (7UL) 
#define I2C_RX_TL_FIFO_9 (8UL) 
#define I2C_RX_TL_FIFO_10 (9UL) 
#define I2C_RX_TL_FIFO_11 (10UL) 
#define I2C_RX_TL_FIFO_12 (11UL) 
#define I2C_RX_TL_FIFO_13 (12UL) 
#define I2C_RX_TL_FIFO_14 (13UL) 
#define I2C_RX_TL_FIFO_15  (14UL)  
#define I2C_RX_TL_FIFO_16  (15UL) 


/* Register: I2C_TX_TL */
/* Description: I2C  发送 FIFO 阈值寄存器 */

/* Bit 0..7:  I2C TX_TL   */
/* Description： TX FIFO 阈值，当 TX FIFO 数
据小于 TX FIFO 阈值时，引发
TX_EMPTY 中断.
当设置的
值大于 15 时，本寄存器的值均
为 15。
0x0：0 个
0x1：1 个
……
0xF：15 个 */
#define I2C_TX_TL_Pos (0UL) /*!< Position of TX_TL field. */
#define I2C_TX_TL_Msk (0xFFUL << I2C_TX_TL_Pos) /*!< Bit mask of TX_TL field. */
#define I2C_TX_TL_FIFO_0 (0UL) 
#define I2C_TX_TL_FIFO_1 (1UL) 
#define I2C_TX_TL_FIFO_2 (2UL) 
#define I2C_TX_TL_FIFO_3 (3UL) 
#define I2C_TX_TL_FIFO_4 (4UL) 
#define I2C_TX_TL_FIFO_5 (5UL) 
#define I2C_TX_TL_FIFO_6 (6UL) 
#define I2C_TX_TL_FIFO_7 (7UL) 
#define I2C_TX_TL_FIFO_8 (8UL) 
#define I2C_TX_TL_FIFO_9 (9UL) 
#define I2C_TX_TL_FIFO_10 (10UL) 
#define I2C_TX_TL_FIFO_11 (11UL) 
#define I2C_TX_TL_FIFO_12 (12UL) 
#define I2C_TX_TL_FIFO_13 (13UL) 
#define I2C_TX_TL_FIFO_14 (14UL) 
#define I2C_TX_TL_FIFO_15 (15UL) 

/* Register: I2C_CLR_INTR */
/* Description: I2C  清除全局中断寄存器 */

/* Bit 0:  I2C CLR_INTR   */
/* Description： 读此寄存器清除组合中断，各个独立中断以及I2C_TX_ABRT_SOURCE 寄存器*/
#define I2C_CLR_INTR_Pos (0UL) /*!< Position of CLR_INTR field. */
#define I2C_CLR_INTR_Msk (0x1UL << I2C_CLR_INTR_Pos) /*!< Bit mask of CLR_INTR field. */



/* Register: I2C_CLR_RX_UNDER */
/* Description: I2C  清除 RX_UNDER 中断寄存器 */

/* Bit 0:  I2C CLR_RX_UNDER   */
/* Description： 读 此 寄 存 器 清 除 中 断 RX_UNDER  */
#define I2C_CLR_RX_UNDER_Pos (0UL) /*!< Position of CLR_RX_UNDER field. */
#define I2C_CLR_RX_UNDER_Msk (0x1UL << I2C_CLR_RX_UNDER_Pos) /*!< Bit mask of CLR_RX_UNDER field. */


/* Register: I2C_CLR_RX_OVER */
/* Description: I2C  清除 RX_OVER 中断寄存器 */

/* Bit 0:  I2C CLR_RX_OVER   */
/* Description： 读 此 寄 存 器 清 除 中 断 RX_OVER  */
#define I2C_CLR_RX_OVER_Pos (0UL) /*!< Position of CLR_RX_OVER field. */
#define I2C_CLR_RX_OVER_Msk (0x1UL << I2C_CLR_RX_OVER_Pos) /*!< Bit mask of CLR_RX_OVER field. */


/* Register: I2C_CLR_TX_OVER */
/* Description: I2C  清除 TX_OVER 中断寄存器 */

/* Bit 0:  I2C CLR_TX_OVER   */
/* Description： 读 此 寄 存 器 清 除 中 断 TX_OVER  */
#define I2C_CLR_TX_OVER_Pos (0UL) /*!< Position of CLR_TX_OVER field. */
#define I2C_CLR_TX_OVER_Msk (0x1UL << I2C_CLR_TX_OVER_Pos) /*!< Bit mask of CLR_TX_OVER field. */


/* Register: I2C_CLR_RD_REQ */
/* Description: I2C  清除 RD_REQ 中断寄存器 */

/* Bit 0:  I2C CLR_RD_REQ   */
/* Description： 读 此 寄 存 器 清 除 中 断 RD_REQ  */
#define I2C_CLR_RD_REQ_Pos (0UL) /*!< Position of CLR_RD_REQ field. */
#define I2C_CLR_RD_REQ_Msk (0x1UL << I2C_CLR_RD_REQ_Pos) /*!< Bit mask of CLR_RD_REQ field. */


/* Register: I2C_CLR_TX_ABRT */
/* Description: I2C  清除 TX_ABRT 中断寄存器 */

/* Bit 0:  I2C CLR_TX_ABRT   */
/* Description： 读 此 寄 存 器 清 除 中 断 TX_ABRT  */
#define I2C_CLR_TX_ABRT_Pos (0UL) /*!< Position of CLR_TX_ABRT field. */
#define I2C_CLR_TX_ABRT_Msk (0x1UL << I2C_CLR_TX_ABRT_Pos) /*!< Bit mask of CLR_TX_ABRT field. */


/* Register: I2C_CLR_RX_DONE */
/* Description: I2C  清除 RX_DONE 中断寄存器 */

/* Bit 0:  I2C CLR_RX_DONE   */
/* Description： 读 此 寄 存 器 清 除 中 断 RX_DONE  */
#define I2C_CLR_RX_DONE_Pos (0UL) /*!< Position of CLR_RX_DONE field. */
#define I2C_CLR_RX_DONE_Msk (0x1UL << I2C_CLR_RX_DONE_Pos) /*!< Bit mask of CLR_RX_DONE field. */



/* Register: I2C_CLR_ACTIVITY */
/* Description: I2C  清除 ACTIVITY 中断寄存器 */

/* Bit 0:  I2C CLR_ACTIVITY   */
/* Description： 读 此 寄 存 器 清 除 中 断 ACTIVITY  */
#define I2C_CLR_ACTIVITY_Pos (0UL) /*!< Position of CLR_ACTIVITY field. */
#define I2C_CLR_ACTIVITY_Msk (0x1UL << I2C_CLR_ACTIVITY_Pos) /*!< Bit mask of CLR_ACTIVITY field. */


/* Register: I2C_CLR_STOP_DET */
/* Description: I2C  清除 STOP_DET 中断寄存器 */

/* Bit 0:  I2C CLR_STOP_DET   */
/* Description： 读 此 寄 存 器 清 除 中 断 STOP_DET  */
#define I2C_CLR_STOP_DET_Pos (0UL) /*!< Position of CLR_STOP_DET field. */
#define I2C_CLR_STOP_DET_Msk (0x1UL << I2C_CLR_STOP_DET_Pos) /*!< Bit mask of CLR_STOP_DET field. */


/* Register: I2C_CLR_START_DET */
/* Description: I2C  清除 START_DET 中断寄存器 */

/* Bit 0:  I2C CLR_START_DET   */
/* Description： 读 此 寄 存 器 清 除 中 断 START_DET  */
#define I2C_CLR_START_DET_Pos (0UL) /*!< Position of CLR_START_DET field. */
#define I2C_CLR_START_DET_Msk (0x1UL << I2C_CLR_START_DET_Pos) /*!< Bit mask of CLR_START_DET field. */


/* Register: I2C_CLR_GEN_CALL */
/* Description: I2C  清除 GEN_CALL 中断寄存器 */

/* Bit 0:  I2C CLR_GEN_CALL   */
/* Description： 读 此 寄 存 器 清 除 中 断 GEN_CALL  */
#define I2C_CLR_GEN_CALL_Pos (0UL) /*!< Position of CLR_GEN_CALL field. */
#define I2C_CLR_GEN_CALL_Msk (0x1UL << I2C_CLR_GEN_CALL_Pos) /*!< Bit mask of CLR_GEN_CALL field. */


/* Register: I2C_ENABLE */
/* Description: I2C 使能寄存器 */

/* Bit 0:  I2C ENABLE  */
/* Description： 接口模块使能控制  
接口模块处于激活状态时，软件
不能进行禁止设置。可通过 I2C
状态寄存器（I2C_STATUS）来
查询模块是否处于激活状态。
使能/禁止存在两个 i2c_clk 延迟*/
#define I2C_ENABLE_EN_Pos (0UL) /*!< Position of ENABLE field. */
#define I2C_ENABLE_EN_Msk (0x1UL << I2C_ENABLE_EN_Pos) /*!< Bit mask of ENABLE field. */
#define I2C_ENABLE_EN_Disable (0UL) /*!< 禁止. */
#define I2C_ENABLE_EN_Enable (1UL) /*!< 使能. */



/* Register: I2C_STATUS */
/* Description: I2C 状态寄存器 */

/* Bit 0:  I2C ACTIVITY  */
/* Description： 激活状态 */
#define I2C_STATUS_ACTIVITY_Pos (0UL) /*!< Position of ACTIVITY field. */
#define I2C_STATUS_ACTIVITY_Msk (0x1UL << I2C_STATUS_ACTIVITY_Pos) /*!< Bit mask of ACTIVITY field. */
#define I2C_STATUS_ACTIVITY_NotGenerated (0UL) /*!< 不激活. */
#define I2C_STATUS_ACTIVITY_Generated (1UL) /*!< 激活. */

/* Bit 1:  I2C TFNF  */
/* Description： TX FIFO 不满 */
#define I2C_STATUS_TFNF_Pos (1UL) /*!< Position of TFNF field. */
#define I2C_STATUS_TFNF_Msk (0x1UL << I2C_STATUS_TFNF_Pos) /*!< Bit mask of TFNF field. */
#define I2C_STATUS_TFNF_NotGenerated (0UL) /*!< TX FIFO 满. */
#define I2C_STATUS_TFNF_Generated (1UL) /*!< TX FIFO 不满. */

/* Bit 2:  I2C TFE  */
/* Description：TX FIFO 空 */
#define I2C_STATUS_TFE_Pos (2UL) /*!< Position of TFE field. */
#define I2C_STATUS_TFE_Msk (0x1UL << I2C_STATUS_TFE_Pos) /*!< Bit mask of TFE field. */
#define I2C_STATUS_TFE_NotGenerated (0UL) /*!< TX FIFO 不空. */
#define I2C_STATUS_TFE_Generated (1UL) /*!< TX FIFO 空. */

/* Bit 3:  I2C RFNE  */
/* Description：RX FIFO 不空 */
#define I2C_STATUS_RFNE_Pos (3UL) /*!< Position of RFNE field. */
#define I2C_STATUS_RFNE_Msk (0x1UL << I2C_STATUS_RFNE_Pos) /*!< Bit mask of RFNE field. */
#define I2C_STATUS_RFNE_NotGenerated (0UL) /*!< RX FIFO 空. */
#define I2C_STATUS_RFNE_Generated (1UL) /*!< RX FIFO 不空. */


/* Bit 4:  I2C RFF  */
/* Description：RX FIFO 满 */
#define I2C_STATUS_RFF_Pos (4UL) /*!< Position of RFF field. */
#define I2C_STATUS_RFF_Msk (0x1UL << I2C_STATUS_RFF_Pos) /*!< Bit mask of RFF field. */
#define I2C_STATUS_RFF_NotGenerated (0UL) /*!< RX FIFO 不满. */
#define I2C_STATUS_RFF_Generated (1UL) /*!< RX FIFO 满. */



/* Register: I2C_TXFLR */
/* Description: 发送 FIFO 数据计数寄存器 */

/* Bit 0..4:  I2C TXFLR  */
/* Description： TX FIFO 有效数据项个数 */
#define I2C_TXFLR_Pos (0UL) /*!< Position of TXFLR field. */
#define I2C_TXFLR_Msk (0x1FUL << I2C_TXFLR_Pos) /*!< Bit mask of TXFLR field. */


/* Register: I2C_RXFLR */
/* Description: 接收 FIFO 数据计数寄存器 */

/* Bit 0..4:  I2C RXFLR  */
/* Description： RX FIFO 有效数据项个数 I2C 被禁止或者传输异常中止时，该寄存器清 0。*/
#define I2C_RXFLR_Pos (0UL) /*!< Position of RXFLR field. */
#define I2C_RXFLR_Msk (0x1FUL << I2C_RXFLR_Pos) /*!< Bit mask of RXFLR field. */


/* Register: I2C_SDA_HOLD */
/* Description: SDA 保持时间 */

/* Bit 0..15:  I2C I2C_SDA_HOLD  */
/* Description： 设置以 i2c_clk 为单位的 SDA 保持时间 */
#define I2C_SDA_HOLD_Pos (0UL) /*!< Position of I2C_SDA_HOLD field. */
#define I2C_SDA_HOLD_Msk (0xFFFFUL << I2C_RXFLR_Pos) /*!< Bit mask of I2C_SDA_HOLD field. */


/* Register: I2C_TX_ABRT_SOURCE */
/* Description: I2C 发送异常中止源寄存器 
指示 TX_ABRT 源，读该寄存器或者处理器发出清除所有中断时清 0。*/

/* Bit 0:  I2C ABRT_7B_ADDR_NOACK  */
/* Description： Master 处于 7 比特地址模式，地址字节无确认 */
#define I2C_TX_ABRT_SOURCE_ABRT_7B_ADDR_NOACK_Pos (0UL) /*!< Position of ABRT_7B_ADDR_NOACK field. */
#define I2C_TX_ABRT_SOURCE_ABRT_7B_ADDR_NOACK_Msk (0x1UL << I2C_TX_ABRT_SOURCE_ABRT_7B_ADDR_NOACK_Pos) 
/*!< Bit mask of ABRT_7B_ADDR_NOACK field. */

/* Bit 1:  I2C ABRT_10ADDR1_NOACK  */
/* Description： Master 处于 10 比特地址模
式，第一地址字节无确认 */
#define I2C_TX_ABRT_SOURCE_ABRT_10ADDR1_NOACK_Pos (1UL) /*!< Position of ABRT_10ADDR1_NOACK field. */
#define I2C_TX_ABRT_SOURCE_ABRT_10ADDR1_NOACK_Msk (0x1UL << I2C_TX_ABRT_SOURCE_ABRT_10ADDR1_NOACK_Pos)
 /*!< Bit mask of ABRT_10ADDR1_NOACK field. */

/* Bit 2:  I2C ABRT_10ADDR2_NOACK  */
/* Description： Master 处于 10 比特地址模
式，第二地址字节无确认 */
#define I2C_TX_ABRT_SOURCE_ABRT_10ADDR2_NOACK_Pos (2UL) /*!< Position of ABRT_10ADDR2_NOACK field. */
#define I2C_TX_ABRT_SOURCE_ABRT_10ADDR2_NOACK_Msk (0x1UL << I2C_TX_ABRT_SOURCE_ABRT_10ADDR2_NOACK_Pos)
 /*!< Bit mask of ABRT_10ADDR2_NOACK field. */


/* Bit 3:  I2C ABRT_TXDATA_NOACK  */
/* Description： Master 收到地址确认，但是
发送数据之后无确认 */
#define I2C_TX_ABRT_SOURCE_ABRT_TXDATA_NOACK_Pos (3UL) /*!< Position of ABRT_TXDATA_NOACK field. */
#define I2C_TX_ABRT_SOURCE_ABRT_TXDATA_NOACK_Msk (0x1UL << I2C_TX_ABRT_SOURCE_ABRT_TXDATA_NOACK_Pos)
 /*!< Bit mask of ABRT_TXDATA_NOACK field. */

/* Bit 4:  I2C ABRT_GCALL_NOACK  */
/* Description： Master 发出通用寻址而无
Slave 响应 */
#define I2C_TX_ABRT_SOURCE_ABRT_GCALL_NOACK_Pos (4UL) /*!< Position of ABRT_GCALL_NOACK field. */
#define I2C_TX_ABRT_SOURCE_ABRT_GCALL_NOACK_Msk (0x1UL << I2C_TX_ABRT_SOURCE_ABRT_GCALL_NOACK_Pos)
 /*!< Bit mask of ABRT_GCALL_NOACK field. */

/* Bit 5:  I2C ABRT_GCALL_READ  */
/* Description： Master 发出通用寻址，而处
理器紧接着发出读请求 */
#define I2C_TX_ABRT_SOURCE_ABRT_GCALL_READ_Pos (5UL) /*!< Position of ABRT_GCALL_READ field. */
#define I2C_TX_ABRT_SOURCE_ABRT_GCALL_READ_Msk (0x1UL << I2C_TX_ABRT_SOURCE_ABRT_GCALL_READ_Pos)
 /*!< Bit mask of ABRT_GCALL_READ field. */

/* Bit 7:  I2C ABRT_SBYTE_ACKDET  */
/* Description： Master 发送 Start Byte 而
Start Byte 得到确认 */
#define I2C_TX_ABRT_SOURCE_ABRT_SBYTE_ACKDET_Pos (7UL) /*!< Position of ABRT_SBYTE_ACKDET field. */
#define I2C_TX_ABRT_SOURCE_ABRT_SBYTE_ACKDET_Msk (0x1UL << I2C_TX_ABRT_SOURCE_ABRT_SBYTE_ACKDET_Pos)
 /*!< Bit mask of ABRT_SBYTE_ACKDET field. */

/* Bit 9:  I2C ABRT_SBYTE_NORSTRT  */
/* Description： Restart 被禁止，处理器企图
发出 Start Byte */
#define I2C_TX_ABRT_SOURCE_ABRT_SBYTE_NORSTRT_Pos (9UL) /*!< Position of ABRT_SBYTE_NORSTRT field. */
#define I2C_TX_ABRT_SOURCE_ABRT_SBYTE_NORSTRT_Msk (0x1UL << I2C_TX_ABRT_SOURCE_ABRT_SBYTE_NORSTRT_Pos)
 /*!< Bit mask of ABRT_SBYTE_NORSTRT field. */


/* Bit 10:  I2C ABRT_10B_RD_NORSTRT  */
/* Description： Master 发送 Start Byte 而
Start Byte 得到确认 */
#define I2C_TX_ABRT_SOURCE_ABRT_10B_RD_NORSTRT_Pos (10UL) /*!< Position of ABRT_10B_RD_NORSTRT field. */
#define I2C_TX_ABRT_SOURCE_ABRT_10B_RD_NORSTRT_Msk (0x1UL << I2C_TX_ABRT_SOURCE_ABRT_10B_RD_NORSTRT_Pos)
 /*!< Bit mask of ABRT_10B_RD_NORSTRT field. */

/* Bit 11:  I2C ABRT_MASTER_DIS  */
/* Description：  处 理 器 企 图 使 用 禁 止 的
Master */
#define I2C_TX_ABRT_SOURCE_ABRT_MASTER_DIS_Pos (10UL) /*!< Position of ABRT_MASTER_DIS field. */
#define I2C_TX_ABRT_SOURCE_ABRT_MASTER_DIS_Msk (0x1UL << I2C_TX_ABRT_SOURCE_ABRT_MASTER_DIS_Pos)
 /*!< Bit mask of ABRT_MASTER_DIS field. */

/* Bit 12:  I2C ABRT_LOST  */
/* Description： ：Master 丢失总线控制权，或
者本寄存器 bit14 置 1，表明发
送数据的 Slave 丢失总线控制
权。 */
#define I2C_TX_ABRT_SOURCE_ABRT_LOST_Pos (10UL) /*!< Position of ABRT_LOST field. */
#define I2C_TX_ABRT_SOURCE_ABRT_LOST_Msk (0x1UL << I2C_TX_ABRT_SOURCE_ABRT_LOST_Pos)
 /*!< Bit mask of ABRT_LOST field. */


////////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////XINCHIP SPI   //////////////////////////////////////////////////////


/* Peripheral: SPI / SSI */
/* Description: SPI compatible Interface  */

/* Register: SSI_CTRL0 */
/* Description: SSI 控制寄存器 0 
该寄存器控制串行数据的传输。当 SSI0 使能时，不能修改该寄存器的值。
*/
/* Bit 0..3:  SSI DFS  */
/* Description： ：数据帧长度： 
0x00~0x02：保留
0x03~0x0F：对应 4~16 比特串行数据传输
*/
#define SSI_SSI_CTRL0_DFS_Pos (0UL) /*!< Position of DFS field. */
#define SSI_SSI_CTRL0_DFS_Msk (0xFUL << SSI_SSI_CTRL0_DFS_Pos)
#define SSI_SSI_CTRL0_DFS_LEN_4bits (0x03UL) 
#define SSI_SSI_CTRL0_DFS_LEN_5bits (0x04UL) 
#define SSI_SSI_CTRL0_DFS_LEN_6bits (0x05UL) 
#define SSI_SSI_CTRL0_DFS_LEN_7bits (0x06UL) 
#define SSI_SSI_CTRL0_DFS_LEN_8bits (0x07UL) 
#define SSI_SSI_CTRL0_DFS_LEN_9bits (0x08UL) 
#define SSI_SSI_CTRL0_DFS_LEN_10bits (0x09UL) 
#define SSI_SSI_CTRL0_DFS_LEN_11bits (0x0aUL) 
#define SSI_SSI_CTRL0_DFS_LEN_12bits (0x0bUL) 
#define SSI_SSI_CTRL0_DFS_LEN_13bits (0x0cUL) 
#define SSI_SSI_CTRL0_DFS_LEN_14bits (0x0dUL) 
#define SSI_SSI_CTRL0_DFS_LEN_15bits (0x0eUL) 
#define SSI_SSI_CTRL0_DFS_LEN_16bits (0x0fUL) 

/* Bit 4..5:  SSI FRF  */
/* Description： 帧格式控制： 
00：
Motorola SPI 协议
01：TI SSP 协议
*/
#define SSI_SSI_CTRL0_FRF_Pos (4UL) /*!< Position of FRF field. */
#define SSI_SSI_CTRL0_FRF_Msk (0x3UL << SSI_SSI_CTRL0_FRF_Pos)
#define SSI_SSI_CTRL0_FRF_Motorola (0UL)
#define SSI_SSI_CTRL0_FRF_Ti        (1UL)


/* Bit 6:  SSI SCPHA  */
/* Description： 串行时钟的相位控制：控制串口时序，决定何时
开始传输数据。
*/
#define SSI_SSI_CTRL0_SCPHA_Pos (6UL) /*!< Position of SCPHA field. */
#define SSI_SSI_CTRL0_SCPHA_Msk (0x1UL << SSI_SSI_CTRL0_SCPHA_Pos)
#define SSI_SSI_CTRL0_SCPHA_Leading (0UL)/*!< 在 ssi0_ssn0/ssi0_ssn1 的下降沿，开始数据传输 */
#define SSI_SSI_CTRL0_SCPHA_Trailing (1UL)/*!< 在 ssi0_ssn0/ssi0_ssn1 有效后的第一个ssi0_clk 的变化沿，开始数据传输 */


/* Bit 7:  SSI SCPOL  */
/* Description： 串行时钟的相位控制：控制串口时序，决定何时
开始传输数据。
*/
#define SSI_SSI_CTRL0_SCPOL_Pos (7UL) /*!< Position of SCPOL field. */
#define SSI_SSI_CTRL0_SCPOL_Msk (0x1UL << SSI_SSI_CTRL0_SCPOL_Pos)
#define SSI_SSI_CTRL0_SCPOL_ActiveHigh (1UL)/*!< 在ssi0_clk 停止时，ssi0_clk 保持高电平 */
#define SSI_SSI_CTRL0_SCPOL_ActiveLow (0UL)/*!< ssi0_clk 停止时，ssi0_clk 保持低电平 */

/* Bit 8..9:  SSI TMOD  */
/* Description： 传输模式：用于控制 SSI0 的传输模式。该控制
不影响 SSI0 传输的双工性，只指示发送/接收的
数据是否有效。
*/
#define SSI_SSI_CTRL0_TMOD_Pos (8UL) /*!< Position of TMOD field. */
#define SSI_SSI_CTRL0_TMOD_Msk (0x3UL << SSI_SSI_CTRL0_TMOD_Pos)
#define SSI_SSI_CTRL0_TMOD_WR (0UL)/*!< 收发 */
#define SSI_SSI_CTRL0_TMOD_W (1UL)/*!< 仅发 */
#define SSI_SSI_CTRL0_TMOD_R (2UL)/*!< 仅收 */

/* Bit 11:  SSI SRL  */
/* Description：移位寄存器环回：控制进入测试模式。该模式下，
连接发送寄存器的输出到接收寄存器的输入。
*/
#define SSI_SSI_CTRL0_SRL_Pos (11UL) /*!< Position of SRL field. */
#define SSI_SSI_CTRL0_SRL_Msk (0x1UL << SSI_SSI_CTRL0_SRL_Pos)
#define SSI_SSI_CTRL0_SRL_Normal (0UL)/*!< 正常模式 */
#define SSI_SSI_CTRL0_SRL_Lookback (1UL)/*!< 环回模式 */


/* Register: SSI_CTRL1 */
/* Description: SSI 控制寄存器 1 在“仅收”模式下，该寄存器可以控制串口传输何时结束。
当 SSI0 使能时，不能修改该寄存器的值。
*/
/* Bit 0..15:  SSI NDF  */
/* Description： ：数据帧的个数：当 SSI0 工作在“仅收” 模式
下（TMOD=10），控制 SSI0 持续接收的数据帧
个数。该 NDF 值为 N，SSI0 将持续接收 N＋1
个数据后，结束串口传输。SSI0 最多连续接收
64K 个数据帧。
*/
#define SSI_SSI_CTRL1_NDF_Pos (0UL) /*!< Position of NDF field. */
#define SSI_SSI_CTRL1_NDF_Msk (0xFFFFUL << SSI_SSI_CTRL1_NDF_Pos)


/* Register: SSI_EN */
/* Description: 使能寄存器
*/
/* Bit 0:  SSI SEN  */
/* Description： ：SSI 使能控制：SSI0 禁用时，所有收发立即停
止，并清空接收 FIFO 和发送 FIFO。SSI0 使能
时，不能修改某些控制寄存器的值。在 SSI0 禁
用时，可以节省系统的功耗。
*/
#define SSI_SSI_EN_SEN_Pos (0UL) /*!< Position of SEN field. */
#define SSI_SSI_EN_SEN_Msk (0x1UL << SSI_SSI_EN_SEN_Pos)
#define SSI_SSI_EN_SEN_Enable (1UL) /*!< 使能. */
#define SSI_SSI_EN_SEN_Disable (0UL) /*!< 不使能 */

/* Register: SSI_SE */
/* Description: 通过该寄存器使能来自 SSI0 的从设备选择。当 SSI0 使能或者 SSI0 处于 BUSY 状态，
不能写该寄存器。
*/
/* Bit 0:  SSI SS0  */
/* Description： ：从设备选择 0：如果设置为‘1’，从设备使能
信号（ssi0_ssn0）会在传输开始时有效。对该
bit 写‘0’或‘1’，只有在传输开始后，才会
在端口信号 ssi0_ssn0 上有所体现。
*/
#define SSI_SSI_SE_SS0_Pos (0UL) /*!< Position of SS0 field. */
#define SSI_SSI_SE_SS0_Msk (0x1UL << SSI_SSI_SE_SS0_Pos)
#define SSI_SSI_SE_SS0_Select (1UL) /*!< 1：选择从设备 0 */
#define SSI_SSI_SE_SS0_NotSelect (0UL) /*!< 0：不选择从设备 0 */


/* Bit 1:  SSI SS1  */
/* Description： ：从设备选择 1：如果设置为‘1’，从设备使能
信号（ssi0_ssn0）会在传输开始时有效。对该
bit 写‘0’或‘1’，只有在传输开始后，才会
在端口信号 ssi0_ssn1 上有所体现。
*/
#define SSI_SSI_SE_SS1_Pos (1UL) /*!< Position of SS1 field. */
#define SSI_SSI_SE_SS1_Msk (0x1UL << SSI_SSI_SE_SS1_Pos)
#define SSI_SSI_SE_SS1_Select (1UL) /*!< 1：选择从设备 1 */
#define SSI_SSI_SE_SS1_NotSelect (0UL) /*!< 0：不选择从设备 1 */


/* Register: SSI_BAUD */
/* Description: SSI 波特率选择寄存器
通过该寄存器使能来自 SSI 的从设备选择。当 SSI 使能或者 SSI 处于 BUSY 状态，
不能写该寄存器。
*/
/* Bit 0..15:  SSI SCKDV  */
/* Description： ：SSI 时钟分频：最低位固定为 0，所以分频比
为偶数。如果该位为 0，那么相应的 ssi0_clk
没有时钟输出。ssi0_clk 有如下公式：
ssi0_clk = ssi0_mclk/SCKDV
SCKDV 是 2 到 65534 之间的偶数。
*/
#define SSI_SSI_BAUD_SCKDV_Pos (0UL) /*!< Position of SCKDV field. */
#define SSI_SSI_BAUD_SCKDV_Msk (0xFFFFFUL << SSI_SSI_BAUD_SCKDV_Pos)

#define SSI_FREQUENCY_FREQUENCY_K125 (0x00000100UL) /*!< 125 kbps */
#define SSI_FREQUENCY_FREQUENCY_K250 (0x00000080UL) /*!< 250 kbps */
#define SSI_FREQUENCY_FREQUENCY_K500 (0x00000040UL) /*!< 500 kbps */

#define SSI_FREQUENCY_FREQUENCY_M1  (0x00000020UL) /*!< 1 Mbps */
#define SSI_FREQUENCY_FREQUENCY_M2  (0x00000010UL) /*!< 2 Mbps */
#define SSI_FREQUENCY_FREQUENCY_M4  (0x00000008UL) /*!< 4 Mbps */
#define SSI_FREQUENCY_FREQUENCY_M8  (0x00000004UL) /*!< 8 Mbps */ 
#define SSI_FREQUENCY_FREQUENCY_M16 (0x00000002UL) /*!< 16 Mbps */  


/* Register: SSI_TXFTL */
/* Description: SSI0 发送 FIFO 阈值寄存器
该寄存器设置发送 FIFO 的阈值。当 SSI0 使能时，不能写该寄存器。
*/
/* Bit 0..2  SSI TFT  */
/* Description： ：发送 FIFO 阈值：用于设置触发发送空中断
（TXE_INTR）的发送 FIFO 阈值。
0x00~0x07：分别对应发送 FIFO 中数据少于等于
0~7 时，触发发送空中断（TXE_INTR）。
*/
#define SSI_SSI_TXFTL_TFT_Pos (0UL) /*!< Position of TFT field. */
#define SSI_SSI_TXFTL_TFT_Msk (0x7UL << SSI_SSI_TXFTL_TFT_Pos)

/* Register: SSI_RXFTL */
/* Description: SSI 接收 FIFO 阈值寄存器
该寄存器设置发送 FIFO 的阈值。当 SSI 使能时，不能写该寄存器。
*/
/* Bit 0..2  SSI RFT  */
/* Description： ：接收 FIFO 阈值：用于设置触发接收满中断
（RXF_INTR）的接收 FIFO 阈值。
0x00~0x07：分别对应接收 FIFO 中数据大于等于
1~8 时，触发接收满中断（RXF_INTR）。
*/
#define SSI_SSI_RXFTL_RFT_Pos (0UL) /*!< Position of RFT field. */
#define SSI_SSI_RXFTL_RFT_Msk (0x7UL << SSI_SSI_RXFTL_RFT_Pos)



/* Register: SSI_TXFL */
/* Description: SSI 发送 FIFO 有效值寄存器
该寄存器包含发送 FIFO 有效数据个数。
*/
/* Bit 0..3  SSI TXTFL  */
/* Description：发送 FIFO 有效值：发送 FIFO 有效数据个数
*/
#define SSI_SSI_TXFL_TXTFL_Pos (0UL) /*!< Position of TXTFL field. */
#define SSI_SSI_TXFL_TXTFL_Msk (0xFUL << SSI_SSI_TXFL_TXTFL_Pos)



/* Register: SSI_RXFL */
/* Description: SSI 接收 FIFO 有效值寄存器
该寄存器包含接收 FIFO 有效数据个数。
*/
/* Bit 0..3  SSI RXTFL  */
/* Description：接收 FIFO 有效值：接收 FIFO 有效数据个数。
*/
#define SSI_SSI_RXFL_RXTFL_Pos (0UL) /*!< Position of RXTFL field. */
#define SSI_SSI_RXFL_RXTFL_Msk (0xFUL << SSI_SSI_RXFL_RXTFL_Pos)



/* Register: SSI_STS */
/* Description: SSI 状态寄存器
该寄存器用于指示 SSI0 当前传输状态、FIFO 状态和可能发生的发送/接收错误。
*/
/* Bit 0:  SSI BUSY  */
/* Description：SSI0 BUSY 标志：如果该比特设置，标志 SSI0
正在进行串行传输；如果 SSI0 处于空闲或者不
使能的状态，该比特清零。
*/
#define SSI_SSI_STS_BUSY_Pos (0UL) /*!< Position of BUSY field. */
#define SSI_SSI_STS_BUSY_Msk (0x1UL << SSI_SSI_STS_BUSY_Pos)
#define SSI_SSI_STS_BUSY_Busy (1UL) /*!< SSI 正在进行串行传输. */
#define SSI_SSI_STS_BUSY_Idle (0UL)  /*!< SSI 空闲或者不使能. */

/* Bit 1:  SSI TFNF  */
/* Description：发送 FIFO 非满：当发送 FIFO 有一个或者多个
空位时，该比特设置。当发送 FIFO 完全满时（8
个），该比特清零。
*/
#define SSI_SSI_STS_TFNF_Pos (1UL) /*!< Position of TFNF field. */
#define SSI_SSI_STS_TFNF_Msk (0x1UL << SSI_SSI_STS_TFNF_Pos)
#define SSI_SSI_STS_TFNF_NotFull (1UL) /*!< 发送 FIFO 未满. */
#define SSI_SSI_STS_TFNF_Full (0UL)  /*!< 发送 FIFO 满 */


/* Bit 2:  SSI TFE  */
/* Description：发送 FIFO 空：当发送 FIFO 完全空时（0 个），
该比特设置。当发送 FIFO 包含一个或多个数据
时，该比特清零。
*/
#define SSI_SSI_STS_TFE_Pos (2UL) /*!< Position of TFE field. */
#define SSI_SSI_STS_TFE_Msk (0x1UL << SSI_SSI_STS_TFE_Pos)
#define SSI_SSI_STS_TFE_Empty (1UL) /*!< 发送 FIFO 空. */
#define SSI_SSI_STS_TFE_NotEmpty (0UL)  /*!<发送 FIFO 非空 */

/* Bit 3:  SSI RFNE  */
/* Description：接收 FIFO 非空：当接收 FIFO 包含有一个或者
多个数据时，该比特设置。当接收 FIFO 为空时，
该比特清零。
*/
#define SSI_SSI_STS_RFNE_Pos (3UL) /*!< Position of RFNE field. */
#define SSI_SSI_STS_RFNE_Msk (0x1UL << SSI_SSI_STS_RFNE_Pos)
#define SSI_SSI_STS_RFNE_Empty (0UL) /*!< 接收 FIFO 空. */
#define SSI_SSI_STS_RFNE_NotEmpty (1UL)  /*!<接收 FIFO 非空 */


/* Bit 3:  SSI RFF  */
/* Description：接收 FIFO 满：当接收 FIFO 完全满时（8 个），
该比特设置。当接收 FIFO 有一个或者多个空位
时，该比特清零。
*/
#define SSI_SSI_STS_RFF_Pos (4UL) /*!< Position of RFF field. */
#define SSI_SSI_STS_RFF_Msk (0x1UL << SSI_SSI_STS_RFF_Pos)
#define SSI_SSI_STS_RFF_Full (1UL) /*!< 接收 FIFO 满. */
#define SSI_SSI_STS_RFF_NotFull (0UL)  /*!<接收 FIFO 未满 */



/* Register: SSI_IE */
/* Description: SSI 中断使能寄存器
该寄存器使能或者不使能所有的 SSI0 产生的中断。
*/
/* Bit 0:  SSI TXEIE  */
/* Description：发送 FIFO 空中断使能
*/
#define SSI_SSI_IE_TXEIE_Pos (0UL) /*!< Position of TXEIE field. */
#define SSI_SSI_IE_TXEIE_Msk (0x1UL << SSI_SSI_IE_TXEIE_Pos)
#define SSI_SSI_IE_TXEIE_Enable (1UL) /*!< TXE_INTR 中断使能. */
#define SSI_SSI_IE_TXEIE_Disable (0UL)  /*!<TXE_INTR 中断不使能 */

/* Bit 1:  SSI TXOIE  */
/* Description：发送 FIFO 上溢出中断使能
*/
#define SSI_SSI_IE_TXOIE_Pos (1UL) /*!< Position of TXOIE field. */
#define SSI_SSI_IE_TXOIE_Msk (0x1UL << SSI_SSI_IE_TXOIE_Pos)
#define SSI_SSI_IE_TXOIE_Enable (1UL) /*!< TXOIE 中断使能. */
#define SSI_SSI_IE_TXOIE_Disable (0UL)  /*!<TXOIE 中断不使能 */

/* Bit 2:  SSI RXUIE  */
/* Description：接收 FIFO 下溢出中断使能
*/
#define SSI_SSI_IE_RXUIE_Pos (2UL) /*!< Position of RXUIE field. */
#define SSI_SSI_IE_RXUIE_Msk (0x1UL << SSI_SSI_IE_RXUIE_Pos)
#define SSI_SSI_IE_RXUIE_Enable (1UL) /*!< RXUIE 中断使能. */
#define SSI_SSI_IE_RXUIE_Disable (0UL)  /*!<RXUIE 中断不使能 */


/* Bit 3:  SSI RXOIE  */
/* Description：接收 FIFO 上溢出中断使能
*/
#define SSI_SSI_IE_RXOIE_Pos (3UL) /*!< Position of RXOIE field. */
#define SSI_SSI_IE_RXOIE_Msk (0x1UL << SSI_SSI_IE_RXOIE_Pos)
#define SSI_SSI_IE_RXOIE_Enable (1UL) /*!< RXOIE 中断使能. */
#define SSI_SSI_IE_RXOIE_Disable (0UL)  /*!<RXOIE 中断不使能 */

/* Bit 4:  SSI RXFIE  */
/* Description：接收 FIFO 满中断使能
*/
#define SSI_SSI_IE_RXFIE_Pos (4UL) /*!< Position of RXFIE field. */
#define SSI_SSI_IE_RXFIE_Msk (0x1UL << SSI_SSI_IE_RXFIE_Pos)
#define SSI_SSI_IE_RXFIE_Enable (1UL) /*!< RXFIE 中断使能. */
#define SSI_SSI_IE_RXFIE_Disable (0UL)  /*!<RXFIE 中断不使能 */




/* Register: SSI_IS */
/* Description: 中断状态寄存器
该寄存器为 SSI 的中断经过使能后的状态。
*/
/* Bit 0:  SSI TXEIS  */
/* Description：发送 FIFO 空中断状态
*/
#define SSI_SSI_IS_TXEIS_Pos (0UL) /*!< Position of TXEIS field. */
#define SSI_SSI_IS_TXEIS_Msk (0x1UL << SSI_SSI_IS_TXEIS_Pos)
#define SSI_SSI_IS_TXEIS_Generated (1UL) /*!< TXE_INTR 中断经使能后有效. */
#define SSI_SSI_IS_TXEIS_NotGenerated (0UL)  /*!<TXE_INTR 中断经使能后无效 */

/* Bit 1:  SSI TXOIS  */
/* Description：发送 FIFO 上溢出中断状态
*/
#define SSI_SSI_IS_TXOIS_Pos (1UL) /*!< Position of TXOIS field. */
#define SSI_SSI_IS_TXOIS_Msk (0x1UL << SSI_SSI_IS_TXOIS_Pos)
#define SSI_SSI_IS_TXOIS_Generated (1UL) /*!< TXOIE 中断经使能后有效. */
#define SSI_SSI_IS_TXOIS_NotGenerated (0UL)  /*!<TXOIE 中断经使能后无效 */

/* Bit 2:  SSI RXUIS  */
/* Description：接收 FIFO 下溢出中断状态
*/
#define SSI_SSI_IS_RXUIS_Pos (2UL) /*!< Position of RXUIS field. */
#define SSI_SSI_IS_RXUIS_Msk (0x1UL << SSI_SSI_IS_RXUIS_Pos)
#define SSI_SSI_IS_RXUIS_Generated (1UL) /*!< RXUIE 中断经使能后有效. */
#define SSI_SSI_IS_RXUIS_NotGenerated (0UL)  /*!<RXUIE 中断经使能后无效 */


/* Bit 3:  SSI RXOIS  */
/* Description：接收 FIFO 上溢出中断状态
*/
#define SSI_SSI_IS_RXOIS_Pos (3UL) /*!< Position of RXOIS field. */
#define SSI_SSI_IS_RXOIS_Msk (0x1UL << SSI_SSI_IS_RXOIS_Pos)
#define SSI_SSI_IS_RXOIS_Generated (1UL) /*!< RXOIS 中断经使能后有效. */
#define SSI_SSI_IS_RXOIS_NotGenerated (0UL)  /*!<RXOIS 中断经使能后无效 */

/* Bit 4:  SSI RXFIS  */
/* Description：接收 FIFO 满中断状态
*/
#define SSI_SSI_IS_RXFIS_Pos (4UL) /*!< Position of RXFIS field. */
#define SSI_SSI_IS_RXFIS_Msk (0x1UL << SSI_SSI_IS_RXFIS_Pos)
#define SSI_SSI_IS_RXFIS_Generated (1UL) /*!< RXFIS 中断经使能后有效. */
#define SSI_SSI_IS_RXFIS_NotGenerated (0UL)  /*!<RXFIS 中断经使能后无效 */



/* Register: SSI_RIS */
/* Description: 原始中断状态寄存器
该寄存器为 SSI 的中断未经使能的状态。
*/
/* Bit 0:  SSI TXEIR  */
/* Description：发送 FIFO 空原始中断状态
*/
#define SSI_SSI_RIS_TXEIR_Pos (0UL) /*!< Position of TXEIS field. */
#define SSI_SSI_RIS_TXEIR_Msk (0x1UL << SSI_SSI_RIS_TXEIR_Pos)
#define SSI_SSI_RIS_TXEIR_Generated (1UL) /*!< TXE_INTR 未经使能中断有效. */
#define SSI_SSI_RIS_TXEIR_NotGenerated (0UL)  /*!<TXE_INTR 未经使能中断无效 */

/* Bit 1:  SSI TXOIR  */
/* Description：发送 FIFO 上溢出原始中断状态
*/
#define SSI_SSI_RIS_TXOIR_Pos (1UL) /*!< Position of TXOIR field. */
#define SSI_SSI_RIS_TXOIR_Msk (0x1UL << SSI_SSI_RIS_TXOIR_Pos)
#define SSI_SSI_RIS_TXOIR_Generated (1UL) /*!< TXOIE 未经使能中断有效. */
#define SSI_SSI_RIS_TXOIR_NotGenerated (0UL)  /*!<TXOIE 未经使能中断无效 */

/* Bit 2:  SSI RXUIR  */
/* Description：接收 FIFO 下溢出原始中断状态
*/
#define SSI_SSI_RIS_RXUIR_Pos (2UL) /*!< Position of RXUIR field. */
#define SSI_SSI_RIS_RXUIR_Msk (0x1UL << SSI_SSI_RIS_RXUIR_Pos)
#define SSI_SSI_RIS_RXUIR_Generated (1UL) /*!< RXUIE 中断有效. */
#define SSI_SSI_RIS_RXUIR_NotGenerated (0UL)  /*!<RXUIE 中断无效 */


/* Bit 3:  SSI RXOIR  */
/* Description：接收 FIFO 上溢出原始中断状态
*/
#define SSI_SSI_RIS_RXOIR_Pos (3UL) /*!< Position of RXOIR field. */
#define SSI_SSI_RIS_RXOIR_Msk (0x1UL << SSI_SSI_IS_SSI_SSI_RIS_RXOIR_PosRXOIS_Pos)
#define SSI_SSI_RIS_RXOIR_Generated (1UL) /*!< RXOIS 未经使能中断有效. */
#define SSI_SSI_RIS_RXOIR_NotGenerated (0UL)  /*!<RXOIS未经使能 中断无效 */

/* Bit 4:  SSI RXFIR  */
/* Description：接收 FIFO 满原始中断状态
*/
#define SSI_SSI_RIS_RXFIR_Pos (4UL) /*!< Position of RXFIR field. */
#define SSI_SSI_RIS_RXFIR_Msk (0x1UL << SSI_SSI_RIS_RXFIR_Pos)
#define SSI_SSI_RIS_RXFIR_Generated (1UL) /*!< RXFIS 未经使能中断有效. */
#define SSI_SSI_RIS_RXFIR_NotGenerated (0UL)  /*!<RXFIS 未经使能中断无效 */


/* Register: SSI_TXOIC */
/* Description:发送 FIFO 上溢出中断清除寄存器 
*/
/* Bit 0:  SSI TXOIC  */
/* Description：清除发送 FIFO 上溢出中断：寄存器值为相应中
断状态。读该寄存器清除 TXO_INTR 中断，写
无效。
*/
#define SSI_SSI_TXOIC_TXOIC_Pos (0UL) /*!< Position of TXOIC field. */
#define SSI_SSI_TXOIC_TXOIC_Msk (0x1UL << SSI_SSI_TXOIC_TXOIC_Pos)


/* Register: SSI_RXOIC */
/* Description:接收 FIFO 上溢出中断清除寄存器
*/
/* Bit 0:  SSI RXOIC  */
/* Description：清除接收 FIFO 上溢出中断：寄存器值为相应中
断状态。读该寄存器清除 RXO_INTR 中断，写
无效。
*/
#define SSI_SSI_RXOIC_RXOIC_Pos (0UL) /*!< Position of RXOIC field. */
#define SSI_SSI_RXOIC_RXOIC_Msk (0x1UL << SSI_SSI_RXOIC_RXOIC_Pos)


/* Register: SSI_RXUIC */
/* Description:接收 FIFO 下溢中断清除寄存器
*/
/* Bit 0:  SSI RXUIC  */
/* Description：清除接收 FIFO 下溢出中断：寄存器值为相应中
断状态。读该寄存器清除 RXU_INTR 中断，写
无效。
*/
#define SSI_SSI_RXUIC_RXUIC_Pos (0UL) /*!< Position of RXUIC field. */
#define SSI_SSI_RXUIC_RXUIC_Msk (0x1UL << SSI_SSI_RXUIC_RXUIC_Pos)


/* Register: SSI_IC */
/* Description:SSI0 中断清除寄存器
*/
/* Bit 0:  SSI IC  */
/* Description：清除中断：如果 TXO_INTR、RXO_INTR、
RXU_INTR 中断状态有效，那么寄存器值为
‘1’，否则为‘0’。读该寄存器清除 TXO_INTR、
RXO_INTR、RXU_INTR 中断，写无效。
*/
#define SSI_SSI_IC_IC_Pos (0UL) /*!< Position of IC field. */
#define SSI_SSI_IC_IC_Msk (0x1UL << SSI_SSI_IC_IC_Pos)


/* Register: SSI_DMAS */
/* Description:DMA 控制寄存器
*  该寄存器控制与 DMAS 的接口是否使能。
*/
/* Bit 0:  SSI RDMAE  */
/* Description：接收 DMA 接口使能：
使能/不使能接收的 DMA接口
*/
#define SSI_SSI_DMAS_RDMAE_Pos (0UL) /*!< Position of RDMAE field. */
#define SSI_SSI_DMAS_RDMAE_Msk (0x1UL << SSI_SSI_DMAS_RDMAE_Pos)
#define SSI_SSI_DMAS_RDMAE_Enable (1UL) /*!< 接收 DMA 接口使能. */
#define SSI_SSI_DMAS_RDMAE_Disable (0UL)  /*!< 接收 DMA 接口不使能 */

/* Bit 1:  SSI TDMAE  */
/* Description：发送 DMA 接口使能：
使能/不使能发送的 DMA接口
*/
#define SSI_SSI_DMAS_TDMAE_Pos (1UL) /*!< Position of TDMAE field. */
#define SSI_SSI_DMAS_TDMAE_Msk (0x1UL << SSI_SSI_DMAS_TDMAE_Pos)
#define SSI_SSI_DMAS_TDMAE_Enable (1UL) /*!< 发送 DMA 接口使能. */
#define SSI_SSI_DMAS_TDMAE_Disable (0UL)  /*!< 发送 DMA 接口不使能 */


/* Register: SSI_DMATDL */
/* Description:DMA 发送阈值寄存器
*/
/* Bit 0..2:  SSI DMATDL  */
/* Description：DMA 发送数据阈值：控制发送数据过程中，产
生 DMA 传输请求的阈值；如果发送 FIFO 中的
数据个数等于小于本阈值，并且 TDMAE=1，发
出 DMA 传输请求。
*/
#define SSI_SSI_DMATDL_DMATDL_Pos (0UL) /*!< Position of DMATDL field. */
#define SSI_SSI_DMATDL_DMATDL_Msk (0x7UL << SSI_SSI_DMATDL_DMATDL_Pos)




/* Register: SSI_DMARDL */
/* Description:DMA 发送阈值寄存器
*/
/* Bit 0..2:  SSI DMARDL  */
/* Description：DMA 接收数据阈值：控制接收数据过程中，产
生 DMA 传输请求的阈值；如果接收 FIFO 中的
数据个数大于本阈值，并且 RDMAE=1，发出
DMA 传输请求。
*/
#define SSI_SSI_DMARDL_DMARDL_Pos (0UL) /*!< Position of DMARDL field. */
#define SSI_SSI_DMARDL_DMARDL_Msk (0x7UL << SSI_SSI_DMARDL_DMARDL_Pos)


/* Register: SSI_DATA */
/* Description:SSI 数据寄存器
*/
/* Bit 0..15:  SSI DR  */
/* Description：数据寄存器：由于数据帧小于等于 16bits，写
数据时保证右对齐；读数据时，返回的数据也
是自动右对齐的。
*/
#define SSI_SSI_DATA_DR_Pos (0UL) /*!< Position of DR field. */
#define SSI_SSI_DATA_DR_Msk (0xFFFFUL << SSI_SSI_DATA_DR_Pos)






#define SPIM_CONFIG_ORDER_MsbFirst (0UL) /*!< Most significant bit shifted out first */
#define SPIM_CONFIG_ORDER_LsbFirst (1UL) /*!< Least significant bit shifted out first */
// /* Description: Polarity of CSN output */
#define SPIM_CSNPOL_CSNPOL_LOW (0UL) /*!< Active low (idle state high) */
#define SPIM_CSNPOL_CSNPOL_HIGH (1UL) /*!< Active high (idle state low) */



///////////////////////////////////////////////////////////////////////XINCHIP SPI END  //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////XINCHIP UART  //////////////////////////////////////////////////////

/* Peripheral: UART  */
/* Description: UART compatible Interface */

/* Register: UARTx_RBR */
/* Description: 接收缓存寄存器
当 DLAB=0 时，对地址偏移 0x00 作“读”操作，访问的是该寄存器。
*/
/* Bit 0..7:  UART RBR  */
/* Description：暂 存 来自 串行 输 入 端口 的 数据 ：在
UARTx_TSR 传输状态寄存器的 DR=1
时 ， 标志 数据 已 经准备 好 ，可 以对
UARTx_RBR 进行读操作。对其进行读
操作时，UARTx_RBR 会自动读取接收
FIFO 中的数据
*/
#define UART_UARTx_RBR_RBR_Pos (0UL) /*!< Position of RBR field. */
#define UART_UARTx_RBR_RBR_Msk (0xFFUL << UART_UARTx_RBR_RBR_Pos)



/* Register: UARTx_THR */
/* Description: 发送保持寄存器
当 DLAB=0 时，对地址偏移 0x00 作“写”操作，访问的是该寄存器。
*/
/* Bit 0..7:  UART THR  */
/* Description：暂存即将通过串行输出端口输出的数据：
在 UARTx_TSR 传输状态寄存器的 THRE=1
时，有两个含义：
1. 当 UARTx_TSR 传输状态寄存器 THRE=1
表示“发送保持寄存器空中断”时，可以往
UARTx_THR 寄存器中写数据；
2. 当 UARTx_TSR 传输状态寄存器 THRE=1
表 示 “FIFO 满 ” 时 ， 应 该 等 待 ， 不 能 往
UARTx_THR 寄存器中写数据，否则新写入
数据会丢失。
当 对 UARTx_THR 进 行 写 操 作 时 ，
UARTx_THR 会自动将数据写入发送 FIFO。
*/
#define UART_UARTx_THR_THR_Pos (0UL) /*!< Position of THR field. */
#define UART_UARTx_THR_THR_Msk (0xFFUL << UART_UARTx_THR_THR_Pos)



/* Register: UARTx_DLL */
/* Description: 除数锁存低位
当 DLAB=1 时，对地址偏移 0x00 作“读/写”操作，访问的是该寄存器。
设置除数寄存器后，至少要等待（2×除数×16）个 uartx_clk（x=0，1）时钟周期后才
能开始收发数据。在对 UARTx_DLL（x=0，1）操作前必须确保 DLAB=1，
并且 UARTx（x=0， 1）不忙（UARTx_USR（x=0，1）的 bit[0]为 0），否则操作无效
*/
/* Bit 0..7:  UART DLL  */
/* Description：锁存除数：用于存 16 位除数的低 8 位，该除数用于
控制波特率。
波特率＝（串行时钟（uartx_clk 频率）/（16×除数）
*/
#define UART_UARTx_DLL_DLL_Pos (0UL) /*!< Position of DLL field. */
#define UART_UARTx_DLL_DLL_Msk (0xFFUL << UART_UARTx_DLL_DLL_Pos)



/* Register: UARTx_IER */
/* Description:中断使能寄存器
当 DLAB=0 时，对地址偏移 0x04 作“读/写”操作，访问的是该寄存器。
*/

/* Bit 0:  UART ERDAI  */
/* Description：使能接收数据有效中断和字符超时中断
中断优先级 2
*/
#define UART_UARTx_IER_ERDAI_Pos (0UL) /*!< Position of ERDAI field. */
#define UART_UARTx_IER_ERDAI_Msk (0x1UL << UART_UARTx_IER_ERDAI_Pos)
#define UART_UARTx_IER_ERDA_Enable (1UL)/*!< 使能. */
#define UART_UARTx_IER_ERDA_Disable (0UL)/*!< 不使能. */


/* Bit 1:  UART ETHEI  */
/* Description：使能发送保持寄存器 THR 空中断
中断优先级 3
*/
#define UART_UARTx_IER_ETHEI_Pos (1UL) /*!< Position of ETHEI field. */
#define UART_UARTx_IER_ETHEI_Msk (0x1UL << UART_UARTx_IER_ETHEI_Pos)
#define UART_UARTx_IER_ETHEI_Enable (1UL)/*!< 使能. */
#define UART_UARTx_IER_ETHEI_Disable (0UL)/*!< 不使能. */

/* Bit 2:  UART ETSI  */
/* Description： 使能错误中断：
中断优先级 1
*/
#define UART_UARTx_IER_ETSI_Pos (2UL) /*!< Position of ETSI field. */
#define UART_UARTx_IER_ETSI_Msk (0x1UL << UART_UARTx_IER_ETSI_Pos)
#define UART_UARTx_IER_ETSI_Enable (1UL)/*!< 使能. */
#define UART_UARTx_IER_ETSI_Disable (0UL)/*!< 不使能. */

/* Bit 3:  UART EMSI  */
/* Description： 使能流控状态改变中断：
中断优先级 4
*/
#define UART_UARTx_IER_EMSI_Pos (3UL) /*!< Position of EMSI field. */
#define UART_UARTx_IER_EMSI_Msk (0x1UL << UART_UARTx_IER_EMSI_Pos)
#define UART_UARTx_IER_EMSI_Enable (1UL)/*!< 使能. */
#define UART_UARTx_IER_EMSI_Disable (0UL)/*!< 不使能. */

/* Bit 7:  UART PTIME  */
/* Description：设置可编程 THRE 中断：
*/
#define UART_UARTx_IER_PTIME_Pos (7UL) /*!< Position of EMSI field. */
#define UART_UARTx_IER_PTIME_Msk (0x1UL << UART_UARTx_IER_PTIME_Pos)
#define UART_UARTx_IER_PTIME_Enable (1UL)/*!< 可编程 THRE 中断模式 */
#define UART_UARTx_IER_PTIME_Disable (0UL)/*!< 非可编程 THRE 中断模式. */


/* Register: UARTx_DLH */
/* Description: 除数锁存高位
当 DLAB=1 时，对地址偏移 0x00 作“读/写”操作，访问的是该寄存器。
设置除数寄存器后，至少要等待（2×除数×16）个 uartx_clk（x=0，1）时钟周期后才
能开始收发数据。在对 UARTx_DLH（x=0，1）操作前必须确保 DLAB=1，
并且 UARTx（x=0， 1）不忙（UARTx_USR（x=0，1）的 bit[0]为 0），否则操作无效
*/
/* Bit 0..7:  UART DLH  */
/* Description：锁存除数：用于存 16 位除数的高 8 位，该除数用于
控制波特率。
波特率＝（串行时钟（uartx_clk 频率）/（16×除数）
*/
#define UART_UARTx_DLH_DLH_Pos (0UL) /*!< Position of DLH field. */
#define UART_UARTx_DLH_DLL_Msk (0xFFUL << UART_UARTx_DLH_DLH_Pos)



/* Register: UARTx_IIR */
/* Description:中断寄存器
*/

/* Bit 0..3:  UART IID  */
/* Description：中断 ID 号：1 是最高优先级，5 是最低优先级。
*/
#define UART_UARTx_IIR_IID_Pos (0UL) /*!< Position of IID field. */
#define UART_UARTx_IIR_IID_Msk (0xFUL << UART_UARTx_IIR_IID_Pos)

#define UART_UARTx_IIR_IID_EMSI (0x0UL) //流控状态改变中断
#define UART_UARTx_IIR_IID_NoInter (0x1UL) //没有发生中断
#define UART_UARTx_IIR_IID_ETHEI (0x02UL) //发送保持寄存器 THR 空中断
#define UART_UARTx_IIR_IID_ERDAI (0x04UL) //接收数据有效中断
#define UART_UARTx_IIR_IID_ETSI (0x06UL) //错误中断，
#define UART_UARTx_IIR_IID_BUSY (0x07UL) //忙检测中断
#define UART_UARTx_IIR_IID_TO (0xCUL) //字符超时中断


/* Bit 6..7:  UART FS  */
/* Description：是否使能 FIFO
*/
#define UART_UARTx_IIR_FS_Pos (6UL) /*!< Position of IID field. */
#define UART_UARTx_IIR_FS_Msk (0x3UL << UART_UARTx_IIR_FS_Pos)
#define UART_UARTx_IIR_FS_Enable (3UL)
#define UART_UARTx_IIR_FS_Disable (0UL)



/* Register: UARTx_FCR */
/* Description:FIFO 控制寄存器
*/

/* Bit 0:  UART FIFO_Enable  */
/* Description：启用发送/接收 FIFO：任何时候此位改变
设置时，接收/发送 FIFO 复位。
*/
#define UART_UARTx_FCR_FIFO_Enable_Pos (0UL) /*!< Position of FIFO_Enable field. */
#define UART_UARTx_FCR_FIFO_Enable_Msk (0x1UL << UART_UARTx_FCR_FIFO_Enable_Pos)
#define UART_UARTx_FCR_FIFO_Enable_Enable (1UL)
#define UART_UARTx_FCR_FIFO_Enable_Disable (0UL)

/* Bit 1:  UART RCVR_FIFO_Reset  */
/* Description：清空接收 FIFO：该位自动清零
*/
#define UART_UARTx_FCR_RCVR_FIFO_Reset_Pos (1UL) /*!< Position of RCVR_FIFO_Reset field. */
#define UART_UARTx_FCR_RCVR_FIFO_Reset_Msk (0x1UL << UART_UARTx_FCR_RCVR_FIFO_Reset_Pos)
#define UART_UARTx_FCR_RCVR_FIFO_Reset_Clear (1UL)
#define UART_UARTx_FCR_RCVR_FIFO_Reset_Not (0UL)

/* Bit 2:  UART XMIT_FIFO_Reset  */
/* Description：清空接收 FIFO：该位自动清零
*/
#define UART_UARTx_FCR_XMIT_FIFO_Reset_Pos (2UL) /*!< Position of XMIT_FIFO_Reset field. */
#define UART_UARTx_FCR_XMIT_FIFO_Reset_Msk (0x1UL << UART_UARTx_FCR_XMIT_FIFO_Reset_Pos)
#define UART_UARTx_FCR_XMIT_FIFO_Reset_Clear (1UL)
#define UART_UARTx_FCR_XMIT_FIFO_Reset_Not (0UL)

/* Bit 4..5:  UART TX_Empty_Trigger  */
/* Description：TX_Empty_Trigger：设置发送 FIFO 的
THRE 中断阈值。此参数仅在 FIFO 使能
和可编程 THRE 中断模式下用来设置THRE 阈值中断
*/
#define UART_UARTx_FCR_TX_Empty_Trigger_Pos (4UL) /*!< Position of TX_Empty_Trigger field. */
#define UART_UARTx_FCR_TX_Empty_Trigger_Msk (0x3UL << UART_UARTx_FCR_TX_Empty_Trigger_Pos)
#define UART_UARTx_FCR_TX_Empty_Trigger_FIFO_Empty (0UL)
#define UART_UARTx_FCR_TX_Empty_Trigger_FIFO_2_BYTE (1UL)
#define UART_UARTx_FCR_TX_Empty_Trigger_FIFO_4_3 (2UL)
#define UART_UARTx_FCR_TX_Empty_Trigger_FIFO_2_1 (3UL)

/* Bit 6..7:  UART RCVR_Trigger  */
/* Description：RCVR_Trigger：设置接收 FIFO 中断的阈
值
*/
#define UART_UARTx_FCR_RCVR_Trigger_Pos (6UL) /*!< Position of RCVR_Trigger field. */
#define UART_UARTx_FCR_RCVR_Trigger_Msk (0x3UL << UART_UARTx_FCR_RCVR_Trigger_Pos)
#define UART_UARTx_FCR_RCVR_Trigger_FIFO_1_BYTE (0UL)
#define UART_UARTx_FCR_RCVR_Trigger_FIFO_4_1 (1UL)
#define UART_UARTx_FCR_RCVR_Trigger_FIFO_2_1 (2UL)
#define UART_UARTx_FCR_RCVR_Trigger_FIFO_LESS_2_BYTE (3UL)



/* Register: UARTx_TCR */
/* Description:传输控制寄存器
*/

/* Bit 0..1:  UART CLS  */
/* Description：设置传输一帧数据的总的数据位的个数：
只有当 UARTx 不忙（UARTx_USR.BUSY=0）
时，对该位写操作有效.
*/
#define UART_UARTx_TCR_CLS_Pos (0UL) /*!< Position of CLS field. */
#define UART_UARTx_TCR_CLS_Msk (0x3UL << UART_UARTx_TCR_CLS_Pos)
#define UART_UARTx_TCR_CLS_5bits (0UL)
#define UART_UARTx_TCR_CLS_6bits (1UL)
#define UART_UARTx_TCR_CLS_7bits (2UL)
#define UART_UARTx_TCR_CLS_8bits (3UL)

/* Bit 2:  UART STOP  */
/* Description：设置 STOP 位的个数：.
*/
#define UART_UARTx_TCR_STOP_Pos (2UL) /*!< Position of STOP field. */
#define UART_UARTx_TCR_STOP_Msk (0x1UL << UART_UARTx_TCR_STOP_Pos)
#define UART_UARTx_TCR_STOP_1bits (0UL)
#define UART_UARTx_TCR_STOP_1_5bits (1UL) //1：当数据位设置为 5 位时，将产生 1.5bits 的停止位；当数据位设置为其他位时，将产生2bits 停止位。
#define UART_UARTx_TCR_STOP_2bits (1UL) 

/* Bit 3:  UART PEN  */
/* Description：奇偶校验使能位：只有当 UARTx 不忙（UARTx_USR.BUSY=0）
时，对该位写操作有效。
*/
#define UART_UARTx_TCR_PEN_Pos (3UL) /*!< Position of PEN field. */
#define UART_UARTx_TCR_PEN_Msk (0x1UL << UART_UARTx_TCR_PEN_Pos)
#define UART_UARTx_TCR_PEN_Excluded (0UL)
#define UART_UARTx_TCR_PENS_Included (1UL)


/* Bit 4:  UART EPS  */
/* Description：奇偶校验选择位：
只有当 UARTx 不忙（UARTx_USR.BUSY=0）
时，对该位写操作有效。
*/
#define UART_UARTx_TCR_EPS_Pos (4UL) /*!< Position of EPS field. */
#define UART_UARTx_TCR_EPS_Msk (0x1UL << UART_UARTx_TCR_EPS_Pos)
#define UART_UARTx_TCR_EPS_Odd (0UL)
#define UART_UARTx_TCR_EPS_Even (1UL)

/* Bit 6:  UART Break  */
/* Description：停止控制位：用于流控模式：往该位写“1”，发送 1 个停止位信号使 uartx_tx
线为低电平，该低电平一直保持到往该位写
“0”。往该位写“0”，uartx_tx 才恢复为高电平
*/
#define UART_UARTx_TCR_Break_Pos (6UL) /*!< Position of Break field. */
#define UART_UARTx_TCR_Break_Msk (0x1UL << UART_UARTx_TCR_Break_Pos)
#define UART_UARTx_TCR_Break_Stop (1UL)
#define UART_UARTx_TCR_Break_Recover (0UL)


/* Bit 7:  UART DLAB  */
/* Description：UARTx_DLL, UARTx_DLH 操作使能位：
只有当 UARTx 不忙（UARTx_USR.BUSY=0）
时，对该位写操作有效。
0：UARTx_DLL, UARTx_DLH 操作不使能
1：操作使能。可以去操作 UARTx_DLL，
UARTx_DLH，操作完成后必须设置 DLAB=0
*/
#define UART_UARTx_TCR_DLAB_Pos (7UL) /*!< Position of DLAB field. */
#define UART_UARTx_TCR_DLAB_Msk (0x1UL << UART_UARTx_TCR_DLAB_Pos)
#define UART_UARTx_TCR_DLAB_DLLH_Enable (1UL)
#define UART_UARTx_TCR_DLAB_DLLH_Disable (0UL)


/* Register: UARTx_MCR */
/* Description:流控控制寄存器
*/

/* Bit 1:  UART RTS  */
/* Description：在非流控模式下，用户可以通过配置 RTS， 来控制 uartx_rts 的输出，即为一个 GPO 端 口。
*/
#define UART_UARTx_MCR_RTS_Pos (1UL) /*!< Position of RTS field. */
#define UART_UARTx_MCR_RTS_Msk (0x1UL << UART_UARTx_MCR_RTS_Pos)
#define UART_UARTx_MCR_RTS_Out_High (1UL)
#define UART_UARTx_MCR_RTS_Out_Low (0UL)

/* Bit 5:  UART AFCE  */
/* Description：是否使能 AFC 模式：
当 FIFO 使能时此参数用于 AFC 控制（必须
保证先使能 FIFO，再使能 AFC）
*/
#define UART_UARTx_MCR_AFCE_Pos (5UL) /*!< Position of AFCE field. */
#define UART_UARTx_MCR_AFCE_Msk (0x1UL << UART_UARTx_MCR_AFCE_Pos)
#define UART_UARTx_MCR_AFCE_Enable (1UL)
#define UART_UARTx_MCR_AFCE_Disable (0UL)



/* Register: UARTx_TSR */
/* Description:传输状态寄存器
*/

/* Bit 0:  UART DR  */
/* Description：接收数据准备好：可以读取数据
*/
#define UART_UARTx_TSR_DR_Pos (0UL) /*!< Position of DR field. */
#define UART_UARTx_TSR_DR_Msk (0x1UL << UART_UARTx_TSR_DR_Pos)
#define UART_UARTx_TSR_DR_Valid (1UL)
#define UART_UARTx_TSR_DR_InValid (0UL)

/* Bit 1:  UART OE  */
/* Description：接收 FIFO 溢出错误
*/
#define UART_UARTx_TSR_OE_Pos (1UL) /*!< Position of OE field. */
#define UART_UARTx_TSR_OE_Msk (0x1UL << UART_UARTx_TSR_OE_Pos)
#define UART_UARTx_TSR_OE_Valid (1UL)
#define UART_UARTx_TSR_OE_InValid (0UL)


/* Bit 2:  UART PE  */
/* Description：奇偶校验错误
*/
#define UART_UARTx_TSR_PE_Pos (2UL) /*!< Position of PE field. */
#define UART_UARTx_TSR_PE_Msk (0x1UL << UART_UARTx_TSR_PE_Pos)
#define UART_UARTx_TSR_PE_Valid (1UL)
#define UART_UARTx_TSR_PE_InValid (0UL)

/* Bit 3:  UART FE  */
/* Description：帧格式错误
*/
#define UART_UARTx_TSR_FE_Pos (3UL) /*!< Position of FE field. */
#define UART_UARTx_TSR_FE_Msk (0x1UL << UART_UARTx_TSR_FE_Pos)
#define UART_UARTx_TSR_FE_Valid (1UL)
#define UART_UARTx_TSR_FE_InValid (0UL)

/* Bit 4:  UART BI  */
/* Description：停止错误：
*/
#define UART_UARTx_TSR_BI_Pos (3UL) /*!< Position of BI field. */
#define UART_UARTx_TSR_BI_Msk (0x1UL << UART_UARTx_TSR_BI_Pos)
#define UART_UARTx_TSR_BI_Valid (1UL)
#define UART_UARTx_TSR_BI_InValid (0UL)

/* Bit 5:  UART THRE  */
/* Description：发送保持寄存器 UARTx_THR 空中断
FIFO 不使能时，当 UARTx_THR 发送保持寄
存器空，发生发送保持寄存器空中断。
FIFO 使能时，在非可编程 THRE 模式下，当
UARTx_THR 空时（此时 FIFO 一定空），发
生发送保持寄存器空中断。
FIFO 使能时，在可编程 THRE 模式下，此位
不再表示发送保持寄存器 THR 空中断是否发
生，表示的是 FIFO 是否为满，当 FIFO 为满
时，THRE=1.
0：正常
1：发生发送保持寄存器空中断（FIFO 不使
能，FIFO 使能并且在非可编程 THRE 模式下）
1：发送 FIFO 满（FIFO 使能并且在可编程
THRE 模式下）
*/
#define UART_UARTx_TSR_THRE_Pos (5UL) /*!< Position of THRE field. */
#define UART_UARTx_TSR_THRE_Msk (0x1UL << UART_UARTx_TSR_THRE_Pos)
#define UART_UARTx_TSR_THRE_Valid (1UL)
#define UART_UARTx_TSR_THRE_InValid (0UL)


/* Bit 7:  UART RX FIFO Error  */
/* Description：接收 FIFO 错误
FIFO 使能时该位有效。如果接收过程中 FIFO
出现了奇偶校验错误，或者帧格式错误，或者
停止错误就会产生接收 FIFO 错误中断。读
UARTx_TSR 可以清除接收 FIFO 错误中断。
0：正常
1：发生接收 FIFO 错误
*/
#define UART_UARTx_TSR_RX_FIFO_Error_Pos (7UL) /*!< Position of THRE field. */
#define UART_UARTx_TSR_RX_FIFO_Error_Msk (0x1UL << UART_UARTx_TSR_RX_FIFO_Error_Pos)
#define UART_UARTx_TSR_RX_FIFO_Error_Valid (1UL)
#define UART_UARTx_TSR_RX_FIFO_Error_InValid (0UL)



/* Register: UARTx_MSR */
/* Description:流控状态寄存器
*/

/* Bit 0:  UART DCTS  */
/* Description：标志从前一次读取 UARTx_MSR，到当前时
刻为止，uartx_cts 管脚信号电平发生变化情
况。用于流控模式。该位寄存器在进行读操作
之后自动清 0。
*/
#define UART_UARTx_MSR_DCTS_Pos (0UL) /*!< Position of DCTS field. */
#define UART_UARTx_MSR_DCTS_Msk (0x1UL << UART_UARTx_MSR_DCTS_Pos)
#define UART_UARTx_MSR_DCTS_Change (1UL)
#define UART_UARTx_MSR_DCTS_NoChange (0UL)

/* Bit 4:  UART CTS  */
/* Description：管脚信号 uartx_cts 的值的
1：uartx_cts 为低电平
0：uartx_cts 为高电平
uartx_cts 是流控状态输入管脚信号，用于
AFC 模式。当启用 AFC 时，且 FIFO 使能，
uartx_cts 高电平（无效）将会使发送无效。
该位的复位值与外部管脚信号有关， UART0
的该位寄存器由于 uart0_cts 管脚是复用的，
在处于非 UART 模式下，该位寄存器复位值
为 0，在 UART 模式下，复位值是 uart0_cts
的反。
*/
#define UART_UARTx_MSR_CTS_Pos (4UL) /*!< Position of DCTS field. */
#define UART_UARTx_MSR_CTS_Msk (0x1UL << UART_UARTx_MSR_CTS_Pos)
#define UART_UARTx_MSR_CTS_Out_Low (1UL)
#define UART_UARTx_MSR_CTS_Out_High (0UL)

/* Register: UARTx_USR */
/* Description:状态寄存器
*/

/* Bit 0:  UART BUSY  */
/* Description：UART 状态：用于指示 UARTx 的工作状态
0：UARTx 处于空闲状态或不工作状态
1：UARTx 正在传输数据
*/
#define UART_UARTx_USR_BUSY_Pos (0UL) /*!< Position of BUSY field. */
#define UART_UARTx_USR_BUSY_Msk (0x1UL << UART_UARTx_USR_BUSY_Pos)
#define UART_UARTx_USR_BUSY_Busy (1UL)
#define UART_UARTx_USR_BUSY_Idle (0UL)



///////////////////////////////////////////////////////////////////////XINCHIP UART END //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////







/* Register: UART_INTENSET */
/* Description: Enable interrupt */

/* Bit 17 : Write '1' to enable interrupt for event RXTO */
#define UART_INTENSET_RXTO_Pos (17UL) /*!< Position of RXTO field. */
#define UART_INTENSET_RXTO_Msk (0x1UL << UART_INTENSET_RXTO_Pos) /*!< Bit mask of RXTO field. */
#define UART_INTENSET_RXTO_Disabled (0UL) /*!< Read: Disabled */
#define UART_INTENSET_RXTO_Enabled (1UL) /*!< Read: Enabled */
#define UART_INTENSET_RXTO_Set (1UL) /*!< Enable */

/* Bit 9 : Write '1' to enable interrupt for event ERROR */
#define UART_INTENSET_ERROR_Pos (9UL) /*!< Position of ERROR field. */
#define UART_INTENSET_ERROR_Msk (0x1UL << UART_INTENSET_ERROR_Pos) /*!< Bit mask of ERROR field. */
#define UART_INTENSET_ERROR_Disabled (0UL) /*!< Read: Disabled */
#define UART_INTENSET_ERROR_Enabled (1UL) /*!< Read: Enabled */
#define UART_INTENSET_ERROR_Set (1UL) /*!< Enable */

/* Bit 7 : Write '1' to enable interrupt for event TXDRDY */
#define UART_INTENSET_TXDRDY_Pos (7UL) /*!< Position of TXDRDY field. */
#define UART_INTENSET_TXDRDY_Msk (0x1UL << UART_INTENSET_TXDRDY_Pos) /*!< Bit mask of TXDRDY field. */
#define UART_INTENSET_TXDRDY_Disabled (0UL) /*!< Read: Disabled */
#define UART_INTENSET_TXDRDY_Enabled (1UL) /*!< Read: Enabled */
#define UART_INTENSET_TXDRDY_Set (1UL) /*!< Enable */

/* Bit 2 : Write '1' to enable interrupt for event RXDRDY */
#define UART_INTENSET_RXDRDY_Pos (2UL) /*!< Position of RXDRDY field. */
#define UART_INTENSET_RXDRDY_Msk (0x1UL << UART_INTENSET_RXDRDY_Pos) /*!< Bit mask of RXDRDY field. */
#define UART_INTENSET_RXDRDY_Disabled (0UL) /*!< Read: Disabled */
#define UART_INTENSET_RXDRDY_Enabled (1UL) /*!< Read: Enabled */
#define UART_INTENSET_RXDRDY_Set (1UL) /*!< Enable */

/* Bit 1 : Write '1' to enable interrupt for event NCTS */
#define UART_INTENSET_NCTS_Pos (1UL) /*!< Position of NCTS field. */
#define UART_INTENSET_NCTS_Msk (0x1UL << UART_INTENSET_NCTS_Pos) /*!< Bit mask of NCTS field. */
#define UART_INTENSET_NCTS_Disabled (0UL) /*!< Read: Disabled */
#define UART_INTENSET_NCTS_Enabled (1UL) /*!< Read: Enabled */
#define UART_INTENSET_NCTS_Set (1UL) /*!< Enable */

/* Bit 0 : Write '1' to enable interrupt for event CTS */
#define UART_INTENSET_CTS_Pos (0UL) /*!< Position of CTS field. */
#define UART_INTENSET_CTS_Msk (0x1UL << UART_INTENSET_CTS_Pos) /*!< Bit mask of CTS field. */
#define UART_INTENSET_CTS_Disabled (0UL) /*!< Read: Disabled */
#define UART_INTENSET_CTS_Enabled (1UL) /*!< Read: Enabled */
#define UART_INTENSET_CTS_Set (1UL) /*!< Enable */

/* Register: UART_INTENCLR */
/* Description: Disable interrupt */

/* Bit 17 : Write '1' to disable interrupt for event RXTO */
#define UART_INTENCLR_RXTO_Pos (17UL) /*!< Position of RXTO field. */
#define UART_INTENCLR_RXTO_Msk (0x1UL << UART_INTENCLR_RXTO_Pos) /*!< Bit mask of RXTO field. */
#define UART_INTENCLR_RXTO_Disabled (0UL) /*!< Read: Disabled */
#define UART_INTENCLR_RXTO_Enabled (1UL) /*!< Read: Enabled */
#define UART_INTENCLR_RXTO_Clear (1UL) /*!< Disable */

/* Bit 9 : Write '1' to disable interrupt for event ERROR */
#define UART_INTENCLR_ERROR_Pos (9UL) /*!< Position of ERROR field. */
#define UART_INTENCLR_ERROR_Msk (0x1UL << UART_INTENCLR_ERROR_Pos) /*!< Bit mask of ERROR field. */
#define UART_INTENCLR_ERROR_Disabled (0UL) /*!< Read: Disabled */
#define UART_INTENCLR_ERROR_Enabled (1UL) /*!< Read: Enabled */
#define UART_INTENCLR_ERROR_Clear (1UL) /*!< Disable */


/* Register: UART_ERRORSRC */
/* Description: Error source */



/* Register: UART_ENABLE */
/* Description: Enable UART */

/* Bits 3..0 : Enable or disable UART */
#define UART_ENABLE_ENABLE_Pos (0UL) /*!< Position of ENABLE field. */
#define UART_ENABLE_ENABLE_Msk (0xFUL << UART_ENABLE_ENABLE_Pos) /*!< Bit mask of ENABLE field. */
#define UART_ENABLE_ENABLE_Disabled (0UL) /*!< Disable UART */
#define UART_ENABLE_ENABLE_Enabled (4UL) /*!< Enable UART */

/* Register: UART_PSEL_RTS */
/* Description: Pin select for RTS */

/* Bit 31 : Connection */
#define UART_PSEL_RTS_CONNECT_Pos (31UL) /*!< Position of CONNECT field. */
#define UART_PSEL_RTS_CONNECT_Msk (0x1UL << UART_PSEL_RTS_CONNECT_Pos) /*!< Bit mask of CONNECT field. */
#define UART_PSEL_RTS_CONNECT_Connected (0UL) /*!< Connect */
#define UART_PSEL_RTS_CONNECT_Disconnected (1UL) /*!< Disconnect */

/* Bit 5 : Port number */
#define UART_PSEL_RTS_PORT_Pos (5UL) /*!< Position of PORT field. */
#define UART_PSEL_RTS_PORT_Msk (0x1UL << UART_PSEL_RTS_PORT_Pos) /*!< Bit mask of PORT field. */

/* Bits 4..0 : Pin number */
#define UART_PSEL_RTS_PIN_Pos (0UL) /*!< Position of PIN field. */
#define UART_PSEL_RTS_PIN_Msk (0x1FUL << UART_PSEL_RTS_PIN_Pos) /*!< Bit mask of PIN field. */

/* Register: UART_PSEL_TXD */
/* Description: Pin select for TXD */

/* Bit 31 : Connection */
#define UART_PSEL_TXD_CONNECT_Pos (31UL) /*!< Position of CONNECT field. */
#define UART_PSEL_TXD_CONNECT_Msk (0x1UL << UART_PSEL_TXD_CONNECT_Pos) /*!< Bit mask of CONNECT field. */
#define UART_PSEL_TXD_CONNECT_Connected (0UL) /*!< Connect */
#define UART_PSEL_TXD_CONNECT_Disconnected (1UL) /*!< Disconnect */

/* Bit 5 : Port number */
#define UART_PSEL_TXD_PORT_Pos (5UL) /*!< Position of PORT field. */
#define UART_PSEL_TXD_PORT_Msk (0x1UL << UART_PSEL_TXD_PORT_Pos) /*!< Bit mask of PORT field. */

/* Bits 4..0 : Pin number */
#define UART_PSEL_TXD_PIN_Pos (0UL) /*!< Position of PIN field. */
#define UART_PSEL_TXD_PIN_Msk (0x1FUL << UART_PSEL_TXD_PIN_Pos) /*!< Bit mask of PIN field. */

/* Register: UART_PSEL_CTS */
/* Description: Pin select for CTS */



/* Register: UART_BAUDRATE */
/* Description: Baud rate. Accuracy depends on the HFCLK source selected. */

/* Bits 31..0 : Baud rate */
#define UART_BAUDRATE_BAUDRATE_Pos (0UL) /*!< Position of BAUDRATE field. */
#define UART_BAUDRATE_BAUDRATE_Msk (0xFFFFFFFFUL << UART_BAUDRATE_BAUDRATE_Pos) /*!< Bit mask of BAUDRATE field. */
#define UART_BAUDRATE_BAUDRATE_Baud2400 (0x0120271CUL) /*!< 2400 baud  */
#define UART_BAUDRATE_BAUDRATE_Baud4800 (0x0240271CUL) /*!< 4800 baud */
#define UART_BAUDRATE_BAUDRATE_Baud9600 (0x0480271CUL) /*!< 9600 baud  */
#define UART_BAUDRATE_BAUDRATE_Baud12800 (0x04802719UL) /*!< 12800 baud  */
#define UART_BAUDRATE_BAUDRATE_Baud14400 (0x04802718UL) /*!< 14400 baud  */
#define UART_BAUDRATE_BAUDRATE_Baud19200 (0x04802716UL) /*!< 19200 baud  */

#define UART_BAUDRATE_BAUDRATE_Baud23040 (0x04802715UL) /*!< 23040 baud  */
#define UART_BAUDRATE_BAUDRATE_Baud38400 (0x04802713UL) /*!< 38400 baud  */
#define UART_BAUDRATE_BAUDRATE_Baud57600 (0x04802712UL) /*!< 57600 baud  */
#define UART_BAUDRATE_BAUDRATE_Baud115200 (0x04802711UL) /*!< 115200 baud */

#define UART_BAUDRATE_BAUDRATE_Baud128000 (0x030007D3UL) /*!< 128000 baud */
#define UART_BAUDRATE_BAUDRATE_Baud230400 (0x0120271CUL) /*!< 230400 baud */
#define UART_BAUDRATE_BAUDRATE_Baud256000 (0x060007D3UL) /*!< 256000 baud */
#define UART_BAUDRATE_BAUDRATE_Baud460800 (0x12002711UL) /*!< 256000 baud */
#define UART_BAUDRATE_BAUDRATE_Baud921600 (0x24002711UL) /*!< 256000 baud */

#define UART_BAUDRATE_BAUDRATE_Baud1M (0x00100011UL) /*!< 1Mega baud */

/* Register: UART_CONFIG */
/* Description: Configuration of parity and hardware flow control */

/* Bit 4 : Stop bits */
#define UART_CONFIG_STOP_Pos (4UL) /*!< Position of STOP field. */
#define UART_CONFIG_STOP_Msk (0x1UL << UART_CONFIG_STOP_Pos) /*!< Bit mask of STOP field. */
#define UART_CONFIG_STOP_One (0UL) /*!< One stop bit */
#define UART_CONFIG_STOP_Two (1UL) /*!< Two stop bits */

/* Bits 3..1 : Parity */
#define UART_CONFIG_PARITY_Pos (1UL) /*!< Position of PARITY field. */
#define UART_CONFIG_PARITY_Msk (0x7UL << UART_CONFIG_PARITY_Pos) /*!< Bit mask of PARITY field. */
#define UART_CONFIG_PARITY_Excluded (0x0UL) /*!< Exclude parity bit */
#define UART_CONFIG_PARITY_Included (0x7UL) /*!< Include parity bit */

/* Bit 0 : Hardware flow control */
#define UART_CONFIG_HWFC_Pos (0UL) /*!< Position of HWFC field. */
#define UART_CONFIG_HWFC_Msk (0x1UL << UART_CONFIG_HWFC_Pos) /*!< Bit mask of HWFC field. */
#define UART_CONFIG_HWFC_Disabled (0UL) /*!< Disabled */
#define UART_CONFIG_HWFC_Enabled (1UL) /*!< Enabled */






/* Bits 31..0 : Baud rate */
#define UARTE_BAUDRATE_BAUDRATE_Pos (0UL) /*!< Position of BAUDRATE field. */
//#define UARTE_BAUDRATE_BAUDRATE_Msk (0xFFFFFFFFUL << UARTE_BAUDRATE_BAUDRATE_Pos) /*!< Bit mask of BAUDRATE field. */
//#define UARTE_BAUDRATE_BAUDRATE_Baud1200 (0x0004F000UL) /*!< 1200 baud (actual rate: 1205) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud2400 (0x0009D000UL) /*!< 2400 baud (actual rate: 2396) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud4800 (0x0013B000UL) /*!< 4800 baud (actual rate: 4808) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud9600 (0x00275000UL) /*!< 9600 baud (actual rate: 9598) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud14400 (0x003AF000UL) /*!< 14400 baud (actual rate: 14401) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud19200 (0x004EA000UL) /*!< 19200 baud (actual rate: 19208) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud28800 (0x0075C000UL) /*!< 28800 baud (actual rate: 28777) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud31250 (0x00800000UL) /*!< 31250 baud */
//#define UARTE_BAUDRATE_BAUDRATE_Baud38400 (0x009D0000UL) /*!< 38400 baud (actual rate: 38369) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud56000 (0x00E50000UL) /*!< 56000 baud (actual rate: 55944) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud57600 (0x00EB0000UL) /*!< 57600 baud (actual rate: 57554) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud76800 (0x013A9000UL) /*!< 76800 baud (actual rate: 76923) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud115200 (0x01D60000UL) /*!< 115200 baud (actual rate: 115108) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud230400 (0x03B00000UL) /*!< 230400 baud (actual rate: 231884) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud250000 (0x04000000UL) /*!< 250000 baud */
//#define UARTE_BAUDRATE_BAUDRATE_Baud460800 (0x07400000UL) /*!< 460800 baud (actual rate: 457143) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud921600 (0x0F000000UL) /*!< 921600 baud (actual rate: 941176) */
//#define UARTE_BAUDRATE_BAUDRATE_Baud1M (0x10000000UL) /*!< 1Mega baud */

/* Register: UARTE_RXD_PTR */
/* Description: Data pointer */












/*lint --flb "Leave library region" */
#endif
