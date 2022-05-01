/***********************************************************************
 *
 * MODULE NAME:    lmp_encode_pdu.c
 * PROJECT CODE:    BlueStream
 * MAINTAINER:     Gary Fleming
 * CREATION DATE:  18 February 2000
 *
 * SOURCE CONTROL: $Id: lmp_encode_pdu.c,v 1.135 2011/09/22 15:46:01 garyf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 *
 * DESCRIPTION: 
 * This module is responsible for encoding outgoing LMP PDUs and placing 
 * them on the outgoing Queues for later transmission by the LC.      
 ***********************************************************************/
#include "sys_config.h"

#include "lmp_config.h"
#include "lmp_ch.h"
#include "lmp_encode_pdu.h"
#include "lmp_acl_container.h" 
#include "lmp_link_control_protocol.h"
#include "lmp_link_policy_protocol.h"
#include "lmp_link_power_control.h"
#include "lmp_link_qos.h"
#include "lmp_utils.h"
#include "lmp_sec_core.h"
#include "lmp_timer.h"
#include "lmp_cmd_disp.h"
#include "lmp_afh.h"
#include "lmp_sec_peer.h"
#include "hc_event_gen.h"
#include "tc_event_gen.h"
#include "tra_queue.h"
#include "uslc_testmode.h"

/*
 * LMP encoding processing flags for Broadcast, Timed operation.
 */ 
#define LMP_CMD_NOFLAGS                 0x00
#define LMP_CMD_BCAST                   0x01
#define LMP_CMD_TIMED                   0x02
#define LMP_CMD_SET_OPERATION_PENDING   0x04
#define LMP_CMD_CLR_OPERATION_PENDING   0x08
#define LMP_CMD_FULLY_ENCODED_PDU       0x10

typedef struct s_lm_flags
{
    u_int8 length;                      /* Length including opcode byte(s)  */
    u_int8 process_flags;               /* See above                        */
} t_lm_flags;

const t_lm_flags _LM_Encode_lmp_command[0x43] = {
/* NO OPERATION                    00*/ { 0x00, LMP_CMD_NOFLAGS},               
/* LMP_NAME_REQ                    01*/ { 0x02, LMP_CMD_TIMED},                 
/* LMP_NAME_RES                    02*/ { 0x11, LMP_CMD_NOFLAGS},               
/* LMP_ACCEPTED                    03*/ { 0x02, LMP_CMD_CLR_OPERATION_PENDING}, 
/* LMP_NOT_ACCEPTED                04*/ { 0x03, LMP_CMD_CLR_OPERATION_PENDING}, 
/* LMP_CLKOFFSET_REQ               05*/ { 0x01, LMP_CMD_TIMED},                 
/* LMP_CLKOFFSET_RES               06*/ { 0x03, LMP_CMD_NOFLAGS},               
/* LMP_DETACH                      07*/ { 0x02, LMP_CMD_SET_OPERATION_PENDING},
/* LMP_IN_RAND                     08*/ { 0x11, LMP_CMD_NOFLAGS},               
/* LMP_COMB_KEY                    09*/ { 0x11, LMP_CMD_NOFLAGS},               
/* LMP_UNIT_KEY                    10*/ { 0x11, LMP_CMD_NOFLAGS}, 
/* LMP_AU_RAND                     11*/ { 0x11, LMP_CMD_NOFLAGS}, 
/* LMP_SRES                        12*/ { 0x05, LMP_CMD_NOFLAGS}, 
/* LMP_TEMP_RAND                   13*/ { 0x11, LMP_CMD_NOFLAGS}, 
/* LMP_TEMP_KEY                    14*/ { 0x11, LMP_CMD_NOFLAGS}, 
/* LMP_ENCRYPTION_MODE_REQ         15*/ { 0x02, LMP_CMD_NOFLAGS}, 

/* LMP_ENCRYPTION_KEY_SIZE_REQ     16*/ { 0x02, LMP_CMD_NOFLAGS}, 
/* LMP_START_ENCRYPTION_REQ        17*/ { 0x11, LMP_CMD_TIMED}, 
/* LMP_STOP_ENCRYPTION_REQ         18*/ { 0x01, LMP_CMD_NOFLAGS},
/* LMP_SWITCH_REQ                  19*/ { 0x05, LMP_CMD_SET_OPERATION_PENDING}, 
/* LMP_HOLD                        20*/ { 0x07, LMP_CMD_NOFLAGS},
/* LMP_HOLD_REQ                    21*/ { 0x07, LMP_CMD_TIMED}, 
/* LMP_SNIFF                       22*/ { 0x0A, LMP_CMD_NOFLAGS}, 
/* LMP_SNIFF_REQ                   23*/ { 0x0A, LMP_CMD_TIMED}, 
/* LMP_UNSNIFF_REQ                 24*/ { 0x01, LMP_CMD_TIMED}, 
/* LMP_PARK_REQ                    25*/ { 0x11, LMP_CMD_TIMED}, 
/* LMP_PARK                        26*/ { 0x11, LMP_CMD_NOFLAGS}, 
/* LMP_SET_BROADCAST_SCAN_WINDOW   27*/ { 0xFF, LMP_CMD_BCAST}, 
/* LMP_MODIFY_BEACON               28*/ { 0xFF, LMP_CMD_BCAST}, 
/* LMP_UNPARK_BD_ADDR_REQ          29*/ { 0xFF, LMP_CMD_BCAST}, 
/* LMP_UNPARK_PM_ADDR_REQ          30*/ { 0xFF, LMP_CMD_BCAST}, 
/* LMP_INCR_POWER_REQ              31*/ { 0x02, LMP_CMD_NOFLAGS}, 

/* LMP_DECR_POWER_REQ              32*/ { 0x02, LMP_CMD_NOFLAGS}, 
/* LMP_MAX_POWER                   33*/ { 0x01, LMP_CMD_NOFLAGS}, 
/* LMP_MIN_POWER                   34*/ { 0x01, LMP_CMD_NOFLAGS}, 
/* LMP_AUTO_RATE                   35*/ { 0x01, LMP_CMD_NOFLAGS}, 
/* LMP_PREFERRED_RATE              36*/ { 0x02, LMP_CMD_NOFLAGS}, 
/* LMP_VERSION_REQ                 37*/ { 0x06, LMP_CMD_TIMED}, 
/* LMP_VERSION_RES                 38*/ { 0x06, LMP_CMD_NOFLAGS}, 
/* LMP_FEATURES_REQ                39*/ { 0x09, LMP_CMD_TIMED}, 
/* LMP_FEATURES_RES                40*/ { 0x09, LMP_CMD_NOFLAGS}, 
/* LMP_QUALITY_OF_SERVICE          41*/ { 0x04, LMP_CMD_NOFLAGS}, 
/* LMP_QUALITY_OF_SERVICE_REQ      42*/ { 0x04, LMP_CMD_TIMED}, 
/* LMP_SCO_LINK_REQ                43*/ { 0x07, LMP_CMD_TIMED}, 
/* LMP_REMOVE_SCO_LINK_REQ         44*/ { 0x03, LMP_CMD_TIMED}, 
/* LMP_MAX_SLOT                    45*/ { 0x02, LMP_CMD_NOFLAGS}, 
/* LMP_MAX_SLOT_REQ                46*/ { 0x02, LMP_CMD_TIMED}, 
/* LMP_TIMING_ACCURACY_REQ         47*/ { 0x01, LMP_CMD_TIMED}, 

/* LMP_TIMING_ACCURACY_RES         48*/ { 0x03, LMP_CMD_NOFLAGS}, 
/* LMP_SETUP_COMPLETE              49*/ { 0x01, LMP_CMD_NOFLAGS},
/* LMP_USE_SEMI_PERMANENT_KEY      50*/ { 0x01, LMP_CMD_NOFLAGS}, 
/* LMP_HOST_CONNECTION_REQ         51*/ { 0x01, LMP_CMD_TIMED}, 
/* LMP_SLOT_OFFSET                 52*/ { 0x09, LMP_CMD_NOFLAGS}, 
/* LMP_PAGE_MODE_REQ               53*/ { 0x03, LMP_CMD_TIMED},                   
/* LMP_PAGE_SCAN_MODE_REQ          54*/ { 0x03, LMP_CMD_TIMED}, 
/* LMP_SUPERVISION_TIMEOUT         55*/ { 0x03, LMP_CMD_NOFLAGS}, 
/* LMP_TEST_ACTIVATE               56*/ { 0x01, LMP_CMD_NOFLAGS}, 
/* LMP_TEST_CONTROL                57*/ { 0x0A, LMP_CMD_NOFLAGS}, 
/* LMP_ENCRYPTION_KEY_SIZE_MASK_REQ58*/ { 0x01, LMP_CMD_TIMED}, 
/* LMP_ENCRYPTION_KEY_SIZE_MASK_RES59*/ { 0x03, LMP_CMD_NOFLAGS}, 
/* LMP_SET_AFH                     60*/ { 0x10, LMP_CMD_FULLY_ENCODED_PDU}, 
/* LMP_ENCAPSULATED_HEADER         61*/ { 0x04, LMP_CMD_NOFLAGS},
/* LMP_ENCAPSULATED_PAYLOAD        62*/ { 0x11, LMP_CMD_NOFLAGS},
/* LMP_SIMPLE_PAIRING_CONFIRM      63*/ { 0x11, LMP_CMD_NOFLAGS},
/* LMP_SIMPLE_PAIRING_NUMBER       64*/ { 0x11, LMP_CMD_NOFLAGS},
/* LMP_DHKEY_CHECK                 65*/ { 0x11, LMP_CMD_NOFLAGS},
 };

#if PRH_BS_CFG_SYS_HCI_V12_FUNCTIONALITY_SUPPORTED
/*****************************************************************************
 *
 * Extended opcode support, for now only escape code 4 (127) required
 *
 *****************************************************************************/
const t_lm_flags _LM_Encode_lmp_command_escape_opcode_4[] = {
/* NO OPERATION                    00*/ { 0x00, LMP_CMD_NOFLAGS},               
/* LMP_Accepted_Ext                01*/ { 0x04, LMP_CMD_NOFLAGS},                 
/* LMP_Not_Accepted_Ext            02*/ { 0x05, LMP_CMD_NOFLAGS},               
/* LMP_Features_Req_Ext            03*/ { 0x0C, LMP_CMD_TIMED | LMP_CMD_FULLY_ENCODED_PDU}, 
/* LMP_Features_Res_Ext            04*/ { 0x0C, LMP_CMD_FULLY_ENCODED_PDU}, 
/* Not Supported                   05*/ { 0x00, LMP_CMD_NOFLAGS},                 
/* Not Supported                   06*/ { 0x00, LMP_CMD_NOFLAGS},               
/* Not Supported                   07*/ { 0x00, LMP_CMD_NOFLAGS},
/* Not Supported                   08*/ { 0x00, LMP_CMD_NOFLAGS},               
/* Not Supported                   09*/ { 0x00, LMP_CMD_NOFLAGS},               
/* Not Supported                   10*/ { 0x00, LMP_CMD_NOFLAGS}, 
/* LMP_Packet_Type_Table_Req       11*/ { 0x03, LMP_CMD_TIMED | LMP_CMD_FULLY_ENCODED_PDU}, 
/* LMP_eSCO_Link_req               12*/ { 0x10, LMP_CMD_TIMED}, 
/* LMP_Remove_eSCO_Link_Req        13*/ { 0x04, LMP_CMD_TIMED}, 
/* Not Supported                   14*/ { 0x00, LMP_CMD_NOFLAGS}, 
/* Not Supported                   15*/ { 0x00, LMP_CMD_NOFLAGS}, 
/* LMP_Channel_Classification_Req  16*/ { 0x07, LMP_CMD_FULLY_ENCODED_PDU}, 
/* LMP_Channel_Classification      17*/ { 0x0C, LMP_CMD_FULLY_ENCODED_PDU},
/* Not Supported                   18*/ { 0x00, LMP_CMD_NOFLAGS},
/* Not Supported                   19*/ { 0x00, LMP_CMD_NOFLAGS},               
/* Not Supported                   20*/ { 0x00, LMP_CMD_NOFLAGS},               
/* LMP_Sniff_Subrating_Req         21*/ { 0x09, LMP_CMD_TIMED | LMP_CMD_FULLY_ENCODED_PDU},
/* LMP_Sniff_Subrating_Res         22*/ { 0x09, LMP_CMD_FULLY_ENCODED_PDU},
/* LMP_Pause_Encryption_Req        23*/ { 0x02, LMP_CMD_TIMED | LMP_CMD_FULLY_ENCODED_PDU},
/* LMP_Resume_Encryption_Req       24*/ { 0x02, LMP_CMD_FULLY_ENCODED_PDU},

/* LMP_IO_Capability_Req           25*/ { 0x05, LMP_CMD_FULLY_ENCODED_PDU},
/* LMP_IO_Capability_Res           26*/ { 0x05, LMP_CMD_FULLY_ENCODED_PDU},
/* LMP_NUMERIC_COMPARISON_FAILED   27*/ { 0x02, LMP_CMD_FULLY_ENCODED_PDU},
/* LMP_Passkey_Entry_Failed        28*/ { 0x02, LMP_CMD_FULLY_ENCODED_PDU},
/* LMP_OOB_Failed                  29*/ { 0x02, LMP_CMD_FULLY_ENCODED_PDU},
/* LMP_KEYPRESS_NOTIFICATION       30*/ { 0x03, LMP_CMD_FULLY_ENCODED_PDU},
/* LMP_Power_Control_Req           31*/ { 0x03, LMP_CMD_FULLY_ENCODED_PDU},               
/* LMP_Power_Control_Resp          32*/ { 0x03, LMP_CMD_FULLY_ENCODED_PDU}
};
static void _LM_Encode_Extended_PDU(
    t_p_pdu p_pdu, t_lmp_pdu_info *pdu_info, u_int8 pdu_length);
#endif
static void _LM_Encode_Normal_PDU(
    t_p_pdu p_pdu, t_lmp_pdu_info *pdu_info, u_int8 pdu_length);

/****************************************************************
 * FUNCTION : _LM_Encode_Set_Name_Fragment
 *
 * DESCRIPTION
 *  Writes a fragment of a name to a location in memory to a
 *  maximum of 14 characters
 ***************************************************************/
static void _LM_Encode_Set_Name_Fragment(t_p_pdu p_pdu, t_lmp_pdu_info* pdu_info)
{
    int i,offset;

    for (offset = 0; ((offset < 14) && (offset < pdu_info->length)); offset++)
    {
        p_pdu[offset] = pdu_info->ptr.p_uint8[offset];
    }
    /* Ensures Zero padding on the name */
    for (i=offset; i < 14; i++)
    {
        p_pdu[i] = 0;
    }
}

/******************************************************
 *  FUNCTION : LM_Encode_LMP_Accepted_PDU
 *
 *  DESCRIPTION :
 *  Send an LMP Accepted message to the respond to a 
 *  request from a peer
 ******************************************************/
void LM_Encode_LMP_Accepted_PDU(t_lmp_link *p_link, u_int8 opcode)
{
   t_lmp_pdu_info pdu_info;

   pdu_info.tid_role = p_link->current_proc_tid_role;
   pdu_info.opcode = LMP_ACCEPTED;
   pdu_info.return_opcode = opcode ;
   LM_Encode_LMP_PDU(p_link->device_index, &pdu_info);
}         

/******************************************************
 *  FUNCTION : LM_Encode_LMP_Accepted_Ext_PDU
 *
 *  DESCRIPTION :
 *  Send an LMP Accepted_Ext message to the respond to a 
 *  request from a peer
 ******************************************************/
void LM_Encode_LMP_Accepted_Ext_PDU(t_lmp_link *p_link, u_int16 opcode)
{
   t_lmp_pdu_info pdu_info;

   pdu_info.tid_role = p_link->tid_role_last_rcvd_pkt;
   pdu_info.opcode = LMP_ACCEPTED_EXT;
   pdu_info.return_opcode = opcode;

   LM_Encode_LMP_PDU(p_link->device_index, &pdu_info);
}         

/******************************************************
 *  FUNCTION : LM_Encode_LMP_Not_Accepted_PDU
 *
 *  DESCRIPTION :
 *  Send an LMP Not Accepted message to the respond to a
 *  request from a peer
 ******************************************************/
void LM_Encode_LMP_Not_Accepted_PDU(t_lmp_link *p_link, u_int8 opcode, t_error reason)
{
   t_lmp_pdu_info pdu_info;

   pdu_info.tid_role = p_link->current_proc_tid_role;
   pdu_info.opcode = LMP_NOT_ACCEPTED;
   pdu_info.return_opcode = opcode ;
   pdu_info.reason = reason;
   LM_Encode_LMP_PDU(p_link->device_index, &pdu_info);
}     

/******************************************************
 *  FUNCTION : LM_Encode_LMP_Not_Accepted_Ext_PDU
 *
 *  DESCRIPTION :
 *  Send an LMP Not Accepted Ext message to the respond to a
 *  request from a peer
 ******************************************************/
void LM_Encode_LMP_Not_Accepted_Ext_PDU(t_lmp_link *p_link, u_int16 opcode, t_error reason)
{
	t_lmp_pdu_info pdu_info;
	pdu_info.tid_role = p_link->current_proc_tid_role;
	pdu_info.opcode = LMP_NOT_ACCEPTED_EXT;
	pdu_info.return_opcode = opcode;
	pdu_info.reason = reason;

    LM_Encode_LMP_PDU(p_link->device_index, &pdu_info);
}

/****************************************************************
 * FUNCTION : LM_Encode_LMP_PDU
 *
 * DESCRIPTION
 * Sends an LMP PDU by placing it on the outgoing LMP Q.

 * This method is responsible for encoding outgoing LMP PDUs and placing
 * them on the outgoing LMP Queue. There are 3 steps in this process :
 *
 * Firstly, sufficient space is reserved in the outgoing LMP Queue for the
 * link (indicated by device index). The _LM_Encode_lmp_command_len array 
 * is used to determine the buffer space required in the queue. 
 *
 * The LMP PDU is then encoded by writing the elements of the pdu_info into
 * the allocated PDU buffer, and committed on completion.
 *
 * NOTE :- When writing a PDU to the Queue it is also possible to include a
 * function pointer for the function which should be called back when the PDU
 * had been successfully transmitted.
 *
 ***************************************************************/
t_error LM_Encode_LMP_PDU(t_deviceIndex device_index, t_lmp_pdu_info *pdu_info)
{

    t_error    status = NO_ERROR;
    t_q_descr  *qd; 
    t_lmp_link *p_link;

    u_int16    opcode;
    u_int8     length;
    u_int8     lmp_command_flags;
    void (*_LM_Encode_function)(t_p_pdu p_pdu, t_lmp_pdu_info *pdu_info, u_int8 pdu_length);


    opcode = pdu_info->opcode;
    if (opcode < 256)
    {
        SYSdebug_Assert(opcode < mNum_Elements(_LM_Encode_lmp_command));
        lmp_command_flags = _LM_Encode_lmp_command[opcode].process_flags;
        length = _LM_Encode_lmp_command[opcode].length;
        _LM_Encode_function = _LM_Encode_Normal_PDU;
    }
#if PRH_BS_CFG_SYS_HCI_V12_FUNCTIONALITY_SUPPORTED
    else
    {
        /* 
         * Escaped opcode 
         */
        u_int8  extended_opcode = (u_int8) opcode;

        SYSdebug_Assert((opcode>>8) == LMP_ESCAPE_4_OPCODE &&
            extended_opcode < mNum_Elements(_LM_Encode_lmp_command_escape_opcode_4));
        lmp_command_flags = 
            _LM_Encode_lmp_command_escape_opcode_4[extended_opcode].process_flags;
        length = _LM_Encode_lmp_command_escape_opcode_4[extended_opcode].length;
        _LM_Encode_function = _LM_Encode_Extended_PDU;
    }
#endif

    /* 
     * If broadcast device (device_index==0) Then
     *     Ensure that only Broadcast Park Messages can only be sent 
     * Endif
     */
    SYSdebug_Assert ((device_index!=0) || (lmp_command_flags & LMP_CMD_BCAST) );

    /*
     * Determine the lmp message length from the length table
     * If the command length is variable (==0xFF) Then
     *     Calculate length from message parameters (normally Park)
     * Endif
     */
    if (length == 0xFF)
    {
        switch(opcode)
        {
        case LMP_UNPARK_PM_ADDR_REQ :
        case LMP_UNPARK_BD_ADDR_REQ :
            /*
             * Length: 2 bytes for timing control flags and opcode
             *         2 bytes for DB if present
             *         N bytes for the AM_ADDR and PM_ADDR/BD_ADDR
             */
            length = ((pdu_info->Db_present) ? 4 : 2) + pdu_info->number;
            break;
            
         case LMP_SET_BROADCAST_SCAN_WINDOW :
            length = (pdu_info->Db_present) ? 6 : 4;
            break;
            
         case LMP_MODIFY_BEACON :
            length = (pdu_info->Db_present) ? 13 : 11;
            break;
        }
    }   

    /*
     * Request and validate queue space for this LMP message
     */
    qd = BTq_Enqueue(LMP_OUT_Q, device_index, length);
    if (!qd)
    {
        /*
         * Out of queue space
         */

		HCeg_Hardware_Error_Event(PRH_BS_HW_ERROR_OUT_OF_LMP_QUEUE_BUFFERS);
        return MEMORY_FULL;
    }
    else if (!qd->data)
    {
        /*
         * Should never occur, queue buffers have been corrupted
         */
		HCeg_Hardware_Error_Event(PRH_BS_HW_ERROR_LMP_QUEUE_CORRUPTED);
        return UNSPECIFIED_ERROR;
    }

    /*
     * Encode the LMP message opcode 
     */
    qd->message_type = LMP_msg;

    if (lmp_command_flags & LMP_CMD_FULLY_ENCODED_PDU) 
    {
        /*
         * Presented with fully encoded PDU then encode directly
         */
        LMutils_Array_Copy(length, qd->data, pdu_info->ptr.p_uint8);
    }
    else
    {
        /*
         * Call the normal or extended encode functions
         */
        _LM_Encode_function(qd->data, pdu_info, length);
    }

    /*
     * Register any call backs that are required.
     */
    if (opcode == LMP_DETACH)
    {
        qd->callback = LMconnection_LMP_Detach_Ack; /* Set the callback */
    }
    else if (opcode == LMP_START_ENCRYPTION_REQ)
    {
        qd->callback = LMsec_core_Enable_Rx_Encryption; /* Set the callback */
    }
#if (PRH_BS_CFG_SYS_LMP_POWER_CONTROL_SUPPORTED==1)
    else if (opcode ==LMP_INCR_POWER_REQ)
    {
        qd->callback = LMlpctrl_Incr_Power_Req_Ack; /* Set the callback */
    }
    else if (opcode == LMP_DECR_POWER_REQ)
    {
        qd->callback = LMlpctrl_Decr_Power_Req_Ack; /* Set the callback */
    }
#endif
#if (PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_MASTER_SUPPORTED==1)
    else if (opcode == LMP_SET_AFH)
    {
        qd->callback = LMafh_LMP_Set_AFH_Ack; /* Set the callback */
    }
#endif
#if (PRH_BS_CFG_SYS_LMP_AFH_CLASSIFICATION_SLAVE_SUPPORTED==1)
    else if (opcode == LMP_CHANNEL_CLASSIFICATION)
    {
        qd->callback = LMafh_LMP_Channel_Classification_Ack; /* Set the callback */
    }
#endif
#if (PRH_BS_DEV_SUSPEND_SLAVE_CLASSIFICATIONS_SUPPORTED==1)
    else if (opcode == LMP_CHANNEL_CLASSIFICATION_REQ)
    {
        qd->callback = LMafh_LMP_Channel_Classification_Req_Ack; /* Set the callback */
    }
#endif
#if (PRH_BS_DEV_EXTENDED_QOS_NOTIFICATIONS_SUPPORTED==1)
    else if (opcode == LMP_QUALITY_OF_SERVICE)
    {
        qd->callback = LMqos_Clear_LMP_QOS_Notification_Ack_Pending; /* Set the callback */
    }
#endif
    else if (opcode == LMP_SUPERVISION_TIMEOUT)
    {
        qd->callback = LMpolicy_LMP_Supervision_Timeout_Ack;
    }
#if (PRH_BS_DEV_MASTER_PARK_REQ_ACK_TIMER==1)
    else if ((opcode == LMP_PARK_REQ) && (pdu_info->ack_required))
    {
        qd->callback = LMpolicy_LMP_Park_Req_Ack;
    }
#endif
    else if ((opcode == LMP_ACCEPTED) || (opcode == LMP_ACCEPTED_EXT))
    {
        switch(pdu_info->return_opcode)
        {

#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
        case LMP_PARK_REQ :
            qd->callback = LMpolicy_LMP_Park_Accepted_Ack;
            break;

        case LMP_UNPARK_BD_ADDR_REQ :
        case LMP_UNPARK_PM_ADDR_REQ :
            qd->callback = LMpolicy_LMP_Unpark_Pmaddr_Accepted_Ack;
            break;
#endif

#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUPPORTED==1)
        case LMP_SNIFF_REQ :           
            qd->callback = LMpolicy_LMP_Sniff_Req_Accepted_Ack;
            break;
        case LMP_UNSNIFF_REQ :
            qd->callback = LMpolicy_LMP_Unsniff_Req_Accepted_Ack;
            break;
#endif
#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
        case LMP_SCO_LINK_REQ :
            if (pdu_info->ack_required)
            {
                qd->callback = LMconnection_LMP_SCO_Accepted_Ack;
            }
            break;
#endif
#if (PRH_BS_CFG_SYS_TEST_MODE_SUPPORTED==1)
        /* 
         * erratum 2014 - change settings after receiving
         *                LC level ACK for LMP_Accepted 
         */
        case LMP_TEST_CONTROL :
            qd->callback = USLCtm_Testmode_Change_Settings;
            break;
#endif
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
		case LMP_ESCO_LINK_REQ :
			qd->callback = LMconnection_LMP_eSCO_Accepted_Ack;
			break;
#endif
#if (PRH_BS_CFG_SYS_LMP_PAUSE_ENCRYPTION_SUPPORTED==1)
        case LMP_STOP_ENCRYPTION_REQ :
            qd->callback = LMsec_peer_LMP_Stop_Encryption_Req_Accepted_Ack;
            break;
        case LMP_START_ENCRYPTION_REQ :
            qd->callback = LMsec_peer_LMP_Start_Encryption_Req_Accepted_Ack;
            break;
#endif
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
        case LMP_PACKET_TYPE_TABLE_REQ:
            qd->callback = LMconnection_LMP_Packet_Type_Table_Req_Accepted_Ack;
            break;
#endif

#if 1 // GF Temp removed 23 Apr - for release testing
	  // Required !!
#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
		case LMP_REMOVE_SCO_LINK_REQ :
            qd->callback = LMconnection_LMP_Remove_Sco_Link_Accepted_Ack;
			break;
#endif
#endif 
        }
    }

    /*
     * Log this message over the TCI if this feature is active
     */
    TCeg_Send_LMP_Event(qd->data, (u_int8) qd->length, 0 /*Transmitted*/);

    /*
     * If Direct Baseband Copy (Big Endian) Then Change Data to Big Endian
     */
    BTq_Change_Endianness_If_Required(qd->data, qd->length);

    BTq_Commit(LMP_OUT_Q,device_index);

    /*
     * Set the operation pending and a response timer for the LMP message 
     * if appropriate for message and handle
     */
    p_link = LMaclctr_Find_Device_Index(device_index);
    if (p_link)
    {
        if (lmp_command_flags & LMP_CMD_TIMED)
        {
            /* 
             * This timer is cleared on receipt of the corresponding
             * response from the peer.
             * If the timer fires the appropiate event is sent to the host.
             */
            p_link->operation_pending = opcode;
            if(!p_link->msg_timer)
            {
                p_link->msg_timer = LMtmr_Set_Timer(PRH_LMP_MSG_TIMEOUT,
                    LM_Encode_Msg_Timeout, p_link, 1);
            }
        }
        if (lmp_command_flags & LMP_CMD_SET_OPERATION_PENDING)
        {
            /* 
             * No timer, just record operation pending
             */
            p_link->operation_pending = opcode;
        }
        if (lmp_command_flags & LMP_CMD_CLR_OPERATION_PENDING)
        {
			/*
			 * Must somehow take into account the fact that some 
			 * LMP_accepted (LMP_accepted_ext), LMP_not_accepted,
			 * and LMP_not_accepted_ext PDUs stop a transaction
			 * that the local LM needs to guard and some LMP_accepted PDUs 
			 * (et al) stop a transaction that the local LM is not guarding.
			 */
			if((p_link->msg_timer)/* && (p_link->operation_pending == )*/)
			{
				LMtmr_Clear_Timer(p_link->msg_timer);
				p_link->msg_timer = 0;
				p_link->operation_pending = NO_OPERATION;
			}
        }
    }
    return status;
}


/****************************************************************
 * FUNCTION : LM_Encode_LMP_PDU_Fully_Encoded
 *
 * DESCRIPTION
 * Sends the LMP PDU for given opcode where the pdu is
 * fully prepared for sending
 *
 * This supports modules where the LMP PDU is fully encoded
 * before sending the LMP message with the opcode, tid and
 * parameters
 ***************************************************************/
t_error LM_Encode_LMP_PDU_Fully_Encoded(
    t_deviceIndex device_index, u_int16 opcode, const u_int8 *p_pdu)
{
    t_lmp_pdu_info pdu_info;          

    pdu_info.opcode = opcode;
    pdu_info.ptr.p_uint8 = p_pdu;
    return LM_Encode_LMP_PDU(device_index, &pdu_info); 
}

/****************************************************************
 * FUNCTION : LM_Encode_Msg_Timeout
 *
 * DESCRIPTION
 *  This function is invoked when a LMP message timeout fires. 
 *  For each LMP Request send, a Message Timer is set. If a 
 *  response is received from the peer the Message Timer is cleared.
 *  If the message timeout fires then the handling is identical to
 *  the handling of a LMP_Not_Accepted from the peer for the outstanding
 *  operation.
 *
 *  This function can also be called directly when a link is being 
 *  disconnected.
 ***************************************************************/
void LM_Encode_Msg_Timeout(t_lmp_link *p_link)
{
    /*
     * Enqueue an equivalent incoming LMP_not_accepted
     * with reason LMP_RESPONSE_TIMEOUT
     */
    u_int8 lmp_not_accepted_pdu_parameters[3];

    LMtmr_Clear_Timer(p_link->msg_timer);
    p_link->msg_timer = 0;

    /*
     * Update for BT1.2
     */
    if(p_link->operation_pending >= LMP_ESCAPE_1_OPCODE)
    {
        lmp_not_accepted_pdu_parameters[0] = p_link->operation_pending >> 8;
        lmp_not_accepted_pdu_parameters[1] = p_link->operation_pending & 0xff;
        lmp_not_accepted_pdu_parameters[2] = LMP_RESPONSE_TIMEOUT;
        LMdisp_LMP_Not_Accepted_Ext(p_link, lmp_not_accepted_pdu_parameters);
    }
    else
    {
        lmp_not_accepted_pdu_parameters[0] = (u_int8) p_link->operation_pending;
        lmp_not_accepted_pdu_parameters[1] = LMP_RESPONSE_TIMEOUT;

        LMdisp_LMP_Not_Accepted(p_link, lmp_not_accepted_pdu_parameters);
	}
}

/***************************************************************************
 * _LM_Encode_Normal_PDU
 *
 * DESCRIPTION
 *      
 * Encodes the opcode and parameters of a V1.1 lmp message
 * where the opcode is a single byte
 ***************************************************************************/
static void _LM_Encode_Normal_PDU(
    t_p_pdu p_pdu, t_lmp_pdu_info *pdu_info, u_int8 length)
{
    u_int8 opcode = (u_int8) pdu_info->opcode;

    *p_pdu++ = (opcode<<1) + (pdu_info->tid_role!=0);

    switch(opcode)
    {
    case LMP_NOT_ACCEPTED :
        *(p_pdu) = (u_int8)(pdu_info->return_opcode);
        *(p_pdu+1) = pdu_info->reason;
        break;

    case LMP_ACCEPTED :
        *(p_pdu) = (u_int8)(pdu_info->return_opcode);
        break;

    case LMP_DETACH :   
        *p_pdu = pdu_info->reason;
        break;

    case LMP_VERSION_REQ :
    case LMP_VERSION_RES :
        {
            const t_versionInfo *p_local_version =  pdu_info->ptr.p_local_version;
            mLMutils_Set_Uint8(p_pdu, p_local_version->lmp_version);
            LMutils_Set_Uint16(p_pdu+1, p_local_version->comp_id);
            LMutils_Set_Uint16(p_pdu+3, p_local_version->lmp_subversion);
        }
        break;

    case LMP_NAME_REQ :
        mLMutils_Set_Uint8(p_pdu, pdu_info->name_offset);
        break;

    case LMP_NAME_RES :
        mLMutils_Set_Uint8(p_pdu, pdu_info->name_offset);
        mLMutils_Set_Uint8(p_pdu+1, pdu_info->name_length);
        _LM_Encode_Set_Name_Fragment(p_pdu+2, pdu_info); 
        break;

#if (PRH_BS_CFG_SYS_LMP_HOLD_SUPPORTED ==1)
    case LMP_HOLD :
    case LMP_HOLD_REQ :
        LMutils_Set_Uint16(p_pdu, pdu_info->timeout);
        LMutils_Set_Uint32(p_pdu+2, pdu_info->instant);
        break;
#endif
    case LMP_SUPERVISION_TIMEOUT :
        LMutils_Set_Uint16(p_pdu,pdu_info->timeout);
        break;

#if (PRH_BS_CFG_SYS_TEST_MODE_TESTER_SUPPORTED==1)
    case LMP_TEST_CONTROL :
        mLMutils_Set_Uint8(p_pdu++, pdu_info->tm_scenario);
        mLMutils_Set_Uint8(p_pdu++, pdu_info->tm_hop_mode);
        mLMutils_Set_Uint8(p_pdu++, pdu_info->tm_tx_freq);
        mLMutils_Set_Uint8(p_pdu++, pdu_info->tm_rx_freq);
        mLMutils_Set_Uint8(p_pdu++, pdu_info->tm_power_mode);
		//
		// if Loopback Test set the Poll Period = 0
		// and only set the length field if we have eSCO packet
		//
		if (((pdu_info->tm_scenario ^ TESTMODE_WHITEN) > 0x04) &&
			((pdu_info->tm_scenario ^ TESTMODE_WHITEN) < 0x09))
		{

			mLMutils_Set_Uint8(p_pdu++, 0x55/*pdu_info->tm_poll_period*/);
			mLMutils_Set_Uint8(p_pdu++, pdu_info->tm_pkt_type);
			// The Length Field is only set for eSCO packet types ( 0x1X or 0x3X )
			if ((pdu_info->tm_pkt_type ^ TESTMODE_WHITEN) & 0x10) 
			{
				LMutils_Set_Uint16(p_pdu, pdu_info->tm_pkt_len);
			}
			else
			{
				LMutils_Set_Uint16(p_pdu, 0x5555 /* pdu_info->tm_pkt_len*/);
			}
		}
		else
		{
			mLMutils_Set_Uint8(p_pdu++, pdu_info->tm_poll_period);
			mLMutils_Set_Uint8(p_pdu++, pdu_info->tm_pkt_type);
			LMutils_Set_Uint16(p_pdu, pdu_info->tm_pkt_len);

		}

        break;
#endif

#if (PRH_BS_CFG_SYS_QOS_SUPPORTED==1)
    case LMP_QUALITY_OF_SERVICE :
    case LMP_QUALITY_OF_SERVICE_REQ :
        LMutils_Set_Uint16(p_pdu, pdu_info->timeout);
        mLMutils_Set_Uint8(p_pdu+2,pdu_info->Nbc);
        break;
#endif
    case LMP_MAX_SLOT :
    case LMP_MAX_SLOT_REQ :
        mLMutils_Set_Uint8(p_pdu,pdu_info->max_slots);
        break;

    /*
     * For the following the full parameters PDU is available 
     */
    case LMP_FEATURES_REQ : 
    case LMP_FEATURES_RES :
    case LMP_AU_RAND :
    case LMP_IN_RAND : 
    case LMP_COMB_KEY :
    case LMP_UNIT_KEY :
    case LMP_TEMP_RAND :
    case LMP_START_ENCRYPTION_REQ :
    case LMP_TEMP_KEY :
    case LMP_SRES :
        LMutils_Array_Copy((u_int8)(length-1), p_pdu, pdu_info->ptr.p_uint8);
        break;

#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
    case LMP_SCO_LINK_REQ :
        {
            const t_sco_info *p_sco_link = pdu_info->ptr.p_sco_link;
            mLMutils_Set_Uint8(p_pdu++, p_sco_link->lm_sco_handle);
            mLMutils_Set_Uint8(p_pdu++, (u_int8)((p_sco_link->timing_ctrl << 1)));
            mLMutils_Set_Uint8(p_pdu++, p_sco_link->d_sco);
            mLMutils_Set_Uint8(p_pdu++, p_sco_link->t_sco);
            mLMutils_Set_Uint8(p_pdu++, p_sco_link->lm_sco_packet);
            mLMutils_Set_Uint8(p_pdu, p_sco_link->lm_air_mode);
        }
        break;

    case LMP_REMOVE_SCO_LINK_REQ :
        mLMutils_Set_Uint8(p_pdu, pdu_info->ptr.p_sco_link->lm_sco_handle);
        mLMutils_Set_Uint8(p_pdu+1,(u_int8)pdu_info->reason);
        break;
#endif

    /* case LMP_PAGE_MODE_REQ :
    case LMP_PAGE_SCAN_MODE_REQ :
        mLMutils_Set_Uint8(p_pdu, pdu_info->mode);
        mLMutils_Set_Uint8(p_pdu+1, pdu_info->settings);
        break; */

#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUPPORTED==1)
    case LMP_SNIFF_REQ : 
        mLMutils_Set_Uint8(p_pdu, (u_int8)(pdu_info->timing_control << 1));
        LMutils_Set_Uint16(p_pdu+1, pdu_info->offset);
        LMutils_Set_Uint16(p_pdu+3, pdu_info->interval);
        LMutils_Set_Uint16(p_pdu+5, pdu_info->attempt);
        LMutils_Set_Uint16(p_pdu+7, pdu_info->timeout); 
        break;
#endif

    case LMP_ENCRYPTION_KEY_SIZE_REQ :
        mLMutils_Set_Uint8(p_pdu, pdu_info->key_size);
        break;

    case LMP_ENCRYPTION_MODE_REQ :
    case LMP_PREFERRED_RATE : 
        mLMutils_Set_Uint8(p_pdu, pdu_info->mode);
        break;

    case LMP_CLKOFFSET_RES :
        LMutils_Set_Uint16(p_pdu, pdu_info->offset);
        break;

    case LMP_SLOT_OFFSET :
        LMutils_Set_Uint16(p_pdu, pdu_info->slot_offset);
        LMutils_Array_Copy(6, p_pdu+2, pdu_info->ptr.p_uint8);
        break;

#if (PRH_BS_CFG_SYS_LMP_TIMING_INFO_SUPPORTED==1)
    case LMP_TIMING_ACCURACY_RES : 
        mLMutils_Set_Uint8(p_pdu, pdu_info->drift);
        mLMutils_Set_Uint8(p_pdu+1, pdu_info->jitter);        
        break;
#endif


#if (PRH_BS_CFG_SYS_LMP_POWER_CONTROL_SUPPORTED==1)
    case LMP_INCR_POWER_REQ :     
    case LMP_DECR_POWER_REQ :
        mLMutils_Set_Uint8(p_pdu, pdu_info->number);
        break;
#endif

    case LMP_SWITCH_REQ :
        LMutils_Set_Uint32(p_pdu, pdu_info->instant);
        break;

#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)

   case LMP_UNPARK_BD_ADDR_REQ :
   case LMP_UNPARK_PM_ADDR_REQ :
        mLMutils_Set_Uint8(p_pdu++, (u_int8)(pdu_info->timing_control<<1));
        if(pdu_info->Db_present)
        {
            LMutils_Set_Uint16(p_pdu, g_LM_config_info.D_bcast);            
            p_pdu+=2;
        }
        LMutils_Array_Copy(pdu_info->number, p_pdu, pdu_info->ptr.p_uint8);
        break;

   case LMP_SET_BROADCAST_SCAN_WINDOW :
        mLMutils_Set_Uint8(p_pdu++, (u_int8)(pdu_info->timing_control<<1));
        if(pdu_info->Db_present)
        {
            LMutils_Set_Uint16(p_pdu,g_LM_config_info.D_bcast);            
            p_pdu+=2;
        }
        LMutils_Set_Uint16(p_pdu, g_LM_config_info.bcast_scan_ext);
        break;

   case LMP_MODIFY_BEACON :
   case LMP_PARK_REQ : 

        /*
         * Bit 2 in timing control is set to 1 if NO access window 
         */
        mLMutils_Set_Uint8(p_pdu++, 
            (u_int8)((pdu_info->ptr.p_park->timing_ctrl<<1) + 
                    ((g_LM_config_info.M_access == 0)<<2)) );

        if((pdu_info->Db_present) || (opcode == LMP_PARK_REQ))
        {
            LMutils_Set_Uint16(p_pdu, g_LM_config_info.D_bcast);            
            p_pdu+=2;
        }
        LMutils_Set_Uint16(p_pdu, g_LM_config_info.T_bcast);
        p_pdu+=2;
        mLMutils_Set_Uint8(p_pdu++, g_LM_config_info.N_bcast);
        mLMutils_Set_Uint8(p_pdu++, pdu_info->ptr.p_park->delta_bcast);
        if (opcode==LMP_PARK_REQ)
        {
            mLMutils_Set_Uint8(p_pdu++, pdu_info->ptr.p_park->pm_addr);
            mLMutils_Set_Uint8(p_pdu++, pdu_info->ptr.p_park->ar_addr);
            mLMutils_Set_Uint8(p_pdu++, g_LM_config_info.N_sleep);
            mLMutils_Set_Uint8(p_pdu++, g_LM_config_info.D_sleep);
        }
        mLMutils_Set_Uint8(p_pdu++, g_LM_config_info.D_access);
        mLMutils_Set_Uint8(p_pdu++, g_LM_config_info.T_access);
        mLMutils_Set_Uint8(p_pdu++, g_LM_config_info.N_access);
        mLMutils_Set_Uint8(p_pdu++, g_LM_config_info.N_poll);
        mLMutils_Set_Uint8(p_pdu, (u_int8)g_LM_config_info.M_access /* Default Access System = 0 */);
        break;
#endif

#if (PRH_BS_CFG_SYS_LMP_BROADCAST_ENCRYPTION_SUPPORTED==1)
   case LMP_ENCRYPTION_KEY_SIZE_MASK_RES:
       LMutils_Set_Uint16(p_pdu, pdu_info->enc_key_len_mask);
       break;
#endif

   case LMP_ENCAPSULATED_HEADER :
        mLMutils_Set_Uint8(p_pdu++, (u_int8)0x01);
        mLMutils_Set_Uint8(p_pdu++, (u_int8)0x01);
        mLMutils_Set_Uint8(p_pdu++, (u_int8)0x30);
	   break;

   case LMP_ENCAPSULATED_PAYLOAD :
   case LMP_SIMPLE_PAIRING_CONFIRM :
   case LMP_SIMPLE_PAIRING_NUMBER :
   case LMP_DHKEY_CHECK :
       LMutils_Array_Copy(16, p_pdu, pdu_info->ptr.p_uint8);
	   break;

    default :
        /*
         * if PDU with no parameters then 
         *      Ignore, all ok
         * else
         *      Report error
         * endif
         */
        SYSdebug_Assert(length == 1);
        break;
    }
}

/***************************************************************************
 * _LM_Encode_Extended_PDU
 *
 * DESCRIPTION
 *      
 * Encodes the opcode and parameters of a V1.2 extended lmp message
 * where the opcode is two bytes.
 * 
 * Messages where raw PDUs are presented are handled by caller 
 ***************************************************************************/
static void _LM_Encode_Extended_PDU(
    t_p_pdu p_pdu, t_lmp_pdu_info *pdu_info, u_int8 length)
{
    u_int16 opcode = pdu_info->opcode;

    switch(opcode)
    {
	case LMP_ACCEPTED_EXT:
		p_pdu[0] = (pdu_info->opcode >> 8);
		p_pdu[1] = pdu_info->opcode & 0xff;
		p_pdu[2] = pdu_info->return_opcode >> 8;
		p_pdu[3] = pdu_info->return_opcode & 0xff;
		p_pdu[0] <<= 1;
		p_pdu[0] |= pdu_info->tid_role;
		break;
	case LMP_NOT_ACCEPTED_EXT:
		p_pdu[0] = (pdu_info->opcode >> 8);
		p_pdu[1] = pdu_info->opcode & 0xff;
		p_pdu[2] = pdu_info->return_opcode >> 8;
		p_pdu[3] = pdu_info->return_opcode & 0xff;
		p_pdu[4] = pdu_info->reason;
		p_pdu[0] <<= 1;
		p_pdu[0] |= pdu_info->tid_role;
		break;
	case LMP_REMOVE_ESCO_LINK_REQ:
	case LMP_ESCO_LINK_REQ:
    case LMP_CHANNEL_CLASSIFICATION:
    case LMP_CHANNEL_CLASSIFICATION_REQ:
	case LMP_FEATURES_RES_EXT:
    case LMP_SNIFF_SUBRATING_REQ:
    case LMP_SNIFF_SUBRATING_RES:
	case LMP_POWER_CONTROL_RESP:
        /*
         * Just move the full pdu including the opcode
         */
        LMutils_Array_Copy(length, p_pdu, pdu_info->ptr.p_uint8);
        break;

    default:
        break;
    }
}
