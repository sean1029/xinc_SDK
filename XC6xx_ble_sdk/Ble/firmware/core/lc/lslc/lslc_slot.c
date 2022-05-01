/*************************************************************************
 *
 * MODULE NAME:    lslc_slot.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    Slot Handling Code to simplify USLC state machines for
 *                 page and inquiry 
 * MAINTAINER:     Ivan Griffin
 * CREATION DATE:  16 March 2000
 *
 * SOURCE CONTROL: $Id: lslc_slot.c,v 1.202 2014/03/11 03:13:43 garyf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *
 *    16 March 2000        IG          Initial Version
 *    20 March 2000        CM          State Machine Diagrams from USLCsched
 *    22 May   2000        JN          New multi-slot State Machines.
 *    02 Aug   2000        IG          Pass multi-slot info to HWradio
 *
 *
 * NOTES TO USERS:
 *
 * LSLCslot_Handler is always invoked by HWirq for the following interrupts:
 * TIM0, TIM2, PKA, PKD_RX_HDR, PKD, NO_PKD
 *
 * and is responsible for invoking the appropriate USLCsched
 * for a given interrupt.
 *
 * See USLCchan_ctrl/USLCsched for state machines on how LBLslot 
 * interacts with the USLC
 *
 ***************************************************************************/
#include "sys_config.h"
#include "sys_types.h"
#include "lc_types.h"

#include "lslc_slot.h"

#include "lslc_stat.h"
#include "lslc_access.h"
#include "lslc_pkt.h"
#include "lslc_hop.h"
#include "lslc_freq.h"
#include "lslc_irq.h"

#include "uslc_scheduler.h"
#include "uslc_chan_ctrl.h"
#include "uslc_page.h"
#include "uslc_inquiry.h"

#include "hw_lc.h"
#include "hw_jal_defs.h"
#include "hw_register.h"
#include "hw_radio.h"
#include "hw_delay.h"

#include "tc_event_gen.h"
#include "sys_mmi.h"
#include "lmp_timer.h"

#include "le_config.h"
#include "patch_function_id.h"
#if defined(SYS_HAL_OS_EVENT_DRIVEN_HOST_CONTROLLER)
#include "bt_mini_sched.h"
#endif

#include "le_const.h"

/*
 * Record request piconet change event for execution at next TIM0
 */
void LSLCslot_Change_To_Local_Piconet(void);
extern boolean _LSLCslot_local_piconet_request;

#if (PRH_BS_CFG_SYS_LOW_POWER_MODE_SUPPORTED==1)
#include "sys_power.h"
#endif

#if (PRH_BS_DEV_TIM3_IDENTIFIES_AR_ADDR_HALF_SLOT==1)
#include "uslc_park_master.h"
#endif

#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
#include "hw_pta.h"
#endif

#include    "le_power.h"
/*
 * States of Tx/Rx State Machine, numbers assigned for easier debug.
 */
typedef enum {
    W4_TX_START         = 0,
    TX_INTR_ERROR       = 1,
    W4_RX_START         = 2,
    RX_INTR_ERROR       = 3,
    W4_RX_INTR          = 4,

    W2_PREPARE_TX       = 5,
    W4_3SLOT_INTR_RX    = 6,
    W4_5SLOT_INTR_RX    = 7,
    W4_5SLOT_1ST_TIM2   = 8,
    W2_COMPLETE_RX      = 9,
    W2_SETUP_ENABLE_RXR = 10,
    W4_PKD_INTR         = 11,

    W2_ENABLE_RXR       = 12,
    W4_3SLOT_INTR_TX    = 13,
    W4_5SLOT_INTR_TX    = 14,
    W4_5SLOT_1ST_TIM0   = 15,
    W4_1ST_PKA_OF2      = 16,

    SCAN_ACTIVE         = 17,
    SCAN_CHECK          = 18,

    W4_FHS_2ND_HALF_TIM0 = 19,
    W4_FHS_2ND_HALF_PKD  = 20,

    W4_RADIO_TO_GO_INTO_STANDBY = 21,
    W4_INTRASLOT_OFFSET_ALIGNMENT = 22,

    W4_PKD_INTR_EXTENDED       = 23,

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    W4_ESCO_3SLOT_RX_START = 24,
    W2_ESCO_3SLOT_NO_PKD = 25,
    W2_ESCO_3SLOT_PKD_HDR = 26,
    NUM_TX_RX_STATES    = 27
#else
    NUM_TX_RX_STATES    = 24
#endif
    } t_tx_rx_state;

/*
 * On logging interrupt errors use the jalapeno defined mask
 */
#define   _LSLCslot_INTERRUPT_ID(Interrupt)  (JAL_##Interrupt##_INTR_MASK)

/*
 * Current Slot Transmit/Receive State
 */
extern t_tx_rx_state _LSLCslot_state;

#if (PRH_BS_CFG_TABASCO_VERSION >= PRH_BS_HW_TABASCO_VERSION_T1_0_0)
#if (PRH_BS_DEV_USE_T1_BT_CLOCK==1)
extern boolean freeze_bt_clk_on_next_tim2;
#endif
#endif

extern t_SYS_Config g_sys_config;

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
boolean _LSLCslot_eSCO_tx_3slot_packet_expected;
boolean _LSLCslot_eSCO_rx_3slot_packet_expected;
#endif

/*
 * If LSLC slot debugging enabled 
 */
#if (PRH_BS_DBG_LC_LSLC_SLOT==1)
/*
 * Counters to associate error interrupts and number of occurences 
 * Errored Interrupt Bit Vector      Currently 12 bits
 * Errored Interrupt Counter
 */
static u_int16       _LSLCslot_error_intr[NUM_TX_RX_STATES];
static u_int16       _LSLCslot_error_cntr[NUM_TX_RX_STATES];

#define LSLC_SLOT_NUM_INTR_LOGGED 128 /*Ensure power of 2 for fast log */
u_int16 _LSLCslot_interrupts[LSLC_SLOT_NUM_INTR_LOGGED];
u_int16 _LSLCslot_next_entry;

t_clock _LSLCslot_BT_Clk[LSLC_SLOT_NUM_INTR_LOGGED];
t_clock _LSLCslot_Native_Clk[LSLC_SLOT_NUM_INTR_LOGGED];


/*
 * Global variable to allow logging to be externally overriden
 */
boolean LSLCslot_Logging_Is_Active = TRUE;

/***************************************************************************
 * LSLCslot_Enable_Interrupt_Log_If_Active
 *
 ***************************************************************************/
void LSLCslot_Enable_Interrupt_Log_If_Active(void)
{    
     LSLCslot_Logging_Is_Active = TRUE;   
}

/***************************************************************************
 * LSLCslot_Disable_Interrupt_Log_If_Active
 *
 ***************************************************************************/
void LSLCslot_Disable_Interrupt_Log_If_Active(void)
{    
     LSLCslot_Logging_Is_Active = FALSE;   
}

/***************************************************************************
 * _LSLCslot_Log_Interrupt
 *
 * Log the interrupt id using the jalapeno defined mask (bottom 16 only)
 ***************************************************************************/
void _LSLCslot_Log_Interrupt(u_int16 interrupt)
{
    if (LSLCslot_Logging_Is_Active)
    {
     _LSLCslot_interrupts[_LSLCslot_next_entry] = interrupt;
     _LSLCslot_BT_Clk[_LSLCslot_next_entry] = HW_get_bt_clk();
     _LSLCslot_Native_Clk[_LSLCslot_next_entry] = HW_get_native_clk();
    _LSLCslot_next_entry = (_LSLCslot_next_entry+1) & 
                            ( sizeof(_LSLCslot_interrupts)/sizeof(_LSLCslot_interrupts[0]) -1 );
    }
}

/***************************************************************************
 * _LSLCslot_Process_Interrupt_Error
 *
 * Unexpected interrupt during TX, record corresponding bit and
 * increment the error counter.
 ***************************************************************************/
void _LSLCslot_Process_Interrupt_Error(u_int32 interrupt_id)
{
     /*
      * Handle any exceptions here 
      * RxS15/16 handled in LSLCslot_Handle_NO_PKD_RCVD
      */

     {
         _LSLCslot_error_intr[_LSLCslot_state] |= interrupt_id;
         ++_LSLCslot_error_cntr[_LSLCslot_state];
     }

     /*
      * Report Hardware Error
      */
}

#else
#define _LSLCslot_Log_Interrupt(interrupt)
#define _LSLCslot_Process_Interrupt_Error(interrupt_id)
#endif

/***************************************************************************
 * _LSLCslot_Handle_Rx_Interrupt_Error
 *
 * Handle PKD_RX_HDR, PKD, NO_PKD unexpected interrupts.
 ***************************************************************************/
#ifndef BLUETOOTH_MODE_LE_ONLY
void _LSLCslot_Handle_Rx_Interrupt_Error(u_int32 interrupt_id)
{
    switch (_LSLCslot_state)
    {
    case W4_RX_START:
    case W2_ENABLE_RXR:
    case W4_1ST_PKA_OF2:
    case W4_3SLOT_INTR_RX:
    case W2_SETUP_ENABLE_RXR:
    case W4_5SLOT_INTR_RX:
    case W4_5SLOT_1ST_TIM0:
        /*
         * TX_INTR_ERROR:  Transmit Slot Related Interrupts
         */
        _LSLCslot_Process_Interrupt_Error( interrupt_id );
        _LSLCslot_state = W4_RX_START;
        break;

    default:
         /*
          * RX_INTR_ERROR:  Receive Slot Related Messages
          *                 Valid interrupts handled in Event Handler
          */
        _LSLCslot_Process_Interrupt_Error( interrupt_id );
        _LSLCslot_state = W4_TX_START;
        break;
    }
}
#endif//BLUETOOTH_MODE_LE_ONLY
#ifndef REDUCE_ROM
/***************************************************************************
 * LSLCslot_Initialise
 *
 * Enable    TIM1 only
 * All other interrupts disabled by Hardware initialisation
 *
 * TIM1 will then be used to take up all other interrupts
 ***************************************************************************/
void LSLCslot_Initialise(void)
{
	if(PATCH_FUN[LSLCSLOT_INITIALISE_ID]){
         ((void (*)(void))PATCH_FUN[LSLCSLOT_INITIALISE_ID])();
         return ;
    }

	
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_IRQ_REG);

    mHWreg_Load_Cache_Register(JAL_COM_CTRL_IRQ_REG);

    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM1_INTR_MSK);
    mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);

    /*
     * Next interrupt after TIM1 is TIM2!
     */
    _LSLCslot_state = W4_RX_START;

#ifdef LSLC_SLOT_INTERRUPT_DEBUGGING
    {
        int i;
        for (i=0; i < NUM_TX_RX_STATES; i++)
        {
            _LSLCslot_error_intr[i] = 0;
            _LSLCslot_error_cntr[i] = 0;
        }
    }
#endif

    _LSLCslot_local_piconet_request = FALSE;

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    _LSLCslot_eSCO_tx_3slot_packet_expected = FALSE;
    _LSLCslot_eSCO_rx_3slot_packet_expected = FALSE;
#endif
}
#endif
#ifndef BLUETOOTH_MODE_LE_ONLY
/***************************************************************************
 * LSLCslot_Handle_PKA
 *
 * Process a PKA interrupt.
 ***************************************************************************/
void LSLCslot_Handle_PKA(void)
{
    t_TXRXstatus tx_status = TX_OK;


    /*
     * Log the interrupt/transmitted packet to baseband logger if active
     */
    TCeg_Send_Interrupt_Event(TCeg_INTR_PKA, (u_int8) _LSLCslot_state);
    TCeg_Send_BB_Event(0 /*tx0_rx1*/);
    
#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 0)
    ((void (*)(void))PATCH_FUN[HWRADIO_TXCOMPLETE])();//HWradio_TxComplete();    
#endif

#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
	HWpta_Service_PKA();
#endif

    switch (_LSLCslot_state)
    {
    /*
     * Normal events
     */
    case W2_ENABLE_RXR:
        USLCsched_Tx_Complete();
        LSLCacc_Enable_Receiver();
        _LSLCslot_state = W4_RX_START;
        break;

    case W4_1ST_PKA_OF2:
        _LSLCslot_state = W2_ENABLE_RXR;
        break;

    case W4_3SLOT_INTR_TX:
        USLCsched_Tx_Complete();
        _LSLCslot_state = W2_SETUP_ENABLE_RXR;
        break;

    case W4_5SLOT_INTR_TX:
        USLCsched_Tx_Complete();
        _LSLCslot_state = W4_5SLOT_1ST_TIM0;
        break;

    /*
     * Errored events
     */
    case W2_SETUP_ENABLE_RXR:           /* TX_INTR_ERROR               */
    case W4_5SLOT_1ST_TIM0:
    case W4_RX_START:
        _LSLCslot_Process_Interrupt_Error( _LSLCslot_INTERRUPT_ID( PKA ) );
        _LSLCslot_state = W4_RX_START;
        tx_status = TX_UNEXPECTED_ERROR;
        break;

    default:                            /* RX_INTR_ERROR               */
         /*
          * PKA should not occur in any RX state
          */
        _LSLCslot_Process_Interrupt_Error( _LSLCslot_INTERRUPT_ID( PKA ) );
        _LSLCslot_state = W4_TX_START;
        tx_status = TX_UNEXPECTED_ERROR;
        break;
    }
    
    _LSLCslot_Log_Interrupt( _LSLCslot_INTERRUPT_ID( PKA ) );

#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 1)
    LSLCirq_Unmask_PKA();
#endif
}

/***************************************************************************
 * LSLCslot_Handle_PKD
 *
 * Process the packet received complete interrupt.
 ***************************************************************************/
void LSLCslot_Handle_PKD(void)
{
    t_TXRXstatus rx_status = RX_OK;

#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
    if (HW_get_page() && HW_get_pta_grant_rx_denied())
	{
    	//t_Receiver_Mode lc_rx_mode = HW_get_rx_mode();
		//HW_set_rx_mode(RXm_RECEIVER_DISABLED);
		LSLCstat_Inc_Rx_Restricted_Packet_Cntr();
		//HWdelay_Wait_For_us(1);
		//HW_set_rx_mode(lc_rx_mode);
		return; // no other action required on IDpkt denied
	}
#endif

    #if (PRH_BS_DEV_EARLY_PREPARE_PAGE_ID_FHS_TURNAROUND==1)
    if((W4_RX_INTR==_LSLCslot_state) && (USLCchac_get_device_state()==Page))
    {
        USLCpage_Potential_Early_Prepare_MasterPageResponse();
    }
#endif


#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 0)
    /*
     * cancel extension of receive slot for possible FHS in second half
     * slot
     */
    HWradio_RxComplete();
#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
    HWpta_Rx_Access_Complete();
#endif
#endif

    /*
     * Log interrupt/received packet to baseband logger if active
     */
    TCeg_Send_Interrupt_Event(TCeg_INTR_PKD, (u_int8) _LSLCslot_state);
    TCeg_Send_BB_Event(1 /*tx0_rx1*/);

    switch (_LSLCslot_state)
    {
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
        case W4_ESCO_3SLOT_RX_START:
        /*
         * An AM_ADDR_ABORT must have been detected to get into this state.
         * If an AM_ADDR_ABORT is detected in the reserved slot of an EV4/5
         * link, then the remainder of the current eSCO window is invalid
         * (i.e. the slave can not transmit in the reserved slot and the
         * master will not transmit in the retransmission window.
         * Therefore, handle this scenario in the same manner than an EV3
         * link is handled.  For an EV3 link the current 
         * _LSLCslot_state = W4_PKD_INTR
         */
        /*
         * Fall through and handle as for any received packet
         */
#endif
    /*
     * Normal events
     */
    case SCAN_ACTIVE:
//        SYSmmi_Display_Event(eSYSmmi_Scan_Inactive_Event);
        /*
         * Fall through and handle as for any received packet
         */
    case W4_RX_INTR:

        /*
         * Must be an ID packet!  Ensure that we don't receive the 2nd ID
         * if in fixed frequency
         */
        LSLCstat_Log_Rx_Packet(RX_OK);

        if(USLCchac_get_device_state()==Page_Scan)
        {
            LSLCfreq_Early_Prepare_TxRxFreqs_SlavePageResponse();
        }
        
        /*
         * Fall through and handle as for any received packet
         */
    case W4_PKD_INTR:
        /*
         * Receiver must be disabled for bt_clk updating
         * Adopt practice of disabling on any packet received.
         */
        LSLCacc_Disable_Receiver();
        LSLCpkt_Process_Rx_Packet();
        USLCsched_Process_Rx();
#if 0
        LSLCacc_Enable_Transmitter();
        _LSLCslot_state = W4_TX_START;
#endif
        if (LE_LL_Is_LE_Active())
		{
			u_int32 current_clk = HW_Get_Native_Clk_Avoid_Race() & 0xFFFFFFFE;
			u_int8 next_le_state = LE_LL_LE_Event_Due(current_clk+8);
			u_int8 current_le_state = LE_LL_Get_Current_State();
      	    u_int8 slots_to_le = LE_LL_Slots_To_Next_LE_Activity((current_clk-8)&0xFFFFFFFE);
    		t_slots num_slots = LMscoctr_Check_Slots_to_next_eSCO_On_Device(current_clk);


         	if ( /* Next State Connection */
         			(( next_le_state == CONNECTION_STATE ) && (slots_to_le <= 1) ) ||
         	        /* Next State other LE state and SCO active */
        	        ((next_le_state != STANDBY_STATE)     && (num_slots == 0x04) && (slots_to_le <= 1) ) ||
        	        /* Current LE state not Standby and SCO active */
        	        ((current_le_state != STANDBY_STATE)  && (num_slots == 0x04)) )
			{

        		t_devicelink *p_device_link = LSLCpkt_Get_Active_Rx_Device_Link_Ref();

        	    //HWradio_DisableAllSpiWrites();
				//HWradio_DisableAllSpiWrites();
		        ((void (*)(void))PATCH_FUN[HWRADIO_DISABLEALLSPIWRITES])();	
        	    HWle_set_le_spi_only();
        	#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 0)
        	   //     HWradio_RxComplete();
        	#endif
        		LEconfig_Switch_To_LE(p_device_link->device_index);
        	    LE_LL_Ctrl_Handle_LE_IRQ(LE_TIM_2);

        	    // Replace the return here -- this is sensitive.
            	return;
        	}
		}

        LSLCacc_Enable_Transmitter();
        _LSLCslot_state = W4_TX_START;

        break;

    case W4_FHS_2ND_HALF_PKD:
    case W4_PKD_INTR_EXTENDED:
        /*
         * Inquiry FHS received in 2nd half of Rx slot and during normal Tx slot
         * Receiver was enabled if required during TIM0 processing
         */
        LSLCacc_Disable_Receiver();
        LSLCpkt_Process_Rx_Packet();
        USLCsched_Process_Rx();
        USLCsched_Tx_Start(); //Deferred from TIM0
        LSLCacc_Enable_Receiver();
        _LSLCslot_state = W4_RX_START;
        break;

    case W4_3SLOT_INTR_RX:
        _LSLCslot_state = W2_COMPLETE_RX;
        break;

    case W4_5SLOT_INTR_RX:
        _LSLCslot_state = W4_5SLOT_1ST_TIM2;
        break;

    /*
     * Error events
     */
    case W2_COMPLETE_RX:                /* RX_INTR_ERROR               */
    case W4_5SLOT_1ST_TIM2:
    case W4_TX_START:
        _LSLCslot_Process_Interrupt_Error( _LSLCslot_INTERRUPT_ID( PKD ) );
        _LSLCslot_state = W4_TX_START;
        rx_status = RX_UNEXPECTED_ERROR;
        break;

    default:                            /* TX_INTR_ERROR               */
         _LSLCslot_Process_Interrupt_Error( _LSLCslot_INTERRUPT_ID( PKD ) );
         _LSLCslot_state = W4_RX_START;
         rx_status = RX_UNEXPECTED_ERROR;
        break;
    }
    _LSLCslot_Log_Interrupt( _LSLCslot_INTERRUPT_ID( PKD ) );

#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 1)
    LSLCirq_Unmask_PKD();
#endif
}

/***************************************************************************
 * LSLCslot_Handle_NO_PKT_RCVD
 *
 * No packet received during current receive slot.
 *
 * if (eSCO is supported)  &&
 *    (Reserved slot) && 
 *    (EV4/EV5 packet was expected (ie _LSLCslot_state=W4_ESCO_3SLOT_RX_START))
 * then the following BT Spec requirement applies:
 *      If the slave does not receive anything in the reserved master-to-slave 
 *    transmission slot it shall transmit in the same slot as if the master 
 *    had transmitted the negotiated packet type.
 *
 * To abstract this requirement from the higher layers, the "passing" of the 
 * NO_PKD_INTR to the higher layers will be delayed by one frame.
 *
 ***************************************************************************/
void LSLCslot_Handle_NO_PKT_RCVD(void)
{
    /*
     * Log interrupt if active
     */
    TCeg_Send_Interrupt_Event(TCeg_INTR_NO_PKT_RCVD, (u_int8) _LSLCslot_state);

    switch (_LSLCslot_state)
    {
    case W4_RX_INTR:
    {

       if (LE_LL_Is_LE_Active())
		{
			u_int32 current_clk = HW_Get_Native_Clk_Avoid_Race() & 0xFFFFFFFE;
			u_int8 next_le_state = LE_LL_LE_Event_Due(current_clk+8);
			u_int8 current_le_state = LE_LL_Get_Current_State();
      	    u_int8 slots_to_le = LE_LL_Slots_To_Next_LE_Activity((current_clk-8)&0xFFFFFFFE);
    		t_slots num_slots = LMscoctr_Check_Slots_to_next_eSCO_On_Device(current_clk);


         	if ( /* Next State Connection */
         			(( next_le_state == CONNECTION_STATE ) && (slots_to_le <= 1) ) ||
         	        /* Next State other LE state and SCO active */
        	        ((next_le_state != STANDBY_STATE)     && (num_slots == 0x04) && (slots_to_le <= 1) ) ||
        	        /* Current LE state not Standby and SCO active */
        	        ((current_le_state != STANDBY_STATE)  && (num_slots == 0x04)) )
			{

        		t_devicelink *p_device_link = LSLCpkt_Get_Active_Rx_Device_Link_Ref();

        	    //HWradio_DisableAllSpiWrites();
				//HWradio_DisableAllSpiWrites();
                ((void (*)(void))PATCH_FUN[HWRADIO_DISABLEALLSPIWRITES])();	
				
        	    HWle_set_le_spi_only();
        	#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 0)
        	   //     HWradio_RxComplete();
        	#endif
				LSLCpkt_Process_No_Rx_Packet();
        	    LSLCacc_Disable_Receiver();
        		LEconfig_Switch_To_LE(p_device_link->device_index);
        	    LE_LL_Ctrl_Handle_LE_IRQ(LE_TIM_2);

        	    // Replace the return here -- this is sensitive.
            	return;
        	}
		}

            /*
             *  Check if the PKD interrupt has already occured.
             *  This is to solve PKD & NO_PKD coming together.#1742
             */
            if (HW_get_page())
            {
#if 1 // GF Temp Removed - 18th Feb - for test of DM release - Seems un-necessary.. 
                HWdelay_Wait_For_us(5);
#endif
                if (HW_get_pkd_intr())
                {
#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 1)
                    LSLCirq_Unmask_NO_PKT_RCVD();
#endif
                    return;
                }
            }
#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 0)
            HWradio_RxComplete();
#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
            HWpta_Rx_Access_Complete();
#endif
#endif
            LSLCacc_Disable_Receiver();
            LSLCpkt_Process_No_Rx_Packet();
            USLCsched_Process_Rx();
            USLCsched_Prepare_Tx();
            LSLCacc_Enable_Transmitter();
            _LSLCslot_state = W4_TX_START;
        }
        break;

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    case W4_ESCO_3SLOT_RX_START:
#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 0)
            HWradio_RxComplete();
#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
            HWpta_Rx_Access_Complete();
#endif
#endif
            /*
             * The following tasks will be performed 
             * once the NO_PKD_INTR is to be passed
             * to the higher layers.
             * LSLCpkt_Process_No_Rx_Packet();
             * USLCsched_Process_Rx();
             * USLCsched_Prepare_Tx();
             * LSLCacc_Enable_Transmitter();
             */
            LSLCacc_Disable_Receiver();
            _LSLCslot_state = W2_ESCO_3SLOT_NO_PKD;
        break;
#endif

    case SCAN_ACTIVE:
        /*
         * Ignore NO_PKT_RCVD interrupts
         */
        break;

    /*
     * Error events
     */
    case W4_PKD_INTR:
        /*
         * In FHS packets a NO_PKD can occur between PKD_RX_HDR and PKD
         */
        break;

    default:
        /*
         * Ignore NO_PKD_RCVD except where expected in W4_RX_INTR.
         */
        break;
    }
   _LSLCslot_Log_Interrupt( _LSLCslot_INTERRUPT_ID( NO_PKT_RCVD ) );
   
#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 1)
    LSLCirq_Unmask_NO_PKT_RCVD();
#endif
}

/***************************************************************************
 * LSLCslot_Handle_PKD_RX_HDR
 *
 * Interrupt indicates that we have received a packet header.
 * R [5.7.81] If a valid packet header is received with no errors, the RX_HDR
 * bit will be asserted. PKD_RX_HDR_INTR will also be asserted.
 ***************************************************************************/
void LSLCslot_Handle_PKD_RX_HDR(void)
{
    /*
     * Extract packet type from Hardware LC
     */
    t_packet rx_packet_type;

#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 0)
    /*
     *  Check if the PKD interrupt has already occured. This is required for 
     *  low Mips platforms.
     */
    if (HW_get_pkd_intr())
    {
        HWradio_RxComplete();
#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
        HWpta_Rx_Access_Complete();
#endif
    }
#endif

    /*
     * Log interrupt if active
     */
    TCeg_Send_Interrupt_Event(TCeg_INTR_PKD_RX_HDR, (u_int8) _LSLCslot_state);

    /*
     * Process the Packet Header
     */
    LSLCpkt_Process_Rx_Packet_Header();

    /*
     * If an error then abort interrupt processing
     */
    
    if (HW_get_hec_err())
    {
        LSLCstat_Log_Rx_Packet(RX_HEC_ERROR);
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
        if (W4_ESCO_3SLOT_RX_START == _LSLCslot_state)
        {
             _LSLCslot_state = W4_3SLOT_INTR_RX;
             /*
              * CM: 
              *
              * Not sure if the following should be called here:
              *    LSLCirq_Unmask_PKD_RX_HDR();
              * 
              * Currently this will be called in slot position=3
              */
        }
        else
#endif
        {
             USLCsched_Prepare_Tx();
             _LSLCslot_state = W4_PKD_INTR;        
        }
#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 1)
        LSLCirq_Unmask_PKD_RX_HDR();
#endif  
        return;
    }

    /*
     * allow radio to extend for multi-slot rx if required
     */
    if (HW_get_page())
    {
        rx_packet_type = IDpkt;

    }
    else
    {
        rx_packet_type = (t_packet) HW_get_rx_type();
    }

    switch (_LSLCslot_state)
    {
    case SCAN_ACTIVE:
        /*
         * Handle Return to Piconet, deliberate fall through
         */
    case W4_RX_INTR:
        /*
         * Setup the outgoing frequency if Rx FHS in advance and not MSS.
         * This is necessary since response ID is sent on PKD
         */
        if (rx_packet_type == FHSpkt) 
        {
            /*
             * Inquiry:  FHS can occur in 2nd Rx half BT_CLK[1:0] = '11'
             *           If a 2nd half slot FHS packet then treat as special
             *           one slot rx packet (ignoring the TIM0).
             * Pagescan: Always will have a first half FHS
             *
             * Low MIPS platforms require half slot check before prepare frequency,
             * as processing may run into next half slot.
             *
             */
            boolean second_half_slot = HW_get_bt_clk()&0x01;

            LSLCfreq_Early_Prepare_Frequency_FHS_Response_ID();
            USLCsched_Prepare_Tx();

            if ((second_half_slot) && (HW_get_slave()==0))
            {
                _LSLCslot_state = W4_FHS_2ND_HALF_TIM0;
            }
            else
            {
                _LSLCslot_state = W4_PKD_INTR;
            }
            LSLCstat_Log_Rx_Packet(RX_OK);
        }
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_INQUIRY_RESPONSE_SUPPORTED==1)
        else if (USLCinq_Is_Extended_Inquiry_Response_Frame())
        {
            USLCsched_Prepare_Tx();
            _LSLCslot_state = W4_PKD_INTR_EXTENDED;
            LSLCstat_Log_Rx_Packet(RX_OK);
        }
#endif
        else if (is_Packet_1_Slot(rx_packet_type) || is_eSCO_Packet_1_Slot(rx_packet_type))
        {
            USLCsched_Prepare_Tx();
            _LSLCslot_state = W4_PKD_INTR;
            LSLCstat_Log_Rx_Packet(RX_OK);
        }
        else if (is_Packet_3_Slot(rx_packet_type) || is_eSCO_Packet_3_Slot(rx_packet_type))
        {
            _LSLCslot_state = W4_3SLOT_INTR_RX;
            LSLCstat_Log_Rx_Packet(RX_OK);
        }
        else if (is_Packet_5_Slot(rx_packet_type))
        {
            _LSLCslot_state = W4_5SLOT_INTR_RX;
            LSLCstat_Log_Rx_Packet(RX_OK);
        }
        break;

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
        /*
         * CM: 22 SEP
         * Optimise this code: Lots in common with W4_RX_INTR
         *
         */
    case W4_ESCO_3SLOT_RX_START:
            if (HW_get_rx_am_addr() == HW_get_esco_lt_addr())
            {
                /* 
                 * POLL, NULL packets received in this state will be 
                 * handled as 3-slot packets.  
                 */
                 _LSLCslot_state = W4_3SLOT_INTR_RX;
                 LSLCstat_Log_Rx_Packet(RX_OK);
            }
            else
            {
                /* 
                 * ACL/SCO packet received, Handle as per normal
                 */
                if (is_Packet_1_Slot(rx_packet_type))
                {
                    USLCsched_Prepare_Tx();
                    _LSLCslot_state = W4_PKD_INTR;
                    LSLCstat_Log_Rx_Packet(RX_OK);
                }
                else if (is_Packet_3_Slot(rx_packet_type))
                {
                    _LSLCslot_state = W4_3SLOT_INTR_RX;
            USLCsched_Update_Chac_Procedure_If_Req();
                    LSLCstat_Log_Rx_Packet(RX_OK);
                }
                else if (is_Packet_5_Slot(rx_packet_type))
                {
                    _LSLCslot_state = W4_5SLOT_INTR_RX;
            USLCsched_Update_Chac_Procedure_If_Req();
                    LSLCstat_Log_Rx_Packet(RX_OK);
                }
            }
            break;
#endif

   case W4_RX_START:

        /****************************************************************
         * Handle FHS reception in Tx Slot
         *
         *   |------|      |------|      |------| CASE 1: FHS Pkt received in
         *   |      |? *  |^     |      |      | 1st half of Rx slot.
         *   |      |______|      |______|      | ID pkt transmitted in the
         *   0  1   2  3   0  1   2  3   0  1   2 next Tx slot.
         *
         *   |------|      |------|      |------|  CASE 2: FHS Pkt received in
         *   |      |   ? |*     |      |^     |  2nd half of Rx slot (only
         *   |      |______|      |______|      |  in state==Inquiry)
         *   0  1   2  3   0  1   2  3   0  1   2  ID pkt transmitted in the
         *                                         next Tx slot (i.e. 1 frame
         *              <--------------->          later than in Case 1).
         *
         *   ^=PKA_INTR   ?PKD_RX_HDR_INTR    *=PKD_INTR
         *
         ****************************************************************/
        if ((rx_packet_type == FHSpkt))
        {
             LSLCfreq_Early_Prepare_Frequency_FHS_Response_ID();
            _LSLCslot_state = W4_PKD_INTR;
             break;
        }
        else
        {
             /*
              * Fall through to default error handler.
              */
        }

    default:
        _LSLCslot_Handle_Rx_Interrupt_Error(_LSLCslot_INTERRUPT_ID( PKD_RX_HDR ));
        break;
    }
    _LSLCslot_Log_Interrupt( _LSLCslot_INTERRUPT_ID( PKD_RX_HDR ) );
    
#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 1)
    LSLCirq_Unmask_PKD_RX_HDR();
#endif
}

/***************************************************************************
 *
 * LSLCslot_Handle_TIM0
 * Determine slot behaviour on receiving TIM0
 *
 * ISSUE  (Resolve.....) :
 * 1. In the current design the receiver is enabled at TIM_INTR0 if the
 *    following conditions apply:
 *      - Device is in any state other than Standby.
 *      - There is nothing to transmit in the current Tx slot.
 *    However, what is the impact of enabling the Receiver once a frame
 *    when the device is Page/Inquiry Scanning (i.e. RxMode=3) ?
 *
 *    If a device is a Master, the Receiver should not be enabled if there
 *    is nothing to transmit in the current Tx slot (i.e. this avoids the
 *    recpetion of spurious packets). However, according to the BT spec
 *    even if a Master does not transmit a SCO packet to a Slave, the Slave
 *    is entitled to transmit (at the appropriate slot) a SCO packet to the
 *    Master.  Therefore, the following requirement may have to be placed on
 *    the SCO object :
 *          - If, in a given 'SCO frame' the Master has no information (SCO/
 *          DM1) to send to the Slave, then a NULL packet should be sent
 *          instead.
 *
 *    If a device is a Slave, the Receiver should be enabled for every Rx slot
 *    while a connection is established (i.e. the Slave does not know when the
 *    Master will transmit a packet to the Slave - so he has to be prepared to
 *    receive a packet at any time (except for Hold, Park conditions).
 *
 ***************************************************************************/
void LSLCslot_Handle_TIM0(void)
{
    t_state  context;
    t_packet tx_packet_type;
    t_Transmitter_Mode  tx_mode;

    /*
     * Log interrupt if active
     */
    TCeg_Send_Interrupt_Event(TCeg_INTR_TIM0, (u_int8) _LSLCslot_state);

    if (_LSLCslot_local_piconet_request)
    {
        _LSLCslot_local_piconet_request = FALSE;
        LSLCslot_Change_To_Local_Piconet();
        _LSLCslot_Log_Interrupt( _LSLCslot_INTERRUPT_ID( TIM0 ) );
#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 1)
        LSLCirq_Unmask_Tim0();
#endif
        return;
    }
    
#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 0)
    HWradio_Setup_Radio_For_Next_Half_Slot(TX_MID);
#endif

#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
    HWpta_Service_Tx_Start();
#endif

    /*
     * After SlavePageResponse ID, PageScan X is incremented on every TIM0.
     * The check below is required as the X is frozen at the start of the 
     * scan and timO intr can come multiple times before receiving the 
     * ID packet.  To ensure that the correct frozen X is used, the guard
     * below is required.
     */
    if (USLCchac_get_device_state() == SlavePageResponse)
    {
        LSLChop_Increment_X_Page_Scan();
    }

   switch (_LSLCslot_state)
    {
    /*
     * Normal events
     */
    case W4_TX_START:
        tx_mode = (t_Transmitter_Mode) HW_get_tx_mode();
        switch (tx_mode)
        {
        case TXm_NORMAL:
        case TXm_2ND_HALF_SLOT:
            if (HW_get_page() ) /* IDpkt */
            {
                 USLCsched_Tx_Start();
                 _LSLCslot_state = W2_ENABLE_RXR;
            }
            else
            {
                tx_packet_type = (t_packet) HW_get_tx_type();
#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 0)
                if (is_Packet_Multi_Slot(tx_packet_type) || 
                    is_eSCO_Packet_3_Slot(tx_packet_type))
                {
                    HWradio_Tx_PacketExtend();
                }
#endif
                if (is_Packet_1_Slot(tx_packet_type) || 
                    is_eSCO_Packet_1_Slot(tx_packet_type))
                {

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
                    if (_LSLCslot_eSCO_tx_3slot_packet_expected)
                    {
                        /*
                         * In the scenario where the Master is replacing an EV4/EV5 with
                         * a POLL packet, then the LSLCslot must be instructed to 
                         * "fake" a 3 slot transmission (from an INTR generation point
                         * of view) so that the Receiver in the Master is enabled
                         * at the right point (ie 3 slots after the transmission of the
                         * POLL packet).
                         */
                        _LSLCslot_state = W4_3SLOT_INTR_TX;
                        _LSLCslot_eSCO_tx_3slot_packet_expected = FALSE;
                    }
                    else
#endif
                    {
                        USLCsched_Tx_Start();
                        _LSLCslot_state = W2_ENABLE_RXR;
                    }
                }
                else if (is_Packet_3_Slot(tx_packet_type) || 
                         is_eSCO_Packet_3_Slot(tx_packet_type))
                {
                    _LSLCslot_state = W4_3SLOT_INTR_TX;
                }
                else if (is_Packet_5_Slot(tx_packet_type))
                {
                    _LSLCslot_state = W4_5SLOT_INTR_TX;
                }
            }
            LSLCstat_Log_Tx_Packet(TX_OK);
            break;

        case TXm_TRANSMITTER_DISABLED:   /* !is_packet_to_transmit */
/*
 * 30 May Temporary fix for when in Inquiry and TIM0 is delayed due to FHS processing.
 * TX is being disabled before interrupt is received.
 */
            {
                t_state context = USLCchac_get_device_state();
                if (context != Inquiry)
                {
                    USLCsched_Tx_Start();
                    LSLCacc_Enable_Receiver();
                    /*
                     * Check if we have just started scanning and go to SCAN_ACTIVE
                     */
                     context = USLCchac_get_device_state();
                     if ( (context == Page_Scan) || (context == Inquiry_Scan) || (context == InquiryResponse)
                        ||(context == Return_To_Piconet) )
                          _LSLCslot_state = SCAN_ACTIVE;
                     else
                        
                         _LSLCslot_state = W4_RX_START;
                    break;
                }
                else
                {
                    /* Fall through to what would have been there */
                }
            }
        case TXm_SLAVE_1ST_HALF__MASTER_BOTH:
             USLCsched_Tx_Start();
#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 1)
            /* The PKA dsr is only called once during page/inquiry. */
            _LSLCslot_state = W2_ENABLE_RXR;
#else
            _LSLCslot_state = W4_1ST_PKA_OF2;
#endif
            LSLCstat_Log_Tx_Packet(TX_OK);
            break;
        }
        break;

    case SCAN_ACTIVE:
        USLCsched_Tx_Start();
        /*
         * Lambda transition to SCAN_CHECK (valid switch fall_through)
         */
    case SCAN_CHECK:
        context = USLCchac_get_device_state();

    /*
     * IF Not Scanning Then Back to normal
     *                 Else Stay in SCAN_ACTIVE state
     *                 Endif
     */
        if ( (context != Page_Scan) && (context != Inquiry_Scan) && (context != InquiryResponse)
             && (context != Return_To_Piconet) )
        {
             LSLCacc_Enable_Receiver();
//             SYSmmi_Display_Event(eSYSmmi_Scan_Inactive_Event);
             _LSLCslot_state = W4_RX_START;
        }
        break;

    case W4_3SLOT_INTR_RX:
    case W4_5SLOT_INTR_RX:
    case W4_5SLOT_1ST_TIM2:
    case W2_COMPLETE_RX:
        /*
         * Ignore, no state change for Rx related states.
         */
        break;

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    case W2_ESCO_3SLOT_NO_PKD:
    case W2_ESCO_3SLOT_PKD_HDR:
        /*
         * Ignore, no state change.
         */
        break;
#endif

    case W4_3SLOT_INTR_TX:
        USLCsched_Tx_Start();
        _LSLCslot_state = W2_ENABLE_RXR;
        break;

    case W2_SETUP_ENABLE_RXR:
        USLCsched_Tx_Start();
        LSLCacc_Enable_Receiver();
        _LSLCslot_state = W4_RX_START;
        break;

    case W4_5SLOT_INTR_TX:
        _LSLCslot_state = W4_3SLOT_INTR_TX;
        break;

    case W4_5SLOT_1ST_TIM0:
        _LSLCslot_state = W2_SETUP_ENABLE_RXR;
        break;

    case W4_FHS_2ND_HALF_TIM0:
        /*USLCsched_Tx_Start(); //Defer to 2ND_HALF_PKD */
        _LSLCslot_state = W4_FHS_2ND_HALF_PKD;
        break;

    case W4_PKD_INTR_EXTENDED:
        /* Multislot/2nd Half EIR Scenario */
        break;

    /*
     * Errored events    All TIM0's should resume at next W4_RX_START
     */

    default:                                /* TX_INTR_ERROR               */
        /*
         * case W4_RX_INTR, W4_PKD_INTR, W4_1ST_PKA_OF2, W2_ENABLE_RXR, W4_RX_START
         */
        _LSLCslot_Process_Interrupt_Error( _LSLCslot_INTERRUPT_ID( TIM0 ) );

        /*
         * Always Process TIM0 as normal by recalling!
         */
        _LSLCslot_state = W4_TX_START;
        LSLCslot_Handle_TIM0();
        return;
    }

#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
	if (HW_get_pta_grant_tx_denied()) 
	{ // requires here as pka will not occur...
		((void (*)(void))PATCH_FUN[HWRADIO_TXCOMPLETE])();//HWradio_TxComplete();
		LSLCacc_Disable_Transmitter();

		USLCsched_Tx_Complete();

		if (_LSLCslot_state == W2_ENABLE_RXR)
		{
			LSLCacc_Enable_Receiver();
			_LSLCslot_state = W4_RX_START;
		}
	}
#endif

   _LSLCslot_Log_Interrupt( _LSLCslot_INTERRUPT_ID( TIM0 ) );

   
#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 1)
    LSLCirq_Unmask_Tim0();
#endif
}


/***************************************************************************
 * LSLCslot_Handle_TIM1
 *
 * Determine slot behaviour on receiving TIM1
 *
 * Used to take up the rest of interrupts and to start in
 * the known state W4_RX_START i.e. Wait for a TIM2
 *
 * Enable    TIM0, TIM2, PKD, NO_PKD, PKD_RX_HDR, PKA
 * Disable   TIM1, TIM3
 ***************************************************************************/
void LSLCslot_Handle_TIM1(void)
{
    static u_int8 slot_sequencer_running = 0;

    /*
     * Log interrupt if active
     */
    TCeg_Send_Interrupt_Event(TCeg_INTR_TIM1, (u_int8) _LSLCslot_state);

#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 0)
    HWradio_Setup_Radio_For_Next_Half_Slot(RX_START);
#endif

#if (PRH_BS_DEV_TIM1_CORRECTED_PAGESCAN_WIDTHS==1)
	if (USLCchac_Get_Active_Procedure()==PAGESCAN_PROCEDURE)
	{ /* LOOK_AHEAD signals special TIM1 case to the state machine */
		USLCpageScan_Page_Scan(LOOK_AHEAD);
	}
#endif

    if (!slot_sequencer_running)
    {
        mHWreg_Create_Cache_Register(JAL_COM_CTRL_IRQ_REG);

        mHWreg_Load_Cache_Register(JAL_COM_CTRL_IRQ_REG);

        mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM0_INTR_MSK);
        mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM2_INTR_MSK);

#if (TABASCO_OLD_SER_BLK_SUPPORTED==1)
        /*
         * Until SUPER_SER arrives, TIM1/TIM3 required for SiWave
         */
        mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM1_INTR_MSK);
        mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM3_INTR_MSK);
#else
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM1_INTR_MSK);
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM3_INTR_MSK);
#endif

        mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_NO_PKT_RCVD_INTR_MSK);
        mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKD_RX_HDR_INTR_MSK);
        mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKD_INTR_MSK);
        mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKA_INTR_MSK);

        mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);

        slot_sequencer_running = 1;
    }
   _LSLCslot_Log_Interrupt( _LSLCslot_INTERRUPT_ID( TIM1 ) );
   
#if defined(SYS_HAL_OS_EVENT_DRIVEN_HOST_CONTROLLER)
   if(LMtmr_Timer_Expired())
   {
       BTms_OS_Post(BTMS_OS_EVENT_EXPIRED_TIMERS);
   }
   if((g_LM_config_info.default_sco_route_via_hci == 0) && LMscoctr_Get_Number_SYN_Connections())
   {
	   //u_int8 TRAcodec_Get_FIFO_Sample_Size(void);
	   u_int8 vci_sample_size = 2; //TRAcodec_Get_FIFO_Sample_Size();
	   if (HW_get_vci_tx_fifo_fill_level() >= (4/vci_sample_size))
	   {
    	   BTms_OS_Post(BTMS_OS_EVENT_SCO_OUT_QUEUE);
	   }
   }
#endif
   
#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 1)
    LSLCirq_Unmask_Tim1();
#endif
}

/***************************************************************************
 * LSLCslot_Handle_TIM3
 *
 * Determine slot behaviour on receiving TIM3 (Currently None)
 ***************************************************************************/
void LSLCslot_Handle_TIM3(void)
{
    /*
     * Log interrupt if active
     */
    TCeg_Send_Interrupt_Event(TCeg_INTR_TIM3, (u_int8) _LSLCslot_state);
    
    if ( W4_INTRASLOT_OFFSET_ALIGNMENT == _LSLCslot_state )
    {
        LSLCirq_R2P_Turn_On_Intr();
        _LSLCslot_state = W4_TX_START;
    }
    else 
    {
#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 0)
        HWradio_Setup_Radio_For_Next_Half_Slot(TX_START);
#endif

        if ((W4_RX_INTR == _LSLCslot_state) && (RXm_SINGLE_WIN == HW_get_rx_mode())
             && (RADIO_MODE_TX_RX == HWradio_GetRadioMode()))
        {
            /*
             * Handle scenario of no rx interrupts eventhough rx mode enabled.
             * Can occur where rx mode enabled too late for RSM due to mips issue.
             * Process as no rx packet to prepare NAK for peer retansmission.
             */
            HWradio_RxComplete();
#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
            HWpta_Rx_Access_Complete();
#endif
            LSLCacc_Disable_Receiver();
            LSLCpkt_Process_No_Rx_Packet();
            USLCsched_Process_Rx();
            USLCsched_Prepare_Tx();
            LSLCacc_Enable_Transmitter();
            _LSLCslot_state = W4_TX_START;
        }

#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
#if (PRH_BS_DEV_TIM3_IDENTIFIES_AR_ADDR_HALF_SLOT==1)
    else if( (USLCchac_Get_Active_Procedure()==PARKMASTER_PARK_PROCEDURE)
     && (g_LM_config_info.park_info.park_state==LMP_ACCESS_WINDOW_ACTIVE))
    {
#if (PRH_BS_DEV_USE_ODD_AR_ADDR_ONLY==1)
            HWradio_RxComplete();
#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
            HWpta_Rx_Access_Complete();
#endif
            LSLCacc_Disable_Receiver();
            LSLCpkt_Process_No_Rx_Packet();
            USLCsched_Process_Rx();
            USLCsched_Prepare_Tx();
            LSLCacc_Enable_Transmitter();
            _LSLCslot_state = W4_TX_START;        
#endif
            USLCparkMaster_Second_Half_Slot();
    }
#endif
#endif

    }
    
    _LSLCslot_Log_Interrupt( _LSLCslot_INTERRUPT_ID( TIM3 ) );
    
#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 1)
    LSLCirq_Unmask_Tim3();
#endif
}

/***************************************************************************
 * LSLCslot_Handle_TIM2
 *
 * Determine slot behaviour on receiving TIM2
 ***************************************************************************/
void LSLCslot_Handle_TIM2(void)
{
    t_Receiver_Mode  rx_mode;
    u_int scanning;

    t_state context = USLCchac_get_device_state();
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    boolean    _LSLCslot_eSCO_rx_3slot_packet_expected_cache = 
        _LSLCslot_eSCO_rx_3slot_packet_expected;
#endif

    /*
     * Log interrupt if active
     */
    TCeg_Send_Interrupt_Event(TCeg_INTR_TIM2, (u_int8) _LSLCslot_state);
   
    scanning = ( (context == Page_Scan) || (context == Inquiry_Scan) || (context == InquiryResponse));

    
#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 0)
    HWradio_Setup_Radio_For_Next_Half_Slot(RX_MID);
#endif

#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
    HWpta_Service_Rx_Start();
#endif

#if (PRH_BS_CFG_TABASCO_VERSION >= PRH_BS_HW_TABASCO_VERSION_T1_0_0) 
#if (PRH_BS_DEV_USE_T1_BT_CLOCK==1)
    if (freeze_bt_clk_on_next_tim2 == TRUE)
    {
        freeze_bt_clk_on_next_tim2 = FALSE;
        HW_set_freeze_bt_clk(1);
    }
#endif
#endif

    switch (_LSLCslot_state)
    {
    /*
     * Normal events
     */
    case W4_RX_START:

        USLCsched_Rx_Start();
        rx_mode = (t_Receiver_Mode) HW_get_rx_mode();
        if (rx_mode == RXm_RECEIVER_DISABLED)
        {
            LSLCacc_Enable_Transmitter();
            _LSLCslot_state = W4_TX_START;
        }
        else if (scanning)
        {
            /*
             * Visual show of scanning
             */
//            SYSmmi_Display_Event(eSYSmmi_Scan_Active_Event);
            /*
             * Receiver enabled at end of previous transmit.
             */
            _LSLCslot_state = SCAN_ACTIVE;
        }
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
        else if (_LSLCslot_eSCO_rx_3slot_packet_expected_cache)
        {
            /*
             * 3 slot eSCO packet (ie EV4/EV5) expected.
             */
            _LSLCslot_state = W4_ESCO_3SLOT_RX_START;
            _LSLCslot_eSCO_rx_3slot_packet_expected = FALSE;
        }
#endif
        else
        {
            _LSLCslot_state = W4_RX_INTR;
        }
        break;

    case SCAN_ACTIVE:
        /*
         * Ensures that in USLCsched updates current_USLC_Frame_Activity = next_USLC_Frame_Activity
         */
        USLCsched_Rx_Start();
#if (PRH_BS_CFG_SYS_PERSISTENT_SCO_IN_SCANS_SUPPORTED==1)
        if(USLCsched_Get_Next_Frame_Activity() == SCO_MASTER)
        {
            LSLCacc_Disable_Receiver();
            LSLCacc_Enable_Transmitter();
            _LSLCslot_state = W4_TX_START;       
        }
#endif
        break;

    case W4_3SLOT_INTR_TX:
    case W4_5SLOT_INTR_TX:
    case W4_5SLOT_1ST_TIM0:
    case W2_SETUP_ENABLE_RXR:
        /*
         * Ignore, no state change for Tx related states.
         */
        break;

    case W4_3SLOT_INTR_RX:
        if (RX_AMADDR_ERROR == DL_Get_Rx_Status(LSLCpkt_Get_Active_Rx_Device_Link_Ref()))
        {
            /*
             * rx_status will only ever == RX_AMADDR_ERROR for an eSCO reserved
             * slot, where a 3-slot eSCO packet is expected.
             */
            USLCsched_Prepare_Tx();
            USLCsched_Process_Rx();
            LSLCacc_Enable_Transmitter();
            _LSLCslot_state = W4_TX_START;

        }
        else
        {
            if (is_Packet_Multi_Slot((t_packet)HW_get_rx_type()) &&
                CONNECTED_TO_SLAVE==DL_Get_Ctrl_State(DL_Get_Local_Device_Ref()))
            {
                /*
                 * Reprepare next master tx/rx frame activity
                 * at last TIM2 of multislot receive.
                 */
                USLCsched_Set_Next_Frame_Activity(NO_ACTIVITY);
                USLCsched_Rx_Start();
            }
            USLCsched_Prepare_Tx();
            _LSLCslot_state = W4_PKD_INTR;
        }
        break;

    case W2_COMPLETE_RX:
        /*
         * At this stage the full multi-slot packet is received and this is
         * the last TIM2.  Therefore we can now Prepare next Tx, and process Rx.
         */
        if (is_Packet_Multi_Slot((t_packet)HW_get_rx_type()) &&
            CONNECTED_TO_SLAVE==DL_Get_Ctrl_State(DL_Get_Local_Device_Ref()))
        {
            /*
             * Reprepare next master tx/rx frame activity
             * at last TIM2 of multislot receive.
             */
            USLCsched_Set_Next_Frame_Activity(NO_ACTIVITY);
            USLCsched_Rx_Start();
        }
        USLCsched_Prepare_Tx();
        LSLCpkt_Process_Rx_Packet();
        USLCsched_Process_Rx();
        LSLCacc_Enable_Transmitter();
        _LSLCslot_state = W4_TX_START;
        break;

    case W4_5SLOT_INTR_RX:
        _LSLCslot_state = W4_3SLOT_INTR_RX;
        break;

    case W4_5SLOT_1ST_TIM2:
        _LSLCslot_state = W2_COMPLETE_RX;
        break;

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    case W2_ESCO_3SLOT_NO_PKD:
        LSLCpkt_Process_No_Rx_Packet();
        USLCsched_Process_Rx();
        USLCsched_Prepare_Tx();
        LSLCacc_Enable_Transmitter();
        _LSLCslot_state = W4_TX_START;
        break;

    case W2_ESCO_3SLOT_PKD_HDR:
        USLCsched_Prepare_Tx();
        _LSLCslot_state = W4_PKD_INTR;        
#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 1)
            LSLCirq_Unmask_PKD_RX_HDR();
#endif
        break;
#endif

    case W4_PKD_INTR_EXTENDED:
        /* Multislot/2nd Half EIR Scenario */
        break;
        
    /*
     * Errored events    All TIM2's should resume at next W4_TX_START
     */

    default:                            /* TX_INTR_ERROR               */
        /*
         * case W4_RX_INTR, W4_PKD_INTR, W4_TX_START, W2_ENABLE_RXR
         */
        _LSLCslot_Process_Interrupt_Error( _LSLCslot_INTERRUPT_ID( TIM2 ) );
        /*
         * Always Process TIM2 as normal
         */
        _LSLCslot_state = W4_RX_START;
        LSLCslot_Handle_TIM2();
#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 1)
        LSLCirq_Unmask_Tim2();
#endif
        return; /* avoid extra log */
    }
    
    _LSLCslot_Log_Interrupt( _LSLCslot_INTERRUPT_ID( TIM2 ) );
   
#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 1)
    LSLCirq_Unmask_Tim2();
#endif
}

/***************************************************************************
 * LSLCslot_Handle_SYNC_DET
 *
 * Determine slot behaviour on sync detection.
 * Note: SYNC_DET not generated for IDpkts (page mode).
 ***************************************************************************/
void LSLCslot_Handle_SYNC_DET(void)
{
    /*
     * Log interrupt if active
     */
    TCeg_Send_Interrupt_Event(TCeg_INTR_SYNC_DET, (u_int8) _LSLCslot_state);

#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 0)
    /*
     * Extend receive slot until PKD for FHS in Second Half
     */
    HWradio_Rx_PacketExtend();
#endif

    g_sys_config.win_ext = PRH_BS_CFG_SYS_DEFAULT_WIN_EXT; // restore default - may have been modified in r2p/sp-type procedures

#if (PRH_BS_CFG_SYS_LMP_RSSI_SUPPORTED==1)
    LSLCacc_Attempt_RSSI_Read(FALSE); /* Early Read Opportunity */
#endif

#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
    if (HW_get_pta_grant_rx_denied())
	{ // requires here as no_pkd will not occur...
    	HWradio_RxComplete();
        LSLCacc_Disable_Receiver();

        LSLCpkt_Process_No_Rx_Packet();
		LSLCstat_Inc_Rx_Restricted_Packet_Cntr();
        USLCsched_Process_Rx();
        USLCsched_Prepare_Tx();

		LSLCacc_Enable_Transmitter();
		_LSLCslot_state = W4_TX_START;
	}
#endif

   _LSLCslot_Log_Interrupt( _LSLCslot_INTERRUPT_ID( SYNC_DET ) );
  
#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 1)
    LSLCirq_Unmask_SYNC_DET();
#endif

}
#endif //#BLUETOOTH_MODE_LE_ONLY

/***************************************************************************
 * LSLCslot_Handle_AUX_TIM
 *
 * Wakeup from low power mode
 ***************************************************************************/
void LSLCslot_Handle_AUX_TIM(void)
{
	   if(PATCH_FUN[LSLCSLOT_HANDLE_AUX_TIM_ID]){
         ((void (*)(void))PATCH_FUN[LSLCSLOT_HANDLE_AUX_TIM_ID])();
         return ;
    }
    /*
     * Log interrupt if active
     */
    TCeg_Send_Interrupt_Event(TCeg_INTR_AUX_TMR, (u_int8) _LSLCslot_state);

#if (PRH_BS_CFG_SYS_LOW_POWER_MODE_SUPPORTED==1)
    /* USLCchac_Wakeup(); */
    /* if (FALSE != SYSpwr_Is_Low_Power_Mode_Active()) */
    {
    	#if (__DEBUG_LE_SLEEP == 0)
        /*SYSpwr_Handle_Early_Wakeup();*/
	    #endif
    }
#endif

	#if (__DEBUG_LE_SLEEP == 1)
    if(1 == g_debug_le_sleep)
    {
 	Le_Wake_Up();
    }
	#elif (2 == __DEBUG_LE_SLEEP)
    //if(2 == g_debug_le_sleep)
    {
    /*Adv_Exit_LowPower();
    Connection_Exit_LowPower();
    Exit_LowPower();*/
    }
	#endif
#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 1)
    LSLCirq_Unmask_AUX_TIM();
#endif
}
#if 0
/***************************************************************************
 * LSLCslot_Local_Piconet_Request
 *
 * Procedure to change to local device link when no connections (required
 * to return to standby after scans and after last link disconnects).
 *
 * As local piconet is equivalent to connected as master, this procedure
 * can also be used to return to master context after scans as master for
 * configurations not supporting scatternet.
 ***************************************************************************/
boolean LSLCslot_Local_Piconet_Request()
{
    /*
     * Log event if active
     */
    TCeg_Send_Interrupt_Event(TCeg_EVENT_RETURN_TO_OWN_PICONET, (u_int8) _LSLCslot_state);

    if (DL_Get_Ctrl_State(DL_Get_Local_Device_Ref()) != CONNECTED_TO_MASTER)
    {
        LSLCirq_Disable_All_Intr_Except(JAL_TIM0_INTR_MSK_MASK);

        _LSLCslot_state = W4_RADIO_TO_GO_INTO_STANDBY;
        _LSLCslot_local_piconet_request = TRUE;
    }

    return _LSLCslot_local_piconet_request;
}
#endif
#ifndef BLUETOOTH_MODE_LE_ONLY
void LSLCslot_Change_To_Local_Piconet()
{
    t_devicelink *p_local_device_link = DL_Get_Local_Device_Ref();

    /* Master's own BD addr must be written in HW */
    HW_set_bd_addr(SYSconfig_Get_Local_BD_Addr_Ref());
    
    HW_set_slave(0);
    HW_set_intraslot_offset(0);
 
    if (DL_Get_Ctrl_State(p_local_device_link) == NOT_CONNECTED)
    {
        HWradio_SetRadioMode(RADIO_MODE_STANDBY);
    }

#if (PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_SUPPORTED==0)
    LSLCfreq_Assign_Frequency_Kernel(p_local_device_link);
#endif
 
    LSLCirq_Disable_All_Intr_Except(JAL_TIM3_INTR_MSK_MASK);
 
    _LSLCslot_state = W4_INTRASLOT_OFFSET_ALIGNMENT;
 }
#endif//BLUETOOTH_MODE_LE_ONLY


#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
/***************************************************************************
 * LSLCslot_eSCO_tx_3slot_packet_expected
 *
 * Used by the higher layers to indicate if a 3slot eSCO packet is expected
 * in the next TX slot.
 * 
 * "LSLCslot_eSCO_tx_3slot_packet_expected" is reset to FALSE at the start 
 * of each TIM0.
 ***************************************************************************/
void LSLCslot_eSCO_tx_3slot_packet_expected(boolean expected)
{
    _LSLCslot_eSCO_tx_3slot_packet_expected = expected;
}

/***************************************************************************
 * LSLCslot_eSCO_rx_3slot_packet_expected
 *
 * Used by the higher layers to indicate if a 3slot eSCO packet is expected
 * in the next RX slot.
 * 
 * "LSLCslot_eSCO_rx_3slot_packet_expected" is reset to FALSE at the start 
 * of each TIM0.
 ***************************************************************************/
void LSLCslot_eSCO_rx_3slot_packet_expected(boolean expected)
{
    _LSLCslot_eSCO_rx_3slot_packet_expected = expected;
}

#if (BUILD_TYPE==UNIT_TEST_BUILD)
/***************************************************************************
 * LSLCslot_Get_eSCO_tx_3slot_packet_expected
 *
 * Used by the ATS to determine if a 3slot eSCO packet was expected to be 
 * transmitted. The function will only be used in the following scenario:
 *  - Device transmits a NULL packet (in place of a EV4/EV5)
 *  - ATS test support functions must now "emulate" the interrupts 
 *    assoc. with the replaced 3 slot EV4/EV5. There is no info
 *    in the Tabasco HW to indicate that the POLL/NULL is replacing 
 *    a 3 slot packet, thus the ATS must use this function to access
 *    stack internal info.
 * 
 ***************************************************************************/
boolean LSLCslot_Get_eSCO_tx_3slot_packet_expected(void)
{
    return _LSLCslot_eSCO_tx_3slot_packet_expected;
}

/***************************************************************************
 * LSLCslot_Get_eSCO_rx_3slot_packet_expected
 *
 * Used by the ATS to determine if a 3slot eSCO packet was expected to be 
 * received. The function will only be used in the following scenario:
 *  - A NULL/POLL packet is placed in the receive registers.
 *  - ATS test support functions must now "emulate" the interrupts 
 *    assoc. with the replaced 3 slot EV4/EV5. There is no info
 *    in the Tabasco HW to indicate that the POLL/NULL is replacing 
 *    a 3 slot packet, thus the ATS must use this function to access
 *    stack internal info.
 * 
 ***************************************************************************/
boolean LSLCslot_Get_eSCO_rx_3slot_packet_expected(void)
{
    return _LSLCslot_eSCO_rx_3slot_packet_expected;
}

#endif
#endif
#ifndef REDUCE_ROM
/***************************************************************************
 * LSLCslot_Write_BT_Clk_Avoid_Race
 *
 * Write BT clock avoiding asynchronous native clock Race
 * To be called in a RX frame first, 
 * with automatic re-scheduling on TIM0 if required 
 *
 * Safe in a   frame_pos     intraslot_offset     Clock Transition Occurs
 *             TX_START      >=625                In RX slot
 *             RX_START      < 625                In TX slot
 ***************************************************************************/
boolean LSLCslot_Write_BT_Clk_Avoid_Race(t_clock const * p_bt_clk)
{

	if(PATCH_FUN[LSLCSLOT_WRITE_BT_CLK_AVOID_RACE_ID]){
         
         return ((boolean (*)(t_clock const * p_bt_clk))PATCH_FUN[LSLCSLOT_WRITE_BT_CLK_AVOID_RACE_ID])(p_bt_clk) ;
    }

#if (BUILD_TYPE!=UNIT_TEST_BUILD) /* Attempt a 2nd half of Rx slot recovery*/
	    t_clock ref_clk, clk_offset;
        /*
         * Defer the write offset to 2nd half of Rx slot (after bt_clk transition)
         * During R2P it was observed that the new clock offset was
         * not effective until after the clock had ticked.
         * Ensure that we re-enter on TIM0 to re-verify since initial
         * tests suggest that this is not working.  More tests required.
         * The implications of leaving to TIM0 is that the bt_clk
         * is not set in time for frequency calculations for next frame
         */
         while ((HW_get_bt_clk() & 0x03) != 0x01) 
         { 
             /* SPIN for TIM3 */ ; 
         }

         ref_clk = HW_get_bt_clk();

         clk_offset = ((*p_bt_clk) & 0x0ffffffc) - ((ref_clk & 0x0ffffffc));

         HW_set_bt_clk_offset(clk_offset);
#endif

         return TRUE;
}
#endif
