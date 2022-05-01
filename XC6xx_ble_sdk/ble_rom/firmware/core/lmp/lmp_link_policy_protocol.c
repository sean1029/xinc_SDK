/**************************************************************************
 * MODULE NAME:    lmp_link_policy_protocol.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    LMP Link Policy Engine
 * MAINTAINER:     Gary Fleming
 * DATE:           09-Feb-2000
 *
 * SOURCE CONTROL: $Id: lmp_link_policy_protocol.c,v 1.346 2014/01/27 19:14:35 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *
 * NOTES:
 * This module encompasses the link policy protocol which determines the link
 * Link Policy mode of a connection.
 * 
 * It encompasses all protocol interactions with the peer and the upper layer
 * to enter/exit/modify HOLD SNIFF or PARK mode
 *
 **************************************************************************/
#include "sys_config.h"

#include "lc_interface.h"

#include "lmp_const.h"
#include "lmp_types.h"
#include "lmp_features.h"
#include "lmp_cmd_disp.h"
#include "lmp_encode_pdu.h"
#include "lmp_link_policy.h"
#include "lmp_link_policy_protocol.h"
#include "lmp_acl_connection.h"
#include "lmp_acl_container.h"
#include "lmp_sco_container.h"
#include "lmp_timer.h"
#include "lmp_config.h"
#include "lmp_link_qos.h"
#include "lmp_utils.h"
#include "lmp_debug.h"
#include "lmp_ch.h"
#include "tra_queue.h"
#include "bt_mini_sched.h"
#include "hc_const.h"
#include "hc_event_gen.h"
#include "sys_mmi.h"
#include "sys_irq.h"

/*
 * LMP policy module, specifically to support HOLD
 */
#if (PRH_BS_CFG_SYS_LMP_HOLD_SUPPORTED==1)
static void LMpolicy_Enter_Hold_Mode(t_lmp_link* p_link,t_slots hold_time);
static void LMpolicy_Execute_Hold(t_lmp_link* p_link);
static void _Send_LMP_Hold(t_lmp_link* p_link,t_slots hold_time,u_int8 opcode);
#endif

/*
 * LMP policy module, specifically to support SNIFF
 */
#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUPPORTED==1)
static void LMpolicy_LM_Return_From_Sniff(
    t_lmp_link* p_link, t_error reason);
static u_int16 LMpolicy_Master_Derive_D_Sniff(t_lmp_link* p_link);
static t_error LMpolicy_Derive_Sniff_Parameters(
    t_lmp_link* p_link, u_int16 max_interval, u_int16 min_interval,
    u_int16 attempt, u_int16 timeout);
static u_int8 LMpolicy_Extract_Sniff_Parameters(
    t_lmp_link* p_link,u_int8* p_payload);
static void _Send_LMP_Sniff_Req(t_lmp_link* p_link);
u_int8 LMpolicy_Determine_D_Sniff_In_Scatternet(t_lmp_link* p_link);
#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUBRATING_SUPPORTED==1)
void _Send_HC_Sniff_Subrating_Event(t_lmp_link* p_link,
         u_int16 max_tx_latency, u_int16 max_rx_latency, u_int16 min_remote_timeout,
         u_int16 min_local_timeout, t_error status);
void _Send_LMP_Sniff_Subrating_Req(t_lmp_link* p_link);
void _Send_LMP_Sniff_Subrating_Res(t_lmp_link* p_link);
u_int32 _Derive_Sniff_Subrating_Instant(t_lmp_link* p_link);
#endif
#endif

#define MIN_T_SNIFF_IN_SCATTERNET 18

/*
 * LMP policy module, specifically to support PARK
 */
#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
extern t_link_entry link_container[PRH_MAX_ACL_LINKS];
static t_error LMpolicy_Extract_Park_Parameters(t_role role, 
    t_lm_park_info* p_park_info, t_p_pdu p_payload, u_int8 opcode);
static t_error LMpolicy_Derive_Park_Parameters(t_role role, 
    t_lm_park_info* p_park_info, u_int16 max_beacon_interval, 
    u_int16 min_beacon_interval, t_deviceIndex device_index);

/*
 * LC invoked functions, registered via Callbacks
 */
static t_error LMpolicy_Slave_Init_Unpark(t_LC_Event_Info* p_lc_event_info);
static t_error LMpolicy_Access_Window_Complete(t_LC_Event_Info* p_lc_event_info);

static t_error _LMpolicy_Execute_Park_Request(
    t_lmp_link* p_link,u_int8 automatic, t_role role);
static void _LMpolicy_Enter_Park_Mode(t_lmp_link* p_link);
static void _LMpolicy_Send_LMP_Park_Req(t_lmp_link* p_link, t_lm_park_info* p_park_info);

/*
 * Containers and accessor functions for managing  parked/unparked links
 */
#define PRH_BS_CFG_MAX_PARK_LINKS  7

#define PARK_NON_AUTO     0
#define PARK_AUTO         1


/*
 * t_lm_park_entry_info  structure to hold information for automatic parking
 */
typedef struct s_lm_park_entry_info{
    t_lmp_link *p_link;
    u_int8     automatic;           /* Automatic unpark and park            */
    u_int8     initiator;           /* Master or Slave                      */
 } t_lm_park_entry_info; 

STATIC t_lm_park_entry_info _LMpol_Pending_Unpark_Links[PRH_BS_CFG_MAX_PARK_LINKS];
STATIC t_lm_park_entry_info _LMpol_Pending_Park_Links[PRH_BS_CFG_MAX_PARK_LINKS];

static t_lm_park_entry_info* _LMpolicy_Get_Next_Unpark_Link(void);
static t_lm_park_entry_info* _LMpolicy_Get_Next_Park_Link(void);

static void _LMpolicy_Insert_UnPark_Link(
     t_lmp_link* p_link, u_int8 automatic, t_role role);
static void _LMpolicy_Insert_Park_Link(
     t_lmp_link* p_link, u_int8 automatic, t_role role);
static void _LMpolicy_Remove_Park_Link(const t_lmp_link* p_link);
static void _LMpolicy_Remove_UnPark_Link(const t_lmp_link* p_link);
#if (PRH_BS_DEV_SLAVE_FORCE_STOP_AUTOPARK_SUPPORTED==1)
static void _LMpolicy_Unforce_Stop_Autopark(t_lmp_link* p_link);
#endif
#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
static u_int8 LMpolicy_Are_Links_Being_Parked_UnParked(void);
static void _LMpolicy_LMP_Park_Accepted_Ack_Timeout(t_lmp_link* p_link);
#endif

u_int8 LMpolicy_Determine_D_Sniff_In_Scatternet(t_lmp_link* p_link);

/*
 * AR and PM address pools, and access methods
 */
STATIC u_int32 LMpolicy_Ar_Addr_Pool[8];
STATIC u_int32 LMpolicy_Pm_Addr_Pool[8];
static u_int8 LMpolicy_Alloc_Pm_Addr(void);
static u_int8 LMpolicy_Alloc_Ar_Addr(void);
static void LMpolicy_Free_Pm_Addr(u_int8 pm_addr);
static void LMpolicy_Free_Ar_Addr(u_int8 ar_addr);

/* 
 * p_pending_unpark_link is used to identify the next link to be unparked.
 * It is set by a HCI initiated Exit Park or an incoming LMP_UNPARK PDU.
 */
STATIC t_lmp_link *p_pending_unpark_link;
#endif

/*
 * LMP policy module, general
 */
static void _Send_HC_Mode_Change_Event(
    t_lmp_link* p_link, t_slots max_interval, t_error status);


/**************************************************************************
 *
 *  Function : LMpolicy_LM_Role_Discovery
 *
 *  Description :
 *  Returns the Role for given ACL link identified using the P_link.
 * 
 *  Note :- This does not apply to SCO links.
 **************************************************************************/
void LMpolicy_LM_Role_Discovery(
    t_lmp_link *p_link, t_cmd_complete_event *cmd_complete_info)
{
    cmd_complete_info->mode = (u_int8)p_link->role;
    cmd_complete_info->handle = p_link->handle;
}

/**************************************************************************
 *
 *  Function : LMpolicy_Initialise
 *
 *  Description :
 *  Register the handler in the LM for the park events from
 *  the LC. 
 *  Initialise the arrays used for AR_ADDR & PM_ADDR
 *  allocation and de-allocation.
 *  Initialise the pending lists used to manage park/unparks.
 *
 **************************************************************************/
void LMpolicy_Initialise(void)
{
#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
    
    int i;
    LC_Register_Event_Handler(LC_EVENT_PARK_COMPLETE,
                                (void*)LMpolicy_Enter_Park_Mode_Complete);
    LC_Register_Event_Handler(LC_EVENT_UNPARK_COMPLETE,
                                (void*)LMpolicy_Exit_Park_Mode_Complete);
    LC_Register_Event_Handler(LC_EVENT_PARK_SLAVE_INIT_UNPARK_REQUEST,
                                (void*)LMpolicy_Slave_Init_Unpark);
    LC_Register_Event_Handler(LC_EVENT_ACCESS_WINDOW_COMPLETE,
                                (void*)LMpolicy_Access_Window_Complete);


    for (i=0; i<=7; i++)
    {    
#if (PRH_BS_DEV_USE_ODD_AR_ADDR_ONLY==1)                                                                    
     /*                                                                    
      * The device is configured only to allocat ODD AR Addresses                                                                    
      * due to LC restrictions.                                                                     
      * Can not use even numbered ar_addrs.                                                                     
      * Master can not receive 2nd half ID packets in the 
      * Access Window.
      */                                                                    
     LMpolicy_Ar_Addr_Pool[i] = 0xAAAAAAAA; /* 0xFFFFFFFF */                                                                     
#elif (PRH_BS_DEV_ALIGN_PARK_BEACON_WITH_SYNCHRONOUS_LINKS==1)
     /*
      * The device is configured only to allocate AR
      * addresses that will allow the slave to unpark
      * if there are 4 eSCOs active in a park beacon
      * which has been aligned with these eSCOS
      */
     if(i%3 == 0)
         LMpolicy_Ar_Addr_Pool[i] = 0x1E01E01E;
     else if (i%3 == 1)
         LMpolicy_Ar_Addr_Pool[i] = 0xE01E01E0;
     else
         LMpolicy_Ar_Addr_Pool[i] = 0x01E01E01;
#else
     LMpolicy_Ar_Addr_Pool[i] = 0xFFFFFFFF;
#endif

      LMpolicy_Pm_Addr_Pool[i] = 0xFFFFFFFF;
    }

    /* Always reserve AR_ADDR - Zero : As this is not used. */
    LMpolicy_Ar_Addr_Pool[0] &= ~1;

    /* Reserve PM_ADDR - Zero : As this has a unique meaning. */
    LMpolicy_Pm_Addr_Pool[0] = 0xFFFFFFFE;

    /*
     * Initialise the pending park/unpark lists to empty
     */
    for (i=0; i<PRH_BS_CFG_MAX_PARK_LINKS; i++)
    {
         _LMpol_Pending_Park_Links[i].initiator = 0;
         _LMpol_Pending_Park_Links[i].automatic = 0;
         _LMpol_Pending_Park_Links[i].p_link = 0;

         _LMpol_Pending_Unpark_Links[i].initiator = 0;
         _LMpol_Pending_Unpark_Links[i].automatic = 0;
         _LMpol_Pending_Unpark_Links[i].p_link = 0;
    }
    
    p_pending_unpark_link = 0;

#endif
}

#ifndef PRH_BS_CFG_MAX_MASTER_UNPARK_ATTEMPTS
#define PRH_BS_CFG_MAX_MASTER_UNPARK_ATTEMPTS 1
#endif

#ifndef PRH_BS_CFG_MIN_MASTER_PARK_BEACON_INTERVAL
#define PRH_BS_CFG_MIN_MASTER_PARK_BEACON_INTERVAL 128
#endif

#if (PRH_BS_CFG_SYS_LMP_HOLD_SUPPORTED==1)
/*************************************************************
 ************  HOLD MODE FUNCTIONALITY  **********************
 *************************************************************
 * 
 * Hold Mode
 * ------------
 * There are two types of Hold [ Forced and Negotiated ]
 * The Forced mechanism is rooted in knowledge gained from
 * previously placing the slave in Hold Mode and assumes that
 * the Host (or Host Controller) in the master has knowledge of 
 * the Hold Time used for a previous hold with the same slave.
 *
 * One of the assumptions to make in this implementation is that
 * a previous Hold time is only relevant to an instance of a link. 
 *
 * So first hold on a link is negotiated and all subsequent Holds
 * can be forced.
 *
 *************************************************************/
void LMpolicy_LM_Return_From_Hold(t_lmp_link* p_link)
{
    //Is no need to check num SYN connections, as will abort wakeup on the
    //USLCr2p_Request due to DL_Get_Piconet_Clock_Last_Access. Adding a 
    //check on the num SYN connections here can cause issue like bug #2665.
    //if (LMscoctr_Get_Number_SYN_Connections() == 0)
    {
        SYSmmi_Display_Event(eSYSmmi_LC_Connection_Event);
        LC_Wakeup(p_link->device_index, 0,
            (t_clock)((p_link->pol_info.hold_info.hold_instant
                       + p_link->pol_info.hold_info.hold_time)<<1));
    }

    p_link->pol_info.hold_info.hold_timeout_index = 0;
    p_link->state = LMP_ACTIVE;

    LMch_Enable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_HOLD);
    p_link->poll_position = 0;
    g_LM_config_info.num_acl_link_on_hold--;
    LMpol_Decr_Links_In_Low_Power();

	LMpol_Adjust_Wakeup_Slot_Time(p_link,0xFFFFFFFF);

    /*
     * Returning from hold may allow the adjusting
     * of ACL packet types on this link.
     */
    LMpol_Adjust_ACL_Packet_Types_On_Link(p_link, LMcontxt_LEAVING_HOLD);
    LMqos_Adjust_Supervision_Timeout_On_Link(p_link,LMcontxt_LEAVING_HOLD,0);

    LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_SUSPEND_PAGE_SCANS |
                                   PRH_DEV_STATE_SUSPEND_INQUIRY_SCANS |
                                   PRH_DEV_STATE_SUSPEND_PERIODIC_INQUIRY);
    
    /* 
     * Return the device to the state it held prior to going on hold 
     */
    _Send_HC_Mode_Change_Event(p_link, 0, NO_ERROR);

    // GF 14 May - Added to Handle HCI disconnet while a link is on hold.

	if (p_link->disconnect_req_reason)
	{
		LMconnection_LM_Disconnect(p_link,p_link->disconnect_req_reason);
		p_link->disconnect_req_reason = NO_ERROR;
	}
    else if (mPol_Check_Automatic_Rehold_Mode(p_link->link_policy_mode))
    {
        LMpolicy_LM_Hold_Mode(p_link, 
            (u_int16)p_link->pol_info.hold_info.hold_time,
            (u_int16)p_link->pol_info.hold_info.hold_time);
    }
}


/**************************************************************************
 *
 *  Function : LMpolicy_LM_Hold
 *
 *  Description :
 *  Request from the service interface to place a link in Hold.
 *
 *  Step 1 :- The limits of the hold (max,min) are first stored 
 *            in the poliy structure.
 *
 *  Step 2 :- If MASTER and previously placed the slave in Hold 
 *            then a forced Hold can be used
 *                 if max > previous_max_hold > min.
 *
 **************************************************************************/
t_error LMpolicy_LM_Hold_Mode(
      t_lmp_link* p_link, u_int16 max_interval, u_int16 min_interval)
{
    u_int16  max_previous_hold;
    t_lm_hold_info* p_link_hold = &p_link->pol_info.hold_info;

    if (!mFeat_Check_Hold_Mode(p_link->remote_features))
        return UNSUPPORTED_REMOTE_FEATURE;

    if (p_link->state != LMP_ACTIVE)
        return COMMAND_DISALLOWED;

    if( p_link->operation_pending == LMP_DETACH)
        return COMMAND_DISALLOWED;

    if (!mPol_Check_Hold_Mode(p_link->link_policy_mode))
        return COMMAND_DISALLOWED;

    if (p_link->supervision_timeout != 0)
    {
        /*
         * The timeout period, supervisionTO, is negotiated by the Link Manager.
         * T_sniff/hold_time shall not exceed supervisionTO * 0.999.
         */
        u_int16 max_allowed_interval = ((p_link->supervision_timeout)-((p_link->supervision_timeout)>>10));

        if (max_allowed_interval < max_interval)
        {
            if (p_link->role == SLAVE)
                return INVALID_HCI_PARAMETERS;

#if (PRH_BS_DEV_SUPERVISION_TIMEOUT_NEGOTIATION_SUPPORTED==0)
            if (max_allowed_interval < min_interval)
                return INVALID_HCI_PARAMETERS;
            max_interval = max_allowed_interval;
#endif
        }
    }

    /* 
     * Store the hold limits 
     */
    p_link_hold->max_hold_time = max_interval;
    p_link_hold->min_hold_time = min_interval;

    /* Set up the Transaction Id */
    p_link->current_proc_tid_role = p_link->role;

    max_previous_hold = p_link_hold->max_previous_hold;

    LMqos_Adjust_Supervision_Timeout_On_Link(p_link,LMcontxt_ENTERING_HOLD,
        max_interval);

    if ((min_interval <= max_previous_hold) && 
         mPol_Check_Hold_Occured(p_link->previous_modes) ) /* Forced Hold Possible */
    {  
        if(max_interval <= max_previous_hold)
        {
            p_link_hold->hold_time = max_interval;
        }
        else
        {
            p_link_hold->hold_time = max_previous_hold;
        }
        _Send_LMP_Hold(p_link, p_link_hold->hold_time, LMP_HOLD);
		if (MASTER == p_link->role)
		{
			LMpolicy_Enter_Hold_Mode(p_link, p_link_hold->hold_instant);
		}
		/*
		 *	else 
		 *    Slave must wait for LMP_Hold from the master
		 */

    }
    else   /* Negotiated Hold  - as  */
    {
        p_link_hold->hold_time = max_interval;
        _Send_LMP_Hold(p_link, p_link_hold->hold_time, LMP_HOLD_REQ);
		LMpolicy_Enter_Hold_Mode(p_link, p_link_hold->hold_instant);

    }

    return NO_ERROR;     
}

/**************************************************************************
 *
 *  Function : LMpolicy_LMP_Hold
 *
 *  Description :
 *  Request from the peer to force a link in Hold.
 *
 **************************************************************************/
t_error LMpolicy_LMP_Hold(t_lmp_link* p_link,t_p_pdu p_payload)
{
    u_int16 proposed_hold_time = LMutils_Get_Uint16(p_payload);
    u_int32 hold_instant = LMutils_Get_Uint32(p_payload+2);
    t_lm_hold_info* p_link_hold = &p_link->pol_info.hold_info;

    /*  Erratum 1119
     *  The local device may also be forced into hold mode (regardless of
     *  whether the local device is master or slave) by the remote device 
     *  regardless of the Link_Policy_Settings.
     *  The forcing of hold mode can however only be done once the connection
     *  has already been placed into the corresponding mode through a
     *  previous LMP_hold_req
     */

    if (!mPol_Check_Hold_Occured(p_link->previous_modes))
    {    
        return LMP_PDU_NOT_ALLOWED;
    } 
    else if ( proposed_hold_time > p_link_hold->max_previous_hold)
    {
        return INVALID_LMP_PARAMETERS;
    }

    else if (proposed_hold_time < PRH_DEFAULT_MIN_HOLD_INTERVAL)
    {
		return UNSUPPORTED_REMOTE_FEATURE;
    }

    if (p_link_hold->num_hold_negotiations == 0)
    {
       /* The peer is the initiator of the LMP Hold */
       p_link->current_proc_tid_role = p_link->role ^ 0x01;  
    }
    else
    {
       /* The Transaction Id has been set up previously */  
    }    

    if ((p_link->role == SLAVE))
    {
        /*******************************************************************
         * Note a slave has to always accept a forced hold from the master.
         *
         * Slave Accepts the Hold and Goes into Hold informing the Host & LC 
         * Sets a timer for the hold mode    
         * If the Hold Instance + Hold Time is not passed the device goes into
         * hold for the remainder of the hold time.
         *
         * Determine the activity to be performed in hold mode 
         *******************************************************************/
        if ( hold_instant < (LC_Get_Piconet_Clock(p_link->device_index)>>1) )
        {
           /* Ensure that the number of negotiations is reset on entering hold */
           p_link_hold->num_hold_negotiations = 0;
           return NO_ERROR;
        }
        else
        {  
            p_link_hold->hold_instant = hold_instant;
            p_link_hold->hold_time = proposed_hold_time;
            LMpolicy_Enter_Hold_Mode(p_link, hold_instant);
        }
     }
     else if(p_link->role == MASTER)
     {
        if ( hold_instant < ((LC_Get_Native_Clock()>>1) + (6 * p_link->poll_interval)) )
        {
            /* Hold Instant is too close */
            p_link_hold->hold_time = proposed_hold_time;
            _Send_LMP_Hold(p_link,p_link_hold->hold_time,LMP_HOLD);
/* #1523 */
            LMpolicy_Enter_Hold_Mode(p_link,p_link_hold->hold_instant);
            return NO_ERROR;
        } 
        else
        {  
            if((p_link->supervision_timeout != 0) && (p_link->supervision_timeout <= proposed_hold_time))
            {
                p_link_hold->hold_time = p_link->supervision_timeout - 100;      
            }
            else
            {
                p_link_hold->hold_time = proposed_hold_time;
            }

            {   /* Send an LMP_HOLD using the existing hold instance */
                t_lmp_pdu_info  pdu_info;

                pdu_info.tid_role = p_link->current_proc_tid_role;
                pdu_info.timeout = p_link_hold->hold_time;      
                pdu_info.opcode = LMP_HOLD;
                pdu_info.instant = p_link_hold->hold_instant = hold_instant;
                LM_Encode_LMP_PDU(p_link->device_index, &pdu_info);
                p_link_hold->num_hold_negotiations++;
            }
            LMpolicy_Enter_Hold_Mode(p_link,hold_instant );
        }
   }
    return NO_ERROR;
}

/**************************************************************************
 *
 *  Function : LMpolicy_LMP_Hold_Req
 *
 *  Description :
 *  Request from the peer to negotiate a link into Hold.
 *
 **************************************************************************/
t_error LMpolicy_LMP_Hold_Req(t_lmp_link* p_link, t_p_pdu p_payload)
{   
    t_error status = NO_ERROR;
    u_int16 proposed_hold_time = LMutils_Get_Uint16(p_payload);
    u_int32 hold_instant = LMutils_Get_Uint32(p_payload+2);
    t_lm_hold_info* p_link_hold = &p_link->pol_info.hold_info;

    if((p_link->tid_role_last_rcvd_pkt != p_link->role) && (!mPol_Check_Hold_Mode(p_link->link_policy_mode)))
    {
        status = LMP_PDU_NOT_ALLOWED;
    } 
    else if ((p_link->supervision_timeout != 0) && (proposed_hold_time > p_link->supervision_timeout))
    {
        status = INVALID_LMP_PARAMETERS;
    }
    else if ( hold_instant < (LC_Get_Piconet_Clock(p_link->device_index)>>1) )
    {
        status = INSTANT_PASSED;
    }  

    if(status != NO_ERROR)
    {
        p_link_hold->min_hold_time = PRH_DEFAULT_MIN_HOLD_INTERVAL;
        p_link_hold->max_hold_time = 0xffff;
        p_link->current_proc_tid_role = p_link->tid_role_last_rcvd_pkt;
    }

    /******************************************************************************
     * The actions performed in this function are different for the following cases
     *     1/ The First HOLD_REQ in a negotiation. 
     *     2/ A subsequent HOLD_REQ in a negotiation.
     *     3/ Initiator of the negotiation procedure (thus have limits from the HCI)
     *     4/ Non Initiator of the negiciation procedure (thus have no limits from the HCI).
     *     5/ Master OR Slave.
     *****************************************************************************/

    else if (p_link_hold->num_hold_negotiations == 0)
    {
       /* The peer is the initiator of the LMP Hold */
       p_link->current_proc_tid_role = p_link->role ^ 0x01;
       status = NO_ERROR;
    }
    else
    {
       /* The Transaction Id has been set up previously */  
       /* #1525
        * Check that the T-Ids of the incoming PDU matchs that of the
        *  current transaction. If not respond with 
        * LMP not accepted TRANSACTION COLLISION
        */
        if (p_link->current_proc_tid_role != p_link->tid_role_last_rcvd_pkt)
        {
            status = LMP_ERROR_TRANSACTION_COLLISION;
        }
        else
        {
            status = NO_ERROR;
        }
    }    

#if (PRH_BS_CFG_SYS_NEGOTIATED_HOLD_SUPPORTED==1)
    if ((status == NO_ERROR) && 
        (p_link_hold->max_hold_time >= proposed_hold_time) && 
        (p_link_hold->min_hold_time <= proposed_hold_time))
    {
#endif 
    {
       p_link_hold->num_hold_negotiations++;
        /*******************************************************************
         * Slave Accepts the Hold and Goes into Hold informing the Host & LC 
         * Sets a timer for the hold mode                       
         * Determine the activity to be performed in hold mode 
         *******************************************************************/
       LMch_Disable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_HOLD);
       p_link_hold->hold_time = proposed_hold_time;
       p_link_hold->hold_instant = hold_instant;
       {
           t_lmp_pdu_info  pdu_info;
		   t_lm_hold_info* p_link_hold = &p_link->pol_info.hold_info;
		    
           pdu_info.tid_role = p_link->current_proc_tid_role;
           pdu_info.opcode = LMP_ACCEPTED;
           pdu_info.return_opcode = LMP_HOLD_REQ ;
           LM_Encode_LMP_PDU(p_link->device_index,&pdu_info);
		   
		   // Added as fix for Bug 2357
		   if (p_link_hold->hold_time > p_link_hold->max_previous_hold)
		   {
			   p_link_hold->max_previous_hold = p_link_hold->hold_time;
		   }
       }
       LMpolicy_Enter_Hold_Mode(p_link,p_link_hold->hold_instant);
       p_link_hold->num_hold_negotiations = 0;
    }
#if (PRH_BS_CFG_SYS_NEGOTIATED_HOLD_SUPPORTED==1)
    }
    else if (status == NO_ERROR)
    {
        /* Support of Maximum 5 Hold_Reqs during negotiation */
        if (p_link_hold->num_hold_negotiations < 5 )
        {
           if (proposed_hold_time > p_link_hold->max_hold_time) 
           {     
              _Send_LMP_Hold(p_link,p_link_hold->max_hold_time,LMP_HOLD_REQ);
              p_link_hold->hold_time = p_link_hold->max_hold_time;
           }
           else if (proposed_hold_time < p_link_hold->min_hold_time) 
           {
              _Send_LMP_Hold(p_link,p_link_hold->min_hold_time,LMP_HOLD_REQ);   
              p_link_hold->hold_time = p_link_hold->min_hold_time;  
           }    
           p_link_hold->num_hold_negotiations++;
           LMpolicy_Enter_Hold_Mode(p_link,p_link_hold->hold_instant);
           status = NO_ERROR;
        }
        else
        {
            LMpol_Adjust_ACL_Packet_Types_On_Link(p_link, LMcontxt_LEAVING_HOLD);
            LMqos_Adjust_Supervision_Timeout_On_Link(p_link,LMcontxt_LEAVING_HOLD,0);
            p_link_hold->num_hold_negotiations = 0;
            p_link_hold->min_hold_time = PRH_DEFAULT_MIN_HOLD_INTERVAL;
            p_link_hold->max_hold_time = 0xffff;
            status = UNSUPPORTED_PARAMETER_VALUE;
        }  
    }
#endif
    if (status != NO_ERROR)
    {
        LMtmr_Clear_Timer(p_link_hold->hold_timeout_index);
        p_link_hold->hold_timeout_index = 0;
        /*
         * Bugfix for 2117.
         *
         * If hold was initiated locally, inform
         * local host that host controller did
         * not go on hold.
         */
        if(status != LMP_ERROR_TRANSACTION_COLLISION)
        {
            if(p_link->current_proc_tid_role == p_link->role)
            {
                _Send_HC_Mode_Change_Event(p_link, 
                    0, status);
            }
        }
    }

    return status;
}

/**************************************************************************
 *
 *  Function : _Send_LMP_Hold
 *
 *  Description :
 *  Sends an LMP_Hold OR LMP_Hold_Req PDU to the peer 
 *
 **************************************************************************/
static void _Send_LMP_Hold(t_lmp_link* p_link, t_slots hold_time, u_int8 opcode)
{
    t_lmp_pdu_info  pdu_info;
    t_lm_hold_info* p_link_hold = &p_link->pol_info.hold_info;
    u_int32 current_slot;

    if (p_link->role == SLAVE)
    {
        current_slot = ((LC_Get_Piconet_Clock(p_link->device_index)>>2)*2);
    }
    else
    {
        current_slot = ((LC_Get_Native_Clock()>>2)*2);
    }

    p_link_hold->num_hold_negotiations++;
    pdu_info.tid_role = p_link->current_proc_tid_role;
    pdu_info.timeout = hold_time;      
    pdu_info.opcode = opcode;
    LMch_Disable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_HOLD);
    if ((PRH_HOLD_INSTANT_DELAY * p_link->poll_interval) > PRH_MIN_HOLD_INSTANT_DELAY)
    {
        pdu_info.instant = p_link_hold->hold_instant = current_slot + 
            (PRH_HOLD_INSTANT_DELAY * p_link->poll_interval);
    }
    else
    {
        pdu_info.instant = p_link_hold->hold_instant = current_slot + PRH_MIN_HOLD_INSTANT_DELAY;
    }
    LM_Encode_LMP_PDU(p_link->device_index, &pdu_info);
}

/**************************************************************************
 * Function : LMpolicy_Enter_Hold_Mode
 *
 * Description 
 * Sets the hold timer & a timer for entering hold. Send the event to the 
 * higher layers.
 *************************************************************************/
static void LMpolicy_Enter_Hold_Mode(t_lmp_link* p_link, u_int32 hold_instant)
{
    t_lm_hold_info* p_link_hold = &p_link->pol_info.hold_info;

    if  (p_link_hold->hold_timeout_index)
    {
        LMtmr_Clear_Timer(p_link_hold->hold_timeout_index);
    }
    p_link_hold->hold_timeout_index = LMtmr_Set_Timer(
        (hold_instant-(LC_Get_Piconet_Clock(p_link->device_index)>>1)-2), 
        &LMpolicy_Execute_Hold, p_link, 1);

    /*
     * Entering hold may require the adjusting of
     * ACL packet types on this link.
     */
    LMpol_Adjust_ACL_Packet_Types_On_Link(p_link, LMcontxt_ENTERING_HOLD);

#if 0 // Removed as fix for Bug 2357
    if (p_link_hold->hold_time > p_link_hold->max_previous_hold)
    {
        p_link_hold->max_previous_hold = p_link_hold->hold_time;
    }
#endif
}

/**************************************************************************
 * Function : LMpolicy_Execute_Hold
 *
 * Description 
 * Executes the Hold procedures in the LC 
 **************************************************************************/
static void LMpolicy_Execute_Hold(t_lmp_link* p_link)
{
    t_lm_hold_info* p_link_hold = &p_link->pol_info.hold_info;
    t_slots hold_time_duration_slots;

    p_link_hold->num_hold_negotiations = 0;
    p_link_hold->max_hold_time = 0xffff;
    p_link_hold->min_hold_time = PRH_DEFAULT_MIN_HOLD_INTERVAL;
    if (p_link_hold->hold_time > 2)  
    {     
        p_link->state = LMP_HOLD_MODE;
        hold_time_duration_slots = p_link_hold->hold_time - 2;
        if (p_link->role == SLAVE)
        {
            /*
             * Adjust hold duration by r2p early wakeup slots
             * Note: the LC_Get_R2P_Early_Wakeup_Slots() is now
             * being called on entering hold and requires the 2nd 
             * parameter in ticks to take account of the actual hold duration
             */
            hold_time_duration_slots -= LC_Get_R2P_Early_Wakeup_Slots(
                p_link->device_index, hold_time_duration_slots<<1);
        }
        p_link_hold->hold_timeout_index = 
            LMtmr_Set_Timer(hold_time_duration_slots+2,
                &LMpolicy_LM_Return_From_Hold, p_link, 1);

#if (PRH_BS_DEV_EXTENDED_QOS_NOTIFICATIONS_SUPPORTED==1)
        /*
         * Clear potential LMP_QOS ACK pending for this device index
         */
        LMqos_Clear_LMP_QOS_Notification_Ack_Pending(p_link->device_index);
#endif

        _Send_HC_Mode_Change_Event(p_link, 
            (u_int16)p_link_hold->hold_time, NO_ERROR);
    }

    mPol_Set_Hold_Occured(p_link->previous_modes);

    g_LM_config_info.num_acl_link_on_hold++;
    g_LM_config_info.links_in_low_power++;

    if (p_link->role == SLAVE)
    {
        LMpol_Adjust_Wakeup_Slot_Time(p_link, ( p_link_hold->hold_time +  (LC_Get_Piconet_Clock(p_link->device_index)>>1)));
    }
    else
    {
        LMpol_Adjust_Wakeup_Slot_Time(p_link, ( p_link_hold->hold_time +  (LC_Get_Native_Clock()>>1)));
    }

    /*************************************************
     * If all ACL links are on hold then the device 
     * connection activity can be replaced by the hold 
     * mode activity.
     **************************************************/

    if (g_LM_config_info.num_acl_link_on_hold == g_LM_config_info.num_acl_links)
    {
        LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_SUSPEND_PAGE_SCANS    |
                                       PRH_DEV_STATE_SUSPEND_INQUIRY_SCANS | 
                                       PRH_DEV_STATE_SUSPEND_PERIODIC_INQUIRY);

        if(g_LM_config_info.hold_mode_activity & 0x00)
        {
            /* Maintain Current Power State */
            
        }
        if(g_LM_config_info.hold_mode_activity & 0x01)
        {
            /* Suspend Page Scan            */
            LMconfig_LM_Set_Device_State(PRH_DEV_STATE_SUSPEND_PAGE_SCANS);
            
        }
        if(g_LM_config_info.hold_mode_activity & 0x02)
        {
            /* Suspend Inquiry Scan         */
            LMconfig_LM_Set_Device_State(PRH_DEV_STATE_SUSPEND_INQUIRY_SCANS);
        }
        if(g_LM_config_info.hold_mode_activity & 0x04)
        { 
            /* Suspend Periodic Inquiries   */
            LMconfig_LM_Set_Device_State(PRH_DEV_STATE_SUSPEND_PERIODIC_INQUIRY);
        } 
    }
	LMpol_Sleep((u_int16)LMpol_Get_Interval_To_Next_Wakeup(), p_link, LC_Get_Piconet_Clock(p_link->device_index));
	
}
#endif

/**************************************************************************
 *  Function : LMpolicy_LMP_Accepted
 *
 *  DESCIPTION 
 *  This function is called on receipt of an LMP_Accepted PDU
 * 
 **************************************************************************/

t_error LMpolicy_LMP_Accepted(t_lmp_link* p_link,t_p_pdu p_payload)
{
    u_int8 opcode = (u_int8)(*p_payload);
    t_error status = NO_ERROR;

#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
    t_lm_park_info* p_park = &p_link->pol_info.park_info;
#endif

    switch(opcode)
    {
#if (PRH_BS_CFG_SYS_LMP_HOLD_SUPPORTED==1)
    case LMP_HOLD_REQ :
		{
			t_lm_hold_info* p_link_hold = &p_link->pol_info.hold_info;
			
			// Added as fix for Bug 2357
			if (p_link_hold->hold_time > p_link_hold->max_previous_hold)
			{
				p_link_hold->max_previous_hold = p_link_hold->hold_time;
			}
		}
        break;
#endif

#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUPPORTED==1)
    case LMP_SNIFF_REQ :
        LMpolicy_Enter_Sniff_Mode(p_link->device_index);
        break;

    case LMP_UNSNIFF_REQ :
        if (p_link->state & LMP_SNIFF_MODE)
        {
            LMpolicy_LM_Return_From_Sniff(p_link, NO_ERROR);
        }
        break;
#endif
#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)

    case LMP_PARK_REQ :
        if (LMconfig_LM_Check_Device_State(PRH_DEV_STATE_PARK_PENDING))
        {
#if (PRH_BS_DEV_MASTER_PARK_REQ_ACK_TIMER==1)
            LMtmr_Clear_Timer(p_link->pol_info.park_info.park_timeout_index);
            p_link->pol_info.park_info.park_timeout_index = 0;
#endif
            p_link->pol_info.park_info.park_timeout_index = LMtmr_Set_Timer(
                (t_slots)(p_link->poll_interval*6), 
                _LMpolicy_Enter_Park_Mode, p_link, 1);

            LMch_Disable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_PARK);
        }
        break;

    case LMP_UNPARK_PM_ADDR_REQ :
    case LMP_UNPARK_BD_ADDR_REQ : 
         
        /* Note there are two seperate scenarios to be considered 
         * Which scenario is pertinent can be determined from the unpark_pending field.
         *
         *   1/ LMP_Accepted arrives during the N-Poll period of the LC_UnPark
         *
         *   2/ LMP_Accepted arrives after the N-Poll period of the LC_UnPark
         */
         
        p_pending_unpark_link->poll_interval = DEFAULT_T_POLL;
        p_pending_unpark_link->poll_position = DEFAULT_T_POLL;

        LMtmr_Clear_Timer(p_pending_unpark_link->pol_info.park_info.unpark_timeout_index);
        p_pending_unpark_link->pol_info.park_info.unpark_timeout_index = 0;
        
		if (g_LM_config_info.park_info.unpark_pending == PRH_NO_UNPARK_PENDING)
        {    /* The LMP_Accept arrives after the LC_UnPark is complete */
            LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_UNPARK_PENDING);            
            p_pending_unpark_link = 0;
        }
        else
        {
            LC_Unpark_Commit();
        }

        g_LM_config_info.park_info.unpark_pending = PRH_NO_UNPARK_PENDING;
        LMpolicy_Free_Ar_Addr(p_park->ar_addr);
        LMpolicy_Free_Pm_Addr(p_park->pm_addr);
        if (g_LM_config_info.num_links_parked == 0)
            g_LM_config_info.park_info.park_state = LMP_BEACON_INACTIVE;

        /* If Automatic Unpark has been performed -
         * Then the Link needs to be automatically parked again..
         */

        if (mPol_Check_Automatic_Unpark_Mode(p_link->link_policy_mode) &&
            p_link->automatic_park_enabled)
        {

#if (PRH_BS_DEV_ACL_DRIVEN_AUTOPARK_SUPPORTED==1)
            if ((!BTq_Is_Queue_Empty(L2CAP_OUT_Q, p_link->device_index)) ||
                (!BTq_Is_Queue_Empty(LMP_OUT_Q, p_link->device_index)))
            {
                p_link->pol_info.park_info.next_park_timer =
                    BTtimer_Set_Slots(g_LM_config_info.T_bcast*2);
            }
            else
#endif
            {
                p_link->pol_info.park_info.next_park_timer =
                    BTtimer_Set_Slots(0);
            }

            _LMpolicy_Remove_UnPark_Link(p_link);
            _LMpolicy_Insert_Park_Link(p_link, PARK_AUTO, p_link->role);
        }
        else
        {   
            if (p_link->pol_info.park_info.mode_change)
            {
                _Send_HC_Mode_Change_Event(p_link,0,NO_ERROR);
#if 1  // GF 14 May - Added to Handle HCI disconnet of Parked link.
				// Link is automatically unparked - and then Disconnected.
				if (p_link->disconnect_req_reason)
				{
					LMconnection_LM_Disconnect(p_link,p_link->disconnect_req_reason);
					p_link->disconnect_req_reason = NO_ERROR;
				}
#endif
            }
            
            LMch_Enable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_PARK);
            /*
            * default for L2CAP data is STOP on init new link,
            * up to LMP to set flow to GO
            */

#if (BUILD_TYPE!=UNIT_TEST_BUILD) // TK BQB 09/01/2013 - LIH/BV-32: Tester waiting on ACL SEQN from IUT.
			{
                t_q_descr *p_qD = BTq_Enqueue(L2CAP_OUT_Q, p_link->device_index, 0);
                if (p_qD)
                {
                    p_qD->message_type = LCH_continue;
                    p_qD->callback = 0;
                    BTq_Commit(L2CAP_OUT_Q, p_link->device_index);
				}
            }
#endif

            LC_Start_Peer_Tx_L2CAP_Data(p_link->device_index);
            LC_Start_Local_Tx_L2CAP_Data(p_link->device_index);
            _LMpolicy_Remove_UnPark_Link(p_link);
        }
		
        LMtmr_Clear_Timer(p_link->pol_info.park_info.park_timeout_index);
		p_link->pol_info.park_info.park_timeout_index = 0;
#if 0  // GF 14 May - Added to Handle HCI disconnet of Parked link.
		// Link is automatically unparked - and then Disconnected.
		if (p_link->disconnect_req_reason)
		{
			LMconnection_LM_Disconnect(p_link,p_link->disconnect_req_reason);
			p_link->disconnect_req_reason = NO_ERROR;
		}
#endif
        break;
#endif

    default :
        status = UNSPECIFIED_ERROR;
    }
    return status;
}

/**************************************************************************
 *  Function : LMpolicy_LMP_Not_Accepted
 *
 *  DESCIPTION 
 *  This function is called on receipt of an LMP_Not_Accepted PDU
 * 
 **************************************************************************/
t_error LMpolicy_LMP_Not_Accepted(t_lmp_link* p_link, u_int16 opcode, t_error reason)
{
    t_error status = NO_ERROR;

    /* B 383 */
    switch(opcode)
    {
#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUPPORTED==1)
    case LMP_UNSNIFF_REQ :
        if (((p_link->role == MASTER) || (reason != LMP_ERROR_TRANSACTION_COLLISION))
            && (p_link->state & LMP_SNIFF_MODE))
        {
            LMpolicy_LM_Return_From_Sniff(p_link, reason);
        }
        break;
#endif

    case LMP_SNIFF_REQ :
        LMch_Enable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_SNIFF);
        if(reason != LMP_ERROR_TRANSACTION_COLLISION)
        {
            _Send_HC_Mode_Change_Event(p_link,0,reason);

#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUPPORTED==1)
            p_link->pol_info.sniff_info.num_sniff_negotiations = 0;
#endif
            /*
             * Link may require adjustment of ACL packet types or supervisionTO
             * when receiving LMP_not_accepted for LMP_sniff_req.
             */
            LMpol_Adjust_ACL_Packet_Types_On_Link(p_link, LMcontxt_LEAVING_SNIFF);
            LMqos_Adjust_Supervision_Timeout_On_Link(p_link, LMcontxt_LEAVING_SNIFF, 0);
        }
        break;

#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUBRATING_SUPPORTED==1)
    case LMP_SNIFF_SUBRATING_REQ:
		if(reason != LMP_ERROR_TRANSACTION_COLLISION)
		{
        _Send_HC_Sniff_Subrating_Event(p_link, 
            p_link->pol_info.sniff_info.max_latency, 
            0 /*rx latency undefined as sniff subrate not accepted*/,
            p_link->pol_info.sniff_info.min_remote_timeout, 
            p_link->pol_info.sniff_info.min_local_timeout,
            reason);
		}
        break;
#endif
    case LMP_HOLD_REQ :
        LMch_Enable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_HOLD);
        if(reason != LMP_ERROR_TRANSACTION_COLLISION)
        {
#if (PRH_BS_CFG_SYS_LMP_HOLD_SUPPORTED==1)
            if (p_link->pol_info.hold_info.hold_timeout_index)
            {
                LMtmr_Clear_Timer(p_link->pol_info.hold_info.hold_timeout_index);
                p_link->pol_info.hold_info.hold_timeout_index = 0;
            }

            p_link->pol_info.hold_info.num_hold_negotiations = 0;

            /*
             * Receiving LMP_not_accepted for LMP_hold_req
             * resets the minimum and maximum hold time.
             */
            p_link->pol_info.hold_info.max_hold_time = 0xffff;
            p_link->pol_info.hold_info.min_hold_time = PRH_DEFAULT_MIN_HOLD_INTERVAL;
#endif
            _Send_HC_Mode_Change_Event(p_link,0,reason);

            /*
             * Receiving LMP_not_accepted for LMP_hold_req
             * may require an adjustment
             * of the ACL packets used on the link.
             */
            LMpol_Adjust_ACL_Packet_Types_On_Link(p_link, LMcontxt_LEAVING_HOLD);
            LMqos_Adjust_Supervision_Timeout_On_Link(p_link,LMcontxt_LEAVING_HOLD,0);

        }
        break;

#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
    case LMP_UNPARK_PM_ADDR_REQ :
        if((p_link->state == LMP_PARK_MODE) &&
            ((p_link->pol_info.park_info.unpark_pending == PRH_SLAVE_INITIATED_UNPARK) ||
            (p_link->pol_info.park_info.unpark_pending == PRH_MASTER_INITIATED_UNPARK)))
        {
            /* preclude  an unparking */
            g_LM_config_info.park_info.unpark_pending = PRH_NO_UNPARK_PENDING;
            /* Reset the  broadcast Q to remove any pending LMP_UNPARK PDUs */
            BTq_Reset(0);
            /* Clear the  device state */
            LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_UNPARK_PENDING);
            p_link->state = LMP_ACTIVE;
            _Send_HC_Mode_Change_Event(p_link,0,UNSPECIFIED_ERROR);
        }
        break;

    case LMP_PARK_REQ :
        LMch_Enable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_PARK);
        /* 1/ Free The AR_Addr & PM_Addr
         * 2/ Send a Mode Change Event 
         */
        _LMpolicy_Remove_Park_Link(p_link);
        LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_PARK_PENDING);
        p_link->num_park_negotiations = 0;
        if (p_link->role == MASTER)
        {
            LMpolicy_Free_Ar_Addr(p_link->pol_info.park_info.ar_addr);
            LMpolicy_Free_Pm_Addr(p_link->pol_info.park_info.pm_addr);  
        }
        _Send_HC_Mode_Change_Event(p_link,0,reason);
        p_link->automatic_park_enabled =  0;

#if (PRH_BS_DEV_MASTER_PARK_REQ_ACK_TIMER==1)
        if (p_link->role == MASTER)
        {
            LMtmr_Clear_Timer(p_link->pol_info.park_info.park_timeout_index);
            p_link->pol_info.park_info.park_timeout_index = 0;
        }
#endif
       
        break;

#endif

    default :
        status = UNSPECIFIED_ERROR;
    }
    return status;
}

/**************************************************************************
 *  Function : _Send_HC_Mode_Change_Event
 *
 *  DESCIPTION 
 *  This function generates a Mode change event to the higher layer 
 * 
 **************************************************************************/
void _Send_HC_Mode_Change_Event(
     t_lmp_link* p_link, t_slots interval, t_error status)
{
    t_lm_event_info event_info;

    event_info.handle = p_link->handle;
    event_info.status = status;

    if ((p_link->state & LMP_ACTIVE) == LMP_ACTIVE)
    {
        LMch_Enable_L2CAP_Traffic(p_link,(LM_CH_L2CAP_EN_PARK|LM_CH_L2CAP_EN_SNIFF|LM_CH_L2CAP_EN_HOLD));
        event_info.mode = 0x00;
#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
        p_link->pol_info.park_info.unpark_pending = 0;
#endif
    }
    else if ((p_link->state & LMP_HOLD_MODE) == LMP_HOLD_MODE)
    {
        event_info.mode = 0x01;
    }
    else if ((p_link->state & LMP_SNIFF_MODE) == LMP_SNIFF_MODE)
    {
        event_info.mode = 0x02;
    }
#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
    else if ((p_link->state & LMP_PARK_MODE)== LMP_PARK_MODE)
    {
        if ((p_link->pol_info.park_info.unpark_pending) && (status == NO_ERROR))
            return;

        event_info.mode = 0x03;
        p_link->pol_info.park_info.unpark_pending = 0;
    }
#endif
    event_info.value16bit = interval;

    HCeg_Generate_Event(HCI_MODE_CHANGE_EVENT, &event_info);
} 

/*************************************************************
 ************  SNIFF MODE FUNCTIONALITY  **********************
 *************************************************************/
#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUPPORTED==1)

/**************************************************************************
 * Function :- LMpolicy_LM_Sniff_Mode
 *
 * Description
 * This function handles a sniff mode request from the higher 
 * layers. The min_interval is chosen initially with the remaining
 * parameters being provided by the higher layers. 
 * 
 * The offset is chosen as 6 * Poll Interval as this is sufficient
 * Time to allow the peer device to receive the Sniff Request 
 * irrespective of the interferance on the Air.
 *************************************************************************/
t_error LMpolicy_LM_Sniff_Mode(t_lmp_link* p_link,u_int16 max_interval,
                                  u_int16 min_interval,u_int16 attempt,
                                  u_int16 timeout)
{
    if (!mFeat_Check_Sniff_Mode(p_link->remote_features))
        return UNSUPPORTED_REMOTE_FEATURE;

    if (p_link->state != LMP_ACTIVE)
        return COMMAND_DISALLOWED;

    if(p_link->operation_pending == LMP_DETACH)
        return COMMAND_DISALLOWED;

    if (!mPol_Check_Sniff_Mode(p_link->link_policy_mode))
        return COMMAND_DISALLOWED;

    /*
     * Protect from transaction collision.
     */
    if (p_link->pol_info.sniff_info.num_sniff_negotiations)
    {
        return COMMAND_DISALLOWED;
    }

    /*
     * Round back max interval and min interval to even values of slots,
     * no matter what. This is to workaround brokenness of HCI layer
     * providing slots to a frame-based procedure.
     */
    min_interval &= 0xFFFE;
    max_interval &= 0xFFFE;

    p_link->pol_info.sniff_info.T_sniff_max = max_interval;
    p_link->pol_info.sniff_info.T_sniff_min = min_interval;

    if (p_link->supervision_timeout != 0)
    {
        /*
         * The timeout period, supervisionTO, is negotiated by the Link Manager.
         * T_sniff/hold_time shall not exceed supervisionTO * 0.999.
         */
        u_int16 max_allowed_interval = ((p_link->supervision_timeout)-((p_link->supervision_timeout)>>10));

        if (max_allowed_interval < max_interval)
        {
            if (p_link->role == SLAVE)
                return INVALID_HCI_PARAMETERS;

#if (PRH_BS_DEV_SUPERVISION_TIMEOUT_NEGOTIATION_SUPPORTED==0)
            if (max_allowed_interval < min_interval)
                return INVALID_HCI_PARAMETERS;
            max_interval = max_allowed_interval;
#endif
        }
    }

    if (min_interval < (p_link->device_index-1) * 16)
    {
        min_interval = max_interval;
    }

    /*
     * Set the num sniff negotiations to zero 
     */
    p_link->pol_info.sniff_info.num_sniff_negotiations = 0;
    if (LMpolicy_Derive_Sniff_Parameters(p_link,max_interval,min_interval,attempt,timeout) == NO_ERROR)
    {
        /*
         * Link may require adjustment of ACL packet types or supervisionTO
         * when starting sniff negotiations.
         */
        LMpol_Adjust_ACL_Packet_Types_On_Link(p_link, LMcontxt_ENTERING_SNIFF);
        LMqos_Adjust_Supervision_Timeout_On_Link(p_link, LMcontxt_ENTERING_SNIFF, max_interval);

        p_link->current_proc_tid_role = p_link->role;

        _Send_LMP_Sniff_Req(p_link);
    }   
    else 
        return UNSUPPORTED_PARAMETER_VALUE;

    return NO_ERROR;
}

/**************************************************************************
 *  Function : LMpolicy_LM_Exit_Sniff_Mode
 *
 *  DESCIPTION 
 *  Request from the upper layers on the local device to exit sniff.
 *  An LMP_Unsniff_Req is sent to the peer. 
 **************************************************************************/
t_error LMpolicy_LM_Exit_Sniff_Mode(t_lmp_link* p_link)
{
    t_lmp_pdu_info pdu_info;

    if (p_link->state == LMP_SNIFF_MODE)
    {
        /* 
         * Set up the Transaction Id and opcode 
         */
        p_link->current_proc_tid_role = p_link->role;

        /*
         * The Master enters sniff transition mode and waits for
         * lmp_accepted before leave sniff transition mode to
         * active state.
         * LMP_SNIFF_MODE set in sniff transition mode, so sniff
         * activites checked while also polls as transmission state.
		 * Set W4_SNIFF_EXIT to avoid a collision of two exit sniff coming
		 * at same time.
		 */
        if (p_link->role == MASTER)
        {
            p_link->state |= W4_SNIFF_EXIT;
        }

		pdu_info.tid_role = p_link->current_proc_tid_role;
		pdu_info.opcode = LMP_UNSNIFF_REQ;
		LM_Encode_LMP_PDU(p_link->device_index, &pdu_info);
    }
    else
    {
        return UNSPECIFIED_ERROR;
    }

    return NO_ERROR;
}

/**************************************************************************
 *  Function : LMpolicy_LMP_Sniff_Req
 *
 *  DESCIPTION 
 *  This function is responsible for handling an LMP SNIFF REQ PDU,
 *  The Actions performed are the following.
 *  
 **************************************************************************/ 
t_error LMpolicy_LMP_Sniff_Req(t_lmp_link* p_link,t_p_pdu p_payload)
{
    if((p_link->tid_role_last_rcvd_pkt != p_link->role) && 
        !mPol_Check_Sniff_Mode(p_link->link_policy_mode))
    {
        return LMP_PDU_NOT_ALLOWED;
    }  

    if((p_link->pol_info.sniff_info.num_sniff_negotiations) && (p_link->role == MASTER)
        && (p_link->current_proc_tid_role != p_link->tid_role_last_rcvd_pkt))
    {
        return LMP_ERROR_TRANSACTION_COLLISION;
    }

    if (!(p_link->state & LMP_ACTIVE))
    {
        return LMP_PDU_NOT_ALLOWED;
    }

    /*
     * If in the first round of negotiations, set the
     * max and min bounds for negotiation
     */
    if(p_link->pol_info.sniff_info.num_sniff_negotiations == 0)
    {
        p_link->pol_info.sniff_info.T_sniff_max = PRH_BS_CFG_SYS_LMP_MAX_SNIFF_INTERVAL;
        p_link->pol_info.sniff_info.T_sniff_min = PRH_BS_CFG_SYS_LMP_MIN_SNIFF_INTERVAL;
    }

    /*
     * Force Max Slots value to 1 only in the first round of negotiation
     */
    if (p_link->pol_info.sniff_info.num_sniff_negotiations == 0)
    {
        /*
         * Link may require adjustment of ACL packet types or supervisionTO
         * when starting sniff negotiations.
         */
         LMpol_Adjust_ACL_Packet_Types_On_Link(p_link, LMcontxt_ENTERING_SNIFF);
    }

    if(LMpolicy_Extract_Sniff_Parameters(p_link,p_payload))
    {
         LMqos_Adjust_Supervision_Timeout_On_Link(p_link, LMcontxt_ENTERING_SNIFF,
             p_link->pol_info.sniff_info.T_sniff);

        /*******************************************************************
         *  Accept the Sniff and Goes into Sniff informing the Host & LC 
         *******************************************************************/
        p_link->current_proc_tid_role = p_link->tid_role_last_rcvd_pkt ;
        LM_Encode_LMP_Accepted_PDU(p_link, LMP_SNIFF_REQ);

        /*
         * Maintain num_sniff_negotiations, as used to detect collisions.
         * Will be cleared after transitioning to LMP_SNIFF_MODE.
         */
        p_link->pol_info.sniff_info.num_sniff_negotiations++;
    }
    else
    {
        p_link->current_proc_tid_role = p_link->tid_role_last_rcvd_pkt ;
        /* Support of Maximun 5 Sniff_Reqs during negotiation */
        if (p_link->pol_info.sniff_info.num_sniff_negotiations < 5 )
        {
           /* Derive the new sniff parameters */
           LMpolicy_Derive_Sniff_Parameters(p_link,p_link->pol_info.sniff_info.T_sniff, p_link->pol_info.sniff_info.T_sniff,
                                            p_link->pol_info.sniff_info.N_sniff, p_link->pol_info.sniff_info.sniff_timeout);

           _Send_LMP_Sniff_Req(p_link);
        }
        else
        {
           p_link->pol_info.sniff_info.num_sniff_negotiations = 0;

           /*
            * A breakdown in sniff negotiations may require
            * the adjustment of ACL packet types on the link
            */
            LMpol_Adjust_ACL_Packet_Types_On_Link(p_link, LMcontxt_ENTERING_SNIFF);

           _Send_HC_Mode_Change_Event(p_link, 0, UNSUPPORTED_PARAMETER_VALUE);
           return UNSUPPORTED_PARAMETER_VALUE;
        }  
    }
    return NO_ERROR;
}

/**************************************************************************
 * Function :- LMpolicy_LMP_Unsniff_Req
 *
 * Description 
 * Request to remove a device from Sniff mode.
 **************************************************************************/
t_error LMpolicy_LMP_Unsniff_Req(t_lmp_link* p_link,t_p_pdu p_payload)
{
    t_error ret_error = LMP_PDU_NOT_ALLOWED;

    if (p_link->state == LMP_SNIFF_MODE)
    {
        /* Set up the Transaction Id */
        t_lmp_pdu_info pdu_info;

        pdu_info.tid_role = p_link->current_proc_tid_role = p_link->role ^ 0x01;
        pdu_info.opcode = LMP_ACCEPTED;
        pdu_info.return_opcode = LMP_UNSNIFF_REQ;
        pdu_info.ack_required = (p_link->role == MASTER);
        LM_Encode_LMP_PDU(p_link->device_index, &pdu_info);

        /*
         * Master enter sniff transition mode on receive unsniff_req
         * from slave, and waits for lmp_accepted bb ack before leave
         * sniff transition mode to active state. 
         * LMP_SNIFF_MODE set in sniff transition mode, so sniff
         * activites checked while also polls as transmission state.
         */
        if (p_link->role == MASTER)
        {
            p_link->state |= W4_SNIFF_EXIT;
        }
        else
        {
            LMpolicy_LM_Return_From_Sniff(p_link, NO_ERROR);
        }
        ret_error = NO_ERROR;
    }
    else if((p_link->role == MASTER)
        && (p_link->current_proc_tid_role != p_link->tid_role_last_rcvd_pkt))
    {
        ret_error = LMP_ERROR_TRANSACTION_COLLISION;
    }

    return ret_error;
}

/**************************************************************************
 * Function :- LMpolicy_Extract_Sniff_Parameters
 *
 * Description 
 * Removes the paramters from an incoming Sniff message.
 **************************************************************************/
u_int8 LMpolicy_Extract_Sniff_Parameters(t_lmp_link* p_link,u_int8* p_payload)
{
    /* Not the validity of these parameters should be checked against local
     * state information
     */
    u_int8 are_good_parameters = TRUE;
    u_int8 local_timing_ctrl=0;

    /*
     * The check on the timing control is done later in this
     * function.
     */
    p_link->pol_info.sniff_info.timing_ctrl = ((*(p_payload) & 0x2) >> 1);
    p_link->pol_info.sniff_info.D_sniff = LMutils_Get_Uint16(p_payload+1);
    p_link->pol_info.sniff_info.T_sniff = LMutils_Get_Uint16(p_payload+3);
    p_link->pol_info.sniff_info.sniff_timeout = LMutils_Get_Uint16(p_payload+7);

    if (LMutils_Get_Uint16(p_payload+5) < PRH_BS_CFG_SYS_LMP_MIN_SNIFF_ATTEMPT)
    {
        p_link->pol_info.sniff_info.N_sniff = PRH_BS_CFG_SYS_LMP_MIN_SNIFF_ATTEMPT;
        are_good_parameters = FALSE;
    }
    else
    {
        p_link->pol_info.sniff_info.N_sniff = LMutils_Get_Uint16(p_payload+5);
    }
#if (PRH_BS_CFG_SYS_LMP_MAX_SNIFF_INTERVAL != 0xFFFF)
    if(LMutils_Get_Uint16(p_payload+5) > PRH_BS_CFG_SYS_LMP_MAX_SNIFF_INTERVAL)
    {
        p_link->pol_info.sniff_info.N_sniff = PRH_BS_CFG_SYS_LMP_MAX_SNIFF_ATTEMPT;
        are_good_parameters = FALSE;
    }
#endif

    if(LMutils_Get_Uint16(p_payload+7) > PRH_BS_CFG_SYS_LMP_MAX_SNIFF_TIMEOUT)
    {
        p_link->pol_info.sniff_info.sniff_timeout = PRH_BS_CFG_SYS_LMP_MAX_SNIFF_TIMEOUT;
        are_good_parameters = FALSE;
    }

    if ((p_link->supervision_timeout != 0) && (p_link->pol_info.sniff_info.T_sniff > p_link->supervision_timeout))
    {
        /*
         * If the peer initiated the sniff, set a sane default for Tsniff
         */
        if(p_link->pol_info.sniff_info.num_sniff_negotiations == 0)
            p_link->pol_info.sniff_info.T_sniff = 12 * p_link->poll_interval;
        else /* local host initiated the sniff, stay within original boundaries from HCI */
            p_link->pol_info.sniff_info.T_sniff = p_link->pol_info.sniff_info.T_sniff_min;
        are_good_parameters = FALSE;
    }

    if (p_link->pol_info.sniff_info.T_sniff == 0)
    {
        /*
         * If the peer initiated the sniff, set a sane default for Tsniff
         */
        if(p_link->pol_info.sniff_info.num_sniff_negotiations == 0)
            p_link->pol_info.sniff_info.T_sniff = 12 * p_link->poll_interval;
        else /* local host initiated the sniff, stay within original boundaries from HCI */
            p_link->pol_info.sniff_info.T_sniff = p_link->pol_info.sniff_info.T_sniff_min;
        are_good_parameters = FALSE;
    }

    if(p_link->pol_info.sniff_info.T_sniff & 1)
    {
        p_link->pol_info.sniff_info.T_sniff &= 0xFFFE;
        are_good_parameters = FALSE;
    }

    if(p_link->pol_info.sniff_info.D_sniff & 1)
    {
        p_link->pol_info.sniff_info.D_sniff &= 0xFFFE;
        are_good_parameters = FALSE;
    }

    /*
     * T_sniff must be even by now.
     */
    if(p_link->pol_info.sniff_info.N_sniff > (p_link->pol_info.sniff_info.T_sniff>>1))
    {
        /* end negotiation */
        p_link->pol_info.sniff_info.num_sniff_negotiations = 6;
        return FALSE;
    }

    local_timing_ctrl = mLMpol_Get_Timing_Ctrl(p_link->device_index);
    if ((p_link->role == MASTER) && (p_link->pol_info.sniff_info.timing_ctrl != local_timing_ctrl))
    {
        /*
         * Use the local timing control (the master's timing control).
         */
        p_link->pol_info.sniff_info.timing_ctrl = local_timing_ctrl;
        are_good_parameters = FALSE;
    }

	/* 
	 * Typically a slave will send D_sniff=0. The master may wish to negotiate a
	 * different value of D_Sniff to that suggested by the slave.
	 */
#if 0 // This Precludes the negociation of the D-Sniff when we are in Scatternet
	  // If this device is Master and our peer is a CEVA device in scatternet
	  // it may propose back a D-Sniff which is non-Zero 

    if ((p_link->role == MASTER) && (LMpolicy_Master_Derive_D_Sniff(p_link)!=
		   p_link->pol_info.sniff_info.D_sniff))
        are_good_parameters = FALSE;
#endif

    if (p_link->pol_info.sniff_info.D_sniff >= p_link->pol_info.sniff_info.T_sniff)
        are_good_parameters = FALSE;

    if(p_link->pol_info.sniff_info.T_sniff_max < p_link->pol_info.sniff_info.T_sniff)
    {
        p_link->pol_info.sniff_info.T_sniff = p_link->pol_info.sniff_info.T_sniff_max;
        are_good_parameters = FALSE;
    }

    if(p_link->pol_info.sniff_info.T_sniff_min > p_link->pol_info.sniff_info.T_sniff)
    {
        p_link->pol_info.sniff_info.T_sniff = p_link->pol_info.sniff_info.T_sniff_min;
        are_good_parameters = FALSE;
    }

	if (LMconfig_LM_Connected_As_Scatternet() || g_LM_config_info.links_in_low_power)
	{
		if (p_link->pol_info.sniff_info.T_sniff < MIN_T_SNIFF_IN_SCATTERNET)
		{
			p_link->pol_info.sniff_info.T_sniff = MIN_T_SNIFF_IN_SCATTERNET;
			are_good_parameters = FALSE;
		}

		if (FALSE == LMpolicy_Determine_D_Sniff_In_Scatternet(p_link))
			are_good_parameters = FALSE;

	}

    return are_good_parameters;
}

/**************************************************************************
 * Function :- LMpolicy_LM_Return_From_Sniff
 *
 * Description 
 * Returns a connection to Active Mode.
 **************************************************************************/
void LMpolicy_LM_Return_From_Sniff(t_lmp_link* p_link, t_error reason)
{
    SYSmmi_Display_Event(eSYSmmi_LC_Connection_Event);
    LMpol_Decr_Links_In_Low_Power();
    p_link->state = LMP_ACTIVE;

	p_link->pol_info.sniff_info.next_sniff_window = 0xFFFFFFFF;

    if (LMconfig_LM_Connected_As_Scatternet())
    {
		p_link->pol_info.sniff_info.T_sniff = 0x00;

		LMpol_Adjust_Wakeup_Slot_Time(p_link,0xFFFFFFFF);
		LMch_Change_Piconet(0);
    }

#if (PRH_BS_CFG_SYS_LOW_POWER_MODE_SUPPORTED==1)
    if ((SLEEP_PROCEDURE == USLCchac_Get_Active_Procedure())
        || USLCchac_Is_Sleep_Request_Pending())
    {
        /*
         * LC Sleep should be cancelled (especially if local is a master).
         */
        USLCsleep_Cancel();

        if ((SLEEP_PROCEDURE == USLCchac_Get_Active_Procedure())
           && (p_link->role == SLAVE))
        {
            /* 
             * A slave will not have re-entered LC_Sleep when master-initiated exit
             * sniff due to baseband snoop for received LMP_UNSIFF_REQ. However, for
             * a slave-initated exit sniff, an LC_Wakeup R2P may still be required.
             */
            t_lm_sniff_info  *p_sniff = &p_link->pol_info.sniff_info;
            LC_Wakeup(p_link->device_index, 0, (p_sniff->next_sniff_window + 1));
        }
    }
#endif

    /*
     * Link may require adjustment of ACL packet types or supervisionTO
     * when returning from sniff.
     */
    LMpol_Adjust_ACL_Packet_Types_On_Link(p_link, LMcontxt_LEAVING_SNIFF);
    LMqos_Adjust_Supervision_Timeout_On_Link(p_link, LMcontxt_LEAVING_SNIFF, 0);

    if (p_link->role == SLAVE)
    {
        p_link->pol_info.sniff_info.N_sniff_rem = 0;
    }
    _Send_HC_Mode_Change_Event(p_link,0, reason);

    /*
     * Check if waiting to initiate detach after exit sniff mode procedure.
     */
    if (p_link->disconnect_req_reason)
    {
        LMconnection_LM_Disconnect(p_link,p_link->disconnect_req_reason);
        p_link->disconnect_req_reason = NO_ERROR;
    }
}

/**************************************************************************
 * Function :- LMpolicy_Enter_Sniff_Mode
 *
 * Description 
 * Device enters sniff mode
 **************************************************************************/
void LMpolicy_Enter_Sniff_Mode(t_deviceIndex device_index)
{
    t_lmp_link* p_link = LMaclctr_Find_Device_Index(device_index);
    t_lm_sniff_info* p_sniff = &p_link->pol_info.sniff_info;
    t_clock current_clock;

	p_link->state &= ~LMP_ACTIVE;
    p_link->state |= LMP_SNIFF_MODE;
    p_sniff->sniff_state = LMP_SNIFF_ACTIVATION_PENDING; 
    p_sniff->N_sniff_rem = p_sniff->N_sniff;
    p_sniff->num_sniff_negotiations = 0;
    g_LM_config_info.links_in_low_power++;

    /*
     * Determine the Sniff Instant.
     */ 
    current_clock = LC_Get_Piconet_Clock(p_link->device_index);
    
    p_sniff->next_sniff_window = \
        BTtimer_Set_Timer_On_Timing_Equation(current_clock,\
        p_sniff->timing_ctrl, p_sniff->T_sniff,p_sniff->D_sniff);



    // As a New Sniff Window has been set on a link we need to Adjust 
	// The next wakup time - to take it into account,
	// No Change Piconet is performed at this point.
	LMpol_Adjust_Wakeup_Slot_Time(p_link,(p_sniff->next_sniff_window>>1));


#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
    /*
     * Set timer to switch back to this piconet before first sniff window.
     */
    if (LMconfig_LM_Connected_As_Scatternet())
    {

		// Added By GF 15:26 on 21 Nov
		LMch_Change_Piconet((t_lmp_link*)0);

    }
#endif

#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUBRATING_SUPPORTED==1)
    if(1 == p_link->pol_info.sniff_info.sniff_subrating_req_pending)
    {
        _Send_LMP_Sniff_Subrating_Req(p_link);
        /*Do not reset ssr pending - Must derive SSR everytime on entering Sniff*/
        /*p_link->pol_info.sniff_info.sniff_subrating_req_pending = 0;*/
    }
#endif
    _Send_HC_Mode_Change_Event(p_link,p_link->pol_info.sniff_info.T_sniff,NO_ERROR);
}

/**************************************************************************
 * Function :- _Send_LMP_Sniff_Req
 *
 * Description 
 * Sends an LMP_SNIFF_REQ to the peer on the link.
 **************************************************************************/
void _Send_LMP_Sniff_Req(t_lmp_link* p_link)
{
    t_lmp_pdu_info  pdu_info;

    p_link->pol_info.sniff_info.num_sniff_negotiations++; 
    pdu_info.tid_role = p_link->current_proc_tid_role;  
    pdu_info.timing_control = p_link->pol_info.sniff_info.timing_ctrl;
    pdu_info.attempt = p_link->pol_info.sniff_info.N_sniff;
    pdu_info.timeout = p_link->pol_info.sniff_info.sniff_timeout ;
    pdu_info.offset = p_link->pol_info.sniff_info.D_sniff ;
    pdu_info.interval = p_link->pol_info.sniff_info.T_sniff;
    pdu_info.opcode = LMP_SNIFF_REQ;
    LM_Encode_LMP_PDU(p_link->device_index, &pdu_info);
}

/**************************************************************************
 * Function :- LMpolicy_Derive_D_Sniff
 *
 * Description 
 * Derives the D_Sniff parameter for Sniff Mode based on a links T_sniff &
 * N_sniff paramaters. Mainly used by LMpolicy_Derive_Sniff_Parameters.
 *
 **************************************************************************/
u_int16 LMpolicy_Master_Derive_D_Sniff(t_lmp_link* p_link)
{
    u_int16 T_sniff = p_link->pol_info.sniff_info.T_sniff;

	u_int16 D_sniff = 0;



	if ((LMconfig_LM_Connected_As_Scatternet()) && (g_LM_config_info.links_in_low_power))
	{
		LMpolicy_Determine_D_Sniff_In_Scatternet(p_link);
		D_sniff = p_link->pol_info.sniff_info.D_sniff;
	}
    else if (T_sniff > ( PRH_BS_CFG_SYS_MAX_ACTIVE_DEVICES_PICONET * 16))
    {
        D_sniff = (((p_link->device_index)-1) * 
                 (T_sniff/PRH_BS_CFG_SYS_MAX_ACTIVE_DEVICES_PICONET))&0xFFFE;
    }
    else
    {
        /* The following conditions are requested due to support point to mutipoint and 
         * short sniff interval for pseudo QoS to slaves that initiates to sniff request 
         * The sample spec (8msec@sniff for 2 slaves) was
         *    - sniff interval = 12slots
         *    - sniff attempt = 2
         *    - sniff timeout = 1
         * The target spec (for 4 sniff slaves or for 7 sniff slaves and 1 park slave) is
         *    - sniff interval = 16slots(typ), 12slots(min)
         *    - sniff attempt = 2(typ), 1(min)
         *    - sniff timeout = 1(typ), 0(min)
         *    - master tx 1 slot pkt/slave tx 3 slot pkt
         *
         * Generic solution below searches for most suitable D_sniff based on offsets in
         * use by existing sniffed links.
         *
         * Assumptions:
         * 1. All sniffed links have same sniff interval and sniff window size/attempts.
         * 2. All sniffed links are configured and remain for same max slots in/out.
         * 3. Sufficient bandwidth is available for all sniffed links to avoid overlap.
         */                
        extern t_link_entry link_container[];
        u_int8 d_sniff_separation, d_sniff_offset;
        u_int16 dmask=0; int i=0,j=0;
        
		u_int16 attempt = p_link->pol_info.sniff_info.N_sniff;
		d_sniff_separation = (attempt)*(p_link->max_slots_in + p_link->max_slots_out);
                
        while(i<(MAX_ACTIVE_DEVICE_LINKS-1) && j<(g_LM_config_info.num_acl_links))
        {
            if (link_container[i].used == 1)
            {
                if ((link_container[i].entry.role == MASTER)
                 && (link_container[i].entry.state == LMP_SNIFF_MODE))
                {
                    d_sniff_offset = (u_int8)(link_container[i].entry.pol_info.sniff_info.D_sniff);
                    dmask |= (1<<(d_sniff_offset/d_sniff_separation));
                }
                j++;
            }
            i++;
        }
            
        for (d_sniff_offset=0; dmask&1; dmask>>=1)
        {
            d_sniff_offset+=d_sniff_separation;
        }
         
        if(d_sniff_offset < (p_link->pol_info.sniff_info.T_sniff))
        {
            D_sniff = d_sniff_offset&0xFE;
        }
        else
        {
            D_sniff = PRH_DEFAULT_D_SNIFF;
        }
    }

	return D_sniff;
}

/**************************************************************************
 * Function :- LMpolicy_Derive_Sniff_Parameters
 *
 * Description 
 * Derives the Sniff Mode parameters. Checks the number of attempts
 * to determine if the values can be supported on the platform.
 *
 **************************************************************************/
t_error LMpolicy_Derive_Sniff_Parameters(t_lmp_link* p_link,u_int16 max_interval,u_int16 min_interval,u_int16 attempt,u_int16 timeout)
{
    /*
     * If peer device asks IUT to sniff for an odd number of slots,
     * the IUT must sniff for an even number of slots (as sniff
     * interval is frame-based (sniffer will always sniff the
     * sniffee in a master TX slot).
     *
     * There is no way of knowing if IUT should round down the
     * sniff interval or round up the sniff interval, e.g.
     * if asked to sniff with an interval 9 slots, should IUT expect to
     * sniff for an interval of 8 slots or an interval of 10 slots.
     */
    if((max_interval & 1) || (min_interval & 1))
        return INVALID_LMP_PARAMETERS;

    if (attempt < PRH_BS_CFG_SYS_LMP_MIN_SNIFF_ATTEMPT)
    {
        return INVALID_LMP_PARAMETERS;
    }
    else
    {
        p_link->pol_info.sniff_info.N_sniff = attempt;
    }

    p_link->pol_info.sniff_info.timing_ctrl = mLMpol_Get_Timing_Ctrl(p_link->device_index);
    p_link->pol_info.sniff_info.sniff_timeout = timeout;

	if (!LMconfig_LM_Connected_As_Scatternet())
	{
		p_link->pol_info.sniff_info.T_sniff = min_interval;
	}
	else
	{ /* TK 7oct2013 - reduce min supported sniff interval in scatternet to 40 slots (previously 200 slots).
	    note: although from experiment sniff interval has proven stable as low as 30 slots (when two piconets),
	     incorporating a safety margin here */
 
		if (min_interval < MIN_T_SNIFF_IN_SCATTERNET)
 		{
			if (max_interval < MIN_T_SNIFF_IN_SCATTERNET)
 				return UNSUPPORTED_PARAMETER_VALUE;
 			else
				p_link->pol_info.sniff_info.T_sniff = MIN_T_SNIFF_IN_SCATTERNET;
 		}
 		else
 		{
 			p_link->pol_info.sniff_info.T_sniff = min_interval;
 		}
	}


    if(p_link->role == MASTER)
    {
		p_link->pol_info.sniff_info.D_sniff = LMpolicy_Master_Derive_D_Sniff(p_link);

    }
	else if ((LMconfig_LM_Connected_As_Scatternet()) && (g_LM_config_info.links_in_low_power))
	{
 
		LMpolicy_Determine_D_Sniff_In_Scatternet(p_link);
	}

#if 0
    else
    {
        p_link->pol_info.sniff_info.D_sniff = PRH_DEFAULT_D_SNIFF;
    }
#endif

    return NO_ERROR;
}

/**************************************************************************
 * Function :- LMpolicy_LMP_Sniff_Req_Accepted_Ack
 *
 * Description 
 * Processes BB Ack to LMP_Accepted(LMP_SNIFF_REQ)
 **************************************************************************/
static void LMpolicy_LMP_Sniff_Req_Accepted_Ack_Handler(t_lmp_link *p_link);

void LMpolicy_LMP_Sniff_Req_Accepted_Ack(t_deviceIndex device_index)
{
    t_lmp_link *p_link = LMaclctr_Find_Device_Index(device_index);
    LM_DEFER_FROM_INTERRUPT_CONTEXT(LMpolicy_LMP_Sniff_Req_Accepted_Ack_Handler, p_link);
}

static void LMpolicy_LMP_Sniff_Req_Accepted_Ack_Handler(t_lmp_link *p_link)
{
    LMpolicy_Enter_Sniff_Mode(p_link->device_index);
}

/**************************************************************************
 * Function :- LMpolicy_LMP_Unsniff_Req_Accepted_Ack
 *
 * Description 
 * Returns a connection to Active Mode on Rx of the ACK  to
 * the LMP_UNSNIFF_REQ.
 **************************************************************************/
static void LMpolicy_LMP_Unsniff_Req_Accepted_Ack_Handler(t_lmp_link *p_link);
void LMpolicy_LMP_Unsniff_Req_Accepted_Ack(t_deviceIndex device_index)
{
    t_lmp_link* p_link = LMaclctr_Find_Device_Index(device_index);
    
    if ((p_link->state & LMP_SNIFF_MODE) && (p_link->role == MASTER))
    {
        LM_DEFER_FROM_INTERRUPT_CONTEXT(LMpolicy_LMP_Unsniff_Req_Accepted_Ack_Handler, p_link);
    }
}

static void LMpolicy_LMP_Unsniff_Req_Accepted_Ack_Handler(t_lmp_link *p_link)
{
    LMpolicy_LM_Return_From_Sniff(p_link, NO_ERROR);
}

#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUBRATING_SUPPORTED==1)

/**************************************************************************
 * Function :- LMpolicy_LM_Sniff_Subrating
 *
 * Description
 * This function handles a sniff subrating request from the higher layers.
 *************************************************************************/
t_error LMpolicy_LM_Sniff_Subrating(t_lmp_link* p_link, u_int16 max_latency,
            u_int16 min_remote_timeout, u_int16 min_local_timeout)
{
    if(!p_link)
        return NO_CONNECTION;

    if (!mFeat_Check_Sniff_Subrating(p_link->remote_features))
        return UNSUPPORTED_REMOTE_FEATURE;

    /* the maximum latency parameter shall define the maximum allowed
     * sniff sub-rate of the remote device, i.e. the transmit latency */ 
    p_link->pol_info.sniff_info.max_latency = max_latency;

    /* the initiating device shall not transition to the new sniff subrating
     * parameters until the sniff subrating instant has passed... */
    p_link->pol_info.sniff_info.min_remote_timeout = min_remote_timeout;
    p_link->pol_info.sniff_info.min_local_timeout = min_local_timeout;

    if(p_link->state & LMP_SNIFF_MODE)
    {
        _Send_LMP_Sniff_Subrating_Req(p_link);
    }
    //else // req_pending used on all subsequent LMpolicy_Enter_Sniff_Mode
    {
        p_link->pol_info.sniff_info.sniff_subrating_req_pending = 1;
    }

    return NO_ERROR;
}

/**************************************************************************
 *  Function : _Send_HC_Sniff_Subrating_Event
 *
 *  DESCIPTION
 *  This function generates a Sniff Subrating event to the higher layer.
 **************************************************************************/
void _Send_HC_Sniff_Subrating_Event(t_lmp_link* p_link,
         u_int16 max_tx_latency, u_int16 max_rx_latency, u_int16 min_remote_timeout,
         u_int16 min_local_timeout, t_error status)
{
    t_lm_event_info event_info;

    event_info.handle = p_link->handle;
    event_info.status = status;
    event_info.latency = ((u_int32)max_tx_latency << 16) + max_rx_latency;
    event_info.token_rate = ((u_int32)min_remote_timeout << 16) + min_local_timeout;

	if (p_link->operation_pending == LMP_SNIFF_SUBRATING_REQ)
		p_link->operation_pending = NO_OPERATION;

    HCeg_Generate_Event(HCI_SNIFF_SUBRATING_EVENT, &event_info);
} 

/**************************************************************************
 * Function :- LMpolicy_LMP_Sniff_Subrating_Req
 *
 * Description 
 * Processes the LMP_SNIFF_SUBRATING_REQ received from peer device.
 **************************************************************************/
t_error LMpolicy_LMP_Sniff_Subrating_Req(t_lmp_link* p_link,t_p_pdu p_payload)
{
    u_int16 max_rx_latency, max_tx_latency;
    u_int16 min_local_timeout, min_remote_timeout;

    t_lm_sniff_info *p_sniff = &p_link->pol_info.sniff_info;

    u_int8 ssr_subrate_req = p_payload[0];
    u_int16 ssr_timeout_req = LMutils_Get_Uint16(p_payload+1);
    u_int32 ssr_instant_req = LMutils_Get_Uint32(p_payload+3);

    if(!(p_link->state & LMP_SNIFF_MODE))
 // Conformance Fix
 //       return COMMAND_DISALLOWED;
		  return LMP_PDU_NOT_ALLOWED;

    if (0 == ssr_subrate_req)
        ssr_subrate_req = 1;
    
	if (p_link->operation_pending == LMP_SNIFF_SUBRATING_REQ)
	{
		if (p_link->role == MASTER) 
		{
			return LMP_ERROR_TRANSACTION_COLLISION;
		}
		else
		{
			p_sniff->min_remote_sniff_timeout = p_sniff->min_remote_timeout;
		}
	}

	
    if ((p_link->link_supervision_timeout != 0) && (p_link->link_supervision_timeout 
            < (p_link->pol_info.sniff_info.T_sniff * ssr_subrate_req)))
        return INVALID_LMP_PARAMETERS;

    if (p_link->role == SLAVE) /* validate requested_sniff_subrating_instant */
    {
        t_clock ssr_instant = ssr_instant_req<<1;
        t_clock next_sniff_instant = p_link->pol_info.sniff_info.next_sniff_window+1;

        if (BTtimer_Is_Expired_For_Time(ssr_instant, next_sniff_instant) || (((ssr_instant 
            - next_sniff_instant) & BT_CLOCK_MAX_TICKS) % ((p_link->pol_info.sniff_info.T_sniff<<1))))
            return INVALID_LMP_PARAMETERS;
    }

    p_sniff->local_sniff_subrate = ssr_subrate_req;

	if (p_sniff->new_local_sniff_timeout == 0)
	{
		p_sniff->new_local_sniff_timeout = ssr_timeout_req;
	}
	else
	{
		if (ssr_timeout_req > p_sniff->new_local_sniff_timeout)
			p_sniff->new_local_sniff_timeout = ssr_timeout_req;
	}
    /* In sniff subrating mode the mandatory sniff subrate anchor points at
     * which the master shall transmit to the slave and the slave shall listen
     * for the master are defined as follows (where M is the max subrate
     * received by the master, N is the max subrate received by the slave,
     * A is the sniff subrating instant, and k is any positive integer
     *             Master                        Slave
     ************************************************************************
     * M=N         CLKm(k)=A+[Tsniff*M*k]        CLKn(k)=A+[Tsniff*N*k]
     ************************************************************************
     * M>N         at min once every j anchor:   CLKn(k)=A+[Tsniff*N*k]
     *             CLKm(k)=A+[Tsniff*N*k]
     *              where j=[M/N]
     ************************************************************************
     * M<N         CLKm(k)=A+[Tsniff*M*k]        at min once every j anchor:
     *                                           CLKm(k)=A+[Tsniff*M*k]
     *                                           where j=[M/N]
     ************************************************************************/

    if(p_sniff->local_sniff_subrate > p_sniff->max_remote_sniff_subrate)
    {
        u_int8 j = (p_sniff->local_sniff_subrate)/(p_sniff->max_remote_sniff_subrate);
        p_sniff->local_sniff_subrate = (p_sniff->max_remote_sniff_subrate)*j;
    }
    
    /* When the LMP_sniff_subrating_req PDU is sent by the slave, the sniff
       subrating instant value shall be ignored. */
    if (p_link->role == SLAVE)
    {
        p_sniff->sniff_subrating_instant = ssr_instant_req;
    }
    else
    {
        p_sniff->sniff_subrating_instant =
            _Derive_Sniff_Subrating_Instant(p_link);
    }

    _Send_LMP_Sniff_Subrating_Res(p_link);

    /* The sniff subrating event indicates that the device associated with
     * the connection handle has either enabled sniff subrating or the sniff
     * subrating parameters have been renegotiated by the link manager. */
    
    max_rx_latency = (p_sniff->local_sniff_subrate)*(p_sniff->T_sniff);

    min_local_timeout = (p_sniff->new_local_sniff_timeout);

    max_tx_latency = (p_sniff->max_remote_sniff_subrate)*(p_sniff->T_sniff);

    min_remote_timeout = (p_sniff->min_remote_sniff_timeout);

    LMpol_Init_Sniff_Subrating_Instant(p_link, p_sniff->sniff_subrating_instant);
    
    _Send_HC_Sniff_Subrating_Event(p_link, max_tx_latency, max_rx_latency,
        min_remote_timeout, min_local_timeout, NO_ERROR);

    return NO_ERROR;
}

/**************************************************************************
 * Function :- LMpolicy_LMP_Sniff_Subrating_Res
 *
 * Description 
 * Processes the LMP_SNIFF_SUBRATING_RES received from peer device.
 **************************************************************************/
t_error LMpolicy_LMP_Sniff_Subrating_Res(t_lmp_link* p_link,t_p_pdu p_payload)
{
    u_int16 max_rx_latency, max_tx_latency;
    u_int16 min_local_timeout, min_remote_timeout;

    t_lm_sniff_info *p_sniff = &p_link->pol_info.sniff_info;

    u_int8 ssr_subrate_res = p_payload[0];
    u_int16 ssr_timeout_res = LMutils_Get_Uint16(p_payload+1);
    u_int32 ssr_instant_res = LMutils_Get_Uint32(p_payload+3);

    if (0 == ssr_subrate_res)
        ssr_subrate_res = 1;

    if ((p_link->link_supervision_timeout != 0) && (p_link->link_supervision_timeout 
            < (p_link->pol_info.sniff_info.T_sniff * ssr_subrate_res)))
        return INVALID_LMP_PARAMETERS;

    /* validate requested_sniff_subrating_instant */
    {
        t_clock ssr_instant = ssr_instant_res<<1;
        t_clock next_sniff_instant = p_link->pol_info.sniff_info.next_sniff_window+1;

        if (BTtimer_Is_Expired_For_Time(ssr_instant, next_sniff_instant) || (((ssr_instant 
            - next_sniff_instant) & BT_CLOCK_MAX_TICKS) % ((p_link->pol_info.sniff_info.T_sniff<<1))))
            return INVALID_LMP_PARAMETERS;
    }

    p_sniff->local_sniff_subrate = ssr_subrate_res;
    
    min_local_timeout = ssr_timeout_res;

    /* a value for minimum local timeout will already have been received from
     * hci_sniff_subrating command. the value for min_sniff_mode_timeout received
     * here is used only if larger than that specified via the hci command. */
    if(p_sniff->min_local_timeout > min_local_timeout)
        min_local_timeout = p_sniff->min_local_timeout;

    p_sniff->new_local_sniff_timeout = min_local_timeout;

    p_sniff->sniff_subrating_instant = ssr_instant_res;
    
    /* In sniff subrating mode the mandatory sniff subrate anchor points at
     * which the master shall transmit to the slave and the slave shall listen
     * for the master are defined as follows (where M is the max subrate
     * received by the master, N is the max subrate received by the slave,
     * A is the sniff subrating instant, and k is any positive integer
     *             Master                        Slave
     ************************************************************************
     * M=N         CLKm(k)=A+[Tsniff*M*k]        CLKn(k)=A+[Tsniff*N*k]
     ************************************************************************
     * M>N         at min once every j anchor:   CLKn(k)=A+[Tsniff*N*k]
     *             CLKm(k)=A+[Tsniff*N*k]
     *              where j=[M/N]
     ************************************************************************
     * M<N         CLKm(k)=A+[Tsniff*M*k]        at min once every j anchor:
     *                                           CLKm(k)=A+[Tsniff*M*k]
     *                                           where j=[M/N]
     ************************************************************************/

     if(p_sniff->local_sniff_subrate > p_sniff->max_remote_sniff_subrate)
     {
         u_int8 j = (p_sniff->local_sniff_subrate)/(p_sniff->max_remote_sniff_subrate);
         p_sniff->local_sniff_subrate = (p_sniff->max_remote_sniff_subrate)*j;
     }

    /* The sniff subrating event indicates that the device associated with
     * the connection handle has either enabled sniff subrating or the sniff
     * subrating parameters have been renegotiated by the link manager. */
    
    max_rx_latency = (p_sniff->local_sniff_subrate) *(p_sniff->T_sniff);

    max_tx_latency = (p_sniff->max_remote_sniff_subrate) *(p_sniff->T_sniff);

    min_remote_timeout = p_sniff->min_remote_sniff_timeout = p_sniff->min_remote_timeout;

    LMpol_Init_Sniff_Subrating_Instant(p_link, p_sniff->sniff_subrating_instant);

    _Send_HC_Sniff_Subrating_Event(p_link, max_tx_latency, max_rx_latency,
        min_remote_timeout, min_local_timeout, NO_ERROR);
        
    return NO_ERROR; 
}

u_int8 _LMpol_LMP_sniff_subrating_req_ext_PDU[9];
u_int8 _LMpol_LMP_sniff_subrating_res_ext_PDU[9];

/**************************************************************************
 * Function :- _Send_LMP_Sniff_Subrating_Req
 *
 * Description 
 * Sends an LMP_SNIFF_SUBRATING_REQ to the peer on the link. This specifies
 * the parameters that the peer device shall use for sniff subrating.
 **************************************************************************/
void _Send_LMP_Sniff_Subrating_Req(t_lmp_link* p_link)
{
    u_int16 max_latency;

    u_int8 *p_pdu;

    _LMpol_LMP_sniff_subrating_req_ext_PDU[0] = (127<<1) + p_link->role;
    _LMpol_LMP_sniff_subrating_req_ext_PDU[1] = (LMP_SNIFF_SUBRATING_REQ & 0xff);

    max_latency = p_link->pol_info.sniff_info.max_latency;

    if (max_latency && p_link->supervision_timeout)
    {
        if (max_latency > p_link->supervision_timeout)
            max_latency = (p_link->supervision_timeout - DEFAULT_T_POLL);
    }

    // max sniff subrate for remote device calculated = [tx latency / T_sniff]
    p_link->pol_info.sniff_info.max_remote_sniff_subrate =
        (max_latency)/(p_link->pol_info.sniff_info.T_sniff);

    if (0 == p_link->pol_info.sniff_info.max_remote_sniff_subrate)
        p_link->pol_info.sniff_info.max_remote_sniff_subrate = 1;

    _LMpol_LMP_sniff_subrating_req_ext_PDU[2] = p_link->pol_info.sniff_info.max_remote_sniff_subrate;

    // min sniff mode timeout for remote device = min_remote_timeout
    p_pdu = & _LMpol_LMP_sniff_subrating_req_ext_PDU[3];
    LMutils_Set_Uint16(p_pdu, p_link->pol_info.sniff_info.min_remote_timeout);

    //if(p_link->role == MASTER)
    {
        p_pdu = & _LMpol_LMP_sniff_subrating_req_ext_PDU[5];
        p_link->pol_info.sniff_info.sniff_subrating_instant = _Derive_Sniff_Subrating_Instant(p_link);
        LMutils_Set_Uint32(p_pdu, p_link->pol_info.sniff_info.sniff_subrating_instant);
    }
    //else 
    //{
        //LMutils_Set_Uint32( &_LMpol_LMP_sniff_subrating_req_ext_PDU[5], 0);
    //}

    LM_Encode_LMP_PDU_Fully_Encoded(p_link->device_index,
        LMP_SNIFF_SUBRATING_REQ, _LMpol_LMP_sniff_subrating_req_ext_PDU);
}



/**************************************************************************
 * Function :- _Send_LMP_Sniff_Subrating_Res
 *
 * Description 
 * Sends an LMP_SNIFF_SUBRATING_RES to the peer on the link. This specifies
 * the parameters that the remote device shall use for sniff subrating.
 **************************************************************************/
void _Send_LMP_Sniff_Subrating_Res(t_lmp_link* p_link)
{
    u_int16 max_latency = p_link->pol_info.sniff_info.max_latency;

    _LMpol_LMP_sniff_subrating_res_ext_PDU[0] = (127<<1) +  !(p_link->role);
    _LMpol_LMP_sniff_subrating_res_ext_PDU[1] = (LMP_SNIFF_SUBRATING_RES & 0xff);

    if (max_latency && p_link->supervision_timeout)
    {
        if (max_latency > p_link->supervision_timeout)
            max_latency = (p_link->supervision_timeout - DEFAULT_T_POLL);
    }

    p_link->pol_info.sniff_info.max_remote_sniff_subrate =
        (max_latency)/(p_link->pol_info.sniff_info.T_sniff);

    if (0 == p_link->pol_info.sniff_info.max_remote_sniff_subrate)
        p_link->pol_info.sniff_info.max_remote_sniff_subrate = 1;

    _LMpol_LMP_sniff_subrating_res_ext_PDU[2] =
        p_link->pol_info.sniff_info.max_remote_sniff_subrate;

    LMutils_Set_Uint16(& _LMpol_LMP_sniff_subrating_res_ext_PDU[3],
        p_link->pol_info.sniff_info.min_remote_sniff_timeout);

    LMutils_Set_Uint32( &_LMpol_LMP_sniff_subrating_res_ext_PDU[5],
        p_link->pol_info.sniff_info.sniff_subrating_instant);

    LM_Encode_LMP_PDU_Fully_Encoded(p_link->device_index,
        LMP_SNIFF_SUBRATING_RES, _LMpol_LMP_sniff_subrating_res_ext_PDU);
}

/**************************************************************************
 * Function :- _Derive_Sniff_Subrating_Instant
 *
 * Description 
 * Derive a suitable sniff subrating instant for the specified link.
 * Assumes the link is already in sniff mode.
 **************************************************************************/
u_int32 _Derive_Sniff_Subrating_Instant(t_lmp_link* p_link)
{
    u_int32 sniff_subrating_instant;
    t_lm_sniff_info *p_sniff = &p_link->pol_info.sniff_info;

    u_int32 common_subrate = (p_sniff->max_remote_sniff_subrate > p_sniff->local_sniff_subrate)?
        (p_sniff->max_remote_sniff_subrate):(p_sniff->local_sniff_subrate);

    // sniff subrating instant = 2*Tsniff from the next sniff window (nominal)        
    sniff_subrating_instant = ((p_link->pol_info.sniff_info.next_sniff_window+1)>>1)
            + (2*(p_link->pol_info.sniff_info.T_sniff)*common_subrate);
 
    return sniff_subrating_instant;
}

#endif

#endif

/*************************************************************
 ************  PARK MODE FUNCTIONALITY  **********************
 *************************************************************
 * 
 * Park Mode
 * ------------
 *  Following Cases to Be Considered CURRENTLY
 *
 * Device Parking
 *
 *  1/ Master Init Park - Success
 *  2/ Master Init Park - Failure
 *         2.1/ Slave rejects with LMP_Not_Accepted.
 *         2.2/ Master LC fails to Park.
 *
 *  3/ Slave Init Park / Master Proposes Park Params / Slave Accepts
 *  4/ Slave Init Park / Master Rejects
 *  5/ Slave Init Park / Master Proposes Park Parmas / Slave Rejects
 *  
 * Device UnParking
 *  1/ Master Initiated Unparking ( By PM_ADDR ) - Success
 *  2/ Master Initiated Unparking ( By PM_ADDR ) - Failure
 *          2.1 /Slave Does not Send LMP_Accept to LMP_UNPARK.
 *          2.2 /Master reports error in LC_EVENT_UNPARK_COMPLETE.
 *  3/ Master Initiated Unparking ( By BD_ADDR ) - Success
 *  4/ Master Initiated Unparking ( By BD_ADDR ) - Failure
 *          4.1 /Slave Does not Send LMP_Accept to LMP_UNPARK.
 *          4.2 /Master reports error in LC_EVENT_UNPARK_COMPLETE.
 *  5/ Slave Initiated Unparking
 *  6/ Automatic UnParking
 *************************************************************/
#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)

/**************************************************************************
 * Function :- LMpolicy_LM_Exit_Park_Mode
 *
 * Description 
 * Called by the HCI when a device needs to be parked. 
 * The role of the device is used to determinine and store the
 * unpark type. A 0 slot timer is set to trigger the execution of the
 * unpark  procedure and the p_link is inserted  into the list of link
 * to be unparked.
 **************************************************************************/    
t_error LMpolicy_LM_Exit_Park_Mode(t_lmp_link* p_link, u_int8 automatic)
{
    if (p_link == 0)
        return UNSPECIFIED_ERROR;

    if ((p_link->role == SLAVE) && (g_LM_config_info.M_access == 0))
        return COMMAND_DISALLOWED;

    if  (p_link->role == SLAVE)
    {  
        if ((!p_link->automatic_park_enabled) && !(p_link->state & LMP_PARK_MODE))
        {
            return COMMAND_DISALLOWED;
        }
        else
        {
            p_link->pol_info.park_info.unpark_pending = PRH_SLAVE_INITIATED_UNPARK;    
            p_link->pol_info.park_info.next_unpark_timer = BTtimer_Set_Slots(0x0);
        }
    }
    else
    {   
        /*
         * In case Park was pending on the link, remove it from the list of 
         * links to be parked.
         */
        if (p_link->automatic_park_enabled)
        {
            _LMpolicy_Remove_Park_Link(p_link);
            p_link->automatic_park_enabled = 0;
            p_link->pol_info.park_info.mode_change = 1;
        }
        else if (p_link->state != LMP_PARK_MODE)
        {
            return COMMAND_DISALLOWED;
        }
        p_link->pol_info.park_info.unpark_pending = PRH_MASTER_INITIATED_UNPARK;   
        p_link->pol_info.park_info.next_unpark_timer = BTtimer_Set_Slots(0x0);
    }

    /*
     *  Insert the link into the list of links to be unparked.
     *  Set the time interval to calling unpark to zero.
     */  
    _LMpolicy_Insert_UnPark_Link(p_link, PARK_NON_AUTO, p_link->role);

    return NO_ERROR;
}


/**************************************************************************
 * Function :- _LMpol_Execute_Unpark_Request
 *
 * Description 
 * Called by the schedular to exectute an unpark procdure.
 * 
 * IF Master
 *    1/ If not parked remove link from Unpark array and Send Mode Change
 *    2/ If first unpark Attempt (i.e. num_unparks_attempts == 0) then
 *       reset number of UnPark Attemps
 *    3/ Alloc AM_Addr
 *    4/ Change Device state to indicate Park/Unpark Pending
 *    5/ Build up UnPark PDU.
 *    6/ Change link and device unpark pending state.
 *    7/ Set Global info for Park/Unpark device index.
 * ELSE IF Slave
 *    1/ Change global unpark pending state
 **************************************************************************/

t_error _LMpol_Execute_Unpark_Request(t_lmp_link*  p_link, u_int8  automatic, t_role role)
{
    t_lmp_pdu_info  pdu_info;
    t_lm_park_info* p_park;
    u_int8 unpark_pdu_array[1*7];

    if (p_link->role == MASTER)
    {
        /* If  HCI Exit Park during auto-parking and link  is not parked */
        if (p_link->state != LMP_PARK_MODE)
        {
            _LMpolicy_Remove_UnPark_Link(p_link);
            _Send_HC_Mode_Change_Event(p_link,0,NO_ERROR);
            p_link->pol_info.park_info.mode_change = 0;
            return NO_ERROR;
        }
        /* Number of re-trys on unparking */

        if (p_link->pol_info.park_info.num_unparks_attempts==0)
            p_link->pol_info.park_info.num_unparks_attempts = PRH_BS_CFG_MAX_MASTER_UNPARK_ATTEMPTS;

        if (!automatic)
        {
            LMtmr_Clear_Timer(p_link->pol_info.park_info.park_timeout_index);
	    p_link->pol_info.park_info.park_timeout_index = 0;
        }

        p_park = &p_link->pol_info.park_info;

        if (NO_ERROR != LC_Alloc_AM_ADDR(&p_park->am_addr))
            return MAX_NUM_CONNECTIONS;

        LMconfig_LM_Set_Device_State(PRH_DEV_STATE_UNPARK_PENDING);
        p_pending_unpark_link = p_link;
        p_link->current_proc_tid_role = MASTER;

        pdu_info.timing_control = p_park->timing_ctrl;

        if(p_park->pm_addr != 0) /* Un Park by PM_ADDR */
        {
            /* For Now Limit the UnPark to a single PM_ADDR 
             * Thus the number of Bytes is set to 2   
             */

            pdu_info.number = 2;
            unpark_pdu_array[1] = p_park->pm_addr;
            pdu_info.opcode = LMP_UNPARK_PM_ADDR_REQ;
        }
        else /* Un Park by BD_ADDR */
        {
            /* For Now Limit the UnPark to a single BD_ADDR 
             * Thus the number of Bytes is set to 7    
             */
            pdu_info.number = 7;
            LMutils_Array_Copy(6,(unpark_pdu_array+1),p_link->bd_addr.bytes);
            pdu_info.opcode = LMP_UNPARK_BD_ADDR_REQ;
        }  
        unpark_pdu_array[0] = p_park->am_addr;
        pdu_info.ptr.p_uint8 = unpark_pdu_array;
        pdu_info.Db_present = 0;
        pdu_info.tid_role = p_link->current_proc_tid_role ;

        LM_Encode_LMP_PDU(PRH_PARK_BROADCAST_Q, &pdu_info);

        if (!automatic)
        {
            g_LM_config_info.park_info.unpark_pending = p_link->pol_info.park_info.unpark_pending ;
        }
        else
        {   /* Automatic unpark is always master initiated */
            p_link->pol_info.park_info.unpark_pending = PRH_MASTER_INITIATED_UNPARK;   
            g_LM_config_info.park_info.unpark_pending = PRH_MASTER_INITIATED_UNPARK;
        }
        g_LM_config_info.park_info.am_addr = p_park->am_addr;
        g_LM_config_info.park_info.park_device_index = p_link->park_device_index;
        g_LM_config_info.park_info.unpark_device_index = p_link->park_device_index;

        p_link->default_pkt_type = POLLpkt;
    }
    else
    {   /* Slave Initiated Un Parking */
        LMconfig_LM_Set_Device_State(PRH_DEV_STATE_UNPARK_PENDING);
        if((p_link->pol_info.park_info.unpark_pending == PRH_SLAVE_INITIATED_UNPARK) ||
           (p_link->pol_info.park_info.unpark_pending == PRH_MASTER_INITIATED_UNPARK))
        {
            g_LM_config_info.park_info.unpark_pending = p_link->pol_info.park_info.unpark_pending;  
        }
        else if (automatic)
        {
            p_link->pol_info.park_info.unpark_pending = PRH_SLAVE_INITIATED_UNPARK;    
            g_LM_config_info.park_info.unpark_pending = p_link->pol_info.park_info.unpark_pending;
            p_link->pol_info.park_info.mode_change = 0;
        }

    }

    p_link->pol_info.park_info.mode_change = (!automatic);
    g_LM_config_info.unpark_handle = p_link->handle;

    return NO_ERROR;
}

/**************************************************************************
 *
 * Function :- LMpolicy_LMP_Unpark_Pmaddr_Accepted_Ack
 * 
 * Description 
 * This function is called in a slave device when a baseband
 * acknowledgement has been received for the LMP_ACCEPTED 
 * sent in reply to the LMP_UNPARK_PM_ADDR_REQ.
 *
 * This results in the LC being instructed to commit the 
 * unpark operation, the LMP changes state for the link and
 * a mode change event is sent to the upper layers.
 **************************************************************************/
static void LMpolicy_LMP_Unpark_Pmaddr_Accepted_Ack_Hanlder(t_lmp_link *p_link);

void LMpolicy_LMP_Unpark_Pmaddr_Accepted_Ack(t_deviceIndex device_index)
{
    t_lmp_link* p_link = LMaclctr_Find_Device_Index(device_index);
    LM_DEFER_FROM_INTERRUPT_CONTEXT(LMpolicy_LMP_Unpark_Pmaddr_Accepted_Ack_Hanlder, p_link);
}

static void LMpolicy_LMP_Unpark_Pmaddr_Accepted_Ack_Hanlder(t_lmp_link *p_link)
{
    _LMpolicy_Remove_UnPark_Link(p_link);
    p_link->state = LMP_ACTIVE;
    if (p_link->pol_info.park_info.mode_change)
    {
        _Send_HC_Mode_Change_Event(p_link,0,NO_ERROR);

#if 1  // GF 14 May - Added to Handle HCI disconnet of Parked link.
		// Link is automatically unparked - and then Disconnected.
		if (p_link->disconnect_req_reason)
		{
			LMconnection_LM_Disconnect(p_link,p_link->disconnect_req_reason);
			p_link->disconnect_req_reason = NO_ERROR;
		}
#endif
    }
    else
    {
        LMch_Enable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_PARK);
    }   
}

/**************************************************************************
 *
 * Function :- LMpolicy_LM_Park_Mode
 * 
 * Description 
 * This function is called by the higher layers to initiate
 * parking of a slave. The Max/Min Interval are used to determine
 * the Beacon Interval to be used.
 **************************************************************************/
t_error LMpolicy_LM_Park_Mode(
     t_lmp_link* p_link, u_int16 max_interval, u_int16 min_interval)
{
    t_error status;

    /*
     * Admission control for HCI invoked Park.  Reject request if
     * 1. LMP link state is not active    
     * 2. One or more SCOs is active for this device
     * 3. Detaching
     * 4. Park is not supported by remote device
     * 5. Link supervision time is less than requested Park min interval
     */
    if ((p_link->state != LMP_ACTIVE) ||
        LMscoctr_Get_Number_SCOs_Active_On_Link(p_link->device_index) || 
        (p_link->operation_pending == LMP_DETACH)
        )
    {
        return COMMAND_DISALLOWED;
    }

    if (!mFeat_Check_Park_State(p_link->remote_features))
    {
        return UNSUPPORTED_REMOTE_FEATURE;
    }

    if (!mPol_Check_Park_Mode(p_link->link_policy_mode))
        return COMMAND_DISALLOWED;

#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
    /*
     * Cannot support multiple parked piconets in scatternet simultaneously.
     */
    else if (g_LM_config_info.num_links_parked && ((p_link->role == SLAVE) ||
        g_LM_config_info.role_in_park == SLAVE))
    {
        return COMMAND_DISALLOWED; // insufficient resources
    }
#endif

    min_interval &= 0xFFFE;
    max_interval &= 0xFFFE;

    if ((p_link->supervision_timeout != 0) && (p_link->supervision_timeout < max_interval))
    {
        if (p_link->supervision_timeout < min_interval)
            return INVALID_HCI_PARAMETERS;
        else
            max_interval = p_link->supervision_timeout;
    }

#if (PRH_BS_DEV_SLAVE_FORCE_STOP_AUTOPARK_SUPPORTED==1)
    p_link->force_stop_autopark = 0;
#endif

    /************************************************/
    /* Determine the park mode parameters 
     * 
     ************************************************/

    status = LMpolicy_Derive_Park_Parameters(p_link->role,
        &p_link->pol_info.park_info, max_interval, min_interval, p_link->device_index);

    if (status == NO_ERROR)
    {
        p_link->automatic_park_enabled = (p_link->link_policy_mode&0x10) && (p_link->role == MASTER);

        p_link->pol_info.park_info.next_park_timer = BTtimer_Set_Slots(0x0);
        _LMpolicy_Insert_Park_Link(p_link, PARK_NON_AUTO, p_link->role);
    }
    return status;
}

/**************************************************************************
 * Function :- _LMpol_Execute_Park_Request
 *
 * Description 
 * Called by the schedular to exectute a Park procdure.
 * 
 * IF Master
 *    1/ Allocate an AR_ADDR and a PM_ADDR
 * ELSE IF Slave
 *    1/ Default AR_ADDR = 01 and Default PM_ADDR = 01
 *
 * Set the Tid
 * Disable L2CAP traffic on the link
 * Set device state to PARK_UNPARK pending
 * Send LMP_Park_Req PDU.
 **************************************************************************/
static t_error _LMpolicy_Execute_Park_Request(t_lmp_link* p_link,u_int8 automatic, t_role role)
{
    if (p_link->role == MASTER)
    {
        p_link->pol_info.park_info.pm_addr = LMpolicy_Alloc_Pm_Addr();
        p_link->pol_info.park_info.ar_addr = LMpolicy_Alloc_Ar_Addr();
    }
    else
    {
        p_link->pol_info.park_info.pm_addr = 0x01;
        p_link->pol_info.park_info.ar_addr = 0x01;
    }

    p_link->current_proc_tid_role = role;
    if (automatic)
        p_link->pol_info.park_info.mode_change = 0;
    else
        p_link->pol_info.park_info.mode_change = 1;

    /* Derive the Timing System required */
    LMch_Disable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_PARK);
    LMconfig_LM_Set_Device_State(PRH_DEV_STATE_PARK_PENDING);
    _LMpolicy_Send_LMP_Park_Req(p_link, &p_link->pol_info.park_info);

    return NO_ERROR;
}

#if (PRH_BS_DEV_MASTER_PARK_REQ_ACK_TIMER==1)
void LMpolicy_LMP_Park_Req_Ack(t_deviceIndex device_index)
{
    t_lmp_link* p_link;

    p_link = LMaclctr_Find_Device_Index(device_index);

    p_link->pol_info.park_info.park_timeout_index = LMtmr_Set_Timer(
        (t_slots)(p_link->poll_interval*6), 
        _LMpolicy_Enter_Park_Mode, p_link, 1);
}
#endif

/**************************************************************************
 *
 * Function :- LMpolicy_LMP_Modify_Beacon
 * 
 * Description 
 * This function is called when an LMP_Modify_Beacon PDU is 
 * received. 
 * 
 * If the device is a SLAVE only 
 **************************************************************************/
t_error LMpolicy_LMP_Modify_Beacon(t_lmp_link* p_link, t_p_pdu p_payload)
{
    t_lm_park_info* p_park_info = &g_LM_config_info.park_info;
    t_clock current_clock;

    if (p_link->state != LMP_PARK_MODE)
        return NO_ERROR;

    if (p_link->role == MASTER)
        return NO_ERROR;

    LMpolicy_Extract_Park_Parameters(p_link->role,
        p_park_info,p_payload,LMP_MODIFY_BEACON);

    /* 
     * Read the piconet clock. Slave only at this point
     */
    current_clock = LC_Get_Piconet_Clock(p_link->device_index);

    /*
     * Reset the beacon instant on the basis of the new parameters.
     */
    g_LM_config_info.beacon_instant = 
        BTtimer_Set_Timer_On_Timing_Equation(
            current_clock,
            p_link->pol_info.park_info.timing_ctrl,
            (g_LM_config_info.N_sleep * g_LM_config_info.T_bcast), 
            ((t_slots)(g_LM_config_info.D_bcast + 
              (t_slots)(g_LM_config_info.D_sleep * g_LM_config_info.T_bcast))));

    return NO_ERROR;
}


/**************************************************************************
 * FUNCTION :- LMpolicy_LMP_Set_Broadcast_Scan_Window
 *
 * DESCRIPTION :- 
 * This message modifies the Broadcast Scan extension window 
 * for the current Beacon ONLY.
 * To allow this PDU to be properly applied to the system the
 * Broadcast mechanism must change.
 * 
 * p_link           pointer to the lmp link container
 * p_payload        pointer to parameters one per byte
 *                  for LMP_set_broadcast_scan_window
 *                        timing control flags      1 byte
 *                        DB (optional)             2 byte
 *                        broadcast scan window     2 byte
 **************************************************************************/
t_error LMpolicy_LMP_Set_Broadcast_Scan_Window(t_lmp_link* p_link, t_p_pdu p_payload)
{
    t_lm_park_info* p_park_info = &g_LM_config_info.park_info;

    u_int8 timing_ctrl_flags = *p_payload++; 

    /*
     * Bit 0 Timing Change indicates if Db is present (if 1)
     * Bit 1 Use initialisation 1 or 2 
     */
    p_park_info->timing_ctrl =  (timing_ctrl_flags & 0x2) >> 1;

    if (p_link->state == LMP_PARK_MODE)
    {
        if (timing_ctrl_flags & 0x01) /* Db Present */
        {
            g_LM_config_info.D_bcast = LMutils_Get_Uint16(p_payload);
            p_payload+=2;
        }

        g_LM_config_info.bcast_scan_ext = LMutils_Get_Uint16(p_payload);
    }

    return NO_ERROR;
}

/**************************************************************************
 *
 * Function :- LMpolicy_LMP_Park_Req
 * 
 * Description 
 * This function is called when an LMP_Park_Req PDU is 
 * received. If park is not allowed in the current device, 
 * an LMP_Not_Accepted is returned.
 * Otherwise if the the park parameters are extracted 
 * If the device is a MASTER only the beacon interval is 
 * considered relevant, all the other parameters are newly 
 * derived.
 * If the device is a SLAVE only 
 **************************************************************************/
t_error LMpolicy_LMP_Park_Req(t_lmp_link *p_link, t_p_pdu p_payload)
{
    t_error status = NO_ERROR;

    /*
     * Admission control for peer invoked Park.  Reject LMP_Park_Req if
     * 1. LMP link state is not active    
     * 2. One or more SCOs is active for this device
     * 3. If initiated by peer AND link policy settings do not allow PARK
     *    (If the local device initiated then ignore policy setttings!).
     */
    if ( (p_link->state != LMP_ACTIVE) ||
         (LMscoctr_Get_Number_SCOs_Active_On_Link(p_link->device_index) > 0) ||
         (p_link->tid_role_last_rcvd_pkt != p_link->role && 
              !mPol_Check_Park_Mode(p_link->link_policy_mode) ) )
    {
        status = LMP_PDU_NOT_ALLOWED;
    }

    /* 
     * If the Master Initiated the Park ensure that the slave responds with the 
     * Master Tid.  Otherwise, a Transaction Collision has occured.
     */
    else if((p_link->operation_pending == LMP_PARK_REQ) && (p_link->role == MASTER)
        && (p_link->current_proc_tid_role != p_link->tid_role_last_rcvd_pkt))
    {
        status = LMP_ERROR_TRANSACTION_COLLISION;
    }

#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
    /*
     * Cannot support multiple parked piconets in scatternet simultaneously.
     */
    else if (g_LM_config_info.num_links_parked && ((p_link->role == SLAVE) ||
        g_LM_config_info.role_in_park == SLAVE))
    {
        status = COMMAND_DISALLOWED; // insufficient resources
    }
#endif

#if (PRH_BS_DEV_SLAVE_FORCE_STOP_AUTOPARK_SUPPORTED==1)
    else if (p_link->force_stop_autopark)
    {
        p_link->force_stop_autopark = 0;
        status = LMP_PDU_NOT_ALLOWED;
    }
#endif

    /* Set the Tid  */
    if  (status==NO_ERROR)
    {
        p_link->current_proc_tid_role = p_link->tid_role_last_rcvd_pkt;  

        /* 
         * Note :- If the Slave park parameters are incorrect we should ignore
         * them as they are only suggested values.
         * Thus we should not send back LMP_Not_Accepted to a slave
         * based on the parameters it sent.
         */
        if(NO_ERROR == LMpolicy_Extract_Park_Parameters(p_link->role,
             &p_link->pol_info.park_info, p_payload, LMP_PARK_REQ) )  
        {            
            if (p_link->role == MASTER)
            {
                /*
                 * Negotiate shorter park interval if close to supervision timeout.
                 */
                if((p_link->supervision_timeout) &&
                   (g_LM_config_info.T_bcast > ((p_link->supervision_timeout)>>2)))
                {
                    g_LM_config_info.T_bcast = ((p_link->supervision_timeout)>>2);
                }

                /* 
                 * Slave Initiated Parking 
                 * No need to check the Error code returned from the Derive 
                 * Park Parameters as the values in the LMP_Park_Req 
                 * are only suggested values from the slave.
                 */
                LMpolicy_Derive_Park_Parameters(p_link->role,
                    &p_link->pol_info.park_info,
                    g_LM_config_info.T_bcast,
                    g_LM_config_info.T_bcast,
                    p_link->device_index);

                    /*
                     * Use park/unpark containers for auto-park. Note this will result
                     * in park reqs as new transaction with master TID.
                     */
                    
                p_link->pol_info.park_info.next_park_timer = BTtimer_Set_Slots(0x0);
                p_link->automatic_park_enabled = (p_link->link_policy_mode&0x10);
                _LMpolicy_Insert_Park_Link(p_link, PARK_NON_AUTO, SLAVE);

            }
            else /* p_link->role == SLAVE */
            {   
                /* 
                 * Master Initiated Parking 
                 */
                if (p_link->tid_role_last_rcvd_pkt==MASTER)
                {
                    p_link->pol_info.park_info.mode_change = 1;
                }

                LMconfig_LM_Set_Device_State(PRH_DEV_STATE_PARK_PENDING);
                LMch_Disable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_PARK);
                LM_Encode_LMP_Accepted_PDU(p_link, LMP_PARK_REQ);
                g_LM_config_info.park_info.unpark_pdu_rx = 0;
                p_link->pol_info.park_info.park_timeout_index = LMtmr_Set_Timer(   
                    p_link->poll_interval*6, 
                    _LMpolicy_LMP_Park_Accepted_Ack_Timeout, p_link, 1);   
            }
        }
        else
        {
            status = INVALID_LMP_PARAMETERS;
        }
    }

    if (status != NO_ERROR)
    {
        LMch_Enable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_PARK);
        p_link->num_park_negotiations = 0;
    }

    return status;
}

/**************************************************************************
 *
 * Function :- _LMpolicy_LMP_Park_Accepted_Ack_Timeout
 *
 * Description :- 
 * IF the 6*Tpol timer expires without receipt of BB ack to accepted, then
 * enter park mode. Must also remove LMP_accepted from lmp out queue to
 * avoid retransmit after unpark, and clear tx ack pending flag.
 **************************************************************************/
static void _LMpolicy_LMP_Park_Accepted_Ack_Timeout(t_lmp_link* p_link)
{
    t_devicelink *p_dev_link;
    u_int32 cpu_flags;
    u_int8 tx_ack_pending;

    p_dev_link = DL_Get_Device_Ref(p_link->device_index);

    SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);
    tx_ack_pending = DL_Get_Tx_Ack_Pending(p_dev_link);
	DL_Set_Tx_Ack_Pending(p_dev_link, FALSE);
    SYSirq_Interrupts_Restore_Flags(cpu_flags);

    if (tx_ack_pending)
    {
        LC_Ack_Last_Tx_Packet(p_link->device_index);
    }

    if (p_link->state != LMP_PARK_MODE)
    {
        _LMpolicy_Enter_Park_Mode(p_link);
    }
}

/**************************************************************************
 *
 * Function :- LMpolicy_LMP_Park_Accepted_Ack
 *
 * Description :- 
 * On receipt of the accepted for LMP_Park_Accepted the timer is e
 * cleared and the device enters park mode 
 **************************************************************************/
void LMpolicy_LMP_Park_Accepted_Ack(t_deviceIndex device_index)
{
   t_lmp_link* p_link;

   p_link = LMaclctr_Find_Device_Index(device_index);

   if(p_link->state != LMP_PARK_MODE)
   {
       LM_DEFER_FROM_INTERRUPT_CONTEXT(_LMpolicy_Enter_Park_Mode, p_link);
   }
}

/**************************************************************************
 *
 * Function :- LMpolicy_Unpark_Timeout
 *
 * Description :- 
 * Handles the condiction where the LC completes the unpark 
 * procedure BUT the LM does not recieve the LMP_Accepted for the 
 * LMP_Unpark_PmAddr_Req.
 * 
 **************************************************************************/
void LMpolicy_Unpark_Timeout(t_lmp_link* p_link)
{
    /* 
     * This function is called when an LMP_Accepted has
     * not been recieved for an LMP_UnPark_XXX_Req.
     * It effectively rolls back the UnPark in the Link Controller.
     */

    if (p_link != 0)
    {
        p_link->pol_info.park_info.unpark_timeout_index = 0;
        
        p_link->poll_interval = DEFAULT_T_POLL;
        p_link->poll_position = DEFAULT_T_POLL;

        /* 
         * Tell the LC to rollback the park 
         */    
        g_LM_config_info.park_info.unpark_pending = PRH_NO_UNPARK_PENDING;
        LC_Unpark_Rollback();
    
        /*
         * Return State to Parked
         * Inc number of links Parked 
         * Free Am_Addr
         */
        p_link->state = LMP_PARK_MODE;
        if (p_link->pol_info.park_info.am_addr)
        {
            DL_Free_Am_Addr(p_link->pol_info.park_info.am_addr);
        }
        g_LM_config_info.num_links_parked++;
        g_LM_config_info.links_in_low_power++;

        /* 
         * If Automatic Unpark has been performed -
         * Then the Link needs to be automatically parked again.
         */
        if (mPol_Check_Automatic_Unpark_Mode(p_link->link_policy_mode))
        {
            /* Immediate retry */
            p_link->pol_info.park_info.next_unpark_timer = 
                BTtimer_Set_Slots(0);
        }
        else
        {    
            /* Return State to Parked */
            _Send_HC_Mode_Change_Event(p_link, 
                g_LM_config_info.T_bcast, LMP_RESPONSE_TIMEOUT);    
            _LMpolicy_Remove_UnPark_Link(p_link);
        }
        LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_UNPARK_PENDING);
    }
}
    
/**************************************************************************
 *
 * Function :- LMpolicy_Clear_Link_If_Parked
 *
 * Description :- 
 * Clears a link which is currently parked. Ensures that if a connection
 * timeout occurs while a link is unparking the device is returned to a
 * Stable state.
 *  i.e  global unpark_pending == NO_UNPARK_PENDING
 *       Any entries on the Broadcast Q are removed.
 *       Device state clear to re-enable scanning
 *       The AM_Addr of the link being unparked is freed.
 *       Mode Change event sent to higher layers.
 **************************************************************************/
void LMpolicy_Clear_Link_If_Parked(t_lmp_link* p_link)
{
    if ((p_link->state & LMP_PARK_MODE) == LMP_PARK_MODE)
    {
        if(((p_link->pol_info.park_info.unpark_pending == PRH_SLAVE_INITIATED_UNPARK) ||
            (p_link->pol_info.park_info.unpark_pending == PRH_MASTER_INITIATED_UNPARK)))
        {
            /* 
             * preclude an unparking 
             */

            g_LM_config_info.park_info.unpark_pending = PRH_NO_UNPARK_PENDING;

            /* 
             * Remove it from the Park serice Qs
             */
            _LMpolicy_Remove_Park_Link(p_link);
            _LMpolicy_Remove_UnPark_Link(p_link);

            /* 
             * Reset the broadcast Q to remove any pending LMP_UNPARK PDUs 
             */
            BTq_Reset(0);
            /* 
             * Clear the  device state 
             */
            LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_UNPARK_PENDING);
            /* 
             * Free any Am_Addr allocated to the device 
             */
            if (p_link->pol_info.park_info.am_addr)
            {
                DL_Free_Am_Addr(p_link->pol_info.park_info.am_addr);       
            }
            p_link->num_park_negotiations = 0;

           _Send_HC_Mode_Change_Event(p_link, 0, UNSPECIFIED_ERROR);
        }
        
        g_LM_config_info.num_links_parked--;
        LMpol_Decr_Links_In_Low_Power();
        if (p_link->role == MASTER)
        {
            LMpolicy_Free_Pm_Addr(p_link->pol_info.park_info.pm_addr);
            LMpolicy_Free_Ar_Addr(p_link->pol_info.park_info.ar_addr);
        }
    }
}

/**************************************************************************
 *
 * Function :- LMpolicy_Alloc_Ar_Addr
 *
 * Description :- 
 * This allocates an Ar_Addr from a pool of 256 IDs. The AR_Addr
 * is allocated by the Master when parking a device. 
 **************************************************************************/
static u_int8 LMpolicy_Alloc_Ar_Addr(void)
{
    return LMutils_Alloc_u_int8_ID(LMpolicy_Ar_Addr_Pool);
}

/**************************************************************************
 *
 * Function :- LMpolicy_Frees_Ar_Addr
 *
 * Description :- 
 * This allocates an Ar_Addr from a pool of 256 IDs. 
 **************************************************************************/
static void LMpolicy_Free_Ar_Addr(u_int8 ar_addr)
{
    LMutils_Free_u_int8_ID(LMpolicy_Ar_Addr_Pool,ar_addr);
}

/**************************************************************************
 *
 * Function :- LMpolicy_Alloc_Pm_Addr
 *
 * Description :- 
 * This allocates an Pm_Addr from a pool of 256 IDs. The PM_Addr
 * is allocated by the Master when parking a device.
 **************************************************************************/
static u_int8 LMpolicy_Alloc_Pm_Addr()
{
#if (PRH_BS_CFG_SYS_UNPARK_USING_BDADDR_ONLY_SUPPORTED==0)
    return LMutils_Alloc_u_int8_ID(LMpolicy_Pm_Addr_Pool);
#else
    return 0;
#endif
}

/**************************************************************************
 *
 * Function :- LMpolicy_Frees_Pm_Addr
 *
 * Description :- 
 * This allocates an Pm_Addr from a pool of 256 IDs. 
 **************************************************************************/
static void LMpolicy_Free_Pm_Addr(u_int8 pm_addr)
{
    LMutils_Free_u_int8_ID(LMpolicy_Pm_Addr_Pool,pm_addr);
}

/**************************************************************************
 *
 * Function :- LMpolicy_LMP_Unpark_Bd_Addr_Req
 *
 * Description :- 
 * This function handles and incoming LMP_UnPark_BD_Addr_Req
 * 
 **************************************************************************/
t_error LMpolicy_LMP_Unpark_Bd_Addr_Req(t_lmp_link *p_link, t_p_pdu p_payload)
{    
    t_lm_park_info  *p_park_info = &g_LM_config_info.park_info;
    boolean         match_found = 0;
    t_bd_addr       pdu_bda;
    u_int8          am_addr_byte;
    u_int8          am_addr;
    u_int8          timing_ctrl_flags;
    
    /* 
     * The Timing Control and Db are obtained and used irrespective of whether this 
     * device is to be unparked or not. 
     */
    if(p_link->state == LMP_PARK_MODE && g_LM_config_info.state != LMP_ACTIVE)
    {
        timing_ctrl_flags = *p_payload++;
        if (timing_ctrl_flags & 0x01) /* Db Present */
        {
            /* 
             * The flag for initialization 2 is not useful unless DB is 
             * present and is then tied to the master's clock.
             * p_park_info->timing_ctrl =  ((*(p_payload++) & 0x2) >> 1); 
             */
            p_park_info->timing_ctrl =  ((timing_ctrl_flags & 0x2) >> 1);
            g_LM_config_info.D_bcast = LMutils_Get_Uint16(p_payload);
            p_payload+=2;
        }
        
        /* 
         * Extract the Am_Addr byte and verify if the first BDADDR matches
         */
        am_addr_byte = *(p_payload++);
        BDADDR_Assign_from_Byte_Array(&pdu_bda, p_payload);
        am_addr = am_addr_byte & 0x07;
        if(am_addr!=0)
        {
            match_found = LMutils_Bd_Addr_Match(SYSconfig_Get_Local_BD_Addr_Ref(),&pdu_bda);
        }

        /*
         * If 1st address doesn't match then verify if the 2nd matches
         */
        if (!match_found)
        {
            am_addr = (am_addr_byte & 0x70) >> 4;
            /* Check if second Bd_Addr Matches */
            BDADDR_Assign_from_Byte_Array(&pdu_bda, p_payload+6);
            if(am_addr!=0)
            {
                match_found = LMutils_Bd_Addr_Match(SYSconfig_Get_Local_BD_Addr_Ref(),&pdu_bda);
            }
        }

        if ((p_park_info->unpark_pdu_rx != 0x01) /* Duplicate Message */ && match_found)
        {
            /* Extract the Parameters used for Unparking the device */
            p_park_info->am_addr = am_addr;
            p_pending_unpark_link = p_link;
            p_pending_unpark_link->pol_info.park_info.unpark_type = LMP_UNPARK_BD_ADDR_REQ;
                        
            /* 
             * Change state to indicate LC_Unpark should be called 
             * at the end of the next Beacon 
             */ 
            p_link->pol_info.park_info.unpark_pending = PRH_MASTER_INITIATED_UNPARK;   
            g_LM_config_info.park_info.unpark_pending = PRH_MASTER_INITIATED_UNPARK; 
            p_park_info->unpark_pdu_rx = 0x01;
        }
    }
    return NO_ERROR;
}

/**************************************************************************
 *
 * Function :- LMpolicy_LMP_Unpark_Pm_Addr_Req
 * 
 * Description 
 * This function is called when an LMP_UnPark_PmAddr_Req PDU is 
 * received. As this PDU is received in the Beacon Train duplicates 
 * must be discarded. Once the first LMP_UnPark_PmAddr_Req PDU is
 * received then the "p_pending_unpark_link" is set. Any further 
 * LMP_UnPark_PmAddr_Req PDUs are discarded if they are received on this link.
 *
 * The core of the function extracts the parameters from the PDU and 
 * encodes an LMP_Accepted and places it on the Queue.
 * 
 * An unpark pending flag is set which allows the LC to be invoked (to unpark) 
 * at the end of the beacon train.
 * 
 * V1.1 Specification
 * For LMP_unpark_PM_ADDR the AM_ADDR and the PM_ADDR of the
 * 2nd - 7th unparked slaves are optional. If N slaves are unparked, the fields up
 * to and including the Nth unparked slave are present. If N is odd, the AM_ADDR
 * (N+1)th unpark must be zero. The length of the message is x + 3N/2 if N is
 * even and x + 3(N+1)/2 -1 if N is odd, where x = 2 or 4 depending on if the DB is
 * incluDed Or Not
 **************************************************************************/
t_error LMpolicy_LMP_Unpark_Pm_Addr_Req(t_lmp_link *p_link, t_p_pdu p_payload)
{
    t_lm_park_info* p_park_info = &g_LM_config_info.park_info;
    u_int8          match_found;
    u_int8          pdu_addr_field_length;
    u_int8          am_addr = 0;
    u_int8          timing_ctrl_flags;
    u_int           offset;

    /* 
     * The Timing Control and Db are obtained and used irrespective  
     * of whether this device is to be unparked or not. 
     */
    if(p_link->state == LMP_PARK_MODE && g_LM_config_info.state != LMP_ACTIVE)
    {
        /*
         * Extract am_addr and pm_addr field length (assuming no DB)
         */
        pdu_addr_field_length = LMdisp_Get_Dispatched_LMP_PDU_Length() - 2;
        timing_ctrl_flags = *p_payload++;

        /* 
         * The presence of a Db parameter is indicated by the value of timing control
         */
        if (timing_ctrl_flags & 0x01) /* Db Present */
        {
            /* 
             * The flag for initialization 2 is not useful unless DB is present
             * and isthen tied to the master's clock.
             * p_park_info->timing_ctrl =  ((*(p_payload++) & 0x2) >> 1); 
             */
            p_park_info->timing_ctrl =  ((timing_ctrl_flags & 0x2) >> 1);
            g_LM_config_info.D_bcast = LMutils_Get_Uint16(p_payload);
            p_payload+=2;
            pdu_addr_field_length -= 2;
        }

        /* 
         * Scan the PDU for an AM_ADDR/PM_ADDR match. 
         *
         * The number of PM_ADDRs in the PDU is determined from the length of 
         * the PDU excluding the timing_ctrl and Db (if present).
         * The length is x + 3N/2, N even; x + 3(N+1)/2 -1 N odd, where x = 2,4
         * #pm_addr:length_in_bytes = 1:2, 2:3,   3:5, 4:6,   5:8, 6:9,   7:11
         * => num pm_addr = 2*length/3   
         *
         * num_pm_addr_in_pdu = pdu_addr_field_length*2/3;
         */
        
        match_found = FALSE;
        for (offset = 0; offset < pdu_addr_field_length && !match_found; offset+=3)
        {
            am_addr = (u_int8)((*p_payload & 0x07));
            match_found = (am_addr != 0) &&
                (p_link->pol_info.park_info.pm_addr == *(p_payload+1));

            if (!match_found)
            {
                /*
                 * Check for 2nd am_addr/pm_addr pair (am_addr will be non 0)
                 */
                am_addr = (u_int8)((*p_payload & 0x70) >> 4);
                match_found = (am_addr != 0) &&
                    (p_link->pol_info.park_info.pm_addr == *(p_payload+2));
            }
            p_payload += 3;
        }

        /*
         * If not a duplicate message and valid pm_addr/am_addr match found Then
         *    Store the Parameters used for Unparking the device
         */
        if ((p_park_info->unpark_pdu_rx != 0x01) && match_found)
        {
            /* 
             * The p_park_info structure points to the global unpark structure 
             * This is used to store the AM_ADDR allocated for the next link to
             * be unparked
             */
            p_park_info->am_addr = am_addr;
        
            /* 
             * The p_pending_unpark_link is used to identify the next link to be used
             * to invoke LC_Unpark 
             */
            p_pending_unpark_link = p_link;
            
            /* 
             * Create and Queue the LMP Accepted message        
             * If the LC unpark fails this message is removed from the Q.
             */
            p_link->current_proc_tid_role = p_link->tid_role_last_rcvd_pkt;
            p_pending_unpark_link->pol_info.park_info.unpark_type = LMP_UNPARK_PM_ADDR_REQ;
            
#if (PRH_BS_DEV_SLAVE_FORCE_STOP_AUTOPARK_SUPPORTED==1)
            if((p_link->pol_info.park_info.unpark_pending == PRH_SLAVE_INITIATED_UNPARK) &&
               (p_park_info->park_state == LMP_BEACON_ACTIVE))
            {
                /*
                 * Collision of master initiated unpark when slave initated exit park.
                 *
                 * ProblemA:
                 * Cannot allow master initated unpark to override slave iniated unpark,
                 * as if master is autoparking, will continue to autopark.
                 * ProblemB:
                 * Cannot allow slave initiated unpark to block master initiated unpark,
                 * as will not interop with CSR (deadlocks as CSR master blocks also).
                 *
                 * Solution:
                 * Override slave initiated unpark by master initiated unpark but force
                 * to stop autoparking on next park req from master by pdu not allowed.
                 *
                 */
                p_park_info->unpark_pending = PRH_NO_UNPARK_PENDING;
                LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_UNPARK_PENDING);

                p_link->force_stop_autopark = 1;
                LMtmr_Set_Timer((((u_int32)g_LM_config_info.T_bcast)<<3),
                      &_LMpolicy_Unforce_Stop_Autopark, p_link, 1);
            }
#endif

            /* 
             * Change state to indicate LC_Unpark should be called 
             * at the end of the next Beacon
             */
            if (p_park_info->unpark_pending != PRH_SLAVE_INITIATED_UNPARK)
            {
                 p_park_info->unpark_pending = PRH_MASTER_INITIATED_UNPARK;
                 p_link->pol_info.park_info.unpark_pending = PRH_MASTER_INITIATED_UNPARK;
                 p_link->pol_info.park_info.next_unpark_timer =  BTtimer_Set_Slots(0);
                _LMpolicy_Insert_UnPark_Link(p_link, PARK_NON_AUTO, MASTER);
            }

            p_park_info->unpark_pdu_rx = 0x01;

        }
    }
    return NO_ERROR;
}


#if (PRH_BS_DEV_SLAVE_FORCE_STOP_AUTOPARK_SUPPORTED==1)
static void _LMpolicy_Unforce_Stop_Autopark(t_lmp_link* p_link)
{
    LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_PARK_PENDING|PRH_DEV_STATE_UNPARK_PENDING);
    p_link->force_stop_autopark = 0;
}
#endif


/**************************************************************************
 * FUNCTION :- LMpolicy_Derive_Park_Parameters
 *
 * DESCRIPTION :- 
 * Thus function derives the park parameters for the initiator
 * of park. The parameters are currently the defaults.
 **************************************************************************/
static t_error LMpolicy_Derive_Park_Parameters(t_role role, 
    t_lm_park_info *p_park_info, u_int16 max_interval, u_int16 min_interval,
    t_deviceIndex device_index)
{
    if((max_interval == min_interval) && (min_interval & 1))
        return INVALID_LMP_PARAMETERS;

    /* 
     * Check if Park Link already exists and ensure beacons Match 
     */
    if ( g_LM_config_info.num_links_parked || 
         LMpolicy_Are_Links_Being_Parked_UnParked())
    {
        if ((max_interval <  g_LM_config_info.T_bcast) ||
            (min_interval >  g_LM_config_info.T_bcast))
        {
            return INVALID_LMP_PARAMETERS;
        }
    }
    else
    {
        g_LM_config_info.T_bcast = min_interval;
    }


    p_park_info->timing_ctrl = mLMpol_Get_Timing_Ctrl(device_index);
    
    if (g_LM_config_info.num_links_parked == 0)
    {
        if(!g_LM_config_info.park_defaults)
        {
            g_LM_config_info.D_bcast = PRH_DEFAULT_D_BEACON;
            g_LM_config_info.N_bcast = PRH_DEFAULT_N_BEACON; 
            g_LM_config_info.park_info.delta_bcast = PRH_DEFAULT_DELTA_BEACON;
            g_LM_config_info.N_sleep = PRH_DEFAULT_N_B_SLEEP; 
            g_LM_config_info.D_sleep = PRH_DEFAULT_D_B_SLEEP;
            g_LM_config_info.D_access = PRH_DEFAULT_D_ACCESS;
            g_LM_config_info.T_access = PRH_DEFAULT_T_ACCESS;
            g_LM_config_info.N_access = PRH_DEFAULT_N_ACCESS;
            g_LM_config_info.N_poll = PRH_DEFAULT_N_POLL;
            g_LM_config_info.M_access = PRH_DEFAULT_M_ACCESS;
            g_LM_config_info.access_scheme = 0;
            g_LM_config_info.park_defaults = 1;
        }
        
        if(max_interval < (g_LM_config_info.D_access +
             (g_LM_config_info.T_access*g_LM_config_info.M_access)))
        {
            /*
             * When requested park beacon interval is shorter than can be used with
             * master park defaults, adjust N_bcast and D_access paramaters to fit.
             */
            g_LM_config_info.park_info.delta_bcast = 2;
			g_LM_config_info.N_bcast = DEFAULT_NBC;

            g_LM_config_info.D_access = (g_LM_config_info.N_bcast)*
                (g_LM_config_info.park_info.delta_bcast) + _LM_MIN_TBRP_DACC_GUARD;

			if ((max_interval - g_LM_config_info.D_access) < PRH_DEFAULT_T_ACCESS)
			{
                g_LM_config_info.T_access = max_interval - g_LM_config_info.D_access;
			}

			g_LM_config_info.M_access = 1;

            if (g_LM_config_info.T_access < (2*(g_LM_config_info.N_access)+6))
            {
                g_LM_config_info.N_access = (g_LM_config_info.T_access > 10)?
                    ((((g_LM_config_info.T_access-6)/2))&~0x01):(2);
            }

			g_LM_config_info.park_defaults = 0;
        }
    }

    if (g_LM_config_info.T_bcast <= g_LM_config_info.D_bcast)
    {
        g_LM_config_info.D_bcast = 0;
    }
    
    p_park_info->delta_bcast = g_LM_config_info.park_info.delta_bcast;
    /*
     * If peer device asks IUT to park for an odd number of slots,
     * the IUT must park for an even number of slots (as beacon
     * interval is frame-based (master will always start the
     * beacon in a master TX slot).
     *
     * There is no way of knowing if IUT should round down the
     * beacon interval or round up the beacon interval, e.g.
     * if asked to use a beacon with an interval 9 slots, should IUT expect to 
     * use a beacon with an interval of 8 slots or an interval of 10 slots?
     */
    return NO_ERROR;
}

/**************************************************************************
 * FUNCTION :- LMpolicy_Extract_Park_Parameters()
 *
 * DESCRIPTION :- 
 * This function extracts the park parameters from an incoming
 * LMP_Park PDU.
 **************************************************************************/
static t_error LMpolicy_Extract_Park_Parameters(  
    t_role role, t_lm_park_info *p_park_info, t_p_pdu p_payload, u_int8 opcode)
{
    u_int8 no_access_win;
    u_int16 T_b;
    u_int16 D_b;
    u_int8  N_b,T_a,N_a,D_a,N_s = 0,D_s = 0;
    u_int8  delta_bcast, pm_addr =0;
	u_int8  ar_addr = 0, N_poll = 0 , M_access = 0, access_scheme = 0;
    u_int8  timing_ctrl;
    u_int8  Db_present;

    no_access_win = ((*(p_payload) & 0x04) >> 2);
    Db_present = (*(p_payload) & 0x01);
    timing_ctrl = ((*(p_payload++) & 0x2) >> 1);
    
    if ((Db_present) || (opcode != LMP_MODIFY_BEACON))
    {
        D_b = LMutils_Get_Uint16(p_payload);
        p_payload+=2;
    }
    else
    {
        D_b = 0;
    }
    
    T_b = LMutils_Get_Uint16(p_payload);
    p_payload+=2;
    N_b = *(p_payload++);
    
    delta_bcast = *(p_payload++);
    
    if (opcode != LMP_MODIFY_BEACON)
    {
        pm_addr = *(p_payload++);
        ar_addr = *(p_payload++);
        
        N_s = *(p_payload++);
        D_s = *(p_payload++);
#if 1 /* latest TTCN defaults these to zero => need to adjust */
        if((N_s == 0) && (D_s == 0)) N_s = 1;
#endif
    }
    if (!no_access_win)
    {
        D_a = *(p_payload++);
        T_a = *(p_payload++);
        N_a = *(p_payload++);
    }
    else
    {
        D_a = 0;
        T_a = 0;
        N_a = 0;
        p_payload +=3;
    }

    N_poll = *(p_payload++);

    if (!no_access_win)
        M_access = (*p_payload) & 0x0F; 
    else
        M_access = 0;
    
    access_scheme = (*p_payload) & 0xF0; 
    
    if ((((D_a+2) & 1) != 0) || (((T_a+2) & 1)!= 0))
        return INVALID_LMP_PARAMETERS;
#if 0 /* NOTE IT IS SUGGESTED THAT T_Access >= 2 * N_access 
       * Any park request not satisfying this should be rejected 
       * NOTE this deviates from conformance BUT allows more interopability.
       */

    if (T_a < (2*N_a))
        return INVALID_LMP_PARAMETERS;
#endif
    if (D_a > T_b)
        return INVALID_LMP_PARAMETERS;
    if ((opcode != LMP_MODIFY_BEACON) && (N_s <= D_s))
        return INVALID_LMP_PARAMETERS;
    if (((N_poll+2) & 1)!=0)
        return INVALID_LMP_PARAMETERS;
    if (access_scheme != 0)
        return INVALID_LMP_PARAMETERS;
    if ((N_b == 0) || (T_b == 0) || (T_b & 1) || ((D_b+2) & 1) || (N_b > T_b)) 
        return INVALID_LMP_PARAMETERS;
    if ((role == SLAVE) && ((D_a + T_a) > T_b))
        return INVALID_LMP_PARAMETERS;
    if (T_b <= D_b)
        return INVALID_LMP_PARAMETERS;
    
    if (role == SLAVE)
    {
        p_park_info->timing_ctrl = timing_ctrl;
        if ((Db_present) || (opcode != LMP_MODIFY_BEACON))
            g_LM_config_info.D_bcast = D_b;
        g_LM_config_info.N_bcast = N_b;
        p_park_info->delta_bcast = delta_bcast;
        if (opcode != LMP_MODIFY_BEACON)
        {
            p_park_info->pm_addr = pm_addr;
            p_park_info->ar_addr = ar_addr;
            g_LM_config_info.N_sleep = N_s;
            g_LM_config_info.D_sleep = D_s;
        }
        g_LM_config_info.D_access = D_a;
        g_LM_config_info.T_access = T_a;
        g_LM_config_info.N_access = N_a;
        g_LM_config_info.N_poll = N_poll;
        g_LM_config_info.M_access = M_access; 
        g_LM_config_info.access_scheme = access_scheme;
        g_LM_config_info.park_defaults = 0;
        g_LM_config_info.T_bcast = T_b;
    }
    else /* Im a master */
    {
        /* In the Master the proposed T_b can only be assigned if there are
         * no other devices in park mode.
         */

        if ((g_LM_config_info.num_links_parked == 0) && 
            !LMpolicy_Are_Links_Being_Parked_UnParked())
        {
            g_LM_config_info.T_bcast = T_b;
            if(g_LM_config_info.T_bcast < PRH_BS_CFG_MIN_MASTER_PARK_BEACON_INTERVAL)
            {
                g_LM_config_info.T_bcast = PRH_BS_CFG_MIN_MASTER_PARK_BEACON_INTERVAL;
            }
        }
    }
    return NO_ERROR;
}

/**************************************************************************
 * Function :- _LMpolicy_Enter_Park_Mode
 *
 * Description 
 * Device enters park mode at the Link Manager Level. It sets
 * Up the Device Parameters for the beacon in a Slave, resets
 * the number of negotiations. 
 * The LC_Park_Request is invoked. 
 *
 * Note : The device does not enter park mode completely until
 * The LC invokes "LMpolicy_Enter_Park_Mode_Complete".
 **************************************************************************/
static void _LMpolicy_Enter_Park_Mode(t_lmp_link* p_link)
{
    t_clock current_clock;

    /* Read the clock for the link Native or Piconet */

    if (p_link->role == SLAVE)
    {
        current_clock = LC_Get_Piconet_Clock(p_link->device_index);
    }
    else
    {
        current_clock = LC_Get_Native_Clock();
    }

    p_link->state = LMP_PARK_MODE;

    /* Clear the enter park mode timer */
    LMtmr_Clear_Timer(p_link->pol_info.park_info.park_timeout_index);
    p_link->pol_info.park_info.park_timeout_index = 0;

    if (((p_link->role == MASTER) && (g_LM_config_info.num_links_parked == 0)) || 
        ( p_link->role == SLAVE))
    {
        g_LM_config_info.park_info.timing_ctrl = p_link->pol_info.park_info.timing_ctrl; 
    }
    g_LM_config_info.park_info.delta_bcast = p_link->pol_info.park_info.delta_bcast;
    
    p_link->pol_info.park_info.unpark_pdu_rx = 0;

    if (g_LM_config_info.num_links_parked == 0)
    {
        g_LM_config_info.beacon_instant = 
            BTtimer_Set_Timer_On_Timing_Equation(
                current_clock,
                p_link->pol_info.park_info.timing_ctrl,
                (g_LM_config_info.N_sleep * g_LM_config_info.T_bcast), 
                ((t_slots)(g_LM_config_info.D_bcast + 
                    (t_slots)(g_LM_config_info.D_sleep * g_LM_config_info.T_bcast))));

#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
        if (LMconfig_LM_Connected_As_Scatternet())
        {
            /*
             * Set timer to switch back to this piconet before first beacon instant.
             */
			LMpol_Adjust_Wakeup_Slot_Time(p_link,(g_LM_config_info.beacon_instant>>1));
        }
#endif

        if (p_link->role == SLAVE)
        {
            g_LM_config_info.park_info.park_state = LMP_BEACON_INACTIVE;
        }
        else if (p_link->role == MASTER)
        {
			g_LM_config_info.park_info.park_state = LMP_BEACON_INACTIVE;

            g_LM_config_info.start_acc_window = 
                BTtimer_Piconet_Reset_Delta_Slots(
                    g_LM_config_info.beacon_instant,
                    current_clock,
                    g_LM_config_info.D_access - 2 );

            g_LM_config_info.next_acc_window = g_LM_config_info.start_acc_window;

            g_LM_config_info.end_acc_window = 
                BTtimer_Piconet_Reset_Delta_Slots(
                    g_LM_config_info.beacon_instant, 
                    current_clock,
                    (t_slots)(g_LM_config_info.D_access + 
                        (t_slots)(g_LM_config_info.T_access*g_LM_config_info.M_access )));
        }
    }
    if (p_link->role == MASTER)
    {
        /* 
         * Remove the entry from the list of links pending park  
         */
        _LMpolicy_Remove_Park_Link(p_link);
        if (p_link->automatic_park_enabled)
        {
        /* Bug Fix :- for autopark with an infinite Link Supervision TO
            */
            if (p_link->supervision_timeout != 0)
            {
                p_link->pol_info.park_info.next_unpark_timer =  
                    BTtimer_Set_Slots(p_link->supervision_timeout/2);
            }
            else /* Infinite Link Supervision Timeout */
            {
                p_link->pol_info.park_info.next_unpark_timer =  
                    BTtimer_Set_Slots(0xC000 /*30 Seconds*/);
            }
            
            _LMpolicy_Insert_UnPark_Link(p_link, PARK_AUTO, p_link->role);
        }
    }
    g_LM_config_info.state = LMP_PARK_MODE;
    g_LM_config_info.park_info.unpark_pending = PRH_NO_UNPARK_PENDING;
    g_LM_config_info.park_info.device_index = p_link->device_index;

    g_LM_config_info.links_in_low_power++;

    p_link->num_park_negotiations = 0;
    /*
     * Note :- It is assumed that the LC frees the AM ADDR 
     */
    LC_Park_Request(p_link->device_index, p_link->pol_info.park_info.pm_addr);
    /* 
     * Only change State after the LC has gone parked 
     */
    LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_PARK_PENDING);    
}

/**************************************************************************
 * Function :- LMpolicy_Enter_Park_Mode_Complete
 *
 * Description 
 * Called by the LC when the LC has entered park for the device.
 **************************************************************************/
static void LMpolicy_Enter_Park_Mode_Complete_Handler(t_lmp_link *p_link_event_info);

t_error LMpolicy_Enter_Park_Mode_Complete(t_LC_Event_Info* p_lc_event_info)
{
    LM_DEFER_FROM_INTERRUPT_CONTEXT(LMpolicy_Enter_Park_Mode_Complete_Handler,
            (t_lmp_link *)p_lc_event_info);
    return NO_ERROR;
}

static void LMpolicy_Enter_Park_Mode_Complete_Handler(t_lmp_link *p_link_event_info)
{
    t_LC_Event_Info* p_lc_event_info = (t_LC_Event_Info*)p_link_event_info;    

    t_lmp_link* p_link;

    p_link = LMaclctr_Find_Device_Index(p_lc_event_info->deviceIndex);
    p_link->park_device_index = p_lc_event_info->parkDeviceIndex;
    g_LM_config_info.park_info.park_device_index = p_link->park_device_index;

    p_link->state = LMP_PARK_MODE;

    if (p_lc_event_info->status == NO_ERROR)
    {
        if(p_link->role == SLAVE)      
        {
            g_LM_config_info.state = LMP_PARK_MODE;
        }
        g_LM_config_info.num_links_parked++;

#if (PRH_BS_DEV_EXTENDED_QOS_NOTIFICATIONS_SUPPORTED==1)
        /*
         * Clear potential LMP_QOS ACK pending for this device index.
         */
        LMqos_Clear_LMP_QOS_Notification_Ack_Pending(p_lc_event_info->deviceIndex);
#endif

#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
        /*
         * Save the roll of the piconet in park, used to check resource availability.
         */
        g_LM_config_info.role_in_park = p_link->role;
#endif

    }
    else
    {
        if(p_link->role == SLAVE)
        {
            g_LM_config_info.state = LMP_ACTIVE;
        }
        else
        {
            p_link->state = LMP_ACTIVE; 
        }
    }

    if (p_link->pol_info.park_info.mode_change)
    {
        _Send_HC_Mode_Change_Event(p_link,
            g_LM_config_info.T_bcast, p_lc_event_info->status);
    }

}

/**************************************************************************
 * Function :- LMpolicy_Exit_Park_Mode_Complete
 *
 * Description 
 * When the LC has finished unparking this function is called
 * resulting in a commit of the underlying LC.
 *
 * NOTE :- It was originally planned that the LC_UnPark_Commit
 * or LC_UnPark_Rollback would be called as a result of receiving
 * the LMP_Accept ( or a timeout ). However, it seems the LC UnPark
 * procedure does not activate the channel until after the LC_UnPark_Commit.
 * Thus there is NO WAY the LMP_ACCEPT can be received prior to calling
 * LC_UnPark_Commit. 
 *
 * Thus the LC_UnPark_Commit/Rollback can only be called from this
 * function. This function need to be decoupled from  the  
 **************************************************************************/
static void LMpolicy_Exit_Park_Mode_Complete_Handler(t_lmp_link *p_link_event_info);

t_error LMpolicy_Exit_Park_Mode_Complete(t_LC_Event_Info* p_lc_event_info)
{ 
    /* 
     * The pending_unpark_link has been set up on the receipt of 
     * a HCI_Exit_Park or an LMP_UNPARK PDU
     */
     if (p_pending_unpark_link==0)
     {
         return UNSPECIFIED_ERROR;
     }

     LM_DEFER_FROM_INTERRUPT_CONTEXT(LMpolicy_Exit_Park_Mode_Complete_Handler,
        (t_lmp_link *)p_lc_event_info);

    return NO_ERROR;
}

static void LMpolicy_Exit_Park_Mode_Complete_Handler(t_lmp_link *p_link_event_info)
{
     t_LC_Event_Info* p_lc_event_info = (t_LC_Event_Info*)p_link_event_info;

     if (p_pending_unpark_link->role==SLAVE)
     {
        if (p_lc_event_info->status == NO_ERROR)
        {
            /* 
             * If the LC succeeded in UNPARKING the LM Commits the unpark.
             * and decrements the number of links parked and in low power mode
             */
            t_lmp_pdu_info pdu_info;
            LC_Unpark_Commit();
            g_LM_config_info.num_links_parked--;
            LMpol_Decr_Links_In_Low_Power();
            g_LM_config_info.park_info.unpark_pending = PRH_NO_UNPARK_PENDING;
            /* 
             * Enqueue the LMP_Accepted to the LMP_UNPARK_REQ 
             */
            pdu_info.tid_role = 0;
            pdu_info.opcode = LMP_ACCEPTED;                     
            pdu_info.return_opcode = p_pending_unpark_link->pol_info.park_info.unpark_type;
            LM_Encode_LMP_PDU(p_pending_unpark_link->device_index,&pdu_info);
            p_pending_unpark_link = 0;            
            LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_UNPARK_PENDING);
        }
        else
        {
            if (g_LM_config_info.park_info.unpark_pending != PRH_NO_UNPARK_PENDING)
            {
                LC_Unpark_Rollback();
                
                p_pending_unpark_link->state = LMP_PARK_MODE;
                /* 
                 * If im a slave and the unpark fails I continue to unpark                 
                 * If the slave needs to continue to unpark ensure
                 * that the device state is still park mode  
                 */
                g_LM_config_info.state = LMP_PARK_MODE;
                g_LM_config_info.park_info.unpark_pdu_rx = 0;
                LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_UNPARK_PENDING);

                /* 
                 * Only If the original Unpark was Slave initiated is an other 
                 * unpark Queued up.
                 */
                if(g_LM_config_info.park_info.unpark_pending == PRH_SLAVE_INITIATED_UNPARK)
                {
                    p_pending_unpark_link->pol_info.park_info.next_unpark_timer =  
                        BTtimer_Set_Slots(0);
                    /*
                     * Schedule Automatic Unpark if not mode change
                     */
                    _LMpolicy_Insert_UnPark_Link(p_pending_unpark_link,
                        (u_int8) (p_pending_unpark_link->pol_info.park_info.mode_change==0), 
                        SLAVE);
                }
            }
        }
    } /* Master */
    else
    {
        /* 
         * If the LC succeeded in UNPARKING the LM Commits the unpark.
         * and decrements the number of links parked and in low power mode
         */
        if (p_lc_event_info->status == NO_ERROR)
        {
            g_LM_config_info.num_links_parked--;
            p_pending_unpark_link->pol_info.park_info.num_unparks_attempts=0;
            LMpol_Decr_Links_In_Low_Power();
            if (g_LM_config_info.park_info.unpark_pending != PRH_NO_UNPARK_PENDING)
            {
                /*
                 * Calculate a suitable timeout based on T_bcast and default poll interval.
                 * PRH_DEFAULT_LMP_TRANSACTION_TIMEOUT is not suitable here.
                 */
                p_pending_unpark_link->poll_interval = 2;
                p_pending_unpark_link->poll_position = 2;
                p_pending_unpark_link->pol_info.park_info.unpark_timeout_index = LMtmr_Set_Timer((g_LM_config_info.T_bcast/2)
                    + g_LM_config_info.T_bcast*((DEFAULT_T_POLL*4)/(g_LM_config_info.T_bcast)),
                    LMpolicy_Unpark_Timeout,p_pending_unpark_link, 1);
                LC_Unpark_Commit();
                g_LM_config_info.park_info.unpark_pending = PRH_NO_UNPARK_PENDING;    
            }
            else
            {
                LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_UNPARK_PENDING);
                LC_Unpark_Commit();
                p_pending_unpark_link = 0;
            }
        }
        else
        {
            if (g_LM_config_info.park_info.unpark_pending != PRH_NO_UNPARK_PENDING)
            {
                LC_Unpark_Rollback();
                p_pending_unpark_link->state = LMP_PARK_MODE;
                p_pending_unpark_link->pol_info.park_info.num_unparks_attempts--;
                if ((p_pending_unpark_link->pol_info.park_info.num_unparks_attempts) &&
                    (g_LM_config_info.park_info.unpark_pending != PRH_SLAVE_INITIATED_UNPARK))
                {
                    g_LM_config_info.park_info.unpark_pending = PRH_NO_UNPARK_PENDING;

                    /* 
                     * If a re-try is to be performed, it is scheduled for 
                     * T-bcast in the future. [ i.e the park interval T-b ].
                     * Expire timer before next T_bcast so unpark attempts on 
                     * successive beacons, overcomes interop issues where slave
                     * listens on only odd/even beacons due
                     * to mis-interpretation of N-Bsleep=1 paramater value.
                     */
                    p_pending_unpark_link->pol_info.park_info.next_unpark_timer = 
                        BTtimer_Set_Slots(0);

                    /*
                     * Schedule Automatic Unpark if not mode change
                     */
                    _LMpolicy_Insert_UnPark_Link(p_pending_unpark_link,
                        (u_int8)(p_pending_unpark_link->pol_info.park_info.mode_change==0), 
                        MASTER);
                }
                else 
                {
                    /* 
                     * Change State Back to LMP Park and send a mode change event 
                     */
                    g_LM_config_info.park_info.unpark_pending = PRH_NO_UNPARK_PENDING;
                    p_pending_unpark_link->pol_info.park_info.unpark_pending = 
                        PRH_NO_UNPARK_PENDING;
                    _LMpolicy_Remove_UnPark_Link(p_pending_unpark_link);
                    if (p_pending_unpark_link->pol_info.park_info.mode_change)
                    {
                        _Send_HC_Mode_Change_Event(p_pending_unpark_link, 
                            g_LM_config_info.T_bcast, p_lc_event_info->status);
                    }
                    p_pending_unpark_link->pol_info.park_info.num_unparks_attempts=0;
                }
                LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_UNPARK_PENDING);
            }
        }
    }
}

/**************************************************************************
 * Function :- LMpolicy_Slave_Init_Unpark
 *
 * Description 
 * Called by the LC when it has received an Access request from the slave
 * Ignores request if currently unparking or no matching p_link
 *
 * Returns
 * NO_ERROR
 **************************************************************************/
static void LMpolicy_Slave_Init_Unpark_Handler(t_lmp_link *p_link);
static t_error LMpolicy_Slave_Init_Unpark(t_LC_Event_Info* p_lc_event_info)
{
    extern t_SYS_Config g_sys_config;
    extern t_LMconfig g_LM_config_info;

    u_int8 ar_addr;
    t_lmp_link *p_link;

    if((g_sys_config.max_active_devices_in_piconet > g_LM_config_info.num_master_acl_links)
        && (g_LM_config_info.park_info.unpark_pending != PRH_SLAVE_INITIATED_UNPARK))
    {
        /* 
         * Not currently performing unpark, hence
         *  Find the p_link corresponding to ar_addr
         *  Note the AR_ARRR is contained in the device index field 
         */
        ar_addr = (u_int8) p_lc_event_info->deviceIndex;
        p_link = LMaclctr_Find_ARADDR(ar_addr);
        if (p_link)
        {
            if (p_link->role == MASTER)
            {
                LMtmr_Clear_Timer(p_link->pol_info.park_info.park_timeout_index);
		p_link->pol_info.park_info.park_timeout_index = 0;
                p_link->automatic_park_enabled = 0;
            }

            LM_DEFER_FROM_INTERRUPT_CONTEXT(LMpolicy_Slave_Init_Unpark_Handler, p_link);

            p_link->pol_info.park_info.unpark_pending = 
                PRH_SLAVE_INITIATED_UNPARK;   
            g_LM_config_info.park_info.unpark_pending = 
                PRH_SLAVE_INITIATED_UNPARK;
        }
    }
    return NO_ERROR;
}

static void LMpolicy_Slave_Init_Unpark_Handler(t_lmp_link *p_link)
{
    _LMpol_Execute_Unpark_Request(p_link, PARK_NON_AUTO, SLAVE);
}

/**************************************************************************
 * Function :- LMpolicy_Access_Window_Complete
 *
 * Description 
 * Called by the LC when the access windows are completed.
 **************************************************************************/
static t_error LMpolicy_Access_Window_Complete(t_LC_Event_Info* p_lc_event_info)
{
    t_lm_park_info* p_park = &g_LM_config_info.park_info;
    if (p_park->unpark_pdu_rx == 0x01)
    {
        p_park->park_state = LMP_UNPARK_AFTER_ACCESS_WINDOW;
    }
    else
    {
        p_park->park_state = LMP_BEACON_INACTIVE;
    }
    return NO_ERROR;
}

/**************************************************************************
 * Function :- _LMpolicy_Send_LMP_Park_Req
 *
 * Description 
 * _Sends an LMP_Park_Req to the peer.
 **************************************************************************/
static void _LMpolicy_Send_LMP_Park_Req(t_lmp_link *p_link, t_lm_park_info *p_park_info)
{
    t_lmp_pdu_info  pdu_info;

    LMch_Disable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_PARK);

    pdu_info.opcode = LMP_PARK_REQ;

    pdu_info.ptr.p_park = p_park_info;
    pdu_info.tid_role = p_link->current_proc_tid_role;
#if (PRH_BS_DEV_MASTER_PARK_REQ_ACK_TIMER==1)
    pdu_info.ack_required = ((p_link->role == MASTER) && (p_link->automatic_park_enabled));
#endif
    LM_Encode_LMP_PDU(p_link->device_index, &pdu_info);

    p_link->num_park_negotiations++;

}

/**************************************************************************
 * Function :- _Send_LMP_Modify_Beacon
 *
 * Description 
 * _Sends an LMP_Modify_Beacon to the peer.
 **************************************************************************/
/* Unused 
static void _Send_LMP_Modify_Beacon(t_lm_park_info *p_park_info)
{
    t_lmp_pdu_info  pdu_info;

    pdu_info.ptr.p_park = p_park_info;
    pdu_info.opcode = LMP_MODIFY_BEACON;
    pdu_info.Db_present = 0;
    pdu_info.tid_role = 0 ;
    LM_Encode_LMP_PDU(PRH_PARK_BROADCAST_Q, &pdu_info);
}
*/  

/**************************************************************************
 * Function :- _Send_LMP_Set_Broadcast_Scan_Window
 *
 * Description 
 * _Sends an LMP_Set_Broadcast_Scan_Window to the peer.
 **************************************************************************/
void _Send_LMP_Set_Broadcast_Scan_Window(void)
{
    t_lmp_pdu_info  pdu_info;

    pdu_info.opcode = LMP_SET_BROADCAST_SCAN_WINDOW;
    pdu_info.timing_control = g_LM_config_info.park_info.timing_ctrl;
    pdu_info.Db_present = 0;
    pdu_info.tid_role = 0 ;     

    LM_Encode_LMP_PDU(PRH_PARK_BROADCAST_Q, &pdu_info);
}

/**************************************************************************
 * Function :- _LMpolicy_Get_Next_Unpark_Link
 *
 * Description 
 * Retrieves the next link which is to be unparked. Returns NULL
 * if the device is already Parking or UnParking a link.
 **************************************************************************/
static t_lm_park_entry_info* _LMpolicy_Get_Next_Unpark_Link(void)
{
    t_lmp_link* p_link;
    u_int8 i;

    /* 
     * Search the Pending Unpark Links. To determine if any of the
     * Unpark timers have expired.
     */
    if (!LMconfig_LM_Check_Device_State(
        PRH_DEV_STATE_UNPARK_PENDING|PRH_DEV_STATE_PARK_PENDING))
    {
        for(i=0;i < PRH_BS_CFG_MAX_PARK_LINKS; i++)
        {
            p_link = _LMpol_Pending_Unpark_Links[i].p_link;
            if  ((p_link != 0) &&
                (BTtimer_Is_Expired(p_link->pol_info.park_info.next_unpark_timer)))
            {
                /*
                 * Immediate return of link to be unparked.
                 */
                return &_LMpol_Pending_Unpark_Links[i];
            }
        }
    }
    return 0;
}

/**************************************************************************
 * Function :- LMpolicy_Service_Park_Links
 *
 * Description 
 * Services the containers of links pending Park or Unpark
 * Returns immediately if device is already Parking or Unparking a link.
 * 
 **************************************************************************/
void LMpolicy_Service_Park_Links(void)

{
    static t_lm_park_entry_info* p_park_entry;

    /* 
     * As unpark is more critical than parking 
     * then we service the unpark array first
     */
    if( !LMconfig_LM_Check_Device_State(
         PRH_DEV_STATE_UNPARK_PENDING|PRH_DEV_STATE_PARK_PENDING) )
     {
     
#if (PRH_BS_DEV_ACL_DRIVEN_AUTOPARK_SUPPORTED==1)
        /* 
         * If the device is a slave and has data pending then it should be 
         * queued up for an UnPark.
         */
        t_lmp_link* p_link;
        u_int8 i;
        for(i=0; i<PRH_MAX_ACL_LINKS; i++)
        { 
           /* 
            * Cycle through the link_container to find all the 
            * active links on which the device is master 
            */
            p_link = &(link_container[i].entry);
            if((p_link != 0) && (LMP_PARK_MODE == p_link->state) &&
               (p_link->pol_info.park_info.unpark_pending == PRH_NO_UNPARK_PENDING))
            {     
                if ((!BTq_Is_Queue_Empty(L2CAP_OUT_Q, p_link->device_index)) ||
                    (!BTq_Is_Queue_Empty(LMP_OUT_Q, p_link->device_index)))
                {
                    /*
                     * Schedule an immediate unpark followed by a timed PARK
                     */
                    p_link->pol_info.park_info.next_unpark_timer =  BTtimer_Set_Slots(0);
                    _LMpolicy_Insert_UnPark_Link(p_link, PARK_AUTO, p_link->role);

                    p_link->pol_info.park_info.next_park_timer =
                        BTtimer_Set_Slots(2*g_LM_config_info.T_bcast);
                    _LMpolicy_Insert_Park_Link(p_link, PARK_AUTO, p_link->role);
                }
            }
        }
#endif

        p_park_entry = _LMpolicy_Get_Next_Unpark_Link();
        if(p_park_entry && p_park_entry->p_link)
        {
            _LMpol_Execute_Unpark_Request(p_park_entry->p_link, 
                p_park_entry->automatic, (t_role)(p_park_entry->initiator));
            _LMpolicy_Remove_UnPark_Link(p_park_entry->p_link);
        }
        else
        {
            p_park_entry = _LMpolicy_Get_Next_Park_Link();
            if (p_park_entry && p_park_entry->p_link)
            {
                _LMpolicy_Execute_Park_Request(p_park_entry->p_link,
                    p_park_entry->automatic, (t_role)(p_park_entry->initiator));
                _LMpolicy_Remove_Park_Link(p_park_entry->p_link);
            }
        }
    }
}

/**************************************************************************
 * Function :- _LMpolicy_Get_Next_Park_Link
 *
 * Description 
 * Retrieves the next link which is to be parked. Returns NULL
 * if the device is already Parking or UnParking a link.
 **************************************************************************/
static t_lm_park_entry_info* _LMpolicy_Get_Next_Park_Link(void)
{
    t_lmp_link* p_link;
    u_int8 i;

    if(!LMconfig_LM_Check_Device_State(
        PRH_DEV_STATE_UNPARK_PENDING|PRH_DEV_STATE_PARK_PENDING))
    {
        for(i=0;i < PRH_BS_CFG_MAX_PARK_LINKS; i++)
        {
            p_link = _LMpol_Pending_Park_Links[i].p_link;
            if  ((p_link != 0) &&
                (BTtimer_Is_Expired(p_link->pol_info.park_info.next_park_timer)))
            {
                /*
                 * Immediate return of link to be parked.
                 */
                return &_LMpol_Pending_Park_Links[i];
            }
        }
    }
    return 0;
}

/**************************************************************************
 * Function :- _LMpolicy_Insert_UnPark_Link
 *
 * Description 
 * Inserts a link which has to be unparked in the future in the
 * container of links to be unparked.
 **************************************************************************/
static void _LMpolicy_Insert_UnPark_Link(
    t_lmp_link* p_link, u_int8 automatic, t_role role)
{
    u_int8 i=0;

    /* 
     * Scan the container to see if this list is already scheduled for park 
     */
    while((i != PRH_BS_CFG_MAX_PARK_LINKS) && 
         (_LMpol_Pending_Unpark_Links[i].p_link!=p_link))
    {
        i++;
    }
   
    /* 
     * The link is not in the container so this is a new entry. 
     * We find the first free entry in the container and insert the link 
     */
    if (i == PRH_BS_CFG_MAX_PARK_LINKS)
    {
        i= 0;
        while((i != PRH_BS_CFG_MAX_PARK_LINKS) && 
            (_LMpol_Pending_Unpark_Links[i].p_link!=0))
        {
            i++;
        }
        if  (i!=PRH_BS_CFG_MAX_PARK_LINKS)
        {
            _LMpol_Pending_Unpark_Links[i].p_link = p_link;
            _LMpol_Pending_Unpark_Links[i].automatic = automatic;
            _LMpol_Pending_Unpark_Links[i].initiator = role;
        }
    }
    else
    {
       /* 
        * The link is already in the container, so we just change the existing 
        * entry.  The p_link entry does not change so we just assign the 
        * Automatic and Role fields.
        */
       _LMpol_Pending_Unpark_Links[i].automatic = automatic;
       _LMpol_Pending_Unpark_Links[i].initiator = role;
    }

#if defined(SYS_HAL_OS_EVENT_DRIVEN_HOST_CONTROLLER)
    BTms_OS_Post(BTMS_OS_EVENT_SERVICE_PARK_LINKS);
#endif

}

/**************************************************************************
 * Function :- _LMpolicy_Insert_Park_Link
 *
 * Description 
 * Inserts a link which has to be parked in the future in the
 * container of links to be unparked.
 **************************************************************************/
static void _LMpolicy_Insert_Park_Link(
    t_lmp_link* p_link, u_int8 automatic, t_role role)
{
    u_int8 i=0;

    /* 
     *Scan the container to see if this list is already scheduled for park 
     */
    while((i != PRH_BS_CFG_MAX_PARK_LINKS) && 
         (_LMpol_Pending_Park_Links[i].p_link!=p_link))
    {
        i++;
    }

    if (i == PRH_BS_CFG_MAX_PARK_LINKS)
    {
        /* 
         * The link is not in the container so this is a new entry. 
         * We find the first free entry in the container and insert the link 
         */
        i= 0;
        while((i != PRH_BS_CFG_MAX_PARK_LINKS) && 
           (_LMpol_Pending_Park_Links[i].p_link!=0))
        {
            i++;
        }

        if (i!=PRH_BS_CFG_MAX_PARK_LINKS)
        {
            _LMpol_Pending_Park_Links[i].p_link = p_link;
            _LMpol_Pending_Park_Links[i].automatic = automatic;
            _LMpol_Pending_Park_Links[i].initiator = role;
        }
    }
    else
    {
        /* 
         * The link is already in the container, so we just change the 
         * existing entry
         * The p_link entry does not change so we just assign the 
         * Automatic and Role fields.
         */
        _LMpol_Pending_Park_Links[i].automatic = automatic;
        _LMpol_Pending_Park_Links[i].initiator = role;
    }

#if defined(SYS_HAL_OS_EVENT_DRIVEN_HOST_CONTROLLER)
    BTms_OS_Post(BTMS_OS_EVENT_SERVICE_PARK_LINKS);
#endif

}

/**************************************************************************
 * Function :- _LMpolicy_Remove_Park_Link
 *
 * Description 
 * Remove a link from container of links to be Parked.
 **************************************************************************/
static void _LMpolicy_Remove_Park_Link(const t_lmp_link* p_link)
{
    u_int8 i=0;

    while((i != PRH_BS_CFG_MAX_PARK_LINKS) && 
       (_LMpol_Pending_Park_Links[i].p_link != p_link))
    {
       i++;
    }
    if  (i!=PRH_BS_CFG_MAX_PARK_LINKS)
    {
        _LMpol_Pending_Park_Links[i].p_link = 0;
    }
}

/**************************************************************************
 * Function :- _LMpolicy_Remove_UnPark_Link
 *
 * Description 
 * Remove a link from container of links to be UnParked.
 **************************************************************************/
static void _LMpolicy_Remove_UnPark_Link(const t_lmp_link* p_link)
{
    u_int8 i=0;

    while((i != PRH_BS_CFG_MAX_PARK_LINKS) &&
       (_LMpol_Pending_Unpark_Links[i].p_link != p_link))
    {
       i++;
    }
    if  (i!=PRH_BS_CFG_MAX_PARK_LINKS)
    {
        _LMpol_Pending_Unpark_Links[i].p_link = 0;
    }
}

/**************************************************************************
 * Function :- _LMpolicy_Remove_Link
 *
 * Description 
 * Remove a link from container of links to be UnParked and the 
 * container of link to be parked.
 **************************************************************************/
void LMpolicy_Remove_Link(const t_lmp_link* p_link)
{
    _LMpolicy_Remove_UnPark_Link(p_link);
    _LMpolicy_Remove_Park_Link(p_link);
}

/**************************************************************************
 * Function :- LMpolicy_Are_Links_Being_Parked_UnParked.
 *
 * Description 
 *  Determines if an links are pending park or unpark.
 * Returns
 *  TRUE if any link is being parked or unparked, FALSE otherwise
 **************************************************************************/
static u_int8 LMpolicy_Are_Links_Being_Parked_UnParked(void)
{
    u_int8 i;

    for (i=0; i < PRH_BS_CFG_MAX_PARK_LINKS; i++)
    {
        if ( (_LMpol_Pending_Park_Links[i].p_link!=0) ||
             (_LMpol_Pending_Unpark_Links[i].p_link!=0) )
        {
            return 0x01;
        }
    }

    return 0;
}

#endif

#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUPPORTED==1)
u_int8 LMpolicy_Determine_D_Sniff_In_Scatternet(t_lmp_link* p_link)
{
	u_int8 are_good_parameters = TRUE;

	if (LMconfig_LM_Connected_As_Scatternet())
	{

		// If we are in scatternet and a Sniff link is already active.
		// Then we must ensure that the Sniff Windows dont overlap. 
		// and are far enough apart to allow us to perform 
		// a piconet switch in between the windows.

        /**************************************************
		 * SNIFF WINDOW OVERLAP/COLLISION IN SCATTERNET !
		 *
		 * This aspect has to be investigated Further !!
		 * Even if the two sniff windows are seperated at the 
		 * start, they can still drift significantly and will
		 * eventually overlap...
		 *
		 * There is no obvious way to prevent this !!!
		 *
		 * One Possible Solution is to ensure that no two links 
		 * go into Sniff with the Same T-Sniff..
		 * 
		 * This may cause Problems/Supprise with the users of the 
		 * HC_Data_Tester - as the default has Max/Min set to the
		 * same value. However, the Spec mandates that widest possible
		 * range be use to allow flexibility in negocition of the T-Sniff
		 *
		 * IMPLEMENTATION OF THIS PROPOSAL WILL BE IN FUTURE RELEASE
         *
		 *  [ Note :- CSR device had identical problem and does not 
		 *            have a solution. ]
		 **************************************************/

		// Delta Piconet#1 = Pico#1_Cur_Clk - Cur_Native_Clk
		//
		//  NOTE !! PRH_BS_DEV_USE_CACHED_BT_CLOCKS must be 0
		//  for this logic to work.....

		// Below only Works of Both T-Sniffs are the Same !!!!
		// NEED TO CONSIDER DIFFERENT T-SNIFFs
		
		// GF 30 March - Turned ON
		if (g_LM_config_info.links_in_low_power)
		{
			
			t_clock pico1_cur_clock;
			t_clock pico2_cur_clock;
			t_clock delta_Pico_1;
			t_clock delta_Pico_2;
			t_clock next_sniff_window_pico_1;
			t_clock next_sniff_window_pico_2;
			t_clock anchor_Pico_1_on_Native;
			t_clock anchor_Pico_2_on_Native;
			
			
			
			if (p_link->role != MASTER)
			{
				pico1_cur_clock = LC_Get_Piconet_Clock(p_link->device_index);
				delta_Pico_1 = (pico1_cur_clock - LC_Get_Native_Clock()) & BT_CLOCK_MAX_TICKS;
			}
			else
			{
				pico1_cur_clock = LC_Get_Native_Clock();
				delta_Pico_1 = 0;
			}
			
			
			if (g_LM_config_info.next_wakeup_link->role != MASTER)
			{
				pico2_cur_clock = LC_Get_Piconet_Clock(g_LM_config_info.next_wakeup_link->device_index);
				delta_Pico_2 = (pico2_cur_clock - LC_Get_Native_Clock()) & BT_CLOCK_MAX_TICKS;
			}
			else
			{
				pico2_cur_clock = LC_Get_Native_Clock();
				delta_Pico_2 = 0;
			}
			
			next_sniff_window_pico_1 = BTtimer_Set_Timer_On_Timing_Equation(pico1_cur_clock,p_link->pol_info.sniff_info.timing_ctrl, p_link->pol_info.sniff_info.T_sniff ,p_link->pol_info.sniff_info.D_sniff);
			next_sniff_window_pico_2 = g_LM_config_info.next_wakeup_link->pol_info.sniff_info.next_sniff_window;
			
			anchor_Pico_1_on_Native = (next_sniff_window_pico_1 - delta_Pico_1) & BT_CLOCK_MAX_TICKS;
			anchor_Pico_2_on_Native = (next_sniff_window_pico_2 - delta_Pico_2) & BT_CLOCK_MAX_TICKS;
			
			
			if (anchor_Pico_1_on_Native > anchor_Pico_2_on_Native)
			{
				u_int32 delta1;
				u_int32 delta2;
				
				delta1 = anchor_Pico_1_on_Native - anchor_Pico_2_on_Native;
				
				if ((anchor_Pico_1_on_Native - (p_link->pol_info.sniff_info.T_sniff<<1)) > anchor_Pico_2_on_Native)
					delta2 = (anchor_Pico_1_on_Native - (p_link->pol_info.sniff_info.T_sniff<<1)) - anchor_Pico_2_on_Native;
				else
					delta2 = anchor_Pico_2_on_Native - (anchor_Pico_1_on_Native - (p_link->pol_info.sniff_info.T_sniff<<1)) ;
				
				if ((delta1 < 0x80) || (delta2 < 0x80))
				{               
					if (delta1 > delta2)
					{
						p_link->pol_info.sniff_info.D_sniff += (((p_link->pol_info.sniff_info.T_sniff/2)- (delta2/2))& 0xFE);
						are_good_parameters = FALSE;
					}
					else
					{
						p_link->pol_info.sniff_info.D_sniff += (((p_link->pol_info.sniff_info.T_sniff/2)- (delta1/2))& 0xFE);
						are_good_parameters = FALSE;
					}
				}
			}
			
			else
			{
				u_int32 delta1;
				u_int32 delta2;
				
				delta1 = anchor_Pico_2_on_Native - anchor_Pico_1_on_Native;
				
				if ((anchor_Pico_2_on_Native - (p_link->pol_info.sniff_info.T_sniff<<1)) > anchor_Pico_1_on_Native)
					delta2 = (anchor_Pico_2_on_Native - (p_link->pol_info.sniff_info.T_sniff<<1)) - anchor_Pico_1_on_Native;
				else
					delta2 = anchor_Pico_1_on_Native - (anchor_Pico_2_on_Native - (p_link->pol_info.sniff_info.T_sniff<<1)) ;
				
				
				if ((delta1 < 0x80) || (delta2 < 0x80))  // Within 40ms
				{                
					if (delta1 > delta2)
					{
						p_link->pol_info.sniff_info.D_sniff = (((p_link->pol_info.sniff_info.T_sniff/2)- (delta2/2)) & 0xFE);
						are_good_parameters = FALSE;
					}
					else
					{
						p_link->pol_info.sniff_info.D_sniff = (((p_link->pol_info.sniff_info.T_sniff/2)- (delta1/2)) & 0xFE);
						are_good_parameters = FALSE;
					}
				}
				
			}
		}
	}
	return  are_good_parameters;

}
#endif
