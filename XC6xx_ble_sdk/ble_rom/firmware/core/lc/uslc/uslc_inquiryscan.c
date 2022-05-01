/*********************************************************************
 * MODULE NAME:    uslc_inquiry_scan.c
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    
 * MAINTAINER:     John Nelson, Conor Morris
 *
 * SOURCE CONTROL: $Id: uslc_inquiryscan.c,v 1.83 2013/10/07 19:30:55 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 *********************************************************************/

#include "sys_config.h"
#include "lc_types.h"
#include "lc_interface.h"

#if (PRH_BS_CFG_SYS_INQUIRY_SCAN_SUPPORTED==1)

#include "lc_log.h"

#include "bt_timer.h"
#include "bt_addr_sync.h"

#include "dl_dev.h"

#include "lslc_pkt.h"
#include "lslc_hop.h"
#include "lslc_access.h" 
#include "uslc_inquiryscan.h"
#include "uslc_chan_ctrl.h"

#include "lmp_acl_container.h"
#include "lmp_sco_container.h"
#include "lmp_config.h"
#include "hw_radio.h"

/*
 * Private constants, types and variables
 */
typedef enum
{ 
    OS_START,               /* start control-state */
    OS_END,                 /* end control-state */
    OS_NOT_ACTIVE,          /* procedure currently not active*/
    OS_INQUIRY_SCAN,        /* inquiry-scan control-state */
    OS_INQUIRY_RAND_WAIT,   /* inquiry random wait control-state */
    OS_INQUIRY_RESPONSE,    /* inquiry scan continue control-state */
    OS_INQUIRY_SCAN_RESUME,  /* Resume Inquiry Scanning after transmitting */
                            /* FHS packet   
                            */
    OS_INQUIRY_W4_RESPONSE_COMPLETE, /* Need to burn a frame to ensure the 
                                       * FHS/EIR packets are sent */
    OS_INQUIRY_SCAN_2ND_TRAIN,

    OS_INQUIRY_GENERATE_EIR_PACKET

} t_USLCinqScan_ctrl_state;


typedef struct t_inquiryScan
{
    t_timer inquiry_scan_window_timer;
    t_timer inquiry_resp_timer;
    t_slots inquiry_scan_window_timeout;
    t_slots inquiry_resp_timeout;

    t_devicelink *p_active_device_link; 
    t_syncword inquiry_scan_syncword;
    t_USLCinqScan_ctrl_state ctrl_state;

    boolean notify_procedure_start;
    t_scan_type scan_type;

    boolean return_to_piconet_DL;
#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
    t_role peer_role;
#endif
} t_inquiryScan;

/*
 * _inquiryScan_info: Stores control information for the Inquiry Scan procedure
 */
t_inquiryScan _inquiryScan_info;

/*
 * Private helper functions.
 */
void _USLCinqScan_OS_START(void);
void _USLCinqScan_Generate_FHS(void);

/******************************************************************
 *
 * USLCchac_Inquiry_Scan_Request
 *
 * Request from a higher layer to start the Inquiry Scan procedure
 *
 * inquiry_lap  The device will scan using Inquiry Access Code 
 *              (IAC) generated from the given lap.
 * scanTO       Timeout (length) of the scan period
 * scan_type    STANDARD_SCAN or CONTINUOUS_SCAN or INTERLACED_SCAN
 *****************************************************************/
t_error USLCinqScan_Request(
    t_lap inquiry_lap, t_slots scanTO, t_scan_type scan_type)
{
    t_error status;
    t_syncword *p_syncword;

    t_inquiryScan volatile *inquiryScan = &_inquiryScan_info;

    LC_Log_Procedure_Request(INQUIRYSCAN_PROCEDURE);

    /*
     * Only activate procedure if currently not active
     */    
    if (inquiryScan->ctrl_state == OS_NOT_ACTIVE)
    {
        LC_Log_Procedure_Accept(INQUIRYSCAN_PROCEDURE);
        inquiryScan->notify_procedure_start=TRUE;

        p_syncword = (t_syncword *) &inquiryScan->inquiry_scan_syncword;
        BTaddr_Build_Sync_Word(inquiry_lap, &p_syncword->high, &p_syncword->low);
        inquiryScan->inquiry_scan_window_timeout=scanTO;
        inquiryScan->scan_type = scan_type;
      
        /*
         * Always InquiryScan using the local deviceLink.
         */
        inquiryScan->p_active_device_link = DL_Get_Local_Device_Ref();

#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
        /*
         * store the piconet link that we intend to return after scan
         */
	    inquiryScan->peer_role = DL_Get_Role_Peer(inquiryScan->p_active_device_link);
#endif

        inquiryScan->return_to_piconet_DL = TRUE;
    
        /*
         * Initialise the ctrl_state of the DeviceLink to osStart
         */
        inquiryScan->ctrl_state = OS_START; 
        USLCchac_Procedure_Request(INQUIRYSCAN_PROCEDURE, TRUE);
        status = NO_ERROR;
    }    
    else
    {
        /*
         * Inquiry Scan procedure has already been requested
         */
        status = UNSPECIFIED_ERROR;
    }

    return status;
}

/******************************************************************
 *
 * USLCinqScan_Cancel
 *
 * Request from a higher layer to cancel the current Inquiry_Scan procedure
 *
 *****************************************************************/
void USLCinqScan_Cancel(void)
{
    t_inquiryScan volatile *inquiryScan = &_inquiryScan_info;

    if ( ( OS_INQUIRY_SCAN == inquiryScan->ctrl_state) ||
         ( OS_INQUIRY_RESPONSE == inquiryScan->ctrl_state) ||
         ( OS_INQUIRY_SCAN_RESUME == inquiryScan->ctrl_state) ||
         ( OS_INQUIRY_W4_RESPONSE_COMPLETE == inquiryScan->ctrl_state) ||
         ( OS_END == inquiryScan->ctrl_state))
    {
        LSLCacc_Disable_Receiver();  

        /*
         * During the InquiryScan, device_link[0] is used for the 
         * scaning. If an ID packet is received during the scanning
         * then the intraslot_offset will be updated in the local
         * device_link.  Thus, if the device is returning to it's 
         * own piconet (ie no connection exists) then must ensure
         * that the intraslot_offset is set to 0.
         */
        DL_Set_Local_Slot_Offset(DL_Get_Local_Device_Ref(),0);
        if (inquiryScan->return_to_piconet_DL)
        {
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
                HW_set_intraslot_offset(0);
                HW_set_slave(0);
            }
            else
#endif
            {
				if ((g_LM_config_info.num_acl_links) && (g_LM_config_info.num_acl_links == g_LM_config_info.links_in_low_power))
				{
					// Was NULL pointer access here from LIH_PARK_BV40c	
					if (g_LM_config_info.next_wakeup_link && (g_LM_config_info.next_wakeup_link->role == SLAVE))
						LC_Set_Active_Piconet(g_LM_config_info.next_wakeup_link->device_index, TRUE);
					else
						LC_Restore_Piconet();
				}
				else
				{ // important to always restore piconet after inquiry scan -
					LC_Restore_Piconet();
				}
            }

            inquiryScan->return_to_piconet_DL = FALSE;
        }
    }

    /*
     * Local device link must reset to Master's channel access
     * My role must be MASTER (role_of_peer = SLAVE)
     */

    /*
     * can only do this if p_active_device_link is not 0 !!! 
     * because this function is called during initialisation
     */

    if (0 != inquiryScan->p_active_device_link)
    { 
        DL_Set_Access_Syncword(inquiryScan->p_active_device_link,
            SYSconfig_Get_Local_Syncword());

#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
        DL_Set_Role_Peer(inquiryScan->p_active_device_link, inquiryScan->peer_role);
#else
        DL_Set_Role_Peer(inquiryScan->p_active_device_link, SLAVE);
#endif
    }

    USLCchac_Procedure_Request(INQUIRYSCAN_PROCEDURE, FALSE);
    inquiryScan->ctrl_state = OS_NOT_ACTIVE;

    LC_Log_Procedure_Cancel(INQUIRYSCAN_PROCEDURE);
}

/******************************************************************
 *
 * _USLCchac_Inquiry_Scan
 *
 * 'One-shot' statemachine for the Inquiry_Scan statemachine
 *
 * Once the Inquiry Scan procedure has started (super state=Inquiry_Scan)
 * this function will be invoked during each Rx slot allocated to the 
 * USLC channel controller.
 *
 *****************************************************************/
void USLCinqScan_Inquiry_Scan(boolean look_ahead_to_next_frame)
{
    t_packet event;
    t_inquiryScan *inquiryScan = &_inquiryScan_info;
    t_LC_Event_Info event_info;

    event = DL_Get_Rx_Packet_Type(inquiryScan->p_active_device_link);
  
    if (inquiryScan->ctrl_state == OS_START)
    {
        _USLCinqScan_OS_START(); /*Lamda transition to OS_INQUIRY_SCAN state*/
    }
      
    switch(inquiryScan->ctrl_state)
    {
    case OS_INQUIRY_SCAN:
        if (event == IDpkt)
        {
            _USLCinqScan_Generate_FHS();
		}
		else if (BTtimer_Is_Expired(inquiryScan->inquiry_scan_window_timer))
		{
			if (INTERLACED_SCAN == inquiryScan->scan_type)
			{
				LSLChop_Set_Train(Alternative_Scan);     
				LSLCpkt_Prepare_Receiver_ID(inquiryScan->p_active_device_link,Inquiry_Scan);
					inquiryScan->inquiry_scan_window_timer=
				BTtimer_Set_Slots(inquiryScan->inquiry_scan_window_timeout);
				inquiryScan->ctrl_state = OS_INQUIRY_SCAN_2ND_TRAIN;
			}
			else if (STANDARD_SCAN == inquiryScan->scan_type)
			{
	            inquiryScan->ctrl_state = OS_END;
			}
			else /* if (CONTINUOUS_SCAN == inquiryScan->scan_type) */
			{ 
				/*
				 * Set the inquiry_scan_window_timer to expire 1 tick
				 * after CLKN bit 12 changes, ie next 1.28 sec boundary
				 */
				inquiryScan->inquiry_scan_window_timer=
				   BTtimer_Set_Ticks( 0x1000 - (HW_get_native_clk() & 0x0fff) );
				LSLCpkt_Prepare_Receiver_ID(inquiryScan->p_active_device_link,Inquiry_Scan);
			}
		}
#if (PRH_BS_CFG_SYS_PERSISTENT_SCO_IN_SCANS_SUPPORTED==1)
        /*
         * Special consideration when scanning with persistent sco, to restore
         * radio to full rx and prepare receiver for id packets again.
         * Issue here:
         * This Calculates new input X for scan frequency here eventhough this
         * is returning to the same inqury scan subroutine.
         */
        else if(HWradio_GetRadioMode()!=RADIO_MODE_FULL_RX && LMscoctr_Get_Number_SYN_Connections())
        {
            HWradio_SetRadioMode(RADIO_MODE_FULL_RX);
            LSLCpkt_Prepare_Receiver_ID(inquiryScan->p_active_device_link,Inquiry_Scan);
        }
#endif

        break;

	case OS_INQUIRY_SCAN_2ND_TRAIN:
        if (event == IDpkt)
        {
            _USLCinqScan_Generate_FHS();
		}
		else if (BTtimer_Is_Expired(inquiryScan->inquiry_scan_window_timer))
		{
            inquiryScan->ctrl_state = OS_END;
		}
#if (PRH_BS_CFG_SYS_PERSISTENT_SCO_IN_SCANS_SUPPORTED==1)
        /*
         * Special consideration when scanning with persistent sco, to restore
         * radio to full rx and prepare receiver for id packets again.
         * Issue here:
         * This Calculates new input X for scan frequency here eventhough this
         * is returning to the same inqury scan subroutine.
         */
        else if(HWradio_GetRadioMode()!=RADIO_MODE_FULL_RX && LMscoctr_Get_Number_SYN_Connections())
        {
            HWradio_SetRadioMode(RADIO_MODE_FULL_RX);
            LSLCpkt_Prepare_Receiver_ID(inquiryScan->p_active_device_link,Inquiry_Scan);
        }
#endif
        break;

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_INQUIRY_RESPONSE_SUPPORTED==1)

    case OS_INQUIRY_GENERATE_EIR_PACKET:
        if (LOOK_AHEAD==look_ahead_to_next_frame)
        {
            LSLCpkt_Generate_EIR(inquiryScan->p_active_device_link);
            inquiryScan->ctrl_state = OS_INQUIRY_W4_RESPONSE_COMPLETE;
        }
        break;
#endif

    case OS_INQUIRY_W4_RESPONSE_COMPLETE:
        if (LOOK_AHEAD==look_ahead_to_next_frame)
        {
#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
            /*
             * Provide opportunity to service other piconets
             */
            if(LMconfig_LM_Connected_As_Scatternet())
            {
                inquiryScan->ctrl_state = OS_END;
                break;
            }
#endif        
         /*
          * Need to burn a frame in order to ensure the FHS/EIR packet
          * is correctly sent.
          */
            /*
             * During the InquiryScan, device_link[0] is used for the 
             * scaning. If an ID packet is received during the scanning
             * then the intraslot_offset will be updated in the local
             * device_link.  Thus, if the device is returning to it's 
             * own piconet (ie no connection exists) then must ensure
             * that the intraslot_offset is set to 0.
             */
            DL_Set_Local_Slot_Offset(DL_Get_Local_Device_Ref(),0);
            LC_Restore_Piconet();

            /*
             * Indication that the the Inquiry Scan procedure
             * has received a valid ID pkt from an Inquirying device and it
             * should now enter the random wait period.
             */
            LC_Callback_Event_Handler(LC_EVENT_INQUIRY_SCAN_RAND_WAIT,
                                                            &event_info);
            inquiryScan->ctrl_state = OS_INQUIRY_RAND_WAIT;
#if (PRH_BS_CFG_SYS_PERSISTENT_SCO_IN_SCANS_SUPPORTED==1)
            /*
             * If scanning with persistent SCO connections, force an immediate
             * reallignment to native timing for next potential SCO_MASTER activity.
             */
            if ((DL_Get_Ctrl_State(DL_Get_Local_Device_Ref()) == CONNECTED_TO_SLAVE)
                && LMscoctr_Get_Number_SYN_Connections())
            {
                HW_set_intraslot_offset(0);
            }
#endif
        }
        break;

    case OS_INQUIRY_RAND_WAIT:
#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
        /*
         * Provide opportunity to service other piconets
         */
        if(LMconfig_LM_Connected_As_Scatternet())
        {
            inquiryScan->ctrl_state = OS_END;
            break;
        }
#endif
        /*
         * This state will be entered after the 'random timer' at the 
         * higher level has expired.
         */
        DL_Set_Role_Peer(inquiryScan->p_active_device_link,MASTER);

        DL_Set_Access_Syncword(inquiryScan->p_active_device_link,
                                         inquiryScan->inquiry_scan_syncword);

        /*
         * Ensure that the Rx_Packet_type is initialised.
         */
        DL_Set_Rx_Packet_Type(inquiryScan->p_active_device_link, INVALIDpkt);
  
        /*
         * From the point of view of the USLC_Chac, during the random wait
         * the InquiryScan procedure is treated as being NOT_ACTIVE.
         * When the random wait period has ended, the USLC_Chac object is 
         * notified that the Inquiry Scan procedure is back in the 
         * ACTIVE state.
         */
        USLCchac_Procedure_Start(INQUIRYSCAN_PROCEDURE, 
                                        inquiryScan->p_active_device_link);          
        USLCchac_Set_Device_State(Inquiry_Scan);

        HWradio_SetRadioMode(RADIO_MODE_FULL_RX);
        LSLCpkt_Prepare_Receiver_ID(inquiryScan->p_active_device_link,
                Inquiry_Scan);
        inquiryScan->inquiry_scan_window_timer=
             BTtimer_Set_Slots(inquiryScan->inquiry_scan_window_timeout);

        inquiryScan->ctrl_state = OS_INQUIRY_SCAN;
        break;

    case OS_NOT_ACTIVE:
        break;

    default:  
        break;
    }

    if (OS_END == inquiryScan->ctrl_state)
    {
        USLCinqScan_Cancel();

        LC_Callback_Event_Handler(LC_EVENT_INQUIRY_SCAN_TIMEOUT,&event_info);
        _inquiryScan_info.ctrl_state = OS_NOT_ACTIVE;
    }
}

/******************************************************************
 *
 * USLCinqScan_Initialise
 * 
 * Initialises the defaults for Inquiry Scan procedure.
 *
 *****************************************************************/
void USLCinqScan_Initialise(void)
{
    t_inquiryScan volatile *inquiryScan = &_inquiryScan_info;
    
    /*
     * Initialise defaults for Inquiry Scan
     */
    inquiryScan->inquiry_resp_timeout = INQ_RESP_TIMEOUT_DEFAULT;
    inquiryScan->scan_type = STANDARD_SCAN;
    inquiryScan->notify_procedure_start=TRUE;
    inquiryScan->ctrl_state = OS_NOT_ACTIVE;
}



/******************************************************************
 *
 *  
 * 
 *****************************************************************/
void _USLCinqScan_OS_START(void)
{
    t_inquiryScan *inquiryScan = &_inquiryScan_info;

    if (inquiryScan->notify_procedure_start)
    {
        LC_Log_Procedure_Start(INQUIRYSCAN_PROCEDURE);
        USLCchac_Procedure_Start(INQUIRYSCAN_PROCEDURE, 
            inquiryScan->p_active_device_link);
        inquiryScan->notify_procedure_start=FALSE;
    }
    else
    {
        USLCchac_Set_Device_State(Inquiry_Scan);
    }

    /*
     * For the purposes of Inquiry Scan the role of the peer device
     * is set to Master (i.e. the device performing an Inquiry is 
     * assigned the role of Master).
     */
    DL_Set_Role_Peer(inquiryScan->p_active_device_link,MASTER);
    DL_Set_Rx_Packet_Type(inquiryScan->p_active_device_link, INVALIDpkt);

    DL_Set_Access_Syncword(inquiryScan->p_active_device_link,
                                         inquiryScan->inquiry_scan_syncword);
    if (CONTINUOUS_SCAN == inquiryScan->scan_type)
	{
		/*
		 * Set the inquiry_scan_window_timer to expire 1 tick
		 * after CLKN bit 12 changes, ie next 1.28 sec boundary
		 */
		inquiryScan->inquiry_scan_window_timer=
		   BTtimer_Set_Ticks( 0x1000 - (HW_get_native_clk() & 0x0fff) );

	}
	else
	{
	inquiryScan->inquiry_scan_window_timer=
           BTtimer_Set_Slots(inquiryScan->inquiry_scan_window_timeout);
	}   
      
    inquiryScan->ctrl_state = OS_INQUIRY_SCAN;
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

    HWradio_SetRadioMode(RADIO_MODE_FULL_RX);
    
    LSLChop_Set_Train(Standard_Scan);
    LSLCpkt_Prepare_Receiver_ID(inquiryScan->p_active_device_link,Inquiry_Scan);
}

/******************************************************************
 *
 * _USLCinqScan_Generate_FHS
 * 
 *****************************************************************/
void _USLCinqScan_Generate_FHS(void)
{
     t_inquiryScan *inquiryScan = &_inquiryScan_info;

     DL_Set_Rx_Packet_Type(inquiryScan->p_active_device_link, INVALIDpkt);
     HWradio_SetRadioMode(RADIO_MODE_TX_RX);
     LSLCpkt_Generate_FHS(inquiryScan->p_active_device_link, InquiryResponse);

	 /*
      * Add an offset of 1 to the phase in the inquiry 
      * hop sequence.
      */
	 LSLChop_Increment_N_Inquiry_Scan();

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_INQUIRY_RESPONSE_SUPPORTED==1)
     if (LC_Get_Local_EIR_Length()!=0)
     {
         inquiryScan->ctrl_state = OS_INQUIRY_GENERATE_EIR_PACKET;
     }
     else
#endif
     {
        inquiryScan->ctrl_state = OS_INQUIRY_W4_RESPONSE_COMPLETE;
     }
}

#else /*(PRH_BS_CFG_SYS_INQUIRY_SCAN_SUPPORTED==0)*/
/*
 * Inquiry scan is not supported, functions should not be invoked.
 */
t_error USLCinqScan_Request(t_lap inquiry_lap,t_slots scan_timeout,
                                                        t_scan_type scan_type)
{
    return UNSUPPORTED_FEATURE;
}

void USLCinqScan_Cancel(void)
{
}

void USLCinqScan_Initialise(void)
{
}

void USLCinqScan_Inquiry_Scan(boolean look_ahead_to_next_frame)
{
    t_LC_Event_Info event_info;

    event_info.status = UNSUPPORTED_FEATURE;
    LC_Callback_Event_Handler(LC_EVENT_INQUIRY_SCAN_TIMEOUT, &event_info);
}

#endif /*(PRH_BS_CFG_SYS_INQUIRY_SCAN_SUPPORTED==1)*/

















