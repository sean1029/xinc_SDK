/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef __XINC66_xxxx_UART_BITS_H
#define __XINC66_xxxx_UART_BITS_H

/*lint ++flb "Enter library region" */

/* Peripheral: UART  */
/* Description: UART compatible Interface */

/* UART0 BASE ADDR:0x40010000 */
/* UART1 BASE ADDR:0x40011000 */
/* UART2 BASE ADDR:0x40011400 */

/* Register: UARTx_RBR */
/* Offse: 0x00 */
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
/* Offse: 0x00 */
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
/* Offse: 0x00 */
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
/* Offse: 0x04 */
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
/* Offse: 0x08 */
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
/* Offse: 0x08 */
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
/* Offse: 0x0C */
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
/* Offse: 0x10 */
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
/* Offse: 0x14 */
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
/* Offse: 0x18 */
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
/* Offse: 0x7C */
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
