/*************************************************************************
 * MODULE NAME:    lslc_stat.c
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    LSLC_Status
 * MAINTAINER:     John Nelson
 * DATE:           27 May 1999
 * 
 * SOURCE CONTROL: $Id: lslc_stat.c,v 1.55 2010/02/11 02:02:39 garyf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *    V1.0     27 May 1999 -   jn       - Initial Version V0.9
 *    V1.1     10 Nov 1999 -   jn       - Added packet logger
 *
 * SOURCE: Based on V1_3 Design
 *         New Packet Log supported
 *         => Logging complete at PKA/PKD
 *         => Frequencies read directly from PIA/SER_DATA
 *
 * ISSUES:
 *    Reimplement as macros.
 * NOTES TO USERS:
 *    
 ************************************************************************/
#include "sys_config.h"
#include "sys_types.h"
#include "lc_types.h"

#include <string.h>

#include "lslc_stat.h"
#include "lslc_freq.h"
#include "uslc_chan_ctrl.h"

#include "hw_register.h"
#include "hw_lc.h"
#include "hw_radio.h"

/*
 * Tx/Rx Packet Monitor
 */
t_LLCmonitors monitors;


/*
 * Log packets if defined
 */

#if (PRTH_BS_DBG_BB_PACKET_LOG_TYPE != PACKETLOG_NONE)
/*
 * Should be static but global to allow direct view from within debugger.
 */
t_LLCpacket       packetLog[PRTH_BS_DBG_MAX_BB_MAX_PACKETS_IN_LOG];
u_int16           next_packet=0;

t_LogPacketTypes  log_type=PACKETLOG_NONE;
static void _LSLCstat_Select_NextPacket(void);
#endif

/************************************************************************
 * LSLCstat_ResetMonitors
 * Resets the monitors to zero.
 ************************************************************************/
void LSLCstat_Reset_Monitors()                    
{
    int i = sizeof(monitors);
    u_int8* ptr=(u_int8*)&monitors;

    while (i-- != 0)
    {
        *ptr++ = 0;
    }

    /* memset(&monitors, 0, sizeof(monitors) );  */
}

/************************************************************************
 * LSLCstat_ReadMonitors
 *
 * Copies the values of the monitors.
 ************************************************************************/
t_LLCmonitors *LSLCstat_Read_Monitors(t_LLCmonitors *pMonitors)           
{
    return (t_LLCmonitors *) memcpy(pMonitors, &monitors, sizeof(monitors));
}

/************************************************************************
 * LSLCstat_ReadResetMonitors
 *
 * Copies the values of the monitors and Resets them.
 ************************************************************************/
t_LLCmonitors *LSLCstat_Read_and_Reset_Monitors(t_LLCmonitors *pMonitors)
{
    pMonitors = LSLCstat_Read_Monitors(pMonitors);
    LSLCstat_Reset_Monitors();

    return pMonitors;
}

/* 
 * Some useful derivatives                                      
 *
 * RxInvalidPackets = 
 */

/************************************************************************
 * LSLCstat_Setup_Packet_Log
 * Establish the log basis None|Valid|All Packets with Buffer Wrap/NoWrap
 ************************************************************************/
void LSLCstat_Setup_Packet_Log(t_LogPacketTypes logtype)
{
#if (PRTH_BS_DBG_BB_PACKET_LOG_TYPE != PACKETLOG_NONE)
   log_type=logtype;
   if (logtype != PACKETLOG_NONE)
      memset(&packetLog, 0, sizeof(packetLog) );
   next_packet = 0;
#endif
}

/************************************************************************
 * LSLCstat_Log_Common_Tx_Rx
 *
 * Logs the common Clocks/Access Code
 * The frequency is logged separately, into the next Packet
 ************************************************************************/
#if (PRTH_BS_DBG_BB_PACKET_LOG_TYPE != PACKETLOG_NONE)
static void
    _LSLCstat_Log_Common_Tx_Rx_Registers 
    (t_LLCpacket *p_packetEntry, t_TXRXstatus status, t_packet packet)
{
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_GP1_REG);
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_GP2_REG);
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_GP1_REG);
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_GP2_REG);
    mHWreg_Move_Cache_Register(JAL_COM_CTRL_GP1_REG, 
                p_packetEntry->com_ctrl_GP1_reg);
    mHWreg_Move_Cache_Register(JAL_COM_CTRL_GP2_REG, 
                p_packetEntry->com_ctrl_GP2_reg);


    HW_get_sync_Ex(p_packetEntry->syncword);

    p_packetEntry->native_clock_when_log = HW_get_native_clk();
    p_packetEntry->bt_clock_when_log     = HW_get_bt_clk();

    p_packetEntry->status = status;

    if (status != TX_OK && status != RX_OK)
        p_packetEntry->packet_type = INVALIDpkt;
    else if (mHWreg_Get_Cache_Bit(JAL_COM_CTRL_GP1_REG, JAL_PAGE) )
        p_packetEntry->packet_type = IDpkt; /* Indicate An ID packet        */
    else
        p_packetEntry->packet_type = packet;

    p_packetEntry->context =  USLCchac_get_device_state();
}
#endif

/************************************************************************
 * LSLCstat_Log_Tx_Packet
 *
 * Logs the Time/Frequency/Access Code/Header and start of Payload
 * The frequency is logged separately, into the next Packet
 ************************************************************************/
void LSLCstat_Log_Tx_Packet(t_TXRXstatus status)
{

#if (PRTH_BS_DBG_BB_PACKET_LOG_TYPE != PACKETLOG_NONE)

    t_LLCpacket *p_packetEntry;
    u_int32 *tx_acl_buf;
    t_packet packet_type;

    mHWreg_Create_Cache_Register(JAL_TX_CTRL_REG);
    mHWreg_Create_Cache_Register(JAL_TX_STATUS_REG);
    mHWreg_Create_Cache_Register(JAL_ESCO_CTRL_REG);
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
    mHWreg_Create_Cache_Register(JAL_EDR_CTRL_REG);
    mHWreg_Load_Cache_Register(JAL_EDR_CTRL_REG);
#endif

/*
 * Log if All Packets OR (Valid Packet AND Packet is ok)
 */
    if ((log_type & LOG_ALL) || ((log_type & LOG_VALID) && (status == TX_OK)))
    {
        mHWreg_Load_Cache_Register(JAL_TX_CTRL_REG);
        mHWreg_Load_Cache_Register(JAL_TX_STATUS_REG);
        mHWreg_Load_Cache_Register(JAL_ESCO_CTRL_REG);
 
        p_packetEntry = packetLog+next_packet;
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
      packet_type = (((mHWreg_Get_Cache_Field(JAL_EDR_CTRL_REG,JAL_EDR_PTT_ACL) | mHWreg_Get_Cache_Field(JAL_EDR_CTRL_REG,JAL_EDR_PTT_ESCO))) << 4) |
		mHWreg_Get_Cache_Field(JAL_TX_CTRL_REG,JAL_TX_TYPE);
#else
        packet_type = (t_packet) mHWreg_Get_Cache_Field(JAL_TX_CTRL_REG, JAL_TX_TYPE);
#endif
        /*
         * distinguish HV3 from EV3
         */
        if(packet_type == HV3)
        {
            if(HW_get_am_addr() == HW_get_esco_lt_addr())
            {
                packet_type = EV3;
            }
        }
        _LSLCstat_Log_Common_Tx_Rx_Registers(p_packetEntry, status, packet_type);

        tx_acl_buf = (u_int32*)(HW_get_tx_acl_buf_addr());
        p_packetEntry->start_of_payload = *tx_acl_buf;

        p_packetEntry->tx0_rx1 = TX_PKT;
/*
 * Log the common control register 0x20-27, and transmit control register 0x30-33
 */
        mHWreg_Move_Cache_Register(JAL_TX_CTRL_REG, p_packetEntry->tx_rx_ctrl_reg);
        mHWreg_Move_Cache_Register(JAL_TX_STATUS_REG, p_packetEntry->tx_rx_stat_reg);
        mHWreg_Move_Cache_Register(JAL_ESCO_CTRL_REG, p_packetEntry->esco_ctrl_reg);
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
		mHWreg_Move_Cache_Register(JAL_EDR_CTRL_REG,p_packetEntry->edr_ctrl_reg);
        /*
         * To minimise impact on ATS log TX packets using "old" TX reg format
         */
		p_packetEntry->tx_rx_ctrl_reg = ( (p_packetEntry->tx_rx_ctrl_reg & 0x000001FF) |
 			                              ((p_packetEntry->tx_rx_ctrl_reg >> 1) & 0xFFFFFE00));


		p_packetEntry->esco_ctrl_reg = ( (p_packetEntry->esco_ctrl_reg & 0x000000FF) |
                                         ((p_packetEntry->esco_ctrl_reg & 0x00FF0000) >> 8) | 
                                         ((p_packetEntry->esco_ctrl_reg & 0x00001C00) << 6) | 
                                         ((p_packetEntry->esco_ctrl_reg & 0x00002000) << 6));
#endif
        p_packetEntry->frequency = LSLCfreq_Get_Frequency(TX_START)
                                   | (LSLCfreq_Get_Frequency(TX_MID) << 8)
                                   | (HWradio_GetRadioMode()<<24);

/*
 * Fast way to exclude POLL/NULLs
 */

#ifdef PACKET_LOG_TX_RESTRICTED
        /*
         * Allow packets or special event sequences to be excluded from packetLog
         * Exclude packet if corresponding bit asserted or if a special.
         */
        if (  !(
                 ( (1u << p_packetEntry->packet_type) & PACKET_LOG_TX_RESTRICTED) ||
                 ( (0x80000000 & PACKET_LOG_TX_RESTRICTED) && p_packetEntry->context == Page )
               )
           )
#endif
           _LSLCstat_Select_NextPacket();

    }
#endif
}

/************************************************************************
 * LSLCstat_Log_Rx_Packet
 *
 * Logs the Time/Access Code/Header and start of Payload
 * The frequency is logged separately, into the next Packet
 ************************************************************************/
void LSLCstat_Log_Rx_Packet(t_TXRXstatus status)
{
#if (PRTH_BS_DBG_BB_PACKET_LOG_TYPE != PACKETLOG_NONE)

    t_LLCpacket *p_packetEntry;
    t_packet    packet_type;

    mHWreg_Create_Cache_Register(JAL_RX_CTRL_REG);
    mHWreg_Create_Cache_Register(JAL_RX_STATUS_GP_REG);
    mHWreg_Create_Cache_Register(JAL_ESCO_CTRL_REG);
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
    mHWreg_Create_Cache_Register(JAL_EDR_CTRL_REG);
    mHWreg_Load_Cache_Register(JAL_EDR_CTRL_REG);
#endif

/*
 * Log if All Packets OR (Valid Packet AND Packet is ok)
 */
    if ( (log_type&LOG_ALL) || ((log_type&LOG_VALID) /*&& (status == RX_OK) */))
    {
        mHWreg_Load_Cache_Register(JAL_RX_STATUS_GP_REG);
        mHWreg_Load_Cache_Register(JAL_RX_CTRL_REG);
        mHWreg_Load_Cache_Register(JAL_ESCO_CTRL_REG);
   
        p_packetEntry = packetLog + next_packet;
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
		{
			u_int32 ctrl_acl;
			u_int32 ctrl_sco;

			ctrl_acl = (mHWreg_Get_Cache_Field(JAL_EDR_CTRL_REG,JAL_EDR_PTT_ACL) << 4);
			ctrl_sco = (mHWreg_Get_Cache_Field(JAL_EDR_CTRL_REG,JAL_EDR_PTT_ESCO) << 4);

			packet_type = (t_packet) mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_TYPE);
			packet_type |= (ctrl_acl | ctrl_sco);

		}
#else
      packet_type = (t_packet) mHWreg_Get_Cache_Field(JAL_RX_STATUS_GP_REG, JAL_RX_TYPE);
#endif

        /*
         * distinguish EV3 from HV3
         */
        if(packet_type == HV3)
        {
            if(HW_get_am_addr() == HW_get_esco_lt_addr())
            {
                packet_type = EV3;
            }
        }

        _LSLCstat_Log_Common_Tx_Rx_Registers(p_packetEntry, status, packet_type);

        p_packetEntry->tx0_rx1 = RX_PKT;
/*
 * Log the common control register 0x20-23, and receive status register 0x50-53
 */

        mHWreg_Move_Cache_Register(JAL_RX_CTRL_REG,
                       p_packetEntry->tx_rx_ctrl_reg);
        mHWreg_Move_Cache_Register(JAL_RX_STATUS_GP_REG,
                       p_packetEntry->tx_rx_stat_reg);
        mHWreg_Move_Cache_Register(JAL_ESCO_CTRL_REG,
                        p_packetEntry->esco_ctrl_reg);

        p_packetEntry->frequency = LSLCfreq_Get_Frequency(RX_START)
                                   | (LSLCfreq_Get_Frequency(RX_MID) << 8)
                                   | (HWradio_GetRadioMode()<<24);

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)

		mHWreg_Move_Cache_Register(JAL_EDR_CTRL_REG,
			p_packetEntry->edr_ctrl_reg);
        /*	
         * To minimise impact on ATS log TX packets using "old" TX reg format
         */
		p_packetEntry->tx_rx_stat_reg = ((p_packetEntry->tx_rx_stat_reg & 0x000000FF) |
 			                             (((p_packetEntry->tx_rx_ctrl_reg >> 1) & 0x00FFFF00) << 8) |
										 (p_packetEntry->tx_rx_stat_reg & 0xFF000000));

		p_packetEntry->esco_ctrl_reg = ( (p_packetEntry->esco_ctrl_reg & 0x000000FF) |
                                         ((p_packetEntry->esco_ctrl_reg & 0x00FF0000) >> 8) | 
                                         ((p_packetEntry->esco_ctrl_reg & 0x00001C00) << 6) | 
                                         ((p_packetEntry->esco_ctrl_reg & 0x00002000) << 6));
#endif
     }
#endif
}

/************************************************************************
 * LSLCstat_Log_Rx_Final_Status
 *
 * Logs the final status, payload start and rx register (CRC may change)
 ************************************************************************/
void LSLCstat_Log_Rx_Final_Status(t_TXRXstatus status)
{
#if (PRTH_BS_DBG_BB_PACKET_LOG_TYPE != PACKETLOG_NONE)
    t_LLCpacket *p_packetEntry = packetLog + next_packet;

    p_packetEntry->status = status;
    p_packetEntry->tx_rx_stat_reg = mHWreg_Get_Register(JAL_RX_STATUS_GP_REG);

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
        /*	
         * To minimise impact on ATS log TX packets using "old" TX reg format
         */
		p_packetEntry->tx_rx_stat_reg = ((p_packetEntry->tx_rx_stat_reg & 0x000000FF) |
 			                             (((p_packetEntry->tx_rx_stat_reg & 0x00FFFF00) >> 1) & 0x00FFFF00) |
										 (p_packetEntry->tx_rx_stat_reg & 0xFF000000));
#endif
    /*
     * Log the first 4 bytes of the payload (note buffer is toggled so ok).
     */
    {
    u_int32     *rx_acl_buf = (u_int32*)(HW_get_rx_acl_buf_addr());
    p_packetEntry->start_of_payload = *rx_acl_buf;
    }
#ifdef PACKET_LOG_RX_RESTRICTED
    /*
     * Allow packets or special event sequences to be excluded from packetLog
     * Exclude packet if corresponding bit asserted or if a special.
     */
    if ( ! ( (1u << p_packetEntry->packet_type) & PACKET_LOG_RX_RESTRICTED) )
#endif
        _LSLCstat_Select_NextPacket();
#endif
}

/************************************************************************
 * LSLCstat_Get_PacketLog
 *
 * Get Read Access to Log
 ************************************************************************/
const t_LLCpacket* LSLCstat_Get_Packet_Log(void)
{
#if (PRTH_BS_DBG_BB_PACKET_LOG_TYPE != PACKETLOG_NONE)
    return packetLog;
#else
    return (t_LLCpacket*)0; 
#endif
}

/************************************************************************
 * LSLCstat_Get_Packet_Log_Type
 *
 * Return the Log Type
 ************************************************************************/
t_LogPacketTypes LSLCstat_Get_Packet_Log_Type(void)
{
#if (PRTH_BS_DBG_BB_PACKET_LOG_TYPE != PACKETLOG_NONE)
    return log_type;
#else
    return (t_LogPacketTypes)0; 
#endif
}

/************************************************************************
 * LSLCstat_Get_Next_Packet_Log
 *
 * Return the index of the next packet
 ************************************************************************/
u_int LSLCstat_Get_Next_Packet_Index(void)
{
#if (PRTH_BS_DBG_BB_PACKET_LOG_TYPE != PACKETLOG_NONE)
    return next_packet;
#else
    return 0; 
#endif
}

/************************************************************************
 * _LSLCstat_Select_NextPacket
 *
 * Locate next packet position in buffer
 * If at end of buffer Then If Wrap go to start Else stay at end Endif
 * Note the frequency is placed at the next_packet before packet logged.
 ************************************************************************/
#if (PRTH_BS_DBG_BB_PACKET_LOG_TYPE != PACKETLOG_NONE)

static void _LSLCstat_Select_NextPacket(void)
{
    ++next_packet;
    if (next_packet==PRTH_BS_DBG_MAX_BB_MAX_PACKETS_IN_LOG)
    {
        if (log_type&LOG_WRAP)
            next_packet=0;
        else
            --next_packet;
    }
}
#endif
