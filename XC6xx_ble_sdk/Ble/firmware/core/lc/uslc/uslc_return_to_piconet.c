/***********************************************************************
 *
 * MODULE NAME:    uslc_return_to_piconet.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    LC Portion of Return to Piconet Procedure
 * MAINTAINER:     Ivan Griffin
 * CREATION DATE:  04 September 2000
 *
 * SOURCE CONTROL: $Id: uslc_return_to_piconet.c,v 1.100 2013/11/21 14:18:00 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *    04 Sept 2000 - IG  - Initial LC State Machine for Return From Hold
 *    02 Oct  2000 - IG  - Verified on the boards
 *
 * ISSUES:
 *    Is DWH correct for first pkt received on return to piconet?
 *    need to wait a state to ensure next pkt has correct header -
 *    need interrupt  uslc_chan_ctrl.c for this to generate NULL resp
 *    need to call LSLCfreq_Realign_BtClk_To_Piconet();
 *    
 ***********************************************************************/
#include "sys_config.h"

#define USLCR2P_ALLOW_MULTI_SLOT_R2P               1
#define USLCR2P_ALLOW_HISTORY_WIDENING_WINDOW      (0 && \
		(USLCR2P_ALLOW_MULTI_SLOT_R2P==1))

#if (PRH_BS_CFG_SYS_RETURN_TO_PICONET_SUPPORTED==1)

/***********************************************************************
 *
 * Module Includes
 *
 ***********************************************************************/
#include "sys_power.h"

#include "lc_types.h"
#include "lc_interface.h"
#include "lc_log.h"

#include "bt_timer.h"
#include "bt_test.h"

#include "hw_delay.h"
#include "hw_leds.h"
#include "hw_radio.h"

#include "dl_dev.h"

#include "lslc_pkt.h"
#include "lslc_slot.h"
#include "lslc_hop.h"
#include "lslc_clk.h"

#include "uslc_chan_ctrl.h"
#include "hc_event_gen.h"

#include "tra_hcit.h"

#define USLC_R2P_MAX_SEARCH_WIN_US 170 /* max 170us WW in TX_RX mode, else FULL_RX mode */

/***********************************************************************
 *
 * Module Typedefs
 *
 ***********************************************************************/

typedef enum 
{
    OS_INACTIVE,
    OS_START,
    OS_RECONNECTING,
    OS_SENTINEL /* NOT A VALID STATE */
} t_USLCr2p_Ctrl_State;


typedef struct
{
    t_USLCr2p_Ctrl_State ctrl_state;
    t_devicelink *p_active_devicelink;

    t_clock r2p_instant;
    t_slots r2p_timeout;
    t_slots r2p_timer;

    t_clock rx_freq_bt_clock;
    /*
     * Counter for the number of remaining frames in this search window
     */
    u_int8 remaining_search_win_slots;
    u_int32 search_win_us;

} t_USLCr2p_Private_Data;

static t_USLCr2p_Private_Data _r2p_info;

typedef void (*t_USLCr2p_State_Fn_Ptr)(t_USLCr2p_Private_Data *);

static u_int8 _r2p_min_search_window = 1;

extern t_SYS_Config g_sys_config;

#if (PRH_BS_CFG_TABASCO_VERSION >= PRH_BS_HW_TABASCO_VERSION_T1_0_0) && (PRH_BS_DEV_USE_T1_BT_CLOCK==1)
/*
 *  This flag is used to instruct lslc_slot.c to freeze the BT clock on the next TIM2 interrupt.
 *  We want the BT clock freeze to occur on a TIM2 interrupt so that the value of the frozen
 *  BT clock matches a value where the slave is supposed to receive a packet from the master.
 *  The freezing of the BT clock must occur on a TIM interrupt because the two least significant
 *  bits of the BT clock cannot be modified by writing the BT clock offset.
 *  The value of the frozen BT clock is used by the decryption engine.
 */
boolean freeze_bt_clk_on_next_tim2 = FALSE;
#endif

/***********************************************************************
 *
 * state machine prototypes
 *
 ***********************************************************************/
static void USLCr2p_State_OS_START_(t_USLCr2p_Private_Data *);
static void USLCr2p_State_OS_RECONNECTING_(t_USLCr2p_Private_Data *);

/*
 * state jump table - this ensures that once state
 * decision is made in previous state, next time
 * state is reached quickly - avoids big dirty switch!
 */

t_USLCr2p_State_Fn_Ptr const uslc_r2p_state_jump_table_[] = 
{
     0, /* OS_INACTIVE */
     
     USLCr2p_State_OS_START_, /* Common State */
     USLCr2p_State_OS_RECONNECTING_,
     0 /* OS_SENTINEL */
};


/***********************************************************************
 ***********************************************************************
 **
 ** Module Public Functions
 **
 ***********************************************************************
 ***********************************************************************/


/***********************************************************************
 *
 * FUNCTION:  USLCr2p_Initialise()
 * PURPOSE:   Initialise the USLC return to piconet procedure
 *
 ***********************************************************************/
void USLCr2p_Initialise(void)
{
    _r2p_info.ctrl_state = OS_INACTIVE;
}

/***********************************************************************
 *
 * FUNCTION:  USLCr2p_Get_Local_Clock_Drift()
 * PURPOSE:   Get the ppm drift of the local clock - LF OSC dependent
 *
 ***********************************************************************/
u_int8 USLCr2p_Get_Local_Clock_Drift(void)
{
#if (PRH_BS_CFG_SYS_LOW_POWER_MODE_SUPPORTED==1)
	return (SYSpwr_LowFrequencyOscillatorAvailable()==SYS_LF_OSC_NONE)?
		(20/*20ppm*/):(SYSconfig_Get_Clock_Drift()/*250ppm*/);
#else
	return 20 /*20ppm*/;
#endif
}

/***********************************************************************
 *
 * FUNCTION:  USLCr2p_Request()
 * PURPOSE:   Request the execution of return to piconet
 *
 * device_index    LC device index for the connection to become active
 * timeout         specifies a timeout value - should be zero for
 *                 Return from Hold, and a number of slots for Sniff etc.
 * instance        The instance when the slave will open its first
 *                 window to return to piconet.
 *
 * Preconditions
 *     Must be slave with active connection to a Master
 ***********************************************************************/
t_error USLCr2p_Request(
    t_deviceIndex device_index, t_slots timeout, t_clock instance)
{
    t_error status = NO_ERROR;
    t_USLCr2p_Private_Data *p_r2p_info = &_r2p_info;
	t_clock last_access;
    
	last_access = DL_Get_Piconet_Clock_Last_Access(DL_Get_Device_Ref(device_index));

    LC_Log_Procedure_Request(R2P_PROCEDURE);

    if ((0 == device_index) || 
        (SYSconfig_Get_Max_Active_Devices() <= device_index))
    {
        HCeg_Hardware_Error_Event(PRH_BS_HW_ERROR_INVALID_DEVICE_LINK);
        status = UNSPECIFIED_ERROR;
    }
    /*
     * If we are not Sleep for Short time - no need to make R2P
     * 0x40 - magic number chosen by LM
     */
    else if (BTtimer_Is_Expired_For_Time(instance, (last_access + (PRH_BS_DEV_MIN_TIME_SINCE_CONTACT_FOR_R2P * 2 + 1)) ))
    {
        status = NO_ERROR;
    }
    else if (OS_INACTIVE == p_r2p_info->ctrl_state)
    {
    	USLCchac_Procedure_Request(R2P_PROCEDURE, TRUE);
    	p_r2p_info->ctrl_state = OS_START;
    	p_r2p_info->r2p_timeout = timeout;
    	p_r2p_info->r2p_instant = instance & 0x0FFFFFFC;
    	p_r2p_info->p_active_devicelink = DL_Get_Device_Ref(device_index);

    	LC_Log_Procedure_Accept(R2P_PROCEDURE);
   		status = NO_ERROR;
    }

    return status;
}

/***********************************************************************
 *
 * FUNCTION:  USLCr2p_Cancel()
 * PURPOSE:   Cancel any active R2P procedure
 *
 ***********************************************************************/
t_error USLCr2p_Cancel(void)
{
    LC_Log_Procedure_Cancel(R2P_PROCEDURE);

    _r2p_info.ctrl_state = OS_INACTIVE; 

	LSLCacc_Disable_Receiver();

    return USLCchac_Procedure_Request(R2P_PROCEDURE, FALSE);
}

/***********************************************************************
 *
 * FUNCTION:  USLCr2p_State_Dispatcher()
 * PURPOSE:   To invoke the event handler associated with a particular
 *            state. Note: This SM is called on a slot-by-slot basis.
 *
 * dummy      unused parameter for consistency in all state machines
 ***********************************************************************/
void USLCr2p_State_Dispatcher(boolean dummy)
{
    t_USLCr2p_Private_Data *p_r2p_info = (t_USLCr2p_Private_Data*)&_r2p_info;
    int state_index = (int)(p_r2p_info->ctrl_state);
    t_devicelink* p_active_devicelink = p_r2p_info->p_active_devicelink;

    if (NOT_CONNECTED == DL_Get_Ctrl_State(p_active_devicelink))
    {
        USLCr2p_Cancel();
        LSLChop_Set_HSE_Engine(LSLChop_HSE_DEFAULT);
        /* if we disconnected then no need to keep history */
#if (USLCR2P_ALLOW_HISTORY_WIDENING_WINDOW==1)
		p_active_devicelink->prev_R2P_failed = 0;
#endif
	}

    else if ((OS_INACTIVE >= state_index) || (OS_SENTINEL <= state_index))
    {
        /* if we exit procedure then no need to keep history */
#if (USLCR2P_ALLOW_HISTORY_WIDENING_WINDOW==1)
	    p_active_devicelink->prev_R2P_failed = 0;
#endif
        /* 
         * Invalid state: Resume previous super state 
         */
        USLCchac_Procedure_Finish(FALSE);
        USLCchac_Set_Device_State(Connection);
        HCeg_Hardware_Error_Event(PRH_BS_HW_ERROR_INCORRECT_CTRLSTATE);
        LSLChop_Set_HSE_Engine(LSLChop_HSE_DEFAULT);
    }
    else
    {
        /* 
         * Jump to function handling current state 
         */
        (uslc_r2p_state_jump_table_[state_index])(p_r2p_info);
    }
}

#if 0 // (PRH_BS_CFG_SYS_HW_WINDOW_WIDENING_SUPPORTED==1)
/***********************************************************************
 *
 * FUNCTION:  USLCr2p_State_Dispatcher_TIM3_OS_START
 * PURPOSE:   Invoke USLCr2p on TIM3 instead of TIM0 for next RX allows
 *            more time for setup in HW window widening scenarios.
 *
 *            Comments: This seemed operational in that it configured
 *            the correct SPI frequencies earlier but did not improve
 *            the situation with max USLC_R2P_MAX_SEARCH_WIN_US vs its
 *            configuration on TIM0. As SPIs are set up sufficiently
 *            early for larger WW values using this function, I suspect
 *            the issue lies elsewhere - e.g. due to WW adjusted GIOs.
 *
 ***********************************************************************/
void USLCr2p_State_Dispatcher_TIM3_OS_START()
{
    t_USLCr2p_Private_Data *p_r2p_info = (t_USLCr2p_Private_Data*)&_r2p_info;

    if(p_r2p_info->ctrl_state == OS_START)
    {
        t_devicelink *p_device_link = p_r2p_info->p_active_devicelink;

        u_int8 search_win_frames = USLCr2p_Get_Search_Window_Frames(p_device_link, 0);

        if (search_win_frames==1)
        {
        	u_int8 clock_offset_for_freq = ((search_win_frames|1)<<1);

        	t_clock next_bt_clk = HW_Get_Bt_Clk_Avoid_Race()+1; /*HW_get_bt_clk();*/
        	t_clock rx_freq_bt_clk = (next_bt_clk + clock_offset_for_freq) & 0x0FFFFFFC;

        	if (rx_freq_bt_clk >= p_r2p_info->r2p_instant)
        	{ /* note if this code is used, the equivalent code for WW in the
               USLCr2p_State_OS_START_ does not need to be and should not be called */

                /*LSLChop_Set_HSE_Engine(LSLChop_HSE_SOFTWARE);*/

                HW_set_win_ext(_r2p_info.search_win_us/2);
    	        g_sys_config.win_ext = (_r2p_info.search_win_us)/2; /* to restore default on sync det */

    	        HWradio_SetRadioMode(RADIO_MODE_TX_RX);
    	        LSLCpkt_Prepare_Receiver_WinExt_Return_To_Piconet(p_device_link, rx_freq_bt_clk);
        	}
        }
    }
}
#endif

/***********************************************************************
 ***********************************************************************
 **
 ** Module State Machines
 **
 ***********************************************************************
 ***********************************************************************/

/***********************************************************************
 *
 * FUNCTION:  USLCr2p_Get_Search_Window_Frames()
 * PURPOSE:   To determine how many frames to search for next frequency
 *
 *    max_drift_us = (drift_master + my_max_drift)*time_since_last_piconet_access
 *    search_win_us = 2*max_drift_us
 *    search_win_frames = (scan_win_us+2*R2P_RADIO_SETUP_TIME)/1250+1
 *
 * The number of frames is currently restricted to odd 1, 3, 5,....
 * 1. Since this means symmetric windows starting around TIM2
 * 2. Likely to ensure relatively prime with polling period.
 *
 * u_int8 is adequate since max drift is 40secs by max drift ~1 frame/sec
 ***********************************************************************/
u_int8 USLCr2p_Get_Search_Window_Frames(
    const t_devicelink *p_device_link,    /* i - ref to device link    */
    t_slots r2p_offset_ticks              /* i - offset to r2p instant */
)
{
    u_int32 search_win_us;
    u_int32 search_win_frames;

#if (BUILD_TYPE!=UNIT_TEST_BUILD) // GF 29 March 2011
    //
    // This is important - to ensure that we have a Zero Frame Search window
    // when we have only a very small interval since the last activity on a
    // link. This corresponds with the check in USLCr2p_Request where we dont begin
    // an R2P procedure if we have recently has contact on a device index.
    //
    // The min values is PRH_BS_DEV_MIN_TIME_SINCE_CONTACT_FOR_R2P  = 0x40 (64) Slots.
	if (BTtimer_Is_Expired_For_Time(LC_Get_Piconet_Clock(p_device_link->device_index),
                        (DL_Get_Piconet_Clock_Last_Access(p_device_link) + (PRH_BS_DEV_MIN_TIME_SINCE_CONTACT_FOR_R2P * 2 + 1)) ))
	{
        return 0;
	}
#endif

    /*
     * Minimum Search Window is 1 frame (or Part of 1 frame if WW)
     */
    search_win_frames = _r2p_min_search_window;

    search_win_us = (DL_Get_Clock_Drift(p_device_link) + USLCr2p_Get_Local_Clock_Drift())
           * ((LC_Get_Piconet_Clock(p_device_link->device_index) + r2p_offset_ticks -  DL_Get_Piconet_Clock_Last_Access(p_device_link))
               & 0x0FFFFFFF)/ 1600 /* /3200 * 2*/;

    _r2p_info.search_win_us = search_win_us;

	/*
     * If not forced SINGLE SLOT R2P then determine window extension
     */
    if ( !SYSconfig_Is_Feature_Supported(
              PRH_BS_CFG_SYS_FORCE_SINGLE_SLOT_R2P_FEATURE) )
    {
        search_win_frames += (search_win_us+2*LC_Get_Radio_Setup_Time_Us())/1250;

#if (USLCR2P_ALLOW_HISTORY_WIDENING_WINDOW==1)
       /* correction if prev R2P failed */
    	if (p_device_link->prev_R2P_failed)
		{
			search_win_frames += p_device_link->not_received_correction * search_win_frames;
		}
#endif

        /*
         * Ensure odd number - No longer necessary as SM is called on a slot-by-slot basis.
         */
        //search_win_frames |= 0x01;
        /*
         * Limit max search frame to 40 seconds drift
         */
        if (search_win_frames > 40)
        {
            search_win_frames = 40;
        }
    }

    return search_win_frames;
}
 
/***********************************************************************
 *
 * FUNCTION:  USLCr2p_State_OS_START_()
 * PURPOSE:   Start the return to piconet.  
 *
 * The receiver is directly setup below
 *
 * Start the Return to Piconet when the Rx Window should start
 * Otherwise stay in this state.
 * 
 *   +---+   +---+   +---+   +---+   +---+   +---+
 *   |Tx |Rx |Tx |Rx |Tx |Rx |Tx |Rx |Tx |Rx |Tx |
 * --+   +---+   +---+   +---+   +---+   +---+   +--
 *                   <-----------I---------------> 
 * Start of Rx Window^           ^R2P Instant (freq Master Tx)
 *
 * This state is entered on each TIM0 (slave TX_START)
 *
 * To ensure that the Rx Window is centred on the expected R2P
 * instant the R2P Request must be called at least 
 * 0.5 of Rx Window length in advance.
 ***********************************************************************/
static void USLCr2p_State_OS_START_(t_USLCr2p_Private_Data *p_r2p_info)
{
    t_devicelink *p_device_link = p_r2p_info->p_active_devicelink;
    u_int8  search_win_frames;
    u_int8  clock_offset_for_freq;
    t_clock rx_freq_bt_clk;
    t_clock cur_bt_clk;
    t_clock r2p_instant;

    search_win_frames = USLCr2p_Get_Search_Window_Frames(p_device_link, 0);
    clock_offset_for_freq = ((search_win_frames|1)<<1);

    LSLChop_Set_HSE_Engine(LSLChop_HSE_SOFTWARE);

    /*
     * Check if we should start the Rx Window to hit the R2P instant
     * As R2P instant will always be centred on a TIM2, clock_offset_for_freq
     * will always be ((4*N)+2) clk value from the bt_clk@TIM0.
     *
     * search_win_frames        1   2   3   4   5
     * clock_offset_for_freq    2   6   6  10  10   = 2*(search_win_frames|1)
     *
     * Normally the conditional equality test will be satisfied 
     * rx_freq_bt_clk = r2p_instant (no need for clock wrap check)
     *
     * freq_clk = bt_clk@TIM0 + (search_win_frames|1)/2 * 4 ticks per frame
     *
     */

    cur_bt_clk = HW_Get_Bt_Clk_Avoid_Race(); /*HW_get_bt_clk();*/
    rx_freq_bt_clk = (cur_bt_clk + clock_offset_for_freq) & 0x0FFFFFFC;
    r2p_instant = p_r2p_info->r2p_instant;

    /*
     * If the window isn't centred on BT clock but we should start
     *    then just start window around instant
     * if (  cur_bt_clk <= r2p_instant < rx_freq_bt_clk)
     *     select freq clk as the r2p_instant clock value
     *     reduce the Rx window by missed frames (#ticks/4)
     * endif
     */
    if ( rx_freq_bt_clk > r2p_instant && cur_bt_clk <= r2p_instant)
    {
        search_win_frames -= ((rx_freq_bt_clk - r2p_instant)>>2);
        rx_freq_bt_clk = r2p_instant;
	}

	/*
     * If the instant has passed then just start window from current clock
     */
    if ( rx_freq_bt_clk >= r2p_instant)
    {
    	rx_freq_bt_clk = r2p_instant;
        DL_Set_Ctrl_State( p_device_link, CONNECTED_TO_MASTER);

        LC_Log_Procedure_Start(R2P_PROCEDURE);
        LSLCacc_Disable_Receiver();

        /*
         * Determine the frame offset (the mid_point of search window
         * search_win_frames     offset in ticks
         *      = 1                  4         the start of the next frame
         *      = 3                  8         then it is the start of the current + 2
         *      = 5                  8
         * freq_clk = current_clk + (search_win_frames+1)/2 * 4 ticks per frame
         * Hence, frame_offset_for_freq = (search_win_frames+1)/2;
         *
         */ 
        p_r2p_info->rx_freq_bt_clock = rx_freq_bt_clk;
        p_r2p_info->remaining_search_win_slots = (search_win_frames)<<1;

        if (!(search_win_frames & 1))
        { /* r2p starts one slot early (as on tim0) for these cases */
        	p_r2p_info->remaining_search_win_slots++;
        }

        /*
         * Inform channel control, (sets super state to Return_To_Piconet)
         */
        USLCchac_Procedure_Start(R2P_PROCEDURE, p_device_link);

#if (PRH_BS_CFG_TABASCO_VERSION >= PRH_BS_HW_TABASCO_VERSION_T1_0_0) && (PRH_BS_DEV_USE_T1_BT_CLOCK==1)
        /*
         *  Set the TIM block configuration bits so that an absolute BT clock value can be 
         *  directly written to hw.
         */
        HW_set_add_bt_clk_relative(0);
        HW_set_write_absolute_bt_clk(1);
        HW_set_delayed_bt_clk_update(0);
        /*
         *  Freeze the BT clock value on the next TIM2.
         */
        freeze_bt_clk_on_next_tim2 = TRUE;
        /*
         *  Write the absolute BT clock for which we expect to hear from the master.
         *  The value written must be adjusted (- 4) so that the hw BT clock will be 
         *  equal to rx_freq_bt_clk on the next TIM2 tick where it will be frozen until
         *  a packet is received from the master.
         */
        mSetHWEntry32(JAL_BT_CLK_OFFSET, (rx_freq_bt_clk - 4));
        /*
         *  Restore the configuration of the BT clock update mechanism to a state where
         *  it will behave like the old TIM block. A delay is required here because the TIM
         *  block is clocked at 1MHz, there must be a delay of at least 1us between the 
         *  change of configuration and the restoration, otherwise the change of configuration
         *  might not be seen by the TIM block.
         *  This restoration and delay can be removed once the rest of the stack use the
         *  new TIM block. i.e. once the TIM block configuration bits are set before writing
         *  the BT clock in other parts of the stack.
         */            
        HWdelay_Wait_For_us(5);
        HW_set_write_absolute_bt_clk(0);
        HW_set_add_bt_clk_relative(1);
#endif

#if (PRH_BS_CFG_SYS_HW_WINDOW_WIDENING_SUPPORTED==1)
        if ((search_win_frames == 1) && (_r2p_info.search_win_us <= USLC_R2P_MAX_SEARCH_WIN_US))
        {
            HW_set_win_ext(_r2p_info.search_win_us/2);
            g_sys_config.win_ext = (_r2p_info.search_win_us)/2; /* to restore default on sync det */

            HWradio_SetRadioMode(RADIO_MODE_TX_RX);
            LSLCpkt_Prepare_Receiver_WinExt_Return_To_Piconet(p_device_link, rx_freq_bt_clk);
        }
        else
#endif
        {
        	HWradio_SetRadioMode(RADIO_MODE_FULL_RX);
	        LSLCpkt_Prepare_Receiver_Return_To_Piconet(p_device_link, rx_freq_bt_clk);
        }

		/*
         * The LSLCslot_Handle_TIM0() enables the receiver on the initial scan start
         * Hence do not invoke LSLCacc_Enable_Receiver(); here.(results in cancel) 
         */

        /*
         * Timeout (e.g. Sniff Attempts) must take account the early wakeup time
         *
         * Therefore adjust by (number of slots -1) from now to instant
         */
        if (0 != p_r2p_info->r2p_timeout)
        {
        	p_r2p_info->r2p_timer = BTtimer_Set_Slots(p_r2p_info->r2p_timeout +
             (( (r2p_instant - cur_bt_clk) & 0x0FFFFFFE) >>1) - 1);
        }

        p_r2p_info->ctrl_state = OS_RECONNECTING;
    }
}

/***********************************************************************
 *
 * FUNCTION:  USLCr2p_State_OS_RECONNECTING_()
 * PURPOSE:   To re-establish active state on valid packet receive
 *
 * Intra-Slot timing will only be adjusted on valid Packet Header
 * The prepare outgoing packet will be done on RX_PKD_HDR interrupt
 * on execution of USLCsched_Prepare_Tx().  This means that the next
 * slave LMP or L2CAP packet will be processed as for Connection state.
 ***********************************************************************/
static void USLCr2p_State_OS_RECONNECTING_(
    t_USLCr2p_Private_Data *p_r2p_info)
{
#if (PRH_BS_CFG_SYS_RETURN_TO_PICONET_SUPPORTED==1)
    t_devicelink *p_device_link = p_r2p_info->p_active_devicelink;
    t_status status = DL_Get_Rx_Status(p_device_link);


#if(PRH_BS_CFG_SYS_TEST_MODE_TESTER_SUPPORTED == 1)
    /*
     * For test mode tester:
     *   If in tester loopback mode or rx_test and a valid packet received Then 
     *       Accept the packet and start an immediate return to piconet
     *   Endif
     */
    if ((DUT_ACTIVE_LOOPBACK == BTtst_Get_DUT_Mode() ||
         DUT_STANDALONE_RXTEST == BTtst_Get_DUT_Mode() ) && 
         (RX_NO_PACKET != status) && (RX_NONE != status) && (RX_HEC_ERROR!=status) )

    {
        u_int8 search_win_frames;
        t_clock rx_freq_bt_clk;
        /*
         * The transmitter is enabled here when state = Return_To_Piconet to
         * ensure that radio uses  HWradio_Put_Slave_In_Single_Tx();
         * It is necessary to use Single_Tx to align GIO signals are correct.
         */
        LSLCacc_Enable_Transmitter();

        search_win_frames = 10;

        rx_freq_bt_clk = (p_r2p_info->rx_freq_bt_clock + (search_win_frames<<2))
                                & 0x0FFFFFFF;
            
        p_r2p_info->rx_freq_bt_clock = rx_freq_bt_clk;
        p_r2p_info->remaining_search_win_slots = search_win_frames<<1;
        LSLCpkt_Prepare_Receiver_Return_To_Piconet(p_device_link, 
            rx_freq_bt_clk);
        return;
    }
#endif


    /*
     * If valid packet then complete, otherwise stay in this state until timeout
     *
     * Valid includes any packet type received from master:
     *    RX_OK, RX_DUPLICATE, RX_AMADDR_ERROR, RX_CRC_ERROR
     *    RX_BROADCAST_PACKET, RX_BROADCAST_DUPLICATE
     *
     * Invalid is if a HEC_ERROR or no received packet
     */
    if ((RX_NO_PACKET != status) && (RX_NONE != status) && (RX_HEC_ERROR!=status) )
    {

        LSLCacc_Disable_Receiver();

#if (USLCR2P_ALLOW_HISTORY_WIDENING_WINDOW==1)
        /* VF: packet received so no need for future extra correction */
        /* can be other strategy for decrementing */
		p_device_link->prev_R2P_failed = FALSE;
        p_device_link->not_received_correction=0;
#endif

        LC_Log_Procedure_Recv_Pkts_State1(R2P_PROCEDURE);

        /*
         * Ensure that bt_clock is aligned with current receive clock.
         * Clocks must be adjusted before Enabling Transmitter
         */

        /*
         * Note: This function is only called on PKD in final rx slot
         * if (rx_packet is DM5 or DH5) then
         *    Hence offset rx_freq_bt_clk by 8 ticks before setting
         * else if (rx_packet is DM3 or DH3) then
         *    Hence offset rx_freq_bt_clk by 4 ticks before setting
         * endif
         *
         * i.e. if any multislot add 4 ticks, and if DM5/DH5 add 4 more ticks
         */
        {
        /*
         * Must extract the actual hardware rx packet type to ensure
         * that multi-slot clock adjustment is valid.
         */
            t_packet rx_packet = (t_packet)HW_get_rx_type();

			if (( rx_packet == DM3)      ||  (rx_packet == DH3)       || 
				( rx_packet == EDR_2DH3) ||  (rx_packet ==  EDR_3DH3) || 
				( rx_packet == EV4)      ||  (rx_packet ==  EV5)      || 
				( rx_packet == EDR_2EV5) ||  (rx_packet ==  EDR_3EV5))
			{
				p_r2p_info->rx_freq_bt_clock += 4;
			}
			else if (( rx_packet == DM5)      ||  (rx_packet == DH5)  || 
					 ( rx_packet == EDR_2DH5) ||  (rx_packet ==  EDR_3DH5))
			{
				p_r2p_info->rx_freq_bt_clock += 8;
			}

        }

        /*
         * The transmitter is enabled here when state = Return_To_Piconet to
         * ensure that radio uses  HWradio_Put_Slave_In_Single_Tx();
         * It is necessary to use Single_Tx to align GIO signals are correct.
         *
         * Important to do this BEFORE writing clock, as clock write may be delayed!!
         */
        LSLCacc_Enable_Transmitter();

#if (PRH_BS_CFG_TABASCO_VERSION >= PRH_BS_HW_TABASCO_VERSION_T1_0_0) && (PRH_BS_DEV_USE_T1_BT_CLOCK==1)
        /*
         *  Remove the freeze from the BT clock on receiving a packet from the master.
         *  There is no need to update the BT clock as it starts ticking back from the correct value.
         */
        HW_set_freeze_bt_clk(0);
#else
        LSLCslot_Write_BT_Clk_Avoid_Race(&p_r2p_info->rx_freq_bt_clock);
#endif

        HWradio_SetRadioMode(RADIO_MODE_TX_RX);
        LSLCacc_Enable_Transmitter();

        /* 
         * Always resume Connection
         */
        USLCchac_Procedure_Finish(TRUE);

        /*
         * No callback to LC_Callback_Event_Handler() in RFH Procedure
         */
        p_r2p_info->ctrl_state = OS_INACTIVE;
        LSLChop_Set_HSE_Engine(LSLChop_HSE_DEFAULT);
    }
    else 
    {
        /*
         * Check if timeout has expired.
         */
        if ((0 != p_r2p_info->r2p_timeout)
            && (BTtimer_Is_Expired(p_r2p_info->r2p_timer)))
        {
            LSLCacc_Disable_Receiver();

#if (USLCR2P_ALLOW_HISTORY_WIDENING_WINDOW==1)
        /* remember we failed this R2P for future window extension */
        p_device_link->prev_R2P_failed = TRUE;
        p_device_link->not_received_correction += 1;
#endif

            /*
             * Resume previous superstate given timeout
             */
            USLCchac_Procedure_Finish(TRUE);

            /*
             * Callback to higher layers
             */
            LC_Callback_Event_Handler(LC_EVENT_R2P_TIMEOUT, 0);

#if (PRH_BS_CFG_TABASCO_VERSION >= PRH_BS_HW_TABASCO_VERSION_T1_0_0) && (PRH_BS_DEV_USE_T1_BT_CLOCK==1)
            /*
             *  Remove the freeze from the BT clock.
             */
            HW_set_freeze_bt_clk(0);
#endif

            p_r2p_info->ctrl_state = OS_INACTIVE;
            LSLChop_Set_HSE_Engine(LSLChop_HSE_DEFAULT);
        }

        /*
         * Check if start of next search window
         */
        else if (--p_r2p_info->remaining_search_win_slots == 0)
        {
            u_int8 search_win_frames;
            t_clock rx_freq_bt_clk;

            /*
             * Always explicitly disable Receiver to ensure that next Scan is new one
             * Critical that this is done to ensure _was_previous_rx_slot_a_scan == 0
             */
            LSLCacc_Disable_Receiver();

            /*
             * Continue to calculate search window as it may need extending
             */
            search_win_frames = USLCr2p_Get_Search_Window_Frames(p_device_link, 0);


#if (USLCR2P_ALLOW_HISTORY_WIDENING_WINDOW==1)
            rx_freq_bt_clk = (HW_Get_Bt_Clk_Avoid_Race() + (search_win_frames<<1))
                                & 0x0FFFFFFF;
#else
            rx_freq_bt_clk = (p_r2p_info->rx_freq_bt_clock + (search_win_frames<<2))
                                & 0x0FFFFFFF;
#endif            

            p_r2p_info->rx_freq_bt_clock = rx_freq_bt_clk;
            p_r2p_info->remaining_search_win_slots = search_win_frames<<1;
            
#if (PRH_BS_CFG_TABASCO_VERSION >= PRH_BS_HW_TABASCO_VERSION_T1_0_0) && (PRH_BS_DEV_USE_T1_BT_CLOCK==1)
            /*
             *  Set the TIM block configuration bits so that an absolute BT clock value can be 
             *  directly written to hw.
             */
            HW_set_add_bt_clk_relative(0);
            HW_set_write_absolute_bt_clk(1);
            HW_set_delayed_bt_clk_update(0);
            /*
             *  Freeze the BT clock value on the next TIM2.
             */
            freeze_bt_clk_on_next_tim2 = TRUE;
            /*
             *  Write the absolute BT clock for which we expect to hear from the master.
             *  The value written must be adjusted (- 4) so that the hw BT clock will be 
             *  equal to rx_freq_bt_clk on the next TIM2 tick where it will be frozen until
             *  a packet is received from the master.
             */
            mSetHWEntry32(JAL_BT_CLK_OFFSET, (rx_freq_bt_clk - 4));
            /*
             *  Restore the configuration of the BT clock update mechanism to a state where
             *  it will behave like the old TIM block. A delay is required here because the TIM
             *  block is clocked at 1MHz, there must be a delay of at least 1us between the 
             *  change of configuration and the restoration, otherwise the change of configuration
             *  might not be seen by the TIM block.
             *  This restoration and delay can be removed once the rest of the stack use the
             *  new TIM block. i.e. once the TIM block configuration bits are set before writing
             *  the BT clock in other parts of the stack.
             */            
            HWdelay_Wait_For_us(5);
            HW_set_write_absolute_bt_clk(0);
            HW_set_add_bt_clk_relative(1);
#endif
            
#if (PRH_BS_CFG_SYS_HW_WINDOW_WIDENING_SUPPORTED==1)
            if ((search_win_frames == 1) && (_r2p_info.search_win_us <= USLC_R2P_MAX_SEARCH_WIN_US))
            {
                HWradio_SetRadioMode(RADIO_MODE_TX_RX);
                LSLCpkt_Prepare_Receiver_WinExt_Return_To_Piconet(p_device_link, rx_freq_bt_clk);
            }
            else
#endif
            {
				HWradio_SetRadioMode(RADIO_MODE_FULL_RX);
				LSLCpkt_Prepare_Receiver_Return_To_Piconet(p_device_link, rx_freq_bt_clk);
            }

            LSLCacc_Enable_Receiver();          
        }
    }
#endif
}

/* 
 * NOTES
 * Multi-slot return to Piconet
 *
 * Define R2P_RADIO_SETUP_TIME 225us
 * 0. On return to piconet
 *    Start Of First Search, and Periodic Searches will be established.
 *
 * 1. Determine scan_window size in slots
 *    max_drift_us = (drift_master + my_max_drift)*time_since_last_piconet_access
 *    search_win_us = 2*max_drift_us
 *    search_win_frames = (scan_win_us+2*R2P_RADIO_SETUP_TIME)/1250 + 1
 * 
 * 2. Determine clock for frequency at half scan_win_frames and store
 *    freq_clock = frequency(cur_bt_clock) 
 *
 *
 *    bt_clk    actual  start   finish
 *    10        
 *
 * Scenario 1  Slave wishes to return to piconet at Mclk 32, 
 *             with 4 secs since last access
 *             max_drift_us  = (20+250)*4secs = 1080us
 *             search_win_us = max_drift_us*2 = 2160us
 *             search_win_frames = (2160+2*225us)%1250 + 1 = 3
 *
 *              T...    U...    V...    WWW.    X...    Y...    Z...  
 * M(CLK)     ..|tx |rx.|   |...|   |...+   |...|   |...|   |...|
 *               20      24      28      32      36      40      44
 *
 *            ..     UUUUUUU VVVVVVV WWWWWWW XXXXXXX YYYYYYY ZZZZZZ
 * S(CLK)       |rx.|tx |...|   |...|   |...|   |...|   |...|   |...
 * Single R2P    20      24      28      32      36      40      44
 *
 *            .. TTTTTTTTTTT WWWWWWWWWWWWWWWWWWWWWWW ZZZZZZZZZZZZZZ
 * S(CLK)       |rx.|tx |...|   |...|   |...|   |...|   |...|   |...
 * 0us           20      24      28      32      36      40      44
 *
 *            .. TTT WWWWWWWWWWWWWWWWWWWWWWW ZZZZZZZZZZZZZZZZZZZZ
 * S(CLK)       |rx.|tx |...|   |...|   |...|   |...|   |...|   |...
 * +1200us       24      28      32      36      40      44      48
 *
 *            .. TTTTTTTTTTTTTTTTTTT WWWWWWWWWWWWWWWWWWWWWWW ZZZZZZZ
 * S(CLK)       |rx.|tx |...|   |...|   |...|   |...|   |...|   |...
 * -1200us      16      20      24      28      32      36      40 
 *
 */
/*
 *
 uslc_return_to_piconet
    Invoked on TIM0, and PKD if packet received


Currently, on non multi-slot R2P the receiver is enabled on each frame.

    LSLCslot_Handle_TIM0()
    ->  USLCsched_Tx_Start()
        ->  _USLCsched_Prepare_Next_Slave_Frame_Activity()
            ->  USLCchac_prepare_next_slave_frame_activity()
                ->  USLCr2p_State_Dispatcher()
                    ->  USLCr2p_State_OS_RECONNECTING_()
        ->  USLCchac_prepareRx() 
            -> LSLCpkt_Activate_Receiver()
                -> LSLCpkt_Prepare_Receiver_Return_To_Piconet()


    LSLCslot_Handle_PKD()
        USLCsched_Process_Rx()
            ->  USLCchac_Process_Rx()
                ->  USLCr2p_State_Dispatcher()
                    ->  USLCr2p_State_OS_RECONNECTING_()
 
For multi-slot R2P the receiver is enabled as required within USLCr2p
 
    LSLCslot_Handle_TIM0()
    ->  USLCsched_Tx_Start()
        ->  _USLCsched_Prepare_Next_Slave_Frame_Activity()
            ->  USLCchac_prepare_next_slave_frame_activity()
                ->  USLCr2p_State_Dispatcher()
                    ->  USLCr2p_State_OS_RECONNECTING_()
                        -> LSLCpkt_Prepare_Receiver_Return_To_Piconet()
 *
 *
 #if 1   Useful logger 
 extern  int Testsup_DebugPrint(int logLevel, char *formatString, ... );

    Testsup_DebugPrint(1, "USLCr2p_State_OS_RECONNECTING_ remaining frames %d \n", 
        p_r2p_info->remaining_search_win_slots  );
 #endif 
 */

/***********************************************************************
 *
 * FUNCTION:  USLCr2p_Write_R2P_Min_Search_Window
 * PURPOSE:   To change min search window by TCI
 *
 ***********************************************************************/
void USLCr2p_Write_R2P_Min_Search_Window(u_int8 new_window)
{
     _r2p_min_search_window=new_window;
}

/***********************************************************************
 *
 * FUNCTION:  USLCr2p_Read_R2P_Min_Search_Window
 * PURPOSE:   To provide min search window value by TCI
 *
 ***********************************************************************/
u_int8 USLCr2p_Read_R2P_Min_Search_Window(void)
{
    return _r2p_min_search_window;
}

#elif !(PRH_BS_CFG_SYS_RETURN_TO_PICONET_SUPPORTED==1)
/*
 * Ensure non-empty source file for pedantic C
 */
void USLCr2p_Initialise(void);

#endif /*(PRH_BS_CFG_SYS_RETURN_TO_PICONET_SUPPORTED==1)*/
