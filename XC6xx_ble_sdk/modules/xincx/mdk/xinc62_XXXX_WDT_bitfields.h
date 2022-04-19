/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef __XINC62_xxxx_WDT_BITS_H
#define __XINC62_xxxx_WDT_BITS_H

/*lint ++flb "Enter library region" */

/* Peripheral: WDT */
/* Description: WDT Interface  */

/* WDT BASE ADDR:0x40004000UL */

/* Register: WDT_CR */
/* Offse: 0x00 */
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
/* Offse: 0x04 */
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
/* Offse: 0x08 */
/* Description: 当前计数值寄存器
只读寄存器，用于显示计数器当前计数值。 */

/* Bit 0..31 : WDT CCVR   */
/* Description: 当前计数值：记录当前计数器的计数值。 */
#define WDT_CCVR_CCVR_Pos (0UL) /*!< Position of CCVR field. */
#define WDT_CCVR_CCVR_Msk (0xFFFFFFFUL << WDT_CCVR_CCVR_Pos) /*!< Bit mask of CCVR field. */


/* Register: WDT_CRR */
/* Offse: 0x0C */
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
/* Offse: 0x10 */
/* Description:中断状态寄存器
该寄存器用于显示 WDT 当前中断状态。 */

/* Bit 0 : WDT STAT   */
/* Description: 当前中断状态：显示 WDT 当前中断状态。 */
#define WDT_STAT_STAT_Pos (0UL) /*!< Position of STAT field. */
#define WDT_STAT_STAT_Msk (0x1UL << WDT_STAT_STAT_Pos) /*!< Bit mask of STAT field. */
#define WDT_STAT_STAT_NotGenerated (0x0UL) /*!< 0：没有发生中断 */
#define WDT_STAT_STAT_Generated (0x1UL) /*!< 1：发生中断 */



/* Register: WDT_ICR */
/* Offse: 0x14 */
/* Description:中断清除寄存器
只读寄存器，用于清除 WDT 中断。 */

/* Bit 0 : WDT ICR   */
/* Description: 清除中断：读此寄存器会清除 WDT 当前中断。 */
#define WDT_ICR_ICR_Pos (0UL) /*!< Position of ICR field. */
#define WDT_ICR_ICR_Msk (0x1UL << WDT_ICR_ICR_Pos) /*!< Bit mask of ICR field. */
#define WDT_ICR_ICR_NotGenerated (0x0UL) /*!< 0：没有发生中断 */
#define WDT_ICR_ICR_Generated (0x1UL) /*!< 1：发生中断 */




/*lint --flb "Leave library region" */
#endif
