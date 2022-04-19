/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef __XINC62_xxxx_PWM_BITS_H
#define __XINC62_xxxx_PWM_BITS_H

/*lint ++flb "Enter library region" */

/* Peripheral: PWM */
/* Description: PWM  Interface */

/* PWM0 BASE ADDR:0x40017000UL */
/* PWM1 BASE ADDR:0x40017040UL */
/* PWM2 BASE ADDR:0x40017400UL */
/* PWM3 BASE ADDR:0x40017440UL */
/* PWM4 BASE ADDR:0x40017800UL */
/* PWM5 BASE ADDR:0x40017840UL */


/* Register: PWM_EN */
/* Offse: 0x00 */
/* Description: PWM 使能寄存器 
*/
#define PWM_PWM_MODE_ACC_100 (0UL) /*!< 0:占空比精度1/100*/

/* Bit 0 : PWM PWM_EN   */
/* Description:PWM 使能控制位。
此寄存器控制 PWM 内部计数
器及输出的工作状态 */
#define PWM_EN_Pos (0UL) /*!< Position of PWM_EN field. */
#define PWM_EN_Msk (0x1UL << PWM_EN_Pos) /*!< Bit mask of PWM_EN field. */
#define PWM_EN_Enable (1UL) /*!< PWM 使能 */
#define PWM_EN_Disable (0UL) /*!< PWM 不使能*/


/* Register: PWM_UP */
/* Offse: 0x04 */
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
/* Offse: 0x08 */
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
/* Offse: 0x0C */
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
/* Offse: 0x10 */
/* Description: PWM 占空比设置寄存器
*/

/* Bit 0..7 : PWM OCPY_RATIO   */
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
#define PWM_OCPY_OCPY_RATIO_Msk (0xFFUL << PWM_OCPY_OCPY_RATIO_Pos) /*!< Bit mask of OCPY_RATIO field. */
#define PWM_OCPY_MAX_100 (100UL)

/* Register: PWM_COMP_EN */
/* Offse: 0x1C */
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
/* Offse: 0x20 */
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



/*lint --flb "Leave library region" */
#endif
