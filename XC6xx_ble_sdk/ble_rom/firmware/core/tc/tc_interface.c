/*************************************************************************
 *
 * MODULE NAME:     tc_interface.c   
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:     test control interface implementations
 * MAINTAINER:      Daire McNamara 
 * CREATION DATE:   27th January 2000     
 *
 * SOURCE CONTROL: $Id: tc_interface.c,v 1.202 2013/10/25 16:48:50 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *     27.Jan.2000     DMN    Initial Draft - support TCI_Read_Packet_Log
 *     01.Feb.2000     IG     Build Params, Active IRQ
 *     ??.???.2000     DMN    Added more functionality and methods
 *     22.Jun.2000     IG     compressed for loops to save code size
 *
 ************************************************************************/
#include "sys_config.h"
#include <string.h>

#include "hc_const.h"

#include "bt_pump.h"
#include "bt_test.h"
#include "bt_tester.h"
#include "bt_addr_sync.h"

#include "dl_dev.h"

#include "lc_interface.h"
#include "lslc_freq.h"
#include "lslc_hop.h"
#include "lslc_stat.h"
#include "uslc_chan_ctrl.h"

#include "lmp_acl_container.h"
#include "lmp_config.h"
#include "lmp_encode_pdu.h"
#include "lmp_link_control_protocol.h"
#include "lmp_sco_container.h"

#include "hci_params.h"

#include "hw_radio.h"
#include "tc_const.h"
#include "tc_event_gen.h"
#include "tc_interface.h"
#include "tc_types.h"

#include "tra_uart.h"

#include "sys_power.h"
#include "sys_irq.h"

#include "hw_lc.h"
#include "hw_radio.h"
#include "hc_event_gen.h"
#include "hw_codec.h"
#include "hw_pta.h"

#include "lmp_cmd_disp.h"
#include "lmp_link_qos.h"

#if defined(TERASIC)
#define TCI_SPI_INTERFACE_STRESS_TEST
#include "system.h"
#include "sys_mmi.h"
#endif

#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==1)


#if (PRH_BS_CFG_SYS_LOW_POWER_MODE_SUPPORTED == 1)
boolean SYSpwr_Set_LowFrequencyOscillatorAvailable_Value(u_int8 osc_available);
#endif
/*
 * Useful constants
 */
#define TC_SIZE_OF_EVENT_HEADER  6

#if (BUILD_TYPE==UNIT_TEST_BUILD)
#include "ut_ats_iut.h"
#endif

#if (PRH_BS_CFG_SYS_DATA_PUMP_SUPPORTED==1)
extern t_BTpump_Log BTpump_PUMP_Log[MAX_ACTIVE_DEVICE_LINKS];
void _TC_Run_Pump_Tests(t_p_pdu pdu);
#endif

#if(PRH_BS_CFG_SYS_TEST_MODE_TESTER_SUPPORTED == 1)
void _TC_Control_Testmode_In_Slave(t_p_pdu pdu);
void _TC_Control_Testmode_Standalone(t_p_pdu pdu);
#endif

#if (PRH_BS_CFG_SYS_RETURN_TO_PICONET_SUPPORTED==1)
void USLCr2p_Write_R2P_Min_Search_Window(u_int8 new_window);
u_int8 USLCr2p_Read_R2P_Min_Search_Window(void);
#endif

#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
void _Send_LMP_Set_Broadcast_Scan_Window(void);
#endif

#if defined(TERASIC)
u_int32 _SYS_Main_Check_Unused_Stack_Space();
#endif
#ifndef BLUETOOTH_MODE_LE_ONLY
extern t_lmp_funct lmp_command_table[];
#endif
#if (PRH_BS_CFG_SYS_CHANNEL_ASSESSMENT_SCHEME_SUPPORTED==1)
extern u_int8 LSLCass_excessive_sco_rssi_variance;
#endif

/***************************************************************************
 * TC_Write_Local_Hardware_Register
 *
 * Takes a relative address, a mask, and a value, and sets the
 * requested register to the requested value.
 *
 * Issues:
 * If this function is called to write a cached register, the register
 * value will be overwritten by the cached value. You should ensure
 * that the register being written to by this function is not
 * a cached register.
 *
 *
 ***************************************************************************/
void TC_Write_Local_Hardware_Register(u_int8 jal_or_hab, u_int32 address, u_int32 value, u_int32 mask)
{
    t_error status = COMMAND_DISALLOWED;
    if(!((jal_or_hab & ~1) || (address & 3)))
    {
        u_int32* reg_addr = (u_int32*)((u_int32)(jal_or_hab ? JAL_BASE_ADDR:HAB_BASE_ADDR) + address);
        u_int32 reg = *reg_addr;
        reg &= ~mask; /* invert mask and AND with current contents -- zero required part of reg */
        value &= mask; /* just to be sure that value is not outside mask */
        reg |= value;
        *reg_addr = reg; /* write back reg */ 
        status = NO_ERROR;
    }
    TCeg_Command_Complete_Event(TCI_WRITE_LOCAL_HARDWARE_REGISTER, status);
}
#ifndef BLUETOOTH_MODE_LE_ONLY
/***************************************************************************
 * TC_Increase_Local_Volume
 *
 * Bluetooth HCI specification has no way of incrementing the
 * volume to this codec. This function allows the volume from the
 * codec to be increased. It calls LC_Increase_Volume, which
 * wraps a hal-specific function, allowing each codec to implement itself.
 *
 ***************************************************************************/
void TC_Increase_Local_Volume(void)
{
    LC_Increase_Volume();
    TCeg_Command_Complete_Event(TCI_INCREASE_LOCAL_VOLUME, NO_ERROR);
}

/***************************************************************************
 * TC_Decrease_Local_Volume
 *
 * Bluetooth HCI specification has no way of decrementing the
 * volume to this codec. This function allows the volume from the
 * codec to be decreased. It calls LC_Decrease_Volume, which
 * wraps a hal-specific function, allowing each codec to implement itself.
 *
 ***************************************************************************/
void TC_Decrease_Local_Volume(void)
{
    LC_Decrease_Volume();
    TCeg_Command_Complete_Event(TCI_DECREASE_LOCAL_VOLUME, NO_ERROR);
}

/***************************************************************************
 * TC_Write_Local_Link_Key_Type
 *
 * TCI function to allow changing the link key type on the fly from
 * unit key to combination key or vice versa. Should only be used
 * in the idle state.
 *
 ***************************************************************************/
void TC_Write_Local_Link_Key_Type(u_int8* auth_key_type) 
{
#if (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED==1)
    extern t_LMconfig g_LM_config_info;
	g_LM_config_info.key_type = (t_link_key_type) *auth_key_type;
#endif
    TCeg_Command_Complete_Event(TCI_WRITE_LOCAL_LINK_KEY_TYPE, NO_ERROR);
}

/***************************************************************************
 * TC_Read_Local_Link_Key_Type
 *
 * TCI function to allow reading back the default device link key type
 * (unit key to combination key). 
 *
 ***************************************************************************/
void TC_Read_Local_Link_Key_Type(void) 
{
#if (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED==1)
    extern t_LMconfig g_LM_config_info;
#endif
    t_p_pdu   p_buffer;
    t_q_descr *qd;

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 1 + TC_SIZE_OF_EVENT_HEADER );
    if(qd)
    {
        p_buffer = qd->data;
        _Insert_Uint8(p_buffer, HCI_COMMAND_COMPLETE_EVENT);
        _Insert_Uint8(p_buffer+1, 5);
        _Insert_Uint8(p_buffer+2, 1);
        _Insert_Uint16(p_buffer+3, TCI_READ_LOCAL_LINK_KEY_TYPE); 
        _Insert_Uint8(p_buffer+5, 0); /* Always good status */
#if (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED==1)
        _Insert_Uint8(p_buffer+6, g_LM_config_info.key_type);
#endif
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}


/***************************************************************************
 * TC_Write_Local_Encryption_Key_Length
 *
 * Function to allow the user to change the default encryption key
 * length. Command is for test purposes, so no range checking enabled,
 * Normal values (5 -- 16). Value in bytes.
 *
 ***************************************************************************/
void TC_Write_Local_Encryption_Key_Length(u_int8* min_enc_key_len, u_int8* max_enc_key_len) 
{
    t_error status;

    if ( ((*min_enc_key_len > 0) && (*min_enc_key_len <= LINK_KEY_SIZE)) &&
		 ((*max_enc_key_len > 0) && (*max_enc_key_len <= LINK_KEY_SIZE)) )
    {
        extern t_LMconfig g_LM_config_info;
        g_LM_config_info.min_encrypt_key_size = *min_enc_key_len;
		g_LM_config_info.max_encrypt_key_size = *max_enc_key_len;
        status = NO_ERROR;
    }
    else
    {
        status = INVALID_HCI_PARAMETERS;
    }

    TCeg_Command_Complete_Event(TCI_WRITE_LOCAL_ENCRYPTION_KEY_LENGTH, status);
}

/***************************************************************************
 * TC_Read_Local_Encryption_Key_Length
 *
 * Function to allow the user to read the default encryption key
 * length. Command is for test purposes.
 *
 ***************************************************************************/
void TC_Read_Local_Encryption_Key_Length(void) 
{
    extern t_LMconfig g_LM_config_info;
    t_p_pdu p_buffer;
    t_q_descr *qd;

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 2 + TC_SIZE_OF_EVENT_HEADER );
    if(qd)
    {
        p_buffer = qd->data;
        _Insert_Uint8(p_buffer, HCI_COMMAND_COMPLETE_EVENT);
        _Insert_Uint8(p_buffer+1, 6);
        _Insert_Uint8(p_buffer+2, 1);
        _Insert_Uint16(p_buffer+3, TCI_READ_LOCAL_ENCRYPTION_KEY_LENGTH); 
        _Insert_Uint8(p_buffer+5, 0); /* Always good status */
        _Insert_Uint8(p_buffer+6, g_LM_config_info.min_encrypt_key_size);
        _Insert_Uint8(p_buffer+7, g_LM_config_info.max_encrypt_key_size);
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}

/***************************************************************************
 * TC_Change_Radio_Modulation
 *
 * Function to allow the user to enable or disable LE radio modulation.
 *
 ***************************************************************************/

void TC_Change_Radio_Modulation(u_int8* enb_or_not) 
{
    t_error error_code = INVALID_HCI_PARAMETERS;

	if (*enb_or_not == 1)
	{
		HWradio_Switch_To_LE_Mode();
        error_code = NO_ERROR;
	}
	else if (*enb_or_not == 0)
	{
		HWradio_Switch_To_Classic_BT_Mode();
        error_code = NO_ERROR;
	}

    TCeg_Command_Complete_Event(TCI_CHANGE_RADIO_MODULATION, error_code);
}

/***************************************************************************
 * TC_Read_Radio_Modulation
 *
 * Function to allow the user to ascertain if LE radio modulation is
 * enabled or not. Command is for test purposes.
 *
 ***************************************************************************/
extern u_int8 le_mode;
void TC_Read_Radio_Modulation(void) 
{
    t_p_pdu p_buffer;
    t_q_descr *qd;

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 1 + TC_SIZE_OF_EVENT_HEADER);
    if(qd)
    {
        p_buffer = qd->data;
        _Insert_Uint8(p_buffer, HCI_COMMAND_COMPLETE_EVENT);
        _Insert_Uint8(p_buffer+1, 5);
        _Insert_Uint8(p_buffer+2, 1);
        _Insert_Uint16(p_buffer+3, TCI_READ_RADIO_MODULATION); 
        _Insert_Uint8(p_buffer+5, 0); /* Always good status */
        _Insert_Uint8(p_buffer+6, le_mode);
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}


/***************************************************************************
 * TC_Reset_Local_Pump_Monitors
 *
 * This command allows the pump monitors to be reset.
 *
 ***************************************************************************/
void TC_Reset_Local_Pump_Monitors(t_p_pdu conn_handle)
{
#if(PRH_BS_CFG_SYS_DATA_PUMP_SUPPORTED==1)
    t_BTpump_Log *log = 0;
    u_int16 handle = conn_handle[0] + (conn_handle[1] << 8);
    int i = 0;

    for(i = 0; i < MAX_ACTIVE_DEVICE_LINKS; i++)
    {
        if(BTpump_PUMP_Log[i].handle == handle)
            log = &BTpump_PUMP_Log[i];
    }
    
    if(log)
    {
        BTpump_Initialise_Device_By_Handle(handle);
        TCeg_Command_Status_Event(NO_ERROR, TCI_RESET_LOCAL_PUMP_MONITORS);
    }
    else
#endif
        TCeg_Command_Status_Event(COMMAND_DISALLOWED, TCI_RESET_LOCAL_PUMP_MONITORS);
}


/***************************************************************************
 * TC_Read_Local_Pump_Monitors
 *
 * This command returns the pump monitors for a particular device
 *
 ***************************************************************************/
void TC_Read_Local_Pump_Monitors(t_p_pdu conn_handle)
{
#if (PRH_BS_CFG_SYS_DATA_PUMP_SUPPORTED==1)
    t_BTpump_Log *log = 0;
    u_int16 handle = conn_handle[0] + (conn_handle[1] << 8);
    int i = 0;

    for(i = 0; i < MAX_ACTIVE_DEVICE_LINKS; i++)
    {
        if(BTpump_PUMP_Log[i].handle == handle)
            log = &BTpump_PUMP_Log[i];
    }

    if(log)
    {
        TCeg_Command_Status_Event(NO_ERROR, TCI_READ_LOCAL_PUMP_MONITORS);
        TCeg_Read_Pump_Monitors_Event(log);
    }
    else
#endif
        TCeg_Command_Status_Event(COMMAND_DISALLOWED, TCI_READ_LOCAL_PUMP_MONITORS);
}


/***************************************************************************
 * TC_Read_Local_Hop_Frequencies
 *
 * This command retrieves the hop frequencies used by the host controller
 * in single hop mode.
 *
 ***************************************************************************/
void TC_Read_Local_Hop_Frequencies(void)
{
    t_p_pdu p_buffer;
    t_q_descr *qd;

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 2 + TC_SIZE_OF_EVENT_HEADER );
    if(qd)
    {
        p_buffer = qd->data;
        _Insert_Uint8(p_buffer, HCI_COMMAND_COMPLETE_EVENT);
        _Insert_Uint8(p_buffer+1, 6);
        _Insert_Uint8(p_buffer+2, 1);
        _Insert_Uint16(p_buffer+3, TCI_READ_LOCAL_HOP_FREQUENCIES);
        _Insert_Uint8(p_buffer+5, 0); /* Always good status */
    _Insert_Uint8(p_buffer+6, (u_int8)SYSconfig_Get_Tx_Freq());
    _Insert_Uint8(p_buffer+7, (u_int8)SYSconfig_Get_Rx_Freq());
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}
    
/***************************************************************************
 * TC_Write_Local_Features
 *
 * If the ability to change features is compiled in, this function
 * will update the local features on the host controller.
 *
 ***************************************************************************/
void TC_Write_Local_Features(u_int8* new_features)
{
    int i = 0;
    u_int8* features = SYSconfig_Get_LMP_Features_Ref();
    for(i = 0; i < 8; i++)
        features[i] = new_features[i];

#if (PRH_BS_CFG_SYS_LMP_TIMING_INFO_SUPPORTED==1)
    if (SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_LMP_TIMING_INFO_FEATURE))
    {
        lmp_command_table[LMP_TIMING_ACCURACY_REQ] = &LMlc_LMP_Timing_Accuracy_Req;
        lmp_command_table[LMP_TIMING_ACCURACY_RES] = &LMlc_LMP_Timing_Accuracy_Res;
    }
    else
    {
       lmp_command_table[LMP_TIMING_ACCURACY_REQ] = &LMdisp_LMP_Unsupported_Command;
       lmp_command_table[LMP_TIMING_ACCURACY_RES] = &LMdisp_LMP_Unsupported_Command;
    }
#endif

#if ( (PRH_BS_CFG_SYS_LM_AUTO_RATE_SUPPORTED==1) && (PRH_BS_CFG_SYS_LMP_CHANNEL_DRIVEN_QUALITY_SUPPORTED==1))
    if (SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_LM_AUTO_RATE_FEATURE)
     && SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_LMP_CHANNEL_DRIVEN_QUALITY_FEATURE))
    {
        lmp_command_table[LMP_AUTO_RATE] = &LMqos_LMP_Auto_Rate;
    }
    else
    {
        lmp_command_table[LMP_AUTO_RATE] = &LMdisp_LMP_Ignore_Command;
    }
#endif

    TCeg_Command_Complete_Event(TCI_WRITE_LOCAL_FEATURES, NO_ERROR);
}

/***************************************************************************
 * TC_Write_Local_Extended_Features
 *
 * If the ability to change the extended features is compileted in,
 * this function will update the local extended features on the
 * host controller.
 *
 ***************************************************************************/
void TC_Write_Local_Extended_Features(u_int8* new_features)
{
    int i = 0;
    u_int8* features = SYSconfig_Get_LMP_Features_Ref();
    features += 8; /* Skip past regular features */
    for(i = 0; i < 8; i++)
        features[i] = new_features[i];

#if ( (PRH_BS_CFG_SYS_LM_AUTO_RATE_SUPPORTED==1) && (PRH_BS_CFG_SYS_LMP_CHANNEL_DRIVEN_QUALITY_SUPPORTED==1))
    if (SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_LM_AUTO_RATE_FEATURE)
     && SYSconfig_Is_Feature_Supported(PRH_BS_CFG_SYS_LMP_CHANNEL_DRIVEN_QUALITY_FEATURE))
    {
        lmp_command_table[LMP_AUTO_RATE] = &LMqos_LMP_Auto_Rate;
    }
    else
    {
        lmp_command_table[LMP_AUTO_RATE] = &LMdisp_LMP_Ignore_Command;
    }
#endif

    TCeg_Command_Complete_Event(TCI_WRITE_LOCAL_EXTENDED_FEATURES, NO_ERROR);
}

/***************************************************************************
 * TC_Read_Local_Extended_Features
 *
 * This function reads back the local extended features.
 *
 ***************************************************************************/
void TC_Read_Local_Extended_Features(void)
{
    t_p_pdu p_buffer;
    t_q_descr *qd;

    u_int8* features = SYSconfig_Get_LMP_Features_Ref();
    features += 8; /* Skip past regular features */
    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 8 + TC_SIZE_OF_EVENT_HEADER );
    if(qd)
    {
        p_buffer = qd->data;
        _Insert_Uint8(p_buffer, HCI_COMMAND_COMPLETE_EVENT);
        _Insert_Uint8(p_buffer+1, 12);
        _Insert_Uint8(p_buffer+2, 1);
        _Insert_Uint16(p_buffer+3, TCI_READ_LOCAL_EXTENDED_FEATURES); 
        _Insert_Uint8(p_buffer+5, 0); /* Always good status */
        memcpy(p_buffer+6, features, 8);
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}

/***************************************************************************
 * TC_Read_Local_Hardware_Version
 *
 * This function reads back the current hardware version.
 *
 ***************************************************************************/
void TC_Read_Local_Hardware_Version(void)
{
    t_p_pdu p_buffer;
    t_q_descr *qd;
 
    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 2 + TC_SIZE_OF_EVENT_HEADER);
    if(qd)
    {
        p_buffer = qd->data;
        _Insert_Uint8(p_buffer, HCI_COMMAND_COMPLETE_EVENT);
        _Insert_Uint8(p_buffer+1, 6);
        _Insert_Uint8(p_buffer+2, 1);
        _Insert_Uint16(p_buffer+3, TCI_READ_LOCAL_HARDWARE_VERSION); 
        _Insert_Uint8(p_buffer+5, 0); /* always good status */
        _Insert_Uint8(p_buffer+6, (u_int8)HW_get_major_revision());
        _Insert_Uint8(p_buffer+7, (u_int8)HW_get_minor_revision());
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}

/***************************************************************************
 * TC_Read_Local_Baseband_Monitors
 *
 * This function retrieves the baseband monitors
 *
 ***************************************************************************/
void TC_Read_Local_Baseband_Monitors(void)
{
    u_int16 para_length;
    t_p_pdu p_buffer;
    t_q_descr *qd;
#if(PRH_BS_DBG_LC_LSLC_STAT==1)
    int i = 0;
#endif

#if(PRH_BS_DBG_LC_LSLC_STAT==1)
    u_int8 num_packet_types = MAX_PACKET_TYPES; /* either report EDR or BR only */    
#endif

    t_LLCmonitors* p_monitors = LSLCstat_Get_Monitors();
    para_length = sizeof(*p_monitors);
    qd = BTq_Enqueue(HCI_EVENT_Q, 0, (t_length)(para_length + TC_SIZE_OF_EVENT_HEADER));
    if(qd)
    {
        p_buffer = qd->data;
        _Insert_Uint8(p_buffer, HCI_COMMAND_COMPLETE_EVENT);
        _Insert_Uint8(p_buffer+1, (u_int8)(para_length+4));
        _Insert_Uint8(p_buffer+2, 1);
        _Insert_Uint16(p_buffer+3, TCI_READ_LOCAL_BASEBAND_MONITORS);
        _Insert_Uint8(p_buffer+5, 0); /* Always good status */
        _Insert_Uint32(p_buffer+6, p_monitors->RxValidPackets);
        _Insert_Uint32(p_buffer+10, p_monitors->RxInvalidPackets);
        _Insert_Uint32(p_buffer+14, p_monitors->RxNoPacketCntr);
        _Insert_Uint32(p_buffer+18, p_monitors->RxHECErrorCntr);
        _Insert_Uint32(p_buffer+22, p_monitors->RxCRCErrorCntr);
        _Insert_Uint32(p_buffer+26, p_monitors->RxDuplicateCntr);
#if(PRH_BS_DBG_LC_LSLC_STAT==1)
        _Insert_Uint32(p_buffer+30, p_monitors->RxRestrictedCntr);
        _Insert_Uint32(p_buffer+34, p_monitors->RxAMADDRErrorCntr);
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
        for(i = 0; i < num_packet_types; i++)
            _Insert_Uint16(p_buffer+38+(i*2), p_monitors->TxPacketCntr[i]);
        for(i = 0; i < num_packet_types; i++)
            _Insert_Uint16(p_buffer+38+(num_packet_types*2)+(i*2), p_monitors->RxPacketCntr[i]);
        _Insert_Uint32(p_buffer+38+(num_packet_types*2*2), p_monitors->TxBroadcastCntr);
        _Insert_Uint32(p_buffer+38+(num_packet_types*2*2)+4, p_monitors->RxBroadcastCntr);
        _Insert_Uint32(p_buffer+38+(num_packet_types*2*2)+8, p_monitors->RxBroadcastDuplicateCntr);
        _Insert_Uint32(p_buffer+38+(num_packet_types*2*2)+12, p_monitors->TxAckSentCntr);
        _Insert_Uint32(p_buffer+38+(num_packet_types*2*2)+16, p_monitors->RxAckProcessedCntr);
#else /* use old DataTester, represent PacketCntrs as full 32 bits */
        for(i = 0; i < num_packet_types; i++)
            _Insert_Uint32(p_buffer+38+(i*4), p_monitors->TxPacketCntr[i]);
        for(i = 0; i < num_packet_types; i++)
            _Insert_Uint32(p_buffer+38+(num_packet_types*4)+(i*4), p_monitors->RxPacketCntr[i]);
        _Insert_Uint32(p_buffer+38+(num_packet_types*2*4), p_monitors->TxBroadcastCntr);
        _Insert_Uint32(p_buffer+38+(num_packet_types*2*4)+4, p_monitors->RxBroadcastCntr);
        _Insert_Uint32(p_buffer+38+(num_packet_types*2*4)+8, p_monitors->RxBroadcastDuplicateCntr);
        _Insert_Uint32(p_buffer+38+(num_packet_types*2*4)+12, p_monitors->TxAckSentCntr);
        _Insert_Uint32(p_buffer+38+(num_packet_types*2*4)+16, p_monitors->RxAckProcessedCntr);
#endif
#endif
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}


/***************************************************************************
 * TC_Increase_Remote_Power
 *
 * This function sends an LMP_increment_power PDU to the peer device
 * specified by the connection handle
 *
 ***************************************************************************/
void TC_Increase_Remote_Power(u_int16 acl_handle)
{
#ifndef BLUETOOTH_MODE_LE_ONLY
    t_lmp_link* p_link;
    t_error status = NO_ERROR;

    acl_handle = (acl_handle & 0xFFF);

    p_link = LMaclctr_Find_Handle(acl_handle);

#if (PRH_BS_CFG_SYS_LMP_POWER_CONTROL_SUPPORTED==1)
    LMlc_LM_Inc_Peer_Power(p_link,0);
#else
    status = COMMAND_DISALLOWED;
#endif
    TCeg_Command_Complete_Event(TCI_INCREASE_REMOTE_POWER, status);
#endif//BLUETOOTH_MODE_LE_ONLY
}


/***************************************************************************
 * TC_Decrease_Remote_Power
 *
 * This function sends an LMP_decrement_power PDU to the peer device
 * specified by the connection handle
 *
 ***************************************************************************/
void TC_Decrease_Remote_Power(u_int16 acl_handle)
{
#ifndef BLUETOOTH_MODE_LE_ONLY
    t_lmp_link* p_link;
    t_error status = NO_ERROR;

    acl_handle = (acl_handle & 0xFFF);

    p_link = LMaclctr_Find_Handle(acl_handle);

#if (PRH_BS_CFG_SYS_LMP_POWER_CONTROL_SUPPORTED==1)
    LMlc_LM_Dec_Peer_Power(p_link,0);
#else
    status = COMMAND_DISALLOWED;
#endif

    TCeg_Command_Complete_Event(TCI_DECREASE_REMOTE_POWER, status);
#endif//BLUETOOTH_MODE_LE_ONLY
}


/***************************************************************************
 * TC_Activate_Remote_DUT
 *
 * This command sends an LMP_test_activate PDU to the peer device
 * specified by the connection handle
 *
 ***************************************************************************/
void TC_Activate_Remote_DUT(u_int16 acl_handle)
{
#if(PRH_BS_CFG_SYS_TEST_MODE_TESTER_SUPPORTED == 1)
    t_error status;
    status = BTtmt_LMP_Send_Test_Activate(acl_handle);

    if (status == NO_ERROR)
    {
        /*
         * Command complete event is returned on LMP_accepted/LMP_not_accepted
         * being processed BTtmt_LMP_Accepted()/BTtmt_LMP_Not_Accepted().
         */
        TCeg_Command_Status_Event(NO_ERROR, TCI_ACTIVATE_REMOTE_DUT);
    }
    else
    {
        TCeg_Command_Complete_Event(TCI_ACTIVATE_REMOTE_DUT, status);
    }

#else
    TCeg_Command_Status_Event(COMMAND_DISALLOWED, TCI_ACTIVATE_REMOTE_DUT);
#endif
}

/***************************************************************************
 * TC_Perform_Test_Control
 *
 * This function distributes a test control PDU and keeps the HCI
 * command flow control happy.
 * It supports Test Control for Data Pump, Remote testmode DUT activation
 * and for standalone testmode device tests.
 *
 ***************************************************************************/
void TC_Perform_Test_Control(t_p_pdu test_control)
{
#if((PRH_BS_CFG_SYS_DATA_PUMP_SUPPORTED==1) || \
   (PRH_BS_CFG_SYS_TEST_MODE_TESTER_SUPPORTED==1))
    t_system_test_group system_test_group = test_control[11] +
       (test_control[12] << 8) + 
       (test_control[13] << 16) + 
       (test_control[14] << 24);
#endif

#if (PRH_BS_CFG_SYS_DATA_PUMP_SUPPORTED==1)
    if (system_test_group == TESTMODE_DATA_PUMP)
    {
        _TC_Run_Pump_Tests(test_control);
        TCeg_Command_Complete_Event(TCI_TEST_CONTROL, NO_ERROR);
    }
    else
#endif

#if(PRH_BS_CFG_SYS_TEST_MODE_TESTER_SUPPORTED == 1)
    if (system_test_group == TESTMODE_DUT_TESTER)
    {
        _TC_Control_Testmode_In_Slave(test_control);
    }
    else if (system_test_group == TESTMODE_TX_RX_TEST_STANDALONE)
    {
        _TC_Control_Testmode_Standalone(test_control);
    }
    else
#endif
    {
        TCeg_Command_Complete_Event(TCI_TEST_CONTROL, ILLEGAL_COMMAND);        
    }
}

/***************************************************************************
 * TC_Write_Local_Hopping_Mode
 *
 * This command sets the hopping mode of the local device, either
 * none, 79, 23 (France) or reduced.
 *
 ***************************************************************************/
void TC_Write_Local_Hopping_Mode(u_int8* mode)
{
    SYSconfig_Set_Hopping_Mode((t_rfSelection)(*mode), 0, 0);
    LSLChop_Initialise( (t_rfSelection)SYSconfig_Get_Hopping_Mode(), 
                        (t_freq)SYSconfig_Get_Rx_Freq(), 
                        (t_freq)SYSconfig_Get_Tx_Freq() );
    /*
     * Use tables for fast frequency selection
     */
    LSLChop_Build_Frequency_Table(Page_Scan,
        BDADDR_Get_UAP_LAP( SYSconfig_Get_Local_BD_Addr_Ref() ));
    /*
     * Build the inquiry scan hop table (always use the GIAC lap, uap=0)
     */
    LSLChop_Build_Frequency_Table(Inquiry_Scan, (t_uap_lap) GIAC_LAP );
    TCeg_Command_Complete_Event(TCI_WRITE_LOCAL_HOPPING_MODE, NO_ERROR);
}


/***************************************************************************
 * TC_Read_Local_Hopping_Mode
 *
 * This command reads back the current hopping mode of the host
 * controller.
 *
 ***************************************************************************/
void TC_Read_Local_Hopping_Mode(void)
{
    t_p_pdu p_buffer;
    t_q_descr *qd;

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 1 + TC_SIZE_OF_EVENT_HEADER );
    if(qd)
    {
        p_buffer = qd->data;
        _Insert_Uint8(p_buffer, HCI_COMMAND_COMPLETE_EVENT);
        _Insert_Uint8(p_buffer+1, 5);
        _Insert_Uint8(p_buffer+2, 1);
        _Insert_Uint16(p_buffer+3, TCI_READ_LOCAL_HOPPING_MODE); 
        _Insert_Uint8(p_buffer+5, 0); /* Always good status */
    _Insert_Uint8(p_buffer+6, (u_int8)SYSconfig_Get_Hopping_Mode());
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}


/***************************************************************************
 * TC_Write_Local_Whitening_Enable
 *
 * This command enables/disables whitening
 *
 ***************************************************************************/
void TC_Write_Local_Whitening_Enable(u_int8* whitening)
{
    SYSconfig_Set_Data_Whitening_Enable(*whitening);
    TCeg_Command_Complete_Event(TCI_WRITE_LOCAL_WHITENING_ENABLE, NO_ERROR);
}


/***************************************************************************
 * TC_Read_Local_Whitening_Enable
 *
 * This command reads back the whitening mode of the local host
 * controller (enabled or disabled)
 *
 ***************************************************************************/
void TC_Read_Local_Whitening_Enable(void)
{
    t_p_pdu p_buffer;
    t_q_descr *qd;

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 1 + TC_SIZE_OF_EVENT_HEADER );
    if(qd)
    {
        p_buffer = qd->data;
        _Insert_Uint8(p_buffer, HCI_COMMAND_COMPLETE_EVENT);
        _Insert_Uint8(p_buffer+1, 5);
        _Insert_Uint8(p_buffer+2, 1);
        _Insert_Uint16(p_buffer+3, TCI_READ_LOCAL_WHITENING_ENABLE); 
        _Insert_Uint8(p_buffer+5, 0); /* Always good status */
        _Insert_Uint8(p_buffer+6, (u_int8)SYSconfig_Get_Data_Whitening_Enable());
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}


/***************************************************************************
 * TC_Write_Local_Hop_Frequencies
 *
 * This command sets the TX and RX hop frequencies that the local
 * host controller must use in single hop mode
 *
 ***************************************************************************/
void TC_Write_Local_Hop_Frequencies( u_int8* frequencies)
{
    SYSconfig_Set_Hopping_Mode( (t_rfSelection)HOP_MODE_SINGLE_FREQ, *frequencies, *(frequencies+1) );
    LSLChop_Initialise( (t_rfSelection)SYSconfig_Get_Hopping_Mode(), 
                        (t_freq)SYSconfig_Get_Rx_Freq(), 
                        (t_freq)SYSconfig_Get_Tx_Freq() );
    /*
     * Use tables for fast frequency selection
     */
    LSLChop_Build_Frequency_Table(Page_Scan,
        BDADDR_Get_UAP_LAP( SYSconfig_Get_Local_BD_Addr_Ref() ));
    /*
     * Build the inquiry scan hop table (always use the GIAC lap, uap=0)
     */
    LSLChop_Build_Frequency_Table(Inquiry_Scan, (t_uap_lap) GIAC_LAP );
    TCeg_Command_Complete_Event(TCI_WRITE_LOCAL_HOP_FREQUENCIES, NO_ERROR);
}


/***************************************************************************
 * TC_Read_Local_Timing_Information
 *
 * This command reads back the local host controller's reported
 * drift and jitter
 *
 ***************************************************************************/
void TC_Read_Local_Timing_Information(void)
{
    t_p_pdu p_buffer;
    t_q_descr *qd;

    extern t_SYS_Config g_sys_config;
    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 2 + TC_SIZE_OF_EVENT_HEADER );
    if(qd)
    {
        p_buffer = qd->data;
        _Insert_Uint8(p_buffer, HCI_COMMAND_COMPLETE_EVENT);
        _Insert_Uint8(p_buffer+1, 6);
        _Insert_Uint8(p_buffer+2, 1);
        _Insert_Uint16(p_buffer+3, TCI_READ_LOCAL_TIMING_INFORMATION);
        _Insert_Uint8(p_buffer+5, 0); /* Always good status */
        _Insert_Uint8(p_buffer+6, g_sys_config.clock_drift_ppm);
        _Insert_Uint8(p_buffer+7, g_sys_config.clock_jitter_us);
            BTq_Commit(HCI_EVENT_Q, 0);
    }
}

/***************************************************************************
 * TC_Write_Local_Timing_Information
 *
 * This command sets the local device's drift and jitter
 *
 ***************************************************************************/
void TC_Write_Local_Timing_Information(u_int8* timing_info)
{
    extern t_SYS_Config g_sys_config;
    g_sys_config.clock_drift_ppm = timing_info[0];
    g_sys_config.clock_jitter_us = timing_info[1];
    TCeg_Command_Complete_Event(TCI_WRITE_LOCAL_TIMING_INFORMATION, NO_ERROR);
}

/***************************************************************************
 * TC_Read_Remote_Timing_Information
 *
 * This command sends an LMP_timing_info_req PDU to the device
 * specified by the connection handle
 *
 ***************************************************************************/
#if (PRH_BS_CFG_SYS_LMP_TIMING_INFO_SUPPORTED==1)
void TC_Read_Remote_Timing_Information(u_int8* conn_handle)
{

    t_lmp_link* p_link;

    u_int16 handle = (conn_handle[0]) + ((conn_handle[1] & 0x04) << 8);
    p_link = LMaclctr_Find_Handle(handle);
    LMlc_LM_Timing_Accuracy_Req(p_link);
    TCeg_Command_Complete_Event(TCI_READ_REMOTE_TIMING_INFORMATION, NO_ERROR);
}
#endif

/***************************************************************************
 * TC_Set_Local_Failed_Attempts_Counter
 *
 * This command sets the failed attempts counter in the security block.
 * Useful for simulating a repeated attempts security attack
 *
 ***************************************************************************/
#include "lmp_sec_core.h"
void TC_Set_Local_Failed_Attempts_Counter(u_int8* pdu)
{
    int i;
    t_bd_addr bd_addr;
    t_p_pdu p_buffer;
    t_q_descr *qd;

    for(i = 0; i < 6; i++)
    {
        bd_addr.bytes[i] = pdu[i];
    }
#ifndef BLUETOOTH_MODE_LE_ONLY
    LMsec_core_RecordAuthFailure(&bd_addr);
#endif

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, TC_SIZE_OF_EVENT_HEADER);
    if(qd)
    {
        p_buffer = qd->data;
        _Insert_Uint8(p_buffer, HCI_COMMAND_COMPLETE_EVENT);
        _Insert_Uint8(p_buffer+1, 4);
        _Insert_Uint8(p_buffer+2, 1);
        _Insert_Uint16(p_buffer+3, TCI_SET_LOCAL_FAILED_ATTEMPTS_COUNTER);
        _Insert_Uint8(p_buffer+5, 0); /* Always good status */
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}

/***************************************************************************
 * TC_Clear_Local_Failed_Attempts_Counter
 *
 * This command clears the repeated attempts counter in the security
 * block. This allows testing of the security block
 *
 ***************************************************************************/
void TC_Clear_Local_Failed_Attempts_Counter(u_int8* pdu)
{
    int i;
    t_bd_addr bd_addr;
    t_p_pdu p_buffer;
    t_q_descr *qd;

    for(i = 0; i < 6; i++)
        bd_addr.bytes[i] = pdu[i];
#ifndef BLUETOOTH_MODE_LE_ONLY
    LMsec_core_RecordAuthSuccess(&bd_addr);
#endif

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, TC_SIZE_OF_EVENT_HEADER );
    if(qd)
    {
        p_buffer = qd->data;
        _Insert_Uint8(p_buffer, HCI_COMMAND_COMPLETE_EVENT);
        _Insert_Uint8(p_buffer+1, 4);
        _Insert_Uint8(p_buffer+2, 1);
        _Insert_Uint16(p_buffer+3, TCI_CLEAR_LOCAL_FAILED_ATTEMPTS_COUNTER);
        _Insert_Uint8(p_buffer+5, 0); /* Always good status */
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}

/***************************************************************************
 * TC_Write_Local_Native_Clock
 *
 * This command sets the native clock
 *
 ***************************************************************************/
void TC_Write_Local_Native_Clock(u_int8* pdu)
{
    t_q_descr *qd;

    int new_clock = pdu[0] + (pdu[1] << 8) + (pdu[2] << 16) + ((pdu[3] & 0x0F ) << 24);
    HW_set_native_clk(new_clock);

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 6);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 4;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_WRITE_LOCAL_NATIVE_CLOCK);
        qd->data[5] = 0;
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}

/***************************************************************************
 * TC_Read_Local_Native_Clock
 *
 * This command reads back the current value of the native clock
 *
 ***************************************************************************/
void TC_Read_Local_Native_Clock(void)
{
    t_q_descr *qd;

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 10);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 8;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_READ_LOCAL_NATIVE_CLOCK);
        qd->data[5] = 0;
        _Insert_Uint32(&(qd->data[6]), HW_get_native_clk());
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}


/***************************************************************************
 * TC_Read_Local_Relative_Host_Controller_Mips
 *
 * This command reads the host controller speed relative to a chimera
 *
 ***************************************************************************/
void TC_Read_Local_Relative_Host_Controller_Mips(void)
{
    /*
     * hw_delay in hw_delay.c   
     */
    extern u_int32 hw_delay_counter;
    t_q_descr *qd;

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 10);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 8;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_READ_LOCAL_HOST_CONTROLLER_RELATIVE_MIPS);
        qd->data[5] = 0;
        _Insert_Uint32(&(qd->data[6]), hw_delay_counter);
        BTq_Commit(HCI_EVENT_Q, 0);
    }

}

/***************************************************************************
 * TC_Read_Local_Default_Packet_Type
 *
 * This command reads back the default packet type (usually either
 * POLL or NULL)
 *
 ***************************************************************************/
void TC_Read_Local_Default_Packet_Type(void)
{
#ifndef BLUETOOTH_MODE_LE_ONLY
    t_q_descr *qd;

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 7);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 5;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_READ_LOCAL_DEFAULT_PACKET_TYPE);
        qd->data[5] = 0;
        qd->data[6] = LMconfig_Get_Default_Pkt_Type();
        BTq_Commit(HCI_EVENT_Q, 0);
    }
#endif//BLUETOOTH_MODE_LE_ONLY
}

/***************************************************************************
 * TC_Write_Local_Default_Packet_Type
 *
 * This command sets the default packet type (normally either POLL or
 * NULL (however, as it is a test command, any packet type can be set)
 *
 ***************************************************************************/
void TC_Write_Local_Default_Packet_Type(u_int8 packet_type)
{
#ifndef BLUETOOTH_MODE_LE_ONLY
    t_q_descr *qd;

    LMconfig_Change_Default_Pkt_Type((t_packet)packet_type);

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 6);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 4;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_WRITE_LOCAL_DEFAULT_PACKET_TYPE);
        qd->data[5] = 0;
        BTq_Commit(HCI_EVENT_Q, 0);
    }
#endif//BLUETOOTH_MODE_LE_ONLY

}



/***************************************************************************
 * TC_Set_Local_Next_Available_Am_Addr
 *
 * This command changes the default AM_ADDR allocation scheme.
 *
 ***************************************************************************/
void TC_Set_Local_Next_Available_Am_Addr(t_am_addr next_am_addr)
{
    extern u_int8 DL_amaddr_pool;
    t_q_descr *qd;
    int i;
    
    DL_amaddr_pool = 0 ;
    for(i= next_am_addr; i < 8; i++)
    {
        DL_amaddr_pool |= (1 << i);
    }

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 6);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 4;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_SET_LOCAL_NEXT_AVAILABLE_AM_ADDR);
        qd->data[5] = NO_ERROR;
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}

/***************************************************************************
 * TC_Set_Local_Bd_Addr
 *
 * This command allows the setting of the local device address.
 * A HCI reset is required after this setting.
 *
 ***************************************************************************/
void TC_Set_Local_Bd_Addr(u_int8* pdu)
{
    t_q_descr *qd;
    t_bd_addr bd_addr;
    extern t_SYS_Config g_sys_config;
    t_syncword *p_syncword;

    BDADDR_Assign_from_Byte_Array(&bd_addr, pdu);
    SYSconfig_Set_Local_BD_Addr(bd_addr);
    p_syncword = &g_sys_config.local_device_syncword;
    BTaddr_Build_Sync_Word(BDADDR_Get_LAP(&g_sys_config.local_device_address),
        &p_syncword->high, &p_syncword->low);

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 6);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 4;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_SET_LOCAL_BD_ADDR);
        qd->data[5] = 0;
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}

/***************************************************************************
 * TC_Write_Local_Radio_Power
 *
 * This command allows changing the local device's radio power to
 * a particular device.
 *
 ***************************************************************************/
void TC_Write_Local_Radio_Power(u_int16 handle, u_int16 radio_power)
{
#ifndef BLUETOOTH_MODE_LE_ONLY
    t_error status = INVALID_HCI_PARAMETERS;
    t_q_descr *qd;

    t_lmp_link *p_link;

    p_link = LMaclctr_Find_Handle(handle);

    if (p_link == 0) /* if handle is invalid, get local... */
	{
		p_link = LMaclctr_Find_Bd_Addr((t_bd_addr*) 
			SYSconfig_Get_Local_BD_Addr_Ref());
	}

    if (p_link != 0)
    {
    	t_devicelink* pDevLink = DL_Get_Device_Ref(p_link->device_index);

#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==0)
    	if (radio_power <= MAX_POWER_LEVEL_UNITS)
    	{
    		DL_Set_Power_Level(pDevLink, (u_int8)radio_power);
    		status = NO_ERROR;
    	}
#else
        if (
			(radio_power <= MAX_POWER_LEVEL_1MBITS)
#if (0 != MIN_POWER_LEVEL_1MBITS)// avoids compiler warning
        	&& (radio_power >= MIN_POWER_LEVEL_1MBITS)
#endif
        ||	(radio_power <= MAX_POWER_LEVEL_2MBITS)
#if (0 != MIN_POWER_LEVEL_2MBITS)// avoids compiler warning
            && (radio_power >= MIN_POWER_LEVEL_2MBITS)
#endif
        ||	(radio_power <= MAX_POWER_LEVEL_3MBITS)
#if (0 != MIN_POWER_LEVEL_3MBITS)// avoids compiler warning
			&& (radio_power >= MIN_POWER_LEVEL_3MBITS)
#endif			
			)
        {
        	pDevLink->power_level = (u_int8)radio_power;
        	pDevLink->power_level_2Mbits = (u_int8)radio_power;
        	pDevLink->power_level_3Mbits = (u_int8)radio_power;

#if (MIN_POWER_LEVEL_1MBITS > 0)// avoids compiler warning
			if (pDevLink->power_level < MIN_POWER_LEVEL_1MBITS)
        	    pDevLink->power_level = MIN_POWER_LEVEL_1MBITS;
#endif
#if (MIN_POWER_LEVEL_2MBITS > 0)// avoids compiler warning
			if (pDevLink->power_level_2Mbits < MIN_POWER_LEVEL_2MBITS)
        		pDevLink->power_level_2Mbits = MIN_POWER_LEVEL_2MBITS;
#endif
#if (MIN_POWER_LEVEL_3MBITS > 0)// avoids compiler warning
        	if (pDevLink->power_level_3Mbits < MIN_POWER_LEVEL_3MBITS)
        		pDevLink->power_level_3Mbits = MIN_POWER_LEVEL_3MBITS;
#endif
        	if (pDevLink->power_level > MAX_POWER_LEVEL_1MBITS)
        	    pDevLink->power_level = MAX_POWER_LEVEL_1MBITS;

        	if (pDevLink->power_level_2Mbits > MAX_POWER_LEVEL_2MBITS)
        		pDevLink->power_level_2Mbits = MAX_POWER_LEVEL_2MBITS;

        	if (pDevLink->power_level_3Mbits > MAX_POWER_LEVEL_3MBITS)
        		pDevLink->power_level_3Mbits = MAX_POWER_LEVEL_3MBITS;

        	status = NO_ERROR;
        }
#endif
    }
    else
    {
        status = NO_CONNECTION; 
    }

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 8);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 6;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_WRITE_LOCAL_RADIO_POWER);
        qd->data[5] = (u_int8)status;
        qd->data[6] = (u_int8)(handle & 0xFF);
        qd->data[7] = (u_int8)((handle & 0xF00)>>8);
        BTq_Commit(HCI_EVENT_Q, 0);
    }
#endif//BLUETOOTH_MODE_LE_ONLY
}

/***************************************************************************
 * TC_Read_Local_Radio_Power
 *
 * This command allows reading the local device's radio power to
 * a particular device.
 *
 ***************************************************************************/
void TC_Read_Local_Radio_Power(u_int16 handle)
{
#ifndef BLUETOOTH_MODE_LE_ONLY
    t_error status = NO_ERROR;
    t_q_descr *qd;
    u_int16 power_level = 0;
#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==1)
    u_int16 power_level_2Mbits = 0;
    u_int16 power_level_3Mbits = 0;
#endif
    t_lmp_link *p_link;

    p_link = LMaclctr_Find_Handle(handle);

    if (p_link == 0) /* if handle is invalid, get local... */
	{
//		p_link = LMaclctr_Find_Bd_Addr((t_bd_addr*) 
//			SYSconfig_Get_Local_BD_Addr_Ref());
	}
	
	if (p_link != 0)
    {
    	t_devicelink* pDevLink = DL_Get_Device_Ref(p_link->device_index);

    	power_level = DL_Get_Power_Level(pDevLink);
#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==1)
    	power_level_2Mbits = pDevLink->power_level_2Mbits;
        power_level_3Mbits = pDevLink->power_level_3Mbits;
#endif
    }
    else
    {
        status = NO_CONNECTION; 
    }

#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==0)
    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 10);
#else
    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 14);
#endif

    if(qd)
    {
    	qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==0)
    	qd->data[1] = 8;
#else
    	qd->data[1] = 12;
#endif
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_READ_LOCAL_RADIO_POWER);
        qd->data[5] = (u_int8)status;
        qd->data[6] = (u_int8)(handle & 0xFF);
        qd->data[7] = (u_int8)((handle & 0xF00)>>8);
        qd->data[8] = (u_int8)(power_level & 0xFF);
        qd->data[9] = (u_int8)((power_level & 0xFF00)>>8);
#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==1)
        qd->data[10] = (u_int8)(power_level_2Mbits & 0xFF);
        qd->data[11] =(u_int8)((power_level_2Mbits & 0xFF00)>>8);
        qd->data[12] = (u_int8)(power_level_3Mbits & 0xFF);
        qd->data[13] =(u_int8)((power_level_3Mbits & 0xFF00)>>8);
#endif
        BTq_Commit(HCI_EVENT_Q, 0);
    }
#endif//BLUETOOTH_MODE_LE_ONLY
}

/***************************************************************************
 * TC_Write_Local_SyncWord
 *
 * This command allows the 64 bits of the syncword to be set.
 * The syncword is supplied in little-endian order, i.e. p_sync
 * points to the least significant byte of the syncword and p_sync+7
 * points to the most significant byte of the syncword
 *
 ***************************************************************************/
void TC_Write_Local_SyncWord(u_int8* p_sync)
{
#if(PRH_BS_CFG_SYS_TEST_MODE_TESTER_SUPPORTED == 1)
    t_lmp_link *p_link;
    t_deviceIndex device_index;
    t_syncword syncword;
    t_error status = NO_ERROR;

    syncword.low = HCIparam_Get_Uint32(p_sync);
    syncword.high = HCIparam_Get_Uint32(p_sync+4);

//    p_link = LMaclctr_Find_Bd_Addr((t_bd_addr*) SYSconfig_Get_Local_BD_Addr_Ref());
    if (p_link != 0)
    {
        device_index = p_link->device_index;
        DL_Set_Access_Syncword(DL_Get_Device_Ref(device_index), syncword);
    } 
    else
    {
        status = NO_CONNECTION;
    }
    TCeg_Command_Complete_Event(TCI_WRITE_LOCAL_SYNCWORD, status);
#endif
}

/***************************************************************************
 * _TC_Run_Pump_Tests
 *
 * This command controls the data pump
 *
 ***************************************************************************/
void _TC_Run_Pump_Tests(u_int8* test_control)
{
#if (PRH_BS_CFG_SYS_DATA_PUMP_SUPPORTED==1)
    u_int16 conn_handle = (test_control[0] + ((test_control[1] & 0x0F) << 8));
    u_int8 test_scenario = test_control[2];
    u_int32 pkt_types = test_control[8] + (test_control[16] << 8);
    u_int16 len_test_data = (test_control[9] + (test_control[10] << 8));
    u_int32 num_pkts = (test_control[19] + (test_control[20] << 8)
        + (test_control[21] << 16) + (test_control[22] << 24));
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
    pkt_types |= ((test_control[23] << 16) + (test_control[24] << 24));
#endif

    /*
     * Handle awkward case of EV3 and HV3 sharing code-space.
     * Only have 16 bits in the test_control PDU.
     * Need 32 to represent the EV3 packet on the BTpump
     * interface. Solution: use two bits on the test_control PDU
     * to represent EV3 (the HV3 and DV bits --> an unlikely
     * combination to pump at the same time).
     */
    if (pkt_types == (HV3_BIT_MASK|DV_BIT_MASK))
    {
        pkt_types = EV3_BIT_MASK;
    }

    /*
     * BTpump_Request_Data_Pump accepts a set of packet types to
     * allow channel driven quality to be supported.
     */
    BTpump_Request_Data_Pump(
         (t_pump_type) test_scenario,
         0,
         (t_connectionHandle) conn_handle,
         pkt_types,
         (t_length) len_test_data,
         num_pkts);
#endif
}


#if(PRH_BS_CFG_SYS_TEST_MODE_TESTER_SUPPORTED == 1)
/***************************************************************************
 * _TC_Control_Testmode_In_Slave
 *
 * This command sends an LMP_test_control PDU to the device, specified
 * by the test_control PDU, with the parameters specified in the
 * test control PDU
 *
 ***************************************************************************/
void _TC_Control_Testmode_In_Slave(t_p_pdu test_control)
{
    t_error status;
    status = BTtmt_LMP_Send_Test_Control(test_control);

    if (status == NO_ERROR)
    {
        /*
         * Command complete event is returned on LMP_accepted/LMP_not_accepted
         * being processed BTtmt_LMP_Accepted()/BTtmt_LMP_Not_Accepted().
         */
        TCeg_Command_Status_Event(NO_ERROR, TCI_TEST_CONTROL);
    }
    else
    {
        TCeg_Command_Complete_Event(TCI_TEST_CONTROL, status);
    }
}

/***************************************************************************
 * _TC_Control_Testmode_Standalone
 *
 * This function controls standalone testmode operation
 *
 * 1. Establish a connection similar to BT Local Loopback
 *
 ***************************************************************************/
void _TC_Control_Testmode_Standalone(t_p_pdu test_control)
{
#ifndef BLUETOOTH_MODE_LE_ONLY
    /*
     * Fool LMP into allocating a connection and hence a device index
     * (similar code to that used for local loopback)
     */
    t_lm_event_info event_info;
    t_lmp_link *p_link;
    t_deviceIndex device_index;
    t_devicelink *p_device_link;

    const t_lap tester_lap = 0xA5F0C3;
    const t_uap tester_uap = 0x00;

    /*
     * The test control PDU from the tester is whitened with 0x55
     */
    {
        u_int8 i;
        for (i=2; i <= 10; i++)
        {
            test_control[i] ^= TESTMODE_WHITEN;
        }
    }

    /*
     * If we don't have a connection then get one
     */
//    p_link = LMaclctr_Find_Bd_Addr((t_bd_addr*) SYSconfig_Get_Local_BD_Addr_Ref());
    if (p_link==0)
    {
        p_link = LMaclctr_Alloc_Link();
        if(p_link != 0)
        {
            event_info.p_bd_addr = (t_bd_addr*) SYSconfig_Get_Local_BD_Addr_Ref();
            event_info.mode = ENCRYPT_NONE;
            event_info.status = NO_ERROR;
            /*
             * It is important to set the link type to invalid
             * to prevent call-back call to LMch_Enable_L2CAP_Traffic()
             */
            event_info.link_type = (t_linkType) 0x255;   
            event_info.handle = p_link->handle;
            p_link->state |= LMP_ACTIVE;
            p_link->role = SLAVE;
            device_index = p_link->device_index;
            BDADDR_Copy(&p_link->bd_addr, SYSconfig_Get_Local_BD_Addr_Ref());


            /*
             * To activate the scheduler, fool LC into thinking a connection
             * is up on the selected device index
             */

#if (PRH_BS_CFG_SYS_SLEEP_IN_STANDBY_SUPPORTED==1)
            if (SLEEP_PROCEDURE == USLCchac_Get_Active_Procedure())
            {
                USLCchac_Set_Saved_Device_State(Connection);
                /*USLCsleep_Cancel(); - already executed on TRAhcit */
            }
#endif

            USLCchac_Set_Device_State(Connection);
            DL_Set_Ctrl_State(DL_Get_Local_Device_Ref(), CONNECTED_TO_MASTER );
            p_device_link = DL_Get_Device_Ref(device_index);
            DL_Set_Ctrl_State(p_device_link, CONNECTED_TO_MASTER );
            DL_Set_AM_Addr(p_device_link, 1);

            /*
             * Initialisation used the inquiry GIAC_LAP as the syncword
             */             
            {
                t_syncword  syncword;                
                BTaddr_Build_Sync_Word(tester_lap, &syncword.high, &syncword .low);

                DL_Set_Access_Syncword(p_device_link, syncword);
                DL_Set_UAP_LAP(p_device_link, tester_uap|tester_lap );
            }
            DL_Set_Local_Rx_L2CAP_Flow(p_device_link, GO);
            DL_Set_Local_Tx_L2CAP_Flow(p_device_link, GO);

            LSLCfreq_Assign_Frequency_Kernel(p_device_link);

            /*
             * Force state to LMP_ACTIVE to allow p_link lookup from tra_hcit.c
             */         
            HCeg_Generate_Event(HCI_CONNECTION_COMPLETE_EVENT, &event_info);
        }
    }

    /*
     * Complete the test mode setup on the new or existing link
     */
    if (p_link != 0)
    {
        device_index = p_link->device_index;
        event_info.status = USLCtm_Testmode_Request(device_index, test_control+2);
        
        if(NO_ERROR == event_info.status)
        {
            USLCtm_Testmode_Change_Settings(device_index);

        /*
         * If a Transmitter test has been selected then force to
         * standalone transmitter test.
         * If a Loopback test has been selected then force to
         * standalone receiver test
         */
        if (DUT_ACTIVE_TXTEST == BTtst_Get_DUT_Mode())
        {
            BTtst_Set_DUT_Mode(DUT_STANDALONE_TXTEST);
        }
        else if(DUT_ACTIVE_LOOPBACK == BTtst_Get_DUT_Mode())
        {
            t_clock bt_clock;
            bt_clock = HW_get_bt_clk();

            /*
             * At 500us/sec drift, pretend that last access at
             * -100 seconds ==> ~ 0.1 second receiver window
             */
            DL_Set_Piconet_Clock_Last_Access(DL_Get_Device_Ref(device_index),
              ((bt_clock-(3200*100))&0x0FFFFFFF) );
            LC_Wakeup(device_index, 0 /* timeout */, bt_clock /*instance*/);
             
            BTtst_Set_DUT_Mode(DUT_STANDALONE_RXTEST);
        }

        }
    } 
    else
    {
        event_info.status = UNSPECIFIED_ERROR;
    }
    
    TCeg_Command_Complete_Event(TCI_TYPE_APPROVAL_TEST_CONTROL, event_info.status);
#endif
}
#endif

/***************************************************************************
 * TC_Write_Radio_Register
 *
 * This command allows the write to radio registers over SPI bus.
 *
 ***************************************************************************/ 
void TC_Write_Local_Radio_Register(u_int8 reg_addr, u_int16 reg_val)
{
    t_q_descr *qd;
    t_error status;
        
#if defined(TCI_SPI_INTERFACE_STRESS_TEST)
    int i;

    char * volatile switches = ( char *)(0x80000000|SLIDE_SWITCHES_BASE);

    for(i=0;(((*switches)&0x40)>>6);i++)
    {
    	HWradio_Set_Radio_Register(reg_addr,reg_val);
//    	SYSmmi_Display_Numeric(reg_val);
//    	SYSmmi_Display_7SEG_A(((i&0xFF00)>>8));
    }

//	SYSmmi_Display_7SEG_A(0);
#endif

#if defined(TERASIC)
    u_int32 flags;
    SYSirq_Disable_Interrupts_Save_Flags(&flags);
    HWradio_Set_Radio_Register(reg_addr,reg_val);
    SYSirq_Interrupts_Restore_Flags(flags);
    status = NO_ERROR;
#else
    status = COMMAND_DISALLOWED;
#endif
            
    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 6);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 4;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_WRITE_LOCAL_RADIO_REGISTER);
        qd->data[5] = status;
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}


/***************************************************************************
 * TC_Read_Radio_Register
 *
 * This command allows the read from radio registers over SPI bus.
 *
 ***************************************************************************/
void TC_Read_Local_Radio_Register(u_int8 reg_addr)
{
    t_q_descr *qd;
    t_error status;
    u_int32 reg_val;
    
#if defined(TCI_SPI_INTERFACE_STRESS_TEST)
    u_int32 reg;
    int i;

    char * volatile switches = ( char *)(0x80000000|SLIDE_SWITCHES_BASE);

    for(i=0;(((*switches)&0x40)>>6);i++)
    {
    	reg = HWradio_Get_Radio_Register(reg_addr);
//    	SYSmmi_Display_Numeric(reg);
//    	SYSmmi_Display_7SEG_A(((i&0xFF00)>>8));
    }

//	SYSmmi_Display_7SEG_A(0);
#endif

#if defined(TERASIC)
    u_int32 flags;
    SYSirq_Disable_Interrupts_Save_Flags(&flags);
    reg_val = HWradio_Get_Radio_Register(reg_addr);
    SYSirq_Interrupts_Restore_Flags(flags);
    status = NO_ERROR;
#else
    reg_val = 0;
    status = COMMAND_DISALLOWED;
#endif    

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 8);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 6;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_READ_LOCAL_RADIO_REGISTER);
        qd->data[5] = (u_int8)status;
        qd->data[6] = (u_int8)(reg_val & 0xFF);
        qd->data[7] = (u_int8)((reg_val & 0xFF00)>>8);
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}

/***************************************************************************
 * TC_Set_HCIT_UART_Baud_Rate
 *
 * This command allows the setting of the local HCIT UART baud rate.
 * IMMEDIATE Option:
 * Immediate effect & command complete event will be sent at new baud rate.
 * Otherwise:
 * Flash update & cold reset are required for new baud rate to take effect.
 *
 ***************************************************************************/ 
void TC_Set_HCIT_UART_Baud_Rate(u_int32 baud_rate)
{
    t_q_descr *qd;
    t_error status;
    
#if (TRA_HCIT_UART_SUPPORTED==1)
    extern struct tra_hcit_info volatile *sys_hcit_info;
    if(sys_hcit_info->type==TRA_HCIT_TYPE_UART)
    {
#if defined(TRA_HCIT_UART_SET_BAUD_RATE_IMMEDIATE)
        TRAhcit_Shutdown();
#endif
        TRAhcit_UART_Set_Default_Baud_Rate(baud_rate);
#if defined(TRA_HCIT_UART_SET_BAUD_RATE_IMMEDIATE)
        TRAhcit_Initialise();
#endif
        status = NO_ERROR;
    }
    else
#endif
    {
#if (BUILD_TYPE==GENERIC_BUILD)
        TRAhcit_UART_Set_Default_Baud_Rate(baud_rate);
        status = NO_ERROR;
#else
        status = COMMAND_DISALLOWED;
#endif
    }
            
    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 6);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 4;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_SET_HCIT_UART_BAUD_RATE);
        qd->data[5] = status;
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}
 
/***************************************************************************
 * TC_Set_Disable_Low_Power_Mode
 *
 * change availiability of Low Frequency Oscillator for TIGER board
 *
 ***************************************************************************/
void    TC_Set_Disable_Low_Power_Mode(void)
{
    t_q_descr *qd;
    t_error status;
    
#if (PRH_BS_CFG_SYS_LOW_POWER_MODE_SUPPORTED == 1)
    if(SYSpwr_Set_LowFrequencyOscillatorAvailable_Value(0))/*Disable*/
    {
        status = NO_ERROR;
    }
    else
#endif
    {
        status = COMMAND_DISALLOWED;
    }

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 6);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 4;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_SET_DISABLE_LOW_POWER_MODE);
        qd->data[5] = status;
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}

/***************************************************************************
 * TC_Set_Enable_Low_Power_Mode
 *
 * change availiability of Low Frequency Oscillator for TIGER board
 *
 ***************************************************************************/
void    TC_Set_Enable_Low_Power_Mode(void)
{
    t_q_descr *qd;
    t_error status;

#if (PRH_BS_CFG_SYS_LOW_POWER_MODE_SUPPORTED == 1)
    if(SYSpwr_Set_LowFrequencyOscillatorAvailable_Value(1))/*Enable*/        
    {
        status = NO_ERROR;
    }
    else
#endif
    {
        status = COMMAND_DISALLOWED;
    }

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 6);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 4;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_SET_ENABLE_LOW_POWER_MODE);
        qd->data[5] = status;
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}

/***************************************************************************
 * TC_Read_R2P_Min_Search_Window
 *
 * report value of minimum search window in return to piconet 
 *
 ***************************************************************************/
void    TC_Read_R2P_Min_Search_Window(void)
{
    t_q_descr *qd;
    t_error status;
    u_int8 window = 0;

#if (PRH_BS_CFG_SYS_RETURN_TO_PICONET_SUPPORTED==1)
    window=USLCr2p_Read_R2P_Min_Search_Window();
    status = NO_ERROR;
#else
    status = COMMAND_DISALLOWED;
#endif
    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 7);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 5;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_READ_R2P_MIN_SEARCH_WINDOW);
        qd->data[5] = (u_int8)status;
        qd->data[6] = window;
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}

/***************************************************************************
 * TC_Write_R2P_Min_Search_Window
 *
 * change minimum search window in return to piconet 
 *
 ***************************************************************************/
void    TC_Write_R2P_Min_Search_Window(u_int8 new_window)
{
    t_q_descr *qd;
    t_error status;
    
#if (PRH_BS_CFG_SYS_RETURN_TO_PICONET_SUPPORTED==1)
    USLCr2p_Write_R2P_Min_Search_Window(new_window);
    status = NO_ERROR;
#else
    status = COMMAND_DISALLOWED;
#endif

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 6);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 4;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_WRITE_R2P_MIN_SEARCH_WINDOW);
        qd->data[5] = NO_ERROR;
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}


/***************************************************************************
 * TC_Write_Park_Parameters
 *
 ***************************************************************************/

void TC_Write_Park_Parameters(u_int8* p_pdu)
{
    u_int8 status = NO_ERROR;    
    t_q_descr *qd;
    
#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
    /*  
     * u_int8  -- Nb
     * u_int16 -- D_bcast
     * u_int8  -- delta_bcast
     * u_int8  -- D_access
     * u_int8  -- T_access
     * u_int8  -- N_sleep
     * u_int8  -- D_sleep
     * u_int8  -- N_access
     * u_int8  -- N_poll
     * u_int8  -- M_access
     */
     
    g_LM_config_info.N_bcast =  p_pdu[0];

    g_LM_config_info.D_bcast =((u_int16)p_pdu[1] + ((u_int16) p_pdu[2] << 8));
    g_LM_config_info.park_info.delta_bcast = p_pdu[3];
    g_LM_config_info.D_access = p_pdu[4];
    g_LM_config_info.T_access = p_pdu[5];
    g_LM_config_info.N_sleep =  p_pdu[6];
    g_LM_config_info.D_sleep =  p_pdu[7];
    g_LM_config_info.N_access = p_pdu[8];
    g_LM_config_info.N_poll = p_pdu[9];
    g_LM_config_info.M_access = p_pdu[10];
#else
    status = COMMAND_DISALLOWED;
#endif
    
    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 6);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 4;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_WRITE_PARK_PARAMETERS);
        qd->data[5] = status;
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}

/***************************************************************************
 * TC_Set_Broadcast_Scan_Window
 *
 ***************************************************************************/
 
void TC_Set_Broadcast_Scan_Window(u_int8* p_pdu)
{
    u_int8 status = NO_ERROR;
    t_q_descr *qd;
    
#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
    if(LMconfig_LM_Connected_As_Master() && (g_LM_config_info.num_links_parked))
    {
        g_LM_config_info.bcast_scan_ext = ((u_int16)p_pdu[0] + ((u_int16) p_pdu[1] << 8));   
		g_LM_config_info.bcast_scan_ext_unpark = p_pdu[2];
        _Send_LMP_Set_Broadcast_Scan_Window();
    }
    else
#endif
    {
        status = COMMAND_DISALLOWED;
    }
    
    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 6);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 4;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_SET_BROADCAST_SCAN_WINDOW);
        qd->data[5] = status;
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}

/***************************************************************************
 * TC_Read_Unused_Stack_Space
 *
 ***************************************************************************/

void TC_Read_Unused_Stack_Space(void)
{
    u_int8 status = NO_ERROR;    
    u_int32 unused_stack = 0;
    t_q_descr *qd;
    
#if defined(TERASIC)
    unused_stack = _SYS_Main_Check_Unused_Stack_Space();
#else
    status = COMMAND_DISALLOWED;
#endif
    
    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 10);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 8;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_READ_UNUSED_STACK_SPACE);
        qd->data[5] = status;
        _Insert_Uint32(&(qd->data[6]), unused_stack);
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}

/***************************************************************************
 * TC_Write_VCI_CLK_Override
 *
 ***************************************************************************/

void TC_Write_VCI_CLK_Override(u_int8 length, u_int8* p_pdu)
{
    u_int8 status;
    t_q_descr *qd;
        
#if defined(SYS_HAL_VCI_CLK_OVERRIDE_SUPPORTED)
    switch(length)
    {
    case 1:
        /* Paramater support checked in hal specific override function */
        status = HWcodec_VCI_CLK_Override((t_VCI_SCO_CLK_Freq)p_pdu[0])
               ? INVALID_HCI_PARAMETERS : NO_ERROR;
        break;
    case 4:
        /* Minimal paramater checking on extended format */
        if( (p_pdu[0] > 3) /* vci clk sel */ || (p_pdu[1] > 1) /* vci clk sel map */
         || (p_pdu[2] > 1) /* codec type */ || (p_pdu[3] > 9) /* sco cfg0 */ )
        {
            status = INVALID_HCI_PARAMETERS;
        }
        else
        {
            HWcodec_VCI_CLK_Override_Enable(p_pdu[0], p_pdu[1], p_pdu[2], p_pdu[3]);
            status = NO_ERROR;
        }
        break;
    default:
        status = INVALID_HCI_PARAMETERS;
    }
#else
    status = COMMAND_DISALLOWED;
#endif
    
    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 6);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 4;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_VCI_CLK_OVERRIDE);
        qd->data[5] = status;
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}

/***************************************************************************
 * TC_Send_Encryption_Key_Size_Mask_Req
 *
 * Sends LMP Encryption_Key_Size_Mask_Req to identified slave.
 *
 ***************************************************************************/
void TC_Send_Encryption_Key_Size_Mask_Req(u_int16 acl_handle)
{
    t_q_descr *qd;
    t_error status;

#if (PRH_BS_CFG_SYS_LMP_BROADCAST_ENCRYPTION_SUPPORTED==1)
    t_lmp_link* p_link = LMaclctr_Find_Handle(acl_handle);
    if ((p_link) && (p_link->role == MASTER))
    {
        t_lmp_pdu_info pdu_info;
        pdu_info.tid_role = MASTER;
        pdu_info.opcode = LMP_ENCRYPTION_KEY_SIZE_MASK_REQ;
        LM_Encode_LMP_PDU(p_link->device_index, &pdu_info);
        status = NO_ERROR;
    }
    else
#endif
    {
        status = COMMAND_DISALLOWED;
    }

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 6);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 4;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_SEND_ENCRYPTION_KEY_SIZE_MASK_REQ);
        qd->data[5] = status;
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}
 

/***************************************************************************
 * TC_Read_Raw_RSSI
 *
 * Read back the raw RSSI value for a particular connection handle.
 *
 ***************************************************************************/
void TC_Read_Raw_RSSI(u_int16 handle)
{
    t_q_descr *qd;
    u_int8 raw_rssi;
    t_lmp_link* p_link;
    t_error status = NO_ERROR;

    handle = (handle & 0xFFF);
#if (PRH_BS_CFG_SYS_LMP_RSSI_SUPPORTED==1)
    p_link = LMaclctr_Find_Handle(handle);


    if (p_link)
    {
        raw_rssi = LC_Get_RSSI(p_link->device_index);
        status = (t_error)0;
    }
    else
    {
        raw_rssi = 0;
        status = NO_CONNECTION;
    }
#else
    raw_rssi = 0;
    status = UNSUPPORTED_FEATURE;
#endif
    
    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 9);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 7;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_READ_RAW_RSSI);
        qd->data[5] = status;
        _Insert_Uint16(&(qd->data[6]), handle);
        qd->data[8] = raw_rssi;
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}
 
/***************************************************************************
 * TC_Read_BER
 *
 * Read back the current BER value (test mode tester).
 *
 ***************************************************************************/
void TC_Read_BER()
{
    t_q_descr *qd;
    u_int32 ber;
    t_error status = NO_ERROR;


#if (PRH_BS_CFG_SYS_TEST_MODE_TESTER_SUPPORTED==1)
    if ((BTtst_Get_DUT_Mode() == DUT_TESTMODE_TESTER) && BTtmt_Is_PER_BER_Available())
    {
        ber = BTtmt_Get_BER();
        status = (t_error)0;
    }
    else
    {
        ber = 0;
        status = COMMAND_DISALLOWED;
    }
#else
    ber = 0;
    status = UNSUPPORTED_FEATURE;
#endif
    
    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 10);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 8;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_READ_BER);
        qd->data[5] = status;
        _Insert_Uint32(&(qd->data[6]), ber);
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}

/***************************************************************************
 * TC_Read_PER
 *
 * Read back the current PER value (test mode tester).
 *
 ***************************************************************************/
void TC_Read_PER()
{
    t_q_descr *qd;
    u_int32 per;
    t_error status = NO_ERROR;


#if (PRH_BS_CFG_SYS_TEST_MODE_TESTER_SUPPORTED==1)
    if ((BTtst_Get_DUT_Mode() == DUT_TESTMODE_TESTER) && BTtmt_Is_PER_BER_Available())
    {
        per = BTtmt_Get_PER();
        status = (t_error)0;
    }
    else
    {
        per = 0;
        status = COMMAND_DISALLOWED;
    }
#else
    per = 0;
    status = UNSUPPORTED_FEATURE;
#endif
    
    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 10);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 8;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_READ_PER);
        qd->data[5] = status;
        _Insert_Uint32(&(qd->data[6]), per);
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}


/***************************************************************************
 * TC_Read_Raw_RSSI_PER_BER
 *
 * Read back the raw RSSI and current PER & BER values (test mode tester).
 *
 ***************************************************************************/
void TC_Read_Raw_RSSI_PER_BER()
{
    t_q_descr *qd;
    u_int8 raw_rssi;
    u_int32 ber;
    u_int32 per;
    t_error status = NO_ERROR;

#if (PRH_BS_CFG_SYS_TEST_MODE_TESTER_SUPPORTED==1)
    if ((BTtst_Get_DUT_Mode() == DUT_TESTMODE_TESTER) && BTtmt_Is_PER_BER_Available())
    {
        extern t_BTtmt_Tester BTtmt_info;
        raw_rssi = LC_Get_RSSI(BTtmt_info.device_index);
        ber = BTtmt_Get_BER();
        per = BTtmt_Get_PER();
        status = (t_error)0;
    }
    else
    {
        raw_rssi = 0;
        per = 0;
        ber = 0;
        status = COMMAND_DISALLOWED;
    }
#else
    raw_rssi = 0;
    per = 0;
    ber = 0;
    status = UNSUPPORTED_FEATURE;
#endif
    
    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 15);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 13;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_READ_RAW_RSSI_PER_BER);
        qd->data[5] = status;
        _Insert_Uint8(&(qd->data[6]), raw_rssi);
        _Insert_Uint32(&(qd->data[7]), per);
        _Insert_Uint32(&(qd->data[11]), ber);
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}

#if (PRH_BS_CFG_SYS_SCO_REPEATER_SUPPORTED==1)
/***************************************************************************
 * TC_Set_Disable_SCO_Repeater_Mode
 *
 * Used for SCO Repeater only
 *
 ***************************************************************************/
void TC_Set_Disable_SCO_Repeater_Mode(void)
{
    t_q_descr *qd;
    t_error status = NO_ERROR;;

    LC_Set_SCO_Repeater_Bit(0);
    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 6);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 4;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_SET_DISABLE_SCO_REPEATER_MODE);
        qd->data[5] = (u_int8)status;
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}

/***************************************************************************
 * TC_Set_Enable_SCO_Repeater_Mode
 *
 * used for SCO Repeater only
 *
 ***************************************************************************/
void TC_Set_Enable_SCO_Repeater_Mode(void)
{
    t_q_descr *qd;
    t_error status = NO_ERROR;;
    LC_Set_SCO_Repeater_Bit(1);

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 6);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 4;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_SET_ENABLE_SCO_REPEATER_MODE);
        qd->data[5] = (u_int8)status;
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}
#endif

#if(PRH_BS_CFG_SYS_BROADCAST_NULL_IN_INQ_PAGE_SUPPORTED==1)

/***************************************************************************
 * TC_Set_Emergency_Poll_Interval
 *
 *
 ***************************************************************************/
void TC_Set_Emergency_Poll_Interval(u_int16 emergency_poll)
{
    t_error status = NO_ERROR;

    if(emergency_poll > 400 || 0 == emergency_poll)
    {
        status = INVALID_HCI_PARAMETERS;
    }
    else if((emergency_poll % 10) == 0)
    {
	 	g_LM_config_info.emergency_poll_interval = emergency_poll;
    }
	else
    {
        status = INVALID_HCI_PARAMETERS;
    }

	TCeg_Command_Complete_Event(TCI_SET_EMERGENCY_POLL_INTERVAL, status);
}
#endif


/***************************************************************************
 *
 * TC_Write_eSCO_Retransmission_Mode
 *
 * This routine forces the host controller to share the eSCO retransmission
 * opportunities of any eSCO connections setup while the eSCO retransmission
 * mode is "shared". The default is "regular" where each eSCO connection
 * has its own retransmission opportunity.
 *
 * Note: if any eSCO link is "shared", all eSCO links must be "shared".
 *       conversely, if any eSCO link is "regular", all eSCO links
 *       must be "regular".
 *
 ***************************************************************************/
void TC_Write_eSCO_Retransmission_Mode(u_int8 mode)
{
#if ((PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1) || (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1))
    t_q_descr *qd;
    t_error status = NO_ERROR;
//#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==1)
//		{
//		extern t_link_entry  link_container[];
//		t_lmp_link *p_link = &(link_container[0].entry);
//        p_link->peer_power_status = mode;
//		}
//#else
    if(LMscoctr_Get_Number_SYN_Connections() == 0)
    {
        LMscoctr_Set_eSCO_Retransmission_Mode((t_esco_retrans_mode)mode);
    }
    else
    {
        status = COMMAND_DISALLOWED;
    }
//#endif    
    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 6);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 4;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_WRITE_ESCO_RETRANSMISSION_MODE);
        qd->data[5] = (u_int8)status;

        BTq_Commit(HCI_EVENT_Q, 0);
    }
#endif
}


/***************************************************************************
 * 
 * TC_Read_eSCO_Retransmission_Mode
 *
 * This function reads the curent eSCO retransmission mode. The
 * eSCO retransmission mode can be either "shared" where all eSCO
 * links share the retransmission opportunities or "regular" where
 * each eSCO link has it's own retransmission opportunities.
 *
 ***************************************************************************/
void TC_Read_eSCO_Retransmission_Mode(void)
{
#if ((PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1) || (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED == 1))
    t_q_descr *qd;
    t_error status = NO_ERROR;

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 7);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 5;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_READ_ESCO_RETRANSMISSION_MODE);
        qd->data[5] = (u_int8)status;
//#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==1)
//		{
//		extern t_link_entry  link_container[];
//		t_lmp_link *p_link = &(link_container[0].entry);
//        qd->data[6] = p_link->peer_power_status;
//		}
//#else
        qd->data[6] = LMscoctr_Get_eSCO_Retransmission_Mode();
//#endif
        BTq_Commit(HCI_EVENT_Q, 0);
    }
#endif
}


/***************************************************************************
 * TC_Write_Security_Timeouts
 *
 * Write new Pin Code and Link Key Request timeouts.
 * Paramater values are interpreted as follows:
 *
 * ==0: timeout unchanged.
 * ==1: restore default timeout.
 * >=2: new timeout
 *
 * If the sum of the requested timeouts are greater than lmp response
 * timeout, rejected as invalid paramater combination.
 *
 ***************************************************************************/
void TC_Write_Security_Timeouts(u_int16 link_key_timeout, u_int16 pin_code_timeout,
								u_int8 pin_code_extend )
{
#ifndef BLUETOOTH_MODE_LE_ONLY


    extern t_slots LMsec_link_key_request_timeout;
    extern t_slots LMsec_pin_code_request_timeout;
	extern boolean LMsec_pin_code_request_timeout_extend;
    t_error status = NO_ERROR;

    if (0 == link_key_timeout)
        link_key_timeout = LMsec_link_key_request_timeout;
    else if(1 == link_key_timeout)
        link_key_timeout = PRH_BS_CFG_SYS_LINK_KEY_REQUEST_TIMEOUT;
    if (0 == pin_code_timeout)
        pin_code_timeout = LMsec_pin_code_request_timeout;
    else if(1 == pin_code_timeout)
        pin_code_timeout = PRH_BS_CFG_SYS_PIN_CODE_REQUEST_TIMEOUT;
  

    if (link_key_timeout + pin_code_timeout > PRH_LMP_MSG_TIMEOUT)
    {
        status = INVALID_HCI_PARAMETERS;
    }
    else
    {
        LMsec_link_key_request_timeout = link_key_timeout;
        LMsec_pin_code_request_timeout = pin_code_timeout;
		LMsec_pin_code_request_timeout_extend = pin_code_extend;
    }

    TCeg_Command_Complete_Event(TCI_WRITE_SECURITY_TIMEOUTS, status);
#endif//BLUETOOTH_MODE_LE_ONLY

}

/***************************************************************************
 * TC_Write_Features
 *
 ***************************************************************************/
void TC_Write_Features(u_int16 feature, boolean enable)
{
    t_error status = COMMAND_DISALLOWED;

    if (feature == PRH_BS_CFG_SYS_LMP_TIMING_INFO_FEATURE)
    {
#if (PRH_BS_CFG_SYS_LMP_TIMING_INFO_SUPPORTED==1)
        if (enable)
        {
            SYSconfig_Enable_Feature(PRH_BS_CFG_SYS_LMP_TIMING_INFO_FEATURE);
            lmp_command_table[LMP_TIMING_ACCURACY_REQ] = &LMlc_LMP_Timing_Accuracy_Req;
            lmp_command_table[LMP_TIMING_ACCURACY_RES] = &LMlc_LMP_Timing_Accuracy_Res;
        }
        else
        {
            SYSconfig_Disable_Feature(PRH_BS_CFG_SYS_LMP_TIMING_INFO_FEATURE);
            lmp_command_table[LMP_TIMING_ACCURACY_REQ] = &LMdisp_LMP_Unsupported_Command;
            lmp_command_table[LMP_TIMING_ACCURACY_RES] = &LMdisp_LMP_Unsupported_Command;
        }
        status = NO_ERROR;
#endif
    }
    else if(feature == PRH_BS_CFG_SYS_LM_AUTO_RATE_FEATURE)
    {
#if (PRH_BS_CFG_SYS_LM_AUTO_RATE_SUPPORTED==1)
        if (enable)
        {
            SYSconfig_Enable_Feature(PRH_BS_CFG_SYS_LM_AUTO_RATE_FEATURE);
            lmp_command_table[LMP_AUTO_RATE] = &LMqos_LMP_Auto_Rate;
        }
        else
        {
            SYSconfig_Disable_Feature(PRH_BS_CFG_SYS_LM_AUTO_RATE_FEATURE);
            lmp_command_table[LMP_AUTO_RATE] = &LMdisp_LMP_Ignore_Command;
        }
        status = NO_ERROR;
#endif
    }

    TCeg_Command_Complete_Event(TCI_WRITE_FEATURES, status);
}

#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==1)
void TC_Write_EPC_Enable(u_int8 epc_enable)
{
    t_q_descr *qd;
    t_error status = NO_ERROR;
	extern boolean epc_enabled;
	u_int8 *local_features = SYSconfig_Get_LMP_Features_Ref();

	if (epc_enable == 0)
	{
		epc_enabled = FALSE;
	    local_features[7] = local_features[7] & 0xFB; // Disable EPC
	}
	else if (epc_enable == 1)
	{
	    local_features[7] = local_features[7] | 0x04; // Enable EPC
	    epc_enabled = TRUE;
	}
	else
	{
		status = INVALID_HCI_PARAMETERS;
	}

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 6);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 4;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_WRITE_EPC_ENABLE);
        qd->data[5] = (u_int8)status;
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}
#endif

#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
void TC_Write_PTA_Enable(u_int8 pta_enable, u_int8 pta_test_mode, 
							u_int8 pta_req_lead_time)
{
    t_q_descr *qd;
    t_error status = NO_ERROR;

    if (!pta_enable && pta_test_mode)
    	status = COMMAND_DISALLOWED;
    else if (!HWpta_Control_Enabled(pta_enable))
		status = UNSUPPORTED_FEATURE;
	else if (!HWpta_Test_Control_Enabled(pta_test_mode))
		status = UNSUPPORTED_FEATURE;
	else if (!HWpta_Set_Bt_Req_Lead_Time(pta_req_lead_time))
		status = INVALID_HCI_PARAMETERS;

    qd = BTq_Enqueue(HCI_EVENT_Q, 0, 6);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 4;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_WRITE_PTA_ENABLE);
        qd->data[5] = (u_int8)status;
        BTq_Commit(HCI_EVENT_Q, 0);
    }
}
#endif

void TC_Set_Disable_SCO_Via_HCI(void)
{
    t_q_descr *qd;
    t_error status = NO_ERROR;

    g_LM_config_info.default_sco_route_via_hci = 0x00;

	qd = BTq_Enqueue(HCI_EVENT_Q, 0, 6);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 4;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_SET_DISABLE_SCO_VIA_HCI);
        qd->data[5] = (u_int8)status;
        BTq_Commit(HCI_EVENT_Q, 0);
	}
}


void TC_Set_Enable_SCO_Via_HCI(void)
{
    t_q_descr *qd;
    t_error status = NO_ERROR;

    g_LM_config_info.default_sco_route_via_hci = 0x01;

	qd = BTq_Enqueue(HCI_EVENT_Q, 0, 6);
    if(qd)
    {
        qd->data[0] = HCI_COMMAND_COMPLETE_EVENT;
        qd->data[1] = 4;
        qd->data[2] = 1;
        _Insert_Uint16(&(qd->data[3]), TCI_SET_ENABLE_SCO_VIA_HCI);
        qd->data[5] = (u_int8)status;
        BTq_Commit(HCI_EVENT_Q, 0);
	}
}
#endif//BLUETOOTH_MODE_LE_ONLY
#endif
