/**************************************************************************
 * MODULE NAME:    hc_flow_control.c
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    Host Controller to/from Host Flow Control
 * AUTHOR:         John Nelson
 * DATE:           03-01-2002
 *
 * LICENSE:
 *     This source code is copyright (c) 2002-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *    02-02-2002 -   JN Initial Version
 *
 * SOURCE CONTROL: $Id: hc_flow_control.c,v 1.24 2013/05/30 16:07:42 tomk Exp $
 * 
 * Handles all fow control on the HCI.  Normally flow control is managed by the
 * Host for Host to Host Controller ACL data.  Optionally, SCO may be supported.
 *
 * In certain cases, flow control may also be necessary in the direction from the
 * Host Controller to the Host. There is therefore a command 
 * Set_Host_Controller_To_Host_Flow_Control  to turn flow control on or off in
 * that direction. If turned on, it works in exactly the same way as described
 * above. On initialization, the Host uses the Host_Buffer_Size command to notify
 * the Host Controller about the maximum size of HCI ACL and SCO Data Pack-ets
 * sent from the Host Controller to the Host. The command also contains two
 * additional command parameters to notify the Host Controller about the total
 * number of ACL and SCO Data Packets that can be stored in the data buffers of
 * the Host. The Host then uses the Host_Number_Of_Completed_Packets com-mand
 * in exactly the same way as the Host Controller uses the Number Of
 * Completed Packets event (as was previously described in this section). The
 * Host_Number_Of_Completed_Packets command is a special command for
 * which no command flow control is used, and which can be sent anytime there
 * is a connection or when in local loopback mode. This makes it possible for the
 * flow control to work in exactly the same way in both directions, and the flow of
 * normal commands will not be disturbed.
 * When the Host receives a Disconnection Complete event, the Host can
 * assume that all HCI Data Packets that have been sent to the Host Controller
 * for the returned Connection_Handle have been flushed, and that the corre-sponding
 * data buffers have been freed. The Host Controller does not have to
 * notify the Host about this in a Number Of Completed Packets event. If flow con-trol
 * is also enabled in the direction from the Host Controller to the Host, the
 * Host Controller can after it has sent a Disconnection_Complete event assume
 * that the Host will flush its data buffers for the sent Connection_Handle when it
 * receives the Disconnection_Complete event. The Host does not have to notify
 * the Host Controller about this in a Host_Number_Of_Completed_Packets com-mand.
 *************************************************************************/

#include "sys_config.h"

#include "lmp_config.h"
#include "lmp_utils.h"
#include "lmp_acl_container.h"
#include "lmp_sco_container.h"
#include "bt_timer.h"

#include "hci_params.h"
#include "hc_flow_control.h"

#include "sys_irq.h"
#include "tra_queue.h"
#include "bt_test.h"

#include "lc_interface.h"
#if 1 // (LE_INCLUDED == 1)
#include "le_connection.h"
#include "le_config.h"
#endif
#include    "global_val.h"
#include    "patch_function_id.h"

/*
 * Host Controller to Host Flow Control 
 *      is for Packets in direction from Host Controller to Host.
 */

/*
 * All data variables for Host <--> Host Controller Flow Control
 */
//t_hc_flow_ctrl hc_flow_ctrl;

/**************************************************************************
 *  FUNCTION :- HCfc_Initialise
 *
 *  DESCRIPTION :-
 *  This function initialises the flow control module parameters
 *
 *  Note that extent refers to 
 *                                                   Default         Extent
 * u_int16  host_ACL_Data_Packet_Length;             0  Unlimited    HCI Config
 * u_int8   host_SCO_Data_Packet_Length;             0  Unlimited    HCI Config
 * u_int16  host_Total_Num_ACL_Data_Packets;         0  Unlimited    HCI Config
 * u_int16  host_Total_Num_SCO_Data_Packets;         0  Unlimited    HCI Config
 * 
 * u_int16  host_available_number_ACL_data_packets   0  Unlimited    Dynamic
 * u_int8   hc2host_flow_control_enable              0  No ACL/SYN   HCI Config
 *
 * u_int8   host2hc_SYN_flow_control_Enable          0  No SYN fc    HCI Config
 * t_timer  hc_completed_data_packets_report_timer   n/a             Dynamic
 * t_slots  hc_completed_data_packets_report_timeout                 TCI Config
 * u_int8   hc_completed_ACL_data_packets_report_threshold   1       TCI Config
 * u_int8   hc_completed_SCO_data_packets_report_threshold   1       TCI Config
 * 
 *
 ***************************************************************************/
void HCfc_Initialise(void)
{
	if(PATCH_FUN[HCFC_INITIALISE_ID]){
         ((void (*)(void))PATCH_FUN[HCFC_INITIALISE_ID])();
         return ;
    }

    /*
     * Clear all fields
     */
    int i = sizeof(hc_flow_ctrl);
    u_int8 *ptr =(u_int8*)&hc_flow_ctrl;
    while (--i != 0)
    {
        *ptr++ = 0;
    }
    hc_flow_ctrl.hc_completed_ACL_data_packets_report_threshold = 
        HCfc_COMPLETED_ACL_DATA_PACKETS_REPORT_THRESHOLD;
    hc_flow_ctrl.hc_completed_SYN_data_packets_report_threshold = 
        HCfc_COMPLETED_SYN_DATA_PACKETS_REPORT_THRESHOLD;

    /*
     * Start timer based reporting based on default timeout
     */
    hc_flow_ctrl.hc_completed_data_packets_report_timeout = 
        HCfc_COMPLETED_DATA_PACKETS_TIMEOUT_DEFAULT_SLOTS;

    hc_flow_ctrl.hc_completed_data_packets_report_timer =
        BTtimer_Set_Slots(HCfc_COMPLETED_DATA_PACKETS_TIMEOUT_DEFAULT_SLOTS);

}

/*****************************************************************************
 * HCfc_Host_Buffer_Size
 *
 * Informs the Host Controller of the Host ACL/SCO Buffer Sizes/Number of.
 *
 * p_hci_payload        Direct pointer to payload bytes (rf V1.1 Spec H.1)
 *****************************************************************************/
t_error HCfc_Host_Buffer_Size(t_p_pdu p_hci_payload)
{
    /*
     * Extract HCI_HOST_BUFFER and store in configuration structure
     */

    hc_flow_ctrl.host_ACL_Data_Packet_Length = 
        HCIparam_Get_Uint16(p_hci_payload);

    hc_flow_ctrl.host_SYN_Data_Packet_Length = p_hci_payload[2];

    hc_flow_ctrl.host_Total_Num_ACL_Data_Packets = 
        HCIparam_Get_Uint16(p_hci_payload+3);

    hc_flow_ctrl.host_Total_Num_SYN_Data_Packets = 
        HCIparam_Get_Uint16(p_hci_payload+5);

    hc_flow_ctrl.host_available_number_ACL_data_packets = 
        hc_flow_ctrl.host_Total_Num_ACL_Data_Packets;

    hc_flow_ctrl.host_available_number_SYN_data_packets = 
        hc_flow_ctrl.host_Total_Num_SYN_Data_Packets;

    return NO_ERROR;
}

#ifndef REDUCE_ROM
/*****************************************************************************
 * HCfc_Read_SYN_Flow_Control_Enable
 *
 * Read the SYN Flow Control Enable for Host to HC Flow Control 
 * i.e. whether HC will send the Number Of Completed Packets Event for SCO
 *
 * V1.1 Spec:
 * The Read_SYN_Flow_Control_Enable command provides the ability to read
 * the SYN_Flow_Control_Enable setting. By using this setting, the Host can
 * decide if the Host Controller will send Number Of Completed Packets events
 * for SYN Connection Handles. This setting allows the Host to enable and 
 * disable SYN flow control.
 * Note: the SYN_Flow_Control_Enable setting can only be changed if no 
 * connections exist. 
 *
 * Returns
 *  0   SYN Flow Control is disabled
 *  1   SYN Flow Control is enabled
 *
 * Globals used
 *   g_LM_config_info
 *   hc_flow_ctrl
 *****************************************************************************/
u_int8 HCfc_Read_SYN_Flow_Control_Enable(void)
{
    return hc_flow_ctrl.host2hc_SYN_flow_control_Enable;
}
#endif
/*****************************************************************************
 * HCfc_Handle_Host_To_Host_Controller_Flow_Control
 *
 * Report Number of Completed Packet Events to Host If Necessary
 *
 *
 * Returns
 *     Nothing
 *
 * For flow control to work there must be space on the queue, not being 
 * able to report number of completed packets event is catastrophic
 * Hence the process below is two phase
 * 1. Extract Handles and Number of Completed Packets
 * 2. if (HCI_Number_Of_Completed_Packets_Event successful) Then
 *       Reduce the Number of Processed packets on the queue.
 *
 * If SCO_VIA_HCI Then
 *   The event will always be generated whenever there are num_of_completed
 *   packets to report, as SCO connections can be abruptly disconnected.
 *
 *****************************************************************************/
void HCfc_Handle_Host_To_Host_Controller_Flow_Control(void)
{ 
	
	if(PATCH_FUN[HCFC_HANDLE_HOST_TO_HOST_CONTROLLER_FLOW_CONTROL_ID]){
         ((void (*)(void))PATCH_FUN[HCFC_HANDLE_HOST_TO_HOST_CONTROLLER_FLOW_CONTROL_ID])();
         return ;
    }

#define MAX_NUMBER_OF_HANDLES 2//(PRH_MAX_ACL_LINKS + 2 /*Broadcast*/ + 3/*SCO*/)
#if (MAX_NUMBER_OF_HANDLES*4 + 3) > PRH_BS_CFG_SYS_SIZEOF_HCI_EVENT_HEAP/2
#error  HCeg_Number_Of_Completed_Packets_Event may not fit on event queue
#endif

    //extern t_link_entry link_container[PRH_MAX_ACL_LINKS];
    u_int8  num_active_handles;         /* Number of Active Handles         */
    u_int8  start_sco_active_handles;   /* Mark which handles are SCO       */
    u_int16 i;                          /* Index for link containers        */
#if 1 //(LE_INCLUDED == 1)
    u_int8  link_id;
#endif
    u_int16 handles[MAX_NUMBER_OF_HANDLES];
    u_int16 completed_packets[MAX_NUMBER_OF_HANDLES];
    t_deviceIndex  q_device_index[MAX_NUMBER_OF_HANDLES];

    u_int16 num_processed_chunks;
    boolean timer_is_expired; 
    
    /*
     * Bit field indicating event report NONE (0), ACL (1), SCO (2), BOTH (3)
     * Define as u_int8 since actually a bit_field
     */
    enum {Host2HC_NONE=0, Host2HC_ACL=1, Host2HC_SCO=2};
    u_int8 host2hc_fc_necessary;

    /*
     * Determine if the event should be sent in 2 separate steps for ACL/SCO
     * First check if timer has expired
     */
    timer_is_expired = (hc_flow_ctrl.hc_completed_data_packets_report_timeout != 0 &&
          BTtimer_Is_Expired(hc_flow_ctrl.hc_completed_data_packets_report_timer) );
    if (timer_is_expired)
    {
        /*
         * Reset the periodic Completed Data Packets event report timer
         */
        hc_flow_ctrl.hc_completed_data_packets_report_timer = BTtimer_Set_Slots(
            hc_flow_ctrl.hc_completed_data_packets_report_timeout);
    }

    /* 
     * Step 1. For ACL only Report Event If
     *   (ACL Threshold !=0 AND  # ACL Processed Chunks > ACL Threshold) OR
     *   (Timeout Defined AND Timer Fired AND # ACL Processed Chunks > 1) 
     */
    num_processed_chunks = BTq_Get_Total_Number_Of_Processed_Chunks(L2CAP_OUT_Q);
    host2hc_fc_necessary = 
         (hc_flow_ctrl.hc_completed_ACL_data_packets_report_threshold != 0 &&
          (num_processed_chunks >= 
           hc_flow_ctrl.hc_completed_ACL_data_packets_report_threshold) ) ||
           (timer_is_expired && num_processed_chunks != 0);

#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
    /* 
     * Step 2. For SYN only Report Event If
     *   (SCO Threshold !=0  AND  # SCO Processed Chunks > SCO Threshold) OR  
     *   (Timeout Defined  AND  Timer Fired  AND  # SCO Processed Chunks > 1) 
     */
    num_processed_chunks = BTq_Get_Total_Number_Of_Processed_Chunks(SCO_OUT_Q);

    /*
     * Set Bit 1 if SYN events necessary
     */
    if (hc_flow_ctrl.host2hc_SYN_flow_control_Enable && (
          (hc_flow_ctrl.hc_completed_SYN_data_packets_report_threshold != 0 &&
          (num_processed_chunks >= 
           hc_flow_ctrl.hc_completed_SYN_data_packets_report_threshold) ) || 
           (timer_is_expired && num_processed_chunks != 0) ) )
    {
        host2hc_fc_necessary |= Host2HC_SCO;
    }
#endif

    num_active_handles = 0;

    /*
     * Extract handles and completed packets for ACL 
     */
    if (host2hc_fc_necessary & Host2HC_ACL) 
    {
#if     0
        /* 
         * Handle broadcast consumed chunks, each with a unique connection handle
         * Active broadcast:    queue index 0
         * Piconet broadcast:   queue index MAX_ACTIVE_DEVICE_LINKS
         */
        num_processed_chunks = 
            BTq_Get_Queue_Number_Of_Processed_Chunks(L2CAP_OUT_Q, 0);
        if (num_processed_chunks != 0) 
        {
            /*
             * Insert active broadcast consumed chunks
             */
            q_device_index[num_active_handles] = 0;
            handles[num_active_handles] = g_LM_config_info.active_broadcast_handle;
            completed_packets[num_active_handles] = num_processed_chunks;
            num_active_handles++;
        }

        num_processed_chunks = BTq_Get_Queue_Number_Of_Processed_Chunks(
            L2CAP_OUT_Q, MAX_ACTIVE_DEVICE_LINKS);
        if (num_processed_chunks != 0) 
        {
            /*
             * Process piconet broadcast consumed chunks
             */ 
            q_device_index[num_active_handles] = MAX_ACTIVE_DEVICE_LINKS;
            handles[num_active_handles] = g_LM_config_info.piconet_broadcast_handle;
            completed_packets[num_active_handles] = num_processed_chunks;
            num_active_handles++;
        }
#endif
        /* 
         * This cycles through handles, reporting on active handles
         */
#if 1 //(LE_INCLUDED == 1)

//#define DEFAULT_MAX_NUM_LE_LINKS 3
        for (link_id=0; link_id < DEFAULT_MAX_NUM_LE_LINKS && num_active_handles < MAX_NUMBER_OF_HANDLES; link_id++)
        {
        	if (LEconnections_Link_In_Use(link_id))
        	{
                t_deviceIndex device_index;
                t_LE_Connection* p_connection = LEconnection_Find_Link_Entry(link_id);

                device_index = p_connection->device_index;

                num_processed_chunks = BTq_Get_Queue_Number_Of_Processed_Chunks(
                   L2CAP_OUT_Q, device_index);

                /*
                 * Normally, event will report handles with completed packets
                 * Note: The following check if removed will report all handles
                 */
                if (num_processed_chunks > 0)
                {
                    q_device_index[num_active_handles] = device_index;

                    handles[num_active_handles] = LEconnection_Determine_Connection_Handle(p_connection);
                    completed_packets[num_active_handles] = num_processed_chunks;
                    num_active_handles++;
                }
            }
        }
#endif
#ifndef BLUETOOTH_MODE_LE_ONLY
        for (i=0; i < PRH_MAX_ACL_LINKS && num_active_handles < MAX_NUMBER_OF_HANDLES; i++) 
        {
            if (link_container[i].used)
            {
                t_deviceIndex device_index;

                device_index = link_container[i].entry.device_index;
                
                num_processed_chunks = BTq_Get_Queue_Number_Of_Processed_Chunks(
                   L2CAP_OUT_Q, device_index);

                /*
                 * Normally, event will report handles with completed packets
                 * Note: The following check if removed will report all handles
                 */ 
                if (num_processed_chunks > 0)
                {
                    q_device_index[num_active_handles] = device_index;
                    handles[num_active_handles] = link_container[i].entry.handle;
                    completed_packets[num_active_handles] = num_processed_chunks;
                    num_active_handles++;
                }
            }
        }
#endif
    }
#if 0
    start_sco_active_handles = num_active_handles;
#endif
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1) 
    /*
     * Extract handles and completed packets for SCO 
     */
    if (host2hc_fc_necessary & Host2HC_SCO) 
    {
        u_int8  sco_index;          /* Index for sco containers         */
        t_sco_info *p_sco_link;

        /* 
         * This cycles through SCO handles, reporting on any handles with processed
         * chunks.
         */
        for (sco_index=0; sco_index < PRH_BS_CFG_SYS_MAX_SYN_LINKS_VIA_HCI; 
                sco_index++) 
        {
            num_processed_chunks = BTq_Get_Queue_Number_Of_Processed_Chunks(
               SCO_OUT_Q, sco_index);

            p_sco_link = LMscoctr_Find_SCO_By_SCO_Index(sco_index);
            if (p_sco_link)
            {
                
                /*
                 * Normally, event will report handles with completed packets
                 * Note: The following check if removed will report all handles
                 */ 
                if (num_processed_chunks > 0)
                {
                    q_device_index[num_active_handles] = sco_index;
                    handles[num_active_handles] = p_sco_link->connection_handle;
                    completed_packets[num_active_handles] = num_processed_chunks;
                    num_active_handles++;
                }
            }
        }
    }
#endif

    /*
     * Generate the Number of Completed Packets Event if required
     */
    if ( num_active_handles > 0 &&
         HCeg_Number_Of_Completed_Packets_Event(num_active_handles, 
            handles, completed_packets) )
    {
        /*
         * Successful enqueue of event 
         * Now it is safe to reduce the queue counters
         */
        for (i=0; i < num_active_handles; i++)
        {
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1) 
            /*
             * ACL and SCO, select either L2CAP_OUT_Q or SCO_OUT_Q
             */
            BTq_Reduce_Queue_Number_Of_Processed_Chunks(
                (u_int8) ((i < start_sco_active_handles) ? L2CAP_OUT_Q : SCO_OUT_Q), 
                q_device_index[i], completed_packets[i]);
#else
            /*
             * ACL only, always select L2CAP_OUT_Q
             */
            BTq_Reduce_Queue_Number_Of_Processed_Chunks(
                L2CAP_OUT_Q, 
                q_device_index[i], completed_packets[i]);
#endif
        }
    }
}

/*****************************************************************************
 * HCfc_Get_Next_HC_to_Host_Data_Descriptor
 *
 * Determines the next data descriptor to be sent to the host
 *
 * If necessary, the data will be segmented, and the descriptor will 
 * represent a segment.
 *
 * Returns
 *   descriptor     Valid data descriptor 
 *   0              NULL represents no data to send
 *
 *****************************************************************************/
t_q_descr *HCfc_Get_Next_HC_to_Host_Data_Descriptor(t_deviceIndex device_index)
{
    t_q_descr *qd = (t_q_descr *)0;

#if (PRH_BS_CFG_SYS_HC_TO_HOST_FLOW_CONTROL_SUPPORTED==0)
    /*
     * Original code from tra_hcit.c
     */
    if ( !BTq_Is_Queue_Empty(L2CAP_IN_Q, device_index) )
    {
        qd = BTq_Dequeue_Next(L2CAP_IN_Q, device_index);
    }
#else

    /*
     * If not flow control enabled OR host has buffers Then    
     *    Dequeue a packet for the device if it exists
     * Endif
     */
    if (!(hc_flow_ctrl.hc2host_flow_control_enable & HC_TO_HOST_ACL_FLOW_CONTROL) || 
        hc_flow_ctrl.host_available_number_ACL_data_packets > 0)
    {
        qd = BTq_Dequeue_Next_Data_By_Length(L2CAP_IN_Q, device_index,
            hc_flow_ctrl.host_ACL_Data_Packet_Length);
        if (qd)
        {
            hc_flow_ctrl.host_available_number_ACL_data_packets--;
        }
    }
#endif
    return qd;
}

#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
/*****************************************************************************
 * HCfc_Get_Next_HC_to_Host_SYN_Descriptor
 *
 * Determines the next SCO descriptor to be sent to the host
 *
 * If necessary, the data will be segmented, and the descriptor will 
 * represent a segment.
 *
 * Returns
 *   descriptor     Valid data descriptor 
 *   0              NULL represents no data to send
 *
 * Restrictions
 *   Must not re-enter on same device entry if flow control inactive
 *****************************************************************************/
t_q_descr *HCfc_Get_Next_HC_to_Host_SYN_Descriptor(u_int8 sco_index)
{
    t_q_descr *qd = (t_q_descr *)0;
    boolean hc_sco_flow_ctrl_inactive =
        !(hc_flow_ctrl.hc2host_flow_control_enable & HC_TO_HOST_SYN_FLOW_CONTROL);
        
    /*
     * If not flow control enabled OR host has buffers Then    
     *    Dequeue a packet for the sco device if it exists
     * Endif
     */
    if (hc_sco_flow_ctrl_inactive || 
        hc_flow_ctrl.host_available_number_SYN_data_packets > 0)
    {
        qd = BTq_Dequeue_Next_Data_By_Length(SCO_IN_Q, sco_index,
            hc_flow_ctrl.host_SYN_Data_Packet_Length);
        if (qd)
        {
            hc_flow_ctrl.host_available_number_SYN_data_packets--;
            /*
             * If no flow control reduce queue number of processed chunks 
             */
            if (hc_sco_flow_ctrl_inactive)
            {
                BTq_Reduce_Queue_Number_Of_Processed_Chunks(
                    SCO_IN_Q, sco_index, 1);
            }
        }
    }

    return qd;
}
#endif
#ifndef REDUCE_ROM
/*****************************************************************************
 * HCfc_Set_Host_To_Host_Controller_Flow_Control_Thresholds
 *
 * Sets the reporting thresholds for Host to Host Controller.
 *
 * This is an extension to the specification that allows the reporting on
 * Number Of Completed Packets event to be controlled.  The thresholds for
 * ACL and SCO packets can be set and an associated periodic timeout defined
 * where events will be sent periodically if the threshold is not reached.
 *
 * The threshold if 0 will result in no Host to HC Flow control.
 *
 * V1.1 Spec
 * While the Host Controller has HCI data packets in its buffer, it must 
 * keep sending the Number Of Completed Packets event to the Host at least 
 * periodically, until it finally reports that all the pending ACL Data
 * Packets have been transmitted or flushed. The rate with which this 
 * event is sent is manufacturer specific.
 * 
 * num_ACL_packets_threshold    Report after this number of ACL data packets 
 * num_SCO_packets_threshold    Report after this number of SCO data packets 
 * timeout_threshold_slots      Report after timeout value if non 0
 *****************************************************************************/
t_error HCfc_Set_Host_To_Host_Controller_Flow_Control_Thresholds(
    u_int8 num_ACL_packets_threshold, u_int8 num_SYN_packets_threshold, 
    u_int16 timeout_threshold_slots)
{
    hc_flow_ctrl.hc_completed_ACL_data_packets_report_threshold 
        = num_ACL_packets_threshold;
    hc_flow_ctrl.hc_completed_SYN_data_packets_report_threshold 
        = num_SYN_packets_threshold;
    hc_flow_ctrl.hc_completed_data_packets_report_timeout 
        = timeout_threshold_slots;

    /*
     * Restart the timer, otherwise it may have aged.
     */
    hc_flow_ctrl.hc_completed_data_packets_report_timer =
        BTtimer_Set_Slots(timeout_threshold_slots);

    return NO_ERROR;
}
#endif
#if (PRH_BS_CFG_SYS_HC_TO_HOST_FLOW_CONTROL_SUPPORTED==1)
#ifndef REDUCE_ROM
/*****************************************************************************
 * HCfc_Read_Host_Controller_To_Host_Flow_Control
 *
 * Read the Flow Control Enable for Host to HC Flow Control 
 *
 * V1.1 Spec:
 * No HCI command to read this
 *
 * Returns
 *  0   No HC to Host  Flow Control is disabled
 *  1   ACL Flow Control is enabled
 *  2   SCO Flow Control is enabled
 *  3   Both ACL/SCO Flow Control is enabled
 * Globals used
 *****************************************************************************/
u_int8 HCfc_Read_Host_Controller_To_Host_Flow_Control(void)
{
    return hc_flow_ctrl.hc2host_flow_control_enable;
}
#endif
#endif
#ifndef REDUCE_ROM
/*****************************************************************************
 * HCfc_Write_SYN_Flow_Control_Enable
 *
 * Write the SCO Flow Control Enable for Host to HC Flow Control 
 * i.e. whether HC will send the Number Of Completed Packets Event for SCO
 *
 * V1.1 Spec:
 * The Write_SYN_Flow_Control_Enable command provides the ability to write
 * the SYN_Flow_Control_Enable setting. By using this setting, the Host can
 * decide if the Host Controller will send Number Of Completed Packets events
 * for SYN Connection Handles. This setting allows the Host to enable and 
 * disable SYN flow control.
 * Note: the SYN_Flow_Control_Enable setting can only be changed if no 
 * connections exist.
 *
 * Returns
 *   NO_ERROR               if success
 *   COMMAND_DISALLOWED     if ACL connections exists
 *
 * Globals used
 *   g_LM_config_info
 *   hc_flow_ctrl
 *****************************************************************************/
t_error HCfc_Write_SYN_Flow_Control_Enable(u_int8 syn_flow_control_enable)
{
    t_error status;
    if (g_LM_config_info.num_acl_links != 0)
    {
        status = COMMAND_DISALLOWED;          
    }
    else if(syn_flow_control_enable > 1)
    {
        status = INVALID_HCI_PARAMETERS;
    }
    else
    {
        hc_flow_ctrl.host2hc_SYN_flow_control_Enable = syn_flow_control_enable;
        status = NO_ERROR;          
    }
    return status;
}
#endif
#if (PRH_BS_CFG_SYS_HC_TO_HOST_FLOW_CONTROL_SUPPORTED==1)
/*****************************************************************************
 * HCfc_Set_Host_Controller_To_Host_Flow_Control
 *
 * Sets the Host ACL/SCO Buffer Sizes/Number of.
 *
 * V1.1 Spec:
 * This command is used by the Host to turn flow control on or off for data and/or
 * voice sent in the direction from the Host Controller to the Host. If flow control is
 * turned off, the Host should not send the Host_Number_Of_Completed_Packets
 * command. That command will be ignored by the Host Controller if it is sent by the
 * Host and flow control is off. If flow control is turned on for HCI ACL Data Packets
 * and off for HCI SCO Data Packets, Host_Number_Of_Completed_Packets
 * commands sent by the Host should only contain Connection Handles for ACL
 * connections. If flow control is turned off for HCI ACL Data Packets and on for HCI
 * SCO Data Packets, Host_Number_Of_Completed_Packets commands sent by
 * the Host should only contain Connection Handles for SCO connections. If flow
 * control is turned on for HCI ACL Data Packets and HCI SCO Data Packets, the
 * Host will send Host_Number_Of_Completed_Packets commands both for ACL
 * connections and SCO connections.Note: The Flow_Control_Enable setting must
 * only be changed if no connections exist. 
 * Returns
 *   NO_ERROR               if success
 *   COMMAND_DISALLOWED     if ACL connections exists
 *
 * Globals used
 *   g_LM_config_info
 *   hc_flow_ctrl
 *****************************************************************************/
t_error HCfc_Set_Host_Controller_To_Host_Flow_Control(u_int8 flow_control_enable)
{
    t_error status;

    /*
     * If connections exist Then
     *    Report Command Disallowed
     * Else If Bits other than 0,1 set Then
     *    Report Unsupported Parameter Value
     *    Store flow_control_enable (0 Off, 1 ACL, 2 SCO, 3 Both)
     * Endif
     */
    if (g_LM_config_info.num_acl_links != 0)
    {
        status = COMMAND_DISALLOWED;          
    }
    else if (flow_control_enable & 0xFC)
    {
        status = INVALID_HCI_PARAMETERS;
    }
    else
    {
        /*
         * Flow_Control_Enable: Bit 0 ACL, Bit 1 SCO
         */
        hc_flow_ctrl.hc2host_flow_control_enable = flow_control_enable;
        status = NO_ERROR;          
    }
    return status;
}

/*****************************************************************************
 * HCIfc_Host_Number_Of_Completed_Packets
 *
 * Host Reports Number of Completed Packets by the Host, by connection handle
 *
 * V1.1 Spec
 * The Host_Number_Of_Completed_Packets command is used by the Host to
 * indicate to the Host Controller the number of HCI Data Packets that have been
 * completed for each Connection Handle since the previous Host_Number_Of_
 * Completed_Packets command was sent to the Host Controller. This means that
 * the corresponding buffer space has been freed in the Host. Based on this infor-mation,
 * and the Host_Total_Num_ACL_Data_Packets and Host_Total_Num_
 * SCO_Data_Packets command parameters of the Host_Buffer_Size command,
 * the Host Controller can determine for which Connection Handles the following
 * HCI Data Packets should be sent to the Host. The command should only be
 * issued by the Host if flow control in the direction from the Host Controller to the
 * Host is on and there is at least one connection, or if the Host Controller is in
 * local loopback mode. Otherwise, the command will be ignored by the Host
 * Controller. While the Host has HCI Data Packets in its buffers, it must keep
 * sending the Host_Number_Of_Completed_Packets command to the Host
 * Controller at least periodically, until it finally reports that all buffer space in the
 * Host used by ACL Data Packets has been freed. The rate with which this com-mand
 * is sent is manufacturer specific.
 *
 * p_hci_payload        Direct pointer to payload bytes.
 *      Number_Of_Handles: Size: 1 Byte
 *      Connection_Handle[i]: Size: Number_Of_Handles*2 Bytes (12 Bits meaningful)
 *      Host_Num_Of_Completed_Packets [i]: Size: Number_Of_Handles * 2 Bytes
 *
 *****************************************************************************/
t_error HCfc_Host_Number_Of_Completed_Packets(t_p_pdu p_hci_payload)
{
    if(PATCH_FUN[HCFC_HOST_NUMBER_OF_COMPLETED_PACKETS_ID]){
         
         return ((t_error (*)(t_p_pdu p_hci_payload))PATCH_FUN[HCFC_HOST_NUMBER_OF_COMPLETED_PACKETS_ID])(p_hci_payload);
    }

    u_int8 total_num_of_completed_packets;
    /*u_int8 number_of_handles;
    t_connectionHandle handle;
    u_int16 completed_packets;
    t_lmp_link *p_link;*/

    t_error status = NO_ERROR;
#ifndef BLUETOOTH_MODE_LE_ONLY
    /*
     * The command should only be issued by the Host if flow control in the
     * direction from the Controller to the Host is on and there is at least
     * one connection, or if the Controller is in local loopback mode.
     * Otherwise, the command will be ignored by the Controller.
     */
    if ((g_LM_config_info.num_acl_links==0) && (BTtst_Read_Loopback_Mode()!=LOCAL_LOOPBACK))
    {
        return status;
    }

    /*
     * If ACL (Bit 0) or SCO (Bit 1) hc 2 host flow control Then
     *    Process each handle/completed packets pair 
     * Endif
     */
    if (hc_flow_ctrl.hc2host_flow_control_enable)
    {

#if 0 /* Perform error checking of all paramater handles initially (To evalaute - previously used on SHOGA) */
        t_p_pdu _p_hci_payload = p_hci_payload;
        number_of_handles = *_p_hci_payload++;

        while (number_of_handles-- != 0)
        {
            handle = *_p_hci_payload++;
            handle += (*_p_hci_payload++ <<8);

            if(!LMaclctr_Find_Handle(handle) && !LMscoctr_Find_SCO_By_Connection_Handle(handle))
            {
                /*
                 * Invalid connection handle, which could be due to a release,
                 * However, if the Host_Number_Of_Completed_Packets command contains
                 * one or more invalid parameters, the Controller shall return a
                 * Command Complete event with a failure status indicating the
                 * Invalid HCI Command Parameters error code. 
                 *
                 * As host cannot know which parameter was invalid, error checking
                 * for all handles must be performed before any parameter processing,
                 * and if an error on any one handle, then no handles should be
                 * processed.
                 */
                return INVALID_HCI_PARAMETERS;
            }

            _p_hci_payload+=2; /* iter past completed_packets paramater */
        }
#endif

        total_num_of_completed_packets = 0;
        number_of_handles = *p_hci_payload++;

        while (number_of_handles-- != 0)
        {
            /*
             * Extract next handle and corresponding number of packets
             */
            handle = *p_hci_payload++;
            handle += (*p_hci_payload++ <<8);

            completed_packets = *p_hci_payload++;
            completed_packets += (*p_hci_payload++ <<8);

            p_link = LMaclctr_Find_Handle(handle);            

            if(p_link) 
            {    
                /*
                 * Q_ID = L2CAP_IN_Q for the device_index + start reference for queue
                 */
                BTq_Reduce_Queue_Number_Of_Processed_Chunks(
                    L2CAP_IN_Q, p_link->device_index, completed_packets);
                hc_flow_ctrl.host_available_number_ACL_data_packets 
                    += completed_packets;
            }
            else
            {
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1) 
                /*
                 * Check if this is for a SCO link
                 */
                t_sco_info *p_sco_link;
                p_sco_link = LMscoctr_Find_SCO_By_Connection_Handle(handle);
                if (p_sco_link)
                {
                    /*
                     * Q_ID = SCO_IN_Q for the sco_index + start reference for queue
                     */
                    BTq_Reduce_Queue_Number_Of_Processed_Chunks(SCO_IN_Q, 
                        (t_deviceIndex)LMscoctr_Get_SCO_Index(p_sco_link), 
                        completed_packets);
                    hc_flow_ctrl.host_available_number_SYN_data_packets 
                        += completed_packets;
                }
#endif
            }
        }
    }
#endif//BLUETOOTH_MODE_LE_ONLY
    return status;
}
#endif /* (PRH_BS_CFG_SYS_HC_TO_HOST_FLOW_CONTROL_SUPPORTED==1) */


/*****************************************************************************
 * HCfc_Release_All_Data_Queue_Packets
 *
 * Release any data packets, adjust flow counters and reset queues
 *
 * V1.1 Spec
 * When the Host receives a Disconnection Complete event, the Host can
 * assume that all HCI Data Packets that have been sent to the Host 
 * Controller for the returned Connection_Handle have been flushed, 
 * and that the corresponding data buffers have been freed. 
 * The Host Controller does not have to notify the Host about this in
 * a Number Of Completed Packets event. 
 *
 * If flow control is also enabled in the direction from the Host
 * Controller to the Host, the Host Controller can after it has sent 
 * a Disconnection_Complete event assume that the Host will flush its 
 * data buffers for the sent Connection_Handle when it receives the 
 * Disconnection_Complete event. The Host does not have to notify the Host
 * Controller about this in a Host_Number_Of_Completed_Packets command. 
 *
 * device_index        device index of connection being released
 *
 * Normally, called as a result of disconnection.
 * This function frees up queue descriptors and buffers to the free
 * list by resetting the queues (includes LMP queue).    
 *****************************************************************************/
void HCfc_Release_All_Data_Queue_Packets(t_deviceIndex device_index)
{
	
	if(PATCH_FUN[HCFC_RELEASE_ALL_DATA_QUEUE_PACKETS_ID]){
         ((void (*)(t_deviceIndex device_index))PATCH_FUN[HCFC_RELEASE_ALL_DATA_QUEUE_PACKETS_ID])(device_index);
         return ;
    }

    /*
     * Reset tx_ack_pending for device link before BTq_Reset to prevent
     * potential subsequent ACKing off empty queue.
     */
    //DL_Set_Tx_Ack_Pending(DL_Get_Device_Ref(device_index),FALSE);

    if (hc_flow_ctrl.hc2host_flow_control_enable & HC_TO_HOST_ACL_FLOW_CONTROL)
    {
        hc_flow_ctrl.host_available_number_ACL_data_packets += 
            BTq_Get_and_Reset_Queue_Number_Of_Processed_Chunks(L2CAP_IN_Q,
                device_index);
    }

    /* 
     * Reset the Queues for this device 
     * (resets the Host to HC Number of Completed Packets for L2CAP Out)
     */
    BTq_Reset(device_index);
}

#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
/*****************************************************************************
 * HCfc_Release_All_SYN_Queue_Packets
 *
 * Release any data packets, adjust flow counters and reset queues
 *
 * V1.1 Spec
 * When the Host receives a Disconnection Complete event, the Host can
 * assume that all HCI Data Packets that have been sent to the Host 
 * Controller for the returned Connection_Handle have been flushed, 
 * and that the corresponding data buffers have been freed. 
 * The Host Controller does not have to notify the Host about this in
 * a Number Of Completed Packets event. 
 *
 * If flow control is also enabled in the direction from the Host
 * Controller to the Host, the Host Controller can after it has sent 
 * a Disconnection_Complete event assume that the Host will flush its 
 * data buffers for the sent Connection_Handle when it receives the 
 * Disconnection_Complete event. The Host does not have to notify the Host
 * Controller about this in a Host_Number_Of_Completed_Packets command. 
 *
 * sco_q_index        device index of connection being released
 *
 * Normally, called as a result of disconnection.
 * This function frees up queue descriptors and buffers to the free
 * list by resetting the queues (includes LMP queue).    
 *****************************************************************************/
void HCfc_Release_All_SYN_Queue_Packets(t_deviceIndex sco_q_index)
{

    /*
     * Ensure that any remaining queue buffers are returned to pool
     * 1. HC to Host packets (SCO_OUT_Q)
     *    HC does not have to send an Host_Number_Of_Completed_Packets event.
     *     => Just zero any non-reported processed chunks.
     */
    BTq_Get_and_Reset_Queue_Number_Of_Processed_Chunks(
        SCO_OUT_Q, sco_q_index);

    /*
     * 2. Host to HC packets (SCO_IN_Q)
     *    Host does not have to send Host_Number_Of_Completed_Packets
     *    command after HCI_Disconnection_Complete event
     *
     *    => Zero any non-acknowledges chunks, ensuring that
     *       total available is adjusted correctly.
     *        If host sends one later, the handle will
     *       be invalid after this disconnection, and hence ignored.
     */
    hc_flow_ctrl.host_available_number_SYN_data_packets += 
        BTq_Get_and_Reset_Queue_Number_Of_Processed_Chunks(
            SCO_IN_Q, sco_q_index);

    /*
     * Ensure that the SCO IN and SCO OUT queues associated
     * with this SCO INDEX are left in a sane state.
     */
    BTq_Deallocate_Queue_Data(SCO_IN_Q, sco_q_index);
    BTq_Deallocate_Queue_Data(SCO_OUT_Q, sco_q_index);
}
#endif /* (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1) */
