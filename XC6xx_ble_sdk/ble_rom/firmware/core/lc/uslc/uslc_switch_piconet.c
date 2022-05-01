/***********************************************************************
 *
 * MODULE NAME:    uslc_switch_piconet.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    LC Scatternet InterPiconet Switch Procedure
 * MAINTAINER:     Tom Kerwick 
 * CREATION DATE:  8 April 2002
 *
 * SOURCE CONTROL: $Id: uslc_switch_piconet.c,v 1.30 2013/11/21 16:37:48 garyf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2002-2004 Ceva Inc.
 *     All rights reserved.
 *
 *
 ***********************************************************************/

#include "sys_config.h"

#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)

#include "tra_queue.h"
#include "lc_types.h"
#include "lc_log.h"
#include "lslc_slot.h"
#include "hw_macro_defs.h"
#include "hw_memcpy.h"
#include "hw_radio.h"
#include "dl_dev.h"
#include "lslc_clk.h"
#include "lslc_hop.h"
#include "lslc_freq.h"
#include "lslc_irq.h"
#include "hc_const.h"
#include "hc_event_gen.h"
#include "uslc_chan_ctrl.h"
#include "uslc_scheduler.h"
#include "uslc_switch_piconet.h"
#include "uslc_sp_ctrl_states.h"
#include "lmp_link_policy.h"
#include "hw_leds.h"
#include "bt_timer.h"
#include "lmp_config.h"
#include "hw_delay.h"


typedef struct _USLCsp_Private_Data
{
    t_USLCsp_Ctrl_State ctrl_state;
    t_devicelink *p_active_devicelink;
    t_ulc_procedure procedure_context;
} t_USLCsp_Private_Data;

static t_USLCsp_Private_Data _sp_info;

static void _USLCsp_Update_Ctrl_State(t_devicelink* p_dev_link);

/***********************************************************************
 *
 * USLCsp_Initialise
 *
 * Initialises the defaults for SwitchPiconet procedure.
 *
 ***********************************************************************/
void USLCsp_Initialise(void)
{
    _sp_info.ctrl_state = OS_INACTIVE;
    _sp_info.p_active_devicelink = DL_Get_Local_Device_Ref();
}

/***********************************************************************
 *
 * USLCsp_Get_Activated_Device_Index
 *
 * Returns device link activated on latest piconet switch.
 *
 ***********************************************************************/
t_deviceIndex USLCsp_Get_Activated_Device_Index(void)
{
    return _sp_info.p_active_devicelink->device_index;
}

/***********************************************************************
 *
 * USLCsp_Set_Activated_Device_Index
 *
 * Set device link activated directly, on condition no SP active.
 *
 ***********************************************************************/
boolean USLCsp_Set_Activated_Device_Index(t_deviceIndex device_index)
{
    t_USLCsp_Private_Data *p_sp_info = &_sp_info;
    t_error status = UNSPECIFIED_ERROR;

    if (OS_INACTIVE == p_sp_info->ctrl_state)
    {
        p_sp_info->p_active_devicelink = DL_Get_Device_Ref(device_index);
        status = NO_ERROR;
    }

    return status;
}

/***********************************************************************
 *
 * USLCsp_Request
 *
 * Request from a higher layer to start a SwitchPiconet procedure.
 *
 ***********************************************************************/
t_error USLCsp_Request(t_deviceIndex device_index, boolean change_piconet)
{
    t_USLCsp_Private_Data *p_sp_info = &_sp_info;
    t_error status = UNSPECIFIED_ERROR;
    t_devicelink *p_requested_devicelink, *p_local_devicelink;

    LC_Log_Procedure_Request(SP_PROCEDURE);

    if ((change_piconet) || (OS_INACTIVE == p_sp_info->ctrl_state))
    {
        p_requested_devicelink = DL_Get_Device_Ref(device_index);
        p_local_devicelink = DL_Get_Local_Device_Ref();

        if ((DL_Get_Ctrl_State(p_requested_devicelink) == NOT_CONNECTED)
            || (p_local_devicelink == p_requested_devicelink))
        {
            /*
             * Returns to local device link as local piconet context.
             */
            USLCsp_Cancel();
            USLCsched_Local_Piconet_Request();
            p_sp_info->p_active_devicelink = p_local_devicelink;
        }
        else
        {
            /*
             * Start procedure in context of device role in requested
             * piconet if requested piconet is not currently active.
             */
        	{
                USLCsp_Cancel();
                p_sp_info->procedure_context = (t_ulc_procedure) (
                    (SLAVE == DL_Get_Role_Peer(p_requested_devicelink))?
                    (SP_PROCEDURE_MASTER_CONTEXT):(SP_PROCEDURE_SLAVE_CONTEXT));
                USLCchac_Procedure_Request(p_sp_info->procedure_context, TRUE);
                p_sp_info->ctrl_state = OS_START;
                LC_Log_Procedure_Accept(SP_PROCEDURE);
            }
        
            p_sp_info->p_active_devicelink = p_requested_devicelink;
        }

        status = NO_ERROR;
    }

    return status;
}

/***********************************************************************
 *
 * USLCsp_Cancel
 *
 * Request from a higher layer to cancel a SwitchPiconet procedure.
 * Must be followed by a new SP procedure.
 *
 ***********************************************************************/
t_error USLCsp_Cancel(void)
{
    t_USLCsp_Private_Data *p_sp_info = &_sp_info;

    if (OS_INACTIVE != p_sp_info->ctrl_state)
    {
        p_sp_info->ctrl_state = OS_INACTIVE;
        p_sp_info->p_active_devicelink = DL_Get_Local_Device_Ref();
        USLCchac_Procedure_Request(p_sp_info->procedure_context,FALSE);
    }

    return NO_ERROR;
}

/***********************************************************************
 *
 * USLCsp_State_Dispatcher
 *
 * One-shot statemachine for the SwitchPiconet procedure.
 *
 * STIMULI: TIM0(SLAVE_CONTEXT), TIM2(MASTER_CONTEXT).
 *
 ***********************************************************************/
void USLCsp_State_Dispatcher(boolean look_ahead)
{
    t_USLCsp_Private_Data *p_sp_info = &_sp_info;
    t_devicelink *p_active_devicelink = p_sp_info->p_active_devicelink;

    switch ((u_int8)p_sp_info->ctrl_state)
    {
        case OS_START:
        {
            LC_Log_Procedure_Start(SP_PROCEDURE);
            USLCchac_Procedure_Start(p_sp_info->procedure_context, p_active_devicelink);

            HWradio_SetRadioMode(RADIO_MODE_STANDBY);

            LSLCirq_Disable_All_Intr_Except(JAL_TIM0_INTR_MSK_MASK|JAL_TIM2_INTR_MSK_MASK);

			HW_set_add_bt_clk_relative(0);

            HW_set_tx_mode(0); HW_set_rx_mode(0);
            LSLChop_Set_HSE_Engine(LSLChop_HSE_SOFTWARE);
            USLCchac_Set_Device_State(Switch_Piconet);

            _USLCsp_Update_Ctrl_State(p_active_devicelink);

            if (SLAVE == DL_Get_Role_Peer(p_active_devicelink))
            {
                /*
                 * Connected as piconet master, so clear slave mode and bypass
                 * piconet_recover_clock for next state as os_end.
                 */
                HW_set_slave(0);
                HW_set_intraslot_offset(0);
                p_sp_info->ctrl_state = OS_END;
                goto _OS_END;
            }
            else /* (MASTER == DL_Get_Role_Peer(p_active_devicelink)) */
            {
                /*
                 * Connected as piconet slave, so set slave mode and intraslot
                 * offset, and update next state for piconet_recover_clock.
                 */

			//	if (!HW_get_slave())
				{
				// !!!!!!!!!!!!!!!!
				// REVIEW BELOW -- 
				// If device has not been in contact with peer for a period of time
				// we need to dynamically widen the window.
				// Below is a Hardcoded first step -- BUT more advanced mechanism needed.
				// !!!!!!!!!!!!!!!!
					extern t_SYS_Config g_sys_config;
					g_sys_config.win_ext=40;

					HW_set_slave(1);
					HWdelay_Wait_For_10us(1);
				}

                HW_set_intraslot_offset(DL_Get_Local_Slot_Offset(p_active_devicelink));
                p_sp_info->ctrl_state = OS_SWITCH_PICONET_RECOVER_CLK;
            }
        }
        //Disable of Reciever - makes no difference
		//LSLCpkt_Disable_Receiver();
        break;

        case OS_SWITCH_PICONET_RECOVER_CLK:
        {
            t_frame_pos current_frame_pos;
            t_clock clk_offset;

            /*
             * Ensure that the clock offset adjust position is safe
             */
            if (LSLCclk_Is_Safe_To_Read_Native_Clk(TX_START))
            {
                if ((HW_Get_Bt_Clk_Avoid_Race() & 0x3) != 2) /* TIM0/TX_START */
                {
                    LSLCclk_Wait_For(TX_START);
                }
                current_frame_pos = TX_START;
            }
            else /*(LSLCclk_Is_Safe_To_Read_Native_Clk(RX_START))*/
            {
                if ((HW_Get_Bt_Clk_Avoid_Race() & 0x3) != 0) /* TIM2/RX_START */
                {
                    LSLCclk_Wait_For(RX_START);
                }
                current_frame_pos = RX_START;
            }

            /*
             * Now safe to Write clock offset, so set offset directly
             */
            clk_offset = DL_Get_Clock_Offset(p_active_devicelink) & BT_CLOCK_MAX_TICKS;

            /*
             * If in a Slave TX_START, CLK_N has just transitioned during Receive Frame
             * Hence clock deltas should be adjusted by 4. Need to revisit logic.
             */
            if (current_frame_pos == TX_START)
            {
                clk_offset -= 4;
            }

            HW_set_bt_clk_offset(clk_offset);

            p_sp_info->ctrl_state = OS_END;
            goto _OS_END;
        }
        //Disable of Reciever - makes no difference
		//LSLCpkt_Disable_Receiver();
        break;

        case OS_END:
_OS_END:
		{
            USLCchac_Procedure_Finish(FALSE/* do not resume previous super state */);
            USLCchac_Set_Device_State(Connection);

            	HWradio_SetRadioMode(RADIO_MODE_TX_RX);

            LSLCirq_R2P_Turn_On_Intr();

            /*
             * Master's BD_ADDR must be in hardware for Encryption/HSE
             */
            if (DL_Get_Role_Peer(p_active_devicelink)==SLAVE)
            {
                HW_set_bd_addr(SYSconfig_Get_Local_BD_Addr_Ref());
            }
            else
            {
                HW_set_bd_addr_via_uap_lap_nap(
                    DL_Get_UAP_LAP(p_active_devicelink),
                    DL_Get_NAP(p_active_devicelink) );
            }

            LSLChop_Set_HSE_Engine(LSLChop_HSE_DEFAULT);

#if (PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_SUPPORTED==0)
            /*
             * Unlike AFH capable builds, which maintain an individual hs_kernel for each
             * piconet (_LSLCafh_AFH_hse_kernel[].hs_kernel), non-AFH capable builds
             * maintain just one hs_kernel (LSLChop_hs_kernel) for all piconets, so need
             * to have hop kernel setup for new piconet on each piconet switch.
             */
            LSLCfreq_Assign_Frequency_Kernel(p_active_devicelink);
#endif

            /*
             * Prepare to poll slaves on switching to a MASTER role.
             */
            if (DL_Get_Role_Peer(p_active_devicelink) == SLAVE)
            {
                LMpol_Adjust_Poll_Position_On_All_Links(0);
            }

            p_sp_info->ctrl_state = OS_INACTIVE;
        }
		HW_set_add_bt_clk_relative(1);
		LSLCacc_Enable_Transmitter();
		break;
    }
}

/***********************************************************************
 *
 * _USLCsp_Update_Ctrl_State
 *
 * Updates the ctrl_state of the active and local device links.
 *
 ***********************************************************************/
static void _USLCsp_Update_Ctrl_State(t_devicelink* p_dev_link)
{
    t_device_ctrl_state ctrl_state = DL_Get_Ctrl_State(p_dev_link);

#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
    /*
     * Although device link is connected to slave inactive due to a
     * parked slave, the overall piconet is active.
     */
    if (ctrl_state == CONNECTED_TO_SLAVE_INACTIVE)
        ctrl_state = CONNECTED_TO_SLAVE;
#endif

    /*
     * Setting the ctrl_state of the local Device Link will allow the 
     * USLCscheduler object to determine whether the device is a 
     * Master or a Slave within the connection.
     */
    DL_Set_Ctrl_State(DL_Get_Local_Device_Ref(), ctrl_state);
}
#else
void USLCsp_Dummy(void) {} /* Prevent empty file */

#endif /*(PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)*/
