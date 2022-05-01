/**************************************************************************
 * MODULE NAME:    lmp_link_policy.c       
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    LMP Link Policy Engine
 * AUTHOR:         Gary Fleming
 * DATE:           09-02-2000
 *
 * SOURCE CONTROL: $Id: lmp_link_policy.c,v 1.390 2014/03/11 03:14:00 garyf Exp $
 *
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *
 * NOTES:
 * This module encompasses the link policy engine which determines what the 
 * next activity to be performed on a piconet is.
 * 
 * It controls the polling intervals for ACLs on a piconet, and the timing 
 * of SCOs. It is also responsible for determining the packet type (Dx1-Dx5)
 * to be used for any given transmission on the piconet channel.
 *
 **************************************************************************/

#include "sys_config.h"

#include "hc_event_gen.h"
#include "hc_const.h"

#include "dl_dev.h"
#include "lc_interface.h"
#include "lslc_access.h"                /* For is_ACL_CRC_Packet check only  */
#include "lslc_slot.h"          /* Indicate if a 3 slot eSCO packet is expected in the next Rx slot */
#include "tra_queue.h"
#include "bt_test.h"
#include "bt_timer.h"
#include "lmp_const.h"
#include "lmp_acl_connection.h"
#include "lmp_acl_container.h"
#include "lmp_ch.h"
#include "lmp_config.h"
#include "lmp_debug.h"
#include "lmp_encode_pdu.h"
#include "lmp_features.h"
#include "lmp_inquiry.h"
#include "lmp_link_policy.h"
#include "lmp_link_policy_protocol.h"
#include "lmp_link_qos.h"
#include "lmp_sco_container.h"
#include "lmp_timer.h"
#include "lmp_scan.h"
#include "sys_mmi.h"
#include "hw_codec.h"
#include "hw_leds.h"
#include "tra_queue.h"

#include "le_link_layer.h"
#include "sys_debug_config.h"
#include "patch_function_id.h"


#if defined(SYS_HAL_OS_EVENT_DRIVEN_HOST_CONTROLLER)
#include "bt_mini_sched.h"
#endif


#if (__DEBUG_LE_SLEEP == 0)
/***********************************************************
* Function :- LMpol_Sleep
*
* Description
* Determines if the device should go in low power mode or not.
* If required it invokes the LC_Sleep procedure.
***********************************************************/
void LMpol_Sleep(u_int16 interval, t_lmp_link *p_link, u_int32 clock)
{
	if(PATCH_FUN[LMPOL_SLEEP_ID]){
         ((void (*)(u_int16 interval, t_lmp_link *p_link, u_int32 clock))PATCH_FUN[LMPOL_SLEEP_ID])(interval, p_link,   clock);
         return ;
    }
    t_deviceIndex dev_index;
	
	// I can only call sleep mode if all ACL links are in Low Power mode
	// If they are not then I have to call Change_Piconet to return to 
	// active ACL until the next sleep interval.
#ifndef BLUETOOTH_MODE_LE_ONLY	
	if ((g_LM_config_info.links_in_low_power) && LMconfig_LM_Connected_As_Scatternet())
	{
		if (g_LM_config_info.num_acl_links != g_LM_config_info.links_in_low_power)
		{
			LMch_Change_Piconet(0);
			return;
		}
		else
		{
			// Store the Sleep information so that we can call the sleep function
			// once the change Piconet has completed..

			LMch_Change_Piconet(g_LM_config_info.next_wakeup_link);
		}
		
	}
	else if (g_LM_config_info.num_acl_links != g_LM_config_info.links_in_low_power)
	{
		return;
	}
#endif
#ifndef BLUETOOTH_MODE_LE_ONLY
	if(LMscoctr_Get_Number_SYN_Connections()==0)
	{			
#if (PRH_BS_CFG_SYS_SLEEP_MASTER_SUPPORTED==0)
		if(LMconfig_LM_Connected_As_Master())
			return;
#endif
		
#if (PRH_BS_CFG_SYS_SLEEP_SLAVE_SUPPORTED==0)
			if(LMconfig_LM_Connected_As_Slave()) 
				return;
#endif			
			//if (LMconfig_LM_Connected_As_Scatternet())
			{
				dev_index = (p_link)?(p_link->device_index):(0);

				{
					t_slots slots_to_scan;
					t_clock next_instant;
					
					LC_Is_Sleep_Possible(&next_instant);
					slots_to_scan = LMscan_Get_Interval_To_Next_Scan(next_instant);
					
					if (slots_to_scan < interval)
					{
						interval = slots_to_scan; 
						dev_index = 0;
					}
				}

				{
					t_slots slots_to_periodic_inq;
					// Check periodic inq
					slots_to_periodic_inq = LMinq_Get_Interval_To_Periodic_Inq();

					if (slots_to_periodic_inq < interval)
					{
						interval = slots_to_periodic_inq; 
						dev_index = 0;
					}
				
				}

				if ((dev_index == 0) && (g_LM_config_info.next_wakeup_link))
					dev_index =g_LM_config_info.next_wakeup_link->device_index;
#endif//BLUETOOTH_MODE_LE_ONLY				
#if (PRH_BS_CFG_SYS_LE_DUAL_MODE==1)
				{
					t_slots le_sleep_slots = LE_LL_InactiveSlots();
					if (le_sleep_slots < interval)
					{
						interval = le_sleep_slots;
						dev_index = 0;
					}
				}
#endif
				 if( (interval > SYS_CFG_MIN_SLOTS_FOR_SLEEP_PROCEDURE) && (g_LM_config_info.next_wakeup_time > ((clock>>1) + SYS_CFG_MIN_SLOTS_FOR_SLEEP_PROCEDURE)) )
				{
					LC_Sleep(interval - SYS_CFG_MIN_SLOTS_FOR_SLEEP_PROCEDURE, dev_index);
				}
				  
				

//				SYSmmi_Display_Event(eSYSmmi_LC_Disconnection_Event);
#ifndef BLUETOOTH_MODE_LE_ONLY
			}
		}
#endif	
}
#endif

