/**************************************************************************
 * MODULE NAME:    hc_cmd_disp.c
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    Host Controller command dispatcher
 * AUTHOR:         Gary Fleming
 * DATE:           04-07-1999
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *    02-06-1999 -   GF
 *    19-07-1999 -   GF  for major code review.
 *
 * SOURCE CONTROL: $Id: hc_cmd_disp.c,v 1.298 2014/03/11 03:13:35 garyf Exp $
 *
 *************************************************************************/

#include "sys_config.h"
#include "sys_mmi.h"
#include "sys_features.h"
#include "hc_const.h"
#include "hc_event_gen.h"
#include "hc_cmd_disp.h"
#include "hc_flow_control.h"
#include "hci_opcode_len.h"
#include "hci_params.h"

#include "lmp_const.h"
#include "lmp_ch.h"
#include "lmp_features.h"
#include "lmp_scan.h"
#include "lmp_timer.h"
#include "lmp_inquiry.h"
#include "lmp_sec_upper.h"
#include "lmp_link_control_protocol.h"
#include "lmp_link_key_db.h"
#include "lmp_link_qos.h"
#include "lmp_link_policy_protocol.h"
#include "lmp_link_policy.h"
#include "lmp_acl_container.h"
#include "lmp_sco_container.h"
#include "lmp_con_filter.h"
#include "lmp_link_qos.h"
#include "lmp_config.h"
#include "lmp_utils.h"
#include "lmp_ssp.h"

#include "bt_test.h"
#include "bt_mini_sched.h"
#include "bt_timer.h"
#include "tra_queue.h"

#include "lc_interface.h"


#include "le_connection.h"
#include "le_white_list.h"
#include "le_link_layer.h"
#include "le_test.h"
#include "le_security.h"
#include "le_frequency.h"


#include "le_config.h"
#include "le_advertise.h"
#include "le_scan.h"

#include "hw_radio.h"

/*
 * V1.2 specific
 */
#include "lslc_afh.h"
#include "lmp_afh.h"

/*
 * Support Vendor Specific Debug Commands
 */
#include "tc_cmd_disp.h"

#include "lmp_ecc.h"
#include "le_config.h"
#include "patch_function_id.h"

extern t_LE_Config LE_config;

/*
 * Dispatchers are organised by OGF field
 */
static t_error _Dispatch_Invalid_OGF_Command(t_p_pdu p_payload, u_int16 opcode);
static t_error _Dispatch_Link_Control_Command(t_p_pdu p_payload, u_int16 opcode);
static t_error _Dispatch_Link_Policy_Command(t_p_pdu p_payload, u_int16 opcode);
static t_error _Dispatch_HC_BB_Command(t_p_pdu p_payload, u_int16 opcode);
static t_error _Dispatch_Info_Command(t_p_pdu p_payload, u_int16 opcode);
static t_error _Dispatch_Local_Status_Command(t_p_pdu p_payload, u_int16 opcode);
static t_error _Dispatch_Test_Command(t_p_pdu p_payload, u_int16 opcode);
static t_error _Dispatch_LE_Command(t_p_pdu p_payload, u_int16 opcode);

//static void _Send_HCI_Encryption_Change_Event_Deffered(t_lmp_link* p_link);

typedef t_error (*t_ogf_jump)(t_p_pdu p_payload, u_int16 opcode);
const t_ogf_jump ogf_jump_table[] = {
    /* OGF 0 - Error     */ &_Dispatch_Invalid_OGF_Command,
    /* LINK_CONTROL_OGF  */ &_Dispatch_Link_Control_Command,
	/* LINK_POLICY_OGF   */ &_Dispatch_Link_Policy_Command,
    /* HOST_CONTROL_OGF  */ &_Dispatch_HC_BB_Command,
    /* LOCAL_INFO_OGF    */ &_Dispatch_Info_Command,
    /* LOCAL_STATUS_OGF  */ &_Dispatch_Local_Status_Command,
    /* TEST_COMMANDS_OGF */ &_Dispatch_Test_Command,
    /* Un-used OGF       */ &_Dispatch_Invalid_OGF_Command,
    /* LE COMMANDS_OGF   */ &_Dispatch_LE_Command };
#ifndef BLUETOOTH_MODE_LE_ONLY
const struct 
    {
    u_int16 opcode;
    t_error (*link_command_jump)(t_lmp_link*);
    } link_command_table[10] = {

    { HCI_AUTHENTICATION_REQUESTED, LMsec_upper_LM_Authentication_Requested },
    { HCI_CHANGE_CONNECTION_LINK_KEY, LMsec_upper_LM_Change_Connection_Link_Key},
    { HCI_READ_REMOTE_FEATURES, LMconnection_LM_Read_Remote_Supported_Features },
    
    { HCI_READ_CLOCK_OFFSET, LMlc_LM_Clock_Offset_Req },

{ HCI_READ_REMOTE_VER_INFO, LMconfig_LM_Read_Remote_Version }


};
#endif

#if (PRH_BS_CFG_SYS_INQUIRY_READ_WRITE_IAC_LAP_SUPPORTED==1)
static boolean HCbound_Check_IAC_LAPs(u_int8 num_iac_laps, u_int8* iac_laps);
static boolean HCbound_Check_IAC_LAP(u_int8* iac_lap);
#endif

extern t_SYS_Config g_sys_config;



/*****************************************************************
 *  HC_Command_Dispatcher
 *
 *  Description 
 *  Takes the next PDU off the incoming HCI queue and Decodes the opcode.
 *  
 *  
 *  If in loopback mode a subset of commands are treated as per usual,
 *  but the majority are looped back to the Host via the loop back event.
 *
 *  If not in Loopback mode the commands are dispatched to the respective 
 *  layers.
 *
 *****************************************************************/ 
t_error HC_Command_Dispatcher()
{
	if(PATCH_FUN[HC_COMMAND_DISPATCHER_ID]){
         
         return ((t_error (*)())PATCH_FUN[HC_COMMAND_DISPATCHER_ID])();
    }

	
    t_error status = UNSPECIFIED_ERROR;
    struct q_desc *pdu_descr;

    t_p_pdu p_payload;
    u_int16 opcode;
    u_int8  ogf;

    /*
     * Dequeue next HCI command
     */
    pdu_descr = BTq_Dequeue_Next(HCI_COMMAND_Q,0);

    if(pdu_descr && pdu_descr->data)
    {
        p_payload = pdu_descr->data;

        opcode = HCIparam_Get_HCI_Opcode(pdu_descr->data);

#if (PRH_BS_CFG_SYS_LOOPBACK_VIA_HCI_SUPPORTED==1)
        /*
         * If in loopback the following commands must not be looped back
         */
        if ((LOCAL_LOOPBACK == BTtst_Read_Loopback_Mode()) && 
           ((opcode != HCI_RESET) && 
            (opcode != HCI_SET_HOST_CONTROLLER_TO_HOST_FLOW_CONTROL) &&
            (opcode != HCI_HOST_BUFFER_SIZE) && 
            (opcode != HCI_HOST_NUMBER_OF_COMPLETED_PACKETS) && 
            (opcode != HCI_READ_BUFFER_SIZE) &&
            (opcode != HCI_READ_LOOPBACK_MODE) && 
            (opcode != HCI_WRITE_LOOPBACK_MODE)))
        {
            HCeg_Loopback_Command_Event(p_payload, opcode);
        }
        else
#endif
        {
            /*
             *  Inspects the OGF in the HCI Command Opcode and then calls the 
             *  appropiate handler for the group of commands
             *  if (Proprietary Command) then
             *       Process via Call
             *  else if (OGF entry in OGF jump table) then
             *       Process via OGF jump table dispatchers
             *  else
             *       Process as invalid command
             *  endif
             */
            ogf = (opcode >> 10);

            if (ogf == VENDOR_SPECIFIC_DEBUG_OGF)
            {
                status = TCI_Dispatch_Prop_Command(p_payload, opcode);
            }
            else if (ogf < (sizeof(ogf_jump_table)/sizeof(ogf_jump_table[0])) )
            {
                status = (ogf_jump_table[ogf])(p_payload, opcode);
            }
            else
            {
                status = _Dispatch_Invalid_OGF_Command(p_payload, opcode);
            }

            /*
             * Any error results in a Command Status Event
             * Errors for command complete events are handled by the dispatchers.
             */
            if (status != NO_ERROR)
                HCeg_Command_Status_Event(status, opcode);
        }

        BTq_Ack_Last_Dequeued(HCI_COMMAND_Q, 0, pdu_descr->length);
    }
    else
    {
        status = NO_ERROR;
    }

    return status;
}

/*****************************************************************
 *  FUNCTION :- _Dispatch_Invalid_OGF_Command
 *
 *  DESCRIPTION 
 *  Handles any errored HCI test commands, for now return ILLEGAL_COMMAND
 *  
 *****************************************************************/ 
t_error _Dispatch_Invalid_OGF_Command(t_p_pdu p_payload, u_int16 opcode)
{
    return ILLEGAL_COMMAND;
}

/*****************************************************************
 *  FUNCTION :- _Dispatch_Test_Command
 *
 *  DESCRIPTION 
 *  Handles the HCI test commands
 *  
 *****************************************************************/ 
t_error _Dispatch_Test_Command(t_p_pdu p_payload, u_int16 opcode)
{
	
	if(PATCH_FUN[_DISPATCH_TEST_COMMAND_ID]){
         
         return ((t_error (*)(t_p_pdu p_payload, u_int16 opcode))PATCH_FUN[_DISPATCH_TEST_COMMAND_ID])(p_payload,   opcode);
    }

#ifndef BLUETOOTH_MODE_LE_ONLY
    t_cmd_complete_event cmd_complete_info;
    t_cmd_complete_event* p_event_info = &cmd_complete_info;
    u_int8 length;
    u_int16 ocf;

    p_event_info->opcode = opcode;
    p_event_info->status = NO_ERROR;

    length = HCIparam_Get_Length(p_payload+2);

    /*
     * Note Must ensure that the OCF does not extend beyond the 
     * bounds of length array
     *
     */

    ocf = ( opcode & 0x03FF);

    if ( ocf > MAX_HC_TEST_COMMAND_OPCODE)
        return ILLEGAL_COMMAND;

    if (length == test_command_len[ocf] )
    {
        switch(opcode) 
        {
        case HCI_READ_LOOPBACK_MODE :
            p_event_info->mode = BTtst_Read_Loopback_Mode();
            break;

#if (PRH_BS_CFG_SYS_LOOPBACK_VIA_HCI_SUPPORTED==1)
        case HCI_WRITE_LOOPBACK_MODE :
            if(*(p_payload+3) > 2)
                p_event_info->status = INVALID_HCI_PARAMETERS;
            else
                p_event_info->status = 
                    BTtst_Write_Loopback_Mode((t_loopback_mode)*(p_payload+3));
            break;
#else
        case HCI_WRITE_LOOPBACK_MODE:
            p_event_info->status = UNSUPPORTED_FEATURE;
            break;
#endif

        case HCI_ENABLE_DEVICE_UNDER_TEST_MODE :
            BTtst_Set_DUT_Mode(DUT_ENABLED);   
            break; 

		case HCI_WRITE_SIMPLE_PAIRING_DEBUG_MODE :
#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
			//
            if(*(p_payload+3) > 1)
                p_event_info->status = INVALID_HCI_PARAMETERS;
			else
				p_event_info->status = LMssp_LM_Write_SSP_Debug_Mode(*(p_payload+3));
			break;
#else
            return UNSUPPORTED_FEATURE;
			break;
#endif

        default :
            return ILLEGAL_COMMAND;
        }
    }
    else
    {
        return INVALID_HCI_PARAMETERS; 
    }

    HCeg_Command_Complete_Event(&cmd_complete_info);
#endif//BLUETOOTH_MODE_LE_ONLY
    return NO_ERROR;
}

/*****************************************************************
 *  FUNCTION :- _Dispatch_Info_Command
 *
 *  DESCRIPTION 
 *  Handles the HCI Information commands
 *  
 *****************************************************************/

t_error _Dispatch_Info_Command(t_p_pdu p_payload, u_int16 opcode)
{
	if(PATCH_FUN[_DISPATCH_INFO_COMMAND_ID]){
         
         return ((t_error (*)(t_p_pdu p_payload, u_int16 opcode))PATCH_FUN[_DISPATCH_INFO_COMMAND_ID])(p_payload,   opcode);
    }

	
    t_cmd_complete_event cmd_complete_info;
    t_cmd_complete_event* p_event_info = &cmd_complete_info;
    u_int8 length;
	u_int8 zero_array[8]= {0,0,0,0,0,0,0,0};
    u_int16 ocf;

    ocf = ( opcode & 0x03FF);

    if ( ocf > MAX_HC_INFO_COMMAND_OPCODE)
        return ILLEGAL_COMMAND;

    length = HCIparam_Get_Length(p_payload+2);

    p_event_info->opcode = opcode;
    p_event_info->status = NO_ERROR;

    if (length == information_command_len[ocf])
    {
        switch(opcode) 
        {

		case HCI_READ_LOCAL_COMMANDS :
			p_event_info->p_u_int8 = SYSconfig_Get_HC_Commands_Ref();
			break;

        case HCI_READ_LOCAL_FEATURES :
            p_event_info->p_u_int8 = SYSconfig_Get_LMP_Features_Ref(); 
            break;

        case HCI_READ_LOCAL_VER_INFO :
            p_event_info->returnParams.readLocalVersion.version = 
                SYSconfig_Get_Version_Info_Ref();
            break;

        case HCI_READ_BUFFER_SIZE :
            p_event_info->returnParams.readBufferSize.p_hc_buffer_size = 
                SYSconfig_Get_HC_Buffer_Size_Ref();
            break;

        case HCI_READ_COUNTRY_CODE :
            /*
             * 0:   North America, Europe (-France), Japan
             * 1:   France
             */
            p_event_info->mode = (SYSconfig_Get_Hopping_Mode()==FRANCE_FREQ);
            break;

        case HCI_READ_BD_ADDR :
            p_event_info->p_bd_addr = SYSconfig_Get_Local_BD_Addr_Ref();
            break;

#if (PRH_BS_CFG_SYS_READ_LOCAL_EXTENDED_FEATURES_SUPPORTED==1)
        case HCI_READ_LOCAL_EXTENDED_FEATURES:
            if(p_payload[3] == 0)
            {
                p_event_info->p_u_int8 = SYSconfig_Get_LMP_Features_Ref();
            }
            else if(p_payload[3] < NUM_FEATURES_PAGES)
            {
#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
				zero_array[0] = g_LM_config_info.ssp_enabled;
#else
				zero_array[0] = 0;
#endif
				if (g_LM_config_info.le_host_supported)
					zero_array[0] |= 2;
				if (g_LM_config_info.le_and_bredr_host)
					zero_array[0] |= 4;

                p_event_info->p_u_int8 = zero_array;
            }
            else
            {
                p_event_info->status = EC_PARAMETER_OUT_OF_MANDATORY_RANGE;
                p_event_info->p_u_int8 = 0;
            }
            p_event_info->returnParams.readLocalExtendedFeatures.page = p_payload[3];
            p_event_info->returnParams.readLocalExtendedFeatures.max_page = NUM_FEATURES_PAGES-1;
            break;
#else
        case HCI_READ_LOCAL_EXTENDED_FEATURES:
			return ILLEGAL_COMMAND; 
				break;
#endif
        default :
            return ILLEGAL_COMMAND;
        }
    }
    else
    {
        return INVALID_HCI_PARAMETERS; 
    }
    HCeg_Command_Complete_Event(&cmd_complete_info);
    return NO_ERROR;
}

/*****************************************************************
 *  FUNCTION :- _Dispatch_Link_Control_Command
 *
 *  DESCRIPTION 
 *  Handles the HCI Information commands
 *  
 *****************************************************************/

t_error _Dispatch_Link_Control_Command(t_p_pdu p_payload, u_int16 opcode)
{
	
	 
    if(PATCH_FUN[_DISPATCH_LINK_CONTROL_COMMAND_ID]){
         
         return ((t_error (*)(t_p_pdu p_payload, u_int16 opcode))PATCH_FUN[_DISPATCH_LINK_CONTROL_COMMAND_ID])(p_payload,   opcode);
    }

	t_error status = NO_ERROR;

    t_cmd_complete_event cmd_complete_info;
    u_int8 length;
    u_int16 ocf;
    //t_bd_addr  bd_addr;
    t_error  reason = NO_ERROR;
    u_int16 handle;
    //t_lmp_link* p_link;
    //u_int8 inquiry_length;
    //u_int8 number;
    //t_lap  lap;
    //t_pageParams page_params;
    //u_int16 hci_packet_types;

    length = HCIparam_Get_Length(p_payload+2);

    /*************************************************************
     * Obtained for all commands even if handle is not a parameter
     *************************************************************/

    handle = HCIparam_Get_Connection_Handle(p_payload+3);
    //p_link = LMaclctr_Find_ACL_Handle(handle);
    //HCIparam_Get_Bd_Addr(p_payload+3,&bd_addr);

    ocf = ( opcode & 0x03FF);

    if ( ocf > MAX_HC_LINK_CONTROL_OPCODE)
    {
        return ILLEGAL_COMMAND;
    }

    cmd_complete_info.opcode = opcode;
    cmd_complete_info.status = NO_ERROR;

    if (length == link_control_command_len[ocf])
    { 
		u_int8 link_id;
        switch(opcode) 
        {
#ifndef BLUETOOTH_MODE_LE_ONLY        
        case HCI_INQUIRY : 
            lap = HCIparam_Get_Lap(p_payload+3);
            inquiry_length = HCIparam_Get_Inquiry_Length(p_payload+6);
            number = HCIparam_Get_Num_Responses(p_payload+7);
            if ((inquiry_length > MAX_INQUIRY_LENGTH) || (inquiry_length < 1) 
                || (lap < 0x9E8B00) || ( lap > 0x9E8B3F))
                status = INVALID_HCI_PARAMETERS;
            else
                status = LMinq_Inquiry_Start(lap,inquiry_length,number);
            break;

        case HCI_INQUIRY_CANCEL :
            cmd_complete_info.status = LMinq_Inquiry_Cancel();
            break;

#if (PRH_BS_CFG_SYS_INQUIRY_PERIODIC_SUPPORTED==1)
        case HCI_PERIODIC_INQUIRY_MODE :
            {
            u_int16 max_len;
            u_int16 min_len;
            u_int8 max_resp;
            max_len = HCIparam_Get_Uint16(p_payload+3);
            min_len = HCIparam_Get_Uint16(p_payload+5);
            lap = HCIparam_Get_Lap(p_payload+7);
            inquiry_length = HCIparam_Get_Inquiry_Length(p_payload+10);
            max_resp = HCIparam_Get_Num_Responses(p_payload+11);
            /* 
             * Specification:  Max_Period_Length > 
             * Min_Period_Length > Inquiry_Length
             */
            if((max_len < 0x03) || (min_len > 0xFFFE) || (min_len < 0x02) ||
               (inquiry_length > MAX_INQUIRY_LENGTH) || (inquiry_length < 0x01) 
               || (max_len <= min_len ) || (min_len <= inquiry_length) || 
               (lap < 0x9E8B00) || ( lap > 0x9E8B3F))
            {
                cmd_complete_info.status = INVALID_HCI_PARAMETERS;
            }
            else if(max_len > 0x8000)
            {
                cmd_complete_info.status = UNSUPPORTED_FEATURE;
            }
            else
                status = LMinq_Periodic_Inquiry(max_len,min_len,lap,
                                                inquiry_length,max_resp);
            }
            break;

        case HCI_EXIT_PERIODIC_INQUIRY_MODE :
            cmd_complete_info.status = LMinq_Exit_Periodic_Inquiry_Mode();
            break;
#else
        case HCI_PERIODIC_INQUIRY_MODE :
        case HCI_EXIT_PERIODIC_INQUIRY_MODE :
            cmd_complete_info.status = UNSUPPORTED_FEATURE;
            break;
#endif

        case HCI_CREATE_CONNECTION :
            page_params.p_bd_addr = &bd_addr;
            page_params.packet_types = (t_HCIpacketTypes)HCIparam_Get_Packet_Types(p_payload+9);
#if (PRH_BS_CFG_SYS_HCI_V20_FUNCTIONALITY_SUPPORTED==1)
            page_params.packet_types ^= HCI_ACL_EDR; /* EDR packet types use inverted logic */
#endif
            page_params.srMode = HCIparam_Get_SrMode(p_payload+11);
            page_params.pageScanMode = 
                (t_pageScanMode)HCIparam_Get_SpMode(p_payload+12);
            page_params.clockOffset = HCIparam_Get_Uint16(p_payload+13);
            page_params.allow_role_switch = HCIparam_Get_Allow_Role_Switch(p_payload+15);
            /*
             * All connection admission checking is completed by LMP
             */
            status = LMconnection_LM_Connection_Req(&page_params);
            break;

#if (PRH_BS_CFG_SYS_HCI_V12_FUNCTIONALITY_SUPPORTED==1)
        case  HCI_CREATE_CONNECTION_CANCEL:
            status = LMconnection_LM_Connection_Cancel(&bd_addr);
            break;
#else
        case  HCI_CREATE_CONNECTION_CANCEL:
            status = UNSUPPORTED_FEATURE;
            break;

#endif
#endif
        case HCI_DISCONNECT :    
            /*
             * The checks below are for HCI disconnects,
             * therefore the scope is correct.
             */
            reason = HCIparam_Get_Reason(p_payload+5);
            if ( (handle > MAX_CONNECTION_HANDLE) || 
                    ((reason != 0x13) && (reason != 0x14) && 
                    (reason != 0x15) && (reason != 0x1A) &&
                    (reason != 0x05) && (reason != 0x29)) )
            {
                status = INVALID_HCI_PARAMETERS;
            }
            else
            {
				if(LEconnection_Is_Valid_Handle(handle))
            	{
					if (NO_ERROR == LEconnection_Find_Link_Id(handle,&link_id))
					{
						t_LE_Connection* p_connection = LEconnection_Find_Link_Entry(link_id);

            			LEconnection_Disconnect(p_connection, map_hci_reason_to_lmp_reason(reason));
					}
            	}
				else
				{
                	//status = LMconnection_LM_Disconnect_Req(handle, map_hci_reason_to_lmp_reason(reason));
                	status = NO_CONNECTION;
                }
            }
            break;
#ifndef BLUETOOTH_MODE_LE_ONLY

        case HCI_ACCEPT_CONNECTION_REQUEST :
            p_link = LMaclctr_Find_Peer_Bd_Addr(&bd_addr);
            if (p_link)
            {
                if((p_link->state == W4_HOST_ACL_ACCEPT) 
                    || (p_link->state & W4_HOST_SCO_ACCEPT))
                    status = NO_ERROR;
                else
                    status = COMMAND_DISALLOWED;
            }
            else
            {
                status = NO_CONNECTION;
            }

            if ((NO_ERROR == status) && (HCIparam_Get_Role(p_payload+9) > 0x01))
            {
                status = INVALID_HCI_PARAMETERS;
            }

            /*
             * Note Have to generate Command Status here. As no 
             * error checking performed in the LM for this command
             */

            HCeg_Command_Status_Event(status,opcode);
            if (NO_ERROR == status)
            {
                LMconnection_LM_Connection_Accept(&bd_addr,
                    HCIparam_Get_Role(p_payload+9),
					g_LM_config_info.default_voice_setting);
            }

            /*
             * Force status == NO_ERROR 
             * to prevent the invoker returning an addional command status event 
             */
            status = NO_ERROR;
            break;

        case HCI_REJECT_CONNECTION_REQUEST :
            /*
             * Note for these commands the parameter checking is done in the HC
             */
            p_link = LMaclctr_Find_Peer_Bd_Addr(&bd_addr);
            if (p_link)
            {
                if((p_link->state == W4_HOST_ACL_ACCEPT) 
                    || (p_link->state & W4_HOST_SCO_ACCEPT))
                    status = NO_ERROR;
                else
                    status = COMMAND_DISALLOWED;
            }           
            else
            {
                status = NO_CONNECTION;
            }

            if (NO_ERROR == status)
            {
                reason = HCIparam_Get_Reason(p_payload+9);
                if ((reason != 0x0D) && (reason != 0x0E) && (reason != 0x0F))
                    status = INVALID_HCI_PARAMETERS;
                else
                    status = NO_ERROR; 
            }

            /*
             * Note Have to generate Command Status here. As no error 
             * checking performed in the LM for this command
             */
            HCeg_Command_Status_Event(status,opcode);
            if (NO_ERROR == status)
            {
				status = LMconnection_LM_Connection_Reject(&bd_addr,map_hci_reason_to_lmp_reason(reason));

            }

            /*
             * Force status == NO_ERROR 
             * to prevent the invoker returning an additional command status event 
             */
            status = NO_ERROR;
            break;

#if (PRH_BS_CFG_SYS_REMOTE_NAME_REQUEST_SUPPORTED==1)
        case HCI_REMOTE_NAME_REQUEST :
            page_params.p_bd_addr = &bd_addr;
            page_params.packet_types = HCI_DM1;
            page_params.srMode = HCIparam_Get_SrMode(p_payload+9);
            page_params.pageScanMode = (t_pageScanMode)HCIparam_Get_SpMode(p_payload+10);
            page_params.clockOffset = HCIparam_Get_Uint16(p_payload+11);
            page_params.allow_role_switch = 0;
            /*
             * All connection admission checking is completed by LMP
             */
            status = LMconnection_LM_Read_Remote_Name(&page_params);
            break;
#else
        case HCI_REMOTE_NAME_REQUEST:
            status = UNSUPPORTED_FEATURE;
            break;
#endif

#if (PRH_BS_CFG_SYS_HCI_V12_FUNCTIONALITY_SUPPORTED==1)
        case  HCI_REMOTE_NAME_REQUEST_CANCEL:
            status = LMconnection_LM_Read_Remote_Name_Cancel(&bd_addr);
            break;
#else
        case  HCI_REMOTE_NAME_REQUEST_CANCEL:
            status = UNSUPPORTED_FEATURE;
            break;

#endif

        case HCI_LINK_KEY_REQUEST_REPLY:
            LMsec_upper_LM_Link_Key_Request_Reply(&bd_addr,p_payload+9,&cmd_complete_info);
            break;

        case HCI_LINK_KEY_REQUEST_NEGATIVE_REPLY:
            LMsec_upper_LM_Link_Key_Request_Negative_Reply(&bd_addr,&cmd_complete_info);
            break;

        case HCI_PIN_CODE_REQUEST_REPLY:
            length = p_payload[9];
            LMsec_upper_LM_Pin_Code_Request_Reply(&bd_addr, p_payload+10,
                                                  length, &cmd_complete_info);
            break;

        case HCI_PIN_CODE_REQUEST_NEGATIVE_REPLY:
            LMsec_upper_LM_Pin_Code_Request_Negative_Reply(&bd_addr,&cmd_complete_info);
            break;

        case HCI_CHANGE_CONNECTION_PACKET_TYPE :
            hci_packet_types = (t_HCIpacketTypes)HCIparam_Get_Packet_Types(p_payload+5);
			
#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
			
			{
				t_sco_info *p_sco_link;
				
				p_sco_link = LMscoctr_Find_SCO_By_Connection_Handle(handle);
				
				if(p_sco_link)
				{
					if (p_sco_link->link_type == SCO_LINK)
						status = LMqos_LM_Change_SCO_Packet_Type(handle, hci_packet_types);
					else
						status = COMMAND_DISALLOWED;
				}
				else
#endif
				{
					
#if (PRH_BS_CFG_SYS_HCI_V20_FUNCTIONALITY_SUPPORTED==1)
					hci_packet_types ^= HCI_ACL_EDR; /* EDR packet types use inverted logic */
#endif
					
					status =  LMqos_LM_Validate_Change_Packet_Type(handle, hci_packet_types);
					
					if (NO_ERROR == status)
					{
						
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
						u_int8 link_ptt = LC_Get_PTT_ACL(p_link->device_index);
						if (((hci_packet_types & HCI_ACL_EDR)?1:0) != link_ptt)
						{
							LMch_Send_LMP_Packet_Type_Table_Req(p_link, (u_int8)(link_ptt ^ 1));
						}
#endif
						status = LMqos_LM_Change_Validated_Packet_Type(handle, hci_packet_types);
					}
					
					/*
					* Workaround one case where the LMqos_LM_Change_Packet_Type
					* function wishes to generate an immediate connection packet
					* type changed event.
					*/
					if(status == UNSPECIFIED_ERROR)
					{
						status = NO_ERROR;
						HCeg_Command_Status_Event(status, HCI_CHANGE_CONNECTION_PACKET_TYPE);
						LMqos_Send_HC_Packet_Type_Changed_Event(p_link, status);
					}
					else
					{
						HCeg_Command_Status_Event(status, HCI_CHANGE_CONNECTION_PACKET_TYPE);
						status = NO_ERROR;
					}
				}
#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
			}
#endif
            break;

#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED ==1)
        case HCI_ADD_SCO_CONNECTION:
            /*
             * All error checking is done in LM
             */
            status = LMconnection_LM_Add_SCO_Connection(p_link, 
                        (t_packetTypes)HCIparam_Get_Uint16(p_payload+5),
						g_LM_config_info.default_voice_setting);
            break;
#else
        case HCI_ADD_SCO_CONNECTION:
            status = UNSUPPORTED_FEATURE;
            break;
#endif

#if ((PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1) || (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1))
        case HCI_READ_LMP_HANDLE:
            {
                t_sco_info *sco_info;
                sco_info = LMscoctr_Find_SCO_By_Connection_Handle(handle);
                if (sco_info)
                {
                    cmd_complete_info.handle = handle;
                    cmd_complete_info.number = sco_info->lm_sco_handle;                    
                }
                else
                { 
                    status = NO_CONNECTION;
                }
            }
            break;
#else
        case HCI_READ_LMP_HANDLE:
            status = UNSUPPORTED_FEATURE;
            break;
#endif

#if (PRH_BS_CFG_SYS_LMP_BROADCAST_ENCRYPTION_SUPPORTED==1)
        case HCI_MASTER_LINK_KEY :
            if((*(p_payload+3)) > 1)
                status = INVALID_HCI_PARAMETERS;
            else
                status = LMsec_upper_LM_Master_Link_Key((t_link_key_life)*(p_payload+3));
            break;
#else
         case HCI_MASTER_LINK_KEY :
            status = UNSUPPORTED_FEATURE;
            break;
#endif

#if (PRH_BS_CFG_SYS_LMP_ENCRYPTION_SUPPORTED==1)
        case HCI_SET_CONNECTION_ENCRYPTION_ENABLE :
            if(p_link)
            {
                if(*(p_payload+5)> 0x01)
                    status = INVALID_HCI_PARAMETERS;
                else
                {
                	t_encrypt_mode encrypt_mode = LC_Read_Encryption_Mode(p_link->device_index);

                    if(p_link->operation_pending == LMP_DETACH)
                        return COMMAND_DISALLOWED;

                    if(!p_link->link_key_exists)
                        return COMMAND_DISALLOWED;

                    if((p_link->encrypt_enable) && (*(p_payload+5)))
                    {
                    	// If encryption is already active - and we are trying to enable.
                    	// if encrypt_mode is

                    	if ((encrypt_mode == ENCRYPT_POINT2POINT ) || (encrypt_mode == ENCRYPT_POINT2POINT_BROADCAST ) ||
                    		(encrypt_mode == ENCRYPT_HARDWARE_ONLY_ALL ))
                    	{
                    		// Already encrypted - but need to generate a Encryption Change Event. However this has to be generated
                    		// by a deffered timer - as we want the command status to be generated before the Encryption changed
                    		// event.

                    		LMtmr_Set_Timer(0x01, _Send_HCI_Encryption_Change_Event_Deffered, p_link, 1);
                    		status = NO_ERROR;
                    	} 
                    	else // Encryption is already transitioning... so no need to send Encryption Change Event
                    		 // as one will be generated when encryption finishes transitioning
                    	{
                    		status =  NO_ERROR;
                    	}
                    }
					else
					{
						status = LMsec_upper_LM_Set_Connection_Encryption(p_link,
                                                                (t_flag)*(p_payload+5));
					}
                }
            }
            else
            {
                status = NO_CONNECTION;
            }
            break;
#else
        case HCI_SET_CONNECTION_ENCRYPTION_ENABLE:
            status = UNSUPPORTED_FEATURE;
            break;
#endif

        case HCI_AUTHENTICATION_REQUESTED :
            if(p_link !=0)
                LMsec_upper_LM_Authentication_Requested(p_link);
            else
                status = NO_CONNECTION;
            break;

        case HCI_CHANGE_CONNECTION_LINK_KEY :
            if(p_link !=0)
                LMsec_upper_LM_Change_Connection_Link_Key(p_link);
            else
                status = NO_CONNECTION;
            break;

        case HCI_READ_REMOTE_FEATURES :
            if(p_link !=0)
                status = LMconnection_LM_Read_Remote_Supported_Features(p_link);
            else
                status = NO_CONNECTION;
            break;

#if (PRH_BS_CFG_SYS_HCI_V12_FUNCTIONALITY_SUPPORTED)
#if ((PRH_BS_CFG_SYS_READ_REMOTE_EXTENDED_FEATURES_SUPPORTED==1) && (PRH_BS_CFG_SYS_LMP_EXTENDED_FEATURES_SUPPORTED == 1))
        case HCI_READ_REMOTE_EXTENDED_FEATURES:
            if(p_link != 0)
                status = LMconnection_LM_Read_Remote_Extended_Features(p_link, p_payload[5]);
            else
                status = NO_CONNECTION;
            break;
#elif (PRH_BS_CFG_SYS_LMP_EXTENDED_FEATURES_SUPPORTED == 1)
        case HCI_READ_REMOTE_EXTENDED_FEATURES:
			return ILLEGAL_COMMAND; 
			break;
#else
		case HCI_READ_REMOTE_EXTENDED_FEATURES:
			return UNSUPPORTED_FEATURE; 
			break;
#endif
#endif
#endif
        case HCI_READ_REMOTE_VER_INFO :
		
			if(LEconnection_Is_Valid_Handle(handle))
            {
            	status = LEconnection_Read_Remote_Version_Info(handle);
            	if (status == REPEATED_ATTEMPTS)
            	{
					t_lm_event_info event_info;
					t_LE_Connection* p_connection = LEconnection_Find_P_Connection(handle);
            		// Have to explicitly generate a command status
            		// Followed by a local read of the Verion info previously obatined
            		// for the peer device.

					HCeg_Command_Status_Event(NO_ERROR, HCI_READ_REMOTE_VER_INFO);

					/*
				 	 * Store the Version for the remote device
				 	 */
					event_info.lmp_version = p_connection->lmp_version;
					event_info.comp_id = p_connection->comp_id;
					event_info.lmp_subversion = p_connection->lmp_subversion;

					event_info.handle = handle;
					event_info.status = NO_ERROR;
					HCeg_Generate_Event(HCI_READ_REMOTE_VERSION_INFORMATION_COMPLETE_EVENT,&event_info);
            	}
            }
            /*else if(p_link !=0)
                status = LMconfig_LM_Read_Remote_Version(p_link);*/
            else
                status = NO_CONNECTION;
            break;
#ifndef BLUETOOTH_MODE_LE_ONLY

        case HCI_READ_CLOCK_OFFSET :
        /* Note : The behaviour of the Read_Clock_Offset is different
        * in the master and slave. In the Master this results in an 
        * LMP PDU being sent/received while on the slave side this is
        * a local command.
        * Thus the command status has to be returned prior to invoking
        * the service interface call on the link manager.
            */
            if(p_link !=0)
            {
                if(p_link->operation_pending == LMP_DETACH)
                    HCeg_Command_Status_Event(COMMAND_DISALLOWED, opcode);
                else
                {
                    HCeg_Command_Status_Event(NO_ERROR,opcode);
                    LMlc_LM_Clock_Offset_Req(p_link);
                }
            }
            else
                status = NO_CONNECTION;
            break;

/*
 * These commands are supported if:
 * 1. Extended SCO is supported
 * 2. SCO + 1.2 is supported.
 * As Extended SCO requires both SCO+1.2,
 * it is sufficient to check them individually to cover
 * all cases.
 */
#if ((PRH_BS_CFG_SYS_HCI_V12_FUNCTIONALITY_SUPPORTED) && (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1))
        case HCI_SETUP_SYNCHRONOUS_CONNECTION:
            /*
             * All error checking is done in LM
             */
            hci_packet_types = HCIparam_Get_Uint16(p_payload+18);
#if (PRH_BS_CFG_SYS_HCI_V20_FUNCTIONALITY_SUPPORTED==1)
            hci_packet_types ^= HCI_SYN_EDR; /* EDR packet types use inverted logic */
#endif
            status = LMconnection_LM_Setup_Synchronous_Connection(
                HCIparam_Get_Uint16(p_payload+3), 
                HCIparam_Get_Uint32(p_payload+5),
                HCIparam_Get_Uint32(p_payload+9),
                HCIparam_Get_Uint16(p_payload+13),
                HCIparam_Get_Uint16(p_payload+15),
                *(p_payload+17),
                hci_packet_types);
        break;
        case HCI_ACCEPT_SYNCHRONOUS_CONNECTION_REQUEST:
            hci_packet_types = HCIparam_Get_Uint16(p_payload+22);
#if (PRH_BS_CFG_SYS_HCI_V20_FUNCTIONALITY_SUPPORTED==1)
            if (hci_packet_types != 0xFFFF /* special case, means all types */)
                hci_packet_types ^= HCI_SYN_EDR; /* EDR packet types use inverted logic */
#endif
            status = LMconnection_LM_Accept_Synchronous_Connection_Request(
                p_payload+3,
                HCIparam_Get_Uint32(p_payload+9),
                HCIparam_Get_Uint32(p_payload+13),
                HCIparam_Get_Uint16(p_payload+17),
                HCIparam_Get_Uint16(p_payload+19),
                *(p_payload+21),
                hci_packet_types);
        break;
        case HCI_REJECT_SYNCHRONOUS_CONNECTION_REQUEST:
            p_link = LMaclctr_Find_Peer_Bd_Addr(&bd_addr);
            status = LMconnection_LM_Reject_Synchronous_Connection_Request(
                p_payload+3,
				(t_error)map_hci_reason_to_lmp_reason(*(p_payload+9)));
        break;
#else
        case HCI_SETUP_SYNCHRONOUS_CONNECTION:
        case HCI_ACCEPT_SYNCHRONOUS_CONNECTION_REQUEST:
        case HCI_REJECT_SYNCHRONOUS_CONNECTION_REQUEST:
            status = UNSUPPORTED_FEATURE;
            break;
#endif

#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
        case HCI_IO_CAPABILITY_REQUEST_REPLY : 
            p_link = LMaclctr_Find_Peer_Bd_Addr(&bd_addr); 
			if ((*(p_payload+9)  < 0x04) &&
			    (*(p_payload+10) < 0x02) &&
			    (*(p_payload+11) < 0x06)) 
			{
				status = LMssp_LM_IO_Capability_Request_Reply(p_link,*(p_payload+9),
					*(p_payload+10), *(p_payload+11));
			}
			else
			{
				status = INVALID_HCI_PARAMETERS;
			}
			cmd_complete_info.status = status;
			cmd_complete_info.p_bd_addr = &bd_addr;
			HCeg_Command_Complete_Event(&cmd_complete_info);
			break;

        case HCI_IO_CAPABILITY_REQUEST_NEGATIVE_REPLY : 
            p_link = LMaclctr_Find_Peer_Bd_Addr(&bd_addr); 
			if (p_link == 0)
			{
				cmd_complete_info.status = NO_CONNECTION;
				cmd_complete_info.p_bd_addr = &bd_addr;
			}
			else
			{
				cmd_complete_info.status = NO_ERROR;
				cmd_complete_info.p_bd_addr = &bd_addr;
			}
			HCeg_Command_Complete_Event(&cmd_complete_info);
			LMssp_LM_IO_Capability_Request_Negative_Reply(p_link,*(p_payload+9));
			break;

		case HCI_USER_CONFIRMATION_REQUEST_REPLY :
			p_link = LMaclctr_Find_Peer_Bd_Addr(&bd_addr); 
			cmd_complete_info.p_bd_addr = &bd_addr;
			cmd_complete_info.status = LMssp_LM_User_Confirmation_Request_Reply(p_link);
			HCeg_Command_Complete_Event(&cmd_complete_info);
			break;

		case HCI_USER_CONFIRMATION_REQUEST_NEGATIVE_REPLY :
			p_link = LMaclctr_Find_Peer_Bd_Addr(&bd_addr); 
			cmd_complete_info.p_bd_addr = &bd_addr;
			cmd_complete_info.status = NO_ERROR;
			HCeg_Command_Complete_Event(&cmd_complete_info);
			LMssp_LM_User_Confirmation_Request_Negative_Reply(p_link);
			break;
#if (PRH_BS_CFG_SYS_SSP_PASSKEY_SUPPORTED==1)
	    case HCI_USER_PASSKEY_REQUEST_REPLY :
			p_link = LMaclctr_Find_Peer_Bd_Addr(&bd_addr); 
			if (p_link)
			{
				cmd_complete_info.p_bd_addr = &bd_addr;
				cmd_complete_info.status =  NO_ERROR;
				HCeg_Command_Complete_Event(&cmd_complete_info);
				LMssp_LM_User_Passkey_Request_Reply(p_link,
					HCIparam_Get_Uint32(p_payload+9));
			}
			else
			{
				cmd_complete_info.status = NO_CONNECTION;
				HCeg_Command_Complete_Event(&cmd_complete_info);
			}
			break;

		case HCI_USER_PASSKEY_REQUEST_NEGATIVE_REPLY :
			p_link = LMaclctr_Find_Peer_Bd_Addr(&bd_addr); 
			if (p_link)
			{
				cmd_complete_info.p_bd_addr = &bd_addr;
				cmd_complete_info.status =  NO_ERROR;
				HCeg_Command_Complete_Event(&cmd_complete_info);
				LMssp_LM_User_Passkey_Request_Negative_Reply(p_link);
			}
			else
			{
				cmd_complete_info.status = NO_CONNECTION;
				HCeg_Command_Complete_Event(&cmd_complete_info);
			}
			break;
#else
	    case HCI_USER_PASSKEY_REQUEST_REPLY :
		case HCI_USER_PASSKEY_REQUEST_NEGATIVE_REPLY :
		    status = UNSUPPORTED_FEATURE;
			break;

#endif
#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)
		case HCI_REMOTE_OOB_DATA_REQUEST_REPLY :
			p_link = LMaclctr_Find_Peer_Bd_Addr(&bd_addr); 
			if (p_link)
			{
			cmd_complete_info.p_bd_addr = &bd_addr;
				cmd_complete_info.status = NO_ERROR;
				HCeg_Command_Complete_Event(&cmd_complete_info);
				LMssp_LM_Remote_OOB_Data_Request_Reply(p_link,(p_payload+9),(p_payload+25));
			}
			else
			{
				cmd_complete_info.status = NO_CONNECTION;
				HCeg_Command_Complete_Event(&cmd_complete_info);
			}
			break;

		case HCI_REMOTE_OOB_DATA_REQUEST_NEGATIVE_REPLY :
			p_link = LMaclctr_Find_Peer_Bd_Addr(&bd_addr); 
			if (p_link)
			{
			cmd_complete_info.p_bd_addr = &bd_addr;
				cmd_complete_info.status = NO_ERROR;
				HCeg_Command_Complete_Event(&cmd_complete_info);
				LMssp_LM_Remote_OOB_Data_Request_Negative_Reply(p_link);
			}
			else
			{
				cmd_complete_info.status = NO_CONNECTION;
				HCeg_Command_Complete_Event(&cmd_complete_info);
			}
			break;
#else
		case HCI_REMOTE_OOB_DATA_REQUEST_NEGATIVE_REPLY :
		case HCI_REMOTE_OOB_DATA_REQUEST_REPLY :
            status = UNSUPPORTED_FEATURE;
            break;
#endif
#else
        case HCI_IO_CAPABILITY_REQUEST_REPLY : 
        case HCI_IO_CAPABILITY_REQUEST_NEGATIVE_REPLY :
		case HCI_USER_CONFIRMATION_REQUEST_REPLY :
		case HCI_USER_CONFIRMATION_REQUEST_NEGATIVE_REPLY :
	    case HCI_USER_PASSKEY_REQUEST_REPLY :
		case HCI_USER_PASSKEY_REQUEST_NEGATIVE_REPLY :
		case HCI_REMOTE_OOB_DATA_REQUEST_NEGATIVE_REPLY :
		case HCI_REMOTE_OOB_DATA_REQUEST_REPLY :
            status = UNSUPPORTED_FEATURE;
            break;
#endif
#endif//BLUETOOTH_MODE_LE_ONLY
        default :
            return ILLEGAL_COMMAND;
            break;  
        }

        /*
         * Command 
         */
        if (status == NO_ERROR)
        {
            switch(opcode)
            {
            case HCI_LINK_KEY_REQUEST_REPLY :
            case HCI_LINK_KEY_REQUEST_NEGATIVE_REPLY :
            case HCI_PIN_CODE_REQUEST_REPLY :
            case HCI_PIN_CODE_REQUEST_NEGATIVE_REPLY :
            case HCI_AUTHENTICATION_REQUESTED :
            case HCI_CHANGE_CONNECTION_LINK_KEY: 
            case HCI_MASTER_LINK_KEY:
            case HCI_CHANGE_CONNECTION_PACKET_TYPE :
            case HCI_ACCEPT_CONNECTION_REQUEST :
            case HCI_REJECT_CONNECTION_REQUEST :
            case HCI_ACCEPT_SYNCHRONOUS_CONNECTION_REQUEST:
            case HCI_REJECT_SYNCHRONOUS_CONNECTION_REQUEST:
            case HCI_READ_CLOCK_OFFSET :
#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
			case HCI_IO_CAPABILITY_REQUEST_REPLY :
			case HCI_IO_CAPABILITY_REQUEST_NEGATIVE_REPLY :

#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)
			case HCI_REMOTE_OOB_DATA_REQUEST_REPLY :
			case HCI_REMOTE_OOB_DATA_REQUEST_NEGATIVE_REPLY :
#endif
#endif
            case HCI_CREATE_CONNECTION_CANCEL:
            case HCI_REMOTE_NAME_REQUEST_CANCEL:
                /*
                 * Do nothing.
                 * Event generation handled by LMPch.
                 */

#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
			case HCI_USER_CONFIRMATION_REQUEST_REPLY :
			case HCI_USER_CONFIRMATION_REQUEST_NEGATIVE_REPLY :
			case HCI_USER_PASSKEY_REQUEST_REPLY :
			case HCI_USER_PASSKEY_REQUEST_NEGATIVE_REPLY :
#endif
                break;

            case HCI_PERIODIC_INQUIRY_MODE :
            case HCI_EXIT_PERIODIC_INQUIRY_MODE :
            case HCI_INQUIRY_CANCEL :
            case HCI_READ_LMP_HANDLE:
                HCeg_Command_Complete_Event(&cmd_complete_info);
                break;

            default :
                HCeg_Command_Status_Event(status,opcode);
                break;
            }

        }
        else if ((status == REPEATED_ATTEMPTS) && (opcode == HCI_READ_REMOTE_VER_INFO))
        {
        	status = NO_ERROR;
        }
    }
    else
    { 
        return INVALID_HCI_PARAMETERS;
    }

    return status;
}

/*****************************************************************
 *  FUNCTION :- _Dispatch_Link_Policy_Commands
 *
 *  DESCRIPTION 
 *  Handles the HCI Information commands
 *  
 *****************************************************************/
t_error _Dispatch_Link_Policy_Command(t_p_pdu p_payload, u_int16 opcode)
{
	if(PATCH_FUN[_DISPATCH_LINK_POLICY_COMMAND_ID]){
         
         return ((t_error (*)(t_p_pdu p_payload, u_int16 opcode))PATCH_FUN[_DISPATCH_LINK_POLICY_COMMAND_ID])(p_payload,   opcode);
    }

	
	t_error status = NO_ERROR;
#ifndef BLUETOOTH_MODE_LE_ONLY
    t_cmd_complete_event cmd_complete_info;
    u_int8 length, expected_length;
    u_int16 ocf;
    t_bd_addr  bd_addr;
    u_int16 handle;
    t_lmp_link* p_link;
    u_int16 max_interval,min_interval;

    cmd_complete_info.opcode = opcode;
    ocf = ( opcode & 0x03FF);

    if ( ocf > MAX_HC_LINK_POLICY_OPCODE)
        return ILLEGAL_COMMAND;

    length = HCIparam_Get_Length(p_payload+2);
    HCIparam_Get_Bd_Addr(p_payload+3,&bd_addr);

    /*
     * Obtain the p_link for all link policy commands
     */
    if (opcode != HCI_SWITCH_ROLE)
    {
        handle = HCIparam_Get_Connection_Handle(p_payload+3);
        cmd_complete_info.handle = handle;
        p_link = LMaclctr_Find_ACL_Handle(handle);
    }
    else
    {
        p_link = LMaclctr_Find_Peer_Bd_Addr(&bd_addr);
    }

    expected_length = link_policy_command_len[ocf];
    cmd_complete_info.status = NO_ERROR;

    if (length == expected_length)
    {
        max_interval = HCIparam_Get_Uint16(p_payload+5);
        min_interval = HCIparam_Get_Uint16(p_payload+7);
        switch(opcode) 
        {
#if ( PRH_BS_CFG_SYS_LMP_HOLD_SUPPORTED == 1)
        case HCI_HOLD_MODE :
            min_interval &= 0xFFFE;
            max_interval &= 0xFFFE;
            if(!p_link)
                return NO_CONNECTION;
            else if(max_interval < min_interval)
                status = INVALID_HCI_PARAMETERS;
            else if(min_interval < PRH_DEFAULT_MIN_HOLD_INTERVAL)
                status = UNSUPPORTED_FEATURE;
            else if(LMconfig_LM_Check_Device_State(PRH_DEV_STATE_PAGE_PENDING))
                return COMMAND_DISALLOWED;
            else
            {
                status = LMpolicy_LM_Hold_Mode(p_link,max_interval,min_interval);
                if (status == NO_ERROR)
                    HCeg_Command_Status_Event(status,opcode);
            }
            break;
#else
        case HCI_HOLD_MODE:
            status = UNSUPPORTED_FEATURE;
            break;
#endif

#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUPPORTED==1)
        case HCI_SNIFF_MODE :
            {
            u_int16 attempt,timeout;
            attempt = HCIparam_Get_Uint16(p_payload+9);
            timeout = HCIparam_Get_Uint16(p_payload+11);
            min_interval &= 0xFFFE;
            max_interval &= 0xFFFE;
            if(!p_link)
                return NO_CONNECTION;
	    /*
	     * Note: Even though odd values of interval are invalid by BT Spec,
	     * cannot reject these as would have large impact on customer host.
	     */
            else if((min_interval > max_interval) || (min_interval < 1) || (attempt < 1)
               || (attempt > 0x7FFF) || (timeout > 0x7FFF) 
               || ((attempt+timeout) >= min_interval))
            {
                HCeg_Command_Status_Event(INVALID_HCI_PARAMETERS, opcode);
            }
            else
            {
                status = LMpolicy_LM_Sniff_Mode(p_link,max_interval,
                                                min_interval,attempt,timeout);
                if (status == NO_ERROR)
                    HCeg_Command_Status_Event(status,opcode);
            }
            }
            break;

        case HCI_EXIT_SNIFF_MODE :
            if(!p_link)
                return NO_CONNECTION;
			/*
			 * !(p_link->state & W4_SNIFF_EXIT) is to avoid collissions.
			 */
            else if ( (p_link->state & LMP_SNIFF_MODE) && (!(p_link->state & W4_SNIFF_EXIT)))
            {
                HCeg_Command_Status_Event(NO_ERROR,opcode);
                status = LMpolicy_LM_Exit_Sniff_Mode(p_link);
            }
            else
            {
                HCeg_Command_Status_Event(COMMAND_DISALLOWED,opcode);
            }
            break;

#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUBRATING_SUPPORTED==1)
        case HCI_SNIFF_SUBRATING:
            {
            u_int16 max_latency, min_remote_timeout, min_local_timeout;

            max_latency = HCIparam_Get_Uint16(p_payload+5);
            min_remote_timeout = HCIparam_Get_Uint16(p_payload+7);
            min_local_timeout = HCIparam_Get_Uint16(p_payload+9);

            status = LMpolicy_LM_Sniff_Subrating(p_link, max_latency, min_remote_timeout, min_local_timeout);

            cmd_complete_info.status = status;
            HCeg_Command_Complete_Event(&cmd_complete_info);
            
            status = NO_ERROR; /* no Command_Status_Event for this command */
            }
            break;
#else
        case HCI_SNIFF_SUBRATING:
            status = UNSUPPORTED_FEATURE;
            break;
#endif

#else
        case HCI_EXIT_SNIFF_MODE :
        case HCI_SNIFF_MODE :
            status = UNSUPPORTED_FEATURE;
            break;
#endif

#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
        case HCI_PARK_MODE :
            if(!p_link)
                return NO_CONNECTION;
            else if((max_interval < min_interval) || (min_interval < 1))
                status = INVALID_HCI_PARAMETERS;
            else
                status = LMpolicy_LM_Park_Mode(p_link,max_interval,min_interval);

            if (status == NO_ERROR)
                HCeg_Command_Status_Event(status,opcode);
            break;

        case HCI_EXIT_PARK_MODE:
            if(!p_link)
                return NO_CONNECTION;
            else
                status = LMpolicy_LM_Exit_Park_Mode(p_link,0);
            if (status == NO_ERROR)
                HCeg_Command_Status_Event(status,opcode);  
            break;
#else
        case HCI_PARK_MODE :
        case HCI_EXIT_PARK_MODE :
            status = UNSUPPORTED_FEATURE;
            break;
#endif

#if (PRH_BS_CFG_SYS_QOS_SUPPORTED==1)
        case HCI_QOS_SETUP :
            {
            u_int32 tk_rate, pk_bandwidth, delay, latency;
            u_int8 service;
            service = *(p_payload+6);
            tk_rate = HCIparam_Get_Uint32(p_payload+7);
            pk_bandwidth = HCIparam_Get_Uint32(p_payload+11);
            latency = HCIparam_Get_Uint32(p_payload+15);
            delay = HCIparam_Get_Uint32(p_payload+19);
            if(!p_link)
                return NO_CONNECTION;
            else if(service > 2)
                status = INVALID_HCI_PARAMETERS;
            else if(p_link->operation_pending == LMP_DETACH)
                status = COMMAND_DISALLOWED;
            else
                status = LMqos_LM_QOS_Setup(p_link,service,
                                            tk_rate,pk_bandwidth,latency,delay);
            if(status == NO_ERROR)
                HCeg_Command_Status_Event(status,opcode);   
            }
            break;
#else
        case HCI_QOS_SETUP :
            status = UNSUPPORTED_FEATURE;
            break;
#endif

        case HCI_ROLE_DISCOVERY :
            if(!p_link)
            {
                cmd_complete_info.status = NO_CONNECTION;
                cmd_complete_info.mode = 0;
                HCeg_Command_Complete_Event(&cmd_complete_info);
                return NO_ERROR;
            }
            else
            {
                LMpolicy_LM_Role_Discovery(p_link,&cmd_complete_info);
                HCeg_Command_Complete_Event(&cmd_complete_info);
            }
            break;

#if (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED == 1)
        case HCI_SWITCH_ROLE :
            if(!p_link)
                return NO_CONNECTION;
            else if(LMconfig_LM_Check_Device_State(PRH_DEV_STATE_PAGE_PENDING))
                return COMMAND_DISALLOWED;
            else
            {
                status =LMconnection_LM_Switch_Role(p_link,(t_role)(*(p_payload+9)));
                if (status == NO_ERROR)
                    HCeg_Command_Status_Event(status,opcode);
            }
            break;
#else       
        case HCI_SWITCH_ROLE:
            status = UNSUPPORTED_FEATURE;
            break;
#endif

        case HCI_WRITE_LINK_POLICY_SETTINGS :
            {
            u_int16 allowed_mask = 0;
#if (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1)
            allowed_mask |= 0x01;
#endif
#if (PRH_BS_CFG_SYS_LMP_HOLD_SUPPORTED==1)
            allowed_mask |= 0x22;
#endif
#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUPPORTED==1)
            allowed_mask |= 0x04;
#endif
#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
            allowed_mask |= 0x18;
#endif

            if(!p_link)
            {
                cmd_complete_info.status = NO_CONNECTION;
                cmd_complete_info.mode = 0;
                HCeg_Command_Complete_Event(&cmd_complete_info);
                return NO_ERROR;
            }
            else if (HCIparam_Get_Uint16(p_payload+5) > allowed_mask)
                cmd_complete_info.status = INVALID_HCI_PARAMETERS;
            else
                p_link->link_policy_mode = HCIparam_Get_Uint16(p_payload+5);

            HCeg_Command_Complete_Event(&cmd_complete_info);
            }
            break;

        case HCI_READ_LINK_POLICY_SETTINGS :
            if(!p_link)
            {
                cmd_complete_info.status = NO_CONNECTION;
                cmd_complete_info.value16bit = 0;
                HCeg_Command_Complete_Event(&cmd_complete_info);
                return NO_ERROR;
            }
            else
            {
                cmd_complete_info.value16bit = p_link->link_policy_mode;
                HCeg_Command_Complete_Event(&cmd_complete_info);
            }
            break;

#if(PRH_BS_CFG_SYS_HCI_V12_FUNCTIONALITY_SUPPORTED==1)
        case HCI_WRITE_DEFAULT_LINK_POLICY_SETTINGS :
            {
            u_int16 allowed_mask = 0;
#if (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1)
            allowed_mask |= 0x01;
#endif
#if (PRH_BS_CFG_SYS_LMP_HOLD_SUPPORTED==1)
            allowed_mask |= 0x22;
#endif
#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUPPORTED==1)
            allowed_mask |= 0x04;
#endif
#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
            allowed_mask |= 0x18;
#endif

            if (HCIparam_Get_Uint16(p_payload+3) > allowed_mask)
                cmd_complete_info.status = INVALID_HCI_PARAMETERS;
            else
                g_LM_config_info.default_link_policy_mode = HCIparam_Get_Uint16(p_payload+3);

            HCeg_Command_Complete_Event(&cmd_complete_info);
            }
            break;

        case HCI_READ_DEFAULT_LINK_POLICY_SETTINGS :
            cmd_complete_info.value16bit = g_LM_config_info.default_link_policy_mode;
            HCeg_Command_Complete_Event(&cmd_complete_info);
            break;
#endif

       case HCI_FLOW_SPECIFICATION :
#if (PRH_BS_CFG_SYS_FLOW_SPECIFICATION_SUPPORTED==1)
            {
            u_int32 tk_rate, pk_bandwidth, tk_bucket_size, latency;
            u_int8 service, direction;

            direction = *(p_payload+6);
            service = *(p_payload+7);

            tk_rate = HCIparam_Get_Uint32(p_payload+8);
            tk_bucket_size = HCIparam_Get_Uint32(p_payload+12);
            pk_bandwidth = HCIparam_Get_Uint32(p_payload+16);
            latency = HCIparam_Get_Uint32(p_payload+20);

            if(!p_link)
                return NO_CONNECTION;
            else if(service > 2)
                status = INVALID_HCI_PARAMETERS;
            else if(p_link->operation_pending == LMP_DETACH)
                status = COMMAND_DISALLOWED;
            else
                status = LMqos_LM_Flow_Specification(direction, p_link,service,
                                            tk_rate,tk_bucket_size,pk_bandwidth,latency);
            if(status == NO_ERROR)
                HCeg_Command_Status_Event(status,opcode);   
            }
#else
            status = UNSUPPORTED_FEATURE;
#endif
            break;

        default :
            status = ILLEGAL_COMMAND;
            break;
        }
    }
    else
        status = INVALID_HCI_PARAMETERS;

#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
    if ((status == NO_ERROR) && LMconfig_LM_Connected_As_Scatternet())
    {
        LMpol_Change_Piconet(p_link->device_index, (PRH_BS_PICONET_SERVICE_TIME/2));
    }
#endif
#endif//BLUETOOTH_MODE_LE_ONLY
    return status;
}

/*****************************************************************
 *  FUNCTION :- _Dispatch_Local_Status_Commands
 *
 *  DESCRIPTION 
 *  Handles the HCI Information commands
 *  
 *****************************************************************/
static t_error _Dispatch_Local_Status_Command(t_p_pdu p_payload, u_int16 opcode)
{
	
	 
    if(PATCH_FUN[_DISPATCH_LOCAL_STATUS_COMMAND_ID]){
         
         return ((t_error (*)(t_p_pdu p_payload, u_int16 opcode))PATCH_FUN[_DISPATCH_LOCAL_STATUS_COMMAND_ID])(p_payload,   opcode) ;
    }

	 t_error status = NO_ERROR;
#ifndef BLUETOOTH_MODE_LE_ONLY
    t_cmd_complete_event cmd_complete_info;
    t_cmd_complete_event *p_event_info = &cmd_complete_info;
    u_int8  length;
    u_int16 ocf;
    u_int16 handle;
    t_lmp_link *p_link;
	t_LE_Connection* p_connection;

    length = HCIparam_Get_Length(p_payload+2);

    /*
     * All commands have handle as first parameter
     */
    handle = HCIparam_Get_Uint16(p_payload+3);

    ocf = ( opcode & 0x03FF);
    if (ocf > MAX_HC_STATUS_COMMAND_OPCODE)
    {
        return ILLEGAL_COMMAND;
    }

    if (length != status_command_len[ocf])
    {
        /*
         * Generate command status event on exit
         */
        return INVALID_HCI_PARAMETERS;
    }

    /*
     * Setup defaults
     */
    p_event_info->opcode = opcode;
    p_event_info->status = NO_ERROR;
    p_event_info->handle = handle;
    p_event_info->number = 0;
    p_event_info->value16bit = 0;

    p_link = LMaclctr_Find_ACL_Handle(handle);
    if (!p_link)
    {
        /*
         * Normally 
         * status = NO_CONNECTION will force a HCI_Command_Status_Event
         *
         */
        p_event_info->status = NO_CONNECTION;
    }


    switch(opcode) 
    {
    /* always support the ability to read failed contact counter
     * if flush is not supported, then it will return 0 failed contacts
     */
    case HCI_READ_FAILED_CONTACT_COUNTER :
#if (PRH_BS_CFG_SYS_FLUSH_SUPPORTED==1)
        if (p_link)
        {
            p_event_info->number = p_link->failed_contact_counter;
        }
#else
        p_event_info->status = UNSUPPORTED_FEATURE;
#endif
        break;

    /*
     * always support the ability to reset the failed contact
     * counter. If flush is not supported, then it will reset
     * the failed contact counter to 0 from 0
     */
    case HCI_RESET_FAILED_CONTACT_COUNTER :
#if(PRH_BS_CFG_SYS_FLUSH_SUPPORTED==1)
        if (p_link)
        {
            p_link->failed_contact_counter = 0;
        }
#else
        p_event_info->status = UNSUPPORTED_FEATURE;
#endif
        break;

    case HCI_GET_LINK_QUALITY :
        if (p_link)
        {
            p_event_info->number = p_link->link_quality;
        }
        break;

    case HCI_READ_RSSI :
#if (PRH_BS_CFG_SYS_LMP_RSSI_SUPPORTED==1)
    	p_connection = LEconnection_Find_P_Connection(handle);

    	if ((p_connection) && (p_connection->active))
        {
            p_event_info->number = p_connection->rssi;
            p_event_info->status = NO_ERROR;
        }
    	else if (p_link)
        {
            s_int8 number = LC_Get_RSSI(p_link->device_index);
            if(number > GOLDEN_RECEIVER_RSSI_MAX)
                number -= GOLDEN_RECEIVER_RSSI_MAX;
            else if(number < GOLDEN_RECEIVER_RSSI_MIN)
                number -= GOLDEN_RECEIVER_RSSI_MIN;
            else
                number = 0;
            p_event_info->number = (u_int8)number;
        }
#else
        p_event_info->number = 0;
        p_event_info->status = UNSUPPORTED_FEATURE;
#endif
        break;

#if PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_SUPPORTED
    case HCI_READ_AFH_CHANNEL_MAP :
        if (p_link)
        {
            /*
             * Get the mode/channel map for the given device
             */
            p_event_info->p_u_int8 = LSLCafh_LM_Read_AFH_Channel_Map(
               p_link->device_index, &p_event_info->mode /*p_afh_mode*/);
        }
        else
        {
            /*
             * Invalid command parameters, allowed to send any
             * data after the command status. Just ensure that
             * p_u_int8 points to somewhere where there are 10
             * bytes to read and won't cause a memory exception.
             */
            p_event_info->p_u_int8 = LSLCafh_LM_Read_AFH_Channel_Map(
                0, &p_event_info->mode);
        }
        break;
#endif

#if PRH_BS_CFG_SYS_HCI_READ_CLOCK_SUPPORTED
    case HCI_READ_CLOCK:
        /*
         * The event handler will handle this command
         * handle and status are already defined above.
         * p_event_info->number is which_clock parameter
         */
        p_event_info->number = p_payload[5];
        break;
#endif

    case HCI_READ_ENCRYPTION_KEY_SIZE:
        if (p_link)
		{
			if (p_link->encrypt_mode != ENCRYPT_NONE)
			{
				p_event_info->number = p_link->encrypt_key_size;
			}
			else
			{
				p_event_info->number = 0;
				status = EC_INSUFFICIENT_SECURITY;
			}
		}
		break;

    default: 
        status = ILLEGAL_COMMAND;
        break;
    }


    if (status == NO_ERROR)
    {
        /*
         * All ok, Command Complete 
         */
        HCeg_Command_Complete_Event(p_event_info);
    }
    else
    {
        /*
         * Error, on return a Command Status Event will be 
         */
    }
#endif//BLUETOOTH_MODE_LE_ONLY
    return status;
}

/*****************************************************************
 *  FUNCTION :- _Dispatch_HC_BB_Commands
 *
 *  DESCRIPTION 
 *  Handles the HCI Information commands
 *  
 *****************************************************************/
#if (PRH_BS_CFG_SYS_INQUIRY_READ_WRITE_IAC_LAP_SUPPORTED==1)
extern u_int8 p_iacs[PRH_BS_CFG_SYS_NUMBER_OF_SUPPORTED_IAC *3];
#endif
t_error _Dispatch_HC_BB_Command(t_p_pdu p_payload, u_int16 opcode)
{
	if(PATCH_FUN[_DISPATCH_HC_BB_COMMAND_ID]){
         
         return ((t_error (*)(t_p_pdu p_payload, u_int16 opcode))PATCH_FUN[_DISPATCH_HC_BB_COMMAND_ID])(p_payload,   opcode);
    }

    t_cmd_complete_event cmd_complete_info;
    t_cmd_complete_event* p_event_info = &cmd_complete_info;
    u_int8 length, expected_length;
    u_int16 ocf;
    u_int16 handle;

    t_lmp_link *p_link;
    u_int8 sp_mode;
#if (PRH_BS_CFG_SYS_INQUIRY_READ_WRITE_IAC_LAP_SUPPORTED!=1)
    u_int8 iac_lap[3] = { 0x33, 0x8b, 0x9e };
#endif

    length = HCIparam_Get_Length(p_payload+2);

    ocf = ( opcode & 0x03FF);
    if ( ocf > MAX_HC_BB_COMMAND_OPCODE)
    {
        return ILLEGAL_COMMAND;
    }

    expected_length = hc_bb_command_len[ocf];
    cmd_complete_info.opcode = opcode;
    cmd_complete_info.status = NO_ERROR;

    if ((length == expected_length) || (expected_length == 0xFF ))
    {
        /*
         * Extract handle even though it may be invalid for command
         */
        p_payload+=3;
        handle = HCIparam_Get_Uint16(p_payload);
        cmd_complete_info.handle= handle;
#ifndef BLUETOOTH_MODE_LE_ONLY
        p_link = LMaclctr_Find_ACL_Handle(handle);
#endif
	XINC_TRACE_U32_PRINT(((length<<16) |opcode), TRACE_INDEX_8); 

        switch(opcode) 
        {
#ifndef BLUETOOTH_MODE_LE_ONLY
        case HCI_READ_PAGE_TIMEOUT :
            p_event_info->timeout = g_LM_config_info.page_timeout;
            break;

#if (PRH_BS_CFG_SYS_MODIFY_TIMERS_SUPPORTED==1)
        case HCI_WRITE_PAGE_TIMEOUT :
            if(HCIparam_Get_Timer(p_payload))
                g_LM_config_info.page_timeout = HCIparam_Get_Timer(p_payload);
            else
                p_event_info->status = INVALID_HCI_PARAMETERS;
            break;
#else
        case HCI_WRITE_PAGE_TIMEOUT:
            p_event_info->status = UNSUPPORTED_FEATURE;
            break;
#endif

        case HCI_READ_SCAN_ENABLE :
            p_event_info->mode = LMscan_Read_Scan_Enable();
            break;

        case HCI_WRITE_SCAN_ENABLE :
            if ( HCIparam_Get_Scan_Enable(p_payload) < 0x04)
                cmd_complete_info.status = LMscan_Write_Scan_Enable(HCIparam_Get_Scan_Enable(p_payload));
            else
                cmd_complete_info.status = INVALID_HCI_PARAMETERS;
           break;

        case HCI_READ_PAGE_SCAN_ACT :
        case HCI_READ_INQUIRY_SCAN_ACT : 
            /*
             * scan_flag (1st arg) below 0x00/0x01 -- Inquiry/Page Scan Activity
             */
            p_event_info->scan_activity =
                LMscan_Read_Scan_Activity((u_int8)(opcode == HCI_READ_PAGE_SCAN_ACT));
            break;

#if (PRH_BS_CFG_SYS_WRITE_SCAN_ACTIVITY_SUPPORTED==1)
        case HCI_WRITE_PAGE_SCAN_ACT :
        case HCI_WRITE_INQUIRY_SCAN_ACT :
            {
            u_int8 scan_flag = (opcode == HCI_WRITE_PAGE_SCAN_ACT);
            /*
             * scan_flag (1st arg) below 0x00/0x01 -- Inquiry/Page Scan Activity
             */
            t_scanActivity scan_activity;
            HCIparam_Get_Scan_Activity(p_payload, &scan_activity);

            cmd_complete_info.status = LMscan_Write_Scan_Activity(scan_flag, 
                                       &scan_activity);
           }
           break;
#else
        case HCI_WRITE_PAGE_SCAN_ACT:
        case HCI_WRITE_INQUIRY_SCAN_ACT:
            cmd_complete_info.status = UNSUPPORTED_FEATURE;
            break;
#endif

#if (PRH_BS_CFG_SYS_LMP_INTERLACED_INQUIRY_SCAN_SUPPORTED==1)
        case HCI_READ_INQUIRY_SCAN_TYPE:
            /*
             * Retrieve the complete scan_activity structure and then
             * only use the scan_type element when building the 
             * HCI_Event.
             */
            p_event_info->scan_activity =
                LMscan_Read_Scan_Activity(0 /* InquiryScan Activity*/);
            break;
#else
        case HCI_READ_INQUIRY_SCAN_TYPE:
            cmd_complete_info.status = UNSUPPORTED_FEATURE;
            break;
#endif

#if (PRH_BS_CFG_SYS_LMP_INTERLACED_INQUIRY_SCAN_SUPPORTED==1)
        case HCI_WRITE_INQUIRY_SCAN_TYPE:
            cmd_complete_info.status = 
                 LMscan_Write_Inquiry_Scan_Type(HCIparam_Get_Scan_Type(p_payload));
            break;
#else
        case HCI_WRITE_INQUIRY_SCAN_TYPE:
            cmd_complete_info.status = UNSUPPORTED_FEATURE;
            break;
#endif

#if (PRH_BS_CFG_SYS_LMP_INTERLACED_PAGE_SCAN_SUPPORTED==1)
        case HCI_READ_PAGE_SCAN_TYPE:
            /*
             * Retrieve the complete scan_activity structure and then
             * only use the scan_type element when building the 
             * HCI_Event.
             */
            p_event_info->scan_activity =
                LMscan_Read_Scan_Activity(1 /*Page Scan Activity*/);
            break;
#else
        case HCI_READ_PAGE_SCAN_TYPE:
            
            cmd_complete_info.status = UNSUPPORTED_FEATURE;
            break;
#endif

#if (PRH_BS_CFG_SYS_LMP_INTERLACED_PAGE_SCAN_SUPPORTED==1)
        case HCI_WRITE_PAGE_SCAN_TYPE:
            cmd_complete_info.status = 
                 LMscan_Write_Page_Scan_Type(HCIparam_Get_Scan_Type(p_payload));
            break;
#else
        case HCI_WRITE_PAGE_SCAN_TYPE:
            cmd_complete_info.status = UNSUPPORTED_FEATURE;
            break;
#endif

#if (PRH_BS_CFG_SYS_LMP_RSSI_INQUIRY_RESULTS_SUPPORTED==1)
        case HCI_READ_INQUIRY_MODE:
            p_event_info->number = LMinq_Get_Inquiry_Result_Mode();
            break;
#else
        case HCI_READ_INQUIRY_MODE:
            cmd_complete_info.status = UNSUPPORTED_FEATURE;
            break;
#endif

#if (PRH_BS_CFG_SYS_LMP_RSSI_INQUIRY_RESULTS_SUPPORTED==1)
        case HCI_WRITE_INQUIRY_MODE:
            if(*p_payload > 2) 
                cmd_complete_info.status = INVALID_HCI_PARAMETERS;
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_INQUIRY_RESPONSE_SUPPORTED==0)
            else if(*p_payload == 2)
                cmd_complete_info.status = UNSUPPORTED_FEATURE;
#endif
            else
                LMinq_Set_Inquiry_Result_Mode((t_inquiry_result_mode)*p_payload);
            break;
#else
        case HCI_WRITE_INQUIRY_MODE:
            cmd_complete_info.status = UNSUPPORTED_FEATURE;
            break;
#endif

        case HCI_READ_CONNECT_ACCEPT_TO :
            p_event_info->timeout =  g_LM_config_info.accept_timeout;
            break;

#if (PRH_BS_CFG_SYS_MODIFY_TIMERS_SUPPORTED==1)
        case HCI_WRITE_CONNECT_ACCEPT_TO :
            {
            t_slots slots = HCIparam_Get_Timer(p_payload);
            if ((slots <= 0xB540) && (slots != 0))
                g_LM_config_info.accept_timeout = HCIparam_Get_Timer(p_payload);
            else
                p_event_info->status = INVALID_HCI_PARAMETERS;
            }
            break;
#else
        case HCI_WRITE_CONNECT_ACCEPT_TO:
            p_event_info->status = UNSUPPORTED_FEATURE;
            break;
#endif
#endif//BLUETOOTH_MODE_LE_ONLY
        case HCI_RESET :
            BTms_Request_HC_Reset();
            break;
		case HCI_READ_LOCAL_NAME :
			LMconfig_LM_Read_Local_Name(&cmd_complete_info);
			break;

	
#if (PRH_BS_CFG_SYS_SET_EVENT_MASK_SUPPORTED==1)
        case HCI_SET_EVENT_MASK :
            HCeg_Set_Event_Mask(p_payload);
            break;
#else
        case HCI_SET_EVENT_MASK :
			return ILLEGAL_COMMAND; 
			break;
#endif
#ifndef BLUETOOTH_MODE_LE_ONLY		
        case HCI_SET_EVENT_FILTER :
#if (PRH_BS_CFG_SYS_FILTERS_SUPPORTED==1)
            {
            u_int8 filter_type;
            t_filter filter;
            u_int8 filter_condition_type;

            filter_type = *p_payload++;
            filter_condition_type = *p_payload++;
            filter.filter_condition_type = filter_condition_type;
            switch(filter_type)
            {
            case 0x00 : /* Clear all Filters */
                break;

            case 0x01 : /* Inquiry Filters */ 
                switch(filter_condition_type)
                {
                case 0x00 : /* Not Used */
                    break;
                case 0x01 : /* specific Class of Device */
                    filter.class_of_device = HCIparam_Get_Uint24(p_payload);
                    filter.class_of_device_mask = HCIparam_Get_Uint24(p_payload+3); 
                    break;
                case 0x02 :  /* Specific BD_ADDR */
                    HCIparam_Get_Bd_Addr(p_payload,&filter.bd_addr);
                    break;
                default :
                    p_event_info->status = INVALID_HCI_PARAMETERS;
                    break;
                }
                break;
            case 0x02 :
                switch(filter_condition_type)
                {
                case 0x00 : /* Auto Accept ONLY */
                    if(*p_payload < 0x04)
                        filter.auto_accept = *p_payload;
                    else
                        p_event_info->status = INVALID_HCI_PARAMETERS;
                    break;
                case 0x01 :
                    if((*(p_payload+6)) < 0x04)
                    {
                        filter.class_of_device = HCIparam_Get_Uint24(p_payload);
                        filter.class_of_device_mask = HCIparam_Get_Uint24(p_payload+3); 
                        filter.auto_accept = *(p_payload+6);
                    }
                    else
                        p_event_info->status = INVALID_HCI_PARAMETERS;
                    break;
                case 0x02 :
                    if((*(p_payload+6)) < 0x04)
                    {
                        HCIparam_Get_Bd_Addr(p_payload,&filter.bd_addr);
                        filter.auto_accept = *(p_payload+6);
                    }
                    else
                        p_event_info->status = INVALID_HCI_PARAMETERS;
                    break;
                default :
                    p_event_info->status = INVALID_HCI_PARAMETERS;
                    break;
                }
                break;
            default :
                p_event_info->status = INVALID_HCI_PARAMETERS;
                break;
            }
            if (p_event_info->status == NO_ERROR)
            {
                p_event_info->status = LMfltr_LM_Set_Filter(filter_type, &filter);
            }
        }
#else
            p_event_info->status = UNSUPPORTED_FEATURE;
#endif
            break;

#if (PRH_BS_CFG_SYS_CHANGE_LOCAL_NAME_SUPPORTED==1)
        case HCI_CHANGE_LOCAL_NAME :
            LMconfig_LM_Change_Local_Name(p_payload);
            break;
#else
        case HCI_CHANGE_LOCAL_NAME:
            p_event_info->status = UNSUPPORTED_FEATURE;
            break;
#endif


        case HCI_FLUSH :
#if(PRH_BS_CFG_SYS_FLUSH_SUPPORTED==1)
            if (p_link != 0)
                cmd_complete_info.status = LMpol_HCI_Flush(p_link);
            else
                cmd_complete_info.status  = NO_CONNECTION;
#else
            cmd_complete_info.status = UNSUPPORTED_FEATURE;
#endif
            break;

		case HCI_ENHANCED_FLUSH :
#if(PRH_BS_CFG_SYS_LMP_NONFLUSHABLE_PBF_SUPPORTED==1)
            if (p_link != 0)
			{
                LMpol_HCI_Enhanced_Flush(p_link);
			}
            else
			{
				HCeg_Command_Status_Event(NO_CONNECTION,HCI_ENHANCED_FLUSH);
			}
#else
			HCeg_Command_Status_Event(UNSUPPORTED_FEATURE,HCI_ENHANCED_FLUSH);
#endif
            break;

        case HCI_READ_PIN_TYPE :
            p_event_info->mode = g_LM_config_info.pin_type;
            break;

        case HCI_WRITE_PIN_TYPE :
            if(*p_payload > 1) 
                cmd_complete_info.status = INVALID_HCI_PARAMETERS;
            else
                g_LM_config_info.pin_type = (t_pin_type)*p_payload;
            break;

#if (PRH_BS_CFG_SYS_CREATE_UNIT_KEY_SUPPORTED==1) && (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED==1)
        case HCI_CREATE_NEW_UNIT_KEY :
            if(COMBINATION_KEY_DEVICE != g_LM_config_info.key_type)
                LMsec_upper_LM_Create_New_Unit_Key(); 
            else
                cmd_complete_info.status = UNSUPPORTED_FEATURE;
            break;
#else
 	         case HCI_CREATE_NEW_UNIT_KEY:
 	            return ILLEGAL_COMMAND;
 	            break;
#endif

#if (PRH_BS_CFG_SYS_LINK_KEY_DATABASE_SUPPORTED==1)
        case HCI_READ_STORED_LINK_KEY :
            {
            t_bd_addr  bd_addr;
            HCIparam_Get_Bd_Addr(p_payload, &bd_addr);
            LMkeydb_LM_Read_Stored_Link_Key(&bd_addr,p_payload[6],&cmd_complete_info);
            }
            break;

       case HCI_WRITE_STORED_LINK_KEY :
           {
               u_int8 num_keys = *p_payload;
               cmd_complete_info.number = 0;
               /* Check the Num Keys Range */
               if ((num_keys == 0)  || (num_keys > MAX_NUM_LINK_KEYS_TO_WRITE ))
               {
                   cmd_complete_info.status = INVALID_HCI_PARAMETERS;
               }
               else if(length != (num_keys * 22) +1)
               {
                   cmd_complete_info.status = INVALID_HCI_PARAMETERS;
               }
               else
               {   
                   int i;
                   u_int8  num_keys_written = 0;
                   t_bd_addr  bd_addr;
                 
                   if (num_keys > PRH_BS_CFG_SYS_MAX_STORED_LINK_KEYS)
                       num_keys = PRH_BS_CFG_SYS_MAX_STORED_LINK_KEYS;
                   for(i = 0; i < num_keys; i++)
                   {
                      HCIparam_Get_Bd_Addr(p_payload+1, &bd_addr);
                      LMkeydb_LM_Write_Stored_Link_Key(/*num_keys,*/
                      &bd_addr, (p_payload+7), &cmd_complete_info);  
                      if (cmd_complete_info.number != 1)     
                          break;
                      p_payload+= 22;
                      num_keys_written++;
                   }
                   cmd_complete_info.number = num_keys_written; 
               }
           }
           break;

        case HCI_DELETE_STORED_LINK_KEY :
            {
            t_bd_addr  bd_addr;
            HCIparam_Get_Bd_Addr(p_payload,&bd_addr);
            if(p_payload[6] > 1)
            {
                cmd_complete_info.status = INVALID_HCI_PARAMETERS;
                cmd_complete_info.value16bit = 0;
            }
            else
                LMkeydb_LM_Delete_Stored_Link_Key(&bd_addr,
                                                  p_payload[6],&cmd_complete_info);
            }
            break;
#else
        case HCI_READ_STORED_LINK_KEY :
            p_event_info->returnParams.readStoredLinkKey.max_num_keys = 0;
            p_event_info->returnParams.readStoredLinkKey.num_keys_read = 0;
            p_event_info->status = UNSUPPORTED_FEATURE;
            break;
        case HCI_WRITE_STORED_LINK_KEY :
            p_event_info->number = 0;
            p_event_info->status = UNSUPPORTED_FEATURE;
            break;
        case HCI_DELETE_STORED_LINK_KEY :
            cmd_complete_info.value16bit = 0;
            cmd_complete_info.status = UNSUPPORTED_FEATURE;
            break;
#endif

        case HCI_WRITE_AUTHENTICATION_ENABLE :
            if (*p_payload < 0x02)
                g_LM_config_info.authentication_enable = (t_flag)*p_payload;
            else
                cmd_complete_info.status = INVALID_HCI_PARAMETERS;
            break;

        case HCI_READ_AUTHENTICATION_ENABLE :
            p_event_info->mode = (u_int8)g_LM_config_info.authentication_enable;
            break;
        case HCI_READ_ENCRYPTION_MODE :
            p_event_info->mode = g_LM_config_info.encryption_mode;
            break;

#if (PRH_BS_CFG_SYS_LMP_ENCRYPTION_SUPPORTED== 1)
        case HCI_WRITE_ENCRYPTION_MODE :
            if (*p_payload < 0x02) /* Change based on errata */
                g_LM_config_info.encryption_mode = (t_encrypt_mode)*p_payload;
            else
                cmd_complete_info.status = INVALID_HCI_PARAMETERS;
            break;
#else
        case HCI_WRITE_ENCRYPTION_MODE:
            cmd_complete_info.status = UNSUPPORTED_FEATURE;
            break;
#endif

        case HCI_READ_CLASS_OF_DEVICE :
            p_event_info->returnParams.readClassDevice.cod = LC_Read_Local_Device_Class();
            break;

        case HCI_WRITE_CLASS_OF_DEVICE :
            SYSconfig_Set_Device_Class(HCIparam_Get_Uint24(p_payload));
            LC_Write_Local_Device_Class(HCIparam_Get_Uint24(p_payload)); 
            cmd_complete_info.status = NO_ERROR;
            break;

        case HCI_READ_SUPERVISION_TIMEOUT :
            if (p_link != 0)  
                LMqos_LM_Read_Supervision_Timeout(p_link,&cmd_complete_info);
            else
            {
                cmd_complete_info.status = NO_CONNECTION;
                cmd_complete_info.timeout = 0;
            }
            break;

#if (PRH_BS_CFG_SYS_MODIFY_TIMERS_SUPPORTED==1)
        case HCI_WRITE_SUPERVISION_TIMEOUT :
            if (p_link != 0)
            {
                u_int16 timeout = HCIparam_Get_Uint16(p_payload+2);
                if (LMqos_LM_Write_Supervision_Timeout(p_link, timeout,
                        &cmd_complete_info) == NO_ERROR)
                {
                    p_link->link_supervision_timeout = timeout;
                }
            }
            else
                cmd_complete_info.status  = NO_CONNECTION;
            break;
#else
        case HCI_WRITE_SUPERVISION_TIMEOUT:
            cmd_complete_info.status = UNSUPPORTED_FEATURE;
            break;
#endif

        case HCI_READ_NUMBER_OF_SUPPORTED_IAC :
            cmd_complete_info.number = PRH_BS_CFG_SYS_NUMBER_OF_SUPPORTED_IAC;
            break;

#if (PRH_BS_CFG_SYS_INQUIRY_READ_WRITE_IAC_LAP_SUPPORTED==1)
        case HCI_WRITE_CURRENT_IAC_LAP :
            if(*p_payload < 1)
                cmd_complete_info.status = INVALID_HCI_PARAMETERS;
            else if(HCbound_Check_IAC_LAPs(*p_payload, p_payload+1))
                cmd_complete_info.status = INVALID_HCI_PARAMETERS;
            else
            {
                if(*p_payload > PRH_BS_CFG_SYS_NUMBER_OF_SUPPORTED_IAC)
                {
                    *p_payload = PRH_BS_CFG_SYS_NUMBER_OF_SUPPORTED_IAC;
                }
                cmd_complete_info.status = 
                    LMscan_LM_Write_Supported_IAC(*p_payload, p_payload+1);
            }
            break;

        case HCI_READ_CURRENT_IAC_LAP :
            {
            cmd_complete_info.p_u_int8 = p_iacs;
            cmd_complete_info.status =
                LMscan_LM_Read_Supported_IAC(&cmd_complete_info.number, p_iacs);
            }
            break;
#else
        case HCI_WRITE_CURRENT_IAC_LAP:
            cmd_complete_info.status = UNSUPPORTED_FEATURE;
        break;

        case HCI_READ_CURRENT_IAC_LAP: /* always support reading back the GIAC */
            {
            cmd_complete_info.p_u_int8 = iac_lap;
            cmd_complete_info.status = NO_ERROR;
            cmd_complete_info.number = 1;
            }
        break;
#endif

#if (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED==1)
        case HCI_READ_PAGE_SCAN_PERIOD_MODE :
            cmd_complete_info.mode = LC_Read_Local_Page_Scan_Period();
            break;

        case HCI_WRITE_PAGE_SCAN_PERIOD_MODE :
            /* Writes the page scan period mode into the local FHS - P0,P1,P2 */
            sp_mode = *p_payload;
            if (sp_mode < 0x03)
                LC_Write_Local_Page_Scan_Period((t_pageScanPeriod)sp_mode);
            else
                cmd_complete_info.status = INVALID_HCI_PARAMETERS;
            break;
#endif

        case HCI_READ_PAGE_SCAN_MODE :
            cmd_complete_info.mode = (u_int8)LC_Read_Local_Page_Scan_Mode();
            break;

        case HCI_WRITE_PAGE_SCAN_MODE :
#if(PRH_BS_CFG_SYS_LMP_OPTIONAL_PAGING_SUPPORTED==1)
            sp_mode = *p_payload;
            if (sp_mode < 0x04)
                LC_Write_Local_Page_Scan_Mode((t_pageScanMode)(sp_mode));
            else
                cmd_complete_info.status = INVALID_HCI_PARAMETERS;
#else
            sp_mode = *p_payload;
            if (sp_mode == 0)
                LC_Write_Local_Page_Scan_Mode((t_pageScanMode)(sp_mode));
            else
                cmd_complete_info.status = UNSUPPORTED_FEATURE;
#endif
            break;

        case HCI_READ_NUM_BROADCAST_RETRANSMISSIONS :
            p_event_info->number = (g_LM_config_info.Nbc-1);
            break;

        case HCI_WRITE_NUM_BROADCAST_RETRANSMISSIONS :
            /*
             * The maximum retransmissions must be less than max 
             * Nbc (=0xFF, where Nbc is the number of transmissions)
             */
            if ((*p_payload) < 0xFF)
                g_LM_config_info.Nbc = ((u_int8)*p_payload+1);
            else
                cmd_complete_info.status = INVALID_HCI_PARAMETERS;
            break;

#if ((PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1) || (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1))
        case HCI_READ_VOICE_SETTINGS :
            p_event_info->value16bit = (g_LM_config_info.default_voice_setting & 0x7FFF);
        break;

        case HCI_WRITE_VOICE_SETTINGS :

            if(LC_Is_Valid_SCO_Conversion(HCIparam_Get_Uint16(p_payload)))
            {
                g_LM_config_info.default_voice_setting = HCIparam_Get_Uint16(p_payload);
            }
            else
                cmd_complete_info.status = INVALID_HCI_PARAMETERS;

        break;
#else
        case HCI_READ_VOICE_SETTINGS :
            cmd_complete_info.value16bit = 0;
        case HCI_WRITE_VOICE_SETTINGS :
            cmd_complete_info.status = UNSUPPORTED_FEATURE;
            break;
#endif

        case HCI_READ_AUTOMATIC_FLUSH_TIMEOUT :
            if (p_link != 0)
                cmd_complete_info.timeout = p_link->flush_timeout;
            else
            {
                cmd_complete_info.status  = NO_CONNECTION;
                cmd_complete_info.timeout = 0;
            }
            break;

#if (PRH_BS_CFG_SYS_FLUSH_SUPPORTED==1 && PRH_BS_CFG_SYS_MODIFY_TIMERS_SUPPORTED==1)
        case HCI_WRITE_AUTOMATIC_FLUSH_TIMEOUT :
            if (p_link != 0)
            {
                if(HCIparam_Get_Uint16(p_payload+2) > 0x7FF)
                    cmd_complete_info.status = INVALID_HCI_PARAMETERS;
                else
                    cmd_complete_info.status =
                     LMpolicy_Write_Flush_Timeout(p_link,HCIparam_Get_Uint16(p_payload+2));
            }
            else
                cmd_complete_info.status  = NO_CONNECTION ;
            break;
#else
        case HCI_WRITE_AUTOMATIC_FLUSH_TIMEOUT :
            cmd_complete_info.status = UNSUPPORTED_FEATURE;
            break;
#endif

        case HCI_READ_HOLD_MODE_ACTIVITY :
            cmd_complete_info.mode = g_LM_config_info.hold_mode_activity;
            break;

        case HCI_WRITE_HOLD_MODE_ACTIVITY :
            if (*(p_payload) < 0x08)
                g_LM_config_info.hold_mode_activity = (u_int8)(*(p_payload));
            else
                cmd_complete_info.status = INVALID_HCI_PARAMETERS;
            break;

#if (PRH_BS_CFG_SYS_LMP_POWER_CONTROL_SUPPORTED==1)
        case HCI_READ_TRANSMIT_POWER_LEVEL :
            if (p_link != 0)
            {
                LMlc_LM_Read_Transmit_Power_Level(p_link,*(p_payload+2),&cmd_complete_info);
            }
            else
            {
            	t_LE_Connection* p_connection = LEconnection_Find_P_Connection(handle);

            	if (p_connection)
            	{
				    cmd_complete_info.number = ((s_int8 (*)(u_int8))PATCH_FUN[HWRADIO_CONVERT_TX_POWER_LEVEL_UNITS_TO_TX_POWER])(MAX_POWER_LEVEL_UNITS);
            		//cmd_complete_info.number = HWradio_Convert_Tx_Power_Level_Units_to_Tx_Power(MAX_POWER_LEVEL_UNITS);
            	}
            	else
            	{
            		cmd_complete_info.number = 0;
            		cmd_complete_info.status = NO_CONNECTION;
            	}
            }
            break;
#else
        case HCI_READ_TRANSMIT_POWER_LEVEL :
            cmd_complete_info.status = UNSUPPORTED_FEATURE;
            cmd_complete_info.number = 0;
            break;
#endif

#if(PRH_BS_CFG_SYS_SYNCHRONOUS_CONNECTIONS_SUPPORTED==1)
        case HCI_READ_SYNCHRONOUS_FLOW_CONTROL_ENABLE:
            cmd_complete_info.number = HCfc_Read_SYN_Flow_Control_Enable();
            break;
        case HCI_WRITE_SYNCHRONOUS_FLOW_CONTROL_ENABLE :
            cmd_complete_info.status = 
                HCfc_Write_SYN_Flow_Control_Enable(*p_payload);
            break;
#else
        case HCI_READ_SYNCHRONOUS_FLOW_CONTROL_ENABLE:
            cmd_complete_info.number = 0;  /*Report SCO flow disabled */
            cmd_complete_info.status = UNSUPPORTED_FEATURE;
            break;
        case HCI_WRITE_SYNCHRONOUS_FLOW_CONTROL_ENABLE :
            cmd_complete_info.status = UNSUPPORTED_FEATURE;
            break;
#endif
#endif
#if (PRH_BS_CFG_SYS_HC_TO_HOST_FLOW_CONTROL_SUPPORTED==1)
        case HCI_SET_HOST_CONTROLLER_TO_HOST_FLOW_CONTROL :
            cmd_complete_info.status = 
                HCfc_Set_Host_Controller_To_Host_Flow_Control(*p_payload);
            break;
        case HCI_HOST_NUMBER_OF_COMPLETED_PACKETS :
            cmd_complete_info.status = HCfc_Host_Number_Of_Completed_Packets(p_payload);
            break;

        case HCI_HOST_BUFFER_SIZE :
            /*
             * Extract and Set the Host ACL/SCO Buffer Sizes
             */
            HCfc_Host_Buffer_Size(p_payload);
            cmd_complete_info.status = NO_ERROR;
            break;
#else
        case HCI_SET_HOST_CONTROLLER_TO_HOST_FLOW_CONTROL:
        case HCI_HOST_NUMBER_OF_COMPLETED_PACKETS:
        case HCI_HOST_BUFFER_SIZE:
            cmd_complete_info.status = UNSUPPORTED_FEATURE;
            break;
#endif

#if (PRH_BS_CFG_SYS_LE_CONTROLLER_SUPPORTED==1)
		case HCI_READ_LE_HOST_SUPPORT :
            // Reads the feature bits of the LMP for the LE
#if 0 // Removed as read directly in the generation of the command_complete event
	  // no point tramping these all over the place.
			p_event_info->mode = g_LM_config_info.le_host_supported;
			p_event_info->number = g_LM_config_info.le_and_bredr_host;
#endif
			cmd_complete_info.status = NO_ERROR;
			break;

		case HCI_WRITE_LE_HOST_SUPPORT :
		    if (*p_payload < 0x02)
			{
		    	g_LM_config_info.le_host_supported = (u_int8)*p_payload;
			}

		    if (*(p_payload+1) < 0x02)
			{
		    	g_LM_config_info.le_and_bredr_host = (u_int8)*(p_payload+1);
			}
			break;
#endif

        default:
            return ILLEGAL_COMMAND;
        }
    }
    else
    {
        return INVALID_HCI_PARAMETERS; 
    }

    if ((opcode != HCI_RESET) && (opcode != HCI_ENHANCED_FLUSH) && ((opcode != HCI_HOST_NUMBER_OF_COMPLETED_PACKETS)
           || (cmd_complete_info.status != NO_ERROR)))
    {
        HCeg_Command_Complete_Event(&cmd_complete_info);
    }
    return NO_ERROR;
}

#if (PRH_BS_CFG_SYS_INQUIRY_READ_WRITE_IAC_LAP_SUPPORTED==1)   
boolean HCbound_Check_IAC_LAPs(u_int8 num_iac_laps, u_int8* iac_laps)
{
    boolean result = 1;
    int i;

    if(num_iac_laps > PRH_BS_CFG_SYS_NUMBER_OF_SUPPORTED_IAC)
    {
        num_iac_laps = PRH_BS_CFG_SYS_NUMBER_OF_SUPPORTED_IAC;
    }

    for(i = 0; i < num_iac_laps; i++)
    {
        result = HCbound_Check_IAC_LAP(iac_laps+(i*3));
        if(result)
            break;
    }
    return result;
}

boolean HCbound_Check_IAC_LAP(u_int8* iac_lap)
{
    u_int32 lap = *iac_lap + ((*(iac_lap+1))<<8) + ((*(iac_lap+2))<<16);
    return((lap > 0x9e8b3f) || (lap < 0x9e8b00))?1:0;
}
#endif
#if 1 //(LE_INCLUDED == 1)
/*****************************************************************
 *  FUNCTION :- _Dispatch_LE_Command
 *
 *  DESCRIPTION 
 *  Handles the HCI LE commands
 *  
 *****************************************************************/ 

t_error _Dispatch_LE_Command(t_p_pdu p_payload, u_int16 opcode)
{
	
	 
    if(PATCH_FUN[_DISPATCH_LE_COMMAND_ID]){
         
         return ((t_error (*)(t_p_pdu p_payload, u_int16 opcode))PATCH_FUN[_DISPATCH_LE_COMMAND_ID])(p_payload,   opcode);
    }

    t_cmd_complete_event cmd_complete_info;
    t_cmd_complete_event* p_event_info = &cmd_complete_info;
    u_int8 length;
    u_int16 ocf;
	t_error status = NO_ERROR;
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1) && ((PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1))
#if (PRH_BS_CFG_SYS_LE_SMP_INCLUDED==0)
    u_int8 temp_byte_array[16];
#endif
#endif
    p_event_info->opcode = opcode;
    p_event_info->status = NO_ERROR;

    length = HCIparam_Get_Length(p_payload+2);

    /*
     * Note Must ensure that the OCF does not extend beyond the 
     * bounds of length array
     *
     */

	p_payload+=3;

    ocf = ( opcode & 0x03FF);

    if ( ocf > MAX_HC_LE_COMMAND_OPCODE)
        return ILLEGAL_COMMAND;

    if (length == le_command_len[ocf] )
    {
        switch(opcode) 
        {
        case HCI_LE_SET_EVENT_MASK :
			LEconfig_Set_Event_Mask(p_payload);
            break;

        case HCI_LE_READ_BUFFER_SIZE :
            p_event_info->returnParams.readBufferSize.p_hc_buffer_size =
                SYSconfig_Get_HC_Buffer_Size_Ref();
            break;

/*#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1)*/
/*- UPDATE FOR BLE QUALIFY. */

        case HCI_LE_READ_LOCAL_SUPPORTED_FEATURES :
			p_event_info->p_u_int8 = LEconfig_Get_LE_Features_Ref(); 
            break;
/*#endif*/

#if (PRH_BS_CFG_SYS_LE_GAP_INCLUDED==0)
        case HCI_LE_SET_RANDOM_ADDRESS :
			LEconfig_Set_Random_Address(p_payload);
            break;
#endif

#if (PRH_BS_CFG_SYS_LE_BROADCAST_DEVICE == 1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE == 1)
        case HCI_LE_SET_ADVERTISING_PARAMETERS :
			cmd_complete_info.status = LEadv_Set_Advertising_Params(p_payload);
            break;

        case HCI_LE_READ_ADVERTISING_CHANNEL_TX_POWER :
			{
			    s_int8 number = LEadv_Get_Advertising_Channel_Tx_Power();
				p_event_info->number = (u_int8)number;
			}
			cmd_complete_info.status = NO_ERROR;
			break;

        case HCI_LE_SET_ADVERTISING_DATA :
			cmd_complete_info.status = LEadv_Set_Advertising_Data((u_int8)*p_payload,p_payload+1);
            break;

        case HCI_LE_SET_ADVERTISE_ENABLE :
			cmd_complete_info.status = LEadv_Set_Advertise_Enable((u_int8)*p_payload);
            break;


        case HCI_LE_SET_SCAN_RESPONSE_DATA :
			cmd_complete_info.status = LEadv_Set_Scan_Response_Data((u_int8)*p_payload,p_payload+1);
            break;
//#endif

#if (PRH_BS_CFG_SYS_LE_GAP_INCLUDED==0)
#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE == 1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1)
        case HCI_LE_SET_SCAN_PARAMETERS :
			cmd_complete_info.status = LEscan_Set_Scan_Parameters(p_payload);
            break;

        case HCI_LE_SET_SCAN_ENABLE :
			cmd_complete_info.status = LEscan_Set_Scan_Enable((u_int8)*p_payload, (u_int8)(*(p_payload+1)));
            break;
#endif
#endif

#if (PRH_BS_CFG_SYS_LE_GAP_INCLUDED==0)
#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1)
        case HCI_LE_CREATE_CONNECTION :
			status = LEconnection_Create_Connection(p_payload);
            break;

        case HCI_LE_CREATE_CONNECTION_CANCEL :
			status = LEconnection_Create_Connection_Cancel(p_payload);
            break;
#if (PRH_BS_CFG_SYS_LE_GAP_INCLUDED==0)
        case HCI_LE_CONNECTION_UPDATE :
			status = LEconnection_HCI_Connection_Update(p_payload);
            break;
#endif
#endif
#endif

        case HCI_LE_READ_WHITE_LIST_SIZE :
			p_event_info->number = LEwl_Read_White_List_Size();
			cmd_complete_info.status = NO_ERROR;
            break;

        case HCI_LE_CLEAR_WHITE_LIST :
//#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)

/*- UPDATE FOR BLE QUALIFY. */

			if (LEwl_Can_I_Modify_White_Lists())
				cmd_complete_info.status = LEwl_Clear_White_List();
			else
//#endif
				cmd_complete_info.status = COMMAND_DISALLOWED;
            break;

        case HCI_LE_ADD_DEVICE_TO_WHITE_LIST :
//#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
			if (LEwl_Can_I_Modify_White_Lists())
				cmd_complete_info.status = LEwl_Add_Device_To_White_List((u_int8)*p_payload,(p_payload+1));
			else
//#endif
				cmd_complete_info.status = COMMAND_DISALLOWED;
            break;

        case HCI_LE_REMOVE_DEVICE_FROM_WHITE_LIST :
//#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
			if (LEwl_Can_I_Modify_White_Lists())
				cmd_complete_info.status = LEwl_Remove_Device_From_White_List((u_int8)*p_payload,(p_payload+1));
			else
//#endif
				cmd_complete_info.status = COMMAND_DISALLOWED;
            break;
            /*break;*/

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1)
        case HCI_LE_SET_HOST_CHANNEL_CLASSIFICATION :
			cmd_complete_info.status = LEfreq_Set_Host_Channel_Classificiation(p_payload);
            break;
#endif

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
        case HCI_LE_READ_CHANNEL_MAP :
			{
				u_int16 handle = HCIparam_Get_Uint16(p_payload);

				if (LEconnection_Is_Valid_Handle(handle))
				{
					cmd_complete_info.p_u_int8 = LEfreq_Read_Channel_Map(handle);
					cmd_complete_info.status = NO_ERROR;
				}

				cmd_complete_info.handle = handle;
			}
            break;
#endif
#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1)
        case HCI_LE_READ_REMOTE_USED_FEATURES :
			{
				u_int16 handle = HCIparam_Get_Uint16(p_payload);

				if (LEconnection_Is_Valid_Handle(handle))
				{
					status = LEconnection_Read_Remote_Used_Features(handle);
				}
			}
            break;
#endif
#endif
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1) && ((PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1))
#if (PRH_BS_CFG_SYS_LE_SMP_INCLUDED==0)
        case HCI_LE_ENCRYPT :
			{
				if (!LEconfig_Is_Features_Encryption_Supported())
				{
					cmd_complete_info.status = UNSUPPORTED_FEATURE;
				}
				else
				{
					LEsecurity_Encrypt(p_payload, temp_byte_array);
					cmd_complete_info.p_u_int8 = temp_byte_array;
					cmd_complete_info.status = NO_ERROR;

				}
			}
            break;

        case HCI_LE_RAND :
			if (!LEconfig_Is_Features_Encryption_Supported())
			{
				cmd_complete_info.status = UNSUPPORTED_FEATURE;
			}
			else
			{
				LEsecurity_Rand(temp_byte_array);
				cmd_complete_info.p_u_int8 = temp_byte_array;
				cmd_complete_info.status = NO_ERROR;
			}
            break;
#endif
#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1)
#if (PRH_BS_CFG_SYS_LE_SMP_INCLUDED==0)
		case HCI_LE_START_ENCRYPTION :
			{
				u_int16 handle = HCIparam_Get_Uint16(p_payload);

				if (!LEconfig_Is_Features_Encryption_Supported())
					status = UNSUPPORTED_FEATURE;
				else
				{
					u_int8 i;
					u_int8 random[8];
					u_int8 ltk[16];
					u_int16 encry_div;
				
					p_payload+=2;

					for (i=0; i<8; i++)
					{
						random[i] = p_payload[i];
					}	
					p_payload+=8;
					encry_div = HCIparam_Get_Uint16(p_payload);
					p_payload+=2;

					for (i=0; i<16; i++)
					{
						ltk[i] = p_payload[i];
					}
					status = LEsecurity_Start_Encryption(handle,random,encry_div,ltk);
				}

			}
			break;
#endif
#endif

#if (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
#if (PRH_BS_CFG_SYS_LE_SMP_INCLUDED==0)
		case HCI_LE_LONG_TERM_KEY_REQUEST_REPLY :
		case HCI_LE_LONG_TERM_KEY_REQUEST_NEGATIVE_REPLY :
			{
				u_int16 handle = HCIparam_Get_Uint16(p_payload);
				
				if (!LEconfig_Is_Features_Encryption_Supported())
					cmd_complete_info.status = UNSUPPORTED_FEATURE;
				else
				{
					u_int8 link_id;
					t_LE_Connection* p_connection;

					if (NO_ERROR != LEconnection_Find_Link_Id(handle,&link_id))
					{
						cmd_complete_info.status = NO_CONNECTION;
					}
					else
					{
						p_connection = LEconnection_Find_Link_Entry(link_id);
						if (p_connection->role == MASTER)
						{
							cmd_complete_info.status = COMMAND_DISALLOWED;
						}
						else
						{
							if (opcode==HCI_LE_LONG_TERM_KEY_REQUEST_REPLY)
							{
								u_int8* p_long_term_key = (p_payload+2);
								cmd_complete_info.status  = LEsecurity_Long_Term_Key_Request_Reply(p_connection,p_long_term_key);
							}
							else
								cmd_complete_info.status  = LEsecurity_Long_Term_Key_Request_Negative_Reply(p_connection);
						}
					}
				}
			}
			break;

#endif
#endif
#else        
			case HCI_LE_ENCRYPT :
			{
				/*if (!LEconfig_Is_Features_Encryption_Supported())
				{
					cmd_complete_info.status = UNSUPPORTED_FEATURE;
				}
				else*/
				{
					u_int8 temp_byte_array[16];
					LEsecurity_Encrypt(p_payload, temp_byte_array);
					cmd_complete_info.p_u_int8 = temp_byte_array;
					cmd_complete_info.status = NO_ERROR;

				}
			}
            break;

			case HCI_LE_RAND :
				/*if (!LEconfig_Is_Features_Encryption_Supported())
				{
					cmd_complete_info.status = UNSUPPORTED_FEATURE;
				}
				else*/
				{
					u_int8 temp_byte_array[16];
					LEsecurity_Rand(temp_byte_array);
					cmd_complete_info.p_u_int8 = temp_byte_array;
					cmd_complete_info.status = NO_ERROR;
				}
	break;

#endif
		case HCI_LE_READ_SUPPORTED_STATES :
			cmd_complete_info.p_u_int8 = LEconfig_Read_Supported_States();
			cmd_complete_info.status = NO_ERROR;
			break;

#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
		case HCI_LE_RECEIVER_TEST :
			if ((LE_config.num_le_links) || (LEadv_Get_Advertising_Enable()) || 
				(LEscan_Get_Scan_Enable()) || (LE_config.next_initiator_scan_timer != BT_TIMER_OFF))
			{
				cmd_complete_info.status = COMMAND_DISALLOWED;
			}
			else
			{
				//cmd_complete_info.status = LEtest_Receiver_Test((u_int8)*p_payload);
				cmd_complete_info.status = ((t_error (*)(u_int8))PATCH_FUN[LETEST_RECEIVER_TEST])((u_int8)*p_payload);

			}
			break;
#endif

#if (PRH_BS_CFG_SYS_LE_BROADCAST_DEVICE==1)
		case HCI_LE_TRANSMITTER_TEST :
			if ((LE_config.num_le_links) || (LEadv_Get_Advertising_Enable()) || 
				(LEscan_Get_Scan_Enable()) || (LE_config.next_initiator_scan_timer != BT_TIMER_OFF))
			{
				cmd_complete_info.status = COMMAND_DISALLOWED;
			}
			else
			{
//				cmd_complete_info.status = LEtest_Transmitter_Test((u_int8)*p_payload,
//															   (u_int8)*(p_payload+1),
//															   (u_int8)*(p_payload+2));
               cmd_complete_info.status = ((t_error (*)(u_int8,u_int8,u_int8))PATCH_FUN[LETEST_TRANSMITTER_TEST])((u_int8)*p_payload,
															                            (u_int8)*(p_payload+1),
															                            (u_int8)*(p_payload+2));
			}
			break;
#endif

		case HCI_LE_TEST_END :
			if (LE_config.test_mode != LE_TEST_MODE_ACTIVE)
			{
				cmd_complete_info.status = COMMAND_DISALLOWED;
			}
			else
			{
				//cmd_complete_info.value16bit = LEtest_Test_End();
				cmd_complete_info.value16bit = ((u_int16 (*)(void))PATCH_FUN[LETEST_TEST_END])();
				
				cmd_complete_info.status = NO_ERROR;
			}
			break;

		default:
			status = COMMAND_DISALLOWED;
			cmd_complete_info.status = COMMAND_DISALLOWED;
			break;

		}

	}    
	else
    {
        return INVALID_HCI_PARAMETERS; 
    }
#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1)
    if ((opcode == HCI_LE_CREATE_CONNECTION) ||
    	(opcode == HCI_LE_CONNECTION_UPDATE) ||
    	(opcode == HCI_LE_READ_REMOTE_USED_FEATURES) ||
    	(opcode == HCI_LE_START_ENCRYPTION))
    {
    	HCeg_Command_Status_Event(status,opcode);
    }
    else
#endif
    	if (opcode != HCI_LE_CREATE_CONNECTION_CANCEL)
    	HCeg_Command_Complete_Event(&cmd_complete_info);
    return NO_ERROR;
}
#endif

#ifndef BLUETOOTH_MODE_LE_ONLY
static void _Send_HCI_Encryption_Change_Event_Deffered(t_lmp_link* p_link)
{
	LMsec_upper_Send_HCI_Event(p_link, HCI_ENCRYPTION_CHANGE_EVENT, NO_ERROR);
}
#endif

