/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef __XINC62_xxxx_GPIO_BITS_H
#define __XINC62_xxxx_GPIO_BITS_H

/*lint ++flb "Enter library region" */


/* Peripheral: GPIO */
/* Description: GPIO Port  */

/* GPIO Base Addr: 0x40001000 */

/* Register: GPIO_PORT_DRx */
/* Offse: 0x00~0x04 */
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
/* Offse: 0x20~0x24 */
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
/* Offse: 0x40 */
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
/* Offse: 0x100~0x11c */
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
/* Offse: 0x180~0x184 */
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
/* Offse: 0x1a0 */
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
/* Offse: 0x1b0 */
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
/* Offse: 0x200~0x204 */
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
/* Offse: 0x220 */
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



/*lint --flb "Leave library region" */
#endif
