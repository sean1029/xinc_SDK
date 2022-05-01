/*
 * MODULE NAME:    hw_pta.c
 * PROJECT CODE:   Bluestream BT3.0
 * DESCRIPTION:    PTA Interface Driver
 * MAINTAINER:     Tom Kerwick
 * DATE:           18 October 2010
 *
 * SOURCE CONTROL: $Id: hw_pta.c,v 1.15 2014/02/21 13:31:28 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2010-2012 Ceva Inc.
 *     All rights reserved.
 *
 */
#ifndef _PARTHUS_HW_PTA_IMPL_
#define _PARTHUS_HW_PTA_IMPL_

#include "sys_config.h"
#include "sys_types.h"
#include "hw_lc.h"
#include "hw_pta.h"
#include "hw_radio.h"


#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)

/*
 * HW_PTA_ALT_3WIRE_INTERFACE
 *
 * An alternative 3-wire PTA interface signalling (AR6000) controlled as follows:
 *
 * RF_ACTIVE: asserts Trf_s prior to tx/rx slots and deasserts within Trf_h max.
 * This is a directly replacement of BT_REQ albeit with new lead time requirements.
 * 
 * BT_STATE: asserts (Trf_s - Tlp) prior to tx slots and deasserts within Trf_h max.
 * Asserts (Trf_s - Thp) earlier prior to high priority tx/rx slots, and pulses at
 * this timing prior to high priority rx/tx slots for a Thp-Tlp duration. This is a
 * modification of the BT_TX_ACCESS with a pulse combine for priority control.
 *
 * WLAN_ACTIVE: expected to assert no later than Tw_s prior to blocked tx/rx slots.
 * This is a direct inversion of PTA_GRANT (inversion to be achieved via HW change).
 * It may be tested with PTA_GRANT_TEST max and min (Tlp+Tw_d) response times.
 *
 */
#define HW_PTA_ALT_3WIRE_INTERFACE 0        //- for LC Platform.


#if (HW_PTA_ALT_3WIRE_INTERFACE==1)

#define HW_PTA_Trf_s_MIN	150
#define HW_PTA_Trf_s_TYP	150
#define HW_PTA_Trf_s_MAX	200
#define HW_PTA_Trf_h_MAX	25
#define HW_PTA_Thp_MIN		0
#define HW_PTA_Thp_MAX		1
#define HW_PTA_Tlp_MIN		15
#define HW_PTA_Tlp_TYP		20
#define HW_PTA_Tlp_MAX		25
#define HW_PTA_Tlp_MAX		25
#define HW_PTA_Tr_w_MAX		30
#define HW_PTA_Tw_d_MAX		55
#define HW_PTA_Tw_s_MIN		75

#define MAX_BT_REQ_LEAD_TIME HW_PTA_Trf_s_MAX /* Max Trf_s on RF_ACTIVE signal */
#define DEFAULT_BT_REQ_LEAD_TIME HW_PTA_Trf_s_TYP /* Default typical Trf_s */
#define MIN_BT_REQ_LEAD_TIME HW_PTA_Trf_s_MIN /* Min Trf_s on RF_ACTIVE signal */

#define BT_REQ_SETUP_TIME HW_PTA_Thp_MIN /* Default to min Thp */

#define HW_PTA_GRANT_TEST_RESPONSE_TIME (HW_PTA_Tr_w_MAX)

#else

#define MAX_BT_REQ_LEAD_TIME 128 //100 /* eg PTA RESPONSE TIME 50us+ */
#define DEFAULT_BT_REQ_LEAD_TIME 118//50 /* eg PTA RESPONSE TIME 10us */ //modified by gwh
#define MIN_BT_REQ_LEAD_TIME 20 /* eg MAX PTA RESPONSE TIME 5us */

#define BT_REQ_SETUP_TIME 2 /* nominal 2us prior to BT_REQ asserts */

#define HW_PTA_GRANT_TEST_RESPONSE_TIME 10

#endif

#if ((__DEBUG_PTA_ENABLE__==1)&&(__DEBUG_PTA_GRANT_TEST_ENABLE__==1)) // LC add (#if #endif)
#define HW_PTA_GRANT_TEST_ENABLED 1         // LC change 0 to 1 for TEST PTA!
#else
#define HW_PTA_GRANT_TEST_ENABLED 0
#endif

#if (BUILD_TYPE!=UNIT_TEST_BUILD)
#define HW_PTA_INCORPORATE_TX_RX_DELAYS 1
#else
#define HW_PTA_INCORPORATE_TX_RX_DELAYS 0
#endif

static u_int16 HWpta_bt_req_lead_time;

typedef struct pta_access
{
	boolean requested;
    boolean priority;
    t_role role;
    boolean double_win;
    boolean full_rx_mode;
    boolean freq_overlap;
} t_pta_access;

static t_pta_access HWpta_Tx_Access, HWpta_Rx_Access;
boolean HWpta_Block_Access_Active;

static u_int8 HWpta_RX_DELAY; 
static u_int8 HWpta_TX_DELAY;

/******************************************************************************
 *
 * FUNCTION: HWpta_Initialise
 *
 *****************************************************************************/
boolean HWpta_Initialise()
{

    HWpta_Tx_Access.requested = FALSE;
	HWpta_Rx_Access.requested = FALSE;

	HWpta_Block_Access_Active = FALSE;

	HWpta_Return_To_Standby();

#if (HW_PTA_ALT_3WIRE_INTERFACE==1)
	HWpta_Generic_BT_Access_Mode(TRUE);
#else
	HWpta_Generic_BT_Access_Mode(FALSE);
#endif

    HWpta_Combine_BT_RX_REQ(TRUE);

    HW_set_pta_mode_enable(1);

#if (HW_PTA_GRANT_TEST_ENABLED==1)
    HW_set_pta_grant_test_enable(1);
#else
    HW_set_pta_grant_test_enable(0);
#endif

	HW_set_pta_tx_active_gio_sel(9); /* default BPKTCTL rf driver */
	HW_set_pta_bpktctl_grant(1); /* override carrier grant by default */

#if (HW_PTA_ALT_3WIRE_INTERFACE==1)
	HW_set_pta_grant_inverse(1); /* inverts pta_grant for a wlan_active signal */
	HW_set_pta_grant_rx_override(0); /* do not allow rx without pta_grant */
#endif

#if (HW_PTA_INCORPORATE_TX_RX_DELAYS==1)
	HWpta_RX_DELAY = mHWradio_RX_DELAY;
    HWpta_TX_DELAY = mHWradio_TX_DELAY;
#else
	HWpta_RX_DELAY = 0;
    HWpta_TX_DELAY = 0;
#endif

	HWpta_Set_Bt_Req_Lead_Time(DEFAULT_BT_REQ_LEAD_TIME);

    /*
     * Below used for automatic Window Widening compensation of the PTA block GIOs.
     * Initially disabled - These are enabled dynamically if/as required.
     */
	HWpta_Set_GIOs_Hi_Early(0);
	HWpta_Set_GIOs_Hi_Late(0);
	HWpta_Set_GIOs_Lo_Early(0);
	HWpta_Set_GIOs_Lo_Late(0);

	return TRUE;
}


/******************************************************************************
 *
 * FUNCTION: HWpta_Control_Enabled
 *
 *****************************************************************************/
boolean HWpta_Control_Enabled(boolean enabled)
{
	HW_set_pta_mode_enable(enabled?1:0);
	return TRUE; /* configured */
}

/******************************************************************************
 *
 * FUNCTION: HWpta_Test_Control_Enabled
 *
 *****************************************************************************/
boolean HWpta_Test_Control_Enabled(boolean enabled)
{
#if (HW_PTA_GRANT_TEST_ENABLED==1)
	HW_set_pta_grant_test_enable(enabled?1:0);
	return TRUE; /* configured */
#else
	return !enabled; /* not configured */
#endif
}

/******************************************************************************
 *
 * FUNCTION: HWpta_Return_To_Standby
 *
 *****************************************************************************/
void HWpta_Return_To_Standby()
{

    HW_set_GIO_LOW_BT_TX_REQ(HW_OVERRIDE_LOW|HW_GIO_TX_END);
	HW_set_GIO_HIGH_BT_TX_REQ(HW_GIO_RX_MID);

	HW_set_GIO_LOW_BT_RX_REQ(HW_OVERRIDE_LOW|HW_GIO_RX_END);
	HW_set_GIO_HIGH_BT_RX_REQ(HW_GIO_TX_MID);

//#if (HW_PTA_GRANT_TEST_ENABLED==1)
	HW_set_GIO_LOW_PTA_TX_GRANT_TEST(HW_OVERRIDE_LOW|HW_GIO_TX_END);
	HW_set_GIO_HIGH_PTA_TX_GRANT_TEST(HW_GIO_RX_MID);

	HW_set_GIO_LOW_PTA_RX_GRANT_TEST(HW_OVERRIDE_LOW|HW_GIO_RX_END);
	HW_set_GIO_HIGH_PTA_RX_GRANT_TEST(HW_GIO_TX_MID);
//#endif

	HW_set_GIO_LOW_BT_TX_PRIORITY(HW_OVERRIDE_LOW|HW_GIO_TX_END);
	HW_set_GIO_HIGH_BT_TX_PRIORITY(HW_GIO_RX_MID);

	HW_set_GIO_LOW_BT_RX_PRIORITY(HW_OVERRIDE_LOW|HW_GIO_RX_END);
	HW_set_GIO_HIGH_BT_RX_PRIORITY(HW_GIO_TX_MID);

	HW_set_GIO_LOW_BT_TX_FREQ_OVERLAP(HW_OVERRIDE_LOW|HW_GIO_TX_END);
	HW_set_GIO_HIGH_BT_TX_FREQ_OVERLAP(HW_GIO_RX_MID);

	HW_set_GIO_LOW_BT_RX_FREQ_OVERLAP(HW_OVERRIDE_LOW|HW_GIO_RX_END);
	HW_set_GIO_HIGH_BT_RX_FREQ_OVERLAP(HW_GIO_TX_MID);

	HW_set_GIO_LOW_TX_ACCESS(HW_OVERRIDE_LOW|HW_GIO_TX_END);
	HW_set_GIO_HIGH_TX_ACCESS(HW_GIO_RX_MID);

	HW_set_GIO_LOW_RX_ACCESS(HW_OVERRIDE_LOW|HW_GIO_RX_END);
	HW_set_GIO_HIGH_RX_ACCESS(HW_GIO_TX_MID);
}

/******************************************************************************
 *
 * FUNCTION: HWpta_Set_Bt_Req_Lead_Time
 *
 *****************************************************************************/
boolean HWpta_Set_Bt_Req_Lead_Time(u_int16 lead_time)
{
	boolean is_configured = FALSE;

    if ((lead_time <= MAX_BT_REQ_LEAD_TIME)
		&& (lead_time >= MIN_BT_REQ_LEAD_TIME))
	{
		HWpta_bt_req_lead_time = lead_time;
		is_configured = TRUE;
	}

	return is_configured;
}

/******************************************************************************
 *
 * FUNCTION: HWpta_Tx_Access_Request
 *
 *****************************************************************************/
void HWpta_Tx_Access_Request(boolean priority, boolean freq_overlap, t_role role)
{
	if (!HWpta_Block_Access_Active)
	{
		u_int16 ref_tx_hi_point, ref_tx_lo_point;

		HWpta_Tx_Access.requested = TRUE;
		HWpta_Tx_Access.double_win = (HWradio_GetRadioMode()==RADIO_MODE_MASTER_DOUBLE_WIN);
		HWpta_Tx_Access.role = role;
		HWpta_Tx_Access.priority = priority;
		HWpta_Tx_Access.freq_overlap = freq_overlap;

		if (HWpta_Tx_Access.double_win)
		{ /* allow a double_win to extend for second ids */
			ref_tx_hi_point = HW_GIO_TX_START + HW_GIO_TOTAL_FRAME_TIME;
			ref_tx_lo_point = HW_GIO_TX_END;
		}
		else
		{
			ref_tx_hi_point = HW_GIO_TX_START + HW_GIO_TOTAL_FRAME_TIME;
			ref_tx_lo_point = HW_GIO_TX_SYNC_OUT;       //- LC Note here!!
		}

		/*
		 * Set TX related signals to go high the configured time prior to TX_START and go
		 * low nominally on TX_END, albeit overridden on TX_START.
		 */
		{
    		u_int16 tx_hi_point = ref_tx_hi_point - HWpta_bt_req_lead_time - HWpta_TX_DELAY;

			HW_set_GIO_HIGH_BT_TX_REQ(tx_hi_point);
			HW_set_GIO_LOW_BT_TX_REQ(ref_tx_lo_point);

#if (HW_PTA_GRANT_TEST_ENABLED==1)
			HW_set_GIO_HIGH_PTA_TX_GRANT_TEST(tx_hi_point + HW_PTA_GRANT_TEST_RESPONSE_TIME);
			HW_set_GIO_LOW_PTA_TX_GRANT_TEST(ref_tx_hi_point - HW_GIO_TOTAL_SLOT_TIME);
#endif
			tx_hi_point += BT_REQ_SETUP_TIME;       //- LC change - to +.

#if (HW_PTA_ALT_3WIRE_INTERFACE==0)
			if (priority)
    		{
				HW_set_GIO_HIGH_BT_TX_PRIORITY(tx_hi_point);
    			HW_set_GIO_LOW_BT_TX_PRIORITY(ref_tx_lo_point);
    		}
    		if (freq_overlap)
    		{
				HW_set_GIO_HIGH_BT_TX_FREQ_OVERLAP(tx_hi_point);
    			HW_set_GIO_LOW_BT_TX_FREQ_OVERLAP(ref_tx_lo_point);
    		}
#else /* indicate priority via Thp-Tlp delta on tx/rx BT_ACCESS */
    		if ((!priority) && (MASTER==role))
    		{
	        	tx_hi_point+=HW_PTA_Tlp_TYP;
			}
#endif
    		HW_set_GIO_HIGH_TX_ACCESS(tx_hi_point);
			HW_set_GIO_LOW_TX_ACCESS(ref_tx_lo_point);
		}
	}
}

/******************************************************************************
 *
 * FUNCTION: HWpta_Service_Tx_Start
 *
 *****************************************************************************/
void HWpta_Service_Tx_Start()
{

#if (HW_PTA_ALT_3WIRE_INTERFACE==1)
	if (!HWpta_Rx_Access.requested && !HWpta_Block_Access_Active)
	{
		HW_set_GIO_LOW_BT_RX_REQ(HW_OVERRIDE_LOW|HW_GIO_RX_END);
		HW_set_GIO_HIGH_BT_RX_REQ(HW_GIO_TX_MID);
	}
#endif

    /*
	 * If Tx_Access has been requested, manage the remaineder of the TX_ACCESS
	 * assertion through OVERRIDE_HIGH. Will OVERRIDE_LOW on PKA.
	 */
	if (HWpta_Tx_Access.requested)
	{
#if (HW_PTA_ALT_3WIRE_INTERFACE==1)
		if (HWpta_Tx_Access.role == MASTER) /* start of a tx-rx frame */
		{
            HW_set_GIO_HIGH_BT_TX_REQ(HW_OVERRIDE_HIGH|HW_GIO_TX_START);
		}
		else /* second half of an rx-tx frame so deassert on hw inactive */
#endif
		{
            HW_set_GIO_HIGH_BT_TX_REQ(HW_ACTIVE_OVERRIDE_HIGH|HW_GIO_TX_START);
		}

        HW_set_GIO_HIGH_TX_ACCESS(HW_ACTIVE_OVERRIDE_HIGH|HW_GIO_TX_START);

#if (HW_PTA_GRANT_TEST_ENABLED==1)
        HW_set_GIO_HIGH_PTA_TX_GRANT_TEST(HW_OVERRIDE_HIGH|HW_GIO_TX_START);
#endif

#if (HW_PTA_ALT_3WIRE_INTERFACE==0)
    	if (HWpta_Tx_Access.priority)
		{
		    HW_set_GIO_HIGH_BT_TX_PRIORITY(HW_ACTIVE_OVERRIDE_HIGH|HW_GIO_TX_START);
		}
		if (HWpta_Tx_Access.freq_overlap)
		{
		    HW_set_GIO_HIGH_BT_TX_FREQ_OVERLAP(HW_ACTIVE_OVERRIDE_HIGH|HW_GIO_TX_START);
		}
#endif
	}
	else if ((HWpta_Rx_Access.requested) && (HWpta_Rx_Access.full_rx_mode))
	{
	    HWpta_Service_Rx_Start(); /* may start full_rx on the tx boundary here */
	}
}

/******************************************************************************
 *
 * FUNCTION: HWpta_Service_PKA
 *
 *****************************************************************************/
void HWpta_Service_PKA()
{
	/*
	 * If Tx_Access has been requested, disable this with an OVERRIDE_LOW and
	 * arbitarily reset the rise time of TX_ACCESS back to TX_START.
	 */
	if (HWpta_Tx_Access.requested)
	{
		if (HWpta_Tx_Access.double_win)
		{ /* first of a double-win => no action required yet */
			HWpta_Tx_Access.double_win = 0;
			return;
		}

        HW_set_GIO_LOW_TX_ACCESS(HW_OVERRIDE_LOW|HW_GIO_TX_END);
	    HW_set_GIO_HIGH_TX_ACCESS(HW_GIO_RX_MID);

#if (HW_PTA_ALT_3WIRE_INTERFACE==0) /* else low on Service_Rx_Start */
	    HW_set_GIO_LOW_BT_TX_REQ(HW_OVERRIDE_LOW|HW_GIO_TX_END);
	    HW_set_GIO_HIGH_BT_TX_REQ(HW_GIO_RX_MID);
#endif

#if (HW_PTA_GRANT_TEST_ENABLED==1)
		HW_set_GIO_LOW_PTA_TX_GRANT_TEST(HW_OVERRIDE_LOW|HW_GIO_TX_END);
		HW_set_GIO_HIGH_PTA_TX_GRANT_TEST(HW_GIO_RX_MID);
#endif

#if (HW_PTA_ALT_3WIRE_INTERFACE==0)
	    if (HWpta_Tx_Access.priority)
		{
			HW_set_GIO_LOW_BT_TX_PRIORITY(HW_OVERRIDE_LOW|HW_GIO_TX_END);
			HW_set_GIO_HIGH_BT_TX_PRIORITY(HW_GIO_RX_MID);
		}
        if (HWpta_Tx_Access.freq_overlap)
		{
            HW_set_GIO_LOW_BT_TX_FREQ_OVERLAP(HW_OVERRIDE_LOW|HW_GIO_TX_END);
			HW_set_GIO_HIGH_BT_TX_FREQ_OVERLAP(HW_GIO_RX_MID);
		}
#endif

		HWpta_Tx_Access.requested = FALSE;
	}
}

/******************************************************************************
 *
 * FUNCTION: HWpta_Tx_Access_Complete
 *
 *****************************************************************************/
void HWpta_Tx_Access_Complete()
{
	/*
	 * If Tx_Access has been requested, disable this with an OVERRIDE_LOW and
	 * arbitarily reset the rise time of TX_ACCESS back to TX_START.
	 */
	if (HWpta_Tx_Access.requested)
	{

		HW_set_GIO_LOW_TX_ACCESS(HW_OVERRIDE_LOW|HW_GIO_TX_END);
		HW_set_GIO_HIGH_TX_ACCESS(HW_GIO_RX_MID);

#if (HW_PTA_ALT_3WIRE_INTERFACE==0) /* else low on Service_Rx_Start */
		HW_set_GIO_LOW_BT_TX_REQ(HW_OVERRIDE_LOW|HW_GIO_TX_END);
	    HW_set_GIO_HIGH_BT_TX_REQ(HW_GIO_RX_MID);
#endif

#if (HW_PTA_GRANT_TEST_ENABLED==1)
		HW_set_GIO_LOW_PTA_TX_GRANT_TEST(HW_OVERRIDE_LOW|HW_GIO_TX_END);
		HW_set_GIO_HIGH_PTA_TX_GRANT_TEST(HW_GIO_RX_MID);
#endif

#if (HW_PTA_ALT_3WIRE_INTERFACE==0)
		if (HWpta_Tx_Access.priority)
		{
			HW_set_GIO_LOW_BT_TX_PRIORITY(HW_OVERRIDE_LOW|HW_GIO_TX_END);
			HW_set_GIO_HIGH_BT_TX_PRIORITY(HW_GIO_RX_MID);
		}
        if (HWpta_Tx_Access.freq_overlap)
		{
            HW_set_GIO_LOW_BT_TX_FREQ_OVERLAP(HW_OVERRIDE_LOW|HW_GIO_TX_END);
			HW_set_GIO_HIGH_BT_TX_FREQ_OVERLAP(HW_GIO_RX_MID);
		}
#endif

		HWpta_Tx_Access.requested = FALSE;
	}
}

/******************************************************************************
 *
 * FUNCTION: HWpta_Rx_Access_Request
 *
 *****************************************************************************/
void HWpta_Rx_Access_Request(boolean priority, boolean freq_overlap, t_role role)
{
	if (!HWpta_Block_Access_Active)
	{
		u_int16 ref_rx_hi_point, ref_rx_lo_point;

		HWpta_Rx_Access.requested = TRUE;
		HWpta_Rx_Access.full_rx_mode = (HWradio_GetRadioMode()==RADIO_MODE_FULL_RX);
		HWpta_Rx_Access.role = role;
		HWpta_Rx_Access.priority = priority;
		HWpta_Rx_Access.freq_overlap = freq_overlap;

		if (HWpta_Rx_Access.full_rx_mode && HW_get_bt_clk()&2)
		{ /* allow a full_rx to start on next tx_slot */
			ref_rx_hi_point = HW_GIO_TX_START + HW_GIO_TOTAL_FRAME_TIME;
			ref_rx_lo_point = HW_GIO_TX_END;
		}
		else
		{ /* assume rx access to start on next rx_slot */
    		ref_rx_hi_point = HW_GIO_RX_START;
    		if (HWradio_GetRadioMode()==RADIO_MODE_TX_RX)
				ref_rx_lo_point = HW_GIO_RX_SYNC_DET;   //- LC Note here!!
    		else /* master double win or full rx mode */
    			ref_rx_lo_point = HW_GIO_RX_END;
		}

		/*
		 * Set RX related signals to go high the configured time prior to RX_START and go
		 * low nominally on RX_END, albeit overridden on RX_START.
		 */
		{
    		u_int16 rx_hi_point = ref_rx_hi_point - HWpta_bt_req_lead_time - HWpta_RX_DELAY;

			HW_set_GIO_HIGH_BT_RX_REQ(rx_hi_point);
			HW_set_GIO_LOW_BT_RX_REQ(ref_rx_lo_point);

#if (HW_PTA_GRANT_TEST_ENABLED==1)
			HW_set_GIO_HIGH_PTA_RX_GRANT_TEST(rx_hi_point + HW_PTA_GRANT_TEST_RESPONSE_TIME);
			HW_set_GIO_LOW_PTA_RX_GRANT_TEST(ref_rx_hi_point - HW_GIO_TOTAL_SLOT_TIME);
#endif

			rx_hi_point += BT_REQ_SETUP_TIME;  //modified from minor to plus 

#if (HW_PTA_ALT_3WIRE_INTERFACE==0)
			if (priority)
			{
        		HW_set_GIO_HIGH_BT_RX_PRIORITY(rx_hi_point);
        		HW_set_GIO_LOW_BT_RX_PRIORITY(ref_rx_lo_point);
			}
			if (freq_overlap)
			{
        		HW_set_GIO_HIGH_BT_RX_FREQ_OVERLAP(rx_hi_point);
        		HW_set_GIO_LOW_BT_RX_FREQ_OVERLAP(ref_rx_lo_point);
			}

			HW_set_GIO_HIGH_RX_ACCESS(rx_hi_point);
			HW_set_GIO_LOW_RX_ACCESS(ref_rx_lo_point);
#else /* indicate priority via Thp-Tlp pulse on rx/tx BT_ACCESS */
    		if ((priority) && (SLAVE==role))
    		{
				HW_set_GIO_HIGH_RX_ACCESS(rx_hi_point);
				HW_set_GIO_LOW_RX_ACCESS(rx_hi_point+HW_PTA_Tlp_TYP);
    		}
#endif
		}
	}
}

/******************************************************************************
 *
 * FUNCTION: HWpta_Service_Rx_Start
 *
 *****************************************************************************/
void HWpta_Service_Rx_Start()
{

#if (HW_PTA_ALT_3WIRE_INTERFACE==1)
	if (!HWpta_Tx_Access.requested && !HWpta_Block_Access_Active)
	{		
		HW_set_GIO_LOW_BT_TX_REQ(HW_OVERRIDE_LOW|HW_GIO_TX_END);
		HW_set_GIO_HIGH_BT_TX_REQ(HW_GIO_RX_MID);
	}
#endif

	/*
	 * If Rx_Access has been requested, manage the remaineder of the RX_ACCESS
	 * assertion through OVERRIDE_HIGH. Will OVERRIDE_LOW on PKD.
	 */
	if (HWpta_Rx_Access.requested)
	{
#if (HW_PTA_ALT_3WIRE_INTERFACE==1)
		if (HWpta_Rx_Access.role == SLAVE) /* start of an rx-tx frame */
		{
			HW_set_GIO_HIGH_BT_RX_REQ(HW_OVERRIDE_HIGH|HW_GIO_RX_START);
		}
		else /* end of a tx-rx frame so deassert on hw inactive */
#endif
		{
			HW_set_GIO_HIGH_BT_RX_REQ(HW_ACTIVE_OVERRIDE_HIGH|HW_GIO_RX_START);
		}

#if (HW_PTA_ALT_3WIRE_INTERFACE==0)
		HW_set_GIO_HIGH_RX_ACCESS(HW_ACTIVE_OVERRIDE_HIGH|HW_GIO_RX_START);
#endif

#if (HW_PTA_GRANT_TEST_ENABLED==1)
		HW_set_GIO_HIGH_PTA_RX_GRANT_TEST(HW_OVERRIDE_HIGH|HW_GIO_RX_START);
#endif

#if (HW_PTA_ALT_3WIRE_INTERFACE==0)
		if (HWpta_Rx_Access.priority)
		{
		    HW_set_GIO_HIGH_BT_RX_PRIORITY(HW_ACTIVE_OVERRIDE_HIGH|HW_GIO_RX_START);
		}
		if (HWpta_Rx_Access.freq_overlap)
		{
		    HW_set_GIO_HIGH_BT_RX_FREQ_OVERLAP(HW_ACTIVE_OVERRIDE_HIGH|HW_GIO_RX_START);
		}
#endif
	}
}

/******************************************************************************
 *
 * FUNCTION: HWpta_Rx_Access_Complete
 *
 *****************************************************************************/
void HWpta_Rx_Access_Complete()
{
	/*
	 * If Rx_Access has been requested, disable this with an OVERRIDE_LOW and
	 * arbitarily reset the rise time of RX_ACCESS back to RX_START.
	 */
	if (HWpta_Rx_Access.requested)
	{

		HW_set_GIO_LOW_RX_ACCESS(HW_OVERRIDE_LOW|HW_GIO_RX_END);
	    HW_set_GIO_HIGH_RX_ACCESS(HW_GIO_TX_MID);

#if (HW_PTA_ALT_3WIRE_INTERFACE==0) /* else low on Service_Tx_Start */
		HW_set_GIO_LOW_BT_RX_REQ(HW_OVERRIDE_LOW|HW_GIO_RX_END);
	    HW_set_GIO_HIGH_BT_RX_REQ(HW_GIO_TX_MID);
#endif

#if (HW_PTA_GRANT_TEST_ENABLED==1)
		HW_set_GIO_LOW_PTA_RX_GRANT_TEST(HW_OVERRIDE_LOW|HW_GIO_RX_END);
		HW_set_GIO_HIGH_PTA_RX_GRANT_TEST(HW_GIO_TX_MID);
#endif

#if (HW_PTA_ALT_3WIRE_INTERFACE==0)
		if (HWpta_Rx_Access.priority)
		{
			HW_set_GIO_LOW_BT_RX_PRIORITY(HW_OVERRIDE_LOW|HW_GIO_RX_END);
			HW_set_GIO_HIGH_BT_RX_PRIORITY(HW_GIO_TX_MID);
		}
		if (HWpta_Rx_Access.freq_overlap)
		{
            HW_set_GIO_LOW_BT_RX_FREQ_OVERLAP(HW_OVERRIDE_LOW|HW_GIO_RX_END);
			HW_set_GIO_HIGH_BT_RX_FREQ_OVERLAP(HW_GIO_TX_MID);
		}
#endif
		
		HWpta_Rx_Access.full_rx_mode = FALSE;
		HWpta_Rx_Access.requested = FALSE;
	}
}

#if (PRH_BS_CFG_SYS_LE_CONTROLLER_SUPPORTED==1)

/******************************************************************************
 *
 * FUNCTION: HWpta_LE_Access_Request
 *
 * Access requests for LE operations do not distinguish TX/RX, due to short
 * TIFS intervals. An LE access request shall assume both TX and RX accesses.
 *
 * PTA is controlled coarse (non-timed) for LE operations, reducing number of
 * hooks required to PTA driver for LE mode - reducing MIPS load. Net effect
 * of asserting ACCESS/REQ simultanously is indicating HIGH PRIORITY accesses.
 *
 *****************************************************************************/
void HWpta_LE_Access_Request(boolean priority, boolean freq_overlap, t_RadioMode rf_mode)
{

	HWpta_Block_Access_Active = TRUE;

#if (HW_PTA_ALT_3WIRE_INTERFACE==0)

	//if (rf_mode == RADIO_MODE_FULL_RX) //modified by gwh
	if (rf_mode == RADIO_MODE_LE_FULL_RX)
    {
    	if (priority)
    	{
    		HW_set_GIO_HIGH_BT_RX_PRIORITY(HW_OVERRIDE_HIGH|HW_GIO_RX_START);
    		HW_set_GIO_LOW_BT_RX_PRIORITY(HW_GIO_RX_END);
    	}

    	if (freq_overlap)
    	{
    		HW_set_GIO_HIGH_BT_RX_FREQ_OVERLAP(HW_OVERRIDE_HIGH|HW_GIO_RX_START);
    		HW_set_GIO_LOW_BT_RX_FREQ_OVERLAP(HW_GIO_RX_END);
    	}

    	HW_set_GIO_HIGH_RX_ACCESS(HW_OVERRIDE_HIGH|HW_GIO_RX_START);
    	HW_set_GIO_LOW_RX_ACCESS(HW_GIO_RX_START); // modified by gwh.
    }
    else
    {
    	if (priority)
    	{
    		HW_set_GIO_HIGH_BT_TX_PRIORITY(HW_OVERRIDE_HIGH|HW_GIO_TX_START);
    		HW_set_GIO_LOW_BT_TX_PRIORITY(HW_GIO_TX_END);
    	}

    	if (freq_overlap)
    	{
    		HW_set_GIO_HIGH_BT_TX_FREQ_OVERLAP(HW_OVERRIDE_HIGH|HW_GIO_TX_START);
    		HW_set_GIO_LOW_BT_TX_FREQ_OVERLAP(HW_GIO_TX_END);
    	}

    	HW_set_GIO_HIGH_TX_ACCESS(HW_OVERRIDE_HIGH|HW_GIO_TX_START);
    	HW_set_GIO_LOW_TX_ACCESS(HW_GIO_TX_END);
    }

#else

	HW_set_GIO_HIGH_TX_ACCESS(HW_OVERRIDE_HIGH|HW_GIO_TX_START);
	HW_set_GIO_LOW_TX_ACCESS(HW_GIO_TX_END);

#endif

	HW_set_GIO_HIGH_BT_TX_REQ(HW_OVERRIDE_HIGH|HW_GIO_TX_START);
	HW_set_GIO_LOW_BT_TX_REQ(HW_GIO_TX_END);

#if (HW_PTA_GRANT_TEST_ENABLED==1)
    HW_set_GIO_HIGH_PTA_TX_GRANT_TEST(HW_OVERRIDE_HIGH|HW_GIO_TX_START);
	HW_set_GIO_LOW_PTA_TX_GRANT_TEST(HW_GIO_TX_END);
#endif

}

/******************************************************************************
 *
 * FUNCTION: HWpta_LE_Access_Complete
 *
 * Disable the overrides Block Access Requests, restore as BT Classic Standby.
 *
 *****************************************************************************/
void HWpta_LE_Access_Complete()
{
	HWpta_Block_Access_Active = FALSE;

#if (HW_PTA_ALT_3WIRE_INTERFACE==0)

	HW_set_GIO_LOW_BT_RX_PRIORITY(HW_OVERRIDE_LOW|HW_GIO_RX_END);
	HW_set_GIO_HIGH_BT_RX_PRIORITY(HW_GIO_TX_MID);

	HW_set_GIO_LOW_BT_RX_FREQ_OVERLAP(HW_OVERRIDE_LOW|HW_GIO_RX_END);
	HW_set_GIO_HIGH_BT_RX_FREQ_OVERLAP(HW_GIO_TX_MID);

	HW_set_GIO_LOW_RX_ACCESS(HW_OVERRIDE_LOW|HW_GIO_RX_END);
	HW_set_GIO_HIGH_RX_ACCESS(HW_GIO_TX_MID);

	HW_set_GIO_LOW_BT_TX_PRIORITY(HW_OVERRIDE_LOW|HW_GIO_TX_END);
	HW_set_GIO_HIGH_BT_TX_PRIORITY(HW_GIO_RX_MID);

	HW_set_GIO_LOW_BT_TX_FREQ_OVERLAP(HW_OVERRIDE_LOW|HW_GIO_TX_END);
	HW_set_GIO_HIGH_BT_TX_FREQ_OVERLAP(HW_GIO_RX_MID);

	HW_set_GIO_LOW_TX_ACCESS(HW_OVERRIDE_LOW|HW_GIO_TX_END);
	HW_set_GIO_HIGH_TX_ACCESS(HW_GIO_RX_MID);

#else

	HW_set_GIO_LOW_TX_ACCESS(HW_OVERRIDE_LOW|HW_GIO_TX_END);
	HW_set_GIO_HIGH_TX_ACCESS(HW_GIO_RX_MID);

#endif

	HW_set_GIO_LOW_BT_TX_REQ(HW_OVERRIDE_LOW|HW_GIO_TX_END);
	HW_set_GIO_HIGH_BT_TX_REQ(HW_GIO_RX_MID);

#if (HW_PTA_GRANT_TEST_ENABLED==1)
    HW_set_GIO_LOW_PTA_TX_GRANT_TEST(HW_OVERRIDE_LOW|HW_GIO_TX_END);
	HW_set_GIO_HIGH_PTA_TX_GRANT_TEST(HW_GIO_RX_MID);
#endif

}

#endif

/******************************************************************************
 *
 * FUNCTION: HWpta_Generic_BT_Access_Mode
 *
 *****************************************************************************/
void HWpta_Generic_BT_Access_Mode(boolean enabled)
{
	/*
	 * Generic BT_Access Mode combines BT_TX_ACCESS and BT_RX_ACCESS onto a
	 * common output BT_ACCESS (nominally BT_TX_ACCESS). This is done through
	 * use of the COMBINE feature within the Tabasco HW GIO configurations.
     */

   // USAGE (From Tabasco Spec for RF GIOs 0-11):
   //If rgf_gio_combinen[m] is set to 1 (where n, m are in the range 0 to 5),
   // the control line rdi_ctrl[6+m] will be OR-ed with rdi_ctrl[n] . The
   //composite signal will appear on the rdi_ctrl[n] pin.

   // USAGE (Deduced from the above for PTA GIOs 12-23):
   //If rgf_gio_combine(12+n)[m] is set to 1 (where n, m are in the range 0 to 5),
   // the control line rdi_ctrl[12+6+m] will be OR-ed with rdi_ctrl[12+n]. The
   //composite signal will appear on the rdi_ctrl[12+n] pin.

	// BT_TX_ACCESS = GIO 14 // PTA GIO 2
	// BT_RX_ACCESS = GIO 15 // PTA GIO 3
	// BT_TX_ACCESS_SHADOW= Not required
	// BT_RX_ACCESS_SHADOW= GIO 20 // m=2 with rdi_ctrl[12+6+m]

	mSetHWEntry(HAB_GIO_COMBINE_14, enabled?(1<<2):0);
}

/******************************************************************************
 *
 * FUNCTION: HWpta_Combine_BT_RX_REQ
 *
 *****************************************************************************/
void HWpta_Combine_BT_RX_REQ(boolean enabled)
{
	// combine BT_RX_REQ GIO 21 onto BT_REQ GIO 12 (BT_TX_REQ)
    mSetHWEntry(HAB_GIO_COMBINE_12, enabled?(1<<3):0);

#if (HW_PTA_GRANT_TEST_ENABLED==1)
	// combine PTA_RX_GRANT_TEST GIO 19 onto PTA_GRANT_TEST GIO 17
    mSetHWEntry(HAB_GIO_COMBINE_17, enabled?(1<<1):0);
#endif

    // combine BT_RX_PRIORITY GIO 22 onto BT_PRIORITY GIO 13
    mSetHWEntry(HAB_GIO_COMBINE_13, enabled?(1<<4):0);

    // combine BT_RX_FREQ_OVERLAP GIO 23 onto BT_FREQ_OVERLAP GIO 16
    mSetHWEntry(HAB_GIO_COMBINE_16, enabled?(1<<5):0);
}

#if (PRH_BS_CFG_SYS_HW_WINDOW_WIDENING_SUPPORTED==1)

/******************************************************************************
 *
 * FUNCTION: HWpta_WinExt_Adjust_En
 *
 *****************************************************************************/
void HWpta_WinExt_Adjust_En()
{
/*	BT_REQ (BT_TX_REQ)                    - gio[12] : 0
    BT_PRIORITY (BT_TX_PRIORITY)          - gio[13] : 0
    BT_TX_ACCESS                          - gio[14] : 0
    BT_RX_ACCESS                          - gio[15] : 1

	BT_FREQ_OVERLAP (BT_TX_FREQ_OVERLAP)  - gio[16] : 0
    PTA_GRANT_TEST  (PTA_TX_GRANT_TEST)   - gio[17] : 0
    BT_EPTA_REQ                           - gio[18] : NA
    PTA_RX_GRANT_TEST                     - gio[19] : 1

    BT_RX_ACCESS_SHADOW                   - gio[20] : 1
    BT_RX_REQ                             - gio[21] : 1
    BT_RX_PRIORITY                        - gio[22] : 1
    BT_RX_FREQ_OVERLAP                    - gio[23] : 1
*/
	HWpta_Set_GIOs_Hi_Early(0xFFF);
    //HWpta_Set_GIOs_Hi_Early(0xF88); // enable winext on all RX PTA singals - high early.
}

/******************************************************************************
 *
 * FUNCTION: HWpta_WinExt_Adjust_Dis
 *
 *****************************************************************************/
void HWpta_WinExt_Adjust_Dis()
{
	HWpta_Set_GIOs_Hi_Early(0); // disable high early winext on all PTA signals.
}

#endif

/******************************************************************************
 *
 * FUNCTION: HWpta_Extended_Access_Request
 *
 *****************************************************************************/
void HWpta_Extended_Access_Request(boolean priority, boolean freq_overlap,
    		  				u_int16 number_of_slots)
{
	/* TBD */
}

#endif // PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED

#endif
