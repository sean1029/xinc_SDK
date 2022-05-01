/**************************************************************************
 * MODULE NAME:    lmp_link_qos.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    LMP Link Quality of Service
 * MAINTAINER:     Gary Fleming
 * DATE:           23-Nov-1999
 *
 * SOURCE CONTROL: $Id: lmp_link_qos.c,v 1.163 2014/03/11 03:14:01 garyf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *   All rights reserved.
 *
 * NOTES:
 * This module is responsible for handling all the LM Quality of Service
 * interactions with the peer.
 *
 * Modification of the following are supported  :
 *
 *     Packet_Type 
 *     Poll Interval 
 *     Max Slots
 *     Preferred Rate
 *     Auto Rate
 *     Supervision Timeout
 *     Quality of Service
 *************************************************************************/
#include "sys_config.h"

#include "lc_interface.h"

#include "lmp_const.h"
#include "lmp_types.h"
#include "lmp_config.h"
#include "lmp_encode_pdu.h"
#include "lmp_acl_connection.h"
#include "lmp_acl_container.h"
#include "lmp_link_policy.h"
#include "lmp_sco_container.h"
#include "lmp_link_qos.h"
#include "lmp_timer.h"
#include "lmp_utils.h"
#include "lmp_features.h"
#include "lmp_ch.h"
#include "lmp_scan.h"

#include "bt_test.h"

#include "lc_types.h"

#include "hc_const.h"
#include "hc_event_gen.h"

/***************************************
 * Local Macros to handle useful mappings
 ***************************************/


/*
 * Packet types  available for max_slots 1, 3, 5
 */
const static u_int16 _LMqos_max_slot_pkt_types[3] = { HCI_1_SLOT,
    (HCI_1_SLOT|HCI_3_SLOT), (HCI_1_SLOT|HCI_3_SLOT|HCI_5_SLOT) };
#define  mLMqos_Max_Slots_2_Packets(max_slots)      \
    (_LMqos_max_slot_pkt_types[(max_slots) >> 1] )

#define  mLMqos_MicroSec_To_Slots(latency)          \
    (((( (latency) *16L)/10000) < 2) ? 2 : ((( (latency) *16L)/10000)/2)*2 )
#define  mLMqos_Slots_to_Micro_Sec(poll_interval)   \
    (( (poll_interval)*10000)/16)  
#define  mLMqos_Valid_SCO_Packets(packet_types)     \
    ((!( (packet_types) & (~0x00E0))) && ((packet_types) & 0x00E0))
#define  mLMqos_Valid_ACL_Packets(packet_types)     \
    ((!( (packet_types) & (~HCI_ACL))) && (packet_types & HCI_ACL))

/***************************************
 * Local Functions to generate events 
 * to the higher layers
 ***************************************/

static void _Send_HC_Max_Slots_Change_Event(t_lmp_link* p_link,u_int8 max_slots);
static void _Send_HC_QoS_Setup_Complete_Event(t_lmp_link* p_link,t_error outcome);
#if (PRH_BS_CFG_SYS_FLOW_SPECIFICATION_SUPPORTED==1)
static void _Send_HC_Flow_Spec_Complete_Event(t_lmp_link* p_link,t_error outcome);
#endif

static u_int32 LMqos_Max_Packet_Size(const t_HCIpacketTypes packet_types);

/***************************************
 * Local Functions manipulate PDU contents
 * and parameters.
 ***************************************/
#if (PRH_BS_CFG_SYS_QOS_SUPPORTED==1)
static t_error LMqos_Update_Link_QoS(t_lmp_link* p_link,u_int8 service,u_int32 latency,u_int32 tk_rate);
#endif
t_error LMqos_Update_Link_Flow_Spec(
     t_lmp_link *p_link, u_int8 direction, u_int8 service, u_int32 tk_rate, u_int32 latency);
u_int32 LMqos_Determine_Latency_From_Packet_Types(t_lmp_link* p_link);
t_error LMqos_Check_Link_QoS(t_lmp_link* p_link,u_int8 service,u_int32 latency,u_int32 tk_rate);

t_error LMqos_Check_Link_Flow_Specification(t_lmp_link* p_link,u_int8 direction,u_int8 service,
                                         u_int32 latency, u_int32 tk_rate, u_int32 pk_bandwidth);
u_int16 LMqos_Get_Communication_Interval(
     t_lmp_link *p_link, u_int8 direction, u_int32 latency, u_int32 tk_rate);
#if (PRH_BS_CFG_SYS_LMP_CHANNEL_DRIVEN_QUALITY_SUPPORTED==1)
const static u_int16 br_rate_to_pkt_mapping[4] = { HCI_DM1, HCI_MEDIUM_RATE, HCI_HIGH_RATE, HCI_ACL_BR };
const static u_int16 edr_rate_to_pkt_mapping[4] = { HCI_DM1, HCI_ACL_2MBS_EDR, HCI_ACL_3MBS_EDR, HCI_ACL_EDR|HCI_DM1 };
#endif

#if (PRH_BS_DEV_EXTENDED_QOS_NOTIFICATIONS_SUPPORTED==1)
STATIC u_int8 _LMqos_qos_ack_pending_on_slaves;
#endif

void LMqos_Initialise(void)
{
#if (PRH_BS_DEV_EXTENDED_QOS_NOTIFICATIONS_SUPPORTED==1)
    LMqos_Clear_LMP_QOS_Notification_Ack_Pending_All_Slaves();
#endif
}


/********************************************************************************
 * FUNCTION :- LMqos_LM_Flow_Specification
 *
 * DESCRIPTION :-
 * This function maps directly to the HCI_Flow_Specification command. It allows the
 * the quality of service to be changed on a link.
 *
 * The only important parameters are the latency and tk_rate and direction.
 * Depending on the Direction this is used to control either the Transmission interval
 * or the poll interval and the Packet Types.
 *
 * Latency is measured in microseconds. The Token Rate is measured in Bytes per
 * second and both the poll interval (or Transmission interval) and the packet type are
 * coupled to the token rate. If the packet type changes the poll interval may also need 
 * to be changed to respect the token rate
 *
 ******************************************************************************/
t_error LMqos_LM_Flow_Specification(u_int8 direction, t_lmp_link* p_link,u_int8 service,
                                    u_int32 tk_rate, u_int32 tk_bucket_size, u_int32 pk_bandwidth,
                                    u_int32 latency)
{
#if (PRH_BS_CFG_SYS_FLOW_SPECIFICATION_SUPPORTED==1)
    t_error status;
    t_lmp_pdu_info pdu_info; 
    u_int16 interval;
  
    /* Firstly Check if this Current Flow Spec can be supported */
    status = LMqos_Check_Link_Flow_Specification(p_link,direction,service,latency,tk_rate,pk_bandwidth);
    
    /* This determines 
     *  What the poll interval OR transmission interval should be.
     */

    interval = LMqos_Get_Communication_Interval(p_link,direction,latency,tk_rate);

    if (interval != 0)
    {
        if (direction == 0)
        {
            p_link->direction = direction;

            if (p_link->role == MASTER)
            {
                p_link->proposed_tx_interval = interval;
                p_link->proposed_poll_interval = p_link->poll_interval;
                p_link->out_latency = latency;
                p_link->out_peak_bandwidth = pk_bandwidth;
                p_link->out_service_type = service;
                p_link->out_token_bucket_size = tk_bucket_size;
                p_link->out_token_rate = tk_rate;

            }
            else
            {
                p_link->out_proposed_latency = latency;
                p_link->out_proposed_peak_bandwidth = pk_bandwidth;
                p_link->out_proposed_service_type = service;
                p_link->out_proposed_token_bucket_size = tk_bucket_size;
                p_link->out_proposed_token_rate = tk_rate;
                p_link->proposed_poll_interval = interval;
            }

        }
        else if (direction== 1)
        {
            p_link->proposed_poll_interval = interval;

            p_link->in_proposed_latency = latency;
            p_link->in_proposed_peak_bandwidth = pk_bandwidth;
            p_link->in_proposed_service_type =  service;
            p_link->in_proposed_token_bucket_size = tk_bucket_size;
            p_link->in_proposed_token_rate = tk_rate;
            p_link->direction = direction;
        }
    }
           
    if ((interval != 0) && (status == NO_ERROR))
    {
        p_link->flow_spec_pending = 0x01;
        p_link->current_proc_tid_role = p_link->role;
        pdu_info.timeout = p_link->proposed_poll_interval;
        pdu_info.opcode = LMP_QUALITY_OF_SERVICE_REQ;
        pdu_info.tid_role = p_link->current_proc_tid_role;
        pdu_info.Nbc = (u_int8)g_LM_config_info.Nbc;
        LM_Encode_LMP_PDU(p_link->device_index,&pdu_info); 
    }      
    return status;
#else /* if (PRH_BS_CFG_SYS_FLOW_SPECIFICATION_SUPPORTED==1) */
    return ILLEGAL_COMMAND;
    
#endif
}

/********************************************************************************
 * FUNCTION :- LMqos_LM_QOS_Setup
 *
 * DESCRIPTION :-
 * This function maps directly to the HCI_Quality_Of_Service command. It allows the
 * the quality of service to be changed on a link.
 *
 * The only important parameters are the latency and tk_rate.
 * latency is measured in microseconds. The Token Rate is measured in Bytes per
 * second and both the poll interval and the packet type are coupled to the token rate
 * If the packet type changes the poll interval may also need to be changed to respect
 * the token rate
 ******************************************************************************/

t_error LMqos_LM_QOS_Setup(t_lmp_link* p_link,u_int8 service,u_int32 tk_rate,
                           u_int32 pk_bandwidth,u_int32 latency,u_int32 delay)
{
#if (PRH_BS_CFG_SYS_QOS_SUPPORTED==1)
    t_error status;
    t_lmp_pdu_info pdu_info;          
    
    status = LMqos_Check_Link_QoS(p_link,service,latency,tk_rate);
    
    if (status == NO_ERROR)
    {
        p_link->current_proc_tid_role = p_link->role;
        pdu_info.timeout = p_link->proposed_poll_interval;
        pdu_info.opcode = LMP_QUALITY_OF_SERVICE_REQ;
        pdu_info.tid_role = p_link->current_proc_tid_role;
        pdu_info.Nbc = (u_int8)g_LM_config_info.Nbc;
        LM_Encode_LMP_PDU(p_link->device_index,&pdu_info); 
    }      
    return status;
#else /* if (PRH_BS_CFG_SYS_QOS_SUPPORTED==1) */
    return ILLEGAL_COMMAND;
    
#endif
}

#if (PRH_BS_DEV_EXTENDED_QOS_NOTIFICATIONS_SUPPORTED==1)

/******************************************************************************
 * FUNCTION :- LMqos_Is_LMP_QOS_Notification_Ack_Pending
 ******************************************************************************/
boolean LMqos_Is_LMP_QOS_Notification_Ack_Pending(void)
{
    return _LMqos_qos_ack_pending_on_slaves;
}

/******************************************************************************
 * FUNCTION :- LMqos_Set_LMP_QOS_Notification_Ack_Pending
 ******************************************************************************/
void LMqos_Set_LMP_QOS_Notification_Ack_Pending(t_deviceIndex device_index)
{
    _LMqos_qos_ack_pending_on_slaves |= (u_int8) (1<<device_index);
}

/******************************************************************************
 * FUNCTION :- LMqos_Clear_LMP_QOS_Notification_Ack_Pending
 ******************************************************************************/
void LMqos_Clear_LMP_QOS_Notification_Ack_Pending(t_deviceIndex device_index)
{
    _LMqos_qos_ack_pending_on_slaves &= ~(u_int8) (1<<device_index);
}

/******************************************************************************
 * FUNCTION :- LMqos_Clear_LMP_QOS_Notification_Ack_Pending_All_Slaves
 ******************************************************************************/
void LMqos_Clear_LMP_QOS_Notification_Ack_Pending_All_Slaves(void)
{
    _LMqos_qos_ack_pending_on_slaves = 0;
}


/******************************************************************************
 * FUNCTION :- LMqos_Notify_QOS_All_Slaves
 *
 * Notify a change in QOS to all active slaves.
 *
 * Used when master performs an inq/page on active connection to notify slaves
 * of temporary suspension to service. Although BT Spec does not require QOS to
 * be respected during inq/page, slave cannot know master is inq/page, so will
 * otherwise cause issues about poll_interval based timeouts on slave in such
 * scenarios, eg detach ack timeout.
 *
 ******************************************************************************/
void LMqos_Notify_QOS_All_Slaves(boolean enable)
{
#if (PRH_BS_CFG_SYS_QOS_SUPPORTED==1)
    extern t_link_entry link_container[PRH_MAX_ACL_LINKS];
    t_lmp_pdu_info pdu_info;
    u_int i;

    _LMqos_qos_ack_pending_on_slaves = 0;

    pdu_info.opcode = LMP_QUALITY_OF_SERVICE;
    pdu_info.tid_role = MASTER;
    pdu_info.Nbc = (u_int8)g_LM_config_info.Nbc;

    if (enable) /* restore QOS poll interval to slaves */
    {
        for (i = 0; i < PRH_MAX_ACL_LINKS; i++)
        {
            if (link_container[i].used) 
            {
                t_lmp_link *p_link = &(link_container[i].entry);
                
                if ((p_link->state & LMP_ACTIVE|LMP_LOW_POWER_STATE) && (MASTER == p_link->role)
                     && (p_link->poll_interval != p_link->proposed_poll_interval))
                {
                    /*
                     * Notify slaves if restored QOS poll interval. If in low
                     * power state, encode for when resume active mode.
                     */
                    p_link->poll_interval = p_link->proposed_poll_interval;
                    pdu_info.timeout = p_link->poll_interval;
                    LM_Encode_LMP_PDU(p_link->device_index,&pdu_info); 
                }
            }
        }
    }
    else /* suspend QOS poll interval to slaves */
    {
        pdu_info.timeout = g_LM_config_info.emergency_poll_interval
            * (g_LM_config_info.num_master_acl_links);

        for (i = 0; i < PRH_MAX_ACL_LINKS; i++)
        {
            if (link_container[i].used) 
            {
                t_lmp_link *p_link = &(link_container[i].entry);
                
                if ((p_link->state & LMP_ACTIVE) && (MASTER == p_link->role))
                {
                    /*
                     * Notify all active slaves of emergency poll slots. Block
                     * new chac procedures until all slaves acknowledge.
                     */
                    p_link->proposed_poll_interval = p_link->poll_interval;
                    p_link->poll_interval = pdu_info.timeout;
                    LMqos_Set_LMP_QOS_Notification_Ack_Pending(p_link->device_index);
                    LM_Encode_LMP_PDU(p_link->device_index,&pdu_info); 
                }
            }
        }
    }
#endif
}

#endif /*(PRH_BS_DEV_EXTENDED_QOS_NOTIFICATIONS_SUPPORTED==1)*/



t_packetTypes LMqos_LM_Gen_Allowed_Packet_Types(t_HCIpacketTypes acl_packet_types)
{
    u_int32 hci_packet_types = acl_packet_types;
    t_packetTypes allowed_pkt_types = 0;

    if (hci_packet_types & HCI_DM1)
        allowed_pkt_types |= DM1_BIT_MASK;
    if (hci_packet_types & HCI_DH1)
        allowed_pkt_types |= DH1_BIT_MASK;
    if (hci_packet_types & HCI_DM3)
        allowed_pkt_types |= DM3_BIT_MASK;
    if (hci_packet_types & HCI_DH3)
        allowed_pkt_types |= DH3_BIT_MASK;
    if (hci_packet_types & HCI_DM5)
        allowed_pkt_types |= DM5_BIT_MASK;
    if (hci_packet_types & HCI_DH5)
        allowed_pkt_types |= DH5_BIT_MASK;

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
    if (hci_packet_types & HCI_2DH1)
        allowed_pkt_types |= EDR_2DH1_BIT_MASK;
    if (hci_packet_types & HCI_2DH3)
        allowed_pkt_types |= EDR_2DH3_BIT_MASK;
    if (hci_packet_types & HCI_2DH5)
        allowed_pkt_types |= EDR_2DH5_BIT_MASK;
    if (hci_packet_types & HCI_3DH1)
        allowed_pkt_types |= EDR_3DH1_BIT_MASK;
    if (hci_packet_types & HCI_3DH3)
        allowed_pkt_types |= EDR_3DH3_BIT_MASK;
    if (hci_packet_types & HCI_3DH5)
        allowed_pkt_types |= EDR_3DH5_BIT_MASK;
#endif
    return allowed_pkt_types;
}

//#if (BUILD_TYPE==UNIT_TEST_BUILD)

t_HCIpacketTypes LMqos_LM_Get_HCI_ACL_Packet_Types(t_packetTypes allowed_packet_types)
{
    u_int32 hci_packet_types = 0;

    if (allowed_packet_types & DM1_BIT_MASK)
        hci_packet_types |= HCI_DM1;
    if (allowed_packet_types & DH1_BIT_MASK)
        hci_packet_types |= HCI_DH1;
    if (allowed_packet_types & DM3_BIT_MASK)
        hci_packet_types |= HCI_DM3;
    if (allowed_packet_types & DH3_BIT_MASK)
        hci_packet_types |= HCI_DH3;
    if (allowed_packet_types & DM5_BIT_MASK)
        hci_packet_types |= HCI_DM5;
    if (allowed_packet_types & DH5_BIT_MASK)
        hci_packet_types |= HCI_DH5;

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
    if (allowed_packet_types & EDR_2DH1_BIT_MASK)
        hci_packet_types |= HCI_2DH1;
    if (allowed_packet_types & EDR_2DH3_BIT_MASK)
        hci_packet_types |= HCI_2DH3;
    if (allowed_packet_types & EDR_2DH5_BIT_MASK)
        hci_packet_types |= HCI_2DH5;
    if (allowed_packet_types & EDR_3DH1_BIT_MASK)
        hci_packet_types |= HCI_3DH1;
    if (allowed_packet_types & EDR_3DH3_BIT_MASK)
        hci_packet_types |= HCI_3DH3;
    if (allowed_packet_types & EDR_3DH5_BIT_MASK)
        hci_packet_types |= HCI_3DH5;
#endif

    return hci_packet_types;
}

//#endif

/***************************************************************
 * Function :- LMqos_LM_Change_SCO_Packet_Type
 *
 * Description
 * Allows SCO packet types to be changed on a given link.
 * First do extensive checks before admitting operation.
 **************************************************************/  
t_error LMqos_LM_Change_SCO_Packet_Type(u_int16 handle, t_HCIpacketTypes packet_types)
{
    t_error status = UNSUPPORTED_FEATURE;

#if (PRH_BS_CFG_SYS_CHANGE_SCO_PACKET_TYPE_SUPPORTED==1)

    t_sco_info* p_sco_link;
    p_sco_link  = LMscoctr_Find_SCO_By_Connection_Handle(handle);
    if (p_sco_link) 
    {
        /**********************************************************
         * It is a SCO handle - with p_sco_link pointing to the SCO
         **********************************************************/
         t_lmp_link *p_link = LMaclctr_Find_Device_Index(p_sco_link->device_index);

        /*
         * Check for validity:
         * 1. Check link is valid and that detach is not pending
         * 2. Check that packets_types are only SCO packets
         * 3. Check that packets selected are supported by remote device
         * 4. Check if more than 1 connection or currently negotiating
         */
        if (!p_link || p_link->operation_pending == LMP_DETACH)
        {
            return COMMAND_DISALLOWED;
        }
        if ( !(packet_types & (HV1_BIT_MASK|HV2_BIT_MASK|HV3_BIT_MASK))
          || (packet_types & ~(HV1_BIT_MASK|HV2_BIT_MASK|HV3_BIT_MASK)))
        {
            return INVALID_HCI_PARAMETERS;
        }
        if ( ((packet_types & HV2_BIT_MASK) && 
                !mFeat_Check_HV2_Packets(p_link->remote_features)) ||
             ((packet_types & HV3_BIT_MASK) && 
                !mFeat_Check_HV3_Packets(p_link->remote_features)))
        {
            return UNSUPPORTED_REMOTE_FEATURE;
        }
        if (LMscoctr_Get_Number_SCO_Connections() > 1 ||
            LMscoctr_Get_SCO_Transitioning_Ref())
        {
            /*
             * Invalid if more than 1 connection or currently negotiating
             */
            return COMMAND_DISALLOWED;
        }
		
		if (g_LM_config_info.num_acl_links > 1)
        {
            packet_types &= ~HV1_BIT_MASK;
        }

        if(LMscoctr_Convert_HCI_To_LM_SCO_Packet_Type(packet_types) == (p_sco_link->lm_sco_packet))
        {
            /*
             * This packet type already supported so no LMP required. Update allowed sco packet
             * types and send packet type changed event.
             */
            t_lm_event_info event_info;
            status = NO_ERROR;
            HCeg_Command_Status_Event(status, HCI_CHANGE_CONNECTION_PACKET_TYPE);
            event_info.handle = handle;
            event_info.status = status;
            event_info.value16bit = p_sco_link->packet_types = packet_types;


            HCeg_Generate_Event(HCI_CONNECTION_PACKET_TYPE_CHANGED_EVENT,&event_info);
        }
        else
        {
            status = LMconnection_LM_Change_SCO_Packet_Type(p_link, p_sco_link, packet_types);
            if(status == NO_ERROR)
            {
                HCeg_Command_Status_Event(status, HCI_CHANGE_CONNECTION_PACKET_TYPE);
                status = NO_ERROR;
            }
        }
    } 
#endif /*(PRH_BS_CFG_SYS_CHANGE_SCO_PACKET_TYPE_SUPPORTED==1)*/
    return status;
}

/***************************************************************
 * Function :- LMqos_LM_Change_Packet_Type
 *
 * Description
 * Allows the packet types to be changed on a given link.
 * First do extensive checks before admitting operation.
 *
 * Split into subfunctions (for operation with ptt requests):
 * LMqos_LM_Validate_Change_Packet_Type
 * LMqos_LM_Change_Validated_Packet_Type
 **************************************************************/  
t_error LMqos_LM_Validate_Change_Packet_Type(u_int16 handle, t_HCIpacketTypes packet_types)
{
    t_error status = UNSUPPORTED_FEATURE;

#if (PRH_BS_CFG_SYS_CHANGE_ACL_PACKET_TYPE_SUPPORTED==1)
    {   /******************************************************
         *Its an ACL handle and I can work on the p_link basis 
         ******************************************************/
        t_lmp_link *p_link = LMaclctr_Find_Handle(handle);

        if(p_link)
        {
            u_int8 *local_features = SYSconfig_Get_LMP_Features_Ref();

            /*
             * Check for validity:
             * 1. Check that packets_types are only ACL packets
             * 2. Check that packets selected are supported by local device
             * 3. Check that packets selected are supported by remote device
             * 4. Check if more than 1 connection or currently negotiating
             */
            if (!(packet_types & HCI_ACL) || (packet_types & ~HCI_ACL) )
            {
                return INVALID_HCI_PARAMETERS;
            }

            if(((packet_types & HCI_3_SLOT) && !mFeat_Check_3_Slot(local_features)) ||
               ((packet_types & HCI_5_SLOT) && !mFeat_Check_5_Slot(local_features)))
            {
                return UNSUPPORTED_FEATURE;
            }

            if (((packet_types & HCI_ACL_2MBS_EDR) && !mFeat_Check_EDR_ACL_2MBPS_Mode(local_features)) ||
                ((packet_types & HCI_ACL_3MBS_EDR) && !mFeat_Check_EDR_ACL_3MBPS_Mode(local_features)) ||
                ((packet_types & HCI_3_SLOT_EDR) && !mFeat_Check_3_Slot_EDR_ACL_Packets(local_features)) ||
                ((packet_types & HCI_5_SLOT_EDR) && !mFeat_Check_5_Slot_EDR_ACL_Packets(local_features)))
            {
                return UNSUPPORTED_FEATURE;
            }
                        
            if(((packet_types & HCI_3_SLOT) && !mFeat_Check_3_Slot(p_link->remote_features)) ||
               ((packet_types & HCI_5_SLOT) && !mFeat_Check_5_Slot(p_link->remote_features)))
            {
                return UNSUPPORTED_REMOTE_FEATURE;
            }

            if (((packet_types & HCI_ACL_2MBS_EDR) && !mFeat_Check_EDR_ACL_2MBPS_Mode(p_link->remote_features)) ||
                ((packet_types & HCI_ACL_3MBS_EDR) && !mFeat_Check_EDR_ACL_3MBPS_Mode(p_link->remote_features)) ||
                ((packet_types & HCI_3_SLOT_EDR) && !mFeat_Check_3_Slot_EDR_ACL_Packets(p_link->remote_features)) ||
                ((packet_types & HCI_5_SLOT_EDR) && !mFeat_Check_5_Slot_EDR_ACL_Packets(p_link->remote_features)))
            {
                return UNSUPPORTED_REMOTE_FEATURE;
            }

            if(p_link->operation_pending == LMP_DETACH)
            {    
                return COMMAND_DISALLOWED;
            }

            /*
             * MULTISLOT_ACL_WITH_SCO: Customer requested the Bandwidth is taken
             * into consideration only when choosing packet type which LC uses, and
             * not to reject parameter of packet type specified based on HVs.
             */
#if (PRH_BS_DEV_MULTISLOT_ACL_WITH_SCO_SUPPORTED==0)
            if((packet_types & (HCI_3_SLOT|HCI_5_SLOT)) && LMscoctr_Get_Number_SCO_Connections())
            {
                return COMMAND_DISALLOWED;
            }
#endif
            status = NO_ERROR; /* Validated */
		}
        else
        {
            status = NO_CONNECTION; /* Invalid Handle */
        }
    }
#endif
    return status;
}

#if (PRH_BS_CFG_SYS_CHANGE_ACL_PACKET_TYPE_SUPPORTED==1)
t_error LMqos_LM_Change_Validated_Packet_Type(u_int16 handle, t_HCIpacketTypes packet_types)
{
    t_error status = NO_ERROR;
    t_lmp_link *p_link = LMaclctr_Find_Handle(handle);
    u_int8 max_slots;
  
    //if(p_link) // p_link etc already validated
    {
        p_link->proposed_packet_types = packet_types;
        max_slots = LMqos_Convert_To_Max_Slots(packet_types);

        /* Only send max slots req if the max slots has changed */
		if (( max_slots != p_link->max_slots_out)&& !((packet_types & (HCI_3_SLOT|HCI_5_SLOT))
			 && LMscoctr_Get_Number_SCO_Connections()))
		{
			if( max_slots != p_link->max_slots_out)
				p_link->max_slots_out_pending = max_slots;

            p_link->packet_types = packet_types;
            LMqos_Update_Allowed_Packets(p_link,max_slots,p_link->rate);
#if (PRH_BS_CFG_SYS_QOS_SUPPORTED==1)
            LMqos_Update_Link_QoS(p_link,p_link->service_type, p_link->latency, p_link->token_rate);
#endif
			p_link->packet_type_changed_event = 1;
			status = LMqos_LM_Max_Slot_Req(p_link, max_slots);
                
		}
		else
		{	
			if( max_slots == p_link->max_slots_out)
			{
				p_link->packet_types = packet_types;
				LMqos_Update_Allowed_Packets(p_link,max_slots,p_link->rate);
#if (PRH_BS_CFG_SYS_QOS_SUPPORTED==1)
				LMqos_Update_Link_QoS(p_link,p_link->service_type, p_link->latency, p_link->token_rate);
#endif
				/*
				 * Special error code to tell hc_cmd_disp that it must generate
				 * a connection packet type changed event.
                 */
				status = UNSPECIFIED_ERROR;
			}
			else
			{
				p_link->max_slots_out_pending = max_slots;
				p_link->packet_type_changed_event = 1;
				status = LMqos_LM_Max_Slot_Req(p_link, max_slots);
			}
		}
	}

    return status;
}
#endif

t_error LMqos_LM_Change_Packet_Type(u_int16 handle, t_HCIpacketTypes packet_types)
{
    t_error status = LMqos_LM_Validate_Change_Packet_Type(handle, packet_types);

#if (PRH_BS_CFG_SYS_CHANGE_ACL_PACKET_TYPE_SUPPORTED==1)
    if (NO_ERROR == status)
    {   /******************************************************
         *Its an ACL handle and I can work on the p_link basis 
         ******************************************************/
        status = LMqos_LM_Change_Validated_Packet_Type(handle, packet_types);
    }
#endif
    return status;
}

#if (PRH_BS_CFG_SYS_LMP_CHANNEL_DRIVEN_QUALITY_SUPPORTED==1)
/***************************************************************
 * Function :- LMqos_Channel_Quality_Timeout
 *
 * Description
 * Reads the LC channel quality parameters and determines if the 
 * default packet type should change. 
 * This function is periodically called based on a time.
 **************************************************************/ 
void LMqos_Channel_Quality_Timeout(t_lmp_link* p_link)
{
    const t_ch_quality* p_chn_quality;
    u_int8 rx_quality; /* the ratio of Rx Packets to Error Rxs - High Value = Good Quality ; Low Value = Poor Quality*/
    u_int8 tx_quality; /* the ratio of Tx Packets to Error Txs - High Value = Good Quality ; Low Value = Poor Quality*/
    boolean reset_cqstats = FALSE;

    if(p_link == 0) 
    {
        return;
    }

#if (PRH_BS_CFG_SYS_TEST_MODE_SUPPORTED==1)
    if (BTtst_Get_DUT_Mode() >= DUT_ACTIVE)
    {
        return;
    }
#endif

    if (p_link->state & LMP_ACTIVE)
    {
        p_chn_quality = LC_Get_Channel_Quality_Ref(p_link->device_index); 
        
        /* Check if there is an Ack pending */

        /********************************/
        /* First check the Tx Direction */
        /********************************/
        
        if (p_link->rate != AUTO)
        {
            /* 
             * Ensure sufficient Tx packets to gather metrics 
             */
            if(p_chn_quality->num_tx_acl_packets >  PRH_CHN_QUALITY_MIN_NUM_PACKETS)
            {
                if(p_chn_quality->num_tx_acl_packets == p_chn_quality->num_tx_acl_packets_acked)
                {
                    /* 
                     * Seperate case to preclude Divide by Zero on calculating tx_quality 
                     */
                    tx_quality = 0xFF;
                    p_link->link_quality = tx_quality;
                }
                else if (p_chn_quality->num_tx_acl_packets_acked > p_chn_quality->num_tx_acl_packets)
                {
                    /* 
                     * B831
                     * Occurs where the LC_Initialise_Channel_Quality has occurred but an ack 
                     * is pending.  Hence, just record the corresponding data packet.
                     */
                    DL_Inc_Tx_ACL_Data_Packets(DL_Get_Device_Ref(p_link->device_index));
                    LMtmr_Reset_Timer(p_link->channel_quality_timeout_index, (p_link->channel_quality_timeout ));
                    return;
                }
                else
                {
                    tx_quality =((p_chn_quality->num_tx_acl_packets)/((p_chn_quality->num_tx_acl_packets+1) \
                                 - p_chn_quality->num_tx_acl_packets_acked));
                    p_link->link_quality = tx_quality;
                }

                if (tx_quality < PRH_TX_CHN_QUALITY_DM_PACKET_THRESHOLD)
                {
                    LMqos_Update_Allowed_Packets(p_link,p_link->max_slots_out,MEDIUM);
                }
                else if(tx_quality > PRH_TX_CHN_QUALITY_DH_PACKET_THRESHOLD)
                {
                    LMqos_Update_Allowed_Packets(p_link,p_link->max_slots_out,HIGH);
                }
                reset_cqstats = TRUE;
            }   
        }

        /*****************************************************************/
        /* Now Check the Rx Direction                                    */
        /* Only Applicable if the peer has send an LMP_Auto_Rate Command */
        /*****************************************************************/

        if (p_link->peer_rate == AUTO)
        {
            /*
             * Gather suffient Rx data => No Modification to the Rx Direction until sufficient
             * data gathered.
             */
            if(p_chn_quality->num_rx_acl_packets >  PRH_CHN_QUALITY_MIN_NUM_PACKETS)
            {
                /* 
                 * Determine the channel Quality --  0..255
                 *    0  = Very Very Poor Quality
                 *   255 = Very Good Quality
                 */
                if (p_chn_quality->num_rx_acl_packets_nacked == 0)
                {
                    rx_quality = 0xff;
                }
                else if(p_chn_quality->num_rx_acl_packets == 
                    p_chn_quality->num_rx_acl_packets_nacked)
                {
                    rx_quality = 0x00;
                }
                else
                {
                    rx_quality = ((p_chn_quality->num_rx_acl_packets)/((p_chn_quality->num_rx_acl_packets+1) 
                        - (p_chn_quality->num_rx_acl_packets - p_chn_quality->num_rx_acl_packets_nacked)));
                }
                p_link->link_quality = rx_quality;

                /* 
                 * Now Take Action dependent on the rx_quality 
                 */
                if ((rx_quality < PRH_RX_CHN_QUALITY_DM_PACKET_THRESHOLD) && (p_link->preferred_rate != MEDIUM))
                {
                    LMqos_Send_LMP_Preferred_Rate(p_link,MEDIUM);
                }
                else if((rx_quality > PRH_RX_CHN_QUALITY_DH_PACKET_THRESHOLD) && (p_link->preferred_rate != HIGH))
                {
                    LMqos_Send_LMP_Preferred_Rate(p_link,HIGH);
                }
                reset_cqstats = TRUE;
            }
        }
    }
    
    if(reset_cqstats)
    {
        LC_Initialise_Channel_Quality(p_link->device_index);
    }
    LMtmr_Reset_Timer(p_link->channel_quality_timeout_index, (p_link->channel_quality_timeout ));
}
#endif

/*********************************************
 *********************************************
 **    SUPPORT FOR MAX SLOTS MODIFICATION   **
 *********************************************
 *********************************************/

/***************************************************************
 * Function :- LMqos_LM_Force_Max_Slots_In
 *
 * Description
 * Propietrary Command to allow the Max Slots on a link
 * to be changed via LM Service Interface.
 **************************************************************/ 
t_error LMqos_LM_Force_Max_Slots_In(t_lmp_link* p_link, u_int8 max_slots)
{
    t_lmp_pdu_info  pdu_info; 

    p_link->current_proc_tid_role = p_link->role;
    pdu_info.max_slots = max_slots;
    pdu_info.opcode = LMP_MAX_SLOT;
    pdu_info.tid_role = p_link->current_proc_tid_role;
    LM_Encode_LMP_PDU(p_link->device_index,&pdu_info);
    return NO_ERROR;
}

/***************************************************************
 * Function :- LMqos_LM_Max_Slot_Req
 **************************************************************/ 
t_error LMqos_LM_Max_Slot_Req(t_lmp_link* p_link, u_int8 max_slots)
{
	t_lmp_pdu_info  pdu_info; 

    if (max_slots != p_link->max_slots_out)
    {
		p_link->current_proc_tid_role = p_link->role;
        pdu_info.max_slots = max_slots;
        pdu_info.opcode = LMP_MAX_SLOT_REQ;
        pdu_info.tid_role = p_link->current_proc_tid_role;
        LM_Encode_LMP_PDU(p_link->device_index,&pdu_info);
    }

    return NO_ERROR;
}

/******************************************************************
 * Function :- LMqos_LMP_Max_Slot
 *
 * Description
 * Handler for a LMP_Max_Slot PDU, which forces a max slot change.
 *
 * Additional Logic needs to included here to check with link poliy 
 * if the device is allowed the max slots value 
 *****************************************************************/
t_error LMqos_LMP_Max_Slot(t_lmp_link *p_link, t_p_pdu p_payload)
{
    u_int8 max_slots = *p_payload;

    if ((max_slots == 1) || (max_slots == 3) || (max_slots == 5))
    {
        u_int8 num_scos_active = LMscoctr_Get_Number_SCO_Connections();

        if(num_scos_active && max_slots>1)
        {
#if (PRH_BS_DEV_MULTISLOT_ACL_WITH_SCO_SUPPORTED==1)
            if( (LMscoctr_Get_Used_LM_SCO_Packet_Type()==LM_SCO_PACKET_HV3)
             && (num_scos_active==1) )
                max_slots = 3;
            else
#endif
                max_slots = 1;
        }

        if ((p_link->role == SLAVE) && (p_link->packet_types == HCI_DM1 /*default*/) && (max_slots > 1))
        {
               p_link->packet_types = HCI_ACL; /* override to allow multislot packets */
        }
        
        LMqos_Update_Allowed_Packets(p_link,max_slots,p_link->rate);
        if (p_link->max_slots_out != max_slots)
        {
            p_link->max_slots_out = max_slots;
            _Send_HC_Max_Slots_Change_Event(p_link,(u_int8)max_slots);
        }
    }
    else
    {
        return INVALID_LMP_PARAMETERS;
    }

    return NO_ERROR;
}


/***********************************************************************
 * FUNCTION :- LMqos_Update_Allowed_Packets
 *
 * DESCRIPTION :- 
 * Using the Max_Slots, current rate ( M/H) and the packet types supported
 * on a link this determines the allowed set of packet for the link.
 *
 * p_link               reference to link container
 * max_slots            1, 3 or 5 slots
 * rate                 MEDIUM or HIGH (when PTT == 0)
 * rate                 DM1, 2MBPS or 3MBPS (when PTT == 1)
 ***********************************************************************/
void LMqos_Update_Allowed_Packets(
     t_lmp_link* p_link, u_int8 max_slots, t_rate rate)
{
    t_HCIpacketTypes acl_packet_types = p_link->packet_types;

    if (max_slots != 0)
    {
        acl_packet_types &= mLMqos_Max_Slots_2_Packets(max_slots);
    }

    p_link->rate = rate; /* note contextual meaning based on ptt value */

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
    if (LC_Get_PTT_ACL(p_link->device_index) == 1)
    {
        acl_packet_types &= HCI_ACL_EDR|HCI_DM1;

#if (PRH_BS_CFG_SYS_LMP_CHANNEL_DRIVEN_QUALITY_SUPPORTED==1)
        if (acl_packet_types & edr_rate_to_pkt_mapping[rate])
        {
            acl_packet_types &= edr_rate_to_pkt_mapping[rate];
        }
#endif
    
    }
    else
#endif
    {
        acl_packet_types &= HCI_ACL_BR;

#if (PRH_BS_CFG_SYS_LMP_CHANNEL_DRIVEN_QUALITY_SUPPORTED==1)
        if (acl_packet_types & br_rate_to_pkt_mapping[rate])
        {
            acl_packet_types &= br_rate_to_pkt_mapping[rate];
        }
#endif
    
    }

    p_link->allowed_pkt_types = LMqos_LM_Gen_Allowed_Packet_Types(acl_packet_types);
}

/******************************************************************* 
 * Function :-  LMqos_LMP_Max_Slot_Req
 *
 * Description
 * This function handles and incoming "LMP_Max_Slot_Req" PDU.
 *
 * If Role of current device = Slave - Check with link policy if the 
 * device supports this Max Slots value.
 *
 * If Role of current device = Master - Check with link policy if the 
 * device support this max slots value AND has sufficient bandwidth on
 * the channel to support this value.
 *********************************************************************/
t_error LMqos_LMP_Max_Slot_Req(t_lmp_link *p_link, t_p_pdu p_payload)
{
    u_int8 max_slots = *p_payload;
    t_lmp_pdu_info  pdu_info;
    t_error status;

    p_link->current_proc_tid_role = p_link->role ^ 0x01;

    status = LMpol_Max_Slots_Allowed(p_link, max_slots);
    if (status == NO_ERROR)
    {
        pdu_info.opcode = LMP_ACCEPTED;
        p_link->max_slots_in = max_slots;
        pdu_info.tid_role = p_link->current_proc_tid_role;

        /*
         * Note on receipt of an LMP_Max_Slot_Req the p_link should NOT be modified
         * as the LMP_Max_Slot is a request to change the max number of slots the 
         * peer can send and thus has not effect on the local link.
         * However, this should be stored on the link as max_in_slots to allow bandwidth
         * calculations for the piconet. 
         */
        
        pdu_info.return_opcode = LMP_MAX_SLOT_REQ;
        LM_Encode_LMP_PDU(p_link->device_index,&pdu_info); 
        status = NO_ERROR;
    }
    return status;
}

/*********************************************
 *********************************************
 **     SUPPORT FOR LINK RATE               **
 *********************************************
 *********************************************/

/***************************************************************
 * Function :- LMqos_Send_LMP_Preferred_Rate
 *
 * Description
 * Propietrary Command to allow the Preferred Rate (DH/DM) on a link
 * to be changed via LM Service Interface.
 **************************************************************/ 
t_error LMqos_Send_LMP_Preferred_Rate(t_lmp_link *p_link, t_rate rate)
{
#if (PRH_BS_CFG_SYS_LMP_CHANNEL_DRIVEN_QUALITY_SUPPORTED==1)
    t_lmp_pdu_info  pdu_info;

    pdu_info.tid_role = p_link->role;

    /*
     * When in Basic Rate mode:
     * MEDIUM==1 => bit0 = 0: use FEC
     * HIGH==2 => bit0 = 1: do not use FEC
     */
    pdu_info.mode = rate-1;

    p_link->preferred_rate = rate;

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
    if(LC_Get_PTT_ACL(p_link->device_index))
    {
       /*
        * When in Enhanced Data Rate mode:
        * MIN==0 => bit3-4=0: use DM1 packets
        * MEDIUM==1 => bit3-4=1: use 2 Mbps packets
        * HIGH==2 => bit3-4=2: use 3 Mbps packets
        * Note, when in EDR mode, the Basic Rate
        * mode shall also be communicated.
        */
       pdu_info.mode |= (rate<<3);
    }
#endif

    pdu_info.opcode = LMP_PREFERRED_RATE;
    LM_Encode_LMP_PDU(p_link->device_index,&pdu_info);
#endif
    return NO_ERROR;
}

#if (PRH_BS_CFG_SYS_LM_AUTO_RATE_SUPPORTED==1)
/***************************************************************
 * Function :- LMqos_LM_Auto_Rate
 *
 * Description
 * Propietrary Command to allow the Auto Rate (DH/DM) on a link
 * to be changed via LM Service Interface.
 **************************************************************/       
t_error LMqos_LM_Auto_Rate(t_lmp_link* p_link)
{
    t_lmp_pdu_info  pdu_info;

    if (SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_LM_AUTO_RATE_FEATURE)
     && SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_LMP_CHANNEL_DRIVEN_QUALITY_FEATURE))
    {
        if (mFeat_Check_CQDDR(p_link->remote_features))
        {
            pdu_info.tid_role = p_link->role;
            pdu_info.opcode = LMP_AUTO_RATE;
            LM_Encode_LMP_PDU(p_link->device_index,&pdu_info);
            p_link->rate = AUTO;
        }
    }

    return NO_ERROR;
}
#endif

#if  ( (PRH_BS_CFG_SYS_LM_PREFERRED_RATE_SUPPORTED==1) && (PRH_BS_CFG_SYS_LMP_CHANNEL_DRIVEN_QUALITY_SUPPORTED==1))
/***************************************************************
 * Function :- LMqos_LMP_Preferred_Rate
 *
 * Description
 * Handles the LMP_Preferred_Rate PDU which allows the 
 * Preferred_Rate (DH/DM) on a link to be changed by the peer.
 **************************************************************/ 
t_error LMqos_LMP_Preferred_Rate(t_lmp_link *p_link, t_p_pdu p_payload)
{
    /*
     * Packet types available for Preffered Slots, Preffered Rate 
     *    [preffered_rate][preffered_slots]
     *   => [DM] = [DM1,DM3,DM5] , [DH] = [DH1,DH3,DH5]
     *
     * The data rate parameter includes the preferred rate for
     * Basic Rate and Enhanced Data Rate modes. When operating in
     * Basic Rate mode, the device shall use bits 0-2 to determine
     * the preferred data rate. When operating in Enhanced Data
     * Rate mode, the device shall use bits 3-6 to determine the
     * preferred data rate. For devices that support Enhanced Data
     * Rate, the preferred rates for both Basic Rate and Enhanced
     * Data Rate modes shall be valid at all times.
     *
     * When in Basic Rate mode:
     *
     * bit0 = 0: use FEC
     * bit0 = 1: do not use FEC
     *
     * bit1-2=0: No packet-size preference available
     * bit1-2=1: use 1-slot packets
     * bit1-2=2: use 3-slot packets
     * bit1-2=3: use 5-slot packets
     *
     * When in Enhanced Data Rate mode:
     *
     * bit3-4=0: use DM1 packets
     * bit3-4=1: use 2 Mbps packets
     * bit3-4=2: use 3 Mbps packets
     * bit3-4=3: reserved
     *
     * bit5-6=0: No packet-size preference available
     * bit5-6=1: use 1-slot packets
     * bit5-6=2: use 3-slot packets
     * bit5-6=3: use 5-slot packets
     *
     * bit7: Reserved - shall be zero
     *
     */

    const static u_int16 _LMqos_max_slot_fec_pkt_types[3][3] = 
            { {HCI_DM1,  HCI_DM3,  HCI_DM5},
              {HCI_DH1,  HCI_DH3,  HCI_DH5}};

    const static u_int16 _LMqos_max_slot_edr_pkt_types[3][3] = 
            { {HCI_DM1,  0,        0},
              {HCI_2DH1, HCI_2DH3, HCI_2DH5},
              {HCI_3DH1, HCI_3DH3, HCI_3DH5}};

    t_rate preffered_rate;
    u_int8 preffered_slots;
    u_int8 slots = 0;



#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
    if(LC_Get_PTT_ACL(p_link->device_index))
    {
        /*
         * Note equality on t_rate representation vs lmp payload representation
         * bit3-4 = 0: use DM1 packets => t_rate = MIN == 0
         * bit3-4 = 1: use 2Mbps packets => t_rate = MEDIUM == 1
         * bit3-4 = 2: use 3Mbps packets => t_rate = HIGH == 2
         */
        preffered_rate = (t_rate)((*p_payload >> 3) & 0x03);
        preffered_slots = (((*p_payload) >> 5) & 0x03);

        if((preffered_slots != 0) && (preffered_slots <= 3))
            slots = (preffered_slots<<1)-1;
        else
            slots = 0;

        if ((slots) && ((p_link->packet_types & _LMqos_max_slot_edr_pkt_types[preffered_rate][preffered_slots-1])
             & (mLMqos_Max_Slots_2_Packets(p_link->max_slots_out)) & (HCI_ACL_EDR|HCI_DM1)))
        {
            LMqos_Update_Allowed_Packets(p_link,slots,preffered_rate);
        }
        else if(p_link->packet_types & edr_rate_to_pkt_mapping[preffered_rate] & mLMqos_Max_Slots_2_Packets(p_link->max_slots_out))
        {
            LMqos_Update_Allowed_Packets(p_link,p_link->max_slots_out,preffered_rate);
        }
    }
    else
#endif
    {
        /*
         * Note +1 on t_rate representation vs lmp payload representation
         * bit0 = 0: use FEC => t_rate = MEDIUM == 1
         * bit0 = 1: do not use FEC => t_rate = HIGH == 2
         */
        preffered_rate = (t_rate)((*p_payload)&0x01)+1;
        preffered_slots = (((*p_payload) >> 1) & 0x03);

        if((preffered_slots != 0) && (preffered_slots <= 3))
            slots = (preffered_slots<<1)-1;

        if ((slots) && ((p_link->packet_types & _LMqos_max_slot_fec_pkt_types[preffered_rate-1][preffered_slots-1])
             & (mLMqos_Max_Slots_2_Packets(p_link->max_slots_out)) & HCI_ACL_BR))
        {
            LMqos_Update_Allowed_Packets(p_link,slots,preffered_rate);
        }
        else if(p_link->packet_types & br_rate_to_pkt_mapping[preffered_rate] & mLMqos_Max_Slots_2_Packets(p_link->max_slots_out))
        {
            LMqos_Update_Allowed_Packets(p_link,p_link->max_slots_out,preffered_rate);
        }
    }

    return NO_ERROR;
}
#endif

#if ( (PRH_BS_CFG_SYS_LM_AUTO_RATE_SUPPORTED==1) && (PRH_BS_CFG_SYS_LMP_CHANNEL_DRIVEN_QUALITY_SUPPORTED==1))
/***************************************************************
 * Function :- LMqos_LMP_Auto_Rate
 *
 * Description
 * Handles the LMP_Preferred_Rate PDU which allows the 
 * Preferred_Rate (DH/DM) on a link to be changed by the peer.
 **************************************************************/ 
t_error LMqos_LMP_Auto_Rate(t_lmp_link *p_link, t_p_pdu p_payload)
{
    p_link->peer_rate = p_link->preferred_rate = AUTO;
    return NO_ERROR;
}
#endif

/*********************************************
 *********************************************
 **   SUPPORT FOR SUPERVISION TIMEOUT       **
 *********************************************
 *********************************************/

/***********************************************************************
 * Function :- LMqos_LM_Write_Supervision_Timeout
 *
 * Handles a request to write a link Supervision Timeout from the 
 * Upper Layer (HCI). If the current device is a master on the link
 * then an LMP_SUPERVISION_TIMEOUT PDU must be sent to the slave.
 **********************************************************************/
t_error LMqos_LM_Write_Supervision_Timeout(
    t_lmp_link *p_link, t_slots timeout, t_cmd_complete_event *p_cmd_complete)
{
    t_lmp_pdu_info  pdu_info;
    t_error status;

    if (p_link->role == MASTER)
    {

        if ((p_link->state & (LMP_SNIFF_MODE|LMP_HOLD_MODE)) && (timeout != 0))
        {
            u_int16 mode_interval, min_supervision_timeout;

            /*
             * The timeout period, supervisionTO, is negotiated by the Link Manager.
             * T_sniff/hold_time shall not exceed supervisionTO * 0.999.
             */
#if (PRH_BS_CFG_SYS_LMP_HOLD_SUPPORTED==1)
            mode_interval = (p_link->state & LMP_SNIFF_MODE)?(p_link->pol_info.sniff_info.T_sniff):
                (p_link->pol_info.hold_info.hold_time);
#else
    #if     0   //- LC add (#if #else #endif) for turn off SNIFF.
           mode_interval = p_link->pol_info.sniff_info.T_sniff;
    #else
           mode_interval = 0; 
    #endif         
#endif

            min_supervision_timeout = (mode_interval) + ((mode_interval)>>10) + DEFAULT_T_POLL;
            
            if (timeout < min_supervision_timeout)
            {
#if (PRH_BS_DEV_SUPERVISION_TIMEOUT_NEGOTIATION_SUPPORTED==1)
                timeout = min_supervision_timeout;
#else
                return COMMAND_DISALLOWED;
#endif
            }
        }

        p_link->current_proc_tid_role = p_link->role;
        pdu_info.tid_role = p_link->role;
        pdu_info.opcode = LMP_SUPERVISION_TIMEOUT;
        pdu_info.timeout = timeout;
        LM_Encode_LMP_PDU(p_link->device_index,&pdu_info);
        p_link->supervision_timeout = timeout;
        LMtmr_Reset_Timer(p_link->supervision_timeout_index,(p_link->supervision_timeout>>1));
        status = NO_ERROR;
    }
    else
    {
#if (PRH_BS_DEV_CHANGE_SUPERVISION_TIMEOUT_ON_SLAVE_SIDE==1)
		 p_link->supervision_timeout = timeout;
         LMtmr_Reset_Timer(p_link->supervision_timeout_index,(p_link->supervision_timeout>>1));
		 status = NO_ERROR
#else
         status = COMMAND_DISALLOWED;
#endif
    }

    if (p_cmd_complete)
    {
        p_cmd_complete->handle = p_link->handle; 
        p_cmd_complete->status = status;
    }

    return status;
}


/***********************************************************************
 * Function :- LMpolicy_LMP_Supervision_Timeout_Ack
 *
 * Refresh the SSR if active after Supervision Timeout Change ack.
 **********************************************************************/
void _Send_LMP_Sniff_Subrating_Req(t_lmp_link* p_link);
void LMpolicy_LMP_Supervision_Timeout_Ack(t_deviceIndex device_index)
{
#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUBRATING_SUPPORTED==1)
    t_lmp_link *p_link = LMaclctr_Find_Device_Index(device_index);
    t_lm_sniff_info *p_sniff =  &p_link->pol_info.sniff_info;

    if (p_link->state & LMP_SNIFF_MODE)
    {
        /*if (p_link->pol_info.sniff_info.max_latency)*/
        if((LMpol_Get_Common_Subrate(p_link)*(p_sniff->T_sniff)) >= p_link->supervision_timeout)
        {//renegotiate the subrates
            _Send_LMP_Sniff_Subrating_Req(p_link);
        }
    }
#endif
}


/***********************************************************************
 * Function :- LMqos_LM_Read_Supervision_Timeout
 *
 * Handles a request for a link Supervision Timeout from the 
 * Upper Layer (HCI).
 **********************************************************************/
t_error LMqos_LM_Read_Supervision_Timeout(
    t_lmp_link *p_link, t_cmd_complete_event *p_cmd_complete)
{
    p_cmd_complete->timeout = p_link->link_supervision_timeout;
    p_cmd_complete->handle = p_link->handle;    
    return NO_ERROR;
}

/**********************************************************************
 * Function :- LMqos_LMP_Supervision_Timeout
 * 
 * Description :-
 * Handles an incoming LMP_Supervision_Timeout PDU. Stores the timeout
 * in the ACL link structure 
 **********************************************************************/
t_error LMqos_LMP_Supervision_Timeout(t_lmp_link *p_link, t_p_pdu p_pdu)
{
#if (PRH_BS_CFG_SYS_LSTO_CHANGED_EVENT_SUPPORTED == 1)
	t_lm_event_info lsto_change_event;
#endif

    if (p_link->role == SLAVE)
    {
        p_link->supervision_timeout = (*p_pdu) + ((*(p_pdu+1)) << 8);
        p_link->link_supervision_timeout = p_link->supervision_timeout;

		// G.F 29 Oct 2008 - Aside = There is interaction between the Rx of this PDU and Sniff Subrating.
		// If we are performing Sniff subrating and LSTO PDU is Rxed then we should disable Sniff Subrating.
        LMtmr_Reset_Timer(p_link->supervision_timeout_index,p_link->supervision_timeout); 

#if (PRH_BS_CFG_SYS_LSTO_CHANGED_EVENT_SUPPORTED == 1)
		lsto_change_event.handle = p_link->handle;
		lsto_change_event.value16bit = p_link->supervision_timeout;
		HCeg_Generate_Event(HCI_LINK_SUPERVISION_TIMEOUT_CHANGED_EVENT,&lsto_change_event);
#endif

    }
    else
    {
        return LMP_PDU_NOT_ALLOWED;
    }   
    return NO_ERROR;
}

#if (PRH_BS_CFG_SYS_QOS_SUPPORTED==1)
/*********************************************
 *********************************************
 **   SUPPORT FOR LINK QUALITY OF SERVICE   **
 *********************************************
 *********************************************/

/**********************************************************************
 * Function :- LMqos_LMP_Quality_Of_Service_Req
 * 
 * Description :-
 * Handles an incoming LMP_Quality_Of_Service_Req PDU (containing a Poll interval
 * and an Nbc). This can be accepted or rejected by the local device.
 **********************************************************************/
t_error LMqos_LMP_Quality_of_Service_Req(t_lmp_link* p_link,
                                               t_p_pdu p_payload)
{
    t_lmp_pdu_info  pdu_info;
    t_error status;
    u_int16 poll_interval;

    p_link->current_proc_tid_role = p_link->role ^ 0x01;
    pdu_info.tid_role = p_link->current_proc_tid_role;

    /*
     * Check if there is a transaction collision
     */
    if( (p_link->operation_pending == LMP_QUALITY_OF_SERVICE_REQ) && 
        (p_link->role == MASTER))
    {
        return LMP_ERROR_TRANSACTION_COLLISION;
    }

    /***********************************************************
     * Check if the new parameters are suitable for Link Policy 
     ***********************************************************/
    poll_interval = LMutils_Get_Uint16(p_payload);

    /*
     * B618,  TP/LIH/BV40C sends poll_interval of 5!!!
     * To pass test accept all odd poll intervals as even
     */
    poll_interval &= 0xFFFE;

    if(poll_interval == 0)
        poll_interval = 2;

    status = LMqos_Update_QoS(p_link, poll_interval,
                                 (u_int8)(*(p_payload+2)));

    if (status == NO_ERROR)
    {
        pdu_info.opcode = LMP_ACCEPTED;
        p_link->latency = mLMqos_Slots_to_Micro_Sec(p_link->poll_interval);
        p_link->token_rate = ((LMqos_Max_Packet_Size(p_link->packet_types) * 1000000L) / p_link->latency);
        _Send_HC_QoS_Setup_Complete_Event(p_link,status);  
        pdu_info.return_opcode = LMP_QUALITY_OF_SERVICE_REQ;
        LM_Encode_LMP_PDU(p_link->device_index,&pdu_info);
    }
    return status;
}

/**********************************************************************
 * Function :- LMqos_LMP_Quality_Of_Service
 * 
 * Description :-
 * Handles an incoming LMP_Quality_Of_Service PDU (containing a Poll interval
 * and an Nbc). This is a forced QoS, which has to be accepted by the 
 * slave and CANNOT be negotiated
 **********************************************************************/
t_error LMqos_LMP_Quality_of_Service(t_lmp_link *p_link, t_p_pdu p_payload)
{
    u_int16 poll_interval;
   /**************************************************************
    * Only the master can force a QoS on the slave NOT vica versa 
    **************************************************************/
   if(p_link->role == SLAVE)
   {
       poll_interval = LMutils_Get_Uint16(p_payload);

    /*
     * B618,  TP/LIH/BV39C sends poll_interval of 5!!!
     * To pass test accept all odd poll intervals as even
     */
       poll_interval &= 0xFFFE;

       {
           LMqos_Write_QoS(p_link,poll_interval, (u_int8)(*(p_payload+2)));
           p_link->latency = mLMqos_Slots_to_Micro_Sec(p_link->poll_interval);
           p_link->token_rate = ((LMqos_Max_Packet_Size(p_link->packet_types) * 1000000L) / p_link->latency );
       
           _Send_HC_QoS_Setup_Complete_Event(p_link,NO_ERROR);   
       }
   }
   return NO_ERROR;
}
#endif /* (PRH_BS_CFG_SYS_QOS_SUPPORTED==1) */

/*****************************************************************************
 * Function : LMqos_Write_QoS
 *
 * Description
 * This function is used to write the LM QoS values (poll interval & number of broadcasts)
 * for a given link.
 *******************************************************************************/
void LMqos_Write_QoS(t_lmp_link *p_link, u_int16 poll_interval, u_int8 num_bcast)
{
   p_link->poll_interval = poll_interval;
   p_link->Nbc = num_bcast;
}

/*****************************************************************************
 * Function : LMqos_Update_QoS
 *
 * Description
 * This function is used to write the LM QoS values (poll interval & number of broadcasts)
 * for a given link.
 *******************************************************************************/
t_error LMqos_Update_QoS(t_lmp_link *p_link, u_int16 poll_interval, u_int8 num_bcast)
{
#if 1 // GF BQB 30 April 
	  // Testers sends Poll Interval = 2 * Max_Tsniff, Since BT_12 there are limits 
	  // on the acceptable Tpoll ( 0x0006 to 0x1000 ). We have to check for these limits
	  // and reject if outside them.

    if ((poll_interval > 0x0005) && (poll_interval < 0x1001))
	{
		// Limits defined in BT 21 Spec page 320.
		p_link->poll_interval = poll_interval;
		if ( /*((p_link->role == MASTER) && (num_bcast != 0)) ||*/ //#2301
          (p_link->role == SLAVE) )
		{
			p_link->Nbc = num_bcast;
		}

		return NO_ERROR;
	}
	else
	{
		return INVALID_LMP_PARAMETERS;
	}

#else
    p_link->poll_interval = poll_interval;
    if ( /*((p_link->role == MASTER) && (num_bcast != 0)) ||*/ //#2301
          (p_link->role == SLAVE) )
    {
        p_link->Nbc = num_bcast;
    }

    return NO_ERROR;
#endif
}

/**********************************************************************
 * Function :- LMqos_LMP_Not_Accepted
 * 
 * Description :-
 * Handles an incoming LMP_Not_Accepted PDU
 **********************************************************************/
t_error LMqos_LMP_Not_Accepted(t_lmp_link *p_link, u_int16 opcode, t_error reason)
{
    switch (opcode)
    {
    case LMP_MAX_SLOT_REQ : 
		p_link->packet_type_changed_event = 0;
        LMqos_Send_HC_Packet_Type_Changed_Event(p_link,reason);
        break;
#if (PRH_BS_CFG_SYS_QOS_SUPPORTED==1)
    case LMP_QUALITY_OF_SERVICE :
    case LMP_QUALITY_OF_SERVICE_REQ :
#if (PRH_BS_CFG_SYS_FLOW_SPECIFICATION_SUPPORTED==1)
        if (p_link->flow_spec_pending)
            _Send_HC_Flow_Spec_Complete_Event(p_link,reason);       
        else
#endif
            _Send_HC_QoS_Setup_Complete_Event(p_link,reason);       
        break;
#endif
    }

    return NO_ERROR;
}

/**********************************************************************
 * Function :- LMqos_LMP_Accepted
 * 
 * Description :-
 * Handles an incoming LMP_Accepted PDU
 **********************************************************************/
t_error LMqos_LMP_Accepted(t_lmp_link *p_link, t_p_pdu p_payload)
{
    u_int8 opcode = *p_payload;

    switch (opcode)
    {
    case LMP_MAX_SLOT_REQ :
        {
			u_int8 max_slots;
			
			p_link->packet_types = p_link->proposed_packet_types;
			max_slots = LMqos_Convert_To_Max_Slots(p_link->packet_types);

			if (LMscoctr_Get_Number_SCO_Connections() && (max_slots > 1) )
			{
				max_slots = ((PRH_BS_DEV_MULTISLOT_ACL_WITH_SCO_SUPPORTED == 1) &&
					(LMscoctr_Get_Used_LM_SCO_Packet_Type() == LM_SCO_PACKET_HV3) &&
					(LMscoctr_Get_Number_SCO_Connections() ==1))?3:1;
			}

			if (max_slots == p_link->max_slots_out_pending)
				p_link->max_slots_out_pending = 0;

			if (p_link->max_slots_out != max_slots)
			{
				p_link->max_slots_out = max_slots;
				LMqos_Update_Allowed_Packets(p_link, p_link->max_slots_out, p_link->rate);
				_Send_HC_Max_Slots_Change_Event(p_link, (u_int8)p_link->max_slots_out);
			}

			if (p_link->packet_type_changed_event)
			{
				p_link->packet_type_changed_event = 0;
				LMqos_Send_HC_Packet_Type_Changed_Event(p_link, NO_ERROR);
			}

#if (PRH_BS_CFG_SYS_QOS_SUPPORTED==1)
            LMqos_Update_Link_QoS(p_link,p_link->service_type, p_link->latency, p_link->token_rate);
#endif
        }
        break;

    case LMP_QUALITY_OF_SERVICE_REQ :
#if (PRH_BS_CFG_SYS_FLOW_SPECIFICATION_SUPPORTED==1)
        if (p_link->flow_spec_pending)
        {
#if (PRH_BS_CFG_SYS_QOS_SUPPORTED==1)
            if (p_link->direction==0)
            {
            LMqos_Update_Link_Flow_Spec(p_link, p_link->direction, p_link->out_proposed_service_type,
                p_link->out_proposed_token_rate, p_link->out_proposed_latency);
            }
            else
            {
            LMqos_Update_Link_Flow_Spec(p_link, p_link->direction, p_link->in_proposed_service_type,
                p_link->in_proposed_token_rate, p_link->in_proposed_latency);
            }
            _Send_HC_Flow_Spec_Complete_Event(p_link,NO_ERROR);
#endif
        }
        else
#endif
        {
            p_link->poll_interval = p_link->proposed_poll_interval;
            p_link->poll_position = p_link->proposed_poll_interval;
#if (PRH_BS_CFG_SYS_QOS_SUPPORTED==1)
            p_link->delay_variation = p_link->proposed_delay_variation;
            LMqos_Update_Link_QoS(p_link,p_link->proposed_service_type, p_link->proposed_latency, p_link->proposed_token_rate);
            _Send_HC_QoS_Setup_Complete_Event(p_link,NO_ERROR);
#endif
        }
        break;
    }
    return NO_ERROR;
}

#if (PRH_BS_CFG_SYS_QOS_SUPPORTED==1)
/***********************************************************************
 * FUNCTION :- LMqos_Update_Link_QoS
 *
 * DESCRIPTION :- 
 * Updates the QoS on the link. Only the latency and token_rate (tk_rate)
 * are important in the Link Manager.
 ***********************************************************************/
t_error LMqos_Update_Link_QoS(
     t_lmp_link *p_link, u_int8 service, u_int32 latency, u_int32 tk_rate)
{
    u_int32 new_latency ;
    u_int32 proposed_latency;
    u_int32 proposed_tk_rate;
    u_int32 proposed_poll_interval;

    if (p_link->latency == 0) /* If the latency is Zero it is un-initialised */
    {
        p_link->latency = latency;   
    }

    p_link->service_type = service;

    if((service == PRH_NO_TRAFFIC) || ((p_link->latency == 0) && (p_link->token_rate == 0)))
    {
        return NO_ERROR;
    }

    /*
     * Given a Token Rate of 0xFFFFFFFF, and Service Type of Guaranteed, the LM
     * should refuse any additional connections from remote devices and disable all
     * periodic scans.
     */

    if ((tk_rate == 0xFFFFFFFF) && (service == PRH_GUARANTEED_SERVICE))
    {
        /* Disable Scaning */
        g_LM_config_info.new_connections_allowed = 0;
        LMconfig_LM_Set_Device_State(PRH_DEV_STATE_GUARANTEED_QOS);
    }
  
    if (tk_rate != 0x0)
    {
        /* Determine a Latency Value to support the token rate
         */
        if (tk_rate != 0xFFFFFFFF)
            new_latency = ((LMqos_Max_Packet_Size(p_link->packet_types) * 1000000L) / tk_rate);
        else
            new_latency = ((p_link->max_slots_out+1) * 625);

        /* If the latency requested is greater than the value required to support 
         * the token rate then the latency for the token rate is used.
         */

        if (latency > new_latency)
        {
            latency = new_latency;
        }
    } 

    /* 
     * Set the latency Values 
     */
    proposed_poll_interval = (u_int16)mLMqos_MicroSec_To_Slots(latency);

    if (proposed_poll_interval <= 0x04)
    {
        u_int16 max_pkt_types;

        max_pkt_types = (u_int16)_LMqos_max_slot_pkt_types[(u_int8)((proposed_poll_interval/2)-1)];
        proposed_tk_rate = ((LMqos_Max_Packet_Size((u_int16)(p_link->packet_types & max_pkt_types)) * 1600) / proposed_poll_interval);
    }
    else
    {
        proposed_tk_rate = ((LMqos_Max_Packet_Size(p_link->packet_types) * 1600) / proposed_poll_interval); 
    }

    proposed_latency = (u_int32)mLMqos_Slots_to_Micro_Sec(proposed_poll_interval);
    
    if ((proposed_latency <= latency)/* && ( proposed_tk_rate >= tk_rate)*/)
    {
       p_link->poll_interval = proposed_poll_interval;
       p_link->latency = proposed_latency;
       p_link->token_rate = proposed_tk_rate;
       return NO_ERROR;
    }
    else
    {
       return QOS_NOT_SUPPORTED;
    }
}

#if (PRH_BS_CFG_SYS_FLOW_SPECIFICATION_SUPPORTED==1)
/* Update FLow Spec */

/***********************************************************************
 * FUNCTION :- LMqos_Update_Link_QoS
 *
 * DESCRIPTION :- 
 * Updates the QoS on the link. Only the latency and token_rate (tk_rate)
 * are important in the Link Manager.
 ***********************************************************************/
t_error LMqos_Update_Link_Flow_Spec(
     t_lmp_link *p_link, u_int8 direction, u_int8 service, u_int32 tk_rate, u_int32 latency)
{
    u_int32 new_latency ;
    u_int32 proposed_latency;
    u_int32 proposed_tk_rate;
    u_int32 proposed_interval;

    if (p_link->latency == 0) /* If the latency is Zero it is un-initialised */
    {
        p_link->latency = latency;   
    }

    p_link->service_type = service;

    if((service == PRH_NO_TRAFFIC) || ((p_link->latency == 0) && (p_link->token_rate == 0)))
    {
        return NO_ERROR;
    }

    /*
     * Given a Token Rate of 0xFFFFFFFF, and Service Type of Guaranteed, the LM
     * should refuse any additional connections from remote devices and disable all
     * periodic scans.
     */

    if ((tk_rate == 0xFFFFFFFF) && (service == PRH_GUARANTEED_SERVICE))
    {
        /* Disable Scaning */
        g_LM_config_info.new_connections_allowed = 0;
        LMconfig_LM_Set_Device_State(PRH_DEV_STATE_GUARANTEED_QOS);
    }
  
    if (tk_rate != 0x0)
    {
        /* Determine a Latency Value to support the token rate
         */
        new_latency = ((LMqos_Max_Packet_Size(p_link->packet_types) * 1000000L) / tk_rate);

        /* If the latency requested is greater than the value required to support 
         * the token rate then the latency for the token rate is used.
         */

        if (latency > new_latency)
        {
            latency = new_latency;
        }
    } 

    /* 
     * Set the latency Values 
     */

    if (direction == 0)
    {
        proposed_interval = p_link->proposed_tx_interval;
    }
    else
    {
        proposed_interval = p_link->proposed_poll_interval;
    }
    proposed_interval = (u_int16)mLMqos_MicroSec_To_Slots(latency);
    proposed_latency = (u_int32)mLMqos_Slots_to_Micro_Sec(proposed_interval);
    proposed_tk_rate = ((LMqos_Max_Packet_Size(p_link->packet_types) * 1600) / proposed_interval); 

    if ((proposed_latency <= latency) && ( proposed_tk_rate >= tk_rate))
    {
       if (p_link->direction==0)
       {
           p_link->transmission_interval = proposed_interval;
           p_link->transmission_position = proposed_interval;

           p_link->out_latency = proposed_latency;
           p_link->out_token_rate = proposed_tk_rate;

           p_link->out_peak_bandwidth = p_link->out_proposed_peak_bandwidth;
           p_link->out_token_bucket_size = p_link->out_proposed_token_bucket_size;

           p_link->poll_interval = p_link->proposed_poll_interval;
           p_link->poll_position = p_link->proposed_poll_interval;
       }
       else
       {
           p_link->poll_interval = proposed_interval;
           p_link->poll_position = proposed_interval;

           p_link->in_latency = proposed_latency;
           p_link->in_token_rate = proposed_tk_rate;

           p_link->in_peak_bandwidth = p_link->in_proposed_peak_bandwidth;
           p_link->in_token_bucket_size = p_link->in_proposed_token_bucket_size;
       }

       return NO_ERROR;
    }
    else
    {
       return QOS_NOT_SUPPORTED;
    }
}

/***************/

u_int16 LMqos_Get_Communication_Interval(
     t_lmp_link *p_link, u_int8 direction, u_int32 latency, u_int32 tk_rate)
{
    u_int32 new_latency ;
    u_int32 proposed_latency;
    u_int32 proposed_tk_rate;
    u_int32 proposed_interval;

    if (p_link->latency == 0) /* If the latency is Zero it is un-initialised */
    {
        p_link->latency = latency;   
    }


    /*
     * Given a Token Rate of 0xFFFFFFFF, and Service Type of Guaranteed, the LM
     * should refuse any additional connections from remote devices and disable all
     * periodic scans.
     */
  
    if (tk_rate != 0x0)
    {
        /* Determine a Latency Value to support the token rate
         */
        if (direction==0)
        {
            new_latency = ((LMqos_Max_Packet_Size(p_link->packet_types) * 1000000L) / tk_rate);
        }
        else
        {
            new_latency = ((LMqos_Max_Packet_Size(_LMqos_max_slot_pkt_types[p_link->max_slots_in]) * 1000000L) / tk_rate);
        }

        /* If the latency requested is greater than the value required to support 
         * the token rate then the latency for the token rate is used.
         */

        if (latency > new_latency)
        {
            latency = new_latency;
        }
    } 

    /* 
     * Set the latency Values 
     */
    proposed_interval = (u_int16)mLMqos_MicroSec_To_Slots(latency);

    proposed_latency = (u_int32)mLMqos_Slots_to_Micro_Sec(proposed_interval);


    if (direction == 0)
        proposed_tk_rate = ((LMqos_Max_Packet_Size(p_link->packet_types) * 1600) / proposed_interval); 
    else
        proposed_tk_rate = ((LMqos_Max_Packet_Size(_LMqos_max_slot_pkt_types[p_link->max_slots_in]) * 1600) / proposed_interval); 

    if ((proposed_latency <= latency) && ( proposed_tk_rate >= tk_rate))
    {
       return proposed_interval;
    }
    else
    {
       return 0;
    }
}
#endif

/***********************************************************************
 * FUNCTION :- LMqos_Check_Link_QoS
 *
 * DESCRIPTION :- 
 * Checks if QoS can be supported on the link. Only the latency and token_rate (tk_rate)
 * are important in the Link Manager.
 *
 *
 ***********************************************************************/
t_error LMqos_Check_Link_QoS(
    t_lmp_link *p_link, u_int8 service, u_int32 latency, u_int32 tk_rate)
{
    u_int32 new_latency ;

    if (p_link->latency == 0) /* If the latency is Zero it is un-initialised */
    {
        p_link->proposed_latency = latency;   
    }

    p_link->proposed_service_type = service;

    if((service == PRH_NO_TRAFFIC) /* || ((p_link->latency == 0) && (p_link->token_rate == 0))*/)
    {
        return NO_ERROR;
    }

    /*
     * Given a Token Rate of 0xFFFFFFFF, and Service Type of Guaranteed, the LM
     * should refuse any additional connections from remote devices and disable all
     * periodic scans.
     */

    if ((tk_rate == 0xFFFFFFFF) && (service == PRH_GUARANTEED_SERVICE))
    {
        /* Disable Scaning */
        g_LM_config_info.new_connections_allowed = 0;
        LMconfig_LM_Set_Device_State(PRH_DEV_STATE_GUARANTEED_QOS);
    }
  
    if (tk_rate != 0x0)
    {
        /* Determine a Latency Value to support the token rate
         */
        if (tk_rate != 0xFFFFFFFF)
            new_latency = ((LMqos_Max_Packet_Size(p_link->packet_types) * 1000000L) / tk_rate);
        else
            new_latency = ((p_link->max_slots_out+1) * 625);

        /* 
         * If the latency requested is greater than the value required to support 
         * the token rate then the latency for the token rate is used.
         */
        if (latency > new_latency)
        {
            latency = new_latency;
        }
    } 

    /* 
     * Set the latency Values 
     */
    p_link->proposed_poll_interval = (u_int16)mLMqos_MicroSec_To_Slots(latency);
    p_link->proposed_latency = (u_int32)mLMqos_Slots_to_Micro_Sec(p_link->proposed_poll_interval);
    if (tk_rate != 0xFFFFFFFF)
        p_link->proposed_token_rate = ((LMqos_Max_Packet_Size(p_link->packet_types) * 1600) / p_link->proposed_poll_interval); 
    else
        p_link->proposed_token_rate = tk_rate;

    return NO_ERROR;
 
}

#endif


#if (PRH_BS_CFG_SYS_QOS_SUPPORTED==1)
/***********************************************************************
 * FUNCTION :- LMqos_Max_Packet_Size
 *
 * DESCRIPTION :- 
 * Determines the greatest length of packet supported using the packet
 * types supported on the link.
 ***********************************************************************/
u_int32 LMqos_Max_Packet_Size(const t_HCIpacketTypes packet_types)
{
    u_int32 max_packet_size;

    /*
     * Binary search on max packet size based on diminishing lengths.
     */
    if (packet_types & (HCI_2DH3|HCI_3DH3|HCI_2DH5|HCI_3DH5))
    {
        if (packet_types & (HCI_3DH5|HCI_2DH5))
            max_packet_size = (packet_types & HCI_3DH5)?MAX_3DH5_USER_PDU:MAX_2DH5_USER_PDU;
        else
            max_packet_size = (packet_types & HCI_3DH3)?MAX_3DH3_USER_PDU:MAX_2DH3_USER_PDU;
    }
    else if (packet_types & (HCI_DH5|HCI_DM5|HCI_DH3|HCI_DM3))
    {
        if (packet_types & (HCI_DH5|HCI_DM5))
            max_packet_size = (packet_types & HCI_DH5)?MAX_DH5_USER_PDU:MAX_DM5_USER_PDU;
        else
            max_packet_size = (packet_types & HCI_DH3)?MAX_DH3_USER_PDU:MAX_DM3_USER_PDU;
    }
    else
    {
        if (packet_types & (HCI_3DH1|HCI_2DH1))
            max_packet_size = (packet_types & HCI_3DH1)?MAX_3DH1_USER_PDU:MAX_2DH1_USER_PDU;
        else
            max_packet_size = (packet_types & HCI_DH1)?MAX_DH1_USER_PDU:MAX_DM1_USER_PDU;    
    }

    return max_packet_size;
}
#endif

/**********************************************************************
 * Function :- LMqos_Convert_To_Max_Slots
 * 
 * Description :-
 * Local Function to convert from Packet Types to Max_Slots
 **********************************************************************/
u_int8 LMqos_Convert_To_Max_Slots(t_HCIpacketTypes packet_types)
{
    if (packet_types & HCI_5_SLOT)
        return 5;
    else if (packet_types & HCI_3_SLOT)
        return 3;
    else 
        return 1;
}  

/**********************************************************************
 * Function :- LMqos_Send_HC_Packet_Type_Changed_Event
 * 
 * Description :-
 * Generates a "Packet_Type_Changed_Event" to the upper layer (HC).
 **********************************************************************/
void LMqos_Send_HC_Packet_Type_Changed_Event(t_lmp_link *p_link, t_error outcome)
{
    t_lm_event_info packet_change_event;

    packet_change_event.handle = p_link->handle;
    packet_change_event.status = outcome;
    packet_change_event.value16bit = p_link->packet_types;
    HCeg_Generate_Event(HCI_CONNECTION_PACKET_TYPE_CHANGED_EVENT,&packet_change_event);
}   

/**********************************************************************
 * Function :- _Send_HC_Max_Slot_Change_Event
 * 
 * Description :-
 * Generates a "Max_Slots_Change_Event" to the upper layer (HC).
 **********************************************************************/
void _Send_HC_Max_Slots_Change_Event(t_lmp_link *p_link, u_int8 max_slots)
{
    t_lm_event_info max_slots_event;

    max_slots_event.handle = p_link->handle;
    max_slots_event.number = max_slots;
    HCeg_Generate_Event(HCI_MAX_SLOTS_CHANGE_EVENT,&max_slots_event);
}

/**********************************************************************
 * Function :- _Send_HC_QoS_Setup_Complete_Event
 * 
 * Description :-
 * Generates a "QoS_Setup_Complete_Event" to the upper layer (HC).
 **********************************************************************/
void _Send_HC_QoS_Setup_Complete_Event(t_lmp_link *p_link, t_error outcome)
{
    t_lm_event_info qos_complete_event;

    qos_complete_event.handle = p_link->handle;
    qos_complete_event.status = outcome;

#if (PRH_BS_CFG_SYS_QOS_SUPPORTED==1)
    qos_complete_event.latency = p_link->latency;
    qos_complete_event.token_rate = p_link->token_rate;

    /* 
     * Following parameters are not relevant to the Link Manager
     *  qos_complete_event.peak_bandwidth = p_link->peak_bandwidth;
     *  qos_complete_event.delay = p_link->delay;                   
     */
#endif

    HCeg_Generate_Event(HCI_QoS_SETUP_COMPLETE_EVENT,&qos_complete_event);  
}

#if (PRH_BS_CFG_SYS_FLOW_SPECIFICATION_SUPPORTED==1)
/**********************************************************************
 * Function :- _Send_HC_Flow_Spec_Complete_Event
 * 
 * Description :-
 * Generates a "Flow_Spec_Complete_Event" to the upper layer (HC).
 **********************************************************************/
void _Send_HC_Flow_Spec_Complete_Event(t_lmp_link *p_link, t_error outcome)
{
    t_lm_event_info flow_spec_complete_event;

    flow_spec_complete_event.handle = p_link->handle;
    flow_spec_complete_event.status = outcome;

    if (outcome == NO_ERROR)
    {
        if(p_link->direction==0)
        {
            flow_spec_complete_event.latency = p_link->out_latency;
            flow_spec_complete_event.token_rate = p_link->out_token_rate;
            p_link->out_qos_active = 1;
        }
        else
        {
            flow_spec_complete_event.latency = p_link->in_latency;
            flow_spec_complete_event.token_rate = p_link->in_token_rate;
            p_link->in_qos_active = 1;
        }
    }
    else
    {
        flow_spec_complete_event.latency = 0;
        flow_spec_complete_event.token_rate = 0;
    }
    /* 
     * Following parameters are not relevant to the Link Manager
     *  flow_spec_complete_event.peak_bandwidth = p_link->peak_bandwidth;
     *  flow_spec_complete_event.delay = p_link->delay;                   
     */


    p_link->flow_spec_pending = 0x0;
    HCeg_Generate_Event(HCI_FLOW_SPECIFICATION_COMPLETE_EVENT,&flow_spec_complete_event);  
}

/* This function checks to see of the request flow specification can be supported
   in the device. 
   
   It checks all the active links and performs an aggregation of the QoS and Peak 
   Bandwidth requirements of them.

*/
t_error LMqos_Check_Link_Flow_Specification(t_lmp_link* p_link,u_int8 direction,u_int8 service,
                                         u_int32 latency, u_int32 tk_rate, u_int32 pk_bandwidth)
{
  
    u_int16 num_slots_occupied_per_sec=0;
    u_int16 num_slots_free;
    t_deviceIndex device_index;


    for (device_index=0;device_index<PRH_MAX_ACL_LINKS;device_index++)
    {
        t_lmp_link* p_cur_link;

        p_cur_link = LMaclctr_Find_Device_Index(device_index);
        if ((p_cur_link != p_link) && (p_cur_link != 0)
            && (p_cur_link->state == LMP_ACTIVE))
        {
            if (p_cur_link->in_qos_active || p_link->out_qos_active)
            {
                if (p_link->out_qos_active)
                {
                    num_slots_occupied_per_sec += (800/p_cur_link->transmission_interval)*(p_cur_link->max_slots_out+1);
                }
                if (p_link->in_qos_active)
                {
                    num_slots_occupied_per_sec += ((800/p_cur_link->poll_interval)*(p_cur_link->max_slots_in+1));
                }
            }
        }
    }
    /* Now we know the total number of slots required per second */ 
    num_slots_free = 1600 - num_slots_occupied_per_sec;

    /* Now check if the Token Rate and latency can be satisfied by this number of slots 
     * (num_slots_free / (max_slots_on_link+1)) * Max_Packet_Size) = Bytes/s 
     */

    if (((num_slots_free/p_link->max_slots_out)*LMqos_Max_Packet_Size(p_link->packet_types))
           > tk_rate)
    {

        /* Possible Now Check the latency */

        p_link->out_proposed_latency = latency;
        p_link->out_proposed_peak_bandwidth = pk_bandwidth;
        p_link->out_proposed_service_type = service;
        p_link->out_proposed_token_rate = tk_rate;
        return NO_ERROR;

    }
    return UNSPECIFIED_ERROR;
}
#endif

/**********************************************************************
 * Function :- LMqos_Adjust_Supervision_Timeout_On_Link
 * 
 * Description :-
 * Negotiates new supervision timeout according to context changes.
 **********************************************************************/
void LMqos_Adjust_Supervision_Timeout_On_Link(t_lmp_link* p_link,
         t_LM_context context, u_int16 interval)
{
#if (PRH_BS_DEV_SUPERVISION_TIMEOUT_NEGOTIATION_SUPPORTED==1) && (BUILD_TYPE!=UNIT_TEST_BUILD)
    if (p_link->role == MASTER)
    {
        switch (context)
        {
        case LMcontxt_ENTERING_SNIFF:
		case LMcontxt_ENTERING_HOLD:
            /*
             * The timeout period, supervisionTO, is negotiated by the Link Manager
             * such that Tsniff shall not exceed supervisionTO * 0.999.
             */
            interval += (interval>>10) + DEFAULT_T_POLL;

            if ((p_link->supervision_timeout) && (p_link->supervision_timeout < interval))
            {
                LMqos_LM_Write_Supervision_Timeout(p_link, interval, NULL);
            }
            break;
        case LMcontxt_LEAVING_SNIFF:
		case LMcontxt_LEAVING_HOLD:
            /*
             * If supervision timeout was extended for sniff mode, retore
             * link supervision timeout previously set by host.
             */
            if (p_link->supervision_timeout != p_link->link_supervision_timeout)
            {
                LMqos_LM_Write_Supervision_Timeout(p_link,
                p_link->link_supervision_timeout, NULL);
            }
            break;
        }
    }
#endif
}
