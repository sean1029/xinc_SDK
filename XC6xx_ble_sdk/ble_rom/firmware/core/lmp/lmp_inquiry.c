/***********************************************************************
 * MODULE NAME:    lmp_inquiry.c
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    Host Controller Inquiry Entity
 * AUTHOR:         Gary Fleming
 * DATE:           27-05-1999
 *
 * SOURCE CONTROL: $Id: lmp_inquiry.c,v 1.94 2014/03/11 03:14:00 garyf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 * 
 * DESCRIPTION
 *   This object caters for initiation and termination of an Inquiry
 *   in the Host Control. It interacts with the baseband and the 
 *   HCI to perform the inquiry procedure. 0nly one inquiry result 
 *   is presented per Inquiry Result event.           
 *******************************************************************/      

#include "sys_config.h"

#include "sys_rand_num_gen.h"
#include "bt_fhs.h"
#include "lc_interface.h"
#include "hc_event_gen.h"
#include "hc_const.h"

#include "lmp_scan.h"
#include "lmp_timer.h"
#include "lmp_utils.h"
#include "lmp_inquiry.h"
#include "lmp_con_filter.h"
#include "lmp_config.h"
#include "lmp_acl_container.h"
#include "lmp_sco_container.h"
#include "lmp_ch.h"
#include "lmp_lc_event_disp.h"

#include "bt_timer.h"

#include "le_connection.h"

#if (PRH_BS_CFG_SYS_LE_DUAL_MODE==1)
#include "le_link_layer.h"
#endif

#define mLMinq_Get_Inquiry_Train_Reps() \
        ((u_int16)(256 * (1+LMscoctr_Get_Number_SCO_Connections())))


/******************************************
 *  INQUIRY Data Structure
 *
 ******************************************/

static struct LM_Inq
{
   t_inquiry_states state;     /* state of the Inquiry Object                            */        
   t_slots inquiryTO;          /* Length of the Inquiry Process ( N * 1.28s)             */
   u_int8 inq_timer_index;     /* Index for the Inquiry Process Timer                    */
   t_lap inquiryLap;           /* The LAP used for the inquiry                           */
   u_int8 maxResp;             /* Max number of reponses in an Inquiry Process           */
   u_int8 numResp;             /* The current number Responses to an Inquiry Process     */
#if (PRH_BS_CFG_SYS_INQUIRY_PERIODIC_SUPPORTED==1)
   u_int32 next_inq_period;    /* Time of the next Inquiry Process       - Periodic Only */
   u_int16 maxPeriod;          /* The Max time between Inquiry Processes - Periodic Only */
   u_int16 minPeriod;          /* The Min time between Inquiry Processes - Periodic Only */ 
   u_int8 mode_timer_index;    /* Timer Index for Periodic Mode                          */
#endif
   u_int8 Inquiry_Mode;        /* Periodic Or One Shot inquiry                           */
#if (PRH_BS_CFG_SYS_LMP_RSSI_INQUIRY_RESULTS_SUPPORTED==1)
   t_inquiry_result_mode inquiry_result_mode; /* Return RSSI,EIR in Inquiry Result events */
#endif
} LM_Inq;

static t_inquiryResult latest_inq_result;

#if (PRH_BS_CFG_SYS_INQUIRY_UNIQUE_RESULTS_SUPPORTED==1)
static t_bd_addr unique_inq_results[MAX_INQUIRY_RESPONSES];
#endif

/* Prototypes of functions local to LM_Inquiry object */

/***************************************
 * Prototypes of local Functions to send LM Inquiry Events
 ***************************************/

static void _Send_LM_Inquiry_Result_Event(void);
static void _Send_LM_Inquiry_Complete_Event(t_error status);

static void LMinq_Inquiry_Complete_Success(t_lmp_link* p_link);

#if (PRH_BS_CFG_SYS_INQUIRY_PERIODIC_SUPPORTED==1)
static void LMinq_Period_Interval_Timeout(t_lmp_link* p_link);
static u_int32 _Get_Interval_Rand(u_int16 max_value, u_int16 min_value);
#endif

static boolean LMinq_Inquiry_Event_Filter(t_bd_addr*, t_classDevice);

t_error LMinq_Inquiry_Suspend(t_LC_Event_Info* p_event_info);
static void LMinq_Inquiry_Suspend_NonIRQ(t_lmp_link* p_link);
void LMinq_Inquiry_Resume(t_lmp_link* p_link);

/*******************************************************
 * Function : LM_Inquiry_Initialise
 *
 * Inputs : none
 *  
 * DESCRIPTION : This initialises the Inquiry objects.
 *
 ********************************************************/
void LMinq_Initialise(void)
{
    LM_Inq.state = INQUIRY_IDLE;
    LM_Inq.inquiryTO = 0;
    LM_Inq.inq_timer_index = 0;
    LM_Inq.inquiryLap = 0;
    LM_Inq.maxResp   = 0;
    LM_Inq.numResp   = 0;
#if (PRH_BS_CFG_SYS_INQUIRY_PERIODIC_SUPPORTED==1)
    LM_Inq.next_inq_period = 0;
    LM_Inq.maxPeriod = 0;
    LM_Inq.minPeriod = 0;
    LM_Inq.mode_timer_index = 0;
#endif
    LM_Inq.Inquiry_Mode = PRH_ONE_SHOT_INQUIRY;

#if (PRH_BS_CFG_SYS_LMP_RSSI_INQUIRY_RESULTS_SUPPORTED==1)
   LM_Inq.inquiry_result_mode = 0 ;  /* Standard Inquiry Result */
#endif

    

    /************************************************
     * Initialise the callback functions from the Baseband
     ************************************************/

    LC_Register_Event_Handler(LC_EVENT_INQUIRY_COMPLETE,(void*)LMinq_Inquiry_Complete);
    LC_Register_Event_Handler(LC_EVENT_INQUIRY_RESULT, (void*)LMinq_Inquiry_Result);

	LC_Register_Event_Handler(LC_EVENT_INQUIRY_SUSPENDED, (void*)LMinq_Inquiry_Suspend);
}

/********************************************************************
*
* FUNCTION :  LMinq_Inquiry_Start
*
* USAGE:- HCI_Inquiry
*       (3 parameters)
*        0. LAP [3 bytes]
*        1. Inquiry_Length [1 bytes]
*             Options :- [ Inquiry_Length_def=0x1]
*        2. Num_Responses [1 bytes]
*
* INPUTS :
* lap          - the LAP to be used to generate the Inquiry Access Code
* inq_length   - the total duration of the Inquiry Mode ( N * 1.28 sec)
* max_resp     - the number of responses that can be received before the
*                 inquiry is halted.
*
*    
* DESCRIPTION : 
* This function is called by the HC Command Dispatcher on receipt
* of a HCI_Inquiry Command. It initiates an inquiry in the baseband. 
*
*  The following steps are performed :-
*     1/ Initiate the local Inquiry Structure
*     3/ Disable Scans.
*     4/ Initiate Baseband Inquiry
*     5/ Move state to W4_INQUIRY_RESULT.
*
****************************************************************************/

t_error LMinq_Inquiry_Start(t_lap lap, u_int8 inq_length, u_int8 max_resp)
{
    t_error status = UNSPECIFIED_ERROR;

    /*
     * Check that we have bandwidth available for the activity
     */
#if ((PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1) || (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1) )
    if (LMscoctr_Is_All_Bandwidth_Used_By_SYN())
    {
        return COMMAND_DISALLOWED;
    }
#endif

#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1)
    /* 
     * Disallow Connection Establishment / Inquiry if both the following are not satisifed:
     * NO SCO connections or 1 HV3 SCO.
     */
    if (!( (LMscoctr_Get_Number_SCO_Connections() == 0) ||
         ( (LMscoctr_Get_Number_SCO_Connections() == 1) && 
         (LMscoctr_Get_Active_LM_SCO_Packet_Type() == LM_SCO_PACKET_HV3)) ))
    {
        return COMMAND_DISALLOWED;
    }
#endif

    if ((INQUIRY_IDLE == LM_Inq.state) &&
        (PRH_PERIODIC_INQUIRY != LM_Inq.Inquiry_Mode))
    {
        LM_Inq.inquiryLap = lap;
        LM_Inq.inquiryTO = ((t_clock)inq_length) << 11 ;
        LM_Inq.numResp = 0;
        LM_Inq.maxResp = max_resp;

        /* Initiate the Inquiry in the baseband */
        LM_Inq.state = W4_INQUIRY_RESULT;

        status = LC_Inquiry_Request(lap, LM_Inq.inquiryTO, 
                    mLMinq_Get_Inquiry_Train_Reps());

        if (status == NO_ERROR)
            LMconfig_LM_Set_Device_State(PRH_DEV_STATE_INQUIRY_PENDING);

        if (status != NO_ERROR)
           LM_Inq.state = INQUIRY_IDLE;
    }
    else
    {
        status = COMMAND_DISALLOWED ;
    }
    return status;
}

/***************************************************************************
* FUNTION : LMinq_Periodic_Inquiry
*
* INPUTS :
* max_len      - The maximum time between the invokation of inquiry ( N * 1.28sec)
* min_len      - The minimum time between the invokation of inquiry ( N * 1.28sec)
* lap          - the LAP to be used to generate the Inquiry Access Code
* inq_length   - the total duration of the Inquiry Mode ( N * 1.28 sec)
* max_resp     - the number of responses that can be received before the
*                inquiry is halted.
* DESCRIPTION :-
* Periodic inquiry extends the one shot inquiry to invoke inquiry at a pseudo
* random interval. Each inquiry however is for a fixed period of time.
*
***************************************************************************/

t_error LMinq_Periodic_Inquiry(u_int16 max_len, u_int16 min_len, t_lap lap, 
                               u_int8 inq_length, u_int8 max_resp)
{
#if (PRH_BS_CFG_SYS_INQUIRY_PERIODIC_SUPPORTED==1)
    t_slots interval_length;
    t_error status = NO_ERROR;

    /*
     * Disallow if current mode is already Periodic Inquiry
     * Disallow if already Inquiring and awaiting result
     */
    if((PRH_PERIODIC_INQUIRY == LM_Inq.Inquiry_Mode)||
        (W4_INQUIRY_RESULT == LM_Inq.state))
        return COMMAND_DISALLOWED;

    /****************************
     * Store the parameters in the
     * Inquiry data structure
     ****************************/
    LM_Inq.maxPeriod = max_len;
    LM_Inq.minPeriod = min_len;
    LM_Inq.inquiryLap = lap;
    LM_Inq.maxResp = max_resp;
    LM_Inq.inquiryTO = ((t_clock)inq_length) << 11 ;
    LM_Inq.Inquiry_Mode = PRH_PERIODIC_INQUIRY;
    LM_Inq.numResp = 0;

   /************************************************      
    * Determine the next Rand interval for timing 
    ************************************************/  

    interval_length = _Get_Interval_Rand(max_len, min_len) << 11;       

    /***********************************************
     * Set a timer for the next Inquiry 
     **********************************************/
    LM_Inq.mode_timer_index = LMtmr_Set_Timer(interval_length, 
         LMinq_Period_Interval_Timeout, (t_lmp_link*)0, 1);

    /***********************************************
     * Initiate the current Inquiry
     **********************************************/
    LM_Inq.state = W4_INQUIRY_RESULT;
    status = LC_Inquiry_Request(lap, LM_Inq.inquiryTO,
                        mLMinq_Get_Inquiry_Train_Reps());

    if (status == NO_ERROR)
        LMconfig_LM_Set_Device_State(PRH_DEV_STATE_INQUIRY_PENDING);

    return status;
#else
    return NO_ERROR;
#endif
}

/***************************************************************************
* FUNTION : LMinq_Inquiry_Result
*
* INPUTS : 
* p_FHS - a pointer to the FHS packet received from the most recent device 
*        to respond to the inquiry procedure.
*
* DESCRIPTION
* This procedure is normally invoked by the baseband to return a result 
* from an inquiry in the baseband. This procedure will be invoked for 
* each seperate result received during the baseband Inquiry Procedure.
*  
* The following steps are performed :-
*    1/ Check if in valid state.  
*    2/ Obtain the Bd_Addr and Class of Device from the FHS
*    3/ Perform a Results Filter using the Bd_Addr and Class of Device
*    4/ If Successful Filter then increment the number of results 
*       obtain the following from the FHS and store them in the 
*       inquiry results array :-
*
*                  scan repitition mode
*                  scan period mode
*                  page scan mode
*                  clock offset
*                  Bd_addr
*                  Class of Device
*
*   5/ Increment the number of results and send an Inquiry Results Event
*      to the HCI.
*
*   6/ If the maximum number of results has been reached then perform the
*      following.
*          6.1/ Stop the baseband Inquiry
*          6.2/ Return the Scan Enables to previous state
*          6.3/ Change Inquiry State to Idle
*          6.4/ Send an Inquiry Complete Event to the HCI
*                               
*
*  NOTE : If the remote device supports only mandatory paging
*  the inquiry result must be sent immediately to the HCI.
* 
*  resultIndex :- is the number of results since the last Inquiry Result
*                 Event was sent to the HCI.
*  
*  LM_Inq.numResp :- is the total number of results since the Inquiry was
*  initiated.
*
***************************************************************************/

static void LMinq_Inquiry_Result_Handler(t_lmp_link *p_link_event_info);

t_error LMinq_Inquiry_Result(t_LC_Event_Info* p_event_info)
{
    t_error status = NO_ERROR;

    if (W4_INQUIRY_RESULT == LM_Inq.state)
    {
		LM_DEFER_FROM_INTERRUPT_CONTEXT(LMinq_Inquiry_Result_Handler, (t_lmp_link *)p_event_info);
    } 
    else 
	{
        p_event_info->fhs_packet = NULL;
        status = COMMAND_DISALLOWED;
	}

    return status;
}

static void LMinq_Inquiry_Result_Handler(t_lmp_link *p_link_event_info)
{
    t_classDevice  class_device;
    t_bd_addr      bd_addr;  
    t_FHSpacket*   p_FHS;
    t_inquiryResult* p_inquiry_result;

    t_LC_Event_Info* p_event_info = (t_LC_Event_Info*)p_link_event_info;

    p_FHS = p_event_info->fhs_packet;

    /******************************************************
     * First Get the Bd_Addr and Class Device from the FHS 
     ******************************************************/

    FHS_Get_Bd_Addr_Ex(p_FHS, &bd_addr);
    class_device = FHS_Get_Device_Class(p_FHS);


    if(LMinq_Inquiry_Event_Filter(&bd_addr,class_device)) 
    {
#if (PRH_BS_CFG_SYS_INQUIRY_UNIQUE_RESULTS_SUPPORTED==1)
        int i;
#endif
        boolean already_found = FALSE;
#if (PRH_BS_CFG_SYS_INQUIRY_UNIQUE_RESULTS_SUPPORTED==1)
        for (i=0; (i < LM_Inq.numResp) && (i < MAX_INQUIRY_RESPONSES); i++)   
            if(LMutils_Bd_Addr_Match(&bd_addr,&unique_inq_results[i]))   
                already_found = TRUE;
#else
		already_found = FALSE;
#endif
        if (!already_found)
        {
            /**********************************************************
             * Save bd_addr. If array is full, overwrite oldest entry 
             **********************************************************/
#if (PRH_BS_CFG_SYS_INQUIRY_UNIQUE_RESULTS_SUPPORTED==1)
            LMutils_Set_Bd_Addr(&unique_inq_results[LM_Inq.numResp%MAX_INQUIRY_RESPONSES],&bd_addr);
#endif
    
            /**************************************** 
             * Increment the total number of results 
             ****************************************/
            LM_Inq.numResp++;
        }

        p_inquiry_result = &latest_inq_result;
        /***********************************************************
         * Store the contents of Inquiry Result in the results array
         *********************************************************/

        p_inquiry_result->srMode = FHS_Get_Page_Scan_Repetition(p_FHS);
        p_inquiry_result->spMode = FHS_Get_Page_Scan_Period(p_FHS);
        p_inquiry_result->pageScanMode  = FHS_Get_Page_Scan_Mode(p_FHS);
        /*
         * Bug:  Bits 16:2 of Clock Offset are returned
         */
        p_inquiry_result->clkOffset= (FHS_Get_CLK(p_FHS) >> 2) & 0x7FFF;
        LMutils_Set_Bd_Addr(&p_inquiry_result->bd_addr,&bd_addr);
        p_inquiry_result->Cod = class_device;

#if (PRH_BS_CFG_SYS_LMP_RSSI_INQUIRY_RESULTS_SUPPORTED==1)
        p_inquiry_result->rssi = p_event_info->rssi;
#endif

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_INQUIRY_RESPONSE_SUPPORTED==1)
        p_inquiry_result->extension_length = p_event_info->extension_length;
#endif

        _Send_LM_Inquiry_Result_Event();
    }

    p_event_info->fhs_packet = NULL;

    /******************************************************************
     *  If max num responses have been reached then halt the inquiry
     *  and Generate an inquiry complete event for the HCI (containing
     *  total nmuber of results) and change the current state to idle.
     ******************************************************************/

    if((LM_Inq.numResp == LM_Inq.maxResp) && (LM_Inq.maxResp != 0))
    {
        LC_Inquiry_Cancel();
    }
}

t_error LMinq_Inquiry_Suspend(t_LC_Event_Info* p_event_info)
{
	LM_DEFER_FROM_INTERRUPT_CONTEXT(LMinq_Inquiry_Suspend_NonIRQ,(t_lmp_link*)0);
	return NO_ERROR;
}

static void LMinq_Inquiry_Suspend_NonIRQ(t_lmp_link* p_link)
{
	extern t_link_entry link_container[PRH_MAX_ACL_LINKS];
	int i;

	if (LMconfig_LM_Connected_As_Slave())
	{
	    for (i = 0; i < PRH_MAX_ACL_LINKS; i++)
		{
			if (link_container[i].used)
			{
				p_link = &(link_container[i].entry);
				if (SLAVE == p_link->role)
					break;
			}
		}

		LMtmr_Set_Timer(0x28, LMinq_Inquiry_Resume, 0, 1);
		LMch_Change_Piconet((t_lmp_link*)p_link);
	}
	else
	{
		LC_Inquiry_Resume();
	}
}

void LMinq_Inquiry_Resume(t_lmp_link* p_link)
{
	LC_Inquiry_Resume();
}



/*******************************************************************
*
* FUNCTION :- LM_Inquiry_Complete
*
* DESCRIPTION :- Invoked by the Baseband to inform that the inquiry
* in the baseband is complete. Under normal condictions the reason for
* completion is that the inquiry timer has expired. However, error 
* condictions in the baseband may also cause an early termination of
* the inquiry procedure.
*
* The Following Steps are performed
*    1/ Return the scan Enable to state held prior to inquiry
*    2/ Move to IDLE state
*    3/ Send an Inquiry Complete Event to the HCI.
*    4/ Reset the result index.
*    5/ If no ACL Links and no scanning then sleep the LC until the
*       the next inquiry process.
*
******************************************************************/

t_error LMinq_Inquiry_Complete(t_LC_Event_Info* p_event_info)
{
    LM_DEFER_FROM_INTERRUPT_CONTEXT(LMinq_Inquiry_Complete_Success,(t_lmp_link*)0);

    return NO_ERROR;
}

static void LMinq_Inquiry_Complete_Success(t_lmp_link* p_link)
{		
    if (LM_Inq.state == W4_INQUIRY_RESULT)
    {
		LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_INQUIRY_PENDING);
		LM_Inq.state = INQUIRY_IDLE;
#if (PRH_BS_CFG_SYS_INQUIRY_SUPPORTED==1)
		_Send_LM_Inquiry_Complete_Event(NO_ERROR);
#else
		_Send_LM_Inquiry_Complete_Event(UNSUPPORTED_FEATURE);
#endif
		
#if (PRH_BS_CFG_SYS_SLEEP_IN_STANDBY_SUPPORTED == 1)
		/* If periodic inquiry then check if we should enable low power */
#if (PRH_BS_CFG_SYS_INQUIRY_PERIODIC_SUPPORTED==1)
		if (LMconfig_LM_Periodic_Inquiry_Allowed() && (LM_Inq.Inquiry_Mode == PRH_PERIODIC_INQUIRY))
		{
			t_slots interval = 0;
			u_int8 dev_index=0;
			u_int32 clock = LC_Get_Native_Clock();

            if (LM_Inq.next_inq_period > clock)
			{
				interval = (BTtimer_Clock_Difference(clock,LM_Inq.next_inq_period) >> 1);
				
				if (NO_SCANS_ENABLED != LMscan_Read_Scan_Enable()) 
				{
					// Only consider Scans if no links 
					t_slots slots_to_scan;
					t_clock next_instant;
					
					LC_Is_Sleep_Possible(&next_instant);
					slots_to_scan = LMscan_Get_Interval_To_Next_Scan(next_instant);
					
					if (slots_to_scan < interval)
						interval = slots_to_scan; 
				}
				
				if(g_LM_config_info.num_acl_links &&
				  (g_LM_config_info.num_acl_links == g_LM_config_info.links_in_low_power))
				{
					t_slots slot_to_acl_activity;
					
					slot_to_acl_activity = (u_int16)(LMpol_Get_Interval_To_Next_Wakeup());
					
					if (slot_to_acl_activity < interval)
					{
						interval = slot_to_acl_activity; 
						dev_index = g_LM_config_info.next_wakeup_link->device_index;
					}

				}	
#if (PRH_BS_CFG_SYS_LE_DUAL_MODE==1)
				{
					t_slots le_sleep_slots = LE_LL_InactiveSlots();
					if (le_sleep_slots < interval)
					{
						interval = le_sleep_slots;
					}
				}
#endif
			}

			if ( ( interval > SYS_CFG_MIN_SLOTS_FOR_SLEEP_PROCEDURE) &&
				((g_LM_config_info.num_acl_links == 0) ||
				 (g_LM_config_info.num_acl_links == g_LM_config_info.links_in_low_power)))
				LC_Sleep(interval - SYS_CFG_MIN_SLOTS_FOR_SLEEP_PROCEDURE,dev_index);
		}
#endif
#endif
	}
	if (LM_Inq.state == W4_INQUIRY_CANCEL_COMPLETE)
	{
		LMconfig_LM_Clear_Device_State(PRH_DEV_STATE_INQUIRY_PENDING);
		LM_Inq.state = INQUIRY_IDLE;
	}
	
	if ((LMconfig_LM_Num_Piconets() > 1) || (LMconfig_LM_Connected_As_Slave()))
		LMch_Change_Piconet((t_lmp_link*)0);
}

/*****************************************************************
 * FUNCTION :- LMinq_Exit_Periodic_Inquiry_Mode
 *
 * DESCRIPTION
 * Invoked by the higher layers to terminat periodic inquiry mode.
 * This results in the mode timer being cleared, scanning re-enabled 
 * and any pending inquiries being canceled in the LC.
 *****************************************************************/

t_error LMinq_Exit_Periodic_Inquiry_Mode()
{
#if (PRH_BS_CFG_SYS_INQUIRY_PERIODIC_SUPPORTED==1)
    if (LM_Inq.Inquiry_Mode == PRH_PERIODIC_INQUIRY)
    {
        LMtmr_Clear_Timer(LM_Inq.mode_timer_index);
        LM_Inq.mode_timer_index = 0;
        LM_Inq.Inquiry_Mode = PRH_ONE_SHOT_INQUIRY;
        if (LM_Inq.state == W4_INQUIRY_RESULT)
        {
            LM_Inq.state = W4_INQUIRY_CANCEL_COMPLETE;
        return LC_Inquiry_Cancel();
        }
        else
            return NO_ERROR;
    }
    else
    {
        return COMMAND_DISALLOWED;
    }
#endif
    return NO_ERROR;
}
    
/*****************************************************************
 * FUNCTION :- LMinq_Period_Interval_Timeout
 *
 * DESCRIPTION
 * Called on timer expiry. This invokes an inquiry process as part
 * of a periodic inquiry.
 *****************************************************************/

void LMinq_Period_Interval_Timeout(t_lmp_link* p_link)
{
#if (PRH_BS_CFG_SYS_INQUIRY_PERIODIC_SUPPORTED==1)
   u_int32 interval_length;
   t_error status;
   u_int32 current_clk= LC_Get_Native_Clock();;
  
   LM_Inq.numResp = 0;

   LMinq_Inquiry_Cancel();

   if (LM_Inq.Inquiry_Mode == PRH_PERIODIC_INQUIRY)
   {
      /************************************************/      
      /* Determine the next Rand interval for timing */ 
      /************************************************/  

      interval_length = _Get_Interval_Rand(LM_Inq.maxPeriod,LM_Inq.minPeriod)<<11;
      LM_Inq.mode_timer_index = LMtmr_Set_Timer(interval_length,LMinq_Period_Interval_Timeout,p_link,1);

      /* set up the time for the next period */
      LM_Inq.next_inq_period = current_clk + (interval_length<<1);

      if (LMconfig_LM_Periodic_Inquiry_Allowed())
	  {
		  /* Initiate the Inquiry in the baseband */
		  LM_Inq.state = W4_INQUIRY_RESULT;
		  status = LC_Inquiry_Request(LM_Inq.inquiryLap, LM_Inq.inquiryTO,
			  mLMinq_Get_Inquiry_Train_Reps());

		  if (status == NO_ERROR)
			  LMconfig_LM_Set_Device_State(PRH_DEV_STATE_INQUIRY_PENDING);
	  }

   }
#endif
}                       

/*************************************************************************
* FUNCTION :- LM_Inquiry_Cancel
*
* DESCRIPTION
* Invoked by the Host this halts the inquiry in the baseband.
*
* The Following Steps are performed
*    1/ Check if in the W4_INQUIRY_RESULT_STATE
*    2/ Return the scan Enable to state held prior to inquiry
*    3/ Move to INQUIRY_IDLE state
*
* NOTE :- The Inquiry Complete Event should not be sent.
*
*************************************************************************/

t_error LMinq_Inquiry_Cancel()
{
    t_error status = COMMAND_DISALLOWED;

    if(W4_INQUIRY_RESULT == LM_Inq.state) 
    {
        LM_Inq.state = W4_INQUIRY_CANCEL_COMPLETE;
        status = LC_Inquiry_Cancel();
    }
    return status;
}

/*************************************************************************
* FUNCTION :- LMinq_Inquiry_Event_Filter
*
* DESCRIPTION
* Filters a baseband inquiry result.
*
*************************************************************************/

boolean LMinq_Inquiry_Event_Filter(t_bd_addr* p_bd_addr,t_classDevice device_class)
{

#if (PRH_BS_CFG_SYS_INQUIRY_UNIQUE_RESULTS_SUPPORTED==1)
    if (LM_INQ_STANDARD_INQUIRY_RESULT == LMinq_Get_Inquiry_Result_Mode())
    {
        int i;
        for (i=0; (i < LM_Inq.numResp) && (i < MAX_INQUIRY_RESPONSES); i++)   
            if(LMutils_Bd_Addr_Match(p_bd_addr,&unique_inq_results[i]))   
                return FALSE;
    }
#endif

#if (PRH_BS_CFG_SYS_FILTERS_SUPPORTED == 1)
    /* Now check the inquiry results filters to see if the result should be filtered. */
    return LMfltr_Inquiry_Filter_Check(p_bd_addr,device_class);
#else
    return TRUE;
#endif

}

/*************************************************************************
* FUNCTION :- _Send_LM_Inquiry_Complete_Event
*
* DESCRIPTION
* Sends an inquiry complete event to the higher layers
*************************************************************************/

void _Send_LM_Inquiry_Complete_Event(t_error status)
{
    t_lm_event_info inq_complete_event;

    /* Generate the HCI event */
    inq_complete_event.status = status;
    inq_complete_event.number = LM_Inq.numResp;
    g_LM_config_info.lmp_event[_LM_INQUIRY_COMPLETE_EVENT] (HCI_INQUIRY_COMPLETE_EVENT,&inq_complete_event);
}

/*************************************************************************
* FUNCTION :- _Send_LM_Inquiry_Result_Event
*
* DESCRIPTION
* Sends an inquiry result event to the higher layers.
*************************************************************************/

void _Send_LM_Inquiry_Result_Event(void)
{
    t_lm_event_info inq_result_event;
    u_int8 event_code;

    t_inquiry_result_mode inquiry_result_mode = LMinq_Get_Inquiry_Result_Mode();
    
    inq_result_event.p_result_list = &latest_inq_result;
    inq_result_event.number = 1;

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_INQUIRY_RESPONSE_SUPPORTED==1)
    if ((LM_INQ_EXTENDED_INQUIRY_RESULT == inquiry_result_mode)
        && (inq_result_event.p_result_list->extension_length))
    {
        event_code = HCI_EXTENDED_INQUIRY_RESULT_EVENT;
    }
    else
#endif    
    if(LM_INQ_STANDARD_INQUIRY_RESULT != inquiry_result_mode)
    {
        event_code = HCI_INQUIRY_RESULT_EVENT_WITH_RSSI;
    }
    else
    {
        event_code = HCI_INQUIRY_RESULT_EVENT;
    }
    
    HCeg_Generate_Event(event_code, &inq_result_event);//temp remove
}

/*************************************************************************
* FUNCTION :- _Get_Interval_Rand
*
* DESCRIPTION
* Determines a random value between "max_value" and "min_value".
*************************************************************************/

u_int32 _Get_Interval_Rand(u_int16 max_value, u_int16 min_value)
{
    /*
     * Cast SYSrand_Get_Rand() from int32 to u_int32 to
     * avoid nasty compiler/platform dependent issue where
     * getting the mod of a signed number returns a positive
     * or a negative number.
     *
     * If the machine generates a negative number and the number
     * is still negative after adding min_value, the the cast of
     * this signed number to unsigned will give a v. large unsigned
     * number. When this unsigned number is added to the current
     * clock it has the potential to be a time value in the past
     * (the clock is modulo).
     */
    return (u_int32)(((u_int32)SYSrand_Get_Rand()) %((max_value+1)-(min_value))) + (min_value);
}

/*************************************************************************
 *
 * Function :- LMinq_Get_Inquiry_Mode
 *
 * Description :-
 * Returns the current inquiry mode of the device. 
 *           PRH_ONE_SHOT_INQUIRY
 *           PRH_PERIODIC_INQUIRY
 *************************************************************************/
u_int8 LMinq_Get_Inquiry_Mode(void)
{
    return LM_Inq.Inquiry_Mode;
}

#if (PRH_BS_CFG_SYS_LMP_RSSI_INQUIRY_RESULTS_SUPPORTED==1)
/*************************************************************************
 *
 * Function :- LMinq_Get_Inquiry_Result_Mode
 *
 * Description :-
 * Returns the current inquiry result mode:
 *           LM_INQ_STANDARD_INQUIRY_RESULT
 *           LM_INQ_INQUIRY_RESULT_WITH_RSSI
 *           LM_INQ_EXTENDED_INQUIRY_RESULT
 *************************************************************************/
t_inquiry_result_mode LMinq_Get_Inquiry_Result_Mode(void)
{
    return (t_inquiry_result_mode)LM_Inq.inquiry_result_mode;
}

/*************************************************************************
 *
 * Function :- LMinq_Set_Inquiry_Result_Mode
 *
 * Description :-
 * Set the current inquiry result mode:
 *           LM_INQ_STANDARD_INQUIRY_RESULT
 *           LM_INQ_INQUIRY_RESULT_WITH_RSSI
 *           LM_INQ_EXTENDED_INQUIRY_RESULT
 *************************************************************************/
void LMinq_Set_Inquiry_Result_Mode(t_inquiry_result_mode inquiry_result_mode)
{
    LM_Inq.inquiry_result_mode = inquiry_result_mode;
}

/*************************************************************************
 *
 * Function :- LMinq_Get_Interval_To_Periodic_Inq
 *
 * Returns the number of slots to the next periodic inquiry
 *************************************************************************/

t_slots LMinq_Get_Interval_To_Periodic_Inq()
{
	u_int32 clock = LC_Get_Native_Clock();
	t_slots interval = 0xFFFFFFFF;

    if (LMconfig_LM_Periodic_Inquiry_Allowed() && (LM_Inq.Inquiry_Mode == PRH_PERIODIC_INQUIRY))
	{
		interval = (BTtimer_Clock_Difference(clock,LM_Inq.next_inq_period) >> 1);
	}
	return interval;
}

#endif

