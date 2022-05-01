/******************************************************************************
 *        
 * MODULE NAME:    sys_irq.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    Hardware Interrupt Interface - Chimera
 * MAINTAINER:     Ivan Griffin/Cyril Jean
 * DATE:           09 August 1999
 *
 * SOURCE CONTROL: $Id: sys_irq.c,v 1.102 2009/04/28 16:08:51 morrisc Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *    09 Aug 1999 -   IG       -  Initial Version
 *    Oct 2000    -   CJ       -  Port to Chimera
 *
 ******************************************************************************/
#include "sys_config.h"
#include "sys_types.h"
#include "sys_features.h"

#include "sys_config.h"
#include "sys_power.h"
#include "sys_irq.h"
#include "lc_types.h"
#include "lslc_slot.h"
#include "lslc_irq.h"
#include "hw_jalapeno.h"
#include "hw_register.h"
#include "hw_macro_defs.h"
#include "sys_mmi.h"
#include "tra_hcit.h"

#ifndef __USE_INLINES__
#include "sys_irq_chimera_impl.h"
#endif

#include "xinc_m0.h"

/******************************************************************************
 *
 * Module variables
 *
 ******************************************************************************/
#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 1)
#define DSRS_TABLE_SIZE		10

static volatile u_int32 pending_dsr_counter;
static volatile u_int32 dsr_lock;
static volatile u_int32 dsr_table_head;
static volatile u_int32 dsr_table_tail;

static void (*pending_dsr_table[DSRS_TABLE_SIZE])(void);
#endif

/******************************************************************************
 *
 * FUNCTION:  UART1_Handler
 * PURPOSE:   Services fast interrupt requests to micro-controller
 *
 ******************************************************************************/

#if (BUILD_TYPE != GENERIC_BUILD)

void UART1_Handler(void)
{
	TRAhcit_Call_Interrupt_Handler();
}

#endif


/* stub for proper compilation */
void SYSirq_IRQ_Tabasco_ISR(void)
{
}

/******************************************************************************
 *
 * FUNCTION:  BLE_Handler
 * PURPOSE:   Services Interrupt Requests to micro-controller
 *
 ******************************************************************************/
void BLE_Handler(void)
{
    LSLCirq_IRQ_Handler();
    
#if (PRH_BS_DEV_PROCESS_UART_RX_DATA_IN_IRQ_CONTEXT == 1)
    TRAhcit_Service_Rx_Fifo_Cache();
#endif

    return;
}

/******************************************************************************
 *
 * FUNCTION:  SYSirq_Disable_Interrupts_Save_Flags
 * PURPOSE:   Disables ARM IRQ and FIQ Interrupts, saves previous
 *            PSR
 *
 ******************************************************************************/
void SYSirq_Disable_Interrupts_Save_Flags(u_int32 *flags)
{
    *flags = __get_PRIMASK();
    __disable_irq();  
}

/******************************************************************************
 *
 * FUNCTION:  SYSirq_Enable_Interrupts_Save_Flags
 * PURPOSE:   Enables ARM IRQ and FIQ Interrupts, saves previous
 *            PSR
 *
 ******************************************************************************/
void SYSirq_Enable_Interrupts_Save_Flags(u_int32 *flags)
{
    *flags = __get_PRIMASK();
    __enable_irq();
}

/******************************************************************************
 *
 * FUNCTION:  SYSirq_Interrupts_Restore_Flags
 * PURPOSE:   Restores previously saved previous PSR
 *
 ******************************************************************************/
void SYSirq_Interrupts_Restore_Flags(u_int32 flags)
{
    __set_PRIMASK(flags);
}


/******************************************************************************
 *
 * FUNCTION:  SYSirq_Initialise
 * PURPOSE:   Initialise Interrupt Requests
 *
 ******************************************************************************/
void SYSirq_Initialise(void)
{
    /*
     * Intialise interrupts
     */
#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 1)
    dsr_table_head = 0;
    dsr_table_tail = 0;
    
    pending_dsr_counter = 0;
    dsr_lock = 0;
#endif
    /* SYSirq_IRQ_Disable_All(); - now done in BT_Init */

    NVIC_EnableIRQ(BLUETOOTH_IRQn);
	

    /* DO NOT RE-ENABLE INTERRUPTS HERE!!! */
}

#ifndef REDUCE_ROM
/******************************************************************************
 *
 * FUNCTION:  SYSirq_Disable_Baseband_ISR_Save_Flags
 * PURPOSE:   Disables ARM IRQ Baseband_ISR, saves previous
 *            PSR
 *
 ******************************************************************************/
void SYSirq_Disable_Baseband_ISR_Save_Flags(u_int32 *flags)
{
    *flags = __get_PRIMASK();
    __disable_irq();  
}

/******************************************************************************
 *
 * FUNCTION:  SYSirq_Enable_Baseband_ISR_Save_Flags
 * PURPOSE:   Enables ARM IRQ Baseband_ISR, saves previous
 *            PSR
 *
 ******************************************************************************/
void SYSirq_Enable_Baseband_ISR_Save_Flags(u_int32 *flags)
{
    *flags = __get_PRIMASK();
    __enable_irq();  
}

/******************************************************************************
 *
 * FUNCTION:  SYSirq_Baseband_ISR_Restore_Flags
 * PURPOSE:   Restores previously saved previous PSR
 *
 ******************************************************************************/
void SYSirq_Baseband_ISR_Restore_Flags(u_int32 flags)
{
    __set_PRIMASK(flags);
}
#endif
