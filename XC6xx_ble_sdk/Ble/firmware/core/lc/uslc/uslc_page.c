/*********************************************************************
 * MODULE NAME:    uslc_page.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    Baseband Paging procedure
 * MAINTAINER:     John Nelson, Conor Morris
 *
 * SOURCE CONTROL: $Id: uslc_page.c,v 1.85 2013/09/24 15:23:46 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *      
 *********************************************************************/

#include "sys_config.h"
#include "lc_interface.h"

#if (PRH_BS_CFG_SYS_PAGE_SUPPORTED==1)

#include "lc_log.h"

#include "bt_timer.h"

#include "sys_mmi.h"

#include "dl_dev.h"

#include "lslc_access.h"
#include "lslc_pkt.h"
#include "lslc_freq.h"
#include "lslc_hop.h"

#include "uslc_page.h"
#include "uslc_chan_ctrl.h"

#include "hc_event_gen.h"
#include "lmp_config.h"
/*
 *  t_USLCpage_ctrl_state
 *  Indicates the state of the Page state-machine.
 *  states  can be split into 2 parts:
 *      ctrl_stateTx:
 *       Part of the state that involves the transmission of a packet.
 *       Active timers are checked in this part of the state.
 *       If a timer has not expired, a packet will be transmitted
 *       and a transition to the ctrl_stateRx will always happen.
 *
 *      ctrl_stateRx:
 *       Part of the state that involves the reception of a packet.
 *       If an incorrect packet has been received then the transition
 *       to the ctrl_stateTx will always happen.
 *       If a correct packet is received then the following transitions
 *       will take place:
 *         (i)  Transition to the next ctrl_stateTx, or
 *        (ii)  in the case where the procedure is finished,
 *              transition to the end ctrl_state. 
 */

typedef enum
{ 
    OS_START,               /* start control-state */
    OS_END,                 /* end control-state */
    OS_NOT_ACTIVE,          /* procedure currently not active*/
    OS_PAGING_TX,           /* paging transmission control-state */
    OS_PAGING_RX,           /* paging reception control-state */
    OS_MASTER_PAGE_RESP_TX, /* master page response transmit control-state */
    OS_MASTER_PAGE_RESP_RX, /* master page response reception control-state */
    OS_MASTER_PAGE_RESP_LOOK_AHEAD_RX,
    OS_POLL_RESP_TX,        /* poll response transmission control-state */
    OS_POLL_RESP_RX,        /* poll response reception control-state */
    OS_POLL_RESP_LOOK_AHEAD_RX,
    OS_CONNECTED_TO_SLAVE,     /* connected-to-slave control-state */
    OS_PAGING_LOOK_AHEAD_RX,
	OS_SUSPEND,
	OS_RESUME
} t_USLCpage_ctrl_state;        /* bluetooth Page control states */

typedef struct t_page
{
    t_slots page_timeout;
    t_slots page_resp_timeout;
    t_slots new_connection_timeout;

    t_timer page_timer;
    t_timer page_resp_timer;
    t_timer Npage_timer;
    t_timer new_connection_timer;
    
    t_devicelink *p_active_device_link;
    t_syncword   slave_syncword;
    t_USLCpage_ctrl_state ctrl_state;
    u_int16 n_page;
	t_timer page_suspend_timer;
}  t_page;

/*
 * Local support functions.
 */
void _USLCpage_Generate_ID(void);
void _USLCpage_Resume_Paging(void);
void _USLCpage_Setup_DL_to_CAC(void);


/*
 *  _page_info: Stores control information for the Page procedure
 */
t_page _page_info;


/******************************************************************
 *
 * USLCpage_Initialise
 *
 * Initialises the defaults for Baseband Inquiry Page procedure.
 *
 *****************************************************************/
void USLCpage_Initialise(void)
{
    t_page volatile *page = &_page_info;

    /*
     * Initialise defaults for Page
     */
    page->page_resp_timeout = PAGE_RESP_TIMEOUT_DEFAULT;
    page->new_connection_timeout = NEW_CONNECTION_TIMEOUT_DEFAULT;
    page->ctrl_state = OS_NOT_ACTIVE;
}

/******************************************************************
 *
 * USLCpage_Request
 *
 * Request from a higher layer to start the Page procedure
 *
 * Pre-Conditions:
 * 1. The BD_ADDR of the device to be paged is placed in the 'device index'
 *
 * Notes:
 * 1. The minimum page_timeout for the LC is 2 slots.  The HCI
 *    insensibly allows 1 slot so hence ensure minimum 2.  The
 *    maximum is 0xFFFF, although for test purposes this is not
 *    enforced here.
 * 2. The trainRepetitions must be at least 1.
 ******************************************************************/
t_error USLCpage_Request(
    t_deviceIndex deviceIndex, /* Indicates the deviceLink to be assigned */
                               /* to the Paged Device. The deviceLink is  */
                               /* assigned by the Link Manager. It is     */
                               /* assumed that the Link Manager has placed*/
                               /* the correct FHS in the deviceLink       */

    u_int16 trainRepetitions,  /* A single train must be repeated at least*/
                               /* trainRepetitions times before a new     */ 
                               /* train is used.                          */

    t_slots page_timeout       /* Maximum ammount of time before the Page */ 
                               /* is halted.                              */
)
{
    t_error status = NO_ERROR;
    t_page volatile *page = &_page_info;

    LC_Log_Procedure_Request(PAGE_PROCEDURE);

    /*
     * Ensure parameters are sensible for the LC
     */
    if (page_timeout < 2)
    {
        page_timeout = 2;
    }
    if (trainRepetitions < 1)
    {
        trainRepetitions = 1;
    }

    /*
     * Only activate procedure if currently not active and the procedure
     * params are within the indicated bounds.
     */
    if ( (page->ctrl_state == OS_NOT_ACTIVE) &&
         ( (deviceIndex!=0) && (deviceIndex<=SYSconfig_Get_Max_Active_Devices())))
    {

        LC_Log_Procedure_Accept(PAGE_PROCEDURE);

        page->p_active_device_link = DL_Get_Device_Ref(deviceIndex);
        page->n_page = trainRepetitions;
        page->page_timeout = page_timeout;
        page->ctrl_state = OS_START;
		page->page_suspend_timer = BTtimer_Set_Slots(64);
        /*
         * Device Link initialisation has taken place in the
         * LC_Alloc_Link(). Just need to ensure that the
         * role_peer is set to SLAVE.
         * The Piconet Index is by default 0 for Master's piconet
         */
        DL_Set_Role_Peer(page->p_active_device_link,SLAVE);

        /*
         * Use the Paged device's syncword in the initial part of the procedure
         * Currently Access=Slave's syncword is initialised before call.
         */
        page->slave_syncword = DL_Get_Access_Syncword(page->p_active_device_link);
        DL_Set_Access_Syncword(page->p_active_device_link, page->slave_syncword);

        /*
         * Build the page hop table for the page device
         */
        LSLChop_Build_Frequency_Table(Page, DL_Get_UAP_LAP(page->p_active_device_link));

        /*
         * Ensure that Hop Selection Kernel is setup
         */
        LSLCfreq_Assign_Frequency_Kernel(page->p_active_device_link);

        USLCchac_Procedure_Request(PAGE_PROCEDURE, TRUE);
    }
    else
    {
        /*
         * Page procedure has already been requested.
         * or page procedure of 1 slot was allocated, so set up
         * some fields in the device link so it can be returned
         * to the pool correctly.
         */
        DL_Set_Role_Peer(DL_Get_Device_Ref(deviceIndex), SLAVE);
        DL_Set_Ctrl_State(DL_Get_Device_Ref(deviceIndex), NOT_CONNECTED);
        status = UNSPECIFIED_ERROR;
    }

    return status;
}

/******************************************************************
 *
 * USLCpage_Page_Resume
 *
 * Request from a higher layer to start the Inquiry procedure
 *
 * Note: The interval between 2 Inquiry requests must be determined
 *       randomly.
 *
 *****************************************************************/
t_error USLCpage_Page_Resume(void)
{
    t_error status = NO_ERROR;
    t_page volatile *page = &_page_info;

	/*
	 * Only activate procedure if currently not active and the procedure
	 * params are within the indicated bounds.
	 */

    page->page_suspend_timer = BTtimer_Set_Slots(64);
    page->ctrl_state = OS_RESUME;

    /*
     * Device Link initialisation has taken place in the
     * LC_Alloc_Link(). Just need to ensure that the
     * role_peer is set to SLAVE.
     * The Piconet Index is by default 0 for Master's piconet
     */
    DL_Set_Role_Peer(page->p_active_device_link,SLAVE);

    /*
     * Ensure that Hop Selection Kernel is setup
     */
    LSLCfreq_Assign_Frequency_Kernel(page->p_active_device_link);

    USLCchac_Procedure_Request(PAGE_PROCEDURE, TRUE);

    return status;
}

/******************************************************************
 *
 * USLCpage_Cancel
 *
 * Request from a higher layer to cancel the Page procedure 
 *
 * Note:
 * Invoking the Page_Cancel() procedure after the Master has received
 * the 1st valid packet on the baseband link, will not result in the 
 * removal of the baseband connection to the Slave. The error
 * 'ILLEGAL_COMMAND' will be returned to the calling object.
 * 
 *****************************************************************/
void USLCpage_Cancel(void)
{
    t_error status;
    t_page volatile *page = &_page_info;
    status = USLCchac_Procedure_Request(PAGE_PROCEDURE, FALSE);
    
    if (page->ctrl_state != OS_NOT_ACTIVE)
    {
        page->ctrl_state = OS_NOT_ACTIVE;
        /*
         * For correct release of AM_ADDR it must be defined in device link
         */
        DL_Set_AM_Addr( page->p_active_device_link,
                     FHS_Get_AM_Addr( LC_Get_Device_FHS_Ref() ));
    }

    LC_Log_Procedure_Cancel(PAGE_PROCEDURE);

}

/******************************************************************
 * 
 * 'One-shot' statemachine for the Page procedure
 *
 * Once the Page procedure has started (super state=Page)
 * this function will be invoked during each Tx slot allocated to the 
 * USLC channel controller.
 *
 * Assumption: 
 *  1- The page_resp_timer is set upon reception of 
 *     the 1st ID pkt from the slave.
 *
 *  2- When the Paging unit enters the MasterPageResp state
 *     (i.e. receives 1st ID pkt from the slave), the USLCpage
 *     freezes the estimate of the slave's clock. The USLCpage
 *     adjusts the X-input value to the frequency calculation
 *     (i.e. incr by 1). The 1st increment is done before
 *      sending the FHS packet to the paged unit.
 *
 *  3- After the master has sent its FHS packet 
 *     it waits for a second response from the slave which acks the 
 *     reception of the FHS packet.  If no response is received, the master
 *     retransmits the FHS packet. It is assumed that the FHS is 
 *     retransmitted every Tx slot until a page_resp_timer timeout.
 *
 *****************************************************************/
void USLCpage_Page(boolean look_ahead_to_next_frame)
{
    t_LC_Event_Info event_info;
    t_packet event;
    t_page *page = &_page_info;


    if (0 == look_ahead_to_next_frame)
    {
        event = DL_Get_Rx_Packet_Type(page->p_active_device_link);
    }
    else
    {
        event = INVALIDpkt;
    }

    if (OS_START == page->ctrl_state) /* super_state=Page */
    {

        LC_Log_Procedure_Start(PAGE_PROCEDURE);

        LSLChop_Set_Train(Atrain);
        /* 
         * Each train is 10ms (i.e. 16 slots long)
         * Thus, train switch must taken place after every : 
         *           (Npage*16) slots
         * Note: It is assumed that all ACL connections have been put on 
         *       hold or parked.
         */
        page->Npage_timer = BTtimer_Set_Slots(page->n_page*16);

        page->page_timer = BTtimer_Set_Slots(page->page_timeout);
	}

	if ((OS_START == page->ctrl_state) || (OS_RESUME == page->ctrl_state))
	{
        /*
         * super_state is implicitly set to 'Page' in function:
         * USLCchac_Procedure_Start(..)
         */ 
        page->page_suspend_timer = BTtimer_Set_Slots(64);
		USLCchac_Procedure_Start(PAGE_PROCEDURE, page->p_active_device_link);
        HWradio_SetRadioMode(RADIO_MODE_MASTER_DOUBLE_WIN);
        page->ctrl_state =  OS_PAGING_TX;
    }


	switch(page->ctrl_state)
    {
    case OS_PAGING_TX: /* super_state=Page */
        if (BTtimer_Is_Expired(page->page_timer))
        {
            HWradio_SetRadioMode(RADIO_MODE_STANDBY);
            page->ctrl_state = OS_END;
        }
        else if (BTtimer_Is_Expired(page->page_suspend_timer))
		{
			if (LMconfig_LM_Connected_As_Slave())
				page->ctrl_state = OS_SUSPEND;
			else
				page->page_suspend_timer = BTtimer_Set_Slots(64);
		}
        else 
        {
            _USLCpage_Generate_ID();
            page->ctrl_state = OS_PAGING_RX;
        }
        break; 

    case OS_PAGING_RX:
#if (LC_LOOK_AHEAD==1)
        if (look_ahead_to_next_frame)
        {
            /*
             * Prepare an ID pkt for Tx in the next slot.
             */
            _USLCpage_Generate_ID();
            page->ctrl_state = OS_PAGING_LOOK_AHEAD_RX;
        }
        else
#endif
        if (IDpkt == event)     /* Received a resp from the slave */
        {
            LSLChop_Freeze_X_for_Page(HW_get_native_clk()+DL_Get_Clock_Offset(page->p_active_device_link));
            USLCchac_Set_Device_State(MasterPageResponse);
            page->page_resp_timer = BTtimer_Set_Slots(page->page_resp_timeout);
            page->ctrl_state = OS_MASTER_PAGE_RESP_TX;
        }
        else
        {
            page->ctrl_state=OS_PAGING_TX;
        }
        break;

#if (LC_LOOK_AHEAD==1)
    case OS_PAGING_LOOK_AHEAD_RX:
        if (IDpkt == event)     /* Received a resp from the slave */
        {
            USLCchac_Set_Device_State(MasterPageResponse);
            page->page_resp_timer = BTtimer_Set_Slots(page->page_resp_timeout);
#if (PRH_BS_DEV_EARLY_PREPARE_PAGE_ID_FHS_TURNAROUND==0)
            LSLChop_Freeze_X_for_Page(HW_get_native_clk()+DL_Get_Clock_Offset(page->p_active_device_link));
            HWradio_SetRadioMode(RADIO_MODE_TX_RX);
            /*
             * Note: X implicitly incremented using clk values.
             */
            LSLCpkt_Generate_FHS(page->p_active_device_link,MasterPageResponse);
#endif

            page->ctrl_state=OS_MASTER_PAGE_RESP_RX;
        }
        else if (BTtimer_Is_Expired(page->page_timer))
        {
            /*
             * CANCEL CURRENT TRANSMISSION OF ID PKTS THAT WAS PREPARED DURING 
             * THE LOOK-AHEAD PROCEDURE.
             */
            LSLCacc_Disable_Transmitter();
            
            page->ctrl_state = OS_END;
        }
        else if (BTtimer_Is_Expired(page->page_suspend_timer))
        {
			if	(LMconfig_LM_Connected_As_Slave())
				page->ctrl_state = OS_SUSPEND;
			else
				page->page_suspend_timer = BTtimer_Set_Slots(64);
        }
        else
        {
            page->ctrl_state=OS_PAGING_RX;
        }
        break;
#endif  /* LC_LOOK_AHEAD */

    case OS_MASTER_PAGE_RESP_TX:  /* super_state=MasterPageResp */
        if (BTtimer_Is_Expired(page->page_resp_timer))
        {
            /* 
             * If the FHS pkt is not acknowledged within 'pagerespTO' 
             * then the device will to state :
             *    OS_END - if both page_resp_timer & page_timer have expired.
             *    OS_PAGING_RX - if page_resp_timer only has expired.
             *    OS_MASTER_PAGE_RESP_RX - none of the timers have expired.             
             */
            if (BTtimer_Is_Expired(page->page_timer))
            {
                /* 
                 * Indicate to the higher layers that the Page has completed, 
                 * without a connection setup
                 */
                page->ctrl_state = OS_END;
            }
            else
            {
                /*
                 * Resume Paging:
                 * Assumption: Resume Paging using the same train.
                 *             Reset Npage timer (i.e. do not toggle train
                 *             until timer expires).
                 */
                _USLCpage_Resume_Paging();
                 page->ctrl_state = OS_PAGING_RX;
            }
        }
        else 
        {
            HWradio_SetRadioMode(RADIO_MODE_TX_RX);
            /*
             * X implicitly incremented using clk values.
             */
            LSLCpkt_Generate_FHS(page->p_active_device_link,MasterPageResponse);
            page->ctrl_state=OS_MASTER_PAGE_RESP_RX;
        }
        break;

    case OS_MASTER_PAGE_RESP_RX:  /* super_state = MasterPageResp */
#if (LC_LOOK_AHEAD==1)
        if (look_ahead_to_next_frame)
        {
            /*
             * Note: X implicitly incremented.
             */
            LSLCpkt_Generate_FHS(page->p_active_device_link,
                                                        MasterPageResponse);
            page->ctrl_state = OS_MASTER_PAGE_RESP_LOOK_AHEAD_RX;
        }
        else 
#endif /* LOOK_AHEAD */
        if (IDpkt == event)
        {
            page->new_connection_timer =
                    BTtimer_Set_Slots(page->new_connection_timeout);
            USLCchac_Set_Device_State(Connection);
            _USLCpage_Setup_DL_to_CAC();

            /*
             * bug 398: 
             * Must ensure that once the connection is established, the
             * local BD_ADDR (i.e. Master) is written to the registers.
             */
            HW_set_bd_addr(SYSconfig_Get_Local_BD_Addr_Ref());

            page->ctrl_state = OS_POLL_RESP_TX;
        }       
        else 
        {
            page->ctrl_state=OS_MASTER_PAGE_RESP_TX;
        }
        break;

#if (LC_LOOK_AHEAD==1)
       case OS_MASTER_PAGE_RESP_LOOK_AHEAD_RX:
           if (IDpkt == event)
           {
                page->new_connection_timer =
                    BTtimer_Set_Slots(page->new_connection_timeout);
                USLCchac_Set_Device_State(Connection);
                _USLCpage_Setup_DL_to_CAC();

                /*
                 * bug 398: 
                 * Must ensure that once the connection is established, the
                 * local BD_ADDR (i.e. Master) is written to the registers.
                 */
                HW_set_bd_addr(SYSconfig_Get_Local_BD_Addr_Ref());

                LSLCpkt_Generate_POLL(page->p_active_device_link);
                page->ctrl_state = OS_POLL_RESP_RX;
           }       
           else if (BTtimer_Is_Expired(page->page_resp_timer))

           {
                /* 
                 * If the FHS pkt is not acknowledged within 'pagerespTO' 
                 * then the device will to state :
                 *    OS_END - if both page_resp_timer & page_timer have expired.
                 *    OS_PAGING_RX - if page_resp_timer only has expired.
                 *    OS_MASTER_PAGE_RESP_RX - none of the timers have expired.             
                 */
               if (BTtimer_Is_Expired(page->page_timer))
               {
                   /*
                    * CANCEL CURRENT TRANSMISSION OF FHS PKT THAT WAS PREPARED
                    * DURING THE LOOK-AHEAD PROCEDURE.
                    */
                  LSLCacc_Disable_Transmitter();
                  page->ctrl_state = OS_END;
               }
               else
               {
                   /* 
                    * Resume Paging: 
                    * Assumption: Resume Paging using the same train.
                    *             Reset Npage timer (i.e. do not toggle train
                    *             until timer expires).
                    */
                   _USLCpage_Resume_Paging();
                   page->ctrl_state = OS_PAGING_RX;
               }
           }
           else 
           {
                /*
                 * Transmit FHS packet.
                 * FHS packet has already been setup for transmission in the
                 * previous LOOK_AHEAD.
                 */
                page->ctrl_state=OS_MASTER_PAGE_RESP_RX;
            }
            break;
#endif /* LC_LOOK_AHEAD */

        case OS_POLL_RESP_TX:  /*super_state = Connection */
            if (BTtimer_Is_Expired(page->new_connection_timer))
            {
                if (BTtimer_Is_Expired(page->page_timer))
                {
                    page->ctrl_state = OS_END;
                }
                else
                {
                    /*
                     * Resume Paging:
                     * Assumption: Resume Paging using the same train.
                     *             Reset Npage timer (i.e. do not toggle train
                     *             until timer expires).
                     * Reset the syncword to that of the Paged device
                     * Reset the AM_ADDR back to 0
                     */
                    DL_Set_Access_Syncword(page->p_active_device_link,
                          page->slave_syncword);
                    DL_Set_AM_Addr(page->p_active_device_link, 0);

                   _USLCpage_Resume_Paging();
                   page->ctrl_state = OS_PAGING_RX;
                }
            }
            else 
            {
                /* 
                 * Using the Master's CAC, generate a POLL packet.
                 */
                LSLCpkt_Generate_POLL(page->p_active_device_link);   
                page->ctrl_state = OS_POLL_RESP_RX;
        }
        break;

    case OS_POLL_RESP_RX:  /* super_state = Connection */
#if (LC_LOOK_AHEAD==1)
        if (look_ahead_to_next_frame)
        {
            LSLCpkt_Generate_POLL(page->p_active_device_link);
            page->ctrl_state = OS_POLL_RESP_LOOK_AHEAD_RX;
        }
        else 
#endif
        if ( (INVALIDpkt!=event) && (IDpkt != event) &&
                (NAK == HW_get_rx_arqn()))
        {
            /*
             * Any pkt received is OK in this state &&
             * last pkt received ARQN == NAK 
             */

            page->ctrl_state = OS_CONNECTED_TO_SLAVE;
            DL_Set_Ctrl_State(page->p_active_device_link, CONNECTED_TO_SLAVE);

            /* Indicate that a Baseband connection has been established */
            /* via SYSmmi */
//            SYSmmi_Display_Event(eSYSmmi_LC_Connection_Event);
        }
        else 
        {
            page->ctrl_state= OS_POLL_RESP_TX;
        }
        break;

#if (LC_LOOK_AHEAD==1)
        case OS_POLL_RESP_LOOK_AHEAD_RX:
        if ( (INVALIDpkt!=event) && (IDpkt != event) && (NAK == HW_get_rx_arqn()))
        {
            /*
             * Any pkt received is OK in this state &&
             * last pkt received ARQN == NAK 
             */                                  

            page->ctrl_state = OS_CONNECTED_TO_SLAVE;
            DL_Set_Ctrl_State(page->p_active_device_link, CONNECTED_TO_SLAVE);

            /*
             *
             * CANCEL CURRENT TRANSMISSION OF PACKET THAT WAS PREPARED DURING 
             * THE LOOK-AHEAD PROCEDURE.
             */
            LSLCacc_Disable_Transmitter();

            /* Indicate that a Baseband connection has been established */
            /* via SYSmmi */
//            SYSmmi_Display_Event(eSYSmmi_LC_Connection_Event);
        }
        else if (BTtimer_Is_Expired(page->new_connection_timer))
        {
               if (BTtimer_Is_Expired(page->page_timer))
               {
                   /*
                    * CANCEL CURRENT TRANSMISSION OF FHS PKT THAT WAS PREPARED
                    * DURING THE LOOK-AHEAD PROCEDURE.
                    */
                  LSLCacc_Disable_Transmitter();
                  page->ctrl_state = OS_END;
               }
               else
               {
                    /*
                     * Resume Paging:
                     * Assumption: Resume Paging using the same train.
                     *             Reset Npage timer (i.e. do not toggle train
                     *             until timer expires).
                     * Reset the syncword to that of the Paged device
                     * Reset the AM_ADDR back to 0
                     */
#if 0
Already in Resume_Paging
                    USLCchac_Set_Device_State(Page);
#endif
                    DL_Set_Access_Syncword(page->p_active_device_link,
                          page->slave_syncword);
                    DL_Set_AM_Addr(page->p_active_device_link, 0);

                   _USLCpage_Resume_Paging();
                   page->ctrl_state = OS_PAGING_RX;
               }
        }
        else 
        {
            /* 
             * Generate POLL packet.
             * Packet already prepared in the previous LOOK_AHEAD
             */
            page->ctrl_state = OS_POLL_RESP_RX;
        }
        break;
#endif /* LOOK_AHEAD */
        
    case OS_END:
        break;
        
    default:
        HCeg_Hardware_Error_Event(PRH_BS_HW_ERROR_INCORRECT_CTRLSTATE);
    }

    if ((OS_END == page->ctrl_state) || (OS_SUSPEND == page->ctrl_state))
    {
        USLCchac_Procedure_Finish(TRUE /*Resume previous super_state*/);

        LSLCacc_Disable_Transmitter(); /* ensure RF output is turned off */

		if (OS_SUSPEND == page->ctrl_state)
		{
	    	event_info.status = NO_ERROR;
        	LC_Callback_Event_Handler(LC_EVENT_PAGE_SUSPENDED,&event_info);
		}
		else // OS_END
		{
			/*
			* For correct release of AM_ADDR it must be defined in device link
			*/
			DL_Set_AM_Addr( page->p_active_device_link,
			         FHS_Get_AM_Addr( LC_Get_Device_FHS_Ref() ));
			event_info.status = PAGE_TIMEOUT;
			LC_Callback_Event_Handler(LC_EVENT_PAGE_COMPLETE,&event_info);
		}

        page->ctrl_state = OS_NOT_ACTIVE;
    }
    else if (OS_CONNECTED_TO_SLAVE == page->ctrl_state)
    {
        USLCchac_Procedure_Finish(FALSE/*Do not resume previous super state*/);

        /*
         * Setting the ctrl_state of the local Device Link will allow the 
         * USLCscheduler object to determine whether the device is a 
         * Master or a Slave within the connection.
         */
        DL_Set_Ctrl_State(DL_Get_Local_Device_Ref(), CONNECTED_TO_SLAVE);
        
        event_info.status = NO_ERROR;
        LC_Callback_Event_Handler(LC_EVENT_PAGE_RESULT, &event_info);

        page->ctrl_state = OS_NOT_ACTIVE;
    }
}

 /******************************************************************
 *
 * _USLCpage_Generate_ID(void)
 *
 *****************************************************************/
void _USLCpage_Generate_ID(void)
{
    t_page *page = &_page_info;

    if ( BTtimer_Is_Expired(page->Npage_timer))
    {
       LSLChop_Toggle_Train();
       page->Npage_timer = BTtimer_Set_Slots(page->n_page*16);
    }

    /*
     * Send the page message using slaves DAC
     */
    HWradio_SetRadioMode(RADIO_MODE_MASTER_DOUBLE_WIN);
    LSLCpkt_Generate_ID(page->p_active_device_link,Page);
}

/******************************************************************
 *
 * _Resume_Paging(void)
 *
 *****************************************************************/
void _USLCpage_Resume_Paging(void)
{
    t_page *page = &_page_info;

    USLCchac_Set_Device_State(Page);
    page->Npage_timer = BTtimer_Set_Slots(page->n_page*16);
    HWradio_SetRadioMode(RADIO_MODE_MASTER_DOUBLE_WIN);
    LSLCpkt_Generate_ID(page->p_active_device_link, Page);
}

/******************************************************************
 *
 * _Setup_DL_to_CAC(void)
 *
 *****************************************************************/
void _USLCpage_Setup_DL_to_CAC(void)
{
    t_page *page = &_page_info;

    /*
     * Set AccessCode of deviceLink = Master CAC
     */
    DL_Set_Access_Syncword(page->p_active_device_link,
                    SYSconfig_Get_Local_Syncword() );
    DL_Set_AM_Addr( page->p_active_device_link,
                     FHS_Get_AM_Addr( LC_Get_Device_FHS_Ref() ));
}

/******************************************************************
 *
 * USLCpage_Potential_Early_Prepare_MasterPageResponse(void)
 *
 *****************************************************************/
#if (PRH_BS_DEV_EARLY_PREPARE_PAGE_ID_FHS_TURNAROUND==1)
void USLCpage_Potential_Early_Prepare_MasterPageResponse(void)
{
    t_page volatile *page = &_page_info;   
    if(OS_PAGING_LOOK_AHEAD_RX==page->ctrl_state)
    {
        LSLCpkt_Generate_FirstFHS_MasterPageResponse(page->p_active_device_link);
    }
}
#endif

void USLCpage_Page_Force_To_Tx_State_If_Req(void)
{
	t_page volatile *page = &_page_info;  
	if (OS_PAGING_RX == page->ctrl_state)
		page->ctrl_state = OS_PAGING_TX;
	else if (OS_MASTER_PAGE_RESP_RX == page->ctrl_state)
		page->ctrl_state = OS_MASTER_PAGE_RESP_TX;
	else if (OS_POLL_RESP_RX == page->ctrl_state)
		page->ctrl_state = OS_POLL_RESP_TX;
}

#else /*(PRH_BS_CFG_SYS_PAGE_SUPPORTED==1)*/
/*
 * Page is not supported, functions should not be invoked.
 */

t_error USLCpage_Request(t_deviceIndex device_index, 
                          u_int16 train_repetitions, t_slots page_timeout)
{
    return UNSUPPORTED_FEATURE;
}

void USLCpage_Cancel(void)
{
}

void USLCpage_Initialise(void)
{
}

void USLCpage_Page(boolean look_ahead_to_next_frame)
{
    t_LC_Event_Info event_info;

    event_info.status = UNSUPPORTED_FEATURE;
    LC_Callback_Event_Handler(LC_EVENT_PAGE_COMPLETE, &event_info);
}

void USLCpage_Page_Force_To_Tx_State_If_Req(void)
{
}

#endif /*(PRH_BS_CFG_SYS_PAGE_SUPPORTED==1)*/
