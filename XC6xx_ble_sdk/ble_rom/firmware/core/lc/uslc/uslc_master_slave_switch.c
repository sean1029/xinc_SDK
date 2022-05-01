/***********************************************************************
 *
 * MODULE NAME:    uslc_master_slave_switch.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    LC Portion of Master Slave Switch Procedure
 * MAINTAINER:     Ivan Griffin
 * CREATION DATE:  17 August 2000
 *
 * SOURCE CONTROL: $Id: uslc_master_slave_switch.c,v 1.159 2013/09/24 15:23:38 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 *
 ***********************************************************************/
 
#include "sys_config.h"

#define PRH_BS_DEV_MSS_USE_LSLC_SLOT_LOG (1 && PRH_BS_DBG_LC_LSLC_SLOT)

#ifndef PRH_BS_DEV_MSS_MAX_NUM_ATTEMPTS
#define PRH_BS_DEV_MSS_MAX_NUM_ATTEMPTS 1
#endif

#define PRH_BS_DEV_MSS_FORCE_PICONET_SWITCH_SUPPORTED 1

#ifndef PRH_BS_DEV_MSS_TRANSITIONS_RADIO_MODE_STANDBY
#define PRH_BS_DEV_MSS_TRANSITIONS_RADIO_MODE_STANDBY 1 //0
#endif

/***********************************************************************
 *
 * Module Includes
 *
 ***********************************************************************/

#include "tra_queue.h"

#include "lc_types.h"
#include "lc_log.h"

#include "bt_timer.h"
#include "bt_fhs.h" 

#include "sys_mmi.h"
#include "hw_macro_defs.h"
#include "hw_memcpy.h"
#include "hw_radio.h"

#include "dl_dev.h"

#include "lc_interface.h"
#include "lslc_pkt.h"
#include "lslc_access.h"
#include "lslc_clk.h"
#include "lslc_freq.h"
#include "lslc_afh.h"
#include "lslc_hop.h"
#include "lslc_slot.h"

#include "lmp_config.h"

#include "uslc_chan_ctrl.h"
#include "uslc_return_to_piconet.h"

#include "hc_const.h"
#include "hc_event_gen.h"

/*
 * GUARD OF ENTIRE MODULE
 */
#if (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1)

/***********************************************************************
 *
 * Module Typedefs
 *
 ***********************************************************************/

#include "uslc_mss_ctrl_states.h"

struct _USLCmss_Private_Data;

typedef struct _USLCmss_Private_Data
{
    t_USLCmss_Ctrl_State ctrl_state;
    t_devicelink *p_active_devicelink;
    t_slots new_connection_timeout;
    t_timer new_connection_timer;

    t_clock slave_a_native_clk;

    t_devicelink stored_devicelink;
    boolean master_b_piconet_switch_complete;

    t_status status;
    u_int8 attempt;
} t_USLCmss_Private_Data;

static t_USLCmss_Private_Data _mss_info;

/***********************************************************************
 *
 * state machine prototypes
 *
 ***********************************************************************/

static void USLCmss_Master_B_Perform_TDD_Switch_(
    t_USLCmss_Private_Data *p_mss_info, t_devicelink* p_active_devicelink);
static void USLCmss_Slave_A_Perform_TDD_Switch_(
    t_USLCmss_Private_Data *p_mss_info, t_devicelink* p_active_devicelink);

static void USLCmss_Update_Ctrl_State_(t_devicelink* p_dev_link,
    t_device_ctrl_state ctrl_state);

/***********************************************************************
 ***********************************************************************
 **
 ** Module Public Functions
 **
 ***********************************************************************
 ***********************************************************************/

/***********************************************************************
 *
 * FUNCTION:  USLCmss_Initialise()
 * PURPOSE:
 *
 ***********************************************************************/
void USLCmss_Initialise(void)
{
    t_USLCmss_Private_Data volatile *p_mss_info = &_mss_info;

    p_mss_info->new_connection_timeout = NEW_CONNECTION_TIMEOUT_DEFAULT;
    p_mss_info->ctrl_state = OS_INACTIVE;
}


/***********************************************************************
 *
 * FUNCTION:  USLCmss_Request()
 * PURPOSE:
 *
 ***********************************************************************/
t_error USLCmss_Request(t_deviceIndex device_index)
{
    t_error status = EC_ROLE_SWITCH_FAILED;

    t_USLCmss_Private_Data volatile *p_mss_info = &_mss_info;

    LC_Log_Procedure_Request(MSS_PROCEDURE);

#if PRH_BS_DEV_MSS_USE_LSLC_SLOT_LOG
    LSLCslot_Enable_Interrupt_Log_If_Active();
#endif
        
    if (OS_INACTIVE == p_mss_info->ctrl_state)
    {
        t_ulc_procedure procedure_candidate = NO_PROCEDURE;

        p_mss_info->p_active_devicelink = DL_Get_Device_Ref(device_index);

        {
            t_device_ctrl_state current_device_ctrl_state =
                DL_Get_Ctrl_State(p_mss_info->p_active_devicelink);

            /* 
             * can't do a MSS on this devicelink if I'm not connected!
             */
            if (CONNECTED_TO_MASTER == current_device_ctrl_state)
            {
                procedure_candidate = MSS_PROCEDURE_MASTER_CONTEXT;
            }
            else if (CONNECTED_TO_SLAVE == current_device_ctrl_state)
            {
                procedure_candidate = MSS_PROCEDURE_SLAVE_CONTEXT;
            }

            if (NO_PROCEDURE != procedure_candidate)
            {
                /*
                 * Switch into piconet.
                 * Restore local device state for procedure start.
                 */
                DL_Set_Ctrl_State(DL_Get_Local_Device_Ref(), current_device_ctrl_state);

                USLCchac_Procedure_Request(procedure_candidate, TRUE);
                p_mss_info->ctrl_state = OS_START;
                p_mss_info->attempt = 0;

                LC_Log_Procedure_Accept(MSS_PROCEDURE);
                status = NO_ERROR;
            }
        }
    }

    p_mss_info->status = status;

    return status;
}


/***********************************************************************
 *
 * FUNCTION:  USLCmss_Cancel()
 * PURPOSE:
 *
 * Does it make sense to try to cancel an MSS procedure, or should it
 * just run to completion ??
 ***********************************************************************/
t_error USLCmss_Cancel(void)
{
    t_USLCmss_Private_Data volatile *p_mss_info = &_mss_info;
    t_error status = USLCchac_Procedure_Request(MSS_PROCEDURE_SLAVE_CONTEXT, FALSE);

    if (ILLEGAL_COMMAND == status)
    {
        /* woops, perhaps it was a master context procedure... */
        status = USLCchac_Procedure_Request(MSS_PROCEDURE_MASTER_CONTEXT,
            FALSE); 

        p_mss_info->ctrl_state =  OS_SLAVE_A_MSS_FAILURE_RECOVER_TDD;
    }
    else
    {
        p_mss_info->ctrl_state = OS_MASTER_B_END;
    }


    LC_Log_Procedure_Cancel(MSS_PROCEDURE);

    return status;
}


/***********************************************************************
 *
 * FUNCTION:  USLCmss_State_Dispatcher()
 * PURPOSE:
 *
 ***********************************************************************/
void USLCmss_State_Dispatcher(boolean look_ahead)
{
    t_USLCmss_Private_Data *p_mss_info = &_mss_info;
    t_devicelink *p_active_devicelink = p_mss_info->p_active_devicelink;
    int state_index = (int)(p_mss_info->ctrl_state);

    t_packet pkt_rcvd = DL_Get_Rx_Packet_Type(p_active_devicelink);
    t_status status = DL_Get_Rx_Status(p_active_devicelink);

    switch (state_index)
    {
/***********************************************************************
 *
 * STATE:  OS_START
 * PURPOSE:   
 * STIMULI EVENTS: TIM2 if SlaveA, TIM0 if MasterB
 *
 ***********************************************************************/
    case OS_START:
    {
        p_mss_info->master_b_piconet_switch_complete = FALSE;

        {
            t_device_ctrl_state current_device_ctrl_state = 
                DL_Get_Ctrl_State(p_active_devicelink);
            t_ulc_procedure active_procedure = (t_ulc_procedure ) (
                (CONNECTED_TO_MASTER == current_device_ctrl_state) 
                ? MSS_PROCEDURE_MASTER_CONTEXT: MSS_PROCEDURE_SLAVE_CONTEXT);

            if (0 == p_mss_info->attempt)
            {
                LC_Log_Procedure_Start(MSS_PROCEDURE);
                USLCchac_Procedure_Start(active_procedure, p_active_devicelink);
            }
            else
            {
                USLCchac_Set_Active_Procedure(active_procedure);
            }
        }

        /*
         * if slave, intraslot_offset-=625us
         * if master, intraslot_offset+=625us
         */
        p_mss_info->new_connection_timer =
            BTtimer_Set_Slots(p_mss_info->new_connection_timeout);

        /* 
         * Store Device Link and AFH state, for recovery if failure 
         */
        p_mss_info->stored_devicelink = *p_active_devicelink;
        LSLCafh_Backup_AFH_Link_State(p_active_devicelink->device_index);


        /* slot offset and clock offset now automatically set from LSLCaccess */

#if (PRH_BS_DEV_MSS_TRANSITIONS_RADIO_MODE_STANDBY==1)
        HWradio_SetRadioMode(RADIO_MODE_STANDBY);
#endif

        /*
         * state machines diverge at OS_START to separate threads of
         * execution, depending on role in existing connection
         */
        HW_set_tx_mode(0); 
        HW_set_rx_mode(0);
        
        if (SLAVE == DL_Get_Role_Peer(p_active_devicelink))
        {
            USLCmss_Master_B_Perform_TDD_Switch_(p_mss_info, p_active_devicelink);
            p_mss_info->ctrl_state = OS_MASTER_B_TDD_SWITCH_RX;
        }
        else
        {
            USLCmss_Slave_A_Perform_TDD_Switch_(p_mss_info, p_active_devicelink);
            p_mss_info->ctrl_state = OS_SLAVE_A_TDD_SWITCH_TX;
        }

        LSLChop_Set_HSE_Engine(LSLChop_HSE_SOFTWARE); 
        USLCchac_Set_Device_State(Master_Slave_Switch);
    }
    break;

/***********************************************************************
 ***********************************************************************
 **
 ** Old Slave Side == New Master Side
 **
 **/


/***********************************************************************
 *
 * STATE:  OS_SLAVE_A_TDD_SWITCH_TX
 * PURPOSE:   Determine if safe to read Native Clk in this slot?
 *            If so, enable transmission of FHS, and prepare receiver
 *            if not, prepare receiver to receive a NO_PKT_RCVD interrupt
 *            in next frame
 *            LOOK_AHEAD (RX_START) will provoke FHS clk generation instead
 * STIMULI EVENTS: TIM0
 *
 ***********************************************************************/
     case OS_SLAVE_A_TDD_SWITCH_TX:
     {
        if (BTtimer_Is_Expired(p_mss_info->new_connection_timer))
        {
            HW_set_rx_mode(0);
            p_mss_info->ctrl_state = OS_SLAVE_A_MSS_FAILURE_RECOVER_TDD;
        }
        else if ((LOOK_AHEAD == look_ahead) && ((HW_get_bt_clk()&2)==0))
        {
            t_am_addr am_addr;

            /*
             * AM_ADDR is defined in FHS of local Device Link when allocated.
             */
            am_addr = DL_Alloc_Am_Addr();
            if (am_addr != NO_AMADDR_AVAILABLE)
            {
                FHS_Set_Am_Addr( LC_Get_Device_FHS_Ref(), am_addr );

                p_mss_info->slave_a_native_clk = HW_Get_Native_Clk_Avoid_Race();

                /* 
                 * Determine FHS packet clock 
                 * Since In RX START (TIM2)
                 * If Native Clock is transitioning in this Rx Slot Then
                 *    Add 4 to stored native clock since it will have
                 *    transitioned before FHS Tx
                 * Else
                 *    Use stored native clock
                 * Endif
                 *
                 * Tab P6A-1 Func Spec, R[9.7.3]:
                 *      TBCm + 625 = TBCs + ISO
                 *
                 * From this it can be deduced that:
                 *     0 <= ISO <  625 -> slave CLKN increment in slave Tx slot
                 *   625 <= ISO < 1250 -> slave CLKN increment in slave Rx slot
                 */

                if (((p_mss_info->slave_a_native_clk)&2) && (HW_get_intraslot_offset() >= 625))
                { /* native not yet transitioned => adjust forward */
                    p_mss_info->slave_a_native_clk += 4;
                }

#if (PRH_BS_DEV_MSS_TRANSITIONS_RADIO_MODE_STANDBY==1)
                HWradio_SetRadioMode(RADIO_MODE_TX_RX);
#endif
                LSLCpkt_Generate_FHS_Ex(p_active_devicelink,
                    Master_Slave_Switch, 
                    p_mss_info->slave_a_native_clk);

                /*
                 * flush the FHS before preparing for Receive
                 * (and overwriting common control regs)
                 */
                LSLCacc_Enable_Transmitter();

                LSLCpkt_Prepare_Receiver_ID(p_active_devicelink, Master_Slave_Switch);

                p_mss_info->ctrl_state = OS_SLAVE_A_TDD_SWITCH_RX;
            }
            else
            {
                p_mss_info->status = NO_AMADDR_AVAILABLE;
                HW_set_rx_mode(0);
                p_mss_info->ctrl_state = OS_SLAVE_A_MSS_FAILURE_RECOVER_TDD;
            }
        }
        else /* if (LOOK_AHEAD = look_ahead) */
        {
            ;
        }
    }
    break;

/***********************************************************************
 *
 * STATE:  OS_SLAVE_A_TDD_SWITCH_RX
 * PURPOSE:
 * STIMULI EVENTS: TIM2, NO_PKT_RCVD or PKD, TIM0
 *
 ***********************************************************************/

    case OS_SLAVE_A_TDD_SWITCH_RX:
    {
        /* FORCE PICONET SWITCH:
         *   If MasterB hears the FHS from SlaveA, but for some reason
         *   the FHS-Ack ID is lost, it makes sense to try the piconet
         *   switch anyway to see if the switch can still succeed.
         *   
         *   In general, there is little chance, since if an EARLY FHS
         *   is received, then MasterB->SlaveB up to 15 frames earlier than
         *   SlaveA will transition to MasterA.
         *   
         *   Nevertheless, anything that improves the reliability of the
         *   switch is a good thing!
         */
#if (PRH_BS_DEV_MSS_FORCE_PICONET_SWITCH_SUPPORTED==0)
        if (BTtimer_Is_Expired(p_mss_info->new_connection_timer))
        {
            /*
             * procedure has timed out, so release the newly allocated AM
             * Address (from OS_SLAVE_A_TDD_SWITCH_TX)
             */
            HW_set_rx_mode(0);
            p_mss_info->ctrl_state = OS_SLAVE_A_MSS_FAILURE_RECOVER_TDD;
        }
        else
#endif
        if (NO_LOOK_AHEAD == look_ahead)
        {
            if (((RX_NO_PACKET != status) && (IDpkt == pkt_rcvd)) ||
                (PRH_BS_DEV_MSS_FORCE_PICONET_SWITCH_SUPPORTED &&
                    BTtimer_Is_Expired(p_mss_info->new_connection_timer))
              )
            {
#if (PRH_BS_DEV_MSS_FORCE_PICONET_SWITCH_SUPPORTED==1)
                if ((HW_get_no_pkt_rcvd_intr() == 0) &&
                    (BTtimer_Is_Expired(p_mss_info->new_connection_timer))
                    ) { break; }
#endif
                p_mss_info->new_connection_timer =
                    BTtimer_Set_Slots(p_mss_info->new_connection_timeout);

                /*
                 * have saved old piconet information - i.e. devicelink (AM_ADDR,
                 * syncword), intraslot offset, etc.
                 */
                {
                    /*
                     * Now that the local device has assumed the role of Master
                     * need to update the BD_ADDR and SYNC_WORD in the deivce_link.
                     * Until now, the BD_ADDR and SYNC_WORD represented that of the 
                     * now 'old' Master. 
                     * Thus, the local BD_ADDR and local SYNC_WORD are stored in the 
                     * device_link.
                     */
                    const t_bd_addr* p_local_bd_addr;

                    p_local_bd_addr = SYSconfig_Get_Local_BD_Addr_Ref();
                    DL_Set_UAP_LAP(p_active_devicelink, 
                                        BDADDR_Get_UAP_LAP(p_local_bd_addr));
                    DL_Set_NAP(p_active_devicelink, 
                                        BDADDR_Get_NAP(p_local_bd_addr));

                    /* update channel access code */
                    DL_Set_Access_Syncword(p_active_devicelink,
                        SYSconfig_Get_Local_Syncword() );
                    DL_Set_AM_Addr(p_active_devicelink,
                        FHS_Get_AM_Addr(LC_Get_Device_FHS_Ref()) );
                }

                /*
                 * the following:
                 *   DL_Set_Role_Peer(p_active_devicelink, SLAVE);
                 *   DL_Set_Tx_Seqn(p_active_devicelink, 1);
                 *   DL_Set_Rx_Expected_Seqn(p_active_devicelink, 1);
                 * are now replaced with:
                 */
                DL_Reset_New_Connection(p_active_devicelink, SLAVE);

                /*
                 * Ensure that Hop Selection Kernel is setup
                 */
                LSLCfreq_Assign_Frequency_Kernel(p_active_devicelink);

               /*
                * Setting the ctrl_state of the local Device Link will allow the 
                * USLCscheduler object to determine whether the device is a 
                * Master or a Slave within the connection.
                */
                USLCmss_Update_Ctrl_State_(p_active_devicelink, CONNECTED_TO_SLAVE);

                USLCchac_Set_Device_State(Connection);

#if (PRH_BS_DEV_MSS_TRANSITIONS_RADIO_MODE_STANDBY==1)
                HWradio_SetRadioMode(RADIO_MODE_STANDBY);
#endif

                HW_set_slave(0);

                p_mss_info->ctrl_state =
                    OS_MASTER_A_PICONET_SWITCH_TX;
            }
            else
            {
                /* 
                 * Determine FHS packet clock 
                 *    Use offset clock from first FHS to ensure determinism
                 */
                p_mss_info->slave_a_native_clk += 4;
                LSLCpkt_Generate_FHS_Ex(p_active_devicelink,
                    Master_Slave_Switch, p_mss_info->slave_a_native_clk);
                
                /*
                 * flush the FHS before preparing for Receive
                 * (Prepare receive will overwrite the common control regs)
                 */
                LSLCacc_Enable_Transmitter();

                LSLCpkt_Prepare_Receiver_ID(p_active_devicelink,
                    Master_Slave_Switch);

                p_mss_info->ctrl_state = OS_SLAVE_A_TDD_SWITCH_RX;
            }
        }
        else /* if (LOOK_AHEAD == look_ahead) */
        {
        }
    }
    break;

/***********************************************************************
 *
 * STATE:  OS_MASTER_A_PICONET_SWITCH_TX
 * PURPOSE:
 * STIMULI EVENTS: TIM2, NO_PKD_RCVD
 *
 ***********************************************************************/
    case OS_MASTER_A_PICONET_SWITCH_TX:
    {
        /* need to ensure that the first POLL or two is absorbed in this
         * state, since the hardware will not have caught up yet with the
         * modification of the intraslots */

        static int gobbled_state = 0;

        if (BTtimer_Is_Expired(p_mss_info->new_connection_timer))
        {
            HW_set_rx_mode(0);
            p_mss_info->ctrl_state = OS_SLAVE_A_MSS_FAILURE_RECOVER_TDD;
        }
        else
        {
            if (gobbled_state == 1)
            {
                /*
                 * POLLs are sent in the Connection context
                 * hence DO NOT Enable_Transmitter immediately, or
                 * explicitly prepare receiver - LSLCslot will sequence
                 * these activities instead
                 */
#if (PRH_BS_DEV_MSS_TRANSITIONS_RADIO_MODE_STANDBY==1)
                HWradio_SetRadioMode(RADIO_MODE_TX_RX);
#endif
                LSLCpkt_Generate_POLL(p_active_devicelink);
                gobbled_state = 0;
                p_mss_info->ctrl_state = OS_MASTER_A_PICONET_SWITCH_RX;
            }
            else
            {                
                gobbled_state ++;
            }
        }
    }
    break;


/***********************************************************************
 *
 * STATE:  OS_MASTER_A_PICONET_SWITCH_RX
 * PURPOSE:
 * STIMULI EVENTS: TIM2, NO_PKD_RCVD or PKD
 *
 ***********************************************************************/
    case OS_MASTER_A_PICONET_SWITCH_RX:
    {
        /*
         * need to check for any packet here, or is it only a NULL?
         * should be liberal in what we will except back from another
         * device, and strict in what we send...
         */
        if (BTtimer_Is_Expired(p_mss_info->new_connection_timer))
        {
            HW_set_rx_mode(0);
            p_mss_info->ctrl_state = OS_SLAVE_A_MSS_FAILURE_RECOVER_TDD;
        }
        else if (NO_LOOK_AHEAD == look_ahead)
        {
            if ((RX_NO_PACKET != status) &&
                (INVALIDpkt != pkt_rcvd) &&
                (IDpkt != pkt_rcvd) )
            /* && (NAK == DL_Get_Rx_Previous_Arqn(p_mss_info->p_active_devicelink))) */
            {
                p_mss_info->ctrl_state = OS_MASTER_A_CONNECTED_TO_SLAVE;

//                SYSmmi_Display_Event(eSYSmmi_LC_Connection_Event);
            }
            else
            {
                /*
                 * POLLs are sent in the Connection context
                 * hence DO NOT Enable_Transmitter immediately, or
                 * explicitly prepare receiver - LSLCslot will sequence
                 * these activities instead
                 */
               LSLCpkt_Generate_POLL(p_active_devicelink);

                p_mss_info->ctrl_state = OS_MASTER_A_PICONET_SWITCH_RX;
            }
        }
        else
        {
             /* LSLCpkt_Generate_POLL(p_active_devicelink); */
        }
    }
    break;


/***********************************************************************
 *
 * STATEs:    OS_SLAVE_A_MSS_FAILURE_RECOVER_{TDD/SLOT_OFFSET/CLK}
 * PURPOSE:  Role Switch Failure - Recover original piconet slave
 *           parameters
 * STIMULIL: TIM2
 *
 ***********************************************************************/
    case OS_SLAVE_A_MSS_FAILURE_RECOVER_TDD:
    {
#if (PRH_BS_DEV_MSS_TRANSITIONS_RADIO_MODE_STANDBY==1)
        HWradio_SetRadioMode(RADIO_MODE_STANDBY);
#endif
        /*
         * Setting the ctrl_state of the local Device Link will allow the 
         * USLCscheduler object to determine whether the device is a 
         * Master or a Slave within the connection.
         */

        DL_Free_Am_Addr(DL_Get_Am_Addr(p_active_devicelink));
        USLCmss_Update_Ctrl_State_(p_active_devicelink, CONNECTED_TO_MASTER);
        USLCchac_Set_Active_Procedure(MSS_PROCEDURE_SLAVE_CONTEXT);

        p_mss_info->ctrl_state = OS_SLAVE_A_MSS_FAILURE_RECOVER_SLOT_OFFSET;

    }
    break;

    /* STIMULUS: TIM0 */
    case OS_SLAVE_A_MSS_FAILURE_RECOVER_SLOT_OFFSET:
    {
        if ((HW_get_bt_clk() & 0x3) != 2 /* i.e. NOT TIM0 in Slave */) { break; } 

        HW_set_slave(1);

        /*
         * set intraslot offset
         */
        {
            /* slot offset of peer */
            u_int16 tmp_slot_offset = (DL_Get_Local_Slot_Offset(p_active_devicelink) + 625);

            if (tmp_slot_offset >= 1250) { tmp_slot_offset -= 1250; } /* tmp_slot_offset %= 1250; */
            if (tmp_slot_offset >= 1250) 
            {
                HCeg_Hardware_Error_Event(PRH_BS_HW_ERROR_MSS_FAILURE);

                USLCmss_Cancel();
                return;
            }
     
            HW_set_intraslot_offset(tmp_slot_offset);
        }

        /* restore device link, intraslot offset, bt clk offset */
        *p_active_devicelink = p_mss_info->stored_devicelink;

        DL_Set_Role_Peer(p_active_devicelink, MASTER);
            
        p_mss_info->ctrl_state = OS_SLAVE_A_MSS_FAILURE_RECOVER_CLK;
    }
    break;

    /* STIMULUS: TIM0 */
    case OS_SLAVE_A_MSS_FAILURE_RECOVER_CLK:
    {
        if ((HW_get_bt_clk() & 0x3) != 2 /* i.e. NOT TIM0 in Slave */) 
        { 
            break; 
        } 

        /*
         * Set the clock offset in hardware directly
         */
        {
            t_clock clk_offset_adjusted =
                DL_Get_Clock_Offset(&p_mss_info->stored_devicelink);

            /* adjustment for bt clk relative */
            clk_offset_adjusted += ((HW_get_native_clk() & 0x0ffffffc) - 
                                    (HW_get_bt_clk() & 0x0ffffffc));

            /* 
             * Since In TIM0
             * If Native Clock is not transitioning in this Tx Slot Then
             *    Substract 4 from clock offset since native clock has transitioned
             *        since previous BT clock T2 reference
             * Else
             *    Defer to TIM2 and write it
             * Endif
             *
             */
            if (HW_get_intraslot_offset() >=  625)
            {
                clk_offset_adjusted -= 4;
            }
            else
            {
                LSLCclk_Wait_For(RX_START);
            }

            HW_set_bt_clk_offset(clk_offset_adjusted);
            
        }

        p_mss_info->ctrl_state = OS_RECOVER_ORIGINAL_PICONET;
    }
    break;


/***********************************************************************
 ***********************************************************************
 **
 ** Old Master Side == New Slave Side
 **/


/***********************************************************************
 *
 * STATE:  OS_MASTER_B_TDD_SWITCH_RX
 * PURPOSE:
 * STIMULI EVENTS: NO_PKD_RCVD or PKD, TIM0
 *
 ***********************************************************************/
    case OS_MASTER_B_TDD_SWITCH_RX:
    {
        if ((NO_LOOK_AHEAD == look_ahead) &&
            (RX_NO_PACKET != status) &&
            (FHSpkt == pkt_rcvd))
        {
            t_FHSpacket rx_fhs_pkt;
            t_FHSpacket *p_fhs_pkt = &rx_fhs_pkt;
            LSLCacc_Extract_FHS_Payload(p_fhs_pkt);
            DL_Set_UAP_LAP(p_active_devicelink,
                FHS_Get_UAP_LAP(p_fhs_pkt) );
            DL_Set_NAP(p_active_devicelink, FHS_Get_NAP(p_fhs_pkt) );

            /*
             * once upon a time, RTX sent us FHSes with AM_ADDR 0
             * we need to protect against that!
             */
            if (!FHS_Get_AM_Addr(p_fhs_pkt))
            {
                if (BTtimer_Is_Expired(p_mss_info->new_connection_timer))
                {
                    p_mss_info->ctrl_state = OS_MASTER_B_END;
                }
                break;
            }

            /*
             * save old piconet information - i.e. devicelink (AM_ADDR,
             * syncword), intraslot offset, etc.
             */

            LSLCpkt_Generate_ID(p_active_devicelink, Master_Slave_Switch);
            /* flush IDpkt out early before comm ctrl registers are modified */
            LSLCacc_Enable_Transmitter();

            /*
             * no need to call
             *     LSLCpkt_Prepare_Receiver_Master_Slave_Switch(pDL);
             * here since we are performing a piconet switch (hence hardware will
             * lag a frame due to mods to intraslot offset), and changing context
             * from Master_Slave_Switch to Connection -> LSLCslot will sequence
             * automatically activities to prepare receiver, enable receiver
             */

            p_mss_info->new_connection_timer =
                BTtimer_Set_Slots(p_mss_info->new_connection_timeout);

            {
                /*
                 * Store clock offset from FHSpkt
                 */
                 t_clock remote_clk  = FHS_Get_CLK(p_fhs_pkt);
                 t_clock current_clk = HW_get_native_clk(); /* should be safe here */
                 t_clock clk_offset = (remote_clk - (current_clk & 0x0ffffffc)) + 4;
                 DL_Set_Clock_Offset(p_active_devicelink, clk_offset);
            }

            {
                /* 
                 * Update channel access code from FHSpkt
                 */
                t_syncword syncword;
                FHS_Get_Syncword_Ex(p_fhs_pkt, &syncword);
                DL_Set_Access_Syncword(p_active_devicelink,syncword);
            }

            /*
             * Setting the ctrl_state of the local Device Link will allow the 
             * USLCscheduler object to determine whether the device is a 
             * Master or a Slave within the connection.
             */
            USLCmss_Update_Ctrl_State_(p_active_devicelink, CONNECTED_TO_MASTER);

            /*
             * the following:
             *   DL_Set_Role_Peer(p_active_devicelink, MASTER);
             *   DL_Set_Tx_Seqn(p_active_devicelink, 1);
             *   DL_Set_Rx_Expected_Seqn(p_active_devicelink, 1);
             * are now replaced with:
             */
            DL_Reset_New_Connection(p_active_devicelink, MASTER);
            DL_Set_AM_Addr(p_active_devicelink, FHS_Get_AM_Addr(p_fhs_pkt));

            /*
             * Ensure that Hop Selection Kernel is setup
             */
            LSLCfreq_Assign_Frequency_Kernel(p_active_devicelink);

            USLCchac_Set_Device_State(Connection);

            p_mss_info->ctrl_state =
                OS_SLAVE_B_PICONET_SWITCH_INTRASLOT_ADJUST;
        }
        else if (BTtimer_Is_Expired(p_mss_info->new_connection_timer))
        {
            p_mss_info->ctrl_state = OS_MASTER_B_END;
        }
        else
        {
#if (PRH_BS_DEV_MSS_TRANSITIONS_RADIO_MODE_STANDBY==1)
            HWradio_SetRadioMode(RADIO_MODE_TX_RX);
#endif
            p_mss_info->ctrl_state = OS_MASTER_B_TDD_SWITCH_RX;
            LSLCpkt_Prepare_Receiver_Master_Slave_Switch(p_mss_info->p_active_devicelink);
        }
    }
    break;

/***********************************************************************
 *
 * STATE:  OS_SLAVE_B_PICONET_SWITCH_INTRASLOT_ADJUST
 * PURPOSE:
 * STIMULI EVENTS: NO_PKD_RCVD, TIM0
 * 
 ***********************************************************************/
    case OS_SLAVE_B_PICONET_SWITCH_INTRASLOT_ADJUST:
    {
        /*
         * set intraslot offset
         */
        if (NO_LOOK_AHEAD == look_ahead)
        {
            /* slot offset of peer */
            u_int16 tmp_slot_offset = 1250 -
            DL_Get_Peer_Slot_Offset(&p_mss_info->stored_devicelink);

            if (tmp_slot_offset == 1250) 
            { 
                tmp_slot_offset = 0; 
            }
            if (tmp_slot_offset > 1250) 
            {
                HCeg_Hardware_Error_Event(PRH_BS_HW_ERROR_MSS_FAILURE);

                USLCmss_Cancel();
                return;
            }
            HW_set_intraslot_offset(tmp_slot_offset);

        p_mss_info->ctrl_state = OS_SLAVE_B_PICONET_SWITCH_RX;
        }
        
    }
    break;

/***********************************************************************
 *
 * STATE:  OS_SLAVE_B_PICONET_SWITCH_RX
 * PURPOSE:
 * STIMULI EVENTS: NO_PKD_RCVD or PKD, TIM0
 * 
 ***********************************************************************/
    case OS_SLAVE_B_PICONET_SWITCH_RX:
    {
        /*
         * staggered updating of BT Clk offset 1 frame after updating of intraslot offset
         * registers to ensure no race conditions in HW
         */
        if (FALSE == p_mss_info->master_b_piconet_switch_complete)
        {
            t_frame_pos current_frame_pos;
            /* 
             * Ensure that the clock offset adjust position is safe
             *
             * If in TX_START Slot Then
             *    If its safe to Read/Write Native_CLK in RX_START Then
             *        Wait for RX_START
             *    Else
             *        TX_START Slot is OK
             *    Endif
             * else If in RX_START Slot Then
             *    If its safe to Read/Write Native_CLK in TX_START Then
             *        Wait for TX_START
             *    Else
             *        RX_START Slot is OK
             *    Endif
             * Endif
             *
             *
             */

            if ((HW_get_bt_clk() & 0x3) == 2) /* TIM0/TX_START */
            { 
                current_frame_pos = TX_START;
                if (!LSLCclk_Is_Safe_To_Read_Native_Clk(TX_START))
                {
                    LSLCclk_Wait_For(RX_START);
                    current_frame_pos = RX_START;
                } 
            }
            else if ((HW_get_bt_clk() & 0x3) == 0) /* TIM2/RX_START */
            {
                current_frame_pos = RX_START;
                if (!LSLCclk_Is_Safe_To_Read_Native_Clk(RX_START))
                {
                    LSLCclk_Wait_For(TX_START);
                    current_frame_pos = TX_START;
                } 
            }
            else
            {
                break;
            }

            /* 
             * Now its safe to do the Clock Offset write
             */
            {
               /* 
                * Hardware Clock -> Set Offset directly 
                */
               t_clock clk_offset_adjusted = DL_Get_Clock_Offset(p_active_devicelink);
 
               /* adjustment for bt clk relative */
               clk_offset_adjusted += ((HW_get_native_clk() & 0x0ffffffc) - (HW_get_bt_clk() & 0x0ffffffc));
               
               /*
                * If in a Slave TX_START, CLK_N has just transitioned during Receive Frame
                * Hence clock deltas should be out bt 4.
                * Need to revisit logic
                */
               if (current_frame_pos == TX_START)
               {
                   clk_offset_adjusted -= 4;
               }
                  
               HW_set_bt_clk_offset(clk_offset_adjusted);
               
               p_mss_info->master_b_piconet_switch_complete = TRUE; 
            }
        }

        if (LOOK_AHEAD == look_ahead)
        {
            return;
        }

        if ((RX_NO_PACKET != status) && (POLLpkt == pkt_rcvd))
        {
            LSLCpkt_Generate_NULL(p_active_devicelink);
            LSLCacc_Enable_Transmitter();
            LSLCpkt_Prepare_Receiver_Connection(p_active_devicelink);

            p_mss_info->ctrl_state = OS_SLAVE_B_CONNECTED_TO_MASTER;
//            SYSmmi_Display_Event(eSYSmmi_LC_Connection_Event);

            /* 
             * free the previously allocated AM Address 
             */
            DL_Free_Am_Addr( DL_Get_Am_Addr(&p_mss_info->stored_devicelink) );

            /* 
             * DL_Set_Rx_Status(p_mss_info->p_active_devicelink, RX_NO_PACKET);
             *   -- out because caused problems with TESTsupport! 
             */
        }
        else if (BTtimer_Is_Expired(p_mss_info->new_connection_timer))
        {
            p_mss_info->ctrl_state = OS_MASTER_B_END;
        }
        else
        {
            LSLCpkt_Prepare_Receiver_Master_Slave_Switch(p_active_devicelink);
        }
    }
    break;

/***********************************************************************
 *
 * STATE:  OS_MASTER_B_END
 * PURPOSE:   Role Switch Failure - Recover original piconet master 
 *            parameters
 * 
 * probably need a common function to clean up and return to piconet
 * for both master and slave cases
 ***********************************************************************/
    case OS_MASTER_B_END:
    {
        USLCchac_Set_Active_Procedure(MSS_PROCEDURE_MASTER_CONTEXT);

        DL_Free_Am_Addr(DL_Get_Am_Addr(p_active_devicelink));

        /*
         * Return the new slave's piconet index is returned to pool.
         */
        DL_Free_Piconet_Index(p_active_devicelink->piconet_index);

        /* 
         * restore intraslot offset, restore devicelink 
         */
        *p_mss_info->p_active_devicelink = p_mss_info->stored_devicelink;

        USLCmss_Update_Ctrl_State_(p_active_devicelink, CONNECTED_TO_SLAVE);

        HW_set_slave(0);

        p_mss_info->ctrl_state = OS_RECOVER_ORIGINAL_PICONET;
    }
    break;

/***********************************************************************/
/*
 * common state - Reconnected on new Piconet
 */

/***********************************************************************
 *
 * STATE:  OS_RECONNECTED_NEW_PICONET
 * PURPOSE:
 * STIMULI EVENTS: TIM0
 * 
 ***********************************************************************/
    case OS_SLAVE_B_CONNECTED_TO_MASTER:
    {
        if ((RX_NO_PACKET != status) &&
            (POLLpkt == pkt_rcvd))
        {
            LSLCpkt_Generate_NULL(p_active_devicelink);
            LSLCacc_Enable_Transmitter();
            LSLCpkt_Prepare_Receiver_Connection(p_active_devicelink);
        }
    }
    /* deliberate fall-through */
    case OS_MASTER_A_CONNECTED_TO_SLAVE:
    {
        USLCchac_Procedure_Finish(TRUE /* resume previous connected super state */);

        /*
         * For encryption the Master's BD_ADDR must be set in the
         * hardware on the slave
         */
        HW_set_bd_addr_via_uap_lap_nap(
            DL_Get_UAP_LAP(p_active_devicelink),
            DL_Get_NAP(p_active_devicelink) );

        /* callback to LM */
        {
            t_LC_Event_Info event_info;

            event_info.status = NO_ERROR;
            event_info.fhs_packet = 0;
            event_info.deviceIndex = p_active_devicelink->device_index;

            /* LM should set max slotsito 1 */
            LC_Callback_Event_Handler(LC_EVENT_MSS_COMPLETE, &event_info);
        } 

        LSLChop_Set_HSE_Engine(LSLChop_HSE_DEFAULT);
        p_mss_info->ctrl_state = OS_INACTIVE;
    }

    break;

/***********************************************************************
 *
 * STATE:  OS_RECOVER_ORIGINAL_PICONET
 * PURPOSE:
 * 
 ***********************************************************************/
    case OS_RECOVER_ORIGINAL_PICONET:
    {

#if PRH_BS_DEV_MSS_USE_LSLC_SLOT_LOG
        /* 
         *Allow  for some logging of new clock (4 visits) 
         */
        static u_int8 change_when_N_visits = 0;
        if (++change_when_N_visits & 0x03)
        {
           break;
        }
#endif

        /*
         * Loosely Based on Errata 2047
         *  This is an idea to repeat the baseband switch procedure
         *  and thus avoid the missing "FHS-Ack" (ID) problem
         */
        p_mss_info->attempt++;
        if (p_mss_info->attempt != PRH_BS_DEV_MSS_MAX_NUM_ATTEMPTS)
        {
            p_mss_info->ctrl_state = OS_START;
            break;
        }

        USLCchac_Procedure_Finish(TRUE /* resume previous super state */);

        if (DL_Get_Role_Peer(p_active_devicelink) == MASTER)
        {
            /* active devicelink is now the stored device link */
            /* USLCr2p_Request(p_active_devicelink->device_index, 0); */
        }

        /* callback to LM */
        {
            t_LC_Event_Info event_info;

            event_info.status = EC_ROLE_SWITCH_FAILED; /* MSS Timeout */
            event_info.deviceIndex = p_active_devicelink->device_index;
            LC_Callback_Event_Handler(LC_EVENT_MSS_COMPLETE, &event_info);
        }

        USLCchac_Set_Device_State(Connection);

        /*
         * Ensure that Hop Selection Kernel is re-established
         */
        LSLCafh_Restore_AFH_Link_State(p_active_devicelink->device_index);
        
        LSLChop_Set_HSE_Engine(LSLChop_HSE_DEFAULT);
        p_mss_info->ctrl_state = OS_INACTIVE;

    
#if PRH_BS_DEV_MSS_USE_LSLC_SLOT_LOG
        LSLCslot_Disable_Interrupt_Log_If_Active();
#endif
    }

    break;
    }
}

/***********************************************************************/

/*
 * helper function to perform Master B TDD Switch
 */
static void USLCmss_Master_B_Perform_TDD_Switch_(
    t_USLCmss_Private_Data *p_mss_info, t_devicelink* p_active_devicelink)
{
    /*
     * I am Master B - Procedure from LKO 31 Aug 2000
     * 
     * Set SLAVE to 1
     * Set TX_MODE and RX_MODE to 0
     * Set INTRASET_OFFSET to 625
     * 
     * Set BT_CLK_OFFSET to 0
     */

    HW_set_intraslot_offset(625);
    HW_set_slave(1);
    HW_set_bt_clk_offset(2);

    /*
     * master will still use its native clk until piconet switch, not BT Clk!
     * hence no need to set BT Clk!
     * HW_set_bt_clk(HW_get_native_clk() + 2);
     */
    DL_Set_Local_Slot_Offset(p_active_devicelink, 625);
}

/***********************************************************************/

/*
 * helper function to perform Slave A TDD Switch
 */
static void USLCmss_Slave_A_Perform_TDD_Switch_(
    t_USLCmss_Private_Data *p_mss_info, 
    t_devicelink* p_active_devicelink)
{
    /*
     * I am Slave A - Procedure from LKO 31 August 2000
     *
     * Set TX_MODE and RX_MODE to 0
     * Set INTRASLOT_OFFSET to (INTRASLOT_OFFSET + 625) % 1250
     *
     * 27/7/2001 - Note this must be performed on TIM2
     * BT_CLK is updated when TIM_BIT_CNT + INTRASLOT_OFFSET = 1250 (0)
     *
     * To ensure an incorrect increment does not occur, then
     * we need to ensure we pull the combined value back in time.
     * the only way of doing this is to ensure the increment happens at TIM2
     * i.e. when 625 <= TIM_BIT_CNT + INTRASLOT_OFFSET < 1250
     *
     */
    u_int16 new_slot_offset = DL_Get_Local_Slot_Offset(p_active_devicelink) + 625;

    if (new_slot_offset >= 1250) { new_slot_offset -= 1250; } /* new_slot_offset %= 1250; */

    HW_set_intraslot_offset(new_slot_offset);
    DL_Set_Local_Slot_Offset(p_active_devicelink, new_slot_offset);

    if (LMconfig_LM_Connected_As_Master())
    {
        /*
         * Switch into piconet. Restore slave role and bdaddr,
         * as SP can be combined into TDD switch.
         */
         HW_set_bd_addr_via_uap_lap_nap(
             DL_Get_UAP_LAP(p_active_devicelink),
             DL_Get_NAP(p_active_devicelink) );
         HW_set_slave(1);
    }
}


/*
 * Spec Erratum 2019
 *
 * Master Slave Switch FHS requires the clock of Slave A, but is sent
 * on the piconet of Master B.  Since the clocks are asynchronous,
 * 
 */
static void USLCmss_Update_Ctrl_State_(t_devicelink* p_dev_link,
    t_device_ctrl_state ctrl_state)
{
    DL_Set_Ctrl_State(p_dev_link, ctrl_state);
    /*
     * Setting the ctrl_state of the local Device Link will allow the 
     * USLCscheduler object to determine whether the device is a 
     * Master or a Slave within the connection.
     */
    DL_Set_Ctrl_State(DL_Get_Local_Device_Ref(), ctrl_state);
}

#else
/*
 * End Guard of entire module (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1)
 */
#endif
