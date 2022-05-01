/**************************************************************************
 * MODULE NAME:    lmp_ch.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    LMP Connection Handler
 * MAINTAINERS:    Gary Fleming 
 * CREATION DATE:  23-02-1999
 *
 * SOURCE CONTROL: $Id: lmp_ch.c,v 1.495 2014/03/11 03:14:00 garyf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * NOTES:
 *  This module controls the establishment and release of LMP connections
 *  between peer BT devices. It triggers the baseband link establishment
 *  and subsequently uses the shared queues to establish the LMP link to the
 *  peer.
 *
 *  While the core responsibility of this module is Connection establishment
 *  it also supports many of the optional operations which can occur during
 *  connection establishment. For example :
 * 
 *     Role Switching
 *     Slot Offset Information
 *     Features Exchange
 *     Name Exchange
 *
 *  The name request is a special case of connection establishment. If no
 *  connection exists prior to the name request from the HCI then a temporary
 *  baseband link is established, the name exchanged and then the link released.
 * 
 *  This module Generates error codes Num 21 in the HW Error Event.
 *************************************************************************/



#include "sys_config.h"

#include "hc_const.h"
#include "hc_event_gen.h"
#include "hc_flow_control.h"
#include "hci_params.h"

#include "sys_mmi.h"

#include "lmp_types.h"
#include "lmp_config.h"
#include "lmp_acl_connection.h"
#include "lmp_acl_container.h"
#include "lmp_sco_container.h"
#include "lmp_link_qos.h"
#include "lmp_ch.h"
#include "lmp_const.h"
#include "lmp_encode_pdu.h"
#include "lmp_sec_core.h"
#include "lmp_con_filter.h"
#include "lmp_timer.h"
#include "lmp_scan.h"
#include "lmp_features.h"
#include "lmp_utils.h"
#include "lmp_link_policy.h"
#include "lmp_debug.h"
#include "lmp_link_qos.h"
#include "lmp_link_control_protocol.h"
#include "lmp_link_policy_protocol.h"
#include "lmp_sec_peer.h"
#include "lmp_sec_core.h"
#include "lmp_afh.h"
#include "lmp_ssp.h"

#include "le_connection.h"
#include "lc_interface.h"
#include "uslc_switch_piconet.h"
#include "uslc_scheduler.h"
#include "bt_test.h"
#include "bt_timer.h"
#include "uslc_chan_ctrl.h"
#include "hw_memcpy.h"

/*
 * Define the lookahead for Master Slave Switch 
 *  2 slots for LM to start LC Procedure, 2 slots to perform TDD 
 */
#define MSS_EARLY_START_SLOT_OFFSET_ADJUST (2 + 2) 
#define MSS_MIN_POLL_IVAL                  0x0008
#define MSS_MAX_POLL_IVAL                  0x0032
#define MSS_DEFAULT_INSTANT_OFFSET            320


/* Set a link timer to switch Piconet */
u_int8 piconet_timer=0;

/*************************************************************
 * Local Functions to Send LMP PDUs and extract PDU contents 
 *************************************************************/
static void LMconnection_Connection_Timeout(t_lmp_link* p_link);
static void LMconnection_Local_Setup_Complete(t_lmp_link* p_link);
static void _Send_LMP_Connection_PDU(u_int8 opcode,t_lmp_link* p_link);
static void LMconnection_Finalise_Connection_Setup(t_lmp_link* p_link);

/*
 * Non static to allow test code access
 */
void LMconnection_Detach_Timeout(t_lmp_link* p_link);
void LMconnection_Disconnect_Acl(t_lmp_link* p_link);

static void _Send_HC_Connection_Complete_Event_ACL(
    t_lmp_link *p_link, t_error status);
static void _Send_HC_Disconnection_Complete(
    t_lmp_link *p_link, t_error status);
static void _Send_HC_Connection_Request(
    t_lmp_link *p_link, t_linkType link_type);
static void _Send_HC_Remote_Features_Complete_Event(   
    t_lmp_link *p_link, t_p_pdu p_pdu, t_error status);
static void _Send_HC_Create_Connection_Cancel_Command_Complete_Event(
            const t_bd_addr *p_bd_addr, t_error status);
static void _Send_HC_Read_Remote_Name_Cancel_Command_Complete_Event(
            const t_bd_addr *p_bd_addr, t_error status);
static void _Send_HC_Remote_Extended_Features_Complete_Event(   
    t_lmp_link *p_link, t_p_pdu p_pdu, t_error status);
static void _Send_HC_Remote_Host_Supported_Features_Notification_Event(   
    t_lmp_link *p_link, t_p_pdu p_pdu, t_error status);

#if (PRH_BS_CFG_SYS_REMOTE_NAME_REQUEST_SUPPORTED==1)
static void _Send_HC_Remote_Name_Request_Complete_Event(
    t_lmp_link *p_link,t_error status);
static void _Send_LMP_Name_Req(t_lmp_link *p_link, u_int8 offset);
#endif /* (PRH_BS_CFG_SYS_REMOTE_NAME_REQUEST_SUPPORTED==1) */

static void _Send_LMP_Features_Req_Ext(t_lmp_link* p_link, u_int8 page);
static void _Send_LMP_Features_Res_Ext(t_lmp_link *p_link, u_int8 page);

#if (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1)
static void LMconnection_Switch_Complete_Failure(t_lmp_link *p_link);
static void LMconnection_Switch_Complete_Success(t_lmp_link *p_link);
#endif /* (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1) */

#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
static void _Send_HC_Connection_Packet_Type_Changed_Event_SCO(
    const t_sco_info *p_sco_link, t_error outcome);
void _Send_LMP_Remove_SCO_Link_Req(
    t_lmp_link *p_link, t_sco_info *p_sco_link, t_error reason);
static void _Accept_SCO_Link(
    const t_lmp_link *p_link, u_int8 t_id);
static void _Send_LMP_SCO_Link_Req(
    const t_lmp_link *p_link, t_sco_info *p_sco_link);
static void _Send_HC_Connection_Complete_Event_SCO(
    const t_lmp_link *p_link, t_connectionHandle sco_handle, t_error status);
static t_error _Cancel_Pending_Page_Inquiry(void);
#endif /* (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1) */

t_error LMconnection_Page_Suspend(t_LC_Event_Info* p_event_info);
static void LMconnection_Page_Suspend_NonIRQ(t_lmp_link* p_link);
void LMconnection_Page_Resume(t_lmp_link* p_link);


/*************************************************************
 ***************** Static data *******************************
 *************************************************************/
  
static t_lmp_link *_LMch_p_pending_link;
static t_sco_info  *p_pending_sco_disconnect_link=0;
static t_error pending_sco_disconnect_reason;


#if ((PRH_BS_CFG_SYS_HCI_V12_FUNCTIONALITY_SUPPORTED==1) && (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1))
STATIC boolean _LMch_host_v12_functionality_confirmed = FALSE;
#endif

t_error LMconnection_Cleanup_Reject_Synchronous_Connection_Request(t_lmp_link* p_link,
    t_sco_info* p_esco_link, t_error status, t_error reject_reason);

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1)
u_int8 _LMsco_LMP_esco_link_req_PDU[16];
u_int8 _LMsco_LMP_remove_esco_link_req_PDU[4];
void _Finalise_Synchronous_Connection_Setup(
    t_lmp_link *p_link, t_sco_info *p_esco_link, t_error status);
void _Finalise_Synchronous_Connection_Change(
    const t_lmp_link *p_link, t_sco_info *p_esco_link, t_error status);
void _Send_HC_Synchronous_Connection_Complete_Event(
    const t_lmp_link *p_link, const t_sco_info *p_esco_link, t_error status);
void _Send_HC_Synchronous_Connection_Changed_Event(
    const t_lmp_link *p_link, const t_sco_info *p_esco_link, t_error status);
static void _Send_LMP_Remove_eSCO_Link_Req(
    t_lmp_link *p_link, t_sco_info *p_esco_link, t_error reason);
void LMconnection_LMP_Accepted_Ext_eSCO_Link_Req(t_lmp_link *p_link);
void LMconnection_LMP_Accepted_Ext_eSCO_Link_Req_Timeout(t_lmp_link *p_link);
void LMconnection_LMP_Accepted_Ext_Remove_eSCO_Link_Req(t_lmp_link *p_link);
void LMconnection_LMP_Not_Accepted_Ext_eSCO_Link_Req(t_lmp_link *p_link, u_int8 reason);
#endif

boolean Is_HCI_eSCO_Packet_Type(u_int16 packet_types)
{
    return (boolean)(0 != (packet_types & HCI_SYN & ~HCI_SCO_BR));
}

boolean Is_HCI_SCO_Packet_Type(u_int16 packet_types)
{
    return (boolean)(0 != (packet_types & HCI_SCO_BR));
}

boolean Is_HCI_SYN_Packet_Type(u_int16 packet_types)
{
    return (boolean)(0 != (packet_types & HCI_SYN));
}

/*************************************************************************
 *  FUNCTION : LMconnection_Initialise
 *
 *  DESCRIPTION :
 *  Invoked to initialise the LMP Connection Protocol states.
 *
 *************************************************************************/
void LMconnection_Initialise(void)
{
    /************************************************
     * Initialise the callback functions from the Baseband
     ************************************************/
    LC_Register_Event_Handler(LC_EVENT_PAGE_COMPLETE, (void*)LMconnection_LM_Page_Complete);
    LC_Register_Event_Handler(LC_EVENT_PAGE_RESULT, (void*)LMconnection_LM_Page_Complete);
    LC_Register_Event_Handler(LC_EVENT_MSS_COMPLETE, (void*)LMconnection_LM_Switch_Role_Complete);

	LC_Register_Event_Handler(LC_EVENT_PAGE_SUSPENDED, (void*)LMconnection_Page_Suspend);

    _LMch_p_pending_link = (t_lmp_link*) 0;

    piconet_timer=0;

}


/************************************************************************
 * FUNCTION : _LMconnection_Validate_ACL_Admission
 *
 * DESCRIPTION :
 * Checks if an ACL request can be added to bd_addr
 *      NO_ERROR            if acl can be added, otherwise reason
 *      ACL_ALREADY_EXISTS  if connection already exists
 *
 * Reject ACL if
 * 1. All bandwidth used by SCO
 * 2. Authentication and Encryption settings mis-match
 * 3. An ACL connection already exists to this device
 * 4. SCO active and packet type mismatch
 *************************************************************************/
static t_error _LMconnection_Validate_ACL_Admission(const t_pageParams *p_page_info)
{
    t_bd_addr *p_bd_addr = p_page_info->p_bd_addr;
    t_HCIpacketTypes packet_types = p_page_info->packet_types;
    u_int8 allow_role_switch = p_page_info->allow_role_switch;

    t_lmp_link *p_link;

    /*
     * Check if already active satisfying a remote name request.
     * Do not allow other connections until complete
     */
    if (g_LM_config_info.operation_pending == LMP_NAME_REQ)
    {
        return COMMAND_DISALLOWED;
    }

    /* #2813:
     * Check if we have already started a connection to this device.
     */
    if((_LMch_p_pending_link) && (_LMch_p_pending_link->state == W4_PAGE_OUTCOME)
	    && (LMutils_Bd_Addr_Match(p_bd_addr, &_LMch_p_pending_link->bd_addr)))
    {
        return COMMAND_DISALLOWED;
    }

    /*
     * Verify the Page Scan Repetition Mode, Page Scan Mode, Role Switch
     * and Packet Types
     * 1. Reject if values invalid
     * 2. Reject if role switch requested but not supported in device
     * 3. Reject if role switch requested but connections exist
     */
    if ( (p_page_info->srMode > 0x02) || 
         (p_page_info->pageScanMode > 0x03) || 
         (allow_role_switch > 0x01) ||
		!(packet_types & HCI_ACL) || 
		 (packet_types & ~HCI_ACL) )
    {
        return INVALID_HCI_PARAMETERS;
    }

    if (allow_role_switch && (!PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED ))
    {
        return ROLE_CHANGE_NOT_ALLOWED;
    }

    /*
     * Check if an existing link exists to this Bd_Addr 
     */
    p_link = LMaclctr_Find_Bd_Addr(p_bd_addr);
    if ((p_link != 0) && (p_link->state != LMP_IDLE))
    {
        if (p_link->operation_pending == LMP_DETACH)
        {
            return COMMAND_DISALLOWED;
        }
        else
        {
            return ACL_ALREADY_EXISTS;
        }
    }

    /*
     * Do not allow connection to one self
     */
    if (LMutils_Bd_Addr_Match(p_bd_addr, SYSconfig_Get_Local_BD_Addr_Ref()))
    {
        return INVALID_HCI_PARAMETERS;
    }

    if(!g_LM_config_info.new_connections_allowed)
    {
        return COMMAND_DISALLOWED;
    }
  
#if ((PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1) || (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1) )
    if (LMscoctr_Get_Number_SYN_Connections())
    {
        /* 
         * Ensure that the Number of SCO or eSCO links does not preclude the ACL setup 
         */
        if (LMscoctr_Is_All_Bandwidth_Used_By_SYN())
        {
            return MAX_NUM_CONNECTIONS;
        }

    }
#endif

    /*
     * Check if we already have a slave connection active
     */
    if (LMconfig_LM_Connected_As_Slave())
    {
#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)

#else
        return MAX_NUM_CONNECTIONS;
#endif 
    }

#if (PRH_BS_CFG_SYS_LMP_ENCRYPTION_SUPPORTED==1)
	// NOTE WAS BUG/ISSUE HERE :  Case #00009759
	// Section of specification was missed in original code.
	// Page 530 of Spec
	//
	// "The Authentication_Enable configuration parameter shall only apply to 
	// connections (e.g. send an LMP_in_rand or LMP_au_rand) when the remote 
	// device’s Host or Controller does not support Secure Simple Pairing or
	// when the local Host does not support Secure Simple Pairing.
    //
    // Note: Requires LM to read host features during connection setup".
    // 
	// Resolution :- Completely remove this check - as it is unnecessary.

    /*
     * Ensure that authentication and encryption settings are sane
     */
    //if(!g_LM_config_info.authentication_enable && g_LM_config_info.encryption_mode)
    //{
    //    return COMMAND_DISALLOWED;
    //}
#endif
 
    /*
     * Check if we already started a connection to peer
     */
    if((_LMch_p_pending_link) && (_LMch_p_pending_link->state == W4_PAGE_OUTCOME))
    {
        return COMMAND_DISALLOWED;
    }

    return NO_ERROR;
}


/************************************************************************
 *  FUNCTION : _LMconnection_Create_ACL_Connection
 *
 *  DESCRIPTION :
 *  Create the ACL Connection
 *
 *  p_page_info             The connection setup parameters
 *  lmp_connection_context  LMch_LMP_CONNECTION for normal
 *                          LMch_TEMPORARY_CONNECTION for temp remote name
 *
 *  Step 1/ - Allocate a Link Structure for the new link. 
 *            [with this a handle is assigned].
 *
 *  Step 2/ - Setup the Bd_Addr 
 *  Step 3/ - Set the pending connection Bd_Addr.
 *  Step 4/ - Initiate Baseband Paging.
 *************************************************************************/
static t_error _LMconnection_Create_ACL_Connection(
    const t_pageParams *p_page_info, u_int8 lmp_connection_context)
{
    u_int16 train_reps;
    t_error status;
    t_HCIpacketTypes packet_types = p_page_info->packet_types;

    //
	// Ensure that we dont initiate an Page longer than the smallest Supervision Timeout
	// in Low Energy links
    {
    	u_int16 min_LE_superTO = LEconnection_Min_Active_Supervision_Timeout();

    	if ((min_LE_superTO != 0xFFFF) && (g_LM_config_info.page_timeout > min_LE_superTO ))
    		return COMMAND_DISALLOWED;
    }

    _LMch_p_pending_link = LMaclctr_Alloc_Link();
    if (_LMch_p_pending_link) 
    {       
        _LMch_p_pending_link->current_proc_tid_role = MASTER; 
        _LMch_p_pending_link->context = lmp_connection_context;
        LMacl_Write_Bd_Addr(_LMch_p_pending_link, p_page_info->p_bd_addr);

#if (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1)
        _LMch_p_pending_link->allow_switch = p_page_info->allow_role_switch;
#endif /* (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1) */


        /*
         * Initiate Baseband Paging                   
         */
        LMconfig_LM_Set_Device_State(PRH_DEV_STATE_PAGE_PENDING);

        train_reps= (u_int16)((((u_int16)(p_page_info->srMode)*128)+1)*
            (1 + LMscoctr_Get_Number_SCO_Connections())); 
        status = LC_Create_Connection(&_LMch_p_pending_link->bd_addr,
                p_page_info->clockOffset, _LMch_p_pending_link->device_index,
                train_reps, g_LM_config_info.page_timeout);

        if (NO_ERROR == status) 
        {
            _LMch_p_pending_link->state = W4_PAGE_OUTCOME;
            _LMch_p_pending_link->proposed_packet_types = packet_types;
            _LMch_p_pending_link->packet_types = packet_types;
            if (lmp_connection_context!=LMch_TEMPORARY_CONNECTION)
            {
                /*
                 * Only set the state here if its not a temporary connection.
                 *
                 * For a temporary connection the state
                 * ( e.g. PRH_DEV_STATE_RNR_CONN_PENDING ) will be set by the invoker 
                 * of the function "_LMconnection_Create_ACL_Connection".
                 */
                LMconfig_LM_Set_Device_State(PRH_DEV_STATE_CREATE_CONN_PENDING);
            }
            
#if (PRH_BS_CFG_SYS_REMOTE_NAME_REQUEST_SUPPORTED==1)
            if (lmp_connection_context==LMch_TEMPORARY_CONNECTION)
            {
                g_LM_config_info.operation_pending = LMP_NAME_REQ;
                g_LM_config_info.remote_name.length = 0;
            }
#endif
        } 
        else 
        {
            /*
             * Page Initiation Failed - Release Link 
             */
            LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_PAGE_PENDING);
            LMaclctr_Free_Link(_LMch_p_pending_link);
        }
    } 
    else
    {
        status = MAX_NUM_CONNECTIONS;
    }
    return status;
}

/*****************************************************
 *****************************************************
 ***    SERVICE INTERFACE TO THE OBJECT           **** 
 *****************************************************
 *****************************************************/

/**************************************************************************
 *  FUNCTION : LMconnection_LM_Connection_Req
 *
 *  DESCRIPTION :
 *  Invoked by the higher layers on the LM to establish a remote ACL
 *  connection. 
 *  First, the function checks that the requested ACL can be
 *  admitted, and if ok, requests its setup.
 *
 *  p_page_info             The connection setup parameters
 *************************************************************************/
t_error LMconnection_LM_Connection_Req(const t_pageParams* p_page_info)
{
    t_error status; 
        
    /*
     * Check that we can admit this connection.
     */

    
	status = _LMconnection_Validate_ACL_Admission(p_page_info);
    if (status == NO_ERROR)
    {
        status = _LMconnection_Create_ACL_Connection(p_page_info, LMch_LMP_CONNECTION);
    }
    return status;
}

/**************************************************************************
 *  FUNCTION : LMconnection_LM_Connection_Cancel
 *
 *  DESCRIPTION :
 *  Invoked by the higher layers on the LM to cancel a previously requested 
 *  connection establishment. 
 *
 *  bd_addr             bd_addr of remote device
 *************************************************************************/
t_error LMconnection_LM_Connection_Cancel(const t_bd_addr *bd_addr)
{
    t_error status = NO_ERROR; 
    t_lmp_link *p_link = _LMch_p_pending_link;
    boolean connection_cancelled = FALSE;

    if (LMconfig_LM_Check_Device_State(PRH_DEV_STATE_CREATE_CONN_PENDING))
    {
        if (LMutils_Bd_Addr_Match(bd_addr,&p_link->bd_addr)) 
        {
            if (LMconfig_LM_Check_Device_State(PRH_DEV_STATE_PAGE_PENDING))
            {
                /* Cancel Baseband Page procedure */
                status = LC_Page_Cancel();

                LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_PAGE_PENDING);
            
                _Send_HC_Create_Connection_Cancel_Command_Complete_Event(bd_addr, 
                                 NO_ERROR);

                _Send_HC_Connection_Complete_Event_ACL(p_link, NO_CONNECTION);
                
                LMaclctr_Free_Link(p_link);

            }
            else /* BB connection, but LMP not complete*/
            {
                   LMconfig_LM_Set_Device_State(PRH_DEV_STATE_CANCEL_CONN_PENDING);
                LMconnection_Send_LMP_Detach(p_link, LOCAL_TERMINATED_CONNECTION);
            }
            connection_cancelled = TRUE;
        }
    }

    if (!connection_cancelled) 
    {
        p_link = LMaclctr_Find_Bd_Addr(bd_addr);
        if (p_link) 
        {
            /* ACL connection already exists */
            _Send_HC_Create_Connection_Cancel_Command_Complete_Event(bd_addr, 
                                 ACL_ALREADY_EXISTS);
        }
        else
        {
            /* No connection exists */
            _Send_HC_Create_Connection_Cancel_Command_Complete_Event(bd_addr, 
                             NO_CONNECTION);
        }
    }

    return status;
}


#if (PRH_BS_CFG_SYS_REMOTE_NAME_REQUEST_SUPPORTED==1)
/**************************************************************************
 *  FUNCTION :- LMconnection_LM_Read_Remote_Name
 *
 *  DESCRIPTION :- 
 *  Invoked by the higher layers on the LM to request the name of
 *  a remote device.
 *  If a connection already exists to the remote device it
 *  is used to exchange the name and the connection is left in place
 *  after the name has been retrieved. In contrast if no connection already
 *  exists to the remote device, a temporary connection is established
 *  and released on retrieval of the last name fragment.
 * 
 *  The Remote device name can be up to 248 characters long and may be
 *  transmitted in multiple LMP PDUs
 *
 *  p_page_info             The connection setup parameters
 ******************************************************************/
t_error LMconnection_LM_Read_Remote_Name(const t_pageParams *p_page_info)
{
    t_error status; 
    t_lmp_link *p_link;
    u_int8 i;

    /*
     * If valid connection to selected bd_addr exists then use it else
     * setup connection if parameters are valid.
     */    
	status = _LMconnection_Validate_ACL_Admission(p_page_info);

    if (status == ACL_ALREADY_EXISTS || status == NO_ERROR)
    {
        /*
         * Zero the remote name before use, in case peer never responds, etc.
         */
        for(i = 0; i < PRH_MAX_NAME_LENGTH; i++)
        {
            g_LM_config_info.remote_name.p_utf_char[i] = 0;
        }

        if (status == ACL_ALREADY_EXISTS) 
        {
           /*
            * Existing connection exists, just send LMP_name_req
            */
            p_link = LMaclctr_Find_Bd_Addr(p_page_info->p_bd_addr);
            if (p_link!=0)
            {
                p_link->current_proc_tid_role = p_link->role; 
                _Send_LMP_Name_Req(p_link, 0);
                status = NO_ERROR;
            }
            else
            {
                status = UNSPECIFIED_ERROR;
            }
        }
        else /* (status == NO_ERROR) */
        {          
            /*
             * No existing connection, create connection.
             */ 
            status = _LMconnection_Create_ACL_Connection(
                p_page_info, LMch_TEMPORARY_CONNECTION);
            if (status == NO_ERROR)
            {
                LMconfig_LM_Set_Device_State(PRH_DEV_STATE_RNR_CONN_PENDING);
            }
        }

        if (status == NO_ERROR)
        {
            g_LM_config_info.operation_pending = LMP_NAME_REQ;
        }
    }

    return status;
}

/**************************************************************************
 *  FUNCTION :- LMconnection_LM_Read_Remote_Name_Cancel
 *
 *  DESCRIPTION :- 
 *  Invoked by the higher layers on the LM to cancel a request for 
 *  the name of a remote device.
 *
 *  bd_addr              BD_ADDR of remote device.
 ******************************************************************/
t_error LMconnection_LM_Read_Remote_Name_Cancel(const t_bd_addr *bd_addr)
{
    t_error status = NO_ERROR; 
    t_lmp_link *p_link = _LMch_p_pending_link;
    boolean connection_cancelled = FALSE;

    if (LMconfig_LM_Check_Device_State(PRH_DEV_STATE_RNR_CONN_PENDING))
    {
        if ( LMutils_Bd_Addr_Match(bd_addr,&p_link->bd_addr) &&
             (p_link->context == LMch_TEMPORARY_CONNECTION)) 
        {
            /*
             * Temporary connection (ie for RNR) is being established for
             * this device.
             */
            if (LMconfig_LM_Check_Device_State(PRH_DEV_STATE_PAGE_PENDING))
            {
                /* Cancel Baseband Page procedure */
                status = LC_Page_Cancel();

                LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_PAGE_PENDING);
            
                _Send_HC_Read_Remote_Name_Cancel_Command_Complete_Event(bd_addr, 
                                 NO_ERROR);

                _Send_HC_Remote_Name_Request_Complete_Event(p_link, NO_CONNECTION);
                
                LMaclctr_Free_Link(p_link);

            }
            else /* BB connection, but LMP not complete*/
            {
                   LMconfig_LM_Set_Device_State(PRH_DEV_STATE_RNR_CANCEL_PENDING);
                LMconnection_Send_LMP_Detach(p_link, LOCAL_TERMINATED_CONNECTION);
            }
            connection_cancelled = TRUE;
        }
    }

    if (!connection_cancelled) 
    {
        /* No connection exists */
        _Send_HC_Read_Remote_Name_Cancel_Command_Complete_Event(bd_addr, 
                                 INVALID_HCI_PARAMETERS);
    }

    return status;
}

/**************************************************************************
 *  FUNCTION : _Send_HC_Remote_Name_Request_Complete_Event
 *
 *  DESCRIPTION :
 *  Send an HC_Remote_Name_Request_Complete_Event to the higher layers.
 *************************************************************************/
void _Send_HC_Remote_Name_Request_Complete_Event(
    t_lmp_link *p_link, t_error status)
{
    t_lm_event_info event_info;
    int i;

    event_info.p_u_int8 = g_LM_config_info.remote_name.p_utf_char;

    /*
     * If status is not set to NO_ERROR, then zero out the
     * name (in case of any confusing fragments) and set
     * the name length to zero.
     */
    if(status == NO_ERROR)
    {
        event_info.number = g_LM_config_info.remote_name.length;
    }
    else
    {
        for(i = 0; i < PRH_MAX_NAME_LENGTH ; i++)
        {
            g_LM_config_info.remote_name.p_utf_char[i] = 0;
        }
        event_info.number = 0;
    }
    event_info.p_bd_addr = &p_link->bd_addr;
    event_info.status = status;
    g_LM_config_info.operation_pending = NO_OPERATION;
    g_LM_config_info.lmp_event[_LM_REMOTE_NAME_CFM]
        (HCI_READ_REMOTE_NAME_REQUEST_COMPLETE_EVENT, &event_info);
        LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_RNR_CONN_PENDING);
}

/**************************************************************************
 *  FUNCTION : _Send_LMP_Name_Req
 *
 *  DESCRIPTION :
 *  Sends an LMP_Name_Req message to the device indicated 
 *  by the p_link. The offset gives the offset into the 
 *  name string of the next segment.
 *************************************************************************/
void _Send_LMP_Name_Req(t_lmp_link *p_link, u_int8 offset)
{
    t_lmp_pdu_info pdu_info;
    pdu_info.tid_role = p_link->role; /*p_link->current_proc_tid_role;*/
    pdu_info.opcode = LMP_NAME_REQ;
    pdu_info.name_offset = offset;
    LM_Encode_LMP_PDU(p_link->device_index, &pdu_info);
}
#endif /* (PRH_BS_CFG_SYS_REMOTE_NAME_REQUEST_SUPPORTED==1) */

/**************************************************************************
 *  FUNCTION :- LMconnection_LMP_Name_Res
 *
 *  DESCRIPTION :- 
 *  Updated to support the fragmentation of a name and the 
 *  retrieval of a name during an active connection.
 *
 *************************************************************************/
t_error LMconnection_LMP_Name_Res(t_lmp_link *p_link, t_p_pdu p_pdu)
{
#if (PRH_BS_CFG_SYS_REMOTE_NAME_REQUEST_SUPPORTED==1)
    u_int8 name_offset;
    u_int8 name_length;
    u_int8 new_offset;
    int i;

    name_offset = (u_int8)(*p_pdu++);
    name_length = (u_int8)(*p_pdu++);
     
    if((name_length - name_offset) <= MAX_NAME_FRAGMENT_SIZE) /* Fix Bug 339 */
    {
        /**************************************************
         * The last Fragment of the name has been received.
         * So the Read_Remote_Name_Request_Complete event
         * is generated and the connection released by sending
         * an LMP_Detach PDU to the peer, if temporary connection.
         **************************************************/
		g_LM_config_info.remote_name.length = 
			(name_length <= PRH_MAX_NAME_LENGTH) ? name_length : PRH_MAX_NAME_LENGTH;
        for (i=name_offset; ((i < name_length) && (i<PRH_MAX_NAME_LENGTH)) ; i++)
        {
            g_LM_config_info.remote_name.p_utf_char[i] = (u_int8)*p_pdu++;
        }

        /*
         * Null Terminate the String if less than full 248 characters
         */
        while(i < PRH_MAX_NAME_LENGTH)
        {
            g_LM_config_info.remote_name.length++;
            g_LM_config_info.remote_name.p_utf_char[i] = 0x00;
            i++;
        }

        /*********************************************************
         * Temporary connection:
         * Explict detach is required on a name complete. Remote name
         * request complete event moved to when disconnect acl occurs
         * to avoid race conditions with host.
         ***********************************************************/
        if (p_link->context == LMch_TEMPORARY_CONNECTION)
        {
            /*
             * If a Remote_Name_Request_Cancel is pending, then the
             * LMP_Detach has already been sent.
             */
            if (!LMconfig_LM_Check_Device_State(PRH_DEV_STATE_RNR_CANCEL_PENDING))
            {
                LMconnection_Send_LMP_Detach(p_link, REMOTE_USER_TERMINATED_CONNECTION);
            }
        }
        else
        {
            _Send_HC_Remote_Name_Request_Complete_Event(p_link,NO_ERROR);
        }
    }
    else
    {
        /***************************************************
         * Not the last fragment of the name.
         * Store the fragement and send LMP_Name_Req for 
         * the next name fragement.
         ***************************************************/
        new_offset = name_offset + MAX_NAME_FRAGMENT_SIZE;
        for (i=name_offset; ((i < new_offset) && (i<PRH_MAX_NAME_LENGTH)) ; i++)
        {
            g_LM_config_info.remote_name.p_utf_char[i] = (u_int8)*p_pdu++;
        }
        _Send_LMP_Name_Req(p_link, new_offset);
    }                   
    return NO_ERROR;
#else
    return UNSUPPORTED_REMOTE_FEATURE;

#endif
} 


/**************************************************************************
 *  FUNCTION : LMconnection_LM_Page_Complete
 *
 *  DESCRIPTION : 
 *  Invoked by the LMP_Page object to inform the connection
 *  handler that the Page is complete. If the Page is as result of a 
 *  connection repuest a LMP_Host_Connection_Req PDU is sent to the
 *  peer. Otherwise if the Page is as a result of a LM_Remote_Name_Request
 *  an LMP_Name_Req PDU is sent to the peer.
 ************************************************************/
void LMconnection_LM_Page_Complete_NO_ERROR(t_lmp_link* p_pending_link)
{
        LMconfig_LM_Set_Device_State(PRH_DEV_STATE_ACLS_ACTIVE);
        g_LM_config_info.num_acl_links++;
        g_LM_config_info.num_master_acl_links++;
        p_pending_link->state = W4_LMP_ACTIVE;

        p_pending_link->supervision_timeout_index = LMtmr_Set_Timer(
                (t_slots)(p_pending_link->supervision_timeout>>1),
                 &LMconnection_LM_Disconnect_Inf,p_pending_link,0);

#if (PRH_BS_CFG_SYS_LMP_PAUSE_ENCRYPTION_SUPPORTED==1)
        p_pending_link->encryption_key_refresh_index = LMtmr_Set_Timer(
                (t_slots)PRH_ENCRYPTION_KEY_REFRESH_TIMEOUT,
                &LMsec_core_Encryption_Key_Refresh,p_pending_link,0);
#endif

        p_pending_link->recent_contact_timeout_index =
            LMtmr_Set_Timer(
                 (((p_pending_link->supervision_timeout >> 1) > 100) 
                 || (p_pending_link->supervision_timeout ==0))
                     ? 100 : (p_pending_link->supervision_timeout >> 1),
                 &LMconnection_LM_Recent_Contact_Timeout, p_pending_link, 0);

		//
		//When the Remote Supported Host Features Notification event is unmasked
        //and when the Remote_Name_Request command initiates a connection, the
        //Link Manager shall read the remote LMP features mask pages 0 and 1.
        //
		// if (HCeg_Is_Event_Masked_On(HCI_REMOTE_HOST_SUPPORTED_FEATURES_NOTIFICATION_EVENT))
		// {
		//     // We request features Page 0 from the peer.
		//     // in the LMP response we check if TEMP_CONNECTION
		//     // if it is then we send request for Ext_Features Page 1.
		//  
#if 1 // GF 09 September, according to TP/INF/BV18 the remote features should now be sent
	  // for Temporary connections.

#if 0 // Temp 04 Nov 2010
		LC_Set_Active_Piconet((p_pending_link->device_index),TRUE);
#endif
		USLCsp_Set_Activated_Device_Index(p_pending_link->device_index);	

        _Send_LMP_Connection_PDU(LMP_FEATURES_REQ,p_pending_link);

#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
    if (g_LM_config_info.num_slave_acl_links > 0)
    {
        u_int16 piconet_service_time;
        /* 
         * The following code ensures that we stay on the piconet
         * corresponding to this link for an extra period of time [PICONET_SERVICE_TIME/2]
         * used to receive LMP connection request
         *
         * In the future this code should be replaced by a more advanced scheduler supporting
         * both piconet and scattermode scheduling.
         */
        piconet_service_time = PRH_BS_PICONET_SERVICE_TIME / 2; /*g_LM_config_info.num_acl_links;*/

        LMch_Set_Next_Change_Piconet_Time(piconet_service_time);
    }
#endif

#else
	    if (p_pending_link->context != LMch_TEMPORARY_CONNECTION )
        {
            /* Newly added to support Role Switch */
            _Send_LMP_Connection_PDU(LMP_FEATURES_REQ,p_pending_link);
        }


	    if (HCeg_Is_Event_Masked_On(HCI_REMOTE_HOST_SUPPORTED_FEATURES_NOTIFICATION_EVENT))
		{
			// Send LMP_FEATURES_REQ and W4 Response.
			// On LMP_FEATURES_RES send LMP_EXTENDED_FEATURES_REQ(Page 1)
			// On LMP_EXTENDED_FEATURES_RES send :
			//      1/HCI_REMOTE_HOST_SUPPORTED_FEATURES_NOTIFICATION_EVENT
			//      2/LMP_NAME_REQ
			// Continues on as previous implementation.

			if (p_pending_link->context == LMch_TEMPORARY_CONNECTION)
			{
				_Send_LMP_Connection_PDU(LMP_FEATURES_REQ,p_pending_link);
			}
			// On the LMP_FEATURES_RES need to check if LMch_TEMPORARY_CONNECTION.

		}
#if (PRH_BS_CFG_SYS_REMOTE_NAME_REQUEST_SUPPORTED==1)
        else /* Temporary Connection for LMP_Name  */ 
        {
			if (p_pending_link->context == LMch_TEMPORARY_CONNECTION)
			{
				_Send_LMP_Name_Req(p_pending_link,0);
			}
        } 
#endif /* PRH_BS_CFG_SYS_REMOTE_NAME_REQUEST_SUPPORTED==1 */

#endif
}
void LMconnection_LM_Page_Complete_ERROR(t_lmp_link* p_pending_link)
{

    if (p_pending_link->context != LMch_TEMPORARY_CONNECTION )
    {
        _Send_HC_Connection_Complete_Event_ACL(
                p_pending_link, p_pending_link->detach_reason);
    }
#if (PRH_BS_CFG_SYS_REMOTE_NAME_REQUEST_SUPPORTED==1)
    else
    {
        _Send_HC_Remote_Name_Request_Complete_Event(
                p_pending_link, p_pending_link->detach_reason);
    }
#endif /* PRH_BS_CFG_SYS_REMOTE_NAME_REQUEST_SUPPORTED==1 */

    LMaclctr_Free_Link(p_pending_link);

#if 1 // GF 19 Feb
    {
		u_int16 interval = 0x2000;
		u_int8 dev_index=0;

		if (NO_SCANS_ENABLED != LMscan_Read_Scan_Enable())
		{
			// Only consider Scans if no links
			t_slots slots_to_scan;
			t_clock next_instant;

			LC_Is_Sleep_Possible(&next_instant);
			slots_to_scan = LMscan_Get_Interval_To_Next_Scan(next_instant);

			if (slots_to_scan < interval)
				interval = slots_to_scan;
		}

		if(g_LM_config_info.num_acl_links &&
		  (g_LM_config_info.num_acl_links == g_LM_config_info.links_in_low_power))
		{
			t_slots slot_to_acl_activity;

			slot_to_acl_activity = (u_int16)(LMpol_Get_Interval_To_Next_Wakeup());

			if (slot_to_acl_activity < interval)
			{
				interval = slot_to_acl_activity;
				dev_index = g_LM_config_info.next_wakeup_link->device_index;
			}

		}
#if (PRH_BS_CFG_SYS_LE_DUAL_MODE==1)
		{
			t_slots le_sleep_slots = LE_LL_InactiveSlots();
			if (le_sleep_slots < interval)
			{
				interval = le_sleep_slots;
			}
		}
#endif
	if ( ( interval > SYS_CFG_MIN_SLOTS_FOR_SLEEP_PROCEDURE) &&
		((g_LM_config_info.num_acl_links == 0) ||
		 (g_LM_config_info.num_acl_links == g_LM_config_info.links_in_low_power)))
		LC_Sleep(interval - SYS_CFG_MIN_SLOTS_FOR_SLEEP_PROCEDURE,dev_index);
    }

#endif
}

t_error  LMconnection_LM_Page_Complete(t_LC_Event_Info *eventInfo)
{
    t_lmp_link *p_pending_link = _LMch_p_pending_link;


    LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_PAGE_PENDING);


    if ((W4_PAGE_OUTCOME == p_pending_link->state) &&
        (NO_ERROR == eventInfo->status)) 
    {
		LMtmr_Set_Timer(0x01, LMconnection_LM_Page_Complete_NO_ERROR, p_pending_link, 1);
    }
    else
    {
        p_pending_link->detach_reason = eventInfo->status;
		LM_DEFER_FROM_INTERRUPT_CONTEXT(LMconnection_LM_Page_Complete_ERROR, p_pending_link);
    }
    return eventInfo->status;
}    

t_error LMconnection_Page_Suspend(t_LC_Event_Info* p_event_info)
{
	LM_DEFER_FROM_INTERRUPT_CONTEXT(LMconnection_Page_Suspend_NonIRQ,(t_lmp_link*)0);
	return NO_ERROR;
}

static void LMconnection_Page_Suspend_NonIRQ(t_lmp_link* p_link)
{
	extern t_link_entry link_container[PRH_MAX_ACL_LINKS];
	int i;

	if (LMconfig_LM_Connected_As_Slave())
	{
	    for (i = 0; i < PRH_MAX_ACL_LINKS; i++)
		{
			if (link_container[i].used)
			{
				p_link = &(link_container[i].entry);
				if (SLAVE == p_link->role)
					break;
			}
		}

		LMtmr_Set_Timer(0x28, LMconnection_Page_Resume, 0, 1);
		LMch_Change_Piconet((t_lmp_link*)p_link);
	}
	else
	{
		LC_Page_Resume();
	}
}

void LMconnection_Page_Resume(t_lmp_link* p_link)
{
	LC_Page_Resume();
}


/**************************************************************************
 *  FUNCTION : LMconnection_LM_Disconnect_Req
 *
 *  DESCRIPTION : 
 *  Invoked by the higher layers on the LMP to remove a remote
 *  LINK. This can be either a SCO or ACL link, the link type (SCO or ACL) 
 *  is determined using the Connection Handle. 
 *
 *  Step 1 :-  Determine if connection handle is for SCO or ACL
 *
 *  If SCO    
 *  Step 2 :-  find SCO handle [ distinct from the Connection Handle]
 *  Step 3 :-  Send LMP_Remove_Sco_Link_Req to the peer
 *  Step 4 :-  Set state ?
 *
 *  If ACL
 *  Step 2 :-  Check if SCOs active on the link [ Sanity check ]
 *  Step 3 :-  Send LMP_Detach PDU to Peer
 *  Step 4 :-  Set the operation pening = LMP_Detach & set state.
 *
 * The  LM  queues the LMP_detach for transmission and starts a timer for 6*T poll slots .
 * If the initiating LM receives the Baseband-level acknowledgement before the timer
 * expires it now starts a timer for 3*T poll slots. When this timer expires (and if the initiating LM is the
 * master) the AM_ADDR can be re-used immediately. If the initial timer expires
 * then the initiating LM drops the link and starts a timer for T linksupervisiontimeout
 * slots after which the AM_ADDR can be re-used (if the initiating LM is the mas-ter).
 *************************************************************************/
t_error LMconnection_LM_Disconnect_Req(
    t_connectionHandle connection_handle, t_error reason)
{
    t_error status = NO_ERROR;
    t_lmp_link *p_link;

#if ((PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1) || (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1))
    t_sco_info *p_sco_link;

    p_sco_link  = LMscoctr_Find_SCO_By_Connection_Handle(connection_handle); 
    if(p_sco_link)
    {
        if ( (p_sco_link->state == SCO_IDLE) || (p_sco_link->state == SCO_MASTER_DEACTIVATION_PENDING))
        {
/*            return COMMAND_DISALLOWED; TODO: Reenable check for idle */
        }
            
        /************************************************************
         * It is a SCO handle - with p_sco_info pointing to the SCO
         * Find the p_link for the ACL linked to the SCO 
         * As the request to remove a SCO link has to be accepted 
         * by the peer, the SCO link state can be immediately moved to 
         * SCO_IDLE.
         *************************************************************/
 
        p_link = LMaclctr_Find_Device_Index(p_sco_link->device_index);   
        p_link->current_proc_tid_role = p_link->role;  
        p_link->detach_reason = LOCAL_TERMINATED_CONNECTION;
        p_sco_link->state = (MASTER==p_link->role) ? 
			((SCO_LINK==p_sco_link->link_type) ? 
			SCO_MASTER_DEACTIVATION_PENDING : SCO_IDLE) : SCO_IDLE;


#if(PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1)
        if(p_sco_link->link_type == ESCO_LINK)
        {
            LMscoctr_Set_Topical_eSCO_Link(p_sco_link, ESCO_REMOVE);
            _Send_LMP_Remove_eSCO_Link_Req(p_link, p_sco_link, reason);			
        }
        else
#endif
        {
            _Send_LMP_Remove_SCO_Link_Req(p_link,p_sco_link,reason);
        }
    } 
    else 
#endif /* (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1) */
    {
         /***********************************************************
          * Its an ACL handle and I can work on the p_link basis 
          ***********************************************************/
         p_link = LMaclctr_Find_Handle(connection_handle);

         if((p_link) && (p_link->operation_pending != LMP_DETACH))
         {
#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUPPORTED==1)
#if 1 // GF This seems to cause problems with CETECOM tester
             if((p_link->state & LMP_SNIFF_MODE) == LMP_SNIFF_MODE)
             {
                 p_link->disconnect_req_reason = reason;
                 LMpolicy_LM_Exit_Sniff_Mode(p_link);
             }
             else 
#endif

#endif
#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
			 if ((p_link->state & LMP_PARK_MODE) == LMP_PARK_MODE)
			 {
                 p_link->disconnect_req_reason = reason;
                 LMpolicy_LM_Exit_Park_Mode(p_link,0);
			 }
			 else 
#endif
				 if ((p_link->state & LMP_HOLD_MODE) == LMP_HOLD_MODE)
			 {
                 p_link->disconnect_req_reason = reason;
			 }
			 else
             {
                 LMconnection_LM_Disconnect(p_link,reason);
             }
         }
         else
         {
             status = INVALID_HCI_PARAMETERS; /* Invalid Handle */
         }
    }
    return status;
}                  

void LMconnection_LM_Disconnect(t_lmp_link *p_link, t_error reason)
{
     /* Need to Disable L2CAP Traffic @ the LM level */
     LMch_Disable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_ALL);
     /*
      * When disconnecting, stop local L2CAP data and peer L2CAP data.
      */
     LC_Stop_Local_Tx_L2CAP_Data(p_link->device_index);
     LC_Stop_Peer_Tx_L2CAP_Data(p_link->device_index);
     p_link->current_proc_tid_role = p_link->role; 
     p_link->detach_reason = LOCAL_TERMINATED_CONNECTION;
     LMconnection_Send_LMP_Detach(p_link,reason);
}


/**************************************************************************
 *  FUNCTION : LMconnection_LM_Connection_Reject
 *
 *  DESCRIPTION : 
 *  Invoked by the higher layers on the LMP to reject an incoming
 *  connection. Results in an LMP_Not_Accepted PDU being sent to the peer
 *  with the Rejected Opcode = LMP_Host_Connection_Req and also containing
 *  the reject reason given by the host.
 *
 *  The actions performed are dependent on the following :
 *             1/  Link Type ( ACL || SCO )
 *             2/  Role  ( MASTER || SLAVE )
 *
 *  Note All Error checking performed by the higher layer 
 *************************************************************************/
t_error LMconnection_LM_Connection_Reject(t_bd_addr* p_bd_addr, t_error reason)
{
    t_error status = NO_ERROR;
    t_lmp_link *p_link;
    
    p_link = LMaclctr_Find_Bd_Addr(p_bd_addr);
    
    LMtmr_Clear_Timer(g_LM_config_info.accept_timeout_index);
    g_LM_config_info.accept_timeout_index = 0;
    if(W4_HOST_ACL_ACCEPT == p_link->state) 
    {
        LM_Encode_LMP_Not_Accepted_PDU(p_link, LMP_HOST_CONNECTION_REQ, reason);  
    }

#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
    else if (W4_HOST_SCO_ACCEPT & p_link->state)
    {       
        p_link->state -= W4_HOST_SCO_ACCEPT;
        LM_Encode_LMP_Not_Accepted_PDU(p_link, LMP_SCO_LINK_REQ, reason);
        _Send_HC_Connection_Complete_Event_SCO(p_link, 0x0000, reason);
        
        /* 
         * Need to free the SCO link.  For both Master/Slave a 
         * SCO link container will have been allocated.
         */
        LMscoctr_Free(LMscoctr_Get_SCO_Transitioning_Ref());
}
#endif /* (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1) */
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1)
    else if (W4_HOST_ESCO_ACCEPT & p_link->state)
    {
        LMconnection_LM_Reject_Synchronous_Connection_Request(
            p_bd_addr->bytes, reason);
    }
#endif
    else
    {
        status = COMMAND_DISALLOWED;
    }

    return status;
}

/**************************************************************************
 *
 *  FUNCTION : LMconnection_Connection_Timeout
 *
 *  DESCRIPTION : 
 *  Invoked on expiration of the Connection Accept Timer.
 *
 *************************************************************************/
void LMconnection_Connection_Timeout(t_lmp_link *p_link)
{
    LMconnection_LM_Connection_Reject(&p_link->bd_addr, CONNECTION_ACCEPT_TIMEOUT);
}
   
/**************************************************************************
 *  FUNCTION : LMconnection_LM_Connection_Accept
 *
 *  DESCRIPTION : 
 *  Invoked by the higher layers on the LMP to accept an incoming
 *  connection. Results in a LMP_Accepted PDU being sent in response to 
 *  the previously received LMP_Host_Connection_Req PDU.
 *
 *  The actions performed are dependent on the following :
 *             1/  Link Type ( ACL || SCO )
 *             2/  Role  ( MASTER || SLAVE )
 *
 *  Note All Error checking performed by the higher layer
 *************************************************************************/
t_error LMconnection_LM_Connection_Accept(t_bd_addr *p_bd_addr, 
    t_role role, u_int16 voice_setting)
{
    t_error status = NO_ERROR;
    t_lmp_link *p_link;

    p_link = LMaclctr_Find_Bd_Addr(p_bd_addr);
#if 1// GF 2 Sept (BUILD_TYPE!=UNIT_TEST_BUILD)
    if (g_LM_config_info.default_sco_route_via_hci)
		voice_setting  |= PRH_BS_HCI_VOICE_SETTING_FOR_SCO_VIA_HCI;
#endif

    /* 
     * Note All Error checking performed by the higher layer 
     */
    LMtmr_Clear_Timer(g_LM_config_info.accept_timeout_index);
    g_LM_config_info.accept_timeout_index = 0;
    if(W4_HOST_ACL_ACCEPT == p_link->state) 
    {
        if (MASTER == role) 
        {    
           /****************************************
            * Master / Slave role switch required 
            ****************************************/

            p_link->current_proc_tid_role = p_link->role;
            p_link->state = W4_LMP_ACTIVE;

#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
            /* 
             * The following change piconet ensures that we return to (or stay on) the piconet
             * corresponding to this link for an extra period of time [PICONET_SERVICE_TIME/2]
             *
             * This is done to support the scenario where a Master Slave Switch imediately follows
             * connection establishment.
             *
             * In the future this code should be replaced by a more advanced scheduler supporting
             * both piconet and scattermode scheduling.
             */
            LMpol_Change_Piconet(p_link->device_index, (PRH_BS_PICONET_SERVICE_TIME/2));
#endif
            p_link->switch_state = SWITCH_LOCAL_ACTIVATION_PENDING;
            LMconfig_LM_Set_Device_State(PRH_DEV_STATE_MSS_ACTIVE);
            _Send_LMP_Connection_PDU(LMP_SLOT_OFFSET,p_link);
            _Send_LMP_Connection_PDU(LMP_SWITCH_REQ,p_link);
        }
        else /* SLAVE role */
        {
            LM_Encode_LMP_Accepted_PDU(p_link,LMP_HOST_CONNECTION_REQ);

			// If Auth_Enable == ON, and Local or Peer Device dont support SSP in features

            if ((ON == g_LM_config_info.authentication_enable) && 
				(LMssp_SSP_Enabled_On_Link(p_link)==0))
            {

                LMsec_core_Init_Authentication(p_link);

                p_link->state = W4_AUTHENTICATION;
            }
            else
            { 
                p_link->state = W4_LMP_ACTIVE;
                LMconnection_Local_Setup_Complete(p_link);              
            }
        }
    }
#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
    else if (W4_HOST_SCO_ACCEPT & p_link->state)
    {
        p_link->state -= W4_HOST_SCO_ACCEPT;

        /*
         * Recalculate voice setting, air mode, and
         * sco cfg after host has explicitly accepted
         * incoming connection. Use the air mode proposed
         * by the peer.
         */
        {
            t_sco_info* p_sco_link = LMscoctr_Get_SCO_Transitioning_Ref();
            p_sco_link->voice_setting = voice_setting & ~3;
            p_sco_link->voice_setting |= LMscoctr_Map_AirMode_LMP2HCI(p_sco_link->lm_air_mode);
            p_sco_link->sco_cfg_hardware = LC_Get_Hardware_SCO_CFG(voice_setting);
            /*
             * Seems to be safe to call LMpol_Init_SCO() again
             * with the new information from the host.
             */
            LMpol_Init_SCO(p_link, p_sco_link->sco_index);
        }
        if (p_link->role == SLAVE)
        {
            _Accept_SCO_Link(p_link, p_link->current_proc_tid_role);
            /*
             * Adding a SCO connection may require the adjusting
             * of ACL packet types on all ACL links.
             */
            LMpol_Adjust_ACL_Packet_Types_On_All_Links(LMcontxt_ENTERING_SCO);
        }
        else
        {
            _Send_LMP_SCO_Link_Req(p_link, LMscoctr_Get_SCO_Transitioning_Ref());
        } 
    }
#endif /* (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1) */
    else
    {
        status = COMMAND_DISALLOWED;
    }
    return status;
}

#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1) 
/**************************************************************************
 *  FUNCTION : LMconnection_LMP_SCO_Accepted_Ack
 *
 *  DESCRIPTION : 
 *  This function is called on the receipt of a ACK for an LMP_Accepted 
 *  for a SCO Link Req which initiates a new SCO link,
 *
 *  NOTE : This is not invoked for a re-negotiation of SCO paremeters on
 *  a link.
 *************************************************************************/
static void LMconnection_LMP_SCO_Accepted_Ack_Handler(t_lmp_link *p_link);

void LMconnection_LMP_SCO_Accepted_Ack(t_deviceIndex device_index)
{
    t_lmp_link *p_link = LMaclctr_Find_Device_Index(device_index);
    LM_DEFER_FROM_INTERRUPT_CONTEXT(LMconnection_LMP_SCO_Accepted_Ack_Handler, p_link);
}

static void LMconnection_LMP_SCO_Accepted_Ack_Handler(t_lmp_link *p_link)
{
    t_sco_info *p_sco_link = LMscoctr_Get_SCO_Transitioning_Ref();

    if (p_sco_link != 0)
    {
        if (p_sco_link->state != SCO_CHANGE_PKT_ACCEPT_ACK_PENDING)
        {
            /* 
             * New SCO link
             * Send a connection complete to the higher layer. 
             * Initialise the SCO link (hardware etc)
             * Later set its state to activation pending. 
             */
            _Send_HC_Connection_Complete_Event_SCO(p_link, 
                p_sco_link->connection_handle, NO_ERROR);
            LMpol_Init_SCO(p_link, p_sco_link->sco_index);
        }
        else 
        {          
            /* 
             * Existing SCO link
             * Change to the negotiated SCO packet type
             *    (procedure complete on BB Ack  Confirm change via HCI
             */
            _Send_HC_Connection_Packet_Type_Changed_Event_SCO(p_sco_link, NO_ERROR);
#if (PRH_BS_DEV_MULTISLOT_ACL_WITH_SCO_SUPPORTED==1)
            /*
             * Changing an existing SCO link may require adjusting of the
             * ACL packet types on every link.
             */
            LMpol_Adjust_ACL_Packet_Types_On_All_Links(LMcontxt_CHANGED_SCO);
#endif
        }
        p_sco_link->state = SCO_ACTIVATION_PENDING;
   }
}



#endif /* (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1) */

/**************************************************************************
 *  FUNCTION : LMconnection_LM_Disconnect_Inf
 *
 *  DESCRIPTION : 
 *  Invoked by the lower layers (Baseband LC) on the LM to inform it the
 *  the Baseband link has failed due to "Link_Supervision Timeout". 
 *     
 *  3 Cases :-
 *      1/ After the Connection Request & Setup Phases.
 *      2/ End of Remote Name Request
 *      3/ Prior to the completion of connection setup 
 *
 *************************************************************************/
void LMconnection_LM_Disconnect_Inf(t_lmp_link *p_link)
{
    p_link->detach_reason = CONNECTION_TIMEOUT;
#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
    LMpolicy_Remove_Link(p_link);
#endif
    LMconnection_Disconnect_Acl(p_link);

    /*
     * If device is in test mode then it must leave test mode when link fails
     */
    LC_Testmode_Exit();
}

/**************************************************************************
 *  
 *  FUNCTION : LMconnection_LM_Recent_Contact_Timeout
 *
 *  DESCRIPTION :
 *    If a recent contact timeout occurs then use POLL packets to
 *    force slave to reply
 *************************************************************************/
void LMconnection_LM_Recent_Contact_Timeout(t_lmp_link *p_link)
{
    p_link->default_pkt_type = POLLpkt;
}

/**************************************************************************
 *  
 *  FUNCTION : LMconnection_LM_Auth_Complete
 *
 *  DESCRIPTION :
 *    Invoked by the higher layer OR the local security object, this primitive
 *    informs the Connection Protocol that Authentication is complete. This 
 *    results in the LMP_Setup_Complete being sent to the peer device.
 *
 *    This function is only called when encryption initiated during LM 
 *    Connection setup. Note this function is not invoked if encryption 
 *    is enabled on a pre-existing LM ACL link.
 *************************************************************************/
t_error LMconnection_LM_Auth_Complete(t_lmp_link *p_link, t_error status)
{
    /* 
     * If authentication failed in any state before
     * connection setup, then detach the link.
     */
    if(status != NO_ERROR)
    {
        p_link->detach_reason = status;
        LMconnection_Send_LMP_Detach(p_link, status);
    }

    /*
     * Else If authentication is complete,
     * its safe to initiate encryption.
     *
     * (If encryption is supported).
     */
    else if(p_link->state == W4_AUTHENTICATION)
    {
#if (PRH_BS_CFG_SYS_LMP_ENCRYPTION_SUPPORTED==1)
      if (ENCRYPT_NONE != g_LM_config_info.encryption_mode)
      {
           p_link->state = W4_ENCRYPTION;

         /* 
          * No need to dis-able L2CAP traffic here as it
          * by default disable on connection setup. 
          * 
          */
           LMsec_core_Init_Encryption(p_link, g_LM_config_info.encryption_mode);
      }
      else  
#endif /* (PRH_BS_CFG_SYS_LMP_ENCRYPTION_SUPPORTED==1) */
      /* 
       * Send an LMP_Setup_Complete PDU to peer on the link 
       */
      {
          LMconnection_Local_Setup_Complete(p_link);
      }
    }
     return NO_ERROR;
}

/**************************************************************************
 *  
 *  FUNCTION : LMconnection_LM_Encrypt_Complete
 *
 *  DESCRIPTION :
 *    Invoked by the higher layer OR the local security object, this primitive
 *    informs the Connection Protocol that Encryption is complete. This 
 *    results in the LMP_Setup_Complete being sent to the peer device.
 *
 *    This function is only called when encryption initiated during LM Connection
 *    setup. Note this function is not invoked if encryption is enabled on a 
 *    pre-existing LM ACL link.
 *************************************************************************/
t_error LMconnection_LM_Encrypt_Complete(t_lmp_link *p_link, t_error status)
{
#if (PRH_BS_CFG_SYS_LMP_ENCRYPTION_SUPPORTED==1)
    /* 
     * Used to determine if the Encryption is being performed as part
     * of connection setup 
     */
    if(p_link->state == W4_ENCRYPTION)
    {
        if (status != NO_ERROR)
        {
            LMconnection_Send_LMP_Detach(p_link,status);
            p_link->detach_reason = status;
        }
        else
        {
            /* No need to enable L2CAP traffic here as it 
             * will be done on the connection complete 
             * event.
             */

            LMconnection_Local_Setup_Complete(p_link);
        }
    }
#endif /* (PRH_BS_CFG_SYS_LMP_ENCRYPTION_SUPPORTED==1) */

    return NO_ERROR;
}

/**************************************************************************
 *  
 *  FUNCTION : LMconnection_LM_Switch_Role
 *
 *  DESCRIPTION :
 *   Handles a role switch request from the LM service interface
 *   The steps to be performed are dependent on the role of the
 *   current device.
 *   
 *    If MASTER
 *          Send an LMP Switch PDU, set the operation pending.
 *    Else (if SLAVE)
 *           Send an LMP Slot Offset PDU, set the operation pending
 *
 *************************************************************************/
t_error  LMconnection_LM_Switch_Role(t_lmp_link *p_link, t_role new_role)
{
#if (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1)
     /***************************************************************
      * NOTE :- The device cannot send the switch until the current 
      * l2cap message has been Txed.
      ***************************************************************/

    if(p_link->state & (LMP_SNIFF_MODE|LMP_PARK_MODE|LMP_HOLD_MODE))
    {
        return COMMAND_DISALLOWED;
    }

    if(p_link->operation_pending == LMP_DETACH)
        return COMMAND_DISALLOWED;

    if(p_link->switch_state != SWITCH_IDLE)
        return COMMAND_DISALLOWED;
     
    /* 
     * Check if there are any SCO links up. 
     */
    if (LMscoctr_Get_Number_SYN_Connections() > 0)
    {
        return COMMAND_DISALLOWED;
    }

    // Role switching a master with more than one link should be allowed
	// if scatternet is supported.
#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED !=1)
    if (g_LM_config_info.num_master_acl_links > 1)
    {
        return COMMAND_DISALLOWED;
    }
#endif

    if ((!mFeat_Check_Slot_Offset(p_link->remote_features)) ||
        (!mFeat_Check_Role_Switch(p_link->remote_features)))
    {
        return UNSUPPORTED_REMOTE_FEATURE;
    }
    
    if (!mPol_Check_Master_Slave_Switch(p_link->link_policy_mode))
    {
        return COMMAND_DISALLOWED;
    }

    /*
     * Check that roles are different and only 1 or 0
     */
    if ((p_link->role ^ new_role) != 1)
    {
        return INVALID_HCI_PARAMETERS;
    }

#if (PRH_BS_CFG_SYS_LMP_PAUSE_ENCRYPTION_SUPPORTED==0)
    if (p_link->encrypt_enable)
    {
        return COMMAND_DISALLOWED;
    }
#else
    if (p_link->encrypt_enable && (!mFeat_Check_Pause_Encryption(p_link->remote_features)))
    {
        return ROLE_CHANGE_NOT_ALLOWED;
    }
#endif

    p_link->current_proc_tid_role = p_link->sec_current_proc_tid_role = p_link->role;

    LMch_Disable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_MSS);

    p_link->switch_state = SWITCH_LOCAL_ACTIVATION_PENDING;

#if (PRH_BS_CFG_SYS_LMP_PAUSE_ENCRYPTION_SUPPORTED==1)
    if (p_link->encrypt_enable)
    {
        p_link->encr_super_state = SET_ENCR_OFF;             
        p_link->encr_sub_state = W4_STOP_ENCR_REQ;
        p_link->encrypt_paused = ENCRYPT_PAUSED_INITIATED_BY_LOCAL_DEVICE;
        LMsec_peer_Send_LMP_Pause_Encryption_Req(p_link);
        return NO_ERROR;
    }
#endif
    
    LMconfig_LM_Set_Device_State(PRH_DEV_STATE_MSS_ACTIVE);
    if (p_link->role == SLAVE)
    {
        _Send_LMP_Connection_PDU(LMP_SLOT_OFFSET, p_link);    
    }
    _Send_LMP_Connection_PDU(LMP_SWITCH_REQ, p_link);
    return NO_ERROR;

#else
     return ROLE_CHANGE_NOT_ALLOWED;
#endif /* (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==) */
}

/**************************************************************************
 *  
 *  FUNCTION : LMconnection_LM_Switch_Req
 *  
 *************************************************************************/
void LMconnection_LM_Switch_Req(t_lmp_link *p_link)
{
    LMconfig_LM_Set_Device_State(PRH_DEV_STATE_MSS_ACTIVE);

    if(p_link->role == SLAVE)
    {
        _Send_LMP_Connection_PDU(LMP_SLOT_OFFSET, p_link);
    }
    
    _Send_LMP_Connection_PDU(LMP_SWITCH_REQ, p_link);
}

/**************************************************************************
 *  
 *  FUNCTION : LMconnection_LM_Switch_Role_Complete
 *
 *  DESCRIPTION :
 *  This function is called back by the LC when a role
 *  switch is complete.
 *
 *************************************************************************/
t_error LMconnection_LM_Switch_Role_Complete(t_LC_Event_Info* event_info)
{
#if (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1)
    t_lmp_link *p_link;

    p_link = LMaclctr_Find_Device_Index(event_info->deviceIndex);

    if (!p_link)
    {
        return UNSPECIFIED_ERROR;
    }

    p_link->poll_interval = DEFAULT_T_POLL;
    p_link->poll_position = DEFAULT_T_POLL;
	p_link->supervision_timeout = DEFAULT_SUPERVISION_TIMEOUT; /* #2725 */
	p_link->link_supervision_timeout = DEFAULT_SUPERVISION_TIMEOUT;

#if (PRH_BS_CFG_SYS_LMP_PAUSE_ENCRYPTION_SUPPORTED==1)
    /* postpone reenable of l2cap traffic until encryption reenabled */
    if (!p_link->encrypt_paused) 
#endif
    {
        LMch_Enable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_MSS);
    }

    if (NO_ERROR == event_info->status)
    {
        p_link->gen_switch_timeout_index = LMtmr_Set_Timer(
            0x01, LMconnection_Switch_Complete_Success, p_link, 1);
    }
    else
    {
        p_link->gen_switch_timeout_index = LMtmr_Set_Timer(
            0x01, LMconnection_Switch_Complete_Failure, p_link, 1);
    }

    LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_MSS_ACTIVE);
#endif /* (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1) */
    return NO_ERROR;
} 

#if (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1)   
/**************************************************************************
 *  
 *  FUNCTION : LMconnection_Switch_Complete_Success
 *
 *  DESCRIPTION :
 *  Handles a successful switch complete from the LC
 * 
 *************************************************************************/
static void LMconnection_Switch_Complete_Success(t_lmp_link *p_link)
{
    boolean activate_afh = FALSE;
    /*
     * Clear out the timer entry that called this function.
     */
    LMtmr_Clear_Timer(p_link->gen_switch_timeout_index); 
    p_link->gen_switch_timeout_index = 0;

#if (PRH_BS_CFG_SYS_LMP_PAUSE_ENCRYPTION_SUPPORTED==1)
    p_link->sec_current_proc_tid_role = !p_link->sec_current_proc_tid_role;
#endif

    if (MASTER == p_link->role) 
    {
        /*
         * I'm currently master, becoming slave
         */
        LMtmr_Clear_Timer(p_link->recent_contact_timeout_index);
        p_link->recent_contact_timeout_index = 0;

#if (PRH_BS_CFG_SYS_LMP_PAUSE_ENCRYPTION_SUPPORTED==1)
        LMtmr_Clear_Timer(p_link->encryption_key_refresh_index);
        p_link->encryption_key_refresh_index = 0;
#endif


        p_link->role = SLAVE;
        g_LM_config_info.num_master_acl_links--;
        g_LM_config_info.num_slave_acl_links++;
#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
        LC_Set_Active_Piconet(p_link->device_index, FALSE);
        if(DL_Max_Piconets())
#endif
        {
            /* 
             * When we obtain Slave role after MSS
             * this prevents any next connections with this device
             */
            LMconfig_LM_Set_Device_State(PRH_DEV_STATE_MAX_SLAVE_ACLS);
        }
    }
    else
    {
        /*
         * I'm currently slave, becoming master
         */
        /* 
         * Set Recent Contact Timer 
         */
        p_link->recent_contact_timeout_index =
            LMtmr_Set_Timer(
            (((p_link->supervision_timeout >> 1) > 100) || 
              (p_link->supervision_timeout ==0))
            ? 100 : (p_link->supervision_timeout >> 1),
            &LMconnection_LM_Recent_Contact_Timeout, p_link, 0);

#if (PRH_BS_CFG_SYS_LMP_PAUSE_ENCRYPTION_SUPPORTED==1)
        p_link->encryption_key_refresh_index = LMtmr_Set_Timer(
                (t_slots)PRH_ENCRYPTION_KEY_REFRESH_TIMEOUT,
                &LMsec_core_Encryption_Key_Refresh,p_link,0);
#endif

        p_link->role = MASTER;
        g_LM_config_info.num_slave_acl_links--;
        g_LM_config_info.num_master_acl_links++;

#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
        /*
         * Clear the change piconet timer if no longer in scatternet.
         */
        if (g_LM_config_info.num_slave_acl_links == 0)
        {
            LMch_Clear_Change_Piconet_Timer();
        }
#endif

        LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_MAX_SLAVE_ACLS);
    
        if(p_link->state != LMP_ACTIVE) 
        {
            /****************************************************************
             * If the role switch is occuring as part of the connection setup
             * send an LMP_Accept for the LMP_Host_Connection_Req
             ***************************************************************/
            p_link->current_proc_tid_role = MASTER;
            LM_Encode_LMP_Accepted_PDU(p_link, LMP_HOST_CONNECTION_REQ); 

#if (PRH_BS_DEV_EARLY_SET_AFH_ON_CONNECTION_ESTABLISHMENT==1)
            LMafh_Activate_AFH(p_link);
#endif

            /* 
             * change role before auth or setup complete 
             */
            p_link->current_proc_tid_role = MASTER;
            
            if( (ON == g_LM_config_info.authentication_enable) && 
				(LMssp_SSP_Enabled_On_Link(p_link)==0))
            {

                LMsec_core_Init_Authentication(p_link);

				p_link->state = W4_AUTHENTICATION;
            }
            else
            { 
                LMconnection_Local_Setup_Complete(p_link);              
            }
        }    
        else
        {
             /*
              * Just became a Master after role switch in connection
              * Ensure that AFH is activated, as appropriate
              */
             activate_afh = TRUE;
        }
    }

#if (PRH_BS_CFG_SYS_LMP_PAUSE_ENCRYPTION_SUPPORTED==1)
    if(p_link->encrypt_paused)
    {
        p_link->event_status = NO_ERROR; // role change event status
        p_link->encrypt_paused |= ENCRYPT_RESUME_PENDING_SWITCH_COMPLETE;
        if(p_link->encrypt_paused & ENCRYPT_PAUSED_INITIATED_BY_LOCAL_DEVICE)
            LMsec_peer_Resume_Encryption(p_link);
        return;
    }
#endif

    if ((MASTER == p_link->role) && (activate_afh))
    {
        LMafh_Activate_AFH(p_link);
    }

    /* 
     * Send a Role change event to the upper layer 
     */
    _Send_HC_Role_Change_Event(p_link, NO_ERROR);

    /* 
    * Reset the allow_switch flag - 
    *    as this is only pertinent to connection establishment 
    */
    
    p_link->allow_switch = 0;
    p_link->switch_state = SWITCH_IDLE;

}

/**************************************************************************
 *  
 *  FUNCTION : LMconnection_Switch_Complete_Failure
 *
 *  DESCRIPTION :
 *  Handles a failed switch complete from the LC
 *  
 *************************************************************************/
static void LMconnection_Switch_Complete_Failure(t_lmp_link *p_link)
{

	u_int8 switch_at_connection_setup = 0;
    /*
     * Clear out the timer entry that called this function.
     */
    LMtmr_Clear_Timer(p_link->gen_switch_timeout_index); 
    p_link->gen_switch_timeout_index = 0;
    
    if (SLAVE == p_link->role)
    {
      //  if (LMP_ACTIVE != p_link->state)
        if ((p_link->state & LMP_ACTIVE) != LMP_ACTIVE)
        {
		   /* 
		    * Slave fails a role switch during connection setup 
			*/

			// Bug #2299
			// If the role switch fails the LM shall continue with the 
			// creation of the connection unless this cannot be supported
			// due to limited resources. 
			switch_at_connection_setup = 1;
            /****************************************************************
			* If the role switch is occuring as part of the connection setup
			* send an LMP_Accept for the LMP_Host_Connection_Req
			***************************************************************/
			p_link->current_proc_tid_role = MASTER;
			LM_Encode_LMP_Accepted_PDU(p_link, LMP_HOST_CONNECTION_REQ); 
			
#if (PRH_BS_DEV_EARLY_SET_AFH_ON_CONNECTION_ESTABLISHMENT==1)
			LMafh_Activate_AFH(p_link);
#endif
			
			/* 
			* change role before auth or setup complete 
			*/
			p_link->current_proc_tid_role = MASTER;
			
			if( (ON == g_LM_config_info.authentication_enable) && 
				(LMssp_SSP_Enabled_On_Link(p_link)==0))
			{

				LMsec_core_Init_Authentication(p_link);

				p_link->state = W4_AUTHENTICATION;
			}
			else
			{ 
				LMconnection_Local_Setup_Complete(p_link);              
			}	
			
        }
        else
        {
            /*
             * Its a role switch after a connection setup
             *
             * Ensure that if classification is supported, it is restarted.
             */
            LMafh_Start_Classification_Reporting(p_link);
        }
    }
    

#if (PRH_BS_CFG_SYS_LMP_PAUSE_ENCRYPTION_SUPPORTED==1)
    if(p_link->encrypt_paused)
    {
        p_link->event_status = EC_ROLE_SWITCH_FAILED; // role change event status
        p_link->encrypt_paused |= ENCRYPT_RESUME_PENDING_SWITCH_COMPLETE;
        if(p_link->encrypt_paused & ENCRYPT_PAUSED_INITIATED_BY_LOCAL_DEVICE)
            LMsec_peer_Resume_Encryption(p_link);
        return;
    }
#endif
    
    /* 
     * Send a Role change event to the upper layer 
     */
	if (!switch_at_connection_setup)
#if (PRH_BS_CFG_SYS_HCI_V12_FUNCTIONALITY_SUPPORTED==1)
		_Send_HC_Role_Change_Event(p_link, EC_ROLE_SWITCH_FAILED);
#else
		_Send_HC_Role_Change_Event(p_link, UNSPECIFIED_ERROR);
#endif
    
    if(LMscoctr_Get_Number_SYN_Connections() || LMconfig_LM_Connected_As_Scatternet())
    {
        /*
         * Connect with role switch has failed, so immediate disconnect link to
         * benefit SCO/ACL persistance if connected to other slaves.
         */
        LMconnection_Disconnect_Acl(p_link);
        USLCchac_Set_Device_State(Connection);
        DL_Set_Ctrl_State(DL_Get_Local_Device_Ref(), CONNECTED_TO_SLAVE);
    }
    
    /* 
     * Reset the allow_switch flag - 
     *    as this is only pertinent to connection establishment 
     */
    p_link->allow_switch = 0;
    p_link->switch_state = SWITCH_IDLE;

}

#if (PRH_BS_CFG_SYS_LMP_PAUSE_ENCRYPTION_SUPPORTED==1)
/**************************************************************************
 *  
 *  FUNCTION : LMconnection_Switch_Complete_Event
 *
 *************************************************************************/
void LMconnection_Switch_Complete_Event(t_lmp_link *p_link)
{
    LMch_Enable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_MSS);

    _Send_HC_Role_Change_Event(p_link, p_link->event_status);

    if ((MASTER == p_link->role) && (p_link->state == LMP_ACTIVE)
        && (p_link->event_status == NO_ERROR))
    { /* Just became a Master after role switch in connection
              * Ensure that AFH is activated */
        LMafh_Activate_AFH(p_link);
    }
    
    p_link->allow_switch = 0;
    p_link->switch_state = SWITCH_IDLE;
}
#endif

#endif

#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
/************************************************************************
 *  FUNCTION : LMconnection_Validate_SCO_Admission
 *
 *  DESCRIPTION :
 *  Checks if an incoming SCO can be added.
 *      NO_ERROR if sco can be added, otherwise reason
 *  
 * The rules below are valid for a single SCO
 * The rigorous rules for multiple SCOs are tbc.
 *
 * Reject SCO if
 * 1. No ACL connection
 * 2. Detach pending on ACL 
 * 3. In Park 
 * 4. Packet Types are valid
 * 5. more than one ACL active refuse a HV1 link request 
 * 6. Violate max 1 HV1, 2 HV2, 3 HV3 or SCO active and packet type mismatch
 * 7. SCO setup collision
 * 8. Air mode is not a local supported feature
 *************************************************************************/
static t_error LMconnection_Validate_SCO_Admission(
    t_lmp_link *p_link, t_packetTypes packet_types, u_int8 lm_air_mode)
{
    if (p_link == 0)
    {
        return NO_CONNECTION;
    }

    if(p_link->operation_pending == LMP_DETACH)
    {
        return COMMAND_DISALLOWED;
    }

    if (p_link->state == LMP_PARK_MODE)
    { 
        return COMMAND_DISALLOWED;
    }

    /*
     * Check that packet types include SCO HV1, HV2 and HV3 only
     * For HCI based request, an error may occur
     * For LMP based request, this error must not occur
     */             
    if ((!(packet_types & (HV1_BIT_MASK|HV2_BIT_MASK|HV3_BIT_MASK)))
      || (packet_types & ~(HV1_BIT_MASK|HV2_BIT_MASK|HV3_BIT_MASK)))
    {
        return INVALID_HCI_PARAMETERS;
    }

    /*
     * Currently, HV1 will be selected if multiple packets specified
     * Hence, refuse if in Piconet and HV1 selected
     */
    if ((g_LM_config_info.num_acl_links > 1) && (packet_types & HV1_BIT_MASK))
    {
        return COMMAND_DISALLOWED;
    }

    /*
     * Check if collisions whereby a SCO is currently being negotiated to
     * any device (Add SCO, Remove SCO and SCO Change Packet)
     */
    if (LMscoctr_Get_SCO_Transitioning_Ref() && 
        p_link->role == MASTER)
    {
        return LMP_ERROR_TRANSACTION_COLLISION;
    }

    /*
     * Disallow if already all SCO bandwidth allocated, or existing SCO packet
     * not included
     */
    if (!LMscoctr_Is_Bandwidth_Available_For_SCO_Packet_Types(packet_types))
    {
        return MAX_NUM_CONNECTIONS;
    }

    if(!LMscoctr_Is_LM_Air_Mode_Supported(lm_air_mode))
    {
        return SCO_AIR_MODE_REJECTED;
    }

    return NO_ERROR;
}

/**************************************************************************
 *  
 *  FUNCTION : LMconnection_LM_Add_SCO_Connection
 *
 *  DESCRIPTION :
 *   Invoked by the higher layers to add a SCO connection to a given
 *   ACL link.
 *   Sends the request to the peer LM to add a SCO connection with a   
 *   particular packet type (HV1-3) 
 *   Called by HCch_HCI_Add_SCO_Connection
 *
 *   The Following Steps have to be performed in a Master
 *     1.  Check that an ACL link is active.
 *     2.  Allocate a SCO link structure.
 *     3.  Add the SCO link to the ACL
 *   IN A MASTER
 *     4.  Derive the SCO parameters
 *     5.  Send an LMP_Sco_Link_Req
 *   IN A SLAVE
 *     4.  Set the SCO handle = 0
 *     5.  Send an LMP_Sco_Link_Req
 *************************************************************************/
t_error LMconnection_LM_Add_SCO_Connection(
    t_lmp_link *p_link, t_packetTypes packet_types,
    u_int16 voice_setting)
{
    t_error    status;
    t_sco_info *p_sco_link;
    u_int8 air_coding;

#if 1// GF 2 Sept (BUILD_TYPE!=UNIT_TEST_BUILD)
    if (g_LM_config_info.default_sco_route_via_hci)
		voice_setting  |= PRH_BS_HCI_VOICE_SETTING_FOR_SCO_VIA_HCI;
#endif
    air_coding = LMscoctr_Get_Air_Coding(voice_setting);

    /*
     * If in Piconet disallow HV1 as a packet type
     */
    if (g_LM_config_info.num_acl_links > 1)
    {
        packet_types &= ~HV1_BIT_MASK;
    }

    status = LMconnection_Validate_SCO_Admission(p_link, packet_types, 
        LMscoctr_Map_AirMode_HCI2LMP(air_coding));
    if (status != NO_ERROR)
    {
        /*
         * If collision detected, then must return to HCI COMMAND_DISALLOWED
         */
        if (status==LMP_ERROR_TRANSACTION_COLLISION)
        {
            status = COMMAND_DISALLOWED;
        }
        return status;
    }

    /* 
     * Check if the remote device supports the SCO & the requested packet types 
     */    
    if(!(mFeat_Check_Sco_Link(p_link->remote_features)))
    {
        return UNSUPPORTED_REMOTE_FEATURE;
    }
    
    if(!( (packet_types & HV1_BIT_MASK) ||
         ((packet_types & HV2_BIT_MASK) && (mFeat_Check_HV2_Packets(p_link->remote_features))) ||
         ((packet_types & HV3_BIT_MASK) && (mFeat_Check_HV3_Packets(p_link->remote_features))) ))
    {
        return UNSUPPORTED_REMOTE_FEATURE;
    }
    else
    {
        if(!mFeat_Check_HV2_Packets(p_link->remote_features))
            packet_types &= ~HV2_BIT_MASK;
        if(!mFeat_Check_HV3_Packets(p_link->remote_features))
            packet_types &= ~HV3_BIT_MASK;
    }

    if( (HCI_AIR_MODE_U_LAW == air_coding) && 
        !mFeat_Check_U_Law(p_link->remote_features) )
    {
        return UNSUPPORTED_REMOTE_FEATURE;
    }

    if( (HCI_AIR_MODE_A_LAW == air_coding) && 
        !mFeat_Check_A_Law(p_link->remote_features) )
    {
        return UNSUPPORTED_REMOTE_FEATURE;
    }

    if( (HCI_AIR_MODE_CVSD == air_coding) && 
        !mFeat_Check_CVSD(p_link->remote_features) )
    {
        return UNSUPPORTED_REMOTE_FEATURE;
    }


     status = _Cancel_Pending_Page_Inquiry();
    /* 
     * Allocate a SCO link structure. 
     * Associate it with an ACL link via device_index and select configuration
     * Note the SCO link is not activated at this stage
     */    
    p_sco_link = LMscoctr_Alloc(
         p_link->device_index, 
         voice_setting, 
         SCO_LINK);
    if (p_sco_link != 0)
    { 
        p_link->current_proc_tid_role = p_link->role;

        LMscoctr_Derive_SCO_Parameters(p_sco_link, packet_types, p_link->role, 0 /* use local air-mode flags */);
        _Send_LMP_SCO_Link_Req(p_link, p_sco_link);

        if(p_link->role == MASTER)
        {
            LMpol_Init_SCO(p_link, p_sco_link->sco_index);
        }
        else 
        {
            /* 
             * A Handle of Zero is required for a Slave initiated SCO Setup 
             * Dsco, timing control flags are invalid (but set to zero above).
             * Free resource and await the Master's request.
             */
             LMscoctr_Store_Topical_Voice_Setting(p_sco_link->voice_setting);
             LMscoctr_Free(p_sco_link);
        }
    }
    else
    {
        status = MAX_NUM_CONNECTIONS;
    }

#if ((PRH_BS_CFG_SYS_HCI_V12_FUNCTIONALITY_SUPPORTED==1) && (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1))	
	_LMch_host_v12_functionality_confirmed = FALSE;
#endif

	return status;
}

/**************************************************************************
 * FUNCTION : LMconnection_LM_Change_SCO_Packet_Type
 *
 * INPUT  : p_link       - A pointer to the link_container for sco device
 *          p_sco_link   - A pointer to the sco link structure.
 *          packet_types - The new packet types for the SCO.
 * 
 * DESCRIPTION
 * Changes the packet type for a SCO link. Send an "LMP_SCO_Link_Req"
 * to the peer and store the new SCO parameters in the 
 * SCO link used for negotiation.
 * First, there is a need to check for possible collisions!
 *
 * Preconditions:
 * 1. Only one SCO exists (since cannot change packet type if multiple).
 * B325
 * 2. There is no transaction collision.
 * 3. All parameters are valid.
 *****************************************************************************/
t_error LMconnection_LM_Change_SCO_Packet_Type(
     t_lmp_link* p_link, t_sco_info *p_sco_link, t_packetTypes packet_types)
{
    t_sco_info *p_sco_renegotiate_link;

    /* 
     * Extract and store parameters in the renegotiated sco link structure 
     * based on selected packet types.  Always returns valid pointer!
     * The air mode doesn't change.
     */
    p_sco_renegotiate_link = LMscoctr_Allocate_Negotation_SCO_Link(
        p_sco_link, p_link->role, packet_types, p_sco_link->lm_air_mode);

    p_link->current_proc_tid_role = p_link->role;
    _Send_LMP_SCO_Link_Req(p_link, p_sco_renegotiate_link); 

    if (p_link->role==SLAVE)
    {
        /*
         * Immediately free negotiation container, since it has no value.
         */
        LMscoctr_Free_Negotiation_SCO_Link();
    }
    return NO_ERROR;
}
#endif /* (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)*/

/**************************************************************************
 * FUNCTION : LMconnection_LM_Incoming_Link_Inf
 *
 * INPUT  : device_index - The device index for the LC link.
 *          p_bd_addr    - A pointer to the Address of the peer
 *                         device on the link.
 *          device_class - The class of the peer device on the Link.
 * 
 * OUTPUT : none
 * 
 * DESCRIPTION
 * Informs the LM of the establishment of a baseband link. The 
 * Bd_Addr and the Device_Class are delivered to the LM to allow
 * them to be stored in the ACL Link Structure.
 *
 * Following Steps are performed :
 *   Step 1/ Allocate a slave role for the link.
 *   Step 2/ Turn off scanning.
 *   Step 3/ Initialise the supervision timer.
 *   Step 4/ Initialise the Bd_Addr in the link structure.
 *   Step 5/ Initialise the Device_Class in the link structure.
 *   Step 6/ Initialise Setup Complete bit mask.
 *************************************************************************/
void LMconnection_LM_Incoming_Link_Inf(t_deviceIndex device_index,
    t_bd_addr* p_bd_addr, t_classDevice device_class)
{
    t_lmp_link *p_link;

    p_link = LMaclctr_Find_Device_Index(device_index);
    pDebugAssert(PRTH_HW_ERROR_LMP_CH,(p_link == 0));

    /*
     * NOTE ROLE MUST BE SET PRIOR TO TURNING OFF SCANNING
     */
    p_link->role = SLAVE; 

#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
    LC_Set_Active_Piconet(p_link->device_index, FALSE);
    if(DL_Max_Piconets())
#endif
    {
        LMconfig_LM_Set_Device_State(PRH_DEV_STATE_MAX_SLAVE_ACLS);
    }

    LMconfig_LM_Set_Device_State(PRH_DEV_STATE_ACLS_ACTIVE);
    g_LM_config_info.num_acl_links++;
    g_LM_config_info.num_slave_acl_links++;
#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
	if (LMconfig_LM_Connected_As_Scatternet())
    {
        u_int16 piconet_service_time;
        /* 
         * The following code ensures that we stay on the piconet
         * corresponding to this link for an extra period of time [PICONET_SERVICE_TIME/2]
         * used to receive LMP connection request
         *
         * In the future this code should be replaced by a more advanced scheduler supporting
         * both piconet and scattermode scheduling.
         */
        piconet_service_time=PRH_BS_PICONET_SERVICE_TIME/(g_LM_config_info.num_acl_links-g_LM_config_info.links_in_low_power);


		/* If there is a Sniff Link then I need to consider this when I set the 
		 * next Change Piconet Time. If the interval to the Next Wakeup is less
		 * than the piconet_service_time then we need to use the Wakeup time to
		 * set the piconet timer.
		 */

		if (g_LM_config_info.links_in_low_power != 0)
		{
			LMch_Set_Next_Change_Piconet_Time(LMpol_Get_Interval_To_Next_Wakeup() - LMP_SLOT_GAURD_FOR_CHANGE_PICONET);
		}
		else
		{
			LMch_Set_Next_Change_Piconet_Time(piconet_service_time);

		}

    }
#endif


    p_link->supervision_timeout_index = LMtmr_Set_Timer((p_link->supervision_timeout),
        &LMconnection_LM_Disconnect_Inf, p_link, 0);

    p_link->msg_timer = LMtmr_Set_Timer(PRH_LMP_MSG_TIMEOUT,LM_Encode_Msg_Timeout, p_link,1);

    LMacl_Write_Bd_Addr(p_link,p_bd_addr);
    p_link->device_class = device_class;

    /* 
     * Initialise the Setup Complete Bitmask & connection context 
     * prior to the receipt of the first LMP message 
     */
    p_link->operation_pending = LMP_HOST_CONNECTION_REQ;
    p_link->state = W4_LMP_ACTIVE;
    p_link->context = LMch_NO_CONNECTION; 
}

#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
/*************************************************************************
 *
 *  FUNCTION : LMch_Clear_Change_Piconet_Timer
 *
 *  Clears change piconet timer if active.
 *
 *************************************************************************/
void LMch_Clear_Change_Piconet_Timer(void)
{
    if (piconet_timer)
    {
        if (g_LM_config_info.num_acl_links)
        {
            LMch_Change_Piconet(0); /* goto remaining piconet if exists */
        }

        LMtmr_Clear_Timer(piconet_timer); /* clear scatternet timer */
        piconet_timer = 0;
    }
}

/*************************************************************************
 *
 *  FUNCTION : LMch_Set_Next_Change_Piconet_Time
 *
 *  Remain on piconet for specified time before next change piconet.
 *
 *************************************************************************/
void LMch_Set_Next_Change_Piconet_Time(t_slots timeout)
{
    if (piconet_timer)
       LMtmr_Reset_Timer(piconet_timer, timeout);
    else
       piconet_timer = LMtmr_Set_Timer(timeout, LMch_Change_Piconet,0,0);
}

/*************************************************************************
 *
 *  FUNCTION : LMch_Adjust_Next_Change_Piconet_Time
 *
 *  Adjust the next change piconet time by extend/curtail the timeout.
 *
 *************************************************************************/
void LMch_Adjust_Next_Change_Piconet_Time(t_slots timeout, boolean extend)
{
    if (piconet_timer)
       LMtmr_Modify_Timer(piconet_timer, timeout, extend);
    else
       piconet_timer = LMtmr_Set_Timer(timeout, LMch_Change_Piconet,0,0);
}


/*************************************************************************
 *
 * FUNCTION : LMch_Change_Piconet
 *
 * Default handler for switch to other piocnets when in scatternet.
 *
 *************************************************************************/
void LMch_Change_Piconet(t_lmp_link *p_link)
{
    u_int16 piconet_service_time=0;
    t_deviceIndex active_device=LC_Get_Active_Piconet();
    boolean       aclActive=FALSE;
    t_devicelink  *pDL=0;
    t_deviceIndex device;
			
	//**********************************************************
	// Change Piconet called with p_link as parameter. 
	// This is only used for handling role switch 
	// e.g Handling LMP_Switch_Req and Sending LMP_SWITCH_REQ
	// *********************************************************
    if (p_link)
    {
	   /*
	    * Change piconet to activate a specific p_link as requested.
		*/
		LC_Set_Active_Piconet((p_link->device_index),TRUE);

		piconet_service_time = PRH_BS_PICONET_SERVICE_TIME/LMconfig_LM_Num_Piconets();
		
		// GF 21 Nov 2010 -- Should below be replaced by call to Adjust Wake up time ?.
		// If I have links in Low Power 
		if (g_LM_config_info.links_in_low_power)
		{

			if (LMpol_Get_Interval_To_Next_Wakeup() > LMP_SLOT_GAURD_FOR_CHANGE_PICONET)
				piconet_service_time = LMpol_Get_Interval_To_Next_Wakeup() - LMP_SLOT_GAURD_FOR_CHANGE_PICONET;
			//else        
				//piconet_service_time = 0;	
		}

		LMch_Set_Next_Change_Piconet_Time(piconet_service_time);
    }

	//*****************************************************************
	// Change Piconet - call with p_link == 0 and No Scatternet Active.
	// This cycles through all the device links finds the first one connected.
	// this then set as the Active Piconet.
	// 
	// This usually occurs on the exit from scatternet 
	// as part of call to LMch_Clear_Change_Piconet_Timer
	//*****************************************************************

    else if (!LMconfig_LM_Connected_As_Scatternet())
    {
        for (device=1; (!aclActive) && (device<SYSconfig_Get_Max_Active_Devices()); device++)
        {
            pDL = DL_Get_Device_Ref(device);
            if (DL_Get_Ctrl_State(pDL) != NOT_CONNECTED)
            {
				/*
				 * Always invoke LC_Set_Active_Piconet regardless of (active_device != device), as
				 * required in such cases when return from CHAC procedures. Also - as no longer in a 
				 * scatternet scenario here - should NOT invoke LMch_Set_Next_Change_Piconet_Time.
				 */
 				LC_Set_Active_Piconet(device, TRUE);
				aclActive = TRUE;
            }
        }

		if (!aclActive) /* No connections => Return to local index */
		{
			USLCsched_Local_Piconet_Request();
		}
    }

	//*********************************************************************
	// Change Piconet - call with p_link == 0 and Scatternet Active.
	// If links with low power are due wake-up within slot guard + N
	// Set the active Piconet to the one indicated by 
	//               g_LM_config_info.next_wakeup_link->device_index
	// Also set the piconet timer to expire in PRH_BS_PICONET_SERVICE_TIME
	//*********************************************************************

    else if ((g_LM_config_info.links_in_low_power) && (LMpol_Get_Interval_To_Next_Wakeup() <= (LMP_SLOT_GAURD_FOR_CHANGE_PICONET+2)))
    {
        /*
         * Activate next wakeup link. If none specified, then this must be a
         * park master (multiple p_link), so use latest park info instead.
         */
		extern t_SYS_Config g_sys_config;

		if (g_LM_config_info.next_wakeup_link)
            device = g_LM_config_info.next_wakeup_link->device_index;
#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
		else if (g_LM_config_info.num_links_parked)
			device = g_LM_config_info.park_info.device_index;
#endif
		else
			device = g_sys_config.local_device_index;
			
			LC_Set_Active_Piconet(device, TRUE);
			LMch_Set_Next_Change_Piconet_Time(PRH_BS_PICONET_SERVICE_TIME/LMconfig_LM_Num_Piconets());
    }

	//*********************************************************************
	// Change Piconet - call with p_link == 0 and Scatternet Active.
	// No links_in_low_power due wake-up within the above slot guard + N.
	// => Select an next aclActive based on least-recently serviced.
	//*********************************************************************
	
	else
    {
        /* 
         * Select the piconet for the next aclActive. These are activated in
         * sequential order, for basic piconet service time distribution.
         */		
		u_int8 dev_cnt = SYSconfig_Get_Max_Active_Devices();

        device = (((active_device+1)<SYSconfig_Get_Max_Active_Devices())?(active_device+1):(1));

		while ((!aclActive) && (dev_cnt--))
        {
            pDL = DL_Get_Device_Ref(device);
            if (DL_Get_Ctrl_State(pDL) != NOT_CONNECTED)
            {
                p_link = LMaclctr_Find_Device_Index(device);
                /*
                 * Scheduling of piconets in low power mode are controlled
                 * autonomously & should not activate by default hander.
                 */
				if(p_link)
				{
					if(LMconfig_LM_Check_Device_State(PRH_DEV_STATE_MSS_ACTIVE))
					{
						// If there is a device waiting to complete MSS ! Then 
						// Stick with that Piconet.
						if (p_link->switch_state == SWITCH_PEER_ACTIVATION_PENDING)
						{
							aclActive = TRUE;
						}
					}
					else if (mLMpol_Is_Transmission_State(p_link->state))
					{
						aclActive = TRUE;
					}
                }
            }
            device = (((device+1)<SYSconfig_Get_Max_Active_Devices())?(device+1):(1));
        }
		
        /*
		* Change the current Active Piconet to selected next aclActive. If no
		* other aclActive identified, remain on the current Active Piconet.
		*/
        if (aclActive)
        {
            LC_Set_Active_Piconet(DL_Get_Device_Index(pDL), TRUE);
        }
		
        piconet_service_time = (PRH_BS_PICONET_SERVICE_TIME/LMconfig_LM_Num_Piconets());
		
        if (g_LM_config_info.links_in_low_power)
		{
			u_int16 wakeup_interval = LMpol_Get_Interval_To_Next_Wakeup();

			if ((wakeup_interval < piconet_service_time) && (wakeup_interval >= LMP_SLOT_GAURD_FOR_CHANGE_PICONET))
			{
				piconet_service_time = wakeup_interval - LMP_SLOT_GAURD_FOR_CHANGE_PICONET;
			}
		}
		
        LMch_Set_Next_Change_Piconet_Time(piconet_service_time);
    }
}
#endif

/**************************************************************************
 * FUNCTION : LMconnection_LM_Read_Remote_Supported_Features
 * 
 * DESCRIPTION
 * Used by the higher layers to request the supported Link Manager features of
 * the peer on link.
 *************************************************************************/
t_error LMconnection_LM_Read_Remote_Supported_Features(t_lmp_link *p_link)
{
    if(p_link->operation_pending == LMP_DETACH)
        return COMMAND_DISALLOWED;

    p_link->current_proc_tid_role = p_link->role;
    _Send_LMP_Connection_PDU(LMP_FEATURES_REQ,p_link);

    return NO_ERROR;
}


/**************************************************************************
 *
 * FUNCTION : LMconnection_LM_Read_Remote_Extended_Features
 * 
 * DESCRIPTION
 *
 * Used by the higher layers to request the supported Link Manager 
 * extended features of the peer on link.
 *
 *************************************************************************/
t_error LMconnection_LM_Read_Remote_Extended_Features(t_lmp_link *p_link, u_int8 page)
{
    if(p_link->operation_pending == LMP_DETACH)
        return COMMAND_DISALLOWED;

    p_link->current_proc_tid_role = p_link->role;
    _Send_LMP_Features_Req_Ext(p_link, page);

    return NO_ERROR;
}


/*****************************************************
 *****************************************************
 ***    PEER INTERFACE TO REMOTE LMP              **** 
 *****************************************************
 *****************************************************/

/**************************************************************************
 *  
 *  FUNCTION : LMconnection_LMP_Host_Connection_Req
 *
 *  DESCRIPTION :
 *    Handles an Incoming Host_Connection_Req PDU. If the LM link is in an
 *    idle state, a Connection_Ind event is sent to the higher layer.
 *
 *************************************************************************/
t_error LMconnection_LMP_Host_Connection_Req(t_lmp_link *p_link, t_p_pdu p_pdu)
{
    if(W4_LMP_ACTIVE == p_link->state)
    {
            return LMconnection_Handle_Incoming_LM_Connection_Request(p_link);
    }
    else 
    {   
        /****************************************************************
         * Send LMP_Not_Accepted PDU to peer on the link with the return
         * opcode set to LMP_Host_Connection_Req
         ****************************************************************/
        return REMOTE_USER_TERMINATED_CONNECTION;
    }
    return NO_ERROR;
}


/**************************************************************************
 *  
 *  FUNCTION : LMconnection_Handle_Incoming_LM_Connection_Request
 *
 *  DESCRIPTION :
 *    Handles an Incoming Host_Connection_Req PDU. 
 *
 *************************************************************************/
t_error LMconnection_Handle_Incoming_LM_Connection_Request(t_lmp_link* p_link)
{
    t_error status = NO_ERROR;

     /*********************************************************
      * Check the connection event filters to determine if the
      * connection should be auto-accepted or not
      *********************************************************/        
     p_link->context = LMch_LMP_CONNECTION;
     p_link->current_proc_tid_role = p_link->role ^ 0x01;
#if (PRH_BS_CFG_SYS_FILTERS_SUPPORTED == 1)
     switch(LMfltr_Connection_Filter_Check(&p_link->bd_addr,p_link->device_class,ACL_LINK))
         {
         case AUTO_ACCEPT:
             /*
              * Send LMP_Accepted PDU to peer on the link
              */
             LM_Encode_LMP_Accepted_PDU(p_link,LMP_HOST_CONNECTION_REQ);
             if((ON == g_LM_config_info.authentication_enable)&& 
				(LMssp_SSP_Enabled_On_Link(p_link)==0))
             {

                 LMsec_core_Init_Authentication(p_link);

                 p_link->state = W4_AUTHENTICATION;
             }
             else
             { 
                 LMconnection_Local_Setup_Complete(p_link);
             }
         break;

         case AUTO_ACCEPT_WITH_MSS:

#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
            /* 
             * The following change piconet ensures that we return to (or stay on) the piconet
             * corresponding to this link for an extra period of time [PICONET_SERVICE_TIME/2]
             *
             * This is done to support the scenario where a Master Slave Switch imediately follows
             * connection establishment.
             *
             * In the future this code should be replaced by a more advanced scheduler supporting
             * both piconet and scattermode scheduling.
             */
             LMpol_Change_Piconet(p_link->device_index, (PRH_BS_PICONET_SERVICE_TIME/2));
#endif
             p_link->current_proc_tid_role = SLAVE;
             p_link->switch_state = SWITCH_LOCAL_ACTIVATION_PENDING;
             LMconfig_LM_Set_Device_State(PRH_DEV_STATE_MSS_ACTIVE);
             _Send_LMP_Connection_PDU(LMP_SLOT_OFFSET,p_link);
             _Send_LMP_Connection_PDU(LMP_SWITCH_REQ,p_link);
             p_link->state = W4_LMP_ACTIVE;
         break;

         case DONT_AUTO_ACCEPT:
             /*
              * If the connection request event is masked off
              * by the host go to the default condition
              * (rejection), else generate a connection
              * request PDU.
              */
             if(0 == HCeg_Is_Event_Masked_On(HCI_CONNECTION_REQUEST_EVENT))
             {
                 p_link->context = LMch_TEMPORARY_CONNECTION;
                 status = REMOTE_USER_TERMINATED_CONNECTION;
             }
             else
             {
                 _Send_HC_Connection_Request(p_link, ACL_LINK);
             }
         break;

/*     case REJECT : */
         default:
             /****************************************************************
              * Send LMP_Not_Accepted PDU to peer on the link with the return
              * opcode set to LMP_Host_Connection_Req. Change link context to 
              * TEMPORARY_CONNECTION to ensure that the Connection complete
              * event does not go to the higher layers when an LMP_Detach is 
              * received from the peer.
              ****************************************************************/
             p_link->context = LMch_TEMPORARY_CONNECTION;
             status = REMOTE_USER_TERMINATED_CONNECTION;
         break;
    }
#else
    if(0 == HCeg_Is_Event_Masked_On(HCI_CONNECTION_REQUEST_EVENT))
    {
        p_link->context = LMch_TEMPORARY_CONNECTION;
        status = REMOTE_USER_TERMINATED_CONNECTION;
    }
    else
    {
        _Send_HC_Connection_Request(p_link, ACL_LINK);
    }
#endif

    return status;
}


/**************************************************************************
 *  
 *  FUNCTION : LMconnection_LMP_Accepted
 *  
 *  DESCRIPTION :
 *    Handles an Incoming LMP Accepted PDU. The actions to be performed on 
 *    receipt of this PDU are heavily dependent on the link state & the 
 *    return operation code in the LMP Accepted PDU
 *
 *************************************************************************/
t_error LMconnection_LMP_Accepted(t_lmp_link *p_link, u_int16 opcode)
{
    t_error status = NO_ERROR;  

    switch(opcode)
    {
    case LMP_HOST_CONNECTION_REQ :
         
         /* 
          * Ensure that the accepted only comes in the setup phase 
          */
         if(p_link->setup_complete_bit_mask != LMP_No_Setup_Complete)
         {
             return COMMAND_DISALLOWED;
         }
		
         /* 2848 */
         p_link->host_connection_request_accepted = TRUE;

         /* 
          * Check if a role switch was performed 
          */
         if (p_link->role != MASTER )
         { 
             /* 
              * Role switch took place
              * - Role Switch event to be included here 
              */
         }

         /*
          * After receipt of an LMP Accept for a LMP_Host_Connection_Req
          * one of 3 things can happen :
          *   1/ Authentication
          *   2/ Encryption
          *   3/ Setup Complete
          */

#if (PRH_BS_DEV_EARLY_SET_AFH_ON_CONNECTION_ESTABLISHMENT==1)
         LMafh_Activate_AFH(p_link);
#endif

         if((ON == g_LM_config_info.authentication_enable)&& 
			(LMssp_SSP_Enabled_On_Link(p_link)==0))
         {    

             LMsec_core_Init_Authentication(p_link);

             p_link->state = W4_AUTHENTICATION;
         }
         else
         { 
             LMconnection_Local_Setup_Complete(p_link);              
         }
         break;

#if (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1)
    case LMP_SWITCH_REQ :
        /*
         * The LC is instructed to perform the switch 
         * A callback from the LC occurs when it is finished processing
         * The functions called back are either
         * LMconnection_Role_Switch_Complete_Success() or
         * LMconnection_Role_Switch_Complete_Failure().
         */
#if (PRH_BS_CFG_SYS_PERSISTENT_SCO_IN_SCANS_SUPPORTED==1)
        /*
         * Persistent SCO in scans - Switch into piconet with SCO.
         * Change back to M(SCO) until TDD.
         */
        if (LMconfig_LM_Connected_As_SCO_Master())
        {
             DL_Set_Ctrl_State(DL_Get_Local_Device_Ref(), CONNECTED_TO_SLAVE);
             LC_Restore_Piconet();
        }
#endif
        break;
#endif 

#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
    case LMP_SCO_LINK_REQ :
    {
        t_sco_info *p_sco_link;

        if (p_link->role == SLAVE)
        {
            return COMMAND_DISALLOWED;
        }

        /* 
         * Determine is this a new link being established or an existing
         * SCO link being re-negotiated 
         */
        p_sco_link = LMscoctr_Find_SCO_Under_Negotiation(MASTER);
        if (p_sco_link)
        {
            /* 
             * Existing SCO link
             * Change to the negotiated SCO packet type
             *    (parameters have been successfully negotiated). 
             */
            LMscoctr_Accept_And_Free_Negotiated_SCO_Link(p_sco_link);
            _Send_HC_Connection_Packet_Type_Changed_Event_SCO(p_sco_link, NO_ERROR);
#if (PRH_BS_DEV_MULTISLOT_ACL_WITH_SCO_SUPPORTED==1)
            /*
             * Changing an existing SCO link may require adjusting of the
             * ACL packet types on every link.
             */
            LMpol_Adjust_ACL_Packet_Types_On_All_Links(LMcontxt_CHANGED_SCO);
#endif
        } 
        else
        {
            /* 
             * New SCO link 
             * The parameters in the p_sco_link are valid
             * and the SCO connection which is transitioning is setup.
             */
            p_sco_link = LMscoctr_Get_SCO_Transitioning_Ref();
            if (p_sco_link)
            {
                /*
                 * Adding a new SCO link may require adjusting of the
                 * ACL packet types on every link.
                 */
                LMpol_Adjust_ACL_Packet_Types_On_All_Links(LMcontxt_ENTERING_SCO);
                /*
                 * Send Successful Connect Complete Event to the Higher Layer
                 */
                _Send_HC_Connection_Complete_Event_SCO(p_link, 
                    p_sco_link->connection_handle, NO_ERROR);
            }
            else
            {
                return NO_CONNECTION;
            }
        }
        p_sco_link->state = SCO_ACTIVATION_PENDING;
        

        break;
    }

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    case LMP_ESCO_LINK_REQ :
    {
        LMconnection_LMP_Accepted_Ext_eSCO_Link_Req(p_link);
    }
    break;

    case LMP_REMOVE_ESCO_LINK_REQ :
    {
        t_sco_info *p_esco_link;
        p_esco_link = LMscoctr_Get_Topical_eSCO_Link();
        if(p_esco_link)
        {
            status = LMconnection_LM_Disconnect_SCO(p_esco_link, 
                p_link->detach_reason);

			LMpol_Adjust_ACL_Packet_Types_On_All_Links(LMcontxt_LEAVING_SCO);
        }
    }
    break;
#endif

    case LMP_REMOVE_SCO_LINK_REQ :
        /*
         * Detach reason was stored in LMconnection_LM_Disconnect_Req() 
         * as result of HCI_DISCONNECT invocation.
         * If the sco does not exist, the function returns NO_CONNECTION
         */
        status = LMconnection_LM_Disconnect_SCO(
            LMscoctr_Get_SCO_Transitioning_Ref(), p_link->detach_reason);

        /*
         * Removing a SCO link may require the adjusting of
         * ACL packet types on all links.
         */
        LMpol_Adjust_ACL_Packet_Types_On_All_Links(LMcontxt_LEAVING_SCO);
        break;       
#endif 

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
	case LMP_PACKET_TYPE_TABLE_REQ :
         /* 
          * Request Local device to restart L2CAP data transmission.
          */
		 if (p_link->ptt_req != PTT_REQ_IDLE)
		 {
			 LC_Set_PTT_ACL(p_link->device_index, p_link->ptt_req);
			 p_link->ptt_req = PTT_REQ_IDLE;
			 LMqos_Update_Allowed_Packets(p_link,p_link->max_slots_out, p_link->rate);
			 LMch_Enable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_PTT);
		 }
         break;
#endif

    }
    return status;
}

/**************************************************************************
 *  
 *  FUNCTION : LMconnection_LMP_Not_Accepted
 *
 *  DESCRIPTION :
 *    Handles an Incoming LMP_Not_Accepted PDU, which indicates
 *    that the remote host has rejected the connection.
 *    The actions performed in this state are dependent on the Opcode
 *    for the last LMP PDU sent to the peer device.
 *
 ************************************************************************/  
t_error LMconnection_LMP_Not_Accepted(
    t_lmp_link *p_link, u_int16 return_opcode, t_error reason)
{
    switch (return_opcode)
    {
    case LMP_HOST_CONNECTION_REQ :
        /*****************************************************************
         * If Im an ACL and the peer has not accepted the connection I 
         * send a Connection_Complete to the local HC and an LMP_DETACH to 
         * the peer and wait for the detach Ack.
         *****************************************************************/
        p_link->detach_reason = reason;
        LMconnection_Send_LMP_Detach(p_link,reason);
        break;

#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
    case LMP_SCO_LINK_REQ : 
    {
        /****************************************************************
         * Send Connect Complete Event to the Higher Layer.
         * the pending handle in the Link Structure is used to identify
         * the instance of the SCO or ACL . In this case the status is set
         * to the reason for the NOT_ACCEPT.
         *****************************************************************/ 
        /*
         * First determine if the SCO is being renegotiated
         */
        t_sco_info *p_sco_link;
        p_sco_link = LMscoctr_Find_SCO_Under_Negotiation(p_link->role);
        if (p_link->role == MASTER)
        {
            if (p_sco_link)
            {
                /*
                 * Not accepted for Master renegotiating a new packet type
                 */
                _Send_HC_Connection_Packet_Type_Changed_Event_SCO(p_sco_link, reason);
                LMscoctr_Free_Negotiation_SCO_Link();
            }
            else
            {
                /*
                 * Not accepted for Master new connection setup
                 * Find the transitioning sco link
                 */
                _Send_HC_Connection_Complete_Event_SCO(p_link, 0x0000, reason);
                LMscoctr_Free(LMscoctr_Get_SCO_Transitioning_Ref());
            }
        }
        else /* if (p_link->role == SLAVE) */
        {    
            /*
             * The slave is either in new connection setup or in packet change
             * renegotiation.  The LMP_not_accepted has inadequate context and
             * must depend on state parameters in LMP_sco_container module.
             */
            if (p_sco_link)
            {
                /*
                 * Not accepted for Slave change packet type
                 */
                _Send_HC_Connection_Packet_Type_Changed_Event_SCO(p_sco_link, reason);
            }
            else 
            { 
                /*
                 * Not accepted for Slave new connection setup
                 */
                _Send_HC_Connection_Complete_Event_SCO(p_link, 0x0000, reason);
            }
        }
        break;
    }
    case LMP_REMOVE_SCO_LINK_REQ :
        /*
         * Peer is not allowed to send LMP_not_accepted
         * Result is that SCO always goes down.
         * Detach reason was stored in LMconnection_LM_Disconnect_Req() 
         * as result of HCI_DISCONNECT invocation.
         */
        LMconnection_LM_Disconnect_SCO(
            LMscoctr_Get_SCO_Transitioning_Ref(), p_link->detach_reason);
        /*
         * Removing a SCO link may allow the IUT to adjust the ACL packet
         * types on all ACL links.
         */
        LMpol_Adjust_ACL_Packet_Types_On_All_Links(LMcontxt_LEAVING_SCO);
        break;   
#endif
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1)
    case LMP_ESCO_LINK_REQ:
        LMconnection_LMP_Not_Accepted_Ext_eSCO_Link_Req(p_link, (u_int8)reason);
    break;

    case LMP_REMOVE_ESCO_LINK_REQ:
        /*
         * Peer not allowed to send LMP_not_accepted to LMP_remove_esco_link_req.
         * Treat as if the peer had sent LMP_accepted to LMP_remove_esco_link_req
         */
         LMconnection_LMP_Accepted_Ext_Remove_eSCO_Link_Req(p_link);
    break;
#endif

#if (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1)
     case LMP_SWITCH_REQ :

        if( SWITCH_IDLE != p_link->switch_state)
		{
			LMtmr_Clear_Timer(p_link->gen_switch_timeout_index);
			p_link->gen_switch_timeout_index = 0;
			p_link->switch_state = SWITCH_IDLE;
			LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_MSS_ACTIVE);

#if (PRH_BS_CFG_SYS_LMP_PAUSE_ENCRYPTION_SUPPORTED==1)
            if(p_link->encrypt_paused)
            {
                p_link->event_status = reason; // role change event status
                p_link->encrypt_paused |= ENCRYPT_RESUME_PENDING_SWITCH_COMPLETE;

                LMsec_peer_Resume_Encryption(p_link);
            }
            else
#endif
			if((p_link->state == LMP_ACTIVE) /*|| (p_link->state = LMP_SNIFF_MODE) Errata */)
			{            
				 LMch_Enable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_MSS);
				_Send_HC_Role_Change_Event(p_link,reason);
            }  
			else /* Peer refuses a role switch during connection setup BF 519 */
			{

				p_link->current_proc_tid_role = MASTER;
				LM_Encode_LMP_Accepted_PDU(p_link,LMP_HOST_CONNECTION_REQ);
				if((ON == g_LM_config_info.authentication_enable) && 
				   (LMssp_SSP_Enabled_On_Link(p_link)==0))             
				{

					LMsec_core_Init_Authentication(p_link);
					
					p_link->state |= W4_AUTHENTICATION;
				}
				else
				{ 
					p_link->state |= W4_LMP_ACTIVE;
					LMconnection_Local_Setup_Complete(p_link);              
				}
			}
    	}
         break;
#endif

#if (PRH_BS_CFG_SYS_REMOTE_NAME_REQUEST_SUPPORTED==1)
     case LMP_NAME_REQ :
         _Send_HC_Remote_Name_Request_Complete_Event(p_link,reason);
         /*
          * Note: explicit detach is required in this case.
          */
          if(p_link->context == LMch_TEMPORARY_CONNECTION)
              LMconnection_Send_LMP_Detach(p_link, REMOTE_USER_TERMINATED_CONNECTION);
         break;
#endif

     case LMP_FEATURES_REQ :
         /* The occurs due to a message response timeout. If the features_req is 
          * sent during a connection send the HCI_Remote_Features_Complete_Event.
          * If no ACL active then send the HCI_Connection_Complete event
          */
         if(p_link->features_complete_bit_mask == LMP_Both_Features_Complete)
         {
             _Send_HC_Remote_Features_Complete_Event(p_link,0,reason);
         }
         else if(p_link->role == MASTER)
         {
             p_link->detach_reason = reason;
             LMconnection_Send_LMP_Detach(p_link, reason);
         }
         else
         {
            LMconnection_Finalise_Connection_Setup(p_link);
            p_link->features_complete_bit_mask = LMP_Both_Features_Complete;
         }
         break;

     case LMP_FEATURES_REQ_EXT :
		 if (p_link->context == LMch_TEMPORARY_CONNECTION)
		 {
			 _Send_LMP_Name_Req(p_link,0);
		 }
		 else
		 {
			 _Send_HC_Remote_Extended_Features_Complete_Event(p_link, 0, reason);
		 }
         break;
     case LMP_PACKET_TYPE_TABLE_REQ :
         /* 
          * Request Local device to restart L2CAP data transmission.
          * Also need to report to HCI that ptt change failed.. consequences on 
          * supported packet types on this ACL link?
          */
    	 p_link->ptt_req = PTT_REQ_IDLE;
         LMch_Enable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_PTT);
         break;
     }

   return NO_ERROR;
}
    
/**************************************************************************
 *  
 *  FUNCTION : LMconnection_Finalise_Connection_Setup
 *
 *  DESCRIPTION :
 *  Completes the setup complete by optionally requesting
 *      Auto rate      
 *      Activate AFH
 *      Timing Accuracy Request
 *      Changing packet types
 *
 ************************************************************************/  
static void LMconnection_Finalise_Connection_Setup(t_lmp_link* p_link)
{
    u_int8 *local_features = SYSconfig_Get_LMP_Features_Ref();
    
    /* 
     * Send Connection Complete to the upper layer 
     */
    _Send_HC_Connection_Complete_Event_ACL(p_link, NO_ERROR);

    if ((BTtst_Get_DUT_Mode() == DUT_DISABLED))
    {          
        /*
         * Request Auto Rate activation if supported on both sides
         */
        LMqos_LM_Auto_Rate(p_link);


#if (PRH_BS_DEV_EARLY_SET_AFH_ON_CONNECTION_ESTABLISHMENT==0)
        /*
         * Instigate AFH if MASTER and AFH is active on piconet
         */
        LMafh_Activate_AFH(p_link);
#endif

        /*
         * Request Timing Accuracy if supported.
         */
        LMlc_LM_Timing_Accuracy_Req(p_link);

        /*
         * Request broadcast encryption key length if supported
         */
        if(p_link->role == MASTER)
        {
            LMsec_peer_Send_LMP_Encryption_Key_Size_Mask_Req(p_link);
        }
    }
    
    /*
     * If local device does not support 3-slot, remove 3-slot packets
     * from the possible packet types.
     */
    if(!(mFeat_Check_3_Slot(local_features)))
    {
        p_link->packet_types &= ~HCI_3_SLOT;
    }

    /*
     * If local device does not support 5-slot, remove 5-slot packets
     * from the possible packet types.
     */
    if(!(mFeat_Check_5_Slot(local_features)))
    {
        p_link->packet_types &= ~HCI_5_SLOT;
    }

    /*
     * If local device does not support 2MBPS, remove 2MBPS packets
     * from the possible packet types.
     */
    if(!(mFeat_Check_EDR_ACL_2MBPS_Mode(local_features)))
    {
        p_link->packet_types &= ~HCI_ACL_2MBS_EDR;
    }

    /*
     * If local device does not support 3MBPS, remove 3MBPS packets
     * from the possible packet types.
     */
    if(!(mFeat_Check_EDR_ACL_3MBPS_Mode(local_features)))
    {
        p_link->packet_types &= ~HCI_ACL_3MBS_EDR;
    }

    /*
     * If local device does not support 3 slot EDR, remove 3 slot EDR
     * from the possible packet types.
     */
    if(!(mFeat_Check_3_Slot_EDR_ACL_Packets(local_features)))
    {
        p_link->packet_types &= ~HCI_3_SLOT_EDR;
    }

    /*
     * If local device does not support 5 slot EDR, remove 5 slot EDR
     * from the possible packet types.
     */
    if(!(mFeat_Check_5_Slot_EDR_ACL_Packets(local_features)))
    {
        p_link->packet_types &= ~HCI_5_SLOT_EDR;
    }

    /*
     * If peer device does not support 3-slot, remove 3-slot packets
     * from the possible packet types.
     */
    if(!(mFeat_Check_3_Slot(p_link->remote_features)))
    {
        p_link->packet_types &= ~HCI_3_SLOT;
    }

    /*
     * If peer device does not support 5-slot, remove 5-slot packets
     * from the possible packet types.
     */
    if(!(mFeat_Check_5_Slot(p_link->remote_features)))
    {
        p_link->packet_types &= ~HCI_5_SLOT;
    }

    /*
     * If peer device does not support 2MBPS, remove 2MBPS packets
     * from the possible packet types.
     */
    if(!(mFeat_Check_EDR_ACL_2MBPS_Mode(p_link->remote_features)))
    {
        p_link->packet_types &= ~HCI_ACL_2MBS_EDR;
    }

    /*
     * If peer device does not support 3MBPS, remove 3MBPS packets
     * from the possible packet types.
     */
    if(!(mFeat_Check_EDR_ACL_3MBPS_Mode(p_link->remote_features)))
    {
        p_link->packet_types &= ~HCI_ACL_3MBS_EDR;
    }

    /*
     * If peer device does not support 3 slot EDR, remove 3 slot EDR
     * from the possible packet types.
     */
    if(!(mFeat_Check_3_Slot_EDR_ACL_Packets(p_link->remote_features)))
    {
        p_link->packet_types &= ~HCI_3_SLOT_EDR;
    }

    /*
     * If peer device does not support 5 slot EDR, remove 5 slot EDR
     * from the possible packet types.
     */
    if(!(mFeat_Check_5_Slot_EDR_ACL_Packets(p_link->remote_features)))
    {
        p_link->packet_types &= ~HCI_5_SLOT_EDR;
    }

#if (PRH_BS_CFG_SYS_LM_PREFERRED_RATE_SUPPORTED==1)

    /*
     * Initial mapping of packet_types to allowed_pkt_types. This may be overriden
     * by preferred rate etc later on the connection.
     */
    p_link->allowed_pkt_types = LMqos_LM_Gen_Allowed_Packet_Types(p_link->packet_types);
#endif
    /*
     * If EDR packet types, then request EDR packet table. 
     * If multislot packet capable also send max slots request.
     * If host has only 5-slot packets enabled, ask for 5-slots from peer.
     * If host has only 3-slot packets enabled, ask for 3-slots from peer.
     * If host has neither 5-slot nor 3-slot packets enabled, do nothing.
     * If host has both 5-slot and 3-slot packets enabled, as for 5-slots from peer.
     */
    if (p_link->packet_types & HCI_ACL_EDR)
    {
        LMch_Send_LMP_Packet_Type_Table_Req(p_link, 1);

        if (p_link->packet_types & (HCI_5_SLOT_EDR|HCI_3_SLOT_EDR))
        {
            LMqos_LM_Change_Packet_Type(p_link->handle, p_link->packet_types);
        }
    }
    else if (p_link->packet_types & (HCI_5_SLOT_BR|HCI_3_SLOT_BR))
    {
     //   if((p_link->packet_types & HCI_5_SLOT_BR|HCI_3_SLOT_BR))
        {
            LMqos_LM_Change_Packet_Type(p_link->handle, p_link->packet_types);
        }
    }
#if (PRH_BS_CFG_SYS_LM_PREFERRED_RATE_SUPPORTED==1)
    else
    {
        p_link->allowed_pkt_types = LMqos_LM_Gen_Allowed_Packet_Types(p_link->packet_types);
    }
#endif  
    /*
     * Don't send the packet type changed event on connection setup.
     */
    p_link->packet_type_changed_event = 0;

}

/**************************************************************************
 *  
 *  FUNCTION : LMconnection_LMP_Setup_Complete
 *
 *  DESCRIPTION :
 *    Handles an Incoming LMP_Setup_Complete PDU, which indicates
 *    that the remote host has accepted the connection.
 *    Once the connection has gone active - it may be necessary to determine
 *    the features supported in the remote device. The request for remote 
 *    features also distribute 
 ************************************************************************/  
t_error LMconnection_LMP_Setup_Complete(t_lmp_link *p_link, t_p_pdu p_pdu)
{
    t_error status = NO_ERROR;

    pDebugAssert(PRTH_HW_ERROR_LMP_CH,(p_link == 0));

    /*	 * #2848:
	 * Need to handle the following error scenario:
	 * Rx LMP_Setup_Complete before rx
	 * LMP_Accepted(Host_Connection_Request)
	 * Detach the connection.
	 */
	if ( (p_link->host_connection_request_sent == TRUE) && 
		 (!p_link->host_connection_request_accepted))
	{

		/* Need to Disable L2CAP Traffic @ the LM level */
		LMch_Disable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_ALL);
			
		/*
		 * When disconnecting, stop local L2CAP data and peer L2CAP data.
         */
		LC_Stop_Local_Tx_L2CAP_Data(p_link->device_index);
		LC_Stop_Peer_Tx_L2CAP_Data(p_link->device_index);
		p_link->current_proc_tid_role = p_link->role; 
        LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_CREATE_CONN_PENDING);
		p_link->detach_reason = LMP_PDU_NOT_ALLOWED;
		LMconnection_Send_LMP_Detach(p_link,LMP_PDU_NOT_ALLOWED);

		return NO_ERROR;
	}
    /*
     * Bug 1503: Guard against errant Ericsson devices which
     * send two LMP_setup_complete messages when their
     * security block gets confused at link establishment.
     *
     * If have already received an LMP_setup_complete from the
     * peer device, take no further action on the second and
     * any subsequent connection setups.
     */
    if(p_link->setup_complete_bit_mask & LMP_Remote_Setup_Complete)
        return status;

    p_link->setup_complete_bit_mask |= LMP_Remote_Setup_Complete;

    if (p_link->setup_complete_bit_mask == LMP_Both_Setup_Completes)
    { 
        p_link->state = LMP_ACTIVE;
        _LMch_p_pending_link = p_link;

        if(p_link->features_complete_bit_mask == LMP_Both_Features_Complete)
        {
            LMconnection_Finalise_Connection_Setup(p_link);
        }
        else
        {
            p_link->current_proc_tid_role = p_link->role;
            _Send_LMP_Connection_PDU(LMP_FEATURES_REQ, p_link);
        }
    }
    return status;
}

/**************************************************************************
 *  
 *  FUNCTION : LMch_CB_Enable_L2CAP_Traffic
 *
 *  DESCRIPTION :
 *    Invoked on a HCI Connection Complete Event to allow ACL data
 *    
 *    _LMch_p_pending_link may be undefined due to generation of a
 *    HCI Connection Complete Event for Loopback Testing
 *
 *  Note the device_index presented in the queue callback is that 
 *  associated with the queue (HCI event queue is 0) and hence
 *  should not be used.
 *************************************************************************/
void LMch_CB_Enable_L2CAP_Traffic(t_deviceIndex device_index)
{
    if (_LMch_p_pending_link != 0)
    {
        LC_Start_Peer_Tx_L2CAP_Data(_LMch_p_pending_link->device_index);
        LC_Start_Local_Tx_L2CAP_Data(_LMch_p_pending_link->device_index);
        _LMch_p_pending_link->l2cap_tx_enable  = LM_CH_L2CAP_EN_ALL;
    }
}

/**************************************************************************
 *  
 *  FUNCTION : LMch_Enable_L2CAP_Traffic
 *
 *  DESCRIPTION :
 *    Ensure ACL data can be transmitted.
 *    
 *************************************************************************/
void LMch_Enable_L2CAP_Traffic(t_lmp_link *p_link, u_int8 l2cap_enable_type)
{
    if ((p_link != 0) && (p_link->state & (LMP_ACTIVE|LMP_SNIFF_MODE)))
    {
        p_link->l2cap_tx_enable  |= l2cap_enable_type;
    }
}

/**************************************************************************
 *  
 *  FUNCTION : LMch_Disable_L2CAP_Traffic
 *
 *  DESCRIPTION :
 *    Prevent ACL data being transmitted.
 *    
 *************************************************************************/
void LMch_Disable_L2CAP_Traffic(t_lmp_link *p_link, u_int8 l2cap_disable_type)
{
    p_link->l2cap_tx_enable &= ~l2cap_disable_type;
}

/*****************************************************
 *****************************************************
 ***                                              ****
 ***    DETACH / DISCONNECTION FUNCTIONALITY      **** 
 ***                                              ****
 *****************************************************
 *****************************************************/


/*************************************************************************
 *  
 *  FUNCTION : LMconnection_LMP_Detach
 *
 *  DESCRIPTION :
 *    Handles an Incoming LMP_Detach PDU, which indicates
 *    that the remote host is closing the connection.
 * 
 *    The Detach can be received in any state so there is no need to check 
 *    the state information for a detach.
 *
 *  POSITION : Master | Slave
 *
 *************************************************************************/

t_error LMconnection_LMP_Detach(t_lmp_link *p_link,t_p_pdu p_pdu)
{
    t_error detach_reason = (t_error)*(p_pdu);
    t_slots detach_ack_timeout;

    /* 
     * If LMP_Detach PDU Rx'd when operation pending is already LMP_DETACH
     * then ignore incoming LMP_DETACH PDU.
     */
    if (p_link->operation_pending == LMP_DETACH)
    {
        return NO_ERROR;
    }

    /* Need to Disable L2CAP Traffic @ the LM level */
    LMch_Disable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_ALL);

    /*
     * Stop all L2CAP data.
     */

    LC_Stop_Local_Tx_L2CAP_Data(p_link->device_index);
    LC_Stop_Peer_Tx_L2CAP_Data(p_link->device_index);

    p_link->detach_reason = detach_reason;

    /*
     * Detach Ack on Master/Slave is 6/3 times poll interval, respectively
     */
    if (p_link->role == MASTER)
    {
        detach_ack_timeout = (t_slots)p_link->poll_interval*6;
    }
    else
    {
        detach_ack_timeout = (t_slots)p_link->poll_interval*3;
        /*
         * Cancel Connection_Accept_TO timer if appropriate
         */
        if ((W4_HOST_ACL_ACCEPT & p_link->state) || 
            (W4_HOST_ESCO_ACCEPT & p_link->state) || 
            (W4_HOST_SCO_ACCEPT & p_link->state))
        {
            LMtmr_Clear_Timer(g_LM_config_info.accept_timeout_index);
            g_LM_config_info.accept_timeout_index = 0;
        }
    }

    if(p_link->gen_detach_timeout_index)
    {
        LMtmr_Clear_Timer(p_link->gen_detach_timeout_index);
    }

    p_link->gen_detach_timeout_index = LMtmr_Set_Timer(
        detach_ack_timeout, &LMconnection_Disconnect_Acl, p_link, 1); 

    p_link->state |= LMP_DETACH_ACK_PENDING; 

    p_link->operation_pending = LMP_DETACH;

#if 1 // Fix for Bug #1653 - if switch was pending
    if (p_link->switch_state != SWITCH_IDLE)
	{
		p_link->switch_state = SWITCH_IDLE;
        LMtmr_Clear_Timer(p_link->gen_switch_timeout_index);
		p_link->gen_switch_timeout_index = 0;
		LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_MSS_ACTIVE);
	}
#endif
    /*
     * If device is in test mode then it must leave test mode on receiving LMP_detach
     */
    LC_Testmode_Exit();

    return NO_ERROR;
}


/************************************************************************
 *  
 *  FUNCTION : LMconnection_LMP_Detach_Ack
 *
 *  DESCRIPTION :
 *  Handles an Detach_Ack from the lower layers, which indicates that Detach
 *  message has been successfully sent. A timer of 3 * Tpoll is then started 
 *  on expiration of the ( 3 * Tpoll) timer the link can be released immediately.
 *
 *************************************************************************/
void LMconnection_LMP_Detach_Ack(t_deviceIndex device_index)
{
    t_lmp_link *p_link;
   
    p_link = LMaclctr_Find_Device_Index(device_index);

    if (p_link != 0)
    {
        /* 
         * Clear the existing timer, and set a new timer 
         */
        LMtmr_Clear_Timer(p_link->gen_detach_timeout_index );
        p_link->gen_detach_timeout_index = LMtmr_Set_Timer(
            3*p_link->poll_interval, LMconnection_Disconnect_Acl, p_link, 1);
    }

    /*
     * If device is in test mode then it must leave test mode
     */
    LC_Testmode_Exit();
}

/**************************************************************************
 * Function :- LMconnection_Disconnect_Acl
 *
 * Description :-
 *
 *    1/ Upper Layer Link Established -- Send Disconnect Complete
 *    2/ Prior to completion of connection setup -- Send Connection Complete
 *    3/ Temporary Connection [ Remote Name ] -- Send Remote Name Complete
 *
 * The setting of the detach reason in the event is obtained from 
 * the "p_link->detach_reason" which contains the error
 * reason send in the last LMP_Detach PDU.
 *
 ****************************************************************************/
void LMconnection_Disconnect_Acl(t_lmp_link *p_link)
{
    if (p_link->state == LMP_IDLE)
    {
        return;
    }
#if (PRH_BS_CFG_SYS_REMOTE_NAME_REQUEST_SUPPORTED==1)
    if ((g_LM_config_info.operation_pending == LMP_NAME_REQ) && (p_link->msg_timer != 0))
    {
        LMtmr_Clear_Timer(p_link->msg_timer);
        p_link->msg_timer = 0;
        if (LMconfig_LM_Check_Device_State(PRH_DEV_STATE_RNR_CANCEL_PENDING))
        {
            const t_bd_addr *bd_addr = &p_link->bd_addr;
            _Send_HC_Read_Remote_Name_Cancel_Command_Complete_Event(bd_addr, 
                         NO_ERROR);
            _Send_HC_Remote_Name_Request_Complete_Event(p_link,NO_CONNECTION);
            LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_RNR_CONN_PENDING);
        }
        else
        {
            _Send_HC_Remote_Name_Request_Complete_Event(p_link,p_link->detach_reason);
        }

    }
#endif
    if(p_link->setup_complete_bit_mask == LMP_Both_Setup_Completes)
    {
        /***************************************************************
         * After the Connection Request & Setup Phases on the connection
         * setup with a higher layer connection existing.
         ***************************************************************/

#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
        LMpolicy_Clear_Link_If_Parked(p_link);
#endif

        _Send_HC_Disconnection_Complete(p_link,p_link->detach_reason); 
    } 
    else if (p_link->context != LMch_TEMPORARY_CONNECTION &&
              (BTtst_Get_DUT_Mode() == DUT_DISABLED) )
    {
        /***************************************************************
         * Detach prior to the completion of connection setup 
         * This event should only be sent when DUT_MODE == DUT_DISABLED 
         * (i.e. not in testmode since tester can use only a baseband 
         *  connection.)
         ***************************************************************/

        if (LMconfig_LM_Check_Device_State(PRH_DEV_STATE_CANCEL_CONN_PENDING))
        {
            _Send_HC_Create_Connection_Cancel_Command_Complete_Event(&p_link->bd_addr, 
                             NO_ERROR);            
            _Send_HC_Connection_Complete_Event_ACL(p_link, NO_CONNECTION);  
        }
        else
        {
			if  ((p_link->role == MASTER) || (p_link->connection_request_send_via_hci))
				_Send_HC_Connection_Complete_Event_ACL(p_link, p_link->detach_reason);  
        }    
    }
#if (PRH_BS_CFG_SYS_REMOTE_NAME_REQUEST_SUPPORTED==1)
    else if ((p_link->context == LMch_TEMPORARY_CONNECTION) && (p_link->role==MASTER))
    {
        if ((p_link->msg_timer == 0) && (g_LM_config_info.remote_name.length))
        {
            if (LMconfig_LM_Check_Device_State(PRH_DEV_STATE_RNR_CANCEL_PENDING))
            {
                const t_bd_addr *bd_addr = &p_link->bd_addr;
                _Send_HC_Read_Remote_Name_Cancel_Command_Complete_Event(bd_addr, 
                         NO_ERROR);
                _Send_HC_Remote_Name_Request_Complete_Event(p_link,NO_CONNECTION);
            }
            else
            {
                _Send_HC_Remote_Name_Request_Complete_Event(p_link,NO_ERROR);
            }
        }
        LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_RNR_CANCEL_PENDING);
        LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_RNR_CONN_PENDING);
    }
#endif


    /* 
     * Release any data packets, adjust flow counters and reset 
     * all queues associated with a device including the LMP
     */
    HCfc_Release_All_Data_Queue_Packets(p_link->device_index);   

    /*
     * If necessary, deactivate AFH and update AFH on piconet
     */
    LMafh_Deactivate_AFH(p_link);

    if (p_link->role == SLAVE)
    {
        LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_MAX_SLAVE_ACLS);
        /*
         * Stop the slave AFH classification reporting if appropriate
         */
        LMafh_Stop_Classification_Reporting(p_link);
    }

    LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_TEST_MODE);

    LMaclctr_Free_Link(p_link);     

    if(p_link->role == SLAVE)
        g_LM_config_info.num_slave_acl_links--;
    else
        g_LM_config_info.num_master_acl_links--;

    g_LM_config_info.num_acl_links--;
    if (!g_LM_config_info.num_acl_links)
    {
        LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_ACLS_ACTIVE);
        /*
         * #2875.
         * Ensure that the Master Broadcasts (via DevLink 0) is reset to
         * not use encryption 
         */
        LC_Write_Encryption_Mode(DL_Get_Local_Device_Index(), ENCRYPT_NONE);
    }
    else
    {  
#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
        /*
         * Clear the change piconet timer if no longer in scatternet.
         */
        if (!LMconfig_LM_Connected_As_Scatternet())
        {
            LMch_Clear_Change_Piconet_Timer();
        }
#else
        /*
         * A slave has disconnected but master acl links remain, so resume
         * master context. Required on detach of non-scatternet capable builds
         * when failed connect with role switch and after detach incoming
         * remote name request.
         */
        if ((p_link->role == SLAVE) && (g_LM_config_info.num_master_acl_links))
        {
            LC_Force_Context(MASTER);
            LC_Restore_Piconet();
        }
		else if((p_link->role == SLAVE) && (g_LM_config_info.num_slave_acl_links))
		{
			// 
			// Need to Force change over to the other piconet.
			LMch_Change_Piconet(0);
		}
#endif
    }

    /* If Guaranteed QoS was enabled on the link, then
     * I must clear the device State and re-set the device 
     * back to allowing other links
     */
    if (p_link->service_type == PRH_GUARANTEED_SERVICE)
    {
        g_LM_config_info.new_connections_allowed = 1;
        LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_GUARANTEED_QOS);
    }

    switch (p_link->operation_pending)
    {
    case LMP_SWITCH_REQ :
        LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_MSS_ACTIVE);
        break;

    case LMP_PARK_REQ :
        LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_PARK_PENDING);
        break;
    }
}

#if (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1)
/**************************************************************************
 * Function :- LMconnection_LMP_Switch_Timeout
 *
 *
 * Description :-
 *      Handle a LMP switch instant timeout by performing switch  
 * 
 **************************************************************************/
void LMconnection_LMP_Switch_Timeout(t_lmp_link *p_link)
{
    /*
     * Stop the slave AFH classification reporting if appropriate
     */
    LMafh_Stop_Classification_Reporting(p_link);

    LC_Role_Switch(p_link->device_index);
}
#endif

/*****************************************************
 *****************************************************
 ***    REMOTE NAME FUNCTIONALITY                 **** 
 *****************************************************
 *****************************************************/

/**************************************************************************
 * Function :- LMconnection_LMP_Name_Req     
 * 
 * Description :-
 * Handles an incoming LMP name request.
 * 
 *************************************************************************/
t_error LMconnection_LMP_Name_Req(t_lmp_link *p_link, t_p_pdu p_pdu)
{
    t_lmp_pdu_info  pdu_info;
    u_int8 offset;
    offset = (u_int8) *p_pdu;

    /******************************************************
     * LMP_Name_Req can be received in any state once the
     * baseband connection is active
     ******************************************************/  
    p_link->current_proc_tid_role = p_link->role ^ 0x01;
    if (p_link->context == LMch_NO_CONNECTION)
    {
        p_link->context = LMch_TEMPORARY_CONNECTION;
    }

    pdu_info.opcode = LMP_NAME_RES;
    if ((offset > 0) && ( offset >= g_LM_config_info.device_name.length))  /* Bug Fix 339 */
    {
        /* Error Case looking for offset greater than the name length */
        /* return UNSPECIFIED_ERROR; */
        pdu_info.length = 0;
    }
    else if ((g_LM_config_info.device_name.length - offset) > 14 )
    {
        /**********************************************************
         * Then this is Not the last fragment => Payload length = 14 
         **********************************************************/
        pdu_info.length = MAX_NAME_FRAGMENT_SIZE;
    } 
    else
    { 
        pdu_info.length = (g_LM_config_info.device_name.length - offset);
    }
  
    pdu_info.tid_role = p_link->current_proc_tid_role;
    pdu_info.name_length = g_LM_config_info.device_name.length;
    pdu_info.name_offset = offset;
    pdu_info.ptr.p_uint8 = g_LM_config_info.device_name.p_utf_char+offset;
    LM_Encode_LMP_PDU(p_link->device_index,&pdu_info);

    return NO_ERROR;
}

/**************************************************************************
 *  
 *  FUNCTION : LMconnection_LMP_Switch_Req
 *
 *  DESCRIPTION :
 *    Handles a role switch request PDU from the peer
 *    The steps to be performed are dependent on the role of the
 *    current device and the supported features
 *   
 *    If MASTER 
 *           if ( Switch Supported )
 *                Send LMP_ACCEPTED pdu
 *           else
 *                Send LMP_NOT_ACCEPTED pdu
 *    Else (if SLAVE)
 *          if ( Switch Supported )
 *               Send LMP_SLOT_OFFSET pdu
 *               Send LMP_ACCEPTED pdu
 *          else
 *               Send LMP_NOT_ACCEPTED pdu
 *
 *************************************************************************/

t_error LMconnection_LMP_Switch_Req(t_lmp_link *p_link, t_p_pdu p_pdu)
{
#if (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1)
    t_error status = NO_ERROR;
    u_int32 switch_instant = LMutils_Get_Uint32(p_pdu);
    
    t_clock clock;
    if (p_link->role == MASTER)
        clock = LC_Get_Native_Clock();
    else
        clock = LC_Get_Piconet_Clock(p_link->device_index);

    p_link->current_proc_tid_role = p_link->tid_role_last_rcvd_pkt;

   /* Reduce the number of clock loads */


    /*
     * Error Checking 
     */
    if (((p_link->state == LMP_ACTIVE) && 
         (!mPol_Check_Master_Slave_Switch(p_link->link_policy_mode))) ||
        ((p_link->state != LMP_ACTIVE) && (!p_link->allow_switch)))
    {
        status = ROLE_CHANGE_NOT_ALLOWED;
    }
    else if(p_link->switch_state != SWITCH_IDLE)
    {
        status = COMMAND_DISALLOWED;
    }
    else if(!SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_LMP_MSS_FEATURE))
    {
        status = UNSUPPORTED_REMOTE_FEATURE;
    }
#if 0  // GF 12 Nov 2013 - Bypass for test ENC/BV-24c - Bite Tester does not report support for Slot offset
       // 
    else if ((!mFeat_Check_Slot_Offset(p_link->remote_features))
      || (!mFeat_Check_Role_Switch(p_link->remote_features)))
    {
        status = ROLE_CHANGE_NOT_ALLOWED;
    }
#endif
    else if((switch_instant <= (clock>>1) )) 
    {
        status = INSTANT_PASSED;
    }
    /*
     * See #bugzilla issue 1700 and 943 regarding setting the maximum limit
     * for the switch instant. This is an arbitary value long enough
     * for the R&S test equipment but under 10 seconds to prevent
     * known problems with buggy implementations requesting switches hours
     * in the future.
     */
    else if ((switch_instant - (clock>>1)) > 10000)
    {
        status = UNSUPPORTED_PARAMETER_VALUE;
    }
    else if (LMscoctr_Get_Number_SYN_Connections() > 0)  /* Check if there are any SCO links up. */
    {
        status = ROLE_CHANGE_NOT_ALLOWED;
    }
    if (TEMPORARY == LMacl_Get_Current_Key_Persistance(p_link))
    {
        status = ROLE_CHANGE_NOT_ALLOWED;
    }
    else if (ON == p_link->encrypt_enable)
    {
        status = ROLE_CHANGE_NOT_ALLOWED;
    }
    // Role switching a master with more than one link should be allowed
	// if scatternet is supported.
#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED !=1)
	else if (g_LM_config_info.num_master_acl_links > 1)
    {
        status = ROLE_CHANGE_NOT_ALLOWED;
    }
#endif
    if (status == NO_ERROR)
    {
        p_link->switch_state = SWITCH_PEER_ACTIVATION_PENDING;
        LMconfig_LM_Set_Device_State(PRH_DEV_STATE_MSS_ACTIVE);
        if (p_link->role == SLAVE)
        {
            _Send_LMP_Connection_PDU(LMP_SLOT_OFFSET,p_link);
        }

        LMch_Disable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_MSS);
        LM_Encode_LMP_Accepted_PDU(p_link,LMP_SWITCH_REQ);
        p_link->gen_switch_timeout_index = LMtmr_Set_Timer(switch_instant - 
            (clock>>1)-MSS_EARLY_START_SLOT_OFFSET_ADJUST,
            LMconnection_LMP_Switch_Timeout, p_link, 1);

#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
        if (LMconfig_LM_Connected_As_Scatternet())
        {
            /*
             * Set timer to switch back to this piconet before switch instant.
             * Note: gaurd time of 0x20 slots.
             */

			LMch_Set_Next_Change_Piconet_Time((switch_instant - (clock>>1) - 
                     MSS_EARLY_START_SLOT_OFFSET_ADJUST) - 0x20);

        }
#endif

    } 
    else
    {
        return status;
    }       
#endif /* (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1) */

    return NO_ERROR;
}

/*****************************************************
 *****************************************************
 ***    REMOTE / LOCAL FEATURES                   **** 
 *****************************************************
 *****************************************************/

/**************************************************************************
 *
 *  FUNCTION : LMconnection_LMP_Features_Req
 *
 *  INPUT :  p_link, p_pdu
 *
 *  DESCRIPTION :
 *  Handler for LMP Read_Remote_Features_Req PDU. The remote features are 
 *  obtained from the PDU and stored for this particular device.
 *  A Remote_Features_Rsp PDU is then sent to the initiating device.
 *
 *************************************************************************/
t_error LMconnection_LMP_Features_Req(t_lmp_link *p_link, t_p_pdu p_pdu)
{
    t_error status = NO_ERROR;

    /******************************************
     * Store the Features for the remote device 
     * and send a Features Resp 
     ******************************************/  
    p_link->current_proc_tid_role = p_link->role ^ 0x01;

    LMutils_Array_Copy(FEATURES_SIZE, p_link->remote_features, (u_int8*)p_pdu);

    _Send_LMP_Connection_PDU(LMP_FEATURES_RES, p_link);

    /*
     * Should only be able to get into this condition at startup
     * if the master has not initiated features exchange before
     * LMP_setup_complete exchange AND the master has initiated
     * LMP features exchange just after the slave has initiated
     * LMP features exchange.
     *
     * In this case, the features_req is a trigger to the slave
     * to complete its normal connection setup.
     */
    if((p_link->state == LMP_ACTIVE) && (p_link->role == SLAVE) && 
        (p_link->features_complete_bit_mask == LMP_Local_Features_Complete))
    {
        LMconnection_Finalise_Connection_Setup(p_link);
    }

    p_link->features_complete_bit_mask |= LMP_Remote_Features_Complete;

    return status;
}


/**********************************************************************
 *  FUNCTION : LMconnection_LMP_Features_Res
 *
 *  INPUT :  p_link, p_pdu
 *
 *  DESCRIPTION :
 *  Handler for LMP Read_Remote_Features_Res PDU. The state of the local
 *  device is initially checked. The remote features are obtained from the 
 *  PDU and stored for this particular device and the state changed to idle.
 *
 *************************************************************************/
t_error LMconnection_LMP_Features_Res(t_lmp_link *p_link, t_p_pdu p_pdu)
{
	t_error status = NO_ERROR;

	LMutils_Array_Copy(FEATURES_SIZE, p_link->remote_features, p_pdu);

	if (p_link->context != LMch_TEMPORARY_CONNECTION)
	{
		if(((p_link->state == LMP_ACTIVE ) || (p_link->state & LMP_SNIFF_MODE) || 
			(p_link->state == LMP_PARK_MODE)) && (p_link->features_complete_bit_mask == LMP_Both_Features_Complete))
		{
			/*********************************************************************
			* NOTE :- It is assumed that a resp to a features req
			* once a connection is active will result in an event being generated
			********************************************************************/
			_Send_HC_Remote_Features_Complete_Event(p_link,p_pdu,NO_ERROR);
		}
		else if((p_link->role == MASTER) && (p_link->state != LMP_ACTIVE))
		{
			if (mFeat_Check_Extended_Features(p_link->remote_features))
			{
				LMconnection_LM_Read_Remote_Extended_Features(p_link,1);

			}
			else
			{
				_LMch_p_pending_link->current_proc_tid_role = MASTER;
				_Send_LMP_Connection_PDU(LMP_HOST_CONNECTION_REQ, _LMch_p_pending_link);
			}
		}
		/*
		* Slave only sends LMP_features_req in two places:
		* 1. Master has not sent LMP_features_req during connection establishment.
		*    Slave then sends LMP_features_req after sending LMP_setup_complete.
		*    In this case, we want the slave to continue with its normal setup.
		* 2. Host initiates Remote Features Request. In this case, the features
		*    will always have been exchanged at connection setup. In this
		*    case we do not want the host controller to do anything else.
		*/
		else if(p_link->features_complete_bit_mask != LMP_Both_Features_Complete)
		{

			LMconnection_Finalise_Connection_Setup(p_link);
		}

		p_link->features_complete_bit_mask |= LMP_Remote_Features_Complete;
#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
		/* 
		* The following change piconet ensures that we return to (or stay on) the piconet
		* corresponding to this link for an extra period of time [PICONET_SERVICE_TIME/2]
		*
		* This is done to support the scenario where a Master Slave Switch imediately follows
		* connection establishment.
		*
		* In the future this code should be replaced by a more advanced scheduler supporting
		* both piconet and scattermode scheduling.
		*/
		if (p_link->role == SLAVE)
			LMpol_Change_Piconet(p_link->device_index, (PRH_BS_PICONET_SERVICE_TIME/LMconfig_LM_Num_Piconets()));
#endif
	}
	else
	{
		// Im in a Temporary Connection - So I now need to get the extended features ( Page 0 ).
		if (mFeat_Check_Extended_Features(p_link->remote_features))
			LMconnection_LM_Read_Remote_Extended_Features(p_link,1);
		else // Start looking for the name
			_Send_LMP_Name_Req(p_link,0);

	}

#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==1)
    if (mFeat_Check_Enhanced_Power_Control(p_link->remote_features))
		DL_Enable_EPC(DL_Get_Device_Ref(p_link->device_index));
#endif

		return status;

}


/**************************************************************************
 *
 *  FUNCTION : LMconnection_LMP_Features_Req_Ext
 *
 *  INPUT :  p_link, p_pdu
 *
 *  DESCRIPTION :
 *  Handler for LMP Read_Remote_Features_Req_Ext PDU. The remote features 
 *  for a particular page are obtained from the PDU.
 *  A Remote_Features_Res_Ext PDU is then sent to the initiating device.
 *
 *************************************************************************/
t_error LMconnection_LMP_Features_Req_Ext(t_lmp_link *p_link, t_p_pdu p_pdu)
{
    t_error status = NO_ERROR;

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_FEATURES_SUPPORTED == 0)
	status = UNSUPPORTED_REMOTE_FEATURE;
	return status;
#endif 

    /******************************************
     * Store the Features for the remote device 
     * and send a Features Resp 
     ******************************************/  
    p_link->current_proc_tid_role = p_link->role ^ 0x01;

#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
	if ((p_pdu[0] == 0x01) && (p_pdu[2] & 0x01))
			p_link->ssp_host_support = 0x01;
#endif
    _Send_LMP_Features_Res_Ext(p_link, p_pdu[0]);

    return status;
}

/**********************************************************************
 *
 *  FUNCTION : LMconnection_LMP_Features_Res_Ext
 *
 *  INPUT :  p_link, p_pdu
 *
 *  DESCRIPTION :
 *  Handler for LMP Read_Remote_Features_Res_Ext PDU. The state of the local
 *  device is initially checked. The remote features are obtained from the 
 *  PDU and the host controller sends an 
 *  HCI_Read_Remote_Extended_Features_Complete event to the host.
 *
 *************************************************************************/
t_error LMconnection_LMP_Features_Res_Ext(t_lmp_link *p_link, t_p_pdu p_pdu)
{
    t_error status = NO_ERROR;


#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
	if ((p_pdu[0] == 0x1) && (p_pdu[2] & 0x01))
		p_link->ssp_host_support = 0x01;
#endif

	if (p_link->context == LMch_TEMPORARY_CONNECTION)
	{
		// Send Host_Supported Features Notification event.
		// And then Send LMP_Name_Req
        _Send_HC_Remote_Host_Supported_Features_Notification_Event(p_link,p_pdu,NO_ERROR);
#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
		if (p_pdu[2] & 0x01)
			p_link->ssp_host_support = 0x01;
#endif
        _Send_LMP_Name_Req(p_link,0);

	}
	else
	{
	    if((p_link->role == MASTER) && (!(p_link->state & (LMP_ACTIVE|LMP_SNIFF_MODE))))
		{
			_LMch_p_pending_link->current_proc_tid_role = MASTER;
			_Send_LMP_Connection_PDU(LMP_HOST_CONNECTION_REQ, _LMch_p_pending_link);
		}
		else
		{

			_Send_HC_Remote_Extended_Features_Complete_Event(p_link,p_pdu,NO_ERROR);
		}
	}

    return status;
}


#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
/**************************************************************************
 *
 *  FUNCTION : LMconnection_LMP_Packet_Type_Table_Req
 *
 *  INPUT :  p_link, p_pdu
 *
 *  DESCRIPTION :
 *  Handler for LMP Packet_Type_Table_Req PDU. A not/accepted_ext is sent
 *  in response, depending on whether achievable.
 *
 *************************************************************************/
t_error LMconnection_LMP_Packet_Type_Table_Req(t_lmp_link *p_link, t_p_pdu p_pdu)
{
    t_error status = NO_ERROR;

    u_int8 ptt = p_pdu[0];

#if (PRH_BS_CFG_SYS_LMP_EDR_ACL_SUPPORTED==0)
    if (1 == ptt)
    {
        /*
         * Reject if EDR mode is requested and EDR packet types not supported.
         */
        LM_Encode_LMP_Not_Accepted_Ext_PDU(p_link, LMP_PACKET_TYPE_TABLE_REQ, EC_QOS_REJECTED);
    }
    else
#endif
    if ((p_link->ptt_req == PTT_REQ_IDLE) || (p_link->ptt_req == ptt))
    {
        /*
         * Adopt and accept the new packet type table.
         */
    	p_link->ptt_req = ptt = p_pdu[0];
        LMch_Disable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_PTT);

        LM_Encode_LMP_Accepted_Ext_PDU(p_link, LMP_PACKET_TYPE_TABLE_REQ);
    }
    else
    {
    	LM_Encode_LMP_Not_Accepted_Ext_PDU(p_link, LMP_PACKET_TYPE_TABLE_REQ, LMP_ERROR_TRANSACTION_COLLISION);
    }

    return status;
}

/*****************************************************************************
 *
 * FUNCTION :- LMch_LMP_Packet_Type_Table_Req_Accepted_Ack
 *
 ****************************************************************************/
void LMconnection_LMP_Packet_Type_Table_Req_Accepted_Ack(t_deviceIndex device_index)
{
    t_lmp_link *p_link = LMaclctr_Find_Device_Index(device_index);

	if(p_link->ptt_req != PTT_REQ_IDLE)
	{
		LC_Set_PTT_ACL(p_link->device_index, p_link->ptt_req);
		p_link->ptt_req = PTT_REQ_IDLE;
		LMqos_Update_Allowed_Packets(p_link,p_link->max_slots_out, p_link->rate);
		LMch_Enable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_PTT);
	}

	/*
	* If peer has autorate enabled, send an updated preferred rate
	* based on available packet types and existing quality of service.
	*/
#if (PRH_BS_CFG_SYS_LMP_CHANNEL_DRIVEN_QUALITY_SUPPORTED==1)
	if(p_link->peer_rate == AUTO)
	{
		u_int8 ptt = LC_Get_PTT_ACL(p_link->device_index);

	    if (((ptt == 1) && !(p_link->packet_types & HCI_ACL_3MBS_EDR))
		 || ((ptt == 0) && !(p_link->packet_types & HCI_HIGH_RATE)))
		{
			LMqos_Send_LMP_Preferred_Rate(p_link, MEDIUM);
		}
		else if (((ptt == 1) && !(p_link->packet_types & HCI_ACL_2MBS_EDR))
		|| ((ptt == 0) && !(p_link->packet_types & HCI_MEDIUM_RATE)))
		{
			LMqos_Send_LMP_Preferred_Rate(p_link, HIGH);
		}
		else
		{
			LMqos_Send_LMP_Preferred_Rate(p_link, p_link->rate);
		}
	}
#endif
}
#endif
/*****************************************************
 *****************************************************
 ***    SCO FUNCTIONALITY                         **** 
 *****************************************************
 *****************************************************/
#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
/*****************************************************************************
 * FUNCTION :- LMconnection_LMP_Remove_SCO_Link_Req
 *
 * DESCRIPTION :- 
 * This function is responsible for handling an incoming LMP_Remove_SCO_Link_Req
 * PDU.
 *****************************************************************************/
t_error LMconnection_LMP_Remove_SCO_Link_Req(t_lmp_link *p_link, t_p_pdu p_pdu)
{
    u_int8 lm_sco_handle;
    t_error reason;
    t_sco_info  *p_sco_link;

    /* Retrieve the SCO handle and detach reason from the incoming PDU */
    lm_sco_handle = *p_pdu;
    p_pdu++;
    reason = (t_error)*p_pdu;

    /* Find the SCO link */
    p_sco_link = LMscoctr_Find_SCO_By_SCO_Handle(lm_sco_handle);
    p_link->current_proc_tid_role = p_link->role ^ 0x01;

    if(p_sco_link)
    {
        /*
         * Master reports collision, slave accepts master's LMP_remove_sco_link_req,
         * else if both sides report collision, no-one actually removes the link.
         */
        if ( ((p_sco_link->state == SCO_IDLE) || (p_sco_link->state == SCO_MASTER_DEACTIVATION_PENDING))
              && (p_link->role == MASTER))
        { 
            return LMP_ERROR_TRANSACTION_COLLISION;
        }
        else
        {
            p_link->detach_reason = reason;
        }
#if 0 // GF 27 Jan 2010 Conf_2 - Still required at BQB 30 March
        /* 
         * Send an Accepted to the peer 
         */
        LM_Encode_LMP_Accepted_PDU(p_link,LMP_REMOVE_SCO_LINK_REQ);

        /* 
         * Send a Disconnection Complete to the upper layer 
         * Detach reason is that obtained from LMP_Remove_SCO_Link_Req
         */
        LMconnection_LM_Disconnect_SCO(p_sco_link, reason);
#else // Change the order of Sending Disconnect and LMP_Accepted.
      // If this does not work try sending the LM_Disconnect on the ACK to the
	  // LMP_Accepted. But first turn off DV..
		/* 
         * Send a Disconnection Complete to the upper layer 
         * Detach reason is that obtained from LMP_Remove_SCO_Link_Req
         */

        // GF 27 Jan - Change SCO link State to Idle 
		p_sco_link->state = (MASTER==p_link->role) ? 
			((SCO_LINK==p_sco_link->link_type) ? SCO_MASTER_DEACTIVATION_PENDING : SCO_IDLE) : SCO_IDLE;

        // GF 27 Jan now disconnect SCO event on Ack to lmp_accepted. 
		//  but need to store "reason" somewhere.

        // LMconnection_LM_Disconnect_SCO(p_sco_link, reason);

		p_pending_sco_disconnect_link = p_sco_link;
		pending_sco_disconnect_reason = reason; 

        /* 
         * Send an Accepted to the peer 
         */
        LM_Encode_LMP_Accepted_PDU(p_link,LMP_REMOVE_SCO_LINK_REQ);

#endif
        /*
         * The removal of a SCO may allow the IUT to adjust
         * the ACL packet types on all links.
         */
#if 0 // The adjusting of the ACL packet types 
	  // should be performed on Receipt of the ACK to the LMP_Accepted.
	  // This is required as we changed the mechanism of SCO release.
        LMpol_Adjust_ACL_Packet_Types_On_All_Links(LMcontxt_LEAVING_SCO);
#endif
    }
    else
    {
        /* 
         * Send a Not Accepted to the peer indicating the SCO handle is invalid 
         */
        return INVALID_LMP_PARAMETERS;
    }

    return NO_ERROR;
}



void LMconnection_LMP_Remove_Sco_Link_Accepted_Ack_Handler(t_lmp_link* p_link)
{
	// This is probably best deffered !!
	if (p_pending_sco_disconnect_link)
		LMconnection_LM_Disconnect_SCO(p_pending_sco_disconnect_link, pending_sco_disconnect_reason);

	p_pending_sco_disconnect_link = 0;

    LMpol_Adjust_ACL_Packet_Types_On_All_Links(LMcontxt_LEAVING_SCO);
}

void LMconnection_LMP_Remove_Sco_Link_Accepted_Ack(t_deviceIndex device_index)
{
    t_lmp_link* p_link = LMaclctr_Find_Device_Index(device_index);

    LM_DEFER_FROM_INTERRUPT_CONTEXT(LMconnection_LMP_Remove_Sco_Link_Accepted_Ack_Handler,p_link);
}

/************************************************************************
 *  FUNCTION : LMconnection_LMP_SCO_Link_Req
 *
 *  DESCRIPTION :
 *  Handles an incoming LMP_SCO_Link_Req from peer (master or slave)
 *
 * The LMP_SCO_link_req parameters in p_pdu are:
 *  SCO handle 
 *  timing control flags 
 *  D sco
 *  T sco
 *  SCO packet
 *  air mode
 *************************************************************************/
t_error LMconnection_LMP_SCO_Link_Req(t_lmp_link *p_link, t_p_pdu p_pdu)
{
    t_error reject_reason = NO_ERROR;
    t_sco_info *p_sco_link, *p_sco_renegotiate_link;
#if (PRH_BS_CFG_SYS_FILTERS_SUPPORTED == 1)
    u_int8 accept_status;
#endif
    u_int8 lm_sco_handle = p_pdu[0];
    u_int8 lm_sco_packet = p_pdu[4];
    u_int8 lm_air_mode   = p_pdu[5];
	t_error status = NO_ERROR;
    u_int16 voice_setting = 0;
    
    /*
     * For a slave received LMP_SCO_link_req 
     * 1. Handle must never be 0
     *    UPF-8 interop issue (where Master didn't assign sco_handle)
     * 2. Packet type must be valid  0 HV1, 1 HV2, 2 HV3
     */
    if ((p_link->role == SLAVE && lm_sco_handle == 0) || lm_sco_packet > 2)
    {
        return INVALID_LMP_PARAMETERS;
    }

  
     status = _Cancel_Pending_Page_Inquiry();

	/* 
     * If the SCO handle is not already in use Then 
     *   Its a new connection
     * Else
     *   The packet type is being negotiated 
     * Endif
     */
    p_sco_link = LMscoctr_Find_SCO_By_SCO_Handle(lm_sco_handle);
    if (p_sco_link == 0)
    {
        /*
         * Override HV1 to negotiate bandwidth for p2mp scenarios.
         */
        if ((g_LM_config_info.num_acl_links > 1) && (lm_sco_packet == 0))
        {
            if (mFeat_Check_HV3_Packets(p_link->remote_features))
                lm_sco_packet = p_pdu[4] = 2;
            else if (mFeat_Check_HV2_Packets(p_link->remote_features))
                lm_sco_packet = p_pdu[4] = 1;
        }

        /* 
         * Validate addition of this new SCO for given LM packet type
         */
        reject_reason = LMconnection_Validate_SCO_Admission(
            p_link, (t_packetTypes) (HV1_BIT_MASK << lm_sco_packet), lm_air_mode);
        if (reject_reason != NO_ERROR)
        {
            return reject_reason;
        }

        /*********************************************************
        * Check the connection event filters to determine if the
        * connection should be auto-accepted or not
        *********************************************************/            
        p_link->current_proc_tid_role = p_link->role ^ 0x01;
#if (PRH_BS_CFG_SYS_FILTERS_SUPPORTED == 1)  
        accept_status = 
            LMfltr_Connection_Filter_Check(&p_link->bd_addr,p_link->device_class,SCO_LINK);
        if ( ( AUTO_ACCEPT == accept_status) || 
             ( AUTO_ACCEPT_WITH_MSS == accept_status) ||
             ( DONT_AUTO_ACCEPT == accept_status))
#endif
        {
            /* If the SCO being allocated is associated with
             * a transaction initiated by the IUT as slave,
             * then use the voice settings proposed by the
             * host as the basis for allocation else
             * use the default voice settings for all incoming
             * connections.
             */
            if((p_link->role == SLAVE) && 
                (p_link->tid_role_last_rcvd_pkt == SLAVE))
            {
                voice_setting = LMscoctr_Restore_Topical_Voice_Setting();
            }
            else
            {
                voice_setting = g_LM_config_info.default_voice_setting;
#if (BUILD_TYPE!=UNIT_TEST_BUILD)
    			if (g_LM_config_info.default_sco_route_via_hci)
					voice_setting  |= PRH_BS_HCI_VOICE_SETTING_FOR_SCO_VIA_HCI;
#endif
            }

            /*
             * In either case, use the air mode proposed by the peer.
             */
            voice_setting &= ~3;
            voice_setting |= LMscoctr_Map_AirMode_LMP2HCI(lm_air_mode);

            /*
             * Allocate a SCO link structure. 
             * Associate it with an ACL link via device_index and 
             * select configuration. The SCO link is not activated yet.
             * The parameters of the incoming PDU are extracted and
             * checked against the features supported on the link. 
             * If the role of the local device is MASTER then
             * the SCO parameters must be derived.
             *
             * All SCOs are allocated with the default SCO CFG
             * value. Later this may be changed (if the
             * peer requests a different air mode).
             * 
             * If a SCO Link cannot be allocated i.e. then the max number of
             * SCO links has been allocated and an LMP_Not_Accepted must be
             * sent to the peer.  The allocation is requested for a slave
             * to ensure that a container is available.
             * A connection handle/lm_sco_handle are also assigned.
             */

            p_sco_link = LMscoctr_Alloc(p_link->device_index,
                voice_setting,
                SCO_LINK);

            if(!p_sco_link)
            {
                return MAX_NUM_SCO;
            }

            if (p_link->role == SLAVE)
            {
                reject_reason = LMscoctr_Extract_SCO_Parameters_From_PDU(
                    p_sco_link, p_pdu, SLAVE);
                if (reject_reason == NO_ERROR)
                {
#if (PRH_BS_CFG_SYS_FILTERS_SUPPORTED == 1)		
                    /* 
                     * Check if this is a Master response to a slave initiated 
                     * SCO_Link_Req using the Transaction Id. 
                     */   
                    u_int8 transaction_id = p_link->tid_role_last_rcvd_pkt;
                    u_int8 master_resp_2_slave_init = 
                        ((p_link->role == SLAVE) && (transaction_id == SLAVE));

                    if ( (AUTO_ACCEPT == accept_status) || 
                         (AUTO_ACCEPT_WITH_MSS == accept_status) || 
                         master_resp_2_slave_init)
                    {
                        _Accept_SCO_Link(p_link, transaction_id);
                        /*
                         * p_sco_link->state remains SCO_IDLE only changing to
                         * SCO_ACTIVATION_PENDING on receipt of ACK to Accepted Message
                         */
                        /*
                         * Adding a SCO connection may require the adjusting
                         * of ACL packet types on all ACL links.
                         */
                        LMpol_Adjust_ACL_Packet_Types_On_All_Links(LMcontxt_ENTERING_SCO);
                    }
                    else /*(DONT_AUTO_ACCEPT == accept_status)*/
#endif
                    {
                        p_link->state |= W4_HOST_SCO_ACCEPT;
                        _Send_HC_Connection_Request(p_link, SCO_LINK);
                    }
#if (PRH_BS_CFG_SYS_PERSISTENT_SCO_IN_SCANS_SUPPORTED==1)
                    /* 
                     * Disallow scans if SCO connection as slave.
                     */
                    LMconfig_LM_Set_Device_State(PRH_DEV_STATE_SCO_PREVENTING_SCANS);    
#endif
                }
                else /* if ( !(reject_reason == NO_ERROR) ) */ 
                {
                    /*
                     * Error has occurred, free SCO container
                     */
                    LMscoctr_Free(p_sco_link);
                }         
            }  
            else if (p_link->role == MASTER)
            {
                /*************************************************************
                 * Note the Timing_Ctrl & Dsco parameters on a slave  
                 * initiated SCO request PDU are to be ignored 
                 *************************************************************/
                
                /* 
                 * Retrieve the relevant SCO parameters from the PDU 
                 * Ignore any invalid parameters to allow negotiation.
                 * Derive additional SCO parameters to support new SCO
                 */
                LMscoctr_Extract_SCO_Parameters_From_PDU(p_sco_link, p_pdu, MASTER);
                LMscoctr_Derive_SCO_Parameters(p_sco_link, p_sco_link->packet_types, MASTER, 1 /* use peer air mode flags */);
               
                /* 
                 * Initialise the Device link with the HW sco configuration.
                 */
                LMpol_Init_SCO(p_link, p_sco_link->sco_index);

#if (PRH_BS_CFG_SYS_FILTERS_SUPPORTED == 1)               
                if (accept_status == AUTO_ACCEPT || accept_status == AUTO_ACCEPT_WITH_MSS)
                {
                    _Send_LMP_SCO_Link_Req(p_link, p_sco_link);
                }
                else
#endif
                {
                    _Send_HC_Connection_Request(p_link, SCO_LINK);
                }
            }
            
        }  /* End if (( AUTO_ACCEPT == accept_status)....  */
#if (PRH_BS_CFG_SYS_FILTERS_SUPPORTED == 1)  
        else
        {
            reject_reason = REMOTE_USER_TERMINATED_CONNECTION; /* Host has no filter for this device */
        }
#endif
    } /* End if (p_sco_link == 0) */


    else /* if (p_sco_link != 0) */
    {
        /* 
         * Re-negotiation of already active SCO (only allowed in single SCO)
         */
        if (LMscoctr_Get_Number_SCO_Connections() != 1)
        {
            reject_reason = LMP_PDU_NOT_ALLOWED;
        }
        else if (p_link->role == MASTER)
        { 
            /*
             * Slave initiated SCO modification (must be only one)
             */
            if (LMscoctr_Get_SCO_Transitioning_Ref())
            {
                /*
                * Already renegotiating a link => report collision
                */
                reject_reason = LMP_ERROR_TRANSACTION_COLLISION;
            }
            else
            {
                /*
                 * Fix for Bug 290
                 */
                p_link->current_proc_tid_role = p_link->role ^ 0x01;
            
                /* 
                 * The parameters lm_sco_handle, timing control flags and D_sco 
                 * are not valid in this message. 
                 * Thus existing p_sco_link values are used for these.
                 */
                lm_air_mode = p_pdu[5];
                p_sco_renegotiate_link = LMscoctr_Allocate_Negotation_SCO_Link(
                    p_sco_link, p_link->role, 
                    (t_packetTypes)(HV1_BIT_MASK << lm_sco_packet), lm_air_mode);

                /* 
                 * If the SCO air mode is supported then return LMP_SCO_Link_Req else reject
                 */
                if (p_sco_renegotiate_link->lm_air_mode == lm_air_mode)
                {              
                    _Send_LMP_SCO_Link_Req(p_link, p_sco_renegotiate_link);
                }
                else
                {
                    reject_reason = SCO_AIR_MODE_REJECTED;
                }
            }
        }
        else /* role == SLAVE */
        {   
            t_sco_info temp_sco_link;

            p_link->current_proc_tid_role = p_link->tid_role_last_rcvd_pkt; /* Fix for Bug 290 */
            reject_reason = LMscoctr_Extract_SCO_Parameters_From_PDU(
                &temp_sco_link, p_pdu, SLAVE);
            if (reject_reason == NO_ERROR)
            { 
                /*
                 * SCO is temporarily disabled awaiting ACK to confirm LMP_accepted
                 * Update the parameters and Send an LMP_accepted.
                 */
                p_sco_link->state = SCO_CHANGE_PKT_ACCEPT_ACK_PENDING;
                LMscoctr_Assign_SCO_Parameters(p_sco_link, &temp_sco_link);
                _Accept_SCO_Link(p_link, p_link->current_proc_tid_role);  
            }
        }
    }

    return reject_reason;
}
#endif /* (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1) */

/*****************************************************
 *****************************************************
 ***    eSCO FUNCTIONALITY                         **** 
 *****************************************************
 *****************************************************/
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
/**************************************************************************
 *  FUNCTION : LMconnection_LMP_eSCO_Accepted_Ack
 *
 *  DESCRIPTION : 
 *  This function is called on the receipt of a ACK for an LMP_Accepted_Ext
 *  for an eSCO Link Req which initiates a new eSCO link,
 *
 *************************************************************************/
static void LMconnection_LMP_eSCO_Accepted_Ack_Handler(t_lmp_link *p_link);

void LMconnection_LMP_eSCO_Accepted_Ack(t_deviceIndex device_index)
{
    t_lmp_link *p_link = LMaclctr_Find_Device_Index(device_index);
    LM_DEFER_FROM_INTERRUPT_CONTEXT(LMconnection_LMP_eSCO_Accepted_Ack_Handler, p_link);
}

static void LMconnection_LMP_eSCO_Accepted_Ack_Handler(t_lmp_link *p_link)
{
    t_sco_info *p_esco_link;
    t_esco_transaction_type type;

    p_esco_link = LMscoctr_Get_Topical_eSCO_Link();
    SYSdebug_Assert(p_esco_link);
    type = LMscoctr_Get_Topical_eSCO_Link_Transaction_Type();

    if(ESCO_SETUP == type)
        _Finalise_Synchronous_Connection_Setup(p_link, p_esco_link, NO_ERROR);
    else
        _Finalise_Synchronous_Connection_Change(p_link, p_esco_link, NO_ERROR);
}

/*****************************************************************************
 * FUNCTION :- LMconnection_LMP_Remove_eSCO_Link_Req
 *
 * DESCRIPTION :- 
 * This function is responsible for handling an incoming LMP_Remove_eSCO_Link_Req
 * PDU.
 *****************************************************************************/
t_error LMconnection_LMP_Remove_eSCO_Link_Req(t_lmp_link *p_link, t_p_pdu p_pdu)
{
    u_int8 lm_esco_handle;
    t_error reason;
    t_sco_info  *p_esco_link;

    /* Retrieve the SCO handle and detach reason from the incoming PDU */
    lm_esco_handle = *p_pdu;
    p_pdu++;
    reason = (t_error)*p_pdu;

    /* Find the eSCO link */
    p_esco_link = LMscoctr_Find_SCO_By_SCO_Handle(lm_esco_handle);
    p_link->current_proc_tid_role = p_link->role ^ 0x01;

    if(p_esco_link)
    {
        LMscoctr_Set_Topical_eSCO_Link(p_esco_link, ESCO_REMOVE);
        p_link->detach_reason = reason;

        /* 
         * Send an Accepted to the peer 
         */
        LM_Encode_LMP_Accepted_Ext_PDU(p_link, LMP_REMOVE_ESCO_LINK_REQ);

        /* 
         * Send a Disconnection Complete to the upper layer 
         * Detach reason is that obtained from LMP_Remove_SCO_Link_Req
         */
        LMconnection_LM_Disconnect_SCO(p_esco_link, reason);

        /*
         * The removal of a SCO may allow the IUT to adjust
         * the ACL packet types on all links.
         */
        LMpol_Adjust_ACL_Packet_Types_On_All_Links(LMcontxt_LEAVING_SCO);
    }
    else
    {
        /* 
         * Send a Not Accepted to the peer indicating the eSCO handle is invalid 
         */
        return INVALID_LMP_PARAMETERS;
    }

    return NO_ERROR;
}


/************************************************************************
 *  FUNCTION : LMconnection_LMP_Accepted_Ext_Remove_eSCO_Link_Req
 *
 *  DESCRIPTION :
 *  Handles an incoming LMP_accepted_ext for an LMP_Remove_SCO_Link_Req 
 *  from peer (master or slave)
 *
 *************************************************************************/
void LMconnection_LMP_Accepted_Ext_Remove_eSCO_Link_Req(t_lmp_link *p_link)
{
        /*
         * Do nothing. Do not wait for the accepted to remove
         * the eSCO link. The peer cannot reject in any case.
         */
}


/************************************************************************
 *  FUNCTION : LMconnection_LMP_Not_Accepted_Ext_eSCO_Link_Req
 *
 *  DESCRIPTION :
 *  Handles an incoming LMP_not_accepted_ext for an LMP_SCO_Link_Req 
 *  from peer (master or slave)
 *
 *************************************************************************/
void LMconnection_LMP_Not_Accepted_Ext_eSCO_Link_Req(t_lmp_link *p_link, u_int8 reason)
{
    t_sco_info *p_esco_link;
    t_esco_transaction_type type;

    /*
     * 1. Is the SCO being re-negotiated or being setup.
     */ 
    p_esco_link = LMscoctr_Get_Topical_eSCO_Link();
    type = LMscoctr_Get_Topical_eSCO_Link_Transaction_Type();
    if(p_esco_link)
    {
        if(type == ESCO_SETUP)
        {
            _Finalise_Synchronous_Connection_Setup(p_link, p_esco_link, (t_error)reason);
        }
        else
        {
            _Finalise_Synchronous_Connection_Change(p_link, p_esco_link, (t_error)reason);
        }
    }

    /*
     * No more topical eSCO link.
     *
     * TODO: Ensure collisions do not turn off the topical link
     */
    LMscoctr_Set_Topical_eSCO_Link(0, (t_esco_transaction_type)0);
}


/************************************************************************
 *
 *  FUNCTION : LMconnection_LMP_Accepted_Ext_eSCO_Link_Req
 *
 *  DESCRIPTION :
 * Set up a timer (2 x t_poll) into the future, which will set up
 * the new eSCO connection. 2 x t_poll chosen as it interoperates
 * with the Cetecom 1.0.5 tester. This effectively gives the remote
 * device two attempts to get the LMP_accepted_ext from the eSCO
 * link request before the local device will switch to the new settings.
 *
 *************************************************************************/
void LMconnection_LMP_Accepted_Ext_eSCO_Link_Req(t_lmp_link *p_link)
{
    LMtmr_Set_Timer(2*p_link->poll_interval, 
        LMconnection_LMP_Accepted_Ext_eSCO_Link_Req_Timeout, p_link, 1);
}


/************************************************************************
 *  FUNCTION : LMconnection_LMP_Accepted_Ext_eSCO_Link_Req_Timeout
 *
 *  DESCRIPTION :
 *  Handles an incoming LMP_accepted_ext for an LMP_SCO_Link_Req 
 *  from peer (master or slave). 2 x t_poll after timer expires.
 *
 *************************************************************************/
void LMconnection_LMP_Accepted_Ext_eSCO_Link_Req_Timeout(t_lmp_link *p_link)
{
        t_sco_info *p_esco_link;
        t_esco_transaction_type transaction_type;

        /*
         * Is this a new link being established, an existing
         * link being re-negotiated, or an incoming PDU
         * in the wrong context
         */
        p_esco_link = LMscoctr_Get_Topical_eSCO_Link();
        transaction_type = LMscoctr_Get_Topical_eSCO_Link_Transaction_Type();

        /*
         * OK. At this point, it is a valid protocol point
         * to receive a transaction closure.
         */
        if(p_esco_link)
        {
            /*
             * Is this a "new" eSCO link or
             * a "re-configuring" eSCO link
             */
            if(transaction_type == ESCO_SETUP)
            {
                _Finalise_Synchronous_Connection_Setup(p_link, 
                    p_esco_link, NO_ERROR);
            }
            else /* transaction_type = ESCO_RECONFIGURE */
            {
                _Finalise_Synchronous_Connection_Change(p_link,
                    p_esco_link, NO_ERROR);

            }

            /*
             * No more topical eSCO link.
             */
            LMscoctr_Set_Topical_eSCO_Link(0, (t_esco_transaction_type)0);
        }

        /*
         * If there is no topical eSCO link 
         * when the LMP_accepted_ext arrives,
         * the LMP_accepted_ext should be completely ignored -
         * responding to LMP_accepted_ext's by sending
         * LMP_not_accepted_ext(LMP_accepted_ext) has the
         * potential for the peer to send 
         * LMP_not_accepted(LMP_not_accepted) starting
         * a closed loopback circuit 
         */
}


/**************************************************************************
 *  FUNCTION : LMconnection_Start_New_eSCO_Connection
 *
 *  DESCRIPTION : 
 *  This function is called once it has been determined that
 *  the incoming LMP_eSCO_link_req PDU represents the start
 *  of a new eSCO connection.
 *
 *************************************************************************/
t_error LMconnection_Start_New_eSCO_Connection(t_lmp_link* p_link, u_int8* p_pdu)
{
    t_sco_info* p_esco_link;
    t_esco_descr* p_esco_descr;
#if (PRH_BS_CFG_SYS_FILTERS_SUPPORTED == 1)  
    t_filter_settings accept_status;
#endif
	u_int16 voice_settings;


    /*
     * New Connection started by peer. Setup TI for replying.
     */
    p_link->current_proc_tid_role = (u_int8)(1-p_link->role);
   
    /*
     * Handle an LMP_eSCO_link_req representing
     * a new incoming eSCO connection.
     *
     * By the time this function receives the PDU,
     * it is sane enough to propose to the host or 
     * the filter block for
     * further processing.
     */

    /*
     * Allocate an eSCO link structure. 
     * Associate it with an ACL link via device_index and 
     * select configuration. The SCO link is not activated yet.
     * The parameters of the incoming PDU are extracted and
     * checked against the features supported on the link. 
     * If the role of the local device is MASTER then
     * the SCO parameters must be derived.
     * 
     * If a SCO Link cannot be allocated i.e. then the max number of
     * SCO links has been allocated and an LMP_Not_Accepted must be
     * sent to the peer.  The allocation is requested for a slave
     * to ensure that a container is available.
     * A connection handle/lm_sco_handle are also assigned.
     *
     * Allocate the eSCO link with the default voice setting
     * in this case. This setting can be overridden later on
     * (and the associated sco_cfg setting re-calculated if
     * either:
     * a. the peer proposes a new air mode.
     * b. the host accepts with a new voice setting
     */

#if (BUILD_TYPE!=UNIT_TEST_BUILD)
    if (g_LM_config_info.default_sco_route_via_hci)
	{
		voice_settings = (g_LM_config_info.default_voice_setting | 0x8000);
	}
	else
	{
		voice_settings = g_LM_config_info.default_voice_setting;
	}
#else
	voice_settings = g_LM_config_info.default_voice_setting;
#endif

    p_esco_link = LMscoctr_Alloc(
       	p_link->device_index, voice_settings, 
        ESCO_LINK);

    if (!p_esco_link)
    {
        return MAX_NUM_SCO;
    }

    /*
     * Record the topical eSCO link for other handlers to locate
     * the eSCO link of interest.
     */
    LMscoctr_Set_Topical_eSCO_Link(p_esco_link, ESCO_SETUP);

    LMscoctr_Extract_eSCO_Parameters_From_LMP_PDU(
        p_pdu);

    if(p_link->role == SLAVE)
    {
        p_esco_descr = LMscoctr_Get_eSCO_Scratchpad();
        p_esco_link->esco_lt_addr = p_esco_descr->esco_lt_addr;
        p_esco_link->lm_sco_handle = p_esco_descr->lm_sco_handle;
    }

    /*
     * Check the connection event filters to determine if the
     * connection should be auto-accepted or not
     */
#if (PRH_BS_CFG_SYS_FILTERS_SUPPORTED == 1)  
    accept_status = 
        (t_filter_settings)LMfltr_Connection_Filter_Check(&p_link->bd_addr,p_link->device_class,ESCO_LINK);
  
    if ( (AUTO_ACCEPT == accept_status) || 
         (AUTO_ACCEPT_WITH_MSS == accept_status))
    {
        LMconnection_LM_AutoAccept_Default_eSCO_Link(p_link);
    }
    else /* (DONT_AUTO_ACCEPT == accept_status) */
#endif
    {
        _Send_HC_Connection_Request(p_link, ESCO_LINK);
    }

    return NO_ERROR;
}


/**************************************************************************
 *  FUNCTION : LMconnection_Continue_New_eSCO_Connection
 *
 *  DESCRIPTION : 
 *  This function is called once it has been determined that
 *  the incoming LMP_eSCO_link_req PDU represents the continuation
 *  of the negotiation of a new eSCO connection.
 *
 *************************************************************************/
t_error LMconnection_Continue_New_eSCO_Connection(t_lmp_link* p_link, u_int8* p_pdu)
{
    t_sco_info* p_esco_link = LMscoctr_Get_Topical_eSCO_Link();
    t_esco_descr* p_esco_descr = LMscoctr_Get_eSCO_Scratchpad();
    t_error status = NO_ERROR;

	if(!p_esco_link)
        return NO_CONNECTION;

    if(!p_esco_descr)
        return UNSPECIFIED_ERROR;

    if(!LMscoctr_Get_LMP_Parameter_Set(p_esco_descr))
    {
        LM_Encode_LMP_Not_Accepted_Ext_PDU(p_link, LMP_ESCO_LINK_REQ, 
            UNSPECIFIED_ERROR);
        _Finalise_Synchronous_Connection_Setup(p_link, 
            p_esco_link, UNSPECIFIED_ERROR /* p_esco_link->rejection_reason */);
    }
    else
    {
        /*
         * Try the parameter set proposed by the peer.
         */
        LMscoctr_Extract_eSCO_Parameters_From_LMP_PDU(
            p_pdu);

        if(LMscoctr_Is_An_LMP_Parameter_Set(p_esco_descr->t_esco, 
            p_esco_descr->w_esco, p_esco_descr->tx_pkt_type, 
            p_esco_descr->tx_pkt_len) )
        {
            if(LMscoctr_Is_D_eSCO_Acceptable(p_esco_descr, p_link))
            {
                LM_Encode_LMP_Accepted_Ext_PDU(p_link, LMP_ESCO_LINK_REQ);
            }
            else
            {
                if(p_esco_descr->negotiation_state != ESCO_NGT_RS_VIOL)
                {
                    LMscoctr_Set_D_eSCO_and_Timing_Flag(p_esco_descr);
                    p_esco_descr->negotiation_state = ESCO_NGT_RS_VIOL;
                    LMch_Send_LMP_eSCO_Link_Req(p_link, p_esco_descr);
                }
                else
                {
                    /*
                     * All the parameters except the d value
                     * are acceptable. The peer has indicated
                     * that the previous IUT's d value (I hope)
                     * is unacceptable, so try a different d_value.
                     *
                     * Have I proposed this set of parameters
                     * before? What d value did I use.
                     *
                     */
					s_int16 alt_d_esco = LMscoctr_Get_Alt_D_eSCO(p_esco_descr, p_link);
					if (-1 != alt_d_esco)
					{
                        p_esco_descr->negotiation_state = ESCO_NGT_RS_VIOL;
                        p_esco_descr->d_esco = alt_d_esco;
                        LMch_Send_LMP_eSCO_Link_Req(p_link, p_esco_descr);
                    }
                    else
                    {
                        LM_Encode_LMP_Not_Accepted_Ext_PDU(p_link, 
                            LMP_ESCO_LINK_REQ, 
                            SCO_OFFSET_REJECTED);
                        _Finalise_Synchronous_Connection_Setup(p_link, 
                            p_esco_link, SCO_OFFSET_REJECTED /* p_esco_link->rejection_reason */);
                    }
                }
            }
        }
        else
        {
			u_int8 next_negotiation_state;

			/*
             * Try to set the flag to something meaningful.
             */
            if((p_esco_link->max_latency*1000) < LMscoctr_Calculate_Latency_In_uSec(p_esco_descr->t_esco,
                p_esco_descr->w_esco, p_esco_descr->tx_pkt_type,
                p_esco_descr->rx_pkt_type) )
            {
                next_negotiation_state = ESCO_NGT_LAT_VIOL;
            }
            else
            {
                next_negotiation_state = ESCO_NGT_UNSUPPORTED_FEATURE;
            }

            if(LMscoctr_Get_LMP_Parameter_Set(p_esco_descr))
            {
				p_esco_descr->negotiation_state = next_negotiation_state;
				LMscoctr_Set_D_eSCO_and_Timing_Flag(p_esco_descr);
				LMch_Send_LMP_eSCO_Link_Req(p_link, p_esco_descr);
            }
            else
            {
                LM_Encode_LMP_Not_Accepted_Ext_PDU(p_link, LMP_ESCO_LINK_REQ, 
                    SCO_INTERVAL_REJECTED);
                _Finalise_Synchronous_Connection_Setup(p_link, 
                    p_esco_link, SCO_INTERVAL_REJECTED);
            }
        }
    }

    return status;
}


/**************************************************************************
 *  FUNCTION : LMconnection_Start_Change_eSCO_Connection
 *
 *  DESCRIPTION : 
 *  This function is called once it has been determined that
 *  the incoming LMP_eSCO_link_req PDU represents the start
 *  of the re-negotiation of an existing eSCO connection.
 *
 *************************************************************************/
t_error LMconnection_Start_Change_eSCO_Connection(t_lmp_link* p_link, u_int8* p_pdu)
{
    t_sco_info* p_esco_link;
    t_esco_descr* p_esco_descr = LMscoctr_Get_eSCO_Scratchpad();
    
    p_esco_link = LMscoctr_Find_SCO_By_SCO_Handle(p_pdu[0] /* lm_sco_handle */);

    SYSdebug_Assert(p_esco_link);

    /*
     * Record the topical eSCO link for other handlers to locate
     * the eSCO link of interest.
     */
    LMscoctr_Set_Topical_eSCO_Link(p_esco_link, ESCO_RECONFIGURE);

    LMscoctr_Suspend_Link(p_esco_link);

    LMscoctr_Resume_Link(p_esco_link);

    /*
     * New Connection started by peer. Setup TI for replying.
     */
    p_link->current_proc_tid_role = (u_int8)(1-p_link->role);

    LMscoctr_Extract_eSCO_Parameters_From_LMP_PDU(p_pdu);

    /*
     * If link policy can determine a basis for rejecting change,
     * negotiate or reject.
     */
    if(p_esco_link->tx_bandwidth != LMscoctr_Calculate_Bandwidth(p_esco_descr) )
    {
        LM_Encode_LMP_Not_Accepted_Ext_PDU(p_link, LMP_ESCO_LINK_REQ, INVALID_LMP_PARAMETERS);
        _Finalise_Synchronous_Connection_Change(p_link, 
            p_esco_link, INVALID_LMP_PARAMETERS);
    }
    else if(LMscoctr_Is_An_LMP_Parameter_Set(p_esco_descr->t_esco, 
        p_esco_descr->w_esco, p_esco_descr->tx_pkt_type,
        p_esco_descr->tx_pkt_len) )
    {
        LM_Encode_LMP_Accepted_Ext_PDU(p_link, LMP_ESCO_LINK_REQ);
    }   
    else
	{
		u_int8 next_negotiation_state;

		if((p_esco_link->max_latency*1000) < LMscoctr_Calculate_Latency_In_uSec(p_esco_descr->t_esco,
		     p_esco_descr->w_esco, p_esco_descr->tx_pkt_type,
			p_esco_descr->rx_pkt_type) )
		{
			 next_negotiation_state = ESCO_NGT_LAT_VIOL;
		}
		else
		{
			 next_negotiation_state = ESCO_NGT_UNSUPPORTED_FEATURE;
		}

		if(LMscoctr_Get_LMP_Parameter_Set(p_esco_descr))
		{
			p_esco_descr->negotiation_state = next_negotiation_state;
			LMscoctr_Set_D_eSCO_and_Timing_Flag(p_esco_descr);
			LMch_Send_LMP_eSCO_Link_Req(p_link, p_esco_descr);
		}
		else
		{
			LM_Encode_LMP_Not_Accepted_Ext_PDU(p_link, LMP_ESCO_LINK_REQ, UNSPECIFIED_ERROR);
			_Finalise_Synchronous_Connection_Change(p_link, 
				p_esco_link, SCO_INTERVAL_REJECTED);
		}
	}

    return NO_ERROR;
}


/**************************************************************************
 *  FUNCTION : LMconnection_Continue_Change_eSCO_Connection
 *
 *  DESCRIPTION : 
 *  This function is called once it has been determined that
 *  the incoming LMP_eSCO_link_req PDU represents the continuation
 *  of the re-negotiation of an existing eSCO connection.
 *
 *************************************************************************/
t_error LMconnection_Continue_Change_eSCO_Connection(t_lmp_link* p_link, u_int8* p_pdu)
{
    t_sco_info* p_esco_link;
    t_esco_descr* p_esco_descr = LMscoctr_Get_eSCO_Scratchpad();
    t_error status = NO_ERROR;
    
    p_esco_link = LMscoctr_Find_SCO_By_SCO_Handle(p_pdu[0] /* lm_sco_handle */);

    if(!p_esco_link)
        return NO_CONNECTION;

    if(!p_esco_descr)
        return UNSPECIFIED_ERROR;

    LMscoctr_Suspend_Link(p_esco_link);

    if(!LMscoctr_Get_LMP_Parameter_Set(p_esco_descr))
    {
                LM_Encode_LMP_Not_Accepted_Ext_PDU(p_link, LMP_ESCO_LINK_REQ,
                    UNSPECIFIED_ERROR);
        _Finalise_Synchronous_Connection_Change(p_link,
            p_esco_link, UNSPECIFIED_ERROR);
    }
    else
    {
        /*
         * Try the parameter set proposed by the peer.
         */
        LMscoctr_Extract_eSCO_Parameters_From_LMP_PDU(p_pdu);

        if(LMscoctr_Is_An_LMP_Parameter_Set(p_esco_descr->t_esco,
            p_esco_descr->w_esco, p_esco_descr->tx_pkt_type,
            p_esco_descr->tx_pkt_len) )
        {
            if(LMscoctr_Is_D_eSCO_Acceptable(p_esco_descr, p_link) )
            {
                LM_Encode_LMP_Accepted_Ext_PDU(p_link, LMP_ESCO_LINK_REQ);
            }
            else
            {
                if(p_esco_descr->negotiation_state != ESCO_NGT_RS_VIOL)
                {
                    LMscoctr_Set_D_eSCO_and_Timing_Flag(p_esco_descr);
                    p_esco_descr->negotiation_state = ESCO_NGT_RS_VIOL;
                    LMch_Send_LMP_eSCO_Link_Req(p_link, p_esco_descr);
                }
                else
                {
					s_int16 alt_d_esco = LMscoctr_Get_Alt_D_eSCO(p_esco_descr, p_link);
					if (-1 != alt_d_esco)
					{
                        p_esco_descr->negotiation_state = ESCO_NGT_RS_VIOL;
                        p_esco_descr->d_esco = alt_d_esco;
                        LMch_Send_LMP_eSCO_Link_Req(p_link, p_esco_descr);
                    }
                    else
                    {
                        LM_Encode_LMP_Not_Accepted_Ext_PDU(p_link,
                            LMP_ESCO_LINK_REQ,
                            SCO_OFFSET_REJECTED);
                        _Finalise_Synchronous_Connection_Change(p_link,
                            p_esco_link, SCO_OFFSET_REJECTED);
                    }
                }

            }
        }
        else
        {
			u_int8 next_negotiation_state;

            /*
             * Try to set the flag to something meaningful.
             */
            if( (p_esco_link->max_latency*1000) < LMscoctr_Calculate_Latency_In_uSec(p_esco_descr->t_esco,
                p_esco_descr->w_esco, p_esco_descr->tx_pkt_type,
                p_esco_descr->rx_pkt_type) )
            {
                next_negotiation_state = ESCO_NGT_LAT_VIOL;
            }
            else
            {
                next_negotiation_state = ESCO_NGT_UNSUPPORTED_FEATURE;
            }

            if(LMscoctr_Get_LMP_Parameter_Set(p_esco_descr))
            {
			    p_esco_descr->negotiation_state = next_negotiation_state;
			    LMscoctr_Set_D_eSCO_and_Timing_Flag(p_esco_descr);
			    LMch_Send_LMP_eSCO_Link_Req(p_link, p_esco_descr);
            }
            else
            {
                LM_Encode_LMP_Not_Accepted_Ext_PDU(p_link, LMP_ESCO_LINK_REQ,
                    SCO_INTERVAL_REJECTED);
                _Finalise_Synchronous_Connection_Change(p_link,
                    p_esco_link, SCO_INTERVAL_REJECTED);
            }
        }
    }

    LMscoctr_Resume_Link(p_esco_link);

    return status;
}


/**************************************************************************
 *  FUNCTION : LMconnection_Get_eSCO_Link_Req_Context
 *
 *  DESCRIPTION : 
 *  This function determines if an LMP_eSCO_link_req PDU is associated
 *  with a new eSCO connection, is the continuation of a new eSCO
 *  connection setup, is the start of a change eSCO link or is
 *  associates with the continuation of the change of an eSCO link.
 *
 *************************************************************************/
t_esco_link_req_context LMconnection_Get_eSCO_Link_Req_Context(u_int8 role, 
    u_int8 lm_sco_handle, u_int8 neg_state)
{
    t_esco_link_req_context context = FIRST_NEW_ESCO_PDU;
    t_sco_info *p_esco_link = 0;
    t_sco_info *p_esco_topical_link = 0;
    t_esco_transaction_type transaction;

    if(lm_sco_handle)
        p_esco_link = LMscoctr_Find_SCO_By_SCO_Handle(lm_sco_handle);
    p_esco_topical_link = LMscoctr_Get_Topical_eSCO_Link();
    transaction = LMscoctr_Get_Topical_eSCO_Link_Transaction_Type();

    /*
     * If p_esco_topical link exists, then there is an outstanding transaction.
     */
    if(p_esco_topical_link)
    {
        /*
         * Is this a setup or a continue?
         */
        if(transaction == ESCO_RECONFIGURE)
            context = CONTINUE_CHANGE_ESCO_PDU;
        else
            context = CONTINUE_NEW_ESCO_PDU;

    }
    else /* this is a start */
    {
        /*
         * If an existing SCO link can be mapped to this PDU
         */
        if(p_esco_link)
            context = FIRST_CHANGE_ESCO_PDU;
        else
            context = FIRST_NEW_ESCO_PDU;
    }


    return context;
}


/************************************************************************
 *  FUNCTION : 
 *
 *  DESCRIPTION :
 *  Handles an incoming LMP_SCO_Link_Req from peer (master or slave)
 *
 * The LMP_eSCO_link_req parameters in p_pdu are:
 *  eSCO handle
 *  eSCO LT_ADDR
 *  timing control flags 
 *  D esco
 *  T esco
 *  W esco
 *  SCO packet type (m-->s)
 *  SCO packet type (s-->m)
 *  packet length (m->s)
 *  packet length (s->m)
 *  air mode
 *  negotiation mode
 *************************************************************************/
t_error LMconnection_LMP_eSCO_Link_Req(t_lmp_link *p_link, t_p_pdu p_pdu)
{
    t_error reject_reason = NO_ERROR;
    t_esco_link_req_context context = FIRST_NEW_ESCO_PDU;

    u_int8 lm_sco_handle = p_pdu[0];
    u_int8 lm_negotiation_state = p_pdu[13];
    t_error status = NO_ERROR;

     status = _Cancel_Pending_Page_Inquiry();
    /*
     * 1. Parameter Range Check
     * 2. Check against local features
     * 3. Check against remote features
     * 4. Check air-mode against current air-mode.
     * 5. Check parameters for sanity check.
     */
    status = LMscoctr_Validate_Peer_eSCO_Parameters(p_link, (u_int8*)p_pdu);
    if(status != NO_ERROR)
        return status;

    /*
     * There are 4 broad cases (states?) where this PDU is
     * valid:
     *
     * 1) New connection, new transaction.
     *    This is the first LMP PDU for an eSCO setup.
     * 2) New connection, old transaction.
     *    This is an LMP PDU involved in eSCO setup.
     * 3) Old connection, new transaction.
     *    This is an LMP PDU starting an eSCO change.
     * 4) Old connection, old transaction.
     *    This is an LMP PDU continuing an eSCO change.
     */
    context = LMconnection_Get_eSCO_Link_Req_Context(
        (u_int8)p_link->role, lm_sco_handle, lm_negotiation_state);
    if(FIRST_NEW_ESCO_PDU == context)
    {
        return LMconnection_Start_New_eSCO_Connection(p_link, (u_int8*)p_pdu);
    }
    else if(CONTINUE_NEW_ESCO_PDU == context)
    {
        return LMconnection_Continue_New_eSCO_Connection(p_link, (u_int8*)p_pdu);
    }
    else if(FIRST_CHANGE_ESCO_PDU == context)
    {
        return LMconnection_Start_Change_eSCO_Connection(p_link, (u_int8*)p_pdu);
    }
    else if(CONTINUE_CHANGE_ESCO_PDU == context)
    {
        return LMconnection_Continue_Change_eSCO_Connection(p_link, (u_int8*)p_pdu);
    }
    else
    {
        /*
         *    TODO
         * 7. Stop and resolve any eSCO link setup
         *    collisions.
         */
        /* ErrorHandling(); TODO */;
    }

    return reject_reason;
}
#endif /* (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1) */


/*****************************************************
 ***    LOCAL HELPER FUNCTIONS                    **** 
 *****************************************************/
/**************************************************************
 * FUNCTION :- LMconnection_Local_Setup_Complete
 *
 * DESCRIPTION :- 
 * This function is called when the local side of connection
 * establishment is complete. A LMP_SETUP_COMPLETE message is 
 * send to the peer and the "setup_complete_bit_mask" checked 
 * to determine if a Connection establishment is completed.
 * If the Connection establishment is complete the connection
 * complete event is sent to the higher layers.
 *************************************************************************/
void LMconnection_Local_Setup_Complete(t_lmp_link *p_link)
{
    /****************************************************
     * Send an LMP_Setup_Complete PDU to peer on the link 
    ****************************************************/
    _Send_LMP_Connection_PDU(LMP_SETUP_COMPLETE,p_link);
    
    p_link->setup_complete_bit_mask |= LMP_Local_Setup_Complete;
    if (p_link->setup_complete_bit_mask == LMP_Both_Setup_Completes)        
    {
        /* 
         * Change the LMP link state to Active 
         */
        p_link->state = LMP_ACTIVE;
        _LMch_p_pending_link = p_link;

        if(p_link->features_complete_bit_mask == LMP_Both_Features_Complete)
        {
            LMconnection_Finalise_Connection_Setup(p_link);
        }
        else
        {
            p_link->current_proc_tid_role = p_link->role;
            _Send_LMP_Connection_PDU(LMP_FEATURES_REQ, p_link);
        }
    }
}

/*****************************************************
 ***    FUNCTIONS TO SEND LMP MESSAGES TO PEERS   **** 
 *****************************************************/


/**************************************************************************
 * FUNCTION :- _Send_LMP_Connection_PDU
 *
 * DESCRIPTION
 * This function is responsible for encoding one of a number of LMP PDUs 
 * The following LMP PDUs are supported :
 *     LMP_SWITCH_REQ, LMP_SLOT_OFFSET, LMP_SETUP_COMPLETE, LMP_FEATURES_REQ 
 *     LMP_FEATURES_RES, LMP_HOST_CONNECTION_REQ 
 *************************************************************************/
void _Send_LMP_Connection_PDU(u_int8 opcode,t_lmp_link *p_link)
{
    t_lmp_pdu_info pdu_info;
    t_error status = NO_ERROR;

    pdu_info.opcode = opcode;
    pdu_info.tid_role = p_link->current_proc_tid_role;

    switch(opcode)
    {
#if (PRH_BS_CFG_SYS_LMP_SLOT_OFFSET_SUPPORTED==1)
    case LMP_SLOT_OFFSET :
        /*
         * slot offset is the difference between the clk edge of the piconet
         * on which the LMP_slot_offset PDU is transmitter, and the master
         * specified by the bd_addr in the LMP_slot_offset message
         */
        pdu_info.slot_offset = LC_Read_Local_Slot_Offset(p_link->device_index);
        pdu_info.ptr.p_uint8 = BDADDR_Get_Byte_Array_Ref((t_bd_addr *)
                               SYSconfig_Get_Local_BD_Addr_Ref());
        break;
#endif /* (PRH_BS_CFG_SYS_LMP_SLOT_OFFSET_SUPPORTED==1) */

#if (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1)
    case LMP_SWITCH_REQ :
    {
        t_clock clock;
        u_int32 switch_instant_offset;

        if (p_link->role == MASTER)
            clock = LC_Get_Native_Clock();
        else
            clock = LC_Get_Piconet_Clock(p_link->device_index);

        clock = clock >> 1;
        
        /* 
         * The switch instant should be at least 2 x Tpoll or 32
         * (whichever is greater).
         *
         * Additionally a guard is used to prevent a switch
         * instant so far in the future that drift may cause the
         * switch to fail.
         *
         * Empirically, it has been detected at UPFs, that
         * 320 slots is an interoperable switch instant
         */
#if (PRH_BS_CFG_SYS_PERSISTENT_SCO_IN_SCANS_SUPPORTED==1)
        /*
         * Persistent SCO in scans - Switch into piconet with SCO.
         * Reduce time to switch instance.
         */
        if (LMconfig_LM_Connected_As_SCO_Master())
        {
            switch_instant_offset = 2 * p_link->poll_interval + 32;
        }
        else
#endif
        if ((p_link->poll_interval > MSS_MIN_POLL_IVAL) && 
            (p_link->poll_interval < MSS_MAX_POLL_IVAL))
        {
            switch_instant_offset = 8 * p_link->poll_interval;
        }
        else
        {
            switch_instant_offset = MSS_DEFAULT_INSTANT_OFFSET;
        }

        /*
         * Remove bottom bit of clock. Shifts are fastest way to do this.
         */
        pdu_info.instant = BTtimer_Clock_Add_Safe_Clockwrap( ((clock>>1)<<1), 
            switch_instant_offset);

        {
        u_int32 mss_lookahead = MSS_EARLY_START_SLOT_OFFSET_ADJUST;

#if (PRH_BS_CFG_SYS_PERSISTENT_SCO_IN_SCANS_SUPPORTED==1)
        if (LMconfig_LM_Connected_As_SCO_Master())
        {
            /*
             * Persistent SCO in scans - Switch into piconet with SCO.
             * Increase lookahead for change back from M(SCO) at TDD.
             */
            mss_lookahead += 2;
        }
#endif
        p_link->gen_switch_timeout_index = LMtmr_Set_Timer(
            (pdu_info.instant - clock - mss_lookahead),
            LMconnection_LMP_Switch_Timeout,
            p_link,
            1);
        } // end scope mss_lookahead

#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
        /*
         * Set timer to switch back to this piconet before switch instant.
         * Note: gaurd time of 0x20 slots.
         */
        if(LMconfig_LM_Connected_As_Scatternet())
        {
            LMtmr_Set_Timer((pdu_info.instant - clock - 
                MSS_EARLY_START_SLOT_OFFSET_ADJUST) - 0x20, &LMch_Change_Piconet, p_link, 1);
        }
#endif
     }
     break;


#endif /* (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1) */

    case LMP_SETUP_COMPLETE :
        pdu_info.tid_role = p_link->role & 0x01;
        break;

    case LMP_FEATURES_REQ : /* Intentional Fall through */
    case LMP_FEATURES_RES :
        p_link->features_complete_bit_mask |= LMP_Local_Features_Complete;
        pdu_info.ptr.p_uint8 = SYSconfig_Get_LMP_Features_Ref();        
        break;
        
    case LMP_HOST_CONNECTION_REQ :  
        /******************************************************
         *  Send an LMP Host Connection Req to initiate a higher
         *  layer connection.
         ******************************************************/    
		p_link->host_connection_request_sent = TRUE;
        break;

    default :
        status = UNSPECIFIED_ERROR;
        break;
    }

    if (status == NO_ERROR)
    {
        LM_Encode_LMP_PDU(p_link->device_index, &pdu_info);
    }
}


/**************************************************************************
 *  FUNCTION : LMconnection_Send_LMP_Detach
 *
 *  DESCRIPTION :
 *  Send an LMP Detach message to release the LMP & Baseband
 *  link to the peer.
 *  The detach reason is stored in a global static to be used
 *  for event generation - on receipt of the detach ack.
 * 
 * The LM queues the LMP_detach for transmission and starts a timer for 6*T poll slots .
 * If the initiating LM receives the Baseband-level acknowledgement before the timer
 * expires it now starts a timer for 3*T poll slots. When this timer expires 
 * (and if the initiating LM is the
 * master) the AM_ADDR can be re-used immediately. If the initial timer expires
 * then the initiating LM drops the link and starts a timer for T linksupervisiontimeout
 * slots after which the AM_ADDR can be re-used (if the initiating LM is the mas-ter).
 *************************************************************************/
void LMconnection_Send_LMP_Detach(t_lmp_link *p_link, t_error reason)
{
    t_lmp_pdu_info pdu_info;

    /*  
     * p_link->detach_reason = reason;   !! Error the reason parameter 
     * is for the peer and not to be assigned to p_link->reason 
     */

    pdu_info.tid_role = p_link->role;
    pdu_info.opcode = LMP_DETACH;
    pdu_info.reason = reason;

    /* 
     * Change the Link State to detach pending - so no further 
     * incoming LMP messages are processed while the detach is 
     * pending.
     */

    /* 
     * Removed as this effects Sniff Mode (i.e Timeout = 1, Attempt = 1) 
     * p_link->state |= LMP_DETACH_ACK_PENDING 
     */
    p_link->gen_detach_timeout_index = LMtmr_Set_Timer(
        6*p_link->poll_interval, LMconnection_Detach_Timeout, p_link, 1);
    LM_Encode_LMP_PDU(p_link->device_index,&pdu_info);

}

/**************************************************************************
 *  FUNCTION : LMconnection_Detach_Timeout
 *
 *  DESCRIPTION :
 *  If the initial timer expires then the initiating LM drops the link 
 *  and starts a timer for T linksupervisiontimeout slots after which 
 *  the AM_ADDR can be re-used.
 *************************************************************************/
void LMconnection_Detach_Timeout(t_lmp_link *p_link)
{
 /*  If the initial timer expires then the initiating LM drops the link and starts a
  *  timer for T linksupervisiontimeout slots after which the AM_ADDR can be re-used 
  *  ( if the supervision timeout is set to infinity, the process can be sped up as 
  *  the AM_ADDR can be re-used immediately).
  */
    if (p_link->supervision_timeout == 0)
    {
        LMconnection_Disconnect_Acl(p_link);
    }
    else
    {
        p_link->state |= LMP_W4_SUPERVISION_TIMEOUT;
    }

    /*
     * if device is in test mode then it must leave testmode when Detach
     * By having it here, it allows the LMP_detach to be sent on the
     * existing frequency
     */
    LC_Testmode_Exit();

}

#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
/**************************************************************************
 *  FUNCTION : _Send_LMP_Remove_SCO_Link_Req
 *
 *  DESCRIPTION :
 *  Send an LMP Remove SCO Link Req PDU to initiate removal
 *  of a SCO link from the peer.
 *************************************************************************/
void _Send_LMP_Remove_SCO_Link_Req(
    t_lmp_link *p_link, t_sco_info *p_sco_link, t_error reason)
{
    t_lmp_pdu_info pdu_info;

    /* 
     * Construct the parameters for the PDU 
     */
    pdu_info.tid_role = p_link->current_proc_tid_role ;
    pdu_info.opcode = LMP_REMOVE_SCO_LINK_REQ;
    pdu_info.ptr.p_sco_link = p_sco_link; 
    pdu_info.reason = reason;
    if(reason != REMOTE_USER_TERMINATED_CONNECTION)
    {
        p_link->detach_reason = reason; 
    }
    else
    {
        p_link->detach_reason = LOCAL_TERMINATED_CONNECTION;
    }

    LM_Encode_LMP_PDU(p_link->device_index,&pdu_info);
}  

/**************************************************************************
 *  FUNCTION : _Accept_SCO_Link
 *
 *  DESCRIPTION :
 *  Send an LMP Accepted for the LMP_SCO_Link_Req PDU
 *  Note that a callback in invoked once this LMP_Accept
 *  has been acknowledged.
 *
 *  Send LMP_max_slot(1) to all peer devices.
 *  Ensure that the local device will only use one slot packets.
 *
 *************************************************************************/
void _Accept_SCO_Link(const t_lmp_link *p_link, u_int8 t_id)
{
    t_lmp_pdu_info pdu_info;
            
    pdu_info.opcode = LMP_ACCEPTED;
    pdu_info.return_opcode = LMP_SCO_LINK_REQ;
    pdu_info.ack_required = 0x01;
    pdu_info.tid_role = t_id; 
    
    LM_Encode_LMP_PDU(p_link->device_index, &pdu_info);
}


/**************************************************************************
 *  FUNCTION : _Send_LMP_SCO_Link_Req
 *
 *  DESCRIPTION :
 *  Send an LMP SCO Link Req message to initiate a sco 
 *  link to the peer
 *
 *  p_link                  Reference to ACL link associated with SCO
 *  p_sco_link              Reference to SCO link.
 *************************************************************************/
void _Send_LMP_SCO_Link_Req(const t_lmp_link *p_link, t_sco_info *p_sco_link)
{
    t_lmp_pdu_info pdu_info;

    pdu_info.opcode = LMP_SCO_LINK_REQ;
    pdu_info.tid_role = p_link->current_proc_tid_role ;
    pdu_info.ptr.p_sco_link = p_sco_link;

    /* 
     * Explicitly add - No SCO ack required 
     */
    pdu_info.ack_required = 0x00;
    LM_Encode_LMP_PDU(p_link->device_index,&pdu_info);
}
#endif /* (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1) */


#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1)

/**************************************************************************
 *  FUNCTION : LMscoctr_Send_LMP_eSCO_Link_Req
 *
 *  DESCRIPTION :
 *  Send an LMP eSCO Link Req message to initiate a sco 
 *  link to the peer
 *
 *  p_link                  Reference to ACL link associated with eSCO
 *  p_esco_link              Reference to eSCO link.
 *************************************************************************/
void LMch_Send_LMP_eSCO_Link_Req(const t_lmp_link *p_link, t_esco_descr *p_esco_descr)
{
    t_lmp_pdu_info pdu_info;

    _LMsco_LMP_esco_link_req_PDU[0] = (127<<1) + p_link->current_proc_tid_role;
    _LMsco_LMP_esco_link_req_PDU[1] = (LMP_ESCO_LINK_REQ & 0xff);
    _LMsco_LMP_esco_link_req_PDU[2] = p_esco_descr->lm_sco_handle;
    _LMsco_LMP_esco_link_req_PDU[3] = p_esco_descr->esco_lt_addr; 
#ifdef SYS_DEBUG_IMITATE_CSR
    _LMsco_LMP_esco_link_req_PDU[4] = ((p_esco_descr->timing_ctrl_flag << 1) | 0x04);
#else
    _LMsco_LMP_esco_link_req_PDU[4] = p_esco_descr->timing_ctrl_flag << 1;
#endif
    _LMsco_LMP_esco_link_req_PDU[5] = p_esco_descr->d_esco;
    _LMsco_LMP_esco_link_req_PDU[6] = p_esco_descr->t_esco;
    _LMsco_LMP_esco_link_req_PDU[7] = p_esco_descr->w_esco;
    if (MASTER == p_link->role) 
    {
        _LMsco_LMP_esco_link_req_PDU[8] = LMscoctr_Get_eSCO_LMP_Type_From_BB_Packet(p_esco_descr->tx_pkt_type);
        _LMsco_LMP_esco_link_req_PDU[9] = LMscoctr_Get_eSCO_LMP_Type_From_BB_Packet(p_esco_descr->rx_pkt_type);
    }
    else
    {
        _LMsco_LMP_esco_link_req_PDU[8] = LMscoctr_Get_eSCO_LMP_Type_From_BB_Packet(p_esco_descr->rx_pkt_type);
        _LMsco_LMP_esco_link_req_PDU[9] = LMscoctr_Get_eSCO_LMP_Type_From_BB_Packet(p_esco_descr->tx_pkt_type);
    }

    _LMsco_LMP_esco_link_req_PDU[10] = p_esco_descr->tx_pkt_len & 0xFF;
    _LMsco_LMP_esco_link_req_PDU[11] = p_esco_descr->tx_pkt_len >> 8;
    _LMsco_LMP_esco_link_req_PDU[12] = p_esco_descr->rx_pkt_len & 0xFF;
    _LMsco_LMP_esco_link_req_PDU[13] = p_esco_descr->rx_pkt_len >> 8;
    _LMsco_LMP_esco_link_req_PDU[14] = LMscoctr_Map_AirMode_HCI2LMP((u_int8)(p_esco_descr->voice_setting & 3));
    _LMsco_LMP_esco_link_req_PDU[15] = p_esco_descr->negotiation_state;

    /* 
     * Explicitly add - No SCO ack required 
     */
    pdu_info.ack_required = 0x00;
    LM_Encode_LMP_PDU_Fully_Encoded(p_link->device_index,
        LMP_ESCO_LINK_REQ, _LMsco_LMP_esco_link_req_PDU);
}

/**************************************************************************
 *  FUNCTION : _Send_LMP_Remove_eSCO_Link_Req
 *
 *  DESCRIPTION :
 *  Send an LMP Remove eSCO Link Req PDU to initiate removal
 *  of a SCO link from the peer.
 *************************************************************************/
void _Send_LMP_Remove_eSCO_Link_Req(
    t_lmp_link *p_link, t_sco_info *p_esco_link, t_error reason)
{
    t_lmp_pdu_info pdu_info;

    _LMsco_LMP_remove_esco_link_req_PDU[0] = (127<<1) + p_link->current_proc_tid_role;
    _LMsco_LMP_remove_esco_link_req_PDU[1] = (LMP_REMOVE_ESCO_LINK_REQ & 0xFF);
    _LMsco_LMP_remove_esco_link_req_PDU[2] = p_esco_link->lm_sco_handle;
    _LMsco_LMP_remove_esco_link_req_PDU[3] = reason;

    /* 
     * Construct the parameters for the PDU 
     */
    if(reason != REMOTE_USER_TERMINATED_CONNECTION)
        p_link->detach_reason = reason; 
    else
        p_link->detach_reason = LOCAL_TERMINATED_CONNECTION;

    pdu_info.ack_required = 0x00; /* no ack required */

    LM_Encode_LMP_PDU_Fully_Encoded(p_link->device_index, LMP_REMOVE_ESCO_LINK_REQ, 
        _LMsco_LMP_remove_esco_link_req_PDU);
}  

#endif /* (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1) */

/**************************************************************************
 *****************************************************
 ***                                              ****
 ***    FUNCTIONS TO SEND EVENTS TO UPPER LAYERS  **** 
 ***                                              ****
 *****************************************************
 *************************************************************************/

/**************************************************************************
 *  FUNCTION : _Send_HC_Connection_Complete_Event_ACL
 *
 *  DESCRIPTION :
 *  Send an HC_Connection_Complete event to the higher layer.
 *************************************************************************/
static void _Send_HC_Connection_Complete_Event_ACL(
    t_lmp_link *p_link, t_error status)
{
    t_lm_event_info event_info;
   
    /* 
     * Construct the parameters for the event 
     */
    event_info.status = status;
    event_info.p_bd_addr = &p_link->bd_addr;
    event_info.mode = p_link->encrypt_mode;
    event_info.link_type = ACL_LINK;

    /* 
     * If the connection complete is for an ACL link, assign the connection 
     * handle to the event structure. 
     * The handle is stored in the p_link structure.
     */
    if (status == NO_ERROR)
    {
        /* 
         * Change Link State to LMP_ACTIVE - 
         *  turn on led to show LMP level connection is up.. 
         *  request ack required on event sent
         */
        SYSmmi_Display_Event(eSYSmmi_LM_Connection_Event);
        p_link->state = LMP_ACTIVE;

        event_info.ack_required = TRUE;
        event_info.handle = p_link->handle; 

#if (PRH_BS_CFG_SYS_LMP_CHANNEL_DRIVEN_QUALITY_SUPPORTED==1)
        p_link->channel_quality_timeout_index = 
            LMtmr_Set_Timer(p_link->channel_quality_timeout,
                LMqos_Channel_Quality_Timeout, p_link, 0);
#endif
    }
    else
    {
        /*
         * Unable to create connection:
         *  No ACK processing required and use default connection handle
         */
        event_info.ack_required = FALSE;
        event_info.handle = 0x0000;
    }

    /* 
     * Raise a connection complete event to the higher layers
     */
    g_LM_config_info.lmp_event[_LM_CONNECT_COMPLETE]
        (HCI_CONNECTION_COMPLETE_EVENT, &event_info);

    LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_CREATE_CONN_PENDING);
}

/**************************************************************************
 *  FUNCTION : _Send_HC_Create_Connection_Cancel_Command_Complete_Event
 *
 *  DESCRIPTION :
 *  Send an HC_Command_Complete event to the higher layer.
 *************************************************************************/
static void _Send_HC_Create_Connection_Cancel_Command_Complete_Event(
            const t_bd_addr *p_bd_addr, t_error status)
{
        t_cmd_complete_event cmd_complete_info;
        
        cmd_complete_info.opcode = HCI_CREATE_CONNECTION_CANCEL;
        cmd_complete_info.status = status;
        cmd_complete_info.p_bd_addr = p_bd_addr;
        HCeg_Command_Complete_Event(&cmd_complete_info);
        LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_CANCEL_CONN_PENDING);
}

/**************************************************************************
 *  FUNCTION : _Send_HC_Read_Remote_Name_Cancel_Command_Complete_Event
 *
 *  DESCRIPTION :
 *  Send an HC_Command_Complete event to the higher layer.
 *************************************************************************/
static void _Send_HC_Read_Remote_Name_Cancel_Command_Complete_Event(
            const t_bd_addr *p_bd_addr, t_error status)
{
        t_cmd_complete_event cmd_complete_info;
        
        cmd_complete_info.opcode = HCI_REMOTE_NAME_REQUEST_CANCEL;
        cmd_complete_info.status = status;
        cmd_complete_info.p_bd_addr = p_bd_addr;
        HCeg_Command_Complete_Event(&cmd_complete_info);
        LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_RNR_CANCEL_PENDING);
}

/**************************************************************************
 *  FUNCTION : _Send_HC_Connection_Request
 *
 *  DESCRIPTION :
 *  Send an HC_Connection_Request event to the higher layer.
 *************************************************************************/
void _Send_HC_Connection_Request(t_lmp_link *p_link, t_linkType link_type)
{
    t_lm_event_info event_info; 

    /* 
     * Construct the event parameters
     */
    event_info.p_bd_addr = &p_link->bd_addr;
    event_info.cod = p_link->device_class;
    event_info.link_type = link_type;

    /* 
     * Change link State 
     */
    if (link_type == ACL_LINK)
    {
       p_link->state = W4_HOST_ACL_ACCEPT;
    } 
#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
    else if (link_type == SCO_LINK)
    {
       p_link->state |= W4_HOST_SCO_ACCEPT;
    } 
#endif  /* (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1) */
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1)
    else if (link_type == ESCO_LINK)
    {
        p_link->state |= W4_HOST_ESCO_ACCEPT;
    }
#endif /* (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1) */

	p_link->connection_request_send_via_hci = 1;

    g_LM_config_info.lmp_event[_LM_CONNECT_IND]
        (HCI_CONNECTION_REQUEST_EVENT, &event_info);

    g_LM_config_info.accept_timeout_index = LMtmr_Set_Timer(
        g_LM_config_info.accept_timeout, &LMconnection_Connection_Timeout, p_link, 1);
}

/**************************************************************************
 *  FUNCTION : _Send_HC_Disconnection_Complete
 *
 *  DESCRIPTION :
 *  Send an HC_Disconnection_Complete for an ACL link.
 *  Any associated active SCO connections are removed and the
 *  disconnection complete events are also sent.
 *************************************************************************/
void _Send_HC_Disconnection_Complete(t_lmp_link *p_link, t_error reason)
{
    t_lm_event_info event_info;
  
    if (p_link->msg_timer)
    {
        LM_Encode_Msg_Timeout(p_link);
    }

#if (PRH_BS_CFG_SYS_LMP_HOLD_SUPPORTED==1)
    if ((p_link->state & LMP_HOLD_MODE) == LMP_HOLD_MODE)
    {
        g_LM_config_info.num_acl_link_on_hold--;
        LMpol_Decr_Links_In_Low_Power();
        LMtmr_Clear_Timer(p_link->pol_info.hold_info.hold_timeout_index);
        p_link->pol_info.hold_info.hold_timeout_index = 0;
    }
#endif

#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUPPORTED==1)
    if (p_link->state & LMP_SNIFF_MODE)
    {
        p_link->state = LMP_IDLE;
        LMpol_Decr_Links_In_Low_Power();
    }
#endif


#if (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1)
    /*
     * Guard against a link supervision timeout while MSS is active.
     */
#endif

    if (g_LM_config_info.num_acl_links == 1)
    {
        LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_ACLS_ACTIVE);
    }
     
    LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_TEST_MODE);

#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
    /*
     * Free all the SCO links
     */
    {
        t_sco_info *p_sco_link;
        u_int i;

        for (i=0; i < PRH_BS_CFG_SYS_MAX_SYN_LINKS; i++)
        {
            p_sco_link = LMscoctr_Find_SCO_By_Device_Index(p_link->device_index);        
            if (p_sco_link)
            {
                /*
                 * Detach reason is that stored for the associated ACL link
                 */
                LMconnection_LM_Disconnect_SCO(p_sco_link, p_link->detach_reason);

                /*
                 * Disconnecting a SCO may allow the adjusting of all
                 * ACL packet types on all links.
                 */
                LMpol_Adjust_ACL_Packet_Types_On_All_Links(LMcontxt_LEAVING_SCO);
            }
        }
    }
#endif

    /*
     * Send the HC_Disconnection_Complete Event for the ACL link
     */
    event_info.handle = p_link->handle;
    event_info.status = NO_ERROR;
    event_info.reason = reason; /* e.g. CONNECTION_TIMEOUT */      
    g_LM_config_info.lmp_event[_LM_DISCONNECT_COMPLETE]
        (HCI_DISCONNECTION_COMPLETE_EVENT, &event_info);
}

#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
/**************************************************************************
 *  FUNCTION : LMconnection_LM_Disconnect_SCO
 *
 *  DESCRIPTION :
 *  Complete disconnect of SCO identified by SCO container pointer
 *
 *  1. Sends a Disconnection Complete to the higher layers for SCO link.
 *  2. Turns of codec if applicable
 *  3. Clean up  flow control, if applicable
 *  4. Free the SCO container
 *
 *  p_sco_link      sco link container to be disconnected
 *                  if NULL returns NO_CONNECTION
 *
 *  reason          reason parameter in HC_Disconnection_Complete event
 *
 *************************************************************************/
t_error LMconnection_LM_Disconnect_SCO(t_sco_info *p_sco_link, t_error reason)
{
    t_error status;
    t_lm_event_info event_info;
    u_int8 sco_index;
	
    if (p_sco_link)
    {
		t_lmp_link* p_link  = LMaclctr_Find_Device_Index(p_sco_link->device_index);

        sco_index = LMscoctr_Get_SCO_Index(p_sco_link);
#if ((PRH_BS_CFG_SYS_LMP_ERRONOUS_DATA_REPORTING_SUPPORTED == 1) && (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1))
		{
			t_sco_fifo hw_active_sco_fifo;
			t_devicelink  *p_dev_link = 0;

			// Find the device_link
			p_dev_link = DL_Get_Device_Ref(p_sco_link->device_index);
			if (p_dev_link)
			{
				hw_active_sco_fifo = p_dev_link->active_sco_fifo;
				if (p_dev_link->sco_commit_pending)
				{
					BTq_Commit(SCO_IN_Q,sco_index);
			        p_dev_link->sco_commit_pending = FALSE;
				}
			}
		}
#endif

		/*
		 * #2806
		 * Immediately set sco_link state to IDLE or DEACTIVATION_PENDING.
		 * Note: DEACTIVATION_PENDING state not relevant for eSCO links.
		 */

        p_sco_link->state = (MASTER==p_link->role) ? 
			((SCO_LINK==p_sco_link->link_type) ? SCO_MASTER_DEACTIVATION_PENDING : SCO_IDLE) : SCO_IDLE;

        /*
         * Send the HC_Disconnection_Complete Event for the SCO link
         */
        event_info.handle = p_sco_link->connection_handle;
        event_info.status = NO_ERROR;
        event_info.reason = reason; 
          
        g_LM_config_info.lmp_event[_LM_DISCONNECT_COMPLETE]
            (HCI_DISCONNECTION_COMPLETE_EVENT, &event_info);

        /* 
         * Turn off codec by selecting the SCO configuration as inactive
         */
        LC_Sync_Setup_SCO_CFG_Ex(sco_index, SCO_CFG_INACTIVE);

        /*
         * Release all SYN queue packets if applicable (if SYN via HCI)
         */
        HCfc_Release_All_SYN_Queue_Packets(sco_index);

        /*
         * All complete, now free the container
         */
        LMscoctr_Free(p_sco_link);

         /*
          * No more topical eSCO link.
          */
#if(PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1)
         LMscoctr_Set_Topical_eSCO_Link(0, (t_esco_transaction_type)0);
#endif
         status = NO_ERROR;
    }
    else
    {
        status = NO_CONNECTION;
    }

    return status;
}

/**************************************************************************
 *  FUNCTION : _Send_HC_Connection_Complete_Event_SCO
 *
 *  DESCRIPTION :
 *  Send an HC_Connection_Complete event to the higher layer for SCO.
 *
 *  p_link          Reference to the ACL link associated with this SCO.
 *  sco_handle      The handle to return on the event.
 *  status          The status to return on the event.
 *************************************************************************/
void _Send_HC_Connection_Complete_Event_SCO(
    const t_lmp_link *p_link, t_connectionHandle sco_handle, t_error status)
{
    t_lm_event_info event_info;
   
#if ((PRH_BS_CFG_SYS_HCI_V12_FUNCTIONALITY_SUPPORTED==1) && (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1))
    if(_LMch_host_v12_functionality_confirmed)
    {
        /* 
         * Construct the parameters for the event 
         */
        event_info.status = status;
        event_info.handle = sco_handle;
        event_info.p_bd_addr = (t_bd_addr *)&p_link->bd_addr;
        event_info.mode = p_link->encrypt_mode;
        event_info.link_type = SCO_LINK;
        event_info.transmission_interval = 0;
        event_info.retransmission_window = 0; /* no retransmission allowed on SCO link */
        event_info.rx_packet_length = 0;
        event_info.tx_packet_length = 0;
        {
        t_sco_info* p_sco_link = LMscoctr_Find_SCO_By_Connection_Handle(sco_handle);
        if(p_sco_link)
        {
            event_info.air_mode = p_sco_link->lm_air_mode;
        }
        else
        {
            event_info.air_mode = LM_AIR_MODE_CVSD; /* This is correct as per v1.2 Mar 2004.*/

        }
        }
        event_info.ack_required = FALSE;

        /* 
         * Raise a connection complete event to the higher layers
         */
        g_LM_config_info.lmp_event[_LM_CONNECT_COMPLETE]
            (HCI_SYNCHRONOUS_CONNECTION_COMPLETE_EVENT, &event_info);

		_LMch_host_v12_functionality_confirmed = FALSE;
     }
     else
#endif
     {
        /* 
         * Construct the parameters for the event 
         */
        event_info.status = status;
        event_info.p_bd_addr = (t_bd_addr *)&p_link->bd_addr;
        event_info.mode = p_link->encrypt_mode;
        event_info.link_type = SCO_LINK;
        event_info.ack_required = FALSE;
        event_info.handle = sco_handle;

        /* 
         * Raise a connection complete event to the higher layers
         */
        g_LM_config_info.lmp_event[_LM_CONNECT_COMPLETE]
            (HCI_CONNECTION_COMPLETE_EVENT, &event_info);
     }
}

/**************************************************************************
 *  FUNCTION : _Send_HC_Connection_Packet_Type_Changed_Event_SCO
 *
 *  DESCRIPTION :
 *  Send an HCI_CONNECTION_PACKET_TYPE_CHANGED_EVENT to the 
 *  higher layers for a SCO link.
 *************************************************************************/
void _Send_HC_Connection_Packet_Type_Changed_Event_SCO(
    const t_sco_info *p_sco_link, t_error outcome)
{
    t_lm_event_info event_info;

    event_info.handle = p_sco_link->connection_handle;
    event_info.status = outcome;
    event_info.value16bit = p_sco_link->packet_types;
    g_LM_config_info.lmp_event[_LM_CONNECTION_PACKET_TYPE_CHANGED_EVENT]
        (HCI_CONNECTION_PACKET_TYPE_CHANGED_EVENT, &event_info);
}
#endif /* (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1) */


#if (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1)
/**************************************************************************
 *  FUNCTION : _Send_HC_Role_Change_Event
 *
 *  DESCRIPTION :
 *  Send an HC_Role_Change_Event to the higher layers
 *
 *************************************************************************/
void _Send_HC_Role_Change_Event(t_lmp_link *p_link, t_error reason)
{
    t_lm_event_info event_info;

#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
	if (LMconfig_LM_Connected_As_Scatternet())
	{
		if (g_LM_config_info.links_in_low_power != 0)
		{
			LMch_Set_Next_Change_Piconet_Time(LMpol_Get_Interval_To_Next_Wakeup() - LMP_SLOT_GAURD_FOR_CHANGE_PICONET);
		}
		else
		{
			LMch_Set_Next_Change_Piconet_Time(PRH_BS_PICONET_SERVICE_TIME/g_LM_config_info.num_acl_links);
		}
	}
#endif
    p_link->operation_pending = NO_OPERATION;
    event_info.p_bd_addr = &p_link->bd_addr;
    event_info.status = reason;
    event_info.role = p_link->role;
    g_LM_config_info.lmp_event[_LM_ROLE_CHANGE_EVENT]
        (HCI_ROLE_CHANGE_EVENT, &event_info);
}
#endif /* (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1) */

/**************************************************************************
 *
 *  FUNCTION : _Send_HC_Remote_Features_Complete_Event
 *
 *  DESCRIPTION :
 *  Send an HC_Remote_Features_Complete_Event to the higher layers.
 *
 *************************************************************************/
void _Send_HC_Remote_Features_Complete_Event(
    t_lmp_link *p_link, t_p_pdu p_pdu, t_error status)
{
    t_lm_event_info event_info;
           
    event_info.handle = p_link->handle;
    event_info.status = status;
    event_info.p_u_int8 = p_pdu;
    g_LM_config_info.lmp_event[_LM_READ_REMOTE_SUPPORTED_FEATURES_COMPLETE_EVENT]
        (HCI_READ_REMOTE_SUPPORTED_FEATURES_COMPLETE_EVENT, &event_info);
}


/**************************************************************************
 *
 *  FUNCTION : _Send_HC_Remote_Extended_Features_Complete_Event
 *
 *  DESCRIPTION :
 *  Send an HC_Remote_Extended_Features_Request_Complete_Event 
 * to the higher layers.
 *
 *************************************************************************/
void _Send_HC_Remote_Extended_Features_Complete_Event(
    t_lmp_link *p_link, t_p_pdu p_pdu, t_error status)
{
#if (PRH_BS_CFG_SYS_HCI_V12_FUNCTIONALITY_SUPPORTED==1)
    t_lm_event_info event_info;
     
	// Only Send this event if the connection_complete has already been sent.
	if (p_link->setup_complete_bit_mask == LMP_Both_Setup_Completes)
	{
		event_info.handle = p_link->handle;
		event_info.status = status;
		if(p_pdu)
		{
			event_info.page = p_pdu[0];
			event_info.max_page = p_pdu[1];
			event_info.p_u_int8 = &(p_pdu[2]);
		}
		else
		{
			event_info.page = 0;
			event_info.max_page = 0;
			event_info.p_u_int8 = 0;
		}
		g_LM_config_info.lmp_event[_LM_READ_REMOTE_EXTENDED_FEATURES_COMPLETE_EVENT]
		(HCI_READ_REMOTE_EXTENDED_FEATURES_COMPLETE_EVENT, &event_info);
	}
#endif
}


/**************************************************************************
 *
 *  FUNCTION : _Send_HC_Remote_Host_Supported_Features_Notification_Event
 *
 *  DESCRIPTION :
 *  Send an HC_Remote_Host_Supported_Features_Notification_Event 
 * to the higher layers.
 *
 *************************************************************************/
void _Send_HC_Remote_Host_Supported_Features_Notification_Event(
    t_lmp_link *p_link, t_p_pdu p_pdu, t_error status)
{
    t_lm_event_info event_info;
           
    event_info.p_bd_addr = (t_bd_addr *)&p_link->bd_addr;
    event_info.status = status;
    if(p_pdu)
    {

        event_info.p_u_int8 = &(p_pdu[2]);
    }
    else
    {
        event_info.p_u_int8 = 0;
    }
    g_LM_config_info.lmp_event[_LM_REMOTE_HOST_SUPPORTED_FEATURES_NOTIFICATION_EVENT]
        (HCI_REMOTE_HOST_SUPPORTED_FEATURES_NOTIFICATION_EVENT, &event_info);
}

#if ((PRH_BS_CFG_SYS_HCI_V12_FUNCTIONALITY_SUPPORTED==1) && (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1))

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
/**********************************************************************************
 *
 * FUNCTION: LMconnection_LM_Setup_Synchronous_Connection
 *
 * DESCRIPTION:
 *  Invoked by the higher layers to add a SCO/eSCO connection to a a 
 *  given ACL link.
 *  Invoked by the higher layers to setup an eSCO connection.
 *
 *  Sends a request to the peer LM to add a SCO / eSCO connection with
 *  a particular packet type, etc.
 *
 **********************************************************************************/
t_error LMconnection_LM_Setup_Synchronous_Connection_New(t_lmp_link* p_link,
    u_int16 handle, u_int32 tx_bandwidth, u_int32 rx_bandwidth, 
    u_int16 max_latency, u_int16 voice_setting,
    u_int8 retransmission_effort, u_int16 esco_packet_types)
{
    t_error    status = NO_ERROR;
    t_sco_info *p_esco_link;
    t_esco_descr* p_esco_descr;

    /* 
     * Validate Admission  
     *
     * Can these parameters be accommodated by the overall scheduling
     * block.
     */
    status = LMscoctr_Validate_Admission(tx_bandwidth);

    /* 
     * Its a new eSCO link.
     * Allocate an eSCO link container object as a record
     * to assist managing the new eSCO link.
     */
    p_esco_link = LMscoctr_Alloc(
         p_link->device_index, 
         voice_setting, 
         ESCO_LINK);


    /*
     * Ooops, couldn't organise a link container object.
     * Maybe no free link containers, maybe no available
     * AM addresses. Cannot communicate with peer.
     * Report to host that resources are low.
     * MAX_NUM_CONNECTIONS is as close an error code
     * as there is available.
     */
    if(!p_esco_link)
        return MAX_NUM_CONNECTIONS;

    /*
     * Record the topical eSCO link so the other eSCO event handlers
     * can this link to move the signalling engine along.
     */
    LMscoctr_Set_Topical_eSCO_Link(p_esco_link, ESCO_SETUP);

    /*
     * Communicate with the LMP protocol peer by sending an
     * LMP_esco_link_req with the first set of proposed 
     * parameters.
     */
    p_link->current_proc_tid_role = p_link->role;

    p_esco_link->tx_bandwidth = tx_bandwidth;
    p_esco_link->rx_bandwidth = rx_bandwidth;
    p_esco_link->max_latency = max_latency;
    p_esco_link->retransmission_effort = retransmission_effort;
    p_esco_link->esco_packet_types = esco_packet_types;

    p_esco_descr = LMscoctr_Get_eSCO_Scratchpad();
    p_esco_descr->negotiation_state = ESCO_NGT_INIT;

    if(LMscoctr_Get_LMP_Parameter_Set(p_esco_descr))
    {
        p_esco_descr->voice_setting = voice_setting;

        /*
         * If master, need to set up d_esco and timing flag,
         * if slave, may as well propose the d_esco and timing
         * flag that would suit best...
         */
        LMscoctr_Set_D_eSCO_and_Timing_Flag(p_esco_descr);

        if(p_link->role == MASTER)
        {
            p_esco_descr->lm_sco_handle = p_esco_link->lm_sco_handle;
            p_esco_descr->esco_lt_addr = p_esco_link->esco_lt_addr;
        }
        else
        {
            p_esco_descr->lm_sco_handle = 0;
            p_esco_descr->esco_lt_addr = 0;
        }
        LMch_Send_LMP_eSCO_Link_Req(p_link, LMscoctr_Get_eSCO_Scratchpad());
    }
    else
    {
        status = INVALID_HCI_PARAMETERS;
    }

    return status;
}


/**************************************************************************
 *  FUNCTION : LMconnection_LM_Setup_Synchronous_Connection_Existing
 *
 *  DESCRIPTION : 
 *  This function handles the HCI Setup Synchronous Connection
 *  command if the command is changing an existing link.
 *
 *************************************************************************/
t_error LMconnection_LM_Setup_Synchronous_Connection_Existing(
    u_int16 handle, u_int32 tx_bandwidth, u_int32 rx_bandwidth, 
    u_int16 max_latency, u_int16 voice_setting,
    u_int8 retransmission_effort, u_int16 esco_packet_types)
{
    t_sco_info *p_esco_link;
    t_error    status = NO_ERROR;
    t_lmp_link* p_link;
    t_esco_descr* p_esco_descr;

    /* 
     * Validate Admission  
     *
     * Can these parameters be accommodated by the overall scheduling
     * block.
     */
    status = LMscoctr_Validate_Admission(tx_bandwidth);

    /*
     * Record the topical eSCO link so the other eSCO event handlers
     * can this link to move the signalling engine along.
     */
    p_esco_link = LMscoctr_Find_SCO_By_Connection_Handle(handle);
    if(!p_esco_link)
        return UNSPECIFIED_ERROR;

    p_link = LMaclctr_Find_Device_Index(p_esco_link->device_index);
    if(!p_link)
        return UNSPECIFIED_ERROR;

    LMscoctr_Set_Topical_eSCO_Link(p_esco_link, ESCO_RECONFIGURE);

	p_esco_link->tx_bandwidth = tx_bandwidth;
	p_esco_link->rx_bandwidth = rx_bandwidth;
	p_esco_link->max_latency = max_latency;
	p_esco_link->retransmission_effort = retransmission_effort;
	p_esco_link->esco_packet_types =  esco_packet_types;

    /*
     * Communicate with the LMP protocol peer by sending an
     * LMP_esco_link_req with the first set of proposed 
     * parameters.
     */
    p_link->current_proc_tid_role = p_link->role;

    p_esco_descr = LMscoctr_Get_eSCO_Scratchpad();	
	p_esco_descr->negotiation_state = ESCO_NGT_INIT;

	if(LMscoctr_Get_LMP_Parameter_Set(p_esco_descr))
    {
        p_esco_descr->voice_setting = voice_setting;
        LMscoctr_Set_D_eSCO_and_Timing_Flag(p_esco_descr);
        p_esco_descr->lm_sco_handle = p_esco_link->lm_sco_handle;
        p_esco_descr->esco_lt_addr = p_esco_link->esco_lt_addr;

        LMch_Send_LMP_eSCO_Link_Req(p_link, LMscoctr_Get_eSCO_Scratchpad());
    }

    return status;
}
#endif

/**************************************************************************
 *  
 *  FUNCTION : LMconnection_LM_Setup_Synchronous_Connection
 *
 *  DESCRIPTION :
 *   Invoked by the higher layers to add/change a SCO/eSCO connection 
 *   to a given ACL link or eSCO link.
 *   Sends the request to the peer LM to add a SCO connection with a   
 *   particular packet type (HV1-3)
 *   Or sends a request to the peer LM to add/change an eSCO connection
 *   with a particular set or parameters (d,tcf, t,w,pkt_type,pkt_len)
 *   that "fit" within the host constraints.
 *
 *************************************************************************/
t_error LMconnection_LM_Setup_Synchronous_Connection(
    u_int16 handle, u_int32 tx_bandwidth, u_int32 rx_bandwidth,
    u_int16 max_latency, u_int16 voice_setting, 
    u_int8 retransmission_effort, u_int16 packet_types)
{
    t_error    status = NO_ERROR;
    t_lmp_link* p_link = LMaclctr_Find_Handle(handle);

#if 1 //(BUILD_TYPE!=UNIT_TEST_BUILD)
    if (g_LM_config_info.default_sco_route_via_hci)
		voice_setting  |= PRH_BS_HCI_VOICE_SETTING_FOR_SCO_VIA_HCI;
#endif

    if(!Is_HCI_SYN_Packet_Type(packet_types))
    {
         status = INVALID_HCI_PARAMETERS;
    }
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    else if(Is_HCI_eSCO_Packet_Type(packet_types))
    {
     status = _Cancel_Pending_Page_Inquiry();
		/* 
         * Validate Host Parameters 
         */
        status = LMscoctr_Validate_Host_eSCO_Parameters(handle, tx_bandwidth, 
             rx_bandwidth, max_latency, voice_setting, retransmission_effort, 
             &packet_types);

        /*
         * If the parameters fail through LMscoctr_Validate_Host_eSCO_Parameters,
         * the HCI parameters cannot be presented to the peer. Inform the
         * host that there is an issue with the host parameters.
         */
        if(status != NO_ERROR)
            return status;

        /* 
         * Manage transaction collisions.
         */
        status = LMscoctr_Detect_Transaction_Collisions();

        /*
         * Does a new eSCO link container object need to be
         * generated?
         *
         *  If the handle is in the ACL handle-map
         *    allocate a new esco container.
         */
         if(p_link)
         {
             status = LMconnection_LM_Setup_Synchronous_Connection_New(p_link,
                 handle, tx_bandwidth, rx_bandwidth, max_latency, 
                 voice_setting, retransmission_effort, packet_types);
         }
         else
         {
            status = LMconnection_LM_Setup_Synchronous_Connection_Existing(handle,
                tx_bandwidth, rx_bandwidth, max_latency, voice_setting,
                retransmission_effort, packet_types);
         }
    }
#endif
    else
    {
        /*
         * Handle scenarios where eSCO packet types are not supported
         * and/or where eSCO packet types are not specified.
         */
        if(!p_link)
        {
            status = NO_CONNECTION;
        }
        else if(!Is_HCI_SCO_Packet_Type(packet_types))
        {
            status = UNSUPPORTED_PARAMETER_VALUE;
        }
        else if((tx_bandwidth != 8000) && (tx_bandwidth != 0xFFFFFFFF))
        {
            status = UNSUPPORTED_PARAMETER_VALUE;
        }
        else if((rx_bandwidth != 8000) && (rx_bandwidth != 0xFFFFFFFF))
        {
            status = UNSUPPORTED_PARAMETER_VALUE;
        }
        else if(max_latency <= 3)
        {
            status = INVALID_HCI_PARAMETERS;
        }
        else if((retransmission_effort != 0x00) && (retransmission_effort != 0xff))
        {
            status = UNSUPPORTED_PARAMETER_VALUE;
        }
        else if(!LC_Is_Valid_SCO_Conversion(voice_setting))
        {
            status = INVALID_HCI_PARAMETERS;
        }
        else
        {
            /*
             *
             * Change packet_types format to that required for deprecated command:
             * HV1(0x0001)->0x0020
             * HV2(0x0002)->0x0040
             * HV3(0x0003)->0x0080
             */
            packet_types = ((packet_types & (HCI_SYN_HV1|HCI_SYN_HV2|HCI_SYN_HV3)) << 5);   

            status = LMconnection_LM_Add_SCO_Connection(p_link, packet_types, voice_setting);
        }
    }

#if ((PRH_BS_CFG_SYS_HCI_V12_FUNCTIONALITY_SUPPORTED==1) && (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1))
    _LMch_host_v12_functionality_confirmed = TRUE;
#endif

    return status;
}


/*************************************************************************
 *
 * FUNCTION : LMconnection_LM_Reject_Synchronous_Connection_Request
 *
 * DESCRIPTION :
 *  This function handles a HCI_Reject_Synchronous_Connection_Request
 *  from the host.
 *
 ************************************************************************/
t_error LMconnection_LM_Reject_Synchronous_Connection_Request(
    u_int8* p_bdaddr_bytes, t_error reject_reason)
{
    t_lmp_link* p_link = 0;
    t_bd_addr bdaddr;
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    t_sco_info* p_esco_link = 0;
	t_esco_descr * p_esco_descr = 0;
#endif

#if ((PRH_BS_CFG_SYS_HCI_V12_FUNCTIONALITY_SUPPORTED==1) && (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1))
    /*
     * Host has issued a new HCI synchronous
     * connection primitive.
     */
    _LMch_host_v12_functionality_confirmed = TRUE;
#endif

    /*
     * Turn off connection timeout handler (if its running)
     */

    HCIparam_Get_Bd_Addr(p_bdaddr_bytes, &bdaddr);
    p_link = LMaclctr_Find_Bd_Addr(&bdaddr);
    if(!p_link)
    {
        return LMconnection_Cleanup_Reject_Synchronous_Connection_Request(0, 0, 
            NO_CONNECTION, reject_reason);
    }
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    else if(W4_HOST_ESCO_ACCEPT & p_link->state)
    {
        LMtmr_Clear_Timer(g_LM_config_info.accept_timeout_index);
        g_LM_config_info.accept_timeout_index = 0;
        p_link->state -= W4_HOST_ESCO_ACCEPT;
    }
#endif
    else if(W4_HOST_SCO_ACCEPT & p_link->state)
    {
        /*
         * If host has used new primitive to reject a SCO
         * use old SCO routine to execute command and
         * return.
         */
        HCeg_Command_Status_Event(NO_ERROR, HCI_REJECT_SYNCHRONOUS_CONNECTION_REQUEST);
        return LMconnection_LM_Connection_Reject(&bdaddr, reject_reason);
    }


#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    /*
     * Validate parameters
     */

    /*
     * Lookup eSCO link.
     */
    p_esco_link = LMscoctr_Get_Topical_eSCO_Link();
    if(!p_esco_link)
    {
        return LMconnection_Cleanup_Reject_Synchronous_Connection_Request(
            p_link, 0, UNSPECIFIED_ERROR, reject_reason);
    }

    /*
     * Tell the peer that the connection is being rejected.
     */
    LM_Encode_LMP_Not_Accepted_Ext_PDU(p_link, LMP_ESCO_LINK_REQ, (t_error)reject_reason);

   	/*
	 * If host timeout then this function was called due to a timeout and not the reject sync connection.
	 */
	if (reject_reason != CONNECTION_ACCEPT_TIMEOUT)
	{	
		HCeg_Command_Status_Event(NO_ERROR, HCI_REJECT_SYNCHRONOUS_CONNECTION_REQUEST);
	}

	/*
     * Tidy up locally.
     */
	p_esco_descr = LMscoctr_Get_eSCO_Scratchpad();
	if (p_esco_link->tx_packet_type == NULLpkt)
	{
		p_esco_link->t_sco = p_esco_descr->t_esco;
		p_esco_link->w_esco = p_esco_descr->w_esco;
		p_esco_link->tx_packet_length = p_esco_descr->tx_pkt_len;
		p_esco_link->rx_packet_length = p_esco_descr->rx_pkt_len;
		p_esco_link->lm_air_mode = LMscoctr_Map_AirMode_HCI2LMP((u_int8)(p_esco_descr->voice_setting & 3));
	}
    _Finalise_Synchronous_Connection_Setup(p_link, p_esco_link, (t_error)reject_reason);
#endif

    return NO_ERROR;
}

/**************************************************************************
 *  
 *  FUNCTION : LMconnection_Cleanup_Reject_Synchronous_Connection_Request
 *
 *  DESCRIPTION : Tidies up from a HCI Reject Synchronous Connection
 *   Request with invalid parameters or sent in the wrong state, etc.
 *   
 *
 *************************************************************************/
t_error LMconnection_Cleanup_Reject_Synchronous_Connection_Request(t_lmp_link* p_link,
    t_sco_info* p_esco_link, t_error status, t_error reject_reason)
{
    HCeg_Command_Status_Event(status,HCI_REJECT_SYNCHRONOUS_CONNECTION_REQUEST);

    /*
     * Tidy up locally.
     */
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    if(status == NO_ERROR)
    {
        _Finalise_Synchronous_Connection_Setup(p_link, p_esco_link, reject_reason);
        return status;
    }
	else
	{
		if ( p_esco_link !=0)
		{
			_Send_HC_Synchronous_Connection_Complete_Event(p_link,
				p_esco_link, status);
		}
		LMscoctr_Set_Topical_eSCO_Link(0, (t_esco_transaction_type)0);
		
		LMscoctr_Clean_eSCO_Scratchpad();
	}
#endif

    return NO_ERROR;
}


/**************************************************************************
 *  
 *  FUNCTION : LMconnection_LM_AutoAccept_Default_eSCO_Link
 *
 *  DESCRIPTION : Handles incoming eSCO connections in auto-accept mode.
 *   
 *
 *************************************************************************/
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
t_error LMconnection_LM_AutoAccept_Default_eSCO_Link(t_lmp_link* p_link)
{
    t_sco_info *p_esco_link = LMscoctr_Get_Topical_eSCO_Link();
    t_esco_descr * p_esco_descr = LMscoctr_Get_eSCO_Scratchpad();

    if (!p_esco_link || !p_esco_descr)
        return UNSPECIFIED_ERROR;

    p_esco_link->tx_bandwidth = LMscoctr_Calculate_Bandwidth(p_esco_descr);
    p_esco_link->rx_bandwidth = LMscoctr_Calculate_Bandwidth(p_esco_descr);

    p_esco_link->max_latency = 0xffff;

    p_esco_link->retransmission_effort = (p_esco_descr->w_esco)/
        (is_eSCO_Packet_3_Slot(p_esco_descr->rx_pkt_type)?(6):(2));
    if (p_esco_link->retransmission_effort > 2) p_esco_link->retransmission_effort = 2;

    p_esco_link->esco_packet_types = p_esco_descr->tx_pkt_type;

	// BUG HERE !! The p_esco_descr only contains the air_mode
	// Thus original code changed the complete voice settings.
	// Changing input sample size to always 8 bit.

    p_esco_link->voice_setting &= ~3;
	p_esco_link->voice_setting |= (p_esco_descr->voice_setting&0x3);

	if (g_LM_config_info.default_sco_route_via_hci)
		p_esco_link->voice_setting |= 0x8000;

    p_esco_descr->esco_lt_addr = p_esco_link->esco_lt_addr;
    p_esco_descr->lm_sco_handle = p_esco_link->lm_sco_handle;

    if(LMscoctr_Is_D_eSCO_Acceptable(p_esco_descr, p_link))
    {
        if(SLAVE == p_link->role)
        {
            LM_Encode_LMP_Accepted_Ext_PDU(p_link, LMP_ESCO_LINK_REQ);
        }
        else
        {
            p_esco_descr->negotiation_state = ESCO_NGT_PREF;
            LMscoctr_Set_D_eSCO_and_Timing_Flag(p_esco_descr);
            LMch_Send_LMP_eSCO_Link_Req(p_link, p_esco_descr);
        }
    }
    else
    {
        LM_Encode_LMP_Not_Accepted_Ext_PDU(p_link, LMP_ESCO_LINK_REQ, UNSPECIFIED_ERROR);
        _Finalise_Synchronous_Connection_Setup(p_link, 
            p_esco_link, SCO_INTERVAL_REJECTED);
    }
	return NO_ERROR;
}
#endif


/**************************************************************************
 *  
 *  FUNCTION : LMconnection_LM_Accept_Synchronous_Connection_Request
 *
 *  DESCRIPTION :
 *   This function handles a HCI_Accept_Synchronous_Connection_Request
 *   from the host.
 *
 *
 *************************************************************************/
t_error LMconnection_LM_Accept_Synchronous_Connection_Request(
    u_int8 *p_bdaddr_bytes, u_int32 tx_bandwidth, u_int32 rx_bandwidth,
    u_int16 max_latency, u_int16 voice_setting, 
    u_int8 retransmission_effort, u_int16 packet_types)
{
    t_error    status = NO_ERROR;
    t_lmp_link* p_link = 0;
    t_bd_addr bdaddr;
	u_int32 tx_bw = tx_bandwidth,
		    rx_bw = rx_bandwidth;
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
	t_esco_descr *p_esco_descr;
    p_esco_descr = LMscoctr_Get_eSCO_Scratchpad();
#if (PRH_BS_CFG_SYS_HCI_V12_FUNCTIONALITY_SUPPORTED==1)
    _LMch_host_v12_functionality_confirmed = TRUE;
#endif
#endif

    /*
     * Turn off connection timeout handler
     */
    HCIparam_Get_Bd_Addr(p_bdaddr_bytes, &bdaddr);
    p_link = LMaclctr_Find_Bd_Addr(&bdaddr);

#if 1 // GF 2 Sept (BUILD_TYPE!=UNIT_TEST_BUILD)
    if (g_LM_config_info.default_sco_route_via_hci)
		voice_setting  |= PRH_BS_HCI_VOICE_SETTING_FOR_SCO_VIA_HCI;
#endif

    if(!p_link)
    {
        status = NO_CONNECTION;
    }
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    else if(W4_HOST_ESCO_ACCEPT & p_link->state)
    {
         /* 
         * Validate Parameters 
         */
        if(!p_esco_descr)
        {
            return UNSPECIFIED_ERROR;
        }
        if(tx_bandwidth == ESCO_DONT_CARE_BANDWIDTH)
            tx_bw = LMscoctr_Calculate_Bandwidth(p_esco_descr);
        if(rx_bandwidth == ESCO_DONT_CARE_BANDWIDTH)
            rx_bw = LMscoctr_Calculate_Bandwidth(p_esco_descr);

        status = LMscoctr_Validate_Host_eSCO_Parameters(p_link->handle, tx_bw,
			rx_bw, max_latency, voice_setting, retransmission_effort, &packet_types);

        /*
         * If the parameters fail through LMscoctr_Validate_Host_eSCO_Parameters,
         * the HCI parameters cannot be presented to the peer. Inform the
         * host that there is an issue with the host parameters.
         */
        if(status == NO_ERROR)
        {
			/*
			 * Only clear the timer if the accept is ok.
			 */
			LMtmr_Clear_Timer(g_LM_config_info.accept_timeout_index);
            g_LM_config_info.accept_timeout_index = 0;
			p_link->state -= W4_HOST_ESCO_ACCEPT;

			status= LMconnection_LM_Accept_Synchronous_Connection_Request_Common(
				p_link, tx_bandwidth, rx_bandwidth, max_latency, voice_setting,
				retransmission_effort, packet_types);
            HCeg_Command_Status_Event(status, HCI_ACCEPT_SYNCHRONOUS_CONNECTION_REQUEST);
        }
    }
#endif
    else if(p_link->state & W4_HOST_SCO_ACCEPT)
    {
        if(!Is_HCI_SCO_Packet_Type(packet_types))
        {
            status = INVALID_HCI_PARAMETERS;
        }
        else if((tx_bandwidth != 8000) && (tx_bandwidth != 0xFFFFFFFF))
        {
            status = INVALID_HCI_PARAMETERS;
        }
        else if(max_latency <= 3)
        {
            status = INVALID_HCI_PARAMETERS;
        }
        else if(!LC_Is_Valid_SCO_Conversion(voice_setting))
        {
            status = UNSUPPORTED_PARAMETER_VALUE;
        }
        else
        {
            t_sco_info* p_sco_link = LMscoctr_Get_SCO_Transitioning_Ref();
        
        /*
         * Match lm_sco_packet with packet_types bit field
         * HV1 (0)<->(0x0001)
         * HV2 (1)<->(0x0002)
         * HV3 (2)<->(0x0004)
         */ 
        u_int16 packet_types_match = packet_types & (1<<(p_sco_link->lm_sco_packet));
        if ((packet_types_match) || (MASTER == p_link->role))
        {
            if (!packet_types_match)
        {
            /*
             * If the master cannot use the same parameters as in the slave
             * request, it is allowed to use other values. Convert packet_types
             * to BT1.1 format and reuse LMscoctr_Derive_SCO_Parameters.
             * HV1 (0x0001)->0x0020
             * HV2 (0x0002)->0x0040
             * HV3 (0x0004)->0x0080
             */
            packet_types = ((packet_types & (HCI_SYN_HV1|HCI_SYN_HV2|HCI_SYN_HV3)) << 5);
            LMscoctr_Derive_SCO_Parameters(p_sco_link, packet_types, p_link->role, 0 /* use local air-mode flags */);
        }
        
                HCeg_Command_Status_Event(status, HCI_ACCEPT_SYNCHRONOUS_CONNECTION_REQUEST);
                LMconnection_LM_Connection_Accept(&bdaddr, p_link->role, voice_setting);
        }
        else
        {
            /*
         * If the slave does not accept the SCO link, but is willing to
         * consider another possible set of SCO parameters, it can indicate
         * what it does not accept in LMP_not_accepted error reason field.
         * The master then has the possibility to issue a new request with
         * the modified parameters.
         */
        LMconnection_LM_Connection_Reject(&bdaddr,SCO_INTERVAL_REJECTED);
        status = UNSUPPORTED_PARAMETER_VALUE;
        }
        }
    }
    else
    {
        return COMMAND_DISALLOWED;
    }

    return status;
}


/**************************************************************************
 *  
 *  FUNCTION : LMconnection_LM_Accept_Synchronous_Connection_Request_Common
 *
 *  DESCRIPTION : Common handler for explicitly
 *   accepted incoming eSCO connections. Determines the host constraints
 *   (e.g. bandwidth, latency, retransmission mode, packet types),
 *   and derives a set of LMP parameters that meet these constraints.
 *   Either rejects, accepts, or negotiates with the peer:
 *    If no set of parameters available --> rejects peer.
 *    If peer proposal meets constraints -> accepts peer.
 *    If peer proposal does not meet constraints AND
 *    there are parameters available    --> negotiate.
 *   
 *
 *************************************************************************/
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
t_error LMconnection_LM_Accept_Synchronous_Connection_Request_Common(t_lmp_link* p_link,
    u_int32 tx_bandwidth, u_int32 rx_bandwidth,
    u_int16 max_latency, u_int16 voice_setting, 
    u_int8 retransmission_effort, u_int16 esco_packet_types)
{
    t_sco_info *p_esco_link = 0;
    t_esco_descr * p_esco_descr = 0;
    t_error    status = NO_ERROR;
    u_int16 latency_in_us;

    /* 
     * Validate Admission  
     *
     * Can these parameters be accomodated
     */
    if(tx_bandwidth != ESCO_DONT_CARE_BANDWIDTH)
        status = LMscoctr_Validate_Admission(tx_bandwidth);

    /* 
     * Manage transaction collisions.
     */
    status = LMscoctr_Detect_Transaction_Collisions();

    /*
     * Lookup eSCO link.
     */
    p_esco_link = LMscoctr_Get_Topical_eSCO_Link();
    if(!p_esco_link)
    {
        return UNSPECIFIED_ERROR;
    }

    /*
     * Look up scratch pad.
     */
     p_esco_descr = LMscoctr_Get_eSCO_Scratchpad();
     if(!p_esco_descr)
     {
         return UNSPECIFIED_ERROR;
     }

    /*
     * Depending on local device's role, either (a)
     *  - send LMP_accepted_ext to the peer device.
     *  - send LMP_eSCO_link_req to the peer device.
     */
    if(tx_bandwidth == ESCO_DONT_CARE_BANDWIDTH)
        tx_bandwidth = LMscoctr_Calculate_Bandwidth(p_esco_descr);
    if(rx_bandwidth == ESCO_DONT_CARE_BANDWIDTH)
        rx_bandwidth = LMscoctr_Calculate_Bandwidth(p_esco_descr);

	p_esco_link->tx_bandwidth = tx_bandwidth;
    p_esco_link->rx_bandwidth = rx_bandwidth;
    p_esco_link->max_latency = max_latency;
    p_esco_link->retransmission_effort = retransmission_effort;
    p_esco_link->esco_packet_types = esco_packet_types;

    /*
     * Recalculate voice setting, air mode, and
     * sco cfg after host has explicitly accepted
     * incoming connection. Use the air mode proposed
     * by the peer.
     */
    p_esco_link->voice_setting = voice_setting;

    p_esco_descr->esco_lt_addr = p_esco_link->esco_lt_addr;
    p_esco_descr->lm_sco_handle = p_esco_link->lm_sco_handle;

    /*
     * If the parameters are fine, accept them.
     */
    if(LMscoctr_Is_An_LMP_Parameter_Set(p_esco_descr->t_esco,
        p_esco_descr->w_esco, p_esco_descr->tx_pkt_type,
        p_esco_descr->tx_pkt_len) &&
      (TRUE==LMscoctr_Is_D_eSCO_Acceptable(p_esco_descr, p_link)))
    {
        if(SLAVE == p_link->role)
        {
            LM_Encode_LMP_Accepted_Ext_PDU(p_link, LMP_ESCO_LINK_REQ);
        }
        else
        {
            p_esco_descr->esco_lt_addr = p_esco_link->esco_lt_addr;
            p_esco_descr->lm_sco_handle = p_esco_link->lm_sco_handle;
            p_esco_descr->negotiation_state = ESCO_NGT_PREF;
            LMscoctr_Set_D_eSCO_and_Timing_Flag(p_esco_descr );
            LMch_Send_LMP_eSCO_Link_Req(p_link, p_esco_descr );
        }
    }
    /*
     * else, if the proposed d value is impossible, but
     * another value is acceptable, propose the new value.
     */
    else if((!LMscoctr_Is_D_eSCO_Acceptable(p_esco_descr, p_link)) && 
        (LMscoctr_Is_An_LMP_Parameter_Set(p_esco_descr->t_esco,
        p_esco_descr->w_esco, p_esco_descr->tx_pkt_type,
        p_esco_descr->tx_pkt_len)))
    {
        LMscoctr_Set_D_eSCO_and_Timing_Flag(p_esco_descr);
        p_esco_descr->negotiation_state = ESCO_NGT_RS_VIOL;
        LMch_Send_LMP_eSCO_Link_Req(p_link, p_esco_descr );
    }
    else
    {
		u_int8 next_negotiation_state;

        latency_in_us = LMscoctr_Calculate_Latency_In_uSec(
            p_esco_descr->t_esco, p_esco_descr->w_esco,
            p_esco_descr->tx_pkt_type, p_esco_descr->rx_pkt_type);

        if(latency_in_us > (p_esco_link->max_latency*1000))
        {
            next_negotiation_state = ESCO_NGT_LAT_VIOL;
        }
        else
        {
            next_negotiation_state = ESCO_NGT_UNSUPPORTED_FEATURE;
        }

        if(LMscoctr_Get_LMP_Parameter_Set(p_esco_descr))
        {
			p_esco_descr->negotiation_state = next_negotiation_state;
            LMscoctr_Set_D_eSCO_and_Timing_Flag(p_esco_descr);
            LMch_Send_LMP_eSCO_Link_Req(p_link, p_esco_descr );
        }
        else
        {
            LM_Encode_LMP_Not_Accepted_Ext_PDU(p_link, LMP_ESCO_LINK_REQ, UNSPECIFIED_ERROR);
            _Finalise_Synchronous_Connection_Setup(p_link, 
                p_esco_link, SCO_INTERVAL_REJECTED);
        }
    }

    return status;
}


/**************************************************************************
 *  FUNCTION : _Send_HC_Synchronous_Connection_Complete_Event
 *
 *  DESCRIPTION :
 *  Send an HC_Synchronous_Connection_Complete event to the higher layer.
 *
 *  p_link          Reference to the ACL link associated with this SCO.
 *  sco_handle      The handle to return on the event.
 *  status          The status to return on the event.
 *************************************************************************/
void _Send_HC_Synchronous_Connection_Complete_Event(
    const t_lmp_link *p_link, const t_sco_info *p_esco_link, t_error status)
{
    t_lm_event_info event_info;
   
    /* 
     * Construct the parameters for the event 
     */
    event_info.status = status;
    event_info.handle = p_esco_link->connection_handle;
    event_info.p_bd_addr = (t_bd_addr *)&p_link->bd_addr;
    event_info.link_type = ESCO_LINK;

    event_info.transmission_interval = p_esco_link->t_sco;
    event_info.retransmission_window = p_esco_link->w_esco;
    event_info.rx_packet_length = p_esco_link->rx_packet_length;
    event_info.tx_packet_length = p_esco_link->tx_packet_length;
    event_info.air_mode = p_esco_link->lm_air_mode;
    event_info.ack_required = FALSE;

    /* 
     * Raise a connection complete event to the higher layers
     */
    g_LM_config_info.lmp_event[_LM_CONNECT_COMPLETE]
        (HCI_SYNCHRONOUS_CONNECTION_COMPLETE_EVENT, &event_info);
}


/**************************************************************************
 *  
 *  FUNCTION : _Finalise_Synchronous_Connection_Setup
 *
 *  DESCRIPTION :
 *   Routine to finalise synchronous connection setup after
 *   negotiation phase has ended.
 *
 *************************************************************************/
void _Finalise_Synchronous_Connection_Setup(t_lmp_link *p_link,
    t_sco_info *p_esco_link, t_error status)
{
    if(status == NO_ERROR)
    {
        LMscoctr_Commit_to_New_Synchronous_Parameters(p_esco_link);

        DL_Set_eSCO_LT_Address(DL_Get_Device_Ref(p_link->device_index), p_esco_link->esco_lt_index,p_esco_link->esco_lt_addr);

        /*
         * If this development feature is enabled, then
         * always OR in the SCO_VIA_HCI bit.
         */
#if (1==PRH_BS_DEV_FORCE_ESCO_VIA_ACL_REGISTERS)
        {
        extern u_int16 _HWcodec_voice_setting;
        _HWcodec_voice_setting |= PRH_BS_HCI_VOICE_SETTING_FOR_SCO_VIA_HCI;
        }
#endif

        /*
         * Setup the SCO_CFG cache registers
         */
        LMpol_Init_SCO(p_link, p_esco_link->sco_index);

        if(p_link->role == MASTER)
            p_esco_link->state = eSCO_MASTER_ACTIVATION_PENDING;
        else
            p_esco_link->state = eSCO_SLAVE_ACTIVATION_PENDING;

#if (PRH_BS_CFG_SYS_PERSISTENT_SCO_IN_SCANS_SUPPORTED==1)
        /* 
         * Disallow scans if SCO connection as slave.
         */
        if(p_link->role == SLAVE)
        {
            LMconfig_LM_Set_Device_State(PRH_DEV_STATE_SCO_PREVENTING_SCANS);    
        }
#endif

        LMpol_Adjust_ACL_Packet_Types_On_All_Links(LMcontxt_ENTERING_SCO);

//        p_esco_link->state = SCO_ACTIVATION_PENDING;
    }
    else
    {
        LMscoctr_Free((t_sco_info* )p_esco_link);
    }

    _Send_HC_Synchronous_Connection_Complete_Event(p_link,
        p_esco_link, status);

    /*
     * No more topical eSCO link.
     */
    LMscoctr_Set_Topical_eSCO_Link(0, (t_esco_transaction_type)0);

    /*
     * Clean up the Scratchpad
     */
    LMscoctr_Clean_eSCO_Scratchpad();
}


/**************************************************************************
 *  
 *  FUNCTION : _Finalise_Synchronous_Connection_Change
 *
 *  DESCRIPTION :
 *   Routine to finalise synchronous connection change after
 *   negotiation phase has ended.
 *
 *************************************************************************/
void _Finalise_Synchronous_Connection_Change(
    const t_lmp_link *p_link, t_sco_info *p_esco_link, t_error status)
{
    /*
     * If sucessfully concluded renegotiation with peer, set
     * the renegotiated_params_available flag, the policy
     * block checks this regularly. This mechanism ensures
     * an ordered transition to the new parameters.
     */
    if(status == NO_ERROR)
    {
        LMscoctr_Commit_to_New_Synchronous_Parameters(p_esco_link);
        p_esco_link->renegotiated_params_available = 1;

		LMpol_Adjust_ACL_Packet_Types_On_All_Links(LMcontxt_CHANGED_SCO);
    }

    p_esco_link->negotiation_state = 0;

    _Send_HC_Synchronous_Connection_Changed_Event(p_link,
        p_esco_link, status);

    /*
     * Close the topical eSCO link.
     */
    LMscoctr_Set_Topical_eSCO_Link(0, (t_esco_transaction_type)0);

    /*
     * Clean up the Scratchpad
     */
    LMscoctr_Clean_eSCO_Scratchpad();
}


/**************************************************************************
 *  FUNCTION : _Send_HC_Synchronous_Connection_Changed_Event
 *
 *  DESCRIPTION :
 *  Send an HC_Synchronous_Connection_Changed event to the higher layer.
 *
 *  p_link          Reference to the ACL link associated with this eSCO.
 *  p_esco_link     Reference to the eSCO link.
 *  status          The status to return on the event.
 *************************************************************************/
void _Send_HC_Synchronous_Connection_Changed_Event(
    const t_lmp_link *p_link, const t_sco_info *p_esco_link, t_error status)
{
    t_lm_event_info event_info;
   
    /* 
     * Construct the parameters for the event 
     */
    event_info.status = status;
    event_info.handle = p_esco_link->connection_handle;
    event_info.p_bd_addr = (t_bd_addr *)&p_link->bd_addr;
    event_info.link_type = ESCO_LINK;

    /*
     * If good status, report the new parameters, else
     * report the old parameters.
     */
    event_info.transmission_interval = p_esco_link->t_sco;
    event_info.retransmission_window = p_esco_link->w_esco;
    event_info.rx_packet_length = p_esco_link->rx_packet_length;
    event_info.tx_packet_length = p_esco_link->tx_packet_length;

    event_info.air_mode = p_esco_link->lm_air_mode;
    event_info.ack_required = FALSE;

    /* 
     * Raise a connection complete event to the higher layers
     */
    g_LM_config_info.lmp_event[_LM_CONNECTION_PACKET_TYPE_CHANGED_EVENT]
        (HCI_SYNCHRONOUS_CONNECTION_CHANGED_EVENT, &event_info);

    /*
     * No more topical eSCO link.
     */
    LMscoctr_Set_Topical_eSCO_Link(0, (t_esco_transaction_type)0);
}
#endif

#endif

u_int8 _LMch_LMP_features_req_ext_PDU[12];
u_int8 _LMch_LMP_features_res_ext_PDU[12];

/**************************************************************************
 *
 * FUNCTION :- _Send_LMP_Features_Req_Ext
 *
 * DESCRIPTION
 * This function is responsible for encoding LMP_FEATURES_REQ_EXT
 *
 *************************************************************************/
static void _Send_LMP_Features_Req_Ext(t_lmp_link *p_link, u_int8 page)
{
    t_lmp_pdu_info pdu_info;

    _LMch_LMP_features_req_ext_PDU[0] = (127<<1) + p_link->current_proc_tid_role;
    _LMch_LMP_features_req_ext_PDU[1] = (LMP_FEATURES_REQ_EXT & 0xff);
    _LMch_LMP_features_req_ext_PDU[2] = page;
    _LMch_LMP_features_req_ext_PDU[3] = NUM_FEATURES_PAGES-1; /* max page */

    if(page == 0)
    {
        LMutils_Array_Copy(8, &(_LMch_LMP_features_req_ext_PDU[4]), SYSconfig_Get_LMP_Features_Ref());
    }
    else if(page < NUM_FEATURES_PAGES)
    {
		u_int8 i;

		for (i=0;i<8;i++)
			_LMch_LMP_features_req_ext_PDU[4+i] = 0;
#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
		if (g_LM_config_info.ssp_enabled)
			_LMch_LMP_features_req_ext_PDU[4] |= 1;
#endif
#if (PRH_BS_CFG_SYS_LE_CONTROLLER_SUPPORTED==1)
		if (g_LM_config_info.le_host_supported)
			_LMch_LMP_features_req_ext_PDU[4] |= 2;
		if (g_LM_config_info.le_and_bredr_host)
			_LMch_LMP_features_req_ext_PDU[4] |= 4;
#endif
    }
    else
    {
        hw_memset(&(_LMch_LMP_features_req_ext_PDU[4]), 0, 8);
    }

    /* 
     * Explicitly add - No ack required 
     */
    pdu_info.ack_required = 0x00;
    LM_Encode_LMP_PDU_Fully_Encoded(p_link->device_index,
        LMP_FEATURES_REQ_EXT, _LMch_LMP_features_req_ext_PDU);
}


/**************************************************************************
 *
 * FUNCTION :- _Send_LMP_Features_Res_Ext
 *
 * DESCRIPTION
 * This function is responsible for encoding LMP_FEATURES_REQ_EXT
 *
 *************************************************************************/
static void _Send_LMP_Features_Res_Ext(t_lmp_link *p_link, u_int8 page)
{
    _LMch_LMP_features_res_ext_PDU[0] = (127<<1) + p_link->current_proc_tid_role;
    _LMch_LMP_features_res_ext_PDU[1] = (LMP_FEATURES_RES_EXT & 0xff);
    _LMch_LMP_features_res_ext_PDU[2] = page;
    _LMch_LMP_features_res_ext_PDU[3] = NUM_FEATURES_PAGES-1; /* max page */

    if(page == 0)
    {
        LMutils_Array_Copy(8, &(_LMch_LMP_features_res_ext_PDU[4]), SYSconfig_Get_LMP_Features_Ref());
    }
    else if(page == 0x01)
    {
		u_int8 i;

		for (i=0;i<8;i++)
			_LMch_LMP_features_res_ext_PDU[4+i] = 0;
#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
		if (g_LM_config_info.ssp_enabled)
			_LMch_LMP_features_res_ext_PDU[4] = 1;
#endif
       // LMutils_Array_Copy(8, &(_LMch_LMP_features_res_ext_PDU[4]), SYSconfig_Get_LMP_Extended_Features_Ref(page));
    }
    else
    {
        hw_memset(&(_LMch_LMP_features_res_ext_PDU[4]), 0, 8);
    }

    LM_Encode_LMP_PDU_Fully_Encoded(p_link->device_index,
        LMP_FEATURES_RES_EXT, _LMch_LMP_features_res_ext_PDU);

}


/**************************************************************************
 *
 * FUNCTION :- LMch_Send_LMP_Packet_Type_Table_Req
 *
 * DESCRIPTION
 * This function is responsible for encoding LMP_PACKET_TYPE_TABLE_REQ.
 * ACL logical transports EDR packet types are explicitly selected via LMP
 * using the packet_type_table (ptt) parameter. Before changing the packet
 * type table, the initiator shall finalize the transmission of the current
 * ACL packet with ACL-U information and shall stop ACL-U transmissions
 * and then send the LMP_packet_type_table_req PDU. 
 *
 * Note: It is expected this will only be called when ptt != p_link->ptt.
 *
 *************************************************************************/
static u_int8 _LMch_LMP_packet_type_table_req_PDU[3];
void LMch_Send_LMP_Packet_Type_Table_Req(t_lmp_link *p_link, u_int8 ptt)
{
    _LMch_LMP_packet_type_table_req_PDU[0] = (127<<1) + p_link->role;
    _LMch_LMP_packet_type_table_req_PDU[1] = (LMP_PACKET_TYPE_TABLE_REQ & 0xff);
    _LMch_LMP_packet_type_table_req_PDU[2] = ptt;
    
    LMch_Disable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_PTT);

    p_link->ptt_req = ptt;

    LM_Encode_LMP_PDU_Fully_Encoded(p_link->device_index,
        LMP_PACKET_TYPE_TABLE_REQ, _LMch_LMP_packet_type_table_req_PDU);
}


t_error _Cancel_Pending_Page_Inquiry(void)
{
	t_error status = NO_ERROR;
	/*
	 * #2789
	 * Cancel Inquiry procedure (that MAY be currently active).
	 * 
	 * Note: 
	 * Can't call LMinq_Inquiry_Cancel() as this will not generate the 
	 * required Inquiry_Complete_Event().  LMinq_Inquiry_Cancel() is currently
	 * only used if a HCI_Inquiry_Cancel() is called.
	 * Calling  LC_Inquiry_Cancel() will invoke all the required functionality
	 * in the LC and LM.
	 */
	LC_Inquiry_Cancel();

	/*
	 * #2952
	 * Cancel Page procedure if one active.
	 */
	if ( (_LMch_p_pending_link) && (_LMch_p_pending_link->state == W4_PAGE_OUTCOME))
	{
		if (LMconfig_LM_Check_Device_State(PRH_DEV_STATE_CREATE_CONN_PENDING))
		{
			if (LMconfig_LM_Check_Device_State(PRH_DEV_STATE_PAGE_PENDING))
			{
				/* Cancel Baseband Page procedure */
				status = LC_Page_Cancel();
				LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_PAGE_PENDING);
           
				_Send_HC_Connection_Complete_Event_ACL(_LMch_p_pending_link, NO_CONNECTION);
               
				LMaclctr_Free_Link(_LMch_p_pending_link);
			}
			else /* BB connection, but LMP not complete*/
			{
				LMconfig_LM_Set_Device_State(PRH_DEV_STATE_CANCEL_CONN_PENDING);
				LMconnection_Send_LMP_Detach(_LMch_p_pending_link, LOCAL_TERMINATED_CONNECTION);
			}
		}
	}
	return status;
}
