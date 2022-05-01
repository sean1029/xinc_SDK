/**************************************************************************
 * MODULE NAME:    le_link_layer.c       
 * DESCRIPTION:    LE Link Layer Engine
 * AUTHOR:         Gary Fleming
 * DATE:           
 *
 *
 * LICENSE:
 *     This source code is copyright (c) 2011-2012 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *
 * NOTES:
 *
 **************************************************************************/

#include "sys_config.h"
#include "sys_rand_num_gen.h"
#include "sys_mmi.h"
#include "sys_develop.h"
#include "hc_const.h"
#include "hc_event_gen.h"
#include "tc_event_gen.h"

#include "hci_params.h"

#include "lmp_utils.h"
#include "le_advertise.h"
#include "le_scan.h"
#include "le_const.h"
#include "le_config.h"
#include "le_connection.h"
#include "le_link_layer.h"
#include "le_white_list.h"
#include "le_frequency.h"
#include "le_security.h"
#include "le_aes.h"
#include "le_test.h"

#include "tc_event_gen.h"

#include "hw_lc.h"
#include "hw_radio.h"
#include "hw_le_lc.h"
#include "hw_le_lc_impl.h"

#include "lc_interface.h"
#include "lslc_irq.h"
#include "lslc_stat.h"
#include "bt_timer.h"
#if (PRH_BS_CFG_SYS_LOW_POWER_MODE_SUPPORTED==1)
#include "sys_power.h"
#include "uslc_sleep.h"
#endif
#include "sys_debug_config.h"
#include "le_power.h"
#include "global_val.h"
#include "patch_function_id.h"
#if (PRH_BS_CFG_SYS_TRACE_ADV_VIA_HCI_SUPPORTED==1)
 u_int8  PRH_rx_adv_dbg_channel = 0;
 u_int8  PRH_rx_adv_dbg_length  = 0;
 u_int16 PRH_rx_adv_dbg_header  = 0;

 u_int8  PRH_tx_adv_dbg_channel = 0;
 u_int8  PRH_tx_adv_dbg_length  = 0;
 u_int16 PRH_tx_adv_dbg_header  = 0;
#endif

//unsigned int after_rx_first_packet = 0;
extern t_LE_Config LE_config;
//extern volatile uint8_t __attribute__((aligned(4))) DMA_buf[4];

#define _LE_LL_CHECK_ADV_PKT_LENGTH(length)  ((length<=LE_MAX_ADV_CHANNEL_PDU_LEN) && (length >= LE_MIN_ADV_CHANNEL_PDU_LEN))

/************************************************************************
 * Prototypes for the Functions used to handle LLC messages 
 ************************************************************************/
t_error LE_LL_Connection_Update_Req(t_LE_Connection* p_connection, t_p_pdu p_pdu);
t_error LE_LL_Version_Ind(t_LE_Connection* p_connection,t_p_pdu p_pdu);
t_error LE_LL_Start_Enc_Req(t_LE_Connection* p_connection,t_p_pdu p_pdu);
t_error LE_LL_Start_Enc_Rsp(t_LE_Connection* p_connection,t_p_pdu p_pdu);
t_error LE_LL_Pause_Enc_Req(t_LE_Connection* p_connection,t_p_pdu p_pdu);
t_error LE_LL_Pause_Enc_Rsp(t_LE_Connection* p_connection,t_p_pdu p_pdu);
t_error LE_LL_Feature_Req(t_LE_Connection* p_connection,t_p_pdu p_pdu);
t_error LE_LL_Feature_Rsp(t_LE_Connection* p_connection,t_p_pdu p_pdu);
t_error LE_LL_Enc_Req(t_LE_Connection* p_connection,t_p_pdu p_pdu);
t_error LE_LL_Enc_Rsp(t_LE_Connection* p_connection,t_p_pdu p_pdu);
t_error LE_LL_Unknown_Rsp(t_LE_Connection* p_connection,t_p_pdu p_pdu);
t_error LE_LL_Terminate_Ind(t_LE_Connection* p_connection,t_p_pdu p_pdu);
t_error LE_LL_Reject_Ind(t_LE_Connection* p_connection,t_p_pdu p_pdu);
t_error LE_LL_UnSupported_Pdu(t_LE_Connection* p_connection,t_p_pdu p_pdu);

typedef t_error (*t_ll_decode_function)(t_LE_Connection* p_connection , t_p_pdu p_pdu);



t_error LE_LL_Ctrl_Handle_LE_IRQ(u_int8 IRQ_Type);
t_error LE_LL_Ctrl_Handle_Connection_Event(u_int8 IRQ_Type,u_int32 current_clk);

//u_int8 LE_LL_Get_CRC_Err(void);

//s_int8 last_adv_rx_rssi = 0;
extern  u_int32 u_int32_TX_buffer[12];
extern  u_int32 u_int32_RX_buffer[12];

#if (__DEBUG_LE_SLEEP == 1)
#define HEADER 4
#define TAILER 2
float period=30.52;

#define CAL_AUX_TIMER(x)	((int)((312.5*((4*(x)) - HEADER - TAILER) -2*period)/period /8))
#define CAL_HEAD_TIMER(x)   ((int)(312.5*x - 2*period)/period /8)
#define CAL_OFFSET(x)	((((int)(x*8*period + 2*period)))%1250)


//cal event times
//unsigned	int pkd_current_clk = 0;

//unsigned char adv_direct_sleep_flg = 0;

//unsigned char bt_bb_sleep_flg = 0;
#endif

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)

/**********************************************************************
 * Function :- Vaidate_Access_Address
 *
 * Description :- 
 * This function validates a proposed Access Address to ensure it is compliant 
 * with the requirements of the Low Energy Spec.
 * The following are the requirement on a valid Access Address
 *
 * ?It shall have no more than six consecutive zeros or ones.
 * ?It shall not be the advertising channel packets?Access Address.
 * ?It shall not be a sequence that differs from the advertising channel packets?
 *   Access Address by only one bit.
 * ?It shall not have all four octets equal.
 * ?It shall have no more than 24 transitions.
 * ?It shall have a minimum of two transitions in the most significant six bits
 **********************************************************************/

u_int8 Vaidate_Access_Address(u_int32 proposed_address)
{
    if(PATCH_FUN[VAIDATE_ACCESS_ADDRESS_ID]){
         
         return ((u_int8 (*)(u_int32 ))PATCH_FUN[VAIDATE_ACCESS_ADDRESS_ID])(proposed_address);
    }
	//
	// First Check that it is different from the Advertising Access Address
	// Second Check that all Four octets are not equal.
	// No More than 6 consequtive zeros or ones.
	// No more than 24 transitions
	// 2 transitions in the most significant 6 bits.

	if (proposed_address == ADVERTISING_ACCESS_ADDRESS)
		return 0;

	// Check the 4 octets are not equal.
	//  
	{
		u_int8 octet1 = proposed_address & 0xFF;
		u_int8 octet2 = (proposed_address & 0xFF00)>>8;
		u_int8 octet3 = (proposed_address & 0xFF0000)>>16;
		u_int8 octet4 = (proposed_address  & 0xFF000000)>>24;

		if ((octet1 & octet2 & octet3 & octet4 ) == octet1)
			return 0;
	}
	// Check for no more than 6 consequtive zeros or ones.
	// 
	{
		u_int8 num_consequtive_ones = 0;
		u_int8 num_consequtive_zeros = 0;
		u_int8 num_transitions = 0;
		u_int8 num_transition_in_upper_6_bits = 0;
		u_int8 pre_bitVal=0;
		u_int8 diffs_from_advertising_access=0;
		u_int8 j;
		u_int8 bitVal;
		u_int8 bitValAdvertising;

		for (j=0;j<32;j++)
		{
			bitVal = (proposed_address >> j) & 0x0001;
			bitValAdvertising = (ADVERTISING_ACCESS_ADDRESS >> j) & 0x0001;

			if (j > 0)
			{
				if (bitVal == pre_bitVal)
				{
					if(bitVal)
					{
						num_consequtive_ones++;
					}
					else
					{
						num_consequtive_zeros++;
					}
				}
				else
				{
					num_consequtive_ones=0;
					num_consequtive_zeros=0;
					num_transitions++;

					if (j > 25)
					{
						num_transition_in_upper_6_bits++;
					}
				}
			}
			if (bitVal != bitValAdvertising)
			{
				diffs_from_advertising_access++;
			}
			pre_bitVal = bitVal;

			if ((num_consequtive_ones > 5)||
				(num_consequtive_zeros > 5) ||
				(num_transitions > 24))
				return 0;
		}
		if (!(diffs_from_advertising_access > 1))
			return 0;

		if (num_transition_in_upper_6_bits < 2)
			return 0;
	}
	return 1;
}
#endif

/***************************************************************************
 * Function :- LEll_Decode_Advertising_ChannelPDU
 *
 * Description :- 
 * This is key function in the LE link layer which decodes incoming advertising channel PDUs.
 * The actions performed are dependant on the PDU type, however for all the function return 
 * value is CRITICAL - as this indicates if the Advertiser should send a response or not.
 *
 * The actions performed are overview for each of the PDU types :
 * 
 * ADV_NONCONN_IND
 * If this PDU is recieved in the SCANNING_STATE then the Advertisiing Event is generated depending
 * on the duplicate handling and white lists.
 * No Response is generated following receipt of this PDU.
 *
 * ADV_IND
 * As this PDU can be recieved in either the Scanning or Initiating States different actions are performed 
 * dependent on the link.
 * 
 * In the scanning state, the Filter Policy, White lists and Backoff info is checked to determine if we proceed
 * with the handling of this PDU. In which case the Scan_type is checked to determine if a Scan_Req should be sent
 * to the peer. Irrespective of the Scan_Type and Advertising Event is generated depending on the Duplicate handling
 * and white lists.
 *
 * If we are in the Initiating State, actions performed depend on the setting of the filter policies. If the 
 * filter policies indicate white list should be used an CONNECT_REQ PDU is sent if the AdvA (Peer_Address) in the Adv_Ind
 * matches the Peer Address used in the HCI_Create_Connection. If the white list is used then a CONNECT_REQ PDU is
 * sent if the AdvA address (peer address) and type is contained in the White List.
 *
 * ADV_SCAN_IND
 * Receipt of this PDU is only valid in the Scanning State.
 *
 * In the scanning state, the Filter Policy, White lists and Backoff info is checked to determine if we proceed
 * with the handling of this PDU. In which case the Scan_type is checked to determine if a Scan_Req should be sent
 * to the peer. Irrespective of the Scan_Type and Advertising Event is generated depending on the Duplicate handling
 * and white lists.
 *
 * ADV_DIRECT_IND
 * If an ADV_DIRECT_IND is recieved in the scanning state, the Advertisiing Event is generated depending
 * on the duplicate handling and white lists, however, no Response is generated.
 * 
 * If an ADV_DIRECT_IND is recieved in the initiating state, the device determines if it is addressed by checking the 
 * InitA in the ADV_DIRECT_IND against its own address. If the white list is not used, the Direct Address contained in the
 * HCI_Create_Connection is checked against the InitA address in the ADV_DIRECT_IND, and if a match is found a CONNECT_REQ
 * is sent to the peer. If the White list is used then the White list is searched for the Direct address of the Advertiser, 
 * and a CONNECT_REQ sent to peer if a match is found.
 *
 * SCAN_REQ
 * The SCAN_REQ PDU is only validly recieved in the Advertising_State, 
 * If the Advertiser recieves SCAN_REQ PDU that contains its device address from a scanner allowed by the advertising 
 * policy it shall reply with a SCAN_RSP PDU on the same advertising channel index.
 * 
 * SCAN_RESP
 * If a SCAN_RESP is recieved and has been address specificly to the local device - in the Scanning State.
 * An Advertising Event is generated.
 *
 * CONNECT_REQ
 * This can only be validly recieved when a device is in the Advertising state. 
 * If the Advertiser recieves CONNECT_REQ PDU that contains its device address from an initiator allowed by the
 * advertising filter policy and/or white-list the Link Layer shall exit the Advertising State and disables connectable
 * advertising. An identifier is allocated for the link and the connection parameters are extracted.
 *
 **************************************************************************/

u_int8 LEll_Decode_Advertising_ChannelPDU(t_p_pdu p_pdu,u_int8 rssi,u_int8 length)
{
	if(PATCH_FUN[LELL_DECODE_ADVERTISING_CHANNELPDU_ID]){
         
         return ((u_int8 (*)(t_p_pdu p_pdu,u_int8 rssi,u_int8 length))PATCH_FUN[LELL_DECODE_ADVERTISING_CHANNELPDU_ID])(p_pdu,  rssi,  length);
    }

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
	t_LE_Connection* p_connection;
#endif

	u_int8 pdu_type = HWle_get_rx_pdu_type();
	u_int8 TxAdd    = HWle_get_rx_tx_add();
	u_int8 RxAdd    = HWle_get_rx_rx_add();

	t_bd_addr peer_bdaddr;

	//change here: add if()!
	if(pdu_type != SCAN_REQ)
	{
		BDADDR_Assign_from_Byte_Array(&peer_bdaddr,p_pdu);
	}
    u_int8* p_peer_address = peer_bdaddr.bytes;

	// Validate pdu_type and length;
    LE_config.last_adv_pdu_rxed = pdu_type;

	if (pdu_type > ADV_SCAN_IND)
		return 0;

	switch(pdu_type)
	{
#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
	case ADV_NONCONN_IND :
#if (PRH_SYS_CFG_HARDWARE_ADDRESS_FILTERING_SUPPORTED == 0)
		// No response is ever sent back to the ADV_NONCONN_IND
		// The ADV_NONCONN_IND contains only the address of the Advertiser.
		if (LE_config.state == SCANNING_STATE)
		{
		    // GF & NC UPF 42 --- Orig there was no white list check on ADV_NONCONN_IND
			if (((LEscan_Get_Scan_Filter_Policy() & 0x01) == 0x00) || (LEwl_Search_White_List(TxAdd,p_peer_address)))
			{
				// Generate Advertising Report and then return scanning;
				LEscan_Genertate_Advertising_Report_Event(1,ADV_NONCONN_IND_EVENT_TYPE /* ADV_NONCONN_IND */,TxAdd,p_peer_address,length-6,p_pdu+6,last_adv_rx_rssi);

			}
		}
#else
		LEscan_Genertate_Advertising_Report_Event(1,ADV_NONCONN_IND_EVENT_TYPE /* ADV_NONCONN_IND */,TxAdd,p_peer_address,length-6,p_pdu+6,last_adv_rx_rssi);
#endif
#if (PRH_BS_CFG_SYS_TRACE_ADV_VIA_HCI_SUPPORTED==1)
		if (LE_config.state != INITIATING_STATE)
			PRH_rx_adv_dbg_channel = LEscan_Get_Scanning_Channel_Index();
		else
			PRH_rx_adv_dbg_channel = LEscan_Get_Initiating_Channel_Index();
		PRH_rx_adv_dbg_length = length;
		PRH_rx_adv_dbg_header = HWle_get_rx_adv_header();
	  //  TCeg_Send_LE_Adv_Event(PRH_adv_dbg_header,(u_int8*)p_pdu, PRH_adv_dbg_length,PRH_adv_dbg_channel, 0 /* Rx */);
#endif
		break;

	case ADV_IND :
		//  The response to an ADV_IND can be either SCAN_REQ or CONNECT_REQ
		//  depending if the device is scanning or initiating.
#if (PRH_BS_CFG_SYS_TRACE_ADV_VIA_HCI_SUPPORTED==1)
		if (LE_config.state != INITIATING_STATE)
			PRH_rx_adv_dbg_channel = LEscan_Get_Scanning_Channel_Index();
		else
			PRH_rx_adv_dbg_channel = LEscan_Get_Initiating_Channel_Index();
		PRH_rx_adv_dbg_length = length;
		PRH_rx_adv_dbg_header = HWle_get_rx_adv_header();
	 //   TCeg_Send_LE_Adv_Event(PRH_adv_dbg_header,(u_int8*)p_pdu, PRH_adv_dbg_length,PRH_adv_dbg_channel, 0 /* Rx */);
#endif
		if(LE_config.state ==  SCANNING_STATE)
		{
#if (PRH_SYS_CFG_HARDWARE_ADDRESS_FILTERING_SUPPORTED == 0)
			/*
			 * Directed advertising packets which are not addressed for this device
			 * shall be ignored.
			 */
			if (((LEscan_Get_Scan_Filter_Policy() & 0x01) == 0x00) || (LEwl_Search_White_List(TxAdd,p_peer_address)))
			{
				if (LEscan_Get_Scan_Type() != LE_PASSIVE_SCAN)
				{
					if (LEscan_Ok_To_Send_Scan_Req())
					{
						/*
						 * Prior to sending a Scan_Req we store the Advertisers Address to ensure that we
						 * only report Scan_Rsp PDUs from Advertisers to which we sent a Scan_Req
						 */

						HWle_set_tx_enable();
						BDADDR_Copy(&LE_config.advertiser_address,&peer_bdaddr);
						LEll_Encode_Advertising_ChannelPDU(SCAN_REQ,p_peer_address,TxAdd,0);
						LEscan_Genertate_Advertising_Report_Event(1,ADV_IND_EVENT_TYPE /* ADV_IND */,TxAdd,p_peer_address,length-6,p_pdu+6,last_adv_rx_rssi);
						return 1;
					}
				}
				else
				{

					HWle_clear_tx_enable();
					LEscan_Genertate_Advertising_Report_Event(1,ADV_IND_EVENT_TYPE /* ADV_IND */,TxAdd,p_peer_address,length-6,p_pdu+6,last_adv_rx_rssi);
				}
			}
#else
			if ((LEscan_Get_Scan_Type() != LE_PASSIVE_SCAN) && (LEscan_Ok_To_Send_Scan_Req()))
			{
				/*
				 * Prior to sending a Scan_Req we store the Advertisers Address to ensure that we
				 * only report Scan_Rsp PDUs from Advertisers to which we sent a Scan_Req
				 */
				HWle_set_tx_enable();
				BDADDR_Copy(&LE_config.advertiser_address,&peer_bdaddr);
				LEll_Encode_Advertising_ChannelPDU(SCAN_REQ,p_peer_address,TxAdd,0);
				LEscan_Genertate_Advertising_Report_Event(1,ADV_IND_EVENT_TYPE /* ADV_IND */,TxAdd,p_peer_address,length-6,p_pdu+6,last_adv_rx_rssi);
				return 1;
			}
			else
				LEscan_Genertate_Advertising_Report_Event(1,ADV_IND_EVENT_TYPE /* ADV_IND */,TxAdd,p_peer_address,length-6,p_pdu+6,last_adv_rx_rssi);
#endif

		}
#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
		else if (LE_config.state == INITIATING_STATE)
		{
			p_connection = LEconnection_Find_Link_Entry(LE_config.initiator_link_id);
#if (PRH_SYS_CFG_HARDWARE_ADDRESS_FILTERING_SUPPORTED == 0)
			if ((LE_config.initiator_filter_policy & 0x01) == 0x00)
			{
				/* White list is not used to determine which advertiser to connect to.
				 * Peer_Address_Type and Peer_Address shall be used.
				 */
				if((TxAdd == LEconnection_Get_Peer_Addr_Type(p_connection)) &&
				   (BDADDR_Is_Equal(&peer_bdaddr,&p_connection->peer_address)))
				{

					HWle_set_tx_enable();
					LEll_Encode_Advertising_ChannelPDU(CONNECT_REQ,p_peer_address,TxAdd,0);
					LE_config.initiating_enable = 0;
					return 1;
				}
			}
			else
			{
				/* White list is used to determine which advertiser to connect to.
				 * Peer_Address_Type and Peer_Address shall be ignored.
				 */
				if (LEwl_Search_White_List(TxAdd,p_peer_address))
				{

					HWle_set_tx_enable();
					BDADDR_Copy(&p_connection->peer_address,&peer_bdaddr);
					LEll_Encode_Advertising_ChannelPDU(CONNECT_REQ,p_peer_address,TxAdd,0);
					LE_config.initiating_enable = 0;
					return 1;
				}
			}
#else
			HWle_set_tx_enable();
			if ((LE_config.initiator_filter_policy & 0x01) != 0x00)
				BDADDR_Copy(&p_connection->peer_address,&peer_bdaddr);

			LEll_Encode_Advertising_ChannelPDU(CONNECT_REQ,p_peer_address,TxAdd,0);
			LE_config.initiating_enable = 0;
			return 1;

#endif

		}
#endif

		HWle_clear_tx_enable();
		return 0;
		break;

	case ADV_SCAN_IND :

		// Payload = AdvA 6 octets, AdvData 0-31 octets
#if (PRH_BS_CFG_SYS_TRACE_ADV_VIA_HCI_SUPPORTED==1)
		if (LE_config.state != INITIATING_STATE)
			PRH_rx_adv_dbg_channel = LEscan_Get_Scanning_Channel_Index();
		else
			PRH_rx_adv_dbg_channel = LEscan_Get_Initiating_Channel_Index();

		PRH_rx_adv_dbg_length = length;
		PRH_rx_adv_dbg_header = HWle_get_rx_adv_header();
	//    TCeg_Send_LE_Adv_Event(PRH_adv_dbg_header,(u_int8*)p_pdu, PRH_adv_dbg_length,PRH_adv_dbg_channel, 0 /* Rx */);
#endif
#if (PRH_SYS_CFG_HARDWARE_ADDRESS_FILTERING_SUPPORTED == 0)
		if(LE_config.state == SCANNING_STATE)
		{
			// Run this through our white lists -
			// Note p_pdu should point to the Advertisers Address
			if (((LEscan_Get_Scan_Filter_Policy() & 0x01) == 0x00) || (LEwl_Search_White_List(TxAdd,p_pdu)))
			{
				if (LEscan_Get_Scan_Type()!= LE_PASSIVE_SCAN)
				{
					if (LEscan_Ok_To_Send_Scan_Req())
					{
					    // GF & NC UPF 42 - These Addr of the advertiser needs to be stored prior to 
						// sending the SCAN_RESP
						BDADDR_Copy(&LE_config.advertiser_address,&peer_bdaddr);
						LEll_Encode_Advertising_ChannelPDU(SCAN_REQ,p_peer_address,TxAdd,0);
						LEscan_Genertate_Advertising_Report_Event(1,ADV_DISCOVER_IND_EVENT_TYPE /* ADV_SCAN_IND */,TxAdd,p_peer_address,length-6,p_pdu+6,rssi);
						return 1;
					}
				}
				LEscan_Genertate_Advertising_Report_Event(1,ADV_DISCOVER_IND_EVENT_TYPE /* ADV_SCAN_IND */,TxAdd,p_peer_address,length-6,p_pdu+6,rssi);
			}
		}
#else
		if (LEscan_Get_Scan_Type()!= LE_PASSIVE_SCAN)
		{
			if (LEscan_Ok_To_Send_Scan_Req())
			{
				// GF & NC UPF 42 - These Addr of the advertiser needs to be stored prior to 
				// sending the SCAN_RESP
				BDADDR_Copy(&LE_config.advertiser_address,&peer_bdaddr);
				LEll_Encode_Advertising_ChannelPDU(SCAN_REQ,p_peer_address,TxAdd,0);
				LEscan_Genertate_Advertising_Report_Event(1,ADV_DISCOVER_IND_EVENT_TYPE /* ADV_SCAN_IND */,TxAdd,p_peer_address,length-6,p_pdu+6,rssi);
				return 1;
			}
		}
		LEscan_Genertate_Advertising_Report_Event(1,ADV_DISCOVER_IND_EVENT_TYPE /* ADV_SCAN_IND */,TxAdd,p_peer_address,length-6,p_pdu+6,rssi);

#endif
		return 0;
		break;

	case ADV_DIRECT_IND :
		// Only a device in the INITIATING_STATE can respond to the ADV_DIRECT_IND with a connect req
		// Payload = AdvA 6 octets, Init A

#if (PRH_BS_CFG_SYS_TRACE_ADV_VIA_HCI_SUPPORTED==1)
		if (LE_config.state != INITIATING_STATE)
			PRH_rx_adv_dbg_channel = LEscan_Get_Scanning_Channel_Index();
		else
			PRH_rx_adv_dbg_channel = LEscan_Get_Initiating_Channel_Index();
		PRH_rx_adv_dbg_length = length;
		PRH_rx_adv_dbg_header = HWle_get_rx_adv_header();
	  //  TCeg_Send_LE_Adv_Event(PRH_rx_adv_dbg_header,(u_int8*)p_pdu, PRH_rx_adv_dbg_length,PRH_rx_adv_dbg_channel, 0 /* Rx */);
#endif

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
		p_connection = LEconnection_Find_Link_Entry(LE_config.initiator_link_id);
#if (PRH_SYS_CFG_HARDWARE_ADDRESS_FILTERING_SUPPORTED == 0)
		/* If an ADV_DIRECT_IND PDU with this Link Layer’s device address is received that
		 * is allowed by the initiator filter policy, the initiator shall send a CONNECT_REQ
		 * PDU to the advertiser; otherwise it shall be ignored.
		 */
		if((LE_config.state == INITIATING_STATE) && (LEconnection_Check_Current_Address(p_connection,RxAdd,(p_pdu+6))))
		{
			if(LE_config.initiator_filter_policy  == 0x00)
			{
				/* White list is not used to determine which advertiser to connect to.
				 * Peer_Address_Type and Peer_Address shall be used.
				 */

				/* Check that the Address of the Advertiser matchs that which a connection */
				if ((LEconnection_Get_Peer_Addr_Type(p_connection) == TxAdd)
					&& (BDADDR_Is_Equal(&peer_bdaddr,&p_connection->peer_address)))
				{
					HWle_set_tx_enable();
					LEll_Encode_Advertising_ChannelPDU(CONNECT_REQ,peer_bdaddr.bytes,TxAdd,0);
					return 1;
				}
			}
			else if (LE_config.initiator_filter_policy == 0x01)
			{
				/*
				 * White list is used to determine which advertiser to connect to.
				 * Peer_Address_Type and Peer_Address shall be ignored.
				 */

				if(LEwl_Search_White_List(TxAdd,peer_bdaddr.bytes))
				{
                    // GF 8 Sept -- Ensure the Peer_Address is written to the p_connection is if connecting
	                // from the white list - it may not have been written by the host.

					HWle_set_tx_enable();
					BDADDR_Copy(&p_connection->peer_address,&peer_bdaddr);

					LEll_Encode_Advertising_ChannelPDU(CONNECT_REQ,peer_bdaddr.bytes,TxAdd,0);
					return 1;
				}
			}
		}
		else
#endif
		if ((LE_config.state == SCANNING_STATE) && (LEconfig_Check_Current_Address(RxAdd,(p_pdu+6))))
		{
		        // GF & NC UPF 42 - These Addr of the advertiser needs to be stored prior to 
				// sending the SCAN_RESP
				HWle_clear_tx_enable();
				BDADDR_Copy(&LE_config.advertiser_address,&peer_bdaddr);
				LEscan_Genertate_Advertising_Report_Event(1,ADV_DIRECT_EVENT_TYPE ,TxAdd,p_pdu,0,0,last_adv_rx_rssi);
				//
				// When in the scanning state,  There is no PDU sent to the Peer in respone to
				// the ADV_DIRECT_IND..
		}
#else
		if(LE_config.state == INITIATING_STATE)
		{
			if (LE_config.initiator_filter_policy == 0x01)
				BDADDR_Copy(&p_connection->peer_address,&peer_bdaddr);

			LEll_Encode_Advertising_ChannelPDU(CONNECT_REQ,peer_bdaddr.bytes,TxAdd,0);
			return 1;
		}
		else
		{
			BDADDR_Copy(&LE_config.advertiser_address,&peer_bdaddr);
			LEscan_Genertate_Advertising_Report_Event(1,ADV_DIRECT_EVENT_TYPE ,TxAdd,p_pdu,0,0,last_adv_rx_rssi);
		}
#endif
		break;
#endif

#if (PRH_BS_CFG_SYS_LE_BROADCAST_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
	case SCAN_REQ :
		// Payload = ScanA 6 octets, AdvA 6 octets
			
		// If the Advertiser recieves SCAN_REQ PDU that contains its device
		// address from a scanner allowed by the advertising filter policy it
		// shall reply with a SCAN_RSP PDU on the same advertising channel index.
#if (PRH_BS_CFG_SYS_TRACE_ADV_VIA_HCI_SUPPORTED==1)
		if (LE_config.state != INITIATING_STATE)
			PRH_rx_adv_dbg_channel = LEscan_Get_Scanning_Channel_Index();
		else
			PRH_rx_adv_dbg_channel = LEscan_Get_Initiating_Channel_Index();
		PRH_rx_adv_dbg_length = length;
		PRH_rx_adv_dbg_header = HWle_get_rx_adv_header();

	    TCeg_Send_LE_Adv_Event(PRH_rx_adv_dbg_header,(u_int8*)p_pdu, PRH_rx_adv_dbg_length,PRH_rx_adv_dbg_channel, 0 /* Rx */);
#endif

#if (PRH_SYS_CFG_HARDWARE_ADDRESS_FILTERING_SUPPORTED == 0)
		if((LE_config.state == ADVERTISING_STATE) && (LEconfig_Check_Current_Address(RxAdd,(p_pdu+6))))
		{
			// The action is dependent on the value in the advertising filter policy.
			// 0x00 or 0x02 -- Allow Scan Request from Any,
			// 0x01 0r 0x03 -- Allow Scan Request from White List Only

			/* Note :- We should not respond to a Scan_Req if we are performing Direct Advertising */
			if ((LEadv_Get_Adv_Type() != 0x01) && (((LEadv_Get_Advertising_Filter_Policy() & 0x01) == 0x00) ||
					(LEwl_Search_White_List(TxAdd,p_pdu))))
			{
				LEll_Encode_Advertising_ChannelPDU(SCAN_RSP,0,0,0);
				return 1;
			}
		}
#else

		//LEll_Encode_Advertising_ChannelPDU(SCAN_RSP,0,0,0);
		return 1;
#endif
		break;
#endif

#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
	case SCAN_RSP :
#if (PRH_BS_CFG_SYS_TRACE_ADV_VIA_HCI_SUPPORTED==1)
		if (LE_config.state != INITIATING_STATE)
			PRH_rx_adv_dbg_channel = LEscan_Get_Scanning_Channel_Index();
		else
			PRH_rx_adv_dbg_channel = LEscan_Get_Initiating_Channel_Index();
		PRH_rx_adv_dbg_length = length;
		PRH_rx_adv_dbg_header = HWle_get_rx_adv_header();
	  //  TCeg_Send_LE_Adv_Event(PRH_rx_adv_dbg_header,(u_int8*)p_pdu, PRH_rx_adv_dbg_length,PRH_rx_adv_dbg_channel, 0 /* Rx */);
#endif
#if (PRH_SYS_CFG_HARDWARE_ADDRESS_FILTERING_SUPPORTED == 0)
		if ((LE_config.state == SCANNING_STATE) && (BDADDR_Is_Equal(&LE_config.advertiser_address,&peer_bdaddr)))
		{
			// Payload = AdvA 6 octets, ScanRspData 0-31 octets
			LEscan_Genertate_Advertising_Report_Event(1,SCAN_RSP_EVENT_TYPE,TxAdd,p_pdu,(length-6),p_pdu+6,last_adv_rx_rssi);
			LEscan_Backoff_Successfull_Scan_Rsp_Rxed();
		}
#else
		LEscan_Genertate_Advertising_Report_Event(1,SCAN_RSP_EVENT_TYPE,TxAdd,p_pdu,(length-6),p_pdu+6,last_adv_rx_rssi);
		LEscan_Backoff_Successfull_Scan_Rsp_Rxed();
#endif
		break;
#endif

#if (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
	case CONNECT_REQ :
		{
		u_int8 link_id;
		u_int8* p_payload = p_pdu + 12;
#if (PRH_BS_CFG_SYS_TRACE_ADV_VIA_HCI_SUPPORTED==1)
		if (LE_config.state != INITIATING_STATE)
			PRH_rx_adv_dbg_channel = LEscan_Get_Scanning_Channel_Index();
		else
			PRH_rx_adv_dbg_channel = LEscan_Get_Initiating_Channel_Index();
		PRH_rx_adv_dbg_length = length;
		PRH_rx_adv_dbg_header = HWle_get_rx_adv_header();
	  //  TCeg_Send_LE_Adv_Event(PRH_rx_adv_dbg_header,(u_int8*)p_pdu, PRH_rx_adv_dbg_length,PRH_rx_adv_dbg_channel, 0 /* Rx */);
#endif
		// Payload = InitA 6 octets, AdvA 6 octets, LLData 22 octets

		// If the Advertiser recieves CONNECT_REQ PDU that contains its device
		// address from an initiator allowed by the advertising filter policy,
		// the Link Layer shall exit the Advertising State and transition to the
		// Connection State in the Slave Role as defined in Section 4.5.5.
#if (PRH_SYS_CFG_HARDWARE_ADDRESS_FILTERING_SUPPORTED == 0)
	    //
	    // NOTE :- Also need to check that we are in Connectable_Advertising
		if((LE_config.state == ADVERTISING_STATE) && (LEconfig_Check_Current_Address(RxAdd,(p_pdu+6))))
		{
	  // The Advertising_Filter_Policy parameter shall be ignored when directed advertising
	  // is enabled.
			if (((LEadv_Get_Adv_Type() == 0x01)||
				 (LEadv_Get_Advertising_Filter_Policy() & 0x02) == 0x00) ||
				 (LEwl_Search_White_List(TxAdd,p_pdu)))
			{
				u_int8 link_id;
				if (NO_ERROR == LEconfig_Allocate_Link_Id(&link_id))
				{
				
					HWle_set_tx_enable();
					// Step 1 - Extract the connection parameters

					p_connection = LEconnection_Find_Link_Entry(link_id);
#if (PRH_BS_CFG_SYS_LE_DUAL_MODE==1)
					//p_connection->device_index = DL_Alloc_Link();

#else
					p_connection->device_index = link_id;
#endif

					// Disable advertising..
					/* The Controller shall continue advertising until the Host issues an LE_Set_Advertise_Enable
					 * command with Advertising_Enable set to 0x00 (Advertising is disabled) or until a connection
					 * is created or until the Advertising is timed out due to Directed Advertising. In these cases,
					 * advertising is then disabled.
					 */

					/* In advertising a SCAN_RESP has been Queued up for transmission -
					 * If a connect_req has been recieved - we must ensure that we dont Tx it - and cleanly
					 * cancle the TX.
					 */
					LEadv_Disable_Connectable_Advertising();

					LE_Connection_Extact_Connection_Req_Parameters(p_connection,p_payload);

					// Step 2 - Setup the connection handle - Link Id + 0x80, and role

					LEconnection_Set_Role(p_connection,SLAVE);

					BDADDR_Copy(&p_connection->peer_address,&peer_bdaddr);

					LE_config.initiator_link_id = link_id;
					LE_config.num_le_links++;
					LE_config.slave_link_active=1;
#if (PRH_BS_CFG_SYS_LE_DUAL_MODE!=1)
					p_connection->device_index = link_id; //LE_config.incoming_connection_device_index;
#endif
					LEconnection_Set_Peer_Addr_Type(p_connection, TxAdd);

					return 1;
				}
			}
		}
#else

//		if (NO_ERROR == LEconfig_Allocate_Link_Id(&link_id))  // if receive connect requset message at scan state, will cause state error
		if((LE_config.state == ADVERTISING_STATE)&&(LEconfig_Check_Current_Address(RxAdd,(p_pdu+6)))&&(NO_ERROR == LEconfig_Allocate_Link_Id(&link_id)))
		{
			//if(0 == g_debug_le_sleep)
			{
				// HWle_set_tx_enable();   //TBD
			}
	
			
			// Step 1 - Extract the connection parameters

			p_connection = LEconnection_Find_Link_Entry(link_id);
#if (PRH_BS_CFG_SYS_LE_DUAL_MODE==1)
			//p_connection->device_index = DL_Alloc_Link();
#else
			p_connection->device_index = link_id;
#endif

			// Disable advertising..
			/* The Controller shall continue advertising until the Host issues an LE_Set_Advertise_Enable
			 * command with Advertising_Enable set to 0x00 (Advertising is disabled) or until a connection
			 * is created or until the Advertising is timed out due to Directed Advertising. In these cases,
			 * advertising is then disabled.
			 */

			 /* In advertising a SCAN_RESP has been Queued up for transmission -
			  * If a connect_req has been recieved - we must ensure that we dont Tx it - and cleanly
			  * cancle the TX.
			  */
			LEadv_Disable_Connectable_Advertising();

			LE_Connection_Extact_Connection_Req_Parameters(p_connection,p_payload);

			// Step 2 - Setup the connection handle - Link Id + 0x80, and role

			LEconnection_Set_Role(p_connection,SLAVE);
			BDADDR_Copy(&p_connection->peer_address,&peer_bdaddr);

			LE_config.initiator_link_id = link_id;
			LE_config.num_le_links++;
			LE_config.slave_link_active=1;

#if (PRH_BS_CFG_SYS_LE_DUAL_MODE!=1)
			p_connection->device_index = link_id; //LE_config.incoming_connection_device_index;
#endif
			LEconnection_Set_Peer_Addr_Type(p_connection, TxAdd);

			return 1;
		}

#endif
		}
		break;
#endif
	}

	HWle_clear_tx_enable();
	return 0;
}

/***************************************************************************
 * Function :- LEll_Encode_Advertising_ChannelPDU
 *
 * Description :- 
 * This function encodes an advertising channel PDU, for most PDUs it simply encodes 
 * the header and Address fields, for the Adv_Ind, Adv_Nonconn_Ind, Adv_Scan_Ind PDUs 
 * an optional data field is added. The contents of the PDU are then copied to HW.
 * 
 * The CONNECT_REQ PDU is encoded quite differently to the other PDUs. To reduce the MIPs
 * requirements most of the CONNECT_REQ PDU is encoded into HW at the beginning of the initiating
 * event. However on receipt of an Advertising PDU which which is allowed by the filter policy and/or
 * the White Lists - the remaining address information of the CONNECT_REQ can be encoded into HW.
 * In addition, as only at the instant of sending the CONNECT_REQ can the transmit window offset and 
 * size be determined. These are determined here and written directly to the appropiate bytes in the 
 * HW buffer. 
 * The Supervision To is setup and the complete_pending flag is set - to allow the mini_sched generate
 * a connection complete event.
 *
 ****************************************************************************/
t_error LEll_Encode_Advertising_ChannelPDU(u_int8 pdu_type, const u_int8* pAddress,
										 u_int8 RxAdd, u_int8* pLLdata)
{
	
	if(PATCH_FUN[LELL_ENCODE_ADVERTISING_CHANNELPDU_ID]){
         
         return ((t_error (*)(u_int8 pdu_type, const u_int8* pAddress,u_int8 RxAdd, u_int8* pLLdata))PATCH_FUN[LELL_ENCODE_ADVERTISING_CHANNELPDU_ID])( pdu_type, pAddress,RxAdd,pLLdata);
    }

	u_int8 i;
	t_p_pdu p_payload = (u_int8*)u_int32_TX_buffer;
#if (PRH_BS_CFG_SYS_LE_BROADCAST_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
	u_int8 data_len;
#endif
#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
	u_int32 current_clk = HW_Get_Native_Clk_Avoid_Race();
#endif

	switch(pdu_type)
	{
#if (PRH_BS_CFG_SYS_LE_BROADCAST_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
		case ADV_IND :	
		case ADV_NONCONN_IND :
		case ADV_SCAN_IND :
			// Payload = AdvA 6 octets, AdvData 0-31 octets

			LEadv_Set_Adv_Header(pdu_type,0x00);
			LEadv_Encode_Own_Address_In_Payload(p_payload);
			p_payload+=6;
			data_len = LEadv_Write_Adv_Data(p_payload);

			hw_memcpy32((void*)HWle_get_tx_acl_buf_addr(), (void*)u_int32_TX_buffer, data_len+6);
#if (PRH_BS_CFG_SYS_TRACE_ADV_VIA_HCI_SUPPORTED==1)
			PRH_tx_adv_dbg_channel = LEadv_Get_Advertising_Channel_Index();;
			PRH_tx_adv_dbg_length = data_len+6;
#endif
			break;

		case ADV_DIRECT_IND :
			// Payload = AdvA 6 octets, Init A 6 octets
			LEadv_Set_Adv_Header(pdu_type,0x00);
			LEadv_Encode_Own_Address_In_Payload(p_payload);
#endif

#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
		case SCAN_REQ :
			// Payload = AdvA 6 octets, Init A 6 octets
			if (pdu_type == SCAN_REQ )
			{	
			    // Payload = ScanA 6 octets, AdvA 6 octets
				LEscan_Set_Adv_Header(pdu_type,RxAdd);
			    LEscan_Encode_Own_Address_In_Payload(p_payload);
			}
#endif
			p_payload+=6;

			for (i=0;i<6;i++)
			{
				p_payload[i] = pAddress[i];
			}		
			hw_memcpy32((void*)HWle_get_tx_acl_buf_addr(), (void*)u_int32_TX_buffer, 12);
#if (PRH_BS_CFG_SYS_TRACE_ADV_VIA_HCI_SUPPORTED==1)
#if (PRH_BS_CFG_SYS_LE_BROADCAST_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
			if (pdu_type == ADV_DIRECT_IND )
				PRH_tx_adv_dbg_channel = LEadv_Get_Advertising_Channel_Index();
			else
#endif
				PRH_tx_adv_dbg_channel = LEscan_Get_Scanning_Channel_Index();
			PRH_tx_adv_dbg_length = 12;
#endif
			break;


#if (PRH_BS_CFG_SYS_LE_BROADCAST_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
		case SCAN_RSP :
			// Payload = AdvA 6 octets, ScanRspData 0-31 octets


			LEadv_Set_Adv_Header(pdu_type,RxAdd);
			LEadv_Encode_Own_Address_In_Payload(p_payload);
			p_payload+=6;

			data_len = LEadv_Write_Scan_Resp_Data(p_payload);

			hw_memcpy32((void*)HWle_get_tx_acl_buf_addr(), (void*)u_int32_TX_buffer, data_len + 6);


#if (PRH_BS_CFG_SYS_TRACE_ADV_VIA_HCI_SUPPORTED==1)
			PRH_tx_adv_dbg_channel = LEadv_Get_Advertising_Channel_Index();
			PRH_tx_adv_dbg_length = LEadv_Get_Scan_Response_Data_Len() + 6;
#endif
			break;
#endif

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
		case CONNECT_REQ :
		{
			t_LE_Connection* p_connection = LEconnection_Find_Link_Entry(LE_config.initiator_link_id);
			const t_bd_addr* p_bd_addr;
			HWle_set_tx_rx_add(RxAdd);

			if(LEconnection_Get_Own_Addr_Type(p_connection) == PUBLIC_ADDRESS)
			{
				p_bd_addr = SYSconfig_Get_Local_BD_Addr_Ref();
			}
			else
			{
				p_bd_addr = &LE_config.random_address;
			}

			HWle_set_advertiser_addr1_in_tx_buffer((u_int32)(p_bd_addr->bytes[4])+ (u_int32)(p_bd_addr->bytes[5] << 8) +
					((u_int32)((*(pAddress))<<16) +  (u_int32)((*(pAddress+1)) << 24)));

			HWle_set_advertiser_addr2_in_tx_buffer(
					((u_int32)(*(pAddress+2)) +
					((u_int32)(*(pAddress+3)) << 8) +
					((u_int32)(*(pAddress+4)) <<16 ) +
					((u_int32)(*(pAddress+5)) <<24 )));

			if (LE_config.TCI_transmit_win_config)
			{
				p_connection->transmit_window_size = LE_config.transmit_win_size;
				p_connection->transmit_window_offset = LE_config.transmit_win_offset;
			}
			else if (LE_config.num_le_links)
			{
				/*
				 * Ensure that the Transmit Window does not overlap with any other connection
				 * Event --
				 * TODO more advanced logic needed here....
				 */

				if ((BTtimer_Clock_Difference(LE_config.next_connection_event_time, current_clk)/4) < (PRH_BS_CFG_DEFAULT_TRANSMIT_WINDOW_OFFSET + PRH_BS_CFG_DEFAULT_TRANSMIT_WINDOW_SIZE))
				{
					p_connection->transmit_window_offset = BTtimer_Clock_Difference(LE_config.next_connection_event_time, current_clk)/4 + 16;
				}
				else
				{
					p_connection->transmit_window_offset = PRH_BS_CFG_DEFAULT_TRANSMIT_WINDOW_OFFSET;
				}
			}
			else
			{
#ifndef BLUETOOTH_MODE_LE_ONLY //no syc connection anymore
				if (LMscoctr_Get_Number_SYN_Connections())
				{
					// The maximum transmit offset is 10ms. --> 0x08 = 16 slots

					u_int16 num_slots_to_classic =  LMscoctr_Check_Slots_to_next_eSCO_On_Device(current_clk);
					if (num_slots_to_classic > 6)
					{
						p_connection->transmit_window_offset = 0;
					}
					else
					{
						p_connection->transmit_window_offset = (num_slots_to_classic + 4)/2;
					}
				}
				else
#endif
				{
					p_connection->transmit_window_offset =  5;// PRH_BS_CFG_DEFAULT_TRANSMIT_WINDOW_OFFSET;
				}
			}

			HWle_set_win_offset_interval_in_tx_buffer(p_connection->transmit_window_offset,p_connection->connInterval);

	 	// Dont set the State to CONNECTION until the PKA is recieved for the CONNECT_REQ

			LEconnection_Set_Role(p_connection, MASTER);

			p_connection->supervision_TO_timer = BTtimer_Safe_Clockwrap(current_clk + ((p_connection->connInterval * 2)* 6 *2));

			//p_connection->active = 0x01;
			LEconnection_Set_Active(p_connection,0x01);
			LE_config.num_le_links++;
			LE_config.master_link_active++;

#if (PRH_BS_CFG_SYS_TRACE_ADV_VIA_HCI_SUPPORTED==1)
			PRH_tx_adv_dbg_channel = LEscan_Get_Scanning_Channel_Index();
			PRH_tx_adv_dbg_length = 34;
			/***********************************************************************************
			 * Need to explicitly copy the 6 bytes of the Advertisers address into the SW Tx Buffer
			 * so that they are shown in the Advertising Trace - this is a MIPs overhead which is
			 * further reasoning why the TRACE should be disabled for low MIPs solutions
			 ************************************************************************************/
			p_payload+=6;
			for (i=0;i<6;i++)
				p_payload[i] = pAddress[i];

			p_payload+=6;
			p_payload[8] = (u_int8)p_connection->transmit_window_offset;
			p_payload[9] = 0x00;
			p_payload[10] = p_connection->connInterval & 0xFF;
			p_payload[11] = (p_connection->connInterval<<8) & 0xFF;

#endif
			// To be picket up by the MiniSched
			LE_config.p_connection_complete_pending = p_connection;
			LE_config.connection_complete_pending = 0x01;

		}
		break;
#endif
	}

#if (PRH_BS_CFG_SYS_TRACE_ADV_VIA_HCI_SUPPORTED==1)
	PRH_tx_adv_dbg_header = HWle_get_tx_adv_header();
    TCeg_Send_LE_Adv_Event(PRH_tx_adv_dbg_header,(u_int8*)u_int32_TX_buffer, PRH_tx_adv_dbg_length,PRH_tx_adv_dbg_channel, 1 /* Tx */);
#endif

	return NO_ERROR;
}


#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE == 1)


/***************************************************************************
 * Function :- LE_LL_Check_For_Pending_Connection_Completes
 *
 * Description :- 
 * This function is called by the mini-sched to generate Connection Completes for 
 * any recently established LE connection. The approach is used to decouple the generation of 
 * the LE Connection Complete from IRQ context. 
 * 
 * When a new connection is established the "LE_config.connection_complete_pending" is set and a pointer to
 * the new link is set in "LE_config.p_connection_complete_pending". This function is then called from the 
 * mini-sched to generate thw LE_SUBEVENT_CONNECTION_COMPLETE.
 *
 ****************************************************************************/
void LE_LL_Check_For_Pending_Connection_Completes()
{
	
	if(PATCH_FUN[LE_LL_CHECK_FOR_PENDING_CONNECTION_COMPLETES_ID]){
         ((void (*)(void))PATCH_FUN[LE_LL_CHECK_FOR_PENDING_CONNECTION_COMPLETES_ID])();
         return ;
    }

	if (LE_config.connection_complete_pending)
	{
		t_LE_Connection* p_connection = LE_config.p_connection_complete_pending;

		LEeventgen_Generate_LE_Meta_Event(LE_SUBEVENT_CONNECTION_COMPLETE,p_connection,NO_ERROR);

		if (LE_config.hc_data_tester_present)
		{
			t_lm_event_info event_info;

			event_info.handle = LEconnection_Determine_Connection_Handle(p_connection);
            event_info.p_bd_addr = &p_connection->peer_address;
            event_info.link_type = ACL_LINK;
            event_info.mode = 0x00;
            event_info.status = NO_ERROR;

			HCeg_Generate_Event(HCI_CONNECTION_COMPLETE_EVENT, &event_info);
		}

	  LE_config.connection_complete_pending = 0x00;


	  // Add Version exchange and Feature Exchange directly after connection complete
	  if (p_connection->role == MASTER)
	  {
			if (NO_ERROR == LEllc_Encode_PDU(LL_VERSION_IND,p_connection, 0x00))
			{
				p_connection->ll_action_pending |= (LL_VERSION_IND_PENDING | LL_AUTONOMOUSLY_INITIATED);

				/*
			 	* Set timer while waiting for response to ll_version_ind
			 	*/
				p_connection->ll_response_timer = BTtimer_Set_Ticks(LE_LL_RESPONSE_TIMEOUT);
	 		}
	 	}
	}
}

//- update for LL/PAC/SLA/BI01-C(LL.TS.5.0.2.PDF).
//static unsigned char ll_pdu_lenth_table[14] = {12,8,2,23,13,1,1,2,9,9,1,1,6,2};

int check_lmp_pdu_lenth(t_LE_Connection* p_connection, unsigned char *pdu,unsigned char len)
{
	uint8_t opcode = *pdu;
	uint8_t oplen = 0;
	uint8_t ret = 0;
		
	if(opcode >= sizeof(ll_pdu_lenth_table)) return 0;

	oplen = ll_pdu_lenth_table[opcode];
	if(p_connection->current_security_state & LE_ENCRYPTION_ACTIVE) {
		oplen += 4;
	}	
	if (oplen == len) ret = 1;
	return ret;
}

/******************************************************************************************
 * LLC Decode Functions.
 ******************************************************************************************/

/*******************************************************************************************
 * Function :- LE_Decode_link_layer_PDU
 *
 * Description :- 
 * This function decodes an incoming LLC PDU. Based on the value of the opcode in the PDU, the 
 * relevant function to handle a specific PDU is called
 *
 ******************************************************************************************/
t_error LE_Decode_link_layer_PDU(t_LE_Connection* p_connection , t_p_pdu p_pdu, unsigned char lenth)
{
	
	if(PATCH_FUN[LE_DECODE_LINK_LAYER_PDU_ID]){
         
         return ((t_error (*)(t_LE_Connection* p_connection , t_p_pdu p_pdu, unsigned char lenth))PATCH_FUN[LE_DECODE_LINK_LAYER_PDU_ID])(p_connection ,   p_pdu,  lenth);
    }

    //- For LL/PAC/SLA/BI01-C(LL.TS.5.0.2.PDF) START.
	if (!check_lmp_pdu_lenth(p_connection, p_pdu, lenth))
	{
		LEllc_Encode_PDU(LL_UNKNOWN_RSP,p_connection,*(p_pdu));
		return NO_ERROR;
	}
    //- For LL/PAC/SLA/BI01-C(LL.TS.5.0.2.PDF) END.
    
	if (*p_pdu < 14)
		return ((t_error)(*LL_Pdu_Decode_Handlers[*(p_pdu)])(p_connection, (p_pdu+1)));
	else
		LEllc_Encode_PDU(LL_UNKNOWN_RSP,p_connection,*(p_pdu));

	return NO_ERROR;

}

t_error LE_LL_Terminate_Ind(t_LE_Connection* p_connection,t_p_pdu p_pdu)
{
	p_connection->ll_action_pending |=  LL_TERMINATE_PENDING;
	p_connection->terminate_reason = *p_pdu;
	return NO_ERROR;
}


#if (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE == 1)

/*******************************************************************************************
 * Function :- LE_LL_Connection_Update_Req
 *
 * Description :- 
 * This function handles an incoming connection update. The parameters of the LLC PDU are checked
 * and the new connection parameters are stored in a copy of the connection parameters in p_connection.
 * 
 * These values ( i.e p_connection->connInterval_new ) are copied to the connection parameters which are
 * used on a line ( i.e p_connection->connInterval ) when the connection update instant has been
 * reached.
 *
 ******************************************************************************************/
t_error LE_LL_Connection_Update_Req(t_LE_Connection* p_connection, t_p_pdu p_pdu)
{
	
	if(PATCH_FUN[LE_LL_CONNECTION_UPDATE_REQ_ID]){
         
         return ((t_error (*)(t_LE_Connection* p_connection, t_p_pdu p_pdu))PATCH_FUN[LE_LL_CONNECTION_UPDATE_REQ_ID])(p_connection,   p_pdu);
    }

	u_int8 win_size;
	u_int16 win_offset;
	u_int16 interval;
	u_int16 latency;
	u_int32 timeout;
	u_int16 instant;

	win_size = *p_pdu;
	win_offset = LMutils_Get_Uint16(p_pdu+1);
	interval = LMutils_Get_Uint16(p_pdu+3);
	latency = LMutils_Get_Uint16(p_pdu+5);
	timeout = LMutils_Get_Uint16(p_pdu+7);
	instant = LMutils_Get_Uint16(p_pdu+9);

	if (win_offset > interval )
		return UNSPECIFIED_ERROR;
	//- Fix bug for TP/CON/SLA/BV-10-C.
 	if ((win_size > 8) || (win_size > (interval/*-1*/)))
 		return UNSPECIFIED_ERROR;

	if ((interval > 3200) || (interval < 6))
		return UNSPECIFIED_ERROR;

	if (latency > 500)
		return UNSPECIFIED_ERROR;

	if (latency > (((timeout*8)/interval)-1))
		return UNSPECIFIED_ERROR;

	// The connSupervisionTimeout shall be a multiple of 10ms in the range 100ms to 
	// 32sec and shall be larger than ( 1 + connSlaveLatency ) * ConnInterval

	if ((timeout*8) <= (u_int32)((1 + latency) * interval))
		return UNSPECIFIED_ERROR;

	if (((u_int16)(instant - p_connection->connection_event_counter)) >= 32767) // Instant in the past
	{
		// Now I need to return to Standby and inform the host.
#if 0 // Conformance - Disconnect Immediately and dont wait for next connection event
		LEconnection_Local_Disconnect(p_connection,INSTANT_PASSED);
#else
		LEconnection_Local_Disconnect(p_connection,INSTANT_PASSED);
		if (LE_config.state != CONNECTION_STATE)
			_LEconnection_Close_Connection_Event(p_connection,0x00);
#endif
		return NO_ERROR;
	}

	// Need to validate the instant against the connEventCount.

	p_connection->connInterval_new = interval;
	p_connection->latency_new = latency;
	p_connection->timeout_new = timeout;
	p_connection->transmit_window_offset_new = win_offset;
	p_connection->transmit_window_size_new = win_size;

	p_connection->connection_update_instant = instant;
    p_connection->ll_action_pending |= LL_CONNECTION_UPDATE_PENDING;

	return NO_ERROR;
}
#endif


/*******************************************************************************************
 * Function :- LE_LL_Version_Ind
 *
 * Description :- 
 * This function handles an incoming LLC_VERSION_IND. If a LLC_VERSION_IND has not already been
 * sent to the peer, one is encoded and queued for transmission. If a version ind has already been
 * sent then this if a response from the peer and a HCI_READ_REMOTE_VERSION_INFORMATION_COMPLETE_EVENT
 * is generated.
 *
 ******************************************************************************************/

t_error LE_LL_Version_Ind(t_LE_Connection* p_connection,t_p_pdu p_pdu)
{
	if(PATCH_FUN[LE_LL_VERSION_IND_ID]){
         
         return ((t_error (*)(t_LE_Connection* p_connection,t_p_pdu p_pdu))PATCH_FUN[LE_LL_VERSION_IND_ID])(p_connection,  p_pdu);
    }

	if ((p_connection) && !(p_connection->version_ind_status & LL_VERSION_IND_RECIEVED))
	{
		// If a version Ind has not been previously sent to the Peer, one should 
		// be sent

		p_connection->version_ind_status |= LL_VERSION_IND_RECIEVED;

		if ((p_connection->ll_action_pending & LL_VERSION_IND_PENDING) == 0x00)
		{
			LEllc_Encode_PDU( LL_VERSION_IND, p_connection, 0x00);
		}
		else // I have got a version ind in response to my HCI version request - or an autonomously generated version req.
		{
			t_lm_event_info event_info;
    
			/*
			 *	Clear the response timer on receiving a version ind.
			 */
			p_connection->ll_response_timer = BT_TIMER_OFF;

			p_connection->ll_action_pending &= ~LL_VERSION_IND_PENDING;

			/*
		 	 * Store the Version for the remote device
		 	 */
			p_connection->lmp_version = event_info.lmp_version = *p_pdu;
			p_connection->comp_id = event_info.comp_id = LMutils_Get_Uint16(p_pdu+1);
			p_connection->lmp_subversion = event_info.lmp_subversion = LMutils_Get_Uint16(p_pdu+3);

			if (!(p_connection->ll_action_pending & LL_AUTONOMOUSLY_INITIATED))
			{
				event_info.handle = LEconnection_Determine_Connection_Handle(p_connection);
				event_info.status = NO_ERROR;
				HCeg_Generate_Event(HCI_READ_REMOTE_VERSION_INFORMATION_COMPLETE_EVENT,&event_info);
			}
			else if (p_connection->role == MASTER)
			{
				// We have got a LL_VERSIONE_IND and the VERSION_REQUEST was Autonomously Initiated.
				// So we not Autonomously Initiate a feature req
				if (NO_ERROR == LEllc_Encode_PDU(LL_FEATURE_REQ,p_connection,0x00) )
				{
					p_connection->ll_action_pending |= (LL_FEATURE_REQ_PENDING | LL_AUTONOMOUSLY_INITIATED);

					/*
				 	* Set timer while waiting for response to ll_version_ind
				 	*/
					p_connection->ll_response_timer = BTtimer_Set_Ticks(LE_LL_RESPONSE_TIMEOUT);
				}
			}

		}
	}
	return NO_ERROR;
}

#if (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE == 1)

/*******************************************************************************************
 * Function :- LE_LL_Feature_Req
 *
 * Description :- 
 * This function handles an incoming LLC_FEATURES_REQ from the master on a connection. 
 *
 ******************************************************************************************/

t_error LE_LL_Feature_Req(t_LE_Connection* p_connection,t_p_pdu p_pdu)
{

	if(p_connection==0x00)
		return NO_CONNECTION;

	if (LEconnection_Get_Role(p_connection) == SLAVE)
	{
		// Store the master set of features - only the first byte is releavant
		// the other 7 bytes can be hardcoded to Zero.

		p_connection->peer_features = *p_pdu;

		LEllc_Encode_PDU(LL_FEATURE_RSP,p_connection,0);
	}
	else // Slave cannot initiate a features request on a master.
	{
		LEllc_Encode_PDU(LL_REJECT_IND, p_connection ,LMP_PDU_NOT_ALLOWED);
	}
	return NO_ERROR;
}
#endif


#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1)

/*******************************************************************************************
 * Function :- LE_LL_Feature_Rsp
 *
 * Description :- 
 * This function handles an incoming LLC_FEATURES_RSP from the slave on a connection. If current
 * device is a MASTER and a LLC_FEATURES_REQ is pending, then an LE_SUBEVENT_READ_REMOTE_USED_FEATURES_COMPLETE
 * event is generated. Otherwise a LLC_REJECT_IND is sent to the peer.
 *
 ******************************************************************************************/

t_error LE_LL_Feature_Rsp( t_LE_Connection* p_connection,t_p_pdu p_pdu)
{
    if(PATCH_FUN[LE_LL_FEATURE_RSP_ID]){
         
         return ((t_error (*)(t_LE_Connection* p_connection,t_p_pdu p_pdu))PATCH_FUN[LE_LL_FEATURE_RSP_ID])(p_connection,  p_pdu);
    }
	if(p_connection==0x00)
		return NO_CONNECTION;

	if (((LEconnection_Get_Role(p_connection) == MASTER)) &&
	    ((p_connection->ll_action_pending & LL_FEATURE_REQ_PENDING)))
	{
		/*
		 * Clear the timer on master receiving ll_features_rsp
		 */
		p_connection->ll_response_timer = BT_TIMER_OFF;
		// Store the master set of features.
		p_connection->peer_features = *p_pdu;

		if(!(p_connection->ll_action_pending & LL_AUTONOMOUSLY_INITIATED))
		{
			LEeventgen_Generate_LE_Meta_Event(LE_SUBEVENT_READ_REMOTE_USED_FEATURES_COMPLETE, p_connection ,NO_ERROR);
		}
		p_connection->ll_action_pending &= ~LL_FEATURE_REQ_PENDING;
		p_connection->ll_action_pending &= ~LL_AUTONOMOUSLY_INITIATED;
	}
	else
	{
		LEllc_Encode_PDU(LL_REJECT_IND,p_connection,UNSPECIFIED_ERROR);
	}

	return NO_ERROR;
}
#endif

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1)

/***************************************************************************
 *
 * Function:	LE_LL_Start_Enc_Req
 *
 * Description: This decodes an incoming LLC_START_ENC_REQ. This PDU is only allowed
 *              in a device which has a Master role on the connection, and is in the
 *              appropiate security state.
 * 
 *              The encryption packet counters are set to zero and a LLC_START_ENC_RSP pdu
 *              is sent to the peer.
 *
 * Role:		Master
 **************************************************************************/
t_error LE_LL_Start_Enc_Req(t_LE_Connection* p_connection,t_p_pdu p_pdu)
{
    if(PATCH_FUN[LE_LL_START_ENC_REQ_ID]){
         
         return ((t_error (*)(t_LE_Connection* p_connection,t_p_pdu p_pdu))PATCH_FUN[LE_LL_START_ENC_REQ_ID])(p_connection,  p_pdu);
    }
	int i;

	if (LEconnection_Get_Role(p_connection) == SLAVE)
		return COMMAND_DISALLOWED;
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
	/* 
	 * This ll pdu is only accepted when in state LE_W4_START_ENC_REQ.
	 */
	if (!(p_connection->current_security_state & LE_W4_START_ENC_REQ))
	{
		return LMP_PDU_NOT_ALLOWED;
	}
	
	p_connection->enc_tx_pkt_counter = 0;
	p_connection->on_air_rx_pkt_count = 0;
	p_connection->on_air_tx_pkt_count=0;


	/* 
	 * The new initialisation vector only gets set here if this is a
	 * pause encryption procedure. To ensure the interim messages get 
	 * encrypted correctly.
	 */
	if (p_connection->current_security_state & LE_ENCRYPTION_ACTIVE)
	{
		for(i=0;i<4;i++)
			p_connection->IV[i] = p_connection->IVnew[i];
	}

	// Set state equal to encryption active.
	p_connection->current_security_state &= ~LE_W4_START_ENC_REQ;
	p_connection->current_security_state |= (LE_ENCRYPTION_ACTIVE | LE_W4_START_ENC_RSP) ;

	// This packet is sent encrypted
	LEllc_Encode_PDU(LL_START_ENC_RSP,p_connection,0);

	/*
	 * Restart timer as master waits to receive ll_start_enc_rsp from slave
	 */
	p_connection->ll_response_timer = BTtimer_Set_Ticks(LE_LL_RESPONSE_TIMEOUT); 

#endif
	return NO_ERROR;
}
#endif
/***************************************************************************
 *
 * Function:	LE_LL_Start_Enc_Rsp
 *
 * Description: This decodes an incoming LLC_START_ENC_RSP PDU. If we are a Slave device
 *              the Tx Packet Counter is reset and a LLC_START_ENC_RSP is sent to the peer
 *         
 *              If this PDU is recieved as part of a Pause Encryption routine then a
 *              HCI_ENCRYPTION_KEY_REFRESH_COMPLETE_EVENT is sent via the HCI. Otherwise a 
 *              HCI_ENCRYPTION_CHANGE_EVENT is sent to the HCI.
 *              
 * Role:		Both
 **************************************************************************/
t_error LE_LL_Start_Enc_Rsp(t_LE_Connection* p_connection,t_p_pdu p_pdu)
{
	
	if(PATCH_FUN[LE_LL_START_ENC_RSP_ID]){
         
         return ((t_error (*)(t_LE_Connection* p_connection,t_p_pdu p_pdu))PATCH_FUN[LE_LL_START_ENC_RSP_ID])(p_connection,  p_pdu);
    }


	t_lm_event_info event_info;
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
	/*
	 * This ll pdu is only accepted when in state LE_W4_START_ENC_RSP.
	 */
	if (!(p_connection->current_security_state & LE_W4_START_ENC_RSP))
	{
		return LMP_PDU_NOT_ALLOWED;
	}

	/*
	 * Turn of timer when master or slave receives ll_start_enc_rsp.
	 */
	p_connection->ll_response_timer = BT_TIMER_OFF;

	/*
	 * If a slave receives this message, send back an LL_START_ENC_RSP
	 * and send encryption change event to the host.
	 */
	if (LEconnection_Get_Role(p_connection) == SLAVE)
	{
		LEllc_Encode_PDU(LL_START_ENC_RSP,p_connection,0);
	}


	event_info.status = NO_ERROR;
	event_info.handle = LEconnection_Determine_Connection_Handle(p_connection);

	if (p_connection->current_security_state & LE_ENCRYPTION_PAUSED)
	{
		HCeg_Generate_Event(HCI_ENCRYPTION_KEY_REFRESH_COMPLETE_EVENT, &event_info);
		/*hardfault_ERR = 1;*/
	}
	else
	{
		if (p_connection->current_security_state & LE_ENCRYPTION_ACTIVE)
			event_info.mode = LE_ENCRYPTION_ACTIVE;
		else
			event_info.status = UNSPECIFIED_ERROR;

#if ((PRH_BS_CFG_SYS_LE_SMP_INCLUDED!=1) && (PRH_BS_CFG_SYS_LE_GAP_INCLUDED!=1))
			HCeg_Generate_Event(HCI_ENCRYPTION_CHANGE_EVENT, &event_info);
#endif
	}

#if (PRH_BS_CFG_SYS_LE_SMP_INCLUDED!=1)
	p_connection->ll_action_pending &= ~LL_ENC_REQ_PENDING;
	if (LEconnection_Get_Role(p_connection) == MASTER)
		p_connection->ll_action_pending &= ~LL_ENCRYPTION_TRANSITIONING;

	/*
	 * Turn off encryption paused state.
	 * It was left on until this point in order to check which event to send.
	 */
	p_connection->current_security_state &= (~LE_W4_START_ENC_RSP & ~LE_ENCRYPTION_PAUSED) ;
#endif

#endif
	return NO_ERROR;
}
/***************************************************************************
 *
 * Function:	LE_LL_Pause_Enc_Req
 *
 * Description: This decodes an incoming LLC_PAUSE_ENC_REQ PDU
 *
 * Role:		Slave
 **************************************************************************/
t_error LE_LL_Pause_Enc_Req(t_LE_Connection* p_connection,t_p_pdu p_pdu)
{
	if(PATCH_FUN[LE_LL_PAUSE_ENC_REQ_ID]){
         
         return ((t_error (*)(t_LE_Connection* p_connection,t_p_pdu p_pdu))PATCH_FUN[LE_LL_PAUSE_ENC_REQ_ID])(p_connection,  p_pdu);
    }

#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
	//Check if le encryption is supported
	if (!LEconfig_Is_Features_Encryption_Supported())
	{
		LEllc_Encode_PDU(LL_REJECT_IND,p_connection,UNSUPPORTED_REMOTE_FEATURE);
		return UNSUPPORTED_REMOTE_FEATURE;
	}

	if (!p_connection)
	{
		return NO_CONNECTION;
	}

	p_connection->ll_action_pending |= LL_ENCRYPTION_TRANSITIONING;

	//This packet is sent encryptied
	LEllc_Encode_PDU(LL_PAUSE_ENC_RSP,p_connection,0);
	
	/*
	 * Set timer for slave to wait to receive ll_pause_enc_rsp from master
	 */
	p_connection->ll_response_timer = BTtimer_Set_Ticks(LE_LL_RESPONSE_TIMEOUT);
	
	p_connection->current_security_state |= LE_W4_PAUSE_ENC_RSP;

#endif
	return NO_ERROR;
}

/***************************************************************************
 *
 * Function:	LE_LL_Pause_Enc_Rsp
 *
 * Description: This decodes an incoming LE_LL_Pause_Enc_Rsp PDU
 *				If the device is a slave role wait for LLC_ENC_REQ pdu.
 *				If the device is a master role, send LLC_PAUSE_ENC_RSP pdu.
 *				Then send LL_ENC_REQ pdu.
 *
 * Role:		Both
 **************************************************************************/
t_error LE_LL_Pause_Enc_Rsp(t_LE_Connection* p_connection,t_p_pdu p_pdu)
{
	
	if(PATCH_FUN[LE_LL_PAUSE_ENC_RSP_ID]){
         
         return ((t_error (*)(t_LE_Connection* p_connection,t_p_pdu p_pdu))PATCH_FUN[LE_LL_PAUSE_ENC_RSP_ID])(p_connection,  p_pdu);
    }

#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
	/* 
	 * This ll pdu is only accepted when in state LE_W4_PAUSE_ENC_RSP.
	 */
	if (!(p_connection->current_security_state & LE_W4_PAUSE_ENC_RSP))
	{
		return LMP_PDU_NOT_ALLOWED;
	}

	if (LEconnection_Get_Role(p_connection) == SLAVE)
	{
		p_connection->current_security_state |= LE_W4_ENC_REQ;
		p_connection->current_security_state &= ~LE_W4_PAUSE_ENC_RSP;

		/*
		 * Restart timer as slave waits to receive ll_enc_req from master
		 */
		p_connection->ll_response_timer = BTtimer_Set_Ticks(LE_LL_RESPONSE_TIMEOUT);

	}
	else
	{
		/*
		 * Clear timer as slave receives ll_pause_enc_rsp from master
		 */
		p_connection->ll_response_timer = BT_TIMER_OFF;
		/*
		 * Set state to encryption paused and clear all other states
		 * except LE_W4_ENC_RSP.
		 */
		p_connection->current_security_state = (LE_ENCRYPTION_PAUSED | LE_W4_ENC_RSP);

		/*
		 * These packets are sent unencrypted
		 */
		LEllc_Encode_PDU(LL_PAUSE_ENC_RSP,p_connection,0);
		LEllc_Encode_PDU(LL_ENC_REQ,p_connection, 0x00);
	}

#endif
	return NO_ERROR;
}
#if (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE == 1)
/***************************************************************************
 *
 * Function:	LE_LL_Enc_Req
 *
 * Description: This decodes an incoming LLC_ENC_REQ PDU
 *
 * Role:		Slave
 **************************************************************************/
t_error LE_LL_Enc_Req(t_LE_Connection* p_connection,t_p_pdu p_pdu)
{
	
	if(PATCH_FUN[LE_LL_ENC_REQ_ID]){
         
         return ((t_error (*)(t_LE_Connection* p_connection,t_p_pdu p_pdu))PATCH_FUN[LE_LL_ENC_REQ_ID])(p_connection, p_pdu);
    }

#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
	int i;

	u_int8 temp_rand_buff[16];

	if (!LEconfig_Is_Features_Encryption_Supported())
	{
		LEllc_Encode_PDU(LL_REJECT_IND,p_connection,UNSUPPORTED_REMOTE_FEATURE);
		return UNSUPPORTED_REMOTE_FEATURE;
	}

	if (!p_connection)
	{
		return NO_CONNECTION;
	}

	if (LEconnection_Get_Role(p_connection) == MASTER)
		return COMMAND_DISALLOWED;

	/*
	 * If there is a start encryption procedure already in the process return
	 * a command disallowed as there can only be one start encryption procedure 
	 * occurring at any time.
	 */
	if (p_connection->ll_action_pending & LL_ENC_REQ_PENDING)
	{		
		return COMMAND_DISALLOWED;
	}

	if (!(p_connection->current_security_state & LE_W4_ENC_REQ) &&
		(p_connection->current_security_state != LE_ENCRYPTION_NOT_ACTIVE))
	{
		return LMP_PDU_NOT_ALLOWED;
	}
	/*
	 * Clear timer as slave receives ll_pause_enc_rsp from master
	 */
	p_connection->ll_response_timer = BT_TIMER_OFF;

	p_connection->ll_action_pending |= LL_ENCRYPTION_TRANSITIONING;
	p_connection->ll_action_pending |= LL_ENC_REQ_PENDING;

	for (i=0;i<8;i++)
		LE_config.random_number[i] = p_pdu[i];

	p_pdu+=8;

	LE_config.encrypt_diversifier =  LMutils_Get_Uint16(p_pdu);

	p_pdu+=2;

	for (i=0;i<8;i++)
		LE_config.SKD[i] = p_pdu[i];

	p_pdu+=8;

	if (p_connection->current_security_state & LE_ENCRYPTION_ACTIVE)
	{
		for (i=0;i<4;i++)
			p_connection->IVnew[i] = p_pdu[i];
	}
	else
	{
		for (i=0;i<4;i++)
			p_connection->IV[i] = p_pdu[i];
	}

	SYSrand_Get_Rand_128_Ex(temp_rand_buff);

	for(i=0;i<8;i++)
		LE_config.SKD[8+i] = temp_rand_buff[i];

	if (p_connection->current_security_state & LE_ENCRYPTION_ACTIVE)
	{
		for(i=0;i<4;i++)
			p_connection->IVnew[i+4] = temp_rand_buff[i+8];
	}
	else
	{
		for(i=0;i<4;i++)
			p_connection->IV[i+4] = temp_rand_buff[i+8];
	}

	LEllc_Encode_PDU(LL_ENC_RSP,p_connection,0);

	/*
	 * if the request ltk event is masked send ll_reject_ind
	 */

	if (!(LE_config.event_mask  & LE_LONG_TERM_REQUEST_EVENT_MASK))
	{
		LEllc_Encode_PDU(LL_REJECT_IND,p_connection,PIN_MISSING);
		p_connection->ll_action_pending &= ~LL_ENCRYPTION_TRANSITIONING;
		p_connection->ll_action_pending &= ~LL_ENC_REQ_PENDING;
		p_connection->current_security_state &= ~LE_W4_ENC_REQ;
		return NO_ERROR;
	}
	else
	{
		LEeventgen_Generate_LE_Meta_Event(LE_SUBEVENT_LONG_TERM_KEY_REQUEST, p_connection,NO_ERROR);
		LE_config.link_key_request_timer = BTtimer_Set_Ticks(LE_LINK_KEY_REQUEST_TIMEOUT);
	}
	
	p_connection->current_security_state &= ~LE_W4_ENC_REQ;
	p_connection->current_security_state |= LE_W4_LTK_REPLY;
#endif
	return NO_ERROR;
}
#endif


#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1)
/***************************************************************************
 *
 * Function:	LE_LL_Enc_Rsp
 *
 * Description: This decodes an incoming LLC_ENC_RSP PDU. This is applicable only in a 
 *              Master device and results in the calculation of the Session Key.
 *
 * Role:		Master
 **************************************************************************/
t_error LE_LL_Enc_Rsp(t_LE_Connection* p_connection,t_p_pdu p_pdu)
{
    if(PATCH_FUN[LE_LL_ENC_RSP_ID]){
         
         return ((t_error (*)(t_LE_Connection* p_connection,t_p_pdu p_pdu))PATCH_FUN[LE_LL_ENC_RSP_ID])(p_connection, p_pdu);
    }
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
	int i;
	u_int8 dummy_key[16];
	
	if (!p_connection)
	{
		return NO_CONNECTION;
	}
	
	if (LEconnection_Get_Role(p_connection) == SLAVE)
		return COMMAND_DISALLOWED;

	/* 
	 * This ll pdu is only accepted when in state LE_W4_ENC_RSP.
	 */
	if (!(p_connection->current_security_state & LE_W4_ENC_RSP))
	{
		return LMP_PDU_NOT_ALLOWED;
	}
#if 0 // GF - Need review of Data handling in Pause/Resume Encryption..
	p_connection->ll_action_pending |= LL_ENCRYPTION_TRANSITIONING;
#endif
	for (i=0;i<8;i++)
		LE_config.SKD[8+i] = p_pdu[i];

	p_pdu+=8;
	if (p_connection->current_security_state & LE_ENCRYPTION_ACTIVE)
	{
		for (i=0;i<4;i++)
			p_connection->IVnew[4+i] = p_pdu[i];
	}
	else
	{
		for (i=0;i<4;i++)
			p_connection->IV[4+i] = p_pdu[i];
	}

	/*
	 * The most significant octet of key corresponds to key[0], the most significant
	 * octet of plaintextData corresponds to in[0] and the most significant octet of
	 * encryptedData corresponds to out[0] using the notation specified in [1].
	 */
	{
		u_int8 inverted_SKD[16];
		u_int8 inverted_LTK[16];

		for(i=0;i<16;i++)
		{
			inverted_SKD[15-i] = LE_config.SKD[i];
			inverted_LTK[15-i] = LE_config.long_term_key[i];
		}
		LEsecurity_HW_aes_encrypt_128(inverted_SKD,p_connection->session_key, inverted_LTK,dummy_key);

	}
	/*
	 * The current security state needs to be updated.
	 * No longer waiting for a encryption response
	 * Next step is to wait for the LL_Start_Enc_req from the slave
	 */
	p_connection->current_security_state &= ~LE_W4_ENC_RSP;
	p_connection->current_security_state |= LE_W4_START_ENC_REQ;
#endif

	/*
	 * ReStart the LL Response Timer.
	 */
	p_connection->ll_response_timer = BTtimer_Set_Ticks(LE_LL_RESPONSE_TIMEOUT); 

	return NO_ERROR;
}
#endif
//#ifndef REDUCE_ROM
#if (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==0) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 0)
t_error LE_LL_UnSupported_Pdu(t_LE_Connection* p_connection,t_p_pdu p_pdu)
{
	LEllc_Encode_PDU(LL_REJECT_IND, p_connection ,LMP_PDU_NOT_ALLOWED);
	return NO_ERROR;
}
#endif
/***************************************************************************
 *
 * Function:	LE_LL_Unknown_Rsp
 *
 * Description: This decodes an incoming LLC_UNKNOWN_RSP PDU
 *
 * Role:		MASTER
 **************************************************************************/

t_error LE_LL_Unknown_Rsp(t_LE_Connection* p_connection,t_p_pdu p_pdu)
{
	
	if(PATCH_FUN[LE_LL_UNKNOWN_RSP_ID]){
         
         return ((t_error (*)(t_LE_Connection* p_connection,t_p_pdu p_pdu))PATCH_FUN[LE_LL_UNKNOWN_RSP_ID])(p_connection,  p_pdu);
    }

	u_int8 opcode = *p_pdu;
	t_lm_event_info event_info;

	switch(opcode)
	{

	case LL_CONNECTION_UPDATE_REQ :
		if (p_connection->ll_action_pending & LL_CONNECTION_UPDATE_PENDING)
		{
			p_connection->ll_action_pending &= ~LL_CONNECTION_UPDATE_PENDING;

			LEeventgen_Generate_LE_Meta_Event(LE_SUBEVENT_CONNECTION_UPDATE_COMPLETE,p_connection,(t_error)0x19);

		}
		break;


	case LL_CHANNEL_MAP_REQ :
		if (p_connection->ll_action_pending & LL_CHANNEL_MAP_REQ_PENDING)
		{
			p_connection->ll_action_pending &= ~LL_CHANNEL_MAP_REQ_PENDING;
		}
		break;

#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
	case LL_ENC_REQ :
		if (p_connection->ll_action_pending & LL_ENC_REQ_PENDING)
		{
			p_connection->ll_action_pending &= ~LL_ENCRYPTION_TRANSITIONING;

			event_info.handle = LEconnection_Determine_Connection_Handle(p_connection);
			event_info.status = (t_error)0x19 /* Unknown LMP PDU */;
			event_info.mode = 0;
			
			p_connection->ll_action_pending &= ~LL_ENC_REQ_PENDING;
			p_connection->current_security_state &= ~LE_W4_ENC_RSP;

			if (p_connection->current_security_state & LE_ENCRYPTION_PAUSED)
			{
				HCeg_Generate_Event(HCI_ENCRYPTION_KEY_REFRESH_COMPLETE_EVENT, &event_info);
			}
			else
			{
				HCeg_Generate_Event(HCI_ENCRYPTION_CHANGE_EVENT, &event_info);
			}

		}
		break;
#endif

	case LL_FEATURE_REQ :
		if (p_connection->ll_action_pending & LL_FEATURE_REQ_PENDING)
		{
			p_connection->ll_action_pending &= ~LL_FEATURE_REQ_PENDING;
			LEeventgen_Generate_LE_Meta_Event(LE_SUBEVENT_READ_REMOTE_USED_FEATURES_COMPLETE,p_connection,(t_error)0x19); 
		}
		break;

	case LL_VERSION_IND :
		// Report - HCI_Read_Remove_Version_Complete Event with Status = Failure

		if (p_connection->ll_action_pending & LL_VERSION_IND_PENDING)
		{
			p_connection->ll_action_pending &= ~LL_VERSION_IND_PENDING;

			event_info.handle = LEconnection_Determine_Connection_Handle(p_connection);
			event_info.status = (t_error)0x19 /* Unknown LMP PDU */; 
			HCeg_Generate_Event(HCI_READ_REMOTE_VERSION_INFORMATION_COMPLETE_EVENT,&event_info);

		}
		break;

	default :
		break;

	}
	return NO_ERROR;
}

/***************************************************************************
 *
 * Function:	LE_LL_Reject_Ind
 *
 * Description: This decodes an incoming LLC_REJECT_IND PDU
 *
 * Role:		MASTER
 **************************************************************************/

t_error LE_LL_Reject_Ind(t_LE_Connection* p_connection,t_p_pdu p_pdu)
{
	
	
	if(PATCH_FUN[LE_LL_REJECT_IND_ID]){
         
         return ((t_error (*)(t_LE_Connection* p_connection,t_p_pdu p_pdu))PATCH_FUN[LE_LL_REJECT_IND_ID])(p_connection,  p_pdu);
    }

	u_int8 reject_reason = *p_pdu;
	t_lm_event_info event_info;

	if (!p_connection)
	{
		return NO_CONNECTION;
	}

	if (p_connection)
	{
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
		if (p_connection->ll_action_pending & LL_ENC_REQ_PENDING)
		{
			p_connection->ll_action_pending &= ~LL_ENCRYPTION_TRANSITIONING;

			event_info.handle = LEconnection_Determine_Connection_Handle(p_connection);
			event_info.status = (t_error)reject_reason;  
			event_info.mode = 0;

		    p_connection->ll_action_pending &= ~LL_ENC_REQ_PENDING;

			if (p_connection->current_security_state & LE_ENCRYPTION_PAUSED)
			{
				p_connection->current_security_state &= (~LE_ENCRYPTION_PAUSED & ~LE_W4_ENC_RSP) ;

				HCeg_Generate_Event(HCI_ENCRYPTION_KEY_REFRESH_COMPLETE_EVENT, &event_info);
			}
			else
			{

				p_connection->current_security_state &= ~LE_W4_ENC_RSP;

				HCeg_Generate_Event(HCI_ENCRYPTION_CHANGE_EVENT, &event_info);
			}
		}
		else
#endif
			if (p_connection->ll_action_pending & LL_VERSION_IND_PENDING)
		{
			p_connection->ll_action_pending &= ~LL_VERSION_IND_PENDING;

			event_info.handle = LEconnection_Determine_Connection_Handle(p_connection);
			event_info.status =  (t_error)reject_reason;    
			HCeg_Generate_Event(HCI_READ_REMOTE_VERSION_INFORMATION_COMPLETE_EVENT,&event_info);
		}
		else if (p_connection->ll_action_pending & LL_CONNECTION_UPDATE_PENDING)
		{
			p_connection->ll_action_pending &= ~LL_CONNECTION_UPDATE_PENDING;
			LEeventgen_Generate_LE_Meta_Event(LE_SUBEVENT_CONNECTION_UPDATE_COMPLETE,p_connection,(t_error)reject_reason); 
		}
		else if (p_connection->ll_action_pending & LL_CHANNEL_MAP_REQ_PENDING)
		{
			p_connection->ll_action_pending &= ~LL_CHANNEL_MAP_REQ_PENDING;
		}
		else if (p_connection->ll_action_pending & LL_FEATURE_REQ_PENDING)
		{
			p_connection->ll_action_pending &= ~LL_FEATURE_REQ_PENDING;
			LEeventgen_Generate_LE_Meta_Event(LE_SUBEVENT_READ_REMOTE_USED_FEATURES_COMPLETE,p_connection,(t_error)reject_reason); 
		}
	}
	return NO_ERROR;
}
#endif

/***************************************************************************
 *
 * Function:	LE_LL_Ctrl_Handle_LE_IRQ
 *
 * Description: 
 * This function is the main interface to the Low Energy component of the system.
 * This is called from lslc_irq when Low Energy is enabled.
 *
 * This is the outer level LE state machine. The actions performed are dependent on the 
 * LE current state, held in 'LE_config.state'
 * 
 **************************************************************************/
t_error LE_LL_Ctrl_Handle_LE_IRQ(u_int8 IRQ_Type)
{
	
	if(PATCH_FUN[LE_LL_CTRL_HANDLE_LE_IRQ_ID]){
         
         return ((t_error (*)(u_int8 IRQ_Type))PATCH_FUN[LE_LL_CTRL_HANDLE_LE_IRQ_ID])(IRQ_Type);
    }

	u_int32 current_clk;
    //uint_8t  state=0;
	if ((LE_config.num_le_links) && (LE_config.slave_link_active))
	{
		if(!after_rx_first_packet)
		{
			current_clk = HW_Get_Native_Clk_Avoid_Race();
			current_clk = current_clk & 0xFFFFFFFC;
		}
		else
		{
			current_clk = HW_Get_Bt_Clk_Avoid_Race(); //- piconet clk
		}
	}
	else
	{
		after_rx_first_packet = 0;
		current_clk = HW_Get_Native_Clk_Avoid_Race()-4; //no link use native clk
	}
			

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
	if ((LE_config.num_le_links) && (LE_config.state != CONNECTION_STATE))
	{
		if (BTtimer_Is_Expired_For_Time(LE_config.next_connection_event_time, current_clk /*+ 1*/))
		{
			LEconnection_Handle_Connection_Event(LE_config.active_link,current_clk);
		}
	}
#endif

	switch(LE_config.state)
	{
	case STANDBY_STATE :
        
		if ((IRQ_Type == LE_TIM_0) || (IRQ_Type == LE_TIM_2) || (IRQ_Type == LE_TIM_1) || (IRQ_Type == LE_TIM_3))
		{
#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1) || (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1)
			if (LEscan_Is_Next_Scan_Due(current_clk))
				LEscan_Scan_Interval_Timeout(current_clk);
			else
#endif
#if (PRH_BS_CFG_SYS_LE_BROADCAST_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)

			if (LEadv_Is_Next_Adv_Due(current_clk))
				LEadv_Advertising_Event_Begin(current_clk);
			else
#endif
#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
			if (BTtimer_Is_Expired_For_Time(LE_config.next_initiator_scan_timer, current_clk + 1))
				LEconnection_Initiator_Scan_Interval_Timeout(current_clk);
			else
#endif
			if (LE_config.test_mode == LE_TEST_MODE_ACTIVE)
				//LEtest_Test_Event_Begin();
			    ((void (*)(void))PATCH_FUN[LETEST_TEST_EVENT_BEGIN])();
		}
		break;

	case TEST_STATE :
		//LEtest_Ctrl_Handle_Test_Mode_Event(IRQ_Type);
	    ((t_error (*)(u_int8))PATCH_FUN[LETEST_CTRL_HANDLE_TEST_MODE_EVENT])(IRQ_Type);
		break;
#if (PRH_BS_CFG_SYS_LE_BROADCAST_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
	case ADVERTISING_STATE :
		LEadv_Handle_Advertising(IRQ_Type,current_clk);
		break;
#endif

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1) || (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1)
	case SCANNING_STATE :
		LEscan_Ctrl_Handle_Scanning(IRQ_Type,current_clk);
		break;
#endif

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
	case INITIATING_STATE :
		LEscan_Ctrl_Handle_Scanning(IRQ_Type,current_clk);
		break;
#endif

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
	case CONNECTION_STATE :
		LE_LL_Ctrl_Handle_Connection_Event(IRQ_Type,current_clk);
		break;
#endif
	}


#if 0
       if ( (IRQ_Type == LE_TIM_2))  // || (SCANNING_STATE == LE_config.state) )
        {
            DMA_buf[0]=current_clk;
            DMA_buf[1]=current_clk>>8;
            DMA_buf[2]=LE_config.sub_state; //HW_get_bt_clk(); //LE_config.next_connection_event_time;
            DMA_buf[3]=0xa7;
            DMA_Uart_SendChar(1);   
        }
#endif 

    
	return NO_ERROR;
}

/***************************************************************************
 *
 * Function:	LE_LL_Set_Adv_Header
 *
 * Description: 
 * This function sets the header for an advertising packet in HW.
 * 
 **************************************************************************/

void LE_LL_Set_Adv_Header(u_int8 type, u_int8 tx_add, u_int8 rx_add, u_int8 length)
{
	HWle_set_tx_adv_header( type,  tx_add,  rx_add,  length);
}	

/***************************************************************************
 *
 * Function:	LE_LL_Encode_Own_Address_In_Payload
 *
 * Description: 
 * This function writes a devices own address into memory location, pointed to by 
 * p_payload
 * 
 **************************************************************************/
 
void LE_LL_Encode_Own_Address_In_Payload(u_int8* p_payload,u_int8 address_type)
{
    if(PATCH_FUN[LE_LL_ENCODE_OWN_ADDRESS_IN_PAYLOAD_ID]){
         ((void (*)(u_int8* p_payload,u_int8 address_type))PATCH_FUN[LE_LL_ENCODE_OWN_ADDRESS_IN_PAYLOAD_ID])(p_payload,address_type);
         return ;
    }

	const t_bd_addr* p_bd_addr;
	u_int8 i;

	if(address_type == PUBLIC_ADDRESS)
	{
		p_bd_addr = SYSconfig_Get_Local_BD_Addr_Ref();
	}
	else
	{
		p_bd_addr = &LE_config.random_address;
	}

	for (i=0;i<6;i++)
	{
		p_payload[i] = p_bd_addr->bytes[i];
	}

}

#if (__DEBUG_LE_SLEEP == 0)

/***************************************************************************
 *
 * Function:	LE_LL_InactiveSlots
 *
 * Description: 
 * This function check for the next Activity in Low Power Mode.
 * It determines the number of slots (0.625mS) the system have no activity.
 * Return: 0xFFFFFFFF - no activity planned
 * 		0 - busy or currently active
 * 		(SYS_CFG_LE_LC_MIN_SLOTS_FOR_SLEEP_PROCEDURE + 1)..0xFFFFFFFE
 * 					 - number of slots till next activity
 **************************************************************************/

u_int32 LE_LL_InactiveSlots(void)
{
	if(PATCH_FUN[LE_LL_INACTIVESLOTS_ID]){
         
         return ((u_int32 (*)(void))PATCH_FUN[LE_LL_INACTIVESLOTS_ID])();
    }
	u_int32 interval = 0xFFFFFFFF;
	
#if (PRH_BS_CFG_SYS_LOW_POWER_MODE_SUPPORTED==1)

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1) ||  (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1)
	u_int32 slots_to_activity;
#endif

	u_int32 current_clk = HW_Get_Native_Clk_Avoid_Race();

#if ((PRH_BS_CFG_SYS_SLEEP_MASTER_SUPPORTED==0) || (SYS_CFG_LE_SLEEP_ACTIVE_CONNECTIONS == 0))
		if(LE_config.num_le_links && !LE_config.slave_link_active)
			return 0; // active
#endif

#if ((PRH_BS_CFG_SYS_SLEEP_SLAVE_SUPPORTED==0) || (SYS_CFG_LE_SLEEP_ACTIVE_CONNECTIONS == 0))
	if(LE_config.num_le_links && !LE_config.slave_link_active)
		return 0; // active
#endif

#if (PRH_BS_CFG_SYS_LE_BROADCAST_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
	interval = LEadv_Get_Slots_To_Next_Advertising_Timer(current_clk);
#endif

#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE)
	slots_to_activity =  LEscan_Get_Slots_To_Next_Scanning_Timer(current_clk);
	if (slots_to_activity < interval)
		interval = slots_to_activity;

#endif

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
	if (LE_config.num_le_links)
	{
		slots_to_activity = (BTtimer_Clock_Difference(current_clk,LE_config.next_connection_event_time) >>1);
		if (slots_to_activity < interval)
			interval = slots_to_activity;
	}
#endif

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
	if (LE_config.next_initiator_scan_timer != BT_TIMER_OFF)
	{
		slots_to_activity = (BTtimer_Clock_Difference(current_clk,LE_config.next_initiator_scan_timer) >>1);
		if (slots_to_activity < interval)
			interval = slots_to_activity;
	}
#endif

	if (interval <=  SYS_CFG_LE_LC_MIN_SLOTS_FOR_SLEEP_PROCEDURE)
	{
		interval = 0;
	} // no need for extra check

#endif
	return interval;
}


u_int8 LE_LL_Get_Current_State(void)
{
	return LE_config.state;

}

u_int32 LE_LL_Slots_To_Next_LE_Activity(u_int32 current_clk)
{
	if(PATCH_FUN[LE_LL_SLOTS_TO_NEXT_LE_ACTIVITY_ID]){
         
         return ((u_int32 (*)(u_int32 current_clk))PATCH_FUN[LE_LL_SLOTS_TO_NEXT_LE_ACTIVITY_ID])(current_clk);
    }
	u_int32 interval = 0xFFFFFFFF;

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1) ||  (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1)
	u_int32 slots_to_activity;
#endif

	//u_int32 current_clk = HW_Get_Native_Clk_Avoid_Race();

#if (PRH_BS_CFG_SYS_LE_BROADCAST_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
	interval = LEadv_Get_Slots_To_Next_Advertising_Timer(current_clk);
#endif

#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE)
	slots_to_activity =  LEscan_Get_Slots_To_Next_Scanning_Timer(current_clk);
	if (slots_to_activity < interval)
		interval = slots_to_activity;

#endif

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
	if (LE_config.num_le_links)
	{
		slots_to_activity = (BTtimer_Clock_Difference(current_clk,LE_config.next_connection_event_time) >>1);
		if (slots_to_activity < interval)
			interval = slots_to_activity;
	}
#endif

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
	if (LE_config.next_initiator_scan_timer != BT_TIMER_OFF)
	{
		slots_to_activity = (BTtimer_Clock_Difference(current_clk,LE_config.next_initiator_scan_timer) >>1);
		if (slots_to_activity < interval)
			interval = slots_to_activity;
	}
#endif
	return interval;
}


/***************************************************************************
 *
 * Function:	LE_LL_Sleep
 *
 * Description:
 * This function performs system Low Power Mode. It determines the length of time the system
 * should be placed in sleep mode and interfaces with the LC to perform the sleep.
 *
 **************************************************************************/

void LE_LL_Sleep(void)
{
	u_int32 slots_to_activity = LE_LL_InactiveSlots();

	if (slots_to_activity >  SYS_CFG_LE_LC_MIN_SLOTS_FOR_SLEEP_PROCEDURE)// SYS_CFG_MIN_SLOTS_FOR_SLEEP_PROCEDURE) &&
	{
		LMpol_Sleep(slots_to_activity,0x00,HW_get_native_clk());
	}
}
#endif



/***************************************************************************
 *
 * Function:	LE_LL_Handle_LLC_Ack
 *
 * Description: 
 * This function handles ACKnowledgements to certain LLC PDUs.
 * 
 **************************************************************************/

void LE_LL_Handle_LLC_Ack(t_LE_Connection* p_connection, u_int8 opcode)
{
	
	if(PATCH_FUN[LE_LL_HANDLE_LLC_ACK_ID]){
         ((void (*)(t_LE_Connection* p_connection, u_int8 opcode))PATCH_FUN[LE_LL_HANDLE_LLC_ACK_ID])(p_connection,   opcode);
         return ;
    }


	switch(opcode)
	{
    case LL_TERMINATE_IND :
#if 0
		LEconnection_Local_Disconnect(p_connection,p_connection->terminate_reason);
#else
		if (p_connection->role == MASTER)
		{
			p_connection->ll_action_pending |=  LL_TERMINATE_PENDING;
		}
		else {
			HWle_clear_tx_enable();
			LEconnection_Local_Disconnect(p_connection,p_connection->terminate_reason);
		}
#endif
    	break;
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
    case LL_START_ENC_REQ :
    	{
    		// Setup to receive encrypted data
    		p_connection->current_security_state |= LE_ENCRYPTION_ACTIVE;
    		p_connection->enc_tx_pkt_counter = 0;
    		p_connection->on_air_tx_pkt_count=0;
			p_connection->on_air_rx_pkt_count=0;
    	}
    	break;

	case LL_START_ENC_RSP :
    	{
			if (LEconnection_Get_Role(p_connection) == SLAVE)
				p_connection->ll_action_pending &= ~LL_ENCRYPTION_TRANSITIONING;
		}
		break;

    case LL_PAUSE_ENC_RSP :
    	if(LEconnection_Get_Role(p_connection) == SLAVE)
    	{

    		// set state to encryption paused and clear encryption active bit.
    		p_connection->current_security_state &= ~LE_ENCRYPTION_ACTIVE;
    		p_connection->current_security_state |= LE_ENCRYPTION_PAUSED;

    	}
    	break;

    case LL_CONNECTION_UPDATE_REQ :
    	break;

#endif
	}
}

/***************************************************************************
 *
 * Function:	LEsecurity_Check_Timers
 *
 * Description: This function checks if the link key request timer has timed
 *				out on any link. As the link key timer will only be set on
 *				one link at any time, the function returns on finding a timeout.
 *
 * Role:		Both
 *
 * Context:		Scheduler
 **************************************************************************/

void LE_LL_Check_Timers(void)
{
	
	if(PATCH_FUN[LE_LL_CHECK_TIMERS_ID]){
         ((void (*)(void))PATCH_FUN[LE_LL_CHECK_TIMERS_ID])();
         return ;
    }

	u_int32 current_clk;
	t_LE_Connection* p_connection; 

	if(native_bt_clk_result()&& (0!=g_debug_le_sleep)) return;
	
	p_connection = LEconnection_Find_Link_Entry(LE_config.active_link);
	current_clk = HW_Get_Native_Clk_Avoid_Race();
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
	if (BTtimer_Is_Expired_For_Time(LE_config.link_key_request_timer,current_clk))
	{
		LEsecurity_Long_Term_Key_Request_Timeout();
		return;
	}
#endif
	if (BTtimer_Is_Expired_For_Time(p_connection->ll_response_timer,current_clk))
	{
		LE_LL_Response_Timeout(p_connection);
		return;
	}

	if (BTtimer_Is_Expired_For_Time(p_connection->terminate_timer,current_clk))
	{
		LEconnection_Local_Disconnect(p_connection,LMP_RESPONSE_TIMEOUT);
	}

	/*--------------------------------------------
		CON/SLA/BV-13-C 
	
	//if (BTtimer_Is_Expired_For_Time(p_connection->supervision_TO_timer, current_clk - 270))  // for: TP/CON/SLA/BV-13-C
	
	if (BTtimer_Is_Expired_For_Time(p_connection->supervision_TO_timer, current_clk + 1))	  // for: Normol WORK.	
	---------------------------------------------*/

	if ((BTtimer_Is_Expired_For_Time(p_connection->supervision_TO_timer, current_clk + 1)) || (1 == wrong_connect_req_pkt))
	{
		wrong_connect_req_pkt = 0;
		LEconnection_Local_Disconnect(p_connection,0x08);

        //- TP/DDI/ADV/BV-06-C
		if (LEconnection_Get_Initial_Anchor_Point_Obtained(p_connection))
	        p_connection->terminate_reason = CONNECTION_TIMEOUT;
	    else
			p_connection->terminate_reason = EC_CONNECTION_FAILED_TO_BE_ESTABLISHED;
	
		if (LE_config.state != CONNECTION_STATE)
			_LEconnection_Close_Connection_Event(p_connection,0x00);

	}
}

/***************************************************************************
 *
 * Function:	LE_LL_Response_Timeout
 *
 * Description: This function handles a LL response timer timeout
 *
 * Role:		Both
 **************************************************************************/
void LE_LL_Response_Timeout(t_LE_Connection* p_connection)
{
	if(PATCH_FUN[LE_LL_RESPONSE_TIMEOUT_ID]){
         ((void (*)(t_LE_Connection* p_connection))PATCH_FUN[LE_LL_RESPONSE_TIMEOUT_ID])(p_connection);
         return ;
    }

	
	p_connection->ll_response_timer = BT_TIMER_OFF;
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
	LE_config.link_key_request_timer = BT_TIMER_OFF;
#endif

#if(PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
    // Added for Conformance 29 May 2013
	if (p_connection->current_security_state & LE_W4_ENC_RSP)
	{
		t_lm_event_info event_info;

		event_info.status = LMP_RESPONSE_TIMEOUT;
		event_info.handle = LEconnection_Determine_Connection_Handle(p_connection);
		event_info.mode = LE_ENCRYPTION_NOT_ACTIVE;
		HCeg_Generate_Event(HCI_ENCRYPTION_CHANGE_EVENT, &event_info);
		LEconnection_Local_Disconnect(p_connection,EC_CONNECTION_TERMINATED_DUE_TO_MIC_FAILURE);
	}
	else
#endif
	LEconnection_Local_Disconnect(p_connection,LMP_RESPONSE_TIMEOUT);
}


