 /*************************************************************************
 * MODULE NAME:    uslc_pagescan.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    Upper Software Link Controller Page Scan statemachine.    
 * MAINTAINER:     Conor Morris
 *
 * SOURCE CONTROL: $Id: uslc_pagescan.c,v 1.139 2013/10/07 19:31:03 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 *************************************************************************/

#include "sys_config.h"
#include "lc_interface.h"

#if (PRH_BS_CFG_SYS_PAGE_SCAN_SUPPORTED==1)

#include "lc_log.h"

#include "bt_timer.h"
#include "bt_fhs.h"


#include "hw_lc.h"
#include "hw_memcpy.h"

#include "sys_mmi.h"

#include "dl_dev.h"

#include "lslc_pkt.h"
#include "lslc_access.h"
#include "lslc_clk.h"
#include "lslc_freq.h"
#include "lslc_hop.h"

#include "uslc_pagescan.h"
#include "uslc_chan_ctrl.h"

#include "lmp_acl_container.h"
#include "lmp_sco_container.h"
#include "lmp_config.h"

#include "sys_debug_config.h"

typedef enum
{ 
    OS_START,               /* start control-state */
    OS_END,                 /* end control-state */
    OS_NOT_ACTIVE,          /* procedure not active */
    OS_RESUME_PAGE_SCANNING,/* resume scanning (after advancing past the */
                            /* OS_PAGE_SCAN state                        */
    OS_PAGE_SCAN,           /* page scan control-state */
    OS_SLAVE_PAGE_RESP,     /* slave page response control-state */
    OS_POLL_RESP_RX,        /* Slave waiting for POLL pkt from Master */
    OS_CONNECTED_TO_MASTER,  /* connected to master control-state */
    OS_END_AT_NEXT_TX_START,
#if (PRH_BS_DEV_TIM1_CORRECTED_PAGESCAN_WIDTHS==1)
    OS_START_2ND_TRAIN,
#endif
    OS_PAGESCAN_2ND_TRAIN
} t_USLCpageScan_ctrl_state; /* bluetooth device control states */

typedef struct t_pageScan
{
    t_timer page_scan_window_timer,
            page_resp_timer,
            new_connection_timer;

    t_slots page_scan_window_timeout;
    t_slots page_resp_timeout;
    t_slots new_connection_timeout;

    t_devicelink *p_active_device_link;

    t_USLCpageScan_ctrl_state ctrl_state;

    boolean scan_for_one_period_only,
            notify_procedure_start;

    t_scan_type scan_type;
    t_FHSpacket fhs_packet;

} t_pageScan;

t_pageScan _pageScan_info;


/*
 * Internal helper functions.
 */
void _USLCpageScan_OS_START(void);
void _USLCpageScan_Process_ID_Pkt_During_Scanning(void);

/*
 * The following code supports different slave clock configurations 
 * as a result of issues of AEB FPGA clocks.
 */

#define BT_CLK_INIT_OFFSET 0

/******************************************************************
 *
 * USLCchac_Page_Scan_Request
 *
 * Request from a higher layer to start a Page Scan procedure
 *
 ******************************************************************/
t_error USLCpageScan_Request(
    t_deviceIndex deviceIndex, /* Indicates the deviceLink to be used for */
                               /* for the Page Scanning.  If the scanning */
                               /* results in a connection establishment,  */
                               /* then this device link will represent the*/
                               /* communication relationship with the     */
                               /* Master                                  */

    t_clock page_scan_window_timeout, /* Maximum ammount of time before the */
                                      /* Page Scanning is halted            */

    t_scan_type scan_type /* Standard, Interlaced or Continuous */
)
{
    t_error    status;
    t_pageScan volatile *pageScan = &_pageScan_info;

    LC_Log_Procedure_Request(PAGESCAN_PROCEDURE);
 
    /*
     * Only activate procedure if currently not active and the procedure
     * parameterss are within the indicated bounds.
     */    
    if ( (pageScan->ctrl_state == OS_NOT_ACTIVE) &&
        ( (deviceIndex!=0) && (deviceIndex<=SYSconfig_Get_Max_Active_Devices()) ) )    
    {

        LC_Log_Procedure_Accept(PAGESCAN_PROCEDURE);
        pageScan->notify_procedure_start=TRUE;

        pageScan->page_scan_window_timeout = page_scan_window_timeout;
        pageScan->scan_type=scan_type;
        pageScan->p_active_device_link=DL_Get_Device_Ref(deviceIndex);

        pageScan->scan_for_one_period_only=TRUE;

        /*
         * Initialise the ctrl_state of the DeviceLink to osStart
         */
        pageScan->ctrl_state = OS_START;

       /*
        * Note: The access syncword needs to be set here because PrepareRx()
        * is called before the start of the Page_Scan statemachine
        */
        DL_Set_Access_Syncword(pageScan->p_active_device_link, SYSconfig_Get_Local_Syncword() );
        DL_Set_UAP_LAP(pageScan->p_active_device_link, BDADDR_Get_UAP_LAP( SYSconfig_Get_Local_BD_Addr_Ref() ) );

        /* Device Link initialisation has taken place in the
         * LC_Alloc_Link(). Just Need to ensure that the
         * role_peer is set to Master.
         * The Piconet Index is uniquely assigned for Slave's piconet
         */
        DL_Set_Role_Peer(pageScan->p_active_device_link, MASTER);
        DL_Assign_Piconet_Index(pageScan->p_active_device_link);

        USLCchac_Procedure_Request(PAGESCAN_PROCEDURE, TRUE);
        status = NO_ERROR;
    }
    else
    {
        status = UNSPECIFIED_ERROR;
    }

    return status;
}


/******************************************************************
 *
 *USLCchac_Page_Scan_Cancel
 *
 * Request a higher layer to cancel the Page Scan procedrue
 *
 *****************************************************************/
void USLCpageScan_Cancel(void)
{
    t_pageScan volatile *pageScan = &_pageScan_info;

     if (OS_NOT_ACTIVE != pageScan->ctrl_state)
     {
         /*
          * Setting the scan_type to STANDARD_SCAN ensures
          * that in the OS_END ctrl_state, the device will
          * not enter continuous scanning.
          */
         pageScan->scan_type = STANDARD_SCAN; 
         pageScan->ctrl_state = OS_END;
     }

    LC_Log_Procedure_Cancel(PAGESCAN_PROCEDURE);
}

/******************************************************************
 *
 * _USLCchac_Page_Scan
 *
 * 'One-shot' statemachine for the Page Scan  procedure
 *
 * Once the Page_Scan  procedure has started (super state=Page_Scan)
 * this function will be invoked during each Tx slot allocated to the
 * USLC channel controller.
 *
 * Assumption:
 * 1- When the device enters the SlavePageResponse state the Slave freezes
 *    the X-input.  For each response slot, it is assumed that an X-input
 *    value 1 larger than the preceeding response slot is used.  However, 
 *    the 1st response is made with the X-input kept at the same value as 
 *    it was when the ID packet was recognised.
 *      
 * 2- If the slave does not receive the FHSpkt within 'page_resp_timeout'
 *    then the slave returns to the page scan state for one
 *    scan period.  If no page message is received during the
 *    additional scan period, the slave will resume scanning at its
 *    regular scan interval (i.e. exit the current 'One shot' Page Scan).
 *    It is assumed that : scan period == page_scan_window_timeout.
 *
 *****************************************************************/
#if (PRH_BS_DEV_TIM1_CORRECTED_PAGESCAN_WIDTHS==1)
void USLCpageScan_Page_Scan(boolean look_ahead)
#else
void USLCpageScan_Page_Scan(boolean dummy)
#endif
{
    t_syncword syncword;
    t_packet event;
    t_LC_Event_Info event_info;
    t_pageScan *pageScan = &_pageScan_info;

    event = DL_Get_Rx_Packet_Type(pageScan->p_active_device_link);

#if (PRH_BS_DEV_TIM1_CORRECTED_PAGESCAN_WIDTHS==1)
    if ((look_ahead) && (pageScan->ctrl_state != OS_PAGE_SCAN)
         && (pageScan->ctrl_state != OS_PAGESCAN_2ND_TRAIN))
    { /* LOOK_AHEAD(TIM1) is only relevant to OS_PAGE_SCAN/OS_PAGESCAN_2ND_TRAIN state */
        return;
    }
#endif

    if (OS_START == pageScan->ctrl_state)
    {
        _USLCpageScan_OS_START(); /*Lamda transition to OS_PAGE_SCAN state*/
    }

    /* 
     * PageScan Statemachine
     */
    switch(pageScan->ctrl_state)
    {

    /*
     * if RESUME_EARLIER is off, then handle
     * resuming here at Tim0 as before, else
     * handle this at the preceding TIM2.
     */
#if (PRH_BS_DEV_RESUME_PAGE_SCANNING_EARLIER==0)
    case OS_RESUME_PAGE_SCANNING:
       if (CONTINUOUS_SCAN == pageScan->scan_type)
        {
            /*
             * Set the page_scan_window_timer to expire 1 tick
             * after CLKN bit 12 changes, ie next 1.28 sec boundary
             */
            pageScan->page_scan_window_timer=
                BTtimer_Set_Ticks( 0x1000 - (HW_get_native_clk() & 0x0fff) );

        }
        else
        {
            pageScan->page_scan_window_timer =
                    BTtimer_Set_Slots(pageScan->page_scan_window_timeout);
        }

        /*
         * If the slave does not receive the FHSpkt within
         * pageScan->page_resp_timeout then the slave returns to the page
         * scan state for one scan period. 
         */

        USLCchac_Set_Device_State(Page_Scan);
        pageScan->ctrl_state = OS_PAGE_SCAN;
        HWradio_SetRadioMode(RADIO_MODE_FULL_RX);
        LSLCpkt_Prepare_Receiver_ID(pageScan->p_active_device_link, Page_Scan);
        break;
#endif

    case OS_PAGE_SCAN: /*super_state=PageScan */
        if (IDpkt == event)
        {
            _USLCpageScan_Process_ID_Pkt_During_Scanning();       
        }
#if (PRH_BS_DEV_TIM1_CORRECTED_PAGESCAN_WIDTHS==1)
        else if (BTtimer_Is_Expired(pageScan->page_scan_window_timer) && (look_ahead))
#else
        else if (BTtimer_Is_Expired(pageScan->page_scan_window_timer))
#endif
        {
            if (INTERLACED_SCAN == pageScan->scan_type)
            {
#if (PRH_BS_DEV_TIM1_CORRECTED_PAGESCAN_WIDTHS==1)
                /* goto standby and start 2nd train on next tx start */
                HWradio_SetRadioMode(RADIO_MODE_STANDBY);
                pageScan->ctrl_state = OS_START_2ND_TRAIN;
#else
                LSLChop_Set_Train(Alternative_Scan);     
                LSLCpkt_Prepare_Receiver_ID(pageScan->p_active_device_link,Page_Scan);
                pageScan->page_scan_window_timer =
                BTtimer_Set_Slots(pageScan->page_scan_window_timeout);
                pageScan->ctrl_state = OS_PAGESCAN_2ND_TRAIN;
#endif
            }
            else if (STANDARD_SCAN == pageScan->scan_type)
            { /* goto standby in os_end */
                pageScan->ctrl_state = OS_END;
            }
            else /* if (CONTINUOUS_SCAN == pageScan->scan_type) */
            { 
                /*
                 * Set the page_scan_window_timer to expire 1 tick
                 * after CLKN bit 12 changes, ie next 1.28 sec boundary
                 */
                pageScan->page_scan_window_timer=
                    BTtimer_Set_Ticks( 0x1000 - (HW_get_native_clk() & 0x0fff) );
                LSLCpkt_Prepare_Receiver_ID(pageScan->p_active_device_link,Page_Scan);
            }
        }
#if (PRH_BS_CFG_SYS_PERSISTENT_SCO_IN_SCANS_SUPPORTED==1)
        /*
         * Special consideration when scanning with persistent sco, to restore
         * radio to full rx and prepare receiver for id packets again.
         */
        else if(HWradio_GetRadioMode()!=RADIO_MODE_FULL_RX && LMscoctr_Get_Number_SYN_Connections())
        {
            HWradio_SetRadioMode(RADIO_MODE_FULL_RX);
            LSLCpkt_Prepare_Receiver_ID(pageScan->p_active_device_link, Page_Scan);
        }
#endif
        break;

#if (PRH_BS_DEV_TIM1_CORRECTED_PAGESCAN_WIDTHS==1)
	case OS_START_2ND_TRAIN:
        LSLChop_Set_Train(Alternative_Scan);     
		HWradio_SetRadioMode(RADIO_MODE_FULL_RX);
        LSLCpkt_Prepare_Receiver_ID(pageScan->p_active_device_link,Page_Scan);
        pageScan->page_scan_window_timer =
        BTtimer_Set_Slots(pageScan->page_scan_window_timeout);
        pageScan->ctrl_state = OS_PAGESCAN_2ND_TRAIN;
	    break;
#endif

    case OS_PAGESCAN_2ND_TRAIN:
        if (event == IDpkt)
        {
            _USLCpageScan_Process_ID_Pkt_During_Scanning();
        }
#if (PRH_BS_DEV_TIM1_CORRECTED_PAGESCAN_WIDTHS==1)
        else if (BTtimer_Is_Expired(pageScan->page_scan_window_timer) && (look_ahead))
#else
        else if (BTtimer_Is_Expired(pageScan->page_scan_window_timer))
#endif
        {
            pageScan->ctrl_state = OS_END;
        }
#if (PRH_BS_CFG_SYS_PERSISTENT_SCO_IN_SCANS_SUPPORTED==1)
        /*
         * Special consideration when scanning with persistent sco, to restore
         * radio to full rx and prepare receiver for id packets again.
         */
        else if(HWradio_GetRadioMode()!=RADIO_MODE_FULL_RX && LMscoctr_Get_Number_SYN_Connections())
        {
            HWradio_SetRadioMode(RADIO_MODE_FULL_RX);
            LSLCpkt_Prepare_Receiver_ID(pageScan->p_active_device_link, Page_Scan);
        }
#endif
        break;

    case OS_SLAVE_PAGE_RESP:   /*super_state=SlavePageResponse*/
        if (FHSpkt == event)
        {
            /*
             * once upon a time, RTX sent us an AM ADDR of 0 in an FHS.
             * Need to protect against this 
             *
             * Since we want to get the ID out before extracting (i.e. memcpying)
             * and processing the FHS, we need to peek directly into the HW buffers
             * here...
             */
#define JAL_PEEK_AT_FHS_AM_ADDR_IN_RX_BUF_ADDR ((u_int8*)HW_get_rx_acl_buf_addr() + 12)
#define JAL_PEEK_AT_FHS_AM_ADDR_IN_RX_BUF_SHFT 16
#define JAL_PEEK_AT_FHS_AM_ADDR_IN_RX_BUF_MASK 0x00070000

            if (0 != mGetHWEntry(JAL_PEEK_AT_FHS_AM_ADDR_IN_RX_BUF))
            {
                t_FHSpacket *p_FHSpacket;
                t_devicelink *p_device_link = pageScan->p_active_device_link;
                /*
                 * Note the sequence: Send an ID, Update BT clock must be preserved.
                 * The old bt_clk is used to determine the X delta in Hop Calculator
                 */
                 LSLCpkt_Generate_ID(p_device_link, SlavePageResponse);
                 LSLCacc_Enable_Transmitter(); /* immediate transmission */

                /*
                 * Extract relevant fields direct from FHS payload to
                 * both local fhs copy and device link
                 */
                p_FHSpacket = (t_FHSpacket*)&pageScan->fhs_packet;
                LSLCacc_Extract_FHS_Payload(p_FHSpacket);

                DL_Set_UAP_LAP(p_device_link,
                    FHS_Get_UAP_LAP(p_FHSpacket) );
                DL_Set_NAP(p_device_link,
                    FHS_Get_NAP(p_FHSpacket) );
                /*
                 * Ensure that Hop Selection Kernel is setup
                 */
                LSLCfreq_Assign_Frequency_Kernel(p_device_link);
               {
                   t_clock clk_offset_adjusted;
                   t_clock remote_clk = FHS_Get_CLK(p_FHSpacket);
                   t_clock current_clk;
                   {
                       t_frame_pos current_frame_posn;

                       /* if ((HW_get_bt_clk() & 0x03) == 0)  */ /* assume TIM2 / RX_START */
                       {
                           current_frame_posn = RX_START;
                           if (!LSLCclk_Is_Safe_To_Read_Native_Clk(RX_START))
                           {
                               LSLCclk_Wait_For(TX_START);
                               current_frame_posn = TX_START;
                           }
                       }
                   }
                   current_clk = HW_get_native_clk();

                   clk_offset_adjusted = (remote_clk - (current_clk & 0x0ffffffc));
                   DL_Set_Clock_Offset(pageScan->p_active_device_link, clk_offset_adjusted);
#if 1 // Added back by GF - 06 April - as page scan did not seem to work - Removedagain on 7 april
                   /* adjustment for bt clk relative */
                   clk_offset_adjusted += ((current_clk & 0x0ffffffc) - (HW_get_bt_clk() & 0x0ffffffc));

#endif
                    HW_set_bt_clk_offset(clk_offset_adjusted);
                }

                /*
                 * Assign the AM_ADDR contained in the FHS packet to the Device_Link
                 */
                DL_Set_AM_Addr(pageScan->p_active_device_link,
                    FHS_Get_AM_Addr(p_FHSpacket));

                /*
                 * The new_connection timer is increased by 1 in order to allow
                 * for the fact that the Native clocks in the Slave and Master
                 * are not alligned. Since the Native clock is used to 'drive'
                 * the timers, the plus 1 is used to ensure that the Slave
                 * doesn't stop listening for the POLL packet 1 slot too early.
                 */
                pageScan->new_connection_timer=
                    BTtimer_Set_Slots(pageScan->new_connection_timeout+1);


                /*
                 * Once the ID packet has been sent, need to set the AccessCode
                 * = Master CAC. However, cannot set the correlator=AccessCode
                 * (i.e. Master CAC)until it has been determined that there
                 * is nothing in the next TxRx slot. This can only be
                 * determined at the USLCCtrl 'level'.
                 */
                FHS_Get_Syncword_Ex(p_FHSpacket, &syncword);
                DL_Set_Access_Syncword(pageScan->p_active_device_link, syncword);

                pageScan->scan_for_one_period_only = TRUE;
                HWradio_SetRadioMode(RADIO_MODE_TX_RX);
                pageScan->ctrl_state = OS_POLL_RESP_RX;

                /*
                 * 
                 For encryption the Master's BD_ADDR must be set in the
                 * hardware on the slave
                 */
                HW_set_bd_addr_via_uap_lap_nap(
                    DL_Get_UAP_LAP(pageScan->p_active_device_link),
                    DL_Get_NAP(pageScan->p_active_device_link) );
            }
            else /* AM ADDR in FHS is equal to zero !!! Illegal FHS, so ignore */
            {
                /*
                 * must check timers here, otherwise a denial of service is to continually
                 * send FHSes with AM_ADDR of 0...
                 */
                goto OS_SLAVE_PAGE_RESP_TIMER_CHECK;
            }
            break;
        }

/* not a case statement, but a goto to common code for this state */
OS_SLAVE_PAGE_RESP_TIMER_CHECK:
        if (BTtimer_Is_Expired(pageScan->page_resp_timer))
        {
            /*
             * If not transitioning radio driver to tx_rx mode from slave_page_resp,
             * then must exit through standby mode: required for some radio drivers.
             */
            HWradio_SetRadioMode(RADIO_MODE_STANDBY);
            
            if ((pageScan->scan_for_one_period_only) || (CONTINUOUS_SCAN == pageScan->scan_type))
            {
                pageScan->scan_for_one_period_only = FALSE;
#if (PRH_BS_DEV_RESUME_PAGE_SCANNING_EARLIER==0)
                pageScan->ctrl_state = OS_RESUME_PAGE_SCANNING;
#else
                USLCpageScan_Resume();
#endif
            }
            else
            {
                pageScan->scan_for_one_period_only = TRUE;
                /*
                 * We know that we entered the state machine from either
                 * a NO_PKD_INTR or PKD_INTR.
                 * Thus, will only exit the procedure at the next
                 * TX_START interrupt.
                 */               
                pageScan->ctrl_state = OS_END_AT_NEXT_TX_START;
            }
        }
        break;

    case OS_POLL_RESP_RX:  /*super_state=Connection*/
        USLCchac_Set_Device_State(Connection);
        if (POLLpkt == event) /* ARQN must == NAK */
        {
            /*
             * At this point should be using the CAC of the Master
             */
            LSLCpkt_Generate_NULL(pageScan->p_active_device_link);

            pageScan->ctrl_state = OS_CONNECTED_TO_MASTER;

//            SYSmmi_Display_Event(eSYSmmi_LC_Connection_Event); /* show USLC connected via SYSmmi */
        }
        else if (DM1 == event)
        {
            /*
             *
             * Due to known hardware bug in first generation Ericsson devices,
             * they transmit a DM1 packet to toggle the SEQN bit.  The DM1 is
             * of zero-payload data length. 
             *
             * Our slave must acknowledge this DM1 packet.
             */

            LSLCpkt_Generate_NULL(pageScan->p_active_device_link);
        }
        else if (BTtimer_Is_Expired(pageScan->new_connection_timer))
        {
            /*
             * If the POLL packet is not received within newconnectionTO then
             * the Slave will return to the PageScan state
             * In order to go back Page Scanning, the syncword used for the
             * scanning must be restored to that of the Slave's syncword
             */
            DL_Set_Access_Syncword(pageScan->p_active_device_link, SYSconfig_Get_Local_Syncword());
            DL_Set_UAP_LAP(pageScan->p_active_device_link, BDADDR_Get_UAP_LAP(SYSconfig_Get_Local_BD_Addr_Ref()));
            HW_set_bd_addr(SYSconfig_Get_Local_BD_Addr_Ref());

            /*
             * Must set the AM_ADDR in the Slave back to 0
             */
            DL_Set_AM_Addr(pageScan->p_active_device_link, 0);

            /*
             * Must reset.
             */
            pageScan->scan_for_one_period_only=TRUE;

#if (PRH_BS_DEV_RESUME_PAGE_SCANNING_EARLIER==0)
            pageScan->ctrl_state = OS_RESUME_PAGE_SCANNING;
#else
            USLCpageScan_Resume();
#endif
        }
        break;

        case OS_END_AT_NEXT_TX_START:
              /*
               * We know that we entered the state machine
               * from a TX_START interrupt. Thus, the procedure
               * can be exited here.
               */
              pageScan->ctrl_state = OS_END;
              break;

    default:
        break;
    }

       if (OS_END == pageScan->ctrl_state)
       {
#if 0 //  GF 10 Feb 2011 Removed for Scatternet - GF BQB from Renesas
           /*
            * Indicate to the USLCchac that the PageScan procedure is finished,
            * and indicate to set super_state=previous_super_state
            */
           USLCchac_Procedure_Request(PAGESCAN_PROCEDURE, FALSE);
           LC_Callback_Event_Handler(LC_EVENT_PAGE_SCAN_TIMEOUT,&event_info);

           LSLCacc_Disable_Receiver();
#else
           LSLCacc_Disable_Receiver();
           DL_Set_Local_Slot_Offset(DL_Get_Local_Device_Ref(), 0);
#endif
#if (PRH_BS_CFG_SYS_PERSISTENT_SCO_IN_SCANS_SUPPORTED==1)
               /*
                * If scanning with persistent SCO connections, force an immediate
                * reallignment to native timing for next potential SCO_MASTER activity.
                */
               if (LSLChop_Get_HSE_Engine()!=LSLChop_HSE_DEFAULT)
               {
                   LSLChop_Set_HSE_Engine(LSLChop_HSE_DEFAULT);
               }
               if ((DL_Get_Ctrl_State(DL_Get_Local_Device_Ref()) == CONNECTED_TO_SLAVE)
                  && LMscoctr_Get_Number_SYN_Connections())
               {
                   DL_Set_Local_Slot_Offset(DL_Get_Local_Device_Ref(),0);
                   HW_set_intraslot_offset(0);
                   HW_set_slave(0);
               }
#endif

		   /*
		    * Note - LMch_Change_Piconet(NULL) invoked on LC_Free_Link (via LC_EVENT_PAGE_SCAN_TIMEOUT).
			* Therefore must not enforce LC_Restore_Piconet/LC_Set_Active_Piconet here.
		    */

#if 1 // GF Returned 10 Feb 2011 - for Scatternet  // GF BQB - Not in Shoga build
               USLCchac_Procedure_Request(PAGESCAN_PROCEDURE, FALSE);
               LC_Callback_Event_Handler(LC_EVENT_PAGE_SCAN_TIMEOUT, &event_info);
#endif
               pageScan->ctrl_state=OS_NOT_ACTIVE;
       }
       else if (OS_CONNECTED_TO_MASTER == pageScan->ctrl_state)
       {
           /*
            * Indicate to the USLCchac that the PageScan procedure is finished,
            * curent super_state not to be modified.
            */
           USLCchac_Procedure_Finish(FALSE/*Dont return to prev super_state*/);
           DL_Set_Ctrl_State(pageScan->p_active_device_link, CONNECTED_TO_MASTER);

           /*
            * Setting the ctrl_state of the local Device Link will allow the 
            * USLCscheduler object to determine whether the device is a 
            * Master or a Slave within the connection.
            */
            DL_Set_Ctrl_State(DL_Get_Local_Device_Ref(), CONNECTED_TO_MASTER);


            /*
             * Notify to the high layers that the Page Scan
             * has been successful
             */
           event_info.deviceIndex = DL_Get_Device_Index(pageScan->p_active_device_link);
           event_info.fhs_packet  = (t_FHSpacket*)&pageScan->fhs_packet;
           LC_Callback_Event_Handler(LC_EVENT_PAGE_SCAN_INCOMING,&event_info);

           pageScan->ctrl_state=OS_NOT_ACTIVE;
       }
}

void USLCpageScan_Resume(void)
{
    t_pageScan *pageScan = &_pageScan_info;

    if (CONTINUOUS_SCAN == pageScan->scan_type)
    {
        /*
         * Set the page_scan_window_timer to expire 1 tick
         * after CLKN bit 12 changes, ie next 1.28 sec boundary
         */
        pageScan->page_scan_window_timer=
            BTtimer_Set_Ticks( 0x1000 - (HW_get_native_clk() & 0x0fff) );

    }
    else
    {
        pageScan->page_scan_window_timer =
                BTtimer_Set_Slots(pageScan->page_scan_window_timeout);
    }

    /*
     * If the slave does not receive the FHSpkt within
     * pageScan->page_resp_timeout then the slave returns to the page
     * scan state for one scan period. 
     */

    USLCchac_Set_Device_State(Page_Scan);
    pageScan->ctrl_state = OS_PAGE_SCAN;
    HWradio_SetRadioMode(RADIO_MODE_FULL_RX);
    LSLCpkt_Prepare_Receiver_ID(pageScan->p_active_device_link, Page_Scan);
}


/******************************************************************
 *
 * void USLCpageScan_Initialise(void)
 *
 ******************************************************************/

void USLCpageScan_Initialise(void)
{
    t_pageScan volatile *pageScan = &_pageScan_info;

    /*
     * Initialise defaults for Page Scan
     */
    pageScan->page_resp_timeout = PAGE_RESP_TIMEOUT_DEFAULT;
    pageScan->new_connection_timeout = NEW_CONNECTION_TIMEOUT_DEFAULT;
    pageScan->scan_for_one_period_only = TRUE;
    pageScan->scan_type = STANDARD_SCAN;
    pageScan->ctrl_state = OS_NOT_ACTIVE;     
}


/******************************************************************
 *
 * void _USLCpageScan_OS_START(void)
 *
 ******************************************************************/
void _USLCpageScan_OS_START(void)
{
    t_pageScan *pageScan = &_pageScan_info;

    if (pageScan->notify_procedure_start)
    {
        LC_Log_Procedure_Start(PAGESCAN_PROCEDURE);
        USLCchac_Procedure_Start(PAGESCAN_PROCEDURE, 
                               pageScan->p_active_device_link);
        pageScan->notify_procedure_start=FALSE;
    }

    if (CONTINUOUS_SCAN == pageScan->scan_type)
    {
        /*
         * Set the page_scan_window_timer to expire 1 tick
         * after CLKN bit 12 changes, ie next 1.28 sec boundary
         */
        pageScan->page_scan_window_timer=
           BTtimer_Set_Ticks( 0x1000 - (HW_get_native_clk() & 0x0fff) );

    }
    else
    {
        pageScan->page_scan_window_timer =
                    BTtimer_Set_Slots(pageScan->page_scan_window_timeout);
    }

    USLCchac_Set_Device_State(Page_Scan);
    pageScan->ctrl_state = OS_PAGE_SCAN;

#if (PRH_BS_CFG_SYS_PERSISTENT_SCO_IN_SCANS_SUPPORTED==1)
    /*
     * If scanning with persistent SCO connections, force an immediate
     * reallignment to native timing for next potential SCO_MASTER activity.
     */
    if ((DL_Get_Ctrl_State(DL_Get_Local_Device_Ref()) == CONNECTED_TO_SLAVE)
       && LMscoctr_Get_Number_SYN_Connections())
    {
        DL_Set_Local_Slot_Offset(DL_Get_Local_Device_Ref(),0);
        HW_set_intraslot_offset(0);
        LSLChop_Set_HSE_Engine(LSLChop_HSE_SOFTWARE);
    }
#endif


    /*
     * When a device is Page Scanning the receive window is permanently
     * enabled (i.e. frame-based (RxTx) receiver activation is not used). 
     * Thus, the receiver should be enabled once (ie here) and not 
     * re-enabled for the remainder of the page scan. Thus, the receiver
     * is  prepared here and not in the USLCsched, as is the case in 
     * frame-based (e.g. during a connection) receiver activation.
     */
    LSLChop_Set_Train(Standard_Scan);
    HWradio_SetRadioMode(RADIO_MODE_FULL_RX);
    LSLCpkt_Prepare_Receiver_ID(pageScan->p_active_device_link, Page_Scan);

}




/******************************************************************
 *
 * void _USLCpageScan_Process_ID_Pkt_During_Scanning
 *
 ******************************************************************/
void _USLCpageScan_Process_ID_Pkt_During_Scanning(void)
{
    t_pageScan *pageScan = &_pageScan_info;

            /* 
             * The page_resp timer is increased by 1 in order to allow for the fact 
             * that the Native clocks in the Slave and Master are not alligned.
             * Since the Native clock is used to 'drive' the timers, the plus 1 is 
             * used to ensure that the Slave doesn't stop listening for the 
             * FHS packet 1 slot too early.
             */
            pageScan->page_resp_timer =
                           BTtimer_Set_Slots(pageScan->page_resp_timeout+1);

            USLCchac_Set_Device_State(SlavePageResponse);
            pageScan->ctrl_state = OS_SLAVE_PAGE_RESP;

            HWradio_SetRadioMode(RADIO_MODE_SLAVE_PAGE_RESP);

            LSLCpkt_Generate_ID(pageScan->p_active_device_link, SlavePageResponse);
}

#else /*(PRH_BS_CFG_SYS_PAGE_SCAN_SUPPORTED==1)*/
/*
 * Page scan is not supported, functions should not be invoked.
 */

t_error USLCpageScan_Request(t_deviceIndex device_index,t_slots scan_timeout)
{
    return UNSUPPORTED_FEATURE;
}

void USLCpageScan_Cancel(void)
{
}

void USLCpageScan_Initialise(void)
{
}

void USLCpageScan_Page_Scan(boolean dummy)
{
    t_LC_Event_Info event_info;

    event_info.status = UNSUPPORTED_FEATURE;
    LC_Callback_Event_Handler(LC_EVENT_PAGE_SCAN_TIMEOUT, &event_info);
}

#endif /*(PRH_BS_CFG_SYS_PAGE_SCAN_SUPPORTED==1)*/

