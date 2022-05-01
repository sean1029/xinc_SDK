#ifndef _PARTHUS_HW_IRQ_
#define _PARTHUS_HW_IRQ_

/******************************************************************************
 * MODULE NAME:    sys_irq.h
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    Hardware Interrupt Functions
 * MAINTAINER:     Ivan Griffin
 * DATE:           09 August 1999
 *
 * SOURCE CONTROL: $Id: sys_irq.h,v 1.27 2004/07/07 14:30:50 namarad Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 1999-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *    V1.0     09 August 1999 -   ig       - Initial Version V0.9
 *
 * SOURCE:
 * ISSUES:
 * NOTES TO USERS:
 ******************************************************************************/
#include "sys_config.h"
#include "hw_jalapeno.h"                   /*Dedicated to hardware platform */

void SYSirq_Initialise(void);

void BLE_Handler(void);
void UART1_Handler(void);


void SYSirq_Disable_Interrupts_Save_Flags(u_int32* flags);
void SYSirq_Enable_Interrupts_Save_Flags(u_int32* flags);
void SYSirq_Interrupts_Restore_Flags(u_int32 flags);

void SYSirq_Disable_Baseband_ISR_Save_Flags(u_int32 *flags);
void SYSirq_Enable_Baseband_ISR_Save_Flags(u_int32 *flags);
void SYSirq_Baseband_ISR_Restore_Flags(u_int32 flags);

u_int32 SYSirq_Get_Active_IRQ(void);
u_int32 SYSirq_Get_Active_FIQ(void);

#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 1)
void SYSirq_IRQ_Tabasco_ISR(void);
void SYSirq_IRQ_wrapper(void);
#endif

#ifdef __USE_INLINES__
#include "sys_irq_chimera_impl.h"
#endif



#endif
