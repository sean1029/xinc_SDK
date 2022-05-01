/***********************************************************************
 *
 * MODULE NAME:    le_connection.c
 * PROJECT CODE:   CEVA Low Energy Single Mode
 * DESCRIPTION:    Low Energy module for handling LLC Connections.
 * MAINTAINER:     Gary Fleming
 * CREATION DATE:  September 2011
 *
 *
 *LICENSE:
 *     This source code is copyright (c) 2011-2012 Ceva Inc.
 *     All rights reserved.
 *
 ***********************************************************************/

#include "sys_config.h"
#include "sys_mmi.h"
#include "sys_irq.h"
#include "hc_const.h"
#include "hci_params.h"
#include "hc_event_gen.h"

#include "lc_interface.h"
#include "hw_lc.h"
#include "hw_le_lc.h"
#include "le_advertise.h"
#include "le_scan.h"
#include "le_const.h"
#include "le_config.h"
#include "le_connection.h"
#include "le_frequency.h"
#include "le_security.h"
#include "le_link_layer.h"

#ifndef BLUETOOTH_MODE_LE_ONLY
#include "lmp_sco_container.h"
#include "lmp_config.h"
#endif
#include "lmp_utils.h"

#include "lslc_stat.h"
#include "lslc_irq.h"
#include "sys_rand_num_gen.h"
#include "hw_pta.h"
#include "hw_radio.h"
#include "tc_const.h"
#include "tc_event_gen.h"
#include "hc_flow_control.h"
#include "bt_timer.h"

#include "hw_lc.h"
#include "hw_hab_defs.h"
#include "hw_habanero.h"
#include "hw_memcpy.h"
#include "hw_delay.h"
#include "hw_habanero_impl.h"

#include "bt_mini_sched.h"
#include "le_power.h"
#include "sys_debug_config.h"
#include "DMA_uart_debug.h"
#include "global_val.h"
#include "patch_function_id.h"

extern volatile unsigned char more_data_enable; 

extern uint8_t continue_miss_packet_number_val;
extern uint8_t next_connect_event_start_time_val;
extern uint8_t transmit_window_remaining_val;
extern uint8_t ext_rx_window_val;

extern t_LE_Config LE_config;
#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
//static t_LE_Connection LE_connections[DEFAULT_MAX_NUM_LE_LINKS];

#if (PRH_BS_CFG_SYS_TRACE_LLDATA_VIA_HCI_SUPPORTED==1)
static u_int8 PRH_lldata_dbg_channel = 0;
static u_int8 PRH_lldata_dbg_length = 0;
static u_int16 PRH_lldata_dbg_header = 0;
#endif
//unsigned    int     Malloc_Error = 0;

/*u_int8 intraslot_settled = 0;*/

void _LEconnection_Encode_DataPDU(t_LE_Connection* p_connection);
u_int32 _LEconnection_Decode_DataPDU(t_LE_Connection* p_connection);


t_slots _LEconnection_Determine_Window_Widening_In_Frames(t_LE_Connection* p_connection);
/*u_int16 _LEconnection_Determine_Window_Widening(t_LE_Connection* p_connection);*/

/*
 * Connection Event Handling Functions
 */

void _LEconnection_Free_Connection(t_LE_Connection* p_connection);
u_int32  LEconnection_Find_Next_Connection_Event(u_int32 cur_clk);
void _LEconnection_Reset_Connection(t_LE_Connection* p_connection);
u_int32 LEconnection_Adjust_Clock(u_int32 clk,u_int16 IntraSlot);
//TODO: window widening
extern t_SYS_Config g_sys_config;
//u_int16 g_LE_Slave_IntraSlot = 0;
extern unsigned	int after_rx_first_packet;
//unsigned	int have_get_no_crc_pkt = 0;
//unsigned	int wrong_connect_req_pkt = 0;
//unsigned	char update_transmit_window_num = 0;
//unsigned	char continue_miss_packet_number = 0;
extern unsigned	char connect_no_pka_count ;
extern uint32_t	ext_rx_window ;
extern uint32_t    rssi_val;
extern uint8_t connect_always_wake;

void ext_rx_window_init(t_LE_Connection* p_connection)
{
	if(PATCH_FUN[EXT_RX_WINDOW_INIT_ID]){
         ((void (*)(t_LE_Connection* p_connection))PATCH_FUN[EXT_RX_WINDOW_INIT_ID])(p_connection);
         return ;
    }

	/* < 1s connectInterval */
	if(p_connection->connInterval < 800 ) {
		ext_rx_window = 1;
	}	
	/* 1s ~ (<2s) connectInterval */
	else if(p_connection->connInterval < 1600 ) {
		ext_rx_window = 2;
	}
	/* 2s ~ (<4s) connectInterval */
	else if(p_connection->connInterval < 3200 ) {
		ext_rx_window = 3;
	}
	/* 4s connectInterval */
	else if(p_connection->connInterval >= 3200 ) {
		ext_rx_window = 4;
	}
}

void ext_rx_window_update(t_LE_Connection* p_connection)
{
	if(PATCH_FUN[EXT_RX_WINDOW_UPDATE_ID]){
         ((void (*)(t_LE_Connection* p_connection))PATCH_FUN[EXT_RX_WINDOW_UPDATE_ID])(p_connection);
         return ;
    }

    if ((p_connection->connInterval <=6) && (ext_rx_window > 3))
    {
        ext_rx_window =3;
    }
    else if((p_connection->connInterval <=12)&& (ext_rx_window > 4))
    {
        ext_rx_window =4;
    }
	if(ext_rx_window > 10) 
    {
        return;
     }
    
	ext_rx_window++;
}
	

#if (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)

/********************************************************************************************
 * Function :- LEconnection_Set_Next_Slave_Connection_Event_Time
 *
 * Description :
 * Updates the next Connection Event time based on the current_clk and the connection interval.
 * If connection update is pending and the next CE event count is the CE update instant, the
 * new connection parameters are used to calculate the next connection instant.
 *
 * This is called at the start of a slave CE state machine if the initial anchor point has not 
 * already been obtained.
 ********************************************************************************************/

__INLINE__ void LEconnection_Set_Next_Slave_Connection_Event_Time(t_LE_Connection* p_connection,u_int32 current_clk)
{
	// If we are have an intraslot greater than 0x2F0 we set the want the next connection event
	// to occur one frame later.
#if 1
	current_clk = LEconnection_Adjust_Clock(current_clk,g_LE_Slave_IntraSlot);
#endif
	if (LE_config.Auto_Widen_MultiSlot==0)
	{
		p_connection->next_connect_event_start_time = BTtimer_Safe_Clockwrap((current_clk - 4)+ \
				(p_connection->connInterval*4));
	}
	else
	{
		p_connection->next_connect_event_start_time = BTtimer_Safe_Clockwrap((current_clk - (4))+ \
				(p_connection->connInterval*4) - (_LEconnection_Determine_Window_Widening_In_Frames(p_connection)*2));
	}

	if( (p_connection->ll_action_pending & LL_CONNECTION_UPDATE_PENDING) &&
	   ((p_connection->connection_event_counter+1)== p_connection->connection_update_instant))
	{
		p_connection->next_connect_event_start_time = BTtimer_Safe_Clockwrap(p_connection->next_connect_event_start_time + (p_connection->transmit_window_offset_new *4) /*+ 4*/); // to account for the T.W.O + 1.25ms in spec;
	}
	LE_config.next_connection_event_time = p_connection->next_connect_event_start_time;
}


/********************************************************************************************
 * Function :- LEconnection_Update_Next_Slave_Connection_Event_Time
 *
 * Description :
 * Updates the next Connection Event time based on the current_clk and the connection interval.
 * If connection update is pending and the next CE event count is the CE update instant, the
 * new connection parameters are used to calculate the next connection instant.
 *
 * This is called in the initial Rx of the Slave State machine, on receipt of a PDK or NO_PKD 
 * interrupt.
 *
 ********************************************************************************************/
__INLINE__ void LEconnection_Update_Next_Slave_Connection_Event_Time(t_LE_Connection* p_connection,u_int32 current_clk)
{
	// Ensure the current clock is adjusted to epire on Tim 0 prior to CE event
	// clk & 0xFFFFFFFC to get Tim 0 - then clk = clk - 1 to ensure it expires
	// SEEMS to WORK PERFECTLY !!
#if 1
		/*current_clk = LEconnection_Adjust_Clock(current_clk,g_LE_Slave_IntraSlot);*/
	current_clk = HW_Get_Bt_Clk_Avoid_Race();
#endif

	p_connection->next_connect_event_start_time = (((current_clk&0xFFFFFFFC)-3)+ (p_connection->connInterval*4));

	if( (p_connection->ll_action_pending & LL_CONNECTION_UPDATE_PENDING) &&
	   ((p_connection->connection_event_counter+1)== p_connection->connection_update_instant))
	{
		p_connection->next_connect_event_start_time += (p_connection->transmit_window_offset_new *4)/*+ 4*/; // to account for the T.W.O + 1.25ms in spec;
	}

	p_connection->next_connect_event_start_time = BTtimer_Safe_Clockwrap(p_connection->next_connect_event_start_time);

	LE_config.next_connection_event_time = p_connection->next_connect_event_start_time;
	
}
#endif

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1)

/********************************************************************************************
 * Function :- LEconnection_Set_Next_Master_Connection_Event_Time
 *
 * Description :
 * Updates the next Connection Event time based on the current_clk and the connection interval.
 * If connection update is pending and the next CE event count is the CE update instant, the
 * new connection parameters are used to calculate the next connection instant.
 *
 * This is called in the initial Tx of the Master State machine.
 *
 ********************************************************************************************/
__INLINE__ void LEconnection_Set_Next_Master_Connection_Event_Time(t_LE_Connection* p_connection,u_int32  current_clk)
{

	p_connection->next_connect_event_start_time += (p_connection->connInterval*4);

	if( (p_connection->ll_action_pending & LL_CONNECTION_UPDATE_PENDING) &&
	   ((p_connection->connection_event_counter+1)== p_connection->connection_update_instant))
	{
		p_connection->next_connect_event_start_time += (p_connection->transmit_window_offset_new *4)+4;
	}

	p_connection->next_connect_event_start_time = BTtimer_Safe_Clockwrap(p_connection->next_connect_event_start_time);
	// If Next CE on this link is earlier than the current to LE_config.next_connection_event_time
	// then we reset the LE_config.next_connection_event_time to the next CE on this link.
}
#endif

/**************************************************************
 * Function :- LEconnection_Init
 *
 * Params :- None
 *
 * Return :- None
 *
 * Description :
 * Initialises the container for LE links. The number of available links is defined by
 * the #define DEFAULT_MAX_NUM_LE_LINKS.
 *
 **********************************************************************************/
void LEconnection_Init(void)
{
	 
    if(PATCH_FUN[LECONNECTION_INIT_ID]){
         ((void (*)(void))PATCH_FUN[LECONNECTION_INIT_ID])();
         return ;
    }


	u_int8 i;
	t_LE_Connection* p_connection;
	after_rx_first_packet = 0;
	have_get_no_crc_pkt = 0;
    continue_miss_packet_number=0;

	for (i=0;i<DEFAULT_MAX_NUM_LE_LINKS;i++)
	{
		p_connection = &LE_connections[i];
		p_connection->link_id = i;
#if (BUILD_TYPE != UNIT_TEST_BUILD) // TK Temp - remove static allocation for UNIT TESTS
		// Temp - Static allocation of DLs here cause array out of bounds in classic BT on full piconets
		p_connection->device_index = DL_Alloc_Link();
#endif
		_LEconnection_Reset_Connection(p_connection);
	}
}

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1)
#if (PRH_BS_CFG_SYS_LE_GAP_INCLUDED!=1)
/************************************************************************
 * Function :- LEconnection_Create_Connection
 *
 * Params :- p_pdu - a pointer to the payload of the Create_Connection command. This
 *                   is decoded to determine the parameters of the command.
 *
 * Returns :- t_error - An error message indicating if the command proceeded. If the command
 *                      proceeds a 'NO_ERROR' is returned.
 *
 * Description
 * This function handles a HCI_LE_Create_Connection form the HCI.
 *
 * Initially this function performs some basic checks
 *     1/ Check create connection not already pending
 *     2/ Allocate link and store in LE_config.initiator_link_id
 *     3/ Extract all the HCI Parameters and validate their range.
 *
 * Use HCI parameters to set up the parameters for the LE connection. And determine the
 * following :
 * 		1/ Access Address.
 *      2/ CRC Init
 *      3/ Connection Interval
 *      4/ Transmit Window Size and offset.
 *      5/ Channel Map and Hop Increment
 *
 *  Finally the Timers for the Initiator Scan Interval are setup.
 *
 *****************************************************************************************/
t_error LEconnection_Create_Connection(t_p_pdu p_pdu)
{
    if(PATCH_FUN[LECONNECTION_CREATE_CONNECTION_ID]){
        return ((t_error (*)(t_p_pdu p_pdu))PATCH_FUN[LECONNECTION_CREATE_CONNECTION_ID])(p_pdu);
    }
	u_int16 connection_scan_interval;
	u_int16 connection_scan_window;
	u_int8  initiator_filter_policy;
	u_int8  peer_address_type;
	u_int8  peer_address[6];
	u_int8  connection_own_address_type;
	u_int16 connection_interval_max;
	u_int16 connection_interval_min;
	u_int16 connection_latency;
	u_int16 supervision_timeout;
	u_int16 minimum_CE_length;
	u_int16 maximum_CE_length;
	u_int8 i;
	u_int32 proposed_access_address;
	t_LE_Connection* p_connection;
	
	// If a timer is already set for initiator scan interval then there is an
	// ongoing connection establishment. So return command disallowed.

	if (LE_config.next_initiator_scan_timer != BT_TIMER_OFF)
		return COMMAND_DISALLOWED;

#if (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
//	if (LEconnection_Does_Slave_Link_Exist())
//		return COMMAND_DISALLOWED;
#endif

	// If an Slave ACL link exists in Classic then we reject the request.
	// As currently Dual Mode Switching is not supported between Classic
	// with a slave role and and Low Energy Links.
#ifndef BLUETOOTH_MODE_LE_ONLY
	if(LMconfig_LM_Connected_As_Slave())
		return COMMAND_DISALLOWED;
#endif
	connection_scan_interval = HCIparam_Get_Uint16(p_pdu);
	p_pdu+=2;
	connection_scan_window = HCIparam_Get_Uint16(p_pdu);
	p_pdu+=2;
	
	initiator_filter_policy = *p_pdu;
	p_pdu++;
	peer_address_type = *p_pdu;
	p_pdu++;

	for (i=0;i<6;i++)
	{
		peer_address[i] = p_pdu[i];
	}

	p_pdu+=6;

	connection_own_address_type = *p_pdu;
	p_pdu++;

	connection_interval_min = HCIparam_Get_Uint16(p_pdu);
	p_pdu+=2;

	connection_interval_max = HCIparam_Get_Uint16(p_pdu);
	p_pdu+=2;

	connection_latency = HCIparam_Get_Uint16(p_pdu);
	p_pdu+=2;

	supervision_timeout = HCIparam_Get_Uint16(p_pdu);
	p_pdu+=2;

	minimum_CE_length = HCIparam_Get_Uint16(p_pdu);
	p_pdu+=2;

	maximum_CE_length = HCIparam_Get_Uint16(p_pdu);
	p_pdu+=2;


	if ((connection_scan_interval < 0x04) || (connection_scan_interval > 0x4000))
		return INVALID_HCI_PARAMETERS;

	if ((connection_scan_window < 0x04) || (connection_scan_window > 0x4000))
		return INVALID_HCI_PARAMETERS;

	if (connection_scan_window > connection_scan_interval)
		return INVALID_HCI_PARAMETERS;

	if ((initiator_filter_policy > 0x01) || (peer_address_type > 0x01) ||
		(connection_own_address_type > 0x01))
		return INVALID_HCI_PARAMETERS;

	if ((connection_interval_min < 0x06) || (connection_interval_min > 0x0C80))
		return INVALID_HCI_PARAMETERS;

	if ((connection_interval_max < 0x06) || (connection_interval_max > 0x0C80))
		return INVALID_HCI_PARAMETERS;

	if (connection_interval_min > connection_interval_max)
		return INVALID_HCI_PARAMETERS;

	if (connection_latency > 0x01F3)
		return INVALID_HCI_PARAMETERS;

	if ((supervision_timeout < 0x0A) || (supervision_timeout > 0x0C80))
		return INVALID_HCI_PARAMETERS;

	/* The connSupervisionTimeout shall be a multiple of 10 ms in the range of 100 ms to 32.0 s and it shall be larger than
     * it shall be larger than (1 + connSlaveLatency) * connInterval.
     */
	if ((supervision_timeout*8) < ((1 + connection_latency) * connection_interval_max))
		return INVALID_HCI_PARAMETERS;

    //***********************************************************************
	// Note :- The Disabling of Connectable Advertising is done automatically
	// by the advertising and connection event handling
	//***********************************************************************

	// Allocate an ID for the link
	if(NO_ERROR!=LEconfig_Allocate_Link_Id(&LE_config.initiator_link_id ))
	{
		return UNSPECIFIED_ERROR;
	}
	else
	{
		p_connection = &LE_connections[LE_config.initiator_link_id];
#if (PRH_BS_CFG_SYS_LE_DUAL_MODE==1)
		//p_connection->device_index = DL_Alloc_Link();
#else
		p_connection->device_index = LE_config.initiator_link_id;
#endif
	}

	// Now Assign the parameters
	//

	LE_config.initiator_scan_interval = connection_scan_interval;
	LE_config.initiator_scan_window = connection_scan_window;
	LE_config.initiator_filter_policy = initiator_filter_policy;

	LEconnection_Set_Peer_Addr_Type(p_connection,peer_address_type);
	for (i=0;i<6;i++)
		p_connection->peer_address.bytes[i] = peer_address[i];
	LEconnection_Set_Own_Addr_Type(p_connection,connection_own_address_type);

#if (PRH_SYS_CFG_HARDWARE_ADDRESS_FILTERING_SUPPORTED == 1)
	// If not connecting from the WhiteList I set the immediate address and address type and
	// configure the HW.
	if (initiator_filter_policy==0x00)
	{
		u_int32 address32_plus_control[2]; 
		u_int8* p_addr_plus_control = (u_int8*)address32_plus_control;
		u_int8 j;

		for (j=0;j<6;j++)
			p_addr_plus_control[j] = peer_address[j];

		//address32_plus_control[1] |= 0x00800000;
		if (peer_address_type==RANDOM_ADDRESS)
			p_addr_plus_control[7] |= 0x40;

		p_addr_plus_control[7] |= 0x80;

		// Use the Shadow Register to set the Immediate - Addr Filter.
		HWle_set_rx_filters_active(0x40);

	    hw_memcpy((u_int8*)JAL_LE_IMMEDIATE_ADDR,address32_plus_control, 8);

	}
#endif

	// Step 1 - Generate an access address
	do
	{
		proposed_access_address = SYSrand_Get_Rand();
		// To improve Access Address - Ensure we have more transitions in the upper 4 bits - thus giving
		// an extended pre-amble.
		proposed_access_address = ((proposed_access_address & 0xFFFFFF00) | 0x00000055);
	}
	while(!Vaidate_Access_Address(proposed_access_address));

#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==1)
	if (LE_config.preConfigured_access_address_available)
		p_connection->access_address = LE_config.preConfigured_access_address;
	else
		p_connection->access_address = proposed_access_address;
#else
	p_connection->access_address = proposed_access_address;
#endif
	// Step 2 - Generate the CRC init value

	p_connection->crc_init =( SYSrand_Get_Rand() & 0x00FFFFFF);

	//
	// Important to determine the ConnInterval - time between connection events.
	// Now - For simplicity we use a ConnInterval which is half way between the
	// max and min connection interval given in the HCI_LE_Create_Connection.
	// The connInterval shall be a multiple of 1.25 ms in the range of 7.5 ms to 4.0 s.
	// The connInterval is set by the Initiator’s Link Layer in the CONNNECT_REQ
	// PDU from the range given by the Host.

	// If we are operating in Dual mode and there are Classic eSCO/SCO links active
	// then we have to ensure that the Connection interval is a Multiple of the eSCO/SCO T value.

	{
#ifndef BLUETOOTH_MODE_LE_ONLY	//no syc connection anymore
		if (LMscoctr_Get_Number_SYN_Connections())
		{
			// Find a value which is N * 6 between the two intervals.
			u_int8 N;
			u_int8 Tsco_Frames;

			Tsco_Frames = LMscoctr_Find_Lowest_Tsco()/2;

			N = connection_interval_min/Tsco_Frames;

			p_connection->connInterval = (N+1)* Tsco_Frames;
		}
		else
#endif
		{

			u_int16 delta = (connection_interval_max - connection_interval_min);

			if (delta != 0)
			{
				p_connection->connInterval = ((delta/2) + connection_interval_min);
			}
			else
			{
				p_connection->connInterval = connection_interval_min;
			}
		}
	}

	// Step 3 & 4 - Transmit Window Size & Offset
	//-----------------------------------------
	// The transmitWindowSize shall be a multiple of 1.25 ms in the
	// range of 1.25 ms to the lesser of 10 ms and (connInterval - 1.25 ms).
	//

#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==1)
	if (LE_config.TCI_transmit_win_config)
	{
		p_connection->transmit_window_size = LE_config.transmit_win_size;
		p_connection->transmit_window_offset = LE_config.transmit_win_offset;
	}
	else
#endif
	{
		if (p_connection->connInterval > 8)
			p_connection->transmit_window_size = PRH_BS_CFG_DEFAULT_TRANSMIT_WINDOW_SIZE; // Transmit Window of 5 ms
		else
			p_connection->transmit_window_size = 2;

		if (p_connection->connInterval > PRH_BS_CFG_DEFAULT_TRANSMIT_WINDOW_OFFSET)
			p_connection->transmit_window_offset = PRH_BS_CFG_DEFAULT_TRANSMIT_WINDOW_OFFSET;
		else
			p_connection->transmit_window_offset = p_connection->connInterval;
	}

	// Step 6 - Latency

	p_connection->latency = connection_latency;

	// Step 7 - timeOut

	p_connection->timeout = supervision_timeout;

	// Step 8 - channelMap
	//
	// For now we assume that the same channel map is used for each link

	for (i=0;i<5;i++)
		p_connection->data_channel_map[i] = LE_config.data_channel_map[i];

	// Step 9 - hopIncrement
	/* The Hop field shall be set to indicate the hopIncrement used in the data
	 *  channel selection algorithm as defined in Section 4.5.8.2. It shall have a random
	 *  value in the range of 5 to 16.
	 */
#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==1)
	if (LE_config.preConfigured_hop_inc_available)
		p_connection->hop_increment = LE_config.preConfigured_hop_inc;
	else
		p_connection->hop_increment = ((SYSrand_Get_Rand() & 0x0000000A) | 0x04)+1;
#else
	p_connection->hop_increment = ((SYSrand_Get_Rand() & 0x0000000A) | 0x04)+1;
#endif

	// Step 10 - Sleep Clock Accuracy.
	//
	//
	p_connection->sleep_clock_accuracy = LE_config.sleep_clock_accuracy;
     // Min CE length are not used
	 // p_connection->minimum_CE_length = minimum_CE_length;

	p_connection->maximum_CE_length = maximum_CE_length;

	// The should only be done on the first link !!!
	// Seems pointless as - all I am doing is updating the channel remap with the
	// existing channel set !!!

	LEfreq_Update_Channel_Remapping_Table(p_connection, p_connection->data_channel_map);

	LE_config.next_initiator_scan_timer = BTtimer_Safe_Clockwrap(HW_Get_Native_Clk_Avoid_Race() + (connection_scan_interval*4));
	LE_config.initiating_enable = 0x01;

	return NO_ERROR;
}
#endif

/*********************************************************************
 * Function :- LEconnection_Encode_CONNECT_REQ_pdu
 *
 * Description
 * This function encode a CONNECT_REQ PDU and places it in the HW Tx Buffer.
 * As a CONNECT_REQ is the only valid transmission when a device is in the
 * initiating state.
 *
 * This function is called every time the device enters the initiating state.
 *
 * As most of the parameters of the PDUs are fixed this should not be problematic.
 *
 *  InitA          :- Fixed
 *  AdvA           :- Dynamic (updated on Rx of Advertising PDU )
 * 	Access Address :- Fixed
 *  CRC Init       :- Fixed
 *  Win Size       :- Dynamic - has to be rewritten on Rx of Advertising PDU
 *  Win Offset     :- Dynamic - has to be rewritten on Rx of Advertising PDU
 *  Interval       :- Fixed
 *  Latency        :- Fixed
 *  Timeout        :- Fixed
 *  Channel Map    :- Fixed
 *  Hop            :- Fixed
 *  SCA            :- Fixed
 *
 * In sets up the LE_Config states for the start of a scan window and sets a new interval
 * timer.
 *
 * CONTEXT :  IRQ
 *************************************************************************/

void LEconnection_Encode_CONNECT_REQ_pdu(void)
{
    if(PATCH_FUN[LECONNECTION_ENCODE_CONNECT_REQ_PDU_ID]){
         ((void (*)(void))PATCH_FUN[LECONNECTION_ENCODE_CONNECT_REQ_PDU_ID])();
         return ;
    }
	extern u_int32 u_int32_TX_buffer[];
	u_int8* p_payload = (u_int8*)u_int32_TX_buffer;
	u_int8 i;
	t_LE_Connection* p_connection = &LE_connections[LE_config.initiator_link_id];

	LE_LL_Encode_Own_Address_In_Payload(p_payload,LEconnection_Get_Own_Addr_Type(p_connection));

	p_payload+=12;

	LMutils_Set_Uint32(p_payload,p_connection->access_address );
	LMutils_Set_Uint32(p_payload+4,(p_connection->crc_init & 0x00FFFFFF ));

	p_payload[7] = p_connection->transmit_window_size;
	LMutils_Set_Uint16(p_payload+8,p_connection->transmit_window_offset);
	LMutils_Set_Uint16(p_payload+10,p_connection->connInterval );
	LMutils_Set_Uint16(p_payload+12,p_connection->latency );
	LMutils_Set_Uint16(p_payload+14,p_connection->timeout );
	for(i=0;i<5;i++)
		p_payload[16+i] = p_connection->data_channel_map[i];

	p_payload[21] =  p_connection->hop_increment + (LE_config.sleep_clock_accuracy<<5);

	LE_LL_Set_Adv_Header(0x05,LEconnection_Get_Own_Addr_Type(p_connection),0x0,(34));
	hw_memcpy32((void*)HWle_get_tx_acl_buf_addr(), (void*)u_int32_TX_buffer, 34);
}
/*********************************************************************
 * Function :- LEconnection_Initiator_Scan_Interval_Timeout
 *
 * Description
 * This function triggers an initiator scan window. It is invoked on expiry of the
 * initiator scan interval timer.
 *
 * In sets up the LE_Config states for the start of a scan window and sets a new interval
 * timer.
 *
 * CONTEXT :  IRQ
 *************************************************************************/
void LEconnection_Initiator_Scan_Interval_Timeout(u_int32 current_clk)
{
    if(PATCH_FUN[LECONNECTION_INITIATOR_SCAN_INTERVAL_TIMEOUT_ID]){
         ((void (*)(u_int32 ))PATCH_FUN[LECONNECTION_INITIATOR_SCAN_INTERVAL_TIMEOUT_ID])(current_clk);
         return ;
    }
	/*
	 * If there is a Connection Event Due - Prior to the end of the Initiator Scan
	 * then abort the SCAN.
	 */

	if (BTtimer_Is_Expired_For_Time(LE_config.next_connection_event_time,((current_clk+(LE_config.initiator_scan_window*2)+8)) & BT_CLOCK_MAX_TICKS))
	{
		// If the initiator Scan window conflicts with a connection interval then adjust by 32 ticks.
		LE_config.next_initiator_scan_timer = BTtimer_Safe_Clockwrap(LE_config.next_connection_event_time + 0x10);
		return; // Do nothing as Initiating Scan will prevent the connection event.
	}
	else
	{
		if (LE_config.initiating_enable==1)
		{
			LEscan_Set_Current_Window_Timer(BTtimer_Safe_Clockwrap(current_clk + (LE_config.initiator_scan_window*2)));
			LE_config.next_initiator_scan_timer = BTtimer_Safe_Clockwrap(current_clk + (LE_config.initiator_scan_interval*2));
		}
		else
		{
			LE_config.next_initiator_scan_timer = BT_TIMER_OFF;
			return;
		}
	}
	LEconnection_Encode_CONNECT_REQ_pdu();


	HW_set_slave(0x00);
    HWle_set_master_mode();

	HWle_set_le_mode();

	HWle_set_acc_addr(ADVERTISING_ACCESS_ADDRESS);
	*((u_int32 volatile*)0x4002c248) = (u_int32)(ADVERTISING_ACCESS_ADDRESS); // 0x8E89BED6 ((0x8e89b3d6); 
	*((u_int32 volatile*)0x4002c24c) = (u_int32)(0x0);  
	
	HWle_set_adv_state();

	// TIFS default is set for the entire scan window and does not need to be
	// re-written.

	HWle_set_tifs_default();
	// GF 30 March -- as the frequency is fixed for the entire width of the Scan window
	// the Scan channel can be determined here and the Whitening in HW can be initialised.

	LEscan_Advance_Initiating_Frequency();
	LEscan_Update_Initiator_Scan_Freq();
	// For SM need to set Win-Ext = 0

	LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM2_INTR_MSK_MASK);

	HWle_set_le_spi_only();
	//HWle_clear_tx_enable();
	//HWradio_LE_Setup_Radio_For_Next_TXRX(RX_START);
    ((void (*)(t_frame_pos const ))PATCH_FUN[HWRADIO_LE_SETUP_RADIO_FOR_NEXT_TXRX])(RX_START);

#if (PRH_SYS_CFG_HARDWARE_ADDRESS_FILTERING_SUPPORTED == 1)
	HWle_set_rx_mode(0x04 /* Initiating */);
	if (LE_config.initiator_filter_policy ==0x01)
		HWle_set_address_filtering();
	else
		HWle_clear_address_filtering();

#endif

	LE_config.state = INITIATING_STATE;
	LE_config.sub_state = W4_SCAN_RX;

	LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM2_INTR_MSK_MASK | JAL_TIM0_INTR_MSK_MASK);
	LEscan_Prep_For_LE_Scan_Rx();
	HWle_set_le_mode();


#if (PRH_BS_CFG_SYS_TRACE_LE_DEVICE_STATE_VIA_HCI_SUPPORTED == 1)
	TCeg_Send_LE_LEDeviceState_Event(LE_config.state,0);
#endif
}

/************************************************************************************
 * Function :- LEconnection_Create_Connection_Cancel
 *
 * Description :
 * This function is invoked on receipt of a HCI_LE_CREATE_CONNECTION_CANCEL. It is only relevant
 * while a device is still in the INITIATING state,
 *
 * NOTE :- The 'initiator_scan_interval_timer' is used to determine if the device is still in
 *         the initiating state, as a timer index will only exist while a device is initiating.
 *
 * CONTEXT : Schedular
 ***************************************************************************************/
#if (PRH_BS_CFG_SYS_LE_GAP_INCLUDED!=1)
t_error LEconnection_Create_Connection_Cancel(t_p_pdu p_pdu  )
{
    if(PATCH_FUN[LECONNECTION_CREATE_CONNECTION_CANCEL_ID]){
        return ((t_error (*)(t_p_pdu ))PATCH_FUN[LECONNECTION_CREATE_CONNECTION_CANCEL_ID])(p_pdu);
    }
    t_cmd_complete_event cmd_complete_info;

    cmd_complete_info.opcode = HCI_LE_CREATE_CONNECTION_CANCEL;
    cmd_complete_info.status = NO_ERROR;

	if (LE_config.next_initiator_scan_timer != BT_TIMER_OFF)
	{
		// Clear the initiators timer.
		LE_config.next_initiator_scan_timer = BT_TIMER_OFF;
		LE_config.initiating_enable = 0;
		// Free the allocated link_id.
		LEconfig_Free_Link_Id(&LE_connections[LE_config.initiator_link_id]);
		// Send Command_Complete -- this is unique as in other commands this is handled
		// by the hc_cmd_disp,c
		HCeg_Command_Complete_Event(&cmd_complete_info);
		//
		// Generate Connection Complete with error = UNKNOWN_CONNECTION
		// Extract from spec..
		/* The LE Connection Complete event with the error code UNKNOWN_CONNECTION
		 * Identifier (0x02) shall be sent after the Command Complete event for the
		 * LE_Create_Connection_Cancel command if the cancellation was successful.
		 */

		LEeventgen_Generate_LE_Meta_Event(LE_SUBEVENT_CONNECTION_COMPLETE,&LE_connections[LE_config.initiator_link_id],NO_CONNECTION);
	}
	else
	{
		cmd_complete_info.status = COMMAND_DISALLOWED;
		HCeg_Command_Complete_Event(&cmd_complete_info);
		return COMMAND_DISALLOWED;
	}
	return NO_ERROR;

}
#endif

/****************************************************************************
 * Function :- LEconnection_HCI_Connection_Update
 *
 * Description :
 * Handles a CONNECTION_UPDATE request from the HCI. The HCI Command parameters are 
 * decoded and range checked. These are then copied into the link container, the
 * transmit window offset and size are determined and a Connection_Update instant is 
 * calculated. 
 * Finally a LL_CONNECTION_UPDATE_REQ is sent to the peer.
 *
 *****************************************************************************/
t_error LEconnection_HCI_Connection_Update(t_p_pdu p_pdu)
{
    if(PATCH_FUN[LECONNECTION_HCI_CONNECTION_UPDATE_ID]){
       return ((t_error (*)(t_p_pdu ))PATCH_FUN[LECONNECTION_HCI_CONNECTION_UPDATE_ID])(p_pdu);
    }
	u_int16 connection_handle;
	u_int16 connection_interval_max;
	u_int16 connection_interval_min;
	u_int16 connection_latency;
	u_int16 supervision_timeout;
	u_int16 minimum_CE_length;
	u_int16 maximum_CE_length;     
	t_LE_Connection* p_connection;

	u_int8 link_id;

	connection_handle =  HCIparam_Get_Uint16(p_pdu);
	p_pdu+=2;

	connection_interval_min = HCIparam_Get_Uint16(p_pdu);
	p_pdu+=2;

	connection_interval_max = HCIparam_Get_Uint16(p_pdu);
	p_pdu+=2;

	connection_latency = HCIparam_Get_Uint16(p_pdu);
	p_pdu+=2;

	supervision_timeout = HCIparam_Get_Uint16(p_pdu);
	p_pdu+=2;

	minimum_CE_length = HCIparam_Get_Uint16(p_pdu);
	p_pdu+=2;

	maximum_CE_length = HCIparam_Get_Uint16(p_pdu);
	p_pdu+=2;

	if ((connection_interval_min < 0x06) || (connection_interval_min > 0x0C80))
		return INVALID_HCI_PARAMETERS;

	if ((connection_interval_max < 0x06) || (connection_interval_max > 0x0C80))
		return INVALID_HCI_PARAMETERS;

	if (connection_interval_min > connection_interval_max)
		return INVALID_HCI_PARAMETERS;

	if (connection_latency > 0x01F3)
		return INVALID_HCI_PARAMETERS;

#if 0 // GF 29th May -- Below causes problems for conformnace
	/* The connSupervisionTimeout shall be a multiple of 10 ms in the range of 100 ms to 32.0 s and it shall be larger than
     * it shall be larger than (1 + connSlaveLatency) * connInterval.
     */
	if ((supervision_timeout*8) < ((1 + connection_latency) * connection_interval_max))
		return INVALID_HCI_PARAMETERS;
#endif

	if ((supervision_timeout < 0x0A) || (supervision_timeout > 0x0C80))
		return INVALID_HCI_PARAMETERS;


	if (NO_ERROR != LEconnection_Find_Link_Id(connection_handle,&link_id))
		return NO_CONNECTION;

	p_connection = LEconnection_Find_Link_Entry(link_id);

	if(LEconnection_Get_Role(p_connection) == SLAVE)
		return COMMAND_DISALLOWED;

	// TO DO :- For now always go with the Max connection interval. As more logic is added to
	// support multipoint we will enhance this.
	p_connection->connInterval_new = connection_interval_max;
	p_connection->latency_new = connection_latency;
	p_connection->timeout_new = supervision_timeout;


#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==1)
	if (LE_config.TCI_transmit_win_config)
	{
		p_connection->transmit_window_size_new = LE_config.transmit_win_size;
		p_connection->transmit_window_offset_new = LE_config.transmit_win_offset;
	}
	else
#endif
	{
	if (p_connection->connInterval > 8)
		p_connection->transmit_window_size_new = PRH_BS_CFG_DEFAULT_TRANSMIT_WINDOW_SIZE; // Transmit Window of 5 ms
	else
		p_connection->transmit_window_size_new = 2;

	if (p_connection->connInterval > PRH_BS_CFG_DEFAULT_TRANSMIT_WINDOW_OFFSET)
		p_connection->transmit_window_offset_new = PRH_BS_CFG_DEFAULT_TRANSMIT_WINDOW_OFFSET;
	else
		p_connection->transmit_window_offset_new = p_connection->connInterval;

	}
	// The connection_update instant should be 10 * ConnInterval in the future.
	// So we set the instant as CurrentConnEventCount + 10

	p_connection->connection_update_instant = p_connection->connection_event_counter + 10;
	p_connection->ll_action_pending |= LL_CONNECTION_UPDATE_PENDING;

	return LEllc_Encode_PDU( LL_CONNECTION_UPDATE_REQ, p_connection, 0x00);

}
#endif

#if (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
/**********************************************************************************
 * Function :- LE_Connection_Extact_Connection_Req_Parameters
 *
 * Description :
 * This function extracts the parameters from a CONNECT_REQ and copies them into the
 * container for the relevant LE link.
 *
 * CONTEXT : IRQ
 ***********************************************************************************/
u_int8 LE_Connection_Extact_Connection_Req_Parameters(t_LE_Connection* p_connection,t_p_pdu p_payload)
{
     if(PATCH_FUN[LE_CONNECTION_EXTACT_CONNECTION_REQ_PARAMETERS_ID]){
         
         return ((u_int8 (*)(t_LE_Connection* p_connection,t_p_pdu p_payload))PATCH_FUN[LE_CONNECTION_EXTACT_CONNECTION_REQ_PARAMETERS_ID])(p_connection,  p_payload);
     }

	// TO DO - Must validate the parameters of an incoming CONNECT_REQ

	/*
	 * CONNECT_REQ - LLData - Contains the following Fields
	 *    AA (4 octets), CRCInit (3 octets), WinSize (1 octet),WinOffset (2 octets) Interval (2 octets)
	 *    Latency (2 octets) Timeout (2 octets) ChM (5 octets) Hop (5 bits) SCA (3 bits)
	 */

	u_int8 i;

	p_connection->access_address = LMutils_Get_Uint32(p_payload);
	p_connection->crc_init =  HCIparam_Get_Uint24(p_payload+4);
	p_connection->transmit_window_size =  *(p_payload+7);
	p_connection->transmit_window_offset = LMutils_Get_Uint16(p_payload+8);
	p_connection->connInterval =  LMutils_Get_Uint16(p_payload+10);
	p_connection->latency = LMutils_Get_Uint16(p_payload+12);
	p_connection->timeout = LMutils_Get_Uint16(p_payload+14);
    if ((p_connection->connInterval < 6) ||  (p_connection->access_address == 0xffffffff))
    {
          wrong_connect_req_pkt = 1;
	}
	for (i=0;i<5;i++)
		p_connection->data_channel_map[i] = p_payload[16+i];
	{
		u_int8 hop_increment;

		hop_increment =  ((*(p_payload+21)) & 0x1F);
		if ((hop_increment > 16) || (hop_increment <5))
			return 0;

		p_connection->hop_increment = hop_increment;
	}
	p_connection->sleep_clock_accuracy = (((*(p_payload+22))& 0xE0)>>5)&0x7;

	return 1;

}
#endif

/********************************************************************************************
 * Function :- LEconnection_Is_Valid_Handle
 *
 * Description :
 * Determines if a given handle is a valid handle for an Active LE link.
 ********************************************************************************************/
u_int8 LEconnection_Is_Valid_Handle(u_int16 handle)
{
	if ((handle < PRH_BS_CFG_LE_CONN_HANDLE_OFFSET) || ((handle - PRH_BS_CFG_LE_CONN_HANDLE_OFFSET) > (DEFAULT_MAX_NUM_LE_LINKS-1))
	)
	{
		return 0;
	}
	else
	{
		return (LEconnections_Link_In_Use(handle - PRH_BS_CFG_LE_CONN_HANDLE_OFFSET));
	}
}

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1)
/********************************************************************************************
 * Function :- LEconnection_Read_Remote_Used_Features
 *
 * Description :
 * Handles a HCI Command to read the remote used features of a peer device.
 ********************************************************************************************/

t_error LEconnection_Read_Remote_Used_Features(u_int16 handle)
{
    if(PATCH_FUN[LECONNECTION_READ_REMOTE_USED_FEATURES_ID]){
         
         return ((t_error (*)(u_int16 ))PATCH_FUN[LECONNECTION_READ_REMOTE_USED_FEATURES_ID])(handle);
     }
	t_error status = NO_ERROR;

	t_LE_Connection* p_connection =  LEconnection_Find_P_Connection(handle);

	if ((p_connection == 0x0) || (p_connection->active==0x00))
		status = NO_CONNECTION;
	else if (LEconnection_Get_Role(p_connection) == SLAVE )
		status = COMMAND_DISALLOWED;

	if (p_connection->ll_action_pending)
		status = COMMAND_DISALLOWED;

	if (status == NO_ERROR)
	{
		status = LEllc_Encode_PDU(LL_FEATURE_REQ,p_connection,0x00);
		if (status == NO_ERROR)
		{
			p_connection->ll_action_pending |= LL_FEATURE_REQ_PENDING;
			/*
			 * Set timer while waiting for response to ll_version_ind
			 */
			p_connection->ll_response_timer = BTtimer_Set_Ticks(LE_LL_RESPONSE_TIMEOUT);
		}
	}
	return status;
}
#endif


/********************************************************************************************
 * Function :- LEconnection_Read_Remote_Version_Info
 *
 * Description :
 * Handles a HCI Command to read the remote version of a peer device.
 ********************************************************************************************/

t_error LEconnection_Read_Remote_Version_Info(u_int16 handle)
{
	if(PATCH_FUN[LECONNECTION_READ_REMOTE_VERSION_INFO_ID]){
         
         return ((t_error (*)(u_int16 handle))PATCH_FUN[LECONNECTION_READ_REMOTE_VERSION_INFO_ID])(handle);
    }

	t_error status = NO_ERROR;
	t_LE_Connection* p_connection =  LEconnection_Find_P_Connection(handle);

	if (p_connection == 0x0)
		status = NO_CONNECTION;

	/* Need to check that an LLC procedure is not alreay taking place
	 * on this link.
	 * "Procedures have specific timeout rules as defined in Section 5.2. The Termination
	 * Procedure may be initiated at any time, even if any other Link Layer Control
	 * Procedure is currently active. For all other Link Layer Control Procedures,
	 * only one Link Layer Control Procedure shall be initiated in the Link Layer at a
	 * time per connection per device. A new Link Layer Control Procedure can be
	 * initiated only after a previous Link Layer Control Procedure has completed.
	 */

	if (p_connection->ll_action_pending)
		status = COMMAND_DISALLOWED;

	if ((status == NO_ERROR) && (!(p_connection->version_ind_status & LL_VERSION_IND_SENT )))
	{
		status = LEllc_Encode_PDU(LL_VERSION_IND,p_connection, 0x00);
		if (status == NO_ERROR)
		{
			p_connection->ll_action_pending |= LL_VERSION_IND_PENDING;
			/*
			 * Set timer while waiting for response to ll_version_ind
			 */
			p_connection->ll_response_timer = BTtimer_Set_Ticks(LE_LL_RESPONSE_TIMEOUT);
		}
	}
	else if (status == NO_ERROR)
	{
		// Special Error code re-used to indicate that the Version Ind was previously sent in this
		// connection. So should be read from local cached information.
		status = REPEATED_ATTEMPTS;
	}
	return status;
}



/********************************************************************************************
 * Function :- LEllc_Encode_PDU
 *
 * Description :
 * This function encode the LLC PDUs and places them on the LLC/LMP outgoing queues for transmission
 * on the air.
 ********************************************************************************************/

t_error LEllc_Encode_PDU(u_int8 opcode,t_LE_Connection* p_connection, u_int8 param1)
{
	 
    if(PATCH_FUN[LELLC_ENCODE_PDU_ID]){
         
         return ((t_error (*)(u_int8 opcode,t_LE_Connection* p_connection, u_int8 param1))PATCH_FUN[LELLC_ENCODE_PDU_ID])(opcode, p_connection, param1);
    }

	// Max Size of a LLC PDU is 23 Bytes including opcode.
    t_q_descr *qd = 0;
    u_int32 u_int32_temp_buffer[12] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,};
	u_int8* payload = (u_int8*)u_int32_temp_buffer;
	u_int8* p_pdu = payload;
	u_int8 i;
	u_int8 length = 0;
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
	u_int32 tx_pkt_count;
#endif
	if ((opcode > 0x0D) || (p_connection == 0x00))
		return UNSPECIFIED_ERROR;

	// Check if the link is active prior ot Queueing
	if (!LEconnection_Is_Active(p_connection))
	{
		return UNSPECIFIED_ERROR;
	}

	p_pdu[0] = opcode;
	p_pdu++;

	switch(opcode)
	{
#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1)
	case LL_CONNECTION_UPDATE_REQ:
		// 11 Bytes of Payload.
		length = 12;

		LMutils_Set_Uint16(p_pdu,p_connection->transmit_window_size_new);
		p_pdu+=1;
		LMutils_Set_Uint16(p_pdu,p_connection->transmit_window_offset_new);
		p_pdu+=2;
		LMutils_Set_Uint16(p_pdu,p_connection->connInterval_new);
		p_pdu+=2;
		LMutils_Set_Uint16(p_pdu,p_connection->latency_new);
		p_pdu+=2;
		LMutils_Set_Uint16(p_pdu,p_connection->timeout_new);
		p_pdu+=2;
		LMutils_Set_Uint16(p_pdu,p_connection->connection_update_instant);

		break;

	case LL_CHANNEL_MAP_REQ :
		// 7 Bytes of Payload.
		length = 8;

		for (i=0;i < 5;i++)
			p_pdu[i] = p_connection->channel_map_new[i];
		p_pdu+=5;

		LMutils_Set_Uint16(p_pdu,p_connection->channel_map_update_instant);
		break;
#endif
	case LL_TERMINATE_IND :
		// 1 Byte of Payload.
		length = 2;
		p_pdu[0] = param1;
		// Only set action pending to terminate on Receipt of the ACK to the LL_TERMINATE

		// Shouldn't the terminate reason for a local disconnect be set differently
		// INVESTIGATE Further - what goes in the local HCI_Disconnect_Complete should
		// be different from what is sent on the air.
		p_connection->terminate_reason = param1;
		p_connection->terminate_timer = BTtimer_Safe_Clockwrap(HW_Get_Native_Clk_Avoid_Race() + ((p_connection->timeout*0x10)*2));

		break;

#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
	case LL_ENC_REQ :
		// 22 Bytes of Payload
		length = 23;
		LMutils_Array_Copy(8, p_pdu, LE_config.random_number);
		p_pdu+=8;
		LMutils_Set_Uint16(p_pdu,LE_config.encrypt_diversifier);
		p_pdu+=2;
		LMutils_Array_Copy(8, p_pdu, LE_config.SKD);
		p_pdu+=8;
		LMutils_Array_Copy(4, p_pdu, p_connection->IV);
		break;


	case LL_ENC_RSP :
		// 12 Bytes of Payload
		length = 13;
		LMutils_Array_Copy(8, p_pdu, LE_config.SKD+8);
		p_pdu+=8;
		LMutils_Array_Copy(4, p_pdu, p_connection->IV+4);
		break;
#endif

	case LL_UNKNOWN_RSP :
		// 1 Byte of Payload
		length = 2;
		p_pdu[0] = param1;
		break;

	case LL_FEATURE_REQ :
	case LL_FEATURE_RSP :
		{
			const u_int8* p_features = LEconfig_Get_LE_Features_Ref();
			// 8 Bytes of Payload
			length = 9;
			// For now the features are hardcoded.
			// As Encryption is not yet supported all the feature bytes are zero.

			for (i=0;i<8;i++)
			{
				p_pdu[i] = p_features[i];
			}
		}
		break;

	case LL_START_ENC_REQ :
	case LL_START_ENC_RSP :
	case LL_PAUSE_ENC_REQ :
	case LL_PAUSE_ENC_RSP :
		// 0 Bytes of Payload
		length = 1;
		break;

	case LL_VERSION_IND :
		// 5 Bytes of Payload
		p_connection->version_ind_status |= LL_VERSION_IND_SENT;
		length = 6;
		{
			const t_versionInfo* p_version_info = SYSconfig_Get_Version_Info_Ref();

			// Explicitly Set the LMP_Version to BT 4.0
			p_pdu[0] = 0x06 ; // p_version_info->lmp_version;
			p_pdu++;
			LMutils_Set_Uint16(p_pdu,p_version_info->comp_id);
			p_pdu+=2;
			LMutils_Set_Uint16(p_pdu,p_version_info->lmp_subversion);
		}
		break;

	case LL_REJECT_IND :
		// 1 Byte of Payload
		length = 2;
		p_pdu[0] = param1;
		break;

	}

#if (PRH_BS_CFG_SYS_TRACE_LLC_VIA_HCI_SUPPORTED==1)
     /* LOG at this point before Encryption.
      * Log this message over the TCI if this feature is active
      */
    TCeg_Send_LE_LLC_Event(payload, (u_int8)length, 0 /*Transmitted*/);
#endif

#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
    {

        tx_pkt_count = p_connection->enc_tx_pkt_counter;

    	if(LEconnection_Encrypt_Payload(p_connection,payload,length,0x03,tx_pkt_count))
    	{
    		length += 4;
    	}
    }
#endif

    /*
     * Request and validate queue space for this LE_LLC message
     */

	qd = BTq_Enqueue(LMP_OUT_Q, p_connection->device_index, length);
    if (!qd)
    {
#if (PRH_BS_CFG_UART_FOR_SENSOR_ONLY==0)
        /*
         * Out of queue space
         */

		HCeg_Hardware_Error_Event(PRH_BS_HW_ERROR_OUT_OF_LE_LLC_QUEUE_BUFFERS);
#endif
        return MEMORY_FULL;
    }
    else if (!qd->data)
    {
#if (PRH_BS_CFG_UART_FOR_SENSOR_ONLY==0)
        /*
         * Should never occur, queue buffers have been corrupted
         */
		HCeg_Hardware_Error_Event(PRH_BS_HW_ERROR_LE_LLC_QUEUE_CORRUPTED);
#endif
        return UNSPECIFIED_ERROR;
    }

    /*
     * Encode the LE_LLC message opcode
     */
    qd->message_type = 0x03;
    qd->device_index = p_connection->device_index;
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
	if(p_connection->current_security_state & LE_ENCRYPTION_ACTIVE)
	{
		qd->enc_pkt_count = tx_pkt_count;
		p_connection->enc_tx_pkt_counter++;
	}
#endif
    qd->opcode_pending = opcode;

    /*
     * Presented with fully encoded PDU then encode directly
     */

    LMutils_Array_Copy(length, qd->data,payload);

     /*
      * If Direct Baseband Copy (Big Endian) Then Change Data to Big Endian
      */
   // BTq_Change_Endianness_If_Required(qd->data, qd->length);

    BTq_Commit(LMP_OUT_Q,p_connection->device_index);
	return NO_ERROR;
}
/***********************************************************************
 * Function :- LEconnection_Handle_Connection_Event
 *
 * Description
 * This function handles the HW configuration for a connection event (CE).
 *
 * Sets the following in HW :
 *
 * 				TIFS Default = 0 
 * 				Master Mode = ( Master = 1 / Slave = 0 )
 * 				Tx_Enable = ( Mqaster = 1 / Slave = 0 )
 * 				Adv State = 0
 *				CRC Init = Connection specific
 *				Access Address = connection specific
 *				HW LE Mode = 1
 *
 * Following States/Flags are set in SW
 * 				device state = CONNECTION_STATE;
 *		        connection sub_state =  W4_T0_PRE_MASTER_FIRST_CONN_TX | W4_T0_PRE_SLAVE_CONN_FIRST_RX
 **************************************************************************/

void LEconnection_Handle_Connection_Event(u_int8 link_id,u_int32 current_clk)
{
	
	if(PATCH_FUN[LECONNECTION_HANDLE_CONNECTION_EVENT_ID]){
         ((void (*)(u_int8 link_id,u_int32 current_clk))PATCH_FUN[LECONNECTION_HANDLE_CONNECTION_EVENT_ID])(link_id,  current_clk);
         return ;
    }

	/*
	 * First check if Supervision TO has already expired. Here we do not strictly follow the LE Spec, as the Spec
	 * is not very sensible. For an efficient/low power implementation the supervision TO should be N * connInterval,
	 * However the BT LE Spec does not impose any restrictions on the Supervision TO. In fact the Test Spec States
	 * "Upper Tester expects an HCI_Disconnetion_Complete event from the IUT, indicating a connection supervision timeout
	 * and containing the connection handle used, after a time equal to the connection supervision timeout selected from
	 * the last event transmitted"
	 *
	 * In addition it States "
	 * "Pass verdict:
	 * The IUT produces the connection termination HCI event after the selected timeout values for the connSupervisionTimeout
	 * from the event."
	 *
	 * THIS DOES NOT MAKE SENSE.. If a device has a connInterval of 100ms and a SupervisionTO of 450ms, After 4 CE's of not
	 * recieving it knows it is going to Timeout - it makes no sense to wait an extra 50ms before you send the HCI_Disconnect_Complete
	 * event to the HCI. Why would a device stay alive/active for an extra 50ms, in the CE event handling the device knows if the timeout
	 * will expire before the next CE event - so why cant it report this to the host....
	 *
	 * Either the BT Core spec should change to indicate that SupervisionTO = N * connInterval OR the Test Spec for the LLC should
	 * have a looser interpretation of when it can recieve the HCI_Disconnection_Complete Event.
	 *
	 * So we have assumed here that the HCI_Disconnection_Complete event will be generated. ERRATA to be submitted.
	 */
	t_LE_Connection* p_connection  = LEconnection_Find_Link_Entry(link_id);

	// Optional Extra -
	// Check if we have missed the Frame for the Connection Event..

	if (BTtimer_Is_Expired_For_Time(p_connection->next_connect_event_start_time, current_clk - 4))
	{
		u_int16 events_missed;
		u_int16 i;

		// We increase Next connection Event Time by N * Interval
		// Also we need to increase the Connection Event Counter for each missed connection
		// event.

		// First find the number of Events Missed
        events_missed = (BTtimer_Safe_Clockwrap(current_clk - 1 - p_connection->next_connect_event_start_time + (p_connection->connInterval*4))/(p_connection->connInterval*4));
        p_connection->next_connect_event_start_time += events_missed * (p_connection->connInterval*4);
        p_connection->next_connect_event_start_time &= BT_CLOCK_MAX_TICKS;

        p_connection->connection_event_counter += events_missed;

        // Need to reconsider the below for MultiPoint.
        LE_config.next_connection_event_time = p_connection->next_connect_event_start_time;

        // Need to Advance the Frequency
        for (i=0;i<(events_missed);i++)
        {
        	LEfreq_Get_Next_Data_Channel_Frequency(p_connection);
        }

        // In this case we need to force a switch back to classic
    	if (p_connection->role == MASTER)
    	{
    		LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM0_INTR_MSK_MASK |JAL_TIM2_INTR_MSK_MASK | JAL_TIM1_INTR_MSK_MASK |JAL_TIM3_INTR_MSK_MASK );
#ifndef BLUETOOTH_MODE_LE_ONLY
    		LEconfig_Switch_To_BT_Classic();
#endif
    	}
        return;
	}
    #if (__DEBUG_MODEM_LOWPOWER_ENABLE == 1) || (__DEBUG_XINCHIP_RF_LOWPOWER_ENABLE)  
    if ( 1 == g_debug_xinchip_rf_lowpower_enable)
    {    
        __write_hw_reg32(CPR_BT_MODEM_CLK_CTL, BLE_MODEM_CLK_ON);
    }
    #endif


	p_connection->current_data_channel = LEfreq_Get_Next_Data_Channel_Frequency(p_connection);
	p_connection->connection_event_rf_channel = LEfreq_Map_Channel_Index_2_RF_Freq(p_connection->current_data_channel);

	HWle_set_crc_init(p_connection->crc_init);
	HWle_set_acc_addr(p_connection->access_address);
	HWle_clear_adv_state();
	HWle_set_whitening_init(p_connection->current_data_channel);
	HWle_set_le_mode();
	HWle_abort_tifs_count();
#if (PRH_SYS_CFG_HARDWARE_ADDRESS_FILTERING_SUPPORTED == 1)
	HWle_set_rx_mode(0x05 /* Connection */);
#endif
#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1)
	if (LEconnection_Get_Role(p_connection) == MASTER)
	{

		HWle_clear_tifs_default();
		HWle_set_master_mode();
#if (PRH_BS_CFG_LE_FIX_PLL_ISSUE==1)
		LEconnection_Set_Substate(p_connection, W4_T0_PRE_MASTER_FIRST_CONN_TX);
#else
		if (BTtimer_Is_Expired_For_Time(p_connection->next_connect_event_start_time,current_clk))
		{
		    HWle_set_le_spi_only();
			//HWradio_Program_Dummy_Rx(p_connection->connection_event_rf_channel);
			((void (*)(uint8_t c))PATCH_FUN[HWRADIO_PROGRAM_DUMMY_RX])(p_connection->connection_event_rf_channel);
            ((void (*)(t_frame_pos const ))PATCH_FUN[HWRADIO_LE_SETUP_RADIO_FOR_NEXT_TXRX])(RX_START);
			//HWradio_LE_Setup_Radio_For_Next_TXRX(RX_START);

			HWle_abort_tifs_count();
			HWle_set_tifs_default();
			HWle_set_tx_enable();
			HW_set_rx_mode(0x00);
			HWle_set_rx_mode(0x00);
#ifndef BLUETOOTH_MODE_LE_ONLY//no pta interface
			HWpta_LE_Access_Request(TRUE, 0, RADIO_MODE_LE_TIFS_TX_RX);
#endif
			//HWradio_LE_Service(RADIO_MODE_LE_INITIAL_TX,p_connection->connection_event_rf_channel,TX_START_FREQ);
              ((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_LE_INITIAL_TX,p_connection->connection_event_rf_channel,TX_START_FREQ);
			_LEconnection_Encode_DataPDU(p_connection);
			LEconnection_Set_Next_Master_Connection_Event_Time(p_connection,(current_clk));

			LEconnection_Set_Substate(p_connection, W4_MASTER_CONN_FIRST_TX);
			LSLCirq_Disable_All_Tim_Intr_Except(0);
			LE_config.state = CONNECTION_STATE;

		}

#endif

	}
#endif
#if (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
	if (LEconnection_Get_Role(p_connection) == SLAVE)
	{
		HWle_set_tifs_default();
		HWle_clear_master_mode();
#if (PRH_BS_CFG_SYS_LE_DUAL_MODE == 1)
		HW_set_slave(0x01);
#endif
		HWle_clear_tx_enable();
		LEconnection_Set_Substate(p_connection, W4_T0_PRE_SLAVE_CONN_FIRST_RX);
	}
	/*intraslot_settled = 0;*/
//	HWradio_LE_Set_Active_Mode(0x01);
    (( void (*)(u_int8 mode))PATCH_FUN[HWRADIO_LE_SET_ACTIVE_MODE])(0x01);
	LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM0_INTR_MSK_MASK);
	LE_config.state = CONNECTION_STATE;
#endif

#if (PRH_BS_CFG_SYS_TRACE_LE_DEVICE_STATE_VIA_HCI_SUPPORTED == 1)
	TCeg_Send_LE_LEDeviceState_Event(LE_config.state,p_connection);
#endif
}

/***********************************************************************
 * Function :- LEconnection_Handle_Connection_Event
 *
 * Description
 * This function handles the HW configuration for a connection event (CE).
 *
 * Sets the following in HW :
 *
 * 				TIFS Default = 0 
 * 				Master Mode = ( Master = 1 / Slave = 0 )
 * 				Tx_Enable = ( Mqaster = 1 / Slave = 0 )
 * 				Adv State = 0
 *				CRC Init = Connection specific
 *				Access Address = connection specific
 *				HW LE Mode = 1
 *
 * Following States/Flags are set in SW
 * 				device state = CONNECTION_STATE;
 *		        connection sub_state =  W4_T0_PRE_MASTER_FIRST_CONN_TX | W4_T0_PRE_SLAVE_CONN_FIRST_RX
 **************************************************************************/
//extern volatile uint8_t __attribute__((aligned(4))) DMA_buf[4];
t_error LE_LL_Ctrl_Handle_Connection_Event(u_int8 IRQ_Type,u_int32 current_clk)
{
	if(PATCH_FUN[LE_LL_CTRL_HANDLE_CONNECTION_EVENT_ID]){
         
         return ((t_error (*)(u_int8 IRQ_Type,u_int32 current_clk))PATCH_FUN[LE_LL_CTRL_HANDLE_CONNECTION_EVENT_ID])(IRQ_Type,  current_clk);
    }

	u_int8 active_link = LE_config.active_link;
	u_int8 crc_error;
	u_int8 back2back_crc_detected=0;
	
	t_LE_Connection* p_connection  = LEconnection_Find_Link_Entry(active_link);

	
#if (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
	if (LEconnection_Get_Role(p_connection) == SLAVE)
	{
		switch(LEconnection_Get_Substate(p_connection))
		{		  
			case W4_T0_PRE_SLAVE_CONN_FIRST_RX  :
				// Slave Device is waiting for First Rx from the Master.


				if (IRQ_Type==LE_TIM_0)
				{
					// Set up the next scanning Rx frequency,
					//
					HWle_set_le_spi_only();
					((void (*)(t_frame_pos const ))PATCH_FUN[HWRADIO_LE_SETUP_RADIO_FOR_NEXT_TXRX])(RX_START);
					//HWradio_LE_Setup_Radio_For_Next_TXRX(RX_START);
					if (LEconnection_Get_Initial_Anchor_Point_Obtained(p_connection)==0)
					{
						p_connection->transmit_window_remaining = p_connection->transmit_window_size+1; // + 1;

						//
						// In this case I need to go full RX Mode. The Search Window is opened on the next Tim1.
						// On each subsequent Tim2 we check the 'transmit_window_remaining' to see if we should
						// close the search window, and place the radio back in standby mode.
                        ((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_LE_FULL_RX,p_connection->connection_event_rf_channel,RX_START_FREQ);
						//HWradio_LE_Service(RADIO_MODE_LE_FULL_RX,p_connection->connection_event_rf_channel,RX_START_FREQ);
						LEconnection_Set_Next_Slave_Connection_Event_Time(p_connection,current_clk);
					}
					else
					{
#if (PRH_BS_CFG_SYS_HW_WINDOW_WIDENING_SUPPORTED==1)
						u_int16 win_us = _LEconnection_Determine_Window_Widening(p_connection);
#endif
						if(1 == have_get_no_crc_pkt)	{
						p_connection->transmit_window_remaining = _LEconnection_Determine_Window_Widening_In_Frames(p_connection);
						}
						else	{
						p_connection->transmit_window_remaining = p_connection->transmit_window_size ; // +(g_debug_le_sleep? 0:1) //  + 1;
						}
						if((0 == g_debug_le_sleep)&&(0xff == continue_miss_packet_number))
						{
							p_connection->transmit_window_remaining += transmit_window_remaining_val;
						}
#if 1 // BELOW DOES NOT LOOK CORRECT FOR LATENCY DURING CONNECTION UPDATE - BUT WORKs
      // Must investigate further.....

						if (p_connection->latency_count == 0)
						{
							p_connection->next_connect_event_start_time = (current_clk-4)+ (p_connection->connInterval*4);
							#if 0
                            if ((continue_miss_packet_number >3) && (0xff != continue_miss_packet_number))
                            {
                                continue_miss_packet_number = 0xff;
                                update_transmit_window_num = 1;
                                
                                p_connection->next_connect_event_start_time -= 5;
                                
                            }
                            if ((update_transmit_window_num == 1) && (0 == g_debug_le_sleep))
                            {
                                p_connection->transmit_window_remaining += 3;
                            }                            
							#endif
							
							p_connection->next_connect_event_start_time = LEconnection_Adjust_Clock(p_connection->next_connect_event_start_time,g_LE_Slave_IntraSlot);

						}
						else
							p_connection->next_connect_event_start_time += (p_connection->connInterval*4);

						if( (p_connection->ll_action_pending & LL_CONNECTION_UPDATE_PENDING) &&
							   ((p_connection->connection_event_counter+1)== p_connection->connection_update_instant))
						{

							p_connection->next_connect_event_start_time += (p_connection->transmit_window_offset_new *4)/*+ 4*/; // to account for the T.W.O + 1.25ms in spec;

						}
						p_connection->next_connect_event_start_time = BTtimer_Safe_Clockwrap(p_connection->next_connect_event_start_time);
						LE_config.next_connection_event_time = p_connection->next_connect_event_start_time;
#endif

						if (p_connection->latency_count!=0)
							p_connection->latency_count--;
							
#if (PRH_BS_CFG_SYS_HW_WINDOW_WIDENING_SUPPORTED==1)
						//!!! to have spi of the tim_bit_cnt need to also clean gio_le_en!!!
					    HWle_set_tifs_abort();
						HWle_clear_le_spi_only();// intraslot based SPI
                        ((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_LE_INITIAL_RX,p_connection->connection_event_rf_channel,RX_START_FREQ);
						//HWradio_LE_Service(RADIO_MODE_LE_INITIAL_RX,p_connection->connection_event_rf_channel,RX_START_FREQ);

						HWle_clear_tifs_abort();
						//set window extension for this receive
						if (win_us>300)
							win_us=300;
						if (win_us>g_sys_config.win_ext+10)
							HW_set_win_ext(win_us-10);
						else
							HW_set_win_ext(g_sys_config.win_ext);
#else
						((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_LE_INITIAL_RX,p_connection->connection_event_rf_channel,RX_START_FREQ);
						//HWradio_LE_Service(RADIO_MODE_LE_INITIAL_RX,p_connection->connection_event_rf_channel,RX_START_FREQ);
#endif
					}
#ifndef BLUETOOTH_MODE_LE_ONLY//no pta interface
					HWpta_LE_Access_Request(TRUE, 0, RADIO_MODE_LE_TIFS_TX_RX);
#endif
					LEconnection_Set_First_Sync_In_Connection_Event(p_connection,0x00);

					LEconnection_Set_Substate(p_connection, W4_T1_SLAVE_CONN_FIRST_FULL_RX);
					LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM1_INTR_MSK_MASK);
				}
			break;

		case W4_T1_SLAVE_CONN_FIRST_FULL_RX :
			if (IRQ_Type == LE_TIM_1)
			{

				if ((LEconnection_Get_Initial_Anchor_Point_Obtained(p_connection)==0) ||
					(LE_config.Auto_Widen_MultiSlot) || (0 == have_get_no_crc_pkt))
				{
					HW_set_rx_mode(0x03);
					HWle_set_rx_mode(0x05 /* Connection */);
					LEconnection_Set_Substate(p_connection, W4_SLAVE_CONN_FIRST_RX);
					LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM2_INTR_MSK_MASK);
				}
				else
				{
					if( (p_connection->latency_count == 0) ||
						(!BTq_Is_Queue_Empty(L2CAP_OUT_Q,p_connection->device_index)) ||
						(!BTq_Is_Queue_Empty(LMP_OUT_Q,p_connection->device_index)))
					{
						HW_set_rx_mode(0x03);
						HWle_set_rx_mode(0x05 /* Connection */);
						LEconnection_Set_Substate(p_connection, W4_SLAVE_CONN_FIRST_RX);
						LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM2_INTR_MSK_MASK);
					}
					else // No need to listen on the link.
					{
						_LEconnection_Close_Connection_Event(p_connection,current_clk);
					}
				}
			}
			break;

		case W4_SLAVE_CONN_FIRST_RX :
			switch(IRQ_Type)
			{
			case LE_SYNC_DET :
				HW_set_rx_mode(0x01);
				__read_hw_reg32(((volatile unsigned *)(0x4002c000 + 0x300)), rssi_val);
				/*HW_set_rx_mode(0x01);*/
				LEconnection_Set_First_Sync_In_Connection_Event(p_connection, 1);
			//	HW_set_bt_clk_offset((p_connection->connection_event_counter*p_connection->connInterval)*4);
#if (PRH_BS_CFG_SYS_HW_WINDOW_WIDENING_SUPPORTED==1)
				//toggle into T_IFS
				((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_LE_INITIAL_RX,p_connection->connection_event_rf_channel,RX_START_FREQ);
				//HWradio_LE_Service(RADIO_MODE_LE_INITIAL_RX,p_connection->connection_event_rf_channel,RX_START_FREQ);
				HWle_set_le_spi_only();
				//set extension back to default
				HW_set_win_ext(g_sys_config.win_ext);
				((void (*)(t_frame_pos const ))PATCH_FUN[HWRADIO_LE_SETUP_RADIO_FOR_NEXT_TXRX])(TX_START);
				//HWradio_LE_Setup_Radio_For_Next_TXRX(TX_START);
#endif


#if (PRH_BS_CFG_SYS_LE_12MHZ_OPTIMISATION==0)
				//p_connection->rssi = HWradio_LE_Read_RSSI();
				p_connection->rssi = ((s_int8 (*)(void))PATCH_FUN[HWRADIO_LE_READ_RSSI])();
#endif
				break;

			case LE_TIM_2 :
             
				if (LEconnection_Get_First_Sync_In_Connection_Event(p_connection) == 0)
				{
					if (p_connection->transmit_window_remaining > 0 ) /* Transmit Window still active */

					{
						p_connection->transmit_window_remaining-=1;
					}
					else
					{	
						ext_rx_window_update(p_connection);
						
						if(continue_miss_packet_number!=0xff)
						{
							continue_miss_packet_number++;

                            if(continue_miss_packet_number > continue_miss_packet_number_val)
                            {
                                continue_miss_packet_number = 0xff;
                                //update_transmit_window_num = 1;
                                p_connection->next_connect_event_start_time -= next_connect_event_start_time_val; 
                                //p_connection->transmit_window_remaining+=3;
                            }
						}
						_LEconnection_Close_Connection_Event(p_connection,current_clk);
						if (ext_rx_window  < ext_rx_window_val)
						{
						}
						else
						{
						    Connection_Close_LowPower();   // if continue missed receive packet, then disable go to sleep at this interval

						}
					}
				}

				if ((p_connection->latency_count) && (LEconnection_Get_Initial_Anchor_Point_Obtained(p_connection)!=0))
				{
					_LEconnection_Close_Connection_Event(p_connection,current_clk);
				}
				break;

			case LE_PKD :
				ext_rx_window_init(p_connection);
				#if(__DEBUG_RX_DATA_DUMP_TO_SHRAM0 == 1)
				*((u_int32 volatile*)0x4002c274) &= ~(0x1<<3);
				#endif

				// If we are in the Transmit window RX OR Window Widened RX. We have to record
				// this point as our anchor point.
				// As the next Anchor point is setup at the start of the connection event,
				// it has to be adjusted here to account for first Rx.
				crc_error = LE_LL_Get_CRC_Err();


				// As this is the First Rx in a connection event the Previous CRC does not mater,
				// so we can set the Prev_CRC to the current CRC
				if (crc_error)  {
					/*crc_count++;*/
					LEconnection_Set_Prev_CRC(p_connection,1);
                    //if(have_get_no_crc_pkt == 0)    {
                    //    _LEconnection_Close_Connection_Event(p_connection,current_clk);
				    //    break;
                    //}
				}
				else    {
					have_get_no_crc_pkt = 1;
				}	
				continue_miss_packet_number = 0;
                //update_transmit_window_num = 0;
				after_rx_first_packet = 1;

				// As this is the First Rx from the master in the connection event.
				// I need to reset the Connection interval timers
				HW_set_freeze_bt_clk(0);
				LEconnection_Update_Next_Slave_Connection_Event_Time(p_connection,current_clk);


				LEconnection_Set_Initial_Anchor_Point_Obtained(p_connection,1);

#if 0 // GF 11 Nov - Removed for Conformance Testing
				// 04 Nov 2013 - Holgers suggestion of Capturing IntrSlot on PKD
				// Only Applicable to First Rx in a connection event
			//	if(LEconnection_Get_First_Sync_In_Connection_Event(p_connection))
				{
					/*p_connection->current_IntroSlotOffset = HW_Get_Intraslot_Avoid_Race();
					if(((p_connection->last_IntroSlotOffset!=0)) &&
						(p_connection->current_IntroSlotOffset>p_connection->last_IntroSlotOffset) && (p_connection->current_IntroSlotOffset-p_connection->last_IntroSlotOffset>0x08)||
						(p_connection->last_IntroSlotOffset>p_connection->current_IntroSlotOffset) && (p_connection->last_IntroSlotOffset-p_connection->current_IntroSlotOffset>0x08))
					{
						HW_set_intraslot_offset(p_connection->last_IntroSlotOffset);
						debug_io(3);
					}
					else
					{
						p_connection->last_IntroSlotOffset = p_connection->current_IntroSlotOffset;
						debug_io(1);
					}*/
					//SYSmmi_Display_Numeric(0xF000 + HW_Get_Intraslot_Avoid_Race());
					// Now I can Freeze the Clk - So intraslot is not effected by additional
					// Receives in the connection event.
					//HW_set_freeze_bit_cnt(1);
					//HW_set_freeze_bt_clk(0);

					#if (0 == 1)
                        bt_clk_no_freeze = HW_Get_Bt_Clk_Avoid_Race();
                        native_bt_clk_no_freeze = HW_Get_Native_Clk_Avoid_Race();
                    #endif
				}
#endif
		    	HW_set_rx_mode(0x00);
				//HWle_set_rx_mode(0x00);
		    	HWle_set_tx_enable();
				((void (*)(void))PATCH_FUN[HWRADIO_LE_RXCOMPLETE])();
				// HWradio_LE_RxComplete();
				((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_LE_TIFS_TX_RX,p_connection->connection_event_rf_channel,TX_START_FREQ);
				//HWradio_LE_Service(RADIO_MODE_LE_TIFS_TX_RX,p_connection->connection_event_rf_channel,TX_START_FREQ);
                
                if(_LEconnection_Decode_DataPDU(p_connection) == 1) {
					_LEconnection_Close_Connection_Event(p_connection, current_clk);
					break;
                }
                
				//- update for TP/CON/SLA/BV-11-C.
				if(p_connection->ll_action_pending & LL_TERMINATE_PENDING ) {
					_LEconnection_Close_Connection_Event(p_connection, 0);
					break;
				}	
			    _LEconnection_Encode_DataPDU(p_connection);

#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==1)
			    if (LE_config.slave_listen_outside_latency==0)
#endif
			    {
			    	/* Note Latency should only come into force once we receive an ACK from the master
			    	 * Basically I should not enforce latency if there is an ACL ACK pending in the slave.
			    	 */
			    	if (!LEconnection_Get_Ack_Pending(p_connection))
			    		p_connection->latency_count = p_connection->latency+1;
			    }

				p_connection->connection_event_of_last_rx = p_connection->connection_event_counter;

#if (PRH_BS_CFG_SYS_HW_WINDOW_WIDENING_SUPPORTED==1)
				// set extension back to default
				HW_set_win_ext(g_sys_config.win_ext);
#endif

				LEconnection_Set_Substate(p_connection, W4_SLAVE_CONN_TX);
				//LSLCirq_Disable_All_Tim_Intr_Except(0); // sometimes at this position, the TIM0 is delayed, cause the pka interrupt wil not come, this will cause error
				LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM3_INTR_MSK_MASK);
				connect_no_pka_count = 0;
				break;

			case LE_NO_PKD :
			    if (LEconnection_Get_Initial_Anchor_Point_Obtained(p_connection))
					LEconnection_Update_Next_Slave_Connection_Event_Time(p_connection,current_clk);

#if (PRH_BS_CFG_SYS_HW_WINDOW_WIDENING_SUPPORTED==1)
				HWle_set_le_spi_only();
				//toggle into T_IFS
				((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_LE_INITIAL_RX,p_connection->connection_event_rf_channel,RX_START_FREQ);
				//HWradio_LE_Service(RADIO_MODE_LE_INITIAL_RX,p_connection->connection_event_rf_channel,RX_START_FREQ);
				//set window extension back to default
				HW_set_win_ext(g_sys_config.win_ext);
#endif
			    _LEconnection_Close_Connection_Event(p_connection,current_clk);
			    Connection_Close_LowPower();
			    break;


			}
			break;

		case W4_SLAVE_CONN_TX :
			switch(IRQ_Type)
			{
			case LE_PKA :
			    HW_set_rx_mode(0x01);
				HWle_set_rx_mode(0x05 /* Connection */);
				if (1 /* Check conditions for continued slave Rx */)
					{
					//_LEconnection_Prep_For_LE_Slave_Rx(p_connection);
					((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_LE_TIFS_TX_RX,p_connection->connection_event_rf_channel,RX_START_FREQ);	
					//HWradio_LE_Service(RADIO_MODE_LE_TIFS_TX_RX,p_connection->connection_event_rf_channel,RX_START_FREQ);
					
					//- add 'call HWradio_LE_TxComplete()'
					((void (*)(void))PATCH_FUN[HWRADIO_LE_TXCOMPLETE])();
				    //HWradio_LE_TxComplete();
					//
					LEconnection_Set_Substate(p_connection, W4_SLAVE_CONN_RX);
					LSLCirq_Disable_All_Tim_Intr_Except(0);
				}
				else
				{
					_LEconnection_Close_Connection_Event(p_connection,current_clk);
				}
				connect_no_pka_count = 0;
				break;


				case LE_TIM_3:
					connect_no_pka_count++;
					if(connect_no_pka_count>3)
					{
						connect_no_pka_count = 0;
						//HWradio_LE_TxComplete();
						((void (*)(void))PATCH_FUN[HWRADIO_LE_TXCOMPLETE])();
						_LEconnection_Close_Connection_Event(p_connection,current_clk);		
					}
					
					break;	

			}
			break;

		case W4_SLAVE_CONN_RX :
			switch(IRQ_Type)
			{
			case LE_SYNC_DET :
				//p_connection->rssi = HWradio_LE_Read_RSSI();
			    p_connection->rssi = ((s_int8 (*)(void))PATCH_FUN[HWRADIO_LE_READ_RSSI])();
				break;

			case LE_PKD :

				crc_error = LE_LL_Get_CRC_Err();
				if (crc_error)
				{
					HW_set_rx_mode(0x00);
					//HWle_set_rx_mode(0x00);
					HWle_set_tifs_abort();
					HWle_clear_tifs_default();
					if (LEconnection_Get_Prev_CRC(p_connection))
					{
						// Need to close the connection event.
						back2back_crc_detected = 0x01;
					}
					else
					{
						LEconnection_Set_Prev_CRC(p_connection,1);
					}
				}
				if (!back2back_crc_detected)
				{
					HW_set_rx_mode(0x00);
					//HWle_set_rx_mode(0x00);
					((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_LE_TIFS_TX_RX,p_connection->connection_event_rf_channel,TX_START_FREQ);
					//HWradio_LE_Service(RADIO_MODE_LE_TIFS_TX_RX,p_connection->connection_event_rf_channel,TX_START_FREQ);
                    
					if(_LEconnection_Decode_DataPDU(p_connection) == 1) {
					    _LEconnection_Close_Connection_Event(p_connection, current_clk);
					    break;
					}					
					//- update for TP/CON/SLA/BV-11-C.
					if(p_connection->ll_action_pending & LL_TERMINATE_PENDING ) {
						_LEconnection_Close_Connection_Event(p_connection, 0);
						break;
					}						
					_LEconnection_Encode_DataPDU(p_connection);
					LEconnection_Set_Substate(p_connection, W4_SLAVE_CONN_TX);
					LSLCirq_Disable_All_Tim_Intr_Except(0);
				}
				else
				{
					_LEconnection_Close_Connection_Event(p_connection,current_clk);
				}
				break;

			case LE_NO_PKD :
				((void (*)(void))PATCH_FUN[HWRADIO_LE_RXCOMPLETE])();
			    //HWradio_LE_RxComplete();
				_LEconnection_Close_Connection_Event(p_connection,current_clk);
				break;
			}
		} // end of switch
	} //
#endif
#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1)
	if (LEconnection_Get_Role(p_connection) == MASTER)
	{
		switch(LEconnection_Get_Substate(p_connection))
		{
#if (PRH_BS_CFG_LE_FIX_PLL_ISSUE==1)
			case W4_T0_PRE_MASTER_FIRST_CONN_TX :
				//
				// THIS IS A SPECIAL CASE to overcome possible Radio Issue
				// Prior to the first Tx of an Advertising Packet we set up an Rx in the Radio
				// BUT maintain the RX_MODE = 1.
				// This should help overcome some PLL setup issues in the Radio.
				switch(IRQ_Type)
				{
				//
				case LE_TIM_0 :
					if (BTtimer_Is_Expired_For_Time(p_connection->next_connect_event_start_time,current_clk))
					{
					    HWle_set_le_spi_only();
						//HWradio_Program_Dummy_Rx(p_connection->connection_event_rf_channel);
			            ((void (*)(uint8_t c))PATCH_FUN[HWRADIO_PROGRAM_DUMMY_RX])(p_connection->connection_event_rf_channel);
						LEconnection_Set_Substate(p_connection, W4_T2_MASTER_CONN_FIRST_TX);

					}
					LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM2_INTR_MSK_MASK);
					break;

				}
				break;

		case W4_T2_MASTER_CONN_FIRST_TX :

			switch(IRQ_Type)
			{
			case LE_TIM_2 :
				// Set up the next  transmit frequency,
				{
                    ((void (*)(t_frame_pos const ))PATCH_FUN[HWRADIO_LE_SETUP_RADIO_FOR_NEXT_TXRX])(RX_START); 				    
					//HWradio_LE_Setup_Radio_For_Next_TXRX(RX_START);
					// Set up the next scanning Rx frequency,

					HWle_abort_tifs_count();
					HWle_set_tifs_default();
					HWle_set_tx_enable();
					HW_set_rx_mode(0x00);
					HWle_set_rx_mode(0x00);
#ifndef BLUETOOTH_MODE_LE_ONLY//no pta interface					
					HWpta_LE_Access_Request(TRUE, 0, RADIO_MODE_LE_TIFS_TX_RX);
#endif
                    ((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_LE_INITIAL_TX,p_connection->connection_event_rf_channel,TX_START_FREQ);
					//HWradio_LE_Service(RADIO_MODE_LE_INITIAL_TX,p_connection->connection_event_rf_channel,TX_START_FREQ);

					_LEconnection_Encode_DataPDU(p_connection);
					LEconnection_Set_Next_Master_Connection_Event_Time(p_connection,(current_clk));

					LEconnection_Set_Substate(p_connection, W4_MASTER_CONN_FIRST_TX);
					LSLCirq_Disable_All_Tim_Intr_Except(0);
				}
				break;
			}
			break;
#endif

		case W4_MASTER_CONN_FIRST_TX :
			switch(IRQ_Type)
			{
			case LE_PKA :
				// Have to insert delay before I call the SetFReq
				// Need this to ensure that we dont place the Radio in Idle state
				// prior to BPKTCTL going low. As BPKTCTL going low is timed as 10us
				// after the TIFS.
#if 0 // GF 18 Dec 2013 - Removed for CES Demo -- Put back on the 1 jan 2014
				HWdelay_Wait_For_us(10);
#endif
				HW_set_rx_mode(0x01);
				HWle_set_rx_mode(0x05 /* Connection */);
				((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_LE_INITIAL_TX,p_connection->connection_event_rf_channel,RX_START_FREQ); 
				//HWradio_LE_Service(RADIO_MODE_LE_INITIAL_TX,p_connection->connection_event_rf_channel,RX_START_FREQ);

				LEconnection_Set_Substate(p_connection, W4_MASTER_CONN_RX);
				// Setup the connection event Timers
				p_connection->connection_event_max_length = current_clk + (p_connection->maximum_CE_length*2);
				LSLCirq_Disable_All_Tim_Intr_Except(0);
				((void (*)(void))PATCH_FUN[HWRADIO_TXCOMPLETE])();//HWradio_TxComplete();
				break;
			}
			break;

		case W4_MASTER_CONN_RX :
			switch(IRQ_Type)
			{
			case LE_SYNC_DET :
				__read_hw_reg32(((volatile unsigned *)(0x4002c000 + 0x300)), rssi_val);
				p_connection->rssi = ((s_int8 (*)(void))PATCH_FUN[HWRADIO_LE_READ_RSSI])();
				break;

			case LE_PKD :
				HW_set_rx_mode(0x00);
				HWle_set_rx_mode(0x00);
				crc_error = LE_LL_Get_CRC_Err();
				LEconnection_Set_Initial_Anchor_Point_Obtained(p_connection,1);

				if (crc_error)
				{
					if (LEconnection_Get_Prev_CRC(p_connection))
					{
						// Need to close the connection event.
						back2back_crc_detected = 0x01;
					}
					else
					{
						LEconnection_Set_Prev_CRC(p_connection,1);
					}
				}


				_LEconnection_Decode_DataPDU(p_connection);
				if ((!back2back_crc_detected) && LEconnection_Connection_Event_Still_Open(p_connection,current_clk))
				{
					HWle_set_tx_enable();
					((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_LE_TIFS_TX_RX,p_connection->connection_event_rf_channel,TX_START_FREQ);
					//HWradio_LE_Service(RADIO_MODE_LE_TIFS_TX_RX,p_connection->connection_event_rf_channel,TX_START_FREQ);
					_LEconnection_Encode_DataPDU(p_connection);
					LEconnection_Set_Substate(p_connection, W4_MASTER_CONN_TX);
					LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM2_INTR_MSK_MASK);
				}
				else
				{

					((void (*)(void))PATCH_FUN[HWRADIO_LE_RXCOMPLETE])();
					//HWradio_LE_RxComplete();
					HWle_clear_tx_enable();
					_LEconnection_Close_Connection_Event(p_connection,current_clk);
				}
				break;


			case LE_NO_PKD :
                ((void (*)(void))PATCH_FUN[HWRADIO_LE_RXCOMPLETE])();
				//HWradio_LE_RxComplete();
				HWle_clear_tx_enable();
				_LEconnection_Close_Connection_Event(p_connection,current_clk);
				break;

			}
			break;

		case W4_MASTER_CONN_TX :
			switch(IRQ_Type)
			{

			case LE_PKA :
				HWle_clear_tx_enable();
				HW_set_rx_mode(0x01);
				HWle_set_rx_mode(0x05 /* Connection */);
				((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_LE_TIFS_TX_RX,p_connection->connection_event_rf_channel,RX_START_FREQ);
				//HWradio_LE_Service(RADIO_MODE_LE_TIFS_TX_RX,p_connection->connection_event_rf_channel,RX_START_FREQ);
				LEconnection_Set_Substate(p_connection, W4_MASTER_CONN_RX);
				break;
			}
			break;
		} // End if switch
	} // End if IF-ELSE
#endif
#if 0	
            //sendchar(LEconnection_Get_Substate(p_connection));
            DMA_buf[0]=LEconnection_Get_Substate(p_connection);
            DMA_buf[1]=IRQ_Type;
            DMA_buf[2]=0xab;
            DMA_buf[3]=0xab;
            DMA_Uart_SendChar(1);
#endif  


	return NO_ERROR;
}

/***********************************************************************
 * Function :- _LEconnection_Encode_DataPDU
 *
 * Description
 * This function is responsible for encoding an LLC or Data PDU into Hardware TX
 * buffer.
 *
 * We first check if a re-transmission is to occur. In this case the packet to be retransmitted is
 * written to the HW buffers.
 *
 * If a new packet is to be transmitted the LLC/LMP and Data Queues are checked.
 * if new L2CAP data is available on the Queues, we first check that the data is already encrypted
 * if Encryption is active on the link. If encryption is active and the data has not already been
 * encrypted then we raise a flag for the MiniSched (to inform it to encrypt the data) and ignore this
 * queue entry.
 *
 * If LLC or L2CAP data is available we construct the header and copy the data into the HW Tx buffers. If no
 * data was available we construct a Zero Length L2CAP continue and write this to the HW.
 *
 * One important aspect of the Encode function is to maintain the sequence/order of LLC/L2CAP packets if
 * encryption is enabled on the link. Each Q descriptor has an entry 'enc_pkt_count' which is the packet count value
 * used when the entry was encrypted. If is essential that this matches the on-air tx packet count.
 *
 **************************************************************************/

void _LEconnection_Encode_DataPDU(t_LE_Connection* p_connection)
{
	if(PATCH_FUN[_LECONNECTION_ENCODE_DATAPDU_ID]){
         ((void (*)(t_LE_Connection* p_connection))PATCH_FUN[_LECONNECTION_ENCODE_DATAPDU_ID])(p_connection);
         return ;
    }

	struct q_desc* tmp_pqD = 0;
	u_int16 length = 0;
	u_int8 UnencryptedBuf = 0;
	// For now only send a header with 0 Len - NOTE LLID fields is not programmed as it has to be added to the HAL

	// First Determine if there is new data to be transmitted.
	// If Pdu_pending_type = 0x00 then there is not PDU PENDING.

	if((p_connection->pqD==0) && (LEconnection_Get_Ack_Pending(p_connection)==0))
	{
		// First Check the Data Queues to see if we have a control message on the LLC Queues

		tmp_pqD =  BTq_LE_Dequeue_Next(LMP_OUT_Q,p_connection->device_index,31);
		// If Encryption is active we have to check that the TX packet count from the
		// queues matchs the expected on-air Tx Packet count.
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
		if ((tmp_pqD != 0) && (p_connection->current_security_state & LE_ENCRYPTION_ACTIVE))
		{
			if (tmp_pqD->enc_pkt_count != p_connection->on_air_tx_pkt_count)
			{

				tmp_pqD = 0;
			}

		}
#endif

		if (tmp_pqD == 0) 
		{
			uint16_t	qlen = 27;
			if(p_connection->current_security_state & LE_ENCRYPTION_ACTIVE) {
				qlen = 31;
			}
			tmp_pqD = BTq_LE_Dequeue_Next(L2CAP_OUT_Q, p_connection->device_index, qlen);
		      /* ADD END */
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
			// Only send L2_DATA packets when Encryption is either fully active or not-active.
			// Thus ensureing that NO L2 Data is sent while transitioning into/out of encryption.

			// In Encryption is not transitioning deQueue a new L2CAP packet
			if (!(p_connection->ll_action_pending & LL_ENCRYPTION_TRANSITIONING))
			{
				//tmp_pqD = BTq_LE_Dequeue_Next(L2CAP_OUT_Q, p_connection->device_index, 31);

			}
			else
			{
				tmp_pqD = 0;
			}


			if ((tmp_pqD) && !(p_connection->ll_action_pending & LL_TERMINATE_PENDING))
			{
				if (p_connection->current_security_state & LE_ENCRYPTION_ACTIVE)
				{
					if ((tmp_pqD->encrypt_status & LE_DATA_ENCRYPT_PENDING)||
						(tmp_pqD->encrypt_status & LE_DATA_NOT_ENCRYPTED))
					{
						if (tmp_pqD->encrypt_status & LE_DATA_NOT_ENCRYPTED)
							tmp_pqD->encrypt_status = LE_DATA_ENCRYPT_PENDING;
						// Raise a flag for the Mini-Shed and ignore this queue entry
						LE_config.encrypt_pending_flag =0x01;
						LE_config.device_index_pending_data_encrypt = p_connection->device_index;
						tmp_pqD = 0;
						UnencryptedBuf = 1;
					}
					else if (tmp_pqD->encrypt_status & LE_DATA_ENCRYPTED)
					{
						if (tmp_pqD->enc_pkt_count != p_connection->on_air_tx_pkt_count)
						{
							tmp_pqD = 0;
						}

					}
				}

			}
			else
			{
				tmp_pqD = 0;
			}
#endif
		}

        if (tmp_pqD)
        {
        	length =  tmp_pqD->length;
        	hw_memcpy32((void*)HWle_get_tx_acl_buf_addr(), (void*)tmp_pqD->data, length);
        	HWle_set_tx_llid(tmp_pqD->message_type);
					
					if(more_data_enable)
					{
    		     HWle_set_tx_md();
					}	
          else
          {
						HWle_clear_tx_md();
					}  

#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
        	if (p_connection->current_security_state & LE_ENCRYPTION_ACTIVE)
				p_connection->on_air_tx_pkt_count++;
#endif

        }
        else
        {
            // Zero length poll packet and MD = 0

    		HWle_set_tx_llid(0x01);
			 if((1 != UnencryptedBuf)||(1 != more_data_enable))
    		    HWle_clear_tx_md();
        }
		HWle_set_tx_length_data(length);
		p_connection->pqD = tmp_pqD;
	   	p_connection->pdu_pending_length = length;
	}
	else
	{
		/************************************************
		 * Review all this section for Multipoint support
		 * For Point to Point I can assume the packet to be
		 * re-transmitted is already in the TX Buffer.
		 * HOWEVER, for multipoint the TX Buffer may contain
		 * a PACKET for another link..
		 ************************************************/

        if (p_connection->pqD)
        {
        	length =  p_connection->pqD->length;
        	hw_memcpy32((void*)HWle_get_tx_acl_buf_addr(), (void*)p_connection->pqD->data, length);

        	HWle_set_tx_llid(p_connection->pqD->message_type);
					
					if(more_data_enable)
					{
    		     HWle_set_tx_md();
					}	
          else
          {
						HWle_clear_tx_md();
					}  
    		HWle_set_tx_length_data(length);

        	p_connection->pdu_pending_length = length;
        }

	}
	LEconnection_Set_Ack_Pending(p_connection, 0x01);
	HWle_set_tx_nesn(LEconnection_Get_NextExpSeqNum(p_connection));
	HWle_set_tx_sn(LEconnection_Get_TxSeqNum(p_connection));
#if (PRH_BS_CFG_SYS_TRACE_LLDATA_VIA_HCI_SUPPORTED==1)
	PRH_lldata_dbg_channel = p_connection->current_data_channel;
	PRH_lldata_dbg_length = p_connection->pdu_pending_length;
	PRH_lldata_dbg_header = HWle_get_tx_data_header();

	TCeg_Send_LE_LLData_Event(PRH_lldata_dbg_header,(u_int8*)p_connection->pqD, PRH_lldata_dbg_length,PRH_lldata_dbg_channel, 1 /* Tx */,(p_connection->link_id+1));
#endif
}
/***********************************************************************
 * Function :- _LEconnection_Decode_DataPDU
 *
 * Description
 * This function is responsible for decoding an LLC or Data PDU from the Hardware RX
 * buffer and placing it in the incoming SW queues.
 *
 * The Header bits are the CRC bit are read first.
 *
 * If there is a CRC on the Packet - no further action is taken.
 *
 * Otherwise
 * 	Steps to perform :
 * 		1/ Check if previous Tx is acked
 * 		2/ Determine if this Rx is a duplicate
 * 		3/ Using the LLID determine what to do.
 ***********************************************************************/
u_int32 _LEconnection_Decode_DataPDU(t_LE_Connection* p_connection)
{
	
	if(PATCH_FUN[_LECONNECTION_DECODE_DATAPDU_ID]){
         
         return ((u_int32 (*)(t_LE_Connection* p_connection))PATCH_FUN[_LECONNECTION_DECODE_DATAPDU_ID])(p_connection);
    }

	u_int8 crc_error = HWle_get_crc_err();

	u_int8 rx_nesn = HWle_get_rx_nesn();
	u_int8 rx_sn = HWle_get_rx_sn();
	u_int8 rx_md = HWle_get_rx_md();
	u_int16 rx_length = HWle_get_rx_length_data();
	u_int8 rx_llid = HWle_get_rx_llid();

    u_int32 error = 0;
    
	LEconnection_Set_Rx_Md(p_connection,rx_md);

	// Still need to check the CRC ?? Even though check by calling function ??!!!
	if(!crc_error)
	{
		// Steps to perform :
		//   1/ Check if previous Tx is acked
		//   2/ Determine if this Rx is a duplicate
		//   3/ Using the LLID determinee what to do.
		//
		u_int8 pdu_pending_type = 0;
		/* Record no CRC */
		LEconnection_Set_Prev_CRC(p_connection,0);

		if(p_connection->pqD != 0)
		{
			pdu_pending_type = p_connection->pqD->message_type;
		}

		/*--------------------------------------------
		CON/SLA/BV-16-C [Slave Retransmission]
		
		//if(rx_nesn != rx_sn) return 0;    // for: TP/CON/SLA/BV-16-C [Slave Retransmission]
		
		if(rx_nesn != rx_sn) return 0;      // for: Normol WORK.	
		---------------------------------------------*/
//20200326		
        //if(rx_nesn != rx_sn) return 0;
		
		// Step 1
		if (rx_nesn != LEconnection_Get_TxSeqNum(p_connection))
		{
			// If there is an ack pending, then this is an ACK for that PDU
            // Step 3
			// If length != 0,  Determine what the Ack was pending for.
			if (((pdu_pending_type == LCH_start) ||
				(pdu_pending_type == LCH_continue)) &&
				(p_connection->pdu_pending_length != 0x00))
			{
				// Need to ACK the L2CAP Queue for the this link.
				BTq_Ack_Last_Dequeued(L2CAP_OUT_Q,p_connection->device_index, p_connection->pdu_pending_length );
						  
				#if defined(SYS_HAL_OS_EVENT_DRIVEN_HOST_CONTROLLER)
					BTms_OS_Post(BTMS_OS_EVENT_HOST_TO_HC_FLOW_CONTROL);
				#endif
				
				p_connection->pqD = 0;
				p_connection->pdu_pending_length = 0x00;
			}
			else if (pdu_pending_type == 0x3)
			{
				u_int8 opcode_pending = p_connection->pqD->opcode_pending;

				// Need to ACK the LL CONTROL for this link.
				BTq_Ack_Last_Dequeued(LMP_OUT_Q,p_connection->device_index,p_connection->pdu_pending_length );

				// Ensure that any handling of an Ack to an LLC message is performed

				LE_LL_Handle_LLC_Ack(p_connection,opcode_pending);
				p_connection->pqD = 0;
				p_connection->pdu_pending_length = 0x00;
			}
			LEconnection_Set_Ack_Pending(p_connection,0x00);
			LEconnection_Set_TxSeqNum(p_connection,rx_nesn);
		}

		// Step 2
		if (rx_sn == LEconnection_Get_NextExpSeqNum(p_connection))
		{
			struct q_desc* tmp_pqD = 0;

			// Check the LLID and decide if we should enqueue
			if (((rx_llid == LCH_start) || (rx_llid == LCH_continue)) &&
				(rx_length > 0))
			{
				{
					tmp_pqD = BTq_Enqueue(L2CAP_IN_Q, p_connection->device_index, rx_length);
					if(tmp_pqD)
					{
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
						// Temp for Conformance Test CON/MAS/BI-07
						if(p_connection->current_security_state & LE_ENCRYPTION_ACTIVE)
						{
							tmp_pqD->enc_pkt_count = p_connection->on_air_rx_pkt_count;
							tmp_pqD->encrypt_status = LE_DATA_DECRYPT_PENDING;
							p_connection->on_air_rx_pkt_count++;
						}
#endif
						tmp_pqD->message_type = rx_llid;
						tmp_pqD->device_index = p_connection->device_index;

						hw_memcpy32((void*)tmp_pqD->data,(void*)HWle_get_rx_acl_buf_addr(), rx_length);
						BTq_Commit(L2CAP_IN_Q, p_connection->device_index);

						// Toggle the value of nextExpected Sequence Number
						LEconnection_Toggle_NextExpSeqNum(p_connection);
					}
                    else {
                        HWle_clear_tx_enable();
						LEconnection_Set_TxSeqNum(p_connection, !rx_nesn);
                        error = 1;
                    }
				}

			}
			else if (/*(rx_llid == LCH_continue) && */(rx_length == 0))
			{
				// Toggle the value of nextExpected Sequence Number
				LEconnection_Toggle_NextExpSeqNum(p_connection);
			}
			else if ((rx_llid == LE_LLC /* Link Control PDU */) &&
					 (rx_length > 0))
			{
				// Prior to decoding the PDU ensure that the ACK is written
				// to the HW.

				tmp_pqD = BTq_Enqueue(LMP_IN_Q, p_connection->device_index, rx_length);
				#if     0
				if(tmp_pqD) LEconnection_Toggle_NextExpSeqNum(p_connection);
				__DEBUG_IO_HIGH7();
	    			HWle_set_tx_llid(0x01);

	    			HWle_set_tx_length_data(0x00);
				HWle_set_tx_nesn(LEconnection_Get_NextExpSeqNum(p_connection));
				HWle_set_tx_sn(LEconnection_Get_TxSeqNum(p_connection));
				__DEBUG_IO_LOW7();
				#endif
				if(tmp_pqD)
				{

#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
					if(p_connection->current_security_state & LE_ENCRYPTION_ACTIVE)
					{
						tmp_pqD->enc_pkt_count = p_connection->on_air_rx_pkt_count;
						p_connection->on_air_rx_pkt_count++;
					}
#endif
					tmp_pqD->message_type = rx_llid;
					tmp_pqD->device_index = p_connection->device_index;
					hw_memcpy32((void*)tmp_pqD->data,(void*)HWle_get_rx_acl_buf_addr(), rx_length);
					BTq_Commit(LMP_IN_Q, p_connection->device_index);

					// Toggle the value of nextExpected Sequence Number
					LEconnection_Toggle_NextExpSeqNum(p_connection);
				}
			}
		}

#if (PRH_BS_CFG_SYS_TRACE_LLDATA_VIA_HCI_SUPPORTED==1)
		PRH_lldata_dbg_channel = p_connection->current_data_channel;
		PRH_lldata_dbg_length = rx_length;
		PRH_lldata_dbg_header = HWle_get_rx_data_header();

		TCeg_Send_LE_LLData_Event(PRH_lldata_dbg_header,(u_int8*)HWle_get_rx_acl_buf_addr(), PRH_lldata_dbg_length,PRH_lldata_dbg_channel, 0 /* Rx */,(p_connection->link_id+1));
#endif
		if (LEconnection_Is_Active(p_connection))
		{
			p_connection->supervision_TO_timer = BTtimer_Safe_Clockwrap(HW_Get_Native_Clk_Avoid_Race() + ((p_connection->timeout*0x10)*2));
		}
	}
	else
	{
		// We have a CRC error.. Action depends if this is a second consecutive CRC or not.
		/* Two consecutive packets received with an invalid CRC match within a connection
		 * event shall close the event.
		 */
        //error=1;
	}
	return error;
}

#if (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE == 1)

/***********************************************************************
 * Function :- ConvertSCAtoPPM
 *
 * Description
 * This function is responsible determining the window widening required.
 ***********************************************************************/
/*static u_int16 _Convert_SCA_to_PPM(u_int8 sleep_clock_accuracy)
{
	u_int16 ppm = 500;
	const u_int16 SCA_PPM_Map[8] = {500,250,150,100,75,50,30,20};

	if (sleep_clock_accuracy <8)
		ppm = SCA_PPM_Map[sleep_clock_accuracy];


	return ppm;
}*/

/***********************************************************************
 * Function :- _LEconnection_Determine_Window_Widening
 *
 * Description
 * This function is responsible determining the window widening required.
 ***********************************************************************/

/*u_int16 _LEconnection_Determine_Window_Widening(t_LE_Connection* p_connection)
{
	u_int16 delta_events;
	u_int16 time_in_slots_since_access;
	u_int32 win_us;
	u_int16 other_Side_SCA = _Convert_SCA_to_PPM(p_connection->sleep_clock_accuracy);
	u_int16 our_Side_SCA = _Convert_SCA_to_PPM(LE_config.sleep_clock_accuracy);

	delta_events = p_connection->connection_event_counter - p_connection->connection_event_of_last_rx;
	// from BT spec V4.0 Vol6 page73
	// windowWidening = ((masterSCA+slaveSCA)/1000000)*timeSinceLastAnchor
	// optimised for slots:
	// windowWidening = ((masterSCA+slaveSCA)*5*timeSinceLastAnchorInSlots)/4000)
	time_in_slots_since_access = delta_events * p_connection->connInterval;


	win_us = (our_Side_SCA+other_Side_SCA) * 5 * time_in_slots_since_access/4000; //in uS
	// add deviation 16uS from BT spec V4.0 Vol6 page 55
	win_us += 16;
	if (win_us > 0xffff)
		win_us = 0xffff;
	return win_us;

}*/

/***********************************************************************
 * Function :- _LEconnection_Determine_Window_Widening_In_Frames
 *
 * Description
 * This function is responsible determining the window widening required in Frames.
 *
 * NOT COMPLETE -
 *      FURTHER WORK IS NEEDED HERE
 ***********************************************************************/

t_slots _LEconnection_Determine_Window_Widening_In_Frames(t_LE_Connection* p_connection)
{
	
	 
    if(PATCH_FUN[_LECONNECTION_DETERMINE_WINDOW_WIDENING_IN_FRAMES_ID]){
         
         return ((t_slots (*)(t_LE_Connection* p_connection))PATCH_FUN[_LECONNECTION_DETERMINE_WINDOW_WIDENING_IN_FRAMES_ID])(p_connection);
    }

	u_int16 delta_events;
	u_int16 time_in_slots_since_access;

	// TODO :- Below is hardcoded for HOW at 500ppm for each device - as HW issue with realigning the
	// slave to the masters transmissions.
	//return 1;

	if (0 == g_debug_le_sleep)
	{
		return 1;
	}

#if	1	
	// Temp Hack - ONLY... To be removed after MultiSlot Window Widening is fully tested.
    if (LE_config.Auto_Widen_MultiSlot == 0)
		return ext_rx_window;

	delta_events = (p_connection->connection_event_counter+1) - p_connection->connection_event_of_last_rx;

	// For Now Hardcode both devices to 500ppm

	time_in_slots_since_access = delta_events * p_connection->connInterval * 2;

	// 2000 Slots = =/- 1.25ms
	if (time_in_slots_since_access<1000)
		return 2;
	else
		return ((time_in_slots_since_access/1000)*2)+2;
#endif
}
#endif

/***********************************************************************
 * Function :-  LEconnection_Find_Device_Index
 *
 * Description
 * Returns a pointer to a Connection, using the device index as search key
 **********************************************************************/

t_LE_Connection* LEconnection_Find_Device_Index(u_int8 device_index)
{
	u_int8 i;
	for(i=0;i< DEFAULT_MAX_NUM_LE_LINKS; i++)
	{
		if(LEconnections_Link_In_Use(i) && (LE_connections[i].device_index == device_index))
		{
			return &LE_connections[i];
		}
	}
	return 0;

}

/***********************************************************************
 * Function :-  LEconnection_Find_P_Connection
 *
 * Description
 * Returns a pointer to a Connection, using the Connection Handle as search key
 **********************************************************************/

t_LE_Connection*  LEconnection_Find_P_Connection(u_int16 handle)
{
	u_int8 i;


	i = handle - PRH_BS_CFG_LE_CONN_HANDLE_OFFSET;

	if(LEconnections_Link_In_Use(i))
		return &LE_connections[i];

	return 0;
}

/***********************************************************************
 * Function :-  LEconnection_Find_Link_Id
 *
 * Description
 * Returns a pointer to a link_id, using the Connection Handle as search key
 **********************************************************************/

t_error LEconnection_Find_Link_Id(u_int16 handle,u_int8* p_link_id)
{
	u_int8 i;

	i = handle - PRH_BS_CFG_LE_CONN_HANDLE_OFFSET;

	if(LEconnections_Link_In_Use(i))
	{
		*p_link_id = i;
		return NO_ERROR;
	}

	return NO_CONNECTION;
}

/***********************************************************************
 * Function :-  _LEconnection_Close_Connection_Event
 *
 * Description
 * This function is called to close a connection event. Following actions are
 * performed.
 *
 * HW Mods
 * 	Set the SPI to LE only - should not be done in LE Dual Mode
 *  Abort the TIFS count and set RX mode = 0
 *  Set the Tx Enable = 0
 *  Call RxComplete on the RF interface if device is a SLAVE
 *  Set the Radio in Standby
 *  Disable all Tim Interrupts except TIM-0 and TIM-2
 *
 * SW
 *  Set the device state to STANDBY
 *  Update the Connection Event Counter
 *
 *  If there is an action/operation pending and the operation instance will expire on the next
 *  connection event. Actions performed are dependent on the action/operation pending :
 *  	LL_CHANNEL_MAP_REQ_PENDING   - Update the Channel Map
 *  	LL_CONNECTION_UPDATE_PENDING - Update connection parameters
 *  	LL_TERMINATE_PENDING         -
 *
 *
 **********************************************************************/

void _LEconnection_Close_Connection_Event(t_LE_Connection* p_connection ,u_int32 current_clk)
{
	
	if(PATCH_FUN[_LECONNECTION_CLOSE_CONNECTION_EVENT_ID]){
         ((void (*)(t_LE_Connection* p_connection ,u_int32 current_clk))PATCH_FUN[_LECONNECTION_CLOSE_CONNECTION_EVENT_ID])(p_connection ,  current_clk);
         return ;
    }

	HWle_clear_le_spi_only();
	((void (*)(void))PATCH_FUN[HWRADIO_DISABLEALLSPIWRITES])();	
	//HWradio_DisableAllSpiWrites();
 
	// No Receive on Slave from Master within the masters Initial Transmit Window.
	// or within the window widened portion of RX.
	p_connection->transmit_window_remaining=0;
	// Clear any CRC info for the connection event.
	LEconnection_Set_Prev_CRC(p_connection,0);
	// Clear the TX and RX modes
	HW_set_rx_mode(0x00);
	//HWle_set_rx_mode(0x00);
	// GF Added 29 April
    HW_set_tx_mode(0x00);
	HWle_abort_tifs_count();
	HWle_set_tifs_abort();
	HWle_clear_tifs_default();
	HWle_clear_tx_enable();

	if (LEconnection_Get_Role(p_connection) == SLAVE)
	{
//		HW_set_freeze_bit_cnt(0);
		((void (*)(void))PATCH_FUN[HWRADIO_LE_RXCOMPLETE])();
		//HWradio_LE_RxComplete();
		LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM0_INTR_MSK_MASK | JAL_TIM2_INTR_MSK_MASK);
	}
	else
	{
		LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM0_INTR_MSK_MASK | JAL_TIM2_INTR_MSK_MASK);
	}

	LE_config.state = STANDBY_STATE;
	// Not so sure about placing RF in standby more..
	((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_STANDBY,0,0);
	//HWradio_LE_Service(RADIO_MODE_STANDBY,0,0);
#ifndef BLUETOOTH_MODE_LE_ONLY//no pta interface
	HWpta_LE_Access_Complete();
#endif

    // Connection_Event Count - now moved to closing of the connection event
	// One the Connection Event has fired I need to change the device state
	// to Connection. And change the substate.
	if (p_connection->connection_event_counter == 0xFFFF)
		p_connection->connection_event_counter=0;
	else
		p_connection->connection_event_counter++;


	if ((p_connection->ll_action_pending & LL_CHANNEL_MAP_REQ_PENDING) &&
		(p_connection->channel_map_update_instant == p_connection->connection_event_counter))
	{
		LEfreq_Update_Channel_Map(p_connection);
	}

	if ((p_connection->ll_action_pending & LL_CONNECTION_UPDATE_PENDING) &&
		(p_connection->connection_update_instant  == p_connection->connection_event_counter))
	{
		LEconnection_Connection_Update(p_connection);
		ext_rx_window_init(p_connection);
	}

	//if ((p_connection->ll_action_pending & LL_TERMINATE_PENDING) ||
	//   BTtimer_Is_Expired_For_Time(p_connection->supervision_TO_timer, current_clk + 1))
	if (p_connection->ll_action_pending & LL_TERMINATE_PENDING)
	{
	    t_lm_event_info event_info;

		g_LE_Slave_IntraSlot = 0;
	   	p_connection->supervision_TO_timer = BT_TIMER_OFF;
		p_connection->ll_response_timer = BT_TIMER_OFF;
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
		LE_config.link_key_request_timer = BT_TIMER_OFF;
#endif
		//
		// Generate a disconnection complete event.

		event_info.handle = LEconnection_Determine_Connection_Handle(p_connection);

	    event_info.status = NO_ERROR;
	    if (p_connection->ll_action_pending & LL_TERMINATE_PENDING)
			event_info.reason = (t_error)(p_connection->terminate_reason); // Remote User Terminated Link.
		else
		{
			if (LEconnection_Get_Initial_Anchor_Point_Obtained(p_connection))
				event_info.reason = CONNECTION_TIMEOUT;
			else
				event_info.reason = EC_CONNECTION_FAILED_TO_BE_ESTABLISHED;
		}
        
//        p_connection->ll_action_pending &= ~LL_TERMINATE_PENDING;
        

		// Clear the connection event timer
	    if (LEconnection_Get_Role(p_connection) == SLAVE)
	    {
	    	LE_config.slave_link_active = 0x00;

#if (PRH_BS_CFG_SYS_LE_DUAL_MODE==1)
			HW_set_add_bt_clk_relative(0);
		    HW_set_bt_clk_offset(0);
			HW_set_slave(0x00);
#endif
		    HWle_set_master_mode();
	    }
	    else
	    {
	    	LE_config.master_link_active--;
	    }

		LE_config.num_le_links--;
		if (LE_config.num_le_links == 0x00)
			LE_config.next_connection_event_time = BT_TIMER_OFF;;

#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==1)
#if (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)

		if (LE_config.auto_advertise)
		{
			// For Auto-Advertise ensure that the Adv Packet type is changed to Non-Connectable

			LEadv_Set_Advertise_Enable(0x01);
		}


#endif
#endif

#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==1)
		if ((LE_config.auto_initiate) && (LEconnection_Get_Role(p_connection) == MASTER))
		{

		 	HCfc_Release_All_Data_Queue_Packets(p_connection->device_index);

			LE_config.next_initiator_scan_timer = BTtimer_Safe_Clockwrap(HW_Get_Native_Clk_Avoid_Race() + (LE_config.initiator_scan_interval*4));
			LE_config.initiating_enable = 0x01;
			_LEconnection_Reset_Connection(p_connection);
		}
		else
		{
		 	HCfc_Release_All_Data_Queue_Packets(p_connection->device_index);

			_LEconnection_Free_Connection(p_connection);
		}
#else
#if (PRH_BS_CFG_UART_FOR_SENSOR_ONLY==0)
	 	HCfc_Release_All_Data_Queue_Packets(p_connection->device_index);
#endif
		_LEconnection_Free_Connection(p_connection);
#endif


#if (PRH_BS_CFG_UART_FOR_SENSOR_ONLY==0)
		HCeg_Generate_Event(HCI_DISCONNECTION_COMPLETE_EVENT, &event_info);
		#if defined(SYS_HAL_OS_EVENT_DRIVEN_HOST_CONTROLLER)
		 BTms_OS_Post(BTMS_OS_EVENT_HCI_EVENT_QUEUE);
		#endif
#endif
#if (PRH_BS_CFG_SYS_EMBEDDED_PROFILES==1)
		LE_App_Local_Disconnect_Indication();
#endif
		return;
	}



	LE_config.next_connection_event_time = LEconnection_Find_Next_Connection_Event(current_clk);

#if (PRH_BS_CFG_SYS_TRACE_LE_DEVICE_STATE_VIA_HCI_SUPPORTED == 1)
	TCeg_Send_LE_LEDeviceState_Event(LE_config.state,0);
#endif


	//if (p_connection->role == MASTER)
	{
	/*	LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM0_INTR_MSK_MASK |JAL_TIM2_INTR_MSK_MASK | JAL_TIM1_INTR_MSK_MASK |JAL_TIM3_INTR_MSK_MASK );*/
#ifndef BLUETOOTH_MODE_LE_ONLY
		LEconfig_Switch_To_BT_Classic();
#endif
#if (PRH_BS_CFG_SYS_LOW_POWER_MODE_SUPPORTED==1)
#if (SYS_CFG_LE_SLEEP_ACTIVE_CONNECTIONS==1)
#if (__DEBUG_LE_SLEEP == 0)
        if(0 == g_debug_le_sleep)
        {
	        LE_LL_Sleep();
        }
#endif
#endif
#if (__DEBUG_LE_SLEEP == 2)
    if(2 == g_debug_le_sleep)
    {
    //if(LE_config.num_le_links != 0) {
    if(connect_always_wake == 0)
    {
    Connection_Enter_LowPower(p_connection);
	  }
    //}
    }

#endif
#endif

	}


}

/***********************************************************************
 * Function :-  LEconnection_Setup_First_Connection_Event
 *
 * Description
 * This function is used to setup the time for the first connection event
 * on a connection. This is call either on the receipt of a CONNECT_REQ in a
 * slave OR on the receipt of a PKA for the Tx of a CONNECT_REQ in a Master.
 ***********************************************************************/

void LEconnection_Setup_First_Connection_Event(u_int8 link_id,u_int32 current_clk)
{
	
	if(PATCH_FUN[LECONNECTION_SETUP_FIRST_CONNECTION_EVENT_ID]){
         ((void (*)(u_int8 link_id,u_int32 current_clk))PATCH_FUN[LECONNECTION_SETUP_FIRST_CONNECTION_EVENT_ID])(link_id,  current_clk);
         return ;
    }

	// Setup the first Connection Event Timers on a new link

	t_LE_Connection* p_connection = &LE_connections[link_id];
	// Actions are dependent on the Role.

	p_connection->connection_event_counter = 0x0000;
	p_connection->connection_event_of_last_rx = 0x00;
	p_connection->lastUnmapped_Channel = 0x00;

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
	if (LEconnection_Get_Role(p_connection) == MASTER)
	{
		// In the master we determine the actual positioning of the Anchor point within the
		// transmit window. Until more advanced resource management is in place, we will position
		// the Anchor point 1.25ms after the start of the transmit window.

		// Not that the transmitWindowSize, transmitWindowOffset and ConnectionInterval are all
		// given as N * 1.25 ms by the host. [ not in slots of .625ms ].
#if (PRH_BS_CFG_LE_FIX_PLL_ISSUE==1)
		p_connection->next_connect_event_start_time = BTtimer_Safe_Clockwrap( current_clk + (p_connection->transmit_window_offset*4) );

		// If Next CE on this link is earlier than the current to LE_config.next_connection_event_time
		// then we reset the LE_config.next_connection_event_time to the next CE on this link.

		LEconnection_Find_Next_Connection_Event(current_clk);

		p_connection->supervision_TO_timer = BTtimer_Safe_Clockwrap(current_clk + ((p_connection->connInterval*6)*4));
#else
		p_connection->next_connect_event_start_time = BTtimer_Safe_Clockwrap( current_clk +4 + (p_connection->transmit_window_offset*4) );

		// If Next CE on this link is earlier than the current to LE_config.next_connection_event_time
		// then we reset the LE_config.next_connection_event_time to the next CE on this link.

		LEconnection_Find_Next_Connection_Event(current_clk);

		p_connection->supervision_TO_timer = BTtimer_Safe_Clockwrap(current_clk +4+ ((p_connection->connInterval*6)*4));

#endif
	}
#endif

#if (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
	if (LEconnection_Get_Role(p_connection)== SLAVE)
	{
#if 0
		current_clk = HW_Get_Bt_Clk_Avoid_Race() & 0xFFFFFFFC;
#else
		current_clk = HW_Get_Native_Clk_Avoid_Race() & 0xFFFFFFFC;
#endif

		LEconnection_Set_Initial_Anchor_Point_Obtained(p_connection,0x00);
		have_get_no_crc_pkt = 0;
#if (PRH_BS_CFG_SYS_LE_SINGLE_MODE==1)
		p_connection->next_connect_event_start_time = BTtimer_Safe_Clockwrap( (current_clk-3) + (p_connection->transmit_window_offset*4));
#else
		p_connection->next_connect_event_start_time = BTtimer_Safe_Clockwrap( ((current_clk & 0xFFFFFFFC) /*+ 4*/) + (p_connection->transmit_window_offset*4));
#endif
		LE_config.next_connection_event_time = p_connection->next_connect_event_start_time;

        if ( 1 == g_le_enable_observer_flag)
        {
			LE_LL_Adjust_scan_timer(p_connection->next_connect_event_start_time, p_connection->connInterval, p_connection->transmit_window_size);
		}	
		//p_connection->supervision_TO_timer = current_clk + ((p_connection->timeout*0x10)*2);
		p_connection->supervision_TO_timer = BTtimer_Safe_Clockwrap(current_clk + ((p_connection->connInterval*6)*8)); // *4  TBD
	}
#endif


}


/***********************************************************************
 * Function :-  LEconnection_Disconnect
 *
 * Description
 * This is invoked in response to receiving a HCI_DISCONNECT from the host.
 * If Encodes an LL_TERMINATE_IND for transmission to the peer.
 ***********************************************************************/


void LEconnection_Disconnect(t_LE_Connection* p_connection,u_int8 reason)
{

	// Enqueue a Terminate for the identified Link Id.
	LEllc_Encode_PDU(LL_TERMINATE_IND,p_connection, reason);

	// GF 29th May -- Added for conformance - test CON/MAS/BV08
	p_connection->terminate_reason = LOCAL_TERMINATED_CONNECTION;

}

void LEconnection_Local_Disconnect(t_LE_Connection* p_connection,u_int8 reason)
{
	p_connection->ll_action_pending |=  LL_TERMINATE_PENDING;
    p_connection->supervision_TO_timer = BT_TIMER_OFF;
    p_connection->terminate_timer = BT_TIMER_OFF;
    p_connection->terminate_reason = reason;
    p_connection->ll_response_timer = BT_TIMER_OFF;

    HCfc_Release_All_Data_Queue_Packets(p_connection->device_index);
}

void _LEconnection_Free_Connection(t_LE_Connection* p_connection)
{
    _LEconnection_Reset_Connection(p_connection);
	LEconfig_Free_Link_Id(p_connection);
}


/********************************************************************************
 * Function :-  _LEconnection_Reset_Connection
 *
 * Description
 * This function is only used if the Auto-Initiate after disconnect feature is in the
 * build. It resets teh connection BUT does not free the link Id.
 *
 *********************************************************************************/


void _LEconnection_Reset_Connection(t_LE_Connection* p_connection)
{
	
	 
    if(PATCH_FUN[_LECONNECTION_RESET_CONNECTION_ID]){
         ((void (*)(t_LE_Connection* p_connection))PATCH_FUN[_LECONNECTION_RESET_CONNECTION_ID])(p_connection);
         return ;
    }

#if (PRH_BS_CFG_UART_FOR_SENSOR_ONLY==0)
	HCfc_Release_All_Data_Queue_Packets(p_connection->device_index);
#endif

	p_connection->pqD = 0;
	LEconnection_Set_Active(p_connection,0x00);
	p_connection->next_connect_event_start_time = BT_TIMER_OFF;
	p_connection->connection_event_counter = 0;
	LEconnection_Set_Substate(p_connection, SUBSTATE_IDLE);
	LEconnection_Set_TxSeqNum(p_connection,0x00);
	LEconnection_Set_NextExpSeqNum(p_connection,0x00);
	LEconnection_Set_Ack_Pending(p_connection,0x00);
	LEconnection_Set_Prev_CRC(p_connection,0x00);
	p_connection->pdu_pending_length = 0x00;
	LEconnection_Set_Initial_Anchor_Point_Obtained(p_connection,0x00);
	p_connection->numUsedChannels =  37;
	p_connection->lastUnmapped_Channel = 0;
	p_connection->supervision_TO_timer = BT_TIMER_OFF;
    p_connection->terminate_timer = BT_TIMER_OFF;
	p_connection->ll_action_pending = 0;
    p_connection->ll_response_timer = BT_TIMER_OFF;
	p_connection->connection_update_instant = 0;
	p_connection->latency_count = 0;
	p_connection->intraslot = 0x00;
	p_connection->timeout = 0xFFFF;

#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
	p_connection->current_security_state = LE_ENCRYPTION_NOT_ACTIVE;
	p_connection->session_key = (u_int8*)p_connection->u32session_key;
	p_connection->IV = (u_int8*)p_connection->u32IV;
	p_connection->on_air_rx_pkt_count = 0;
	p_connection->enc_tx_pkt_counter = 0;
	p_connection->on_air_tx_pkt_count=0;
#endif
	p_connection->rssi = 127;
	p_connection->version_ind_status = 0x00;

	p_connection->last_IntroSlotOffset = 0x00;
	p_connection->current_IntroSlotOffset = 0x00;
}

/********************************************************************************
 * Function :-  LEconnection_Connection_Event_Still_Open
 *
 * Description
 * Determines if a connection event should remain open. First checks is the CE Max Length
 * has expired, then checks the queues to see if any L2CAP or LLC data to be sent.
 *
 *********************************************************************************/


u_int8 LEconnection_Connection_Event_Still_Open(t_LE_Connection* p_connection, u_int32 current_clk)
{
     
    if(PATCH_FUN[LECONNECTION_CONNECTION_EVENT_STILL_OPEN_ID]){
         
         return ((u_int8 (*)(t_LE_Connection* p_connection, u_int32 current_clk))PATCH_FUN[LECONNECTION_CONNECTION_EVENT_STILL_OPEN_ID])(p_connection,   current_clk);
    }

	// Connection Event Stays open as long
	//              1 / packets are Rxed from Slave with MD = 1
	//              2 / Packets on the Current Outgoing Queues for this link.
	//              1 / MAX Connection Event Time exceeded -- End CE

	if (BTtimer_Is_Expired_For_Time(p_connection->connection_event_max_length,current_clk))
	{
		return 0;
	}
	else if ( BTq_Is_Queue_Empty(L2CAP_OUT_Q,p_connection->device_index) &&
			 BTq_Is_Queue_Empty(LMP_OUT_Q,p_connection->device_index) &&
			 (LEconnection_Get_Rx_Md(p_connection) == 0))
	{
		// If a Terminate is pending we continue to use the event max length
		if (p_connection->ll_action_pending != LL_TERMINATE_PENDING )
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}

	return 1;
}


/********************************************************************************
 * Function :-  LEconnection_Connection_Update
 *
 * Description
 * Assigns the new connection parameters to the relevant elements in the container structure
 * ( i.e  connInterval = connInterval_new ). Turns off the connection update pending flag
 * and then generates a LE Meta Event ( CONNECTION_UPDATE_COMPLETE ).
 *
 *********************************************************************************/

t_error LEconnection_Connection_Update(t_LE_Connection* p_connection)
{
	
	 
    if(PATCH_FUN[LECONNECTION_CONNECTION_UPDATE_ID]){
         
         return ((t_error (*)(t_LE_Connection* p_connection))PATCH_FUN[LECONNECTION_CONNECTION_UPDATE_ID])(p_connection);
    }

	u_int8 non_anchor_point_change = 1;
	// Conformance 29 May
	// Need to check if only the anchor point is being changed.
	
#if 1   //- TP/TIM/SLA/BV-01-C
    
	if ((p_connection->connInterval == p_connection->connInterval_new) &&
		(p_connection->latency == p_connection->latency_new)&&
		(p_connection->timeout == p_connection->timeout_new))
	{
		non_anchor_point_change = 0;
	}
#endif
	LEconnection_Set_Initial_Anchor_Point_Obtained(p_connection,0x00);
	have_get_no_crc_pkt = 0;

	if (non_anchor_point_change)
	{
		p_connection->connInterval = p_connection->connInterval_new;
		p_connection->latency = p_connection->latency_new;
		p_connection->timeout = p_connection->timeout_new;
	}

#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==1)
	if (LE_config.TCI_transmit_win_config)
	{
		p_connection->transmit_window_size = LE_config.transmit_win_size;
		p_connection->transmit_window_offset = LE_config.transmit_win_offset;
	}
	else
#endif
	{
		p_connection->transmit_window_offset  = p_connection->transmit_window_offset_new /*+ 4*/; // to account for the T.W.O + 1.25ms in spec;
		p_connection->transmit_window_size = p_connection->transmit_window_size_new;
	}

    if ( 1 == g_le_enable_observer_flag)
    {
		LE_LL_Adjust_scan_timer(p_connection->next_connect_event_start_time, p_connection->connInterval, p_connection->transmit_window_size);
	}	
    p_connection->ll_action_pending &= ~LL_CONNECTION_UPDATE_PENDING;
	// Generate HCI Event
	//
    if (non_anchor_point_change)
    	LEeventgen_Generate_LE_Meta_Event(LE_SUBEVENT_CONNECTION_UPDATE_COMPLETE,p_connection,NO_ERROR);

	return NO_ERROR;
}



/********************************************************************************
 * Function :- LEeventgen_Generate_LE_Meta_Event
 *
 * Generates the HCI LE Meta Event
 *
 *********************************************************************************/
t_error LEeventgen_Generate_LE_Meta_Event(u_int8 event_id,t_LE_Connection* p_connection,t_error status)
{
	
	if(PATCH_FUN[LEEVENTGEN_GENERATE_LE_META_EVENT_ID]){
         
         return ((t_error (*)(u_int8 event_id,t_LE_Connection* p_connection,t_error status))PATCH_FUN[LEEVENTGEN_GENERATE_LE_META_EVENT_ID])(event_id, p_connection,  status);
    }

#if (PRH_BS_CFG_SYS_LE_GAP_INCLUDED!=1)
	t_length metaEventParaLen=0;
	t_length queue_length;
    t_p_pdu p_buffer =0x0;
    struct q_desc* qd;
	u_int8 i;
#endif


	if (!HCeg_Is_Event_Masked_On(LE_META_EVENT))
		return NO_ERROR;
		
	if (!(( 0x01 << (event_id-1) ) & LE_config.event_mask))
		return NO_ERROR;

#if 1//(PRH_BS_CFG_SYS_LE_GAP_INCLUDED!=1)
	// First Determine the length of the data required.
	// Initially we limit this to one event.

	switch(event_id)
	{
	case LE_SUBEVENT_CONNECTION_COMPLETE :
		metaEventParaLen = 19;
		break;

	case LE_SUBEVENT_CONNECTION_UPDATE_COMPLETE :
		metaEventParaLen = 10;
		break;

	case LE_SUBEVENT_READ_REMOTE_USED_FEATURES_COMPLETE :
		metaEventParaLen = 12;
		break;
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
	case LE_SUBEVENT_LONG_TERM_KEY_REQUEST :
		metaEventParaLen = 13;
		break;
#endif
	}

	queue_length = metaEventParaLen + 2;

	qd = BTq_Enqueue(HCI_EVENT_Q, 0, queue_length);

	if(qd)
	{
		p_buffer=qd->data;
	}
	else
	{
		return MEMORY_FULL;
	}

	if (p_buffer == 0)
	{
		return UNSPECIFIED_ERROR;
	}

	*(p_buffer)= LE_META_EVENT;
	*(p_buffer+1)= metaEventParaLen;
	*(p_buffer+2)= event_id; // SubEventCode
	*(p_buffer+3)= status;

	switch(event_id)
	{

	case LE_SUBEVENT_CONNECTION_COMPLETE :
		if (p_connection)
		{

			_Insert_Uint16((p_buffer+4),LEconnection_Determine_Connection_Handle(p_connection));
			*(p_buffer+6)= LEconnection_Get_Role(p_connection);
			*(p_buffer+7)= LEconnection_Get_Peer_Addr_Type(p_connection);

			for (i=0;i<6;i++)
			{
				p_buffer[8+i] = p_connection->peer_address.bytes[i];
			}

			_Insert_Uint16(p_buffer+14, p_connection->connInterval);
			_Insert_Uint16(p_buffer+16, p_connection->latency);
			_Insert_Uint16(p_buffer+18, p_connection->timeout);
			if (LEconnection_Get_Role(p_connection) == SLAVE)
				*(p_buffer+20)= p_connection->sleep_clock_accuracy;
			else
				*(p_buffer+20)= 0x00;
		}
		else
		{   // Event is the result of a direct advertising Timeout
			u_int8* p_direct_address = LEadv_Get_Direct_Adv_Address();

			_Insert_Uint16((p_buffer+4),0x00);
			*(p_buffer+6)= SLAVE;
			*(p_buffer+7)= LEadv_Get_Direct_Adv_Type();

			for (i=0;i<6;i++)
			{
				p_buffer[8+i] = p_direct_address[i];
			}
			_Insert_Uint16(p_buffer+14, 0x00);
			_Insert_Uint16(p_buffer+16, 0x00);
			_Insert_Uint16(p_buffer+18, 0x00);
			*(p_buffer+20)= 0x00;
		}
		break;

	case LE_SUBEVENT_CONNECTION_UPDATE_COMPLETE :
		if (p_connection)
		{
			_Insert_Uint16((p_buffer+4),LEconnection_Determine_Connection_Handle(p_connection));
			_Insert_Uint16((p_buffer+6), p_connection->connInterval);
			_Insert_Uint16((p_buffer+8), p_connection->latency);
			_Insert_Uint16((p_buffer+10), p_connection->timeout);
		}
		break;
#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
	case LE_SUBEVENT_READ_REMOTE_USED_FEATURES_COMPLETE :
		if (p_connection)
		{
			/*
			 * NOTE :- Only the first byte of the 8 byte Feature Set is relevant
			 */

			_Insert_Uint16((p_buffer+4),LEconnection_Determine_Connection_Handle(p_connection));

			*(p_buffer+6) = p_connection->peer_features;

			for(i=0;i<7;i++)
				*(p_buffer+7+i) = 0x00;
		}
		break;
#endif

#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
	case LE_SUBEVENT_LONG_TERM_KEY_REQUEST :
		if(p_connection)
		{
			_Insert_Uint16((p_buffer+3),LEconnection_Determine_Connection_Handle(p_connection));

			for(i=0;i<8;i++)
				*(p_buffer+5+i) = LE_config.random_number[i];
			_Insert_Uint16((p_buffer+13), LE_config.encrypt_diversifier);
		}
		break;
#endif
	}
	BTq_Commit(HCI_EVENT_Q,0);
#endif

	return NO_ERROR;
}

#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)

/*extern	unsigned	int		hardfault_ERR;*/
/********************************************************************************
 * Function :- LEconnection_Encrypt_Payload
 *
 * Description
 * This function uses HW to encrypt a payload from the Queues and copies it back to the
 * Queues.
 *********************************************************************************/

u_int8 LEconnection_Encrypt_Payload(t_LE_Connection* p_connection,u_int8* payload,u_int8 length,u_int8 llid,u_int32 tx_pkt_count)
{
	
	 
    if(PATCH_FUN[LECONNECTION_ENCRYPT_PAYLOAD_ID]){
         
         return ((u_int8 (*)(t_LE_Connection* p_connection,u_int8* payload,u_int8 length,u_int8 llid,u_int32 tx_pkt_count))PATCH_FUN[LECONNECTION_ENCRYPT_PAYLOAD_ID])(p_connection, payload, length, llid, tx_pkt_count);
    }


    u_int32 u_int32_Temp_buffer[12] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	u_int8* temp = (u_int8*)u_int32_Temp_buffer ;
	u_int32 cpu_flags=0;

	/*
	 * Configure the HW for AES encryption of this packet
	 */
	/*if(hardfault_ERR == 1) {
			hardfault_ERR = 2;
	}*/
	if(p_connection->current_security_state & LE_ENCRYPTION_ACTIVE)
	{
		u_int32 debug32_iv[2];

		u_int8* debug8_iv = (u_int8*)debug32_iv;

		u_int8 i;


		for(i=0;i<8;i++)
			debug8_iv[i] = p_connection->IV[8-i-1];

		/*
		 * NOTE Session Key has to be determined from the SDK and LTK prior to encryption.
		 */
		SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);
		HWle_set_aes_mode(0x02); // AES-CCM encryption
		HWle_set_aes_enable(); // AES-Enable Bit
		SYSirq_Interrupts_Restore_Flags(cpu_flags);


		/*
		 * Write the IV and Session Key if this is the first encryption on a link OR
		 * we are in Multipoint.
		 */

		{
			hw_memcpy32((void*)HWle_get_aes_iv_addr(), (void*)debug8_iv, 8);
			hw_memcpy32((void*)HWle_get_aes_key_addr(), (void*)p_connection->session_key, 16);
		}

		LEconnection_Set_HW_Pkt_Counter(tx_pkt_count);

		SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);
		HWle_set_aes_llid(llid);
		HWle_set_aes_pkt_length(length+4);
		SYSirq_Interrupts_Restore_Flags(cpu_flags);

		for (i=0;i<length;i++)
			temp[i] = payload[i];

		for (i=0;i<length;i++)
			payload[i] = temp[i];

		// The +4 is to account for the MIC which will be written in the first
		// four bytes of the AES buffer

		hw_memcpy32((void*)((u_int32)(HWle_get_aes_data_addr()+4)),(void*)temp,length);

		// Last Thing done on the AES

		SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);
		HWle_set_aes_start();
		HWle_set_aes_data_ready();
		SYSirq_Interrupts_Restore_Flags(cpu_flags);

		while(HWle_get_aes_active())
			;

		while (!HWle_get_aes_finished())
			;

		hw_memcpy32((void*)temp,(void*)HWle_get_aes_data_addr(),(length+4));

		SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);
		HWle_clear_aes_enable();
		HWle_set_aes_mode(0x00);
		SYSirq_Interrupts_Restore_Flags(cpu_flags);

		//gf 3 feb Test if had negative side effect 	HWle_set_aes_mode(0x00);
		LE_config.AES_complete = 0x00;


		for(i=0;i<(length);i++)
		{
			payload[i] = temp[i+4];

		}

        payload[length]= temp[0];
        payload[length+1]= temp[1];
		payload[length+2]= temp[2];
		payload[length+3]= temp[3];

		return 1;
	}
	else
	{
		return 0;
	}
}


/********************************************************************************
 * Function :- LEconnection_Decrypt_Incoming_Pdu
 *
 * Description
 * This function takes and entry from the Queues and uses HW to decrypt it. The
 * resultant decrypted data is placed back on the Queues.
 *
 *********************************************************************************/

t_error LEconnection_Decrypt_Incoming_Pdu(t_LE_Connection* p_connection,u_int8* payload,u_int8 length,u_int32 rx_pkt_counter,u_int8 llid)
{
	
	if(PATCH_FUN[LECONNECTION_DECRYPT_INCOMING_PDU_ID]){
         
         return ((t_error (*)(t_LE_Connection* p_connection,u_int8* payload,u_int8 length,u_int32 rx_pkt_counter,u_int8 llid))PATCH_FUN[LECONNECTION_DECRYPT_INCOMING_PDU_ID])(p_connection, payload,  length,  rx_pkt_counter,  llid);
    }


	u_int32 MICinHW32[1];
	u_int8* MICinHW = (u_int8*)MICinHW32;
	u_int8  mic_status = 0;
	u_int32 cpu_flags = 0x00;
    u_int32 u_int32_Temp_buffer[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	u_int8* temp = (u_int8*)u_int32_Temp_buffer ;


	u_int32 debug32_iv[2];

	u_int8* debug8_iv = (u_int8*)debug32_iv;

	u_int8 i;

	for(i=0;i<8;i++)
		debug8_iv[i] = p_connection->IV[8-i-1];

	/*
	 * First thing to do is to Flip the order of the MIC
	 */

	if (!(p_connection->current_security_state & LE_ENCRYPTION_ACTIVE))
    	return NO_ERROR;

	for(i=0;i<4;i++)
		MICinHW[i] = payload[length-4+i];

	temp[0] = MICinHW[0];
	temp[1] = MICinHW[1];
	temp[2] = MICinHW[2];
	temp[3] = MICinHW[3];

	for(i=0;i< (length-4);i++)
		temp[i+4] = payload[i];

	SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);
	HWle_set_aes_mode(0x03); // AES-CCM decryption
	//HWle_set_aes_enable(); // AES-Enable Bit
	SYSirq_Interrupts_Restore_Flags(cpu_flags);


	{
		hw_memcpy32((void*)HWle_get_aes_iv_addr(), (void*)debug8_iv, 8);
		hw_memcpy32((void*)HWle_get_aes_key_addr(), (void*)p_connection->session_key, 16);
	}
    // Optimise to a single write
	SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);
	HWle_set_aes_start();
	HWle_set_aes_llid(llid);
	HWle_set_aes_pkt_length(length);
	LEconnection_Set_HW_Pkt_Counter(rx_pkt_counter);
	HWle_set_aes_enable(); // AES-Enable Bit
	SYSirq_Interrupts_Restore_Flags(cpu_flags);


	hw_memcpy32((void*)HWle_get_aes_data_addr(),(void*)temp,length);

	SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);
	HWle_set_aes_data_ready();
	SYSirq_Interrupts_Restore_Flags(cpu_flags);


	while(HWle_get_aes_active())
		;
	while (!HWle_get_aes_finished())
		;

	hw_memcpy32(temp,(void*)((u_int32*)(HWle_get_aes_data_addr()+4)),(length-4));
    // GF 3 Jan 2014 - CES
    // Dont update the payload unless we had a successfull decrypt -

	mic_status = HWle_get_aes_mic_status();


	/*
	 * Note the HW MIC status is reset when we Clear AES Enable or change the AES Mode
	 * However, always best to do the Local_Disconnect last as it may invoke a callback
	 * to the upper layers.
	 *
	 * Hence we store the mic_status
	 */
	if (mic_status)
	{
		for (i=0;i<(length-4);i++)
			payload[i]= temp[i];
	}

	SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);
	HWle_clear_aes_enable(); // AES-Enable Bit
	HWle_set_aes_mode(0x00);
	SYSirq_Interrupts_Restore_Flags(cpu_flags);

	LE_config.AES_complete = 0x00;

	if (mic_status == 0)
	{
#if 0 // For Debug of MIC failures
		while (1)
		{
			SYSmmi_Display_Numeric(0xECEC);
		}
#endif
		return EC_CONNECTION_TERMINATED_DUE_TO_MIC_FAILURE;
	}
	else
	{
		return NO_ERROR;
	}
}


/********************************************************************************
 * Function :- LEconnection_Set_HW_Pkt_Counter
 *
 * Description
 * Writes the 5 byte packet counter into HW. We only use a 32 bit word to store the
 * packet counter - however this is more than sufficient.
 *
 *********************************************************************************/

void LEconnection_Set_HW_Pkt_Counter(u_int32 pkt_counter)
{

	HWle_set_aes_pktcntr_byte0((u_int8) (pkt_counter & 0x000000FF));
	HWle_set_aes_pktcntr_byte1((u_int8)((pkt_counter & 0x0000FF00) >> 8));
	HWle_set_aes_pktcntr_byte2((u_int8)((pkt_counter & 0x00FF0000) >> 16));
	HWle_set_aes_pktcntr_byte3((u_int8)((pkt_counter & 0xFF000000) >> 24));
	HWle_set_aes_pktcntr_byte4(0x00);

}
#endif

#if (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)

/********************************************************************************
 * Function :- LEconnection_Does_Slave_Link_Exist
 *
 * Description
 * Checks does a slave link exists.
 *
 *********************************************************************************/
/*
boolean LEconnection_Does_Slave_Link_Exist(void)
{
	u_int8 i;

	for(i=0;i< DEFAULT_MAX_NUM_LE_LINKS; i++)
	{
		if(LEconnections_Link_In_Use(i) && (LEconnection_Get_Role((&LE_connections[i])) == SLAVE))
			return TRUE;
	}
	return FALSE;
}*/
#endif

/********************************************************************************
 * Function :- LEconnection_Find_Next_Connection_Event
 *
 * Description
 * Determines the time of the next connection event
 *
 *********************************************************************************/

u_int32  LEconnection_Find_Next_Connection_Event(u_int32 cur_clk)
{
	if(PATCH_FUN[LECONNECTION_FIND_NEXT_CONNECTION_EVENT_ID]){
         
         return ((u_int32  (*)(u_int32 cur_clk))PATCH_FUN[LECONNECTION_FIND_NEXT_CONNECTION_EVENT_ID])(cur_clk);
    }

	
	u_int32 dist_to_next_CE_time = BT_TIMER_OFF ;
	u_int8 next_link=0;
	u_int8 i;

	// Find the first link - get the
	for(i=0;i< DEFAULT_MAX_NUM_LE_LINKS; i++)
	{
		if(LEconnections_Link_In_Use(i) && ((LE_connections[i].next_connect_event_start_time!=BT_TIMER_OFF) && ((LE_connections[i].next_connect_event_start_time - cur_clk) < dist_to_next_CE_time)))
		{
			dist_to_next_CE_time = BTtimer_Safe_Clockwrap(LE_connections[i].next_connect_event_start_time - cur_clk);
			next_link = i;
		}
	}

	LE_config.active_link = next_link;
	LE_config.next_connection_event_time = LE_connections[next_link].next_connect_event_start_time;

	return LE_connections[next_link].next_connect_event_start_time ;
}

/********************************************************************************
 * Function :- LEconnection_Find_Link_Entry
 *
 * Description
 * returns a pointer to a connection using the link_id as Key.
 *
 *********************************************************************************/

t_LE_Connection* LEconnection_Find_Link_Entry(u_int8 link_id)
{
	return &LE_connections[link_id];
}

#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)

/********************************************************************************
 * Function :- LEconnection_Handle_Data_To_Encrypt
 *
 * Description
 * Called from schedular context. This checks if there is L2CAP data on the Queues which
 * needs to be encrypted. If there is it calls LEconnection_Encrypt_Payload(..) to encrypt
 * the data.
 *
 *********************************************************************************/

void LEconnection_Handle_Data_To_Encrypt(void)
{
	
	if(PATCH_FUN[LECONNECTION_HANDLE_DATA_TO_ENCRYPT_ID]){
         ((void (*)(void))PATCH_FUN[LECONNECTION_HANDLE_DATA_TO_ENCRYPT_ID])();
         return ;
    }
#if ((PRH_BS_CFG_SYS_LE_GATT_INCLUDED==1) || (PRH_BS_CFG_SYS_LE_SMP_INCLUDED==1) || (PRH_BS_CFG_SYS_LE_L2CAP_INCLUDED==1))
    // If the Host Layers are included - they encrypt the data prior to placing it on the Queue.
#else
	if (LE_config.encrypt_pending_flag)
	{
		t_q_descr *qd;
		t_LE_Connection* p_connection;

		qd = BTq_LE_Dequeue_Next(L2CAP_OUT_Q, LE_config.device_index_pending_data_encrypt, 31);
		/*
		 * There is data to be encrypted...
		 */
		if ((qd) && ((qd->encrypt_status & LE_DATA_ENCRYPT_PENDING) || 
		((p_connection->current_security_state & LE_ENCRYPTION_ACTIVE)&&(qd->encrypt_status & LE_DATA_NOT_ENCRYPTED))))
		//if ((qd) && ((qd->encrypt_status & LE_DATA_ENCRYPT_PENDING) /*|| (qd->encrypt_status & LE_DATA_NOT_ENCRYPTED)*/))
		{
			u_int32 tx_pkt_count;

			p_connection = LEconnection_Find_Device_Index(LE_config.device_index_pending_data_encrypt);

			if (p_connection->ll_action_pending & LL_ENCRYPTION_TRANSITIONING)
			{
				// Security Transitioning -- Dont Encrypt Data
				// Exit imediately - without modifying qd or p_connection
				return;
			}

			tx_pkt_count = p_connection->enc_tx_pkt_counter;

			/*
			 * Note - Adjust the length by 4 as the space for the MIC is already saved in the Queue.
			 */
			if (p_connection)
			{

				LEconnection_Encrypt_Payload(p_connection,qd->data,(u_int8)(qd->length-4), qd->message_type,tx_pkt_count);
			}

			qd->encrypt_status = LE_DATA_ENCRYPTED;
			qd->enc_pkt_count = tx_pkt_count;
			p_connection->enc_tx_pkt_counter++;

		}
		else
		{
			LE_config.encrypt_pending_flag=0;
		}
	}
#endif
}
#endif

#if	0
/********************************************************************************
 * Function :- LEconnection_Check_Current_Address
 *
 * Description
 * Checks if there is a match to the current device address..
 *
 *********************************************************************************/

u_int8 LEconnection_Check_Current_Address(t_LE_Connection* p_connection,u_int8 RxAdd, u_int8* p_pdu)
{
	//  Checks if there is a match to the current device address..
	//
	u_int8 i;

	if (RxAdd == LEconnection_Get_Own_Addr_Type(p_connection))
	{
		const t_bd_addr* p_bd_addr;

		if (RxAdd == 0x01 /* RANDOM_ADDRESS */)
		{
			p_bd_addr = &LE_config.random_address;
		}
		else
		{
			p_bd_addr = SYSconfig_Get_Local_BD_Addr_Ref();
		}

		for (i=0;i<6;i++)
		{
			if(p_pdu[i] != p_bd_addr->bytes[i])
				return 0;
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

/********************************************************************************
 * Function :- LEconnection_Min_Active_Supervision_Timeout
 *
 * Description
 *  This function cycles through all Active LE links to find the
 *  shortest supervision Timeout
 *
 *********************************************************************************/

t_slots LEconnection_Min_Active_Supervision_Timeout(void)
{

	t_slots smallest_supervision_TO = 0xFFFF;
	t_LE_Connection* p_connection;
	u_int16 max_link_absense_time;
	u_int8 i;


	for(i=0;i< DEFAULT_MAX_NUM_LE_LINKS; i++)
	{
		if(LEconnections_Link_In_Use(i) && (LE_connections[i].active))
		{
			p_connection = &LE_connections[i];
			if ((p_connection->timeout*0x10) > (p_connection->connInterval*4))
				max_link_absense_time = ((p_connection->timeout*0x10) - (p_connection->connInterval*4));
			else
				max_link_absense_time = p_connection->connInterval*3;

			if (max_link_absense_time < smallest_supervision_TO)
				smallest_supervision_TO = max_link_absense_time;
		}
	}
	return smallest_supervision_TO;
}

u_int8 LEconnection_LE_Connected_As_Master(void)
{
	if (LE_config.num_le_links)
	{
		if (LE_config.slave_link_active)
			return 0;
		else
			return 1;
	}
	else
	{
		return 0;
	}

}
#endif
/**********************************************************
 * EXTENDED HCI COMMANDS
 *
 * See prh_bs_LE_test_ctrl_interface.doc
 *
 ***********************************************************/
#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==1)

t_error LEconnetion_TCI_Read_Access_Code(u_int16 handle)
{
	t_LE_Connection* p_connection = LEconnection_Find_P_Connection( handle);

	 t_q_descr *qd;

	 qd = BTq_Enqueue(HCI_EVENT_Q, 0, 12);
	 if(qd)
	 {
		 qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
		 qd->data[1] = 10;
		 qd->data[2] = 1;
		 _Insert_Uint16(&(qd->data[3]), TCI_LE_READ_ACCESS_CODE);
		 qd->data[5] = 0;
		_Insert_Uint16(&(qd->data[6]),LEconnection_Determine_Connection_Handle(p_connection));
		 _Insert_Uint32(&(qd->data[8]),p_connection->access_address);
		 BTq_Commit(HCI_EVENT_Q, 0);
	 }

 	return NO_ERROR;
}


t_error LEconnetion_TCI_Read_Hop_Increment(u_int16 handle)
{
	t_LE_Connection* p_connection = LEconnection_Find_P_Connection( handle);

	 t_q_descr *qd;

	 qd = BTq_Enqueue(HCI_EVENT_Q, 0, 9);
	 if(qd)
	 {
		 qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
		 qd->data[1] = 7;
		 qd->data[2] = 1;
		 _Insert_Uint16(&(qd->data[3]), TCI_LE_READ_HOP_INCREMENT);
		 qd->data[5] = 0;
		_Insert_Uint16(&(qd->data[6]),LEconnection_Determine_Connection_Handle(p_connection));
		 qd->data[8] = p_connection->hop_increment;
		 BTq_Commit(HCI_EVENT_Q, 0);
	 }

 	return NO_ERROR;
}

t_error LEconnection_TCI_Read_Peer_SCA(u_int16 handle)
{
	t_LE_Connection* p_connection = LEconnection_Find_P_Connection(handle);

	t_q_descr *qd;

	qd = BTq_Enqueue(HCI_EVENT_Q, 0, 9);
	if(qd)
	{
		qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
		qd->data[1] = 7;
		qd->data[2] = 1;
		_Insert_Uint16(&(qd->data[3]), TCI_LE_READ_PEER_SCA);
		qd->data[5] = 0;
		_Insert_Uint16(&(qd->data[6]),LEconnection_Determine_Connection_Handle(p_connection));
		qd->data[8] = p_connection->sleep_clock_accuracy;
		BTq_Commit(HCI_EVENT_Q, 0);
	}
	return NO_ERROR;
}


t_error LEconnection_TCI_Read_Session_Key(u_int16 handle)
{
	t_LE_Connection* p_connection = LEconnection_Find_P_Connection(handle);
	u_int8 i;
	t_q_descr *qd;

	qd = BTq_Enqueue(HCI_EVENT_Q, 0, 24);
	if(qd)
	{
		qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
		qd->data[1] = 22;
		qd->data[2] = 1;
		_Insert_Uint16(&(qd->data[3]), TCI_LE_READ_SESSION_KEY);
		qd->data[5] = 0;
		_Insert_Uint16(&(qd->data[6]),LEconnection_Determine_Connection_Handle(p_connection));
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
		for (i=0;i<16;i++)
			qd->data[8+i] = p_connection->session_key[i];
#endif
		BTq_Commit(HCI_EVENT_Q, 0);
	}
	return NO_ERROR;
}


t_error LEconnection_TCI_Set_Transmit_Window_Params(u_int16 transmit_win_offset, u_int8 transmit_win_size)
{
	LE_config.TCI_transmit_win_config = 0x01;
	LE_config.transmit_win_offset = transmit_win_offset;
	LE_config.transmit_win_size = transmit_win_size;

	TCeg_Command_Complete_Event(TCI_LE_SET_TRANSMIT_WINDOW_PARAMS, NO_ERROR);
	return NO_ERROR;
}

t_error LEconnection_TCI_Read_Window_Size_and_Offset(u_int16 handle)
{
	t_LE_Connection* p_connection = LEconnection_Find_P_Connection(handle);
	t_q_descr *qd;

	qd = BTq_Enqueue(HCI_EVENT_Q, 0, 11);

	if(qd)
	{
		qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
		qd->data[1] = 9;
		qd->data[2] = 1;
		_Insert_Uint16(&(qd->data[3]), TCI_LE_ECHO_TRANSMIT_WINDOW_SIZE_AND_OFFSET);
		qd->data[5] = NO_ERROR;
		_Insert_Uint16(&(qd->data[6]),LEconnection_Determine_Connection_Handle(p_connection));
		qd->data[8] = p_connection->transmit_window_size;
		_Insert_Uint16(&(qd->data[9]),p_connection->transmit_window_offset);

		BTq_Commit(HCI_EVENT_Q, 0);
	}

	return NO_ERROR;
}

#endif

u_int32 LEconnection_Adjust_Clock(u_int32 clk,u_int16 IntraSlot)
{
    // Function to be completed -- Part of Slave connection intra-slot resolution.
	//if (IntraSlot > 625)
	//	clk += 4;

	return clk;
}

#endif
