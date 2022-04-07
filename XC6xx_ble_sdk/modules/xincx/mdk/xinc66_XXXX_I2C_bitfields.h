/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef __XINC66_xxxx_I2C_BITS_H
#define __XINC66_xxxx_I2C_BITS_H

/*lint ++flb "Enter library region" */

/* Peripheral: I2C */
/* Description: I2C compatible Two-Wire Interface  */

/* I2C BASE ADDR:0x40006000UL */

/* Register: I2C_CON */
/* Offse: 0x00 */
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
/* Offse: 0x04 */
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
/* Offse: 0x08 */
/* Description: I2C 从地址控制寄存器 
该寄存器只有在模块不使能时（相对应 I2C_ENABLE 寄存器置0）才能被写入，其余
时刻写入数据无效。*/

/* Bit 0..9: I2C I2C_SAR   */
/* Description： Slave 数据传输的目标地址   */
#define I2C_TAR_I2C_SAR_Pos (0UL) /*!< Position of I2C_TAR field. */
#define I2C_TAR_I2C_SAR_Msk (0x1FFUL << I2C_TAR_I2C_SAR_Pos) /*!< Bit mask of I2C_TAR field. */


/* Register: I2C_DATA_CMD */
/* Offse: 0x10 */
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
/* Offse: 0x14 */
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
/* Offse: 0x18 */
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
/* Offse: 0x1C */
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
/* Offse: 0x20 */
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
/* Offse: 0x2C */
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
/* Offse: 0x30 */
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
/* Offse: 0x34 */
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
/* Offse: 0x38 */
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
/* Offse: 0x3C */
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
/* Offse: 0x40 */
/* Description: I2C  清除全局中断寄存器 */

/* Bit 0:  I2C CLR_INTR   */
/* Description： 读此寄存器清除组合中断，各个独立中断以及I2C_TX_ABRT_SOURCE 寄存器*/
#define I2C_CLR_INTR_Pos (0UL) /*!< Position of CLR_INTR field. */
#define I2C_CLR_INTR_Msk (0x1UL << I2C_CLR_INTR_Pos) /*!< Bit mask of CLR_INTR field. */



/* Register: I2C_CLR_RX_UNDER */
/* Offse: 0x44 */
/* Description: I2C  清除 RX_UNDER 中断寄存器 */

/* Bit 0:  I2C CLR_RX_UNDER   */
/* Description： 读 此 寄 存 器 清 除 中 断 RX_UNDER  */
#define I2C_CLR_RX_UNDER_Pos (0UL) /*!< Position of CLR_RX_UNDER field. */
#define I2C_CLR_RX_UNDER_Msk (0x1UL << I2C_CLR_RX_UNDER_Pos) /*!< Bit mask of CLR_RX_UNDER field. */


/* Register: I2C_CLR_RX_OVER */
/* Offse: 0x48 */
/* Description: I2C  清除 RX_OVER 中断寄存器 */

/* Bit 0:  I2C CLR_RX_OVER   */
/* Description： 读 此 寄 存 器 清 除 中 断 RX_OVER  */
#define I2C_CLR_RX_OVER_Pos (0UL) /*!< Position of CLR_RX_OVER field. */
#define I2C_CLR_RX_OVER_Msk (0x1UL << I2C_CLR_RX_OVER_Pos) /*!< Bit mask of CLR_RX_OVER field. */


/* Register: I2C_CLR_TX_OVER */
/* Offse: 0x4C */
/* Description: I2C  清除 TX_OVER 中断寄存器 */

/* Bit 0:  I2C CLR_TX_OVER   */
/* Description： 读 此 寄 存 器 清 除 中 断 TX_OVER  */
#define I2C_CLR_TX_OVER_Pos (0UL) /*!< Position of CLR_TX_OVER field. */
#define I2C_CLR_TX_OVER_Msk (0x1UL << I2C_CLR_TX_OVER_Pos) /*!< Bit mask of CLR_TX_OVER field. */


/* Register: I2C_CLR_RD_REQ */
/* Offse: 0x50 */
/* Description: I2C  清除 RD_REQ 中断寄存器 */

/* Bit 0:  I2C CLR_RD_REQ   */
/* Description： 读 此 寄 存 器 清 除 中 断 RD_REQ  */
#define I2C_CLR_RD_REQ_Pos (0UL) /*!< Position of CLR_RD_REQ field. */
#define I2C_CLR_RD_REQ_Msk (0x1UL << I2C_CLR_RD_REQ_Pos) /*!< Bit mask of CLR_RD_REQ field. */


/* Register: I2C_CLR_TX_ABRT */
/* Offse: 0x54 */
/* Description: I2C  清除 TX_ABRT 中断寄存器 */

/* Bit 0:  I2C CLR_TX_ABRT   */
/* Description： 读 此 寄 存 器 清 除 中 断 TX_ABRT  */
#define I2C_CLR_TX_ABRT_Pos (0UL) /*!< Position of CLR_TX_ABRT field. */
#define I2C_CLR_TX_ABRT_Msk (0x1UL << I2C_CLR_TX_ABRT_Pos) /*!< Bit mask of CLR_TX_ABRT field. */


/* Register: I2C_CLR_RX_DONE */
/* Offse: 0x58 */
/* Description: I2C  清除 RX_DONE 中断寄存器 */

/* Bit 0:  I2C CLR_RX_DONE   */
/* Description： 读 此 寄 存 器 清 除 中 断 RX_DONE  */
#define I2C_CLR_RX_DONE_Pos (0UL) /*!< Position of CLR_RX_DONE field. */
#define I2C_CLR_RX_DONE_Msk (0x1UL << I2C_CLR_RX_DONE_Pos) /*!< Bit mask of CLR_RX_DONE field. */



/* Register: I2C_CLR_ACTIVITY */
/* Offse: 0x5C */
/* Description: I2C  清除 ACTIVITY 中断寄存器 */

/* Bit 0:  I2C CLR_ACTIVITY   */
/* Description： 读 此 寄 存 器 清 除 中 断 ACTIVITY  */
#define I2C_CLR_ACTIVITY_Pos (0UL) /*!< Position of CLR_ACTIVITY field. */
#define I2C_CLR_ACTIVITY_Msk (0x1UL << I2C_CLR_ACTIVITY_Pos) /*!< Bit mask of CLR_ACTIVITY field. */


/* Register: I2C_CLR_STOP_DET */
/* Offse: 0x60 */
/* Description: I2C  清除 STOP_DET 中断寄存器 */

/* Bit 0:  I2C CLR_STOP_DET   */
/* Description： 读 此 寄 存 器 清 除 中 断 STOP_DET  */
#define I2C_CLR_STOP_DET_Pos (0UL) /*!< Position of CLR_STOP_DET field. */
#define I2C_CLR_STOP_DET_Msk (0x1UL << I2C_CLR_STOP_DET_Pos) /*!< Bit mask of CLR_STOP_DET field. */


/* Register: I2C_CLR_START_DET */
/* Offse: 0x64 */
/* Description: I2C  清除 START_DET 中断寄存器 */

/* Bit 0:  I2C CLR_START_DET   */
/* Description： 读 此 寄 存 器 清 除 中 断 START_DET  */
#define I2C_CLR_START_DET_Pos (0UL) /*!< Position of CLR_START_DET field. */
#define I2C_CLR_START_DET_Msk (0x1UL << I2C_CLR_START_DET_Pos) /*!< Bit mask of CLR_START_DET field. */


/* Register: I2C_CLR_GEN_CALL */
/* Offse: 0x68 */
/* Description: I2C  清除 GEN_CALL 中断寄存器 */

/* Bit 0:  I2C CLR_GEN_CALL   */
/* Description： 读 此 寄 存 器 清 除 中 断 GEN_CALL  */
#define I2C_CLR_GEN_CALL_Pos (0UL) /*!< Position of CLR_GEN_CALL field. */
#define I2C_CLR_GEN_CALL_Msk (0x1UL << I2C_CLR_GEN_CALL_Pos) /*!< Bit mask of CLR_GEN_CALL field. */


/* Register: I2C_ENABLE */
/* Offse: 0x6C */
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
/* Offse: 0x70 */
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
/* Offse: 0x74 */
/* Description: 发送 FIFO 数据计数寄存器 */

/* Bit 0..4:  I2C TXFLR  */
/* Description： TX FIFO 有效数据项个数 */
#define I2C_TXFLR_Pos (0UL) /*!< Position of TXFLR field. */
#define I2C_TXFLR_Msk (0x1FUL << I2C_TXFLR_Pos) /*!< Bit mask of TXFLR field. */


/* Register: I2C_RXFLR */
/* Offse: 0x78 */
/* Description: 接收 FIFO 数据计数寄存器 */

/* Bit 0..4:  I2C RXFLR  */
/* Description： RX FIFO 有效数据项个数 I2C 被禁止或者传输异常中止时，该寄存器清 0。*/
#define I2C_RXFLR_Pos (0UL) /*!< Position of RXFLR field. */
#define I2C_RXFLR_Msk (0x1FUL << I2C_RXFLR_Pos) /*!< Bit mask of RXFLR field. */


/* Register: I2C_SDA_HOLD */
/* Offse: 0x7C */
/* Description: SDA 保持时间 */

/* Bit 0..15:  I2C I2C_SDA_HOLD  */
/* Description： 设置以 i2c_clk 为单位的 SDA 保持时间 */
#define I2C_SDA_HOLD_Pos (0UL) /*!< Position of I2C_SDA_HOLD field. */
#define I2C_SDA_HOLD_Msk (0xFFFFUL << I2C_RXFLR_Pos) /*!< Bit mask of I2C_SDA_HOLD field. */


/* Register: I2C_TX_ABRT_SOURCE */
/* Offse: 0x80 */
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

/*lint --flb "Leave library region" */
#endif
