/***********************************************************************
 *
 * MODULE NAME:    le_white_list.c
 * PROJECT CODE:   CEVA Low Energy Single Mode
 * DESCRIPTION:    Low Energy White Lists.
 * MAINTAINER:     Gary Fleming
 * CREATION DATE:  July 2011
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
#include "hci_params.h"
#include "bt_timer.h"
#include "le_const.h"
#include "le_white_list.h"
#include "le_config.h"
#include "le_scan.h"
#include "le_advertise.h"
#include "lmp_utils.h"
#include "hw_le_lc.h"
#include "global_val.h"
#include "patch_function_id.h"
extern t_LE_Config LE_config;

/****************************************************************************
 * Duplicate Lists
 *
 * The duplicate lists contain a list of devices from which a given advertising
 * type has been recieved since scanning began. Each time a Advertising Report is
 * sent over the HCI a new entry is added to the corresponding duplicate list.
 *
 * A duplicate list is maintained independently for each type of Advertising Type
 * and the Scan_Resp.
 *****************************************************************************/

#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
extern t_ListEntry LE_adv_ind_duplicate_list[PRH_CFG_MAX_ADV_IND_DUPLICATE_ENTRIES];
extern t_ListEntry LE_adv_direct_duplicate_list[PRH_CFG_MAX_ADV_DIRECT_DUPLICATE_ENTRIES];
extern t_ListEntry LE_scan_ind_duplicate_list[PRH_CFG_MAX_ADV_SCAN_IND_DUPLICATE_ENTRIES];
extern t_ListEntry LE_adv_nonconn_ind_duplicate_list[PRH_CFG_MAX_ADV_NONCONN_IND_DUPLICATE_ENTRIES];
extern t_ListEntry LE_adv_scan_rsp_duplicate_list[PRH_CFG_MAX_SCAN_RSP_DUPLICATE_ENTRIES];
#endif

extern const u_int8 LE_DuplicateListSize[SCAN_RSP_EVENT_TYPE+1] ;

#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
extern t_ListEntry* LE_DuplicateList[SCAN_RSP_EVENT_TYPE+1] ;
#endif
#if (PRH_SYS_CFG_HARDWARE_ADDRESS_FILTERING_SUPPORTED == 0)
t_ListEntry  LEwhiteList[PRH_BS_CFG_SYS_MAX_WHITE_LIST_ENTRIES];
#endif
/**************************************************************************
 * Function :- LEwl_Read_White_List_Size
 *
 * Returns the Maximum number of entries which can be stored in the White List
 *
 ***************************************************************************/
u_int8 LEwl_Read_White_List_Size(void)
{
	return PRH_BS_CFG_SYS_MAX_WHITE_LIST_ENTRIES;
}

/**************************************************************************
 * Function :- LEwl_Clear_White_List
 *
 * Initialises the White list.
 *
 ***************************************************************************/

t_error LEwl_Clear_White_List(void)
{
#if (PRH_SYS_CFG_HARDWARE_ADDRESS_FILTERING_SUPPORTED == 0)
	u_int8 i;

	for (i=0;i<PRH_BS_CFG_SYS_MAX_WHITE_LIST_ENTRIES;i++)
	{
		LEwhiteList[i].used = 0;
	}
//#else
	HWle_set_rx_filters_active(0x00);
#else
	HWle_set_rx_filters_active(0x00);


#endif
	return NO_ERROR;
}

/**************************************************************************
 * Function :- LEwl_Add_Device_To_White_List
 *
 * Adds a device to the white list. Searchs for the first free entry, and adds the
 * device at this point.
 *
 ***************************************************************************/

t_error LEwl_Add_Device_To_White_List(u_int8 address_type, t_p_pdu p_pdu)
{


#if (PRH_SYS_CFG_HARDWARE_ADDRESS_FILTERING_SUPPORTED == 0)
	u_int8 i=0;

	if (address_type > 0x01)
		return INVALID_HCI_PARAMETERS;

	// First Find empty space in White list.

	while (i<PRH_BS_CFG_SYS_MAX_WHITE_LIST_ENTRIES)
	{
		if((LEwhiteList[i].used) && !LEwl_address_compare(LEwhiteList[i].address,p_pdu))
		{
			i++;
		}
		else
		{

			LEwhiteList[i].used = 1;
			LEwhiteList[i].address_type = address_type;

			LMutils_Array_Copy(6,LEwhiteList[i].address,p_pdu);
			return NO_ERROR;
		}
	}
#else
	{
	u_int8 filters_active = HWle_get_rx_filters_active();
	u_int8 filter_index = 0;


	if (filters_active >= 0x3F)
	{
		return MEMORY_FULL;
	}
	else
	{
		// Find first Free Entry in HW.
		if ((filters_active & 0x01) == 0)
		{
			filter_index = 0;
		}
		else if ((filters_active & 0x02) == 0)
		{
			filter_index = 1;
		}
		else if ((filters_active & 0x04) == 0)
		{
			filter_index = 2;
		}
		else if ((filters_active & 0x08) == 0)
		{
			filter_index = 3;
		}
		else if ((filters_active & 0x10) == 0)
		{
			filter_index = 4;
		}
		else if ((filters_active & 0x20) == 0)
		{
			filter_index = 5;
		}

		// Now I have a filter index I can write to the HW.
		{
			u_int32 address32_plus_control[2]; //  = {0xFFFFFFFF,0xFFFFFFFF};
			u_int8* p_addr_plus_control = (u_int8*)address32_plus_control;
			u_int8 j;

			for (j=0;j<6;j++)
				p_addr_plus_control[j] = p_pdu[j];

			//address32_plus_control[1] |= 0x00800000;
			if (address_type==RANDOM_ADDRESS)
				p_addr_plus_control[7] |= 0x40;

			p_addr_plus_control[7] |= 0x80;

			// Use the Shadow Register to set the Addr Filter.
			//HWle_set_rx_filters_active(0x01);
			filters_active |= ((1 << filter_index) );
			HWle_set_rx_filters_active(filters_active);

		    hw_memcpy((u_int8*)(JAL_LE_FILTER_ADDR+(filter_index*8)),address32_plus_control, 8);
		}

		// Address Filtering should only be turned on in the Events...
		// As this will allow different address filtering for each state.
	}
	}
#endif
	return NO_ERROR;

}


/**************************************************************************
 * Function :- LEwl_Remove_Device_From_White_List
 *
 * Removes a device from the Whitelist. Cycles through the Whitelist until it finds the device,
 * then clears the 'used' flag for the entry.
 *
 ***************************************************************************/

t_error LEwl_Remove_Device_From_White_List(u_int8 address_type,t_p_pdu p_pdu)
{
	u_int8 filter_index=0;


	if (address_type > 0x01)
		return INVALID_HCI_PARAMETERS;

#if (PRH_SYS_CFG_HARDWARE_ADDRESS_FILTERING_SUPPORTED == 0)
	filter_index = 0;
	while (i<PRH_BS_CFG_SYS_MAX_WHITE_LIST_ENTRIES)
	{
		if(LEwhiteList[filter_index].used == 1)
		{
			if (LEwhiteList[filter_index].address_type == address_type)
			{
				if (LEwl_address_compare(LEwhiteList[filter_index].address,p_pdu))
				{
					LEwhiteList[filter_index].used = 0;
					return NO_ERROR;
				}
			}

		}
		filter_index++;
	}
#else
	filter_index = 0;
	{   //- LC Add {}.
    	u_int8 filter_array[8];
    	while (filter_index<PRH_BS_CFG_SYS_MAX_WHITE_LIST_ENTRIES)
    	{
    		hw_memcpy(filter_array,(u_int8*)(JAL_LE_FILTER_ADDR+(filter_index*8)), 8);
    		if(filter_array[7] & 0x80)
    		{
    		    u_int8 filter_addr_type=00;

    		    if (filter_array[7] & 0x40)
    		    	filter_addr_type = 0x01;

    			if (filter_addr_type == address_type)
    			{
    				if (LEwl_address_compare(filter_array,p_pdu))
    				{
    					u_int8 j;
    					filter_array[7] &= 0x7F;
    					// Now write the filter back
    					for(j=0;j<6;j++)
    						filter_array[j] = 0;

    				    hw_memcpy((u_int8*)(JAL_LE_FILTER_ADDR+(filter_index*8)),filter_array, 8);
    				    return NO_ERROR;
    				}
    			}
    		}
    		filter_index++;
    	}
	}
#endif
	return INVALID_HCI_PARAMETERS;
}

/**************************************************************************
 * Function :- LEwl_address_compare
 *
 * Compares two 6 byte arrays.
 ***************************************************************************/

u_int8 LEwl_address_compare(const u_int8* addr1,const u_int8* addr2)
{
	u_int8 i;

	for (i=0;i<6;i++)
	{
		if (addr1[i]!= addr2[i])
			return 0;
	}
	return 1;
}

/**************************************************************************
 * Function :- LEwl_Search_White_List
 *
 * Searches the white list for a specific device address and address type.
 *
 ***************************************************************************/

u_int8 LEwl_Search_White_List(u_int8 address_type,const u_int8* p_address)
{
#if (PRH_SYS_CFG_HARDWARE_ADDRESS_FILTERING_SUPPORTED == 0)

	u_int8 i=0;

	while (i<PRH_BS_CFG_SYS_MAX_WHITE_LIST_ENTRIES)
	{
		if(LEwhiteList[i].used == 1)
		{
			if (LEwhiteList[i].address_type == address_type)
			{
				if (LEwl_address_compare(LEwhiteList[i].address,p_address))
				{
					return 1;
				}
			}

		}
		i++;
	}
#endif
	return 0;
}


#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)

/**************************************************************************
 * Function :- LEwl_Add_Device_To_Duplicate_List
 *
 * Description
 * Adds a new device to the duplicate list.
 *
 ***************************************************************************/
u_int8 LEwl_Add_Device_To_Duplicate_List(u_int8 event_type,u_int8 address_type,const u_int8* p_address)
{
  if(PATCH_FUN[LEWL_ADD_DEVICE_TO_DUPLICATE_LIST_ID]){
         
         return ((u_int8 (*)(u_int8,u_int8,const u_int8*))PATCH_FUN[LEWL_ADD_DEVICE_TO_DUPLICATE_LIST_ID])(event_type,address_type,p_address);
      }
	u_int8 i=0;
	u_int8 max_list_entries = LE_DuplicateListSize[event_type];
	t_ListEntry* p_list = LE_DuplicateList[event_type];

	if ((address_type > 0x01) || (p_list == 0))
		return INVALID_HCI_PARAMETERS;

	//
	// First Search is the device already in the dupicate list
	//
	i = 0;
	while (i<max_list_entries)
	{
		if(p_list[i].used == 1)
		{
			if (p_list[i].address_type == address_type)
			{
				if (LEwl_address_compare(p_list[i].address,p_address))
				{
					return 0;
				}
			}

		}
		i++;
	}

	//
	// Device Not Found - so has to be added to list
	//

	// First Find empty space in  list.
	i = 0;
	while (i<max_list_entries)
	{
		if(p_list[i].used)
		{
			i++;
		}
		else
		{
			p_list[i].used = 1;
			p_list[i].address_type = address_type;

			LMutils_Array_Copy(6,p_list[i].address,p_address);

			return 1;
		}
	}
	return 1;
}

/**************************************************************************
 * Function :- LEwl_Init_Duplicates_List
 *
 * Description
 * This function initialises the Duplicate lists.
 *
 ***************************************************************************/

void LEwl_Init_Duplicates_List(void)
{
	
	if(PATCH_FUN[LEWL_INIT_DUPLICATES_LIST_ID]){
         
          ((void (*)(void))PATCH_FUN[LEWL_INIT_DUPLICATES_LIST_ID])();
		      return;
      }
	u_int8 i;

	u_int8 max_list_entries;
	t_ListEntry* p_list=0;
	u_int8 j;
	for (i=0; i <(SCAN_RSP_EVENT_TYPE+1); i++)
	{
		max_list_entries = LE_DuplicateListSize[i];
		p_list = LE_DuplicateList[i];

		for (j=0;j<max_list_entries;j++)
		{
			p_list[j].used = 0;
			p_list[j].address_type = 0;
		}
	}

}
#endif
/**********************************************************************
 * Function :- LEwl_Can_I_Modify_White_Lists
 *
 * Description :- 
 * This function is used to check if the white lists can be modifed.
 * The LE_Clear White List Command, LE Add Device To White List Command and
 * LE Remove Device From White List Command from the HCI all place restrictions
 * on condictions when the White List can be modified.
 **********************************************************************/

u_int8 LEwl_Can_I_Modify_White_Lists(void)
{
#if (PRH_BS_CFG_SYS_LE_BROADCAST_DEVICE==1) || (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
	if (LEadv_Get_Advertising_Enable() && LEadv_Get_Advertising_Filter_Policy())
		return 0;
#endif
#if	0
	if (LEscan_Get_Scan_Enable() && LEscan_Get_Scan_Filter_Policy())
		return 0;
	if ((LE_config.next_initiator_scan_timer != BT_TIMER_OFF) &&
		(LE_config.initiator_filter_policy))
		return 0;
#endif

	return 1;

}
//#endif
