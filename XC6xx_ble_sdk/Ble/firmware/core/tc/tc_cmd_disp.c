/**************************************************************************
 *
 * MODULE NAME:    tc_cmd_disp.c
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    Test Control command dispatcher
 * MAINTAINER:     Daire McNamara <Daire.McNamara@sslinc.com>
 * CREATION DATE:  13 April 2000
 *
 * SOURCE CONTROL: $Id: tc_cmd_disp.c,v 1.82 2013/06/20 11:54:03 garyf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *
 * SOURCE: Based on V2.0 HC Design
 * ISSUES:
 *
 *************************************************************************/

#include "sys_config.h"
#include "hc_const.h"
#include "hc_event_gen.h"
#include "hci_params.h"
#include "hw_radio.h"
#include "tc_const.h"
#include "tc_interface.h"
#include "tc_event_gen.h"

#include "lslc_stat.h"
#include "lmp_afh.h"

#include "le_config.h"
#include "le_advertise.h"
#include "le_scan.h"
#include "le_connection.h"


#include "hw_lc.h"
#include "hw_le_lc.h"
#include "patch_function_id.h"

extern t_LE_Config LE_config;

#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==1)
/*****************************************************************
 *  FUNCTION :- TCI_Dispatch_Prop_Command
 *
 *  DESCRIPTION 
 *  Handles any Parthus TCI proprietary commands
 *****************************************************************/ 
t_error TCI_Dispatch_Prop_Command(t_p_pdu p_payload, u_int16 opcode)
{
    //u_int16 ocf;
    //ocf = ( opcode & 0x03FF);
    if(PATCH_FUN[TCI_DISPATCH_PROP_COMMAND_ID]){
         
         return ((t_error (*)(t_p_pdu p_payload, u_int16 opcode))PATCH_FUN[TCI_DISPATCH_PROP_COMMAND_ID])(p_payload,   opcode);
    }

	
	
    switch(opcode) 
    {
#ifndef BLUETOOTH_MODE_LE_ONLY
    case TCI_WRITE_LOCAL_HARDWARE_REGISTER:
        TC_Write_Local_Hardware_Register(*(p_payload+3), HCIparam_Get_Uint32(p_payload+4), HCIparam_Get_Uint32(p_payload+8), HCIparam_Get_Uint32(p_payload+12));
        break;
    case TCI_ACTIVATE_REMOTE_DUT:
        TC_Activate_Remote_DUT((u_int16)(p_payload[3] + ((p_payload[4]&0x0F) << 8)));
        break;
    case TCI_TEST_CONTROL:
    case TCI_TYPE_APPROVAL_TEST_CONTROL:
        TC_Perform_Test_Control(p_payload+3);
        break;
    case TCI_INCREASE_REMOTE_POWER:
        TC_Increase_Remote_Power((unsigned short)(((*(p_payload+3)) + ((*(p_payload+4)) << 8))));
        break;
    case TCI_WRITE_LOCAL_HOP_FREQUENCIES:
        TC_Write_Local_Hop_Frequencies(p_payload+3);
        break;
    case TCI_READ_LOCAL_HARDWARE_VERSION:
        TC_Read_Local_Hardware_Version();
        break;
    case TCI_DECREASE_REMOTE_POWER:
        TC_Decrease_Remote_Power((unsigned short)(((*(p_payload+3)) + ((*(p_payload+4)) << 8))));
        break;
	case TCI_INCREASE_LOCAL_VOLUME:
        TC_Increase_Local_Volume();
        break;
    case TCI_DECREASE_LOCAL_VOLUME:
        TC_Decrease_Local_Volume();
        break;
    case TCI_WRITE_LOCAL_NATIVE_CLOCK:
        TC_Write_Local_Native_Clock(p_payload+3);
        break;
    case TCI_READ_LOCAL_NATIVE_CLOCK:
        TC_Read_Local_Native_Clock();
        break;
    case TCI_READ_LOCAL_HOST_CONTROLLER_RELATIVE_MIPS:
        TC_Read_Local_Relative_Host_Controller_Mips();
        break;
    case TCI_WRITE_LOCAL_HOPPING_MODE:
        TC_Write_Local_Hopping_Mode(p_payload+3);
        break;
    case TCI_READ_LOCAL_HOPPING_MODE:
        TC_Read_Local_Hopping_Mode();
        break;
    case TCI_WRITE_LOCAL_WHITENING_ENABLE:
    	if (*(p_payload+3))
    		HWle_set_whitening_enable();
    	else
    		HWle_clear_whitening_enable();

        TC_Write_Local_Whitening_Enable(p_payload+3);
        break;
    case TCI_READ_LOCAL_WHITENING_ENABLE:
        TC_Read_Local_Whitening_Enable();
        break;
    case TCI_WRITE_LOCAL_LINK_KEY_TYPE:
        TC_Write_Local_Link_Key_Type(p_payload+3);
        break;
    case TCI_READ_LOCAL_LINK_KEY_TYPE:
        TC_Read_Local_Link_Key_Type();
        break;
    case TCI_READ_LOCAL_EXTENDED_FEATURES:
        TC_Read_Local_Extended_Features();
        break;
    case TCI_WRITE_LOCAL_FEATURES:
        TC_Write_Local_Features(p_payload+3);
        break;
    case TCI_WRITE_LOCAL_EXTENDED_FEATURES:
        TC_Write_Local_Extended_Features(p_payload+3);
        break;
    case TCI_READ_LOCAL_TIMING_INFORMATION:
        TC_Read_Local_Timing_Information();
        break;
    case TCI_WRITE_LOCAL_TIMING_INFORMATION:
        TC_Write_Local_Timing_Information(p_payload+3);
        break;
#if (PRH_BS_CFG_SYS_LMP_TIMING_INFO_SUPPORTED==1)
    case TCI_READ_REMOTE_TIMING_INFORMATION:
        TC_Read_Remote_Timing_Information(p_payload+3);
        break;
#endif
    case TCI_RESET_LOCAL_PUMP_MONITORS:
        TC_Reset_Local_Pump_Monitors(p_payload+3);
        break;
    case TCI_READ_LOCAL_PUMP_MONITORS:
        TC_Read_Local_Pump_Monitors(p_payload+3);
        break;
    case TCI_WRITE_LOCAL_ENCRYPTION_KEY_LENGTH:
        TC_Write_Local_Encryption_Key_Length(p_payload+3,p_payload+4);
        break;
    case TCI_READ_LOCAL_ENCRYPTION_KEY_LENGTH:
        TC_Read_Local_Encryption_Key_Length();
        break;
    case TCI_READ_LOCAL_HOP_FREQUENCIES:
        TC_Read_Local_Hop_Frequencies();
        break;
    case TCI_READ_LOCAL_BASEBAND_MONITORS:
        TC_Read_Local_Baseband_Monitors();
        break;
    case TCI_RESET_LOCAL_BASEBAND_MONITORS:
        LSLCstat_Reset_Monitors();
        TCeg_Command_Complete_Event(TCI_RESET_LOCAL_BASEBAND_MONITORS, NO_ERROR);
        break;
    case TCI_READ_LOCAL_DEFAULT_PACKET_TYPE:
        TC_Read_Local_Default_Packet_Type();
        break;
    case TCI_WRITE_LOCAL_DEFAULT_PACKET_TYPE:
        TC_Write_Local_Default_Packet_Type(*(p_payload+3));
        break;
    case TCI_SET_LOCAL_FAILED_ATTEMPTS_COUNTER:
        TC_Set_Local_Failed_Attempts_Counter(p_payload+3);
        break;
    case TCI_CLEAR_LOCAL_FAILED_ATTEMPTS_COUNTER:
        TC_Clear_Local_Failed_Attempts_Counter(p_payload+3);
        break;
    case TCI_SET_LOCAL_NEXT_AVAILABLE_AM_ADDR:
        TC_Set_Local_Next_Available_Am_Addr(*(p_payload+3));      
        break;
    case TCI_SET_LOCAL_BD_ADDR:
        TC_Set_Local_Bd_Addr((p_payload+3));
        break;
    case TCI_WRITE_LOCAL_RADIO_POWER:
        TC_Write_Local_Radio_Power(HCIparam_Get_Uint16(p_payload+3), HCIparam_Get_Uint16(p_payload+5));
        break;
    case TCI_READ_LOCAL_RADIO_POWER:
        TC_Read_Local_Radio_Power(HCIparam_Get_Uint16(p_payload+3));
        break;
    case TCI_WRITE_LOCAL_SYNCWORD:
        TC_Write_Local_SyncWord((p_payload+3));
        break;
    case TCI_WRITE_LOCAL_RADIO_REGISTER:
        TC_Write_Local_Radio_Register(*(p_payload+3), HCIparam_Get_Uint16(p_payload+4));
        break;
    case TCI_READ_LOCAL_RADIO_REGISTER:
        TC_Read_Local_Radio_Register(*(p_payload+3));
        break;
    case TCI_CHANGE_RADIO_MODULATION:
        TC_Change_Radio_Modulation(p_payload+3);
        break;
    case TCI_READ_RADIO_MODULATION:
        TC_Read_Radio_Modulation();
        break;
    case TCI_SET_DISABLE_LOW_POWER_MODE:
        TC_Set_Disable_Low_Power_Mode();
        break;
    case TCI_SET_ENABLE_LOW_POWER_MODE:
        TC_Set_Enable_Low_Power_Mode();
        break;
    case TCI_READ_R2P_MIN_SEARCH_WINDOW:
        TC_Read_R2P_Min_Search_Window();
        break;
    case TCI_WRITE_R2P_MIN_SEARCH_WINDOW:
        TC_Write_R2P_Min_Search_Window(*(p_payload+3));
        break;
    case TCI_SET_HCIT_UART_BAUD_RATE:
        TC_Set_HCIT_UART_Baud_Rate(HCIparam_Get_Uint32(p_payload+3));
        break;
    case TCI_WRITE_PARK_PARAMETERS :
        TC_Write_Park_Parameters(p_payload+3);
        break;
    case TCI_SET_BROADCAST_SCAN_WINDOW:
        TC_Set_Broadcast_Scan_Window(p_payload+3);
        break;
    case TCI_READ_UNUSED_STACK_SPACE :
        TC_Read_Unused_Stack_Space();
        break;
#if (PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_SUPPORTED==1)
    case TCI_WRITE_AFH_CONTROL :
        LMafh_Set_AFH_Control_Byte(p_payload[3]);
        TCeg_Command_Complete_Event(TCI_WRITE_AFH_CONTROL, NO_ERROR);
        break;
#endif
    case TCI_VCI_CLK_OVERRIDE:
        TC_Write_VCI_CLK_Override(p_payload[2],p_payload+3);
        break;
    case TCI_SEND_ENCRYPTION_KEY_SIZE_MASK_REQ:
        TC_Send_Encryption_Key_Size_Mask_Req(HCIparam_Get_Uint16(p_payload+3));
        break;
    case TCI_READ_RAW_RSSI:
        TC_Read_Raw_RSSI(HCIparam_Get_Uint16(p_payload+3));
        break;
    case TCI_READ_BER:
        TC_Read_BER();
        break;
    case TCI_READ_PER:
        TC_Read_PER();
        break;
    case TCI_READ_RAW_RSSI_PER_BER:
        TC_Read_Raw_RSSI_PER_BER();
        break;
#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==1)
    case TCI_WRITE_EPC_ENABLE:
        TC_Write_EPC_Enable(*(p_payload+3));
        break;
#endif
#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
    case TCI_WRITE_PTA_ENABLE:
        TC_Write_PTA_Enable(*(p_payload+3),*(p_payload+4),*(p_payload+5));
        break;
#endif
    case TCI_SET_DISABLE_SCO_VIA_HCI:
		TC_Set_Disable_SCO_Via_HCI();
        break;
    case TCI_SET_ENABLE_SCO_VIA_HCI:
		TC_Set_Enable_SCO_Via_HCI();
        break;

     case TCI_WRITE_ESCO_RETRANSMISSION_MODE:
        TC_Write_eSCO_Retransmission_Mode(*(p_payload+3));
        break;
    case TCI_READ_ESCO_RETRANSMISSION_MODE:
        TC_Read_eSCO_Retransmission_Mode();
        break;
    case TCI_WRITE_SECURITY_TIMEOUTS:
	TC_Write_Security_Timeouts(HCIparam_Get_Uint16(p_payload+3), HCIparam_Get_Uint16(p_payload+5), *(p_payload+7));
	break;
     case TCI_WRITE_FEATURES:
	TC_Write_Features(HCIparam_Get_Uint16(p_payload+3),p_payload[5]);
	break;

#if(PRH_BS_CFG_SYS_BROADCAST_NULL_IN_INQ_PAGE_SUPPORTED==1)
	case TCI_SET_EMERGENCY_POLL_INTERVAL:
		TC_Set_Emergency_Poll_Interval(HCIparam_Get_Uint16(p_payload+3));
		break;
#endif

#endif//BLUETOOTH_MODE_LE_ONLY

	case TCI_LE_SET_TRANSMIT_WINDOW_PARAMS :
		LEconnection_TCI_Set_Transmit_Window_Params(HCIparam_Get_Uint16(p_payload+3), *(p_payload+5));
		break;

	case TCI_LE_SET_DIRECT_ADVERTISING_TIMEOUT :
		LEadv_TCI_Set_Direct_Adv_Timeout(HCIparam_Get_Uint16(p_payload+3));
		break;

	case TCI_LE_SET_TIFS_TX_ADJUSTMENT :
		LEconfig_TCI_Set_TIFS_Tx_Adjustment(*(p_payload+3));
		break;

	case TCI_LE_SET_SEARCH_WINDOW_DELAY :
		LEconfig_TCI_Set_Search_Window_Delay(*(p_payload+3));
		break;

	case TCI_LE_AUTO_ADVERTISE_AFTER_SLAVE_DISCONNECT :
		LE_config.auto_advertise = *(p_payload+3);
		TCeg_Command_Complete_Event(TCI_LE_AUTO_ADVERTISE_AFTER_SLAVE_DISCONNECT, NO_ERROR);
		break;

	case TCI_LE_AUTO_INITIATE_AFTER_MASTER_DISCONNECT :
		LE_config.auto_initiate = *(p_payload+3);
		TCeg_Command_Complete_Event(TCI_LE_AUTO_INITIATE_AFTER_MASTER_DISCONNECT, NO_ERROR);
		break;

#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
	case TCI_LE_WRITE_SCAN_FREQUENCIES :
		LEscan_TCI_Write_Scan_Freqs(*(p_payload+3));
		break;

	case TCI_LE_READ_SCAN_FREQUENCIES :
		LEscan_TCI_Read_Scan_Freqs();
		break;

	case TCI_LE_WRITE_INITIATING_FREQUENCIES :
		LEscan_TCI_Write_Initiating_Freqs(*(p_payload+3));
		break;

	case TCI_LE_READ_INITIATING_FREQUENCIES :
		LEscan_TCI_Read_Initiating_Freqs();
		break;
#endif

	case TCI_LE_SLAVE_LISTEN_OUTSIDE_LATENCY :
		LE_config.slave_listen_outside_latency = (*(p_payload+3));
		TCeg_Command_Complete_Event(TCI_LE_SLAVE_LISTEN_OUTSIDE_LATENCY, NO_ERROR);
		break;

#if ((PRH_BS_CFG_SYS_LE_BROADCAST_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1))
	case TCI_LE_DIRECT_ADV_TIMEOUT :
		LEadv_TCI_Set_Direct_Adv_Timeout(HCIparam_Get_Uint16(p_payload+3));
		break;

	case TCI_LE_WRITE_ADVERTISING_DELTA :
		LEadv_TCI_Write_Advertising_Delta(*(p_payload+3));
		break;
#endif
	case TCI_LE_PRECONFIGURE_HOP_INC:
		LE_config.preConfigured_hop_inc_available = 1;
		LE_config.preConfigured_hop_inc = (*(p_payload+3));
		TCeg_Command_Complete_Event(TCI_LE_PRECONFIGURE_HOP_INC, NO_ERROR);
		break;

	case TCI_LE_PRECONFIGURE_ACCESS_CODE :
		LE_config.preConfigured_access_address_available = 1;
		LE_config.preConfigured_access_address = HCIparam_Get_Uint32(p_payload+3);
		TCeg_Command_Complete_Event(TCI_LE_PRECONFIGURE_ACCESS_CODE, NO_ERROR);
		break;

#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
	case TCI_LE_ENABLE_SCAN_BACKOFF :
		LEscan_TCI_Enable_Scan_Backoff(*(p_payload+3));
		TCeg_Command_Complete_Event(TCI_LE_ENABLE_SCAN_BACKOFF, NO_ERROR);
		break;

	case TCI_LE_READ_SCAN_BACKOFF_INFO :
		LEscan_TCI_Read_Scan_Backoff_Info();
		break;

	case TCI_LE_READ_SCAN_PARAMS :
		LEscan_TCI_Read_Scan_Params();
		break;

	case TCI_LE_READ_LOCAL_POWER_LEVEL :

		break;


	case TCI_LE_INC_LOCAL_POWER_LEVEL :
		{
			t_q_descr *qd;

			qd = BTq_Enqueue(HCI_EVENT_Q, 0, 6);
			if(qd)
			{
				qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
				qd->data[1] = 4;
				qd->data[2] = 1;
				_Insert_Uint16(&(qd->data[3]), TCI_LE_INC_LOCAL_POWER_LEVEL);
				//qd->data[5] = HWradio_Inc_Tx_Power_Level();
                qd->data[5] = ((t_error (*)(void))PATCH_FUN[HWRADIO_INC_TX_POWER_LEVEL])();
				BTq_Commit(HCI_EVENT_Q, 0);
			}
		}
		break;

	case TCI_LE_DEC_LOCAL_POWER_LEVEL :
		{
			t_q_descr *qd;

			qd = BTq_Enqueue(HCI_EVENT_Q, 0, 6);
			if(qd)
			{
				qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
				qd->data[1] = 4;
				qd->data[2] = 1;
				_Insert_Uint16(&(qd->data[3]), TCI_LE_DEC_LOCAL_POWER_LEVEL);
				//qd->data[5] = HWradio_Dec_Tx_Power_Level();
                qd->data[5] = ((t_error (*)(void))PATCH_FUN[HWRADIO_DEC_TX_POWER_LEVEL])();
				BTq_Commit(HCI_EVENT_Q, 0);
			}
		}
		break;
#endif

#if ((PRH_BS_CFG_SYS_LE_BROADCAST_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1))
	case TCI_LE_READ_ADV_PARAMS :
		LEadv_TCI_Read_Advertising_Params();
		break;
#endif

	case TCI_LE_READ_ACCESS_CODE :
		LEconnetion_TCI_Read_Access_Code(HCIparam_Get_Uint16(p_payload+3));
		break;

	case TCI_LE_READ_HOP_INCREMENT :
		LEconnetion_TCI_Read_Hop_Increment(HCIparam_Get_Uint16(p_payload+3));
		break;

	case TCI_LE_READ_SESSION_KEY:
		LEconnection_TCI_Read_Session_Key(HCIparam_Get_Uint16(p_payload+3));
		break;

	case TCI_LE_READ_PEER_SCA :
		LEconnection_TCI_Read_Peer_SCA(HCIparam_Get_Uint16(p_payload+3));
		break;
#if 0
	case TCI_WRITE_LOCAL_WHITENING_ENABLE :
		if (*(p_payload+3))
			HWle_set_whitening_enable();
		else
			HWle_clear_whitening_enable();

		TCeg_Command_Complete_Event(TCI_WRITE_LOCAL_WHITENING_ENABLE, NO_ERROR);
		break;
#endif
	case TCI_LE_SET_TRACE_LEVEL :
		LE_config.trace_level = *(p_payload+3);
		TCeg_Command_Complete_Event(TCI_LE_SET_TRACE_LEVEL, NO_ERROR);
		break;

	case TCI_LE_ECHO_TRANSMIT_WINDOW_SIZE_AND_OFFSET :
		LEconnection_TCI_Read_Window_Size_and_Offset(HCIparam_Get_Uint16(p_payload+3));
		break;

	/* Below commands have yet to be completed - to be added prior to UPF 42 - June 2012 */
	case TCI_LE_WRITE_NUM_PACKETS_PER_CE :
	case TCI_LE_GET_DEVICE_STATES :
	case TCI_LE_NUM_TIMES_MASTER_DOESNT_TX_FIRST_WIN :

     default :
         return ILLEGAL_COMMAND;
     }
    return NO_ERROR;
}

#endif
