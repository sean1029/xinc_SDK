/*****************************************************************************
 * MODULE NAME:    tra_hcit.c
 * PROJECT CODE:   Bluetooth
 * DESCRIPTION:    HCI Generic Transport Interface
 * MAINTAINER:     Tom Kerwick
 * CREATION DATE:  19 April 2000
 *
 * SOURCE CONTROL: $Id: tra_hcit.c,v 1.87 2014/03/11 03:14:17 garyf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc
 *     All rights reserved.
 *
 * NOTES TO USERS: <none>
 * 
 *
 *****************************************************************************/
#include "sys_config.h"

#include "tra_hcit.h"
#include "tra_uart.h"
#include "tra_usb.h"
#include "tra_queue.h"

#if (PRH_BT_COMBINED==1)
#include "tra_combined.h"
#endif

#include "lc_interface.h"

#if (PRH_BS_CFG_SYS_SLEEP_IN_STANDBY_SUPPORTED==1)
#include "uslc_chan_ctrl.h"
#include "lmp_timer.h"
#endif

#include "lmp_config.h"
#include "lmp_acl_container.h"
#include "lmp_acl_connection.h"
#include "lmp_sco_container.h"
#include "hc_flow_control.h"
#include "hc_event_gen.h"
#include "hc_const.h"

#include "sys_mmi.h"
#if 1// (LE_INCLUDED == 1)
#include "le_config.h"
#include "le_security.h"
#include "le_connection.h"
#endif

#ifndef __USE_INLINES__
#include "tra_hcit_impl.h"
#endif

#include "bt_test.h"
#include "global_val.h"
#include "patch_function_id.h"

extern t_LE_Config LE_config;
/*
 * Structure to store necessary information for HCI transport
 * subsystem.
 */ 

extern     struct tra_hcit_info volatile *sys_hcit_info;

/*
 * Function pointer, pointing to the HCI transport registration function
 * This function is implemented in sys_main.c (or equivalent) in the appropriate
 * hal subdirectory. The assignment is also performed in sys_main.c
 */

extern void* (*TRAhcit_Register)(void);

extern u_int32 (*hci_generic_event_callback)(u_int8 *data, u_int32 pdu_len, u_int8 *header, 
                            u_int8 head_len);

extern void TRAhcit_Generic_Dispatcher(u_int8 *data, u_int32 pdu_len, u_int8 *header, 
                            u_int8 head_len,u_int8 q_type,t_deviceIndex device_index);
/*
 * If Debug TRA hcit enabled (sys_debug.h) then use counters
 */
#if (PRH_BS_DBG_TRA_HCIT==1)
u_int32 tra_num_acl_pkts_enqueued;
u_int32 tra_num_acl_pkts_missed;
u_int32 tra_num_acl_ser_commit;
u_int32 tra_num_sco_pkts_enqueued;
u_int32 tra_num_sco_pkts_missed;
u_int32 tra_num_sco_ser_commit;
#define TRAhcit_Inc_Cntr(cntr) (++cntr) 
#else
#define TRAhcit_Inc_Cntr(cntr) 
#endif

/*
 * This is a table of function pointers corresponding to initialisation functions
 * of layers which use the physical devices.
 * At present, only a UART decoding layer is supported. TRA_HCIT_TYPE_ENDS
 * is the last enumerator in the HCI physical transport device list. It is
 * used to terminate the list.
 */

extern void (*tra_hcit_initialise_table[TRA_HCIT_TYPE_ENDS])(void) ;


/*
 * Corresponding table for shutdown routines.
 */

extern  void (*tra_hcit_shutdown_table[TRA_HCIT_TYPE_ENDS])(void);


/*
 * And a table for the transmit routines - NOTE: See the optimisation below
 * for the case of UART support only being included.
 */

extern void (*tra_hcit_transmit_table[TRA_HCIT_TYPE_ENDS])
    (u_int8 *, u_int32, u_int8 *, u_int8, u_int8, t_deviceIndex);

/*
 * Variable determining if the HCI transport is running or not
 */

extern volatile u_int8 _tra_hcit_active;

/*****************************************************************************
 * TRAhcit_Initialise
 *
 * This function is not re-entrant
 *
 * Returns:     0   if initialisation completed successfully
 *              1   if transport system already active
 ****************************************************************************/
int TRAhcit_Initialise(void)
{
	if(PATCH_FUN[TRAHCIT_INITIALISE_ID]){
         
         return ((int (*)(void))PATCH_FUN[TRAHCIT_INITIALISE_ID])();
    }
    /*
     * If it is already initialised, do nothing, return 1
     */
    if(_tra_hcit_active)
    {
        return 1;
    }    

    /*
     * Call the register function pointer - this is implemented 
     * in the appropriate HAL. This will return a pointer to the 
     * appropriate structure, casted to the general structure.
     * see tra_hcit.h for details.
     */

    sys_hcit_info = (struct tra_hcit_info volatile*)TRAhcit_Register();

    /*
     * Initialise the handling layer (which in turn will initialise the physical device)
     */
    if (tra_hcit_initialise_table[sys_hcit_info->type])
        tra_hcit_initialise_table[sys_hcit_info->type]();
    _tra_hcit_active = 1;

    return 0;
}
#ifndef REDUCE_ROM2
/*****************************************************************************
 * TRAhcit_Shutdown
 *
 * This function is not re-entrant 
 *
 * Returns:     0   if shutdown completed successfully
 *              1   if transport system not active
 ****************************************************************************/
int TRAhcit_Shutdown(void)
{
	
     if(PATCH_FUN[TRAHCIT_SHUTDOWN_ID]){
         
         return ((int (*)(void))PATCH_FUN[TRAHCIT_SHUTDOWN_ID])();
      }
    /*
     * Shutdown the physical layer.
     * If not already initialised, return 1
     */
    if(!_tra_hcit_active)
        return 1;

    /*
     * Shut down the decoding layer (which in turn will shutdown the physical device)
     */
    if(tra_hcit_shutdown_table[sys_hcit_info->type])
       tra_hcit_shutdown_table[sys_hcit_info->type]();
    _tra_hcit_active = 0;
    return 0;
}
#endif

/*****************************************************************************
 * TRAhcit_Generic_Get_Rx_Buf
 *
 * Gets a buffer from the queuing system
 *
 * ptype            TRA_HCIT_COMMAND | TRA_HCIT_ACLDATA | TRA_HCIT_SCODATA
 * size             Length of command, of acl data, of sco data
 * hci_header       Header of HCI packet
 * 
 ****************************************************************************/
u_int8 *TRAhcit_Generic_Get_Rx_Buf(u_int8 ptype, int size, u_int8 *hci_header)
{
	
	if(PATCH_FUN[TRAHCIT_GENERIC_GET_RX_BUF_ID]){
         
         return ((u_int8 *(*)(u_int8 ptype, int size, u_int8 *hci_header))PATCH_FUN[TRAHCIT_GENERIC_GET_RX_BUF_ID])(ptype, size, hci_header);
    }

    t_q_descr *qd;
 
//    SYSmmi_Display_Event(eSYSmmi_HCIt_Activity_Event);

    /*
     * At this point I have the length (in size) and the 
     * handle/pb/bc flags in rx_scratch [0] and [1]
     */

    if (ptype==TRA_HCIT_COMMAND)
    {
        qd = BTq_Enqueue(HCI_COMMAND_Q, 0, (t_length) size);
        if(qd)
        {    
            /* Explicit copies for unrolling */
            qd->data[0] = hci_header[0];
            qd->data[1] = hci_header[1];
            qd->data[2] = hci_header[2];
            return &qd->data[3];    /* Return the next location to write into (but don't modify qd->data!) */
        }
        return 0x0;
    }
    else if (ptype==TRA_HCIT_ACLDATA)
    {
        /* Need to decode the handle and the pb/bc flag  */
       
        /*
         * In this case, if at any stage during the decoding, something
         * is found to be out of range or otherwise malformed, a goto
         * to the "received_pdu_is_malformed" is executed. The other paths
         * of the function will return before this code is reached, if no
         * errors have occurred. This is done to minimise both code duplication
         * and the amount of time spent in the transport driver interrupt
         * e.g. UARTs may use FIQ (highest priority) on an ARM based platform
         * and as such need to be as fast as possible.
         */

        t_deviceIndex device_index=0;
        u_int16 handle = hci_header[0] | ((hci_header[1] & 0xf) << 8);
        u_int8 pb_flag = (hci_header[1] >> 4) & 0x3;
        u_int8 bc_flags = (hci_header[1] >> 6) & 0x3;
    	t_LE_Connection* p_le_connection;

        /*
         * Is this a broadcast packet ?
         */

        if(bc_flags != 0x0)
        {
      
            /*
             * Any reasonably optimising compiler should generate a short jump
             * table for this switch.
             */            
            switch(bc_flags)
            {
                case 0x1: 
                    /* 
                     * Active broadcast - packet sent to all slaves,  
                     * Queue corresponds to device 0
                     */ 
                    device_index = 0;
#ifndef BLUETOOTH_MODE_LE_ONLY
                    if(g_LM_config_info.active_broadcast_handle == 0xFFFF) 
                        g_LM_config_info.active_broadcast_handle = handle;
                    else if (handle != g_LM_config_info.active_broadcast_handle)
#endif//BLUETOOTH_MODE_LE_ONLY
                        return 0x0;

                    break;

                case 0x2: 
                    /* 
                     * Piconet broadcast - all slaves including parked ones, 
                     * Queue corresponds to device MAX_ACTIVE_DEVICE_LINKS
                     */    
#ifndef BLUETOOTH_MODE_LE_ONLY
                    device_index = MAX_ACTIVE_DEVICE_LINKS; 
                    if(g_LM_config_info.piconet_broadcast_handle == 0xFFFF) 
                        g_LM_config_info.piconet_broadcast_handle = handle;
                    else if (handle != g_LM_config_info.piconet_broadcast_handle)
#endif//BLUETOOTH_MODE_LE_ONLY
                        return 0x0;
                    break;
       
                case 0x3: /* Exception, invalid flags */
                    HCeg_Hardware_Error_Event(PRH_BS_HW_ERROR_HCIT_RX_PDU_MALFORMED);   
                    return 0x0;
                    break;
            }
        }
        else 
        {
#ifndef BLUETOOTH_MODE_LE_ONLY//lmp link in "link_container", LE link in "LE_connections"
            t_lmp_link *p_link = LMaclctr_Find_Handle((t_connectionHandle)handle);            

            if(p_link) 
            {    
                device_index = p_link->device_index;
            }
			else
#endif
			{
			   	p_le_connection = LEconnection_Find_LE_Link((t_connectionHandle)handle);
#if 0 // GF 31 Dec 2013 - For CES demo - IVT send data after initiating a disconnect so HW error may occur
      // best to silently discard the data

            	if ((p_le_connection) && (p_le_connection->active))
            	{
            		device_index = p_le_connection->device_index;
         			/*
			         * If encrypted link add 4 bytes for the MIC which will be added by the
			         * MiniSched when encrypting the payload.
         			*/
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
			        if (p_le_connection->current_security_state & LE_ENCRYPTION_ACTIVE)
			        {
        				size+=4;
			        }
#endif
            	}
#else
            	if ((p_le_connection) && (p_le_connection->active))
            	{
            		device_index = p_le_connection->device_index;

         			/*
			         * If encrypted link add 4 bytes for the MIC which will be added by the
			         * MiniSched when encrypting the payload.
         			*/
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
			        if ((p_le_connection->current_security_state & LE_ENCRYPTION_ACTIVE) ||
						(p_le_connection->current_security_state & LE_ENCRYPTION_PAUSED))
			        {
        				size+=4;
			        }
#endif
            	}
            	else
            	{
                	HCeg_Hardware_Error_Event(PRH_BS_HW_ERROR_INVALID_LMP_LINK);   
                	return 0x0;
				}
            }
        }
#if(PRH_BS_CFG_SYS_LMP_NONFLUSHABLE_PBF_SUPPORTED !=1)

        if ((pb_flag == 0) || (pb_flag == 3)) /* Exception, invalid flags */
        {
            HCeg_Hardware_Error_Event(PRH_BS_HW_ERROR_HCIT_RX_PDU_MALFORMED);
            return 0x0;
        }
      
        /*
         * If ACL Data Queue Space is Available Then
         *    Enqueue packet (assigns qd->device_index, qd->length)
         * Else
         *    Report HCI_DATA_BUFFER_OVERFLOW_EVENT
         * Endif
         */
        qd = BTq_Enqueue(L2CAP_OUT_Q, device_index, (t_length) size);                    
        if(qd)
        {
             TRAhcit_Inc_Cntr(tra_num_acl_pkts_enqueued);
             qd->message_type = (t_LCHmessage)pb_flag;
             qd->broadcast_flags = bc_flags;
			 qd->auto_flushable = AUTO_FLUSHABLE;
			 if (pb_flag == 0x02)
			 {
				 qd->insert_time = LC_Get_Native_Clock();
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
				if ((p_le_connection) && (p_le_connection->active) && 
				    (p_le_connection->current_security_state & LE_ENCRYPTION_ACTIVE))
			 	{
				 	qd->encrypt_status = LE_DATA_ENCRYPT_PENDING;
			 	}
			 	else
#endif
				 	qd->encrypt_status = LE_DATA_NOT_ENCRYPTED;
			 }

             sys_hcit_info->rx_device_index = device_index;
             return qd->data; /* Early return for speed (this call is in the context of UART interrupt) */
        }
        else 
        {
             t_lm_event_info event_info;

             TRAhcit_Inc_Cntr(tra_num_acl_pkts_missed);
             event_info.link_type = ACL_LINK;
             HCeg_Generate_Event(HCI_DATA_BUFFER_OVERFLOW_EVENT, &event_info);
             return 0x0;
        }
#else  // #if(PRH_BS_CFG_SYS_LMP_NONFLUSHABLE_PBF_SUPPORTED ==1)
        if (pb_flag == 3) /* Exception, invalid flags  pb = 0 is now valid*/
        {
            HCeg_Hardware_Error_Event(PRH_BS_HW_ERROR_HCIT_RX_PDU_MALFORMED);
            return 0x0;
        }
      
        /*
         * If ACL Data Queue Space is Available Then
         *    Enqueue packet (assigns qd->device_index, qd->length)
         * Else
         *    Report HCI_DATA_BUFFER_OVERFLOW_EVENT
         * Endif
         */
        qd = BTq_Enqueue(L2CAP_OUT_Q, device_index, (t_length) size);                    
        if(qd)
        {
             TRAhcit_Inc_Cntr(tra_num_acl_pkts_enqueued);
			 if (pb_flag == 00)
			 {
				 qd->auto_flushable = NON_AUTO_FLUSHABLE;
                 qd->message_type = LCH_start;
				 qd->insert_time = LC_Get_Native_Clock();

				 	qd->encrypt_status = LE_DATA_NOT_ENCRYPTED;//for more data
			 }
			 else if (pb_flag == 0x02)
			 {
				 qd->auto_flushable = AUTO_FLUSHABLE;
                 qd->message_type = LCH_start;
				 qd->insert_time = LC_Get_Native_Clock();
			 }
			 else
				 qd->message_type = (t_LCHmessage)pb_flag;


             qd->broadcast_flags = bc_flags;
             sys_hcit_info->rx_device_index = device_index;
             return qd->data; /* Early return for speed (this call is in the context of UART interrupt) */
        }
        else 
        {
             t_lm_event_info event_info;

             TRAhcit_Inc_Cntr(tra_num_acl_pkts_missed);
             event_info.link_type = ACL_LINK;
             HCeg_Generate_Event(HCI_DATA_BUFFER_OVERFLOW_EVENT, &event_info);
             return 0x0;
        }
#endif
    }

#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
    else if (ptype==TRA_HCIT_SCODATA)
    {
       /* 
        * Need to decode the handle and find the SCO Q index for this handle 
        */
        t_deviceIndex sco_index;
        u_int16 handle = hci_header[0] | ((hci_header[1] & 0xf) << 8);
        t_sco_info *p_sco_link = LMscoctr_Find_SCO_By_Connection_Handle(handle);

        /*
         * Customer request: Where SCO flow control is not enabled, prevent app/host
         * getting buffer through embedded generic hci interface when SCO is not active.
         */
        if(p_sco_link && ((BUILD_TYPE!=GENERIC_BUILD) || HCfc_Read_SYN_Flow_Control_Enable()
        ||(p_sco_link->state > SCO_IDLE) || (BTtst_Read_Loopback_Mode()==LOCAL_LOOPBACK)))
        {
            t_q_descr *qd;
            sco_index = LMscoctr_Get_SCO_Index(p_sco_link);
            /*
             * If SCO Data Queue Space is Available Then
             *    Enqueue packet (assigns qd->device_index, qd->length)
             * Else
             *    Report HCI_DATA_BUFFER_OVERFLOW_EVENT
             * Endif
             */
            qd = BTq_Enqueue(SCO_OUT_Q, sco_index, (t_length) size);
            if(qd)
            {
                TRAhcit_Inc_Cntr(tra_num_sco_pkts_enqueued);
                sys_hcit_info->rx_sco_device_index = sco_index;
                return qd->data;
            }
            else 
            {
                TRAhcit_Inc_Cntr(tra_num_sco_pkts_missed);
                /*
                 * One 3rd-party host unfortunately uses return value 0 instead of SCO flow control
                 * in generic build to determine buffer availability, suppress overflow event in this
                 * case as overflow condition can be expected and is monitored and by host directly.
                 */
                if((BUILD_TYPE!=GENERIC_BUILD) || HCfc_Read_SYN_Flow_Control_Enable())
                {
                    t_lm_event_info event_info;
                    event_info.link_type = SCO_LINK;
                    HCeg_Generate_Event(HCI_DATA_BUFFER_OVERFLOW_EVENT, &event_info);
                }
                return 0x0;
            }
       }    
   }
#endif
   return 0x0;
}

/*****************************************************************************
 * TRAhcit_Generic_Commit_Rx_Buf
 *
 * Commits a buffer allocated by HCit_generic_get_buf to the queueing system
 ****************************************************************************/
void TRAhcit_Generic_Commit_Rx_Buf(u_int8 buffer_type)
{
	
	if(PATCH_FUN[TRAHCIT_GENERIC_COMMIT_RX_BUF_ID]){
         ((void (*)(u_int8 buffer_type))PATCH_FUN[TRAHCIT_GENERIC_COMMIT_RX_BUF_ID])(buffer_type);
         return ;
    }

//    SYSmmi_Display_Event(eSYSmmi_HCIt_Activity_Event);

    if (buffer_type == TRA_HCIT_COMMAND)
    {
        BTq_Commit(HCI_COMMAND_Q,0);

#if (PRH_BS_CFG_SYS_LOW_POWER_MODE_SUPPORTED==1)
#if (PRH_BS_CFG_SYS_SLEEP_IN_STANDBY_SUPPORTED==1)
#ifndef BLUETOOTH_MODE_LE_ONLY
        if (g_LM_config_info.num_acl_links==0)
        {
            LMtmr_Reset_Timer(g_LM_config_info.standby_power_timer_index,LM_CONFIG_AWAKE_IN_STANDBY_MONITOR_PERIOD);
        }
#endif//BLUETOOTH_MODE_LE_ONLY
#endif
#ifndef BLUETOOTH_MODE_LE_ONLY

        if ((SLEEP_PROCEDURE == USLCchac_Get_Active_Procedure())
           || USLCchac_Is_Sleep_Request_Pending())
        {
            USLCsleep_Cancel();
        }
#endif        
#endif

    }
    else if (buffer_type == TRA_HCIT_ACLDATA)
    {
        TRAhcit_Inc_Cntr(tra_num_acl_ser_commit);
        BTq_Commit(L2CAP_OUT_Q, sys_hcit_info->rx_device_index);  
		
        {
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
        	t_LE_Connection* p_connection = LEconnection_Find_Device_Index(sys_hcit_info->rx_device_index);

        	if ((p_connection) && (LEconnection_Is_Active(p_connection)) && (p_connection->current_security_state & LE_ENCRYPTION_ACTIVE))
        	{
        		LE_config.encrypt_pending_flag =0x01;
        		LE_config.device_index_pending_data_encrypt = sys_hcit_info->rx_device_index;// p_connection->link_id;
        	}
#endif
        }        
    }

#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
    else if (buffer_type == TRA_HCIT_SCODATA)
    {
        TRAhcit_Inc_Cntr(tra_num_sco_ser_commit);
        BTq_Commit(SCO_OUT_Q, sys_hcit_info->rx_sco_device_index);          
    }
#endif
 
}

/*****************************************************************************
 * TRAhcit_Generic_Acknowledge_Complete_Tx
 *
 * Acknowledge the completion of tranmission - this allows the next item 
 * on the queue to be transmitted.
 ****************************************************************************/
void TRAhcit_Generic_Acknowledge_Complete_Tx(u_int8 queue, u_int32 buf_len)
{
	
	if(PATCH_FUN[TRAHCIT_GENERIC_ACKNOWLEDGE_COMPLETE_TX_ID]){
         ((void (*)(u_int8 queue, u_int32 buf_len))PATCH_FUN[TRAHCIT_GENERIC_ACKNOWLEDGE_COMPLETE_TX_ID])(queue,   buf_len);
         return ;
    }

    t_deviceIndex i;
    
    if (queue == HCI_EVENT_Q)
        i=0;
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
    else if ((BUILD_TYPE==GENERIC_BUILD) && (queue == SCO_IN_Q))
        i=sys_hcit_info->tx_sco_device_index;
#endif
    else
        i=sys_hcit_info->tx_device_index;
        
    if(!BTq_Is_Queue_Empty(queue, i)) /* PROTECT QUEUES FROM APP ISSUES */
    {
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
    	t_LE_Connection* p_connection = LEconnection_Find_Device_Index(i);

    	if (p_connection && (p_connection->active) && (p_connection->current_security_state & LE_ENCRYPTION_ACTIVE))
    		buf_len+=4;
#endif
        BTq_Ack_Last_Dequeued(queue, i, (t_length)buf_len);
    }
}

#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
/*****************************************************************************
 * TRAhcit_Dispatch_Pending_SCO
 *
 * Dispatch any SCO to be transmitted to the host
 *
 * Check the SCO incoming queues for entries.
 * Service each SCO device in a round robin fashion - priorities can 
 * be easily retrofitted if required for added value
 *
 * HC to Host  flow control needs to be added if necessary
 ****************************************************************************/
t_error TRAhcit_Dispatch_Pending_SCO(void)
{
    u_int8 sco_index;    
    t_sco_info *p_sco_link;
    t_q_descr *qd;

    /*
     * This array stores the header of the PDU to be transmitted to the host.
     * The maximum header is 4, 1 for the type byte and 3 for an ACL header.
     */
    static u_int8 hcit_tx_to_host_sco_header[4];

#if (BUILD_TYPE==GENERIC_BUILD)
    if(! sys_hcit_info->tx_sco_busy)
#else
    if(! sys_hcit_info->tx_busy)
#endif
    {

        /* 
         * Start with device index 0 
         */
    
        for(sco_index=0; 
            sco_index<PRH_BS_CFG_SYS_MAX_SYN_LINKS_VIA_HCI; sco_index++)
        {
            qd = HCfc_Get_Next_HC_to_Host_SYN_Descriptor(sco_index);

            if(qd)
            {
                /*
                 * At this point, we have to construct the HCI payload header 
                 * The event queue entries will already contain this.
                 * Format = (lsb l to r) handle : pb : bc : length
                 */                
                u_int16 handle;

                p_sco_link = LMscoctr_Find_SCO_By_SCO_Index(sco_index);
                handle = p_sco_link->connection_handle;


                hcit_tx_to_host_sco_header[0] = TRA_HCIT_SCODATA;
                hcit_tx_to_host_sco_header[1] = handle & 0xFF;
				hcit_tx_to_host_sco_header[2] = (u_int8) ((handle >> 8) | (qd->message_type << 4));
                hcit_tx_to_host_sco_header[3] = (u_int8) qd->length;

                /*
                 * The PDU is now ready for transmission to the host. At this
                 * point, the decoding sub-layer transmit function is called. 
                 * This will in turn call the appropriate physical layer 
                 * transmit function.
                 */

#if TRA_HCIT_UART_ONLY_SUPPORTED == 1
                /*
                 * This is an optimisation for the most common case 
                 *  - only a UART supported for hci transport.
                 */
                TRAhcit_UART_Transmit(
                    qd->data, qd->length, hcit_tx_to_host_sco_header, 4,
                    SCO_IN_Q, qd->device_index);

#else                
                /* 
                 * Otherwise, the slightly more expensive array lookup is used 
                 */
#if (BUILD_TYPE==GENERIC_BUILD)
                sys_hcit_info->tx_sco_device_index = qd->device_index;
                sys_hcit_info->tx_sco_busy = 1;
#endif
                (*tra_hcit_transmit_table[sys_hcit_info->type])
                    (qd->data, qd->length, hcit_tx_to_host_sco_header, 4,
                    SCO_IN_Q, qd->device_index);                    
#endif

                return (t_error)0;

                /* 
                 * This means we only send one PDU per iteration, so as not to
                 * overload the physical transport driver.
                 */
            }
        }
    }
    return (t_error)1;
}
#else
#define TRAhcit_Dispatch_Pending_SCO()  ((t_error) 0)
#endif
extern void LEconnection_Local_Disconnect(t_LE_Connection* p_connection,u_int8 reason);

/*****************************************************************************
 * TRAhcit_Dispatch_Pending_Data
 *
 * Dispatch any data to be transmitted to the host
 *
 * Check the L2CAP incoming queues for entries.
 * Service each device in a round robin fashion - priorities can 
 * be easily retrofitted if required for added value
 *
 * HC to Host  flow control needs to be added if necessary
 ****************************************************************************/
t_error TRAhcit_Dispatch_Pending_Data(void)
{
	
	if(PATCH_FUN[TRAHCIT_DISPATCH_PENDING_DATA_ID]){
         
         return ((t_error (*)(void))PATCH_FUN[TRAHCIT_DISPATCH_PENDING_DATA_ID])();
    }

    t_deviceIndex device_index;    
    t_lmp_link *p_link ;
	u_int8 link_id;
    t_q_descr *qd;

    /*
     * This array stores the header of the PDU to be transmitted to the host.
     * The maximum header is 5, 1 for the type byte and 4 for an ACL header.
     */
    extern u_int8 hcit_tx_to_host_acl_header[5];

#if (BUILD_TYPE==GENERIC_BUILD)
    if(! sys_hcit_info->tx_acl_busy)
#else
    if(! sys_hcit_info->tx_busy)
#endif
    {
//#define DEFAULT_MAX_NUM_LE_LINKS  3

    	for (link_id = 0; link_id < DEFAULT_MAX_NUM_LE_LINKS;link_id++)
        {
            t_LE_Connection* p_connection = LEconnection_Find_Link_Entry(link_id);
			

            // GF 8-Nov -- Extend to not allow Data be sent to the host which a connection
            // complete event is pending.
#if 1
			if((p_connection) && (p_connection->active) && (LE_config.connection_complete_pending==0x00))
#else
			if((p_connection) && (p_connection->active))
#endif
        	{
        		u_int16 length;

        		qd = HCfc_Get_Next_HC_to_Host_Data_Descriptor(p_connection->device_index);
        		if(qd)
        		{

#if 0 // GF Temp 12 Dec
        			/*
        			 * As this is run from MiniSched we can Decrypt any L2CAP data at this point.
        			 * As the original Queue Entry had a MIC field included we have to adjust the length
        			 * field by 4.
        			 */
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
        			if ((qd->encrypt_status & LE_DATA_DECRYPT_PENDING) && (p_connection->current_security_state & LE_ENCRYPTION_ACTIVE))
        			{
#if 0 // GF 04 Sept
        				if (LE_config.state == STANDBY_STATE)
#endif
        				{

							status = LEconnection_Decrypt_Incoming_Pdu(p_connection,qd->data,(u_int8)qd->length,qd->enc_pkt_count,qd->message_type);
							if (status == NO_ERROR)
							{
								length = qd->length - 4;
								qd->encrypt_status = LE_DATA_NOT_ENCRYPTED;
							}

					      return NO_ERROR;
					

        			}
					else

#endif
#endif
					{
        				/*
        				 * At this point, we have to construct the HCI payload header
        				 * The event queue entries will already contain this.
        				 * Format = (lsb l to r) handle : pb : bc : length
        				 */
        				u_int16 handle;
#if 1 //(LE_INCLUDED == 1)


        				handle = LEconnection_Determine_Connection_Handle(p_connection);
#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
        				if ((qd->encrypt_status & LE_DATA_DECRYPT_PENDING) && (p_connection->current_security_state & LE_ENCRYPTION_ACTIVE) && 	(!(p_connection->ll_action_pending & LL_ENCRYPTION_TRANSITIONING)))
        				{

#if 0
							t_error status;
							status = LEconnection_Decrypt_Incoming_Pdu(p_connection,qd->data,(u_int8)qd->length,qd->enc_pkt_count,qd->message_type);
							if (status == NO_ERROR)
							{
								length = qd->length - 4;
								qd->encrypt_status = LE_DATA_NOT_ENCRYPTED;
							}
							else
							{
								BTq_Ack_Last_Dequeued(L2CAP_IN_Q,qd->device_index,qd->length);
								LEconnection_Local_Disconnect(p_connection, status /* MIC Mismatch */);

								return (t_error)0;
							}
#else
							//while (NO_ERROR != LEconnection_Decrypt_Incoming_Pdu(p_connection,qd->data,(u_int8)qd->length,qd->enc_pkt_count,qd->message_type))
							//{
							//	;
							//}
							while (NO_ERROR != LEconnection_Decrypt_Incoming_Pdu(p_connection,qd->data,(u_int8)qd->length,qd->enc_pkt_count,qd->message_type))
							{
                                 // slove bug sev/sla/bi03c
								 LEconnection_Local_Disconnect(p_connection , EC_CONNECTION_TERMINATED_DUE_TO_MIC_FAILURE);
								 
								 //- NOTE :1) change 0 to p_connection->device_index. 
								 //-       2) delete call the function. 
								 //BTq_Ack_Last_Dequeued(L2CAP_IN_Q, p_connection->device_index, qd->length);
								 
								 return (t_error)0;

							}
							length = qd->length - 4;
							qd->encrypt_status = LE_DATA_NOT_ENCRYPTED;
#endif
        				}
        				else if ((p_connection->ll_action_pending & LL_ENCRYPTION_TRANSITIONING))
        				{
        					return (t_error)0;
        				}
        				else
#endif
        				{
        					length = qd->length;
        				}

#else
        				p_link = LMaclctr_Find_Device_Index(qd->device_index);

        				handle = p_link->handle;
#endif

        				hcit_tx_to_host_acl_header[0] = TRA_HCIT_ACLDATA;
        				hcit_tx_to_host_acl_header[1] = handle & 0xff;
        				hcit_tx_to_host_acl_header[2] = (u_int8)(handle >> 8)
                                                | (qd->message_type<<4);
        				hcit_tx_to_host_acl_header[3] = length & 0xff;
        				hcit_tx_to_host_acl_header[4] = (u_int8)((length >> 8) & 0xff);

        				/*
        				 * The PDU is now ready for transmission to the host. At this
        				 * point, the decoding sub-layer transmit function is called.
        				 * This will in turn call the appropriate physical layer
        				 * transmit function.
        				 */
#if TRA_HCIT_UART_ONLY_SUPPORTED == 1
        				TRAhcit_UART_Transmit(
        						qd->data, length, hcit_tx_to_host_acl_header, 5,
        						L2CAP_IN_Q, qd->device_index);
#else
#if (BUILD_TYPE==GENERIC_BUILD)
                        sys_hcit_info->tx_acl_busy = 1;
                        sys_hcit_info->tx_device_index = qd->device_index;	 		
                        (*tra_hcit_transmit_table[sys_hcit_info->type])(qd->data, length,
                                hcit_tx_to_host_acl_header, 5, L2CAP_IN_Q, qd->device_index); 
#endif
#endif
        				return (t_error)0;

        				/*
        				 * This means we only send one PDU per iteration, so as not to
        				 * overload the physical transport driver.
        				 */
        			}

        		}
			}

		}

#endif
    }
    return (t_error)1;
}

/*****************************************************************************
 * TRAhcit_Dispatch_Pending_Event
 *
 * Take pending events off the HCI Event queue and transmit them
 * to the host.
 ****************************************************************************/
t_error TRAhcit_Dispatch_Pending_Event(void)
{
	
	if(PATCH_FUN[TRAHCIT_DISPATCH_PENDING_EVENT_ID]){
         
         return ((t_error (*)(void))PATCH_FUN[TRAHCIT_DISPATCH_PENDING_EVENT_ID])();
    }

    t_q_descr *qd;
    u_int8 _header_byte;

#if (BUILD_TYPE==GENERIC_BUILD)
    if(!sys_hcit_info->tx_evt_busy)
#else
    if(!sys_hcit_info->tx_busy)
#endif
    {
        qd = BTq_Dequeue_Next(HCI_EVENT_Q,0);
        if(qd)
        {        
            _header_byte = TRA_HCIT_EVENT;

#if TRA_HCIT_UART_ONLY_SUPPORTED == 1

            /*
             * This is an optimisation for the most common case 
             * - only a UART supported for hci transport.
             */

            TRAhcit_UART_Transmit(qd->data, qd->length, 
                &_header_byte, 1, HCI_EVENT_Q, qd->device_index);

#else        
            /* 
             * Otherwise, the slightly more expensive array lookup is used 
             */
#if (BUILD_TYPE==GENERIC_BUILD)
            sys_hcit_info->tx_evt_busy = 1;
#endif
            (*tra_hcit_transmit_table[sys_hcit_info->type])(qd->data, qd->length,
                &_header_byte, 1, HCI_EVENT_Q, qd->device_index);                    
#endif
            return (t_error)0;
        }
    }
    return (t_error)1;
}


/*****************************************************************************
 * TRAhcit_Register_Generic_Dispatcher
 *
 * Register the generic data dispatcher for HCI Generic
 ****************************************************************************/
t_error TRAhcit_Register_Generic_Dispatcher(
    u_int32 (*callback)(u_int8 *data, u_int32 pdu_len, u_int8 *header, u_int8 head_len))
{
    hci_generic_event_callback = callback;
    return NO_ERROR;
}

/*****************************************************************************
 * TRAhcit_Generic_Dispatcher
 *
 * The generic data dispatcher for HCI Generic
 ****************************************************************************/
void TRAhcit_Generic_Dispatcher(u_int8 *data, u_int32 pdu_len, u_int8 *header, 
                            u_int8 head_len, u_int8 q_type, t_deviceIndex device_index)
{
#if (BUILD_TYPE==GENERIC_BUILD)

		if(0 == hci_generic_event_callback(data, pdu_len, header, head_len)) return;		
		TRAhcit_Generic_Acknowledge_Complete_Tx(q_type, pdu_len);
		if(HCI_EVENT_Q == q_type)	sys_hcit_info->tx_evt_busy = 0;
		else if(L2CAP_IN_Q == q_type)  sys_hcit_info->tx_acl_busy = 0;

#endif	
}

