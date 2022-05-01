/***********************************************************************
 *
 * MODULE NAME:    lmp_acl_connection.c
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    
 * MAINTAINER:     Gary Fleming
 * CREATION DATE:  18 February 2000x
 *
 * SOURCE CONTROL: $Id: lmp_acl_connection.c,v 1.138 2013/03/11 16:39:32 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * DESCRIPTION
 * This module is responsible for initialisation of the lmp ACL object.
 *    
 ***********************************************************************/

#include "sys_config.h"

#include "lmp_types.h"
#include "lmp_sec_types.h"
#include "lmp_const.h"
#include "lmp_config.h"
#include "lmp_acl_connection.h"
#include "lmp_utils.h"
#include "lmp_features.h"

/**************************************************************************
 *
 * Function :- LMacl_Initialise
 *
 * Input :-  t_lmp_link* p_link 
 *           A pointer to the structure describing the LMP link.
 * 
 * Description
 * This function is responsible for initialisation of the LMP ACL link structure
 **************************************************************************/

void LMacl_Initialise(t_lmp_link* p_link)
{
    int i = 0;

    p_link->state             = LMP_IDLE;
    /* p_link->device_index is initialised by LMaclctr_Initialise() */
    p_link->role              = MASTER;
    p_link->rate              = AUTO;
    p_link->peer_rate         = MEDIUM;
    p_link->preferred_rate    = MEDIUM;

    /* 
     * Set the BD_ADDR to zero 
     */
    BDADDR_Set_LAP(&p_link->bd_addr,0x0);
    BDADDR_Set_UAP(&p_link->bd_addr,0x0);
    BDADDR_Set_NAP(&p_link->bd_addr,0x0);

    /* 
     * Set the class of device to unspecified 
     */
    p_link->device_class = 0x001f00;

    p_link->allowed_pkt_types = DM1_BIT_MASK;
    p_link->proposed_packet_types = HCI_DM1;
    p_link->proposed_poll_interval = DEFAULT_T_POLL;

    /*
     * Assume power control feature only
     */
    p_link->remote_features[0] = 0;
    p_link->remote_features[1] = 0;
    p_link->remote_features[2] = POWER_CONTROL_MASK;
	p_link->remote_features[3] = 0;
#if (BUILD_TYPE==UNIT_TEST_BUILD)
	p_link->remote_features[4] = AFH_CAPABLE_SLAVE_MASK|AFH_CLASSIFICATION_SLAVE_MASK;
#else
	p_link->remote_features[4] = 0;
#endif
	p_link->remote_features[5] = 0;
	p_link->remote_features[6] = 0;
	p_link->remote_features[7] = 0;
    p_link->context           = NO_CONNECTION;
    p_link->operation_pending = NO_OPERATION;
    p_link->setup_complete_bit_mask = LMP_No_Setup_Complete;
    p_link->features_complete_bit_mask = LMP_No_Features_Complete;
    p_link->host_connection_request_accepted = FALSE;
	p_link->host_connection_request_sent = FALSE;

    p_link->index = 0;
    p_link->Nbc = DEFAULT_NBC;
    /* p_link->handle is initialised by LMaclctr_Initialise() */

    /* 
     * Transaction Ids.
     */
    p_link->tid_role_last_rcvd_pkt = 0; /* ROLE_TID(0):tran initiated locally OR PEER_ROLE_TID(1): tran initiated by peer  */  
    p_link->current_proc_tid_role  = 0;  /* role of currently initiated procedure, determines whether proc. initiated locally or by peer */

    /*
     * Security Related Info 
     */
    p_link->sec_current_proc_tid_role = 0;
    p_link->auth_super_state       = SUPER_STATE_IDLE;
    p_link->auth_sub_state         = SUB_STATE_IDLE;
    p_link->encr_super_state       = SUPER_STATE_IDLE;
    p_link->encr_sub_state         = SUB_STATE_IDLE;
    for(i = 0; i < 16; i++)
    {
        p_link->link_key[i] = 0;
        p_link->pin_code[i] = 0;
        p_link->rand[i] = 0;
        p_link->local_combkey_part[i] = 0;
        p_link->encry_key[i]  = 0;
#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
		p_link->peer_commitment[i]=0;
		p_link->peer_random[i] = 0;
		p_link->DHkeyCheck[i] = 0;
		p_link->Rpeer[i] = 0;
		p_link->peer_key_check[i] = 0;
#endif
    }
    p_link->link_key_exists = FALSE;
    p_link->key_type = COMBINATION_KEY;
    LMacl_Set_Key_Persistance(p_link, SEMI_PERMANENT);
    p_link->pin_code_length = 0x0;
    p_link->encrypt_key_size = 0x0;

    for(i = 0; i < SRES_SIZE; i++)
        p_link->sres[i] = 0;
    for(i = 0; i < ACO_SIZE; i++)
        p_link->aco[i] = 0;
    p_link->encrypt_mode = ENCRYPT_NONE;
    p_link->encrypt_enable = OFF;
    p_link->bcast_enc_key_size_mask = 0xFFFF; /* this will only change if the host controller
                                                 is a BT1.2 or greater device */

    /*
     * Link policy related settings
     */
#if ( PRH_BS_CFG_SYS_LMP_HOLD_SUPPORTED == 1)
    p_link->pol_info.hold_info.hold_instant = 0;
    p_link->pol_info.hold_info.max_hold_time = 0xFFFF;
    p_link->pol_info.hold_info.min_hold_time = PRH_DEFAULT_MIN_HOLD_INTERVAL;
    p_link->pol_info.hold_info.max_previous_hold = 0;  
    p_link->pol_info.hold_info.hold_time = 0;
    p_link->pol_info.hold_info.num_hold_negotiations = 0;
    p_link->pol_info.hold_info.hold_timeout_index = 0;
#endif 

#if ( PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED == 1)
    p_link->switch_state = SWITCH_IDLE;
#endif

#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUPPORTED == 1)
    p_link->pol_info.sniff_info.end_current_sniff_window = 0;
    p_link->pol_info.sniff_info.next_sniff_window = 0;
    p_link->pol_info.sniff_info.D_sniff = 0;
    p_link->pol_info.sniff_info.T_sniff = 0;
    p_link->pol_info.sniff_info.T_sniff_min = 0;
    p_link->pol_info.sniff_info.T_sniff_max = 0;
    p_link->pol_info.sniff_info.N_sniff = 0;
    p_link->pol_info.sniff_info.N_sniff_rem = 0;
    p_link->pol_info.sniff_info.sniff_timeout = 0;
#if (PRH_BS_CFG_SYS_LMP_SNIFF_SUBRATING_SUPPORTED==1)
    p_link->pol_info.sniff_info.sniff_subrating_req_pending = 0;
    p_link->pol_info.sniff_info.max_latency = 0;
    p_link->pol_info.sniff_info.min_local_timeout = 0;
    p_link->pol_info.sniff_info.min_remote_timeout = 0;
    p_link->pol_info.sniff_info.next_subrate_window = 0;
    p_link->pol_info.sniff_info.subrate_state = 0;
    p_link->pol_info.sniff_info.subrate_timeout_index = 0;
    p_link->pol_info.sniff_info.max_remote_sniff_subrate = 1;
    p_link->pol_info.sniff_info.local_sniff_subrate = 0;
    p_link->pol_info.sniff_info.min_remote_sniff_timeout = 0;
    p_link->pol_info.sniff_info.new_local_sniff_timeout = 0;
    p_link->pol_info.sniff_info.local_sniff_timeout = 0;
    p_link->pol_info.sniff_info.sniff_subrating_instant = 0;
#endif
    p_link->pol_info.sniff_info.sniff_state = 0;
    p_link->pol_info.sniff_info.num_sniff_negotiations = 0;
    p_link->pol_info.sniff_info.timing_ctrl = 0;
#endif

#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED == 1)
    p_link->pol_info.park_info.delta_bcast = 0;
    p_link->pol_info.park_info.pm_addr = 0;
    p_link->pol_info.park_info.ar_addr = 0;
    p_link->pol_info.park_info.am_addr = 0;
    p_link->pol_info.park_info.park_state = 0;
    p_link->pol_info.park_info.device_index = 0;
    p_link->pol_info.park_info.unpark_device_index = 0;
    p_link->pol_info.park_info.park_device_index = 0;
    p_link->pol_info.park_info.unpark_pending = 0;
    p_link->pol_info.park_info.timing_ctrl = 0;
    p_link->pol_info.park_info.unpark_pdu_rx = 0;
    p_link->pol_info.park_info.park_timeout_index = 0;
    p_link->pol_info.park_info.unpark_timeout_index = 0;
    p_link->pol_info.park_info.num_unparks_attempts = 0;
#endif

    /*
     * Power Information 
     */
    p_link->peer_power_status = INTERMEDIATE_POWER;
    p_link->max_power_level = TX_POWER_LEVEL_Pmax;
    p_link->peer_power_req_tx_pending = FALSE;
    p_link->peer_power_counter = 0;
 
    /*
     * QoS Information
     */
    p_link->service_type = PRH_BEST_EFFORT;

#if (PRH_BS_CFG_SYS_QOS_SUPPORTED==1)
    p_link->token_rate = 0;
    p_link->peak_bandwidth = 0;
    p_link->latency = 0;
    p_link->delay_variation = 0;
    p_link->proposed_service_type = 0;
    p_link->proposed_token_rate = 0;
    p_link->proposed_peak_bandwidth = 0;
    p_link->proposed_latency = 0;
    p_link->proposed_delay_variation = 0;
#endif    

#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
    p_link->automatic_park_enabled =0;
#endif
#if (PRH_BS_DEV_SLAVE_FORCE_STOP_AUTOPARK_SUPPORTED==1)
    p_link->force_stop_autopark = 0;
#endif
    /*
     * Timers and Timer Indexes 
     */
    p_link->channel_quality_timeout = 0x100 /*DEFAULT_CHANNEL_QUALITY_TIMEOUT*/;
    p_link->flush_timeout = DEFAULT_FLUSH_TIMEOUT;
    p_link->link_supervision_timeout = DEFAULT_SUPERVISION_TIMEOUT;
    p_link->supervision_timeout = DEFAULT_SUPERVISION_TIMEOUT;
    p_link->flush_timeout_index = 0;
    p_link->flush_execut_timeout_index = 0x0;
    p_link->supervision_timeout_index = 0x0;
    p_link->recent_contact_timeout_index = 0x0;
    p_link->channel_quality_timeout_index = 0x0;

    /*
     * MSS fields

     */
#if (PRH_BS_CFG_SYS_LMP_MSS_SUPPORTED==1)
    p_link->allow_switch = 0x0;
#endif

    p_link->flush_status = PRH_NO_FLUSH_PENDING;
    p_link->flush_type = PRH_NO_FLUSH;
    p_link->queue_flush_pending = 0;
    p_link->sched_queue_flush_pending = 0;
#if(PRH_BS_CFG_SYS_LMP_NONFLUSHABLE_PBF_SUPPORTED==1)
    p_link->flush_packet_type = AUTO_FLUSHABLE;
	p_link->enhanced_queue_flush_pending = 0;
#endif
    p_link->sched_queue_flush_pending = 0;

    p_link->failed_contact_counter = 0x00;

    /*
     * Link policy settings
     */
    p_link->packet_types      = HCI_DM1; 
    p_link->poll_interval = DEFAULT_T_POLL;
    p_link->poll_position = DEFAULT_T_POLL;
    p_link->l2cap_ack_pending = 0x0;
    p_link->previous_modes = LINK_POLICY_NONE_ENABLED; /* No link policy modes entered yet */
    p_link->packet_type_changed_event = 0;
    p_link->max_slots_out     = 1;
    p_link->max_slots_out_pending = 0; /* No max_slots_out pending */
    p_link->max_slots_in      = 1;
    p_link->prev_slots_tx     = 2; /* Assume 2 Slots used in previous Tx Frame */
    p_link->l2cap_tx_enable = 0x0;
    p_link->slot_offset     = 0x0;
	p_link->connection_request_send_via_hci = 0x0;
    p_link->gen_switch_timeout_index = 0;
    p_link->gen_detach_timeout_index = 0;
    p_link->gen_security_timeout_index = 0;

#if(PRH_BS_CFG_SYS_HCI_V12_FUNCTIONALITY_SUPPORTED==1)
    p_link->link_policy_mode = g_LM_config_info.default_link_policy_mode;
#else
    p_link->link_policy_mode = LINK_POLICY_NONE_ENABLED;
#endif
    p_link->msg_timer = 0x0;

#if (PRH_BS_CFG_SYS_LMP_PARK_SUPPORTED==1)
    p_link->park_device_index = 0x0;
    p_link->num_park_negotiations = 0x00;
#endif
    p_link->default_pkt_type = g_LM_config_info.default_pkt_type;
    p_link->detach_reason = REMOTE_USER_TERMINATED_CONNECTION; /* normal */
    p_link->disconnect_req_reason = NO_ERROR;
    p_link->l2cap_tx_enable = 0;
    p_link->link_quality = 0x7F;  /* Initialise to between best (0xFF) and worst (0x00) link quality */
    p_link->current_l2cap_pkt_length = 0;
    p_link->l2cap_pkt_len_pending = 0;
    p_link->sec_timer = 0;

#if (PRH_BS_CFG_SYS_FLOW_SPECIFICATION_SUPPORTED==1)
    p_link->direction = 0;
    p_link->transmission_interval = DEFAULT_TRANS_INTERVAL;
    p_link->transmission_position = DEFAULT_TRANS_INTERVAL;
    p_link->flow_spec_pending = 0;
    p_link->in_qos_active = 0;
    p_link->out_qos_active = 0;
#endif

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
    p_link->ptt_req = PTT_REQ_IDLE;
#endif
#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
	p_link->peer_user_authenticated = 0;
	p_link->local_user_authenticated = 0;

	p_link->ssp_debug_mode = 0;

	for (i=0;i<24;i++)
	{
		p_link->peer_public_key_x[i] = 0;
		p_link->peer_public_key_y[i] = 0;
		p_link->DHkey[i] = 0;

	}

	p_link->Incoming_Encapsulated_P192_len_pending=0;
	p_link->ssp_initiator = 0;
	p_link->DHkeyCheckComplete = 0;
	p_link->DHkeyCalculationComplete = 0;
#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)
	p_link->oob_data_present = 0;
#endif
	p_link->auth_requirements = 0;
	p_link->io_cap = 0;
	p_link->auth_type = 0x88;
	p_link->ssp_tid = 0;
	p_link->peer_user_authenticated = 0;
	p_link->local_user_authenticated = 0;
#if (PRH_BS_CFG_SYS_SSP_PASSKEY_SUPPORTED==1)
	p_link->passKey = 0;
	p_link->passKeyCount = 0;
#endif
#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)
	p_link->local_oob_verified = 0;
#endif
	p_link->rxed_simple_pairing_number = 0;
	p_link->ssp_host_support = 0;
	p_link->ssp_hci_timer_index = 0;
	p_link->peer_user_dh_key_check_rxed = 0;

#endif
	p_link->call_lc_wakeup = TRUE;
}


/***********************************************************
 * Function :- LMacl_Write_Combkey_Part
 *
 * Description :-
 * Writes a partial combination key into the ACL link structure
 ************************************************************/
void LMacl_Write_CombKey_Part(t_lmp_link *p_link, u_int8 *local_comb_key_part)
{
    LMutils_Array_Copy(LINK_KEY_SIZE, p_link->local_combkey_part, local_comb_key_part);
}


/***********************************************************
 * Function :- LMacl_Write_Bd_Addr
 *
 * Description :-
 * Writes a Bluetooth device address into the ACL link structure
 ************************************************************/
void LMacl_Write_Bd_Addr(t_lmp_link* current_link, t_bd_addr* p_bd_addr)
{
    BDADDR_Copy(&current_link->bd_addr, p_bd_addr);
}

/***********************************************************
 * Function :- LMacl_Set_Key_Persistance
 *
 * Description :-
 ************************************************************/
void LMacl_Set_Key_Persistance(t_lmp_link* p_link, t_link_key_life key_persistance)
{
    p_link->link_key_persistance = (t_link_key_life)
        ( ((p_link->link_key_persistance & 0x0F) << 4) |
         (key_persistance & 0x0F) );
}

/***********************************************************
 * Function :- LMacl_Get_Current_Key_Persistance
 *
 * Description :-
 ************************************************************/
t_link_key_life LMacl_Get_Current_Key_Persistance(t_lmp_link* p_link)
{
    return (t_link_key_life)(p_link->link_key_persistance & 0x0F);
}

/***********************************************************
 * Function :- LMacl_Get_Previous_Key_Persistance
 *
 * Description :-
 ************************************************************/
t_link_key_life LMacl_Get_Previous_Key_Persistance(t_lmp_link* p_link)
{
    return (t_link_key_life) ((p_link->link_key_persistance & 0xF0) >> 4);
}

