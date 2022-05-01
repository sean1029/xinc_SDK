/*************************************************************************
 * MODULE NAME:    hc_event_gen.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    Host Controller Event Generator
 * AUTHOR:         Gary Fleming
 * DATE:           04-07-1999
 * SOURCE CONTROL: $Id: hc_event_gen.c,v 1.179 2014/03/11 03:13:35 garyf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *    V1.0     02-06-1999 -   GF
 *  
 ************************************************************************/

#include "sys_config.h"
#include "hc_const.h"
#include "hc_event_gen.h"
#include "tra_queue.h"
#include "bt_timer.h"
#include "bt_test.h"

#include "lmp_config.h"
#include "lmp_utils.h"
#include "lmp_ch.h"
#include "lmp_acl_container.h"
#include "lmp_inquiry.h"
#include "lmp_sco_container.h"
#include "uslc_inquiry.h"
#include "hw_memcpy.h"
#include "sys_irq.h"
#include "global_val.h"
#include "patch_function_id.h"


/*
 * Define array of corresponding parameter lengths for each HCI event
 */
#define MAX_NUM_HC_EVENTS (1 + 0x3D)
const u_int8 hc_event_param_len[MAX_NUM_HC_EVENTS] =
/*  Event_Mask -:- Parameter Length  */
{  0x00  /* NO_EVENT                                      0x00 */   ,
   0x01  /* INQUIRY_COMPLETE_EVENT                        0x01 */   ,
   0xFF  /* INQUIRY_RESULT_EVENT                          0x02 */   ,
   0x0B  /* CONNECTION_COMPLETE_EVENT                     0x03 */   ,
   0x0A  /* CONNECTION_REQUEST_EVENT                      0x04 */   ,
   0x04  /* DISCONNECTION_COMPLETE_EVENT                  0x05 */   ,
   0x03  /* AUTHENTICATION_COMPLETE_EVENT                 0x06 */   ,
   0xFF  /* REMOTE_NAME_REQUEST_COMPLETE_EVENT            0x07 */   ,
   0x04  /* ENCRYPTION_CHANGE_EVENT                       0x08 */   ,
   0x03  /* CHANGE_CONNECTION_LINK_KEY_COMPLETE_EVENT     0x09 */   ,
   0x04  /* MASTER_LINK_KEY_COMPLETE_EVENT                0x0A */   ,
   0x0B  /* READ_REMOTE_SUPPORTED_FEATURES_COMPLETE_EVENT 0x0B */   ,
   0x08  /* READ_REMOTE_VERSION_INFORMATION_COMPLETE_EVENT x0C */   ,
   0x15  /* QOS_SETUP_COMPLETE_EVENT                      0x0D */   ,
   0xFF  /* COMMAND_COMPLETE_EVENT                        0x0E */   ,
   0x04  /* COMMAND_STATUS_EVENT                          0x0F */   ,
   0x01  /* HARDWARE_ERROR_EVENT                          0x10 */   ,
   0x02  /* FLUSH_OCCURED_EVENT                           0x11 */   ,
   0x08  /* ROLE_CHANGE_EVENT                             0x12 */   ,
   0xFF  /* NUMBER_OF_COMPLETED_PACKETS_EVENT             0x13 */   ,
   0x06  /* MODE_CHANGE_EVENT                             0x14 */   ,
   0x17  /* RETURN_LINK_KEYS_EVENT                        0x15 */   ,
   0x06  /* PIN_CODE_REQUEST_EVENT                        0x16 */   ,
   0x06  /* LINK_KEY_REQUEST_EVENT                        0x17 */   ,
   0x17  /* LINK_KEY_NOTIFICATION_EVENT                   0x18 */   ,
   0xFF  /* LOOPBACK_COMMAND_EVENT                        0x19 */   ,
   0x01  /* DATA_BUFFER_OVERFLOW_EVENT                    0x1A */   ,
   0x03  /* MAX_SLOTS_CHANGE_EVENT                        0x1B */   ,
   0x05  /* READ_CLOCK_OFFSET_COMPLETE_EVENT              0x1C */   ,
   0x05  /* CONNECTION_PACKET_TYPE_CHANGED_EVENT          0x1D */   ,
   0x02  /* QOS_VIOLATION_EVENT                           0x1E */   ,
   0x07  /* PAGE_SCAN_MODE_CHANGE_EVENT (OBSOLETE)        0x1F */   ,
   0x07  /* PAGE_SCAN_REPETITION_MODE_CHANGE_EVENT        0x20 */   ,
   0x16  /* FLOW_SPECIFICATION_COMPLETE_EVENT             0x21 */   ,
   0xFF  /* INQUIRY_RESULT_WITH_RSSI_EVENT                0x22 */   ,
   0x0D  /* READ_REMOTE_EXTENDED_FEATURES_COMPLETE_EVENT  0x23 */   ,
   0x09  /* FIXED_ADDRESS_EVENT                           0x24 */   ,
   0x08  /* ALIAS_ADDRESS_EVENT                           0x25 */   ,
   0x02  /* GENERATE_ALIAS_REQUEST_EVENT                  0x26 */   ,
   0x0C  /* ACTIVE_ADDRESS_EVENT                          0x27 */   ,
   0x02  /* ALLOW_PRIVATE_PAIRING_EVENT                   0x28 */   ,
   0x06  /* ALIAS_ADDRESS_REQUEST_EVENT                   0x29 */   ,
   0x07  /* ALIAS_NOT_RECOGNISED_EVENT                    0x2A */   ,
   0x04  /* FIXED_ADDRESS_ATTEMPT_EVENT                   0x2B */   ,
   0x11  /* SYNCHRONOUS_CONNECTION_COMPLETE_EVENT         0x2C */   ,
   0x09  /* SYNCHRONOUS_CONNECTION_CHANGED_EVENT          0x2D */   ,
   0x0B  /* SNIFF_SUBRATING_EVENT                         0x2E */   ,
   0xFF  /* EXTENDED_INQUIRY_RESULT_EVENT                 0x2F */   ,
   0x03  /* ENCRYPTION_KEY_REFRESH_COMPLETE               0x30 */   ,

   0x06  /* IO_CAPABILITY_REQUEST_EVENT                   0x31 */   ,
   0x09  /* IO_CAPABILITY_RESPONSE_EVENT                  0x32 */   ,
   0x0A  /* USER_CONFIRMATION_REQUEST_EVENT               0x33 */   ,
   0x06  /* USER_PASSKEY_REQUEST_EVENT                    0x34 */   ,
   0x06  /* REMOTE_OOB_REQUEST_EVENT                      0x35 */   ,
   0x07  /* SIMPLE_PAIRING_COMPLETE_EVENT                 0x36 */   ,
   0x00  /* NO EVENT                                      0x37 */   ,
   0x04  /* LINK_SUPERVISION_TIMEOUT_CHANGED_EVENT        0x38 */   ,
   0x02  /* ENHANCED_FLUSH_COMPLETE_EVENT                 0x39 */   ,
   0x00  /* NO EVENT                                      0x3A */   ,
   0x0A  /* USER_PASSKEY_NOTIFICATION_EVENT               0x3B */   ,
   0x07  /* KEYPRESS_NOTIFICATION_EVENT                   0x3C */   ,
   0x0E  /* REMOTE_HOST_SUPPORTED_FEATURES_NOTIFICATION_EVENT     0x3D */  
};            

/*
 * Define structure to hold Command_Complete_Event opcodes and parameter lengths 
 */
typedef struct {
    u_int16 opcode;
    u_int8  para_len;
} t_cmd_complete_event_entry;

#if (PRH_BS_CFG_SYS_LE_CONTROLLER_SUPPORTED==1)
#define MAX_CMD_COMPLETE_ARRAY_SIZE 86
#else
#define MAX_CMD_COMPLETE_ARRAY_SIZE 85
#endif

const t_cmd_complete_event_entry hc_cmd_complete_event_array[MAX_CMD_COMPLETE_ARRAY_SIZE] =
{ { HCI_READ_PAGE_TIMEOUT               , 3 },
  { HCI_READ_CONNECT_ACCEPT_TO          , 3 },
  { HCI_READ_SCAN_ENABLE                , 2 },
  { HCI_READ_AUTHENTICATION_ENABLE      , 2 },
  { HCI_READ_ENCRYPTION_MODE            , 2 },
  { HCI_READ_PIN_TYPE                   , 2 },
  { HCI_READ_PAGE_SCAN_ACT              , 5 },
  { HCI_READ_INQUIRY_SCAN_ACT           , 5 },
  { HCI_READ_LOOPBACK_MODE              , 2 },
  { HCI_READ_BUFFER_SIZE                , 8 },
  { HCI_READ_LOCAL_FEATURES             , 9 },
  { HCI_READ_LOCAL_VER_INFO             , 9 },  /* BF 465 */
  { HCI_READ_BD_ADDR                    , 7 },
  { HCI_LINK_KEY_REQUEST_REPLY          , 7 },
  { HCI_LINK_KEY_REQUEST_NEGATIVE_REPLY , 7 },
  { HCI_PIN_CODE_REQUEST_REPLY          , 7 },
  { HCI_PIN_CODE_REQUEST_NEGATIVE_REPLY , 7 },
  { HCI_READ_CLASS_OF_DEVICE            , 4 },
  { HCI_READ_FAILED_CONTACT_COUNTER     , 5 },
  { HCI_READ_SUPERVISION_TIMEOUT        , 5 },  /* 20 CMDS */
  { HCI_WRITE_SUPERVISION_TIMEOUT       , 3 },
  { HCI_READ_AUTOMATIC_FLUSH_TIMEOUT    , 5 },
  { HCI_WRITE_AUTOMATIC_FLUSH_TIMEOUT   , 3 },
  { HCI_READ_STORED_LINK_KEY            , 5 },
  { HCI_WRITE_STORED_LINK_KEY           , 2 },
  { HCI_DELETE_STORED_LINK_KEY          , 3 },
  { HCI_READ_NUM_BROADCAST_RETRANSMISSIONS, 2},
  { HCI_READ_HOLD_MODE_ACTIVITY         , 2 }, 
  { HCI_READ_LOCAL_NAME                 , 249},//249},
  { HCI_ROLE_DISCOVERY                  , 4  }, /* 30 CMDS */
  { HCI_READ_LINK_POLICY_SETTINGS       , 5 },
  { HCI_WRITE_LINK_POLICY_SETTINGS      , 3 },
  { HCI_READ_PAGE_SCAN_PERIOD_MODE      , 2 },
  { HCI_READ_PAGE_SCAN_MODE             , 2 },
  { HCI_READ_NUMBER_OF_SUPPORTED_IAC    , 2 },
  { HCI_READ_CURRENT_IAC_LAP            , 5 },
  { HCI_RESET_FAILED_CONTACT_COUNTER    , 3 },
  { HCI_READ_VOICE_SETTINGS             , 3 },
  { HCI_READ_COUNTRY_CODE               , 2 },
  { HCI_READ_RSSI                       , 4 }, /* 40 CMDS */
  { HCI_READ_TRANSMIT_POWER_LEVEL       , 4 }, 
  { HCI_READ_SYNCHRONOUS_FLOW_CONTROL_ENABLE, 2 },
  { HCI_GET_LINK_QUALITY                , 4 },
  { HCI_FLUSH                           , 3 },
  { HCI_SNIFF_SUBRATING, 3 },
  /*
   * V1.2 specific
   */
  { HCI_READ_CLOCK                           , 9  },
  { HCI_READ_LMP_HANDLE                      , 8  },
  { HCI_READ_AFH_CHANNEL_MAP                 , 14 },
  { HCI_READ_AFH_CHANNEL_ASSESSMENT_MODE     , 2  },
  { HCI_READ_INQUIRY_SCAN_TYPE               , 2  },/* 50 CMDS */
  { HCI_READ_INQUIRY_MODE                    , 2  },  
  { HCI_READ_PAGE_SCAN_TYPE                  , 2  },
  { HCI_READ_DEFAULT_LINK_POLICY_SETTINGS    , 3  },
  { HCI_READ_LOCAL_COMMANDS					 , 65 },
  { HCI_READ_LOCAL_EXTENDED_FEATURES         , 11 },
  { HCI_CREATE_CONNECTION_CANCEL                , 7},
  { HCI_REMOTE_NAME_REQUEST_CANCEL              , 7},
  { HCI_READ_EXTENDED_INQUIRY_RESPONSE_COMMAND, 242},
  { HCI_ENHANCED_FLUSH                        , 3 },
  { HCI_READ_DEFAULT_ERRONEOUS_DATA_REPORTING , 2 },/* 60 CMDS */
{ HCI_READ_INQUIRY_RESPONSE_TRANSMIT_POWER_LEVEL, 2}, 
/* Secure Simple Pairing BT 2.1 */
  { HCI_IO_CAPABILITY_REQUEST_REPLY            , 7 },
  { HCI_USER_CONFIRMATION_REQUEST_REPLY        , 7 },
#if 1 // GF Added 29 Dec
  { HCI_SEND_KEYPRESS_NOTIFICATION             , 7 },
#endif
  { HCI_USER_PASSKEY_REQUEST_REPLY             , 7 },
  { HCI_USER_PASSKEY_REQUEST_NEGATIVE_REPLY, 7 },
  { HCI_USER_CONFIRMATION_REQUEST_NEGATIVE_REPLY, 7 },
  { HCI_IO_CAPABILITY_REQUEST_NEGATIVE_REPLY    , 7 },
  { HCI_READ_LOCAL_OOB_DATA                    , 33 },
  { HCI_REMOTE_OOB_DATA_REQUEST_REPLY          , 7 }, /* 70 CMDS */
  { HCI_REMOTE_OOB_DATA_REQUEST_NEGATIVE_REPLY  , 7 },
  { HCI_READ_SIMPLE_PAIRING_MODE				, 2 },


  /*
   * V3.0 specific
   */
  { HCI_READ_ENHANCED_TRANSMIT_POWER_LEVEL , 6  },
  { HCI_READ_ENCRYPTION_KEY_SIZE            , 4 },
  /*
   * V4.0 BT LE Specific
   */

  { HCI_LE_READ_BUFFER_SIZE,                4},
  { HCI_LE_READ_LOCAL_SUPPORTED_FEATURES,   9},
  { HCI_LE_READ_ADVERTISING_CHANNEL_TX_POWER,              2},
  { HCI_LE_READ_WHITE_LIST_SIZE,           2},
  { HCI_LE_READ_CHANNEL_MAP,                      8},
  { HCI_LE_ENCRYPT,                              17},/* 80 CMDS */
  { HCI_LE_RAND,                                  9}, 
  { HCI_LE_LONG_TERM_KEY_REQUEST_REPLY,           3},
  { HCI_LE_LONG_TERM_KEY_REQUEST_NEGATIVE_REPLY,  3},
  { HCI_LE_READ_SUPPORTED_STATES,                 9},
  { HCI_LE_TEST_END,                              3},
#if (PRH_BS_CFG_SYS_LE_CONTROLLER_SUPPORTED==1)
  { HCI_READ_LE_HOST_SUPPORT,                     3}
#endif

};

/*
 * Holds event mask, on initialisation all events are on
 */
//static u_int32 hci_event_mask, hci_event_mask_extended;


/*********************************
 * Local function prototypes
 ***********************************/
void _Insert_Uint8(t_p_pdu p_buffer, u_int8 value_8_bit)
{
    p_buffer[0]=(u_int8)value_8_bit & 0xFF;
}

void _Insert_Uint16(t_p_pdu p_buffer, u_int16 value_16_bit)
{
    p_buffer[0]=(u_int8)value_16_bit & 0xFF;
    p_buffer[1]=(u_int8)(value_16_bit >> 8);
}

void _Insert_Uint32(t_p_pdu p_buffer, u_int32 value_32_bit)
{
    p_buffer[0]=(u_int8)value_32_bit;
    p_buffer[1]=(u_int8)(value_32_bit >> 8);
    p_buffer[2]=(u_int8)(value_32_bit >> 16);
    p_buffer[3]=(u_int8)(value_32_bit >> 24);
}

void _Insert_Uint24(t_p_pdu p_buffer,u_int32 val24)
{
    p_buffer[0]=(u_int8)val24;
    p_buffer[1]=(u_int8)(val24 >> 8);
    p_buffer[2]=(u_int8)(val24 >> 16);
}


static void _Insert_Bd_Addr_In_PDU(t_p_pdu p_buffer, const t_bd_addr* p_bd_addr)
{
    BDADDR_Get_Byte_Array_Ex(p_bd_addr, p_buffer);
}

/*
 * Local helpers
 */
t_error HCeg_Prepare_Command_Complete_Event_Read_Clock_Payload(
    t_cmd_complete_event *p_event_info, t_p_pdu p_buffer);

/******************************************************************
 *  FUNCTION :- HCeg_Initialise
 *
 *  DESCRIPTION :-
 *  This function resets the event mask
 *
 ******************************************************************/
void HCeg_Initialise(void)
{
    hci_event_mask = 0xFFFFFFFF; 
 //   hci_event_mask_extended = 0x00001FFF;
    hci_event_mask_extended = 0x20001FFF;  // Turn on LE Meta Event by Default
}

/******************************************************************
 *  FUNCTION :- HCeg_Set_Event_Mask
 *
 *  DESCRIPTION :-
 *  This function sets the event mask.  In the Specification the
 *  event mask is 8 bytes but currently V1.1 only 4 bytes are used.
 *
 *
 * From V1.1
 * "The Set_Event_Mask command is used to control which events are 
 * generated by HCI for the Host. If the bit in the Event_Mask is 
 * set to a one, then the event associated with that bit will be enabled. 
 * The Host has to deal with each event that occurs by the 
 * Bluetooth devices. The event mask allows the Host to control how
 * much it is interrupted.
 * Note: the Command Complete event, Command Status event and Number Of
 * Completed Packets event cannot be masked. These events always occur."
 *
 * Note the compulsory events above are handled by separate
 * functions that do not check the event mask.
 ******************************************************************/
t_error HCeg_Set_Event_Mask(t_p_pdu p_pdu)
{
    hci_event_mask = (*p_pdu) + ((*(p_pdu+1)) << 8) + 
                 ((*(p_pdu+2)) <<16 ) + ((*(p_pdu+3)) <<24 );
 
    /*
     * #1986
     */
    hci_event_mask_extended = *(p_pdu+4)+ ((*(p_pdu+5)) << 8) + 
                 ((*(p_pdu+6)) <<16 ) + ((*(p_pdu+7)) <<24 );

    /* 
     * Cannot mask out: Command_Complete, Command Status and
     * num_packets_complete event 
     */

    hci_event_mask |= 0x00046000;

    return NO_ERROR;
}
#ifndef REDUCE_ROM
/******************************************************************
 *  FUNCTION :- HCeg_Inquiry_Result_Event
 *
 *  DESCRIPTION :-
 *  Creates an inqury result and places it on outgoing HCI Queue:
 *  HCI_INQUIRY_RESULT_EVENT
 *  HCI_INQUIRY_RESULT_EVENT_WITH_RSSI
 *  HCI_EXTENDED_INQUIRY_RESULT_EVENT
 *
 ******************************************************************/
t_error HCeg_Inquiry_Result_Event(u_int8 event_code, t_lm_event_info* p_event_info)
{
	
	if(PATCH_FUN[HCEG_INQUIRY_RESULT_EVENT_ID]){
         
         return ((t_error (*)(u_int8 event_code, t_lm_event_info* p_event_info))PATCH_FUN[HCEG_INQUIRY_RESULT_EVENT_ID])(event_code, p_event_info);
    }

    u_int8 length;
    t_p_pdu p_buffer = 0x0;

    struct q_desc* qd;

    t_inquiryResult* p_result_list;

    /*
     * Do not allow Inquiry_Result_Event to fill
     * HCI_EVENT_Q completely, so can let other events through.
     */
    if (!HCeg_Prevent_HCI_Event_Queue_from_Overfill())
        return COMMAND_DISALLOWED;

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_INQUIRY_RESPONSE_SUPPORTED==1)
    if (event_code == HCI_EXTENDED_INQUIRY_RESULT_EVENT)
    {    
        /* The extended inquiry result event always contains a single response,
         * while the Extended_Inquiry_Response parameter is filled out with a non
         * significant part of all 0s to make up the full 240 octets.
         */
        length = 15 + MAX_EIR_PACKET_LENGTH;
    }
    else
#endif
    {
        /* Each response takes 14 bytes... (note currently the below code only
           handles the case where there is only one response per event) */
        length = (14 * p_event_info->number)+1;
    }

    /* extra 2 bytes for event code & packet length */
    qd=BTq_Enqueue(HCI_EVENT_Q, 0, (t_length)(length+2));
    if(qd) 
    {
        p_buffer=qd->data;
    }
    else
    {
        return MEMORY_FULL;
    }

    *(p_buffer) = event_code;
    p_buffer[1] = (u_int8) length;
    p_buffer[2] = p_event_info->number;

    /* Add 3 to p_buffer to get location for the responses */

    p_result_list = p_event_info->p_result_list;

    p_buffer+=3;
 
    _Insert_Bd_Addr_In_PDU(p_buffer, &(p_result_list->bd_addr));
    p_buffer+= SIZE_OF_BD_ADDR;

    *(p_buffer) = p_result_list->srMode;
    p_buffer+= SIZE_OF_SR_MODE;

    *(p_buffer) = p_result_list->spMode;
    p_buffer+= SIZE_OF_SP_MODE;

#if (PRH_BS_CFG_SYS_LMP_RSSI_INQUIRY_RESULTS_SUPPORTED==1)
    if (event_code == HCI_INQUIRY_RESULT_EVENT)
    {
        *(p_buffer) = 0x0; /* Reserved */
        p_buffer+= 1;
    }
#else /* BT1.1 Device  */
    *(p_buffer) = p_result_list->pageScanMode;
    p_buffer+= SIZE_OF_PAGE_SCAN_MODE;
#endif

    _Insert_Uint24(p_buffer, p_result_list->Cod);
    p_buffer+= SIZE_OF_DEVICE_CLASS;

    _Insert_Uint16(p_buffer, p_result_list->clkOffset);
    p_buffer+= SIZE_OF_CLK_OFFSET;

#if (PRH_BS_CFG_SYS_LMP_RSSI_INQUIRY_RESULTS_SUPPORTED==1)
    if (event_code != HCI_INQUIRY_RESULT_EVENT)
    {
        *(p_buffer) = p_result_list->rssi;
        p_buffer+= SIZE_OF_RSSI;
    }
#endif

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_INQUIRY_RESPONSE_SUPPORTED==1)
    if (event_code == HCI_EXTENDED_INQUIRY_RESULT_EVENT)
    {
        LMutils_Array_Copy( MAX_EIR_PACKET_LENGTH, p_buffer,
            USLCinq_Get_Stored_Extended_Inquiry_Response());
    }
#endif

    BTq_Commit(HCI_EVENT_Q,0);

    return NO_ERROR;
}
#endif
/******************************************************************
 *  FUNCTION :- HCeg_Command_Complete_Event
 *
 *  DESCRIPTION :-
 *  This function creates an HCI Command Complete Event and places it on
 *  outgoing HCI Queue. 
 *
 ******************************************************************/
void HCeg_Command_Complete_Event(t_cmd_complete_event* p_event_info)
{
	if(PATCH_FUN[HCEG_COMMAND_COMPLETE_EVENT_ID]){
         ((void (*)(t_cmd_complete_event* p_event_info))PATCH_FUN[HCEG_COMMAND_COMPLETE_EVENT_ID])(p_event_info);
         return ;
    }

	
    u_int8 para_length;
    u_int8 index = 0;
    int match_found = 0;
    t_p_pdu p_buffer = 0x0;

    struct q_desc* qd;
    const t_bufferSize* p_hc_buffer_size;

	/*
     * Do not allow Command_Complete_Event to fill
     * HCI_EVENT_Q completely, so can let other events through.
     */
	if (!HCeg_Prevent_HCI_Event_Queue_from_Overfill())
		return;

    /* 
     * First Determine the length of the p_buffer required 
     */

    if (p_event_info->opcode == HCI_READ_CURRENT_IAC_LAP)
    {
        para_length = (p_event_info->number * 3) + 2 ;
    }
    else
    {
        do
        {  
            if(p_event_info->opcode == hc_cmd_complete_event_array[index].opcode)
                match_found = 1;
            else
               index++;
        } 
        while((match_found != 1)&&(index<MAX_CMD_COMPLETE_ARRAY_SIZE));

        if(match_found)
            para_length = hc_cmd_complete_event_array[index].para_len;
        else
            para_length = 1; /* the default command complete carries only a status parameter */
     
    }   

    qd=BTq_Enqueue(HCI_EVENT_Q, 0, (t_length) 
        (para_length+SIZE_OF_EVENT_HEADER + SIZE_OF_COMMAND_COMPLETE_HEADER) );
    if(qd) 
    {
        p_buffer = qd->data;
    }
    else
    {
        /*
         * Error handler needs to be defined.
         */
        return;
    }

    switch(p_event_info->opcode) 
    {
    case HCI_READ_PAGE_TIMEOUT :
    case HCI_READ_CONNECT_ACCEPT_TO :
       /**************************************
        * Returns a status field (1 byte) and
        * a timeout value (2 bytes) 
        **************************************/
        _Insert_Uint16(p_buffer+6, (u_int16) p_event_info->timeout);
        break;  
 
    case HCI_READ_SCAN_ENABLE :
    case HCI_READ_AUTHENTICATION_ENABLE :
    case HCI_READ_ENCRYPTION_MODE :
    case HCI_READ_PIN_TYPE :
    case HCI_READ_LOOPBACK_MODE :
    case HCI_READ_HOLD_MODE_ACTIVITY :
    case HCI_READ_PAGE_SCAN_PERIOD_MODE :
    case HCI_READ_PAGE_SCAN_MODE :
    case HCI_READ_COUNTRY_CODE :
    case HCI_READ_AFH_CHANNEL_ASSESSMENT_MODE:
	case HCI_READ_SIMPLE_PAIRING_MODE:

		/* Returns status field (1 byte) and enable/mode field (1 byte) */
        *(p_buffer+6) = p_event_info->mode;
        break;

    case HCI_READ_PAGE_SCAN_ACT :
    case HCI_READ_INQUIRY_SCAN_ACT :
       /**************************************
        * Returns a status field (1 byte), 
        * a scan window field (2 bytes)
        * and a scan interval field (2 Bytes).
        **************************************/
        _Insert_Uint16(p_buffer+6, (u_int16) p_event_info->scan_activity.interval);
        _Insert_Uint16(p_buffer+8, (u_int16) p_event_info->scan_activity.window);
        break;

#if (PRH_BS_CFG_SYS_LMP_INTERLACED_PAGE_SCAN_SUPPORTED==1)
        case HCI_READ_PAGE_SCAN_TYPE:
            *(p_buffer+6) = (u_int8) p_event_info->scan_activity.scan_type;
            break;
#endif

#if (PRH_BS_CFG_SYS_LMP_INTERLACED_INQUIRY_SCAN_SUPPORTED==1)
        case HCI_READ_INQUIRY_SCAN_TYPE:
            *(p_buffer+6) = (u_int8) p_event_info->scan_activity.scan_type;
            break;
#endif

#if (PRH_BS_CFG_SYS_LMP_RSSI_INQUIRY_RESULTS_SUPPORTED==1)
        case HCI_READ_INQUIRY_MODE:
            *(p_buffer+6) = (u_int8) p_event_info->number;
            break;
#endif

    case HCI_READ_BUFFER_SIZE :
       /* Returns :
        *   status field (1 byte),
        *   HC_ACL_Data_Packet_Length (2 Bytes),
        *   HC_SCO_Data_Packet_Length (1 Byte), 
        *   HC_Total_Num_ACL_Data_Packets (2 Bytes),
        *   HC_Total_Num_SCO_Data_Packets (2 Bytes).
        */
        p_hc_buffer_size = p_event_info->returnParams.readBufferSize.p_hc_buffer_size;
        _Insert_Uint16(p_buffer+6, p_hc_buffer_size->aclDataPacketLength);
        *(p_buffer+8) = p_hc_buffer_size->scoDataPacketLength;
        _Insert_Uint16(p_buffer+9, p_hc_buffer_size->numAclDataPackets);
        _Insert_Uint16(p_buffer+11, p_hc_buffer_size->numScoDataPackets); /* 2001/03/03 adegawa */
        break;

    case HCI_LE_READ_BUFFER_SIZE :
        #if 0
        _Insert_Uint16(p_buffer+6, 27); // No dedicated LE Buffer ?use Read_Buffer_Size command
        *(p_buffer+8) = 0x08; // No dedicated LE Buffer ?use Read_Buffer_Size command
	 /* _Insert_Uint16(p_buffer+6, 27); // No dedicated LE Buffer ?use Read_Buffer_Size command
        *(p_buffer+8) = 15;  */
        #else
        p_hc_buffer_size = p_event_info->returnParams.readBufferSize.p_hc_buffer_size;
        _Insert_Uint16(p_buffer+6, (p_hc_buffer_size->aclDataPacketLength-4)); // No dedicated LE Buffer ?use Read_Buffer_Size command
        *(p_buffer+8) = p_hc_buffer_size->numAclDataPackets; // No dedicated LE Buffer ?use Read_Buffer_Size command
        
        #endif
        break;

    case HCI_READ_LOCAL_FEATURES :
       /* Returns a status field (1 byte), LMP_features (8 Bytes)
        */
        LMutils_Array_Copy(8,p_buffer+6,p_event_info->p_u_int8);
        break;

	case HCI_READ_LOCAL_COMMANDS :
       /* Returns a status field (1 byte) and HCI_Commands (64 Bytes)
        */
        LMutils_Array_Copy(64,p_buffer+6,p_event_info->p_u_int8);
        break;

    case HCI_READ_LOCAL_EXTENDED_FEATURES :
        /*
         */
        p_buffer[6] = p_event_info->returnParams.readLocalExtendedFeatures.page;
        p_buffer[7] = p_event_info->returnParams.readLocalExtendedFeatures.max_page;
        if(p_event_info->p_u_int8)
            LMutils_Array_Copy(8, p_buffer+8, p_event_info->p_u_int8);
        else
            hw_memset(p_buffer+8, 0, 8);
        break;

    case HCI_READ_LOCAL_VER_INFO :
       /****************************************************************
        * Returns a status field (1 byte), the HCI_Version (1 Byte),
        * HCI_Revision (2 Bytes) LMP_Version (1 Byte), Manufacturer Name
        * (2 Bytes), LMP_Subversion (2 Bytes)
        ****************************************************************/
        {
            const t_versionInfo*  p_version;

            p_version= p_event_info->returnParams.readLocalVersion.version;
            *(p_buffer+6) = p_version->HCI_version;
            _Insert_Uint16(p_buffer+7, p_version->HCI_revision);
            *(p_buffer+9) = p_version->lmp_version;
            _Insert_Uint16(p_buffer+10, p_version->comp_id);
            _Insert_Uint16(p_buffer+12, p_version->lmp_subversion);
        }
        break; 
  
    case HCI_READ_BD_ADDR :
    case HCI_LINK_KEY_REQUEST_REPLY :
    case HCI_LINK_KEY_REQUEST_NEGATIVE_REPLY :
    case HCI_PIN_CODE_REQUEST_REPLY :
    case HCI_PIN_CODE_REQUEST_NEGATIVE_REPLY :   
    case HCI_CREATE_CONNECTION_CANCEL:
    case HCI_REMOTE_NAME_REQUEST_CANCEL:
	case HCI_IO_CAPABILITY_REQUEST_REPLY:
	case HCI_IO_CAPABILITY_REQUEST_NEGATIVE_REPLY:
	case HCI_USER_CONFIRMATION_REQUEST_REPLY:
#if (PRH_BS_CFG_SYS_SSP_PASSKEY_SUPPORTED==1)
	case HCI_USER_PASSKEY_REQUEST_REPLY :
	case HCI_USER_PASSKEY_REQUEST_NEGATIVE_REPLY :
	case HCI_SEND_KEYPRESS_NOTIFICATION :
#endif
	case HCI_USER_CONFIRMATION_REQUEST_NEGATIVE_REPLY:
#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)
	case HCI_REMOTE_OOB_DATA_REQUEST_REPLY :
	case HCI_REMOTE_OOB_DATA_REQUEST_NEGATIVE_REPLY :
#endif
        _Insert_Bd_Addr_In_PDU(p_buffer+6,p_event_info->p_bd_addr);
        break;

    case HCI_READ_VOICE_SETTINGS :
        _Insert_Uint16(p_buffer+6,p_event_info->value16bit);
        break;
    
    case HCI_READ_SYNCHRONOUS_FLOW_CONTROL_ENABLE :
    case HCI_WRITE_STORED_LINK_KEY :
    case HCI_READ_NUM_BROADCAST_RETRANSMISSIONS :
	case HCI_READ_INQUIRY_RESPONSE_TRANSMIT_POWER_LEVEL :
        *(p_buffer+6) = p_event_info->number;
        break;

    case HCI_READ_STORED_LINK_KEY :
        _Insert_Uint16(p_buffer+6, 
            p_event_info->returnParams.readStoredLinkKey.max_num_keys);  
        _Insert_Uint16(p_buffer+8, 
            p_event_info->returnParams.readStoredLinkKey.num_keys_read);      
        break;

    case HCI_DELETE_STORED_LINK_KEY :
        *(p_buffer+6) = (p_event_info->value16bit & 0xFF);
        *(p_buffer+7) = ((p_event_info->value16bit & 0xFF00) >> 8);
        break;

    case HCI_READ_LOCAL_NAME :
        /*
         * The returned name parameter is always 248
         * Copy the name string (p_u_int8) to correct position in the event pdu
         * If name length < event name parameter length(248) then pad with 0
         * which implicitly terminates the name string with NULL
         */
        {
            t_p_pdu p_byte_pos = p_buffer+6;

            index = p_event_info->number;
            LMutils_Array_Copy(index, p_byte_pos, p_event_info->p_u_int8);
            while (index < 248)
            {
                p_byte_pos[index] = 0;
                index++;
            }
        }
        break;

    case HCI_ROLE_DISCOVERY :
        _Insert_Uint16(p_buffer+6,p_event_info->handle);
        *(p_buffer+8) = p_event_info->mode;
        break;

    case HCI_READ_LINK_POLICY_SETTINGS :
        _Insert_Uint16(p_buffer+6,p_event_info->handle);
        _Insert_Uint16(p_buffer+8,p_event_info->value16bit);  
        break;

#if(PRH_BS_CFG_SYS_HCI_V12_FUNCTIONALITY_SUPPORTED==1)
    case HCI_READ_DEFAULT_LINK_POLICY_SETTINGS:
        _Insert_Uint16(p_buffer+6,p_event_info->value16bit);
        break;
#endif

    case HCI_READ_CLASS_OF_DEVICE :
        _Insert_Uint24(p_buffer+6,p_event_info->returnParams.readClassDevice.cod);
        break;

    case HCI_WRITE_CLASS_OF_DEVICE :
    case HCI_WRITE_VOICE_SETTINGS :
    case HCI_WRITE_PAGE_SCAN_PERIOD_MODE :
    case HCI_WRITE_PAGE_SCAN_MODE :
        break;

    case HCI_READ_FAILED_CONTACT_COUNTER :
       /*****************************************************************
        * Returns a status field (1 byte), a Connection Handle (2 Bytes)
        * a counter value (1 bytes)       
        *****************************************************************/
        _Insert_Uint16(p_buffer+6,p_event_info->handle);
        _Insert_Uint16(p_buffer+8,p_event_info->number);  
        break;

    case HCI_RESET_FAILED_CONTACT_COUNTER :
    case HCI_FLUSH :
        _Insert_Uint16(p_buffer+6,p_event_info->handle);
        break;
         
    case HCI_ENHANCED_FLUSH :
        _Insert_Uint16(p_buffer+6,p_event_info->handle);
        break;

    case HCI_READ_SUPERVISION_TIMEOUT : 
    case HCI_READ_AUTOMATIC_FLUSH_TIMEOUT :
       /*****************************************************************
        * Returns a status field (1 byte), a Connection Handle (2 Bytes)
        * a timeout value (2 bytes)       
        *****************************************************************/
        _Insert_Uint16(p_buffer+6, p_event_info->handle);
        _Insert_Uint16(p_buffer+8, (u_int16) p_event_info->timeout);  
        break;

    case HCI_READ_NUMBER_OF_SUPPORTED_IAC :
        *(p_buffer+6) = p_event_info->number; 
        break;

    case HCI_READ_CURRENT_IAC_LAP :
        *(p_buffer+6) = p_event_info->number;          
        LMutils_Array_Copy((u_int8)(p_event_info->number*3),p_buffer+7,p_event_info->p_u_int8);
        break;

    case HCI_READ_RSSI:
        _Insert_Uint16(p_buffer+6, (u_int16)p_event_info->handle); /* connection handle */
        *(p_buffer+8) = (u_int8)(p_event_info->number); /* rssi */
        break;

    case HCI_READ_TRANSMIT_POWER_LEVEL:
    case HCI_GET_LINK_QUALITY:
        _Insert_Uint16(p_buffer+6, (u_int16)p_event_info->handle); /* connection handle */
        *(p_buffer+8)=(u_int8)(p_event_info->number); /* power level */
        break;

    case HCI_WRITE_AUTOMATIC_FLUSH_TIMEOUT :
    case HCI_WRITE_SUPERVISION_TIMEOUT :
    case HCI_WRITE_LINK_POLICY_SETTINGS :
        /*****************************************************************
        * Returns a status field (1 byte), a Connection Handle (2 Bytes)     
        *****************************************************************/
        _Insert_Uint16(p_buffer+6,p_event_info->handle);
        break;

	case HCI_READ_DEFAULT_ERRONEOUS_DATA_REPORTING :
		*(p_buffer+6)=(u_int8)(p_event_info->number);  // Enable flag
		break;

#if PRH_BS_CFG_SYS_HCI_READ_CLOCK_SUPPORTED
    case HCI_READ_CLOCK:
        p_event_info->status = 
            HCeg_Prepare_Command_Complete_Event_Read_Clock_Payload(
                p_event_info, p_buffer);
        break;
#endif

    case HCI_READ_ENCRYPTION_KEY_SIZE:
        _Insert_Uint16(p_buffer+6, (u_int16)p_event_info->handle); /* connection handle */
        *(p_buffer+8) = (u_int8)(p_event_info->number); /* key size */
        break;


#if PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_SUPPORTED
    case HCI_READ_AFH_CHANNEL_MAP :
        _Insert_Uint16(p_buffer+6, p_event_info->handle);
        p_buffer[8] = p_event_info->mode;
        LMutils_Array_Copy(10, p_buffer+9, p_event_info->p_u_int8);
        break;
#endif
#if ((PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED == 1) || (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1))
    case HCI_READ_LMP_HANDLE:
        _Insert_Uint16(p_buffer+6, p_event_info->handle);
        p_buffer[8] = p_event_info->number; /* LMP_handle */
        _Insert_Uint32(p_buffer+9, 0 /*Reserved*/);
        break;
#endif  

#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUBRATING_SUPPORTED==1)
    case HCI_SNIFF_SUBRATING:
        _Insert_Uint16(p_buffer+6, p_event_info->handle);
        break;
#endif  

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_INQUIRY_RESPONSE_SUPPORTED==1)
    case HCI_READ_EXTENDED_INQUIRY_RESPONSE_COMMAND:
        p_buffer[6] = p_event_info->mode;
        LMutils_Array_Copy(MAX_EIR_PACKET_LENGTH, p_buffer+7, p_event_info->p_u_int8);
        break;
#endif

#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==1)
		case HCI_READ_ENHANCED_TRANSMIT_POWER_LEVEL:
			{
				_Insert_Uint16(p_buffer+6,p_event_info->handle);
				p_buffer[8] = p_event_info->number;
				p_buffer[9] = p_event_info->mode;
				p_buffer[10] = (u_int8) p_event_info->value16bit;
			}
			break;
#endif

#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)
	case HCI_READ_LOCAL_OOB_DATA :
		{
			u_int8 i;

			for (i=0;i<16;i++)
			{
				p_buffer[6+i] = g_LM_config_info.hash_C[15-i];
				p_buffer[22+i] = g_LM_config_info.randomizer_R[15-i];
			}
		}
		break;
#endif
#endif
    case HCI_LE_READ_ADVERTISING_CHANNEL_TX_POWER :
    case HCI_LE_READ_WHITE_LIST_SIZE :
        *(p_buffer+6) = p_event_info->number;
        break;

    case HCI_LE_READ_CHANNEL_MAP :
    	_Insert_Uint16(p_buffer+6,p_event_info->handle);
    	LMutils_Array_Copy(5,p_buffer+8,p_event_info->p_u_int8);
    	break;

#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
    case HCI_LE_ENCRYPT :
    	LMutils_Array_Copy(16,p_buffer+6,p_event_info->p_u_int8);
    	break;

    case HCI_LE_RAND :
    	LMutils_Array_Copy(8,p_buffer+6,p_event_info->p_u_int8);
    	break;

    case HCI_LE_LONG_TERM_KEY_REQUEST_REPLY :
    case HCI_LE_LONG_TERM_KEY_REQUEST_NEGATIVE_REPLY :
    
        /*! NOTE THE HANDLE @20170103 */
        
    	_Insert_Uint16(p_buffer+6, 0x0010/*p_event_info->handle*/);
    	
    	break;
#else
	case HCI_LE_ENCRYPT :
		LMutils_Array_Copy(16,p_buffer+6,p_event_info->p_u_int8);
		break;

	case HCI_LE_RAND :
		LMutils_Array_Copy(8,p_buffer+6,p_event_info->p_u_int8);
		break;

#endif

    case HCI_LE_READ_SUPPORTED_STATES :
    	LMutils_Array_Copy(8,p_buffer+6,p_event_info->p_u_int8);
    	break;

   // case HCI_LE_RECEIVER_TEST :
  // case HCI_LE_TRANSMITTER_TEST :

    case HCI_LE_TEST_END :
    	_Insert_Uint16(p_buffer+6,p_event_info->value16bit);
    	break;

    case HCI_LE_READ_LOCAL_SUPPORTED_FEATURES :
		/* Returns a status field (1 byte), LMP_features (8 Bytes)
		 */

		LMutils_Array_Copy(8,p_buffer+6,p_event_info->p_u_int8);
		break;

    case HCI_READ_LE_HOST_SUPPORT :
    	*(p_buffer+6) = g_LM_config_info.le_host_supported;
    	*(p_buffer+7) = g_LM_config_info.le_and_bredr_host;
		break;
    }
        
    *(p_buffer) =HCI_COMMAND_COMPLETE_EVENT;
    *(p_buffer+1) = para_length+SIZE_OF_COMMAND_COMPLETE_HEADER;  /*Note the command status is already considered */
    *(p_buffer+2) = 1;
    _Insert_Uint16(p_buffer+3,p_event_info->opcode);
    *(p_buffer+5) = p_event_info->status;

    BTq_Commit(HCI_EVENT_Q,0);
}

/******************************************************************
 *  FUNCTION :- HCeg_Command_Status__Event
 *
 *  DESCRIPTION :-
 *  This function creates an HCI Command Status Event and places it on
 *  outgoing HCI Queue. 
 *
 *  NOTE COMMAND STATUS CANNOT BE MASKED - See Pg 597 of spec
 ******************************************************************/
void HCeg_Command_Status_Event(t_error status, u_int16 opcode)
{
    u_int8 num_hci_commands=1; 

    t_lm_event_info event_info;
    
	/*
     * Do not allow Command_Status_Event to fill
     * HCI_EVENT_Q completely, so can let other events through.
     */
	if (!HCeg_Prevent_HCI_Event_Queue_from_Overfill())
		return;

    event_info.status = status;
    event_info.value16bit = opcode;
    event_info.number = num_hci_commands;

    HCeg_Generate_Event(HCI_COMMAND_STATUS_EVENT,&event_info);
}

#if (PRH_BS_CFG_SYS_LOOPBACK_VIA_HCI_SUPPORTED == 1)
/******************************************************************
 *  FUNCTION :- HCeg_Loopback_Command_Event
 *
 *  DESCRIPTION :-
 *  This function creates an HCI Loopback Command Event and places it on
 *  outgoing HCI Queue. 
 *
 ******************************************************************/
void HCeg_Loopback_Command_Event(t_p_pdu p_command_buff, u_int16 opcode)
{
    u_int8 length;
    u_int8 opcode_param_length;
    t_p_pdu p_buffer = 0x0;

    struct q_desc* qd;

    if (hci_event_mask & LOOPBACK_COMMAND_EVENT_MASK)
    {
        /* The length to be reserved is the parameter length of the 
         * original HCI command PLUS
         *    3 bytes for the Command Opcode and Parameter length field
         *    2 bytes for the Event Code and Parameter length field.
         */

        opcode_param_length=p_command_buff[2];
        length=opcode_param_length+3;

        qd = BTq_Enqueue(HCI_EVENT_Q, 0, (t_length) (length+2) );
        if(qd)
        {
            p_buffer=qd->data;

            p_buffer[0] = HCI_LOOPBACK_COMMAND_EVENT;
            p_buffer[1] = length;

            p_buffer += 2;

            LMutils_Array_Copy(length, p_buffer, p_command_buff);

            BTq_Commit(HCI_EVENT_Q, 0);
        }
        else
        {
           /*
            * Error handler needs to be defined.
            */
        }
    }
}
#endif

/******************************************************************
 *  FUNCTION :- HCeg_Number_Of_Completed_Packets_Event
 *
 *  DESCRIPTION :-
 *  This function creates the Number Of Completed Packets Event and it 
 *  places it in the outgoing HCI Queue. This scans the queues to find
 *  the appropiate parameters to send in this event  - JS/MM
 *
 *  NOTE    - This event cannot be masked 
 *  CONTEXT - system scheduler
 *
 *  The message is built on the stack.  This won't cause additional
 *  stack requirements since single level function from scheduler.
 *
 * Find the number of active handles
 * 1. Check consumed broadcast packets on broadcast queues:
 *    index 0                         is active broadcast
 *    index MAX_ACTIVE_DEVICE_LINKS   is piconet broadcast (a separate queue)
 * 2. Traverse all link containers for active
 * 
 ******************************************************************/
boolean HCeg_Number_Of_Completed_Packets_Event(u_int8 num_handles, 
     const u_int16* handles, const u_int16 *completed_packets) 
{
   
    if(PATCH_FUN[HCEG_NUMBER_OF_COMPLETED_PACKETS_EVENT_ID]){
         
         return ((boolean (*)(u_int8 num_handles, const u_int16* handles, const u_int16 *completed_packets))PATCH_FUN[HCEG_NUMBER_OF_COMPLETED_PACKETS_EVENT_ID])(num_handles, handles, completed_packets);
    }

	u_int16 length;                     /* HCI Event Length                 */
    t_q_descr *qd;                      /* Q descriptor pointer             */
    u_int8  *p_buffer;                  /* Pointer to pdu data buffer       */
    u_int16 i;                          /* Index for link containers        */
	
    /*
     * Do not allow Number_Of_Completed_Packets_Events to fill
     * HCI_EVENT_Q completely, so can let other events through.
     */
#ifndef BLUETOOTH_MODE_LE_ONLY
	// If in testmode Loopback we ignore the number of completed packets event
	if (BTtst_Get_DUT_Mode() >= DUT_ACTIVE)
		return 1;
#endif//BLUETOOTH_MODE_LE_ONLY

	if (!HCeg_Prevent_HCI_Event_Queue_from_Overfill())
		return 0;

    /*
     * Determine event length:
     *     2 bytes for event header
     *     1 byte for number of handles
     *     4 bytes per handle
     */
    length = 2 + 1 + (num_handles<<2); 

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, length);

    if(qd)
    {
        p_buffer = qd->data;

        *p_buffer++ = HCI_NUMBER_OF_COMPLETED_PACKETS_EVENT;
        *p_buffer++ = length - 2;
        *p_buffer++ = num_handles;
   
        for(i=0; i < num_handles; i++)
        {
            *p_buffer++ = (u_int8) (handles[i] & 0xFF);
            *p_buffer++ = (u_int8) (handles[i] >> 8);
            *p_buffer++ = (u_int8) (completed_packets[i] & 0xFF);
            *p_buffer++ = (u_int8) (completed_packets[i] >> 8);
        }
        BTq_Commit(HCI_EVENT_Q, 0);
    }
			
    return (qd!=0);
}

#if PRH_BS_CFG_SYS_HCI_READ_CLOCK_SUPPORTED
/******************************************************************
 *  FUNCTION :- HCeg_Prepare_Command_Complete_Event_Read_Clock
 *
 *  DESCRIPTION :-
 *  This function prepares payload for HCI_Read_Clock 
 *  command complete event
 *
 * p_event_info->handle     handle for command
 * p_event_info->status 
 * p_event_info->number     which_clock 
 *
 *
 * Returns via structure the status for later insertion.
 ******************************************************************/
t_error HCeg_Prepare_Command_Complete_Event_Read_Clock_Payload(
    t_cmd_complete_event *p_event_info, t_p_pdu p_buffer)
{
    u_int8  which_clock = p_event_info->number;
    u_int16 handle = 0;
    t_error status = p_event_info->status;
    u_int16 accuracy = 0;
    u_int32 clock = 0;

    /*
     * Return parameters on command complete event      Initialised
     *  Status              p_event_info->status        n/a
     *  Connection_Handle   p_event_info->handle        cmd handle
     *  Clock               p_event_info->u_int32val    n/a
     *  Accuracy            p_event_info->value16bit    0
     */
    if (which_clock==0)
    {
        /*
         * Return local Bluetooth clock i.e. native clock
         * Default accuracy is always 0.
         */
        status = NO_ERROR;
        handle = 0;
        clock = LC_Get_Native_Clock();
    }
    else if (which_clock==1) 
    {
        handle = p_event_info->handle;

        if (status != NO_CONNECTION)
        {
            /*
             * Valid handle and which_clock == 1 then return piconet clock
             * p_event_info->handle defined above
             */
            t_lmp_link *p_link = LMaclctr_Find_ACL_Handle(handle);
            p_event_info->status = NO_ERROR;
            clock = LC_Get_Piconet_Clock(p_link->device_index);
            /*
             * Accuracy = +/- N * 0.3125 msec (1 Bluetooth Clock)
             *  Slave       Unknown   0xFFFF until this makes more sense 
             *  Master      Accurate  0x0000 p_event_info->value16bit default
             * Note queue/transport delivery times are not accounted for.
             */
            if (p_link->role==SLAVE)
            {
                /*
                 * Accuracy: (Normal worse case results in ~33 ticks)!
                 *      = Time_since_last_access_secs*drift_secs/tick_secs
                 *      = Time_since_last_access_secs*drift_us/tick_us
                 *      = Time_since_last_access_ticks*drift_us/tick_us^2
                 *      = 3.2e-3*3.2e-3*Time_since_last_access_ticks*drift_us
                 *      = Time_since_last_access_ticks*drift_ppm/1e6
                 *      ~ Time_since_last_access_ticks*drift_ppm/2^20 (within 0.2%)
                 * For 40seconds with drift of 250us => Actual +/-32 ticks 
                 * Below
                 */
                t_devicelink *p_device_link = 
                    DL_Get_Device_Ref(p_link->device_index);

                accuracy = (u_int16) (
                    (BTtimer_Clock_Difference(
                        DL_Get_Piconet_Clock_Last_Access(p_device_link), 
                        clock) *
                    DL_Get_Clock_Drift(p_device_link))/3906 ) >> 8;
            }
        }
    }
    else
    {
        status = INVALID_HCI_PARAMETERS;
    }

    _Insert_Uint16(p_buffer+6, handle);
    _Insert_Uint32(p_buffer+8, clock);
    _Insert_Uint16(p_buffer+12, accuracy);

    return status;
}
#endif

/******************************************************************
 *  FUNCTION :- HCeg_Hardware_Error_Event
 *
 *  DESCRIPTION :-
 *  This function generates a hardware error event.
 *
 *  hardware_code   The cause of the event to be reported
 ******************************************************************/
void HCeg_Hardware_Error_Event(u_int8 hardware_code)
{
    t_lm_event_info event_info;

	/*
     * Do not allow Hardware_Error_Event to fill
     * HCI_EVENT_Q completely, so can let other events through.
     */
	if (!HCeg_Prevent_HCI_Event_Queue_from_Overfill())
		return;

    event_info.number = hardware_code;
    HCeg_Generate_Event(HCI_HARDWARE_ERROR_EVENT, &event_info);
}

/******************************************************************
 *  FUNCTION :- HCeg_Generate_Event
 *
 *  DESCRIPTION :-
 *  This function handles all events except
 *      HCI_COMMAND_STATUS_EVENT
 *      HCI_COMMAND_COMPLETE_EVENT
 *      HCI_NUMBER_OF_COMPLETED_PACKETS_EVENT
 *  which have their own handlers and cannot be masked.
 ******************************************************************/
t_error HCeg_Generate_Event(u_int8 event_code,t_lm_event_info* p_event_info)
{
	if(PATCH_FUN[HCEG_GENERATE_EVENT_ID]){
         
         return ((t_error (*)(u_int8 event_code,t_lm_event_info* p_event_info))PATCH_FUN[HCEG_GENERATE_EVENT_ID])(event_code, p_event_info);
    }

	
    u_int8 para_length;
    u_int16 queue_length;
    t_p_pdu p_buffer =0x0;
#if (PRH_BS_CFG_SYS_QOS_SUPPORTED==1)
    t_lmp_link* p_link;
#endif
    struct q_desc* qd;
  
    if((event_code > 0)&&(event_code <= MAX_NUM_HC_EVENTS))
    {  
        para_length = hc_event_param_len[event_code];
        queue_length = para_length+2;

        if(HCeg_Is_Event_Masked_On(event_code))
        {
#ifndef BLUETOOTH_MODE_LE_ONLY       
            if ( (event_code == HCI_INQUIRY_RESULT_EVENT) || 
                 (event_code ==  HCI_INQUIRY_RESULT_EVENT_WITH_RSSI) ||
                 (event_code == HCI_EXTENDED_INQUIRY_RESULT_EVENT) )
            {
                HCeg_Inquiry_Result_Event(event_code, p_event_info);
            }
            else
#endif//BLUETOOTH_MODE_LE_ONLY 
            {
                qd = BTq_Enqueue(HCI_EVENT_Q, 0, queue_length);
                if(qd)
                {
                    p_buffer=qd->data;
                }
                else
                {
                    return MEMORY_FULL;
                }

                if (p_buffer == 0)
                {
                    return UNSPECIFIED_ERROR;
                }

                *(p_buffer)= event_code;
                *(p_buffer+1)= para_length;
                /*
                 * Status is always 2nd byte if it exists
                 */
                *(p_buffer+2) = p_event_info->status;

/*debug*/
/*			if(event_code == HCI_DISCONNECTION_COMPLETE_EVENT)
				{
					__NOP();
					while(1);
				}
*/

//- add {switch }.
			switch(event_code)
			{
                case HCI_DISCONNECTION_COMPLETE_EVENT :
                    _Insert_Uint16(p_buffer+3, p_event_info->handle);
                    *(p_buffer+5) = p_event_info->reason; 
                    break;

                case HCI_COMMAND_STATUS_EVENT :
                    *(p_buffer+3) = p_event_info->number;
                    _Insert_Uint16(p_buffer+4,p_event_info->value16bit);
                    break;
                    
                case HCI_READ_REMOTE_VERSION_INFORMATION_COMPLETE_EVENT :
                    _Insert_Uint16(p_buffer+3,(u_int16)p_event_info->handle);
                    *(p_buffer+5) = p_event_info->lmp_version;
                    _Insert_Uint16(p_buffer+6,(u_int16)p_event_info->comp_id);
                    _Insert_Uint16(p_buffer+8,(u_int16)p_event_info->lmp_subversion);
                    break;

				case HCI_ENCRYPTION_CHANGE_EVENT : 
					//*(p_buffer+3) = 0x01;//p_event_info->status;
					//_Insert_Uint16(p_buffer+4,p_event_info->handle);
					*(p_buffer+4) = (p_event_info->handle >> 8);
					*(p_buffer+3) = p_event_info->handle;
					*(p_buffer+5) = 0x01;
					break;

				case HCI_ENCRYPTION_KEY_REFRESH_COMPLETE_EVENT :
                    _Insert_Uint16(p_buffer+3,p_event_info->handle);
                    break;

                    
				default:
					break;
			}	

#ifndef BLUETOOTH_MODE_LE_ONLY 

                switch(event_code)
                {

                case HCI_INQUIRY_COMPLETE_EVENT :
                    /* *(p_buffer+3) = p_event_info->number; Removed in Version 1.1 of BT Spec */
                    break;

                case HCI_CONNECTION_COMPLETE_EVENT :
                    _Insert_Uint16(p_buffer+3,p_event_info->handle);
                    _Insert_Bd_Addr_In_PDU(p_buffer+5,p_event_info->p_bd_addr);
                    *(p_buffer+11) = p_event_info->link_type;
                    *(p_buffer+12) = p_event_info->mode;
                    if ((ACL_LINK == p_event_info->link_type) && (p_event_info->status == NO_ERROR))
                    {
                        if (p_event_info->ack_required)
                        {
                           /* 
                            * Set a call back to the LMP to enable L2CAP Traffic
                            */
                            qd->callback = LMch_CB_Enable_L2CAP_Traffic;
                        }
                    }
                    break;

                case HCI_CONNECTION_REQUEST_EVENT :
                    _Insert_Bd_Addr_In_PDU(p_buffer+2,p_event_info->p_bd_addr);
                    _Insert_Uint32(p_buffer+8,p_event_info->cod);
                    *(p_buffer+11) = (u_int8)p_event_info->link_type;
                    break;

                case HCI_DISCONNECTION_COMPLETE_EVENT :
                    _Insert_Uint16(p_buffer+3, p_event_info->handle);
                    *(p_buffer+5) = p_event_info->reason; 
                    break;

                case HCI_MASTER_LINK_KEY_COMPLETE_EVENT:
                //case HCI_ENCRYPTION_CHANGE_EVENT : 
                    *(p_buffer+5) = p_event_info->mode;
                    /* Intentional Fall Through */

                case HCI_AUTHENTICATION_COMPLETE_EVENT :  
                case HCI_CHANGE_CONNECTION_LINK_KEY_COMPLETE_EVENT :
                    _Insert_Uint16(p_buffer+3,p_event_info->handle);   
                    break;

				case HCI_ENCRYPTION_CHANGE_EVENT : 
					*(p_buffer+3) = p_event_info->status;
					_Insert_Uint16(p_buffer+5,p_event_info->handle);
					*(p_buffer+6) = 0x01;
					break;

                case HCI_QOS_VIOLATION_EVENT :
                case HCI_FLUSH_OCCURED_EVENT :
#if(PRH_BS_CFG_SYS_LMP_NONFLUSHABLE_PBF_SUPPORTED==1)
				case HCI_ENHANCED_FLUSH_COMPLETE_EVENT :
#endif
                    _Insert_Uint16(p_buffer+2,p_event_info->handle);

                    break;
                case HCI_ENCRYPTION_KEY_REFRESH_COMPLETE_EVENT :
                    _Insert_Uint16(p_buffer+3,p_event_info->handle);
                    break;

                case HCI_READ_REMOTE_SUPPORTED_FEATURES_COMPLETE_EVENT :
                    _Insert_Uint16(p_buffer+3,p_event_info->handle);
                    if (NO_ERROR == p_event_info->status)
                    {
                        p_buffer+=5;
                        LMutils_Array_Copy(8,p_buffer,p_event_info->p_u_int8);
                    }
                    else
                    {
                        /*
                         * Theres been a problem, tell the host that
                         * there are no features available on the peer device.
                         */
                        LMutils_Set_Uint32(p_buffer+5, 0);
                        LMutils_Set_Uint32(p_buffer+9, 0);
                    }
                    break;

#if (PRH_BS_CFG_SYS_HCI_V12_FUNCTIONALITY_SUPPORTED==1)
                case HCI_READ_REMOTE_EXTENDED_FEATURES_COMPLETE_EVENT:
                    _Insert_Uint16(&(p_buffer[3]),p_event_info->handle);
                    if(NO_ERROR == p_event_info->status)
                    {
                        p_buffer[5] = p_event_info->page;
                        p_buffer[6] = p_event_info->max_page;
                        LMutils_Array_Copy(8, &(p_buffer[7]), p_event_info->p_u_int8);
                    }
                    else
                    {
                        /*
                         * There's been a problem. Tell the host that
                         * the peer device has no extended features
                         * available.
                         */
                        LMutils_Set_Uint16(p_buffer+5, 0);
                        LMutils_Set_Uint32(p_buffer+7, 0);
                        LMutils_Set_Uint32(p_buffer+11, 0);
                    }
                    break;
#endif

				case HCI_REMOTE_HOST_SUPPORTED_FEATURES_NOTIFICATION_EVENT :
                    _Insert_Bd_Addr_In_PDU(p_buffer+2,p_event_info->p_bd_addr);
					LMutils_Array_Copy(8, &(p_buffer[8]), p_event_info->p_u_int8);
					break;

                case HCI_READ_REMOTE_VERSION_INFORMATION_COMPLETE_EVENT :
                    _Insert_Uint16(p_buffer+3,(u_int16)p_event_info->handle);
                    *(p_buffer+5) = p_event_info->lmp_version;
                    _Insert_Uint16(p_buffer+6,(u_int16)p_event_info->comp_id);
                    _Insert_Uint16(p_buffer+8,(u_int16)p_event_info->lmp_subversion);
                    break;

                case HCI_COMMAND_STATUS_EVENT :
                    *(p_buffer+3) = p_event_info->number;
                    _Insert_Uint16(p_buffer+4,p_event_info->value16bit);
                    break;

#if (PRH_BS_CFG_SYS_QOS_SUPPORTED==1)
                case HCI_QoS_SETUP_COMPLETE_EVENT : 
                    _Insert_Uint16(p_buffer+3,(u_int16)p_event_info->handle);
                    p_link = LMaclctr_Find_Handle(p_event_info->handle);
                    *(p_buffer+5) = 0; /* Flags field current not used in Bluetooth */
                    *(p_buffer+6) = p_link->service_type;
                    _Insert_Uint32(p_buffer+7,(u_int32)p_link->token_rate);
                    _Insert_Uint32(p_buffer+11,(u_int32)p_link->peak_bandwidth);
                    _Insert_Uint32(p_buffer+15,(u_int32)p_link->latency);
                    _Insert_Uint32(p_buffer+19,(u_int32)p_link->delay_variation);
                    break;
#endif       

#if (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1)              
                case HCI_ROLE_CHANGE_EVENT :
                    _Insert_Bd_Addr_In_PDU(p_buffer+3,p_event_info->p_bd_addr);
                    *(p_buffer+9) = p_event_info->role;
                    break;
#endif  
                case HCI_MODE_CHANGE_EVENT :
                    _Insert_Uint16(p_buffer+3,(u_int16)p_event_info->handle);  
                    *(p_buffer+5) = p_event_info->mode;
                    _Insert_Uint16(p_buffer+6,p_event_info->value16bit); 
                    break;             

                case HCI_RETURN_LINK_KEYS_EVENT :
                    *(p_buffer+2) = p_event_info->number;
                    _Insert_Bd_Addr_In_PDU(p_buffer+3,p_event_info->p_bd_addr);
                    LMutils_Array_Copy(LINK_KEY_SIZE,p_buffer+9,p_event_info->p_u_int8);
                    break;


                case HCI_LINK_KEY_NOTIFICATION_EVENT : /* Intentional Fall Through */
                    LMutils_Array_Copy(LINK_KEY_SIZE,p_buffer+8,p_event_info->p_u_int8);
                    *(p_buffer+24) = p_event_info->key_type;

                    /* Intentional Fall Through */
                
                case HCI_PIN_CODE_REQUEST_EVENT : 
                case HCI_LINK_KEY_REQUEST_EVENT :
                    _Insert_Bd_Addr_In_PDU(p_buffer+2,p_event_info->p_bd_addr);
                    break;

                case HCI_MAX_SLOTS_CHANGE_EVENT :
                    _Insert_Uint16(p_buffer+2,p_event_info->handle);   
                    *(p_buffer+4) = p_event_info->number;
                    break;

                case HCI_READ_CLOCK_OFFSET_EVENT :
                    _Insert_Uint16(p_buffer+3,p_event_info->handle);   
                    _Insert_Uint16(p_buffer+5,p_event_info->value16bit);  
                    break;

                case HCI_CONNECTION_PACKET_TYPE_CHANGED_EVENT :

#if (PRH_BS_CFG_SYS_HCI_V20_FUNCTIONALITY_SUPPORTED==1)
                    /* if ACL handle invert HCI_ACL_EDR packet bits, which are inverted logic on hci */
                    if (p_event_info->value16bit & HCI_ACL)
                        p_event_info->value16bit ^= HCI_ACL_EDR;
#endif
                    _Insert_Uint16(p_buffer+3,p_event_info->handle);   
                    _Insert_Uint16(p_buffer+5,p_event_info->value16bit);  
                    break;

                case HCI_READ_REMOTE_NAME_REQUEST_COMPLETE_EVENT :
#if (PRH_BS_CFG_SYS_REMOTE_NAME_REQUEST_SUPPORTED==1)
                    _Insert_Bd_Addr_In_PDU(p_buffer+3,p_event_info->p_bd_addr);  
                    p_buffer+= 9;
                    LMutils_Array_Copy(PRH_MAX_NAME_LENGTH,p_buffer,p_event_info->p_u_int8);
#endif
                    break;
					
                case HCI_DEBUG_EVENT :
                	*(p_buffer+2) = p_event_info->number;
                	{
                		u_int8 i;

                		for (i=0;i<p_event_info->number;i++)
                		{
                			p_buffer[i+3] = p_event_info->p_u_int8[i];
                		}
                	}
                	break;
					
                case HCI_DATA_BUFFER_OVERFLOW_EVENT :
                    *(p_buffer+2) = p_event_info->link_type;
                    break;

                case HCI_HARDWARE_ERROR_EVENT :
                    *(p_buffer+2) = p_event_info->number;
                    break;

#if (PRH_BS_CFG_SYS_FLOW_SPECIFICATION_SUPPORTED==1)
               case HCI_FLOW_SPECIFICATION_COMPLETE_EVENT :
                    _Insert_Uint16(p_buffer+3,(u_int16)p_event_info->handle);
                    p_link = LMaclctr_Find_Handle(p_event_info->handle);
                    *(p_buffer+5) = 0; /* Flags field current not used in Bluetooth */
                    *(p_buffer+6) = p_link->direction;
                    *(p_buffer+7) = p_link->service_type;

                    if (p_link->direction==0)
                    {
                        _Insert_Uint32(p_buffer+8,(u_int32)p_link->out_token_rate);
                        _Insert_Uint32(p_buffer+12,(u_int32)p_link->out_token_bucket_size);
                        _Insert_Uint32(p_buffer+16,(u_int32)p_link->out_peak_bandwidth);
                        _Insert_Uint32(p_buffer+20,(u_int32)p_link->out_latency);
                    }
                    else
                    {
                        _Insert_Uint32(p_buffer+8,(u_int32)p_link->in_token_rate);
                        _Insert_Uint32(p_buffer+12,(u_int32)p_link->in_token_bucket_size);
                        _Insert_Uint32(p_buffer+16,(u_int32)p_link->in_peak_bandwidth);
                        _Insert_Uint32(p_buffer+20,(u_int32)p_link->in_latency);
                    }
                    break;
#endif

#if (0 && PRH_BS_CFG_SYS_OPTIONAL_PAGING_SUPPORTED==1)
                case HCI_PAGE_SCAN_MODE_CHANGE_EVENT :
                case HCI_PAGE_SCAN_REPETITION_MODE_CHANGE_EVENT :
                    _Insert_Bd_Addr_In_PDU(p_buffer+2,p_event_info->p_bd_addr);  
                    *(p_buffer+8) = p_event_info->mode;
                    break;
#endif

#if ((PRH_BS_CFG_SYS_HCI_V12_FUNCTIONALITY_SUPPORTED==1) && (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1))
                case HCI_SYNCHRONOUS_CONNECTION_COMPLETE_EVENT:
                    _Insert_Uint16(p_buffer+3, p_event_info->handle);
                    _Insert_Bd_Addr_In_PDU(p_buffer+5, p_event_info->p_bd_addr);
                    *(p_buffer+11) = p_event_info->link_type; /* synchronous connection */
                    *(p_buffer+12) = p_event_info->transmission_interval;
                    *(p_buffer+13) = p_event_info->retransmission_window;
                    _Insert_Uint16(p_buffer+14, p_event_info->rx_packet_length);
                    _Insert_Uint16(p_buffer+16, p_event_info->tx_packet_length);
                    *(p_buffer+18) = p_event_info->air_mode;
                    break;

                case HCI_SYNCHRONOUS_CONNECTION_CHANGED_EVENT:
                    _Insert_Uint16(p_buffer+3, p_event_info->handle);
                    *(p_buffer+5) = p_event_info->transmission_interval;
                    *(p_buffer+6) = p_event_info->retransmission_window;
                    _Insert_Uint16(p_buffer+7, p_event_info->rx_packet_length);
                    _Insert_Uint16(p_buffer+9, p_event_info->tx_packet_length);
                    break;
#endif
#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUBRATING_SUPPORTED==1)
                case HCI_SNIFF_SUBRATING_EVENT:
                    _Insert_Uint16(p_buffer+3,(u_int16)p_event_info->handle);
                    _Insert_Uint16(p_buffer+5,(u_int16)(0xffff & (p_event_info->latency)>>16));
                    _Insert_Uint16(p_buffer+7,(u_int16)(0xffff & (p_event_info->latency)));
                    _Insert_Uint16(p_buffer+9,(u_int16)(0xffff & (p_event_info->token_rate)>>16));
                    _Insert_Uint16(p_buffer+11,(u_int16)(0xffff & (p_event_info->token_rate)));
                    break;
#endif

#if (PRH_BS_CFG_SYS_LSTO_CHANGED_EVENT_SUPPORTED == 1)
				case HCI_LINK_SUPERVISION_TIMEOUT_CHANGED_EVENT :
                    _Insert_Uint16(p_buffer+2,(u_int16)p_event_info->handle);
                    _Insert_Uint16(p_buffer+4,(u_int16)p_event_info->value16bit);
					break;
#endif

				case HCI_IO_CAPABILITY_REQUEST_EVENT :
#if (PRH_BS_CFG_SYS_SSP_PASSKEY_SUPPORTED==1)
				case HCI_USER_PASSKEY_REQUEST_EVENT:
#endif
#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)
				case HCI_REMOTE_OOB_DATA_REQUEST_EVENT :
#endif
					_Insert_Bd_Addr_In_PDU(p_buffer+2,p_event_info->p_bd_addr);  
					break;

				case HCI_SIMPLE_PAIRING_COMPLETE_EVENT :
                    _Insert_Bd_Addr_In_PDU(p_buffer+3,p_event_info->p_bd_addr);  
					break;

				case HCI_IO_CAPABILITY_RESPONSE_EVENT :
                    _Insert_Bd_Addr_In_PDU(p_buffer+2,p_event_info->p_bd_addr);  
					_Insert_Uint8(p_buffer+8,(u_int8)p_event_info->io_cap);
                    _Insert_Uint8(p_buffer+9,(u_int8)p_event_info->oob_data_present);
					_Insert_Uint8(p_buffer+10,(u_int8)p_event_info->auth_requirements);
					break;
#if (PRH_BS_CFG_SYS_SSP_KEYPRESS_SUPPORTED==1)
				case HCI_KEYPRESS_NOTIFICATION_EVENT :
                    _Insert_Bd_Addr_In_PDU(p_buffer+2,p_event_info->p_bd_addr);  
					_Insert_Uint8(p_buffer+8,(u_int8)p_event_info->numeric_value);
					break;
#endif
				case HCI_USER_CONFIRMATION_REQUEST_EVENT :
#if (PRH_BS_CFG_SYS_SSP_PASSKEY_SUPPORTED==1)
				case HCI_USER_PASSKEY_NOTIFICATION_EVENT :
#endif
                    _Insert_Bd_Addr_In_PDU(p_buffer+2,p_event_info->p_bd_addr);  
					_Insert_Uint32(p_buffer+8,p_event_info->numeric_value);
					break;

				

                }
#endif//BLUETOOTH_MODE_LE_ONLY
                BTq_Commit(HCI_EVENT_Q,0);

            } /* end event generator */
       } /* end if !masked */
       else if(event_code == HCI_CONNECTION_COMPLETE_EVENT)
       {
#ifndef BLUETOOTH_MODE_LE_ONLY
           /*
            * HCI Connection Complete event has been masked off.
            * Need to turn on the L2CAP data pipe.
            */
           p_link = LMaclctr_Find_Handle(p_event_info->handle);
           if(p_link)
           {
               LMch_CB_Enable_L2CAP_Traffic(p_link->device_index);
           }
#endif//BLUETOOTH_MODE_LE_ONLY
       }
   } /* end if good event code */

   return NO_ERROR;
}


/*
 * Helper function used to determine if an event has been masked.
 * Returns 0 if the event is masked off (disabled).
 * Returns a positive integeter if the event is enabled.
 */
u_int32 HCeg_Is_Event_Masked_On(u_int32 event_code)
{
    if (event_code <= HCI_PAGE_SCAN_REPETITION_MODE_CHANGE_EVENT)
    {
        return (hci_event_mask & ((u_int32)1 << (event_code-1)));
    }
    else
    {
        return (hci_event_mask_extended & ((u_int32)1 << (event_code-1-32)));
    }
}

/*
 * Helper function used to ensure the HCI_Event_Q does not overfill.
 * Returns 1 if ok.
 * Returns 0 is the q is full and a HCI_Data_Buffer_Overflow_Event is
 * being sent.
 */
u_int8 HCeg_Prevent_HCI_Event_Queue_from_Overfill(void)
{
	t_lm_event_info event_info;
    /*
     * Do not allow an Events to fill HCI_EVENT_Q completely.
	 * Send HCI_Data_Buffer_Overflow_Event just before the queue gets full.
	 * so can let other events through.
     */
    if (BTq_Get_Queue_Number_Of_Entries(HCI_EVENT_Q,0) >= 
           (PRH_BS_CFG_SYS_MAX_HCI_EVENTS-1))
    {
		event_info.link_type = ACL_LINK;

		HCeg_Generate_Event(HCI_DATA_BUFFER_OVERFLOW_EVENT, &event_info);
		return 0;
    }
	return 1;
}

