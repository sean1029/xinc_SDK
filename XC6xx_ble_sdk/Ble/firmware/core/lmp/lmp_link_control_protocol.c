/**************************************************************************
 * MODULE NAME:    lmp_link_control_protocol.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    Handles LMP PDU & responses for link control
 * AUTHOR:         Gary Fleming
 * DATE:           12-12-1999
 *
 * SOURCE CONTROL:  $Id: lmp_link_control_protocol.c,v 1.83 2012/03/12 17:16:45 tomk Exp $ 
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *    
 * DESCRIPTION
 * This module is responsible for interacting with the link control layer to
 * satisfy LM protocol messages - which control the lower layer.
 *
 *******************************************************************/

#include "sys_types.h"
#include "sys_config.h"
#include "lmp_types.h"
#include "lmp_const.h"
#include "lmp_features.h"
#include "lmp_acl_connection.h"
#include "lmp_encode_pdu.h"
#include "lmp_utils.h"
#include "lmp_config.h"
#include "lmp_link_control_protocol.h"
#include "lmp_scan.h"
#include "lmp_ch.h"
#include "hc_event_gen.h"
#include "hc_const.h"
#include "bt_test.h"
#include "lc_interface.h"


static void _Send_UL_Read_Clock_Offset_Event(t_lmp_link* p_link,u_int16 clock_offset, t_error status);

#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==1)
boolean epc_enabled;
boolean  mFeat_Check_Enhanced_Power_Control(u_int8 *features)
{
	return (epc_enabled && (features[7] & ENHANCED_POWER_CONTROL_MASK));
}
#endif

/*************************************
 *         SERVICE INTERFACE
 *************************************/
#if (PRH_BS_CFG_SYS_LMP_POWER_CONTROL_SUPPORTED==1)
/**********************************************************************
 * Function :- LMlc_LM_Inc_Peer_Power
 * 
 * Description :-
 * Used to reqest that the Tx power of a peer on
 * a given link be increased.
 *********************************************************************/

t_error LMlc_LM_Inc_Peer_Power(t_lmp_link* p_link, u_int8 power_units)
{
     t_lmp_pdu_info pdu_info;

     if(!mFeat_Check_Power_Control(p_link->remote_features))
         return UNSUPPORTED_REMOTE_FEATURE;

     if (p_link->state == LMP_PARK_MODE)
        return COMMAND_DISALLOWED;

     p_link->current_proc_tid_role = p_link->role;
     pdu_info.tid_role = p_link->current_proc_tid_role;

     switch(p_link->peer_power_status)
     {    
     case MAX_POWER :
            /* Cannot Increment the peer power */
            break;

     case MIN_POWER :
            /* Change the state power state of the peer device */
           p_link->peer_power_status = INTERMEDIATE_POWER;
         
     case INTERMEDIATE_POWER :  /* Intential Fall Through */
#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==1)
           if (mFeat_Check_Enhanced_Power_Control(p_link->remote_features))
		   {
				u_int8 LMlc_LMP_pwr_control_req[3];
				LMlc_LMP_pwr_control_req[0] = (127<<1) +  (p_link->role);
				LMlc_LMP_pwr_control_req[1] = (LMP_POWER_CONTROL_REQ & 0xff);
				if (MAX_TRANSMIT_POWER_LEVEL == (t_requested_transmit_power_level) power_units )
					LMlc_LMP_pwr_control_req[2] = INCR_MAX_POWER;
				else
					LMlc_LMP_pwr_control_req[2] = INCR_POWER_ONE_STEP;
				
				LM_Encode_LMP_PDU_Fully_Encoded(p_link->device_index,
					LMP_POWER_CONTROL_REQ, LMlc_LMP_pwr_control_req);
		   }
		   else
#endif
		   {
			power_units = 0; /* Force to 0 as this param is used "internally" for EPC */
           pdu_info.opcode = LMP_INCR_POWER_REQ;
           pdu_info.number = power_units;
           LM_Encode_LMP_PDU(p_link->device_index,&pdu_info);   
		   }
           break;
     }  
     return NO_ERROR;
}

/**********************************************************************
 * Function :- LMlc_LM_Dec_Power
 *
 * Description :-
 * Used to request that the Tx power of a peer on
 * a given link be decreased.
 **********************************************************************/

t_error LMlc_LM_Dec_Peer_Power(t_lmp_link* p_link, u_int8 power_units)
{
     t_lmp_pdu_info pdu_info;

     if(!mFeat_Check_Power_Control(p_link->remote_features))
         return UNSUPPORTED_REMOTE_FEATURE;

     if (p_link->state == LMP_PARK_MODE)
        return COMMAND_DISALLOWED;

     p_link->current_proc_tid_role = p_link->role;
     pdu_info.tid_role = p_link->current_proc_tid_role;

     switch(p_link->peer_power_status)
     {
     case MIN_POWER :
            /* Cannot Decrement the peer power */
            break;
     case MAX_POWER :
            /* Change the state power state of the peer device */
           p_link->peer_power_status = INTERMEDIATE_POWER;
         
     case INTERMEDIATE_POWER :  /* Intential Fall Through */
#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==1)

		 if (mFeat_Check_Enhanced_Power_Control(p_link->remote_features))
		   {
				u_int8 LMlc_LMP_pwr_control_req[3];
				LMlc_LMP_pwr_control_req[0] = (127<<1) +  (p_link->role);
				LMlc_LMP_pwr_control_req[1] = (LMP_POWER_CONTROL_REQ & 0xff);
				LMlc_LMP_pwr_control_req[2] = DECR_POWER_ONE_STEP;
				
				LM_Encode_LMP_PDU_Fully_Encoded(p_link->device_index,
					LMP_POWER_CONTROL_REQ, LMlc_LMP_pwr_control_req);
		   }
		   else
#endif

		   if (mFeat_Check_Power_Control(p_link->remote_features))
		   {
           pdu_info.opcode = LMP_DECR_POWER_REQ;
           pdu_info.number = power_units;
           LM_Encode_LMP_PDU(p_link->device_index,&pdu_info);   
		   }
           break;
 
     default :
           break;
     }
     return NO_ERROR;
}

/**********************************************************************
 * Function :- LMlc_Read_Transmit_Power_Level
 *
 * Description :-
 * Used by the higher layers to re
 **********************************************************************/

t_error LMlc_LM_Read_Transmit_Power_Level(t_lmp_link* p_link, u_int8 power_type,
                                                  t_cmd_complete_event* p_cmd_complete)
{
    t_error status = NO_ERROR;

    if(!SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_LMP_POWER_CONTROL_FEATURE))
    {
        return UNSUPPORTED_FEATURE;   
    }
    
    switch(power_type)
    {
    case 0x00 : /* Current Transmit Power Level */
        p_cmd_complete->number = LC_Read_Power(p_link->device_index);
        break;

    case 0x01 : /* Maximum Transmit Power Level */
        p_cmd_complete->number = p_link->max_power_level;
        break;

    default:
        p_cmd_complete->number = 0;
        status = INVALID_HCI_PARAMETERS;
    }   
    p_cmd_complete->handle = p_link->handle;
    p_cmd_complete->status = status;

    return status;
}

#endif

/**********************************************************************
 * Function :- LMlc_LM_Clock_Offset_Req
 * 
 * Description :-
 * Handles a request for the Clock Offset to a device from the Upper Layer (HCI).
 * If the role on the current link is master then an LMP PDU is sent to the
 * peer. ( LMP_CLOCK_OFFSET_REQ ). Otherwise if Im a slave on the link the
 * clock offset can be determined locally.
 **********************************************************************/

t_error LMlc_LM_Clock_Offset_Req(t_lmp_link* p_link)
{
    t_lmp_pdu_info  pdu_info;
    u_int16  clock_offset;

    p_link->current_proc_tid_role = p_link->role;

    if ((p_link->state == LMP_PARK_MODE) && (p_link->role == MASTER))
    {
        _Send_UL_Read_Clock_Offset_Event(p_link, 0, COMMAND_DISALLOWED);
    }

    else if (p_link->role == MASTER)
    {
        pdu_info.tid_role = p_link->current_proc_tid_role;
        pdu_info.opcode = LMP_CLKOFFSET_REQ;
        LM_Encode_LMP_PDU(p_link->device_index,&pdu_info);
    }
    else
    {
        /************************************************************
         * A slave stores the clock offset locally and does not need 
         * to exchange LMP PDUs to obtain the Clock Offset.
         *
         * Note: 
         * Clock_Offset (at the HCI level) is defined as:
         *     [CLKN16-2 (Slave) - CLKN16-2 (Master) ] mod 2^15
         ***********************************************************/
        clock_offset = (LC_Read_Clock_Offset(p_link->device_index) >>2) & 0x7fff;
        _Send_UL_Read_Clock_Offset_Event(p_link,clock_offset,NO_ERROR);
   }   
   return NO_ERROR;
}


#if (PRH_BS_CFG_SYS_LMP_TIMING_INFO_SUPPORTED==1)
/**********************************************************************
 * Function :- LMlc_LM_Timing_Accuracy_Req
 * 
 * Description :- 
 * Handles a request for the drift and jitter of a peer device.
 **********************************************************************/
t_error LMlc_LM_Timing_Accuracy_Req(t_lmp_link *p_link)
{
    t_lmp_pdu_info pdu_info;

    if (SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_LMP_TIMING_INFO_FEATURE))
    {
        if (mFeat_Check_Timing_Accuracy(p_link->remote_features))
        {

            if (p_link->state == LMP_PARK_MODE)
            {
                return COMMAND_DISALLOWED;
            }

            p_link->current_proc_tid_role = p_link->role;
            pdu_info.tid_role = p_link->current_proc_tid_role;
            pdu_info.opcode = LMP_TIMING_ACCURACY_REQ;
            LM_Encode_LMP_PDU(p_link->device_index, &pdu_info);
        }
    }
    return NO_ERROR;
}
#endif

/*************************************
 *         PEER INTERFACE
 *************************************/

/**********************************************************************
 * Function :- LMlc_LMP_Incr_Power_Req
 * 
 * Description :-
 * This function handle an incoming LMP message to increment the power 
 * on the link by one unit. If the max power is reached or (if
 * the local device can transmit over 4dBm AND the peer device
 * does not support RSSI) it replies with an LMP_Max_Power Primitive
 * on the link. 
 *
 * In V1.1 and V1.2 the power_units argument is for future use and 
 * the LC_Incr_Power() will increment by 1 unit
 *
 **********************************************************************/
t_error LMlc_LMP_Incr_Power_Req(t_lmp_link* p_link,t_p_pdu p_payload)
{
#if (PRH_BS_CFG_SYS_LMP_POWER_CONTROL_SUPPORTED==1)
    u_int8 power_units = *p_payload;
    t_lmp_pdu_info pdu_info;

    p_link->current_proc_tid_role = p_link->role ^ 0x01;
    pdu_info.tid_role = p_link->current_proc_tid_role;

    /*
     * There are two possible criteria for sending LMP_max_power.
     * 1. (If local device can transmit > 4 dBm) AND
     *    (peer device does not support RSSI).
     * OR
     * 2. Local device has reached max transmit power
     *
     * NOTE: LC_Incr_Power will increment the power level
     * by one notch if not at max power. Need to do
     * the features check first.
     */
    if(((TX_POWER_LEVEL_Pmax > 4) && (!mFeat_Check_RSSI(p_link->remote_features))) 
        || (!LC_Incr_Power(p_link->device_index,power_units))
    )
    {
        pdu_info.opcode = LMP_MAX_POWER;
        LM_Encode_LMP_PDU(p_link->device_index,&pdu_info);        
    }
    return NO_ERROR;
#else
    return UNSUPPORTED_REMOTE_FEATURE;
#endif
}

/**********************************************************************
 * Function :- LMlc_LMP_Decr_Power_Req
 * 
 * Description :
 * This function handles an incoming LMP message to decrement the power 
 * on the link by one unit. If the min power is reached it replies with
 * an LMP_Min_Power Primitive on the link.
 *
 * In V1.1 the power_units argument is for future use 
 * and the LC_Decr_Power() will decrement by 1 unit
 *********************************************************************/

t_error LMlc_LMP_Decr_Power_Req(t_lmp_link* p_link,t_p_pdu p_payload)
{
#if (PRH_BS_CFG_SYS_LMP_POWER_CONTROL_SUPPORTED==1)
    u_int8 power_units = *p_payload;
    t_lmp_pdu_info pdu_info;

    p_link->current_proc_tid_role = p_link->role ^ 0x01;
    if(!LC_Decr_Power(p_link->device_index,power_units))
    {
        pdu_info.tid_role = p_link->current_proc_tid_role;
        pdu_info.opcode = LMP_MIN_POWER;
        LM_Encode_LMP_PDU(p_link->device_index,&pdu_info);        
    }
    return NO_ERROR;
#else
    return UNSUPPORTED_REMOTE_FEATURE;
#endif
}

/**********************************************************************
 * Function :- LMlc_LMP_Max_Power
 * 
 * Description :
 * Used by the peer to inform the local LM that it is at max power on the
 * link
 **********************************************************************/

t_error LMlc_LMP_Max_Power(t_lmp_link* p_link,t_p_pdu p_payload)
{
#if (PRH_BS_CFG_SYS_LMP_POWER_CONTROL_SUPPORTED==1)
     p_link->peer_power_status = MAX_POWER;
     return NO_ERROR;
#else
    return UNSUPPORTED_REMOTE_FEATURE;
#endif
}
/**********************************************************************
* Function :- LMlc_LMP_Min_Power
 * 
 * Description :-
 * Used by the peer to inform the local LM that it is at min power on the
 * link.
 **********************************************************************/

t_error LMlc_LMP_Min_Power(t_lmp_link* p_link,t_p_pdu p_payload)
{
#if (PRH_BS_CFG_SYS_LMP_POWER_CONTROL_SUPPORTED==1)
     p_link->peer_power_status = MIN_POWER;
     return NO_ERROR;
#else
    return UNSUPPORTED_REMOTE_FEATURE;
#endif
}

/**********************************************************************
* Function :- LMlc_LMP_Power_Control_Req
 * 
 * Description :-
 * Handles incoming LMP message to change the Tx power on the link.
 * 
 **********************************************************************/
t_error LMlc_LMP_Power_Control_Req(t_lmp_link* p_link,t_p_pdu p_payload)
{
#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==1)
	t_powerAdjustmentResp power_adjustment_resp;

    p_link->current_proc_tid_role = p_link->role ^ 0x01;
	if (p_payload[0] > INCR_MAX_POWER)
		return INVALID_LMP_PARAMETERS;

	if(LC_Power_Control_Req(p_link->device_index, p_link->allowed_pkt_types, (t_power_adjustment_req) p_payload[0],
		&power_adjustment_resp))
    {
		u_int8 LMlc_LMP_pwr_control_resp[3];
		LMlc_LMP_pwr_control_resp[0] = (127<<1) +  !(p_link->role);
        LMlc_LMP_pwr_control_resp[1] = (LMP_POWER_CONTROL_RESP & 0xff);
		LMlc_LMP_pwr_control_resp[2] = ((power_adjustment_resp.Mbits_1 & 0x3) | ((power_adjustment_resp.Mbits_2 & 0x3) << 2) |
                                          ((power_adjustment_resp.Mbits_3 & 0x3) << 4));

        LM_Encode_LMP_PDU_Fully_Encoded(p_link->device_index,
        LMP_POWER_CONTROL_RESP, LMlc_LMP_pwr_control_resp);

    }     
     return NO_ERROR;
#else
    return UNSUPPORTED_REMOTE_FEATURE;
#endif
}

/**********************************************************************
* Function :- LMlc_LMP_Power_Control_Resp
 * 
 * Description :-
 * Handles response LMP message to change the Tx power on the link.
 * 
 **********************************************************************/
t_error LMlc_LMP_Power_Control_Resp(t_lmp_link* p_link,t_p_pdu p_payload)
{
#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==1)
	t_powerAdjustmentResp power_adjustment_resp;

    power_adjustment_resp.Mbits_1 = (t_power_adjustment_status) (p_payload[0] & 0x3);
    power_adjustment_resp.Mbits_2 = (t_power_adjustment_status) ((p_payload[0] >> 2) & 0x3);
    power_adjustment_resp.Mbits_3 = (t_power_adjustment_status) ((p_payload[0] >> 4) & 0x3);		

	if ( ((power_adjustment_resp.Mbits_1 == NOT_SUPPORTED) || (power_adjustment_resp.Mbits_1 == AT_MAX_POWER)) &&
         ((power_adjustment_resp.Mbits_2 == NOT_SUPPORTED) || (power_adjustment_resp.Mbits_2 == AT_MAX_POWER)) &&
		 ((power_adjustment_resp.Mbits_3 == NOT_SUPPORTED) || (power_adjustment_resp.Mbits_3 == AT_MAX_POWER)) )
		p_link->peer_power_status = MAX_POWER;
	else if ( ((power_adjustment_resp.Mbits_1 == NOT_SUPPORTED) || (power_adjustment_resp.Mbits_1 == AT_MIN_POWER)) &&
              ((power_adjustment_resp.Mbits_2 == NOT_SUPPORTED) || (power_adjustment_resp.Mbits_2 == AT_MIN_POWER)) &&
       		  ((power_adjustment_resp.Mbits_3 == NOT_SUPPORTED) || (power_adjustment_resp.Mbits_3 == AT_MIN_POWER)) )
		p_link->peer_power_status = MIN_POWER;
	else
		p_link->peer_power_status = INTERMEDIATE_POWER;

    p_link->peer_power_req_tx_pending = FALSE;
    DL_Reset_RSSI_History(DL_Get_Device_Ref(p_link->device_index));

    return NO_ERROR;
#else
    return UNSUPPORTED_REMOTE_FEATURE;
#endif
}

/**********************************************************************
 * Function :- LMlc_LMP_Clock_Offset_Req
 * 
 * Description :-
 * Handles a request for the Clock Offset to a device from the Peer.
 * A Clock Offset Resp is sent to the peer.
 **********************************************************************/

t_error LMlc_LMP_Clock_Offset_Req(t_lmp_link* p_link, t_p_pdu p_payload)
{
    u_int16 clock_offset;
    t_lmp_pdu_info  pdu_info;

    p_link->current_proc_tid_role = p_link->role ^ 0x01;
    if(p_link->context == LMch_NO_CONNECTION)
    {
        p_link->context = LMch_TEMPORARY_CONNECTION;
    }

    /**************************************************
     * Note: 
     * Clock_Offset (at the LMP level) is defined as:
     *     [CLKN16-2 (Slave) - CLKN16-2 (Master) ] mod 2^15
     **************************************************/
    clock_offset = (LC_Read_Clock_Offset(p_link->device_index) >>2) & 0x7fff;
    
    pdu_info.tid_role = p_link->current_proc_tid_role;
    pdu_info.opcode = LMP_CLKOFFSET_RES;
    pdu_info.offset = clock_offset;
    LM_Encode_LMP_PDU(p_link->device_index,&pdu_info);

    return NO_ERROR;
}

/**********************************************************************
 * Function :- LMlc_LMP_Clock_Offset_Res
 * 
 * Description :-
 * Handles a response to an LMP Clock Offset Req from the Peer.
 **********************************************************************/

t_error LMlc_LMP_Clock_Offset_Res(t_lmp_link* p_link, t_p_pdu p_payload)
{   
    u_int16 clock_offset;

    /*
     * Only supply a HCI_Clock_Offset_Complete event
     * if the upper layer is expecting one.
     */
#if 0 // GF 10-July-2011 Interop Issue with propietrary HOST.
  // Below Check can result HCI_Clock_Offset_Complete_Event
  // being blocked. There is no need for this check - legacy.
  //  if(p_link->operation_pending == LMP_CLKOFFSET_REQ)
#endif
    {
        clock_offset = LMutils_Get_Uint16(p_payload);
        _Send_UL_Read_Clock_Offset_Event(p_link,clock_offset,NO_ERROR);
    }
    return NO_ERROR;
}

/**********************************************************************
 * Function :- LMlc_LMP_Slot_Offset
 * 
 * Description :-
 * Handles an incoming LMP_Slot_Offset PDU and stores it in the link 
 * structure.
 **********************************************************************/

t_error LMlc_LMP_Slot_Offset(t_lmp_link* p_link,t_p_pdu p_payload)
{  
#if (PRH_BS_CFG_SYS_LMP_SLOT_OFFSET_SUPPORTED==1)
    p_link->slot_offset = LMutils_Get_Uint16(p_payload);
    LC_Write_Remote_Slot_Offset(p_link->device_index, p_link->slot_offset);
#endif

    return NO_ERROR;
    
}

#if (PRH_BS_CFG_SYS_LMP_TIMING_INFO_SUPPORTED==1)
/**********************************************************************
 * Function :- LMlc_LMP_Timing_Accuracy_Req
 * 
 * Description :-
 * Handles a response to an LMP_Timing_Accuracy_Req from the Peer.
 **********************************************************************/

t_error LMlc_LMP_Timing_Accuracy_Req(t_lmp_link* p_link, t_p_pdu p_payload)
{
    t_lmp_pdu_info  pdu_info;
 
    p_link->current_proc_tid_role = p_link->role ^ 0x01;

    pdu_info.tid_role = p_link->current_proc_tid_role;
    pdu_info.opcode = LMP_TIMING_ACCURACY_RES;
    pdu_info.drift = LC_Read_Local_Device_Drift();
    pdu_info.jitter = LC_Read_Local_Device_Jitter();
    LM_Encode_LMP_PDU(p_link->device_index,&pdu_info);
     
    return NO_ERROR;
}
#endif

#if (PRH_BS_CFG_SYS_LMP_TIMING_INFO_SUPPORTED==1)
/**********************************************************************
 * Function :- LMlc_LMP_Timing_Accuracy_Res
 * 
 * Description :-
 * Handles a response to a "Timing_Accuracy_Req" LMP PDU
 **********************************************************************/

t_error LMlc_LMP_Timing_Accuracy_Res(t_lmp_link* p_link, t_p_pdu p_payload)
{
    /*
     * Safe enough to receive an lmp timing accuracy
     * response in all states.
     */
    LC_Write_Device_Drift(p_link->device_index,(u_int8)*(p_payload));
    LC_Write_Device_Jitter(p_link->device_index, (u_int8)*(p_payload+1));
    return NO_ERROR;
}
#endif
/**********************************************************************
 * Function :- LMlc_LMP_Test_Activate
 * 
 * Description 
 * Triggers a slave to enter Test Mode. The slave device can only enter 
 * test mode if DUT is enabled. Receipt of the LMP_Test_Activate by a 
 * device with DUT enabled will cause the device to go to the DUT ACTIVE 
 * state.
 *********************************************************************/

t_error LMlc_LMP_Test_Activate(t_lmp_link* p_link, t_p_pdu p_payload)
{
    if ((DUT_ENABLED == BTtst_Get_DUT_Mode()) && (SLAVE == p_link->role))
    {
        BTtst_Set_DUT_Mode(DUT_ACTIVE);
        /*
         * Bug 723 scanning should only be temporarily suppressed
         *    [was LMscan_Write_Scan_Enable(NO_SCANS_ENABLED);]
         */
        LMconfig_LM_Set_Device_State(PRH_DEV_STATE_TEST_MODE );

        /* Enable L2CAP Traffic */
        LC_Start_Peer_Tx_L2CAP_Data(p_link->device_index);
        LC_Start_Local_Tx_L2CAP_Data(p_link->device_index);

        /* Send the LMP_Accepted */
        p_link->current_proc_tid_role = p_link->tid_role_last_rcvd_pkt;
        LM_Encode_LMP_Accepted_PDU(p_link,LMP_TEST_ACTIVATE);
    }
    else
    {
        return LMP_PDU_NOT_ALLOWED;
    }
    return NO_ERROR;
}

/***************************************************************
 *  FUNCTION : LMlc_LMP_Test_Control
 *
 *  DESCRIPTION :
 *  Controls/Configures the tests being performed in the remote device.
 *
 ****************************************************************/

t_error LMlc_LMP_Test_Control(t_lmp_link* p_link, t_p_pdu p_payload)
{
    t_error     status = NO_ERROR; 

    if ((BTtst_Get_DUT_Mode() >= DUT_ACTIVE) && (SLAVE == p_link->role))
    {
        /* INTERACT WITH THE LC TO INVOKE TESTMODE */
        /* RGB this sets up test mode and must happen before the LMP_accept
            is sent since the LC is setup to do a callback to change
            the IUT to test settings (whitening, hopping mode etc)
            when the command is acked. Also we can check if the test requested
            is supported and so return a not accepted if not. */
        status = LC_Testmode_Request(p_link->device_index,p_payload);

		p_link->l2cap_tx_enable = LM_CH_L2CAP_EN_ALL;
        if (status == NO_ERROR)
        {
            if (p_payload[0] == 0xFF)
			{
                LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_TEST_MODE);

			}

            LM_Encode_LMP_Accepted_PDU(p_link,LMP_TEST_CONTROL);

			if (p_payload[0] == 0xFF)
			{
				p_link->allowed_pkt_types = (1UL<<DM1);
			}
			else
			{
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
				p_link->allowed_pkt_types = BTtst_Get_Allowed_Packets();
#endif
			}

            return status;
        }
    }
    else
    {
        status = LMP_PDU_NOT_ALLOWED;
    }
    return status;
}

/***************************************************************
 *  FUNCTION : LMlc_LMP_Not_Accepted
 *
 *  DESCRIPTION :
 *  Handles the non acceptance of an lmp operation by the peer.
 *
 ****************************************************************/

t_error LMlc_LMP_Not_Accepted(t_lmp_link* p_link, u_int16 opcode, t_error reason)
{
    t_error status = NO_ERROR;

    switch(opcode)
    {
#if (PRH_BS_CFG_SYS_LMP_TIMING_INFO_SUPPORTED==1)
    case LMP_TIMING_ACCURACY_REQ:
        /* 
         * If the device refuses the timing accuracy request worst case 
         * parameters must be assumed. See page 206 of BT Ver 1.1
         */
        LC_Write_Device_Drift(p_link->device_index,(u_int8)250);
        LC_Write_Device_Jitter(p_link->device_index,(u_int8)10);
        break;
#endif
    case LMP_CLKOFFSET_REQ :
       _Send_UL_Read_Clock_Offset_Event(p_link,0,reason);
       break;
#if (PRH_BS_CFG_SYS_LMP_POWER_CONTROL_SUPPORTED==1)
    case LMP_DECR_POWER_REQ:
        if(reason == UNSUPPORTED_REMOTE_FEATURE)
            p_link->remote_features[2] &= ~POWER_CONTROL_MASK;
        else
            LMlc_LMP_Min_Power(p_link, 0);
        break;

    case LMP_INCR_POWER_REQ:
        if(reason == UNSUPPORTED_REMOTE_FEATURE)
            p_link->remote_features[2] &= ~POWER_CONTROL_MASK;
        else
            LMlc_LMP_Max_Power(p_link, 0);
        break;
#endif

    case LMP_PAGE_MODE_REQ :
    case LMP_PAGE_SCAN_MODE_REQ :
        break;

    default :
        status = UNSPECIFIED_ERROR;
        break;
    }

    return status;
}


/***************************************************************
 *  FUNCTION : _Send_UL_Read_Clock_Offset_Event
 *
 *  DESCRIPTION :
 *  Sends a clock offset event to the higher layer.
 *
 ****************************************************************/

void _Send_UL_Read_Clock_Offset_Event(t_lmp_link* p_link,u_int16 clock_offset, t_error status)
{
    t_lm_event_info clock_offset_event;
                         
    clock_offset_event.status = status;
    clock_offset_event.handle = p_link->handle;
    clock_offset_event.value16bit = clock_offset;
    g_LM_config_info.lmp_event[_LM_READ_CLOCK_OFFSET_EVENT](HCI_READ_CLOCK_OFFSET_EVENT,&clock_offset_event);
}

t_error LMlc_LMP_Page_Mode_Req(t_lmp_link* p_link, t_p_pdu p_pdu)
{
     return UNSUPPORTED_REMOTE_FEATURE;
}

t_error LMlc_LMP_Page_Scan_Mode_Req(t_lmp_link* p_link, t_p_pdu p_pdu)
{
     return UNSUPPORTED_REMOTE_FEATURE;
}

