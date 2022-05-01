/************************************************************************
 * MODULE NAME:    bt_test.c
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    Routines to manage loopback & DUT mode.
 * MAINTAINER:     Gary Fleming
 * CREATION DATE:  06 August 1999
 *
 * SOURCE CONTROL: $Id: bt_test.c,v 1.90 2011/01/19 17:23:33 garyf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * This module implements the functionality to support the 
 * HCI Tester Commands:  
 *      HCI_Read_Loopback_Mode, HCI_Write_Loopback_Mode
 *      HCI_Enable_Device_Under_Test_Mode
 *
 *************************************************************************/
#include "sys_config.h"

#include "bt_test.h"

#include "tra_queue.h"

#include "hc_const.h"
#include "hc_event_gen.h"

#include "lmp_scan.h"
#include "lmp_acl_container.h"
#include "lmp_sco_container.h"
#include "lmp_config.h"
#include "lmp_ch.h"
#include "lmp_link_policy.h"
#include "hw_memcpy.h"

#ifndef __USE_INLINES__
#include "bt_test_impl.h"
#endif
#include "uslc_testmode.h"

extern t_tm_control volatile testmode_ctrl;
/* 
 * Module variables (non-static since inline access)
 */
t_loopback_info BTtst_loopback_info;
t_dut_mode      BTtst_dut_mode;

#if (PRH_BS_CFG_SYS_LOOPBACK_VIA_HCI_SUPPORTED == 1)

/*
 * Helper functions
 */
static t_error _BTtst_Enter_Local_Loopback(void);
static t_error _BTtst_Exit_Local_Loopback(void);
static void _BTtst_HCI_Callback_On_Data_Remote_Loopback(t_deviceIndex device_index);
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
static void _BTtst_HCI_Callback_On_SCO_Remote_Loopback(t_deviceIndex sco_index);
#endif



/**************************************************************************
 *
 *  FUNCTION : BTtst_Write_Loopback_Mode
 *
 *  SCOPE : Global
 *
 *  INPUTS :  
 *     t_loopback_mode - New value of the loopback mode     
 * 
 *  OUTPUTS : - NONE
 *
 *  RETURNS : - t_error 
 *
 *  DESCRIPTION :
 *  Writes a value of the loopback mode. Actions performed are dependend on
 *  on the previous loopback mode.
 *   
 **************************************************************************/
t_error BTtst_Write_Loopback_Mode(t_loopback_mode loop_back_mode)
{
    t_error status = NO_ERROR;

    switch(loop_back_mode)
    {
    case LOCAL_LOOPBACK :
        /*
         * Enter Local Loopback only if currently in No loopback
         */
        if (BTtst_loopback_info.loopback_mode==NO_LOOPBACK)
        {
            if (g_LM_config_info.num_acl_links != 0)
            {
                 return COMMAND_DISALLOWED;
            }

            /* 
             * Assign handles and send the Connection Complete events
             */
            status = _BTtst_Enter_Local_Loopback();       
            BTtst_loopback_info.loopback_mode = LOCAL_LOOPBACK; 
        }
        else
        {
            status = COMMAND_DISALLOWED;
        }
        break;
                
    case NO_LOOPBACK :
        /*
         * Always accept NO loopback request
         */
        if (BTtst_loopback_info.loopback_mode==LOCAL_LOOPBACK)
        {
            /*
             * Handle exit of local loopback (returning disconnects)
             */
            status = _BTtst_Exit_Local_Loopback();
        }
        BTtst_loopback_info.loopback_mode = NO_LOOPBACK;
        break;
        

    case REMOTE_LOOPBACK :
        /*
         * Enter Remote Loopback only if currently in No loopback 
         */
        if (BTtst_loopback_info.loopback_mode==NO_LOOPBACK)
        {
            BTtst_loopback_info.loopback_mode = REMOTE_LOOPBACK; 
       // Have to change the voice settings for any SYNC links 
	   // to SCO_VIA_HCI

			{
			// Can only have one ACL active so should be easy to find p-link
            // p_link is not actually used ! so set to zero.

			u_int8 sco_index;
			for (sco_index=0;sco_index<3;sco_index++)
			LMpol_Init_SCO(((t_lmp_link*)/*p_link*/0),sco_index);
			}

        }
        else
        {
            status = COMMAND_DISALLOWED;            
        }
        break;
   }

   return status;
}

#endif

#if (PRH_BS_CFG_SYS_LOOPBACK_VIA_HCI_SUPPORTED == 1)

/**************************************************************************
 *
 *  FUNCTION : _BTtst_Exit_Local_Loopback
 *
 *  SCOPE : local
 *
 *  INPUTS :  none
 *     
 *  OUTPUTS : - NONE
 *
 *  RETURNS : - t_error     
 *
 *  DESCRIPTION :
 *  Exits loopback mode for the current device. Sends four Disconnection 
 *  complete events accross the HCI and deallocates the connection handle.
 *   
 **************************************************************************/
t_error _BTtst_Exit_Local_Loopback(void)
{
    t_lm_event_info event_info;
    t_connectionHandle connection_handle;
    
    /*
     * ACL index is 0,  SCO indices if SCOs over HCI supported are 1..3
     */
    u_int8 handle_index = 1 /*ACL*/ + PRH_BS_CFG_SYS_MAX_SYN_LINKS_VIA_HCI
            * (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED == 1);

    /*
     * For each handle remove in reverse order (ACL last)
     */
    event_info.status = NO_ERROR;
    event_info.reason = LOCAL_TERMINATED_CONNECTION; 
    do 
    {
        handle_index--;

        connection_handle = BTtst_loopback_info.connection_handle[handle_index];

        if(handle_index == 0)
        {
            /*
             * Release ACL Connection (last to be released) if it exists
             */
            LMaclctr_Free_Link(LMaclctr_Find_Handle(connection_handle));

            event_info.handle = connection_handle;
            HCeg_Generate_Event(HCI_DISCONNECTION_COMPLETE_EVENT,&event_info);
        }
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
        else
        {
            /*
             * Disconnect SCO and generate HCI Event if SCO exists
             */
            LMconnection_LM_Disconnect_SCO(
                LMscoctr_Find_SCO_By_Connection_Handle(connection_handle), 
                LOCAL_TERMINATED_CONNECTION);

            /*
             * Disconnecting a SCO may allow the IUT to adjust
             * the ACL packet types on all ACL links.
             */
            LMpol_Adjust_ACL_Packet_Types_On_All_Links(LMcontxt_LEAVING_SCO);
        }
#endif
    }
    while (handle_index != 0);
 
    LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_LOOPBACK_MODE);
    return NO_ERROR;
}

#endif

#if (PRH_BS_CFG_SYS_LOOPBACK_VIA_HCI_SUPPORTED == 1)

/**************************************************************************
 *
 *  FUNCTION : _BTtst_Enter_Local_Loopback
 *
 *  SCOPE : local
 *
 *  INPUTS :  none
 *     
 *  OUTPUTS : - NONE
 *
 *  RETURNS : - t_error     
 *
 *  DESCRIPTION :
 *  Enters loopback mode for the current device. Sends four Connection complete
 *  events accross the HCI and allocates 4 connection handles.
 *
 **************************************************************************/
t_error _BTtst_Enter_Local_Loopback(void)
{
    t_connectionHandle connection_handle;
    t_lm_event_info event_info;
    t_lmp_link *p_link=0;

    /*
     * ACL index is 0,  SCO indices if SCOs over HCI supported are 1..3
     */
    const u_int8 num_handle_index = 1 /*ACL*/ + PRH_BS_CFG_SYS_MAX_SYN_LINKS_VIA_HCI
        * (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1);
    u_int8 handle_index;

    
    LMconfig_LM_Set_Device_State(PRH_DEV_STATE_LOOPBACK_MODE);
    event_info.p_bd_addr = (t_bd_addr*) SYSconfig_Get_Local_BD_Addr_Ref();
    event_info.mode = ENCRYPT_NONE;
    event_info.status = NO_ERROR;

    for (handle_index = 0; handle_index < num_handle_index; handle_index++)
    {
        connection_handle = 0;
        if (handle_index==0)
        {
            /*
             * ACL Handle (last to be released)
             */
            event_info.link_type = ACL_LINK;

            p_link = LMaclctr_Alloc_Link();
            if(p_link != 0)
            {
                connection_handle = p_link->handle;
                event_info.handle = connection_handle;
                /*
                 * Force state to LMP_ACTIVE to allow p_link lookup from tra_hcit.c
                 */         
                p_link->state = LMP_ACTIVE;
            } 
            else
            {
                event_info.status = UNSPECIFIED_ERROR;
            }
        }
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
        else if (p_link)    //- LC note here!! .
        {
            t_sco_info* p_sco_link;
            /*
             * SCO Handles are 1..3
             */
            event_info.link_type = SCO_LINK;

            p_sco_link = LMscoctr_Alloc(p_link->device_index, SCO_CFG_VIA_ACL, SCO_LINK);
            if(p_sco_link != 0) 
            {
                connection_handle = p_sco_link->connection_handle;
                event_info.handle = connection_handle;
                event_info.ack_required = 0x00;
            }
            else
            {
                event_info.status = MAX_NUM_SCO;
            }
        }
#endif
        
        HCeg_Generate_Event(HCI_CONNECTION_COMPLETE_EVENT, &event_info);
        BTtst_loopback_info.connection_handle[handle_index] = connection_handle;
    }
    return NO_ERROR;
}

#endif

#if (PRH_BS_CFG_SYS_LOOPBACK_VIA_HCI_SUPPORTED == 1)

/**************************************************************************
 *
 *  FUNCTION : BTtst_Handle_Loopback_HCI_Data
 *
 *  SCOPE :    Global
 *
 *  INPUTS :   NONE
 *     
 *  OUTPUTS :  NONE
 *
 *  RETURNS : - void     
 *
 *  DESCRIPTION :
 *  This function moves ACL and SCO data between input and output queues 
 *  for both local and remote loopback.
 *
 *  For each device link, move any pending input L2CAP data to 
 *  the corresponding output L2CAP data queue.  
 *  Broadcast queues are ignored.
 *
 *  For each SCO over the HCI queue, move any pending input SCO data to
 *  the corresponding output SCO data queue.
 *
 *  Normally the input/output queues are equivalent and 
 *  support equal packet sizes.
 *  Asymmetric queues where the data OUT and IN queue buffers are
 *  different sizes are also supported (B1893).
 *
 **************************************************************************/
void BTtst_Handle_Loopback_HCI_Data(void)
{
    /*
     * Define the ACL and SCO loopback parameters
     * Allows the re-use of code below at expense of data structure below
     */
    struct q_loopback {
        u_int8 bb_data_out_q_type;      /* Baseband Data Output Q           */
        u_int8 bb_data_in_q_type;       /* Baseband Data Input Q            */
        u_int8 start_queue;             /* The queue to start processing    */
        u_int8 stop_queue;              /* The queue to finish processing   */
        u_int16 bb_data_out_q_length;   /* The buffer length on output q  */
        u_int16 bb_data_in_q_length;    /* The buffer length on input q  */
        void (*remote_callback_fp)(t_deviceIndex);
    };

    /*
     * L2CAP (and HCI SCO if supported) loopback description(s)
     */
    const struct q_loopback loopback[] = 
    {
        {L2CAP_OUT_Q, L2CAP_IN_Q, 1, MAX_ACTIVE_DEVICE_LINKS,
            PRH_BS_CFG_SYS_ACL_OUT_PACKET_LENGTH,
            PRH_BS_CFG_SYS_ACL_IN_PACKET_LENGTH,
            _BTtst_HCI_Callback_On_Data_Remote_Loopback},

#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
        {SCO_OUT_Q,   SCO_IN_Q,   0, PRH_BS_CFG_SYS_MAX_SYN_LINKS_VIA_HCI,
            PRH_BS_CFG_SYS_SCO_OUT_PACKET_LENGTH,
            PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH,
            _BTtst_HCI_Callback_On_SCO_Remote_Loopback}
#endif           
    };

    const struct q_loopback *p_loopback;

    t_loopback_mode loopback_mode;

    loopback_mode = BTtst_loopback_info.loopback_mode;

    if (loopback_mode != NO_LOOPBACK)
    {
        u_int8 in_q_type = 0, out_q_type = 0;
        u_int16 out_q_buffer_length = 0;
        t_deviceIndex queue_index;

        for (p_loopback = loopback;
             p_loopback < loopback + sizeof(loopback)/sizeof(loopback[0]);
             p_loopback++)
        {
            /*
             *    If loopback is local loopback Then
             *        Move L2CAP/SCO_OUT_Q packets to L2CAP/SCO_IN_Q
             *    Else if loopback is remote loopback Then
             *        Move L2CAP/SCO_IN_Q packets to L2CAP/SCO_OUT_Q
             *    Endif
             */
            if (loopback_mode == LOCAL_LOOPBACK)
            {
                in_q_type = p_loopback->bb_data_out_q_type; 
                out_q_type = p_loopback->bb_data_in_q_type;
                out_q_buffer_length = p_loopback->bb_data_in_q_length;
                if (out_q_buffer_length < p_loopback->bb_data_out_q_length)
                {
                    /*
                     * Ensure length word32 aligned to ensure no endianess swap issues.
                     */
                    out_q_buffer_length &= 0xFFFC;
                }
            }
            else if (loopback_mode == REMOTE_LOOPBACK)
            {
                in_q_type = p_loopback->bb_data_in_q_type; 
                out_q_type = p_loopback->bb_data_out_q_type;
                out_q_buffer_length = p_loopback->bb_data_out_q_length;
                if (out_q_buffer_length < p_loopback->bb_data_in_q_length)
                {
                    /*
                     * Ensure length word32 aligned to ensure no endianess swap issues.
                     */
                    out_q_buffer_length &= 0xFFFC;
                }
            }

            /*
             * Process only potential queues (not broadcast qeueus)
             */
            for (queue_index = p_loopback->start_queue; 
                          queue_index < p_loopback->stop_queue; queue_index++)
            {
                t_q_descr *tmp_qd_src;
                t_q_descr *tmp_qd_dest;

                /*
                 * Segment to loopback queue if necessary.
                 * Note cannot re-enter dequeue next without ACK
                 */
                if (BTq_Is_Queue_Not_Full(out_q_type, queue_index))
                {
                    tmp_qd_src = BTq_Dequeue_Next_Data_By_Length(in_q_type,
                        queue_index, out_q_buffer_length );

                    if (tmp_qd_src != 0)
                    {
                        t_length length = tmp_qd_src->length;

                        tmp_qd_dest = BTq_Enqueue(out_q_type, queue_index, length);
                        if (tmp_qd_dest != 0)
                        {
                            /*
                             * Use the faster complete word copies and align
                             * to ensure endianness issues are resolved
                             */
                            hw_memcpy32_transparent(tmp_qd_dest->data,
                                tmp_qd_src->data, length);

                            tmp_qd_dest->message_type = tmp_qd_src->message_type;

                            BTq_Ack_Last_Dequeued(in_q_type, queue_index, length);
                            if (loopback_mode == REMOTE_LOOPBACK)
                            {
                                /*
                                 * Ensure that no flow control events are sent
                                 *
                                 * For L2CAP_IN_Q/SCO_IN_Q reset queue counters now
                                 * For L2CAP_OUT_Q/SCO_OUT_Q reset queue counters via callback
                                 */
                                BTq_Get_and_Reset_Queue_Number_Of_Processed_Chunks(
                                    in_q_type, queue_index);

#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
                                if(in_q_type != SCO_IN_Q)
                                {
                                    tmp_qd_dest->callback = loopback[0].remote_callback_fp;
                                }
                                else
#endif
                                {
                                    tmp_qd_dest->callback = loopback[1].remote_callback_fp;
                                }
                            }
                            BTq_Commit(out_q_type, queue_index);  
                        } /* if (tmp_qd_dest != 0) */
                    } /* if (tmp_qd_src != 0) */
                } /* BTq_Is_Queue_Not_Full */
            } /* for (queue_index */
        } /* for (p_loopback */   
    } /*if (loopback_mode != NO_LOOPBACK)*/
}

#endif


#if (PRH_BS_CFG_SYS_LOOPBACK_VIA_HCI_SUPPORTED == 1)

/**************************************************************************
 *
 *  FUNCTION : _BTtst_HCI_Callback_On_Data_Remote_Loopback
 *
 *  SCOPE :    Local
 *
 *  INPUTS :   device index for active remote loopback device
 *     
 *  OUTPUTS :  NONE
 *
 *  RETURNS : - void     
 *
 *  DESCRIPTION :
 *  Ensure that the flow control event (HCI_Number_Completed_Packets_Event)
 *  is not generated when in Remote Loopback.
 * 
 *  Addresses Bug 1264 (Flow control events in remote loopback)
 **************************************************************************/
static void _BTtst_HCI_Callback_On_Data_Remote_Loopback(t_deviceIndex device_index)
{
    BTq_Get_and_Reset_Queue_Number_Of_Processed_Chunks(
        L2CAP_OUT_Q, device_index);
}

#endif


#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
#if (PRH_BS_CFG_SYS_LOOPBACK_VIA_HCI_SUPPORTED == 1)
/**************************************************************************
 *
 *  FUNCTION : _BTtst_HCI_Callback_On_SCO_Remote_Loopback
 *
 *  SCOPE :    Local
 *
 *  INPUTS :   device index for active remote loopback device
 *     
 *  OUTPUTS :  NONE
 *
 *  RETURNS : - void     
 *
 *  DESCRIPTION :
 *  Ensure that the flow control event (HCI_Number_Completed_Packets_Event)
 *  is not generated when in Remote Loopback.
 * 
 *  Addresses Bug 1264 (Flow control events in remote loopback)
 **************************************************************************/
static void _BTtst_HCI_Callback_On_SCO_Remote_Loopback(t_deviceIndex sco_index)
{
    BTq_Get_and_Reset_Queue_Number_Of_Processed_Chunks(
        SCO_OUT_Q, sco_index);
}

#endif
#endif

#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
u_int32 BTtst_Get_Allowed_Packets(void)
{

	u_int32 allowed_packet;

	switch(testmode_ctrl.packetType)
	{
	case EDR_2DH1 :
        allowed_packet = EDR_2DH1_BIT_MASK;
		break;

	case EDR_3DH1 :
        allowed_packet = EDR_3DH1_BIT_MASK;
		break;

	case EDR_2DH3 :
        allowed_packet = EDR_2DH3_BIT_MASK;
		break;

	case EDR_3DH3 :
        allowed_packet = EDR_3DH3_BIT_MASK;
		break;

	case EDR_2DH5 :
        allowed_packet = EDR_2DH5_BIT_MASK;
		break;

	case EDR_3DH5 :
        allowed_packet = EDR_3DH5_BIT_MASK;
		break;

	default :
	    allowed_packet = (1UL<<testmode_ctrl.packetType);
		break;

	}

	return allowed_packet;
}


boolean BTtst_Loopback_SCO_Enabled() 
{

	if (BTtst_Get_DUT_Mode() != DUT_ACTIVE_LOOPBACK)
		return FALSE;

	if (is_SYN_Packet(testmode_ctrl.packetType))
	{
		return TRUE;
	}

	return FALSE;

}


t_packet BTtst_Get_Loopback_SCO_Packet()
{
	return testmode_ctrl.packetType;
}

u_int16  BTtst_Get_Loopback_Packet_Length()
{
	if (testmode_ctrl.packetLength)
		return testmode_ctrl.packetLength;
	else 
	{
		return SYSconst_Get_Packet_Length(testmode_ctrl.packetType);
	}
}

t_packet BTtst_Get_Loopback_Packet()
{
	return testmode_ctrl.packetType;
}

#endif

