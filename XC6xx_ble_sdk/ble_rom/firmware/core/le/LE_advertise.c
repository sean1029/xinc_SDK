/***********************************************************************
 *
 * MODULE NAME:    le_advertise.c
 * PROJECT CODE:   CEVA Low Energy Single Mode
 * DESCRIPTION:    Low Energy Advertising file.
 * MAINTAINER:     Gary Fleming
 * CREATION DATE:  September 2011
 *
 *
 * LICENSE:
 *     This source code is copyright (c) 2011-2012 Ceva Inc.
 *     All rights reserved.
 *
 ***********************************************************************/

#include "sys_config.h"
#include "sys_features.h"
#include "sys_mmi.h"
#include "hc_const.h"
#include "bt_timer.h"

#include "bt_mini_sched.h"

#include "le_const.h"
#include "le_link_layer.h"
#include "le_white_list.h"
#include "le_frequency.h"
#include "le_config.h"
#include "le_white_list.h"
#include "le_advertise.h"

#include "sys_rand_num_gen.h"

#include "hw_pta.h"
#include "hw_radio.h"
#include "hw_lc.h"
#include "hw_le_lc.h"
#include "hw_memcpy.h"

#include "tc_const.h"
#include "tc_event_gen.h"


#include "lslc_irq.h"
#include "le_adv.h"
#include "le_power.h"
#include "sys_debug_config.h"
#include "global_val.h"
#include "patch_function_id.h"

extern void _Insert_Uint16(t_p_pdu p_buffer, u_int16 value_16_bit);
extern void _Insert_Uint32(t_p_pdu p_buffer, u_int32 value_32_bit);
extern void _Insert_Uint24(t_p_pdu p_buffer,u_int32 val24);
extern t_LE_Config LE_config;

 //u_int32 u_int32_TX_buffer[12];
 //u_int32 u_int32_RX_buffer[12];
 extern uint32_t adv_no_pka_count;
 extern u_int32 pkd_error_cnt;
 extern uint8_t adv_always_wake;

#if(__DEBUG_RX_DATA_DUMP_TO_SHRAM0 == 1)
u_int32		pkd_ok_cnt = 0;
u_int32		pkd_crc_cnt = 0;
#endif

#if (PRH_BS_CFG_SYS_LE_BROADCAST_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
static t_error _LEadv_Set_Next_Advertising_Event_Start_Timer(void);
#endif

void _LEadv_Advertising_Event_Complete(u_int32 current_clk);

//t_LE_Advertising LE_advertise;
#if (__DEBUG_LE_SLEEP == 1)
void set_adv_next_timer(unsigned int nxt_adv_timer)
{
	LE_advertise.next_adv_timer = nxt_adv_timer;
}
#endif

t_error LEadv_Init(void)
{

    if(PATCH_FUN[LEADV_INIT_ID]){
         
         return ((t_error (*)(void))PATCH_FUN[LEADV_INIT_ID])();
    }
#if (PRH_BS_CFG_SYS_LE_BROADCAST_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
	LE_advertise.advertising_tx_power_level = MAX_POWER_LEVEL_UNITS;
	// Initialise the Advertising and Scan Data to  NULL
	LE_advertise.advertising_data_len = 0;
	/*LE_advertise.scan_response_data_len = 0;*/
	LE_advertise.advertise_enable = 0;
	LE_advertise.advertising_interval = LE_DEFAULT_MIN_ADVERTISING_INTERVAL;
	LE_advertise.advertising_type = ADV_IND_TYPE; // The Default Type
	LE_advertise.advertising_own_address_type = PUBLIC_DEVICE_ADDRESS_TYPE; // Default
	LE_advertise.advertising_channel_map = 0x07; // All advertising channels enabled
	LE_advertise.advertising_filter_policy = 0x00; // Default
	LE_advertise.advertising_channel_index = 37;
	LE_advertise.advertising_channels_used = 0;
	LE_advertise.next_adv_timer  = (t_timer)BT_TIMER_OFF;
#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==1)
	LE_advertise.user_direct_adv_timeout = 0;
	LE_advertise.direct_adv_timeout = 0;
	LE_advertise.advertising_delta = ADV_DELTA_DEFINE;//0;
	LE_advertise.delta_count = 0;
#endif
    LE_advertise.direct_advertising_timer = (t_timer)BT_TIMER_OFF; 
#endif
	return NO_ERROR;
}

/**************************************************************************
 * FUNCTION :- LEadv_Get_Advertising_Own_Address_Type
 *
 * DESCRIPTION
 * Returns a devices own address type
 *
 **************************************************************************/

u_int8 LEadv_Get_Advertising_Own_Address_Type(void)
{
	return LE_advertise.advertising_own_address_type;
}


#if (PRH_BS_CFG_SYS_LE_BROADCAST_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
/**************************************************************************
 * FUNCTION :- LEadv_Set_Advertising_Params
 *
 * DESCRIPTION
 * Corresponds to the HCI_LE_Set_Advertising_Parameters command. Sets the following
 * parameters advertising.
 *
 *	Advertising Interval
 *	Advertising Type  ( Adv_Ind, Adv_Direct_Ind .... etc ..)
 *	Advertising Address Type   ( PUBLIC | RANDOM )
 *	Direct Address Type  ( PUBLIC | RANDOM )
 *	Direct Address
 *	Advertising Channel Map
 *	Advertising Filter Policy
 *
 **************************************************************************/
#if (PRH_BS_CFG_SYS_LE_GAP_INCLUDED!=1)
t_error LEadv_Set_Advertising_Params(t_p_pdu p_pdu)
{
	if(PATCH_FUN[LEADV_SET_ADVERTISING_PARAMS_ID]){
         
         return ((t_error (*)(t_p_pdu p_pdu))PATCH_FUN[LEADV_SET_ADVERTISING_PARAMS_ID])(p_pdu);
    }

	u_int16 interval_min;
	u_int16 interval_max;
	u_int8  advertising_type;
	u_int8  own_address_type;
	u_int8  direct_address_type;
	u_int8  direct_address[6];
	u_int8  advertising_channel_map;
	u_int8  advertising_filter_policy;
	u_int8  advertising_tx_power_level;
	u_int8 i;

	if (LE_advertise.advertise_enable)
		return COMMAND_DISALLOWED;

	interval_min = HCIparam_Get_Uint16(p_pdu);
	p_pdu+=2;
	interval_max = HCIparam_Get_Uint16(p_pdu);
	p_pdu+=2;
	advertising_type = *p_pdu;
	p_pdu++;
	own_address_type = *p_pdu;
	p_pdu++;
	direct_address_type = *p_pdu;
	p_pdu++;

	for (i=0;i<6;i++)
	{
		direct_address[i]= p_pdu[i];
	}

	p_pdu+=6;
	advertising_channel_map = *p_pdu;
	p_pdu++;
	advertising_filter_policy = *p_pdu;
	p_pdu++;
	advertising_tx_power_level = *p_pdu;
	p_pdu++;


	//
	// Now check that all parameters have a valid range.
	//

	if (advertising_type != 0x01 /* Adv_Direct_Ind - High Duty*/)
	{
		if ((interval_min < 0x0020) || (interval_min > 0x4000))
			return INVALID_HCI_PARAMETERS;

		if ((interval_max < 0x0020) || (interval_max > 0x4000))
			return INVALID_HCI_PARAMETERS;

		if (interval_min > interval_max)
			return INVALID_HCI_PARAMETERS;
	}
#if (PRH_BS_CFG_SYS_LE_LOW_DUTY_DIRECT_ADVERTISING==1)
	if (advertising_type > 0x04) // ADV_DIRECT_IND Low Duty
		return INVALID_HCI_PARAMETERS;
#else
	if (advertising_type > 0x03) // ADV_DIRECT_IND Low Duty
		return INVALID_HCI_PARAMETERS;
#endif
	// If the Advertising Event Type is either Discoverable undirected event type or
	// a non-connectable unidirected event type, the advInterval shall not be less than
	// 100ms.

	if ((advertising_type == 0x02) || (advertising_type == 0x03))
	{
		if ((interval_min < 0x0030) || (interval_max < 0x0030))
			return INVALID_HCI_PARAMETERS;
	}


	if (own_address_type > 0x01)
		return INVALID_HCI_PARAMETERS;

	if (direct_address_type > 0x01)
		return INVALID_HCI_PARAMETERS;

	if ((advertising_channel_map == 0)||(advertising_channel_map > 0x07))
		return INVALID_HCI_PARAMETERS;

	if (advertising_filter_policy > 0x03)
		return INVALID_HCI_PARAMETERS;

	// For Now - until resource management is in place choose the Min-Interval
	LE_advertise.advertising_interval = interval_min;

	// Note the Advertising Type parameter passed at the HCI level is not equivalent
	// to that used in the LE link level. This needs to be converted.
	// HCI                 				Link Level PDU
	// ADV_IND = 0x00      				ADV_IND = 0x00
	// ADV_DIRECT_IND (HD)= 0x01        ADV_DIRECT_IND = 0x01
	// ADV_SCAN_IND = 0x02              ADV_NONCONN_IND = 0x02
	// ADV_NONCONN_IND = 0x03           ADV_SCAN_IND = 0x06
	// ADV_DIRECT_IND (LD) = 0x04

	if (advertising_type == 0x02)
		LE_advertise.advertising_type = 0x06; // ADV_SCAN_IND
	else if (advertising_type == 0x03)
		LE_advertise.advertising_type = 0x02; // ADV_NONCONN_IND
	else
		LE_advertise.advertising_type = advertising_type; // ADV_IND or ADV_DIRECT_IND

	LE_advertise.advertising_own_address_type = own_address_type;
#if (PRH_BS_CFG_SYS_LE_LOW_DUTY_DIRECT_ADVERTISING==1)
	if ((LE_advertise.advertising_type == 0x01) ||
		(LE_advertise.advertising_type == 0x04))// If Directed Advertising
	{
#else
	if (LE_advertise.advertising_type == 0x01) // If Directed Advertising
	{
#endif
		LE_advertise.direct_address_type = direct_address_type;

		for (i=0;i<6;i++)
		{
			LE_advertise.direct_address[i] = direct_address[i];
		}
	}
	LE_advertise.advertising_channel_map = advertising_channel_map;
	LE_advertise.advertising_filter_policy = advertising_filter_policy;

	return NO_ERROR;
}
#endif

/**************************************************************************
 * FUNCTION :- LEadv_Get_Advertising_Channel_Tx_Power
 *
 * DESCRIPTION
 * Simple function to read the Advertising Tx Power
 *************************************************************************/

u_int8 LEadv_Get_Advertising_Channel_Tx_Power(void)
{
	//return HWradio_Convert_Tx_Power_Level_Units_to_Tx_Power(LE_advertise.advertising_tx_power_level);
	return ((s_int8 (*)(u_int8))PATCH_FUN[HWRADIO_CONVERT_TX_POWER_LEVEL_UNITS_TO_TX_POWER])(LE_advertise.advertising_tx_power_level);
}

/**************************************************************************
 * FUNCTION :- LEadv_Set_Advertising_Data
 *
 * DESCRIPTION
 * Simple function to set the Advertising Data field.
 *************************************************************************/

t_error LEadv_Set_Advertising_Data(u_int8 length,t_p_pdu p_pdu)
{
	u_int8 i;

	if (length > 0x1f)
		return INVALID_HCI_PARAMETERS;

	LE_advertise.advertising_data_len = length;

	for(i=0;i<length;i++)
		LE_advertise.advertising_data[i] = p_pdu[i];

	return NO_ERROR;
}

/**************************************************************************
 * FUNCTION :- LEadv_Set_Scan_Response_Data
 *
 * DESCRIPTION
 * Simple function to set the Scan Response Data field.
 *************************************************************************/


t_error LEadv_Set_Scan_Response_Data(u_int8 length,t_p_pdu p_pdu )
{
	u_int8 i;
	if (length > 0x1f)
		return INVALID_HCI_PARAMETERS;

	LE_advertise.scan_response_data_len = length;

	for(i=0;i<length;i++)
		LE_advertise.scan_response_data[i] = p_pdu[i];

	return NO_ERROR;

}
/**************************************************************************
 * FUNCTION :- LEadv_Set_Advertise_Enable
 *
 * DESCRIPTION
 * Enables Advertising. Set a timer for the Next Advertising Event.
 *
 * Any request to enable connectable advertising on a device which already has
 * link(s) active will be rejected.
 *
 * If direct advertising is the active advertising type then a seperate timer for
 * direct advertising will be setup.
 *************************************************************************/
t_error LEadv_Set_Advertise_Enable(u_int8 advertise_enable)
{
	if(PATCH_FUN[LEADV_SET_ADVERTISE_ENABLE_ID]){
         
         return ((t_error (*)(u_int8 advertise_enable))PATCH_FUN[LEADV_SET_ADVERTISE_ENABLE_ID])(advertise_enable);
    }

	t_error status = NO_ERROR;

	if (advertise_enable > 0x01)
		status = INVALID_HCI_PARAMETERS;

	if (LE_config.slave_link_active)
		return COMMAND_DISALLOWED;
		

	if (LE_advertise.advertise_enable != advertise_enable)
		LE_advertise.advertise_enable = advertise_enable;
	else
		status = COMMAND_DISALLOWED;

	if (status == NO_ERROR)
	{
		if (advertise_enable)
		{
			if ((LE_config.num_le_links) && (LE_advertise.advertising_type < 0x02 /* Adv_Direct_Ind or  Adv_Ind */ ))
			{
				status = COMMAND_DISALLOWED;
			}
			else
			{
				_LEadv_Set_Next_Advertising_Event_Start_Timer();
				/*
				 * If direct advertising set the end time for Direct Advertising
				 */
				if (LE_advertise.advertising_type == 0x01 ) // DIRECT_ADV_IND
#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==0)
				{
					LE_advertise.direct_advertising_timer = BTtimer_Set_Ticks(DIRECT_ADVERTISING_TIME_IN_TICKS);
				}
#else
				{
					if (LE_advertise.user_direct_adv_timeout)
						LE_advertise.direct_advertising_timer = BTtimer_Set_Ticks(LE_advertise.direct_adv_timeout*4);
					else
						LE_advertise.direct_advertising_timer = BTtimer_Set_Ticks(DIRECT_ADVERTISING_TIME_IN_TICKS);
				}
#endif
			}
		}
		else // Clear the Next Advertising Event Start Timer.
		{
			LE_advertise.next_adv_timer = BT_TIMER_OFF;
		}
	}
    
    if ( 1 == g_le_enable_observer_flag)
    {
		LE_Adv_Adjust_scan_timer(LE_advertise.next_adv_timer, LE_advertise.advertising_interval);
	}
    
	return status;
}

/***************************************************************************
 * Function :- LEadv_Disable_Connectable_Advertising
 *
 * Description :-
 * This function disables Connectable Advertising (Adv_Direct_Ind or Adv_Ind) if advertising is
 * enabled in the device.
 *
 * This is called when an LE link is established, as an LE device can only be a slave on a
 * single link.
 *
 * Context :- IRQ and Schedular
 ***************************************************************************/

void LEadv_Disable_Connectable_Advertising(void)
{
	if(PATCH_FUN[LEADV_DISABLE_CONNECTABLE_ADVERTISING_ID]){
         ((void (*)(void))PATCH_FUN[LEADV_DISABLE_CONNECTABLE_ADVERTISING_ID])();
         return ;
    }

	/* If the Link Layer is already operating in the Connection State or Initiating
	 * State, the Link Layer shall not operate in the Advertising State with a type of
	 * advertising that could result in the Link Layer entering the Connection State
	 * in the Slave Role
	 */
#if (PRH_BS_CFG_SYS_LE_LOW_DUTY_DIRECT_ADVERTISING==1)
	if ((LE_advertise.advertise_enable) &&
	    ((LE_advertise.advertising_type < 0x02)  /* Adv_Direct_Ind High Duty or  Adv_Ind */  ||
	     (LE_advertise.advertising_type == 0x04) /* Adv_Direct_Ind Low Duty */))
	{
#else
	if ((LE_advertise.advertise_enable) &&
	    (LE_advertise.advertising_type < 0x02 /* Adv_Direct_Ind or  Adv_Ind */ ))
	{
#endif
		LE_advertise.advertise_enable = 0x00;
		LE_advertise.next_adv_timer = BT_TIMER_OFF;
		LE_advertise.direct_advertising_timer  =  BT_TIMER_OFF;
	}
}

/***************************************************************************
 * Function :- LEadv_Adverting_Event_Begin
 *
 * Description :-
 * This initiates an advertising event. This function is call when advertising interval
 * timer expires. It prepares the HW for the start of an advertising event.
 *
 * HW Writes :
 * 		HW_set_rx_mode(0x00)
 *		HWle_set_tx_enable();
 *		HWle_set_adv_state();
 *		HWle_set_le_mode();
 *		HWle_set_master_mode()
 *
 * The device state is changed to ADVERTISING, and the sub_state is changed to
 * W4_T2_PRE_ADV_TX or W4_T0_PRE_ADV_TX
 *
 * NOTE :- The initial sub-state for advertising is normally W4_T2_PRE_ADV_TX. However
 * due to some setup delays in the Rohm Radio we need to perform a dummy Rx on radio prior
 * to the first Tx. The behaviour is configured via #define FIX_PLL_ISSUE
 *
 * If Direct Advertising is not being performed then the Advertising timer is reset.
 *
 * NOTE :- If a connection event is due prior to the completion of the advertising event,
 * then the Advertising event will be delayed until after the collection event.
 *
 * Context :- IRQ
 ***************************************************************************/
void LEadv_Advertising_Event_Begin(u_int32 current_clk)
{
	if(PATCH_FUN[LEADV_ADVERTISING_EVENT_BEGIN_ID]){
         ((void (*)(u_int32 current_clk))PATCH_FUN[LEADV_ADVERTISING_EVENT_BEGIN_ID])(current_clk);
         return ;
    }

	if((LE_config.state == STANDBY_STATE) /*&& (LE_config.num_le_links == 0) Now support connections and advertising */)
	{
		t_bd_addr *p_bd_addr;
           
		HW_set_rx_mode(0x00);
		//HWle_set_rx_mode(0x00);
		HWle_set_tx_enable();
		HWle_set_adv_state();

		// Set the timer for the next interval -
	//	HWradio_Program_Dummy_Rx(LEfreq_Map_Channel_Index_2_RF_Freq(LEadv_Get_Advertising_Channel_Index()));
		LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM2_INTR_MSK_MASK);

	    HWle_set_le_mode();
	    HW_set_slave(0x0);

	    HWle_set_master_mode();
	    LE_config.state = ADVERTISING_STATE;

#if (PRH_SYS_CFG_HARDWARE_ADDRESS_FILTERING_SUPPORTED == 1)
	    // Setup the Hardware Address Filtering.

		HWle_set_rx_mode(0x3 /* Advertising */);

	    if (LE_advertise.advertising_filter_policy & 0x1)
	    	HWle_set_SCAN_REQ_filtering();
	    else
	    	HWle_clear_SCAN_REQ_filtering();

	    if (LE_advertise.advertising_filter_policy & 0x2)
	    	HWle_set_CONNECT_REQ_filtering();
	    else
	    	HWle_clear_CONNECT_REQ_filtering();


	    // Setup the Target Address - based on the Address Type
	    if (LEadv_Get_Advertising_Own_Address_Type()==PUBLIC_ADDRESS)
	    {
			p_bd_addr = (t_bd_addr *)SYSconfig_Get_Local_BD_Addr_Ref();
		}
		else
		{
			p_bd_addr = &LE_config.random_address;
		}

	    {
			u_int32 address32_plus_control[2]; //  = {0xFFFFFFFF,0xFFFFFFFF};
			u_int8* p_addr_plus_control = (u_int8*)address32_plus_control;
			u_int8 j;

			for (j=0;j<6;j++)
				p_addr_plus_control[j] = p_bd_addr->bytes[j];

			p_addr_plus_control[6] = 0;
			p_addr_plus_control[7] = 0;
			if (LEadv_Get_Advertising_Own_Address_Type()==RANDOM_ADDRESS)
				p_addr_plus_control[7] |= 0x40;

		    hw_memcpy((u_int8*)JAL_LE_TARGET_ADDR,address32_plus_control, 8);

	    }
#endif

		((void (*)(void))PATCH_FUN[HWRADIO_LE_RXCOMPLETE])();
		//HWradio_LE_RxComplete();
		HWle_set_le_spi_only();
		((void (*)(t_frame_pos const ))PATCH_FUN[HWRADIO_LE_SETUP_RADIO_FOR_NEXT_TXRX])(RX_START);
		//HWradio_LE_Setup_Radio_For_Next_TXRX(RX_START);
		LEadv_Reset_Adv_Frequency();
		LEadv_Prep_For_LE_Advert_Tx();
		LE_config.sub_state = W4_ADV_TX;

		LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM0_INTR_MSK_MASK);

#if (PRH_BS_CFG_SYS_TRACE_LE_DEVICE_STATE_VIA_HCI_SUPPORTED == 1)
		TCeg_Send_LE_LEDeviceState_Event(ADVERTISING_STATE,0);
#endif
	}
    // The advInterval is only relevant for NON-DIRECTED Advertising. For DIRECTED Advertising it seems
	// that Back-to-Back advertising events are the norm.

	if ((LE_advertise.advertising_type != 0x01 /* Connectable Directed Advertising - ADV_DIRECT_IND */) &&
		(LE_advertise.advertise_enable))
		_LEadv_Set_Next_Advertising_Event_Start_Timer();
}
#endif


#if (PRH_BS_CFG_SYS_LE_BROADCAST_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
/*****************************************************************
 *  FUNCTION :- _LEconfig_Set_Next_Advertising_Event_Start_Timer
 *
 *  Params :-
 *  	none
 *
 *  DESCRIPTION
 *  This function determines the time for the next advertising event. Based on
 *  the advertising interval and a pseudo-random advertising delay (0-10ms).
 *
 *  CONTEXT : IRQ and Schedular
 *****************************************************************/

static t_error _LEadv_Set_Next_Advertising_Event_Start_Timer(void)
{
	if(PATCH_FUN[_LEADV_SET_NEXT_ADVERTISING_EVENT_START_TIMER_ID]){
         
         return ((t_error (*)(void))PATCH_FUN[_LEADV_SET_NEXT_ADVERTISING_EVENT_START_TIMER_ID])();
    }

	// T_advEvent = advInterval + advDelay
	//
	// The advDelay is a pseudo-random value with a range of 0 ms to 10 ms generated
    // by the Link Layer for each advertising event.

	u_int8 advDelay = (u_int8)((SYSrand_Get_Rand() & 0x0000000F)*2); // (0 - 16)*.625
	//u_int8 advDelay = 0;
	u_int32 advInterval = LE_advertise.advertising_interval*2;

	  // Modified to ensure Advertising continues correctly after Page/Inquiry.
	  // As we may have missed multiple Advertising Intervals during Page/Inquiry we need
	  // To calculate the next interval from the Native clock and not base it on the
	  // existing value of next_adv_timer.

    LE_advertise.next_adv_timer = HW_Get_Native_Clk_Avoid_Race();
    
   	if (LE_advertise.next_adv_timer == BT_TIMER_OFF)
    {
        load_advDelay(0);
        
    	if (LE_advertise.advertising_type != 0x01)
    	    LE_advertise.next_adv_timer += advInterval;
    }
    else
    {
        load_advDelay(advDelay);
    	LE_advertise.next_adv_timer += (advInterval + advDelay);
    }


	LE_advertise.next_adv_timer = BTtimer_Safe_Clockwrap(LE_advertise.next_adv_timer);
	return NO_ERROR;

}
#ifndef REDUCE_ROM2
/*****************************************************************
 *  FUNCTION :- LEadv_Adjust_Advertising_Event_Start_Timer
 *
 *  Params :-
 *  	    none
 *
 *  DESCRIPTION
 *  This functions if called if the Advertising Event could not begin due
 *  to another activity. The Advertising event is pushed to a value 8 clock
 *  ticks beyond the current clock value.
 *
 *  CONTEXT : IRQ and Schedular
 *****************************************************************/

void LEadv_Adjust_Advertising_Event_Start_Timer(void)
{
	if(PATCH_FUN[LEADV_ADJUST_ADVERTISING_EVENT_START_TIMER_ID]){
         ((void (*)(void))PATCH_FUN[LEADV_ADJUST_ADVERTISING_EVENT_START_TIMER_ID])();
         return ;
    }

	u_int32 current_clk = HW_Get_Native_Clk_Avoid_Race();
	if (LEadv_Is_Next_Adv_Due(current_clk))
	{
		LE_advertise.next_adv_timer = BTtimer_Safe_Clockwrap(HW_Get_Native_Clk_Avoid_Race() + 8);
	}

}
#endif
#endif

#if (PRH_BS_CFG_SYS_LE_BROADCAST_DEVICE == 1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE == 1)

/**************************************************************************
 * FUNCTION :- LEadv_Reset_Adv_Frequency
 *
 * DESCRIPTION
 * Resets the Advertising channel map. All channel are indicated as unused.
 * The Advertising channel index is set to the first available frequency.
 * This is called at the start of an advertising event.
 *************************************************************************/

void LEadv_Reset_Adv_Frequency(void)
{
	if(PATCH_FUN[LEADV_RESET_ADV_FREQUENCY_ID]){
         ((void (*)(void))PATCH_FUN[LEADV_RESET_ADV_FREQUENCY_ID])();
         return ;
    }

	// Set the advertising_channel to the first one in the channel map
	LE_advertise.advertising_channels_used = 0x00;
	if(LE_advertise.advertising_channel_map & 0x01)
	{
		LE_advertise.advertising_channel_index = 37;
		LE_advertise.advertising_channels_used = 0x01;
	}
	else if(LE_advertise.advertising_channel_map & 0x02)
	{
		LE_advertise.advertising_channel_index = 38;
		LE_advertise.advertising_channels_used = 0x02;
	}
	else if(LE_advertise.advertising_channel_map & 0x04)
	{
		LE_advertise.advertising_channel_index = 39;
		LE_advertise.advertising_channels_used = 0x04;
	}
}


/**************************************************************************
 * FUNCTION :- LEadv_Try_Advance_Adv_Frequency
 *
 * DESCRIPTION
 * Advances the avertising channel to the next available. This is called within
 * an advertising event to select the channel used for the 2nd or 3rd Adv Transmission
 *************************************************************************/

u_int8 LEadv_Try_Advance_Adv_Frequency(void)
{
	if(PATCH_FUN[LEADV_TRY_ADVANCE_ADV_FREQUENCY_ID]){
         
         return ((u_int8 (*)(void))PATCH_FUN[LEADV_TRY_ADVANCE_ADV_FREQUENCY_ID])();
    }

	if(LE_advertise.advertising_channel_map != LE_advertise.advertising_channels_used)
	{
		u_int8 channels_remaining;

		channels_remaining = (LE_advertise.advertising_channel_map - LE_advertise.advertising_channels_used);
		if(channels_remaining & 0x02)
		{
			LE_advertise.advertising_channel_index = 38;
			LE_advertise.advertising_channels_used |= 0x02;
		}
		else if(channels_remaining & 0x04)
		{
			LE_advertise.advertising_channel_index = 39;
			LE_advertise.advertising_channels_used |= 0x04;
		}
		return 1;
	}
	else
	{
		return 0;
	}
}
#endif


#if (PRH_BS_CFG_SYS_LE_BROADCAST_DEVICE==1)||(PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE ==1)

/***********************************************************************
 *
 * Handlers for Packet Exchange on the Advertiser
 *
 ***********************************************************************/

/**************************************************************************
 * FUNCTION :- LEadv_Prep_For_LE_Advert_Tx
 *
 * DESCRIPTION
 * Prepares the system for the Transmission on an Advertising Packet
 * The CRC Init, Whitening Init and Access Address are first set in HW.
 * The TIFS count is setup and the Actual Advertising packet is encoded and
 * written into HW buffer.
 *
 * Finally the Radio is configured for Tx of the packet.
 *
 *************************************************************************/

void LEadv_Prep_For_LE_Advert_Tx(void)
{
	if(PATCH_FUN[LEADV_PREP_FOR_LE_ADVERT_TX_ID]){
         ((void (*)(void))PATCH_FUN[LEADV_PREP_FOR_LE_ADVERT_TX_ID])();
         return ;
    }

	
	// Setup the LC-LE HW for the First advertising packet Transmission
	// GF 30 March - moved from Prep_For_LE_Advert_Tx
    #if(__DEBUG_MODEM_LOWPOWER_ENABLE == 1)     
    __write_hw_reg32(CPR_BT_MODEM_CLK_CTL, BLE_MODEM_CLK_ON);
    #endif

	HWle_set_crc_init(0x555555);
	HWle_set_whitening_init(LE_advertise.advertising_channel_index);
	HWle_set_acc_addr(ADVERTISING_ACCESS_ADDRESS);
    *((u_int32 volatile*)0x4002c248) = (u_int32)(ADVERTISING_ACCESS_ADDRESS); // 0x8E89BED6 ((0x8e89b3d6); 
	*((u_int32 volatile*)0x4002c24c) = (u_int32)(0x0); 
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// NB :- SCAN STATE SHOULD BE SET TO 0 HERE
	// As this is not alway Zero when entering this
	// function - specifically on 2nd or 3rd ADV packet.
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	HWle_set_tifs_default();
	HWle_clear_tifs_abort();
	//HWle_set_tx_enable(); //add for CRC condition

	// Key Variables are :
	//
	// 1. Advertising Type -
	// 2. Channel
	//
#if (PRH_BS_CFG_SYS_LE_LOW_DUTY_DIRECT_ADVERTISING==1)
	if ((LE_advertise.advertising_type == ADV_DIRECT_TYPE_HIGH_DUTY_TYPE) ||
		(LE_advertise.advertising_type == ADV_DIRECT_TYPE_LOW_DUTY_TYPE))
#else
	if (LE_advertise.advertising_type == ADV_DIRECT_TYPE)
#endif
	{
	/*
	    #if(__DEBUG_RF_LOWPOWER_ENABLE == 1)
		SER_WRITE(0x10, 0x3f);		// - ldo off
		#endif
	*/	
		// The Direct Address field is only used in this ADV_DIRECT_IND messages
		// The address is obtained from the HCI_LE_Set_Advertising_Parameters Command
		LEll_Encode_Advertising_ChannelPDU(ADV_DIRECT_IND, LE_advertise.direct_address,
				LE_advertise.direct_address_type,0x00);
	}
	else
	{
		LEll_Encode_Advertising_ChannelPDU((t_LE_advertising_type)LE_advertise.advertising_type,0x00,0x00,0x00);

	}

    HW_set_rx_mode(0x00);
//	HWle_set_rx_mode(0x00);
#ifndef BLUETOOTH_MODE_LE_ONLY//no pta interface
	HWpta_LE_Access_Request(TRUE, 0, RADIO_MODE_LE_TIFS_TX_RX);
#endif
    ((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_LE_INITIAL_TX,LEfreq_Map_Channel_Index_2_RF_Freq(LE_advertise.advertising_channel_index),TX_START_FREQ);
	//HWradio_LE_Service(RADIO_MODE_LE_INITIAL_TX,LEfreq_Map_Channel_Index_2_RF_Freq(LE_advertise.advertising_channel_index),TX_START_FREQ);
}

/**************************************************************************
 * FUNCTION :- _LEadv_Advertising_Event_Complete
 *
 * DESCRIPTION
 * This function is called on the completion of an advertising event.
 *
 * For non-direct advertising the Radio is placed in Standby mode.
 *
 *************************************************************************/

void _LEadv_Advertising_Event_Complete(u_int32 current_clk)
{
	
	if(PATCH_FUN[_LEADV_ADVERTISING_EVENT_COMPLETE_ID]){
         ((void (*)(u_int32 current_clk))PATCH_FUN[_LEADV_ADVERTISING_EVENT_COMPLETE_ID])(current_clk);
         return ;
    }

	HWle_clear_le_spi_only();
    // Abort the TIFS count
	HWle_abort_tifs_count();
	HWle_clear_tifs_default();

	// Turn off TX and RX
	HW_set_rx_mode(0x00);
	//HWle_set_rx_mode(0x00);

	if ((LE_advertise.advertising_type != 0x01 /* Not - ADV_DIRECT_IND */) ||
		(LE_advertise.advertise_enable==0))
	{
		HWle_clear_tx_enable();
		LE_config.state = STANDBY_STATE;

	    // Not so sure about placing RF in standby more..HWradio_LE_Service(RADIO_MODE_STANDBY,0,0);
		((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_STANDBY,0,0);
	}
	else if ((LE_advertise.advertise_enable==1) /* ADV_DIRECT_IND */ &&
			!(BTtimer_Is_Expired_For_Time(LE_advertise.direct_advertising_timer,current_clk)))
	{
		if (LE_config.num_le_links==0)
		{
			((void (*)(void))PATCH_FUN[HWRADIO_LE_RXCOMPLETE])();
			//HWradio_LE_RxComplete();
			LEadv_Reset_Adv_Frequency();
			LEadv_Prep_For_LE_Advert_Tx();
			HWle_set_tx_enable();
			LE_config.sub_state = W4_ADV_TX;
			LE_config.state = ADVERTISING_STATE;
			LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM0_INTR_MSK_MASK);
		}
		else
		{
			HWle_clear_tx_enable();
			LE_config.state = STANDBY_STATE;

		    // Not so sure about placing RF in standby more..HWradio_LE_Service(RADIO_MODE_STANDBY,0,0)
		    ((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_STANDBY,0,0);
		}
	}
#if (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
	else if ((LE_advertise.advertising_type == 0x01 /* Not - ADV_DIRECT_IND */) &&
	         (BTtimer_Is_Expired_For_Time(LE_advertise.direct_advertising_timer,current_clk)))
	{
		/* If the Advertising_Type parameter is 0x01 (ADV_DIRECT_IND) and the
		 * directed advertising fails to create a connection, an LE Connection Complete
		 * event shall be generated with the Status code set to Directed Advertising Timeout
		 * (0x3C).
		 */
		HWle_clear_tx_enable();
		LE_config.state = STANDBY_STATE;

	    // Not so sure about placing RF in standby more..HWradio_LE_Service(RADIO_MODE_STANDBY,0,0)
		((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_STANDBY,0,0);

		LEeventgen_Generate_LE_Meta_Event(LE_SUBEVENT_CONNECTION_COMPLETE,0x00,(t_error)0x3C); //- LC add (t_error) in order to avoid compiler warning.
		LE_advertise.advertise_enable = 0x00;
		LE_advertise.next_adv_timer = BT_TIMER_OFF;
		LE_advertise.direct_advertising_timer  =  BT_TIMER_OFF;
	}
#endif
	LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM0_INTR_MSK_MASK |JAL_TIM2_INTR_MSK_MASK );
#if (PRH_BS_CFG_SYS_TRACE_LE_DEVICE_STATE_VIA_HCI_SUPPORTED == 1)
    TCeg_Send_LE_LEDeviceState_Event(LE_config.state,0);
#endif
#ifndef BLUETOOTH_MODE_LE_ONLY//no pta interface
	HWpta_LE_Access_Complete();
#endif

    
#if (PRH_SYS_CFG_HARDWARE_ADDRESS_FILTERING_SUPPORTED == 1)

    if(LE_config.state == STANDBY_STATE) {
    HWle_clear_address_filtering();
    HWle_clear_SCAN_REQ_filtering();
    HWle_clear_CONNECT_REQ_filtering();
    }
#endif

#if 0 // Slave links in Dual Mode to be reworked.
    if (LE_config.slave_link_active)
    {
    	t_LE_Connection* p_connection = LEconnection_Find_Link_Entry(LE_config.initiator_link_id);

    	if (LEconnection_Get_Initial_Anchor_Point_Obtained(p_connection)!=0)
    		LEconfig_Switch_To_BT_Classic();
    }
    else
#endif


 	if (((LE_advertise.next_adv_timer != BT_TIMER_OFF) &&
 	    (!BTtimer_Is_Expired_For_Time(LE_advertise.next_adv_timer, current_clk)) &&
 	    ((BTtimer_Clock_Difference(current_clk,LE_advertise.next_adv_timer) >> 1) > SYS_CFG_LE_LC_MIN_SLOTS_FOR_SLEEP_PROCEDURE)) ||
 	    ((LE_advertise.advertise_enable==1) /* ADV_DIRECT_IND */ &&
 	     (BTtimer_Is_Expired_For_Time(LE_advertise.direct_advertising_timer,current_clk))))
 	{
#ifndef BLUETOOTH_MODE_LE_ONLY	
 		LEconfig_Switch_To_BT_Classic();
#endif
#if (PRH_BS_CFG_SYS_LOW_POWER_MODE_SUPPORTED==1)
	#if (__DEBUG_LE_SLEEP == 0)
    if(0 == g_debug_le_sleep)
    {
		LE_LL_Sleep();
    }   
	#elif (__DEBUG_LE_SLEEP == 1)
    if (1 == g_debug_le_sleep)
    {
		if (!LE_config.num_le_links)
			Le_Adv_Direct_Sleep(LE_advertise.next_adv_timer - HW_Get_Bt_Clk_Avoid_Race());
    }    
	#elif (2 == __DEBUG_LE_SLEEP)
    if (2 == g_debug_le_sleep)
    {
        if (0 == LE_config.num_le_links) {
        
         if(adv_always_wake == 0)
          {
            Adv_Enter_LowPower();
		  }
        } 
    }    
	#endif
#endif
 	}
}


/**************************************************************************
 * FUNCTION :- LEadv_Get_Advertising_Channel_Index
 *
 * DESCRIPTION
 * This function returns the next advertising channel index.
 *
 *************************************************************************/

u_int8 LEadv_Get_Advertising_Channel_Index(void)
{
	return LE_advertise.advertising_channel_index;
}
#ifndef REDUCE_ROM2
/**************************************************************************
 * FUNCTION :- LEadv_Get_Scan_Response_Data_Len
 *
 * DESCRIPTION
 * This function returns the length of the Scan Response.
 *
 *************************************************************************/

u_int8 LEadv_Get_Scan_Response_Data_Len(void)
{
	return LE_advertise.scan_response_data_len;
}
#endif
/**************************************************************************
 * FUNCTION :- LEadv_Get_Advertising_Filter_Policy
 *
 * DESCRIPTION
 * Returns the advertising filter policy.
 *************************************************************************/

u_int8 LEadv_Get_Advertising_Filter_Policy(void)
{
	return LE_advertise.advertising_filter_policy;
}

/**************************************************************************
 * FUNCTION :- LEadv_Set_Advertising_Filter_Policy
 *
 * DESCRIPTION
 * Sets the advertising filter policy.
 *************************************************************************/

__INLINE__ void LEadv_Set_Advertising_Filter_Policy(u_int8 filter_policy)
{
	LE_advertise.advertising_filter_policy = filter_policy;
}

#ifndef REDUCE_ROM2
/**************************************************************************
 * FUNCTION :- LEadv_Get_Slots_To_Next_Advertising_Timer
 *
 * DESCRIPTION
 * Determines the number of slots (integral of 0.625ms) to the next Advertising
 * Event 
 *************************************************************************/

u_int32 LEadv_Get_Slots_To_Next_Advertising_Timer(u_int32 current_clk)
{
	 
    if(PATCH_FUN[LEADV_GET_SLOTS_TO_NEXT_ADVERTISING_TIMER_ID]){
         
         return ((u_int32 (*)(u_int32 current_clk))PATCH_FUN[LEADV_GET_SLOTS_TO_NEXT_ADVERTISING_TIMER_ID])(current_clk);
    }

	if(LE_advertise.advertise_enable)
	{
		if (((LE_config.master_link_active) &&
			((LE_advertise.advertising_type == 0x06) || (LE_advertise.advertising_type == 0x02))) ||
			!LE_config.master_link_active)
			return (BTtimer_Clock_Difference(current_clk,LE_advertise.next_adv_timer) >> 1);
		else
			return 0xFFFFFFFF;
	}
	else
		return 0xFFFFFFFF;
}
#endif
#endif

#if (PRH_BS_CFG_SYS_LE_BROADCAST_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)

/**************************************************************************
 * FUNCTION :- _LEadv_Advance_For_Next_Adv_Tx
 *
 * DESCRIPTION
 * This is a helper function for the Advertising state machine (in LEadv_Handle_Advertising)
 * It move the adveritising state machine to the W4_N_x_T2 state.
 *************************************************************************/

void _LEadv_Advance_For_Next_Adv_Tx(void)
{
	
	if(PATCH_FUN[_LEADV_ADVANCE_FOR_NEXT_ADV_TX_ID]){
         ((void (*)(void))PATCH_FUN[_LEADV_ADVANCE_FOR_NEXT_ADV_TX_ID])();
         return ;
    }

    HW_set_rx_mode(0x00);
	//HWle_set_rx_mode(0x00);
	HWle_clear_tifs_default();
	//HWle_set_tifs_abort();
	LE_config.sub_state = W4_N_x_T2;

    #if((__DEBUG_ADV_INTERVAL_LOW_PWR == 1) &&(__DEBUG_RF_MPW1_SHDN_ENABLE == 1))   
    __write_hw_reg32(BTPHY_CTL0,  BTPHY_CTL0_SHDN);
    #endif
    //#if((__DEBUG_ADV_INTERVAL_LOW_PWR == 1) && (__DEBUG_RF_LOWPOWER_ENABLE == 1))
    //SER_WRITE(0x10, 0x3f);
    //#endif
	#if((__DEBUG_ADV_INTERVAL_LOW_PWR == 1) && (__DEBUG_MODEM_LOWPOWER_ENABLE == 1))		
    __write_hw_reg32(CPR_BT_MODEM_CLK_CTL, BLE_MODEM_CLK_OFF);
	#endif
    
                
#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==1)
	LE_advertise.delta_count = LE_advertise.advertising_delta;
#endif
	LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM0_INTR_MSK_MASK | JAL_TIM3_INTR_MSK_MASK);
}


/**************************************************************************
 * FUNCTION :- LEadv_Handle_Advertising
 *
 * DESCRIPTION
 * This is the Main Advertising State Machine. Called from the IRQ handler.
 * The following sub_states are supported :
 *
 *
 *   W4_ADV_TX
 *          In this state the system is waiting for the completion of the transmission
 *          of an advertising packet. On receipt of a PKA the actions performed are dependent
 *          on the advertising type. If a Rx is allowed then we prepare the RF and
 *          hardware for the RX. If an Rx is not allowed then we either prepare for the 
 *          next Advertising Packet Tx or complete the advertising event.
 *
 *          Valid IRQ :- PKA
 *          Next States : W4_ADV_RX | W4_N_x_T2
 *
 *   W4_ADV_RX
 *          In this state the system is waiting for a response to the Advertising Packet.
 *          If a PKD is received the packet is decoded and the action performed depends on the
 *          PDU type. If a CONNECT_REQ is recieved then a link is established and a Connection_Complete
 *          sent to the HCI and the Advertising Event is closed. 
 *          If a SCAN_REQ is received then if required a SCAN_RESP is written to HW and state is
 *          changed to W4_SCAN_RESP_RX.
 *
 *          Valid IRQ :- PKD, NO_PKD, SYNC_DETECT
 *          Next States :- W4_SCAN_RESP_RX | W4_N_x_T2
 *
 *   W4_SCAN_RESP_TX
 *          In this state the system is waiting for the completion of the Transmission of a 
 *          SCAN_RESP message to the peer. On receipt of a PKA the system either prepares for
 *          transmission of the next Advertising Packet or closes the Advertising Event.
 *  
 *          Valid IRQ :- PKA
 *          Next States :- W4_ADV_TX  or Close Event
 *         
 *   W4_N_x_T2
 *          This status is used to introduce a N*1.25ms Delay between Advertising Packets.
 *          The default is 1.25ms delay, but this can be extended by using the 
 *          Advertising_Delta. The Advertising Delta is controlled by a specific TCI command.
 * 
 *          Valid IRQ :- Tim 2
 *			Next States :- W4_ADV_TX
 *
 *************************************************************************/
t_error LEadv_Handle_Advertising(u_int8 IRQ_Type,u_int32 current_clk)
{
	if(PATCH_FUN[LEADV_HANDLE_ADVERTISING_ID]){
         
         return ((t_error (*)(u_int8 IRQ_Type,u_int32 current_clk))PATCH_FUN[LEADV_HANDLE_ADVERTISING_ID])(IRQ_Type,   current_clk);
    }

	u_int8 length = 40;
	u_int8 tifs_aborted = 0x00;
	uint32_t white_list_en = 0;
	uint32_t white_list_out = 0;	
	uint32_t Le_rx_filtered = 0;
	uint32_t white_reg_en = 0;
    LE_config.pre_pkd_bt_clk = 0;

	switch(LE_config.sub_state)
	{
	case W4_ADV_TX :
		switch(IRQ_Type)
		{
		case LE_PKA :
	       	// Have to insert delay before I call the SetFReq
	       	// Need this to ensure that we dont place the Radio in Idle state
	       	// prior to BPKTCTL going low. As BPKTCTL going low is timed as 10us
	       	// after the TIFS.
		#if(__DEBUG_RX_DATA_DUMP_TO_SHRAM0 == 1)
		*((u_int32 volatile*)0x4002c274) |= 1 << 3;  // 
		#endif
		
	        if (LE_advertise.advertising_type !=  ADV_NONCONN_IND)
	       	{
	        	//_LE_LL_Prep_For_LE_Advert_Rx();
	        	HW_set_rx_mode(0x01);
				//HWle_set_rx_mode(0x3 /* Advertising */);
				((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_LE_INITIAL_TX,LEfreq_Map_Channel_Index_2_RF_Freq(LEadv_Get_Advertising_Channel_Index()),RX_START_FREQ);
	        	//HWradio_LE_Service(RADIO_MODE_LE_INITIAL_TX,LEfreq_Map_Channel_Index_2_RF_Freq(LEadv_Get_Advertising_Channel_Index()),RX_START_FREQ);
	        	LE_config.sub_state = W4_ADV_RX;
				//LCirq_Disable_All_Tim_Intr_Except(0); // sometimes if pkd not come in, this will cause error 
				LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM0_INTR_MSK_MASK);
				
	       	}
	        else // ADV_NONCONN_IND_TYPE
	        {
			    if(LEadv_Try_Advance_Adv_Frequency())
			    {
			    	_LEadv_Advance_For_Next_Adv_Tx();
			    }
			    else
			    {
			        _LEadv_Advertising_Event_Complete(current_clk);
			    }
	        }

			pkd_error_cnt=0;
			
			break;
            case LE_TIM_0:
                pkd_error_cnt++;
                if (pkd_error_cnt >20)
                {
                    pkd_error_cnt=0;
  		        	// Advertising event is complete
  		        	//LEadv_Reset_Adv_Frequency();
		        	((void (*)(void))PATCH_FUN[HWRADIO_LE_RXCOMPLETE])();
					//HWradio_LE_RxComplete();
		        	_LEadv_Advertising_Event_Complete(current_clk);                  
                }
			break;

		default :
			break;
		}
			//
		break;

	case W4_ADV_RX :
		switch(IRQ_Type)
		{
#if (PRH_BS_CFG_SYS_LE_12MHZ_OPTIMISATION==1)
		case LE_SYNC_DET :
			// As we are the advertised - no need to store RSSI - scanner  only

			LEadv_Prep_For_LE_Scan_Resp_Tx();
			break;
#endif
		case LE_PKD :

			#if(__DEBUG_RX_DATA_DUMP_TO_SHRAM0 == 1)
			*((u_int32 volatile*)0x4002c274) &= ~(1 << 3);  // 
			#endif
			//HWle_set_rx_mode(0x0 /* Advertising */);
			// This is either a Scan Req or Connect Req PDU
			HWle_clear_tifs_abort();
			length = HWle_get_rx_length_adv();
			tifs_aborted = HWle_get_tifs_abort();
			
			if (CONNECT_REQ == HWle_get_rx_pdu_type())
			{
			// Disable LE Timed SPIs & all SPIs.
				HWle_clear_le_spi_only();
				((void (*)(void))PATCH_FUN[HWRADIO_DISABLEALLSPIWRITES])();
			}

			if (tifs_aborted)
			{
				HWle_clear_le_spi_only();
				((void (*)(void))PATCH_FUN[HWRADIO_DISABLEALLSPIWRITES])();	
				//HWradio_DisableAllSpiWrites();
				HWle_clear_tx_enable();
			}

			if ((!LE_LL_Get_CRC_Err()) &&  _LE_LL_CHECK_ADV_PKT_LENGTH(length) && !tifs_aborted)
			{
#if (PRH_SYS_CFG_HARDWARE_ADDRESS_FILTERING_SUPPORTED==0)
				hw_memcpy32((void*)u_int32_RX_buffer, (void*)HWle_get_rx_acl_buf_addr(), length);

				if (LEll_Decode_Advertising_ChannelPDU((t_p_pdu)u_int32_RX_buffer ,0x00,length))
#else
				u_int8 filters_match=0;
#if(__DEBUG_RX_DATA_DUMP_TO_SHRAM0 == 1)
	pkd_ok_cnt++;
#endif
                white_list_en = *((u_int32  volatile*)0x4002A058);
                white_list_en = (white_list_en>>16) & 0x7F ;

                Le_rx_filtered = *((u_int32  volatile*)0x4002A054);
                Le_rx_filtered = (Le_rx_filtered>>8) & 0x07;

                white_list_out = (white_list_en == 0 )? 1: (Le_rx_filtered != 7);
				
				white_reg_en = *((u_int32  volatile*)0x4002A058);
				white_reg_en = (white_reg_en>>14)&0x03;
				if(white_reg_en == 0) white_list_out = 1;
					
				// If filters are ON and get a match
				if (HWle_get_target_address_match() && white_list_out)
				{
					hw_memcpy32((void*)u_int32_RX_buffer, (void*)HWle_get_rx_acl_buf_addr(), length);
					filters_match = 0x1;
					LEll_Decode_Advertising_ChannelPDU((t_p_pdu) u_int32_RX_buffer ,0x00, length);

				}

				if(filters_match)
#endif
				{
					// Setup for next Transmit - the PDU is already in the TX buffer
#if (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
					if (LE_config.last_adv_pdu_rxed == CONNECT_REQ)
					{
						t_LE_Connection* p_connection = LEconnection_Find_Link_Entry(LE_config.initiator_link_id);
						u_int8 link_id = LE_config.initiator_link_id;

						HW_set_freeze_bt_clk(1);
						HWle_clear_tx_enable();
						// GF 12 April -- Need to but the Radio in Standby mode earlier... and ensure the Tx Pulse does not go to the
						// radio.

						HWle_clear_le_spi_only();

						// Abort the TIFS count
						HWle_abort_tifs_count();
						HWle_clear_tifs_default();

						// Turn off TX and RX
						HW_set_rx_mode(0x00);
						//HWle_set_rx_mode(0x00);

						//LE_config.state = STANDBY_STATE;

						// Not so sure about placing RF in standby more..
						//HWradio_LE_Service(RADIO_MODE_STANDBY,0,0);

						//p_connection->supervision_TO_timer = BTtimer_Safe_Clockwrap(current_clk + ((p_connection->connInterval * 2)* 6 *2));
						LEconnection_Set_Active(p_connection,0x01);
						//LE_config.slave_link_active = 0x01;

						HW_set_slave(0x01);
					    HWle_clear_master_mode();
#if (PRH_BS_CFG_SYS_LE_DUAL_MODE == 1)// DUAL MODE - need to configure the BT Clock....
					    {
					    	extern u_int16 	g_LE_Slave_IntraSlot;

					    	g_LE_Slave_IntraSlot = HW_Get_Intraslot_Avoid_Race();
					    	HW_set_bt_clk_offset(0);
					    	HW_set_intraslot_offset(0);
					    	
					    }
					   // HW_set_add_bt_clk_relative(0);
#endif
						LEconnection_Setup_First_Connection_Event(link_id,current_clk);
						LEfreq_Update_Channel_Remapping_Table(p_connection, p_connection->data_channel_map);
						_LEadv_Advertising_Event_Complete(current_clk);

						// To be picked up by the MiniSched
						LE_config.connection_complete_pending = 0x01;
						LE_config.p_connection_complete_pending = p_connection;
						
						ext_rx_window_init(p_connection);
						// +++
                        #if defined(SYS_HAL_OS_EVENT_DRIVEN_HOST_CONTROLLER)
                         BTms_OS_Post(BTMS_OS_EVENT_CONNECT_REQ_QUEUE);
                        #endif
						// +++
					}
					else
#endif
					{
						// This should only be done if the Rx PDU is not a connect_req.
						// This should be done here - as we want to disable the SPIs for
						// the Connect_Req as fast as possible.
						//
						
					//	vLEadv_Prep_For_LE_Scan_Resp_Tx = SysTick_Value_GET();
						LEadv_Prep_For_LE_Scan_Resp_Tx();

						LE_config.sub_state = W4_SCAN_RESP_TX;
						LSLCirq_Disable_All_Tim_Intr_Except(0);
					}
				}
		        else
		        {
		        	// Advertising event is complete
		        	((void (*)(void))PATCH_FUN[HWRADIO_LE_RXCOMPLETE])();
					//HWradio_LE_RxComplete();
		        	_LEadv_Advertising_Event_Complete(current_clk);
		        }

			}
			else
			{
		#if(__DEBUG_RX_DATA_DUMP_TO_SHRAM0 == 1)
			pkd_crc_cnt++;
		#endif
				/*if (LE_config.last_adv_pdu_rxed == CONNECT_REQ)
				{
					while(1);

				}*/
				// Need to move on to the next advertising Tx or end the Advertising event
		        if(LEadv_Try_Advance_Adv_Frequency())
		        {
			    	_LEadv_Advance_For_Next_Adv_Tx();

		        }
		        else
		        {
		        	// Advertising event is complete
		        	((void (*)(void))PATCH_FUN[HWRADIO_LE_RXCOMPLETE])();
					//HWradio_LE_RxComplete();
		        	_LEadv_Advertising_Event_Complete(current_clk);
		        }

			}

			adv_no_pka_count = 0;
			break;

		case LE_NO_PKD :

			#if(__DEBUG_RX_DATA_DUMP_TO_SHRAM0 == 1)
			*((u_int32 volatile*)0x4002c274) &= ~(1 << 3);  // 
			#endif

			
	        if(LEadv_Try_Advance_Adv_Frequency())
	        {
	        	((void (*)(void))PATCH_FUN[HWRADIO_LE_RXCOMPLETE])();
				//HWradio_LE_RxComplete();
			    _LEadv_Advance_For_Next_Adv_Tx();
	        }
	        else
	        {
	        	_LEadv_Advertising_Event_Complete(current_clk);
	        }

		#if(__DEBUG_RX_DATA_DUMP_TO_SHRAM0 == 1)
		*((u_int32 volatile*)0x4002c274) &= ~(1 << 3);  // 
		#endif

            adv_no_pka_count=0;
        
	        break;

		/* add case LE_RX_HDR:*/	
		case	LE_RX_HDR:
			{
				u_int8 pdu_type = HWle_get_rx_pdu_type();
				if(pdu_type == SCAN_REQ)
				LEll_Encode_Advertising_ChannelPDU(SCAN_RSP,0,0,0);
			}				
			break;

		case LE_TIM_0:
			adv_no_pka_count++;
			if(adv_no_pka_count>5)
			{
				adv_no_pka_count = 0;
				((void (*)(void))PATCH_FUN[HWRADIO_LE_RXCOMPLETE])();
				_LEadv_Advertising_Event_Complete(current_clk);
		
				
			}
			break;		
		}
		break;

	case W4_SCAN_RESP_TX :
		switch(IRQ_Type)
		{
		case LE_PKA :
	       	// Have to insert delay before I call the SetFReq
	       	// Need this to ensure that we dont place the Radio in Idle state
	       	// prior to BPKTCTL going low. As BPKTCTL going low is timed as 10us
	       	// after the TIFS.

        	HWle_set_tifs_abort();
		    if(LEadv_Try_Advance_Adv_Frequency())
		    {
		    	//LEadv_Prep_For_LE_Advert_Tx();
				//LE_config.sub_state = W4_ADV_TX;
				_LEadv_Advance_For_Next_Adv_Tx();
		    }
		    else
		    {
		    	_LEadv_Advertising_Event_Complete(current_clk);
		    }
		    break;

		default :
			break;
		}
		break;

	case W4_N_x_T2 :
#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==1)
		if ((IRQ_Type == LE_TIM_3) && (LE_advertise.delta_count > 0))
		{
			LE_advertise.delta_count--;
		}
		if (LE_advertise.delta_count==0)
#endif
		{
			
			LEadv_Prep_For_LE_Advert_Tx();
			LE_config.sub_state = W4_ADV_TX;
		}
		break;

	default:
		break;
	}
	return NO_ERROR;
}
#endif

/**************************************************************************
 * FUNCTION :- LEadv_Encode_Own_Address_In_Payload
 *
 * DESCRIPTION
 * This simple function writes the address used for advertising into a memory
 * location identified by p_payload. 
 * The address is either the device Public address or the Random address, depending
 * on the current Advertising Address Type.
 **************************************************************************/

void LEadv_Encode_Own_Address_In_Payload(u_int8* p_payload)
{
	if(PATCH_FUN[LEADV_ENCODE_OWN_ADDRESS_IN_PAYLOAD_ID]){
         ((void (*)(u_int8* p_payload))PATCH_FUN[LEADV_ENCODE_OWN_ADDRESS_IN_PAYLOAD_ID])(p_payload);
         return ;
    }


	const t_bd_addr* p_bd_addr;
	u_int8 i;

	if(LE_advertise.advertising_own_address_type == PUBLIC_ADDRESS)
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
#if (PRH_BS_CFG_SYS_LE_BROADCAST_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)


/**************************************************************************
 * FUNCTION :- LEadv_Set_Adv_Header
 *
 * DESCRIPTION
 * Sets the Header for an Advertising Packet
 **************************************************************************/

void LEadv_Set_Adv_Header(u_int8 pdu_type,u_int8 rxAddr)
{
	 
    if(PATCH_FUN[LEADV_SET_ADV_HEADER_ID]){
         ((void (*)(u_int8 pdu_type,u_int8 rxAddr))PATCH_FUN[LEADV_SET_ADV_HEADER_ID])(pdu_type,  rxAddr);
         return ;
    }

	switch(pdu_type)
	{
	case ADV_IND :
	case ADV_NONCONN_IND :
	case ADV_SCAN_IND :
		LE_LL_Set_Adv_Header(pdu_type,LE_advertise.advertising_own_address_type, rxAddr, (6 + LE_advertise.advertising_data_len) );
		break;

	case ADV_DIRECT_IND :
		LE_LL_Set_Adv_Header(pdu_type,LE_advertise.advertising_own_address_type,
				LE_advertise.direct_address_type,12);
		break;

	case SCAN_RSP :
		LE_LL_Set_Adv_Header(pdu_type,LE_advertise.advertising_own_address_type,rxAddr,(LE_advertise.scan_response_data_len+6));
		break;
	}
}

/**************************************************************************
 * FUNCTION :- LEadv_Write_Adv_Data
 *
 * DESCRIPTION
 * Writes the Advertising Data Field into a memory location - p_payload
 **************************************************************************/

u_int8 LEadv_Write_Adv_Data(u_int8* p_payload)
{
	u_int8 i;

	for(i=0;i<LE_advertise.advertising_data_len;i++)
	{
		p_payload[i] = LE_advertise.advertising_data[i];
	}


	return LE_advertise.advertising_data_len;
}

/**************************************************************************
 * FUNCTION :- LEadv_Write_Scan_Resp_Data
 *
 * DESCRIPTION
 * Writes the Scan Response Data Field into a memory location - p_payload
 **************************************************************************/

u_int8 LEadv_Write_Scan_Resp_Data(u_int8* p_payload)
{
	if(PATCH_FUN[LEADV_WRITE_SCAN_RESP_DATA_ID]){
         
         return ((u_int8 (*)( u_int8* p_payload))PATCH_FUN[LEADV_WRITE_SCAN_RESP_DATA_ID])(p_payload);
    }

#if		0
	u_int8 i;

	for(i=0;i<LE_advertise.scan_response_data_len;i++)
	{
		p_payload[i] = LE_advertise.scan_response_data[i];
	}
		

	return LE_advertise.scan_response_data_len;

#endif
	u_int8  i = 2;
	u_int8 	cnt = LE_advertise.scan_response_data_len; 
	
	u_int8	byteblk;
	u_int8  j;
	
    u_int32 *dest_cpy = (u_int32*) (p_payload+2),
    *src_cpy = (u_int32*) &(LE_advertise.scan_response_data[2]);

	if(cnt == 0 ) return 0;
	
	p_payload[0] = LE_advertise.scan_response_data[0];
	p_payload[1] = LE_advertise.scan_response_data[1];

	if(cnt>2) 
	{
		byteblk= (cnt-2)>>2;
		j =  cnt - 2 - byteblk;
	}
	for(; byteblk > 0; byteblk--)
	{
		 *dest_cpy++ = *src_cpy++;
		 i += 4;
	}
	for(; j>0; j--)
	{
		p_payload[i++] = LE_advertise.scan_response_data[i];
	}
	
	return LE_advertise.scan_response_data_len;



}


/**************************************************************************
 * FUNCTION :- LEadv_Is_Next_Adv_Due
 *
 * DESCRIPTION
 * Determines if the next Advertising Event is due.
 **************************************************************************/

u_int8 LEadv_Is_Next_Adv_Due(u_int32 current_clk)
{
	return (BTtimer_Is_Expired_For_Time(LE_advertise.next_adv_timer, current_clk + 1) && (LE_advertise.advertise_enable == 1));

}
#ifndef REDUCE_ROM2
/**************************************************************************
 * FUNCTION :- LEadv_Get_Adv_Channel_Map
 *
 * DESCRIPTION
 * Returns the advertising channel map.
 **************************************************************************/

u_int8 LEadv_Get_Adv_Channel_Map()
{
	return LE_advertise.advertising_channel_map;
}
#endif
#ifndef REDUCE_ROM2
/**************************************************************************
 * FUNCTION :- LEadv_Get_Adv_Type
 *
 * DESCRIPTION
 * Returns the current advertising type 
 **************************************************************************/

u_int8 LEadv_Get_Adv_Type()
{
	return LE_advertise.advertising_type;
}
#endif
/**************************************************************************
 * FUNCTION :- LEadv_Get_Advertising_Enable
 *
 * DESCRIPTION
 * Returns the value of the Advertising Enable flag
 **************************************************************************/

u_int8 LEadv_Get_Advertising_Enable()
{
	return LE_advertise.advertise_enable;
}

/**************************************************************************
 * FUNCTION :- LEadv_Prep_For_LE_Scan_Resp_Tx
 *
 * DESCRIPTION
 * Prepares the HW and Radio for transmission of SCAN_RESP
 **************************************************************************/

void LEadv_Prep_For_LE_Scan_Resp_Tx(void)
{
	if(PATCH_FUN[LEADV_PREP_FOR_LE_SCAN_RESP_TX_ID]){
         ((void (*)(void))PATCH_FUN[LEADV_PREP_FOR_LE_SCAN_RESP_TX_ID])();
         return ;
    }

	HWle_set_tifs_default();
	HWle_set_tx_enable();
	HWle_clear_tifs_abort();
    HW_set_rx_mode(0x00);
	//HWle_set_rx_mode(0x00);
     ((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_LE_TIFS_TX_RX,LEfreq_Map_Channel_Index_2_RF_Freq(LEadv_Get_Advertising_Channel_Index()),TX_START_FREQ);
	//HWradio_LE_Service(RADIO_MODE_LE_TIFS_TX_RX,LEfreq_Map_Channel_Index_2_RF_Freq(LEadv_Get_Advertising_Channel_Index()),TX_START_FREQ);
}

/**************************************************************************
 *
 *  TCI FUNCTIONS
 *
 *  The following are a set of TCI functions which have proven usefull in the Debug
 *  of Advertising and Radio. These are mainly intended for development and not intended
 *  to be included in PRODUCT.
 *************************************************************************/


#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==1)

/**************************************************************************
 * FUNCTION :- LEadv_TCI_Set_Direct_Adv_Timeout
 *
 * DESCRIPTION
 * Handler for TCI Command which allows the user to program the Direct Advertising
 * timeout.
 **************************************************************************/

void LEadv_TCI_Set_Direct_Adv_Timeout(u_int16 timeout)
{
	LE_advertise.user_direct_adv_timeout = 1;
	LE_advertise.direct_adv_timeout = timeout;
	TCeg_Command_Complete_Event(TCI_LE_DIRECT_ADV_TIMEOUT, NO_ERROR);
}

/**************************************************************************
 * FUNCTION :- LEadv_TCI_Write_Advertising_Delta
 *
 * DESCRIPTION
 * Handler for TCI Command which allows the user to program delta in time between 
 * Advertising Packet Transmission in a single Advertising Event
 **************************************************************************/

t_error LEadv_TCI_Write_Advertising_Delta(u_int8 delta)
{
	LE_advertise.advertising_delta = delta;
	TCeg_Command_Complete_Event(TCI_LE_WRITE_ADVERTISING_DELTA, NO_ERROR);
	return NO_ERROR;
}

/**************************************************************************
 * FUNCTION :- LEadv_TCI_Read_Advertising_Params
 *
 * DESCRIPTION
 * Handler for TCI command to read the current Advertising Parameters used.
 **************************************************************************/

t_error LEadv_TCI_Read_Advertising_Params(void)
{
	 
    if(PATCH_FUN[LEADV_TCI_READ_ADVERTISING_PARAMS_ID]){
         
         return ((t_error (*)(void))PATCH_FUN[LEADV_TCI_READ_ADVERTISING_PARAMS_ID])();
    }

	u_int8 i;
	t_q_descr *qd;
	qd = BTq_Enqueue(HCI_EVENT_Q, 0, 19);
	if(qd)
	{
		qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
		qd->data[1] = 17;
		qd->data[2] = 1;
		_Insert_Uint16(&(qd->data[3]), TCI_LE_READ_ADV_PARAMS);
		qd->data[5] = 0;
		_Insert_Uint16(&(qd->data[6]), LE_advertise.advertising_interval);
		qd->data[8] = LE_advertise.advertising_type;
		qd->data[9] = LE_advertise.advertising_own_address_type;
		qd->data[10] = LE_advertise.direct_address_type;
		for(i=0;i<6;i++)
			qd->data[11+i]= LE_advertise.direct_address[i];
		qd->data[17] = LE_advertise.advertising_channel_map;
		BTq_Commit(HCI_EVENT_Q, 0);
	}
	return NO_ERROR;
}
#endif

u_int8 LEadv_Get_Direct_Adv_Type(void)
{
	return LE_advertise.direct_address_type;
}

u_int8* LEadv_Get_Direct_Adv_Address(void)
{
	return LE_advertise.direct_address;
}

#endif
