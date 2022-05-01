#ifndef     _HCIT_LC1540_SERIAL_H
#define     _HCIT_LC1540_SERIAL_H

/******************************************************************************
 * MODULE NAME:    hcit_chimera_16550_serial.h
 * PROJECT CODE:   Bluestream
 * DESCRIPTION:    HCI RS232 Transport header for 16550 compatible UART on Chimera
 * AUTHOR:         John Sheehy
 * DATE:           09 March 2001
 *
 * SOURCE CONTROL: $Id: hcit_chimera_16550_serial.h,v 1.7 2009/04/28 16:08:48 morrisc Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2001-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *    09 March 2001 - Rewritten for new framework
 *
******************************************************************************/

#include    "sys_config.h"    
#include    "sys_types.h"
#include    "sys_features.h"
#include    "hci_generic.h"

/* 
 * Interrupt ID bits 
 */

#define IIR_INT0                0x00        //流控状态改变中断
#define IIR_INT1                0x01        //没有中断
#define IIR_INT2                0x02        //发送保持寄存器THR空中断
#define IIR_INT4                0x04        //接收数据有效中断
#define IIR_INT6                0x06        //错误中断
#define IIR_INT7                0x07        //忙检测中断
#define	IIR_INT12   		    0x0c	    //字符超时中断

/* 
 * Line status LSR 
 */

#define UART_TSR_THRE           0x20        //发送保存寄存器空. 
#define UART_TSR_FE             0x08        //帧格式错误.
#define UART_TSR_OE             0x02        //接收FIFO溢出.
#define UART_TSR_DR             0x01        //接收缓存寄存器中有数据.

/* 
 * Modem status MSR 
 */
#define UART_MSR_CTS            0x10        //管脚信号uartx_cts 的值的反.
#define UART_MSR_DCTS           0x01        //


/*
 * Function Interfaces
 */

void HCIT_hw_Initialise(u_int32 baud_rate);
void HCIT_hw_Shutdown(void);

void HCIT_hw_Tx_Block(volatile u_int8 **bufp, volatile u_int32 *length, void (*txcb)(void));
void HCIT_hw_Tx_Char_Polled(volatile u_int8 **bufp, volatile u_int32 *length);

u_int16 HCIT_hw_Get_Char(void);

void HCIT_hw_Interrupt_Handler(void);
void HCIT_hw_Empty_FIFO(void);

#if (PRH_BS_DEV_PROCESS_UART_RX_DATA_IN_IRQ_CONTEXT == 1)
void HCIT_Chimera_16550_Service_Rx_Fifo_Cache(void);
#endif


#endif  /* _HCIT_LC1540_SERIAL_H */
