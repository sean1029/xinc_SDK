/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef __XINC62_xxxx_RTC_BITS_H
#define __XINC62_xxxx_RTC_BITS_H

/*lint ++flb "Enter library region" */

/* Peripheral: RTC */
/* Description: RTC Interface  */

/* RTC BASE ADDR:0x40002000UL */

/* Register:RTC_CCVR    R */
/* Offse: 0x00 */

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
/* Offse: 0x04 */
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
/* one Offse: 0x08 */
/* two Offse: 0x0c */
/* three Offse: 0x10 */
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
/* Offse: 0x14 */
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
/* Offse: 0x18 */
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
/* Offse: 0x18 */
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
/* Offse: 0x1C */
/* Description: 当前周计数器值寄存器
*/

/* Bit 0..2 : RTC WVR  R */
/* Description:记录当前日期为星期几：
*/
#define RTC_WVR_Pos (0UL) /*!< Position of WVR field. */
#define RTC_WVR_Msk (0x7UL << RTC_WVR_Pos) /*!< Bit mask of WVR field. */

/* Register:RTC_WLR  WR*/
/* Offse: 0x20 */
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
/* Offse: 0x24 */
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
/* Offse: 0x28 */
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
/* Offse: 0x2C */
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
/* Offse: 0x30 */
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
/* Offse: 0x34 */
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
/* Offse: 0x38 */
/* Description: 当前秒内计数值寄存器
*/
/* Bit 0..15 : RTC RVR  R */
/* Description:当前秒内计数值：
记录 1 秒计数器的计数值。
*/
#define RTC_RVR_Pos (0UL) /*!< Position of RVR field. */
#define RTC_RVR_Msk (0xFFFFUL << RTC_ISR_RAW_T2F_raw_Pos) /*!< Bit mask of RVR field. */


/* Register:AO_TIMER_CTL  WR*/
/* Offse: 0x40 */
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
/* Offse: 0x4C */
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
/* Offse: 0x50 */
/* Description: FREQ_TIMER
*/
/* Bit 0..23 : FREQ_TIMER_VALUE  R */
/* Description:32K 校准计数周期
*/
#define RTC_FREQ_TIMER_VAL_Pos (0UL) /*!< Position of FREQ_TIMER_VALUE field. */
#define RTC_FREQ_TIMER_VAL_Msk (0xFFFFFFUL << RTC_FREQ_TIMER_VAL_Pos) /*!< Bit mask of FREQ_TIMER_VALUE field. */



/*lint --flb "Leave library region" */
#endif
