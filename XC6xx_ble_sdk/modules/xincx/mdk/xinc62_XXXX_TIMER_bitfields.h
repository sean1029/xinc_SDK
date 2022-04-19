/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef __XINC62_xxxx_TIMER_BITS_H
#define __XINC62_xxxx_TIMER_BITS_H

/*lint ++flb "Enter library region" */

/* Peripheral: TIMERx */
/* Description: TIMERx Interface  */

/* TIMER0 BASE ADDR:0x40003000UL */
/* TIMER1 BASE ADDR:0x40003014UL */
/* TIMER2 BASE ADDR:0x40003028UL */
/* TIMER3 BASE ADDR:0x4000303CUL */

/* TIMER 全局寄存器 BASE ADDR:0x400030A0UL */

/* Register:TIMERx_TLC    RW */
/* Offse: 0x00 */
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
/* Offse: 0x04 */
/* Description:读该寄存器可以得到计数器当前值。
 */
/* Bit 0..31 : TIMER TCV   RW */
/* Description: 当前计数器的值：显示当前计数器的计
数值。*/
#define TIMERx_TCV_TCV_Pos (0UL) /*!< Position of TCV field. */
#define TIMERx_TCV_TCV_Msk (0xFFFFFFFFUL << TIMERx_TCV_TCV_Pos) /*!< Bit mask of TCV field. */



/* Register:TIMERx_TCR    RW */
/* Offse: 0x08 */
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
/* Offse: 0x0C */
/* Description:中断清除寄存器
 */
/* Bit 0 : TIMERx TIC   R */
/* Description: 中断清除：读该寄存器将清除定时器中断，
因此该寄存器的值也被清 0。
*/
#define TIMERx_TIC_TIC_Pos (0UL) /*!< Position of TIC field. */
#define TIMERx_TIC_TIC_Msk (0x1UL << TIMERx_TIC_TIC_Pos) /*!< Bit mask of TIC field. */


/* Register:TIMERx_TIS    R */
/* Offse: 0x10 */
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
/* Offse: 0x00 */
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
/* Offse: 0x04 */
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
/* Offse: 0x08 */
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


/*lint --flb "Leave library region" */
#endif
