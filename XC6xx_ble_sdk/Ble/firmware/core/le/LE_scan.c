
/***********************************************************************
 *
 * MODULE NAME:    le_scan.c
 * PROJECT CODE:   CEVA Low Energy Single Mode
 * DESCRIPTION:    Low Energy module for handling LE Scanning.
 * MAINTAINER:     Gary Fleming
 * CREATION DATE:  December 2011
 *
 *
 *LICENSE:
 *     This source code is copyright (c) 2011-2012 Ceva Inc.
 *     All rights reserved.
 *
 ***********************************************************************/


#include "sys_config.h"
#include "sys_features.h"
#include "sys_mmi.h"
#include "hc_const.h"
#include "hci_params.h"

#include "le_const.h"
#include "le_link_layer.h"
#include "le_white_list.h"
#include "le_frequency.h"
#include "le_config.h"
#include "le_connection.h"
#include "sys_rand_num_gen.h"
#include "hw_pta.h"
#include "hw_radio.h"
#include "hw_memcpy.h"

#include "bt_timer.h"
#include "le_white_list.h"

#include "hw_lc.h"
#include "hw_le_lc.h"
#include "hw_hab_defs.h"

#include "tc_event_gen.h"

#include "le_scan.h"
#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==1)
#include "tc_const.h"
#endif
#include "lslc_irq.h"
#include "lslc_slot.h"
#include "lmp_config.h"
#include "le_adv.h"
#include "global_val.h"
#include "patch_function_id.h"
#include <math.h>


//extern t_LE_Advertising LE_advertise;

extern void _Insert_Uint16(t_p_pdu p_buffer, u_int16 value_16_bit);
extern void _Insert_Uint32(t_p_pdu p_buffer, u_int32 value_32_bit);
extern void _Insert_Uint24(t_p_pdu p_buffer,u_int32 val24);

extern  t_LE_Config LE_config;
extern  s_int8 last_adv_rx_rssi;
extern  u_int32 u_int32_TX_buffer[];
extern  u_int32 u_int32_RX_buffer[];

extern uint8_t adv_adjust_scan_window_val;
extern uint8_t con_adjust_scan_window_val1;
extern uint8_t con_adjust_scan_window_val2;
extern u_int32 adv_rssi_val;
//t_LE_Scan LE_scan;

#if (PRH_BS_CFG_SYS_TRACE_ADV_VIA_HCI_SUPPORTED==1)
extern u_int8  PRH_rx_adv_dbg_channel;
extern u_int8  PRH_rx_adv_dbg_length;
extern u_int16 PRH_rx_adv_dbg_header;
#endif

#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
extern t_advert_event  advertising_array[PRH_BS_CFG_LE_MAX_DEPTH_ADVERTISING_ARRAY];
extern u_int8 advertising_array_index;
t_advert_event* LEscan_Get_Free_Adv_Event_Pointer(void);
#endif

// Functions of local scope only
void LEscan_Prep_For_LE_Scan_Rx(void);
void _LEscan_Scan_Window_Complete(void);
void _LEscan_Return_Scanning(void);
void _LEscan_Backoff_Failure_Scan_Rsp_Rxed(void);
void _LEscan_End_Initiating(void);
#define DEFAULT_SWITCH_FROM_LE_TO_CLASSIC_GAURD 2

/***************************************************************************
 * Function :- LEscan_Init
 *
 * Description :-
 * This function initialies all the relevant elements of the LE_scan structure. 
 * It also initialises the Duplicate lists and also initialises the advertising 
 * event array ( i.e advertising_array)
 *
 * Context :- Schedular
 ***************************************************************************/

t_error LEscan_Init(void)
{
    if(PATCH_FUN[LESCAN_INIT_ID]){
         
         return ((t_error (*)(void))PATCH_FUN[LESCAN_INIT_ID])();
    }

#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
	u_int8 i;

	LE_scan.scan_enable = 0;
	LE_scan.scan_type = LE_PASSIVE_SCAN; // LE_ACTIVE_SCAN;
	LE_scan.scan_interval = LE_DEFAULT_SCAN_INTERVAL;
	LE_scan.scan_window = LE_DEFAULT_SCAN_WINDOW;
	LE_scan.scanning_channel_index = 37;
	LE_scan.scanning_channels_used = 0;
	LE_scan.scan_own_address_type = PUBLIC_DEVICE_ADDRESS_TYPE; // Default
	LE_scan.scan_filter_policy = 0; // Default
	LE_scan.scan_filter_duplicates = 0x00; // Default - no filtering of duplicates
	LE_scan.next_scan_timer = BT_TIMER_OFF;
#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==1)
	LE_scan.scanning_channel_map = 0x07;
	LE_scan.initiating_channel_map = 0x07;

	// For Orca Radio I have to disable the Scan BackOff
	//LE_scan.backoff_enable = 0x01;
	LE_scan.backoff_enable = 0x00;
#endif
	LEwl_Init_Duplicates_List();
#endif
	LE_scan.num_tims_during_tx = 0;
	LE_scan.num_tims_during_rx = 0;

#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)

	for (i=0;i<PRH_BS_CFG_LE_MAX_DEPTH_ADVERTISING_ARRAY;i++)
	{
		t_advert_event* p_Advert = &advertising_array[i];

		p_Advert->event_type = 0x00;
		p_Advert->write_complete = 0x00;

	}
	advertising_array_index = 0x00;
#endif

	return NO_ERROR;
}


#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
/***************************************************************************
 * Function :- LEscan_Set_Scan_Parameters
 *
 * Description :-
 * This function corresponds to the HCI_LE_Set_Scan_Parameters.
 *
 * The input is the byte-stream of HCI_LE_Set_Scan_Parameters command.
 * The following Scan Parameters are range checked and stored in LE_config.
 *
 *  Scan Interval      - Distance between 2 scan windows
 *  Scan Window        - The width of the scan window.
 *  Scan Type          - ACTIVE | PASSIVE
 *  Scan Address Type  - PUBLIC | RANDOM
 *  Scan Filter Policy.
 *
 * Context :- Schedular
 ***************************************************************************/

#if (PRH_BS_CFG_SYS_LE_GAP_INCLUDED!=1)
t_error LEscan_Set_Scan_Parameters(t_p_pdu p_pdu)
{
    if(PATCH_FUN[LESCAN_SET_SCAN_PARAMETERS_ID]){
         
         return ((t_error (*)(t_p_pdu))PATCH_FUN[LESCAN_SET_SCAN_PARAMETERS_ID])(p_pdu);
    }
	u_int16 scan_interval;
	u_int16 scan_window;
	u_int8  scan_type;
	u_int8  scan_own_address_type;
	u_int8  scan_filter_policy;

	if (LE_scan.scan_enable==0x01)
		return COMMAND_DISALLOWED;

	scan_type = *p_pdu;
	p_pdu++;

	scan_interval = HCIparam_Get_Uint16(p_pdu);
	p_pdu+=2;

	scan_window = HCIparam_Get_Uint16(p_pdu);
	p_pdu+=2;

    //- (start) LC change here for android4.2.
/*
    if(scan_interval == scan_window) 
    {
        scan_interval = 128; 
        scan_window = 18;
    }
    */
    //- (end)
	scan_own_address_type = *p_pdu;
	p_pdu++;

	scan_filter_policy = *p_pdu;
	p_pdu++;

    // Now check the range of the parameters

	if ((scan_interval < 0x0004) || (scan_interval > 0x4000))
		return INVALID_HCI_PARAMETERS;

	if ((scan_window < 0x0004) || (scan_window > 0x4000))
		return INVALID_HCI_PARAMETERS;

	if (scan_interval < scan_window)
		return INVALID_HCI_PARAMETERS;

	if (scan_type > 0x01)
		return INVALID_HCI_PARAMETERS;

	if (scan_own_address_type > 0x01)
		return INVALID_HCI_PARAMETERS;

	if (scan_filter_policy > 0x01)
		return INVALID_HCI_PARAMETERS;


	LE_scan.scan_interval = scan_interval; // 0x1e0; //300MS
	LE_scan.scan_window = scan_window; // 0x30 ; //30MS
	LE_scan.scan_type = scan_type; //scan_type;   //TBD current no scan request send 
	LE_scan.scan_own_address_type = scan_own_address_type;
	LE_scan.scan_filter_policy = scan_filter_policy;

	return NO_ERROR;
}
#endif

/***************************************************************************
 * Function :- LEscan_Set_Scan_Enable
 *
 * Description :-
 * This function corresponds to the HCI_LE_Scan_Enable. It allows scanning to be
 * enabled/disabled in the LE device. It also enables/disables the filtering of duplicates.
 *
 * The scan back-off handling is also initialised in this funtions and the timer for the
 * next scan window is set.
 *
 * Context :- Schedular
 ***************************************************************************/


t_error LEscan_Set_Scan_Enable(u_int8 enable,u_int8 filter_duplicates)
{
    if(PATCH_FUN[LESCAN_SET_SCAN_ENABLE_ID]){
         
         return ((t_error (*)(u_int8 enable,u_int8 filter_duplicates))PATCH_FUN[LESCAN_SET_SCAN_ENABLE_ID])(enable,filter_duplicates);
    }
	if ((enable > 0x01) || (filter_duplicates > 0x01))
		return INVALID_HCI_PARAMETERS;
	if (
#ifndef BLUETOOTH_MODE_LE_ONLY	//no acl link anymore
		(LMconfig_LM_Connected_As_Slave() && enable) || 
#endif
		(LE_scan.scan_enable == enable))
		return COMMAND_DISALLOWED;

	LE_scan.scan_enable = enable;

	if (enable)
	{
		LE_scan.scan_filter_duplicates = filter_duplicates;
		LEwl_Init_Duplicates_List();

		LE_scan.scan_backoff_count = 1;
		LE_scan.scan_upper_limit = 1;

		LE_scan.scan_num_consecutive_scan_resp_failure = 0;
		LE_scan.scan_num_consecutive_scan_resp_success = 0;

		// Set the timer for the next interval -

    	if (LE_scan.next_scan_timer == BT_TIMER_OFF)
    	{
    		u_int32 current_clk = HW_Get_Native_Clk_Avoid_Race();
    		LE_scan.next_scan_timer = current_clk + (LE_scan.scan_interval*2);
    	}
    	else
    	{
    		LE_scan.next_scan_timer += (LE_scan.scan_interval*2);
    	}
		LE_scan.next_scan_timer = BTtimer_Safe_Clockwrap(LE_scan.next_scan_timer);
	}

	return NO_ERROR;
}

#endif


#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
/*****************************************************************
 *  FUNCTION :- LEscan_Scan_Interval_Timeout
 *
 *  Params :-
 *  	current_clk - current value of the native clock.
 *
 *  DESCRIPTION
 *  This function is called when the scan interval times-out.
 *  It sets up the HW for the Scan Window, by writing the following to the HW
 *
 *  	LE Mode - On
 *  	CRC Init - ADVERTISING_ACCESS_ADDRESS
 *      Adv State - 1
 *      Tx Enable - 1
 *
 *  The device state is changed to SCANNING_STATE, and scan state changed to W4_T2_PRE_SCAN
 *  The time for the next scan window is determined.
 *
 *  If a Connection Event is due to occur with the Scan Window, the Connection Event will
 *  take precedence and no Scan window will be setup until after the connection event.
 *
 *  CONTEXT : IRQ
 *****************************************************************/

void LEscan_Scan_Interval_Timeout(u_int32 current_clk)
{
    if(PATCH_FUN[LESCAN_SCAN_INTERVAL_TIMEOUT_ID]){
         
          ((void (*)(u_int32))PATCH_FUN[LESCAN_SCAN_INTERVAL_TIMEOUT_ID])(current_clk);
          return;
	}
    if (LE_scan.scan_enable)
    {


    	if (BTtimer_Is_Expired_For_Time(LE_config.next_connection_event_time, (LE_scan.scan_window*2) + current_clk + 1))
    	{
    		// If the Scan Event is going to overlap with the Connection Event - then
    		// we push the scan event a bit later in time.. Lets say 16 clock ticks after the next connection event time.

    	  	LE_scan.next_scan_timer = BTtimer_Safe_Clockwrap(LE_config.next_connection_event_time + 0x10);

    		return; // Do nothing as Initiating Scan will prevent the connection event.
    	}

		//HW_set_slave(0x00);  //  set as master cause bt clk change to native clk

	    HWle_set_master_mode(); //aes encryption

    	HWle_set_le_mode();
    	HWle_set_crc_init(0x555555);

    	HWle_set_acc_addr(ADVERTISING_ACCESS_ADDRESS);
		*((u_int32 volatile*)0x4002c248) = (u_int32)(ADVERTISING_ACCESS_ADDRESS); // 0x8E89BED6 ((0x8e89b3d6); 
		*((u_int32 volatile*)0x4002c24c) = (u_int32)(0x0);  
		
    	HWle_set_adv_state();

    	// TIFS default is set for the entire scan window and does not need to be
    	// re-written.

    	HWle_set_tifs_default();
    	// GF 30 March -- as the frequency is fixed for the entire width of the Scan window
    	// the Scan channel can be determined here and the Whitening in HW can be initialised.

    	LEscan_Advance_Scan_Frequency();

    	// For SM need to set Win-Ext = 0

    	// Determine the RF frequency to be used for scanning.
    	LE_scan.scan_rf_freq = LEfreq_Map_Channel_Index_2_RF_Freq(LE_scan.scanning_channel_index);

		LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM2_INTR_MSK_MASK);
    	LE_config.state = SCANNING_STATE;

	// Set up the next scanning Rx frequency,
    	HWle_set_le_spi_only();

#if (PRH_SYS_CFG_HARDWARE_ADDRESS_FILTERING_SUPPORTED == 0)

    	//if ((LE_config.state == INITIATING_STATE) || (LEscan_Get_Scan_Type() != LE_PASSIVE_SCAN))
    	if (LEscan_Get_Scan_Type() != LE_PASSIVE_SCAN)
			HWle_set_tx_enable();
    	else
    		HWle_clear_tx_enable();
#else
    	if (LEscan_Get_Scan_Type() != LE_PASSIVE_SCAN)
    		HWle_set_rx_mode(0x01 /* Active Scanning */);
    	else
    		HWle_set_rx_mode(0x02 /* Passive Scanning */);

    	if (LE_scan.scan_filter_policy==0x01)
    		HWle_set_address_filtering();

    	// Clear any immediate filterins as it does not apply to
    	// scanning
    	 HWle_clear_le_immediate_filter_shadow();
#endif
    	//HWradio_LE_Setup_Radio_For_Next_TXRX(RX_START);
        ((void (*)(t_frame_pos const ))PATCH_FUN[HWRADIO_LE_SETUP_RADIO_FOR_NEXT_TXRX])(RX_START);
    	LEscan_Prep_For_LE_Scan_Rx();

    	LE_config.sub_state = W4_SCAN_RX;

		LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM2_INTR_MSK_MASK | JAL_TIM0_INTR_MSK_MASK);

		if (BTtimer_Is_Expired_For_Time(LE_advertise.next_adv_timer, (LE_scan.scan_window*2) + current_clk + 1)) // avoid the adv time and scan time to conflict
		{
			uint16_t temp_scan_window = (LE_advertise.next_adv_timer > current_clk)? (LE_advertise.next_adv_timer - current_clk):(current_clk - LE_advertise.next_adv_timer);
			LE_scan.current_scan_window_timer =  BTtimer_Safe_Clockwrap(LE_scan.next_scan_timer + ((temp_scan_window>2)?(temp_scan_window-2):1));
//			LE_advertise.next_adv_timer =  BTtimer_Safe_Clockwrap(LE_scan.next_scan_timer + (LE_scan.scan_window*2)+1);
		}
		else
		{
			LE_scan.current_scan_window_timer =  BTtimer_Safe_Clockwrap(LE_scan.next_scan_timer + (LE_scan.scan_window*2));
		}
    	// Set the timer for the next interval -
    	LE_scan.next_scan_timer = BTtimer_Safe_Clockwrap(current_clk + (LE_scan.scan_interval*2));

#if (PRH_BS_CFG_SYS_TRACE_LE_DEVICE_STATE_VIA_HCI_SUPPORTED == 1)
    	TCeg_Send_LE_LEDeviceState_Event(SCANNING_STATE,0);
#endif
    }
    else
    {
    	// If Scan is not enabled ensure the timer is set to NULL
    	LE_scan.next_scan_timer = BT_TIMER_OFF;
    }
}
#endif

void LEscan_Update_Initiator_Scan_Freq(void)
{
	LE_scan.scan_rf_freq = LEfreq_Map_Channel_Index_2_RF_Freq(LE_scan.initiating_channel_index);
}


#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1)
/**************************************************************************
 * FUNCTION :- LEscan_Advance_Initiating_Frequency
 *
 * DESCRIPTION
 * Advances the scan channel to the next available Scan channel.
 *************************************************************************/

void LEscan_Advance_Initiating_Frequency(void)
{
    if(PATCH_FUN[LESCAN_ADVANCE_INITIATING_FREQUENCY_ID]){
         
          ((void (*)(void))PATCH_FUN[LESCAN_ADVANCE_INITIATING_FREQUENCY_ID])();
          return;
	}
	// Set the advertising_channel to the first one in the channel map
#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==0)
	if (!(LE_scan.initiating_channels_used & 0x01))
	{
		LE_scan.initiating_channel_index = 37;
		LE_scan.initiating_channels_used |= 0x01;
	}
	else if(!(LE_scan.initiating_channels_used & 0x02))
	{
		LE_scan.initiating_channel_index = 38;
		LE_scan.initiating_channels_used |= 0x02;
	}
	else if(!(LE_scan.initiating_channels_used & 0x04))
	{
		LE_scan.initiating_channel_index = 39;
		LE_scan.initiating_channels_used |= 0x04;
	}

	// If all channels have been used - then reset the used channel map.

	if(LE_scan.initiating_channels_used == 0x07 )
		LE_scan.initiating_channels_used = 0;

#else

	if ((!(LE_scan.initiating_channels_used & 0x01)) && (LE_scan.initiating_channel_map & 0x01))
	{
		LE_scan.initiating_channel_index = 37;
		LE_scan.initiating_channels_used |= 0x01;
	}
	else if((!(LE_scan.initiating_channels_used & 0x02)) && (LE_scan.initiating_channel_map & 0x02))
	{
		LE_scan.initiating_channel_index = 38;
		LE_scan.initiating_channels_used |= 0x02;
	}
	else if((!(LE_scan.initiating_channels_used & 0x04)) && (LE_scan.initiating_channel_map & 0x04))
	{
		LE_scan.initiating_channel_index = 39;
		LE_scan.initiating_channels_used |= 0x04;
	}

	// If all channels have been used - then reset the used channel map.

	if(LE_scan.initiating_channels_used == LE_scan.initiating_channel_map)
		LE_scan.initiating_channels_used = 0;

#endif
}
#endif

#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE == 1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1)
/**************************************************************************
 * FUNCTION :- LEscan_Advance_Scan_Frequency
 *
 * DESCRIPTION
 * Advances the scan channel to the next available Scan channel.
 *************************************************************************/

void LEscan_Advance_Scan_Frequency(void)
{
    if(PATCH_FUN[LESCAN_ADVANCE_SCAN_FREQUENCY_ID]){
         
          ((void (*)(void))PATCH_FUN[LESCAN_ADVANCE_SCAN_FREQUENCY_ID])();
          return;
	}
	// Set the advertising_channel to the first one in the channel map
#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==0)
	if (!(LE_scan.scanning_channels_used & 0x01))
	{
		LE_scan.scanning_channel_index = 37;
		LE_scan.scanning_channels_used |= 0x01;
	}
	else if(!(LE_scan.scanning_channels_used & 0x02))
	{
		LE_scan.scanning_channel_index = 38;
		LE_scan.scanning_channels_used |= 0x02;
	}
	else if(!(LE_scan.scanning_channels_used & 0x04))
	{
		LE_scan.scanning_channel_index = 39;
		LE_scan.scanning_channels_used |= 0x04;
	}

	// If all channels have been used - then reset the used channel map.

	if(LE_scan.scanning_channels_used == 0x07 )
		LE_scan.scanning_channels_used = 0;

#else
	if ((!(LE_scan.scanning_channels_used & 0x01)) && (LE_scan.scanning_channel_map & 0x01))
	{
		LE_scan.scanning_channel_index = 37;
		LE_scan.scanning_channels_used |= 0x01;
	}
	else if((!(LE_scan.scanning_channels_used & 0x02)) && (LE_scan.scanning_channel_map & 0x02))
	{
		LE_scan.scanning_channel_index = 38;
		LE_scan.scanning_channels_used |= 0x02;
	}
	else if((!(LE_scan.scanning_channels_used & 0x04)) && (LE_scan.scanning_channel_map & 0x04))
	{
		LE_scan.scanning_channel_index = 39;
		LE_scan.scanning_channels_used |= 0x04;
	}

	// If all channels have been used - then reset the used channel map.

	if(LE_scan.scanning_channels_used == LE_scan.scanning_channel_map )
		LE_scan.scanning_channels_used = 0;

#endif
}
#ifndef REDUCE_ROM2
/**************************************************************************
 * FUNCTION :- LEscan_Get_Scanning_Channel_Index
 *
 * DESCRIPTION
 * Gets the next scan channel index.
 *************************************************************************/

u_int8 LEscan_Get_Scanning_Channel_Index(void)
{
	return LE_scan.scanning_channel_index;
}

/**************************************************************************
 * FUNCTION :- LEscan_Get_Initiating_Channel_Index
 *
 * DESCRIPTION
 * Gets the next initiating channel index.
 *************************************************************************/

u_int8 LEscan_Get_Initiating_Channel_Index(void)
{
	return LE_scan.initiating_channel_index;
}
#endif

#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)

/**************************************************************************
 * FUNCTION :- LEscan_Genertate_Advertising_Report_Event
 *
 * DESCRIPTION
 * Checks the duplicate filtering to determine if we have already generated an
 * equivalent report. If this is not a duplicate OR duplicate filtering is disabled
 * then an Advertising Report Event is generated
 *************************************************************************/

t_error	LEscan_Genertate_Advertising_Report_Event(u_int8 num_reports,u_int8 event_type,u_int8 addressType,
												  u_int8* p_address,u_int8 lenData,u_int8* p_data,s_int8 rssi)
{
    if(PATCH_FUN[LESCAN_GENERTATE_ADVERTISING_REPORT_EVENT_ID]){
         
         return ((t_error (*)(u_int8 ,u_int8 ,u_int8 ,u_int8* ,u_int8 ,u_int8* ,s_int8 ))PATCH_FUN[LESCAN_GENERTATE_ADVERTISING_REPORT_EVENT_ID])(num_reports,event_type,addressType,
												  p_address,lenData,p_data,rssi);
	}
	boolean generateAdvertisingReport = FALSE;

	if (!(LE_config.event_mask  & 0x0002))
		return NO_ERROR;

	// Prior to reporting the Advertising Event we must check if this device was previously
	// reported during this scanning - see below section from Spec

	/* For each non-duplicate ADV_DIRECT_IND PDU received that contains this
	 * Link Layer’s device address from an advertiser the Link Layer shall send an
	 * advertising report to the Host. For each non-duplicate ADV_IND,
	 * ADV_SCAN_IND, ADV_NONCONN_IND, or SCAN_RSP PDU from an advertiser, the Link
	 * Layer shall send an advertising report to the Host.
	 *
	 * A duplicate advertising report is an advertising report for the same device address
	 * while the Link Layer stays in the Scanning State. The advertising data may change;
	 * advertising data or scan response data is not considered significant when determining
	 * duplicate advertising reports.
	 */
#if (SYS_CFG_LE_FILTER_DUPLICATES_IN_SCHED_CONTEXT==0)
	if (LE_scan.scan_filter_duplicates)
	{
		// Check if the Address pre-exists in list of Duplicates for ADV_DIRECT_EVENT_TYPE
		// If it doesnt exist it is added
		// Function Add Device to Device List -
		//         Returns :-  1 if Device Was Added. Did not pre-exist in list
		//                     0 if Device Was not Added. Pre-existed in list

		if (LEwl_Add_Device_To_Duplicate_List(event_type,addressType,p_address))
		{
			generateAdvertisingReport = TRUE;
		}
	}
	else
#endif
	{
		generateAdvertisingReport = TRUE;
	}
	// First Determine the lenght of the data required.
	// Initially we limit this to one event.

	if (generateAdvertisingReport==TRUE)
	{
		if (event_type <= 5) // Was BUG HERE for ADV)_DIRECT_IND
		{
			u_int8 i;
			t_advert_event *p_Advert_Event;

			p_Advert_Event = LEscan_Get_Free_Adv_Event_Pointer();
			if (p_Advert_Event)
			{
				p_Advert_Event->event_type = event_type;
				p_Advert_Event->addressType = addressType;
				for(i=0;i<6;i++)
					p_Advert_Event->address[i] = p_address[i];
				p_Advert_Event->lenData = lenData;

				for(i=0;i<lenData;i++)
					p_Advert_Event->data[i] = p_data[i];
				p_Advert_Event->rssi = rssi;
				p_Advert_Event->write_complete = 0x01;
			}
		}
		else
		{
			return UNSPECIFIED_ERROR;
		}
	}
	return NO_ERROR;
}
#endif


#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1) || (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1)

/**************************************************************************
 * FUNCTION :- LEscan_Ctrl_Handle_Scanning
 *
 * DESCRIPTION
 * This is the Scanning State Machine. It caters for both Normal Scanning and Initiating 
 * Scanning.
 *
 * W4_T2_PRE_SCAN   This sets up the radio for Scanning in Full RxMode 3. 
 *				  Either the Initiator Window or the Scan Window is 
 *				  used. 
 *
 *	Valid IRQs    Tim 2
 *	Next State    W4_SCAN_RX
 *
 * W4_SCAN_RX     In this state the device is waiting to recieve a packet or 
 *                close the scan window. 
 *
 *				  If a packet is recieved without error the HW and RF are 
 * 			      prepared for a Tx. (this is done in advance of decode to
 *				  facilitate low MIPs platforms.)
 *
 *				  The revieved packet is then decoded, with the decodeing funtcion
 *				  determining if a response is needed and encoding it into the HW
 *				  Tx buffer.
 *
 *				  If a packet is recieved with a CRC error, or the decode function 
 *				  determines no respose is required - then the device is returned to the 
 *				  Scanning.
 *
 *				  If no packet is recieved then this state is exited once the Scan Window 
 *				  expires.
 *
 *
 *	Valid IRQ     Sync_Detect, Tim 2, PKD
 *	Next State    W4_SCAN_TX, W4_SCAN_RX
 *
 * W4_SCAN_TX     In this state the device is waiting for the PKA interrupt which indicates that
 *				  either a SCAN_REQ or CONNECT_REQ PDU has been sent.
 *
 *				  If a SCAN_REQ has been sent we setup the HW and RX for recieve and wait to
 *				  Rx a SCAN_RESP. If a CONNECT_REQ has been sent (i.e. we are in the initiating state)
 *			      then Scanning/Initiating is complete and the Event is closed and Initiating completed.
 *			      The time for the first connection event will be determined and stored prior to exiting 
 *			      this state.
 *
 *   Valid IRQ     PKA
 *   Next State    W4_SCAN_RESP_RX or Event Complete
 *
 * W4_SCAN_RESP_RX  In the state the device is waiting for a SCAN_RESP from the peer. If a SCAN_RESP is recieved
 *                  without CRC it is decoded and the device returns to SCANNING. If no SCAN_RESP is recieved the 
 *				    device returns to SCANNING
 *		  
 *  Valid IRQ     PKD, NO_PKD, Sync_Detect
 *  Next State    W4_SCAN_TX, W4_SCAN_RX
 *************************************************************************/

t_error LEscan_Ctrl_Handle_Scanning(u_int8 IRQ_Type,u_int32 current_clk)
{
	if(PATCH_FUN[LESCAN_CTRL_HANDLE_SCANNING_ID]){
        return ((t_error (*)(u_int8 ,u_int32 ))PATCH_FUN[LESCAN_CTRL_HANDLE_SCANNING_ID])(IRQ_Type,current_clk);
          
   }
	u_int8 length = 40;
	t_p_pdu p_payload = (u_int8*)u_int32_RX_buffer;
#ifndef BLUETOOTH_MODE_LE_ONLY
   	t_slots num_slots_to_classic = LMpol_Check_Slots_To_Next_Classic_High_Priority_Activity(HW_Get_Native_Clk_Avoid_Race());
#endif
	//
	// Use current_clk to determine how far away we are from the next activity in classic.
	// If we have to switch - we need to store the information on the scanning which needs
	// to be configured again if we return.
	//
	// We perform our switch based on the Tim-2.
	// Need to store - the following information
	//
	//    1/ The Scanning Frequency
	//    2/ Time remaining in LE_scan.current_scan_window
	//
	// We need to do the following when we return to scanning.
	//    1/ Setup the Frequency in the Radio.
	//    2/ _LEscan_Return_Scanning(..)

	switch(LE_config.sub_state)
	{
	case W4_RESUME_SCAN_NEXT_TIM :
		switch(IRQ_Type)
		{
		case LE_TIM_2 :
		case LE_TIM_0 :
			_LEscan_Return_Scanning();
			LE_config.sub_state = W4_RESUME_SCAN;
			break;
		}
		break;

	case W4_RESUME_SCAN :
		switch(IRQ_Type)
		{
		case LE_TIM_2 :
		case LE_TIM_0 :

			// Step 2
			// Call Functionality similar to what is invoked in SCAN_timeout
		    {

		    	HWle_set_adv_state();

		    	// TIFS default is set for the entire scan window and does not need to be
		    	// re-written.

		    	HWle_set_tifs_default();
		    	// For SM need to set Win-Ext = 0

		    	// Determine the RF frequency to be used for scanning.
		    	LE_scan.scan_rf_freq = LEfreq_Map_Channel_Index_2_RF_Freq(LE_scan.scanning_channel_index);

				LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM2_INTR_MSK_MASK);
		    	LE_config.state = SCANNING_STATE;

		    	HWle_set_le_spi_only();

		    	//HWradio_LE_Setup_Radio_For_Next_TXRX(RX_START);
                ((void (*)(t_frame_pos const ))PATCH_FUN[HWRADIO_LE_SETUP_RADIO_FOR_NEXT_TXRX])(RX_START);
		    	// Should we clear the Tx Enable.???

		    	HWle_set_adv_state();
		    	HWle_set_tifs_default();
		    	HWle_clear_tifs_abort();
#if (PRH_SYS_CFG_HARDWARE_ADDRESS_FILTERING_SUPPORTED == 0)

    	//if ((LE_config.state == INITIATING_STATE) || (LEscan_Get_Scan_Type() != LE_PASSIVE_SCAN))
		    	if (LEscan_Get_Scan_Type() != LE_PASSIVE_SCAN)
		    		HWle_set_tx_enable();
		    	else
		    		HWle_clear_tx_enable();
#else
		    	if (LEscan_Get_Scan_Type() != LE_PASSIVE_SCAN)
		    		HWle_set_rx_mode(0x01 /* Active Scanning */);
		    	else
		    		HWle_set_rx_mode(0x02 /* Passive Scanning */);

#endif


		    	//HWradio_LE_Service(RADIO_MODE_LE_FULL_RX,LE_scan.scan_rf_freq ,RX_START_FREQ);
                ((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_LE_FULL_RX,LE_scan.scan_rf_freq ,RX_START_FREQ);

		    	HW_set_rx_mode(0x03);

		    	LE_config.sub_state = W4_SCAN_RX;

				LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM2_INTR_MSK_MASK | JAL_TIM0_INTR_MSK_MASK);
		    }

			break;
		}
		break;

	case W4_SCAN_RX :
		if(BTtimer_Is_Expired_For_Time(LE_scan.current_scan_window_timer,current_clk+1))
		{
			if (!HW_get_sync_det_intr())
			{
				//HWradio_DisableAllSpiWrites();
				//HWradio_DisableAllSpiWrites();
		        ((void (*)(void))PATCH_FUN[HWRADIO_DISABLEALLSPIWRITES])();	
				_LEscan_Scan_Window_Complete();
				return NO_ERROR;
			}
		}

		switch(IRQ_Type)
		{
		case LE_SYNC_DET :
			__read_hw_reg32(((volatile unsigned *)(0x4002c000 + 0x300)), adv_rssi_val);
			last_adv_rx_rssi = 10*log((adv_rssi_val&0xfffff)/512.0/512.0)-60;			
			LSLCirq_Disable_All_Tim_Intr_Except(0);
			HWle_set_le_spi_only();

		   // GF :- Below is needed for Passive Scanning
			if ((LEscan_Get_Scan_Type() != LE_PASSIVE_SCAN) || (LE_config.state == INITIATING_STATE))
			{
				//HWradio_Setup_For_TIFS_Event(LE_scan.scan_rf_freq);
				((void (*)(u_int8))PATCH_FUN[HWRADIO_SETUP_FOR_TIFS_EVENT])(LE_scan.scan_rf_freq);
			}

			LE_config.sub_state = W4_SCAN_PKD;
			break;
#if 0 // Not currently used - may be required for SCO/eSCO in scan, currently being
	  // re-tested.
		case LE_TIM_2 :
            //

			if (num_slots_to_classic <= (DEFAULT_SWITCH_FROM_LE_TO_CLASSIC_GAURD ) )
			{
				LE_config.sub_state = W4_SCAN_RESUME;
				LEconfig_Switch_To_BT_Classic();
				LSLCslot_Handle_TIM2();
			}

			break;
#endif

		case LE_TIM_0 :
#ifndef BLUETOOTH_MODE_LE_ONLY	//no need to switch to classic and handle TIM0
			if (num_slots_to_classic <= (DEFAULT_SWITCH_FROM_LE_TO_CLASSIC_GAURD+2) )
			{
				LE_config.sub_state = W4_SCAN_RESUME;

				LEconfig_Switch_To_BT_Classic();
				LSLCslot_Handle_TIM0();
			}
#endif
			break;
		}
		break;

	case W4_SCAN_PKD :
		switch(IRQ_Type)
		{
		case LE_PKD :
		{
			u_int8 filters_match = 0;
			u_int8 tifs_aborted = HWle_get_tifs_abort();

			if (!tifs_aborted)
			{
				//
				//Ordering here is important - Critical that Rx Complete and Pre_For_LE_Scan_Req_Tx are
				//performed as early as possible - prior to the extraction of the incoming PDU for decoding.

		    	HW_set_rx_mode(0x00);
		    	length = HWle_get_rx_length_adv();
#if 1 // The Rx Complete is important.
				//HWradio_LE_RxComplete();
				((void (*)(void))PATCH_FUN[HWRADIO_LE_RXCOMPLETE])();
#endif

			}
			else if (tifs_aborted)
			{
			//	HWle_clear_le_spi_only();
				//HWradio_DisableAllSpiWrites();
				//HWradio_DisableAllSpiWrites();
		        ((void (*)(void))PATCH_FUN[HWRADIO_DISABLEALLSPIWRITES])();	
			//	HWle_clear_tx_enable();
			}


			if (!tifs_aborted && (!HWle_get_crc_err()) && _LE_LL_CHECK_ADV_PKT_LENGTH(length))
			{
#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
				if(LE_config.state == INITIATING_STATE)
				{
					u_int8 pdu_type = HWle_get_rx_pdu_type();
					// If we are in the initiating state, any AdvData is irrelevant - so can be
					// ignored. This allows us to reduce the length - and minimise the MIPs required
					// to extract the data from HW.

					if(pdu_type == ADV_IND)
					{
						length = 6;
					}
					else if (pdu_type == ADV_DIRECT_IND)
					{
						length = 12;
					}
				}
#endif
#if (PRH_SYS_CFG_HARDWARE_ADDRESS_FILTERING_SUPPORTED == 1)
			//	hw_memcpy32((void*)u_int32_RX_buffer, (void*)HWle_get_rx_acl_buf_addr(), length);

				if(LE_config.state == INITIATING_STATE)
				{
					// If white list filtering is activated
					if (LE_config.initiator_filter_policy & 0x01)
					{
						if (HWle_get_rx_filtered_enum() == 0x07)
						{
							// No filter match - in white list
							filters_match = 0x0;
						}
						else // White list match found.
						{
							/* White list is not used to determine which advertiser to connect to.
							 * Peer_Address_Type and Peer_Address shall be used.
							 */
							hw_memcpy32((void*)u_int32_RX_buffer, (void*)HWle_get_rx_acl_buf_addr(), length);
							filters_match = 0x1;
						}
					}
					else // White List not used I have to check against a specific address
					{
						// Immedate Filter Addr was matched
						if (HWle_get_rx_filtered_enum() == 0x06)
						{
							hw_memcpy32((void*)u_int32_RX_buffer, (void*)HWle_get_rx_acl_buf_addr(), length);
							filters_match = 0x1;
						}
					}

				}
				else // Scanning
				{

					// If filters are ON and get a match
					if ((LEscan_Get_Scan_Filter_Policy() & 0x01) && (HWle_get_rx_filtered_enum() == 0x07))
					{
						// No Filter Match
						filters_match = 0x0;
					}
					else
					{
						hw_memcpy32((void*)u_int32_RX_buffer, (void*)HWle_get_rx_acl_buf_addr(), length);
						filters_match = 0x1;
					}
				}
				if(filters_match && (LEll_Decode_Advertising_ChannelPDU((t_p_pdu) p_payload ,last_adv_rx_rssi, length)))
#else
				hw_memcpy32((void*)u_int32_RX_buffer, (void*)HWle_get_rx_acl_buf_addr(), length);
				if(LEll_Decode_Advertising_ChannelPDU((t_p_pdu) p_payload ,last_adv_rx_rssi, length))

#endif
				{
					// We only need to wait for PKA - so Tim interrupts are not important
                    ((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_LE_TIFS_TX_RX, LE_scan.scan_rf_freq ,TX_START_FREQ);
					//HWradio_LE_Service(RADIO_MODE_LE_TIFS_TX_RX, LE_scan.scan_rf_freq ,TX_START_FREQ);
					LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM2_INTR_MSK_MASK | JAL_TIM0_INTR_MSK_MASK);

				    LE_config.sub_state = W4_SCAN_TX;
				}
				else
				{
					_LEscan_Return_Scanning();
				}
#if (PRH_BS_CFG_SYS_TRACE_ADV_VIA_HCI_SUPPORTED==1)
			    TCeg_Send_LE_Adv_Event(PRH_rx_adv_dbg_header,(u_int8*)p_payload, PRH_rx_adv_dbg_length,PRH_rx_adv_dbg_channel, 0 /* Rx */);
#endif
			}
			else // CRC or Length Error
			{
				_LEscan_Return_Scanning();
			}
		}
		break;

		case LE_NO_PKD :
			_LEscan_Backoff_Failure_Scan_Rsp_Rxed();
			LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM2_INTR_MSK_MASK | JAL_TIM0_INTR_MSK_MASK);
			_LEscan_Return_Scanning();
			break;

		}
		break;


	case W4_SCAN_TX :
		switch(IRQ_Type)
		{
		case LE_TIM_0 :
		case LE_TIM_2 :
			// if a full .625 has passed then we need to abort TX because it has
			// failed for some reason.
			if (LE_scan.num_tims_during_tx)
			{
				HW_set_tx_mode(0x00);
				HWle_clear_tx_enable();
				_LEscan_Return_Scanning();
				LE_scan.num_tims_during_tx=0;
			}
			else
			{
				LE_scan.num_tims_during_tx++;
			}
		    break;

		case LE_PKA :
	       	// Have to insert delay before I call the SetFReq
	       	// Need this to ensure that we dont place the Radio in Idle state
	       	// prior to BPKTCTL going low. As BPKTCTL going low is timed as 10us
	       	// after the TIFS.

			// GF 24 July
			// For Orca need 10us Delay here.
			HW_set_tx_mode(0x00);
			HWle_clear_tx_enable();
#if 0 // 19 Nov
			//HWradio_LE_TxComplete();
		    ((void (*)(void))PATCH_FUN[HWRADIO_LE_TXCOMPLETE])();
#endif
			LE_scan.num_tims_during_tx=0;
			LE_scan.num_tims_during_rx=0;
			// GF 24 July
			//HWle_clear_tx_enable();
			if (LE_config.state == SCANNING_STATE)
			{
				HW_set_rx_mode(0x01);
			    //HWradio_LE_Service(RADIO_MODE_LE_TIFS_TX_RX, LE_scan.scan_rf_freq,RX_START_FREQ);
                ((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_LE_TIFS_TX_RX, LE_scan.scan_rf_freq,RX_START_FREQ);

				LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM2_INTR_MSK_MASK | JAL_TIM0_INTR_MSK_MASK);

				LE_config.sub_state = W4_SCAN_RESP_RX;
			}
#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
			else if (LE_config.state == INITIATING_STATE)
			{
				// A Connect_Request is the only valid transmission in the Initiating State.
				// Once it is transmitted - we need to get out of Initiating State and move to
				// the standby state - waiting for the first connection event.
				{

					LEconnection_Setup_First_Connection_Event(LE_config.initiator_link_id,current_clk);
					_LEscan_End_Initiating();
				}

			}
#endif
			break;
		}
		break;

	case W4_SCAN_RESP_RX:
		switch(IRQ_Type)
		{
		case LE_TIM_0 :
		case LE_TIM_2 :
			if (LE_scan.num_tims_during_rx)
			{

				_LEscan_Return_Scanning();
				LE_scan.num_tims_during_rx=0;
			}
			else
			{
				LE_scan.num_tims_during_rx++;
			}
			break;
		case LE_SYNC_DET :
			HWle_clear_le_spi_only();
			//HWradio_DisableAllSpiWrites();
		    //HWradio_DisableAllSpiWrites();
            ((void (*)(void))PATCH_FUN[HWRADIO_DISABLEALLSPIWRITES])();	
			HWle_clear_tx_enable();
			LE_config.sub_state = W4_SCAN_RESP_PKD;
			//last_adv_rx_rssi = HWradio_LE_Read_RSSI();
		    last_adv_rx_rssi = ((s_int8 (*)(void))PATCH_FUN[HWRADIO_LE_READ_RSSI])();
			break;

		case LE_NO_PKD :
			//HWradio_DisableAllSpiWrites();
		    ((void (*)(void))PATCH_FUN[HWRADIO_DISABLEALLSPIWRITES])();	
			_LEscan_Backoff_Failure_Scan_Rsp_Rxed();
			// GF 7 march
			//HWradio_LE_RxComplete();
		    ((void (*)(void))PATCH_FUN[HWRADIO_LE_RXCOMPLETE])();
			_LEscan_Return_Scanning();
			break;
		}
		break;

	case W4_SCAN_RESP_PKD :

		switch(IRQ_Type)
		{
		case LE_PKD :
		{

			length = HWle_get_rx_length_adv();

			// CRC Doesnt Matter I have to transmit
			if ((!LE_LL_Get_CRC_Err()) &&  _LE_LL_CHECK_ADV_PKT_LENGTH(length))
			{
			    hw_memcpy32((void*)u_int32_RX_buffer, (void*)HWle_get_rx_acl_buf_addr(),length);
				LEll_Decode_Advertising_ChannelPDU(p_payload ,last_adv_rx_rssi,length);
				_LEscan_Return_Scanning();
#if (PRH_BS_CFG_SYS_TRACE_ADV_VIA_HCI_SUPPORTED==1)
			    TCeg_Send_LE_Adv_Event(PRH_rx_adv_dbg_header,(u_int8*)p_payload, PRH_rx_adv_dbg_length,PRH_rx_adv_dbg_channel, 0 /* Rx */);
#endif
			}
			else
			{

				_LEscan_Return_Scanning();
			}
		}
		break;

		case LE_NO_PKD :

			//HWradio_DisableAllSpiWrites();
		    //HWradio_DisableAllSpiWrites();
            ((void (*)(void))PATCH_FUN[HWRADIO_DISABLEALLSPIWRITES])();	
			_LEscan_Backoff_Failure_Scan_Rsp_Rxed();
			_LEscan_Return_Scanning();

			break;
		}
		break;


	case W4_SCAN_RESUME:
		/*
		 * This state is applicable when a device is switching from classic back to LE,
         * 1/ Setup the Frequency in the Radio.
	     * 2/ _LEscan_Return_Scanning(..)
	     *
		 */

		_LEscan_Return_Scanning();
		LE_config.sub_state = W4_RESUME_SCAN;
#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1)
		if (LE_config.state == INITIATING_STATE)
			LEconnection_Encode_CONNECT_REQ_pdu();
#endif
		break;
	}
	return NO_ERROR;
}

/**************************************************************************
 * FUNCTION :- _LEscan_Return_Scanning
 *
 * DESCRIPTION
 * Returns the device to Scanning ( Full Rx Mode) after a PDU has been
 * Recieved or Transmitted
 *************************************************************************/

void _LEscan_Return_Scanning(void)
{
  
	if(PATCH_FUN[_LESCAN_RETURN_SCANNING_ID]){
         
          ((void (*)(void))PATCH_FUN[_LESCAN_RETURN_SCANNING_ID])();
		      return;
  }
	// This Return Scanning procedure has been the source of many issues.

	// For the Orca RF the CleanUpRx is critical as if RxComplete is used
	// we can end up with BPKTCTL being left high is some scenarios - and
	// thus crashing the ORCA radio.

	{
		//extern void _HWradioCleanUpRx(void);
		//_HWradioCleanUpRx();
		((void (*)(void))PATCH_FUN[_HWRADIOCLEANUPRX])(); 
	}

	HWle_clear_le_spi_only();

	// Turn off Tx and Rx
	HW_set_rx_mode(0x00);
	// Toggle the TIFS abort bit to end current TIFS count
	HWle_abort_tifs_count();
	HWle_clear_tifs_default();
	HWle_clear_tx_enable();

	LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM0_INTR_MSK_MASK | JAL_TIM2_INTR_MSK_MASK);

	//LE_config.sub_state = W4_RESUME_SCAN_NEXT_TIM;
	LE_config.sub_state = W4_RESUME_SCAN;
	// Not so sure about placing RF in standby more..
   ((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_STANDBY,0,0);
	
	//HWradio_LE_Service(RADIO_MODE_STANDBY,0,0);
#ifndef BLUETOOTH_MODE_LE_ONLY//no pta interface
	HWpta_LE_Access_Complete();
#endif

}


/**************************************************************************
 * FUNCTION :- _LEscan_Scan_Window_Complete
 *
 * DESCRIPTION
 * This is called when a Scan Window is complete. Actions performed depend
 * if we are continous scanning or not.
 *************************************************************************/

void _LEscan_Scan_Window_Complete(void)
{
    if(PATCH_FUN[_LESCAN_SCAN_WINDOW_COMPLETE_ID]){
         
          ((void (*)(void))PATCH_FUN[_LESCAN_SCAN_WINDOW_COMPLETE_ID])();
		  return;
      }
	u_int8 previous_state=0;

	// For the Orca RF the CleanUpRx is critical as if RxComplete is used
	// we can end up with BPKTCTL being left high is some scenarios - and
	// thus crashing the ORCA radio.

	{
		//extern void _HWradioCleanUpRx(void);
		//_HWradioCleanUpRx();
		((void (*)(void))PATCH_FUN[_HWRADIOCLEANUPRX])();
	}

	if ((LE_config.num_le_links) && (LE_config.slave_link_active))
	{
		HW_set_freeze_bit_cnt(0); //  make sure scan coexist with slave connect link, not change the bitcount
	}

	HWle_clear_le_spi_only();

	// Turn off Tx and Rx
	HW_set_rx_mode(0x00);
	// Toggle the TIFS abort bit to end current TIFS count
	HWle_abort_tifs_count();

	HWle_clear_tifs_default();

	HWle_clear_tx_enable();

	LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM0_INTR_MSK_MASK | JAL_TIM2_INTR_MSK_MASK);

	previous_state = LE_config.state;
	LE_config.state = STANDBY_STATE;
	// Not so sure about placing RF in standby more..
	//HWradio_LE_Service(RADIO_MODE_STANDBY,0,0);
    ((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_STANDBY,0,0);

#ifndef BLUETOOTH_MODE_LE_ONLY//no pta interface
	HWpta_LE_Access_Complete();
#endif

	// If advertising has been delayed due to scanning we need to schedule the Next Advertising Immediately
	//	LEadv_Adjust_Advertising_Event_Start_Timer();
#if (PRH_SYS_CFG_HARDWARE_ADDRESS_FILTERING_SUPPORTED == 1)
	HWle_clear_address_filtering();
#endif

#if (PRH_BS_CFG_SYS_TRACE_LE_DEVICE_STATE_VIA_HCI_SUPPORTED == 1)
		TCeg_Send_LE_LEDeviceState_Event(STANDBY_STATE,0);
#endif
	{
	    LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM0_INTR_MSK_MASK |JAL_TIM2_INTR_MSK_MASK | JAL_TIM1_INTR_MSK_MASK |JAL_TIM3_INTR_MSK_MASK );

	   {
	    	u_int32 current_clk = HW_Get_Native_Clk_Avoid_Race();

	    	if ((previous_state == SCANNING_STATE ) && (!BTtimer_Is_Expired_For_Time(LE_scan.next_scan_timer, current_clk)) &&
	    	    ((BTtimer_Clock_Difference(current_clk,LE_scan.next_scan_timer) >> 1) > SYS_CFG_LE_LC_MIN_SLOTS_FOR_SLEEP_PROCEDURE))
	    	{
#ifndef BLUETOOTH_MODE_LE_ONLY
	    		LEconfig_Switch_To_BT_Classic();
#endif
#if (PRH_BS_CFG_SYS_LOW_POWER_MODE_SUPPORTED==1)
#if (__DEBUG_LE_SLEEP == 0)
                if (0 == g_debug_le_sleep)
                {
	    		    LE_LL_Sleep();
                }
#endif                
#endif
	    	}
	    	else if ((previous_state == INITIATING_STATE) && (!BTtimer_Is_Expired_For_Time(LE_config.next_initiator_scan_timer, current_clk)) &&
		    	    ((BTtimer_Clock_Difference(current_clk,LE_config.next_initiator_scan_timer) >> 1) > SYS_CFG_LE_LC_MIN_SLOTS_FOR_SLEEP_PROCEDURE))
	    	{
#ifndef BLUETOOTH_MODE_LE_ONLY
	    		LEconfig_Switch_To_BT_Classic();
#endif
#if (PRH_BS_CFG_SYS_LOW_POWER_MODE_SUPPORTED==1)
#if (__DEBUG_LE_SLEEP == 0)
                if (0 == g_debug_le_sleep)
                {
	    		    LE_LL_Sleep();
                }
#endif                
#endif
	    	}
	   }
	}
}
#endif

/**************************************************************************
 * FUNCTION :- _LEscan_Prep_For_LE_Scan_Rx
 *
 * DESCRIPTION
 * This prepares the device for receipt Scan Rx, The device is placed in Full Rx Mode
 * and the LE HW is configure for the receipt of an advertising PDU.
 *************************************************************************/

void LEscan_Prep_For_LE_Scan_Rx(void)
{
    if(PATCH_FUN[LESCAN_PREP_FOR_LE_SCAN_RX_ID]){
         
          ((void (*)(void))PATCH_FUN[LESCAN_PREP_FOR_LE_SCAN_RX_ID])();
			    return;
      }
    //HW_set_tx_mode(0x00);
	//HWle_abort_tifs_count();


	if ((LE_config.num_le_links) && (LE_config.slave_link_active))   //&& (1 != g_le_enable_central_flag)
	{
		HW_set_freeze_bit_cnt(1); //  make sure scan coexist with slave connect link, not change the bitcount
	}
	else
	{
		HW_set_slave(0x00);  //  set as master cause bt clk change to native clk	
	}

    HWle_set_master_mode();

    // Should we clear the Tx Enable.???

	HWle_clear_tx_enable();
	HWle_set_le_mode();
	HWle_set_crc_init(0x555555);

	// Note Whitening is initialised with the Channel Index NOT the channel number
	// RF Channel 0 = Index 37, RF Channel 12 = Index 38, RF Channel 39 = Index 39

	if (LE_config.state == SCANNING_STATE)
		HWle_set_whitening_init(LE_scan.scanning_channel_index);
	else if (LE_config.state == INITIATING_STATE)
		HWle_set_whitening_init(LE_scan.initiating_channel_index);

	HWle_set_acc_addr(ADVERTISING_ACCESS_ADDRESS);
	*((u_int32 volatile*)0x4002c248) = (u_int32)(ADVERTISING_ACCESS_ADDRESS); // 0x8E89BED6 ((0x8e89b3d6); 
	*((u_int32 volatile*)0x4002c24c) = (u_int32)(0x0);  
	
    HWle_set_adv_state();
    HWle_set_tifs_default();
	HWle_clear_tifs_abort();
#ifndef BLUETOOTH_MODE_LE_ONLY//no pta interface
	HWpta_LE_Access_Request(FALSE, 0, RADIO_MODE_LE_FULL_RX);
#endif
#if 0   
	if (((LEscan_Get_Scan_Type() != LE_PASSIVE_SCAN) && (LE_config.state == SCANNING_STATE)) || (LE_config.state == INITIATING_STATE))
		  (( void (*)(u_int8 mode))PATCH_FUN[HWRADIO_LE_SET_ACTIVE_MODE])(0x01); //HWradio_LE_Set_Active_Mode(0x01);
	else
		(( void (*)(u_int8 mode))PATCH_FUN[HWRADIO_LE_SET_ACTIVE_MODE])(0x00); //HWradio_LE_Set_Active_Mode(0x00); //0x00
#endif
	// for adv /passive scan/active scan/init/ receive
	//HWradio_LE_Set_Active_Mode(0x01);
    (( void (*)(u_int8 mode))PATCH_FUN[HWRADIO_LE_SET_ACTIVE_MODE])(0x01);
    ((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_LE_FULL_RX,LE_scan.scan_rf_freq ,RX_START_FREQ);
    //HWradio_LE_Service(RADIO_MODE_LE_FULL_RX,LE_scan.scan_rf_freq ,RX_START_FREQ);
    HW_set_rx_mode(0x03);/* full rx mode */

}

#endif


#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)

/********************************************************************************
 * After sending a SCAN_REQ PDU the Link Layer shall listen for a SCAN_RSP
 * PDU from that advertiser. If the SCAN_RSP PDU was not received from that
 * advertiser, it is considered a failure otherwise it is considered a success. On
 * every two consecutive failures, the upperLimit shall be doubled until it reaches
 * the value of 256. On every two consecutive successes, the upperLimit shall be
 * halved until it reaches the value of one. After success or failure of receiving the
 * SCAN_RSP PDU, the Link Layer shall set backoffCount to a new pseudo-random
 * integer between one and upperLimit.
 ********************************************************************************/

/**************************************************************************
 * FUNCTION :- LEscan_Ok_To_Send_Scan_Req
 *
 * DESCRIPTION
 * If the Scan Back-off Count = 1 a Scan Request Can be sent. This is part of the
 * scanning back-off Algorithm.
 *************************************************************************/

u_int8 LEscan_Ok_To_Send_Scan_Req(void)
{
	u_int8 ok_to_send = 1;
#if 0 // Disabled for Dual Mode on ORCA - this seems to add too much overhead - must investigate further
#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==1)
	if(LE_scan.backoff_enable)
#endif
	{
		if (LE_scan.scan_backoff_count)
			LE_scan.scan_backoff_count--;

		if(LE_scan.scan_backoff_count!=0)
			ok_to_send = 0;
	}
#endif
	return ok_to_send;
}

/**************************************************************************
 * FUNCTION :- LEscan_Backoff_Successfull_Scan_Rsp_Rxed
 *
 * DESCRIPTION
 * Part of the Scan Backoff Algorithm
 *************************************************************************/

void LEscan_Backoff_Successfull_Scan_Rsp_Rxed(void)
{
	if(PATCH_FUN[LESCAN_BACKOFF_SUCCESSFULL_SCAN_RSP_RXED_ID]){
         
          ((void (*)(void))PATCH_FUN[LESCAN_BACKOFF_SUCCESSFULL_SCAN_RSP_RXED_ID])();
		      return;
      }
	LE_scan.scan_num_consecutive_scan_resp_success++;

	// For every two consequtive SCAN_RSP the upper_limit is halfed until it
	// reaches value of 1.

	if (LE_scan.scan_num_consecutive_scan_resp_success == 0x02)
	{
        // half the upper limit
		LE_scan.scan_upper_limit = (LE_scan.scan_upper_limit >> 1) & 0xFF;
		if (LE_scan.scan_upper_limit == 0)
			LE_scan.scan_upper_limit = 1;

		LE_scan.scan_num_consecutive_scan_resp_success = 0;
		LE_scan.scan_num_consecutive_scan_resp_failure = 0;
	}

	/* After success or failure of receiving the SCAN_RSP PDU, the Link Layer shall
	 * set backoffCount to a new pseudo-random integer between one and upperLimit.
	 */

	LE_scan.scan_backoff_count =(((u_int8)(SYSrand_Get_Rand() & 0xFF)) & (LE_scan.scan_upper_limit-1));
	LE_scan.scan_backoff_count++;
}

/**************************************************************************
 * FUNCTION :- _LEscan_Backoff_Failure_Scan_Rsp_Rxed
 *
 * DESCRIPTION
 * Part of the Scan Backoff Algorithm
 *************************************************************************/

void _LEscan_Backoff_Failure_Scan_Rsp_Rxed(void)
{
	if(PATCH_FUN[_LESCAN_BACKOFF_FAILURE_SCAN_RSP_RXED_ID]){
         
          ((void (*)(void))PATCH_FUN[_LESCAN_BACKOFF_FAILURE_SCAN_RSP_RXED_ID])();
		      return;
      }
	LE_scan.scan_num_consecutive_scan_resp_failure++;

	// For every two consequtive SCAN_RSP failures the upper_limit is doubled until it
	// reaches value of 256.
	if (LE_scan.scan_num_consecutive_scan_resp_failure == 0x02)
	{
        // double the upper limit
		if (LE_scan.scan_upper_limit < 0x81)
			LE_scan.scan_upper_limit = (LE_scan.scan_upper_limit << 1) & 0xFF;
		else
			LE_scan.scan_upper_limit = 0x100;

		LE_scan.scan_num_consecutive_scan_resp_success = 0;
		LE_scan.scan_num_consecutive_scan_resp_failure = 0;
	}

	/* After success or failure of receiving the SCAN_RSP PDU, the Link Layer shall
	 * set backoffCount to a new pseudo-random integer between one and upperLimit.
	 */

	LE_scan.scan_backoff_count =(((u_int8)(SYSrand_Get_Rand() & 0xFF)) & (LE_scan.scan_upper_limit-1));
	LE_scan.scan_backoff_count++;
}
#endif

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)


/**************************************************************************
 * FUNCTION :- _LEscan_End_Initiating
 *
 * DESCRIPTION
 * Cleans up after the end of an initiating Scan Window.
 *************************************************************************/

void _LEscan_End_Initiating(void)
{
	if(PATCH_FUN[_LESCAN_END_INITIATING_ID]){
         
          ((void (*)(void))PATCH_FUN[_LESCAN_END_INITIATING_ID])();
		      return;
  }
	HWle_clear_le_spi_only();

    HW_set_rx_mode(0x00);
	HWle_abort_tifs_count();
	LE_config.state = STANDBY_STATE;

    // Not so sure about placing RF in standby more..
   ((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_STANDBY,0,0);
	//HWradio_LE_Service(RADIO_MODE_STANDBY,0,0);
#ifndef BLUETOOTH_MODE_LE_ONLY//no pta interface
	HWpta_LE_Access_Complete();
#endif
    // After ending initiating - we need to leave Tim0 and Tim2 enabled.
	LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM0_INTR_MSK_MASK | JAL_TIM2_INTR_MSK_MASK);
    // Clear the initiating timers

	LE_config.next_initiator_scan_timer = BT_TIMER_OFF;
    LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM0_INTR_MSK_MASK |JAL_TIM2_INTR_MSK_MASK | JAL_TIM1_INTR_MSK_MASK |JAL_TIM3_INTR_MSK_MASK );
#ifndef BLUETOOTH_MODE_LE_ONLY
    LEconfig_Switch_To_BT_Classic();
#endif

}
#endif

/**************************************************************************
 * FUNCTION :- LEscan_Get_Scan_Filter_Policy
 *
 * DESCRIPTION
 * Returns the Scan Filter Policy in force
 *************************************************************************/


u_int8 LEscan_Get_Scan_Filter_Policy(void)
{
	return LE_scan.scan_filter_policy;
}

/**************************************************************************
 * FUNCTION :- LEscan_Get_Scan_Type
 *
 * DESCRIPTION
 * Returns the current Scan_Type
 *************************************************************************/

u_int8 LEscan_Get_Scan_Type(void)
{
	return LE_scan.scan_type;
}

/**************************************************************************
 * FUNCTION :- LEscan_Is_Next_Scan_Due
 *
 * DESCRIPTION
 * Checks if the next Scan timer has expired
 *************************************************************************/

u_int8 LEscan_Is_Next_Scan_Due(u_int32 current_clk)
{
	return (BTtimer_Is_Expired_For_Time(LE_scan.next_scan_timer, current_clk + 1) && (LE_scan.scan_enable == 1));
}
#ifndef REDUCE_ROM2
/**************************************************************************
 * FUNCTION :- LEscan_Get_Slots_To_Next_Scanning_Timer
 *
 * DESCRIPTION
 * Returns the number of slots to the next scan timer.
 *************************************************************************/

u_int32 LEscan_Get_Slots_To_Next_Scanning_Timer(u_int32 current_clk)
{
	if(LE_scan.scan_enable)
	{
		if (BTtimer_Is_Expired_For_Time(LE_scan.next_scan_timer, current_clk + 1))
			return 0;
		if (LE_config.state == SCANNING_STATE)
			return 0;

		return (BTtimer_Clock_Difference(current_clk,LE_scan.next_scan_timer) >> 1);
	}
	else
		return 0xFFFFFFFF;
}
#endif
/**************************************************************************
 * FUNCTION :- LEscan_Set_Adv_Header
 *
 * DESCRIPTION
 * Sets the header for a SCAN_REQ PDU.
 *************************************************************************/

void LEscan_Set_Adv_Header(u_int8 pdu_type,u_int8 rxAddr)
{
	if (pdu_type==SCAN_REQ)
	{
		LE_LL_Set_Adv_Header(pdu_type,LE_scan.scan_own_address_type, rxAddr, 12);
	}
}

/**************************************************************************
 * FUNCTION :- LEscan_Encode_Own_Address_In_Payload
 *
 * DESCRIPTION
 *
 *************************************************************************/

void LEscan_Encode_Own_Address_In_Payload(u_int8* p_payload)
{

	const t_bd_addr* p_bd_addr;
	u_int8 i;

	if(LE_scan.scan_own_address_type == PUBLIC_ADDRESS)
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
#ifndef REDUCE_ROM2
/**************************************************************************
 * FUNCTION :- LEscan_Get_Scan_Backoff_Count
 *
 * DESCRIPTION
 * Returns the Scan Backoff Count
 *************************************************************************/

u_int8 LEscan_Get_Scan_Backoff_Count(void)
{
	return LE_scan.scan_backoff_count;
}

/**************************************************************************
 * FUNCTION :- LEscan_Get_Scan_Upper_Limit
 *
 * DESCRIPTION
 * Returns the Scan Backoff Upper Limit
 *************************************************************************/

u_int8 LEscan_Get_Scan_Upper_Limit(void)
{
	return LE_scan.scan_upper_limit;
}
#endif

#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)

/**************************************************************************
 * FUNCTION :- LEscan_Get_Free_Adv_Event_Pointer
 *
 * DESCRIPTION
 * Returns the next free pointer for and Advertising Event.
 *************************************************************************/

t_advert_event* LEscan_Get_Free_Adv_Event_Pointer(void)
{
	    if(PATCH_FUN[LESCAN_GET_FREE_ADV_EVENT_POINTER_ID]){
         
         return ((t_advert_event* (*)(void))PATCH_FUN[LESCAN_GET_FREE_ADV_EVENT_POINTER_ID])();
      }
	u_int8 i;
	u_int8 index;
	for(i=0;i<PRH_BS_CFG_LE_MAX_DEPTH_ADVERTISING_ARRAY;i++)
	{
		index = i+advertising_array_index;
		if (index > (PRH_BS_CFG_LE_MAX_DEPTH_ADVERTISING_ARRAY-1))
			index -= PRH_BS_CFG_LE_MAX_DEPTH_ADVERTISING_ARRAY;

		if (advertising_array[index].write_complete == 0x00)
			return &advertising_array[index];
	}
	return 0;
}
#endif
/**************************************************************************
 * FUNCTION :- LEscan_Get_Scan_Enable
 *
 * DESCRIPTION
 * This function returns the scan enable value.
 *************************************************************************/
 u_int8 LEscan_Get_Scan_Enable()
{
	return LE_scan.scan_enable;
}

#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
/**************************************************************************
 * FUNCTION :- LEscan_Check_For_Pending_Advertising_Reports
 *
 * DESCRIPTION
 * Called by the MiniSched this funtion checks if there are pending advertising reports
 * which need to be enqueued.
 *************************************************************************/

 t_error LEscan_Check_For_Pending_Advertising_Reports(void)
 {
	 if(PATCH_FUN[LESCAN_CHECK_FOR_PENDING_ADVERTISING_REPORTS_ID]){
         
         return ((t_error (*)(void))PATCH_FUN[LESCAN_CHECK_FOR_PENDING_ADVERTISING_REPORTS_ID])();
      }
	 t_length metaEventParaLen=0;
	 t_length queue_length;
	 t_p_pdu p_buffer =0x0;
	 u_int8 generateReport = 1;
	 struct q_desc* qd;
	 u_int8 i;

	 if(advertising_array[advertising_array_index].write_complete)
	 {
		 t_advert_event *p_Advert_Event = &advertising_array[advertising_array_index];

#if (SYS_CFG_LE_FILTER_DUPLICATES_IN_SCHED_CONTEXT==1)
		if (LE_scan.scan_filter_duplicates)
		{
			if (LEwl_Add_Device_To_Duplicate_List(p_Advert_Event->event_type,p_Advert_Event->addressType,p_Advert_Event->address))
			{
				generateReport = 1;
			}
			else
			{
				generateReport = 0;
			}
		}
#endif
		if (generateReport==1)
		{
			metaEventParaLen = 12 + p_Advert_Event->lenData ;

			queue_length = metaEventParaLen + 2;

			qd = BTq_Enqueue(HCI_EVENT_Q, 0, queue_length);
			if(qd)
			{
				p_buffer=qd->data;
			}
			else
			{
				// As the Q is full have to discard this Adv_Event
				 advertising_array[advertising_array_index].write_complete = 0x00;
				 advertising_array_index++;
				 if (advertising_array_index == PRH_BS_CFG_LE_MAX_DEPTH_ADVERTISING_ARRAY)
					 advertising_array_index -= PRH_BS_CFG_LE_MAX_DEPTH_ADVERTISING_ARRAY;
				return MEMORY_FULL;
			}

			if (p_buffer == 0)
			{
				return UNSPECIFIED_ERROR;
			}

			*(p_buffer)= LE_META_EVENT;
			*(p_buffer+1)= metaEventParaLen;
			*(p_buffer+2)= LE_SUBEVENT_ADVERTISING_REPORT; // SubEventCode
			*(p_buffer+3)= 1;
			*(p_buffer+4)= p_Advert_Event->event_type;
			*(p_buffer+5)= p_Advert_Event->addressType;
			for (i=0;i<6;i++)
			{
				p_buffer[6+i] = p_Advert_Event->address[i];
			}
			*(p_buffer+12)=  p_Advert_Event->lenData;
			for (i=0;i< p_Advert_Event->lenData;i++)
			{
				p_buffer[13+i] =  p_Advert_Event->data[i];
			}
			p_buffer[13+ p_Advert_Event->lenData] =  p_Advert_Event->rssi;

			BTq_Commit(HCI_EVENT_Q,0);
		 }
		 advertising_array[advertising_array_index].write_complete = 0x00;

		 advertising_array_index++;
		 if (advertising_array_index == PRH_BS_CFG_LE_MAX_DEPTH_ADVERTISING_ARRAY)
			 advertising_array_index -= PRH_BS_CFG_LE_MAX_DEPTH_ADVERTISING_ARRAY;
	 }
	 return NO_ERROR;
}

 /**************************************************************************
  * FUNCTION :- LEscan_Get_Scanning_Own_Address_Type
  *
  * DESCRIPTION
  * This function returns the scan address type used by the device.
  *************************************************************************/

 u_int8 LEscan_Get_Scanning_Own_Address_Type(void)
 {
	 return LE_scan.scan_own_address_type;
 }

 void LEscan_Set_Current_Window_Timer(t_timer end_scan_window)
 {
 	LE_scan.current_scan_window_timer = end_scan_window;
 }

#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==1)
 /**************************************************************************
 *
 *  TCI FUNCTIONS
 *
 *  The following are a set of TCI functions which have proven usefull in the Debug
 *  of Scanning and Radio. These are mainly intended for development and not intended
 *  to be included in PRODUCT.
 *************************************************************************/

 
/**************************************************************************
 * FUNCTION :- LEscan_TCI_Write_Scan_Freqs
 *
 * DESCRIPTION
 * This is the handler for TCI Command to write a new set of scanning frequencies
 * which are to be used for scanning
 *************************************************************************/

 t_error LEscan_TCI_Write_Scan_Freqs(u_int8 freqs)
 {
 	LE_scan.scanning_channel_map = freqs;
 	TCeg_Command_Complete_Event(TCI_LE_WRITE_SCAN_FREQUENCIES, NO_ERROR);
 	return NO_ERROR;
 }

 
/**************************************************************************
 * FUNCTION :- LEscan_TCI_Write_Initiating_Freqs
 *
 * DESCRIPTION
 * This is the handler for TCI Command to write a new set of initiating frequencies
 * which are to be used for initiating.
 *************************************************************************/

 t_error LEscan_TCI_Write_Initiating_Freqs(u_int8 freqs)
 {
 	LE_scan.initiating_channel_map = freqs;
 	TCeg_Command_Complete_Event(TCI_LE_WRITE_INITIATING_FREQUENCIES, NO_ERROR);
 	return NO_ERROR;
 }

 
/**************************************************************************
 * FUNCTION :- LEscan_TCI_Read_Scan_Freqs
 *
 * DESCRIPTION
 * This is the handler for TCI Command to read the set of  frequencies
 * which are used for scanning.
 *************************************************************************/

 t_error LEscan_TCI_Read_Scan_Freqs(void)
 {
	 t_q_descr *qd;

	 qd = BTq_Enqueue(HCI_EVENT_Q, 0, 7);
	 if(qd)
	 {
		 qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
		 qd->data[1] = 5;
		 qd->data[2] = 1;
		 _Insert_Uint16(&(qd->data[3]), TCI_LE_READ_SCAN_FREQUENCIES);
		 qd->data[5] = 0;
		 qd->data[6] = LE_scan.scanning_channel_map;
		 BTq_Commit(HCI_EVENT_Q, 0);
	 }

  	return NO_ERROR;
 }

 
/**************************************************************************
 * FUNCTION :- LEscan_TCI_Read_Initiating_Freqs
 *
 * DESCRIPTION
 * This is the handler for TCI Command to read the set of frequencies
 * which are used for initiating.
 *************************************************************************/

t_error LEscan_TCI_Read_Initiating_Freqs(void)
{
	t_q_descr *qd;
	qd = BTq_Enqueue(HCI_EVENT_Q, 0, 7);
	if(qd)
	{
		qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
		qd->data[1] = 5;
		qd->data[2] = 1;
		_Insert_Uint16(&(qd->data[3]), TCI_LE_READ_INITIATING_FREQUENCIES);
		qd->data[5] = 0;
		qd->data[6] = LE_scan.initiating_channel_map;
		BTq_Commit(HCI_EVENT_Q, 0);
	}
	return NO_ERROR;
}


/**************************************************************************
 * FUNCTION :- LEscan_TCI_Enable_Scan_Backoff
 *
 * DESCRIPTION
 * Allows the user to ENABLE | DISABLE the scan backoff freature
 *************************************************************************/

t_error LEscan_TCI_Enable_Scan_Backoff(u_int8 backoff_enable)
{
	LE_scan.backoff_enable = backoff_enable;
	return NO_ERROR;
}

/**************************************************************************
 * FUNCTION :- LEscan_TCI_Read_Scan_Backoff_Info
 *
 * DESCRIPTION
 * This TCI reads the Upper Limit and Backoff Count.
 *************************************************************************/

t_error LEscan_TCI_Read_Scan_Backoff_Info(void)
{
	t_q_descr *qd;
	qd = BTq_Enqueue(HCI_EVENT_Q, 0, 8);
	if(qd)
	{
		qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
		qd->data[1] = 6;
		qd->data[2] = 1;
		_Insert_Uint16(&(qd->data[3]), TCI_LE_READ_SCAN_BACKOFF_INFO);
		qd->data[5] = 0;
		qd->data[6] = LE_scan.scan_backoff_count;
		qd->data[7] = LE_scan.scan_upper_limit & 0xFF;
		BTq_Commit(HCI_EVENT_Q, 0);
	}
	return NO_ERROR;
}

/**************************************************************************
 * FUNCTION :- LEscan_TCI_Read_Scan_Params
 *
 * DESCRIPTION
 * This TCI reads the parameters which are currently used for scanning
 *************************************************************************/

t_error LEscan_TCI_Read_Scan_Params(void)
{
	t_q_descr *qd;
	qd = BTq_Enqueue(HCI_EVENT_Q, 0, 14);
	if(qd)
	{
		qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
		qd->data[1] = 12;
		qd->data[2] = 1;
		_Insert_Uint16(&(qd->data[3]), TCI_LE_READ_SCAN_PARAMS);
		qd->data[5] = 0;
		qd->data[6] = LE_scan.scan_type;
		_Insert_Uint16(&(qd->data[7]), LE_scan.scan_interval);
		_Insert_Uint16(&(qd->data[9]), LE_scan.scan_window);
		qd->data[11] = LE_scan.scan_own_address_type;
		qd->data[12] = LE_scan.scan_filter_policy;
		qd->data[13] = LE_scan.scan_filter_duplicates;
		BTq_Commit(HCI_EVENT_Q, 0);
	}
	return NO_ERROR;
}
#endif

#endif

#ifndef REDUCE_ROM2
u_int32 lcm(u_int32 a,u_int32 b)
{
	u_int32 i;
	if(a<=0||b<=0)
	return -1;
	for(i=1;i<b;i++)
	{
		if(i*a%b==0)
		return i*a;
	}
	return b*a;
}
#endif
/*
connect packet(no more data) with scan coexist:
|-------------------------connect interval---------------|
|-connect packet-|---------------------------------------|-connect packet-|
|------2.5ms delay--|-----------scan window------|-guard-|------2.5ms delay--|-----------scan window----------|-guard-|
                    |-------------------------scan interval------------------|
*/
#define FIXED_SCAN_INTERVAL   1
#ifndef REDUCE_ROM
void LE_Connection_Adjust_scan_timer(u_int32 current_connect_event_end_timer, u_int32 next_connect_event_start_timer, u_int32 connInterval)
{
        //u_int32 new_scan_interval = lcm(LE_scan.scan_interval, connection_interval);
       //LE_scan.scan_interval = connection_interval; //0x320; //0x780; //(new_scan_interval < 0x780 ) ? new_scan_interval: 0x780;  //0x1e0; //connection_interval;
#if (FIXED_SCAN_INTERVAL == 1)
	//LE_scan.scan_interval = 0x1e0;
	//LE_scan.scan_window  = 0x30
	//LE_scan.next_scan_timer = BTtimer_Safe_Clockwrap(adv_timer + 8);
#else
	if (connInterval < 0X50)   // connInterval: 80*2*0.625= 100ms
	{
		LE_scan.scan_window  = 0x10; // 10ms
		LE_scan.scan_interval = 0xA0; //100ms
		LE_scan.next_scan_timer = BTtimer_Safe_Clockwrap(current_connect_event_end_timer + 2);		
	}
	else
	{
		LE_scan.scan_window  = (next_connect_event_start_timer - current_connect_event_end_timer - 2)>>1; //&0xFFFF;
		LE_scan.scan_interval = connInterval;
		LE_scan.next_scan_timer = BTtimer_Safe_Clockwrap(current_connect_event_end_timer + 2);		
	}

#endif	


#if 0
DMA_buf[0]=LE_scan.scan_interval; //LE_scan.scan_interval;
DMA_buf[1]=LE_scan.scan_interval>>8; //LE_scan.scan_interval>>8;
DMA_buf[2]=LE_scan.scan_window;
DMA_buf[3]=0xa2;
DMA_Uart_SendChar(1);
#endif
		return;
}
#endif

/*
adv interval: 48*0.625 = 30ms
adv window:
scan interval: 48*0.625 = 30ms
scan window: 30ms - 8*0.625 delay - 4*0.625 = 22.5ms

connection interval:
connection window:

connect packet(no more data) with scan coexist:
|-------------------------connect interval---------------|
|-connect packet-|---------------------------------------|-connect packet-|
|------2.5ms delay--|-----------scan window------|-guard-|------2.5ms delay--|-----------scan window----------|-guard-|
                    |-------------------------scan interval------------------|
*/

void LE_LL_Adjust_scan_timer(u_int32 connection_timer, u_int32 connection_interval, u_int32 connection_window_size)
{
      if(PATCH_FUN[LE_LL_ADJUST_SCAN_TIMER_ID]){
         
          ((void (*)(u_int32 ,u_int32 ,u_int32))PATCH_FUN[LE_LL_ADJUST_SCAN_TIMER_ID])(connection_timer,connection_interval,connection_window_size);
		     return;
      }
#if (FIXED_SCAN_INTERVAL == 1)
	//LE_scan.scan_interval = 0x1e0;
	//LE_scan.scan_window  = 0x30
	//LE_scan.next_scan_timer = BTtimer_Safe_Clockwrap(adv_timer + 8);
	if (connection_interval < 32)   // connInterval: 32*2*0.625= 40ms
	{
//		LE_scan.scan_interval = connection_interval; //0x780; //(new_scan_interval < 0x780 ) ? new_scan_interval: 0x780;	//0x1e0; //connection_interval;
		LE_scan.scan_window   = con_adjust_scan_window_val1; //connection_interval - 8 -4;
//		LE_scan.next_scan_timer = BTtimer_Safe_Clockwrap(connection_timer + connection_window_size + 0x8);
	}
	else
	{
		//LE_scan.scan_interval = 32; //0x780; //(new_scan_interval < 0x780 ) ? new_scan_interval: 0x780;	//0x1e0; //connection_interval;
		LE_scan.scan_window   = con_adjust_scan_window_val2; 
	}
#else
	 //u_int32 new_scan_interval = lcm(LE_scan.scan_interval, connection_interval);
	if (connection_interval > 0X20)   // connInterval: 32*2*0.625= 40ms
	{
		LE_scan.scan_interval = connection_interval; //0x780; //(new_scan_interval < 0x780 ) ? new_scan_interval: 0x780;	//0x1e0; //connection_interval;
		LE_scan.scan_window   = 0x20; //connection_interval - 8 -4;
		LE_scan.next_scan_timer = BTtimer_Safe_Clockwrap(connection_timer + connection_window_size + 0x8);
	}
	else
	{
	}
#endif

#if 0
DMA_buf[0]=connection_interval; //LE_scan.scan_interval;
DMA_buf[1]=connection_interval>>8; //LE_scan.scan_interval>>8;
DMA_buf[2]=connection_window_size;
DMA_buf[3]=0xa2;
DMA_Uart_SendChar(1);
#endif
		return;
}

/*
adv interval: 48*0.625 = 30ms
adv window:
scan interval: 48*0.625 = 30ms
scan window: 30ms - 8*0.625 delay - 4*0.625 = 22.5ms

connection interval:
connection window:

adv with scan coexist:
|-------------------------adv interval------------------|
|-adv1-adv2-adv3|---------------------------------------|-adv1-adv2-adv3|
|--------5ms delay--|-----------scan window-----|-guard-|--------5ms delay--|-----------scan window----------|-guard-|
                    |-------------------------scan interval-----------------|
*/

void LE_Adv_Adjust_scan_timer(u_int32 adv_timer, u_int32 adv_interval)
{
      if(PATCH_FUN[LE_ADV_ADJUST_SCAN_TIMER_ID]){
         
          ((void (*)(u_int32,u_int32))PATCH_FUN[LE_ADV_ADJUST_SCAN_TIMER_ID])(adv_timer,adv_interval);
		    return;
      }
#if (FIXED_SCAN_INTERVAL == 1)
	LE_scan.scan_interval = adv_interval;
	LE_scan.scan_window   = adv_adjust_scan_window_val; // adv_interval - 8 -2
	LE_scan.next_scan_timer = BTtimer_Safe_Clockwrap(adv_timer + 16);
#else
	LE_scan.scan_interval = adv_interval;
	LE_scan.scan_window   = adv_interval>>1; // adv_interval - 8 -2
	LE_scan.next_scan_timer = BTtimer_Safe_Clockwrap(adv_timer + 8);
#endif

#if 0
DMA_buf[0]=LE_scan.scan_interval;
DMA_buf[1]=LE_scan.scan_interval>>8;
DMA_buf[2]=adv_interval;
DMA_buf[3]=0xa3;
DMA_Uart_SendChar(1);
#endif
		return;
}

