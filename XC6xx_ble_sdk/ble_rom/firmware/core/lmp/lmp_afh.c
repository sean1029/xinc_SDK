/****************************************************************************
 * MODULE NAME:    lmp_afh.c
 * PROJECT CODE:   BlueStream
 * AUTHOR:         John Nelson
 * DATE:           01-08-2003
 *
 * LICENSE:
 *     This source code is copyright (c) 2002-2004 Ceva Inc.
 *     All rights reserved.
 *
 * SOURCE CONTROL: $Id: lmp_afh.c,v 1.46 2012/09/28 11:47:14 tomk Exp $
 * 
 * DESCRIPTION:     
 * This module is responsible for supporting AFH capability and
 * AFH classification reporting.
 *
 ****************************************************************************/ 
#include "sys_config.h"

#if (PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_SUPPORTED==1)

#include "lmp_afh.h"
#include "lmp_config.h"
#include "lmp_acl_container.h"
#include "lmp_acl_connection.h"
#include "lmp_link_policy_protocol.h"
#include "lmp_ch.h"
#include "lmp_encode_pdu.h"
#include "lmp_timer.h"
#include "lmp_features.h"
#include "lmp_utils.h"

#include "lc_interface.h"
#include "lslc_afh.h"
#include "lslc_class.h"
#include "uslc_chan_ctrl.h"
#include "dl_dev.h"
#include "hw_memcpy.h"
#include "bt_timer.h"

#include "bt_test.h"

/*
 * AFH classification 
 * Defined here as required even if no device classification
 */
STATIC u_int8 _LMafh_AFH_host_channel_classification[AFH_BYTES_PER_CLASSIFICATION];

/*
 * _LMafh_AFH_channel_assessment_mode
 *  The channel assessment mode refers to this devices own scheme
 */
STATIC u_int8  _LMafh_AFH_channel_assessment_mode;

/*
 * _LMafh_AFH_control_byte
 *
 * Global bit vector indicating AFH activities such as:
 *  LMPafh_CTRL_IDLE
 *  LMPafh_CTRL_UPDATE_PENDING          AFH state change       
 *  LMPafh_CTRL_FORCE_AFH_DISABLE       Force AFH disable
 *  LMPafh_CTRL_FORCE_RECOVERY_TESTMODE Force the instant to pass before BB Ack
 */
u_int8  _LMafh_AFH_control_byte;

/*
 * _LMafh_AFH_LMP_set_afh_pending_on_am_addrs
 *
 * Record and manage pending LMP_set_afh commands.
 * A single LMP_set_afh per slave can be pending (bit vector based on am_addr).
 */
STATIC u_int8 _LMafh_AFH_LMP_set_afh_pending_on_am_addrs;
#define mLMafh_Clear_AFH_Change_Pending_All_Slaves() \
    _LMafh_AFH_LMP_set_afh_pending_on_am_addrs = 0;
#define mLMafh_Set_AFH_Change_Pending(device_index) \
    _LMafh_AFH_LMP_set_afh_pending_on_am_addrs |= \
        (u_int8) (1<<DL_Get_Am_Addr(DL_Get_Device_Ref(device_index)));
#define mLMafh_Clear_AFH_Change_Pending(device_index) \
    _LMafh_AFH_LMP_set_afh_pending_on_am_addrs &= \
        ~(u_int8) (1<<DL_Get_Am_Addr(DL_Get_Device_Ref(device_index)));
#define mLMafh_Is_AFH_Change_Pending(device_index) \
    (_LMafh_AFH_LMP_set_afh_pending_on_am_addrs & \
        (u_int8) (1<<DL_Get_Am_Addr(DL_Get_Device_Ref(device_index))))

u_int8 LMafh_Is_AFH_Change_Pending(t_deviceIndex device_index)
{
    return (_LMafh_AFH_LMP_set_afh_pending_on_am_addrs &
        (u_int8) (1<<DL_Get_Am_Addr(DL_Get_Device_Ref(device_index))));
}


/*
 * _LMafh_AFH_request_channel_classification_on_piconets
 *
 * Record and manage requests for channel classification updates.
 *  (bit vector based on piconet index)
 */
STATIC u_int8 _LMafh_AFH_request_channel_classification_on_piconets;
#define mLMafh_Set_Request_Channel_Classification_All_Piconets() \
    _LMafh_AFH_request_channel_classification_on_piconets = 0xFF;
#define mLMafh_Clear_Request_Channel_Classification_All_Piconets() \
    _LMafh_AFH_request_channel_classification_on_piconets = 0x00;
#define mLMafh_Clear_Request_Channel_Classification(piconet_index) \
    _LMafh_AFH_request_channel_classification_on_piconets &= \
        ~(u_int8) (1<<piconet_index);
#define mLMafh_Is_Request_Channel_Classification(piconet_index) \
    (_LMafh_AFH_request_channel_classification_on_piconets & \
        (u_int8) (1<<piconet_index))


#if (PRH_BS_CFG_SYS_CHANNEL_ASSESSMENT_SCHEME_SUPPORTED==1)

void LMafh_Set_Request_Channel_Classification(u_int8 piconet_index)
{
    _LMafh_AFH_request_channel_classification_on_piconets |= 
        (u_int8) (1<<piconet_index);
}

void LMafh_Set_Device_Channel_Map_Update_Notification()
{
    _LMafh_AFH_control_byte |= LMPafh_CTRL_DEVICE_UPDATE;
}

#endif

/*
 * _LMafh_AFH_LMP_channel_classification_pending_on_piconets
 *
 * Record and manage pending LMP_channel_classification command.
 * A single LMP_channel_classification per master can be pending 
 * (bit vector based on piconet index ).
 */
STATIC u_int8 _LMafh_AFH_LMP_channel_classification_pending_on_piconets;
#define mLMafh_Set_LMP_Channel_Classification_Pending(piconet_index) \
    _LMafh_AFH_LMP_channel_classification_pending_on_piconets |= \
        (u_int8) (1<<piconet_index);
#define mLMafh_Clear_LMP_Channel_Classification_Pending(piconet_index) \
    _LMafh_AFH_LMP_channel_classification_pending_on_piconets &= \
        ~(u_int8) (1<<piconet_index);
#define mLMafh_Is_LMP_Channel_Classification_Pending(piconet_index) \
    (_LMafh_AFH_LMP_channel_classification_pending_on_piconets & \
        (u_int8) (1<<piconet_index))

#if (PRH_BS_DEV_SUSPEND_SLAVE_CLASSIFICATIONS_SUPPORTED==1)
/*
 * _LMafh_AFH_LMP_channel_classification_req_pending_on_slaves
 *
 * Record and manage pending LMP_channel_classification_req command.
 * A single LMP_channel_classification_req per slave can be pending 
 * (bit vector based on device index).
 */
STATIC u_int8 _LMafh_AFH_LMP_channel_classification_req_pending_on_slaves;
#define mLMafh_Set_LMP_Channel_Classification_Req_Pending(device_index) \
    _LMafh_AFH_LMP_channel_classification_req_pending_on_slaves |= \
        (u_int8) (1<<device_index);
#define mLMafh_Clear_LMP_Channel_Classification_Req_Pending(device_index) \
    _LMafh_AFH_LMP_channel_classification_req_pending_on_slaves &= \
        ~(u_int8) (1<<device_index);
#define mLMafh_Clear_LMP_Channel_Classification_Req_Pending_All_Slaves() \
    _LMafh_AFH_LMP_channel_classification_req_pending_on_slaves = 0;
#define mLMafh_Is_LMP_Channel_Classification_Req_Pending() \
    (_LMafh_AFH_LMP_channel_classification_req_pending_on_slaves)
#endif

/*
 * AFH will use an LMP timer to schedule the updates as necessary
 *
 * Master Piconet 0     schedules updates for AFH channel maps
 * Slave  Piconets      schedules updates for AFH classification
 */
STATIC u_int8  _LMafh_AFH_update_timer_index[PRH_BS_CFG_SYS_MAX_PICONETS];
STATIC t_slots _LMafh_AFH_update_timeout[PRH_BS_CFG_SYS_MAX_PICONETS];

/*
 * _LMafh_Master_LMP_set_AFH_PDU
 *
 * Maintains full AFH PDU including channel map which describes piconet AFH
 *
 *      Bytes: 0:Opcode, 1-4:AFH_instant, 5:AFH_mode, 6-15:AFH_channel_map
 */
STATIC u_int8  _LMafh_Master_LMP_set_AFH_PDU[16];

#define _LMafh_AFH_p_opcode      (_LMafh_Master_LMP_set_AFH_PDU+0)
#define _LMafh_AFH_p_instant     (_LMafh_Master_LMP_set_AFH_PDU+1)
#define _LMafh_AFH_p_mode        (_LMafh_Master_LMP_set_AFH_PDU+5)
#define _LMafh_AFH_p_channel_map (_LMafh_Master_LMP_set_AFH_PDU+6)

/*
 * Helper functions
 */
static void LMafh_Update_Slave_AFH_If_Required(t_lmp_link *p_link);
static void _LMafh_Set_AFH_Instant(t_clock afh_instant_ticks);
t_clock _LMafh_Get_AFH_Instant(void);

void LMafh_Update_Master_AFH_Piconet(t_lmp_link *p_link);

/****************************************************************************
 * LMafh_Initialise()
 *
 * Description
 * Handles any initialisation for LMP AFH module
 ****************************************************************************/ 
void LMafh_Initialise(void)
{
    u_int i;
    /*
     * Default for AFH classification is enabled if supported
     */
#if 0 // For ARQ_BV30 need to change our default setting for channel ASSESSMENT
	  // now it is disabled by default - but can be re-enabled via HCI Command

    _LMafh_AFH_channel_assessment_mode = 
        PRH_BS_CFG_SYS_CHANNEL_ASSESSMENT_SCHEME_SUPPORTED;
#else

    _LMafh_AFH_channel_assessment_mode = 0;

#endif
    hw_memcpy8(_LMafh_AFH_host_channel_classification, 
        mLSLCafh_Get_79_Bit_Vector_Set_Ref(), AFH_BYTES_PER_CLASSIFICATION);

    hw_memset8(_LMafh_Master_LMP_set_AFH_PDU, 0x00, 
        mNum_Elements(_LMafh_Master_LMP_set_AFH_PDU));

    _LMafh_AFH_control_byte = LMPafh_CTRL_IDLE;
    mLMafh_Clear_AFH_Change_Pending_All_Slaves();
#if (PRH_BS_DEV_SUSPEND_SLAVE_CLASSIFICATIONS_SUPPORTED==1)
    mLMafh_Clear_LMP_Channel_Classification_Req_Pending_All_Slaves();
#endif

    /*
     * Start timer based AFH reporting based on default timeout
     */
    _LMafh_AFH_update_timer_index[0] = LMtmr_Set_Timer(
         LMPafh_AFH_UPDATE_DEFAULT_SLOTS, LMafh_Update_Master_AFH_Piconet, 
         (t_lmp_link*) 0, 0 /*Persistent Timer*/);
    _LMafh_AFH_update_timeout[0] = LMPafh_AFH_UPDATE_DEFAULT_SLOTS;

    for(i=1; i<PRH_BS_CFG_SYS_MAX_PICONETS; i++)
    {
        _LMafh_AFH_update_timer_index[i] = 0;
        _LMafh_AFH_update_timeout[i] = 0;
    }

    /*
     * Setup the LMP_set_AFH pdu, opcode 60, master tid
     */
    *_LMafh_AFH_p_opcode = (LMP_SET_AFH << 1) + 0;

    /*
     * Initialise the LMP Slave LMP classification PDU 
     * including opcode.  All channels are good.
     */
    LMafh_Initialise_Classification();
}

/****************************************************************************
 * LMafh_LMP_Set_AFH()
 *
 * Description
 *  Handles the incoming LMP_Set_AFH PDU which enables/disables AFH.
 *
 * From V1.2 Draft Spec:
 ****************************************************************************/ 
t_error LMafh_LMP_Set_AFH(t_lmp_link *p_link, t_p_pdu p_payload)
{   
    t_error status;
    if ( SYSconfig_Is_Feature_Supported(
            PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_SLAVE_FEATURE) )
    {
        status = LSLCafh_set_AFH(p_link->device_index, p_payload);
    }
    else
    {
        status = UNSUPPORTED_REMOTE_FEATURE;
    }
    return status;
}

/****************************************************************************
 * Function :- LMafh_Device_Channel_Map_Update_Notification()
 *
 * Description
 *
 * Informs the LMP that a Device Channel Map update has now occurred.
 *
 ****************************************************************************/
#if (PRH_BS_CFG_SYS_CHANNEL_ASSESSMENT_SCHEME_SUPPORTED==1)
void LMafh_Device_Channel_Map_Update_Notification(void)
{
    if (LMconfig_LM_Connected_As_Master())
    {
        _LMafh_AFH_control_byte |= LMPafh_CTRL_DEVICE_UPDATE;
    }
    if (LMconfig_LM_Connected_As_Slave())
    {
        mLMafh_Set_Request_Channel_Classification_All_Piconets();
    }
}
#endif

/****************************************************************************
 * Function :- LMafh_Update_Master_AFH_Piconet()
 *
 * Description
 *
 * The following steps are required
 *
 * For each active slave with AFH supported, it is defined that:
 *
 * 1. Stop L2CAP and finalise any L2CAP transmission
 * 2. Send the LMP_set_AFH command with 
 *    where instant is at least 6*Tpoll_max in the future after L2CAP stop
 * 3. Enable L2CAP on baseband Acknowledgement of LMP_set_AFH
 *
 * Note in our architecture LMP has priority.
 * It seems that L2CAP is stopped to prevent message overrun.
 * Option is to put a 0 length L2CAP message on queue.
 *
 * Release 1.0 AFH will just send a message to each slave based on
 * 6 * default POLL
 *
 * Parameters
 *    p_link                    Only used in case of single device change
 *
 ****************************************************************************/ 
void LMafh_Update_Master_AFH_Piconet(t_lmp_link *p_link)
{
    u_int i, num_afh_slaves;
    t_clock current_afh_instant_ticks, link_afh_instant_ticks;
    t_clock native_clock_ticks;

    boolean afh_capable_piconet;
    t_lmp_link *piconet_plink[PRH_MAX_ACL_LINKS];

    extern t_link_entry link_container[PRH_MAX_ACL_LINKS];

    /*
     * Schedule the next update
     */
    LMtmr_Reset_Timer(_LMafh_AFH_update_timer_index[0],
                    _LMafh_AFH_update_timeout[0]);


#if (PRH_BS_CFG_SYS_TEST_MODE_SUPPORTED==1)
    if (BTtst_Get_DUT_Mode() > DUT_ACTIVE)
    {
        return;
    }
#endif

    /*
     * If previous AFH instant not expired, do not perform an update.
     */
    if (!p_link && !BTtimer_Is_Expired(_LMafh_Get_AFH_Instant()))
    {
        return;
    }

	if ((_LMafh_AFH_control_byte & (LMPafh_CTRL_PICONET_UPDATE|LMPafh_CTRL_DEVICE_UPDATE|LMPafh_CTRL_FORCE_AFH_DISABLE))
     && (!LMconfig_LM_Check_Device_State(PRH_DEV_STATE_MSS_ACTIVE|PRH_DEV_STATE_PARK_PENDING|PRH_DEV_STATE_UNPARK_PENDING)))
    {
        /*
         * Piconet is default enabled if not forced disable.
         */
        afh_capable_piconet = 
            !(_LMafh_AFH_control_byte & LMPafh_CTRL_FORCE_AFH_DISABLE);

        num_afh_slaves = 0;

        native_clock_ticks = LC_Get_Native_Clock();
        current_afh_instant_ticks = native_clock_ticks + 192 /*min 96 slots*/;

        /*
         * Determine whether Piconet is AFH capable 
         */
        for (i = 0; i < PRH_MAX_ACL_LINKS; i++)
        {
            if (link_container[i].used) 
            {
                t_lmp_link *p_link = &(link_container[i].entry);

                if (mLMafh_Is_AFH_Update_State(p_link->state) && MASTER == p_link->role)
                {
                    if (mLMPfeat_Is_Remote_Feature_Supported( 
                            p_link->remote_features, 
                            PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_SLAVE_FEATURE) )
                    {
                        piconet_plink[num_afh_slaves++] = p_link;
                    }
                    else
                    {
                        afh_capable_piconet = FALSE;
                    }

                    /*
                     * Now figure out a minimum safe instant if in Sniff/Hold or Park
                     */
#if (PRH_BS_DEV_EARLY_SET_AFH_ON_CONNECTION_ESTABLISHMENT==1)
					if (p_link->state & (LMP_ACTIVE|W4_LMP_ACTIVE))
#else
					if (p_link->state & LMP_ACTIVE)
#endif
                    {
                        link_afh_instant_ticks = native_clock_ticks;

#if (PRH_BS_CFG_SYS_SCATTERNET_SUPPORTED==1)
                        if (LMconfig_LM_Connected_As_Scatternet())
                        {
                            /*
                             * It is safer to pick an instant the piconet service period from
                             * here, than to use the time this piconet is next activated.
                             */
                            link_afh_instant_ticks += PRH_BS_PICONET_SERVICE_TIME;
                        }
#endif
                    }
#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUPPORTED==1)
                    else if (p_link->state & LMP_SNIFF_MODE)
                    {
                        t_lm_sniff_info *p_sniff = &p_link->pol_info.sniff_info;
                        /*
                         * It is safer to pick an instant Tsniff from here,
                         * than to use the next_sniff_window which may
                         * be currently active
                         */
                        link_afh_instant_ticks = 
                            native_clock_ticks + (p_sniff->T_sniff<<1)
#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUBRATING_SUPPORTED==1)
                            * LMpol_Get_Common_Subrate(p_link)
#endif
                            ;
                    }
#endif
                    else if (p_link->state & LMP_HOLD_MODE)
                    {
                        /*
                         * If any device on hold, do not perform an update
                         */
                        return;

                        /*
                         * Alternatively we can select the afh instant
                         * for after hold completes.  Above option results 
                         * in updates that are more recent.
                         * 
                         * t_lm_hold_info *p_hold = &p_link->pol_info.hold_info;
                         *
                         * link_afh_instant_ticks = 
                            ((p_hold->hold_instant + p_hold->hold_time)<<1);
                         */
                    }
                    else /*if (p_link->state & LMP_PARK_MODE)*/
                    {
#if (PRH_BS_DEV_ACL_DRIVEN_AUTOPARK_SUPPORTED==0)
                        /*
                         * If any device in park, do not perform an update
                         */
                        return;
#else
                        /*
                         * Inserting an LMP message on the LMP queue will
                         * activate an automatic LC unpark/park operation
                         *
                         * Currently one device can be unparked per beacon, 
                         * hence instant must allow for all devices to be 
                         * unparked.  Allow a number of beacon attempts.
                         */
                        link_afh_instant_ticks = 
                            g_LM_config_info.beacon_instant +
                            (g_LM_config_info.num_links_parked * 
                                g_LM_config_info.T_bcast /*slots*/ * 4);
#endif
                    }

                    {
                        u_int16 poll_interval;
#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUPPORTED==1)
                        if(p_link->state & LMP_SNIFF_MODE)
                        {
                            poll_interval = p_link->pol_info.sniff_info.T_sniff
#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUBRATING_SUPPORTED==1)
                                  * LMpol_Get_Common_Subrate(p_link)
#endif
                                ;
                        }
                        else
#endif
                        {
                            poll_interval = p_link->poll_interval;
                        }
                                                
                        if (USLCchac_get_device_state() & (Inquiry|Page))
                        {
#if (PRH_BS_DEV_PRIORITY_ACL_IN_INQ_PAGE_SUPPORTED==0)
                            /*
                             * If any device in inquiry/page, do not perform an update
                             */
                            return;
#else
                            /*
                             * Override to use 6 by emergency poll slots where necessary
                             */
                            if (g_LM_config_info.emergency_poll_interval > poll_interval)
                            {
								poll_interval = g_LM_config_info.emergency_poll_interval;
                            }
#endif
                        }
#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUPPORTED==1)
                        if(p_link->state & LMP_SNIFF_MODE)
                        {
                            /*
                             * Ensure equivalent of 6 by T_poll. For low N_sniff,
                             * multiple sniff intervals are required.
                             */
                            u_int16 polls_per_interval = p_link->pol_info.sniff_info.N_sniff;
                            
                            if (polls_per_interval < 6)
                                poll_interval *= (12/(polls_per_interval+1));

                            link_afh_instant_ticks += (poll_interval<<1);
                        }
                        else
#endif
                        {
                            /*
                             * Ensure addition of 6 by T_poll
                             */
                            link_afh_instant_ticks += 12 * poll_interval;
                        }
                    }

                    /*
                     * Update current afh instant if this link instant is later
                     */
                    link_afh_instant_ticks &= 0x0FFFFFFC;
                    if (BTtimer_Is_Expired_For_Time(
                            current_afh_instant_ticks /*timer*/, 
                            link_afh_instant_ticks  /*ref time ticks*/) )
                    {
                        current_afh_instant_ticks = link_afh_instant_ticks;
                    }
                }
                else if ((p_link->state != LMP_IDLE) && (MASTER == p_link->role))
                {
                    if (AFH_MODE_DISABLED != mLSLCafh_Get_AFH_Current_Mode(p_link->device_index))
                    {
                        /*
                         * If any AFH enabled slave cannot be updated, do not perform an update
                         */
                        return;
                    }
                }
            }
        }

        /*
         * Complete the update to the Master controlled piconet
         */

        {
           /*
            * Determine the channel map from host and device classifications
            */
		    boolean change_occurred = afh_capable_piconet && LSLCcls_Build_Channel_Map(
			    _LMafh_AFH_p_channel_map, _LMafh_AFH_host_channel_classification);

            if(change_occurred || (*_LMafh_AFH_p_mode != afh_capable_piconet)
               || (_LMafh_AFH_control_byte & LMPafh_CTRL_PICONET_UPDATE))
			{
                /*
                 * Cancel the requested update (a new request may result as
                 * a consequence of the update attempts below).
                 */
                _LMafh_AFH_control_byte &= ~(LMPafh_CTRL_PICONET_UPDATE|LMPafh_CTRL_DEVICE_UPDATE);

                /*
                 * Enable/Disable for all devices on the piconet at instant
                 */
                *_LMafh_AFH_p_mode = afh_capable_piconet;
                _LMafh_Set_AFH_Instant(current_afh_instant_ticks);

                /* 
                 * Update each active slave and its associated Kernel
                 */
                for (i = 0; i < num_afh_slaves; i++)
                {
                    LMafh_Update_Slave_AFH_If_Required(piconet_plink[i]);
                }

                /*
                 * Update the broadcast device index
                 */
                LSLCafh_set_AFH(0, _LMafh_Master_LMP_set_AFH_PDU+1);
            }
        }
    }

}

/****************************************************************************
 * Function :- LMafh_Update_Slave_AFH_If_Required()
 *
 * Description
 *
 * If there is no pending requested Then
 *    Send request to peer via LMP_set_afh message
 *    Send request to local AFH engine
 * Endif
 * 
 * Parameters
 *  p_link        Reference to lmp link container
 *
 * Global parameters
 *  _LMafh_Master_LMP_set_AFH_PDU
 ****************************************************************************/ 
static void LMafh_Update_Slave_AFH_If_Required(t_lmp_link *p_link)
{
    t_deviceIndex device_index = p_link->device_index;

    if (mLMafh_Is_AFH_Change_Pending(device_index))
    {
        /*
         * Request timed update at later date for the piconet
         */
        _LMafh_AFH_control_byte |= LMPafh_CTRL_PICONET_UPDATE;
    }
    else
    {
        /*
         * Prevent other updates until this completed.
         */ 
        mLMafh_Set_AFH_Change_Pending(device_index);

		LMch_Disable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_AFH);
		/*
         * Encode SET_AFH. AFH Change Pending cleared on BB ack.
         */
        if (LM_Encode_LMP_PDU_Fully_Encoded(device_index, LMP_SET_AFH,
            _LMafh_Master_LMP_set_AFH_PDU)!=NO_ERROR)
        {
            /*
             * If LMP_SET_AFH enqueue error (eg queue full), then must not leave
             * the AFH_Change_Pending flag set, and must not disable L2CAP traffic,
             * as will be no BB ack to clear flag and re-enable. Allow update of
             * local AFH engine however, so will recover to AFH(79) after HSSI.
             */
            mLMafh_Clear_AFH_Change_Pending(device_index);
			LMch_Enable_L2CAP_Traffic(p_link, LM_CH_L2CAP_EN_AFH);
		}

		/*
         * Update the local AFH engine for this device
         */
        LSLCafh_set_AFH(device_index, _LMafh_Master_LMP_set_AFH_PDU+1);
    }
}

/****************************************************************************
 * LMafh_LMP_Set_AFH_Ack
 *
 * Process the baseband ACK to the LMP_Set_AFH
 *
 * The baseband ACK if it occurs after HSSI, required a recovery 
 * procedure where an AFH update is taking place.
 * This effectively means using AFH(79) between HSSI and ACK.
 ****************************************************************************/
void LMafh_LMP_Set_AFH_Ack(t_deviceIndex device_index)
{
    /*
     * L2CAP traffic was required to be stopped on AFH update
     */
    LMch_Enable_L2CAP_Traffic(LMaclctr_Find_Device_Index(device_index),LM_CH_L2CAP_EN_AFH);
    /*
     * Indicate to LSLC that any recovery period can be terminated.
     */
    LSLCafh_Process_Baseband_Ack(device_index);
    /*
     * Clear the AFH pending flag for this device (am_addr)
     */
    mLMafh_Clear_AFH_Change_Pending(device_index);
}

/****************************************************************************
 * LMafh_LMP_Channel_Classification_Ack
 *
 * Process the baseband ACK to the LMP_Channel_Classification
 *
 * Prevent sending a LMP_Channel_Classification while one is pending
 ****************************************************************************/
void LMafh_LMP_Channel_Classification_Ack(t_deviceIndex device_index)
{
    t_lmp_link *p_link = LMaclctr_Find_Device_Index(device_index);

    t_piconet_index piconet_index = 0;

    u_int8 *p_timer_index = 0;
    
    if(p_link)
    {
        piconet_index = DL_Get_Piconet_Index(
            DL_Get_Device_Ref(p_link->device_index));
    }

    p_timer_index = _LMafh_AFH_update_timer_index+piconet_index;

    mLMafh_Clear_LMP_Channel_Classification_Pending(
        DL_Get_Piconet_Index(DL_Get_Device_Ref(device_index)));

    /*
     * Schedule the next update 
     */
    if(p_link)
    {
        LMtmr_Reset_Timer(*p_timer_index, 
            _LMafh_AFH_update_timeout[piconet_index]);
    }
}

/************************************************************************
 * LMafh_Activate_AFH
 *
 * Activate AFH engine for device if supported.
 *
 * p_link           Reference to the associated lmp_link for this device
 *
 * Notes:
 * Ensuring that the broadcast device link is correct requires care.
 * A full piconet update is required on first link to ensure that
 * the broadcast device link is correct for AFH active.
 ************************************************************************/
void LMafh_Activate_AFH(t_lmp_link *p_link)
{
    boolean slave_is_afh_capable;

    if (p_link->role == MASTER)
    {
        slave_is_afh_capable = mLMPfeat_Is_Remote_Feature_Supported( 
             p_link->remote_features, PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_SLAVE_FEATURE);

        if (slave_is_afh_capable && *_LMafh_AFH_p_mode &&
            g_LM_config_info.num_acl_links > 1)        
        {
            /*
             * If peer supports AFH and piconet already AFH active, then just
             * send the same PDU to this new slave with updated instant:
             * Set the instant to min (96, 6 * Default T_poll) in slots
             */
            const u_int16 min_poll_interval_slots = 16;  
            t_clock poll_interval_slots = p_link->poll_interval;

            if (poll_interval_slots < min_poll_interval_slots)
            {
                poll_interval_slots = min_poll_interval_slots;
            }

            _LMafh_Set_AFH_Instant(LC_Get_Native_Clock()+12*poll_interval_slots);

            LMafh_Update_Slave_AFH_If_Required(p_link);
        }
        else
        {
            /*
             * Either slave is not active OR we have no AFH active OR 1st slave
             * Request an immediate update of the piconet.
             * For 1st slave this is required to ensure broadcast is valid.
             */
            _LMafh_AFH_control_byte |= LMPafh_CTRL_PICONET_UPDATE; 
            if (slave_is_afh_capable)
            {
                LMafh_Update_Master_AFH_Piconet(p_link);
            }
        }
        
        /*
         * Request classification reporting on slave if supported.
         */
        LMPafh_Request_Classification_Reporting(p_link,1/*enable*/);
    }
}

/************************************************************************
 * LMafh_Deactivate_AFH
 *
 * De-activate AFH engine for device if supported.
 *
 * p_link           Reference to the associated lmp_link for this device
 *                  If NULL deactivate for all devices
 *
 * if I'm the master then
 *    if disconnecting slave doesn't support AFH then
 *        I need to schedule an full piconet update of remaining slaves
 *    else
 *        Nothing to do
 *    endif
 * endif
 *
 * Notes:
 * The broadcast device link is implicitly handled by ensuring that
 * when activating the first slave (rf LMafh_Activate_AFH(), 
 * that it is correctly setup by always forcing a full piconet update..
 ************************************************************************/
void LMafh_Deactivate_AFH(t_lmp_link *p_link)
{
    if (p_link->role == MASTER)
    {
        if (!mLMPfeat_Is_Remote_Feature_Supported( 
            p_link->remote_features, PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_SLAVE_FEATURE) )
        {
            /*
             * Schedule an immediate full piconet review at next update
             */
            _LMafh_AFH_control_byte |= (LMPafh_CTRL_PICONET_UPDATE);   
        }

        /*
         * Clear potential set_AFH ACK pending for this device index 
         */
        mLMafh_Clear_AFH_Change_Pending(p_link->device_index);

#if (PRH_BS_DEV_SUSPEND_SLAVE_CLASSIFICATIONS_SUPPORTED==1)
        /*
         * Clear potential channel_clasification_req ACK pending for this device index
         */
        mLMafh_Clear_LMP_Channel_Classification_Req_Pending(p_link->device_index);
#endif

        /*
         * Reset all known channels to good (am_addr identifies slave)
         */
        LSLCcls_Initialise_Slave_Channel_Classification(
            DL_Get_Am_Addr(DL_Get_Device_Ref(p_link->device_index)) );
    }
}

/****************************************************************************
 * LMafh_Set_AFH_Control_Byte
 *
 * Set the AFH control byte, allows AFH tests to be completed
 * See definition of _LMafh_AFH_control_byte for details
 ****************************************************************************/
void LMafh_Set_AFH_Control_Byte(u_int8 afh_channel_control)
{
    _LMafh_AFH_control_byte = afh_channel_control;
}

/****************************************************************************
 * _LMafh_Set_AFH_Instant
 *
 * Set the instant (slots i.e. Bits 27:1) 
 * Normally use min(6 * max T_poll,96)) or adjust for sniff, park or hold
 * with reference to the beginning of the next frame.
 *
 * afh_instant_ticks          Instant in ticks (must be converted to slots)
 ****************************************************************************/
static void _LMafh_Set_AFH_Instant(t_clock afh_instant_ticks)
{
    afh_instant_ticks += 4;             /* Align to next frame      */
    LMutils_Set_Uint32(_LMafh_AFH_p_instant, 
        (afh_instant_ticks>>1) & 0x07FFFFFE);
}

/****************************************************************************
 * _LMafh_Get_AFH_Instant
 ****************************************************************************/
t_clock _LMafh_Get_AFH_Instant(void)
{
    return (t_clock)((LMutils_Get_Uint32(_LMafh_AFH_p_instant)<<1));
}


/****************************************************************************
 * AFH Channel Classification
 ****************************************************************************/

/****************************************************************************
 * LMafh_LM_Set_AFH_Channel_Classification()
 *
 * Description
 *  Invoked by the higher layers on the LMP to set the AFH
 *  Channel Classification.
 *
 * From V1.2 Draft Spec:
 *  The Set_AFH_Channel_Classification command allows the Bluetooth host to
 *  specify a channel classification based on its “local information”. This 
 *  classification persists until overwritten with a subsequent HCI
 *  Set_AFH_Channel_Classification command or until the Host Controller is
 *  reset. Channels marked bad in this classification shall be marked as 
 *  unused in the LMP_set_AFH PDU sent from a master or marked as bad in
 *  the LMP_channel_classification PDU sent from a slave.
 *  This command shall be supported by a device that declares support for
 *  the AFH_classification feature.
 *  If this command is used, then updates shall be sent within
 *  HCI_maximum_adaptation_delay, defined as 10 seconds, of the host knowing
 *  that the channel classification has changed. The interval between 
 *  two successive commands sent by the host shall be no less than
 *  HCI_minimum_adaptation_interval, defined as 1 second.
 *
 *  AFH_Host_Channel_Classification: Size: 10 Bytes (79 Bits meaningful)
 ****************************************************************************/ 
t_error LMafh_LM_Set_AFH_Channel_Classification(
    u_int8 AFH_Host_Channel_Classification[/*10*/])
{
    u_int num_unknown_channels;
    t_error status;

    /*
     * Count the number of channels to verify that the minimum is satisfied.
     */
    num_unknown_channels = LSLCafh_Count_1_Bits(AFH_Host_Channel_Classification);

    if (num_unknown_channels < AFH_N_MIN_USED_CHANNELS)
    {
        status = INVALID_HCI_PARAMETERS;
    }
    else
    {
        /*
         * Store valid host classification and schedule update
         */
        hw_memcpy8(_LMafh_AFH_host_channel_classification,
            AFH_Host_Channel_Classification, AFH_BYTES_PER_CLASSIFICATION);
        _LMafh_AFH_host_channel_classification[AFH_BYTES_PER_CLASSIFICATION-1] &= 0x7F;
        _LMafh_AFH_control_byte |= LMPafh_CTRL_PICONET_UPDATE;

        /*
         * Schedule update to all Masters if active channel classification
         */
        mLMafh_Set_Request_Channel_Classification_All_Piconets();
        status = NO_ERROR;
    }
    return status;
}

/****************************************************************************
 * LMafh_LM_Write_AFH_Channel_Assessment_Mode()
 *
 * Description
 *  Invoked by the higher layers on the LMP to write the AFH
 *  Channel Assessment Mode 
 *
 * From V1.2 Draft Spec:
 *  The Write_AFH_Channel_Assessment_Mode command writes the value for
 *  the AFH_Channel_Assessment_Mode parameter. The
 *  AFH_Channel_Assessment_Mode parameter controls whether the Host Controller’s
 *  channel assessment scheme is enabled or disabled.
 *  Disabling channel assessment forces all channels to be unknown in the local
 *  classification, but does not affect the AFH_reporting_mode or support for the
 *  Set_AFH_Channel_Assessment command. A slave in the
 *  AFH_reporting_enabled state shall continue to send LMP channel classification
 *  messages for any changes to the channel classification caused by either
 *  this command (altering the AFH_Channel_Assessment_Mode) or HCI
 *  Set_AFH_Channel_Classification (providing a new channel classification from
 *  the Host).
 *  This command shall be supported by a device that declares support for any of
 *  the AFH_capable_master, AFH_classification_slave or
 *  AFH_classification_master features.
 *  If the AFH_Channel_Classification_Mode parameter is enabled and the Host
 *  Controller does not support a channel classification scheme, other than via the
 *  Set_AFH_Channel_Classification command, then a Status parameter of
 *  ‘Channel Classification Not Supported’ should be returned. See Table 6.1 on
 *  page 913 for list of Error Codes.
 *  If the Host Controller supports a channel assessment scheme then the default
 *  AFH_Channel_Classification_Mode is enabled, otherwise the default is disabled.
 *
 *  AFH_Channel_Classification_Mode: 0 Disabled, 1 Enabled
 ****************************************************************************/ 
t_error LMafh_LM_Write_AFH_Channel_Assessment_Mode(
    u_int8 AFH_channel_classification_mode)
{
    t_error status;

    if ((AFH_channel_classification_mode == 1) && 
            !SYSconfig_Is_Feature_Supported(
                PRH_BS_CFG_SYS_CHANNEL_ASSESSMENT_SCHEME_FEATURE))
    {
        /*
         * If requested enable and master doesn't support it then 
         */
        status = EC_CHANNEL_CLASSIFICATION_NOT_SUPPORTED;
    }
    else if (AFH_channel_classification_mode < 2)
    {
        if(_LMafh_AFH_channel_assessment_mode != AFH_channel_classification_mode)
        {
            /*
             * Channel assessment should mark all 79 channels as used when disabled
             * and/or when first enabled.
             */
            LSLCcls_Record_Device_Channel_Map(mLSLCafh_Get_79_Bit_Vector_Set_Ref());
            LSLCass_Initialise_Device_Channel_Assessment();
            _LMafh_AFH_channel_assessment_mode = AFH_channel_classification_mode;
            mLMafh_Set_Request_Channel_Classification_All_Piconets();
            _LMafh_AFH_control_byte |= LMPafh_CTRL_PICONET_UPDATE;
        }
        status = NO_ERROR;
    }
    else
    {
        status = INVALID_HCI_PARAMETERS;
    }

    return status;
}

/****************************************************************************
 * Function :- LMafh_LM_Read_AFH_Channel_Assessment_Mode
 *
 * Description
 *  Invoked by the higher layers on the LMP to read the AFH
 *  Channel Assessment Mode (0 Disable, 1 Enable)
 *
 ****************************************************************************/ 
u_int8 LMafh_LM_Read_AFH_Channel_Assessment_Mode(void)
{
    return _LMafh_AFH_channel_assessment_mode;
}





#if PRH_BS_CFG_SYS_LMP_AFH_CLASSIFICATION_SUPPORTED
/*
 * LMP Channel Classification Request PDU (note full message predefined)
 */
#if (PRH_BS_DEV_SUSPEND_SLAVE_CLASSIFICATIONS_SUPPORTED==0)
/*
 * If suspension of slave classifications not supported, then
 * LMP_channel_classification_req_PDU can remain as const.
 */
const
#endif
u_int8 _LMafh_Master_LMP_channel_classification_req_PDU[7] =
    {((LMP_CHANNEL_CLASSIFICATION_REQ>>8)<<1), 
    LMP_CHANNEL_CLASSIFICATION_REQ & 0xFF, 1, 
    LMPafh_AFH_MIN_INTERVAL_SLOTS&0xFF, LMPafh_AFH_MIN_INTERVAL_SLOTS>>8,
    LMPafh_AFH_MAX_INTERVAL_SLOTS&0xFF, LMPafh_AFH_MAX_INTERVAL_SLOTS>>8
};

/*
 * LMP Channel Classification complete PDU
 */
u_int8 _LMafh_Slave_LMP_channel_classification_PDU[12];

#define LMP_AFH_CLASSIFICATION_UPDATE_STATES    \
    (LMP_ACTIVE | LMP_SNIFF_MODE )

void LMafh_Report_Slave_Channel_Classification(t_lmp_link *p_link);
#define mLMafh_Is_AFH_Report_Classification_State(state) \
    ( state & (LMP_AFH_CLASSIFICATION_UPDATE_STATES))

/****************************************************************************
 * LMafh_Initialise_Classification
 *
 * Description
 *  Initialise the LMP classification
 *
 ****************************************************************************/ 
void LMafh_Initialise_Classification(void)
{
    /*
     * Initialise the LMP_channel_classification PDU
     */
    hw_memset8(_LMafh_Slave_LMP_channel_classification_PDU, 0x55, 
        mNum_Elements(_LMafh_Slave_LMP_channel_classification_PDU));
    _LMafh_Slave_LMP_channel_classification_PDU[0] = 
        ((LMP_CHANNEL_CLASSIFICATION>>8)<<1)+1 /*Slave TID*/;
    _LMafh_Slave_LMP_channel_classification_PDU[1] = 
        LMP_CHANNEL_CLASSIFICATION & 0xFF;

    mLMafh_Clear_Request_Channel_Classification_All_Piconets();
}


#if (PRH_BS_DEV_SUSPEND_SLAVE_CLASSIFICATIONS_SUPPORTED==1)
/****************************************************************************
 * LMPafh_Request_Classification_Reporting_All_Slaves
 *
 * Description
 *  Request active slaves to start/stop classification reporting by sending
 *  the predefined LMP_channel_classification_req
 *
 ****************************************************************************/ 
void LMPafh_Request_Classification_Reporting_All_Slaves(u_int8 mode)
{
    u_int i;

    extern t_link_entry link_container[PRH_MAX_ACL_LINKS];

    for (i = 0; i < PRH_MAX_ACL_LINKS; i++)
    {
        if (link_container[i].used) 
        {
            t_lmp_link *p_link = &(link_container[i].entry);

            if (mLMafh_Is_AFH_Report_Classification_State(p_link->state)
                && (MASTER == p_link->role))
            {
                LMPafh_Request_Classification_Reporting(p_link, mode);
            }
        }
    }
}
#endif

/****************************************************************************
 * LMPafh_Request_Classification_Reporting
 *
 * Description
 *  Request slave to start/stop classification reporting by sending the
 *  predefined LMP_channel_classification_req
 *
 ****************************************************************************/ 
void LMPafh_Request_Classification_Reporting(t_lmp_link *p_link, u_int8 mode)
{
    if (SYSconfig_Is_Feature_Supported(
            PRH_BS_CFG_SYS_LMP_AFH_CLASSIFICATION_MASTER_FEATURE) &&
        mLMPfeat_Is_Remote_Feature_Supported( p_link->remote_features,  
            PRH_BS_CFG_SYS_LMP_AFH_CLASSIFICATION_SLAVE_FEATURE))
    {
#if (PRH_BS_DEV_SUSPEND_SLAVE_CLASSIFICATIONS_SUPPORTED==1)
        _LMafh_Master_LMP_channel_classification_req_PDU[2] = mode;

        if (LM_Encode_LMP_PDU_Fully_Encoded(p_link->device_index, 
                LMP_CHANNEL_CLASSIFICATION_REQ,
                _LMafh_Master_LMP_channel_classification_req_PDU)==NO_ERROR)
        {
            if (p_link->state & LMP_ACTIVE) 
            {
                mLMafh_Set_LMP_Channel_Classification_Req_Pending(p_link->device_index);
            }
        }
#else
        if(mode == 1/*enable*/)
        {
            LM_Encode_LMP_PDU_Fully_Encoded(p_link->device_index, 
                LMP_CHANNEL_CLASSIFICATION_REQ,
                _LMafh_Master_LMP_channel_classification_req_PDU);
        }
#endif
    }
}

#if (PRH_BS_DEV_SUSPEND_SLAVE_CLASSIFICATIONS_SUPPORTED==1)
/****************************************************************************
 * LMafh_LMP_Channel_Classification_Req_Ack
 *
 * Process the baseband ACK to the LMP_Channel_Classification_Req.
 ****************************************************************************/
void LMafh_LMP_Channel_Classification_Req_Ack(t_deviceIndex device_index)
{
    mLMafh_Clear_LMP_Channel_Classification_Req_Pending(device_index);
}

/****************************************************************************
 * LMafh_Is_LMP_Channel_Classification_Req_Pending
 *
 * Returns TRUE if specified classification reporting request pending.
 *
 ****************************************************************************/
boolean LMafh_Is_LMP_Channel_Classification_Req_Pending(u_int8 mode)
{
    return ((_LMafh_Master_LMP_channel_classification_req_PDU[2]==mode)
        && mLMafh_Is_LMP_Channel_Classification_Req_Pending());
}
#endif

/****************************************************************************
 * LMafh_Is_Classification_Reporting
 *
 * Returns TRUE if classifications reporting is enabled from this slave.
 *
 ****************************************************************************/ 
boolean LMafh_Is_Classification_Reporting(t_lmp_link *p_link)
{
    t_piconet_index piconet_index =
        DL_Get_Piconet_Index(DL_Get_Device_Ref(p_link->device_index));
    u_int8 *p_timer_index = _LMafh_AFH_update_timer_index+piconet_index;
    return (piconet_index && *p_timer_index);
}

/****************************************************************************
 * LMafh_LMP_Channel_Classification_Req
 *
 * Description
 *  Handles the LMP_channel_classification_req
 *
 * If the parameters are invalid then just ensure no classifications sent. 
 ****************************************************************************/ 
t_error LMafh_LMP_Channel_Classification_Req(t_lmp_link *p_link, t_p_pdu p_payload)
{
    t_piconet_index piconet_index =
        DL_Get_Piconet_Index(DL_Get_Device_Ref(p_link->device_index));
    t_slots afh_report_time_interval_slots;

    afh_report_time_interval_slots = (p_payload[2]<<8) + p_payload[1];
    if (p_payload[0] == LMPafh_AFH_REPORTING_ENABLED && 
        afh_report_time_interval_slots >= 0x0640 )
    {
        /*
         * Update interval rate is the afh_min_interval selected
         * This also fulfils the afh_max_interval requirements
         */

        /*
         * Do not schedule an update, rather reset monitoring
         * on this device to all ok, the default for the
         * device requesting.  Note the limitation:
         *   If this is the second piconet with role as SLAVE, 
         *   then all device channels are reset to good.  
         */
        if(g_LM_config_info.num_acl_links == 1) /* #2102 */
        {
            LSLCcls_Record_Device_Channel_Map(mLSLCafh_Get_79_Bit_Vector_Set_Ref());
        }

        _LMafh_AFH_update_timeout[piconet_index] = afh_report_time_interval_slots;
        LMafh_Start_Classification_Reporting(p_link);
    }
    else
    {
        /*
         * Stop the channel classification by resetting timer and
         * marking timer timeout as not active.
         */
        LMafh_Stop_Classification_Reporting(p_link);
        _LMafh_AFH_update_timeout[piconet_index] = 0;
    }

    mLMafh_Clear_LMP_Channel_Classification_Pending(piconet_index);
    return NO_ERROR;
}

/************************************************************************
 * LMafh_Stop_Classification_Reporting
 *
 * Stop the AFH classification reporting by this slave device
 *
 * p_link           Reference to the associated lmp_link for this device
 *
 * If I'm a slave piconet and timer exists then
 *    Clear the timer
 * endif
 *
 ************************************************************************/
void LMafh_Stop_Classification_Reporting(t_lmp_link *p_link)
{
    t_piconet_index piconet_index =
        DL_Get_Piconet_Index(DL_Get_Device_Ref(p_link->device_index));
    u_int8 *p_timer_index = _LMafh_AFH_update_timer_index+piconet_index;

    /*
     * If Channel Classification Active Then Free Timer
     * i.e. If I'm a slave role based piconet and timer exists then turn off
     */
    if (piconet_index != MASTER_PICONET_INDEX && *p_timer_index)
    {
        LMtmr_Clear_Timer(*p_timer_index);
        *p_timer_index = 0;
    }
}

/************************************************************************
 * LMafh_Start_Classification_Reporting
 *
 * Start the AFH classification reporting by this slave device
 *
 * p_link           Reference to the associated lmp_link for this device
 *
 * If slave associated timeout is valid then
 *    if timer is not already active then 
 *          Allocate a timer for this piconet 
 *    else
 *          Reset current timer to required interval
 *    endif
 * endif
 *
 * Two scenarios for invocation:
 * 1. As a result of a valid LMP_channel_classification_req
 * 2. Following a failed master slave switch
 ************************************************************************/
void LMafh_Start_Classification_Reporting(t_lmp_link *p_link)
{
    t_piconet_index piconet_index =
        DL_Get_Piconet_Index(DL_Get_Device_Ref(p_link->device_index));
    t_slots afh_update_interval = _LMafh_AFH_update_timeout[piconet_index]; 
    u_int8 *p_timer_index = _LMafh_AFH_update_timer_index+piconet_index;

    if (piconet_index != MASTER_PICONET_INDEX && afh_update_interval != 0)
    {
        if (*p_timer_index == 0)
        {
            *p_timer_index = LMtmr_Set_Timer(afh_update_interval,
                                LMafh_Report_Slave_Channel_Classification, 
                                p_link, 0 /*Persistent Timer*/);            
        }
        else
        {
            LMtmr_Reset_Timer(*p_timer_index, afh_update_interval);
        }
    }
}

/****************************************************************************
 * Function :- LMafh_LMP_Channel_Classification
 *
 * Description
 *  Processes any received slave classifications in 
 * LMP_channel_classification.   Store for now, and then schedule a
 * piconet update at the next afh update period.
 *
 ****************************************************************************/ 
t_error LMafh_LMP_Channel_Classification(t_lmp_link *p_link, t_p_pdu p_payload)
{
    LSLCcls_Record_Slave_Channel_Classification(
        DL_Get_Am_Addr(DL_Get_Device_Ref(p_link->device_index)), p_payload);

    /*
     * Request timed update at later date for the piconet
     */
    _LMafh_AFH_control_byte |= LMPafh_CTRL_PICONET_UPDATE;

    return NO_ERROR;
}

/****************************************************************************
 * Function :- LMafh_Report_Slave_Channel_Classification
 *
 * Description
 * Periodically scheduled to update any classification changes
 *
 ****************************************************************************/ 
void LMafh_Report_Slave_Channel_Classification(t_lmp_link *p_link)
{
    t_piconet_index piconet_index =
        DL_Get_Piconet_Index(DL_Get_Device_Ref(p_link->device_index));
    u_int8 *p_timer_index = _LMafh_AFH_update_timer_index+piconet_index;

#if (PRH_BS_CFG_SYS_TEST_MODE_SUPPORTED==1)
    if(BTtst_Get_DUT_Mode() <= DUT_ACTIVE)
#endif
    {
        /*
         * Report if in valid state, an update has occurred and no update pending
         */
        if (mLMafh_Is_AFH_Report_Classification_State(p_link->state) &&
            mLMafh_Is_Request_Channel_Classification(piconet_index) &&
            !mLMafh_Is_LMP_Channel_Classification_Pending(piconet_index) )
        {
            /*
             * Build the channel classification vector using all known sources
             * directly into the parameter section of LMP_channel_classification PDU
             */
            boolean change_occurred = LSLCcls_Build_Channel_Classification(
                _LMafh_Slave_LMP_channel_classification_PDU+2,
                _LMafh_AFH_host_channel_classification);

            if(change_occurred || mLMafh_Is_Request_Channel_Classification(0))
            {
                /*
                 * Note that LMP message sent. Cleared on BB ack.
                 */
                mLMafh_Set_LMP_Channel_Classification_Pending(piconet_index);
 
                /*
                 * Prepare LMP_channel_classification message
                 */
                LM_Encode_LMP_PDU_Fully_Encoded(p_link->device_index, 
                    LMP_CHANNEL_CLASSIFICATION,
                    _LMafh_Slave_LMP_channel_classification_PDU); 

            }
            else
            {
                /*
                 * Schedule the next update 
                 */
                LMtmr_Reset_Timer(*p_timer_index, 
                    _LMafh_AFH_update_timeout[piconet_index]);
            }

            /*
             * Cancel request update
             */
            mLMafh_Clear_Request_Channel_Classification(piconet_index);
        }
        else
        {
            /*
             * Schedule the next update 
             */
            LMtmr_Reset_Timer(*p_timer_index, _LMafh_AFH_update_timeout[piconet_index]);
        }
    }
}
#endif /*PRH_BS_CFG_SYS_LMP_AFH_CLASSIFICATION_SUPPORTED*/


#else
/* 
 * Completely empty C files are illegal in ANSI 
 */
void LMafh_Dummy__(void);
#endif /*(PRH_BS_CFG_SYS_LMP_AFH_CAPABLE_SUPPORTED==1)*/
