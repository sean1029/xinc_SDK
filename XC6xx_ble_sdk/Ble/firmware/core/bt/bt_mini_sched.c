/***********************************************************************
 *
 * MODULE NAME:    bt_mini_sched.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    HC scheduling methods for non-interrupt activities.
 * MAINTAINER:     Tom Kerwick
 * CREATION DATE:  November 1999
 *
 * SOURCE CONTROL: $Id: bt_mini_sched.c,v 1.120 2014/03/11 03:13:32 garyf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 1999-2004 Ceva Inc.
 *     All rights reserved.
 *    
 ***********************************************************************/

#include "sys_config.h"
#include "sys_rand_num_gen.h"

#include "lc_interface.h"
#include "lmp_link_policy.h"
#include "lmp_link_policy_protocol.h"
#include "lmp_link_power_control.h"
#include "lmp_timer.h"
#include "bt_mini_sched.h"
#include "bt_init.h"
#include "bt_test.h"
#include "bt_pump.h"
#include "bt_timer.h"
#include "hc_const.h"
#include "hc_event_gen.h"
#include "hc_cmd_disp.h"
#include "hc_flow_control.h"
#include "tra_hcit.h"
#include "tra_uart.h"
#include "tra_codec.h"
#include "lslc_irq.h"
#include "uslc_chan_ctrl.h"
#include "sys_power.h"
#include "lslc_class.h"
#include "le_scan.h"
#include "le_config.h"
#include "le_link_layer.h"
#include "le_connection.h"
#include "le_security.h"
#include "lmp_cmd_disp.h"
#include "global_val.h"
#include "patch_function_id.h"


#if (PRH_BS_CFG_SYS_LE_SMP_INCLUDED==1)
#include "le_smp.h"
#endif
#if (PRH_BS_CFG_SYS_LE_GATT_INCLUDED==1)
#include "le_att.h"
#endif
#if(PRH_BS_CFG_SYS_LE_GAP_INCLUDED==1)
#include "le_gap.h"
#endif
extern t_LE_Config LE_config;
extern u_int8 le_mode;

static t_error _BTms_Handle_HC_Reset_If_Reqd(void);
static t_error _BTms_Handle_HC_Sleep_If_Reqd(void);

//u_int8 _BTms_pending_hc_reset = 0;


t_error BTms_Sched(u_int32 num_interations)
{
	
	if(PATCH_FUN[BTMS_SCHED_ID]){
         
         return((t_error (*)(u_int32 num_interations))PATCH_FUN[BTMS_SCHED_ID])(num_interations) ;
    }

    t_error status = NO_ERROR;
    u_int32 inter;

    for(inter = 0; inter < num_interations; num_interations--)
    {
		/*
         * Dispatch any LMP commands
         */
		status = LMdisp_LMP_Command_Dispatcher();
		#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1) && ((PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE))
        {
        	LEconnection_Handle_Data_To_Encrypt();
        }
		#endif
		/*
		 * Report Number of Completed Packets Event to Host, if necessary
		 */
				 
		HCfc_Handle_Host_To_Host_Controller_Flow_Control();
			
		/*
		 * Dispatch any pending HCI events (e.g. there could be a num_completed_packets
		 * event, recently created, due to go to the host.
		 */
        status = TRAhcit_Dispatch_Pending_Event();
        {
			if(status != 0) 
			{
                status = TRAhcit_Dispatch_Pending_Data();
			}
		#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
            if(1 == g_le_enable_observer_flag)
            {
                LEscan_Check_For_Pending_Advertising_Reports();
            }
		#endif

		#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
         // GF 08 July 2013
         //   if (le_mode) // TK: Modified to isolate LE from executing in BT Classic mode. To review.
            {
                LE_LL_Check_For_Pending_Connection_Completes();
            }
		#endif
        }
      
	    /*
	     * Finally, handle any incoming commands that may have arrived. This is placed here
	     * to give priority to upward (in the host direction) data, and events (which could
	     * possibly be num_completed_packets) to ensure high data transfer efficiency.
	     */  
	    status = HC_Command_Dispatcher();

		LE_LL_Check_Timers();

    } /* end for */

    /*
     * Handle sleep opportunities or HCI reset if one pending
     */
    if(_BTms_pending_hc_reset == 0)
    {
        _BTms_Handle_HC_Sleep_If_Reqd();
    }
    else
    {
        _BTms_Handle_HC_Reset_If_Reqd();
    }
    
    /*
     * Dump one random number
     */
    (void)SYSrand_Get_Rand();

    return status;
}


/*****************************************************************************
 * BTms_Request_HC_Reset
 *
 * Request that scheduler completes a full reset e.g. HCI_Reset command
 ****************************************************************************/
void BTms_Request_HC_Reset(void)
{
    _BTms_pending_hc_reset = 1;
#if defined(SYS_HAL_OS_EVENT_DRIVEN_HOST_CONTROLLER)
    BTms_OS_Post(BTMS_OS_EVENT_RESET_PENDING);
#endif
}

/*****************************************************************************
 * _BTms_Handle_HC_Reset_If_Reqd
 *
 * Perform a full reset e.g. HCI_Reset command
 *
 * Reset will be completed if not sending data/event to Host 
 * If busy in HCI transmit then defer the reset.
 ****************************************************************************/
static t_error _BTms_Handle_HC_Reset_If_Reqd(void)
{
	
	if(PATCH_FUN[_BTMS_HANDLE_HC_RESET_IF_REQD_ID]){
         
         return ((t_error (*)(void))PATCH_FUN[_BTMS_HANDLE_HC_RESET_IF_REQD_ID])();
    }

    t_cmd_complete_event cmd_complete_info;
    
    /* 
     * Reset if requested  and not currently sending Data/Event to Host
     * If busy sending data then will check on next pass of the mini-scheduler
     */
    if(!TRAhcit_Is_Tx_Busy())
    {
        LSLCirq_Disable_And_Clear_Tabasco_Intr();

        /*
         * Full initialisation of device
         */
        BT_Initialise();

#if defined(TERASIC)
        _SYS_Main_Verify_Tabasco_Revision_ID();
#endif
        /*
         * Now that HCI_Reset has been performed
         * Instruct the Host that the Host Controller is now ready
         */
        cmd_complete_info.opcode = HCI_RESET;
        cmd_complete_info.status = NO_ERROR;
        HCeg_Command_Complete_Event(&cmd_complete_info);

        /*
         * Cancel request
         */
        _BTms_pending_hc_reset = 0;
        
        return NO_ERROR;
    }
    else
    {
        return UNSPECIFIED_ERROR;
    }
}

/*****************************************************************************
 * _BTms_Handle_HC_Sleep_If_Reqd
 *
 * Handles opportunities to sleep processor to various low power modes.
 ****************************************************************************/
static t_error _BTms_Handle_HC_Sleep_If_Reqd(void)
{
    return NO_ERROR;
}



