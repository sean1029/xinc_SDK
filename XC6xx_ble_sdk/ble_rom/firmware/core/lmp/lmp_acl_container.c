/**************************************************************************
 * MODULE NAME:    lmp_acl_container.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    LMP ACL link container
 *     
 * AUTHOR:         Gary Fleming
 * DATE:           15-12-1999
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * SOURCE CONTROL: $Id: lmp_acl_container.c,v 1.93 2010/05/18 12:20:24 garyf Exp $
 *
 * DESCRIPTION
 * This is the container object for ACL links. It is responsible for the
 * allocation and de-allocation of ACL links at the LM level.
 *
 *************************************************************************/
#include "sys_config.h" 

#include "lmp_acl_connection.h"
#include "lmp_acl_container.h"
#include "lc_interface.h"
#include "lmp_const.h"
#include "lmp_scan.h"
#include "lmp_utils.h"
#include "lmp_timer.h"
#include "lmp_config.h"
#include "lmp_sec_core.h"
#include "lmp_link_qos.h"

t_link_entry  link_container[PRH_MAX_ACL_LINKS];

/***********************************************************
 * Array used for quick mapping of device index to "p_link" 
 ***********************************************************/
#if ((BUILD_TYPE!=UNIT_TEST_BUILD) && (NON_RF_LINK==0))
static
#endif
t_lmp_link *dev_index_2_p_link[MAX_ACTIVE_DEVICE_LINKS];

/************************************************************************
 *  Function :- LMaclctr_Initialise
 *
 *  Description
 *      Initialises the container of ACL links.
 *
 *************************************************************************/
void LMaclctr_Initialise(void)
{
    int i;
    t_link_entry *p_cont_item;

    for (i=0; i< PRH_MAX_ACL_LINKS; i++)
    {
        p_cont_item = &link_container[i];
        /* 
         * Initialise the link structure 
         */
        LMacl_Initialise(&p_cont_item->entry);

        p_cont_item->entry.handle = i + PRH_ACL_CONNECTION_HANDLE_OFFSET;
        p_cont_item->entry.device_index = i + 1;

        p_cont_item->used = 0;
        dev_index_2_p_link[i] = 0;
   }
}

/************************************************************************
 *  Function :- LMaclctr_Alloc_Link
 *
 *  Description
 *      Allocates an LM ACL link object.
 *************************************************************************/
t_lmp_link* LMaclctr_Alloc_Link(void)
{
    int i = 0;

    t_link_entry* p_cont_item;
    t_deviceIndex device_index;

    while((link_container[i].used == 1) && (i < (PRH_MAX_ACL_LINKS-1)))
    {
        i++;
    }

    p_cont_item = &link_container[i];

    if (p_cont_item->used == 0)
    {
        LMacl_Initialise(&p_cont_item->entry);

        if ( MAX_NUM_CONNECTIONS != LC_Alloc_Link(&device_index ))
        {
            p_cont_item->used = 1;
            p_cont_item->entry.index = i;
            p_cont_item->entry.state = LMP_IDLE;
            p_cont_item->entry.device_index = device_index;
            dev_index_2_p_link[p_cont_item->entry.device_index] = &p_cont_item->entry;
            return &link_container[i].entry;
        }
    }

    return 0;  /* Null pointer indicating that all links allocated */
}     

/************************************************************************
 *  Function :- LMaclctr_Free_Link
 *
 *  Input :-  t_lmp_link*  (A pointer to an LMP ACL link structure)
 *
 *  Description
 *      Frees an ACL link. Both the LM ACL link object is freed and the 
 *      corresponding Baseband Link is also released.
 *************************************************************************/
void  LMaclctr_Free_Link(t_lmp_link *p_link)
{
    if (p_link)
    {
        p_link->state = LMP_IDLE;

        BDADDR_Set_LAP(&p_link->bd_addr, 0);
        BDADDR_Set_UAP(&p_link->bd_addr, 0);
        BDADDR_Set_NAP(&p_link->bd_addr, 0);

#if 1 // GF 13 May - #2285 Safest to move this mapping prior to the clearing of 
	  // any timers. As this prevents interrupt context from trying to access 
	  // timers for a link which has been detached.
        dev_index_2_p_link[p_link->device_index] = 0;
#endif
        /*
         * Ensure that any timers associated with a link are also cleared.
         * Several other timers are cleared in Disconnection_Complete event.
         */
        if(p_link->flush_timeout_index)
        {
            LMtmr_Clear_Timer(p_link->flush_timeout_index);
        }
        if(p_link->msg_timer)
        {
            LMtmr_Clear_Timer(p_link->msg_timer);       
        }
        LMtmr_Clear_Timer(p_link->supervision_timeout_index);
        LMtmr_Clear_Timer(p_link->recent_contact_timeout_index);

        if(p_link->gen_detach_timeout_index)
        {
             LMtmr_Clear_Timer(p_link->gen_detach_timeout_index);
        }

        if(p_link->gen_switch_timeout_index)
        {
             LMtmr_Clear_Timer(p_link->gen_switch_timeout_index);
        }

        if (p_link->gen_security_timeout_index)
        {
             LMtmr_Clear_Timer(p_link->gen_security_timeout_index);
        }


#if (PRH_BS_CFG_SYS_LMP_PAUSE_ENCRYPTION_SUPPORTED==1)
		if(p_link->encryption_key_refresh_index)
             LMtmr_Clear_Timer(p_link->encryption_key_refresh_index);
#endif

#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
        if (p_link->pol_info.park_info.park_timeout_index)
        {
            LMtmr_Clear_Timer(p_link->pol_info.park_info.park_timeout_index);
            p_link->pol_info.park_info.park_timeout_index = 0;
            LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_PARK_PENDING);
        }
        if (p_link->pol_info.park_info.unpark_timeout_index)
        {
            LMtmr_Clear_Timer(p_link->pol_info.park_info.unpark_timeout_index);
            p_link->pol_info.park_info.unpark_timeout_index = 0;
            LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_UNPARK_PENDING);
        }
#endif

#if (PRH_BS_CFG_SYS_LMP_CHANNEL_DRIVEN_QUALITY_SUPPORTED==1)
    LMtmr_Clear_Timer(p_link->channel_quality_timeout_index);
#endif

        p_link->flush_timeout_index = 0;
        p_link->supervision_timeout_index = 0;
        p_link->recent_contact_timeout_index = 0;
        p_link->msg_timer = 0;

        p_link->channel_quality_timeout_index = 0;
      
        p_link->gen_detach_timeout_index = 0;
        p_link->gen_switch_timeout_index = 0;
        p_link->gen_security_timeout_index = 0;

        p_link->queue_flush_pending = 0;
		p_link->sched_queue_flush_pending = 0;
        p_link->flush_type = PRH_NO_FLUSH;
		p_link->flush_status = PRH_NO_FLUSH_PENDING;

#if(PRH_BS_CFG_SYS_LMP_NONFLUSHABLE_PBF_SUPPORTED==1)
        p_link->flush_packet_type = AUTO_FLUSHABLE;
	    p_link->enhanced_queue_flush_pending = 0;
#endif
		// GF  05 November 2008
		p_link->l2cap_tx_enable = 0;

		p_link->connection_request_send_via_hci = 0x0;

#if (PRH_BS_DEV_EXTENDED_QOS_NOTIFICATIONS_SUPPORTED==1)
        /*
         * Clear potential LMP_QOS ACK pending for this device index
         */
        LMqos_Clear_LMP_QOS_Notification_Ack_Pending(p_link->device_index);
#endif

        /*
         * Clean up any pending security transactions.
         */
        LMsec_core_Clean_On_Detach(p_link);
#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
		/*
		 * Force a simple pairing complete event ( TBD )
		 * and re-init all the SSP related vars. 
		 *
		 */ 
		p_link->ssp_initiator = 0x00;
#endif
        /* 
         * Free the associated LC link 
         */
        LC_Free_Link(p_link->device_index);

        /* 
         * Free the entry in the link container 
         *  
         */

        link_container[p_link->index].used = 0;

#if 0 // GF 13 May - #2285 Safest to move this mapping prior to the clearing of 
	  // any timers. As this prevents interrupt context from trying to access 
	  // timers for a link which has been detached.
        dev_index_2_p_link[p_link->device_index] = 0;
#endif
    }
}

/************************************************************************
 *  Function :- LMaclctr_Find_Bd_Addr
 *
 *  Inputs :-  t_bd_addr *p_bd_addr  (A pointer to a BD Address)
 *
 *  Output :-  t_lmp_link*  (A pointer to an LMP ACL link structure)
 *
 *  Description
 *      Finds a LM ACL link object given a BD_ADDR
 *************************************************************************/
t_lmp_link* LMaclctr_Find_Bd_Addr(const t_bd_addr *p_bd_addr)
{
    u_int8 index = 0;
    t_lmp_link *p_link;

    while(index < PRH_MAX_ACL_LINKS) 
    {
        p_link = &link_container[index].entry;      
        if ((link_container[index].used) && 
            (LMutils_Bd_Addr_Match(p_bd_addr,&p_link->bd_addr)))
        {
            return p_link;
        }
        else
        {
            index++;
        }
    }
    return 0;
}

/************************************************************************
 *  Function :- LMaclctr_Find_Peer_Bd_Addr
 *
 *  Inputs :-  t_bd_addr *p_bd_addr  (A pointer to a BD Address)
 *
 *  Output :-  t_lmp_link*  (A pointer to an LMP ACL link structure)
 *
 *  Description
 *      Finds a LM ACL link object given a BD_ADDR of a remote device. 
 *************************************************************************/
t_lmp_link* LMaclctr_Find_Peer_Bd_Addr(const t_bd_addr *p_bd_addr)
{

    if (LMutils_Bd_Addr_Match(p_bd_addr, SYSconfig_Get_Local_BD_Addr_Ref()))
    {
        return 0;
    }
    else
    {
        return LMaclctr_Find_Bd_Addr(p_bd_addr);
    }
}

/************************************************************************
 *  Function :- LMaclctr_Find_Device_Index
 *
 *  Inputs :-  t_deviceIndex device_index (The device index for the ACL Link)
 *
 *  Output :-  t_lmp_link*  (A pointer to an LMP ACL link structure)
 *
 *  Description
 *      Finds a LM ACL link object given a Device Index
 *************************************************************************/
t_lmp_link* LMaclctr_Find_Device_Index(t_deviceIndex device_index)
{  
    return dev_index_2_p_link[device_index];
}

#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
/************************************************************************
 *  Function :- LMaclctr_Find_ARADDR
 *
 *  Inputs :-  u_int8 ar_addr
 *
 *  Output :-  t_lmp_link*
 *
 *  Description
 *  Finds the lmp_link with a given AR_ADDR. Used during slave initiated  
 *  unpark in the master.
 *************************************************************************/
t_lmp_link* LMaclctr_Find_ARADDR(u_int8 ar_addr)
{
    u_int8 index=0;
    t_link_entry* p_cont_item;

    do
    {
        p_cont_item = &link_container[index];

        if (p_cont_item->used )
        {
            if ((p_cont_item->entry.state == LMP_PARK_MODE) && 
                (p_cont_item->entry.pol_info.park_info.ar_addr == ar_addr))
            {
                return &p_cont_item->entry;
            }
        }
        index++;
    } while (index < PRH_MAX_ACL_LINKS);

    return 0;
}
#endif



/************************************************************************
 *
 *  Function :- LMaclctr_Find_Handle
 *
 *  Inputs :-  t_connectionHandle handle  (The connection handle for the ACL link)
 *
 *  Output :-  t_lmp_link*   (A pointer to an LMP ACL link structure)
 *
 *  Description
 *      Finds a LM link object given a Device Index. 
 *      Encompasses ACL links and the broadcast object.
 *************************************************************************/
t_lmp_link* LMaclctr_Find_Handle(t_connectionHandle handle)
{
    u_int8 index = 0;
    t_lmp_link* p_link;

    while(index < PRH_MAX_ACL_LINKS) 
    {
        if (link_container[index].used)
        {
            p_link = &link_container[index].entry;
            if ((p_link->handle == handle) && (p_link->state != LMP_IDLE))
            {
                return p_link;
            }
            else
            {
                index++;
            }
        }
        else
        {
            index++;
        }
    }
    return 0;
}

/************************************************************************
 *  Function :- LMaclctr_Find_ACL_Handle
 *
 *  Inputs :-  t_connectionHandle handle  (The connection handle for the ACL link)
 *
 *  Output :-  t_lmp_link*   (A pointer to an LMP ACL link structure)
 *
 *  Description
 *      Finds a LM ACL link object given a Device Index. 
 *      ONLY encompasses valid ACL links. EXCLUDES the Broadcast link.
 *************************************************************************/
t_lmp_link* LMaclctr_Find_ACL_Handle(t_connectionHandle handle)
{
    t_lmp_link *p_link;

    p_link = LMaclctr_Find_Handle(handle);

    /* 
     * if a broadcast link then exclude
     */
    if (p_link && (p_link->role == MASTER) && (
         (handle == g_LM_config_info.active_broadcast_handle) || 
         (handle == g_LM_config_info.piconet_broadcast_handle)) )
    {
        p_link = 0;
    }
    
    return  p_link;
}
