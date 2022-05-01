/********************************************************************
 * MODULE NAME:    lmp_cmd_disp.c
 * PROJECT CODE:   BlueStream
 * AUTHOR:         Gary Fleming
 * DATE:           01-08-1999
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * SOURCE CONTROL: $Id: lmp_cmd_disp.c,v 1.139 2014/03/11 03:14:00 garyf Exp $
 * 
 * DESCRIPTION:     
 * This module is responsible for decoding incoming LMP messages and 
 * dispatching them to the appropiate object in the Link Manager.
 *
 ********************************************************************/
#include "sys_config.h"

#include "hc_event_gen.h"
#include "hc_const.h"

#include "tra_queue.h"

#include "lmp_acl_connection.h"
#include "lmp_acl_container.h"
#include "lmp_config.h"
#include "lmp_const.h"
#include "lmp_ch.h"
#include "lmp_cmd_disp.h"
#include "lmp_encode_pdu.h"
#include "lmp_link_control_protocol.h"
#include "lmp_link_policy_protocol.h"
#include "lmp_link_qos.h"
#include "lmp_timer.h"
#include "lmp_utils.h"
#include "lmp_afh.h"
#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
#include "lmp_ssp.h"
#endif
#include "tc_event_gen.h"
#include "bt_tester.h"                  /* Optional internal testmode tester */

#include "le_link_layer.h"
#include "patch_function_id.h"
/*
 * Local helper functions
 */
/*static void _LMdisp_Clear_Any_Associated_Message_Timer(
    u_int16 rx_cmd_opcode, const u_int8 *rx_lmp_payload, t_lmp_link *p_link);
static t_error _LMdisp_Dispatch_Extended_Opcode(
    u_int16 opcode_15bit, t_p_pdu rx_lmp_payload, t_lmp_link *p_link);*/

/*
 * Maintain length of current dispatched command
 * (Only the LMP_unpark_pm_addr_pdu has a length context).
 */
//static u_int8 _LMPdisp_dispatched_pdu_length;

/*
 * Setup lmp command table as array of pointers to handler functions
 */
#ifndef BLUETOOTH_MODE_LE_ONLY
t_lmp_funct lmp_command_table[] = {
/* 00 */    &LMdisp_LMP_Unsupported_Command,
/* 01 */    &LMconnection_LMP_Name_Req,
/* 02 */    &LMconnection_LMP_Name_Res,
/* 03 */    &LMdisp_LMP_Accepted,
/* 04 */    &LMdisp_LMP_Not_Accepted,
/* 05 */    &LMlc_LMP_Clock_Offset_Req,
/* 06 */    &LMlc_LMP_Clock_Offset_Res,
/* 07 */    &LMconnection_LMP_Detach,
/* 08 */    &LMsec_peer_LMP_In_Rand,
/* 09 */    &LMsec_peer_LMP_Comb_Key,
#if (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED == 1)
/* 10 */    &LMsec_peer_LMP_Unit_Key,
#else
/* 10 */    &LMdisp_LMP_Unsupported_Command,
#endif
/* 11 */    &LMsec_peer_LMP_Au_Rand,
/* 12 */    &LMsec_peer_LMP_Sres,
/* 13 */    &LMsec_peer_LMP_Temp_Rand,
/* 14 */    &LMsec_peer_LMP_Temp_Key,

#if (PRH_BS_CFG_SYS_LMP_ENCRYPTION_SUPPORTED==1)  
/* 15 */    &LMsec_peer_LMP_Encryption_Mode_Req,
/* 16 */    &LMsec_peer_LMP_Encryption_Key_Size_Req,
/* 17 */    &LMsec_peer_LMP_Start_Encryption_Req, 
/* 18 */    &LMsec_peer_LMP_Stop_Encryption_Req,
#else
/* 15 */    &LMdisp_LMP_Unsupported_Command,
/* 16 */    &LMdisp_LMP_Unsupported_Command,
/* 17 */    &LMdisp_LMP_Unsupported_Command,
/* 18 */    &LMdisp_LMP_Unsupported_Command,
#endif

#if (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1)
/* 19 */    &LMconnection_LMP_Switch_Req,
#else
/* 19 */    &LMdisp_LMP_Unsupported_Command,
#endif

#if(PRH_BS_CFG_SYS_LMP_HOLD_SUPPORTED==1)
/* 20 */    &LMpolicy_LMP_Hold,
/* 21 */    &LMpolicy_LMP_Hold_Req,
#else
/* 20 */    &LMdisp_LMP_Unsupported_Command,
/* 21 */    &LMdisp_LMP_Unsupported_Command,
#endif

#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUPPORTED==1)
/* 22 */    &LMdisp_LMP_Unsupported_Command,    /* LMP_sniff not supported in V1.1*/
/* 23 */    &LMpolicy_LMP_Sniff_Req,
/* 24 */    &LMpolicy_LMP_Unsniff_Req,
#else
/* 22 */    &LMdisp_LMP_Unsupported_Command,
/* 23 */    &LMdisp_LMP_Unsupported_Command,
/* 24 */    &LMdisp_LMP_Unsupported_Command,
#endif

#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
/* 25 */    &LMpolicy_LMP_Park_Req,
/* 26 */    &LMdisp_LMP_Unsupported_Command,
/* 27 */    &LMpolicy_LMP_Set_Broadcast_Scan_Window,
/* 28 */    &LMpolicy_LMP_Modify_Beacon,
/* 29 */    &LMpolicy_LMP_Unpark_Bd_Addr_Req,
/* 30 */    &LMpolicy_LMP_Unpark_Pm_Addr_Req,
#else
/* 25 */    &LMdisp_LMP_Unsupported_Command,
/* 26 */    &LMdisp_LMP_Unsupported_Command,
/* 27 */    &LMdisp_LMP_Unsupported_Command,
/* 28 */    &LMdisp_LMP_Unsupported_Command,
/* 29 */    &LMdisp_LMP_Unsupported_Command,
/* 30 */    &LMdisp_LMP_Unsupported_Command,
#endif

#if(PRH_BS_CFG_SYS_LMP_POWER_CONTROL_SUPPORTED==1)
/* 31 */    &LMlc_LMP_Incr_Power_Req,
/* 32 */    &LMlc_LMP_Decr_Power_Req,
/* 33 */    &LMlc_LMP_Max_Power,
/* 34 */    &LMlc_LMP_Min_Power,
#else
/* 31 */    &LMdisp_LMP_Unsupported_Command,
/* 32 */    &LMdisp_LMP_Unsupported_Command,
/* 33 */    &LMdisp_LMP_Unsupported_Command,
/* 34 */    &LMdisp_LMP_Unsupported_Command,
#endif

#if ( (PRH_BS_CFG_SYS_LM_AUTO_RATE_SUPPORTED==1) && (PRH_BS_CFG_SYS_LMP_CHANNEL_DRIVEN_QUALITY_SUPPORTED==1))
/* 35 */    &LMqos_LMP_Auto_Rate,
#else
/* 35 */    &LMdisp_LMP_Ignore_Command,
#endif

#if  ( (PRH_BS_CFG_SYS_LM_PREFERRED_RATE_SUPPORTED==1) && (PRH_BS_CFG_SYS_LMP_CHANNEL_DRIVEN_QUALITY_SUPPORTED==1))
/* 36 */    &LMqos_LMP_Preferred_Rate,
#else
/* 36 */    &LMdisp_LMP_Ignore_Command,
#endif

/* 37 */    &LMconfig_LMP_Version_Req,
/* 38 */    &LMconfig_LMP_Version_Res,
/* 39 */    &LMconnection_LMP_Features_Req,
/* 40 */    &LMconnection_LMP_Features_Res,

#if (PRH_BS_CFG_SYS_QOS_SUPPORTED==1)
/* 41 */    &LMqos_LMP_Quality_of_Service,
/* 42 */    &LMqos_LMP_Quality_of_Service_Req,
#else
/* 41 */    &LMdisp_LMP_Unsupported_Command,
/* 42 */    &LMdisp_LMP_Unsupported_Command,
#endif

#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
/* 43 */    &LMconnection_LMP_SCO_Link_Req,
/* 44 */    &LMconnection_LMP_Remove_SCO_Link_Req,
#else
/* 43 */    &LMdisp_LMP_Unsupported_Command,
/* 44 */    &LMdisp_LMP_Unsupported_Command,
#endif

/* 45 */    &LMqos_LMP_Max_Slot,
/* 46 */    &LMqos_LMP_Max_Slot_Req,

#if (PRH_BS_CFG_SYS_LMP_TIMING_INFO_SUPPORTED==1) /* Fix Bug 338 */
/* 47 */    &LMlc_LMP_Timing_Accuracy_Req,
/* 48 */    &LMlc_LMP_Timing_Accuracy_Res,
#else
/* 47 */    &LMdisp_LMP_Unsupported_Command,
/* 48 */    &LMdisp_LMP_Unsupported_Command,
#endif

/* 49 */    &LMconnection_LMP_Setup_Complete,
/* 50 */    &LMsec_peer_LMP_Use_Semi_Permanent_Key,
/* 51 */    &LMconnection_LMP_Host_Connection_Req,
/* 52 */    &LMlc_LMP_Slot_Offset,

#if (PRH_BS_CFG_SYS_LMP_OPTIONAL_PAGING_SUPPORTED==1)
/* 53 */    &LMlc_LMP_Page_Mode_Req,
/* 54 */    &LMlc_LMP_Page_Scan_Mode_Req,
#else
/* 53 */    &LMdisp_LMP_Unsupported_Command,
/* 54 */    &LMdisp_LMP_Unsupported_Command,
#endif

/* 55 */    &LMqos_LMP_Supervision_Timeout,
/* 56 */    &LMlc_LMP_Test_Activate,
/* 57 */    &LMlc_LMP_Test_Control,
#if (PRH_BS_CFG_SYS_LMP_BROADCAST_ENCRYPTION_SUPPORTED==1)
            &LMsec_peer_LMP_Encryption_Key_Size_Mask_Req,
            &LMsec_peer_LMP_Encryption_Key_Size_Mask_Res,
#else
/* 58 */    &LMdisp_LMP_Unsupported_Command,
/* 59 */    &LMdisp_LMP_Unsupported_Command,
#endif

#if (PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_SLAVE_SUPPORTED==1)
/* 60 */    &LMafh_LMP_Set_AFH,
#else
/* 60 */    &LMdisp_LMP_Unsupported_Command,
#endif
#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
/* 61 */    &LMssp_LMP_Encapsulated_Header, 
/* 62 */    &LMssp_LMP_Encapsulated_Payload,
/* 63 */    &LMssp_LMP_Simple_Pairing_Confirm,
/* 64 */    &LMssp_LMP_Simple_Pairing_Number,
/* 65 */    &LMssp_LMP_DHkey_Check,
#else
/* 61 */    &LMdisp_LMP_Unsupported_Command, 
/* 62 */    &LMdisp_LMP_Unsupported_Command,
/* 63 */    &LMdisp_LMP_Unsupported_Command,
/* 64 */    &LMdisp_LMP_Unsupported_Command,
/* 65 */    &LMdisp_LMP_Unsupported_Command,
#endif
};

#endif//BLUETOOTH_MODE_LE_ONLY
/*****************************************************************************
 *  
 *  FUNCTION : LMPdisp_LMP_Command_Dispatcher
 *
 *  DESCRIPTION :
 *  Retrieves the next incoming LMP PDU off the incoming LMP Queues.
 *  Determines the link for which this PDU is targeted and decodes the opcode.
 *  Using the opcode as an index the command opcode table is looked up and
 *  the matching function called.
 *
 *****************************************************************************/
t_error LMdisp_LMP_Command_Dispatcher(void)
{
	
	if(PATCH_FUN[LMDISP_LMP_COMMAND_DISPATCHER_ID]){
         
         return ((t_error (*)(void))PATCH_FUN[LMDISP_LMP_COMMAND_DISPATCHER_ID])();
    }


    u_int16         opcode_15bit;
    t_error         status = NO_ERROR;
    t_p_pdu         p_payload = (t_p_pdu) 0;
    t_lmp_link      *p_link;
    t_q_descr       *qd; 
	t_LE_Connection*  p_le_link;

    /* 
     * Payload is not moved from queue, p_payload is just assigned qd->data 
     */
    qd = BTq_Dequeue_Next(LMP_IN_Q, 0);
    if (qd) 
    {
       p_payload = qd->data;
    }

    if (p_payload) 
    {
        /*
         * If Direct Baseband Copy (Big Endian) Then Change Data to Little Endian
         */
        BTq_Change_Endianness_If_Required(p_payload, qd->length);

		p_le_link = LEconnection_Find_Device_Index(qd->device_index);

        if ( (p_le_link) && (p_le_link->active))
		{
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
			 if(p_le_link->current_security_state & LE_ENCRYPTION_ACTIVE)
			 {
				 //
				 // Copy payload to a 32bit alligned buf
				 //LEconnection_Decrypt_Incoming_Pdu(p_le_link,p_payload,qd->length,qd->enc_pkt_count,LE_LLC/* llid = LLC Msg */);
				 status = LEconnection_Decrypt_Incoming_Pdu(p_le_link,p_payload,qd->length,qd->enc_pkt_count,LE_LLC/* llid = LLC Msg */);
				 if (status != NO_ERROR)  // slove bug sev/sla/bi03c
				 {
				 	//LEconnection_Local_Disconnect(p_le_link , EC_CONNECTION_TERMINATED_DUE_TO_MIC_FAILURE);
					BTq_Ack_Last_Dequeued(LMP_IN_Q, 0, qd->length);
					return 0;
				 }
			 }
#endif
#if (PRH_BS_CFG_SYS_TRACE_LLC_VIA_HCI_SUPPORTED==1)
			 TCeg_Send_LE_LLC_Event(p_payload, (u_int8) qd->length, 1 /*Recieved*/);
#endif
			 LE_Decode_link_layer_PDU(p_le_link, p_payload, (u_int8) qd->length);
		     BTq_Ack_Last_Dequeued(LMP_IN_Q, 0, qd->length);

		     return NO_ERROR;
        }
#ifndef BLUETOOTH_MODE_LE_ONLY
		else
		{
			 
        /*
         * Log this message over the TCI if this feature is active
         */
        TCeg_Send_LMP_Event(p_payload, (u_int8) qd->length, 1 /*received*/);

        /* 
         * Translate from device index to p_link, and validate
         */ 
        p_link = LMaclctr_Find_Device_Index(qd->device_index);
        if (!p_link)
        {
            qd->callback = 0;
            BTq_Ack_Last_Dequeued(LMP_IN_Q, 0, qd->length);
            return UNSPECIFIED_ERROR;
        }

        /*
         * Extract the opcode and move p_payload to parameters
         */
        p_link->tid_role_last_rcvd_pkt = (*p_payload) & 0x01;
        opcode_15bit = ((*p_payload++) >> 1);
        if (opcode_15bit >= LMP_ESCAPE_1_OPCODE)
        {
            opcode_15bit = (opcode_15bit <<8) + *p_payload++;
        }

        /*  
         * If a Detach is pending on the link Then
         *    Ignore all further LMP commands (B.378).
         */
        if((p_link->operation_pending == LMP_DETACH) ||
			((p_link->state & LMP_W4_SUPERVISION_TIMEOUT) ==  LMP_W4_SUPERVISION_TIMEOUT))
        {
            /*
             * Ignore the command
             */
        }

        /*
         * Check that we have entry in V1.1 table or its an escape code.
         */
        else if (opcode_15bit >= mNum_Elements(lmp_command_table) && 
            opcode_15bit < LMP_ESCAPE_4_OPCODE)
        {
            LM_Encode_LMP_Not_Accepted_PDU(p_link, 
                (u_int8) opcode_15bit, UNSUPPORTED_REMOTE_FEATURE);
        }
        else
        {
            _LMdisp_Clear_Any_Associated_Message_Timer(
                opcode_15bit, p_payload, p_link);

            _LMPdisp_dispatched_pdu_length = (u_int8) qd->length;

            if (opcode_15bit < LMP_ESCAPE_1_OPCODE)
            {
                /*
                 * V1.1 7 bit opcode => Process directly
                 */
                if (opcode_15bit <= mNum_Elements(lmp_command_table))
                    status = lmp_command_table[opcode_15bit](p_link, p_payload);    
                else
                    status = UNSUPPORTED_REMOTE_FEATURE;


                if (status != NO_ERROR)
                { 
                    /*
                     * Return an LMP_not_accepted message B.340
                     * with the same transaction id as received LMP message B.351 
                     */
                    p_link->current_proc_tid_role = p_link->tid_role_last_rcvd_pkt;
                    LM_Encode_LMP_Not_Accepted_PDU(
                        p_link, (u_int8) opcode_15bit, status);
                }
            }
            else
            {
                /*
                 * V1.2 15 bit opcode => Call Handler
                 */
                status = _LMdisp_Dispatch_Extended_Opcode(
                    opcode_15bit, p_payload, p_link);
            }
       }
       BTq_Ack_Last_Dequeued(LMP_IN_Q, 0, qd->length);
	   }
#endif //BLUETOOTH_MODE_LE_ONLY
   } 
   else 
   {
        status = NO_ERROR;
   }

   return status;
}

/*****************************************************************************
 *  
 *  FUNCTION : LMdisp_Get_Dispatched_LMP_PDU_Length
 *
 *  DESCRIPTION :
 *  Returns the length of the dispatched LMP command.
 *
 *  In V1.1 only the LMP_unpark_PM_ADDR_req PDU is of variable length, 
 *  and its length cannot be determined from the PDU parameters.
 *
 *****************************************************************************/
#ifndef BLUETOOTH_MODE_LE_ONLY
u_int8 LMdisp_Get_Dispatched_LMP_PDU_Length(void)
{
    return _LMPdisp_dispatched_pdu_length;
}

/*****************************************************************************
 *  
 *  FUNCTION : LMdisp_LMP_Accepted
 *
 *  DESCRIPTION :
 *  On Receipt of an LMP_Accepted PDU this method is called to
 *  route the LMP_Accepted PDU to the appropiate Object in the 
 *  LM layer.
 *  The correct object is determined from the Accepted Opcode in 
 *  the payload of the PDU.
 *
 *****************************************************************************/
t_error LMdisp_LMP_Accepted(t_lmp_link *p_link, t_p_pdu payload)
{
	return LMdisp_LMP_Accepted_Common(p_link, payload, 0);
}


/*****************************************************************************
 *  FUNCTION : LMdisp_LMP_Accepted_Ext
 *
 *  DESCRIPTION :
 *  Dispatch an LMP_accepted_ext command
 *
 *  p_link          link container for the associated lmp logical channel
 *  payload         LMP command parameters
 *****************************************************************************/
t_error LMdisp_LMP_Accepted_Ext(t_lmp_link *p_link, t_p_pdu payload)
{
	return LMdisp_LMP_Accepted_Common(p_link, payload, 1 /* extended */);
}    
    

t_error LMdisp_LMP_Accepted_Common(t_lmp_link *p_link, t_p_pdu payload, u_int8 extended_flag)
{
    u_int16 accepted_opcode = (u_int8)(*payload);

	if(extended_flag)
	{
		accepted_opcode <<= 8;
		accepted_opcode |= *(payload+1);
	}

    /* 
     * Cancel the operation pending if same as the accepted LMP command opcode
     */
    if(p_link->operation_pending == accepted_opcode)
    {
        p_link->operation_pending = NO_OPERATION;
    }

    /*
     * Handle the accepted LMP command
     */
    switch(accepted_opcode)
    {
        case LMP_HOST_CONNECTION_REQ :
        case LMP_SWITCH_REQ :
        case LMP_SCO_LINK_REQ :
        case LMP_REMOVE_SCO_LINK_REQ :
		case LMP_ESCO_LINK_REQ:
		case LMP_REMOVE_ESCO_LINK_REQ:
        case LMP_PACKET_TYPE_TABLE_REQ:
            LMconnection_LMP_Accepted(p_link, accepted_opcode);
            break;

        case LMP_AU_RAND :  
        case LMP_IN_RAND :
        case LMP_USE_SEMI_PERMANENT_KEY :
#if (PRH_BS_CFG_SYS_LMP_ENCRYPTION_SUPPORTED==1)
        case LMP_ENCRYPTION_MODE_REQ :
        case LMP_ENCRYPTION_KEY_SIZE_REQ :
        case LMP_START_ENCRYPTION_REQ :
        case LMP_STOP_ENCRYPTION_REQ :
#endif
            LMsec_peer_LMP_Accepted(p_link, payload);
            break;
 
#if (PRH_BS_CFG_SYS_LMP_HOLD_SUPPORTED==1)
        case LMP_HOLD_REQ :
#endif
#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUPPORTED==1)
        case LMP_SNIFF_REQ :
        case LMP_UNSNIFF_REQ :
#endif
#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
        case LMP_PARK_REQ :
        case LMP_UNPARK_BD_ADDR_REQ :
        case LMP_UNPARK_PM_ADDR_REQ :
#endif
            LMpolicy_LMP_Accepted(p_link, payload);
            break;

        case LMP_QUALITY_OF_SERVICE_REQ :
        case LMP_MAX_SLOT_REQ :
            LMqos_LMP_Accepted(p_link, payload);
            break;

        case LMP_PAGE_MODE_REQ :
        case LMP_PAGE_SCAN_MODE_REQ :
            break;

#if (PRH_BS_CFG_SYS_TEST_MODE_TESTER_SUPPORTED==1)
        /*
         * Testmode Tester, route LMP_accepted messages to handler
         */
        case LMP_TEST_ACTIVATE :
        case LMP_TEST_CONTROL :
            BTtmt_LMP_Accepted(p_link, payload);
            break;
#endif

#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
		case LMP_ENCAPSULATED_HEADER :
		case LMP_ENCAPSULATED_PAYLOAD :
		case LMP_SIMPLE_PAIRING_NUMBER :
		case LMP_SIMPLE_PAIRING_CONFIRM :
		case LMP_DHKEY_CHECK :
			LMssp_LMP_Accepted(p_link,payload);
			break;
#endif
        default : 
            break;
     }
   return NO_ERROR;
}


/*****************************************************************************
 *  FUNCTION : LMdisp_LMP_Not_Accepted
 *
 *  DESCRIPTION :
 *  On Receipt of an LMP_Not_Accepted PDU this method is called to
 *  route the LMP_Not_Accepted PDU to the appropiate Object in the 
 *  LM layer.
 *  The correct object is determined from the Rejected Opcode in 
 *  the payload of the PDU.
 *
 *****************************************************************************/
t_error LMdisp_LMP_Not_Accepted(t_lmp_link *p_link, t_p_pdu payload)
{
	return LMdisp_LMP_Not_Accepted_Common(p_link, (u_int8 *) payload, 0);
}


/*****************************************************************************
 *  FUNCTION : LMdisp_LMP_Not_Accepted_Ext
 *
 *  DESCRIPTION :
 *  Dispatch an LMP_not_accepted_ext command
 *
 *  p_link          link container for the associated lmp logical channel
 *  payload         LMP command parameters
 *****************************************************************************/
t_error LMdisp_LMP_Not_Accepted_Ext(t_lmp_link *p_link, t_p_pdu payload)
{
    return LMdisp_LMP_Not_Accepted_Common(p_link, (u_int8 *) payload, 1);
}

t_error LMdisp_LMP_Not_Accepted_Common(t_lmp_link *p_link, u_int8* payload, u_int8 extended_flag)
{
    t_error reason;
    u_int16 rejected_opcode = *payload++;

	if(extended_flag)
	{
		rejected_opcode <<= 8;
		rejected_opcode |= *payload++;
	}

    reason = (t_error) *payload;

    /*
     * There is a little bit of fudge required to cover
     * the case where a peer device (out of spec) sends
     * LMP_not_accepted to an extended LMP PDU.
     *
     * The best possible cover for this case is if the
     * LMP_not_acccepted is for 0xFF, then check
     * the operation pending. If the operation
     * pending is for an 0xFF-related PDU, assume
     * that the not accepted relates to this transaction
     */
    if(rejected_opcode == 0xFF)
    {
        if((p_link->operation_pending>>8) == (rejected_opcode>>1))
        {
            rejected_opcode = p_link->operation_pending;
        }
    }
 
    /* 
     * Cancel the operation pending if same as the rejected LMP command opcode
     */
    if(p_link->operation_pending == rejected_opcode)
    {
        p_link->operation_pending = NO_OPERATION;
    }

    /*
     * Handle the rejected LMP command
     */
    switch(rejected_opcode)
    {
        case LMP_HOST_CONNECTION_REQ :
        case LMP_SWITCH_REQ :
        case LMP_SCO_LINK_REQ :
        case LMP_REMOVE_SCO_LINK_REQ :
		case LMP_ESCO_LINK_REQ:
		case LMP_REMOVE_ESCO_LINK_REQ:
        case LMP_NAME_REQ :
        case LMP_FEATURES_REQ :
        case LMP_FEATURES_REQ_EXT :
        case LMP_PACKET_TYPE_TABLE_REQ :
            LMconnection_LMP_Not_Accepted(p_link, rejected_opcode, reason);
            break;

        case LMP_AU_RAND :
        case LMP_IN_RAND :
        case LMP_USE_SEMI_PERMANENT_KEY :
        case LMP_ENCRYPTION_MODE_REQ :
        case LMP_ENCRYPTION_KEY_SIZE_REQ :
        case LMP_START_ENCRYPTION_REQ :
        case LMP_STOP_ENCRYPTION_REQ :
        case LMP_PAUSE_ENCRYPTION_REQ:
        case LMP_RESUME_ENCRYPTION_REQ:
        case LMP_COMB_KEY:
        case LMP_UNIT_KEY:
        case LMP_ENCRYPTION_KEY_SIZE_MASK_REQ:
            LMsec_peer_LMP_Not_Accepted(p_link, rejected_opcode, reason);
            break;

        case LMP_HOLD_REQ :
        case LMP_SNIFF_REQ :
        case LMP_UNSNIFF_REQ :
        case LMP_SNIFF_SUBRATING_REQ:
        case LMP_PARK_REQ :
        case LMP_UNPARK_BD_ADDR_REQ :
        case LMP_UNPARK_PM_ADDR_REQ :
            LMpolicy_LMP_Not_Accepted(p_link, rejected_opcode, reason);
            break;

        case LMP_MAX_SLOT_REQ :
        case LMP_QUALITY_OF_SERVICE_REQ :
            LMqos_LMP_Not_Accepted(p_link, rejected_opcode, reason);
            break;

#if (PRH_BS_CFG_SYS_LMP_POWER_CONTROL_SUPPORTED==1)
        case LMP_INCR_POWER_REQ:
        case LMP_DECR_POWER_REQ:
#endif
#if (PRH_BS_CFG_SYS_LMP_TIMING_INFO_SUPPORTED==1)
        case LMP_TIMING_ACCURACY_REQ : 
#endif
        case LMP_PAGE_MODE_REQ :
        case LMP_PAGE_SCAN_MODE_REQ :
        case LMP_CLKOFFSET_REQ:
            LMlc_LMP_Not_Accepted(p_link, rejected_opcode, reason);
            break;

#if (PRH_BS_CFG_SYS_TEST_MODE_TESTER_SUPPORTED==1)
        /*
         * Testmode Tester, route LMP_not_accepted messages to handler
         */
        case LMP_TEST_ACTIVATE :
        case LMP_TEST_CONTROL :
            BTtmt_LMP_Not_Accepted(p_link, rejected_opcode, reason);
            break;
#endif

        case LMP_VERSION_REQ:
            LMconfig_LMP_Not_Accepted(p_link, rejected_opcode, reason);
            break;

#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
		case LMP_ENCAPSULATED_HEADER :
		case LMP_ENCAPSULATED_PAYLOAD :
		case LMP_SIMPLE_PAIRING_NUMBER :
		case LMP_SIMPLE_PAIRING_CONFIRM :
		case LMP_IO_CAPABILITY_REQUEST :
		case LMP_IO_CAPABILITY_RESPONSE :
		case LMP_DHKEY_CHECK :
			LMssp_LMP_Not_Accepted(p_link, rejected_opcode, reason);
			break;
#endif
        default : 
             break;
    }
    return NO_ERROR;
}

/*****************************************************************************
 *  FUNCTION : LMdisp_LMP_Ignore_Command
 *
 *  DESCRIPTION :
 *
 *  This function processes the receipt of an LMP command which if the 
 *  associated functionality is not supported must be ignored.
 *****************************************************************************/
t_error LMdisp_LMP_Ignore_Command(t_lmp_link  *p_link, t_p_pdu p_payload)
{
    return NO_ERROR;
}

/*****************************************************************************
 *  FUNCTION : LMdisp_LMP_Unsupported_Command
 *
 *  DESCRIPTION :
 *
 *  This function processes the receipt of an LMP command which if the 
 *  associated functionality is not supported must be rejected.
 *****************************************************************************/
t_error LMdisp_LMP_Unsupported_Command(t_lmp_link  *p_link, t_p_pdu p_payload)
{
    return UNSUPPORTED_REMOTE_FEATURE;
}





/*****************************************************************************
 *  FUNCTION : _LMdisp_Clear_Any_Associated_Message_Timer
 *
 *  DESCRIPTION :
 *  Clear any associated timer if it is active.
 *  This is a bit of a bodge to improve matters.
 *  At least check that the PDU is the expected response PDU
 *  before turning off the response timer.
 *
 *  rx_cmd_opcode        LMP command opcode of received command
 *  rx_lmp_params        LMP command parameters
 *  p_link               Pointer to associated link container
 *
 *  N.B. security block looks after itself
 *****************************************************************************/
static void _LMdisp_Clear_Any_Associated_Message_Timer(
    u_int16 rx_cmd_opcode, const u_int8 *rx_lmp_payload, t_lmp_link *p_link)
{
    boolean is_valid_response;
    u_int16 operation_pending = p_link->operation_pending;
    
    /*
     * Determine if the message is a valid response to the pending operation
     */
    switch(rx_cmd_opcode)
    {
    case LMP_ACCEPTED:
    case LMP_NOT_ACCEPTED:
        if(rx_lmp_payload[0] != 0xFF)
            is_valid_response = (rx_lmp_payload[0] == operation_pending);
        else
            is_valid_response = ( (operation_pending>>8) == 0xFF);
        break;

	case LMP_ACCEPTED_EXT:
	case LMP_NOT_ACCEPTED_EXT:
		is_valid_response = (( (rx_lmp_payload[0]<<8) + rx_lmp_payload[1]) == operation_pending);
		break;

    case LMP_CLKOFFSET_RES:
        is_valid_response = (LMP_CLKOFFSET_REQ == operation_pending);
        break;

    case LMP_FEATURES_RES:
        is_valid_response = (LMP_FEATURES_REQ == operation_pending);
        break;

    case LMP_FEATURES_RES_EXT:
        is_valid_response = (LMP_FEATURES_REQ_EXT == operation_pending);
        break;

    case LMP_NAME_RES:
        is_valid_response = (LMP_NAME_REQ == operation_pending);
        break;

    case LMP_TIMING_ACCURACY_RES:
        is_valid_response = (LMP_TIMING_ACCURACY_REQ == operation_pending);
        break;

    case LMP_VERSION_RES:
        is_valid_response = (LMP_VERSION_REQ == operation_pending);
        break;

    /* 
     * special case - slave needs a way to guard against no LM traffic 
     * from peer on new connection 
     */
    case LMP_HOST_CONNECTION_REQ:
        is_valid_response = (LMP_HOST_CONNECTION_REQ == operation_pending);
        break;

    /* 
     * These can be valid response PDUs 
     */
    case LMP_HOLD_REQ:
        is_valid_response = (LMP_HOLD_REQ == operation_pending);
        break;

    case LMP_HOLD:
        /*
         * If IUT is a slave, receiving LMP_hold is a valid closure.
         */
        is_valid_response = ((LMP_HOLD == operation_pending) && (p_link->role == SLAVE));
        break;

    case LMP_SNIFF_REQ:
        is_valid_response = (LMP_SNIFF_REQ == operation_pending);
        break;

    case LMP_PARK_REQ:
        /*
         * If IUT is a slave, receiving LMP_park_req in response
         * to LMP_park_req is a valid closure.
         */
        is_valid_response = ((LMP_PARK_REQ == operation_pending) && (p_link->role == SLAVE));
        break;

    case LMP_SCO_LINK_REQ:
        /*
         * If IUT is a slave, receiving LMP_sco_link_req in response
         * to LMP_park_req is a valid closure.
         */
        is_valid_response = ((LMP_SCO_LINK_REQ == operation_pending) && (p_link->role == SLAVE));
        break;

    case LMP_ENCRYPTION_KEY_SIZE_MASK_RES:
        is_valid_response = (LMP_ENCRYPTION_KEY_SIZE_MASK_REQ == operation_pending);
        break;

    case LMP_SNIFF_SUBRATING_RES:
        is_valid_response = (LMP_SNIFF_SUBRATING_REQ == operation_pending);
        break;

    default:
        is_valid_response = FALSE;
    }

    /* 
     * Clear the Message Timer if active and response is for the timer
     */
    if ((p_link->msg_timer != 0) && is_valid_response)
    {
        LMtmr_Clear_Timer(p_link->msg_timer);
        p_link->msg_timer = 0;
    }
}



/*****************************************************************************
 *
 * Extended opcode support
 *
 *****************************************************************************/

const t_lmp_funct lmp_command_table_escape_opcode_4[] = {
/* 00 */    &LMdisp_LMP_Unsupported_Command,
/* 01 */    &LMdisp_LMP_Accepted_Ext,
/* 02 */    &LMdisp_LMP_Not_Accepted_Ext,
#if (PRH_BS_CFG_SYS_HCI_V12_FUNCTIONALITY_SUPPORTED==1)
/* 03 */    &LMconnection_LMP_Features_Req_Ext,
/* 04 */    &LMconnection_LMP_Features_Res_Ext,
#else
/* 03 */    &LMdisp_LMP_Unsupported_Command,
/* 04 */    &LMdisp_LMP_Unsupported_Command,
#endif
/* 05 */    &LMdisp_LMP_Unsupported_Command,
/* 06 */    &LMdisp_LMP_Unsupported_Command,
/* 07 */    &LMdisp_LMP_Unsupported_Command,
/* 08 */    &LMdisp_LMP_Unsupported_Command,
/* 09 */    &LMdisp_LMP_Unsupported_Command,
/* 10 */    &LMdisp_LMP_Unsupported_Command,
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
/* 11 */    &LMconnection_LMP_Packet_Type_Table_Req,
#else
/* 11 */    &LMdisp_LMP_Unsupported_Command,
#endif
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
/* 12 */    &LMconnection_LMP_eSCO_Link_Req,
/* 13 */    &LMconnection_LMP_Remove_eSCO_Link_Req,
#else
/* 12 */    &LMdisp_LMP_Unsupported_Command,
/* 13 */    &LMdisp_LMP_Unsupported_Command,
#endif
/* 14 */    &LMdisp_LMP_Unsupported_Command,
/* 15 */    &LMdisp_LMP_Unsupported_Command,

#if (PRH_BS_CFG_SYS_LMP_AFH_CLASSIFICATION_SLAVE_SUPPORTED==1)  
/* 16 */    &LMafh_LMP_Channel_Classification_Req,
#else
/* 16 */    &LMdisp_LMP_Unsupported_Command,
#endif
#if (PRH_BS_CFG_SYS_LMP_AFH_CLASSIFICATION_MASTER_SUPPORTED==1)  
/* 17 */    &LMafh_LMP_Channel_Classification,
#else
/* 17 */    &LMdisp_LMP_Unsupported_Command,
#endif
/* 18 */    &LMdisp_LMP_Unsupported_Command,
/* 19 */    &LMdisp_LMP_Unsupported_Command,
/* 20 */    &LMdisp_LMP_Unsupported_Command,
#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUBRATING_SUPPORTED==1)
/* 21 */    &LMpolicy_LMP_Sniff_Subrating_Req,
/* 22 */    &LMpolicy_LMP_Sniff_Subrating_Res,
#else
/* 21 */    &LMdisp_LMP_Unsupported_Command,
/* 22 */    &LMdisp_LMP_Unsupported_Command,
#endif
#if (PRH_BS_CFG_SYS_LMP_PAUSE_ENCRYPTION_SUPPORTED==1)
/* 23 */    &LMsec_peer_LMP_Pause_Encryption_Req,
/* 24 */    &LMsec_peer_LMP_Resume_Encryption_Req,
#else
/* 23 */    &LMdisp_LMP_Unsupported_Command,
/* 24 */    &LMdisp_LMP_Unsupported_Command,
#endif
#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
/* 25 */    &LMspp_LMP_IO_Capability_Request,
/* 26 */    &LMspp_LMP_IO_Capability_Response,
/* 27 */    &LMssp_LMP_Numeric_Comparison_Failure,
#if (PRH_BS_CFG_SYS_SSP_PASSKEY_SUPPORTED==1)
/* 28 */    &LMssp_LMP_Passkey_Entry_Failed,
#else
/* 28 */    &LMdisp_LMP_Unsupported_Command, 
#endif
#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)
/* 29 */    &LMssp_LMP_OOB_Failed,
#else
/* 29 */    &LMdisp_LMP_Unsupported_Command,
#endif
#if (PRH_BS_CFG_SYS_SSP_KEYPRESS_SUPPORTED==1)
/* 30 */    &LMssp_LMP_Keypress_Notification,
#else
/* 30 */    &LMdisp_LMP_Unsupported_Command,
#endif
#else
/* 25 */    &LMdisp_LMP_Unsupported_Command,
/* 26 */    &LMdisp_LMP_Unsupported_Command,
/* 27 */    &LMdisp_LMP_Unsupported_Command,
/* 28 */    &LMdisp_LMP_Unsupported_Command,
/* 29 */    &LMdisp_LMP_Unsupported_Command,
/* 30 */    &LMdisp_LMP_Unsupported_Command,
#endif
#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==1)
/* 31 */    &LMlc_LMP_Power_Control_Req,
/* 32 */    &LMlc_LMP_Power_Control_Resp
#else
/* 31 */    &LMdisp_LMP_Unsupported_Command,
/* 32 */    &LMdisp_LMP_Unsupported_Command
#endif

};

const t_lmp_funct *p_lmp_command_escape_table[] = {
/*124 */    NULL,
/*125 */    NULL,
/*126 */    NULL,
/*127 */    lmp_command_table_escape_opcode_4
};

/*****************************************************************************
 *  FUNCTION : _LMdisp_Dispatch_Extended_Opcode
 *
 *  DESCRIPTION :
 *  Dispatch an extended opcode command
 *
 *  rx_escape_code        LMP command opcode of received command
 *  rx_lmp_params        LMP command parameters
 *  p_link               Pointer to associated link container
 *
 *  Precondition:
 *  rx_escape_code must be Escape 4 (127), currently only escape required
 *  External error handling should handle invalid escape codes
 *
 *  Limitations
 *  Reject using LMP_not_accepted if an error occurs.
 *****************************************************************************/
static t_error _LMdisp_Dispatch_Extended_Opcode(
    u_int16 opcode_15bit, t_p_pdu rx_lmp_payload, t_lmp_link *p_link)
{
    t_error status;
    u_int8  extended_opcode = (u_int8) opcode_15bit;

    /*
     * For now only Escape Codes 4 are defined
     */
    if( (opcode_15bit >> 8) != LMP_ESCAPE_4_OPCODE)
    {
        status = UNSUPPORTED_REMOTE_FEATURE;
    }
    else if (extended_opcode >= mNum_Elements(lmp_command_table_escape_opcode_4))
    {
        /*
         * For now just reject with normal LMP_Not_Accepted
         */
        status = UNSUPPORTED_REMOTE_FEATURE;
    }
    else
    {
        status = lmp_command_table_escape_opcode_4[extended_opcode]
                        (p_link, rx_lmp_payload);    
    }

    if (status != NO_ERROR)
    { 
        /*
         * For extended commands may use LMP_not_accepted_ext
         */
        p_link->current_proc_tid_role = p_link->tid_role_last_rcvd_pkt;

        /*
         * If built as a 1.2 host controller use LMP_not_accepted_ext
         * for extended opcodes, else use LMP_not_accepted for
         * extended opcodes.
         */
#if(PRH_BS_CFG_SYS_HCI_V12_FUNCTIONALITY_SUPPORTED)
        LM_Encode_LMP_Not_Accepted_Ext_PDU(p_link,
            opcode_15bit, status);
#else
        LM_Encode_LMP_Not_Accepted_PDU(
            p_link, (u_int8) (opcode_15bit>>8), status);
#endif
    }

    return status;
}
#endif//BLUETOOTH_MODE_LE_ONLY
