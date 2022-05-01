/*************************************************************************
 * MODULE NAME:    lc_log.c
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    Baseband Link Controller Interface Module
 * MAINTAINER:     Conor Morris
 *
 * SOURCE CONTROL: $Id: lc_log.c,v 1.9 2011/01/21 05:51:22 garyf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 *
 * NOTES TO USERS:
 * This code will be optimised to inlines where possible.
 *
 *************************************************************************/

#include "sys_config.h"
#include "lc_types.h"
#include "lc_log.h"
#include "tc_event_gen.h"
#include "uslc_chan_ctrl.h" 

#if (PRH_BS_DBG_LC_LOG==1 || PRH_BS_CFG_SYS_TRACE_LC_VIA_HCI_SUPPORTED)

/*
 * The LC log used for on-board debugging
 */

t_lc_log_procedure_info lc_log_procedure_list[LC_MAX_NUM_PROCEDURES];


/*************************************************************************
 * LC_Log_Initialise
 *
 * Initialise the Link Controller Log
 *************************************************************************/
void LC_Log_Initialise(void)
{
    /*
     * Initialise lc_log_procedure_list
     */
    u_int i;
    u_int8 *ptr = (u_int8 *) &lc_log_procedure_list[0];

    for(i=0; i < sizeof(lc_log_procedure_list); i++) 
    {
        *ptr=0;
        ptr++;
    }
    
}

/*************************************************************************
 * Set of LC logging functions.
 *
 * These functions should only be used by objects below this layer.
 *************************************************************************/

void LC_Log_Procedure_Request(t_ulc_procedure procedure)
{
    TCeg_Send_LC_Event(procedure, USLCchac_get_device_state(), 
                        0 /*lc_arg*/, 0 /*tx0_rx1*/);
    lc_log_procedure_list[procedure].procedure_request++;
}

void LC_Log_Procedure_Accept(t_ulc_procedure procedure)
{
    TCeg_Send_LC_Event(procedure, USLCchac_get_device_state(), 
                        1 /*lc_arg*/, 0 /*tx0_rx1*/);
    lc_log_procedure_list[procedure].procedure_accept++;
}

void LC_Log_Procedure_Start(t_ulc_procedure procedure)
{
    TCeg_Send_LC_Event(procedure, USLCchac_get_device_state(), 
                        2 /*lc_arg*/, 0 /*tx0_rx1*/);
    lc_log_procedure_list[procedure].procedure_start++;
}

void LC_Log_Procedure_Cancel(t_ulc_procedure procedure)
{
    TCeg_Send_LC_Event(procedure, USLCchac_get_device_state(), 
                        3 /*lc_arg*/, 0 /*tx0_rx1*/);
    lc_log_procedure_list[procedure].procedure_cancel++;
}

void LC_Log_Procedure_Recv_Pkts_State1(t_ulc_procedure procedure)
{
    TCeg_Send_LC_Event(procedure, USLCchac_get_device_state(), 
                        4 /*lc_arg*/, 0 /*tx0_rx1*/);
    lc_log_procedure_list[procedure].pkts_recv_state1++;
}

void LC_Log_Procedure_Recv_Pkts_State2(t_ulc_procedure procedure)
{
    TCeg_Send_LC_Event(procedure, USLCchac_get_device_state(), 
                        5 /*lc_arg*/, 0 /*tx0_rx1*/);
    lc_log_procedure_list[procedure].pkts_recv_state2++;
}
#else
/*
 * Completely empty modules are illegal in ANSI C
 */
void _LClog_Dummy(void) { }
#endif
