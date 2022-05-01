/*************************************************************************
 *
 * MODULE NAME:    tc_event_gen.c
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    TCI Event Generator
 * MAINTAINER:     Daire McNamara <Daire.McNamara@sslinc.com>
 * CREATION DATE:  13 April 2000
 *
 * SOURCE CONTROL: $Id: tc_event_gen.c,v 1.51 2012/09/28 11:56:05 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *
 * SOURCE: Based on V1.0 HC Design
 * ISSUES:
 * NOTES TO USERS:
 *  
 ************************************************************************/

#include "sys_config.h"

#include "hc_const.h"
#include "hc_event_gen.h"
#include "hw_radio.h"
#include "tc_const.h"
#include "tc_event_gen.h"
#include "tc_interface.h"
#include "tra_queue.h"
#include "bt_pump.h"
#include "hw_lc.h"
#include "hw_le_lc.h"

#include "hw_memcpy.h"
#include "hw_jal_defs.h"
#include "lmp_utils.h"
#include "patch_function_id.h"

#if (PRH_BS_CFG_SYS_TRACE_LLC_VIA_HCI_SUPPORTED==1)
#include "hw_lc.h"
#endif
#include "le_scan.h"
#include "le_advertise.h"
#if ((PRH_BS_CFG_SYS_TRACE_LE_DEVICE_STATE_VIA_HCI_SUPPORTED == 1) ||  (PRH_BS_CFG_SYS_TRACE_ADV_VIA_HCI_SUPPORTED==1) || \
	 (PRH_BS_CFG_SYS_TRACE_LLC_VIA_HCI_SUPPORTED==1) || (PRH_BS_CFG_SYS_TRACE_LE_DEVICE_STATE_VIA_HCI_SUPPORTED == 1) || \
	 (PRH_BS_CFG_SYS_TRACE_ATT_VIA_HCI_SUPPORTED==1) || (PRH_BS_CFG_SYS_TRACE_SMP_VIA_HCI_SUPPORTED == 1) )
#include "le_config.h"
#include "le_link_layer.h"
#include "mem_log.h"

extern void _Insert_Uint16(t_p_pdu p_buffer, u_int16 value_16_bit);
extern void _Insert_Uint32(t_p_pdu p_buffer, u_int32 value_32_bit);
extern void _Insert_Uint24(t_p_pdu p_buffer,u_int32 val24);
extern t_LE_Config LE_config;
#endif
#if (PRH_BS_CFG_BYTE_12 != 0)
/*
 * Sequence number incremented for each event placed on TCI queue
 */
static u_int8 _TCeg_seqn;
#endif

#ifndef REDUCE_ROM2
/*****************************************************************************
 *  FUNCTION :- TCeg_Command_Status_Event
 *
 *  DESCRIPTION :-
 *  This function creates an TCI Command Status Event and places it on
 *  outgoing HCI Queue. 
 *
 *****************************************************************************/
void TCeg_Command_Status_Event(t_error status, u_int16 opcode)
{
    u_int8 queue_length = 6; /* Length of a Status Event */
    t_p_pdu p_buffer = 0;
    struct q_desc* qd = 0;

    qd = BTq_Enqueue(HCI_EVENT_Q,0,queue_length);
    if(qd)
    {
        p_buffer=qd->data;
        *(p_buffer+0) = 0x0F; /* event code */
        *(p_buffer+1) = 4; /* length */
        *(p_buffer+2) = (u_int8)status;
        *(p_buffer+3) = 1;
        _Insert_Uint16(p_buffer+4, opcode);
        BTq_Commit(HCI_EVENT_Q,0);
    }
}
#endif
/*****************************************************************************
 *  FUNCTION :- TCI_Command_Complete_Event
 *
 *  DESCRIPTION :-
 *  This function creates an TCI Command Complete Event and places it on
 *  outgoing HCI Queue. 
 *
 *****************************************************************************/
void TCeg_Command_Complete_Event(u_int16 opcode, t_error status)
{
	
	if(PATCH_FUN[TCEG_COMMAND_COMPLETE_EVENT_ID]){
         ((void (*)(u_int16 opcode, t_error status))PATCH_FUN[TCEG_COMMAND_COMPLETE_EVENT_ID])(opcode,   status);
         return ;
    }

    u_int8 para_length;
    t_p_pdu p_buffer = 0;
    u_int8 size_of_header = SIZE_OF_EVENT_HEADER + SIZE_OF_COMMAND_COMPLETE_HEADER;

    struct q_desc *qd = 0;

    /* the default command complete carries only a status parameter */
    para_length=1;
    qd = BTq_Enqueue(HCI_EVENT_Q, 0, (t_length)(para_length + size_of_header));
    if(qd) 
    {
        p_buffer = qd->data;
        _Insert_Uint8(p_buffer,HCI_COMMAND_COMPLETE_EVENT);
        _Insert_Uint8(p_buffer+1,(u_int8)(para_length+SIZE_OF_COMMAND_COMPLETE_HEADER));  
        _Insert_Uint8(p_buffer+2,1);
        _Insert_Uint16(p_buffer+3,opcode);
        _Insert_Uint8(p_buffer+5,(unsigned char)status);

    BTq_Commit(HCI_EVENT_Q,0);
    }
}
#ifndef REDUCE_ROM2
/*****************************************************************************
 *  FUNCTION :- TCeg_Read_Pump_Monitors_Event
 *
 *  DESCRIPTION :-
 *  This function creates an TCI Read Pump Monitors Event and places it on
 *  outgoing HCI Queue. 
 *  The event returns all details about the data pump
 *****************************************************************************/
void TCeg_Read_Pump_Monitors_Event(t_BTpump_Log* pump)
{
    u_int8 length = 140;
    u_int8* p_buf;
    struct q_desc *qd;
    
    qd = BTq_Enqueue(HCI_EVENT_Q, 0, (t_length)(length + 2));
    if(qd)
    {
        p_buf = qd->data;
        p_buf[0] = 0xFF;          /* Reserved Event Code */
        p_buf[1] = length;        /* PDU Length          */
        p_buf[2] = 0x01;          /* pump event          */
        p_buf[3] = 0x00;          /* status              */

        LMutils_Set_Uint32(&(p_buf[4]), pump->pump_type);
        LMutils_Set_Uint32(&(p_buf[8]), pump->pump_options);
        LMutils_Set_Uint32(&(p_buf[12]), pump->packet_type);
        LMutils_Set_Uint32(&(p_buf[16]), pump->packet_length);
        LMutils_Set_Uint32(&(p_buf[20]), pump->target_device_index);
        LMutils_Set_Uint32(&(p_buf[24]), pump->handle);
        LMutils_Set_Uint32(&(p_buf[28]), pump->rx_expected_packet_num);
        LMutils_Set_Uint32(&(p_buf[32]), pump->tx_max_packets);
        LMutils_Set_Uint32(&(p_buf[36]), pump->TX_PDUS_ENQUEUED);
        LMutils_Set_Uint32(&(p_buf[40]), pump->TX_ENQUEUE_FULL_DETECTED);
        
        LMutils_Set_Uint32(&(p_buf[44]), pump->tx_start_time);
        LMutils_Set_Uint32(&(p_buf[48]), pump->tx_last_time);
        LMutils_Set_Uint32(&(p_buf[52]), pump->tx_bytes);
        LMutils_Set_Uint32(&(p_buf[56]), pump->tx_bytes_per_sec);
        LMutils_Set_Uint32(&(p_buf[60]), pump->tx_bits_per_sec);

        LMutils_Set_Uint32(&(p_buf[64]), pump->RX_PDUS_DEQUEUED);
        LMutils_Set_Uint32(&(p_buf[68]), pump->RX_CRC_ERRS_ON_DEQUEUE);
        LMutils_Set_Uint32(&(p_buf[72]), pump->RX_SEQ_ERRS_ON_DEQUEUE);
        LMutils_Set_Uint32(&(p_buf[76]), pump->RX_LAST_ERROR_PACKET_NUM_ON_DEQUEUE);

        LMutils_Set_Uint32(&(p_buf[80]), pump->rx_start_time);
        LMutils_Set_Uint32(&(p_buf[84]), pump->rx_last_time);
        LMutils_Set_Uint32(&(p_buf[88]), pump->rx_bytes);
        LMutils_Set_Uint32(&(p_buf[92]), pump->rx_bytes_per_sec);
        LMutils_Set_Uint32(&(p_buf[96]), pump->rx_bits_per_sec);
        LMutils_Set_Uint32(&(p_buf[100]), pump->rx_hw_crc_errors);
        LMutils_Set_Uint32(&(p_buf[104]), pump->rx_hw_hec_errors);
        LMutils_Set_Uint32(&(p_buf[108]), pump->rx_duplicates);
        LMutils_Set_Uint32(&(p_buf[112]), pump->rx_no_pkt_count);

        LMutils_Set_Uint32(&(p_buf[116]), pump->LSLCacc_NUM_PUMP_PDUS_TX);
        LMutils_Set_Uint32(&(p_buf[120]), pump->LSLCacc_NUM_PUMP_CRC_ERRS_BEFORE_TX);
        LMutils_Set_Uint32(&(p_buf[124]), pump->LSLCacc_NUM_PUMP_MSGTYPE_ERRS_AFTER_RX);
        LMutils_Set_Uint32(&(p_buf[128]), pump->LSLCacc_MIN_PUMP_LEN);
        LMutils_Set_Uint32(&(p_buf[132]), pump->LSLCacc_MAX_PUMP_LEN);
        LMutils_Set_Uint32(&(p_buf[136]), pump->LSLCacc_PUMP_LAST_ERROR_PACKET_NUM);
            
        BTq_Commit(HCI_EVENT_Q,0);
    }
}
#endif

#define mHWreg_Move_Cache_Register_To_Byte_Array(Register, Destination_Byte_Array) \
       {  u_int8 *dest = Destination_Byte_Array; \
          *dest++ = Cache_##Register ;     \
          *dest++ = Cache_##Register>>8 ;  \
          *dest++ = Cache_##Register>>16 ; \
          *dest = Cache_##Register>>24; }

#if (PRH_BS_CFG_SYS_TRACE_LMP_VIA_HCI_SUPPORTED==1)
/*****************************************************************************
 * TCeg_Send_LMP_Event
 * 
 * Sends the lmp message data over the HCI
 *
 * Two alternatives supported
 *
 * 1. HC_Data_Tester compatible
 *  tci_data[0]      event code     0xFF
 *  tci_data[1]      event length   19
 *  tci_data[2]      sub code       0x22
 *  tci_data[3]      direction      tx = 0x00, rx = 0x01
 *  tci_data[4:20]   lmp_data[0...]       
 *
 *  Always sends the complete 17 bytes for later processing
 *
 * 2. Software Loopback and ffs for HC_Data_Tester
 *  tci_data[0]      event code     0xFF
 *  tci_data[1]      event length   6+lmp_length
 *  tci_data[2]      sub code       tx = 0x22, rx = 0x23
 *  tci_data[3]      seqn number    8 bit tci event sequence number
 *  tci_data[4..7]   Piconet Clock
 *  tci_data[8...]   lmp_data[0...]     
 *
 *****************************************************************************/
void TCeg_Send_LMP_Event(const u_int8* lmp_data, u_int8 lmp_length, u_int8 tx0_rx1)
{
    int i;
    struct q_desc *qd_tci;

#if (BUILD_TYPE!=UNIT_TEST_BUILD)
    /*
     * If currently active then send as special Event
     */
    if (SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_TRACE_LMP_VIA_HCI_FEATURE))
    {
        qd_tci = BTq_Enqueue(HCI_EVENT_Q, 0, 21);
        if(qd_tci)
        {
           qd_tci->data[0] = 0xFF;    /* TCI event code    */
           qd_tci->data[1] = 19;      /* Max LMP msg Len   */
           qd_tci->data[2] = 0x22;    /* subcode           */
           qd_tci->data[3] = tx0_rx1; /* receive direction */
           for(i = 0; i < lmp_length; i++)
           {
               qd_tci->data[i+4] = lmp_data[i];
           }
       for(i = lmp_length; i < 17; i++)
       {
           qd_tci->data[i+4] = 0;
       }
           BTq_Commit(HCI_EVENT_Q,0);
        }
    }
#else
    u_int8 *p_tci_data;

    /*
     * If currently active then send as special Event
     */
    if (SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_TRACE_LMP_VIA_HCI_FEATURE))
    {
        /*
         * Sequence number incremented for each log attempt.
         */
        _TCeg_seqn++;

        qd_tci = BTq_Enqueue(TCI_DEBUG_Q, 0, (t_length)(8+lmp_length));
        if(qd_tci)
        {
            p_tci_data = qd_tci->data;
            *p_tci_data++ = 0xFF;              /* TCI event code    */
            *p_tci_data++ = 6+lmp_length;      /* Max LMP msg Len   */
            *p_tci_data++ = tx0_rx1 ? TCI_LMP_RX_EVENT : TCI_LMP_TX_EVENT; /* subcode*/
            *p_tci_data++ = _TCeg_seqn;        /* For detection of missed packets */

            {
                mHWreg_Create_Cache_Register(JAL_COM_CTRL_BT_CLK_READ_REG);
                mHWreg_Load_Cache_Register(JAL_COM_CTRL_BT_CLK_READ_REG);
                mHWreg_Move_Cache_Register_To_Byte_Array(
                    JAL_COM_CTRL_BT_CLK_READ_REG, p_tci_data);
                p_tci_data+=4;
            }

            for(i = 0; i < lmp_length; i++)
            {
               *p_tci_data++ = lmp_data[i];
            }
            BTq_Commit(TCI_DEBUG_Q,0);
        }
    }
#endif
}
#endif

#if (PRH_BS_CFG_SYS_TRACE_LC_VIA_HCI_SUPPORTED==1)
/*****************************************************************************
 * TCeg_Send_LC_Event
 * 
 * Sends the LC event over the HCI
 *
 *  tci_data[0]      event code     0xFF
 *  tci_data[1]      event length   12
 *  tci_data[2]      sub code       tx = 0x24, rx = 0x25
 *  tci_data[3]      seqn number    8 bit tci event sequence number
 *  tci_data[4..7]   Piconet Clock
 *  tci_data[8..9]   device state   u_int16 little endian     
 *  tci_data[10..13] arg            32 bit context argument
 *
 *****************************************************************************/
void TCeg_Send_LC_Event(t_ulc_procedure lc_event, t_state lc_state, 
                        u_int32 lc_arg, u_int8 tx0_rx1)
{
    const u_int8  lc_msg_len = 16;
    struct q_desc *qd_tci;

    u_int8 *p_tci_data;

    /*
     * If currently active then send as special Event
     */
    if (SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_TRACE_LC_VIA_HCI_FEATURE))
    {
        /*
         * Sequence number incremented for each log attempt.
         */
        _TCeg_seqn++;

        qd_tci = BTq_Enqueue(TCI_DEBUG_Q, 0, (t_length)(2 + lc_msg_len));
        if(qd_tci)
        {
            p_tci_data = qd_tci->data;
            *p_tci_data++ = 0xFF;              /* TCI event code    */
            *p_tci_data++ = lc_msg_len;        /* LC LMP msg Len   */
            *p_tci_data++ = tx0_rx1 ? TCI_LC_RX_EVENT : TCI_LC_TX_EVENT; /* subcode*/
            *p_tci_data++ = _TCeg_seqn;        /* For detection of missed packets */

            {
                mHWreg_Create_Cache_Register(JAL_COM_CTRL_BT_CLK_READ_REG);
                mHWreg_Load_Cache_Register(JAL_COM_CTRL_BT_CLK_READ_REG);
                mHWreg_Move_Cache_Register_To_Byte_Array(
                    JAL_COM_CTRL_BT_CLK_READ_REG, p_tci_data);
                p_tci_data+=4;
            }

            *p_tci_data++ = (u_int8) lc_event;
            *p_tci_data++ = (u_int8) (lc_event >> 8);

            *p_tci_data++ = (u_int8) lc_state;
            *p_tci_data++ = (u_int8) (lc_state >> 8);
            *p_tci_data++ = (u_int8) (lc_state >> 16);
            *p_tci_data++ = (u_int8) (lc_state >> 24);

            *p_tci_data++ = (u_int8) lc_arg;
            *p_tci_data++ = (u_int8) (lc_arg >> 8);
            *p_tci_data++ = (u_int8) (lc_arg >> 16);
            *p_tci_data++ = (u_int8) (lc_arg >> 24);
            BTq_Commit(TCI_DEBUG_Q,0);
        }
    }
}
#endif

#if (PRH_BS_CFG_SYS_TRACE_BB_VIA_HCI_SUPPORTED==1)
#include "hw_lc.h"
/*****************************************************************************
 * TCeg_Send_BB_Event
 * 
 * Sends the BB message data over the HCI on the separater HCI_DEBUG_Q
 *
 * Will send summary of the current BB packets Tx/Rx over the HCI.
 * Configuration will indicate which packet types will be logged.
 * This function will pick up information directly from BB interface.
 * 
 * Encoding is
 *  tci_data[0]      event code     0xFF
 *  tci_data[1]      event length   6+bb_length
 *  tci_data[2]      sub code       tx = 0x24, rx = 0x25
 *  tci_data[3]      tci number     8 bit tci event sequence number
 *  tci_data[4..7]   Piconet Clock
 *  tci_data[8...]   bb_data[0...]     
 *
 * where bb_data is:
 *  bb_data[ 0: 3]   is frequencies, Tx_Start, Tx_Mid, Rx_Start, Rx_Mid
 *  bb_data[ 4: 7]   is Common Control GP1 (0x20)
 *  bb_data[ 8:11]   is Common Control GP2 (0x24)
 *  bb_data[12:15]   is [Tx|Rx] Control Register
 *  bb_data[16:19]   is [Tx|Rx] Status Register
 *  bb_data[20:23]   is [Tx|Rx] Data Register
 * 
 * Alternative Generic (not implemented)
 * bb_data[0]    is packet type
 * bb_data[1]    is freq
 * bb_data[2]    is [b5-seqn b4-arqn b3-flow  b2:b0 am_addr] | freq2 if ID]
 * bb_data[3]    is reserved
 * bb_data[4:5]  is payload header
 * bb_data[6..]  is data
 *
 *****************************************************************************/
/*
 * 1st Word in Tx/Rx Data Register
 */
#define JAL_TX_DATA_W1_REG_ADDR            (JAL_BASE_ADDR+0x200)
#define JAL_RX_DATA_W1_REG_ADDR            (JAL_BASE_ADDR+0x400)

/*
 * Masks for logging
 */
u_int32 TCeg_log_tx_packets = 0xFFFFFFFF;
u_int32 TCeg_log_rx_packets = 0xFFFFFFFF;


void TCeg_Send_BB_Event(u_int8 tx0_rx1)
{
    const u_int16 bb_length = 36;
    struct q_desc *qd_tci;
    u_int8 *p_tci_data;
    extern t_freq _LSLCfreq_channel[4];
    t_packet packet_type;

    mHWreg_Create_Cache_Register(JAL_TX_CTRL_REG) = 0;
    mHWreg_Create_Cache_Register(JAL_RX_STATUS_GP_REG) = 0;
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_GP1_REG) = 0;

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
    mHWreg_Create_Cache_Register(JAL_EDR_CTRL_REG);
    mHWreg_Load_Cache_Register(JAL_EDR_CTRL_REG);
#endif

    /*
     * If currently active then log BB packet as a special TCI Event
     */
    if (SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_TRACE_BB_VIA_HCI_FEATURE) )
    {
        mHWreg_Load_Cache_Register(JAL_COM_CTRL_GP1_REG);

        if (tx0_rx1==0)
        {
            mHWreg_Load_Cache_Register(JAL_TX_CTRL_REG);
            packet_type = (t_packet) mHWreg_Get_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_TYPE);
            if (mHWreg_Get_Cache_Bit(JAL_COM_CTRL_GP1_REG, JAL_PAGE) )
            {
                packet_type = IDpkt; 
            }
            if ( !((1<<packet_type) & TCeg_log_tx_packets))
                return;
            /*
             * Do not log if log idle is off and this is a 'No Slot Transmit'
             */
            if (!SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_TRACE_BB_IDLE_VIA_HCI_FEATURE)
                && !mHWreg_Get_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_MODE))
                return;
        }
        else
        {
            mHWreg_Load_Cache_Register(JAL_RX_STATUS_GP_REG);

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
      packet_type = (((mHWreg_Get_Cache_Field(JAL_EDR_CTRL_REG,JAL_EDR_PTT_ACL) | mHWreg_Get_Cache_Field(JAL_EDR_CTRL_REG,JAL_EDR_PTT_ESCO))) << 4) |
		mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG,JAL_RX_TYPE);
#else
      packet_type = (t_packet) mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_TYPE);
#endif

            if (mHWreg_Get_Cache_Bit(JAL_COM_CTRL_GP1_REG, JAL_PAGE) )
            {
                packet_type = IDpkt; 
            }
            if ( !( (1<<packet_type) & TCeg_log_rx_packets))
                return;
            /*
             * Do not log if log idle is off and this is a 'Receiver Disabled'
             */
            if (!SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_TRACE_BB_IDLE_VIA_HCI_FEATURE)
                && !HW_get_rx_mode())
                return;
        }

        /*
         * Sequence number incremented for each log attempt.
         */
        _TCeg_seqn++;

        qd_tci = BTq_Enqueue(TCI_DEBUG_Q, 0, (t_length)(9+bb_length));
        if (qd_tci)
        {
            p_tci_data = qd_tci->data;
            *p_tci_data++ = 0xFF;             /* TCI event code    */
            *p_tci_data++ = 7+bb_length;      /* Max LMP msg Len   */
            *p_tci_data++ = tx0_rx1 ? TCI_BB_RX_EVENT : TCI_BB_TX_EVENT; /* subcode*/
            *p_tci_data++ = _TCeg_seqn;       /* For detection of missed packets */

            {
                mHWreg_Create_Cache_Register(JAL_COM_CTRL_BT_CLK_READ_REG);
                mHWreg_Load_Cache_Register(JAL_COM_CTRL_BT_CLK_READ_REG);
                mHWreg_Move_Cache_Register_To_Byte_Array(
                    JAL_COM_CTRL_BT_CLK_READ_REG, p_tci_data);
                p_tci_data+=4;
            }

            *p_tci_data++ = _LSLCfreq_channel[0];  /* Tx_Start   */          
            *p_tci_data++ = _LSLCfreq_channel[1];  /* Tx_Mid     */
            *p_tci_data++ = _LSLCfreq_channel[2];  /* Rx_Start   */
            *p_tci_data++ = _LSLCfreq_channel[3];  /* Rx_Mid     */
            {
                mHWreg_Create_Cache_Register(JAL_COM_CTRL_GP2_REG);
                mHWreg_Load_Cache_Register(JAL_COM_CTRL_GP2_REG);
        
                mHWreg_Move_Cache_Register_To_Byte_Array(JAL_COM_CTRL_GP1_REG, 
                    p_tci_data);
                p_tci_data+=4;

                mHWreg_Move_Cache_Register_To_Byte_Array(JAL_COM_CTRL_GP2_REG, 
                    p_tci_data);
                p_tci_data+=4;
            }

            if (tx0_rx1==0)
            {
                mHWreg_Create_Cache_Register(JAL_TX_STATUS_REG);
                mHWreg_Create_Cache_Register(JAL_TX_DATA_W1_REG);
                mHWreg_Load_Cache_Register(JAL_TX_STATUS_REG);
                mHWreg_Load_Cache_Register(JAL_TX_DATA_W1_REG);
                mHWreg_Move_Cache_Register_To_Byte_Array(JAL_TX_CTRL_REG, p_tci_data);
                p_tci_data+=4;
                mHWreg_Move_Cache_Register_To_Byte_Array(JAL_TX_STATUS_REG, p_tci_data);
                p_tci_data+=4;
                mHWreg_Move_Cache_Register_To_Byte_Array(JAL_TX_DATA_W1_REG, p_tci_data);
                p_tci_data+=4;
            }
            else
            {
                mHWreg_Create_Cache_Register(JAL_RX_CTRL_REG);
                mHWreg_Create_Cache_Register(JAL_RX_DATA_W1_REG);
                mHWreg_Load_Cache_Register(JAL_RX_CTRL_REG);
                mHWreg_Load_Cache_Register(JAL_RX_DATA_W1_REG);
                mHWreg_Move_Cache_Register_To_Byte_Array(JAL_RX_CTRL_REG, p_tci_data);
                p_tci_data+=4;
                mHWreg_Move_Cache_Register_To_Byte_Array(JAL_RX_STATUS_GP_REG, p_tci_data);
                p_tci_data+=4;
                mHWreg_Move_Cache_Register_To_Byte_Array(JAL_RX_DATA_W1_REG, p_tci_data);
                p_tci_data+=4;
            }
        {
            mHWreg_Create_Cache_Register(JAL_COM_CTRL_SYNC_LO_REG);
            mHWreg_Create_Cache_Register(JAL_COM_CTRL_SYNC_HI_REG);
			mHWreg_Create_Cache_Register(JAL_ESCO_CTRL_REG);
            mHWreg_Load_Cache_Register(JAL_COM_CTRL_SYNC_LO_REG);
            mHWreg_Load_Cache_Register(JAL_COM_CTRL_SYNC_HI_REG);
            mHWreg_Move_Cache_Register_To_Byte_Array(JAL_COM_CTRL_SYNC_LO_REG, p_tci_data);
            p_tci_data+=4;
            mHWreg_Move_Cache_Register_To_Byte_Array(JAL_COM_CTRL_SYNC_HI_REG, p_tci_data);
            p_tci_data+=4;
			mHWreg_Load_Cache_Register(JAL_ESCO_CTRL_REG);
			mHWreg_Move_Cache_Register_To_Byte_Array(JAL_ESCO_CTRL_REG, p_tci_data);
			p_tci_data+=4;
			mHWreg_Load_Cache_Register(JAL_EDR_CTRL_REG);
			mHWreg_Move_Cache_Register_To_Byte_Array(JAL_EDR_CTRL_REG, p_tci_data);
			p_tci_data+=4;
        }

           BTq_Commit(TCI_DEBUG_Q, 0);
        } /* if (qd_tci) */
    } /* if (SYSconfig_Is_Feature_Supported ... */
}
#endif

#if (PRH_BS_CFG_SYS_TRACE_RADIO_VIA_HCI_SUPPORTED==1)
/*****************************************************************************
 * TCeg_Send_Radio_Event
 * 
 * Sends the radio event message data over the HCI
 *
 *  tci_data[0]      event code     0xFF
 *  tci_data[1]      event length   9
 *  tci_data[2]      sub code       0x30
 *  tci_data[3]      seqn number    8 bit tci event sequence number
 *  tci_data[4..7]   Piconet Clock
 *  tci_data[8..9]   radio state    u_int16  little endian     
 *  tci_data[10..13] arg            32 bit context argument
 *
 *****************************************************************************/
void TCeg_Send_Radio_Event(t_tci_eg_radio_state radio_state, u_int32 radio_arg)
{
    const u_int8 radio_msg_len=12;
    struct q_desc *qd_tci;
    u_int8 *p_tci_data;

    /*
     * If currently active then send as special Event
     */
    if (SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_TRACE_RADIO_VIA_HCI_FEATURE))
    {
        /*
         * Sequence number incremented for each log attempt.
         */
        _TCeg_seqn++;

        qd_tci = BTq_Enqueue(TCI_DEBUG_Q, 0, (t_length)(radio_msg_len+2));
        if(qd_tci)
        {
            p_tci_data = qd_tci->data;
            *p_tci_data++ = 0xFF;              /* TCI event code    */
            *p_tci_data++ = radio_msg_len; 
            *p_tci_data++ = TCI_RADIO_EVENT;   /* subcode*/
            *p_tci_data++ = _TCeg_seqn;        /* For detection of missed packets */

            {
                mHWreg_Create_Cache_Register(JAL_COM_CTRL_BT_CLK_READ_REG);
                mHWreg_Load_Cache_Register(JAL_COM_CTRL_BT_CLK_READ_REG);
                mHWreg_Move_Cache_Register_To_Byte_Array(
                    JAL_COM_CTRL_BT_CLK_READ_REG, p_tci_data);
                p_tci_data+=4;
            }
            *p_tci_data++ = (u_int8) ((u_int16)radio_state & 0xFF);
            *p_tci_data++ = (u_int8) ((u_int16)radio_state >> 8);
            *p_tci_data++ = (u_int8) radio_arg;
            *p_tci_data++ = (u_int8) (radio_arg >> 8);
            *p_tci_data++ = (u_int8) (radio_arg >> 16);
            *p_tci_data++ = (u_int8) (radio_arg >> 24);

            BTq_Commit(TCI_DEBUG_Q,0);
        }
    }
}
#endif

#if (PRH_BS_CFG_SYS_TRACE_INTR_VIA_HCI_SUPPORTED==1)
/*****************************************************************************
 * TCeg_Send_Interrupt_Event
 * 
 * Sends the interrupt event message data over the HCI
 *
 *  tci_data[0]      event code         0xFF
 *  tci_data[1]      event length       8
 *  tci_data[2]      sub code           0x30
 *  tci_data[3]      seqn number        8 bit interrupt event sequence number
 *  tci_data[4..7]   Piconet Clock
 *  tci_data[8]      interrupt event    TIM0-3, PKD, PKA, NO_PKT_RCVD, 
 *                                      PKD_RX_HDR, AUX_TMR
 *  tci_data[9]      interrupt state    rf to t_tx_rx_state in lslc_slot.c
 *
 *****************************************************************************/
void TCeg_Send_Interrupt_Event(t_tci_eg_interrupt interrupt_event, u_int8 interrupt_state)
{
    const u_int8 interrupt_msg_len=8;
    struct q_desc *qd_tci;

    u_int8 *p_tci_data;

    /*
     * If currently active then send as special Event
     */
    if (SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_TRACE_INTR_VIA_HCI_FEATURE))
    {
        /*
         * Sequence number incremented for each log attempt.
         */
        _TCeg_seqn++;

        qd_tci = BTq_Enqueue(TCI_DEBUG_Q, 0, (t_length)(interrupt_msg_len+2));
        if(qd_tci)
        {
            p_tci_data = qd_tci->data;
            *p_tci_data++ = 0xFF;               /* TCI event code    */
            *p_tci_data++ = interrupt_msg_len;
            *p_tci_data++ = TCI_INTERRUPT_EVENT;/* subcode*/
            *p_tci_data++ = _TCeg_seqn;         /* For detection of missed packets */

            {
                mHWreg_Create_Cache_Register(JAL_COM_CTRL_BT_CLK_READ_REG);
                mHWreg_Load_Cache_Register(JAL_COM_CTRL_BT_CLK_READ_REG);
                mHWreg_Move_Cache_Register_To_Byte_Array(
                    JAL_COM_CTRL_BT_CLK_READ_REG, p_tci_data);
                p_tci_data+=4;
            }
            *p_tci_data++ = (u_int8) interrupt_event;
            *p_tci_data++ = (u_int8) interrupt_state;

            BTq_Commit(TCI_DEBUG_Q,0);
        }
    }
}
#endif

#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==1)
#if (PRH_BS_CFG_SYS_TRACE_LLC_VIA_HCI_SUPPORTED==1)
void TCeg_Send_LE_LLC_Event( const u_int8* le_llc_data, u_int8 le_llc_length, u_int8 tx0_rx1)
{
    int i;
    struct q_desc *qd_tci;

    /*
     * If currently active then send as special Event
     */
    if (LE_config.trace_level & 0x01) // Needs to be build based (SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_TRACE_LMP_VIA_HCI_FEATURE))
    {
        qd_tci = BTq_Enqueue(HCI_EVENT_Q, 0,8+le_llc_length);
        if(qd_tci)
        {
           qd_tci->data[0] = 0xFF;    /* TCI event code    */
           qd_tci->data[1] = 6+le_llc_length;      /* Max LLC msg Len   */
           qd_tci->data[2] = 0x29;    /* subcode           */
           qd_tci->data[3] = tx0_rx1; /* receive direction */
           for(i = 0; i < le_llc_length; i++)
           {
               qd_tci->data[i+4] = le_llc_data[i];
           }
       for(i = le_llc_length; i < 32; i++)
       {
           qd_tci->data[i+4] = 0;
       }
           BTq_Commit(HCI_EVENT_Q,0);
        }
    }
}
#endif


#if (PRH_BS_CFG_SYS_TRACE_ADV_VIA_HCI_SUPPORTED==1)
static u_int8 PKD_counter=0;
static u_int8 CRC_counter=0;

void TCeg_Send_LE_Adv_Event(u_int16 header, const u_int8* le_adv_data, u_int8 le_adv_length, u_int8 channel,u_int8 tx)
{
    int i;
    struct q_desc *qd_tci;
    u_int8 crc_error = LE_LL_Get_CRC_Err();
	if(crc_error)
		CRC_counter++;
	PKD_counter++;
	
	//BT_PRINTF("PKD_counter = %d, CRC_counter = %d\n",PKD_counter, CRC_counter);
#if 1
    if(LE_config.trace_level & 0x02)
    {
    	crc_error = LE_LL_Get_CRC_Err();

    	qd_tci = BTq_Enqueue(HCI_EVENT_Q, 0, 11+3+le_adv_length);
    	if(qd_tci)
    	{
    		qd_tci->data[0] = 0xFF;    /* TCI event code    */
    		qd_tci->data[1] = 9+le_adv_length+3;      /* Max Adv msg Len   */
    		qd_tci->data[2] = 0x23;    /* subcode           */
    		_Insert_Uint32(qd_tci->data+3,HW_get_native_clk());
    		qd_tci->data[7] = tx;
    		if ((tx==0) && (crc_error))
    			qd_tci->data[8] = 99;
    		else
    			qd_tci->data[8] = channel; /* receive direction */
    		_Insert_Uint16(qd_tci->data+9,header);
    		for(i = 0; i < le_adv_length; i++)
    		{
    			qd_tci->data[i+11] = le_adv_data[i];
    		}

    		/* Include the Backoff Count and Upper Limit */
    		if ((tx == 0x00) && (((header & 0x000F) == 0x00 /* Adv_Ind */) || ((header & 0x000F) == 0x06 /* Adv_Scan_Ind */)))
    		{
    			qd_tci->data[le_adv_length+11] = LEscan_Get_Scan_Backoff_Count();
    			_Insert_Uint16(qd_tci->data+le_adv_length+12,LEscan_Get_Scan_Upper_Limit());
    		}
    		else
    		{
    			qd_tci->data[le_adv_length+11] = 0;
    			qd_tci->data[le_adv_length+12] = 0;
    			qd_tci->data[le_adv_length+13] = 0;
    		}
    		BTq_Commit(HCI_EVENT_Q,0);
    	}
    }
#endif
}
#endif

#if (PRH_BS_CFG_SYS_TRACE_LLDATA_VIA_HCI_SUPPORTED==1)

void TCeg_Send_LE_LLData_Event(u_int16 header, const u_int8* le_ll_data, u_int8 le_lldata_length, u_int8 channel,u_int8 tx,u_int8 link_id)
{
    int i;
    struct q_desc *qd_tci;

    /*
     * If currently active then send as special Event
     */
    if(LE_config.trace_level & 0x04)
    {
    	qd_tci = BTq_Enqueue(HCI_EVENT_Q, 0, 12+le_lldata_length);
    	if(qd_tci)
    	{
    		qd_tci->data[0] = 0xFF;    /* TCI event code    */
    		qd_tci->data[1] = 10+le_lldata_length;      /* Max Adv msg Len   */
    		qd_tci->data[2] = 0x24;    /* subcode           */
    		_Insert_Uint32(qd_tci->data+3,HW_get_native_clk());
    		qd_tci->data[7] = tx;
    		qd_tci->data[8] = link_id;
    		qd_tci->data[9] = channel; /* receive direction */
    		_Insert_Uint16(qd_tci->data+10,header);
    		for(i = 0; i < le_lldata_length; i++)
    		{
    			qd_tci->data[i+12] = le_ll_data[i];
    		}

    		BTq_Commit(HCI_EVENT_Q,0);
    	}
    }
}

#endif


#if (PRH_BS_CFG_SYS_TRACE_LE_DEVICE_STATE_VIA_HCI_SUPPORTED == 1)

void TCeg_Send_LE_LEDeviceState_Event(u_int8 state, t_LE_Connection* p_connection )
{
    struct q_desc *qd_tci;

    /*
     * If currently active then send as special Event
     */

    if(LE_config.trace_level & 0x08)
    {
    	qd_tci = BTq_Enqueue(HCI_EVENT_Q, 0, 12); //+le_lldata_length);
    	if(qd_tci)
    	{
    		qd_tci->data[0] = 0xFF;    /* TCI event code    */
    		qd_tci->data[1] = 10; //+le_lldata_length;      /* Max Adv msg Len   */
    		qd_tci->data[2] = 0x25;    /* subcode           */
    		_Insert_Uint32(qd_tci->data+3,HW_get_native_clk());
    		qd_tci->data[7] = state;
    		qd_tci->data[9] = 0;
    		qd_tci->data[10] = 0;
    		qd_tci->data[11] = 0;
    		qd_tci->data[12] = 0;
    		switch(state)
    		{
    		case ADVERTISING_STATE :
				{
					u_int8 channel_map = LEadv_Get_Adv_Channel_Map();

					if(channel_map & 0x01)
						qd_tci->data[8] = 37;
					if(channel_map & 0x02)
						qd_tci->data[9] = 38;
					if(channel_map & 0x04)
						qd_tci->data[10] = 39;

					qd_tci->data[11] = LEadv_Get_Adv_Type();
				}
				break;

    		case INITIATING_STATE :
    			qd_tci->data[8] = LEscan_Get_Initiating_Channel_Index();
    			break;

    		case SCANNING_STATE :
    			qd_tci->data[8] =  LEscan_Get_Scanning_Channel_Index();
    			break;

    		case CONNECTION_STATE :
    			if (p_connection)
    			{
    				qd_tci->data[8] = (p_connection->link_id+1);
    				qd_tci->data[9] = p_connection->current_data_channel;
    				_Insert_Uint16(&qd_tci->data[10],p_connection->connection_event_counter);
    			}
    			break;

    		case TEST_STATE :

    			break;


    		}

    		BTq_Commit(HCI_EVENT_Q,0);
    	}
    }
}

#endif
#else

void TCeg_Send_LE_LLC_Event( const u_int8* le_llc_data, u_int8 le_llc_length, u_int8 tx0_rx1)
{
	;
}
void TCeg_Send_LE_Adv_Event(u_int16 header, const u_int8* le_adv_data, u_int8 le_adv_length, u_int8 channel,u_int8 tx)
{
	;
}
void TCeg_Send_LE_LLData_Event(u_int16 header, const u_int8* le_ll_data, u_int8 le_lldata_length, u_int8 channel,u_int8 tx,u_int8 link_id)
{
	;
}

void TCeg_Send_LE_LEDeviceState_Event(u_int8 state, t_LE_Connection* p_connection )
{
	;
}

#endif

