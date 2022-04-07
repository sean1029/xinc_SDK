/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef __XINC66_xxxx_SSI_BITS_H
#define __XINC66_xxxx_SSI_BITS_H

/*lint ++flb "Enter library region" */


/* Peripheral: SPI / SSI */
/* Description: SPI compatible Interface  */

/* SSI0 BASE ADDR:0x40013000UL */
/* SSI1 BASE ADDR:0x40014000UL */
/* SSI2 BASE ADDR:0x40014800UL */


/* Register: SSI_CTRL0 */
/* Offse: 0x00 */
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
/* Offse: 0x04 */
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
/* Offse: 0x08 */
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
/* Offse: 0x10 */
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
/* Offse: 0x14 */
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
/* Offse: 0x18 */
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
/* Offse: 0x1C */
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
/* Offse: 0x20 */
/* Description: SSI 发送 FIFO 有效值寄存器
该寄存器包含发送 FIFO 有效数据个数。
*/

/* Bit 0..3  SSI TXTFL  */
/* Description：发送 FIFO 有效值：发送 FIFO 有效数据个数
*/
#define SSI_SSI_TXFL_TXTFL_Pos (0UL) /*!< Position of TXTFL field. */
#define SSI_SSI_TXFL_TXTFL_Msk (0xFUL << SSI_SSI_TXFL_TXTFL_Pos)



/* Register: SSI_RXFL */
/* Offse: 0x24 */
/* Description: SSI 接收 FIFO 有效值寄存器
该寄存器包含接收 FIFO 有效数据个数。
*/

/* Bit 0..3  SSI RXTFL  */
/* Description：接收 FIFO 有效值：接收 FIFO 有效数据个数。
*/
#define SSI_SSI_RXFL_RXTFL_Pos (0UL) /*!< Position of RXTFL field. */
#define SSI_SSI_RXFL_RXTFL_Msk (0xFUL << SSI_SSI_RXFL_RXTFL_Pos)



/* Register: SSI_STS */
/* Offse: 0x28 */
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
/* Offse: 0x2C */
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
/* Offse: 0x30 */
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
/* Offse: 0x34 */
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
/* Offse: 0x38 */
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
/* Offse: 0x3C */
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
/* Offse: 0x40 */
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
/* Offse: 0x48 */
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
/* Offse: 0x4C */
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
/* Offse: 0x50 */
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
/* Offse: 0x54 */
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
/* Offse: 0x60 */
/* Description:SSI 数据寄存器
*/
/* Bit 0..15:  SSI DR  */
/* Description：数据寄存器：由于数据帧小于等于 16bits，写
数据时保证右对齐；读数据时，返回的数据也
是自动右对齐的。
*/
#define SSI_SSI_DATA_DR_Pos (0UL) /*!< Position of DR field. */
#define SSI_SSI_DATA_DR_Msk (0xFFFFUL << SSI_SSI_DATA_DR_Pos)


/* Register: SSI2_SPI_CTRLR0 */
/* Description:SSI2 SPI控制寄存器
*/
/* Bit 0..1:  TRANS_TYPE   */
/* Description：地址和指令传输格式。
00:指令和地址都采用标准（STD）SPI发送。
01:指令采用标准SPI发送，地址采用SPI_FRF设置的格式发送。
02:指令和地址都将采用SPI_FRF设置的格式发送。
03:保留
*/
#define SSI_SSI2_SPI_CTRLR0_TRANS_TYPE_Pos (0UL) /*!< Position of TRANS_TYPE field. */
#define SSI_SSI2_SPI_CTRLR0_TRANS_TYPE_Msk (0x3UL << SSI_SSI2_SPI_CTRLR0_TRANS_TYPE_Pos)
#define SSI_SSI2_SPI_CTRLR0_TRANS_TYPE_CMD_ADDR_STD (0x0UL)
#define SSI_SSI2_SPI_CTRLR0_TRANS_TYPE_CMD_STD_ADDR_FRF (0x1UL)
#define SSI_SSI2_SPI_CTRLR0_TRANS_TYPE_CMD_ADDR_FRF (0x2UL)

/* Bit 2..5:  ADDR_L   */
/* Description：地址长度
00： 0bit 地址长度
01： 4bit 地址长度
02： 8bit 地址长度
03：12bit 地址长度
04：16bit 地址长度
05：20bit 地址长度
06：24bit 地址长度
07：28bit 地址长度
08：32bit 地址长度
09：36bit 地址长度
0A: 40bit 地址长度
0B：44bit 地址长度
0C: 48bit 地址长度
0D: 52bit 地址长度
0E: 56bit 地址长度
0F: 60bit 地址长度
*/
#define SSI_SSI2_SPI_CTRLR0_ADDR_L_Pos (2UL) /*!< Position of ADDR_L field. */
#define SSI_SSI2_SPI_CTRLR0_ADDR_L_Msk (0xFUL << SSI_SSI2_SPI_CTRLR0_ADDR_L_Pos)
#define SSI_SSI2_SPI_CTRLR0_ADDR_L_0BIT (0x0UL)
#define SSI_SSI2_SPI_CTRLR0_ADDR_L_4BIT (0x1UL)
#define SSI_SSI2_SPI_CTRLR0_ADDR_L_8BIT (0x2UL)
#define SSI_SSI2_SPI_CTRLR0_ADDR_L_12BIT (0x3UL)
#define SSI_SSI2_SPI_CTRLR0_ADDR_L_16BIT (0x4UL)
#define SSI_SSI2_SPI_CTRLR0_ADDR_L_20BIT (0x5UL)
#define SSI_SSI2_SPI_CTRLR0_ADDR_L_24BIT (0x6UL)
#define SSI_SSI2_SPI_CTRLR0_ADDR_L_28BIT (0x7UL)
#define SSI_SSI2_SPI_CTRLR0_ADDR_L_32BIT (0x8UL)
#define SSI_SSI2_SPI_CTRLR0_ADDR_L_36BIT (0x9UL)
#define SSI_SSI2_SPI_CTRLR0_ADDR_L_40BIT (0xAUL)
#define SSI_SSI2_SPI_CTRLR0_ADDR_L_44BIT (0xBUL)
#define SSI_SSI2_SPI_CTRLR0_ADDR_L_48BIT (0xCUL)
#define SSI_SSI2_SPI_CTRLR0_ADDR_L_52BIT (0xDUL)
#define SSI_SSI2_SPI_CTRLR0_ADDR_L_56BIT (0xEUL)
#define SSI_SSI2_SPI_CTRLR0_ADDR_L_60BIT (0xFUL)

/* Bit 8..9:  INST_L   */
/* Description：指令长度
双/四/八进制模式下指令长度(以bit为单位)
00: 0 bit (No Instruction)
01: 4 bit Instruction
02: 8 bit Instruction
03:16 bit Instruction
*/
#define SSI_SSI2_SPI_CTRLR0_INST_L_Pos (8UL) /*!< Position of INST_L field. */
#define SSI_SSI2_SPI_CTRLR0_INST_L_Msk (0x3UL << SSI_SSI2_SPI_CTRLR0_INST_L_Pos)
#define SSI_SSI2_SPI_CTRLR0_INST_L_0BIT (0UL)
#define SSI_SSI2_SPI_CTRLR0_INST_L_4BIT (1UL)
#define SSI_SSI2_SPI_CTRLR0_INST_L_8BIT (2UL)
#define SSI_SSI2_SPI_CTRLR0_INST_L_16BIT (3UL)

/* Bit 11..15:  WAIT_CYCLE */
/* Description：
双/四/八进制模式下控制帧传输和数据接收之间的等待周期数。
*/
#define SSI_SSI2_SPI_CTRLR0_WAIT_CYCLE_Pos (11UL) /*!< Position of WAIT_CYCLE field. */
#define SSI_SSI2_SPI_CTRLR0_WAIT_CYCLE_Msk (0x1FUL << SSI_SSI2_SPI_CTRLR0_WAIT_CYCLE_Pos)




#define SPIM_CONFIG_ORDER_MsbFirst (0UL) /*!< Most significant bit shifted out first */
#define SPIM_CONFIG_ORDER_LsbFirst (1UL) /*!< Least significant bit shifted out first */
// /* Description: Polarity of CSN output */
#define SPIM_CSNPOL_CSNPOL_LOW (0UL) /*!< Active low (idle state high) */
#define SPIM_CSNPOL_CSNPOL_HIGH (1UL) /*!< Active high (idle state low) */



/*lint --flb "Leave library region" */
#endif
