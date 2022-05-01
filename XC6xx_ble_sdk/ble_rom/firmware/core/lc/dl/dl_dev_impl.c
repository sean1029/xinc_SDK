/******************************************************************************
 *
 * MODULE NAME:    dl_dev_impl.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    
 * MAINTAINER:     Ivan Griffin
 * DATE:           20 July 1999
 *
 * SOURCE CONTROL: $Id: dl_dev_impl.c,v 1.75 2013/04/08 11:03:54 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *    20 July 1999 -   IG       -
 *    09 July 2000 -   JN Incorporation of bt_addr_man functionality
 *
 ******************************************************************************/
#include "sys_config.h"
#include "hc_event_gen.h" /* for HCeg_Hardware_Error_Event */
#include "dl_dev.h"
#include "lmp_sco_container.h"
#include "lslc_access.h" /* For is_EDR_2Mbit_packet */
#include "global_val.h"
#include "patch_function_id.h"
//extern t_devicelink  device_links[];
#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==1)
extern const s_int8 Power_control_table_dBm[];
#endif

/*
 * Device links transmit descriptors normally point to queue entry
 * This is a general holding descriptor for testmode/initialisation
 */
//STATIC t_q_descr DL_transmit_descr; 

#ifndef __USE_INLINES__
#include "dl_dev_impl.h"
#endif

/*
 * Allocation pool for device links  (max is determined by sizeof(type)*8)
 */
//STATIC u_int32 DL_LC_link_pool;

/*
 * Allocation pool for amaddr       (max is 8 in V1.1)
 * AMADDR 0 is always reserved for the local device for broadcast.
 */
//u_int8 DL_amaddr_pool;
 

/******************************************************************************
 * DL_Initialise_All
 *
 * Initialise the Link Controller Device Links
 ******************************************************************************/
void DL_Initialise_All(void)
{
    t_deviceIndex device_index;
	u_int max_active_devices = SYSconfig_Get_Max_Active_Devices();
    /*
     * Initialise the pool of LC links and active member addresses
     */
    DL_LC_link_pool = (( 1 << max_active_devices) - \
                                 ( 1 << SYSconfig_Get_Local_Device_Index())  - 1 );
    //DL_amaddr_pool  = ALL_AMADDRS_FREE;

    //DL_piconet_index_pool = ALL_PICONETS_FREE;

#if (PRH_BS_CFG_SYS_EXTRA_SYN_LINKS_SUPPORTED==1)
	for(device_index = 1; device_index <= PRH_BS_CFG_SYS_MAX_ACTIVE_DEVICES_PICONET; device_index++)
        DL_eSCO_shared_lt_addrs[device_index] = 0;
#endif

}

/******************************************************************************
 * DL_Alloc_Link
 * Allocate and Initialise Link Controller Device Link if one exists
 *
 * Returns:     valid device index					   if a device link exists
 *              SYSconfig_Get_Max_Active_Devices()     if no LINK available
 ******************************************************************************/
t_deviceIndex DL_Alloc_Link(void)
{
    if(PATCH_FUN[DL_ALLOC_LINK_ID]){
         
         return ((t_deviceIndex (*)(void))PATCH_FUN[DL_ALLOC_LINK_ID])();
    }
    t_deviceIndex device_index;
    u_int         LC_link_pool_entry;
	u_int max_active_devices = SYSconfig_Get_Max_Active_Devices();


    device_index = 0;
    while (device_index < max_active_devices)
    {
        /*
         * Check if entry free by checking if pool entry bit is set
         * If entry is free Then Allocate and Exit Else check next Endif
         */
        LC_link_pool_entry = (1 << device_index);
        if (DL_LC_link_pool & LC_link_pool_entry)
        {
            DL_LC_link_pool &= ~LC_link_pool_entry;
            //DL_Initialise_Link(device_index);
            break;
        }
        else
        {
            device_index++;
        }
    }

    return device_index;
}

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1) // not used

/******************************************************************************
 * DL_Map_LT_ADDR_to_LT_Handle(t_devicelink* p_device_link, t_lt_addr lt_addr)
 * 
 * 
 *
 * Returns:     None
 ******************************************************************************/

u_int8 DL_Map_LT_ADDR_to_LT_Handle(t_devicelink* p_device_link, t_lt_addr lt_addr)
{
	boolean found_match = FALSE;
	u_int8 lt_info_index=0;

	while (!found_match && lt_info_index<(sizeof(p_device_link->lt_info)/sizeof(p_device_link->lt_info[0])))
	{
		if (p_device_link->lt_info[lt_info_index].lt_addr == lt_addr)
		{
			found_match=TRUE;
		}
		else
		{
			lt_info_index++;
		}
	}

	return lt_info_index;
}


void DL_Set_Default_Rx_LT_All_Links(void)
{
    t_deviceIndex device_index;
	t_devicelink *p_device_links = (t_devicelink*) SYSconfig_Get_Device_Links_Location();
	t_devicelink *pDL;
	u_int max_active_devices = SYSconfig_Get_Max_Active_Devices();

    /*
     * Initialise each device link including LOCAL_DEVICE_LINK
     */
    for(device_index=0; device_index < max_active_devices; device_index++)
    {
		pDL = &p_device_links[device_index];
		pDL->rx_lt_index = ACL_SCO_LT;
    }
}

void DL_Set_Default_Tx_LT_All_Links(void)
{
    t_deviceIndex device_index;
    t_devicelink *p_device_links = (t_devicelink*) SYSconfig_Get_Device_Links_Location();
	t_devicelink *pDL;
	u_int max_active_devices = SYSconfig_Get_Max_Active_Devices();

    /*
     * Initialise each device link including LOCAL_DEVICE_LINK
     */
    for(device_index=0; device_index < max_active_devices; device_index++)
    {
		pDL = &p_device_links[device_index];
		pDL->tx_lt_index = ACL_SCO_LT;
    }
}

u_int8 DL_Allocate_eSCO_Link(t_devicelink* p_device_link)
{
	int i=1;
	while (i <=	PRH_BS_CFG_SYS_NB_SUPPORTED_EXTENDED_SCO_CHANNEL)
	{
		if (p_device_link->lt_info[i].lt_addr == 0)
		{
			/*
			 * A non-zero lt_addr is used to indicate whether the LT container
			 * is used.
			 * On finding an unused LT container, it is marked as used by
			 * temporarily setting the lt_addr to a non-zero value.
			 * Note: This is a temporary / invalid lt_addr assignment.
			 * At a later stage, once the lt_addr for this eSCO link is 
			 * known it is expected that the correct lt_addr is assigned
			 * to the LT container.
			 */
			p_device_link->lt_info[i].lt_addr = ESCO_LINK_ALLOCATED;
			return i;
		}
		i++;
	}
	return 0; /*no LT container free */
}
#endif

