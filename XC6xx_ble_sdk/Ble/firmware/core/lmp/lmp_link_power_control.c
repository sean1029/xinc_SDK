/***********************************************************************
 *
 * MODULE NAME:    lmp_link_power_control.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    Scans links requesting peer to update Tx power if required
 * MAINTAINER:     Ivan Griffin
 * CREATION DATE:  30 August 2001
 *
 * SOURCE CONTROL: $Id: lmp_link_power_control.c,v 1.28 2011/10/28 15:41:44 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2001-2004 Ceva Inc.
 *     All rights reserved.
 *
 *    
 ***********************************************************************/
#include "sys_config.h"

#if (PRH_BS_CFG_SYS_LMP_POWER_CONTROL_SUPPORTED==1)

#include "lmp_types.h"
#include "lmp_const.h"
#include "lmp_config.h"
#include "lmp_features.h"
#include "lmp_acl_connection.h"
#include "lmp_acl_container.h"
#include "lmp_link_control_protocol.h"
#include "lmp_link_power_control.h"
#include "lmp_timer.h"
#include "lc_interface.h"

/*
 * Included in power control to tick over the
 * random number generator. Power control
 * is the most unpredictable item on a link.
 * Obscure the pseudo-rand algorithm
 * by dumping an uncertain amount of
 * output.
 */
#include "sys_rand_num_gen.h"

#if (PRH_BS_CFG_SYS_TEST_MODE_SUPPORTED==1)

#include "bt_test.h"
#include "uslc_testmode.h"

extern t_tm_control volatile testmode_ctrl;

#endif

/***********************************************************************
 * FUNCTION: LMlpctrl_Initialise  (place holder)
 * PURPOSE:  No direct associated variables.  
 *           Hence, no initialisation required.
 ***********************************************************************/

/***********************************************************************
 * FUNCTION: LMlpctrl_Scan_All_Links_Request_Peer_Power_If_Reqd
 * PURPOSE:  Scan each active link every 64 frames (80ms)
 *           and determine if power levels should be adjusted.
 *
 ***********************************************************************/
void LMlpctrl_Scan_All_Links_Request_Peer_Power_If_Reqd(t_lmp_link* p_link)
{
    u_int8 i;
    extern t_link_entry link_container[PRH_MAX_ACL_LINKS];
    
    LMtmr_Reset_Timer(g_LM_config_info.peer_power_timer_index,LM_CONFIG_PEER_POWER_MONITOR_PERIOD);

#if(PRH_BS_CFG_SYS_TEST_MODE_SUPPORTED==1)


	/* do not send power control messages if test mode with fixed tx output power */
    if( BTtst_Get_DUT_Mode() > DUT_ENABLED && testmode_ctrl.powerControlMode == 0 )
        return;


#endif

    for (i = 0; i < PRH_MAX_ACL_LINKS; i++)
    {
        if (link_container[i].used) 
        {
            t_lmp_link *p_link = &(link_container[i].entry);

            /* Errata 2034 to Bluetooth Spec allows Power Control signalling before LMP connection is active */
            if (!(~(W4_LMP_ACTIVE|LMP_ACTIVE|LMP_SNIFF_MODE) & p_link->state) &&
                (FALSE == p_link->peer_power_req_tx_pending) &&
                (mFeat_Check_Power_Control(p_link->remote_features)))
            {
                s_int8 rssi_average = LC_Get_RSSI(p_link->device_index);
#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==1)
                s_int8 rssi_average1 = LC_Get_1Mb_RSSI(p_link->device_index);
                s_int8 rssi_average2 = LC_Get_2Mb_RSSI(p_link->device_index);
                s_int8 rssi_average3 = LC_Get_3Mb_RSSI(p_link->device_index);
#endif

                /* try to keep peer device within golden receiver window: -56dBm to -30dBm */
#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==0)
                if (rssi_average < GOLDEN_RECEIVER_RSSI_MIN) 
#else
                if ( (rssi_average1 < GOLDEN_RECEIVER_RSSI_MIN) 
			      || (rssi_average2 < GOLDEN_RECEIVER_RSSI_MIN) 
				  || (rssi_average3 < GOLDEN_RECEIVER_RSSI_MIN) )
#endif
                {
#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==1)
					if ( mFeat_Check_Enhanced_Power_Control(p_link->remote_features) &&
						((GOLDEN_RECEIVER_RSSI_MIN - rssi_average) >= EPC_MAX_TX_POWER_DIFFERENCE))
					{
						LMlc_LM_Inc_Peer_Power(p_link, MAX_TRANSMIT_POWER_LEVEL);
					}
					else
#endif
                    {
					    LMlc_LM_Inc_Peer_Power(p_link, 0);
                    }

                    p_link->peer_power_req_tx_pending = TRUE;

                    /*
                     * Dump 3 random numbers.
                     */
                    (void)SYSrand_Get_Rand();
                    (void)SYSrand_Get_Rand();
                    (void)SYSrand_Get_Rand();
                }
#if (PRH_BS_CFG_SYS_ENHANCED_POWER_CONTROL_SUPPORTED==0)
                else if (rssi_average > GOLDEN_RECEIVER_RSSI_MAX)
#else
                else if ( (rssi_average1 > GOLDEN_RECEIVER_RSSI_MAX) 
			           || (rssi_average2 > GOLDEN_RECEIVER_RSSI_MAX) 
				       || (rssi_average3 > GOLDEN_RECEIVER_RSSI_MAX) )
#endif
                {
                    LMlc_LM_Dec_Peer_Power(p_link, 0); /* parameter is for future use as of v1.1 final, and must be 0x00 */
                    p_link->peer_power_req_tx_pending = TRUE;

                    /*
                     * Dump two random numbers.
                     */
                    (void)SYSrand_Get_Rand();
                    (void)SYSrand_Get_Rand();
                }
                else
                {
                    /*
                     * Dump one random number.
                     */
                    (void)SYSrand_Get_Rand();
                }

            }

            if ((p_link->peer_power_status == MAX_POWER) || 
               (p_link->peer_power_status == MIN_POWER))
            {
                p_link->peer_power_req_tx_pending = FALSE;
            }
        }
    }
}

/***********************************************************************
 * FUNCTION: LMlpctrl_Decr_Power_Req_Ack
 * PURPOSE:  Once a LMP_DECR_POWER has been
 *           transmitted, need to ensure that no further power control
 *           messages for this device are enqueued until the first is ack'd
 *           by the baseband to prevent overflowing the queues...
 *
 *           Also need to stop sending LMP_Decr_Power_Req's if
 *           the peer device does not obey them
 ***********************************************************************/
void LMlpctrl_Decr_Power_Req_Ack(t_deviceIndex device_index)
{
    t_lmp_link *p_link;

    p_link = LMaclctr_Find_Device_Index(device_index);
    LMlpctrl_Power_Req_Ack(p_link);

    if(p_link)
    {

        p_link->peer_power_counter--;
        if(p_link->peer_power_counter < (s_int8)(0-MAX_PEER_POWER_LEVEL_UNITS) )
        {
            LMlc_LMP_Min_Power(p_link, 0);
        }
    }
}

/***********************************************************************
 * FUNCTION: LMlpctrl_Incr_Power_Req_Ack
 * PURPOSE:  Once a LMP_INCR_POWER has been
 *           transmitted, need to ensure that no further power control
 *           messages for this device are enqueued until the first is ack'd
 *           by the baseband to prevent overflowing the queues...
 *
 *           Also need to stop sending LMP_Incr_Power_Req's if
 *           the peer device does not obey them
 ***********************************************************************/
void LMlpctrl_Incr_Power_Req_Ack(t_deviceIndex device_index)
{
    t_lmp_link *p_link;

    p_link = LMaclctr_Find_Device_Index(device_index);
    LMlpctrl_Power_Req_Ack(p_link);

    if(p_link)
    {
        p_link->peer_power_counter++;
        if(p_link->peer_power_counter > (s_int8)(MAX_PEER_POWER_LEVEL_UNITS) )
        {
            LMlc_LMP_Max_Power(p_link, 0);
        }
    }
}

/***********************************************************************
 * FUNCTION: LMlpctrl_Power_Req_Ack
 * PURPOSE:  Once an LMP_INCR_POWER or LMP_DECR_POWER has been
 *           transmitted, need to ensure that no further power control
 *           messages for this device are enqueued until the first is ack'd
 *           by the baseband to prevent overflowing the queues...
 ***********************************************************************/
void LMlpctrl_Power_Req_Ack(t_lmp_link* p_link)
{
    if (0 == p_link)
    {
        return;
    }
     
    p_link->peer_power_req_tx_pending = FALSE;
    DL_Reset_RSSI_History(DL_Get_Device_Ref(p_link->device_index));
}

#else
/*
 * Prevent compiler warnings.
 */
void LMlpctrl_Dummy__(void) {}
#endif
