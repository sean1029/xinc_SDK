/**************************************************************************
 * MODULE NAME:    lmp_scan.c
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    Host Controller Inquiry/Page Scan entity
 * MAINTAINER:     Gary Fleming
 * CREATION DATE:  27-May-1999
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * SOURCE CONTROL: $Id: lmp_scan.c,v 1.79 2013/11/27 15:16:56 vladimirf Exp $
 *
 * NOTES TO USERS:
 *
 * DESCRIPTION
 * This object caters for the Host Control Paging Scan and
 * Inquiry Scan responsibilities. 
 * 
 * NOTE : Extra enable bit added to allow page scanning to be disabled during
 * connections or connection establishment. This bit is then only checked before
 * invoking the LC scan procedures. Thus all timers are uneffected by going into 
 * connection mode. This is more suitable for a state when all scanning is done 
 * in the LC ( either HW or SW ).
 *******************************************************************/

#include "sys_config.h"

#include "lc_interface.h"

#include "lmp_timer.h"
#include "lmp_acl_connection.h"
#include "lmp_acl_container.h"
#include "lmp_ch.h"
#include "lmp_const.h"
#include "lmp_config.h"
#include "lmp_scan.h"
#include "lmp_utils.h"
#include "lmp_inquiry.h"
#include "lmp_link_policy.h"
#include "bt_timer.h"
#include "lmp_lc_event_disp.h"
#include "sys_mmi.h"
#include "uslc_chan_ctrl.h"
#include "hc_const.h"

#if (PRH_BS_CFG_SYS_LE_DUAL_MODE==1)
#include "le_link_layer.h"
#endif

extern u_int32 g_LM_device_state;

/*
 * Scan Data Object (contains all information to manage scanning)
 */

static struct s_LM_Scan
{
   u_int8  inq_scan_timer_index;
   u_int8  page_scan_timer_index;

   u_int8  page_scan_active;   /* Flag to allow the scanning disabled/enabled */

   t_scanEnable       scan_enable;
   t_scanActivity     page_scan_act;  /* [Window,Interval] */
   t_scanActivity     inq_scan_act;   /* [Window,Interval] */
   t_lmp_link*        p_link;  /* ACL link reserved for page scaning   */

#if (PRH_BS_CFG_SYS_INQUIRY_READ_WRITE_IAC_LAP_SUPPORTED==1) 
   t_lap              inquiry_lap[PRH_BS_CFG_SYS_NUMBER_OF_SUPPORTED_IAC];
   u_int8             current_num_iac;
   u_int8             current_iac_index; /* The index of the current IAC in the IAC array */
#endif

   t_timer            next_inq_scan_time;
   t_timer            next_page_scan_time;
} LM_Scan;  

#if (PRH_BS_CFG_SYS_WRITE_SCAN_ACTIVITY_SUPPORTED==1)
static void LMscan_Update_SR_Mode(void);
#endif

static t_error LMscan_Inquiry_Scan_Complete(t_LC_Event_Info* eventInfo);
static t_error LMscan_Page_Scan_Complete(t_LC_Event_Info* eventInfo);
static void  LMscan_Sleep(void);
#define mLMscan_ValidRange(scan_range) ((scan_range >= 0x0011) && (scan_range <= 0x1000 ))

/****************************************************************
 * FUNCTION :- LMscan_Initialise
 *
 * DESCRIPTION :- This initialises the Scan Object to default values
 *
 *****************************************************************/ 
void LMscan_Initialise(void)
{

#if (PRH_BS_CFG_SYS_INQUIRY_READ_WRITE_IAC_LAP_SUPPORTED==1) 
    /*
     * The default GIAC is the default
     */
     LM_Scan.current_num_iac = 1;
     LM_Scan.inquiry_lap[0] = GIAC_LAP;
     LM_Scan.current_iac_index = 0;
#endif

    /************************************************
     * The Default in the Bluetooth Ver 1.1 spec is
     * no scans enabled (page 647)
     ************************************************/

    LM_Scan.scan_enable = NO_SCANS_ENABLED; 
    LC_Register_Event_Handler(LC_EVENT_PAGE_SCAN_INCOMING,(void*)LMscan_Page_Scan_Incoming);
    LC_Register_Event_Handler(LC_EVENT_PAGE_SCAN_TIMEOUT,(void*)LMscan_Page_Scan_Complete);
    LC_Register_Event_Handler(LC_EVENT_INQUIRY_SCAN_TIMEOUT,(void*)LMscan_Inquiry_Scan_Complete);
  

    /************************************************
     * The values below are the defaults from the 
     * Bluetooth spec Version 1.1 See Pg 650 & 654
     * These values are given in slots.
     ************************************************/

    LM_Scan.inq_scan_act.interval = DEFAULT_INQUIRY_SCAN_INTERVAL; 
    LM_Scan.page_scan_act.interval = DEFAULT_PAGE_SCAN_INTERVAL;
    LM_Scan.inq_scan_act.window = DEFAULT_INQUIRY_SCAN_WINDOW; 
    LM_Scan.page_scan_act.window = DEFAULT_PAGE_SCAN_WINDOW;
    LM_Scan.inq_scan_act.scan_type = STANDARD_SCAN; 
    LM_Scan.page_scan_act.scan_type = STANDARD_SCAN;
    LM_Scan.p_link = 0;
    LM_Scan.page_scan_active = 0;

    LM_Scan.page_scan_timer_index = LMtmr_Set_Timer(
        LM_Scan.page_scan_act.interval, 
        &LMscan_Page_Scan_Start,LM_Scan.p_link, 0);

    LM_Scan.inq_scan_timer_index = LMtmr_Set_Timer(
        LM_Scan.inq_scan_act.interval/2,
        &LMscan_Inquiry_Scan_Start,LM_Scan.p_link, 0);  
    
    LM_Scan.next_inq_scan_time = 
        BTtimer_Set_Slots((t_slots)LM_Scan.inq_scan_act.interval/2);
    LM_Scan.next_page_scan_time = 
        BTtimer_Set_Slots((t_slots)LM_Scan.page_scan_act.interval);
}


#if (PRH_BS_CFG_SYS_INQUIRY_READ_WRITE_IAC_LAP_SUPPORTED==1) 
/****************************************************************
 * FUNCTION :- LMscan_LM_Write_Supported_IAC
 *
 * DESCRIPTION :- Writes the LAPs used for inquiry scanning. 
 *
 * NOTE :- Currently only 1 IAC is supported
 *****************************************************************/
t_error LMscan_LM_Write_Supported_IAC(u_int8 num_iac, u_int8* p_iac_list)
{
    int i;
 
    for (i=0; i<num_iac && i<PRH_BS_CFG_SYS_NUMBER_OF_SUPPORTED_IAC; i++)
    {
        LM_Scan.inquiry_lap[i] = LMutils_Get_Uint24(p_iac_list);
        p_iac_list+=3;
    }
    LM_Scan.current_num_iac = i;

    return NO_ERROR;
}

/****************************************************************
 * FUNCTION :- LMscan_LM_Read_Supported_IAC
 *
 * DESCRIPTION :- Returns the number of IACs currently being used
 *                for Inquiry Scanning and pointer to the list of 
 *                IACs.
 *
 *****************************************************************/
t_error LMscan_LM_Read_Supported_IAC(u_int8* p_num_iac, u_int8* p_iac)
{
    u_int8 i;
    *p_num_iac = LM_Scan.current_num_iac;
  
    for (i=0; i < LM_Scan.current_num_iac; i++)
    {
        p_iac[0] = (u_int8) LM_Scan.inquiry_lap[i];
        p_iac[1] = (u_int8) (LM_Scan.inquiry_lap[i] >> 8);
        p_iac[2] = (u_int8) (LM_Scan.inquiry_lap[i] >> 16);
        p_iac += 3;
    }
    return NO_ERROR;
}
#endif

/************************************************************************
 *  FUNCTION :- LMscan_Read_Scan_Activity
 *
 *  DESCRIPTION :- Triggered from the coresponding HCI command this
 *                 function returns the Page/Inquiry_Scan_Activity
 *  PARAMETERS  :-
 *                 u_int8  scan_flag 
 *                 Indicates either Page or Inquiry scan activity
 *                         0x00  -- Inquiry Scan Activity
 *                         0x01  -- Page Scan Activity
 ************************************************************************/
t_scanActivity LMscan_Read_Scan_Activity(u_int8 scan_flag)
{
    return scan_flag ? LM_Scan.page_scan_act : LM_Scan.inq_scan_act;
}


#if (PRH_BS_CFG_SYS_WRITE_SCAN_ACTIVITY_SUPPORTED==1)
/************************************************************************
 * Function :- LMscan_Write_Scan_Activity
 *
 * DESCRIPTION
 * Triggered from the HCI this function sets the Page/Inquiry Scan Window and Interval. 
 *   
 * PARAMETERS  :-
 *                 u_int8  scan_flag 
 *                 Indicates either Page or Inquiry scan activity
 *                         0x00  -- Inquiry Scan Activity
 *                         0x01  -- Page Scan Activity
 * 
 * Valid Range of each field of the scan activity is 0x0012 - 0x1000
 * Continous scanning is denoted by a Interval being equal to the Window.
 *  
 * ISSUE/COMMENT
 * There is a tight coupling between these values for paging and the  SR mode in the FHS. 
 * Thus modifying page scan activity values should also result in a  modification of the
 * SRMode of the local FHS. 
 *
 * Note the default page values in the HCI spec are for R1 with window = 0x0012 and
 * interval = 0x0800. 
 *************************************************************************/
t_error LMscan_Write_Scan_Activity(u_int8 scan_flag,t_scanActivity* p_scan_act)
{
    t_error status = NO_ERROR;

    /***************************************************************
     *First Check that window <= interval and they have valid ranges 
     ***************************************************************/ 

    if((p_scan_act->window <= p_scan_act->interval) &&
        mLMscan_ValidRange(p_scan_act->interval) && 
        mLMscan_ValidRange(p_scan_act->window) &&
        (((p_scan_act->interval)&1)==0))
    {
        if (scan_flag == 0x01) /* Page Scan */
        {
           /*
           * If continuous scan Then Cancel Page Scanning and alloc link for scanning.
           */
            if (LM_Scan.page_scan_act.window == LM_Scan.page_scan_act.interval )
            {
                LC_Page_Scan_Cancel();
                if (LM_Scan.p_link)
                {
                    LMaclctr_Free_Link(LM_Scan.p_link);
                    LM_Scan.p_link = 0;
                }
            }
            LM_Scan.page_scan_act.interval = p_scan_act->interval;
            LM_Scan.page_scan_act.window = p_scan_act->window;

            if (LM_Scan.page_scan_act.window == LM_Scan.page_scan_act.interval )
            {
                if (!LM_Scan.p_link)
                    LM_Scan.p_link = LMaclctr_Alloc_Link();
            }
            /*******************************************************
             * The srMode for the FHS of the local device is updated 
             * The srMode is determined from the values of the interval
             * and window. 
             *******************************************************/
        
            LMscan_Update_SR_Mode();
        } 
        else if (scan_flag == 0x00) /* Inquiry Scan */
        {
            if (LM_Scan.inq_scan_act.interval == LM_Scan.inq_scan_act.window)
            {
                LC_Inquiry_Scan_Cancel();
            }
            LM_Scan.inq_scan_act.interval = p_scan_act->interval;
            LM_Scan.inq_scan_act.window = p_scan_act->window;
        }
    } 
    else 
    {
        status = INVALID_HCI_PARAMETERS;
    }
    return status;
}
#endif

/************************************************************************
 *  Function :- LMscan_Write_Scan_Enable
 *
 *  DESCRIPTION :- Modifies the scan enable flag.
 *
 ************************************************************************/
t_error LMscan_Write_Scan_Enable(t_scanEnable scanEnable)
{
    t_error status = NO_ERROR;

    if(LMconfig_LM_Enforce_Scans_Disabled() && (scanEnable != NO_SCANS_ENABLED))
    {
        return COMMAND_DISALLOWED;
    }


    /***********************************
     * Check if valid Scan Enable Value
     ***********************************/
    if (scanEnable <= BOTH_SCANS_ENABLED)
    {
        /********************************************
         * Return Scan to Known State NO_SCAN_ENABLED
         ********************************************/  

        LC_Page_Scan_Cancel();
        LC_Inquiry_Scan_Cancel();

        if (LM_Scan.p_link)
        {
            LMaclctr_Free_Link(LM_Scan.p_link);
            LM_Scan.p_link = 0; 
        }

        LM_Scan.scan_enable = scanEnable;

        switch(scanEnable)
        {
        case PAGE_SCAN_ONLY_ENABLED :
            LMscan_Page_Scan_Start(0 /* Dummy Value */);
            break;
        
        case INQUIRY_SCAN_ONLY_ENABLED :
            LMscan_Inquiry_Scan_Start(0 /* Dummy Value */);
            break;

        case BOTH_SCANS_ENABLED :
            LMscan_Page_Scan_Start(0 /* Dummy Value */);
            //LMscan_Inquiry_Scan_Start(0 /* Dummy Value */);
            // offset inq scan window from page scan window - TK 17/06/2013
            LMtmr_Reset_Timer( LM_Scan.inq_scan_timer_index, LM_Scan.page_scan_act.interval/2);
            break;     
        default :
            break;
        }
    }
    else
    {
        status = INVALID_HCI_PARAMETERS;
    }
    return status;
}

/************************************************************************
 *  Function :- LMscan_Read_Scan_Enable
 *
 *  DESCRIPTION :- Reads the scan enable flag.
 *
 ************************************************************************/
t_scanEnable LMscan_Read_Scan_Enable(void)
{
   if(LMconfig_LM_Enforce_Scans_Disabled())
       return NO_SCANS_ENABLED;
   else
       return LM_Scan.scan_enable;
}

#if (PRH_BS_CFG_SYS_LMP_INTERLACED_INQUIRY_SCAN_SUPPORTED==1)
/************************************************************************
 *  Function :- LMscan_Write_Inquiry_Scan_Type
 *
 *  DESCRIPTION :- Writes the Inquiry Scan Type: standard or interlaced.
 *
 ************************************************************************/
t_error LMscan_Write_Inquiry_Scan_Type(t_scan_type scan_type)
{
    t_error status = NO_ERROR;

	if ( (STANDARD_SCAN==scan_type) || (INTERLACED_SCAN==scan_type) )
	{
        LM_Scan.inq_scan_act.scan_type = scan_type;
	}
	else
	{
        status = INVALID_HCI_PARAMETERS;
	}
		
	return status;
}

/************************************************************************
 *  Function :- LMscan_Read_Inquiry_Scan_Type
 *
 *  DESCRIPTION :- Reads the current Inquiry Scan Type:
 *                        standard or interlaced.
 *
 ************************************************************************/
t_scan_type LMscan_Read_Inquiry_Scan_Type(void)
{
	return LM_Scan.inq_scan_act.scan_type;
}

#endif

#if (PRH_BS_CFG_SYS_LMP_INTERLACED_PAGE_SCAN_SUPPORTED==1)
/************************************************************************
 *  Function :- LMscan_Write_Page_Scan_Type
 *
 *  DESCRIPTION :- Writes the Page Scan Type: standard or interlaced.
 *
 ************************************************************************/
t_error LMscan_Write_Page_Scan_Type(t_scan_type scan_type)
{
    t_error status = NO_ERROR;

	if ( (STANDARD_SCAN==scan_type) || (INTERLACED_SCAN==scan_type) )
	{
        LM_Scan.page_scan_act.scan_type = scan_type;
	}
	else
	{
        status = INVALID_HCI_PARAMETERS;
	}
	return status;
}

/************************************************************************
 *  Function :- LMscan_Write_Page_Scan_Type
 *
 *  DESCRIPTION :- Writes the Page Scan Type: standard or interlaced.
 *
 ************************************************************************/
t_scan_type LMscan_Read_Page_Scan_Type(void)
{
	return LM_Scan.page_scan_act.scan_type;
}
#endif

/************************************************************************
 *  Function :- LMscan_Page_Scan_Start
 *
 *  DESCRIPTION :- Initiates page scaning 
 *
 ************************************************************************/
void  LMscan_Page_Scan_Start(t_lmp_link* p_link)
{  

    volatile u_int32       current_clk;
	t_scan_type scan_type;
    current_clk = (LC_Get_Native_Clock() & 0x0FFFFFFC) + 4;  /*Next Tx */

    /****************************************
     * Check if Page Scan is enabled 
     * Note if page scan enable flag is not set  
     * then page scanning cannot be performed. 
     * ( pg 625 of draft 1.0 )
     ***************************************/

    if((PAGE_SCAN_ONLY_ENABLED == LM_Scan.scan_enable) ||
        (BOTH_SCANS_ENABLED == LM_Scan.scan_enable)) 
    {  
        // If there is not enough time to the next_wake_time to perform the Scan then
		// Delay it by X slots. Need to consider what to do when the LC is already performing
		// a WakeUp (Sniff Window).. 
		
		if (!(USLCchac_get_device_state() & Inquiry_Scan) && ((g_LM_config_info.links_in_low_power == 0) ||
			(g_LM_config_info.links_in_low_power && 
            !LMpol_Is_Wakeup_Interval(LM_Scan.page_scan_act.window+LMP_SLOT_GAURD_FOR_CHANGE_PICONET) &&
		    !(g_LM_device_state & (PRH_DEV_STATE_SNIFF_WINDOW_ACTIVE|PRH_DEV_STATE_WAKEUP_IMMINENT))
			    )
			))
		{
			
			LMtmr_Reset_Timer(LM_Scan.page_scan_timer_index, 
				LM_Scan.page_scan_act.interval);

			LM_Scan.next_page_scan_time=BTtimer_Set_Slots((t_slots)(LM_Scan.page_scan_act.interval));
			
			/*******************************************************************
			* Invoke the lower layer page scanning if page scanning is allowed
			* and not already active (i.e pending a completion callback from the LC.
			**********************************************************************/
			if (LMconfig_LM_Page_Scans_Allowed() && !LM_Scan.page_scan_active && !DL_Max_Piconets())
			{
#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
				if (!g_LM_config_info.num_links_parked || ((g_LM_config_info.park_info.park_state == LMP_BEACON_INACTIVE)
					&& !BTtimer_Is_Expired_For_Time(g_LM_config_info.beacon_instant, current_clk+12)))
#endif                
				{
					if((LM_Scan.page_scan_act.window != LM_Scan.page_scan_act.interval) ||
						(LM_Scan.p_link == 0))
						LM_Scan.p_link = LMaclctr_Alloc_Link();
					
					if(LM_Scan.p_link)
					{
						LM_Scan.page_scan_active = 0x01;                        
						if (R0==LC_Read_Local_Page_Scan_Repetition())
						{
							scan_type=CONTINUOUS_SCAN;
						}
						else
						{
							scan_type=LM_Scan.page_scan_act.scan_type;
						}
						LC_Page_Scan_Request(LM_Scan.p_link->device_index, LM_Scan.page_scan_act.window, scan_type); 
					}
					
				}  
			}
		}
		else
		{
			LMtmr_Reset_Timer(LM_Scan.page_scan_timer_index, 
				LMP_SLOT_GAURD_FOR_CHANGE_PICONET+LM_Scan.page_scan_act.window);

			LM_Scan.next_page_scan_time=BTtimer_Set_Slots((t_slots)(LMP_SLOT_GAURD_FOR_CHANGE_PICONET+LM_Scan.page_scan_act.window));
		}
		
    }
	
}


/************************************************************************
 *  Function :- LMscan_Inquiry_Scan_Start
 *
 *  DESCRIPTION :- Initiates Inquiry scaning   
 *
 ************************************************************************/
void LMscan_Inquiry_Scan_Start(t_lmp_link* p_link)
{
    t_lap iac;
    t_scan_type scan_type;
    volatile u_int32       current_clk;
    current_clk = (LC_Get_Native_Clock() & 0x0FFFFFFC) + 4;  /*Next Tx */


	if (!(USLCchac_get_device_state() & Page_Scan) && ((g_LM_config_info.links_in_low_power == 0) ||
		(g_LM_config_info.links_in_low_power && 
		(
			!LMpol_Is_Wakeup_Interval(LM_Scan.inq_scan_act.window+LMP_SLOT_GAURD_FOR_CHANGE_PICONET)) &&
			 !(g_LM_device_state &(PRH_DEV_STATE_SNIFF_WINDOW_ACTIVE|PRH_DEV_STATE_WAKEUP_IMMINENT))
		) ) )
	{
		/*
		* First, select IAC 
		* (sequential rotation if multiple IACs supported)
		*/
#if (PRH_BS_CFG_SYS_INQUIRY_READ_WRITE_IAC_LAP_SUPPORTED==1)
		iac = LM_Scan.inquiry_lap[LM_Scan.current_iac_index];

		if (++LM_Scan.current_iac_index == LM_Scan.current_num_iac)
		{
			LM_Scan.current_iac_index = 0;
	    }
#else
		iac = GIAC_LAP;
#endif


		if((INQUIRY_SCAN_ONLY_ENABLED == LM_Scan.scan_enable) ||
			(BOTH_SCANS_ENABLED == LM_Scan.scan_enable)) 
		{
		    LMtmr_Reset_Timer(LM_Scan.inq_scan_timer_index,LM_Scan.inq_scan_act.interval);
	        LM_Scan.next_inq_scan_time= 
				BTtimer_Set_Slots((t_slots)(LM_Scan.inq_scan_act.interval));
        /*********************************
         * Invoke the LC inquiry scanning 
         *********************************/

			if (LMconfig_LM_Inquiry_Scans_Allowed())
			{
#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
			    if (!g_LM_config_info.num_links_parked || ((g_LM_config_info.park_info.park_state == LMP_BEACON_INACTIVE)
				    && !BTtimer_Is_Expired_For_Time(g_LM_config_info.beacon_instant, current_clk+12)))
#endif
				{
					if (LM_Scan.inq_scan_act.interval == LM_Scan.inq_scan_act.window)
					{
						scan_type = CONTINUOUS_SCAN; 
					}
					else
					{
						scan_type = LM_Scan.inq_scan_act.scan_type;
					}
					LC_Inquiry_Scan_Request(iac, LM_Scan.inq_scan_act.window, scan_type); 
				}
			}
		}
	}
	else
	{
		if((INQUIRY_SCAN_ONLY_ENABLED == LM_Scan.scan_enable) ||
			(BOTH_SCANS_ENABLED == LM_Scan.scan_enable)) 
		{

			LMtmr_Reset_Timer(LM_Scan.inq_scan_timer_index, LMP_SLOT_GAURD_FOR_CHANGE_PICONET+LM_Scan.inq_scan_act.window);
			LM_Scan.next_inq_scan_time=BTtimer_Set_Slots((t_slots)(LMP_SLOT_GAURD_FOR_CHANGE_PICONET+LM_Scan.inq_scan_act.window));
		}
		
	}
}

/************************************************************************
 *  Function :- LMscan_Page_Scan_Incoming
 *
 *  DESCRIPTION :- Triggered by the baseband when a page has been received.   
 *  The Bluetooth Address ( Bd_Addr) and the device class are extracted out
 *  of the the device link and the LM Connection handler is called.
 *
 ************************************************************************/
t_error LMscan_Page_Scan_Incoming(t_LC_Event_Info* eventInfo)
{
    t_error status = NO_ERROR;
    t_classDevice device_class;

    /**********************************************
     * Inform the LMP CH of an incoming connection 
     **********************************************/

    FHS_Get_Bd_Addr_Ex(eventInfo->fhs_packet, &LM_Scan.p_link->bd_addr);
    device_class = FHS_Get_Device_Class(eventInfo->fhs_packet);
    LM_Scan.page_scan_active = 0x0;
    LMconnection_LM_Incoming_Link_Inf(LM_Scan.p_link->device_index,
                                       &LM_Scan.p_link->bd_addr, device_class);
    LM_Scan.p_link = 0;

    return status;
}


/************************************************************************
 *  Function :- LMscan_Update_SR_Mode
 *
 *  Description 
 *  Uses the values of the Window and Interval for paging to determine 
 *  the SR mode of the local device - and writes the value to the LC.
 *
 *  SR mode     T page scan
 *  -------     -----------
 *  R0          continuous
 *  R1          <= 1.28s
 *  R2          <= 2.56s
 *
 ************************************************************************/
#if (PRH_BS_CFG_SYS_WRITE_SCAN_ACTIVITY_SUPPORTED==1)
static void LMscan_Update_SR_Mode(void)
{
    t_pageScanRepit sr_mode;

    if (LM_Scan.page_scan_act.window == LM_Scan.page_scan_act.interval) 
    {
        sr_mode = R0;
    } 
    else if (LM_Scan.page_scan_act.interval <=  MAX_Tpagescan_FOR_R1) 
    { 
        sr_mode = R1;
    } 
    else /*if (LM_Scan.page_scan_act.interval <=  MAX_Tpagescan_FOR_R2)*/
    {
        sr_mode = R2;
    } 

    /*
     * Inform the Link Controller of the SR mode 
     */
    LC_Write_Local_Page_Scan_Repetition(sr_mode);
}
#endif
    
/************************************************************************
 *  Function :- LMscan_Page_Scan_Complete
 *
 *  DESCRIPTION :- Triggered by the baseband when a page scan has been 
 *                 completed.
 *
 ************************************************************************/
  
  t_error LMscan_Page_Scan_Complete(t_LC_Event_Info* eventInfo)
  {
    LM_Scan.page_scan_active = 0x0;
    if (LM_Scan.page_scan_act.window != LM_Scan.page_scan_act.interval)
    {
        LMaclctr_Free_Link(LM_Scan.p_link);
        LM_Scan.p_link = 0;
    }
    LM_Scan.next_page_scan_time=BTtimer_Set_Slots((t_slots)(LM_Scan.page_scan_act.interval - LM_Scan.page_scan_act.window));

    LMscan_Sleep();

    return NO_ERROR;
  }

  /************************************************************************
   *  Function :- LMscan_Inquiry_Scan_Complete
   *
   *  DESCRIPTION :- Triggered by the baseband when a inquiry scan has been 
   *                 completed.
   *
   ************************************************************************/
  
t_error LMscan_Inquiry_Scan_Complete(t_LC_Event_Info* eventInfo)
{
    LM_Scan.next_inq_scan_time = BTtimer_Set_Slots((t_slots)(LM_Scan.inq_scan_act.interval - LM_Scan.inq_scan_act.window));
    LMscan_Sleep();
    return NO_ERROR;
}
  
/****************************************************************************
 * Function :- LMscan_Sleep
 *
 * DESCRIPTION :-
 * Determines the inverval for which LC sleep should be called and invokes it 
 * in the LC.
 ***************************************************************************/
  
void LMscan_Sleep(void)
{
#if (PRH_BS_CFG_SYS_SLEEP_IN_STANDBY_SUPPORTED == 1)
    u_int8 dev_index= 0; //SYSconfig_Get_Local_Device_Index();
    u_int32 interval=0;
    t_clock next_instant;
	
    if (((g_LM_config_info.num_acl_links == 0) || (g_LM_config_info.num_acl_links == g_LM_config_info.links_in_low_power)) &&
		LC_Is_Sleep_Possible(&next_instant))
    { 
		
		{
			interval = LMscan_Get_Interval_To_Next_Scan(next_instant);
			
			if (g_LM_config_info.num_acl_links && 
				(g_LM_config_info.num_acl_links == g_LM_config_info.links_in_low_power))
			{
				t_slots slots_to_next_acl_activity = LMpol_Get_Interval_To_Next_Wakeup();

				if (interval > slots_to_next_acl_activity)
				{
					interval = slots_to_next_acl_activity;
					dev_index = g_LM_config_info.next_wakeup_link->device_index;	
				}
			}	
			// Consider Periodic Inquiry... 
			{
				t_slots slots_to_periodic_inq = LMinq_Get_Interval_To_Periodic_Inq();
				
				if (interval > slots_to_periodic_inq)
				{
					interval = slots_to_periodic_inq; 
					dev_index = SYSconfig_Get_Local_Device_Index();
				}
				
			}	

#if (PRH_BS_CFG_SYS_LE_DUAL_MODE==1)
			{
				t_slots le_sleep_slots = LE_LL_InactiveSlots();
				if (le_sleep_slots < interval)
				{
					interval = le_sleep_slots;
					dev_index = 0;
				}
			}
#endif
			/* Determine the sleep time.  */
			if ((interval > SYS_CFG_MIN_SLOTS_FOR_SLEEP_PROCEDURE) && (interval != 0xFFFFFFFF))
			{ 
				LC_Sleep(interval - SYS_CFG_MIN_SLOTS_FOR_SLEEP_PROCEDURE , dev_index);
				SYSmmi_Display_Event(eSYSmmi_LC_Disconnection_Event);
			}		

		}
	}	
#endif
}

	

t_slots LMscan_Get_Interval_To_Next_Scan(t_clock next_instant)
{
    u_int32 clock = LC_Get_Native_Clock();
    u_int32 interval=0;

/* If the time interval to the next scan is large enough then sleep the device
	* Determine which if the Scans is nearest in time. */


	// As Scans should be disabled when we are in scatternet (2 piconets only supported for now)
	// we return an
	if (LMconfig_LM_Connected_As_Scatternet())
	{
 		return 0xFFFFFFFF;
	}

	if (LM_Scan.scan_enable == 0x03)/* Both Scans Enabled */
	{
		
		if (( LM_Scan.next_page_scan_time >= LM_Scan.next_inq_scan_time ) ||
			( !LMconfig_LM_Page_Scans_Allowed() && LMconfig_LM_Inquiry_Scans_Allowed()))
		{
			/* If next inquiry scan time not passed */
			if (!BTtimer_Is_Expired(LM_Scan.next_inq_scan_time))
			{
                /* Sleep to the next inquiry scan Time */
                interval = (BTtimer_Clock_Difference(clock,LM_Scan.next_inq_scan_time) >> 1);
			}    
		}
		else if (( LM_Scan.next_inq_scan_time > LM_Scan.next_page_scan_time ) ||
			( !LMconfig_LM_Inquiry_Scans_Allowed() && LMconfig_LM_Page_Scans_Allowed()))
		{
			/* If next inquiry scan time not passed */
			if (!BTtimer_Is_Expired(LM_Scan.next_page_scan_time))
			{
                /* Sleep to the next page scan time */
                interval = (BTtimer_Clock_Difference(clock,LM_Scan.next_page_scan_time) >> 1);
			}
		}    
		else
		{
			return 0xFFFFFFFF;
		}
	}
	else if ((LM_Scan.scan_enable == 0x02) && LMconfig_LM_Page_Scans_Allowed())
	{
		interval = ((LM_Scan.next_page_scan_time - clock) >> 1);
	}
	else if ((LM_Scan.scan_enable == 0x01) && LMconfig_LM_Inquiry_Scans_Allowed())
	{
		interval =  ((LM_Scan.next_inq_scan_time - clock) >> 1);
		
	}
	else
		return 0xFFFFFFFF;
	
	if ((next_instant) && (interval > (BTtimer_Clock_Difference(clock, next_instant) >> 1)))
		interval = (BTtimer_Clock_Difference(clock, next_instant) >> 1);

	return interval;
}
