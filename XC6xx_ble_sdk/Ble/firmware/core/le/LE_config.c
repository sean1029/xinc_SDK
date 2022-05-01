/***********************************************************************
 *
 * MODULE NAME:    le_config.c
 * PROJECT CODE:   CEVA Low Energy Single Mode
 * DESCRIPTION:    Low Energy Configuration file.
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
#include "hc_const.h"
#include "hci_params.h"
#ifndef BLUETOOTH_MODE_LE_ONLY
#include "lmp_scan.h"
#include "lmp_ch.h"
#include "lmp_acl_container.h"
#endif
#include "le_advertise.h"
#include "le_scan.h"
#include "le_const.h"
#include "le_config.h"
#include "le_connection.h"
#include "le_link_layer.h"
#include "le_white_list.h"
#include "le_frequency.h"
#include "le_scan.h"
#include "sys_rand_num_gen.h"
#include "hw_radio.h"

#include "bt_timer.h"
#include "lslc_irq.h"
#include "le_white_list.h"

#include "hw_lc.h"
#include "hw_le_lc.h"

#include "tc_event_gen.h"
#include "tc_const.h"
#include "global_val.h"
#include "patch_function_id.h"
/*--------------------------------------------
#define      __DEBUG_LE_SLEEP           0     //- for: TP/CON/SLA/BI-01-C
												 TP/CON/ADV/BV-04-C
		 										 TP/CON/SLA/BV-15-C
		 										 TP/CON/SLA/BI-02-C
		 										 TP/SEC/SLA/BV-01-C

#define      __DEBUG_LE_SLEEP          	2	  //- for: Normol WORK.	
---------------------------------------------*/
extern u_int8 g_debug_le_sleep;   //  no sleep:0   sleep: 2     __DEBUG_LE_SLEEP 
extern u_int8 g_debug_xinchip_rf_lowpower_enable;

extern u_int8 g_le_enable_observer_flag; //PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE;
extern u_int8 g_le_enable_central_flag;   //  PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE




//t_LE_Config LE_config;



extern u_int8 le_mode;
#define LE_MAX_NUMBER_OF_EVENTS 20

/*
 * Initial release is a fully functional system with all states and state combinations supported.
 * Later to be combined with Build options which will define the supported states in any given
 * configuration.
 */
const u_int8 LEconfig_Classic_BT_Features[8] = {0x00,0x00,0x00,0x00,0x60,0x00,0x00,0x00}; /* LE supported, BR/EDR not supported */
const u_int8 _LEconfig_supported_states[8] = {0xFF,0xFF,0xFF,0x1F,0x00,0x00,0x00,0x00};
const u_int8 _LEconfig_local_features[8]   = {PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED,0x00,0x00,0x00,0x00,0x00,0x00,0x00};


//void *le_call_back_list[LE_MAX_NUMBER_OF_EVENTS];
//u_int8 g_LE_Config_Classic_Dev_Index = 0;
/**************************************************************************
 * FUNCTION :- LEconfig_Init
 *
 * DESCRIPTION
 * Initialisation of the LE_config container and all its elements.
 *
 **************************************************************************/

t_error LEconfig_Init(void)
{

    if(PATCH_FUN[LECONFIG_INIT_ID]){
         
         return ((t_error (*)(void))PATCH_FUN[LECONFIG_INIT_ID])();
    }

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
	u_int8 i;
#endif

	BDADDR_Set_to_Zero(&LE_config.random_address);
	g_LE_Config_Classic_Dev_Index = 0;
	LEadv_Init();
	LEscan_Init();
#if 1
	LE_config.hc_data_tester_present = 0;	// WW change 1 to 0
#else // Below Code Lets the behaviour of the connection completes be dependent on the Switch Settings on the
	  // Terasic Platform
	{
		u_int16* volatile switches =  ( u_int16 *)(0x80000000|SLIDE_SWITCHES_BASE);

		if ((*switches) & 0x8000)
		{
			LE_config.hc_data_tester_present = 1;
		}
		else
		{
			LE_config.hc_data_tester_present = 0;
		}
	}
#endif
	LE_config.next_initiator_scan_timer = BT_TIMER_OFF;
	LE_config.initiating_active = 0;
	LE_config.state = STANDBY_STATE;
	LE_config.sub_state = SUBSTATE_IDLE;
	LE_config.event_mask = 0x1F;
	LE_config.sleep_clock_accuracy = PRH_BS_CFG_MASTER_SLEEP_CLOCK_ACCURACY;

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
	LE_config.link_id_pool = 0xFFFFFFFF;

	LE_config.next_connection_event_time = BT_TIMER_OFF;

	for (i=0;i<4;i++)
		LE_config.data_channel_map[i]=0xFF;

	LE_config.data_channel_map[4] = 0x1F;

	LE_config.num_le_links=0;

	LE_config.connection_complete_pending = 0;
	LE_config.p_connection_complete_pending = 0;
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
	LE_config.link_key_request_timer = BT_TIMER_OFF;
	LE_config.encrypt_pending_flag=0x00;
#endif
	LEconnection_Init();
#endif
	LE_config.device_sleep=0;
	LE_config.tabasco_asleep = 0;
	LE_config.test_mode = 0;
	LE_config.slave_link_active = 0;
	LE_config.master_link_active = 0;
	LEwl_Clear_White_List();
#if (PRH_BS_CFG_SYS_LE_SMP_INCLUDED==1)
	LEsmp_Init_SMP();
#if ((PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1) || (PRH_BS_CFG_SYS_LE_SMP_PERIPHERAL_SUPPORTS_KEY_HIERARCHY!=1))
	LEsecdb_Clear_Security_DB();
#endif

#endif
	HWle_set_whitening_enable();

	// Full investigation of below options required for Each Specific Radio
	//!!!!!! !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// This one of the most sensitive parameter settings in HW for LE
	//
	//HWle_set_delay_search_win(50);
	//HWle_set_delay_search_win(63);
	
	LE_config.Auto_Widen_MultiSlot = 0;
	//HWle_set_tifs_delay(TIFS_TX_ADJUSTMENT-1);
	//HWle_set_tifs_delay(63);
	*((volatile unsigned int*)(0x4002A0A0)) = ((RX_DELAY_DEFINE<<8)|(RX_DELAY_DEFINE+11));	//- 
	//*((volatile unsigned int*)(0x4002A0A0)) = ((RX_DELAY_DEFINE<<8)|(RX_DELAY_DEFINE));	//- 
	//*((volatile unsigned int*)(0x4002A0A0)) = ((60<<8)|60);	//- 
	
	/* Ensure that no TIFS count is running
	 */
	HWle_abort_tifs_count();
	HWle_clear_tifs_default();

#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==1)
	LE_config.auto_advertise = 0x00;
	LE_config.auto_initiate = 0x00;
	LE_config.slave_listen_outside_latency = 0;
	LE_config.TCI_transmit_win_config = 0x00;
#endif
	LE_config.trace_level = 0x31;
	LE_config.resuming_scan = 0;

	LE_config.initiating_enable = 0;

#if (SYS_CFG_LE_ADVERTISE_BY_DEFAULT_TO_OVERCOME_ORCA_RF_ISSUE==1)
	LE_config.auto_advertise = 0x01;
	LEadv_Set_Advertise_Enable(1);
#else
	LE_config.auto_advertise = 0x00;
#endif

	return NO_ERROR;
}
#ifndef REDUCE_ROM
/*************************************************************************
 * LEconfig_Register_Event_Handler
 *
 * Register the presented event handler for immediate call back on event
 *************************************************************************/
void LEconfig_Register_Event_Handler(u_int8 event_id,
                                 void *func_ptr)
{
  // Registration for HCI not complete yet - optional extra //(PRH_BS_CFG_SYS_LE_HCI_INCLUDED)
  //  le_call_back_list[event_id] = func_ptr;
}
#endif

/**************************************************************************
 * FUNCTION :- LEconfig_Set_Event_Mask
 *
 * DESCRIPTION
 * Sets the HCI event mask for LE events.
 *
 **************************************************************************/

void LEconfig_Set_Event_Mask(u_int8* p_pdu)
{
	/*
	 * As only one of the eight bytes in the event mask is actually used.
	 * We restrict our event mask to a single byte.
	 */

    LE_config.event_mask = p_pdu[0];
} 

/**************************************************************************
 * FUNCTION :- LEconfig_Get_LE_Features_Ref
 *
 * DESCRIPTION
 * Returns a pointer to the set of LE supported features.
 *
 **************************************************************************/


const u_int8* LEconfig_Get_LE_Features_Ref(void)
{
	return _LEconfig_local_features;
}

/**************************************************************************
 * FUNCTION :- LEconfig_Is_Features_Encryption_Supported
 *
 * DESCRIPTION
 * Used to determine if encryption is supported in the current device.
 * Should be replaced by a build parameter as this is a waste of code.
 *
 **************************************************************************/


u_int8 LEconfig_Is_Features_Encryption_Supported(void)
{
	return (_LEconfig_local_features[0] & PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED);
}

/**************************************************************************
 * FUNCTION :- LEconfig_Set_Random_Address
 *
 * DESCRIPTION
 * Sets the RANDOM Address to be used by a device.
 *
 **************************************************************************/

void LEconfig_Set_Random_Address(u_int8* p_random_address)
{
	BDADDR_Assign_from_Byte_Array(&LE_config.random_address, p_random_address);
}


/***************************************************************************
 * Function :- LEconfig_Read_Supported_States
 *
 * Description :-
 * Simple function to return the current combination of states supported in
 * an LE device.
 *
 * Context :- Schedular
 ***************************************************************************/

const u_int8* LEconfig_Read_Supported_States(void)
{
	return _LEconfig_supported_states;
}

/***************************************************************************
 * Function :- LEconfig_Check_Current_Address
 *
 * Description :-
 * This fuction checks if a given address and address type match the local
 * address type and address.
 *
 * Context :- IRQ
 ***************************************************************************/

u_int8 LEconfig_Check_Current_Address(u_int8 RxAdd,t_p_pdu p_pdu)
{	 
    if(PATCH_FUN[LECONFIG_CHECK_CURRENT_ADDRESS_ID]){
         
         return ((u_int8 (*)(u_int8 RxAdd,t_p_pdu p_pdu))PATCH_FUN[LECONFIG_CHECK_CURRENT_ADDRESS_ID])(RxAdd,  p_pdu);
    }


	//  Checks if there is a match to the current device address..
	// 
	u_int8 i;

	if (
#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
		((LE_config.state == SCANNING_STATE) && ( RxAdd == LEscan_Get_Scanning_Own_Address_Type())) ||
#endif
		 ((LE_config.state == ADVERTISING_STATE) && (RxAdd == LEadv_Get_Advertising_Own_Address_Type())))

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

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)

/***************************************************************************
 * Function :- LEconfig_Allocate_Link_Id
 *
 * Description :-
 * This function allocates an Identifier for an LE link.
 *
 ***************************************************************************/

u_int8 LEconfig_Allocate_Link_Id(u_int8* p_link_id)
{
	if(PATCH_FUN[LECONFIG_ALLOCATE_LINK_ID_ID]){
         
         return ((u_int8 (*)(u_int8* p_link_id))PATCH_FUN[LECONFIG_ALLOCATE_LINK_ID_ID])(p_link_id);
    }

	u_int8 i=0;
	u_int32 pool_entry;
	// This allocates a free link id.
	// use a u_int32 to store 32 link id as a bitfield.

	while( i < DEFAULT_MAX_NUM_LE_LINKS)
	{
		pool_entry = (1 << i);
		if ( LE_config.link_id_pool & pool_entry)
		{
			LE_config.link_id_pool &= ~pool_entry;
			*p_link_id = i;
			return NO_ERROR;
		}
		else
		{
			i++;
		}
	}
	return UNSPECIFIED_ERROR;
}

/***************************************************************************
 * Function :- LEconfig_Free_Link_Id
 *
 * Description :-
 * This function allocates an Identifier for an LE link.
 *
 ***************************************************************************/

void LEconfig_Free_Link_Id(t_LE_Connection* p_connection)
{
	LE_config.link_id_pool |=  (1 << p_connection->link_id);
}


/***************************************************************************
 * Function :- LEconnections_Link_In_Use
 *
 * Description :-
 * This funtion checks if a link id is in use.
 *
 ***************************************************************************/

u_int8 LEconnections_Link_In_Use(u_int8 link_id)
{
	return !(LE_config.link_id_pool & (1 << link_id));
}

#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==1)

/***************************************************************************
 * Function :- LEconfig_TCI_Set_TIFS_Tx_Adjustment
 *
 * Description :-
 * This functions is used to support the equivalent TCI command. This writes
 * the TIFS adjustment directly to the HW.
 *
 ***************************************************************************/

t_error LEconfig_TCI_Set_TIFS_Tx_Adjustment(u_int8 tifs_adjustment)
{
	HWle_set_tifs_delay(tifs_adjustment);

	TCeg_Command_Complete_Event(TCI_LE_SET_TIFS_TX_ADJUSTMENT, NO_ERROR);
	return NO_ERROR;
}

/***************************************************************************
 * Function :- LEconfig_TCI_Set_Search_Window_Delay
 *
 * Description :-
 * This functions is used to support the equivalent TCI command. This writes
 * the Search Delay directly to the HW.
 *
 ***************************************************************************/

t_error LEconfig_TCI_Set_Search_Window_Delay(u_int8 search_win_delay)
{
	HWle_set_delay_search_win(31-search_win_delay);

	TCeg_Command_Complete_Event(TCI_LE_SET_SEARCH_WINDOW_DELAY, NO_ERROR);
	return NO_ERROR;
}

#endif
#ifndef REDUCE_ROM
void LEconfig_Switch_To_LE(u_int8 device_index)
{
	if(PATCH_FUN[LECONFIG_SWITCH_TO_LE_ID]){
         ((void (*)(u_int8 device_index))PATCH_FUN[LECONFIG_SWITCH_TO_LE_ID])(device_index);
         return ;
    }

#if 0
	g_LE_Config_Classic_Dev_Index = device_index;

	// Determine the Next LE Activity --
	// This will determine what Role the device should Take
	//    1/ Advertising -- Slave Role
	//    2/ Scanning   -- Master Role
	//    3/ Initiating -- Master Role
	//    4/ Slave ACL  -- Slave Role
	//    5/ Master ACL -- Master Role

	if (LE_config.slave_link_active )
	{
		// Advertising || Slave ACL
	    HW_set_slave(1);
	    HWle_clear_master_mode();
#if 0
	    {
	    	extern u_int16 g_LE_Slave_IntraSlot;
	        HW_set_intraslot_offset(g_LE_Slave_IntraSlot);
	    }
#endif
	}
	else
	{
		// Scanning || Initiating || Master ACL

    	HW_set_slave(0);
		HWle_set_master_mode();
	}
	HWradio_Switch_To_LE_Mode();
	HW_set_page(0);
#endif		
	HWle_set_le_mode();

}
#endif
#ifndef BLUETOOTH_MODE_LE_ONLY	//no need to switch to classic
void LEconfig_Switch_To_BT_Classic(void)
{

    // If scanning is the only thing active in classic. Then we need to set the
	// Slave bit when switching to Classic.
	HW_set_sync_det_intr_mask(0);
	HW_set_pkd_intr_mask(0);

   	LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM0_INTR_MSK_MASK |JAL_TIM2_INTR_MSK_MASK | JAL_TIM1_INTR_MSK_MASK |JAL_TIM3_INTR_MSK_MASK);
	HWradio_Switch_To_Classic_BT_Mode();

	//if ((g_LE_Config_Classic_Dev_Index!=0))
	{
		t_lmp_link *p_link;

		p_link = LMaclctr_Find_Device_Index(g_LE_Config_Classic_Dev_Index);
		LMch_Change_Piconet(p_link);
	}

}
#endif
#endif
	
