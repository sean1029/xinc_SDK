/*************************************************************************
 * MODULE NAME:    uslc_inquiry.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    Baseband Channel Control Processor Module
 * MAINTAINER:     Tom Kerwick
 *
 * SOURCE CONTROL: $Id: uslc_inquiry.c,v 1.68 2014/03/11 03:13:47 garyf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 *
 * NOTES TO USERS:
 *    
 *************************************************************************/

#include "sys_config.h"
#include "lc_interface.h"

#if (PRH_BS_CFG_SYS_INQUIRY_SUPPORTED==1)

#include "lc_types.h"

#include "bt_addr_sync.h"
#include "bt_timer.h"

#include "hw_lc.h"

#include "hw_memcpy.h"

#include "uslc_inquiry.h"
#include "lc_log.h"

#include "dl_dev.h"

#include "lslc_pkt.h"
#include "lslc_access.h"
#include "lslc_hop.h"
#include "uslc_chan_ctrl.h"

#include "lmp_config.h"
#include "lmp_inquiry.h"

/*
 * Private constants, types and variables
 */

typedef enum
{ 
    OS_START,               /* start control-state */
    OS_END,                 /* end control-state */
    OS_NOT_ACTIVE,          /* procedure currently not active */
    OS_INQUIRY_TX_RX,        /* inquiry tx/rx control-state */ 
    OS_EXTENDED_INQUIRY_RESPONSE, /* extended inquiry response */
	OS_SUSPEND,
	OS_RESUME
} t_inquiry_ctrl_state;         

typedef struct t_inquiry
{
    t_timer inquiry_timer;
    t_timer Ninquiry_timer;
	t_timer inquiry_suspend_timer;
    t_slots inquiry_timeout;
    t_syncword inquiry_syncword;
    t_devicelink *p_active_device_link;
    t_inquiry_ctrl_state ctrl_state;
    u_int16 n_inquiry;
} t_inquiry;


/*
 * Local support funtions used by the Inquiry statemachine
 */
void _USLCinq_Generate_ID(void);
t_error _USLCinq_Extract_FHS(void);


/*
 * _inquiry_info: Stores control information for the Inquiry procedure
 */
t_inquiry _inquiry_info;

static t_LC_Event_Info event_info;
static t_FHSpacket FHS;

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_INQUIRY_RESPONSE_SUPPORTED==1)
u_int8 _extended_inquiry_response[MAX_EIR_PACKET_LENGTH];
#endif

/******************************************************************
 *
 * USLCinq_Inquiry_Initialise
 *
 * Initialise the defaults for the baseband Inquiry procedure.
 *
 *****************************************************************/
void USLCinq_Inquiry_Initialise(void)
{
    t_inquiry volatile *inquiry = &_inquiry_info;

    event_info.fhs_packet = NULL;

    /*
     * Initialise defaults for Inquiry
     */
    inquiry->ctrl_state = OS_NOT_ACTIVE;
}

/******************************************************************
 *
 * USLCchac_Inquiry_Request
 *
 * Request from a higher layer to start the Inquiry procedure
 *
 * Note: The interval between 2 Inquiry requests must be determined
 *       randomly.
 *
 *****************************************************************/
t_error USLCinq_Inquiry_Request(
    t_lap inq_lap,      /* Syncword to be used to generate the Inquiry */ 
                             /* Access Code (IAC).                          */
 
    t_slots inquiry_timeout, /* Maximum ammount of time before the Inquiry  */ 
                             /* is halted.                                  */

    u_int16 Ninquiry           /* A single train must be repeated at least    */
                             /* Ninquiry times before a new train is used   */
) 
{
    t_inquiry volatile *inquiry = &_inquiry_info;
    t_error status = NO_ERROR;
    t_syncword syncword;

    LC_Log_Procedure_Request(INQUIRY_PROCEDURE);

    /*
     * Only activate procedure if currently not active and the procedure 
     * params are within the indicated bounds.
     * 
     * Note: INQUIRY_LENGTH_DEFAULT is expressed in units of 1.28secs
     *
     */
    if ( (inquiry->ctrl_state == OS_NOT_ACTIVE) &&
         ( inquiry_timeout > 0 ) && 
		 
		 (Ninquiry > 0) ) 
    {

        LC_Log_Procedure_Accept(INQUIRY_PROCEDURE);

        BTaddr_Build_Sync_Word(inq_lap, &syncword.high, &syncword.low);
        inquiry->inquiry_syncword = syncword;
        inquiry->inquiry_timeout = inquiry_timeout;
        inquiry->n_inquiry = Ninquiry;

        /*
         * Use the local device link to perform the Inquiry operation.
         */
        inquiry->p_active_device_link=DL_Get_Local_Device_Ref();

        /*
         * Build the inquiry hop table for the inquiry address
         * Note all inquiry frequencies use the GIAC_LAP with UAP=0!!!!
         */
        LSLChop_Build_Frequency_Table(Inquiry, (t_uap_lap) GIAC_LAP);
                 
        /* 
         * Initialise the ctrl_state of the DeviceLink to osStart
         */
        inquiry->ctrl_state = OS_START;
                
        status = USLCchac_Procedure_Request(INQUIRY_PROCEDURE,TRUE);       
    }
    else
    {   
        status = UNSPECIFIED_ERROR;
    }
    
    return status;
}

/******************************************************************
 *
 * USLCchac_Inquiry_Resume
 *
 * Request from a higher layer to start the Inquiry procedure
 *
 * Note: The interval between 2 Inquiry requests must be determined
 *       randomly.
 *
 *****************************************************************/
t_error USLCinq_Inquiry_Resume(void) 
{
    t_inquiry volatile *inquiry = &_inquiry_info;
    t_error status = NO_ERROR;
 
    /*
     * Only activate procedure if currently not active and the procedure 
     * params are within the indicated bounds.
     * 
     * Note: INQUIRY_LENGTH_DEFAULT is expressed in units of 1.28secs
     *
     */

    if((inquiry->ctrl_state != OS_END))
    {

        LC_Log_Procedure_Accept(INQUIRY_PROCEDURE);

		inquiry->inquiry_suspend_timer = BTtimer_Set_Slots(64); 
        /*
         * Use the local device link to perform the Inquiry operation.
         */
        inquiry->p_active_device_link=DL_Get_Local_Device_Ref();
  
        /* 
         * Initialise the ctrl_state of the DeviceLink to osStart
         */
        inquiry->ctrl_state = OS_RESUME;
                
        status = USLCchac_Procedure_Request(INQUIRY_PROCEDURE,TRUE);       
    }
    
    return status;
}

/******************************************************************
 * 
 * USLCchac_Inquiry_Cancel
 *
 * Request from a higher layer to cancel the current Inquiry procedure
 *
 *****************************************************************/

void USLCinq_Inquiry_Cancel(void)
{
    t_inquiry volatile *inquiry = &_inquiry_info;

    /*
     * the LM has a habit of cancelling inquiries that have not started :-)
     */
    if (inquiry->ctrl_state != OS_NOT_ACTIVE)
    {
        inquiry->ctrl_state = OS_END;
    }
}

/******************************************************************
 *
 * _USLCchac_Inquiry
 *
 * 'One-shot' statemachine for the Inquiry Procedure.
 *
 * Once the Inquiry procedure has started (super state=Inquiry) this
 * function will be invoked during each Rx slot allocated to the 
 * USLC channel controller.
 *
 *****************************************************************/

void USLCinq_Inquiry(boolean look_ahead_to_next_frame)
{
    t_inquiry *inquiry = &_inquiry_info;
    
    if (OS_START == inquiry->ctrl_state)   /* super_state=Inquiry */
    {          
        LC_Log_Procedure_Start(INQUIRY_PROCEDURE);
        
        LSLChop_Set_Train(Atrain);
              
        /* 
         * Each train is 10ms / 16 slots long.
         * Thus, train switch must taken place after every : 
         *           (Ninquiry*16 (slots)
         * Note: It is assumed that all ACL connections have been put on 
         *       hold or parked.
         */
        inquiry->Ninquiry_timer = BTtimer_Set_Slots(inquiry->n_inquiry*16);
        
       /*
        * Set timer to 2 slots earlier, since looking ahead 
        */
        inquiry->inquiry_timer = BTtimer_Set_Slots(inquiry->inquiry_timeout-2); 
	}

	if ((OS_START == inquiry->ctrl_state) || (OS_RESUME == inquiry->ctrl_state))
	{
		inquiry->inquiry_suspend_timer = BTtimer_Set_Slots(64); 

		USLCchac_Procedure_Start(INQUIRY_PROCEDURE, inquiry->p_active_device_link);
        inquiry->ctrl_state=OS_INQUIRY_TX_RX; 
	}

    /* 
     * Inquiry is a very simple procedure, no need for multiple states,
     * as can use look_ahead directly to determine what is required...
     */
    if ((OS_INQUIRY_TX_RX == inquiry->ctrl_state) ||
        (OS_EXTENDED_INQUIRY_RESPONSE == inquiry->ctrl_state))
    {    
        /*
         * If look_ahead, USLCsched_Rx_Start is reserving next frame for
         * inquiry, so generate IDs for next frame.
         */
        if (look_ahead_to_next_frame)
        {
            if(OS_EXTENDED_INQUIRY_RESPONSE != inquiry->ctrl_state)
            {
                _USLCinq_Generate_ID();
            }  
        }
        /*
         * If not a look_ahead, USLCsched_Process_Rx is concluding an
         * inquiry frame, so check for an FHSpkt reception.
         */
        else if ((FHSpkt == DL_Get_Rx_Packet_Type(inquiry->p_active_device_link))
			&& (event_info.fhs_packet == NULL))
        {
            _USLCinq_Extract_FHS();
        }
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_INQUIRY_RESPONSE_SUPPORTED==1)
        /*
         * If an extended inquiry response frame, then check for the acl
         * packet, generate the event and return to tx_rx state.
         */
        else if (OS_EXTENDED_INQUIRY_RESPONSE == inquiry->ctrl_state)
        {
            if (RX_BROADCAST_PACKET == DL_Get_Rx_Status(inquiry->p_active_device_link))
            {
                event_info.extension_length = HW_get_rx_len();
                LSLCacc_Extract_EIR_Payload(_extended_inquiry_response, 
                    event_info.extension_length);
            }

            /*
			 * Need to set the ctrl_state here as the Callback_Event handler may require
			 * to modify the ctrl_state (e.g. may call USLCinq_Inquiry_Cancel).
			 */
			inquiry->ctrl_state=OS_INQUIRY_TX_RX; 
            LC_Callback_Event_Handler(LC_EVENT_INQUIRY_RESULT,&event_info);
            USLCchac_Set_Device_State(Inquiry);
        }
#endif
        /*
         * If inquiry timer has expired, move to the OS_END state. Only
         * other way to exit this state is via USLCinq_Inquiry_Cancel.
         */
        if (BTtimer_Is_Expired(inquiry->inquiry_timer))
        {
            inquiry->ctrl_state = OS_END;
        }
        else if (BTtimer_Is_Expired(inquiry->inquiry_suspend_timer))
		{
			if	(LMconfig_LM_Connected_As_Slave())
				inquiry->ctrl_state = OS_SUSPEND;
			else
				inquiry->inquiry_suspend_timer = BTtimer_Set_Slots(64);
		}
    }

    if ((OS_END == inquiry->ctrl_state) || (OS_SUSPEND == inquiry->ctrl_state))
    {
            HWradio_SetRadioMode(RADIO_MODE_STANDBY);
        
        /*
         * cancel inquiry and implicitly disable Sync Det interrupt to
         * save interrupt processing overhead */

        /*
         * Disable the transmitter if the Inquiry procedure is currently active.
         */
        LSLCacc_Disable_Transmitter();

        USLCchac_Procedure_Request(INQUIRY_PROCEDURE, FALSE);
        
        /*
         * Local device link must reset to Master's channel access
         * My role must be MASTER as expected (role_of_peer = SLAVE)
         */
        DL_Set_Access_Syncword(inquiry->p_active_device_link,
           SYSconfig_Get_Local_Syncword() );

        LC_Log_Procedure_Cancel(INQUIRY_PROCEDURE);

        /*
         * If inquiry complete then report and end.
         * Set up event structure and
         * Use Callback to report event to higher layers 
         */
		if (OS_SUSPEND == inquiry->ctrl_state)
		{
            event_info.status = NO_ERROR;
		    LC_Callback_Event_Handler(LC_EVENT_INQUIRY_SUSPENDED,&event_info);
		}
		else // OS_END
		{
			event_info.status = NO_ERROR;
			LC_Callback_Event_Handler(LC_EVENT_INQUIRY_COMPLETE,&event_info);
		}

        inquiry->ctrl_state = OS_NOT_ACTIVE;
    }
}



/******************************************************************
 * 
 * _USLCinq_Generate_ID
 *
 * Local support funtion used by the Inquiry statemachine.
 *
 *****************************************************************/
void _USLCinq_Generate_ID(void)
{
    t_inquiry *inquiry = &_inquiry_info;

    if (BTtimer_Is_Expired(inquiry->Ninquiry_timer))
    {
        LSLChop_Toggle_Train();
        inquiry->Ninquiry_timer = BTtimer_Set_Slots(inquiry->n_inquiry*16);
    }

    /*
     * Transmit the Access Code.
     */
    HWradio_SetRadioMode(RADIO_MODE_MASTER_DOUBLE_WIN);
    
    DL_Set_Role_Peer(inquiry->p_active_device_link, SLAVE);
    DL_Set_Rx_Packet_Type(inquiry->p_active_device_link, INVALIDpkt);
    DL_Set_Access_Syncword(inquiry->p_active_device_link,inquiry->inquiry_syncword);

    LSLCpkt_Generate_ID(inquiry->p_active_device_link,Inquiry);
}

/******************************************************************
 * 
 * _USLCinq_Extract_FHS
 *
 * Local support funtion used by the Inquiry statemachine to extract
 * the received FHS packet from the H/W and report its reception to 
 * the higher layers (via call-back).
 *  
 * Note: Before the FHs packet is passed to the higer layer, the 
 *       FHS packet is modified to store the clock offset (rather
 *       than the absolute clock value of the slave's BT clk).
 *
 *****************************************************************/
t_error _USLCinq_Extract_FHS(void)
{
    t_error status = NO_ERROR;
#if (PRH_BS_CFG_SYS_LMP_RSSI_SUPPORTED==1)
    t_inquiry *inquiry = &_inquiry_info;
    s_int8 rssi = DL_Get_RSSI_For_Last_Packet_Received(inquiry->p_active_device_link);
#else
	s_int8 rssi = 0;
#endif

    LSLCacc_Extract_FHS_Payload(&FHS);

    /*
     * Store the difference in received FHS clock from the 
     * native clock FHS clock.
     */
    FHS_Change_CLK_To_Offset(&FHS, BTtimer_Get_Native_Clock());

    /* 
     * If inquiry response then report.
     * Set up event structure - Use Callback 
     * to report event to higher layers 
     */

    event_info.fhs_packet = &FHS;
    event_info.rssi = rssi; /* Carry the RSSI in the device_index field */
    event_info.status = NO_ERROR;

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_INQUIRY_RESPONSE_SUPPORTED==1)
    if((FHS_Get_EIR(&FHS)==1) && (LM_INQ_EXTENDED_INQUIRY_RESULT == LMinq_Get_Inquiry_Result_Mode()))
    {
        LSLCacc_Disable_Transmitter(); /* Cancel double IDs perpared on lookahead */
        event_info.extension_length = MAX_EIR_PACKET_LENGTH; /* default all 0s */
        hw_memset8(_extended_inquiry_response, 0x00, MAX_EIR_PACKET_LENGTH);
        inquiry->ctrl_state=OS_EXTENDED_INQUIRY_RESPONSE; 
        USLCchac_Set_Device_State(MasterExtendedInquiryResponse);
    }
    else
#endif
    {
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_INQUIRY_RESPONSE_SUPPORTED==1)
        event_info.extension_length = 0; /* signify no EIR with this event */
#endif
        status = (t_error) LC_Callback_Event_Handler(LC_EVENT_INQUIRY_RESULT,&event_info);
        LC_Log_Procedure_Recv_Pkts_State1(INQUIRY_PROCEDURE);
    }
     
    return status;
}

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_INQUIRY_RESPONSE_SUPPORTED==1)
/******************************************************************
 * 
 * USLCinq_Is_Extended_Inquiry_Response_Frame
 *
 ******************************************************************/
boolean USLCinq_Is_Extended_Inquiry_Response_Frame(void)
{
    t_inquiry *inquiry = &_inquiry_info;
    return (OS_EXTENDED_INQUIRY_RESPONSE==inquiry->ctrl_state);
}

/******************************************************************
 * 
 * USLCinq_Get_Stored_Extended_Inquiry_Response
 *
 ******************************************************************/
u_int8* USLCinq_Get_Stored_Extended_Inquiry_Response(void)
{
    return _extended_inquiry_response;
}
#endif

/******************************************************************
 * 
 * USLCinq_Inquiry_Force_To_Tx_State_If_Req
 *
 ******************************************************************/
void USLCinq_Inquiry_Force_To_Tx_State_If_Req(void)
{
    t_inquiry *inquiry = &_inquiry_info;
    inquiry->ctrl_state = OS_INQUIRY_TX_RX;
}

#else /*#if (PRH_BS_CFG_SYS_INQUIRY_SUPPORTED==1)*/
/*
 * Inquiry is not supported, functions should not be invoked.
 */

t_error USLCinq_Inquiry_Request(t_lap inq_lap,
                        t_slots inquiry_timeout, u_int16 Ninquiry)
{
    return UNSUPPORTED_FEATURE;
}

void USLCinq_Inquiry_Cancel(void)
{
}

void USLCinq_Inquiry(boolean look_ahead_to_next_frame)
{
    t_LC_Event_Info event_info;

    event_info.status = UNSUPPORTED_FEATURE;
    LC_Callback_Event_Handler(LC_EVENT_INQUIRY_COMPLETE,&event_info);
}

void USLCinq_Inquiry_Initialise(void)
{
}

#endif /*(PRH_BS_CFG_SYS_INQUIRY_SUPPORTED==1)*/
