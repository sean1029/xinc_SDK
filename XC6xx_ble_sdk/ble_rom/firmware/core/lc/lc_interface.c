/*************************************************************************
 * MODULE NAME:    lc_interface.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    Baseband Link Controller Interface Module
 * MAINTAINER:     Gary Fleming, John Nelson
 *
 * SOURCE CONTROL: $Id: lc_interface.c,v 1.222 2013/10/16 17:00:45 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc. 
 *     All rights reserved.
 *
 *
 * NOTES TO USERS:
 * This code will be optimised to inlines where possible.
 *
 *************************************************************************/

#include "sys_config.h"

#include <string.h>

#include "hc_const.h"
#include "lc_types.h"

#include "dl_dev.h"
#include "bt_timer.h"

#include "bt_fhs.h"
#include "bt_addr_sync.h"
#include "bt_test.h"

#include "lc_interface.h"
#include "lc_log.h"

#include "sys_mmi.h"

#include "lmp_config.h"
#include "lmp_acl_connection.h"
#include "lmp_acl_container.h"
#include "lmp_sco_container.h"
#include "lmp_ch.h"

#include "uslc_page.h"
#include "uslc_pagescan.h"
#include "uslc_inquiry.h"
#include "uslc_inquiryscan.h"
#include "uslc_testmode.h"
#include "uslc_scheduler.h"
#include "uslc_chan_ctrl.h"
#include "uslc_return_to_piconet.h"
#include "uslc_master_slave_switch.h"
#include "uslc_switch_piconet.h"
#include "uslc_sleep.h"
#include "sys_power.h"
#include "uslc_park_slave.h"
#include "uslc_park_master.h"

#include "lslc_hop.h"
#include "lslc_stat.h"
#include "lslc_slot.h"
#include "lslc_irq.h"
#include "lslc_access.h"
#include "lslc_clk.h"
#include "lslc_afh.h"
#include "lslc_class.h"
#include "lslc_freq.h"

#include "hw_radio.h"
#include "hw_codec.h"

#include "tra_codec.h"

#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
#include "hw_pta.h"
#endif

#include "le_config.h"
#include "patch_function_id.h"

/*
 * Array of funcion pointers for call back functions for higher layers.
 * Initialise via LC_Reset_All_Callbacks(), then LC_Register_Event_Handler()
 */
//void *lc_call_back_list[LC_MAX_NUMBER_OF_EVENTS];

/*
 * This device's fhs structure, effectively the LC signature of this device
 */
//t_FHSpacket LC_local_device_fhs;

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_INQUIRY_RESPONSE_SUPPORTED==1)
t_EIRpacket LC_local_EIR;
#endif

/*
 * Main device link containers, (not static since inlines used).
 */
//t_devicelink  device_links[MAX_ACTIVE_DEVICE_LINKS];

#if (PRH_BS_CFG_SYS_SCO_REPEATER_SUPPORTED==1)
u_int32 _sco_repeater_bit;            /* Cache value of SCO_Repeater_bit                */
u_int32 _sco_repeater_mode_requested; /* Flag to set SCO_REPEATER mode in safe point   */
t_deviceIndex _sco_rep_dev_index;     /* Base reference for clock calculations        */
t_clock _sco_rep_dev_offs;            /* clock offset for clock calculation          */
#endif


/*************************************************************************
 ******              Initialisation Related Methods                *******
 *************************************************************************/

/*************************************************************************
 * LC_Initialise
 *
 * Initialise all functionality of the Link Controller
 * global LC parameters, and callbacks.
 *
 * The last task is and must be to turn on LC interrupts.
 *************************************************************************/
void LC_Initialise(void)
{
 
    if(PATCH_FUN[LC_INITIALISE_ID]){
         ((void (*)(void))PATCH_FUN[LC_INITIALISE_ID])();
         return ;
    }

    const t_bd_addr  *p_local_bd_addr;

    /*
     * Initialise the global LC parameters
     */
    //SYSconfig_Set_Device_Links_Location( (void*) device_links);
    SYSconfig_Set_Local_Device_Index(0);
    //SYSconfig_Set_Max_Active_Devices_In_Piconet(PRH_BS_CFG_SYS_MAX_ACTIVE_DEVICES_PICONET);
    SYSconfig_Set_Max_Active_Devices(MAX_ACTIVE_DEVICE_LINKS);
    
    /*
     * Initialise the bd_addr for this device
     */
    p_local_bd_addr = SYSconfig_Get_Local_BD_Addr_Ref();   
    HW_set_bd_addr(p_local_bd_addr);

#if ((PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1) || (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1))
	g_LM_config_info.default_voice_setting = HWcodec_Get_Voice_Setting();
#endif
    /*
     * Initialise all the LC device links
     */
    DL_Initialise_All();
    /*
     * LSLC Initialise 
     * 1. slot interrupts handle and optional error counters
     * 2. interrupt handle
     * 3. hardware access module
     */
    /*LSLCslot_Initialise();*/

    //LSLCacc_Initialise();

    LC_Log_Initialise();

    //LSLCclk_Initialise();

    LEconfig_Init();
	
#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
	HWpta_Initialise();
#else /* ensure PTA functionality is disabled */
    HW_set_pta_mode_enable(0);
    HW_set_pta_grant_test_enable(0);
#endif

    /* 
     * Initialise the Baseband interrupts after all else initialised.
     * - This must be the last call here (and also from BT_Initialise()
     */
    LSLCirq_Initialise();
#if (PRH_BS_CFG_SYS_SCO_REPEATER_SUPPORTED==1)
    /* SCO repeater mode disabled by default*/
    _sco_repeater_bit=0;
    _sco_repeater_mode_requested = 0; 
    HW_set_sco_repeater_bit(0);
#endif

}
#ifndef REDUCE_ROM
/*************************************************************************
 * LC_Sleep
 *
 * Put LC to sleep.  (Actions still to be determined.)
 *************************************************************************/
t_error LC_Sleep(t_slots slots, t_deviceIndex device_index)
{
    /*
     * IMPORTANT: ensure that all receiver disabling, putting of radio into
     * standby etc. is done synchronised by LC, and not in this async
     *  call from the LM
     *  - i.e. no LSLCacc or HWradio calls here!
     */
#if (PRH_BS_CFG_SYS_LOW_POWER_MODE_SUPPORTED==1)
#ifndef BLUETOOTH_MODE_LE_ONLY

    if (PAGE_PROCEDURE != USLCchac_Get_Active_Procedure())
    {
         USLCsleep_Request(device_index, slots);
    }

#endif
#endif

    return NO_ERROR;
}

/*************************************************************************
 * LC_Wakeup
 *
 * Wakeup the LC.
 * timeout - specifies a timeout value - should be zero for Return from Hold,
 * and a number of slots for Sniff etc.
 * instance - specifices the precice clock instance the wakup should occur.
 *************************************************************************/
t_error LC_Wakeup(t_deviceIndex device_index, t_slots timeout, t_clock instance)
{
#if (PRH_BS_CFG_SYS_RETURN_TO_PICONET_SUPPORTED==1)
    t_devicelink* p_devicelink = DL_Get_Device_Ref(device_index);
    t_slots R2P_timeout_slots = timeout;

    if (MASTER == DL_Get_Role_Peer(p_devicelink))
    {
        /* timeout==0 - for infinite period */
        if (R2P_timeout_slots)
          R2P_timeout_slots +=
                 LC_Get_R2P_Early_Wakeup_Slots(device_index, 0); /* *2/2 */
        USLCr2p_Request(device_index, R2P_timeout_slots, instance);
    }
#endif
    return NO_ERROR;
}

#endif

/*************************************************************************
 ******      Channel Procedure Activation/Deactivation Methods     *******
 *************************************************************************/

#if (PRH_BS_CFG_SYS_PAGE_SUPPORTED==1)
/*************************************************************************
 * LC_Create_Connection
 *
 * Request a baseband connection to the presented bluetooth device address
 *************************************************************************/
t_error LC_Create_Connection(const t_bd_addr *p_bd_addr, t_clock bt_clk_offset,
                t_deviceIndex device_index, u_int16 train_reps, t_slots pageTO)
{
    t_devicelink* p_device_link;
    t_error       status;
    t_am_addr     am_addr;
    t_syncword    access_syncword;

    /*
     * AM_ADDR is defined in FHS of local Device Link when allocated.
     */
    am_addr = DL_Alloc_Am_Addr();
    if (am_addr != NO_AMADDR_AVAILABLE)
    {
        p_device_link = DL_Get_Device_Ref(device_index);

        /*
         * Set Am Addr in local FHS structure
         */
        FHS_Set_Am_Addr(&LC_local_device_fhs, am_addr);
        BTaddr_Build_Sync_Word(BDADDR_Get_LAP(p_bd_addr),
                               &access_syncword.high, &access_syncword.low);
        DL_Set_Access_Syncword (p_device_link, access_syncword);
        DL_Set_UAP_LAP(p_device_link, BDADDR_Get_UAP_LAP(p_bd_addr) );

        /*
         * bits 0->14 of bt_clk_offset is the difference between the
         * Masters own clock and the clock of the remote device.
         * Only bits 2->16 of the difference are used. Thus, when
         * setting the clock_offset in the device_link we should
         * use the following:
         *                  (bt_clk_offset << 2) & 0x1ffff
         * Clock validity is ignored at LC.
         */

         DL_Set_Clock_Offset(p_device_link, (bt_clk_offset << 2) & 0x1ffff);

#if 1 // GF added 12 Nov 01:48  
      //--- IMPORTANT AS THIS ALLOWS US TO RECOVER CORRECTLY FROM A FAILED PAGE 
	  //--- WHEN WE HAVE ANOTHER PICONET ACTIVE.

		 USLCsp_Set_Activated_Device_Index(device_index);
#endif
         status = USLCpage_Request(device_index, train_reps, pageTO);
         /*
          * Add the AM address to the device link, so the AM address is
          * returned to the pool when the LM closes the link.
          */
         if(status == UNSPECIFIED_ERROR)
         {
             DL_Set_AM_Addr(p_device_link, am_addr);
         }
    }
    else
    {
        status = MAX_NUM_CONNECTIONS;
    }
    return status;
}

/*************************************************************************
 * LC_Page_Cancel
 *
 * Cancel a page request or ongoing page, if still active.
 *************************************************************************/
t_error LC_Page_Cancel(void)
{
   USLCpage_Cancel();
   return NO_ERROR;
}
#endif /*(PRH_BS_CFG_SYS_PAGE_SUPPORTED==1)*/


#if (PRH_BS_CFG_SYS_PAGE_SCAN_SUPPORTED==1)
/*************************************************************************
 * LC_Page_Scan_Request
 *
 * Request an page scan for the presented lap for the page window timeout.
 *************************************************************************/
t_error LC_Page_Scan_Request(t_deviceIndex device_index,t_slots window_timeout,
                             t_scan_type scan_type)
{
    return USLCpageScan_Request(device_index,window_timeout,scan_type);
}

/*************************************************************************
 * LC_Page_Resume
 *
 *************************************************************************/
t_error LC_Page_Resume(void)
{
	USLCpage_Page_Resume();
	return NO_ERROR;
}
/*************************************************************************
 * LC_Page_Scan_Cancel
 *
 * Cancel a previously requested page scan
 *************************************************************************/
void LC_Page_Scan_Cancel(void)
{
    USLCpageScan_Cancel();
}
#endif /*(PRH_BS_CFG_SYS_PAGE_SCAN_SUPPORTED==1)*/


#if (PRH_BS_CFG_SYS_INQUIRY_SUPPORTED==1)
/*************************************************************************
 * LC_Inquiry_Request
 *
 * Request an inquiry for the presented lap address
 *
 *************************************************************************/
t_error LC_Inquiry_Request(t_lap inq_lap, t_slots window, u_int16 train_reps)
{
	/*
	 * Note - LC_Restore_Piconet checks LC_Get_Active_Piconet, so must
	 * not invalidate/override this via USLCsp_Set_Activated_Device_Index.
	 */
    return USLCinq_Inquiry_Request(inq_lap, window, train_reps);
}

/*************************************************************************
 * LC_Inquiry_Resume
 *
 *************************************************************************/
t_error LC_Inquiry_Resume(void)
{
	USLCinq_Inquiry_Resume();
	return NO_ERROR;
}

/*************************************************************************
 * LC_Inquiry_Cancel
 *
 * Cancel a previously requested inquiry
 *************************************************************************/
t_error LC_Inquiry_Cancel(void)
{
    USLCinq_Inquiry_Cancel();
    return NO_ERROR;
}
#endif /*(PRH_BS_CFG_SYS_INQUIRY_SUPPORTED==1)*/


#if (PRH_BS_CFG_SYS_INQUIRY_SCAN_SUPPORTED==1)
/*************************************************************************
 * LC_Inquiry_Scan_Request
 *
 * Request an inquiry scan for the presented lap for the inquiry window
 * and inquiry timeout.
 *************************************************************************/
t_error LC_Inquiry_Scan_Request(t_lap inquiry_lap, t_slots window, 
                                                t_scan_type scan_type)
{
    return USLCinqScan_Request(inquiry_lap, window,scan_type);
}

/*************************************************************************
 * LC_Inquiry_Scan_Cancel
 *
 * Cancel a previously requested inquiry scan
 *************************************************************************/
void LC_Inquiry_Scan_Cancel(void)
{
    USLCinqScan_Cancel();
}
#endif /*(PRH_BS_CFG_SYS_INQUIRY_SCAN_SUPPORTED==1)*/


/*************************************************************************
 * LC_Set_Active_Piconet
 *
 * Main scatternet interface. Sets the active piconet and activates the
 * identified piconet device link if it is not the active piconet.
 *
 * If change_piconet not requested, then assumed already on that piconet,
 * in which case only necessary to reactive for error recovery.
 * 
 *************************************************************************/
#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
void  LC_Set_Active_Piconet(t_deviceIndex device_index, boolean change_piconet)
{
    if ((change_piconet) || (USLCsp_Set_Activated_Device_Index(device_index) != NO_ERROR))
    {
        USLCsp_Request(device_index,USLCsp_CHANGE_PICONET);
    }
}
#endif

/*************************************************************************
 * LC_Get_Active_Piconet
 *
 * Returns activated piconet device index.
 *
 *************************************************************************/
#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
t_deviceIndex LC_Get_Active_Piconet(void)
{
    return USLCsp_Get_Activated_Device_Index();
}
#endif


/*
 * Park related interface.
 */
#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
/*************************************************************************
 * LC_Register_Access_Window_Callback_Handler
 *
 * Register the callback handler for the Access Window 
 *************************************************************************/
void LC_Register_Access_Window_Callback_Handler(t_callback func_ptr)
{
    USLCparkMaster_Register_Access_Window_Callback_Handler( func_ptr);
}

/*************************************************************************
 * LC_Park_Request
 *
 * Request to park a device.
 *************************************************************************/
t_error LC_Park_Request(t_deviceIndex device_index, t_pm_addr pm_addr)
{
    if (MASTER == DL_Get_Role_Peer(DL_Get_Device_Ref(device_index)))
        return USLCparkSlave_Park_Request(device_index, pm_addr);
    else
        return USLCparkMaster_Park_Request(device_index, pm_addr);
}

/*************************************************************************
 * LC_Unpark_Request
 *
 * Request to unpark a device.
 * 
 *************************************************************************/
t_error LC_Unpark_Request(t_deviceIndex* device_index,t_parkDeviceIndex park_device_index, t_role role_of_peer, 
                    t_am_addr am_addr, t_unpark_type unpark_type,t_clock start_timer_ref, t_slots n_poll)
{
    t_error status = NO_ERROR;

    if (role_of_peer==MASTER)
    {
        status = USLCparkSlave_Unpark_Request(device_index,park_device_index,unpark_type,am_addr,
                       start_timer_ref,n_poll);
    }
    else
    {
        status = USLCparkMaster_Unpark_Request(device_index,park_device_index,unpark_type,am_addr,
                                start_timer_ref,n_poll);
    }

    return status;
}


/*************************************************************************
 * LC_Access_Window_Request
 *
 * Request to create an access window.
 *************************************************************************/
t_error LC_Access_Window_Request(u_int n_acc_slot, t_slots t_access, u_int m_access, 
                 u_int8 n_poll, t_deviceIndex device_index, t_ar_addr ar_addr)
{
    if (m_access != 0) 
        return USLCparkSlave_Access_Window_Request(t_access, m_access, 
                                        n_acc_slot, n_poll, device_index, ar_addr);
    else
        return USLCparkMaster_Access_Window_Request(n_acc_slot);
}

/*************************************************************************
 * LC_Unpark_Rollback
 *
 * Rollback the Unparked device
 *
 *************************************************************************/
t_error LC_Unpark_Rollback(void)
{
    if (PARKSLAVE_PARK_PROCEDURE == USLCchac_Get_Active_Procedure())
        return USLCparkSlave_Unpark_Rollback();
    else if ( PARKMASTER_PARK_PROCEDURE == USLCchac_Get_Active_Procedure())
        return USLCparkMaster_Unpark_Rollback();
    else
        return UNSPECIFIED_ERROR;
}

/*************************************************************************
 * LC_Unpark_Commit
 *
 * Rollback the Unparked device
 *
 *************************************************************************/
t_error LC_Unpark_Commit(void)
{
    if (PARKSLAVE_PARK_PROCEDURE == USLCchac_Get_Active_Procedure())
        return USLCparkSlave_Unpark_Commit();
    else if ( PARKMASTER_PARK_PROCEDURE == USLCchac_Get_Active_Procedure())
        return USLCparkMaster_Unpark_Commit();
    else
        return UNSPECIFIED_ERROR;
}

#endif /* (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1) */

#if (BUILD_TYPE==UNIT_TEST_BUILD)
/*************************************************************************
 * LC_Get_Event_Handler
 *
 * Enable access to handlers for checking.
 *************************************************************************/
void *LC_Get_Event_Handler(t_LC_Event_Id event_id)
{
    return lc_call_back_list[event_id];
}
#endif


#if (PRH_BS_CFG_SYS_SCO_REPEATER_SUPPORTED==1)
/*************************************************************************
 * LC_Set_SCO_Repeater_Bit
 *
 * Sets the SCO repeater Bit in the Tabasco.
 *
 * Note! Must be Set ONLY while HW_get_slave()==1 !!!
 *
 *************************************************************************/
void LC_Set_SCO_Repeater_Bit(u_int32 value)
{
    if (value)
       _sco_repeater_mode_requested = 1;
    else
    {
       HW_set_sco_repeater_bit(0);
       _sco_repeater_mode_requested = 0;
       _sco_repeater_bit=0;
    }
}
 
/*************************************************************************
 * LC_Get_SCO_Repeater_Bit
 *
 * Set if SCO repeater Bit is set in the Tabasco.
 *
 *************************************************************************/
u_int32 LC_Get_SCO_Repeater_Bit(void)
{
    return _sco_repeater_bit;
}
#endif

/*
 * EDR related
 */
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
/*************************************************************************
 * LC_Set_PTT_ACL
 *
 * Enable EDR ACL packets for this link 
 *************************************************************************/
void LC_Set_PTT_ACL(t_deviceIndex device_index, boolean ptt_acl){
   DL_Set_PTT_ACL(DL_Get_Device_Ref(device_index), ptt_acl);
}

/*************************************************************************
 * LC_Get_PTT_ACL
 *
 * Determine if EDR ACL packets are enabled for this link ?
 *************************************************************************/
boolean LC_Get_PTT_ACL(t_deviceIndex device_index){
   return DL_Get_PTT_ACL(DL_Get_Device_Ref(device_index));
}

/*************************************************************************
 * LC_Set_PTT_ESCO
 *
 * Enable EDR eSCO packets for this link 
 *************************************************************************/
void LC_Set_PTT_ESCO(t_deviceIndex device_index, boolean ptt_esco){
   DL_Set_PTT_ESCO(DL_Get_Device_Ref(device_index), ptt_esco);
}

/*************************************************************************
 * LC_Get_PTT_ESCO
 *
 * Determine if EDR eSCO packets are enabled for this link ?
 *************************************************************************/
boolean LC_Get_PTT_ESCO(t_deviceIndex device_index){
   return DL_Get_PTT_ESCO(DL_Get_Device_Ref(device_index));
}
#endif

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_INQUIRY_RESPONSE_SUPPORTED==1)
/*************************************************************************
 * LC_Get_Local_EIR
 *
 * Fetches the local EIR from LC_local_EIR.
 *************************************************************************/
u_int8 LC_Get_Local_EIR(t_cmd_complete_event* p_event)
{
    p_event->p_u_int8 = &(LC_local_EIR.data[0]);
    p_event->number = LC_local_EIR.length;
    p_event->mode = LC_local_EIR.fec_required;
    return NO_ERROR;
}

/*************************************************************************
 * LC_Get_Local_EIR_Payload
 *
 * Fetches the local EIR payload from LC_local_EIR.
 *************************************************************************/
u_int8* LC_Get_Local_EIR_Payload(void)
{
    return &(LC_local_EIR.data[0]);
}

/*************************************************************************
 * LC_Get_Local_EIR_Length
 *
 * Fetches the local EIR length from LC_local_EIR.
 *************************************************************************/
u_int8 LC_Get_Local_EIR_Length(void)
{
    return LC_local_EIR.length;
}

/*************************************************************************
 * LC_Get_Local_Packet_Type
 *
 * Fetches the local EIR packet type from LC_local_EIR.
 *************************************************************************/
t_packet LC_Get_Local_EIR_Packet_Type(void)
{
    return LC_local_EIR.optimal_packet_type;
}

/*************************************************************************
 * LC_Set_Local_EIR
 *
 * Writes a new local EIR into LC_local_EIR.
 *************************************************************************/
u_int8 LC_Set_Local_EIR(u_int8 fec_required, u_int8* p_eir)
{
    u_int16 length=0;
    t_packet pkt;

    LC_local_EIR.fec_required = fec_required;

    while (p_eir[length]!=0 && length<MAX_EIR_PACKET_LENGTH)
        length+=(p_eir[length]+1);

    if(length>MAX_EIR_PACKET_LENGTH)
        length = MAX_EIR_PACKET_LENGTH;

    LC_local_EIR.length = length;
    
    memcpy(&LC_local_EIR.data[0], p_eir, length);
    memset(&LC_local_EIR.data[length], 0, MAX_EIR_PACKET_LENGTH-length);

    if (length<=MAX_DM1_USER_PDU)
        pkt = DM1;
    else if(fec_required==0 && length<=MAX_DH1_USER_PDU)
        pkt = DH1;
#if (PRH_BS_CFG_SYS_LMP_THREE_SLOT_PKT_SUPPORTED==1)
    else if(length<=MAX_DM3_USER_PDU)
        pkt = DM3;
    else if(fec_required==0 && length<=MAX_DH3_USER_PDU)
        pkt = DH3;
#endif
#if (PRH_BS_CFG_SYS_LMP_FIVE_SLOT_PKT_SUPPORTED==1)
    else if(length<=MAX_DM5_USER_PDU)
        pkt = DM5;
    else if(fec_required==0)
        pkt = DH5;
#endif
    else
        return EC_EXTENDED_INQUIRY_RESPONSE_TOO_LARGE;

    LC_local_EIR.optimal_packet_type = pkt;

    return NO_ERROR;
}
#endif

#if (PRH_BS_CFG_SYS_INQUIRY_RESPONSE_TX_PWR_SUPPORTED == 1)

void LC_Set_Inquiry_Tx_Power(s_int8 power_level)
{
	// HW Radio which conterts for s_int Power Level to a corresponding
	// closest Power Level in the HW_Radio Power Table.

	g_LM_config_info.inquiry_tx_power_level = HWradio_Convert_Tx_Power_to_Tx_Power_Level_Units(power_level);
}

s_int8 LC_Get_Inquiry_Response_Tx_Power(void)
{
	   //return HWradio_Convert_Tx_Power_Level_Units_to_Tx_Power(g_LM_config_info.inquiry_response_tx_power_level);
       return ((s_int8 (*)(u_int8))PATCH_FUN[HWRADIO_CONVERT_TX_POWER_LEVEL_UNITS_TO_TX_POWER])(g_LM_config_info.inquiry_response_tx_power_level);

}

#endif

