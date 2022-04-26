/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef __XINC62_xxxx_CPR_BITS_H
#define __XINC62_xxxx_CPR_BITS_H

/*lint ++flb "Enter library region" */


/* Peripheral: CPR PD */
/* Description: CPR PD Clock control */

/* CPR PD BASE ADDR:0x40000000 */

/* Register: CPR_SLP_SRC_MASK */
/* Offse: 0x00 */
/* Description : 睡眠源屏蔽寄存器
*/

/* Bit 0..7: SLP_SRC_MASK */
/* Description: 
睡眠源屏蔽，对应 bit 为 1
屏蔽相应的睡眠来源
*/
#define CPR_SLP_SRC_MASK_SLP_SRC_MASK_Pos (0UL) 
#define CPR_SLP_SRC_MASK_SLP_SRC_MASK_Msk (0xFFUL << CPR_SLP_SRC_MASK_SLP_SRC_MASK_Pos) 

/* Bit 8: SLP_MASK */
/* Description: 
全局屏蔽信号，屏蔽全部睡
眠源
*/
#define CPR_SLP_SRC_MASK_SLP_MASK_Pos (8UL) 
#define CPR_SLP_SRC_MASK_SLP_MASK_Msk (0x1UL << CPR_SLP_SRC_MASK_SLP_MASK_Pos) 


/* Register: CPR_SLP_CNT_LIMIT */
/* Offse: 0x10 */
/* Description : 配置睡眠计数器计数到
SLP_CONFIG_NUM 之前的多少个周期将中断屏蔽信
号 sys_intr_mask 拉高
*/

/* Bit 0..11: SLP_CONFIG_NUM */
/* Description: 
睡眠条件满足的情况下，
26M 计数器需要计数到多
少，才将睡眠满足信号传递
给睡眠状态机：最小值为
0x6，最大值为 0xFFFF
*/
#define CPR_SLP_CNT_LIMIT_SLP_CONFIG_NUM_Pos (0UL) 
#define CPR_SLP_CNT_LIMIT_SLP_CONFIG_NUM_Msk (0xFFFUL << CPR_SLP_CNT_LIMIT_SLP_CONFIG_NUM_Pos) 

/* Bit 12..14: SYS_INTR_MASK_CYCLE */
/* Description: 
配 置 睡 眠 计 数 器 计 数 到
SLP_CONFIG_NUM 之 前
的多少个周期将中断屏蔽信
号 sys_intr_mask 拉高。
*/
#define CPR_SLP_CNT_LIMIT_SYS_INTR_MASK_CYCLE_Pos (12UL) 
#define CPR_SLP_CNT_LIMIT_SYS_INTR_MASK_CYCLE_Msk (0x7UL << CPR_SLP_CNT_LIMIT_SYS_INTR_MASK_CYCLE_Pos) 


/* Register: CPR_SLP_ST */
/* Offse: 0x14 */
/* Description : 睡眠启动寄存器
*/

/* Bit 0: SLP_ST */
/* Description: 
软件睡眠进入控制位：
1：启动睡眠控制器进入睡
眠；经睡眠过程之后，此位
自清零； 0：无影响
*/
#define CPR_SLP_ST_SLP_ST_Pos (0UL) 
#define CPR_SLP_ST_SLP_ST_Msk (0x1UL << CPR_SLP_ST_SLP_ST_Pos) 
#define CPR_SLP_ST_SLP_ST_Enable (1UL) 

/* Bit 1: SLP_ST_FINAL */
/* Description: 
只读，内部软件睡眠信号的
状态，该位在 SLP_ST 置为
1 时拉高，在进入睡眠之后
拉低。
可以用来监测睡眠的进入。
*/
#define CPR_SLP_ST_SLP_ST_FINAL_Pos (1UL) 
#define CPR_SLP_ST_SLP_ST_FINAL_Msk (0x1UL << CPR_SLP_ST_SLP_ST_FINAL_Pos) 
#define CPR_SLP_ST_SLP_ST_FINAL_NotGenerated (0UL) 
#define CPR_SLP_ST_SLP_ST_FINAL_Generated (1UL) 


/* Register: CPR_SLP_FSM_STATE */
/* Offse: 0x18 */
/* Description : 睡眠状态机状态寄存器
*/

/* Bit 0..2: SYS_CS */
/* Description: 
0x0：状态机处于工作状态；
0x1：状态机晶振起振阶段；
0x2：状态机主时钟关闭阶
段；
0x4：状态机复位系统阶段；
0x5：状态机主时钟打开阶
段；
0x6：状态机晶振关闭阶段；
0x7：状态机断电状态；
*/
#define CPR_SLP_FSM_STATE_SYS_CS_Pos (0UL) 
#define CPR_SLP_FSM_STATE_SYS_CS_Msk (0x7UL << CPR_SLP_FSM_STATE_SYS_CS_Pos) 
#define CPR_SLP_FSM_STATE_SYS_CS_Working (0UL) 
#define CPR_SLP_FSM_STATE_SYS_CS_XtalStarting (1UL) 
#define CPR_SLP_FSM_STATE_SYS_CS_MclkCloseing (2UL) 
#define CPR_SLP_FSM_STATE_SYS_CS_Reseting (4UL) 
#define CPR_SLP_FSM_STATE_SYS_CS_MclkOpening (5UL) 
#define CPR_SLP_FSM_STATE_SYS_CS_XtalCloseing (6UL) 
#define CPR_SLP_FSM_STATE_SYS_CS_PowerOff (7UL)


/* Register: CPR_M0_FCLK_CTL */
/* Offse: 0x20 */
/* Description : 
备注：1. BIT 16 写屏蔽 BIT[3:0]，BIT 20 写屏蔽 BIT[7:4]，BIT 24 写屏蔽 BIT[11:8]，BIT 
28 写屏蔽 BIT[15:12]
2. M0_FCLK 在 M0_FCLK_DIRECT_SW 为 0 时，新配置的分频系数在系统经过睡眠流程
后才能发生作用
*/


/* Bit 0..3: M0_FCLK_DIV_P */
/* Description: M0_FCLK 当前配置的时钟
分频系数
*/
#define CPR_M0_FCLK_CTL_M0_FCLK_DIV_P_Pos (0UL) 
#define CPR_M0_FCLK_CTL_M0_FCLK_DIV_P_Msk (0xFUL << CPR_M0_FCLK_CTL_M0_FCLK_DIV_P_Pos) 

/* Bit 4: M0_FCLK_DIRECT_SW */
/* Description: M0_FCLK 时钟分频立即作
用
*/
#define CPR_M0_FCLK_CTL_M0_FCLK_DIRECT_SW_Pos (4UL) 
#define CPR_M0_FCLK_CTL_M0_FCLK_DIRECT_SW_Msk (0x1UL << CPR_M0_FCLK_CTL_M0_FCLK_DIRECT_SW_Pos) 

/* Bit 8..11: M0_FCLK_DIV */
/* Description: M0_FCLK 当前起作用的时
钟分频系统
*/
#define CPR_M0_FCLK_CTL_M0_FCLK_DIV_Pos (8UL) 
#define CPR_M0_FCLK_CTL_M0_FCLK_DIV_Msk (0xFUL << CPR_M0_FCLK_CTL_M0_FCLK_DIVPos) 


/* Register: CPR_CTL_PCLK_GRCTL */
/* Offse: 0x24 */
/* Description : 
备注：1. BIT 16 写屏蔽 BIT[3:0]，BIT 20 写屏蔽 BIT[7:4]，BIT 24 写屏蔽 BIT[11:8]，BIT 
28 写屏蔽 BIT[15:12]
*/


/* Bit 0..3: CTL_PCLK_GR */
/* Description: CTL_PCLK 频 率 为
BUS_CLK 时 钟 频 率 的 ：
CTL_PCLK_GR/8
*/
#define CPR_CTL_PCLK_GRCTL_CTL_PCLK_GR_Pos (0UL) 
#define CPR_CTL_PCLK_GRCTL_CTL_PCLK_GR_Msk (0xFUL << CPR_CTL_PCLK_GRCTL_CTL_PCLK_GR_Pos) 

/* Bit 4: CTL_PCLK_GR_UPD */
/* Description: CTL_PCLK GR 参数更新寄
存器，1 有效，自清 0
*/
#define CPR_CTL_PCLK_GRCTL_CTL_PCLK_GR_UPD_Pos (4UL) 
#define CPR_CTL_PCLK_GRCTL_CTL_PCLK_GR_UPD_Msk (0x1UL << CPR_CTL_PCLK_GRCTL_CTL_PCLK_GR_UPD_Pos) 



/* Register: CPR_SIM_CLK_GRCTL */
/* Offse: 0x28 */
/* Description : SIM_CLK 时钟 GR 控制寄存器
备注：1. BIT 16 写屏蔽 BIT[3:0]，BIT 20 写屏蔽 BIT[7:4]，BIT 24 写屏蔽 BIT[11:8]，BIT 
28 写屏蔽 BIT[15:12]
*/


/* Bit 0..3: SIM_CLK_GR */
/* Description: sim_clk 相对于 32MHz 时钟
的 一 级 分 频 比
为:SIM_CLK_GR/8
*/
#define CPR_CTL_PCLK_GRCTL_SIM_CLK_GR_Pos (0UL) 
#define CPR_CTL_PCLK_GRCTL_SIM_CLK_GR_Msk (0xFUL << CPR_CTL_PCLK_GRCTL_SIM_CLK_GR_Pos) 

/* Bit 4: SIM_CLK_GR_UPD */
/* Description:sim_clk GR 参数更新寄存
器，1 有效，自清 0
*/
#define CPR_CTL_PCLK_GRCTL_SIM_CLK_GR_UPD_Pos (4UL) 
#define CPR_CTL_PCLK_GRCTL_SIM_CLK_GR_UPD_Msk (0x1UL << CPR_CTL_PCLK_GRCTL_SIM_CLK_GR_UPD_Pos) 


/* Register: CPR_SIM_CLK_CTL */
/* Offse: 0x2C */
/* Description : SIM_CLK 时钟控制寄存器
*/

/* Bit 0..15: SIM_CLK_DIV */
/* Description: sim_clk 相对于 32MHz 时钟
的二级分频系数 DIV
*/
#define CPR_SIM_CLK_CTL_SIM_CLK_DIV_Pos (0UL) 
#define CPR_SIM_CLK_CTL_SIM_CLK_DIV_Msk (0xFFFFUL << CPR_SIM_CLK_CTL_SIM_CLK_DIV_Pos) 

/* Bit 0..15: SIM_CLK_MUL */
/* Description: sim_clk 相对于 32MHz 时钟
的二级分频系数 MUL
*/
#define CPR_SIM_CLK_CTL_SIM_CLK_MUL_Pos (16UL) 
#define CPR_SIM_CLK_CTL_SIM_CLK_MUL_Msk (0xFFFFUL <<CPR_SIM_CLK_CTL_SIM_CLK_MUL_Pos16UL) 



/***********************************************************/
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


/* Register: CPR_BT_CLK_CTL */
/* Offset: 0x40 */
/* Description :BT_CLK 时钟控制寄存器
    BIT 16 写屏蔽 BIT[3:0]，BIT 20 写屏蔽 BIT[7:4]，BIT 24 写屏蔽 BIT[11:8]，BIT 28
写屏蔽 BIT[15:12]*/


/* Bit 0..3: BT_CLK_DIV */
/* Description: BT_CLK 时钟分频系数
*/
#define CPR_BT_CLK_CTL_BT_CLK_DIV_Pos (0UL)
#define CPR_BT_CLK_CTL_BT_CLK_DIV_Msk (0xFUL << CPR_BT_CLK_CTL_BT_CLK_DIV_Pos) 

/* Bit 4: BT_CLK_EN */
/* Description: BT_CLK 时钟使能
*/
#define CPR_BT_CLK_CTL_BT_CLK_EN_Pos (4UL)
#define CPR_BT_CLK_CTL_BT_CLK_EN_Msk (0x1UL << CPR_BT_CLK_CTL_BT_CLK_EN_Pos) 
#define CPR_BT_CLK_CTL_BT_CLK_EN_Disable (0UL) 
#define CPR_BT_CLK_CTL_BT_CLK_EN_Enable (1UL)

/* Register: CPR_BT_MODEM_CLK_CTL */
/* Offset: 0x48 */
/* Description :BT_MODEM_CLK 时 钟 控 制 寄 存 器
    BIT 16 写屏蔽 BIT[3:0]，BIT 20 写屏蔽 BIT[7:4]，BIT 24 写屏蔽 BIT[11:8]，BIT 28
写屏蔽 BIT[15:12]*/


/* Bit 0..3: BT_MODEM_CLK_DIV */
/* Description: BT_MODEM_CLK 时钟分频系数
*/
#define CPR_BT_MODEM_CLK_CTL_BT_MODEM_CLK_DIV_Pos (0UL)
#define CPR_BT_MODEM_CLK_CTL_BT_MODEM_CLK_DIV_Msk (0xFUL << CPR_BT_MODEM_CLK_CTL_BT_MODEM_CLK_DIV_Pos) 

/* Bit 4: BT_MODEM_CLK_EN */
/* Description: BT_MODEM_CLK 时钟使能
*/
#define CPR_BT_MODEM_CLK_CTL_BT_MODEM_CLK_EN_Pos (4UL)
#define CPR_BT_MODEM_CLK_CTL_BT_MODEM_CLK_EN_Msk (0x1UL << CPR_BT_MODEM_CLK_CTL_BT_MODEM_CLK_EN_Pos) 
#define CPR_BT_MODEM_CLK_CTL_BT_MODEM_CLK_EN_Disable (0UL) 
#define CPR_BT_MODEM_CLK_CTL_BT_MODEM_CLK_EN_Enable (1UL)


/* Register: CPR_QDEC_CLK_CTL */
/* Offset: 0x4C */
/* Description :QDEC_CLK 时钟控制寄存器
    BIT 16 写屏蔽 BIT[3:0]，BIT 20 写屏蔽 BIT[7:4]，BIT 24 写屏蔽 BIT[11:8]，BIT 28
写屏蔽 BIT[15:12]*/


/* Bit 0..7: QDEC_CLK_DIV */
/* Description: QDEC_CLK 时钟分频系数
*/
#define CPR_QDEC_CLK_CTL_QDEC_CLK_DIV_Pos (0UL)
#define CPR_QDEC_CLK_CTL_QDEC_CLK_DIV_Msk (0xFFUL << CPR_QDEC_CLK_CTL_QDEC_CLK_DIV_Pos) 

/* Bit 8: BT_MODEM_CLK_EN */
/* Description: BT_MODEM_CLK 时钟使能
*/
#define CPR_QDEC_CLK_CTL_QDEC_CLK_EN_Pos (8UL)
#define CPR_QDEC_CLK_CTL_QDEC_CLK_EN_Msk (0x1UL << CPR_QDEC_CLK_CTL_QDEC_CLK_EN_Pos) 
#define CPR_QDEC_CLK_CTL_QDEC_CLK_EN_Disable (0UL) 
#define CPR_QDEC_CLK_CTL_QDEC_CLK_EN_Enable (1UL)



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





/* Register: CPR_TIMER_CLK_CTL */
/* Offset: 0x58 -0x64 */
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
/* Description: 
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



/* Register: CPR_OTHERCLKEN_GRCTL */
/* Offset: 0x74 */
/* Description :CTL_APB 总 线 时 钟 使 能 寄 存 器 
    高 16BITS 写屏蔽低 16BITS
    此寄存器位于 AO 域，始终保持上电状态
*/
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


/* Register: CPR_INT */
/* Offset: 0xA0 */
/* Description :M0 的 CPR 中断状态寄存器   
*/

/* Bit 10: CDBGPWRUPREQ_INTR */
/* Description: CDBGPWRUPREQ 唤醒中断
*/
#define CPR_INT_CDBGPWRUPREQ_INTR_Pos (10UL)  
#define CPR_INT_CDBGPWRUPREQ_INTR_Msk (0x01UL << CPR_INT_CDBGPWRUPREQ_INTR_Pos) 
#define CPR_INT_CDBGPWRUPREQ_INTR_NotGenerated (0UL) 
#define CPR_INT_CDBGPWRUPREQ_INTR_Generated (1UL)


/* Register: CPR_INT_EN */
/* Offset: 0xA4 */
/* Description :M0 的 CPR 中断使能寄存器
*/

/* Bit 10: CDBGPWRUPREQ_INTR_EN */
/* Description: CDBGPWRUPREQ 唤醒中断
使能
*/
#define CPR_INT_CDBGPWRUPREQ_INTR_EN_Pos (10UL)  
#define CPR_INT_CDBGPWRUPREQ_INTR_EN_Msk (0x01UL << CPR_INT_CDBGPWRUPREQ_INTR_EN_Pos) 
#define CPR_INT_CDBGPWRUPREQ_INTR_EN_Disable (0UL) 
#define CPR_INT_CDBGPWRUPREQ_INTR_EN_Enable (1UL)

/* Register: CPR_INT_RAW */
/* Offset: 0xA8 */
/* Description :CPR 中断原始状态寄存器
*/

/* Bit 10: CDBGPWRUPREQ_INTR_RAW */
/* Description: CDBGPWRUPREQ 唤醒中断
原始状态
*/
#define CPR_INT_CDBGPWRUPREQ_INTR_RAW_Pos (10UL)  
#define CPR_INT_CDBGPWRUPREQ_INTR_RAW_Msk (0x01UL << CPR_INT_CDBGPWRUPREQ_INTR_RAW_Pos) 
#define CPR_INT_CDBGPWRUPREQ_INTR_RAW_NotGenerated (0UL) 
#define CPR_INT_CDBGPWRUPREQ_INTR_RAW_Generated (1UL)


/* Register: CPR_GPIO_FUN_SEL */
/* Offset: 0xC0 -- 0xDC */
/* RegCount: 8 */
/* Description :CPR 中断原始状态寄存器
*/

/* Bit 0..4: GPIO_FUN_SELx */
/* Description: GPIOx 的功能选择
*/
#define CPR_GPIO_FUN_SEL_GPIO_FUN_SEL0_Pos (0UL)  
#define CPR_GPIO_FUN_SEL_GPIO_FUN_SEL0_Msk (0x1FUL << CPR_GPIO_FUN_SEL_GPIO_FUN_SEL0_Pos) 
#define GPIO_FUN_SEL_GPIO_Dx (0UL) 
#define GPIO_FUN_SEL_UART0_TX (1UL) 
#define GPIO_FUN_SEL_UART0_RX (2UL) 
#define GPIO_FUN_SEL_UART0_CTS (3UL) 
#define GPIO_FUN_SEL_UART0_RTS (4UL) 
#define GPIO_FUN_SEL_I2C_SCL (5UL) 
#define GPIO_FUN_SEL_I2C_SDA (6UL) 
#define GPIO_FUN_SEL_UART1_RX (7UL) 
#define GPIO_FUN_SEL_UART1_TX (8UL)
#define GPIO_FUN_SEL_SIM_IO (9UL)
#define GPIO_FUN_SEL_SIM_RST (10UL)
#define GPIO_FUN_SEL_SIM_CLK_OUT (11UL)
#define GPIO_FUN_SEL_PWM0 (12UL)
#define GPIO_FUN_SEL_PWM1 (13UL)
#define GPIO_FUN_SEL_SSI1_CLK (14UL)
#define GPIO_FUN_SEL_SSI1_SSN (15UL)
#define GPIO_FUN_SEL_SSI1_RX (16UL)
#define GPIO_FUN_SEL_SSI1_TX (17UL)



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
0：复位
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

/* Register: CPR_RSTCTL_M0RST_SW */
/* Offset: 0x108 */
/* Description :M0 相关软复位寄存器
    高 16BITS 写屏蔽低 16BITS*/
#define CPR_RSTCTL_M0RST_SW_MASK_OFFSET (16UL)

/* Bit 0: M0_SYSRESETN */
/* Description: M0 系 统 软 复 位 ， 持 续 5 个
M0_FCLK 周期，自动恢复为 1： 
0：复位；
1：不复位
*/
#define CPR_RSTCTL_M0RST_SW_M0_SYSRESETN_Pos (0UL) // 
#define CPR_RSTCTL_M0RST_SW_M0_SYSRESETN_Msk (0x1UL << CPR_RSTCTL_M0RST_SW_M0_SYSRESETN_Pos) 
#define CPR_RSTCTL_M0RST_SW_M0_SYSRESETN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_M0RST_SW_M0_SYSRESETN_Enable (RSTN_VALID) /*!< 0：复位*/

/* Bit 1: M0_PORESETN */
/* Description: M0 上电复位软复位，持续 5 个
M0_FCLK 周期，自动恢复为 1： 
0：复位；
1：不复位
*/
#define CPR_RSTCTL_M0RST_SW_M0_PORESETN_Pos (1UL) // 
#define CPR_RSTCTL_M0RST_SW_M0_PORESETN_Msk (0x1UL << CPR_RSTCTL_M0RST_SW_M0_PORESETN_Pos) 
#define CPR_RSTCTL_M0RST_SW_M0_PORESETN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_M0RST_SW_M0_PORESETN_Enable (RSTN_VALID) /*!< 0：复位*/

/* Bit 2: DBG_PORESETN */
/* Description: DEBUG 模块软软复位，持续 5 个
M0_FCLK 周期，自动恢复为 1： 
0：复位；
1：不复位
*/
#define CPR_RSTCTL_M0RST_SW_DBG_PORESETN_Pos (2UL) // 
#define CPR_RSTCTL_M0RST_SW_DBG_PORESETN_Msk (0x1UL << CPR_RSTCTL_M0RST_SW_DBG_PORESETN_Pos) 
#define CPR_RSTCTL_M0RST_SW_DBG_PORESETN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_M0RST_SW_DBG_PORESETN_Enable (RSTN_VALID) /*!< 0：复位*/

/* Bit 3: ROM_RSTN */
/* Description: ROM 软复位
0：复位；
1：不复位
*/
#define CPR_RSTCTL_M0RST_SW_ROM_RSTN_Pos (3UL) // 
#define CPR_RSTCTL_M0RST_SW_ROM_RSTN_Msk (0x1UL << CPR_RSTCTL_M0RST_SW_ROM_RSTN_Pos) 
#define CPR_RSTCTL_M0RST_SW_ROM_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_M0RST_SW_ROM_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/

/* Bit 4: M0_RAM_RSTN */
/* Description: M0_RAM 软复位：
0：复位；
1：不复位
*/
#define CPR_RSTCTL_M0RST_SW_M0_RAM_RSTN_Pos (4UL) // 
#define CPR_RSTCTL_M0RST_SW_M0_RAM_RSTN_Msk (0x1UL << CPR_RSTCTL_M0RST_SW_M0_RAM_RSTN_Pos) 
#define CPR_RSTCTL_M0RST_SW_M0_RAM_RSTN_Disable (RSTN_INVALID) /*!< 1：不复位 */
#define CPR_RSTCTL_M0RST_SW_M0_RAM_RSTN_Enable (RSTN_VALID) /*!< 0：复位*/


/* Register: CPR_RSTCTL_M0RST_MASK */
/* Offset: 0x10C */
/* Description :M0 相关软复位寄存器
*/

/* Bit 0: M0_SYSRESETN_MASK */
/* Description: 屏蔽 M0 系统软复位
0：不屏蔽；
1：屏蔽
*/
#define CPR_RSTCTL_M0RST_SW_M0_SYSRESETN_MASK_Pos (0UL) // 
#define CPR_RSTCTL_M0RST_SW_M0_SYSRESETN_MASK_Msk (0x1UL << CPR_RSTCTL_M0RST_SW_M0_SYSRESETN_MASK_Pos) 
#define CPR_RSTCTL_M0RST_SW_M0_SYSRESETN_MASK_Disable (0UL) /*!< 0：不屏蔽； */
#define CPR_RSTCTL_M0RST_SW_M0_SYSRESETN_MASK_Enable (1UL) /*!< 1：屏蔽*/

/* Bit 1: M0_PORESETN_MASK */
/* Description: 屏蔽 M0 上电复位软复位
0：不屏蔽；
1：屏蔽
*/
#define CPR_RSTCTL_M0RST_SW_M0_PORESETN_MASK_Pos (1UL) // 
#define CPR_RSTCTL_M0RST_SW_M0_PORESETN_MASK_Msk (0x1UL << CPR_RSTCTL_M0RST_SW_M0_PORESETN_MASK_Pos) 
#define CPR_RSTCTL_M0RST_SW_M0_PORESETN_MASK_Disable (0UL) /*!< 1：不复位 */
#define CPR_RSTCTL_M0RST_SW_M0_PORESETN_MASK_Enable (1UL) /*!< 0：复位*/

/* Bit 2: DBG_PORESETN */
/* Description: 屏蔽 DEBUG 模块软复位
0：不屏蔽；
1：屏蔽
*/
#define CPR_RSTCTL_M0RST_SW_DBG_PORESETN_MASK_Pos (2UL) // 
#define CPR_RSTCTL_M0RST_SW_DBG_PORESETN_MASK_Msk (0x1UL << CPR_RSTCTL_M0RST_SW_DBG_PORESETN_MASK_Pos) 
#define CPR_RSTCTL_M0RST_SW_DBG_PORESETN_MASK_Disable (0UL) /*!< 1：不复位 */
#define CPR_RSTCTL_M0RST_SW_DBG_PORESETN_MASK_Enable (1UL) /*!< 0：复位*/

/* Bit 3: M0_SYSRESETREQ_MASK */
/* Description: 屏蔽 M0 的 SYSRESETREQ
信号引起的复位
0：不屏蔽；
1：屏蔽
*/
#define CPR_RSTCTL_M0RST_SW_M0_SYSRESETREQ_MASK_Pos (3UL) // 
#define CPR_RSTCTL_M0RST_SW_M0_SYSRESETREQ_MASK_Msk (0x1UL << CPR_RSTCTL_M0RST_SW_M0_SYSRESETREQ_MASK_Pos) 
#define CPR_RSTCTL_M0RST_SW_M0_SYSRESETREQ_MASK_Disable (0UL) /*!< 1：不复位 */
#define CPR_RSTCTL_M0RST_SW_M0_SYSRESETREQ_MASK_Enable (1UL) /*!< 0：复位*/


/* Bit 4: M0_LOCKUP_MASK */
/* Description: 屏蔽 M0 的 LOCKUP 信号引
起的复位
0：不屏蔽；
1：屏蔽
*/
#define CPR_RSTCTL_M0RST_SW_M0_LOCKUP_MASK_Pos (4UL) // 
#define CPR_RSTCTL_M0RST_SW_M0_LOCKUP_MASK_Msk (0x1UL << CPR_RSTCTL_M0RST_SW_M0_LOCKUP_MASK_Pos) 
#define CPR_RSTCTL_M0RST_SW_M0_LOCKUP_MASK_Disable (0UL) /*!< 1：不复位 */
#define CPR_RSTCTL_M0RST_SW_M0_LOCKUP_MASK_Enable (1UL) /*!< 0：复位*/


/* Register: CPR_RSTCTL_LOCKUP_STATE */
/* Offset: 0x110 */
/* Description :死 锁 状 态 指 示 寄 存 器
*/

/* Bit 0: M0_LOCKUP */
/* Description: M0 的 LOCKUP 状态指示符
0：M0 处于正常状态；
1：M0 处于 LOCKUP 状态
*/
#define CPR_RSTCTL_LOCKUP_STATE_M0_LOCKUP_Pos (0UL) // 
#define CPR_RSTCTL_LOCKUP_STATE_M0_LOCKUP_Msk (0x1UL << CPR_RSTCTL_LOCKUP_STATE_M0_LOCKUP_Pos) 
#define CPR_RSTCTL_LOCKUP_STATE_M0_LOCKUP_NotGenerated (0UL) 
#define CPR_RSTCTL_LOCKUP_STATE_M0_LOCKUP_Generated (1UL)




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


/* Register: CPR_CTL_M0_STCALIB */
/* Offse: 0x144 */
/* Description :M0 配置寄存器
*/

/* Bit 0..23: M0_STCALIB_CYCLE */
/* Description: 10ms 的周期数
*/
#define CPR_CTL_M0_STCALIB_M0_STCALIB_CYCLE_Pos (0UL) 
#define CPR_CTL_M0_STCALIB_M0_STCALIB_CYCLE_Msk (0xFFFFFFUL << CPR_CTL_M0_STCALIB_M0_STCALIB_CYCLE_Pos) 

/* Bit 24: M0_STCALIB_10MS_ACCURACY */
/* Description: 10ms 是否准确：
0：准确；
1：不准确
*/
#define CPR_CTL_M0_STCALIB_M0_STCALIB_10MS_ACCURACY_Pos (24UL) 
#define CPR_CTL_M0_STCALIB_M0_STCALIB_10MS_ACCURACY_Msk (0x1UL << CPR_CTL_M0_STCALIB_M0_STCALIB_10MS_ACCURACY_Pos) 
#define CPR_CTL_M0_STCALIB_M0_STCALIB_10MS_ACCURACY (0UL) 
#define CPR_CTL_M0_STCALIB_M0_STCALIB_10MS_InACCURACY (1UL) 

/* Bit 25: USE_M0_STCLK */
/* Description:  是 否 使 用 参 考 时 钟
M0_STCLK：：
0：使用；
1：不使用
*/
#define CPR_CTL_M0_STCALIB_USE_M0_STCLK_Pos (24UL) 
#define CPR_CTL_M0_STCALIB_USE_M0_STCLK_Msk (0x1UL << CPR_CTL_M0_STCALIB_M0_STCALIB_10MS_ACCURACY_Pos) 
#define CPR_CTL_M0_STCALIB_USE_M0_STCLK_Enable (0UL) 
#define CPR_CTL_M0_STCALIB_USE_M0_STCLK_Disable (1UL) 


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

/*lint --flb "Leave library region" */
#endif
