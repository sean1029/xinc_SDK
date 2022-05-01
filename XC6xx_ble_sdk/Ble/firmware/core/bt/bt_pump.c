/************************************************************************
 * MODULE NAME:    bt_pump.c
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    Routines to support Internal Host Controller Data Pumping.
 * MAINTAINER:     John Nelson
 * CREATION DATE:  06 June 2001
 *
 * SOURCE CONTROL: $Id: bt_pump.c,v 1.41 2008/03/03 11:58:00 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * Notes:
 *     Extracted from bt_test.c
 *************************************************************************/

#include "sys_config.h"
#include "sys_types.h"

#include "bt_init.h"
#include "bt_pump.h"
#include "bt_timer.h" /* for BTtimer_Is_Timing_Equation_Satisfied */

#include "tra_queue.h"
#include "tra_hcit.h"

#include "hc_event_gen.h"

#include "lmp_scan.h"
#include "lmp_ch.h"
#include "lmp_acl_container.h"
#include "lmp_con_filter.h"
#include "lmp_acl_container.h"
#include "lmp_sco_container.h"
#include "lmp_config.h"


#include "lc_interface.h"
#include "lslc_stat.h"
#include "lslc_pkt.h"
#include "lslc_access.h"
#include "hw_memcpy.h"
#include "hw_lc.h"

/**************************************************************************
 *
 * Data pump test functionality conditional on PRH_BS_CFG_SYS_DATA_PUMP_SUPPORTED
 *
 **************************************************************************/
#if (PRH_BS_CFG_SYS_DATA_PUMP_SUPPORTED==1)

/*
 * The next define allows an immediate test of Local Loopback
 */
#define BTPUMP__TEST_L2CAP_LOCAL_LOOPBACK 0

/*
 * The main data structure storing all associated test parameters and logs
 */
t_BTpump_Log BTpump_PUMP_Log[MAX_ACTIVE_DEVICE_LINKS];

/*
 * Bit-field data structure indicating active devices
 */
u_int16 BTpump_Active_Devices = 0; // bitfield of MAX_ACTIVE_DEVICE_LINKS

/*
 * DMN: Pull in the eSCO container
 *      so the pump can at least pump on
 *      one eSCO connection.
 */
extern t_sco_info sco_link_container[];

/*
 * Local helper functions
 */
t_error _BTpump_Process_Tx_Device(t_deviceIndex device, t_BTpump_Log *p_PUMP_Log);
t_error _BTpump_Process_Tx_SCO_Device(t_deviceIndex device, t_BTpump_Log *p_PUMP_Log);
t_error _BTpump_Process_Tx_eSCO_Device(t_deviceIndex device, t_BTpump_Log *p_PUMP_Log);
t_error _BTpump_Process_Rx_Device(t_deviceIndex device, t_BTpump_Log *p_PUMP_Log);
void _BTpump_Initialise_Device_Data_Pump(t_deviceIndex device, u_int32 pump_type);

/**************************************************************************
 *  FUNCTION : BTpump_Initialise
 *
 *  Initialise all appropriate fields for the data pump
 **************************************************************************/
void BTpump_Initialise(void)
{
    u_int8 device;
    for(device=0; device< sizeof(BTpump_PUMP_Log)/sizeof(BTpump_PUMP_Log[0]); ++device)
    {
        _BTpump_Initialise_Device_Data_Pump(device, PUMP_OFF);
    }
}

/**************************************************************************
 *  FUNCTION : BTpump_Reset_Device_By_Handle
 *
 *  Initialise all appropriate fields for the data pump device
 **************************************************************************/
void BTpump_Initialise_Device_By_Handle(u_int16 handle)
{
    t_deviceIndex device;

    for(device = 0; device < MAX_ACTIVE_DEVICE_LINKS; device++)
    {
        if(BTpump_PUMP_Log[device].handle == handle)
        {
            _BTpump_Initialise_Device_Data_Pump(device,PUMP_TYPES_TX|PUMP_TYPES_RX);
        }
    }
}

/**************************************************************************
 *  FUNCTION : BTpump_Auto_Actiavtions
 *
 *  Auto-activation of data pump capabilities (post BT initialisation).
 **************************************************************************/
void BTpump_Auto_Actiavtions(void)
{

#if (BTPUMP__TEST_L2CAP_LOCAL_LOOPBACK==1)
    /*
     * Support an immediate test of local loopback. If it has not been
     * requested yet, request a local loopback of the data pump.
     */
    static test_loopback_init = 0;
    if (!test_loopback_init)
    {
        /*
         * This will result in _BTpump_Execute_Data_Pump_Loopback called
         * for each call to BTpump_Generate_Data_Pump_Tx_Packet.
         */

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
        BTpump_Request_Data_Pump(PUMP_LOCAL_LOOPBACK,
            L2CAP_DATA_PUMP_CHECK_LSLC_TX | L2CAP_DATA_PUMP_CHECK_LSLC_RX,
            PUMP_LOCAL_LOOPBACK__CONNECTION_HANDLE, 
            EDR_3DH5, MAX_3DH5_USER_PDU, 1000);
#else
        BTpump_Request_Data_Pump(PUMP_LOCAL_LOOPBACK,
            L2CAP_DATA_PUMP_CHECK_LSLC_TX | L2CAP_DATA_PUMP_CHECK_LSLC_RX,
            PUMP_LOCAL_LOOPBACK__CONNECTION_HANDLE, 
            DH5, MAX_DH5_USER_PDU, 1000);
#endif

        test_loopback_init = TRUE;
    }
#endif

}

/**************************************************************************
 *  FUNCTION : _BTpump_Initialise_Device_Data_Pump
 *  INPUTS :  none  
 *  OUTPUTS : none
 *
 *  RETURNS : t_error     
 *
 *  DESCRIPTION :
 *  Initialise appropriate fields for the data pump for device
 **************************************************************************/
void _BTpump_Initialise_Device_Data_Pump(t_deviceIndex device, u_int32 pump_type)
{
    t_BTpump_Log *p_PUMP_Log;
    p_PUMP_Log = &BTpump_PUMP_Log[device];

    /*
     * If Off Then
     *    Clear all fields
     * Else If Loopback Then
     *    Initialise all rx fields
     * Else Transmit test
     *    Initialise all tx fields
     * Endif
     */

    if (pump_type == PUMP_OFF )
    {
        /*
         * Clear all fields
         */
        int i = sizeof(BTpump_PUMP_Log);
        u_int8* ptr =(u_int8*)&BTpump_PUMP_Log;
        while (--i != 0)
        {
            *ptr++ = 0;
        }
        /*
         * Mark as not active
         */
        BTpump_Active_Devices &= ~(1<<device);
    }
    else
    {
        if (pump_type & PUMP_TYPES_TX || pump_type==PUMP_LOOPBACK)
        {
            /*
             * A Tx function is selected: Clear all transmit fields
             */
            p_PUMP_Log->TX_PDUS_ENQUEUED =0;
            p_PUMP_Log->TX_ENQUEUE_FULL_DETECTED = 0;

            p_PUMP_Log->LSLCacc_NUM_PUMP_PDUS_TX = 0;
            p_PUMP_Log->LSLCacc_NUM_PUMP_CRC_ERRS_BEFORE_TX = 0;

            p_PUMP_Log->tx_start_time = 0;
            p_PUMP_Log->tx_last_time = 0;
            p_PUMP_Log->tx_bytes = 0;
            p_PUMP_Log->tx_bytes_per_sec = 0;
            p_PUMP_Log->tx_bits_per_sec = 0;
        }

        if (pump_type & PUMP_TYPES_RX)
        {
            /*
             * A Rx function is selected: Clear all receive fields
             * Autoclearance of LSLC Rx fields
             */
            p_PUMP_Log->rx_expected_packet_num = 0;
            p_PUMP_Log->RX_PDUS_DEQUEUED = 0;
            p_PUMP_Log->RX_CRC_ERRS_ON_DEQUEUE = 0;
            p_PUMP_Log->RX_SEQ_ERRS_ON_DEQUEUE = 0;
            p_PUMP_Log->RX_LAST_ERROR_PACKET_NUM_ON_DEQUEUE = 0;

            p_PUMP_Log->rx_start_time = 0;
            p_PUMP_Log->rx_last_time = 0;
            p_PUMP_Log->rx_bytes = 0;
            p_PUMP_Log->rx_bytes_per_sec = 0;
            p_PUMP_Log->rx_bits_per_sec = 0;
            p_PUMP_Log->rx_hw_crc_errors = 0;
            p_PUMP_Log->rx_hw_hec_errors = 0;
            p_PUMP_Log->rx_duplicates = 0;
            p_PUMP_Log->rx_no_pkt_count = 0;
        }
        /*
         * Mark as active
         */
        BTpump_Active_Devices |= (1<<device);

    }
}

/**************************************************************************
 *  FUNCTION : BTpump_Get_Preferred_Packet_Type
 *  INPUTS :  t_packetTypes packet_types
 *  RETURNS : t_packet preferrred_packet_type
 *
 *  DESCRIPTION :
 *  Given a list of packet types selected by the user, returns which of the
 *  packet types is the preferred one for the data pump to use.
 *
 **************************************************************************/
t_packet BTpump_Get_Preferred_Packet_Type(t_packetTypes packet_types)
{
    t_packet preferred_packet_type = 0;

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
    if (packet_types & EDR_PACKET_MASK)
    {
        if (packet_types & (1UL<<EDR_3DH5))
            preferred_packet_type = EDR_3DH5;
        else if (packet_types & (1UL<<EDR_2DH5))
            preferred_packet_type = EDR_2DH5;
        else if (packet_types & (1UL<<EDR_3DH3))
            preferred_packet_type = EDR_3DH3;
        else if (packet_types & (1UL<<EDR_2DH3))
            preferred_packet_type = EDR_2DH3;
        else if (packet_types & (1UL<<EDR_3DH1))
            preferred_packet_type = EDR_3DH1;
        else if (packet_types & (1UL<<EDR_2DH1))
            preferred_packet_type = EDR_2DH1;
        else if (packet_types & (1UL<<EDR_2EV3))
            preferred_packet_type = EDR_2EV3;
        else if (packet_types & (1UL<<EDR_3EV3))
            preferred_packet_type = EDR_3EV3;
        else if (packet_types & (1UL<<EDR_2EV5))
            preferred_packet_type = EDR_2EV5;
        else if (packet_types & (1UL<<EDR_3EV5))
            preferred_packet_type = EDR_3EV5;
    }
    else
#endif
    {
        if (packet_types & (1UL<<DH5))
            preferred_packet_type = DH5;
        else if (packet_types & (1UL<<DM5))
            preferred_packet_type = DM5;
        else if (packet_types & (1UL<<DH3))
            preferred_packet_type = DH3;
        else if (packet_types & (1UL<<DM3))
            preferred_packet_type = DM3;
        else if (packet_types & (1UL<<DH1))
            preferred_packet_type = DH1;
        else if (packet_types & (1UL<<DM1))
            preferred_packet_type = DM1;
        else if (packet_types & (1UL<<DV))
            preferred_packet_type = DV;
        else if (packet_types & (1UL<<HV1))
            preferred_packet_type = HV1;
        else if (packet_types & (1UL<<HV2))
            preferred_packet_type = HV2;
        else if (packet_types & (1UL<<HV3))
            preferred_packet_type = HV3;
        else if (packet_types & (1UL<<EV3))
            preferred_packet_type = EV3;
        else if (packet_types & (1UL<<EV4))
            preferred_packet_type = EV4;
        else if (packet_types & (1UL<<EV5))
            preferred_packet_type = EV5;
    }

    return preferred_packet_type;
}

/**************************************************************************
 *  FUNCTION : BTpump_Request_Data_Pump
 *  INPUTS :  none
 *  OUTPUTS : none
 *
 *  RETURNS : t_error
 *
 *  DESCRIPTION :
 *  Request that data pump starts and clear Tx or Rx counters
 *
 *  if Pump Role is Master
 *      if packet length is 0 then tx cycle all packet lengths
 *      else if packet length is 1 then tx cycle all packet types/lengths
 *      else tx packet length (min 9) endif
 *  if Pump Role is Slave
 *  if packet length is 0 then Receive else Loopback endif
 *
 **************************************************************************/
t_error BTpump_Request_Data_Pump(t_pump_type pump_type, u_int32 pump_options, 
        t_connectionHandle handle, t_packetTypes packet_types,
        t_length packet_length, u_int32 num_packets)
{
    t_lmp_link *p_link;
    t_BTpump_Log *p_PUMP_Log;
    t_packet preferred_packet_type = 0;

    p_link = LMaclctr_Find_ACL_Handle(handle);

    if (handle == PUMP_RESET__CONNECTION_HANDLE || pump_type == PUMP_RESET_ALL)
    {
        /*
         * Reset all devices
         */
        t_deviceIndex device_index;
        for (device_index = 0; device_index < MAX_ACTIVE_DEVICE_LINKS; ++device_index)
        {
            _BTpump_Initialise_Device_Data_Pump(device_index, (t_pump_type)PUMP_OFF);
        }    
    }
    else if (p_link || handle == PUMP_LOCAL_LOOPBACK__CONNECTION_HANDLE ||
             pump_type == PUMP_LOCAL_LOOPBACK )
    {
        /*
         * Select preferred packet in packet types
         */
        preferred_packet_type = BTpump_Get_Preferred_Packet_Type(packet_types);

        /*
         *  If Local Loopback Then Note It.
         */
        if (handle == PUMP_LOCAL_LOOPBACK__CONNECTION_HANDLE)
        {
            p_PUMP_Log = &BTpump_PUMP_Log[PUMP_LOCAL_LOOPBACK__DEVICE];
            p_PUMP_Log->pump_type |= PUMP_LOCAL_LOOPBACK;
            p_PUMP_Log->pump_options = (L2CAP_DATA_PUMP_CHECK_LSLC_TX |
                                        L2CAP_DATA_PUMP_CHECK_LSLC_RX );
            p_PUMP_Log->target_device_index = PUMP_LOCAL_LOOPBACK__DEVICE;
        }
        else
        {
            /*
             * Setup the device associated data pump as requested.
             * B1119, only allow selected packet, no default to DM1
             */
            p_PUMP_Log = &BTpump_PUMP_Log[p_link->device_index];
            p_PUMP_Log->target_device_index = p_link->device_index;
            p_PUMP_Log->pump_options = pump_options;

            /*
             * If packet types contain SCO packets or eSCO packets
             * then set p_link->packet_types to DM1. The
             * related p_sco_link will have been setup elsewhere.
             * -- this should have been done externally also?
             */
            if( (is_SYN_Packet(preferred_packet_type)) || (is_EDR_ESCO_packet(preferred_packet_type)) )
            {
                p_link->packet_types = HCI_DM1|HCI_DH1; 
            }
            else
            {
                p_link->packet_types = LMqos_LM_Get_HCI_ACL_Packet_Types(packet_types);
            }

//#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
//            if (p_link->packet_types & HCI_ACL_EDR)
//            {
//                LMch_Send_LMP_Packet_Type_Table_Req(p_link, 1);
//            }
//#endif

            if (p_link->packet_types & HCI_HIGH_RATE|HCI_ACL_3MBS_EDR)
            {
                p_link->rate = HIGH;
            } 
        }

        if ( !(pump_type & (PUMP_LOOPBACK |PUMP_RECEIVE)) /*my_role==MASTER*/)
        {
            if (pump_type == PUMP_CYCLE_PACKET_LENGTH || packet_length==0)
            {
                p_PUMP_Log->pump_type |= PUMP_CYCLE_PACKET_LENGTH;
                p_PUMP_Log->tx_max_packets =
                            SYSconst_Get_Packet_Length(preferred_packet_type) - 9 + 1;
            }
            else if (pump_type == PUMP_CYCLE_PACKET_TYPES_AND_LENGTH || packet_length==1)
            {
                p_PUMP_Log->pump_type |= PUMP_CYCLE_PACKET_TYPES_AND_LENGTH;
                p_PUMP_Log->tx_max_packets =
                            SYSconst_Get_Packet_Length(preferred_packet_type) - 9 + 1;
            }
            else
            {
                p_PUMP_Log->pump_type |= PUMP_FIXED_PACKET_LENGTH;
                p_PUMP_Log->tx_max_packets = num_packets;
            }
        }
        else if (pump_type == PUMP_RECEIVE || pump_type == PUMP_LOOPBACK)
        {
            p_PUMP_Log->pump_type = pump_type;
        }
        else if (pump_type & (PUMP_LOOPBACK |PUMP_RECEIVE))  /* As above */
        {
            if (packet_length==0)
            {
                p_PUMP_Log->pump_type = PUMP_RECEIVE;
            }
            else
            {
                p_PUMP_Log->pump_type = PUMP_LOOPBACK;
            }
        }

        /*
         * Minimum is 4 byte header, 4 byte packet number, 1 byte CRC
         */
        if (packet_length < 9 )
        {
            packet_length = 9;
        }

        p_PUMP_Log->handle = handle;
        p_PUMP_Log->packet_length = packet_length;
        p_PUMP_Log->packet_type = preferred_packet_type;

        /*
         * B1181  DVs now adopted if HV1 setup and < 9 bytes
         */
        if (preferred_packet_type >= HV1 && preferred_packet_type <= HV3)
        {
            p_PUMP_Log->pump_type |= PUMP_SCO_PACKET;
        }
        else if(is_eSCO_Packet(preferred_packet_type))
        {
            p_PUMP_Log->pump_type |= PUMP_ESCO_PACKET;
        }

        p_PUMP_Log->LSLCacc_MAX_PUMP_LEN = 0x0000;
        p_PUMP_Log->LSLCacc_MIN_PUMP_LEN = 0xFFFF;

        _BTpump_Initialise_Device_Data_Pump(
            (t_deviceIndex)p_PUMP_Log->target_device_index, pump_type);
    }
    else /* Invalid Connection Handle*/
    {
    }

    return NO_ERROR;
}


/**************************************************************************
 *  FUNCTION : BTpump_Is_Tx_Data_Pump_Active

 *  INPUTS :  none
 *  OUTPUTS : none
 *
 *  RETURNS : t_error
 *
 *  DESCRIPTION :
 *  Check if a pump is transmit active
 **************************************************************************/
u_int8 BTpump_Is_Tx_Data_Pump_Active(void)
{
   return (BTpump_Active_Devices!=0);
}

/**************************************************************************
 *  FUNCTION : BTpump_Is_Rx_Data_Pump_Active

 *  INPUTS :  none
 *  OUTPUTS : none
 *
 *  RETURNS : t_error
 *
 *  DESCRIPTION :
 *  Check if a pump is receive active
 **************************************************************************/
u_int8 BTpump_Is_Rx_Data_Pump_Active(void)
{
   return (BTpump_Active_Devices!=0);
}

/**************************************************************************
 *  FUNCTION : _BTpump_Execute_Data_Pump_Loopback

 *  INPUTS :  none
 *  OUTPUTS : none
 *
 *  RETURNS : t_error
 *
 *  DESCRIPTION :
 *      Execute the pump via a local register loopback test
 *
 *   If local loopback then just send packet straight to hardware
 *   Queues set to DH5 to support all packet lengths
 *   1. Enqueue a Tx packet via _BTpump_Process_Tx_Device()
 *   2. Move Queue Entry to Tx ACL Buffer and Verify Move
 *   3. Move Tx ACL Buffer to Rx ACL Buffer and Verify Move
 *   4. Move to Rx Queue
 *   5. Process the Rx packet via _BTpump_Process_Rx_Device()
 *
 **************************************************************************/
void _BTpump_Execute_Data_Pump_Loopback(void)
{
    struct q_desc *tx_qd, *rx_qd;
    t_length length;

    t_BTpump_Log *p_PUMP_Log = &BTpump_PUMP_Log[PUMP_LOCAL_LOOPBACK__DEVICE];
    t_deviceIndex device_index = PUMP_LOCAL_LOOPBACK__DEVICE;

    length = p_PUMP_Log->packet_length;
    _BTpump_Process_Tx_Device(device_index, p_PUMP_Log);

    tx_qd = BTq_BB_Dequeue_Next(L2CAP_OUT_Q, device_index, (1<<(p_PUMP_Log->packet_type)));
    
    if (tx_qd)
    {
        hw_memcpy((u_int8*)HW_get_tx_acl_buf_addr(), tx_qd->data, length);
        BTpump_Verify_Data_Pump_Before_Tx(device_index,
                    (u_int8 *) HW_get_tx_acl_buf_addr(), length, LCH_start);

        BTq_Ack_Last_Dequeued(L2CAP_OUT_Q, device_index, length);

        hw_memcpy((u_int8 *) HW_get_rx_acl_buf_addr(),
                  (u_int8 *) HW_get_tx_acl_buf_addr(), length);

        rx_qd = BTq_Enqueue(L2CAP_IN_Q, device_index, length);
        if (rx_qd)
        {
            rx_qd->message_type = LCH_start;
            hw_memcpy(rx_qd->data, (u_int8*)HW_get_rx_acl_buf_addr(), length);
            BTpump_Verify_Data_Pump_After_Rx(device_index,
                    (u_int8 *) HW_get_rx_acl_buf_addr(), length, LCH_start);
            BTq_Commit(L2CAP_IN_Q, device_index);
        }

        _BTpump_Process_Rx_Device(device_index, p_PUMP_Log);
    }
}


/**************************************************************************
 *
 *  FUNCTION : _BTpump_Consume_Received_eSCO_Packets
 *
 *  INPUTS :  none
 *  OUTPUTS : none
 *
 *  RETURNS : void
 *
 *  DESCRIPTION :
 *      Designed for use when using the pump to transmit eSCO
 *      packets. In this case, the peer device may or may not
 *      be sending eSCO packets to trigger the local device
 *      to send its eSCO packets.
 *      If the peer device transmits eSCO packets, the pump
 *      will sink them internally in the host controller.
 *
 **************************************************************************/
void _BTpump_Consume_Received_eSCO_Packets(void)
{
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    t_q_descr *p_rx_qd;
    t_sco_info *p_esco_link;

    p_esco_link = &(sco_link_container[0]);

    p_rx_qd = BTq_Dequeue_Next_Data_By_Length(SCO_IN_Q, LMscoctr_Get_SCO_Index(p_esco_link), 
        LMscoctr_Derive_Rx_Dequeue_Length(p_esco_link));

    if(p_rx_qd && p_rx_qd->data)
    {
        BTq_Ack_Last_Dequeued(SCO_IN_Q, LMscoctr_Get_SCO_Index(p_esco_link), 
            LMscoctr_Derive_Rx_Dequeue_Length(p_esco_link));
    }
#endif
}

/**************************************************************************
 *  FUNCTION : BTpump_Generate_Data_Pump_Tx_Packet
 *  INPUTS :  none
 *  OUTPUTS : none
 *
 *  RETURNS : t_error
 *
 *  DESCRIPTION :
 *  Determine next device for a Tx packet.
 *  Simple round robin selection.
 *
 **************************************************************************/
t_error BTpump_Generate_Data_Pump_Tx_Packet(void)
{
    t_deviceIndex device_index, stop_device_index;
    t_lmp_link*   p_link;

    /*
     * Records last_active_device_index (no need to re-initialise)
     */
    static t_deviceIndex last_active_device_index = 1;

    if (BTpump_PUMP_Log[PUMP_LOCAL_LOOPBACK__DEVICE].pump_type & PUMP_LOCAL_LOOPBACK)
    {
        _BTpump_Execute_Data_Pump_Loopback();
    }

    /*
     * If the host controller is transmitting eSCO data,
     * sink any associated incoming eSCO data from the peer
     * in the host controller.
     *
     * If the host controller is looping back eSCO data,
     * _BTpump_Execute_Data_Pump_Loopback() can handle it.
     */
    if((BTpump_PUMP_Log[1].pump_type & PUMP_ESCO_PACKET) && 
        (!(BTpump_PUMP_Log[1].pump_type & PUMP_LOOPBACK)))
    {
        _BTpump_Consume_Received_eSCO_Packets();
    }

    /*
     * Always start scheduling with the next device after the previously
     * active device  current [1..N]  implies next [2..N,1] if 1 was active.
     */
    if (last_active_device_index == MAX_ACTIVE_DEVICE_LINKS - 1)
    {
        stop_device_index = device_index = 1;
    }
    else
    {
        stop_device_index = device_index = last_active_device_index + 1;
    }

    do
    {
        p_link = LMaclctr_Find_Device_Index(device_index);
        if (p_link != 0 && (p_link->state != LMP_IDLE) )
        {
            last_active_device_index = device_index;

            _BTpump_Process_Tx_Device(device_index, &BTpump_PUMP_Log[device_index]);
            /*
             * Can escape here!
             */
        }

        /*
         * Select next device (wrap if at last device)
         */
        if (++device_index >= MAX_ACTIVE_DEVICE_LINKS)
        {
            device_index = 1;
        }
    }
    while  (device_index != stop_device_index);

    return NO_ERROR;
}

/**************************************************************************
 *  FUNCTION : _BTpump_Update_Tx_Timing
 *  INPUTS :  Pointer to Active PUMP log
 *  OUTPUTS : none
 *
 *  RETURNS : none
 *
 *  DESCRIPTION :
 *  Update the Tx Timing Counters
 **************************************************************************/
void _BTpump_Update_Tx_Timing(t_BTpump_Log *p_PUMP_Log)
{
    t_clock tx_time_stamp = LC_Get_Piconet_Clock(p_PUMP_Log->target_device_index) & 0x0FFFFFFC;

    if (p_PUMP_Log->TX_PDUS_ENQUEUED == 0)
    {
        p_PUMP_Log->tx_start_time = tx_time_stamp;
    }
    else
    {
        /*
         * Update the throughput information (note allowance for clock wrap)
         */
        t_clock tx_duration_ticks;
            p_PUMP_Log->tx_last_time = tx_time_stamp;
        tx_duration_ticks = ( (p_PUMP_Log->tx_last_time -
                               p_PUMP_Log->tx_start_time) & 0xFFFFFFF);

        /*
         * Calculate data rate, ensuring no rounding issues
         */
        if (tx_duration_ticks == 0)
        {
            p_PUMP_Log->tx_bytes_per_sec = 0;
        }
        else if (p_PUMP_Log->tx_bytes < 0xF0000000/3200)
        {
            p_PUMP_Log->tx_bytes_per_sec =
                (p_PUMP_Log->tx_bytes*3200) / tx_duration_ticks;
        }
        else
        {
            p_PUMP_Log->tx_bytes_per_sec =
                (p_PUMP_Log->tx_bytes) / (tx_duration_ticks/3200);
        }
        p_PUMP_Log->tx_bits_per_sec = p_PUMP_Log->tx_bytes_per_sec * 8;
    }
}

/*****************************************************************************
 *  FUNCTION : BTpump_Build_Tx_Payload
 *
 *  INPUTS :  l2cap_header  start packet with l2cap header
 *            pkt_length    number of bytes in packet
 *            pkt_number    seqence number of packet
 *
 *  OUTPUTS : pkt_buffer    pointer to byte buffer for packet
 *
 *  RETURNS : u_int8 * to buffer
 *
 *  DESCRIPTION :
 *  Builds a payload for the data pump in the presented buffer
 *
 *             Byte 3    Byte 2    Byte 1   Byte 0
 *    Word 0   Length 0  Length 1  CID 0    CID 1    L2CAP Length/CID
 *    Word 1   Full packet number in Bytes 7:4       i.e. sequence number
 *    Word 2   Pattern   Pattern   Pattern  Pattern
 *     ...     Pattern   Pattern   Pattern  Pattern
 *
 *    Word n   1 byte CRC <Pattern to last byte of packet - 1 >
 *
 *
 *****************************************************************************/
u_int8* BTpump_Build_Tx_Payload(boolean l2cap_header,
     u_int8 *pkt_buffer, u_int32 pkt_length, u_int32 pkt_num)
{
    u_int16 payload_length = pkt_length - 4;
    u_int16 i;
    u_int8  crc, pattern;
    u_int8  *ptr_byte;

    pattern = (pkt_num & 0x1) ? 0x55 : 0xAA;
    ptr_byte = pkt_buffer;

    if (l2cap_header)
    {
        /*
         * Setup L2CAP Length and Channel Identifier
         */      
        *ptr_byte = payload_length & 0xFF;
        crc = *ptr_byte++;
        *ptr_byte = payload_length >> 8;
        crc ^= *ptr_byte++;
        *ptr_byte = 0x41;
        crc ^= *ptr_byte++;
        *ptr_byte = 0;
        crc ^= *ptr_byte++;
    }
    else 
    {
        for(i=0; i<4; i++)  
        {
            *ptr_byte = pattern++;
            crc ^= *ptr_byte++;
        }
    }

    /*
     * Store 32 bit packet sequence number in bytes 7:4 
     */ 
    for(i=0; i<4; i++)  
    {
        *ptr_byte = (u_int8) (pkt_num >> (i<<3)) ;
        crc ^= *ptr_byte++;
    }

    /*
     * Store data bytes in remaining bytes
     */
    for(i=4; i<payload_length-1; i++)
    {
        *ptr_byte = pattern++;
        crc ^= *ptr_byte++;
    }
    *ptr_byte = crc;

    return pkt_buffer;
}

/**************************************************************************
 *  FUNCTION : _BTpump_Process_Tx_Device
 *
 *  INPUTS :  t_BTpump_Log *p_PUMP_Log  to log to be processed.
 *  OUTPUTS : none
 *
 *  RETURNS : t_error
 *
 *  DESCRIPTION :
 *  Generates a Data Pump Packet on L2CAP queue with valid L2CAP headers
 *  L2CAP length is L2CAP Payload length  (excludes Length/CID)
 *  Sequence number is the full 32-bit packet number
 *  Packet is as follows where Pattern is either 0x55 or 0xAA:
 *
 *             Byte 3    Byte 2    Byte 1   Byte 0
 *    Word 0   Length 0  Length 1  CID 0    CID 1    L2CAP Length/CID
 *    Word 1   Full packet number in Bytes 7:4       i.e. sequence number
 *    Word 2   Pattern   Pattern   Pattern  Pattern
 *     ...     Pattern   Pattern   Pattern  Pattern
 *
 *    Word n   1 byte CRC <Pattern to last byte of packet - 1 >
 *
 *    Note:
 *    Full HCI Data Packet
 *    Handle[0:11] PB Flag [12:13] BC Flag [14:15] Total Length [16:31]
 *    Payload [e.g. L2CAP message as above]
 *
 **************************************************************************/
t_error _BTpump_Process_Tx_Device(t_deviceIndex device, t_BTpump_Log *p_PUMP_Log)
{
    struct q_desc *tmp_qd;

    if (p_PUMP_Log->pump_type & PUMP_LOOPBACK)
    {
        /*
         * If loopback, all tx's are created on rx of packet.  No business here.
         */
        return NO_ERROR;
    }

#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
    if (p_PUMP_Log->pump_type & PUMP_SCO_PACKET)
    {
        /*
         * Prepare SCO Transmission based on selected packet type
         */
        return _BTpump_Process_Tx_SCO_Device(device, p_PUMP_Log);
    }
#endif

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    if(p_PUMP_Log->pump_type & PUMP_ESCO_PACKET)
    {
        /*
         * Prepare eSCO Transmission based on selected packet type
         */
        return _BTpump_Process_Tx_eSCO_Device(device, p_PUMP_Log);
    }
#endif

    if (device != p_PUMP_Log->target_device_index)
    {
        p_PUMP_Log->pump_type |= PUMP_ERROR;
    }

    if (p_PUMP_Log->TX_PDUS_ENQUEUED == p_PUMP_Log->tx_max_packets)
    {
        if (p_PUMP_Log->pump_type & PUMP_CYCLE_PACKET_TYPES_AND_LENGTH)
        {
             /*
              * Ensure that queue is completely empty on first packet before
              *  restarting with new packet type. This ensures that the
              *  existing packet type/rate are not changed until all pending
              *  tx packets are transmitted.
              */
            if (BTq_Is_Queue_Empty(L2CAP_OUT_Q, (t_deviceIndex)p_PUMP_Log->target_device_index) )
            {
                 /*
                  * Select next packet type in sequence DM1, DH1, DM3, DH3, DM5, DH5
                  * Packet length of 1 indicates use
                  * On last packet, clear all Tx activities, but maintain Rx activities
                  */
                 t_packet packet_type = (t_packet) p_PUMP_Log->packet_type;

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
                 t_packetTypes packet_type_mask = (t_packetTypes)(EDR_ACL_PACKET_MASK|BR_ACL_PACKET_MASK);
#else
                 t_packetTypes packet_type_mask = (t_packetTypes)(BR_ACL_PACKET_MASK);
#endif

                 packet_type_mask >>= packet_type;

                 if (packet_type_mask)
                 {
                     packet_type++;
                     while(0 == (packet_type_mask&1))
                     {
                         packet_type_mask >>= 1;
                         packet_type++;
                     }

                     BTpump_Request_Data_Pump(PUMP_CYCLE_PACKET_TYPES_AND_LENGTH, 0, 
                        (t_connectionHandle) p_PUMP_Log->handle, 
                        (1<<packet_type), 1, 0);
                 }
                 else
                 {
                     p_PUMP_Log->pump_type &= (PUMP_TYPES_RX);
                 }
            }
        }
        else
        {
             /*
              * tx_max_packets completed and not cycling ==> turn off Tx pump
              * but leave loopback
              */
             p_PUMP_Log->pump_type &= PUMP_TYPES_RX;
        }
    }

    else if (p_PUMP_Log->pump_type!=PUMP_OFF)
    {
        tmp_qd = BTq_Enqueue(L2CAP_OUT_Q, (t_deviceIndex)p_PUMP_Log->target_device_index,
                             (t_length) p_PUMP_Log->packet_length);

        if(!tmp_qd)
        {
            p_PUMP_Log->TX_ENQUEUE_FULL_DETECTED++;
        }
        else
        {
            BTpump_Build_Tx_Payload(TRUE /*Include valid L2CAP header*/,
                tmp_qd->data, p_PUMP_Log->packet_length, p_PUMP_Log->TX_PDUS_ENQUEUED);

            /*
             * If Direct Baseband Copy (Big Endian) Then Change Data to Big Endian
             */
            BTq_Change_Endianness_If_Required(tmp_qd->data, (u_int16)(p_PUMP_Log->packet_length));

            tmp_qd->message_type = LCH_start;

            BTq_Commit(L2CAP_OUT_Q, (t_deviceIndex)p_PUMP_Log->target_device_index);

            _BTpump_Update_Tx_Timing(p_PUMP_Log);

            p_PUMP_Log->TX_PDUS_ENQUEUED++;
            p_PUMP_Log->tx_bytes += p_PUMP_Log->packet_length;

            if ((p_PUMP_Log->pump_type & PUMP_CYCLE_PACKET_LENGTH) ||
                (p_PUMP_Log->pump_type & PUMP_CYCLE_PACKET_TYPES_AND_LENGTH) )
            {
                /*
                 * Next packet length
                 */
                 p_PUMP_Log->packet_length++;
            }
        }
    }
    return NO_ERROR;
}

#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
/**************************************************************************
 *  FUNCTION : _BTpump_Process_Tx_SCO_Device

 *  INPUTS :  t_BTpump_Log *p_PUMP_Log  to log to be processed.
 *  OUTPUTS : none
 *
 *  RETURNS : t_error
 *
 *  DESCRIPTION :
 *  Generates a SCO packet with psuedo L2CAP headers similar to ACL
 *  Sequence number is the full 32-bit packet number
 *
 *  Purpose is to act like psuedo tester for generating SCO packets
 *  Limited use due to directly accessing the hardware registers.
 *
 *  Recommended that DVs tested by adding a normal SCO HV1 connection
 *  and then transfer data
 *
 *  B1181 Normal DV Ack Processing should take place in BB
 **************************************************************************/
t_error _BTpump_Process_Tx_SCO_Device(t_deviceIndex device, t_BTpump_Log *p_PUMP_Log)
{
    /*
     * Define the sco_index to use for the pump
     */
    const u_int8 sco_index = 0;

    t_devicelink *p_dev_link = DL_Get_Device_Ref(device);
    u_int8 buffer_data[30];
    static boolean sco_packet_pending = FALSE;

    u_int32 current_time = LC_Get_Piconet_Clock(device);

    if (p_PUMP_Log->TX_PDUS_ENQUEUED == p_PUMP_Log->tx_max_packets)
    {
         p_PUMP_Log->pump_type = PUMP_OFF;
         return NO_ERROR;
    }

    /* 
     * Restrict to prepare on every 4 Rx frames for now (arbitrary) and on T2 frame
     * 
     */
    if ( (current_time & 0x0F) < 0x0E)
    {
        sco_packet_pending = FALSE;
        return NO_ERROR;
    }
    else if (!sco_packet_pending)
    {
        sco_packet_pending = TRUE;
        _BTpump_Update_Tx_Timing(p_PUMP_Log);
        p_PUMP_Log->TX_PDUS_ENQUEUED++;
        p_PUMP_Log->tx_bytes += p_PUMP_Log->packet_length;
        DL_Use_Local_Tx_Descriptor(p_dev_link);


        /*
         * Generate the SCO packet directly
         */
        DL_Set_Tx_SCO_Packet_Type(p_dev_link, (t_packet) p_PUMP_Log->packet_type);
        DL_Set_Active_SCO_Index(p_dev_link, sco_index);
        LC_Sync_Setup_SCO_CFG_Ex(sco_index,  SCO_CFG_VIA_ACL);
#if 1
        {
            u_int32 i;
            for(i=0; i<p_PUMP_Log->packet_length; i++)
            {
                buffer_data[i] = 0x10 + (u_int8) i;
            }
        }
#else
        BTpump_Build_Tx_Payload(TRUE /*Include valid L2CAP header*/,
            tmp_qd->data, p_PUMP_Log->packet_length, p_PUMP_Log->TX_PDUS_ENQUEUED);
#endif

#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==0)
        /*
         * Copy buffer immediately to hardware
         */
        hw_memcpy_byte_pairs_to_word32((void *)HW_get_tx_sco_buf_addr(),
                (void *)buffer_data, p_PUMP_Log->packet_length/10*10);
    
#else
        /*
         * When SCO via HCI is supported, must use queues
         * Packet is Acked off when sent
         */
        {
            struct q_desc* p_qD;
            p_qD = BTq_Enqueue(
                SCO_OUT_Q, sco_index, (t_length) p_PUMP_Log->packet_length);

            if(p_qD && p_qD->data)
            {
                hw_memcpy8(p_qD->data, buffer_data, p_PUMP_Log->packet_length);
                BTq_Commit(SCO_OUT_Q, sco_index);
            }
        }
#endif

        if (p_PUMP_Log->packet_type==DV)
        {
            /*
             * Ensure that build descriptor is Local Descriptor
             */
            DL_Set_Tx_Message_Type(p_dev_link, LCH_start);
            DL_Set_Tx_Packet_Type(p_dev_link, DV);
            DL_Set_Tx_Length(p_dev_link, 9);
            DL_Set_Tx_Qpdu(p_dev_link, buffer_data);
            LSLCpkt_Generate_DV(p_dev_link);
        }
        else
        {
            LSLCpkt_Generate_SCO(p_dev_link);
        }

        LSLCacc_Enable_Transmitter();
    }

    return NO_ERROR;
}
#endif

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
/**************************************************************************
 *  FUNCTION : _BTpump_Process_Tx_eSCO_Device

 *  INPUTS :  t_BTpump_Log *p_PUMP_Log  to log to be processed.
 *  OUTPUTS : none
 *
 *  RETURNS : t_error
 *
 *  DESCRIPTION :
 *  Generates an eSCO packet with psuedo L2CAP headers similar to ACL
 *  Sequence number is the full 32-bit packet number
 *
 *  Purpose is to act like psuedo tester for generating eSCO packets
 *  Limited use due to directly accessing the hardware registers.
 *
 **************************************************************************/
t_error _BTpump_Process_Tx_eSCO_Device(t_deviceIndex device, t_BTpump_Log *p_PUMP_Log)
{
    struct q_desc* p_qD;
    const u_int8 sco_index = 0;
    u_int32 i;

    t_sco_info* p_esco_link = &sco_link_container[0];

    u_int32 current_clk = (LC_Get_Piconet_Clock(device) & 0x0FFFFFFC);  /*Next Tx */

    /*
     * After transmitting eSCO packets using the pump, go to PUMP_ESCO_PACKET_COMPLETE
     * state. This (probably) is a more intuitive experience for the user.
     * I.e. the pump has been "sinking" any eSCO packets from the peer
     * device as it has been pumping eSCO packets to the peer device.
     *
     * It continues to sink eSCO packets to the device until the pump is turned off
     */
    if (p_PUMP_Log->TX_PDUS_ENQUEUED == p_PUMP_Log->tx_max_packets)
    {
         p_PUMP_Log->pump_type = PUMP_ESCO_PACKET_COMPLETE;
         return NO_ERROR;
    }

    /* 
     * Restrict to prepare on the start of every t_esco.
     */
    if (BTtimer_Is_Timing_Equation_Satisfied(current_clk, p_esco_link->timing_ctrl,
        p_esco_link->t_sco, p_esco_link->d_sco ))
    {
        _BTpump_Update_Tx_Timing(p_PUMP_Log);
        p_PUMP_Log->TX_PDUS_ENQUEUED++;
        p_PUMP_Log->tx_bytes += p_PUMP_Log->packet_length;

        /*
         * When eSCO is supported, SCO via HCI is supported.
         * Use queues
         * Packet is Acked off when sent
         */
        p_qD = BTq_Enqueue(
            SCO_OUT_Q, sco_index, (t_length) p_PUMP_Log->packet_length);

        if(p_qD && p_qD->data)
        {
            for(i=0; i<p_PUMP_Log->packet_length; i++)
            {
                p_qD->data[i] = 0x10 + (u_int8) i;
            }

            BTq_Commit(SCO_OUT_Q, sco_index);
        }
    }

    return NO_ERROR;
}
#endif

/**************************************************************************
 *  FUNCTION : BTpump_Process_Data_Pump_Rx_Packet

 *  INPUTS :  none
 *  OUTPUTS : none
 *
 *  RETURNS : t_error
 *
 *  DESCRIPTION :
 *  Process a Rx Data pump packet if one arrives.  Loopback if enabled.
 *
 **************************************************************************/
t_error BTpump_Process_Data_Pump_Rx_Packet(void)
{
    t_deviceIndex device_index;
    /*
     * Find the active devices and check if there is L2CAP input
     */
    for (device_index = 0; device_index < MAX_ACTIVE_DEVICE_LINKS; ++device_index)
    {
        t_lmp_link *p_link;
        p_link = LMaclctr_Find_Device_Index(device_index);
        if(p_link != 0 && p_link->state!=LMP_IDLE)
        {
            _BTpump_Process_Rx_Device(device_index, &BTpump_PUMP_Log[device_index]);
        }
    }

    return NO_ERROR;
}

/**************************************************************************
 *  FUNCTION : _BTpump_Process_Rx_Device

 *  INPUTS :  none
 *  OUTPUTS : none
 *
 *  RETURNS : t_error
 *
 *  DESCRIPTION :
 *  Process a Rx Data pump packet if one arrives.  Loopback if enabled.
 *  If an L2CAP start message Then new packet, else continue from previous
 *
 **************************************************************************/
t_error _BTpump_Process_Rx_Device(t_deviceIndex device, t_BTpump_Log *p_PUMP_Log)
{
    static u_int32  num_crcs_at_rx_start[MAX_ACTIVE_DEVICE_LINKS];
    static u_int32  num_hecs_at_rx_start[MAX_ACTIVE_DEVICE_LINKS];
    static u_int32  num_duplicates_at_rx_start[MAX_ACTIVE_DEVICE_LINKS];
    static u_int32  num_no_pkts_at_rx_start[MAX_ACTIVE_DEVICE_LINKS];

    static u_int16 pump_pkt_number_segments;
    static u_int8  pump_pkt_actual_crc;
    static u_int16 pump_pkt_total_length, pump_pkt_current_length;
    static u_int32 pump_pkt_sequence_number;

    u_int16 rx_pkt_length;

    struct q_desc *tmp_qd;
    struct q_desc *tmp_qd_lb = 0;
    u_int16 i;
    t_clock rx_time_stamp = LC_Get_Piconet_Clock(device) & 0x0FFFFFFC;
    t_sco_info* p_esco_link;
    boolean is_esco_pump_active = 0;

    tmp_qd = BTq_Dequeue_Next(L2CAP_IN_Q, device);

    /*
     * If there is no ACL data present, check if
     * there is any eSCO data present for loopback.
     */
    if(!tmp_qd)
    {
        p_esco_link = &(sco_link_container[0]);
        tmp_qd = BTq_Dequeue_Next(SCO_IN_Q, LMscoctr_Get_SCO_Index(p_esco_link));
        is_esco_pump_active = 1;
    }

    if(tmp_qd)
    {
        /*
         * Calculate the crc including that stored in packet (overall expect 0)
         */
        rx_pkt_length = tmp_qd->length;

        /*
         * If loopback and eSCO, loopback via SCO queues.
         */
        if( (p_PUMP_Log->pump_type & PUMP_LOOPBACK) && 
            (p_PUMP_Log->pump_type & PUMP_ESCO_PACKET))
        {
            tmp_qd_lb = BTq_Enqueue(SCO_OUT_Q, LMscoctr_Get_SCO_Index(p_esco_link), rx_pkt_length);
        }

        /*
         * else if loopback check that we can put on corresponding Tx queue
         * If so the check CRC/SEQN otherwise wait until Tx queue available
         */
        else if( p_PUMP_Log->pump_type & PUMP_LOOPBACK )
        {
            /*
             *  Loopback the previous received packet
             */
            tmp_qd_lb = BTq_Enqueue(L2CAP_OUT_Q, device, rx_pkt_length);
        }

        /*
         * If Not Loopback OR there is a loopback queue descriptor??
         */
        if( (!(p_PUMP_Log->pump_type&PUMP_LOOPBACK)) || tmp_qd_lb)
        {
            /*
             * Segmentation of PUMP L2CAP Messages by baseband is supported.
             *
             * If L2CAP start message Then
             *    Initialise the actual CRC
             *    Stored the Actual Pump Packet Length (L2CAP length + 4)
             * Endif
             *
             * Process L2CAP Start or Continue Message
             *
             * If PUMP Message Length Totally received Then
             *    Check that CRC is 0
             *    Check sequence 
             * Endif
             *
             */

            /*
             * If loopback and eSCO, loopback via SCO queues.
             */
            if( (p_PUMP_Log->pump_type & PUMP_LOOPBACK) && 
                (p_PUMP_Log->pump_type & PUMP_ESCO_PACKET))
            {
                pump_pkt_actual_crc = 0;
                pump_pkt_number_segments = 0;
                pump_pkt_current_length = 0;
                pump_pkt_total_length = rx_pkt_length;
                pump_pkt_sequence_number = ((u_int32) tmp_qd->data[3] <<24) | 
                    ((u_int32) tmp_qd->data[2]<<16) | ((u_int32) tmp_qd->data[1] << 8) | tmp_qd->data[0];
            }
            else if (tmp_qd->message_type == LCH_start)
            {
                pump_pkt_actual_crc = 0;
                pump_pkt_number_segments = 0;
                pump_pkt_current_length = 0;
                pump_pkt_total_length = 
                    (u_int16) (tmp_qd->data[0] + (tmp_qd->data[1] << 8)) + 4;
                pump_pkt_sequence_number = ((u_int32) tmp_qd->data[7] <<24) | 
                    ((u_int32) tmp_qd->data[6]<<16) | ((u_int32) tmp_qd->data[5] << 8) | tmp_qd->data[4];
            }

            for(i=0; i < rx_pkt_length; i++)
            {
                pump_pkt_actual_crc ^= tmp_qd->data[i];
            }
            pump_pkt_current_length += rx_pkt_length;
            pump_pkt_number_segments++;

            if (pump_pkt_current_length >= pump_pkt_total_length)
            {
                /*
                 * pump_pkt_actual_crc includes stored CRC and should be 0 if correct
                 */
                if(pump_pkt_actual_crc != 0)
                {
                    p_PUMP_Log->RX_LAST_ERROR_PACKET_NUM_ON_DEQUEUE = pump_pkt_sequence_number;
                    p_PUMP_Log->RX_CRC_ERRS_ON_DEQUEUE++;
                    p_PUMP_Log->rx_expected_packet_num++;
                }
                else if(pump_pkt_sequence_number == 0)
                {
                    /*
                     * Restart checking/rate timer on packet number of 0 
                     * unless overridden
                     *
                     * Due to segmentation this may be deferred and a number
                     * of segments already received.
                     * It is important to check it after CRC check to ensure
                     * that it isn't a corrupted packet sequence number.
                     */
                    if (!(p_PUMP_Log->pump_options & 
                          L2CAP_DATA_PUMP_NO_RX_RESET_ON_PACKET_0))
                    {
                        _BTpump_Initialise_Device_Data_Pump(device, PUMP_RECEIVE);
                    }

                    p_PUMP_Log->pump_type |= PUMP_RECEIVE;
                    p_PUMP_Log->rx_start_time = rx_time_stamp;
                    p_PUMP_Log->rx_expected_packet_num = 1;
                    num_crcs_at_rx_start[device] = monitors.RxCRCErrorCntr;
                    num_hecs_at_rx_start[device] = monitors.RxHECErrorCntr;
                    num_duplicates_at_rx_start[device] = monitors.RxDuplicateCntr;
                    num_no_pkts_at_rx_start[device] =  monitors.RxNoPacketCntr;
                    /*
                     * Record packets/length so far
                     * Current packet will be recorded later
                     */
                    p_PUMP_Log->rx_bytes = pump_pkt_current_length - rx_pkt_length;
                    p_PUMP_Log->RX_PDUS_DEQUEUED = pump_pkt_number_segments - 1;
                }
                else if(p_PUMP_Log->rx_expected_packet_num != pump_pkt_sequence_number)
                {
                    p_PUMP_Log->RX_LAST_ERROR_PACKET_NUM_ON_DEQUEUE = pump_pkt_sequence_number;
                    p_PUMP_Log->RX_SEQ_ERRS_ON_DEQUEUE++;
                    p_PUMP_Log->rx_expected_packet_num = pump_pkt_sequence_number + 1;
                 }
                else
                {
                    p_PUMP_Log->rx_expected_packet_num++;
                }
            }


            /*
             * Update the throughput information (note allowance for clock wrap)
             */
            {
                t_clock rx_duration_ticks;
                p_PUMP_Log->rx_last_time = rx_time_stamp;

                rx_duration_ticks =
                  ( ( p_PUMP_Log->rx_last_time - p_PUMP_Log->rx_start_time) & 0xFFFFFFF);

                /*
                 * Calculate data rate, ensuring no rounding issues
                 */
                if (rx_duration_ticks == 0)
                {
                    p_PUMP_Log->rx_bytes_per_sec = 0;
                }
                else if (p_PUMP_Log->rx_bytes < 0xF0000000/3200)
                {
                    p_PUMP_Log->rx_bytes_per_sec =
                        (p_PUMP_Log->rx_bytes*3200) / rx_duration_ticks;
                }
                else
                {
                    p_PUMP_Log->rx_bytes_per_sec =
                        (p_PUMP_Log->rx_bytes) / (rx_duration_ticks/3200);
                }
                p_PUMP_Log->rx_bits_per_sec = p_PUMP_Log->rx_bytes_per_sec * 8;

                /*
                 * Note CRC/HEC/Duplicates are totals for all active devices
                 */
                p_PUMP_Log->rx_hw_crc_errors  =
                    monitors.RxCRCErrorCntr -  num_crcs_at_rx_start[device];
                p_PUMP_Log->rx_hw_hec_errors  =
                    monitors.RxHECErrorCntr - num_hecs_at_rx_start[device];
                p_PUMP_Log->rx_duplicates =
                    monitors.RxDuplicateCntr - num_duplicates_at_rx_start[device];
                p_PUMP_Log->rx_no_pkt_count  =
                    monitors.RxNoPacketCntr - num_no_pkts_at_rx_start[device];
            }
        }

        if (p_PUMP_Log->pump_type & PUMP_LOOPBACK)
        {
            /*
             *  Loopback the previous received packet by placing on Tx queue
             */
            if(!tmp_qd_lb)
            {
                p_PUMP_Log->TX_ENQUEUE_FULL_DETECTED++;
            }
            else
            {
                for(i=0; i < rx_pkt_length; i++)
                {
                    tmp_qd_lb->data[i] = tmp_qd->data[i];
                }
                tmp_qd_lb->message_type = tmp_qd->message_type;

                /*
                 * If loopback and eSCO
                 */
                if( (p_PUMP_Log->pump_type & PUMP_LOOPBACK) && 
                    (p_PUMP_Log->pump_type & PUMP_ESCO_PACKET))
                {
                    BTq_Commit(SCO_OUT_Q, LMscoctr_Get_SCO_Index(p_esco_link));
                }
                else
                {
                    BTq_Commit(L2CAP_OUT_Q, device);
                }

                /*
                 * If Direct Baseband Copy (Big Endian) Then Change Data to Big Endian
                 *
                 * Note the BTq_Dequeue_Next converted Rx PDU to little endian
                 * Hence code below to re-swap pdu to big endian for direct copy
                 */
                BTq_Change_Endianness_If_Required(tmp_qd_lb->data, rx_pkt_length);

                _BTpump_Update_Tx_Timing(p_PUMP_Log);
                p_PUMP_Log->TX_PDUS_ENQUEUED++;
                p_PUMP_Log->tx_bytes += p_PUMP_Log->packet_length;

                /*
                 * If eSCO
                 */
                if(p_PUMP_Log->pump_type & PUMP_ESCO_PACKET)
                {
                    BTq_Ack_Last_Dequeued(SCO_IN_Q, LMscoctr_Get_SCO_Index(p_esco_link), rx_pkt_length);
                }
                else
                {
                    BTq_Ack_Last_Dequeued(L2CAP_IN_Q, device, rx_pkt_length);
                }
                p_PUMP_Log->rx_bytes += rx_pkt_length;
                p_PUMP_Log->RX_PDUS_DEQUEUED++;
            }
        }
        else
        {
            if (p_PUMP_Log->pump_options & L2CAP_DATA_PUMP_RX_TO_HCI)
            {
                /*
                 * Dispatch the packet to the HCI
                 */
                TRAhcit_Dispatch_Pending_Data();
            }
            else
            {
                /*
                 * Remove the processed packet from the queue
                 */
                /*
                 * If eSCO
                 */
                /* if(p_PUMP_Log->pump_type & PUMP_ESCO_PACKET) */
                if( is_esco_pump_active)
                {
                    BTq_Ack_Last_Dequeued(SCO_IN_Q, LMscoctr_Get_SCO_Index(p_esco_link), rx_pkt_length);
                }
                else
                {
                    BTq_Ack_Last_Dequeued(L2CAP_IN_Q, device, rx_pkt_length);
                }
            }
            p_PUMP_Log->rx_bytes += rx_pkt_length;
            p_PUMP_Log->RX_PDUS_DEQUEUED++;
        }
    }

    return NO_ERROR;
}



#if (PRH_BS_DBG_PUMP_VERIFY_HARDWARE_BEFORE_TX_AFTER_RX==1)
/**************************************************************************
 *  FUNCTION : BTpump_Verify_Data_Pump_Before_Tx

 *  INPUTS :  payload to be verified and its length
 *  OUTPUTS : none
 *
 *  RETURNS : status of verification  TX_OK or TX_ERROR
 *
 *  DESCRIPTION :
 *    Check that packet is ok after being placed in tx register
 *    by recalculating the CRC and comparing to software stored CRC
 **************************************************************************/
t_TXRXstatus BTpump_Verify_Data_Pump_Before_Tx(t_deviceIndex device,
             const u_int8 *p_payload, t_length tx_length, t_LCHmessage msg_type)
{
    t_TXRXstatus status = TX_OK;
    t_BTpump_Log *p_PUMP_Log;

    p_PUMP_Log = &BTpump_PUMP_Log[device];

    if (p_PUMP_Log->pump_options & L2CAP_DATA_PUMP_CHECK_LSLC_TX)
    {
        u_int8 rx_pkt_stored_crc, rx_pkt_actual_crc;
        u_int16 i;
        u_int32  tx_buffer_32[(MAX_USER_PDU+15)/4];
        u_int8   *tx_buffer = (u_int8 *) tx_buffer_32;

        hw_memcpy128(tx_buffer, p_payload, tx_length);

        rx_pkt_stored_crc = tx_buffer[tx_length-1];
        rx_pkt_actual_crc = tx_buffer[0];

        if (BTpump_Is_Tx_Data_Pump_Active() && msg_type == LCH_start)
        {
            for (i=1; i<tx_length-1; i++)
                rx_pkt_actual_crc ^= tx_buffer[i];

            p_PUMP_Log->LSLCacc_NUM_PUMP_PDUS_TX++;
            if(rx_pkt_actual_crc != rx_pkt_stored_crc)
            {
                p_PUMP_Log->LSLCacc_NUM_PUMP_CRC_ERRS_BEFORE_TX++;
                status = TX_UNEXPECTED_ERROR;
            }
        }
    }
    return status;
}

/**************************************************************************
 *  FUNCTION : BTpump_Verify_Data_Pump_After_Rx

 *  INPUTS :  payload to be verified and its length
 *  OUTPUTS : none
 *
 *  RETURNS : status of verification  RX_OK or RX_CRC_ERROR
 *
 *  DESCRIPTION :
 *    Check that packet is ok a fter being extracted from rx register
 *    by recalculating the CRC and comparing to software stored CRC
 *    Also check after DATA_PUMP has started that no invalid message
 *    types have been received.
 **************************************************************************/
t_TXRXstatus BTpump_Verify_Data_Pump_After_Rx(t_deviceIndex device,
             const u_int8 *p_payload, t_length rx_length,  t_LCHmessage msg_type)
{
    t_TXRXstatus status = RX_OK;
    t_BTpump_Log *p_PUMP_Log;

    p_PUMP_Log = &BTpump_PUMP_Log[device];

    if (p_PUMP_Log->pump_options & L2CAP_DATA_PUMP_CHECK_LSLC_TX)
    {
        u_int8  rx_pkt_stored_crc, rx_pkt_actual_crc;
        u_int16 i;
        u_int32 packet_num;
        u_int32 rx_buffer_32[(MAX_USER_PDU+15)/16*4];
        u_int8  *rx_buffer = (u_int8*) rx_buffer_32;

        if (msg_type == LCH_start)
        {
            if (rx_length < p_PUMP_Log->LSLCacc_MIN_PUMP_LEN)
                p_PUMP_Log->LSLCacc_MIN_PUMP_LEN = rx_length;
            if (rx_length > p_PUMP_Log->LSLCacc_MAX_PUMP_LEN)
                p_PUMP_Log->LSLCacc_MAX_PUMP_LEN = rx_length;

            hw_memcpy128(rx_buffer, p_payload, rx_length);

            rx_pkt_stored_crc = rx_buffer[rx_length-1];
            rx_pkt_actual_crc = rx_buffer[0];

            for (i=1;i<rx_length-1;i++)
               rx_pkt_actual_crc ^= rx_buffer[i];

            packet_num = ((u_int32) rx_buffer[7] << 24) |
                         ((u_int32) rx_buffer[6] << 16) |
                         ((u_int32) rx_buffer[5] <<  8) | rx_buffer[4];

            p_PUMP_Log->LSLCacc_NUM_PUMP_PDUS_RX++;

            if(rx_pkt_actual_crc != rx_pkt_stored_crc)
            {
                /*
                 *  Software CRC error then over-ride H/W valid packet received.
                 */
                p_PUMP_Log->LSLCacc_PUMP_LAST_ERROR_PACKET_NUM = packet_num;
                p_PUMP_Log->LSLCacc_NUM_PUMP_CRC_ERRS_AFTER_RX++;
                status = RX_CRC_ERROR;  /* CRC error packet */
            }
            else if ( (packet_num == 0) && (!(p_PUMP_Log->pump_options & 
                          L2CAP_DATA_PUMP_NO_RX_RESET_ON_PACKET_0)) )
            {
                /*
                 * New pump sequence (packet_num = 0 and not expected)
                 */
                p_PUMP_Log->LSLCacc_NUM_PUMP_CRC_ERRS_AFTER_RX= 0;
                p_PUMP_Log->LSLCacc_MIN_PUMP_LEN = rx_length;
                p_PUMP_Log->LSLCacc_MAX_PUMP_LEN = rx_length;
                p_PUMP_Log->LSLCacc_PUMP_LAST_ERROR_PACKET_NUM = 0;
                p_PUMP_Log->LSLCacc_NUM_PUMP_MSGTYPE_ERRS_AFTER_RX = 0;
                p_PUMP_Log->LSLCacc_NUM_PUMP_PDUS_RX = 1;
            }
        }
        else if (p_PUMP_Log->LSLCacc_NUM_PUMP_PDUS_RX > 0 && (msg_type != LMP_msg))
        {
            /*
             * Pump is running ==> should only receive L2CAP messages or LMP
             * Mark with a unique packet number
             */
            p_PUMP_Log->LSLCacc_PUMP_LAST_ERROR_PACKET_NUM = 88888888;
            p_PUMP_Log->LSLCacc_NUM_PUMP_MSGTYPE_ERRS_AFTER_RX++;
            status = RX_CRC_ERROR;
        }
    }
    return status;
}

#endif /* #if (PRH_BS_DBG_PUMP_VERIFY_HARDWARE_BEFORE_TX_AFTER_RX==1) */

#endif /* #if (PRH_BS_CFG_SYS_DATA_PUMP_SUPPORTED==1)*/

