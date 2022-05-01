/*****************************************************************************
 *
 * MODULE NAME:    lmp_sec_core.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    LMP security core component
 * MAINTAINER:     Daire McNamara
 * CREATION DATE:  7 February 2001
 *
 * SOURCE CONTROL: $Id: lmp_sec_core.c,v 1.58 2013/11/27 13:20:38 garyf Exp $
 *
 * LICENSE:
 *   This source code is copyright (c) 2001-2004 Ceva Inc.
 *   All rights reserved.
 *
 ****************************************************************************/

#include "sys_config.h"

#include "lmp_sec_core.h"
#include "lmp_sec_peer.h"
#include "lmp_sec_upper.h"
#include "lmp_link_key_db.h"
#include "lmp_const.h"
#include "lmp_encode_pdu.h"
#include "lmp_acl_container.h"
#include "lmp_acl_connection.h"
#include "lmp_sec_engine.h"
#include "lmp_ch.h"
#include "lmp_config.h"
#include "lmp_link_policy.h"
#include "lmp_timer.h"
#include "lmp_utils.h"
#include "lmp_features.h"
#include "hc_const.h"
#include "sys_rand_num_gen.h"
#include "bt_timer.h"


extern t_slots LMsec_link_key_request_timeout;
extern t_slots LMsec_pin_code_request_timeout; 
extern boolean LMsec_pin_code_request_timeout_extend;

static struct repeatAttemptEntry repeatAttemptEntries[MAX_REPEATATTEMPTENTRIES];


/* static functions */

static void _AgeRepeatAttemptsList(void);
void LMsec_core_Retry_Auth_Challenge(t_lmp_link *p_link);

/******************************************************************************
 * FUNCTION: LMsec_core_Array_Xor
 *
 ******************************************************************************/
void LMsec_core_Array_Xor(u_int8 ARRAY_SIZE, u_int8* result, 
    u_int8* dest, u_int8* src)
{
    int i=0;

    do
    {
        result[i] = dest[i] ^ src[i];
        i++;
    } while (i<ARRAY_SIZE);
}


/******************************************************************************
 * FUNCTION: LMsec_core_Auth_Allowed
 *
 * DESCRIPTION:-
 * Authentication allowed if
 * 1. No entry in repeated attempt table
 * 2. An entry exists and the wait interval has expired.
 ******************************************************************************/
u_int8 LMsec_core_Auth_Allowed(t_bd_addr *address)
{
    int i;

    for(i=0; i<MAX_REPEATATTEMPTENTRIES; i++)
    {
        if(LMutils_Bd_Addr_Match(&(repeatAttemptEntries[i].address), address))
            break;
    }
  
    if(i >= MAX_REPEATATTEMPTENTRIES)
        return 1;

    return BTtimer_Is_Expired( repeatAttemptEntries[i].lastFailureUpdate + 
                               (repeatAttemptEntries[i].waitInterval<<1) );
}


/******************************************************************************
 * FUNCTION :- LMsec_core_Auth_Challenge
 *
 * INPUTS :- p_link
 *
 * DESCRIPTION :- Initiates a challenge to the peer device (the claimant) 
 *     associated with p_link.
 *
 * STEPS :-
 *       1/ Generate and store random no.
 *       2/ Get current link key, claimant bd_addr.
 *       3/ Call E1 Sec. function and store resulting Sres.
 *       4/ If a semi_permanent key is being used, store aco 
 *              (later used to gen. encryption key).
 *       5/ Set Sub_State to W4_SRES and send LMP_au_rand with random no. 
 *          to claimant
 *       
 *****************************************************************************/
void LMsec_core_Auth_Challenge(t_lmp_link *p_link, u_int8 tid_role)
{
    t_rand au_rand;
    t_sres sres;
    t_aco aco;
    u_int8* p_current_link_key;
    t_bd_addr* p_claimant_bd_addr;
    
    // GF 13 Jan 2011 - Trial fix to overcome ViMicro encryption issue
	// with Broadcom Dongle and Host.

    // Based on the following lines from BT Spec
    // "After a device has received an LMP_encryption_mode_req PDU and
    //  sent an LMP_accepted PDU it shall not send an LMP_au_rand PDU
    //  before encryption is started"

	if (((p_link->encr_sub_state >= W4_ENCR_MODE_REPLY) &&
         (p_link->encr_sub_state <= W4_START_ENCR_REPLY)) ||
		(p_link->encrypt_paused !=0))
	{
		// 
		// Postpone the Authentication challenge by 0.5 second.
		// During this period the Encryption should be complete.

		LMtmr_Set_Timer(800, LMsec_core_Retry_Auth_Challenge, p_link, 1);
		return;
	}
    
    SYSrand_Get_Rand_128_Ex(au_rand);
    LMutils_Array_Copy(RANDOM_NUM_SIZE, p_link->rand, au_rand);
    if(SEMI_PERMANENT == LMacl_Get_Current_Key_Persistance(p_link))
    {
        p_current_link_key = p_link->link_key;
    }
    else /* TEMPORARY key */
    {
        p_current_link_key = g_LM_config_info.master_key;
    }
    p_claimant_bd_addr = &p_link->bd_addr;  
    
    LM_SecEng_Authenticate_E1(au_rand, p_current_link_key, p_claimant_bd_addr, 
        sres, aco);
    
    LMutils_Array_Copy(SRES_SIZE, p_link->sres, sres);
    if(SEMI_PERMANENT == LMacl_Get_Current_Key_Persistance(p_link))
        LMutils_Array_Copy(ACO_SIZE, p_link->aco, aco);       
    p_link->auth_sub_state = W4_SRES;
    LMsec_peer_Send_LMP_Message(p_link, LMP_AU_RAND, tid_role, au_rand);
    
}

//
// This Function is called by Timer.
// 
// It is a re-try of an authentication which was postponed by 0.5 sec
// because it overlaps with Encryption Change. 
//
// Based on the following lines from BT Spec
// "After a device has received an LMP_encryption_mode_req PDU and
//  sent an LMP_accepted PDU it shall not send an LMP_au_rand PDU
//  before encryption is started"

void LMsec_core_Retry_Auth_Challenge(t_lmp_link *p_link)
{
	LMsec_core_Auth_Challenge(p_link,(u_int8)p_link->role);
}

/*
 * Clean out any existing timers on an unexpected detach
 */
void LMsec_core_Clean_On_Detach(t_lmp_link* p_link)
{
    if(p_link->sec_timer)
    {
        LMtmr_Clear_Timer(p_link->sec_timer);
        p_link->sec_timer = 0;
    }
}

/******************************************************************************
 * FUNCTION: LMsec_core_Auth_Complete
 *
 ******************************************************************************/
void LMsec_core_Auth_Complete(t_lmp_link *p_link, t_error status)
{
    /*
     * Turn off the timer guard.
     */
    LMtmr_Clear_Timer(p_link->sec_timer);
    p_link->sec_timer = 0;

    /*
     * No matter how you get here, the sub state is forced
     * to idle at this point. This should only matter for
     * arcane error scenarios. At least the next authentication/pairing
     * procedure should not be compromised.
     */
    p_link->auth_sub_state = SUB_STATE_IDLE;

    /* 
     * If auth. part of pairing/change link key, send Link Key Notification,  
     * Store Link Key???, if not, must delete old if it exists in database 
     */
	// GF 22 June 2009
#if 0//(PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
	if (p_link->auth_super_state == INITIAL_SSP)
	{
        if(NO_ERROR == status)  
        {           
            LMsec_upper_Send_HCI_Event(p_link, HCI_LINK_KEY_NOTIFICATION_EVENT,
                NO_ERROR);

			LMsec_upper_Send_HCI_Event(p_link, HCI_AUTHENTICATION_COMPLETE_EVENT, 
            status);  

			p_link->auth_super_state ^= INITIAL_SSP;

        }

	}
    else 
#endif
	if(ORIG_PAIRING_PROC_ON(p_link->auth_super_state) || 
        TERM_PAIRING_PROC_ON(p_link->auth_super_state) ||
        HCI_CHANGE_COMBKEY_PROC_ON(p_link->auth_super_state))       
    {   
        if(ORIG_PAIRING_PROC_ON(p_link->auth_super_state))      
            p_link->auth_super_state ^= ORIG_PAIRING;

        if(TERM_PAIRING_PROC_ON(p_link->auth_super_state))
            p_link->auth_super_state ^= TERM_PAIRING;
        
        if(NO_ERROR == status)  
        {           
			if (HCI_CHANGE_COMBKEY_PROC_ON(p_link->auth_super_state))
			{
#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
				if (g_LM_config_info.ssp_enabled)
					p_link->key_type = CHANGED_COMBINATION_KEY;
				else
#endif
				p_link->key_type = COMBINATION_KEY;
			}
            LMsec_upper_Send_HCI_Event(p_link, HCI_LINK_KEY_NOTIFICATION_EVENT,
                NO_ERROR);
        }
    }
    if(p_link->setup_complete_bit_mask != 3)
    {
        p_link->auth_super_state &= ~0x1F;
        LMconnection_LM_Auth_Complete(p_link, status);
    }
    else if(HCI_SUPER_STATE(p_link->auth_super_state))
    {
        p_link->auth_super_state &= ~0x1F;
        LMsec_upper_Send_HCI_Event(p_link, HCI_AUTHENTICATION_COMPLETE_EVENT, 
            status);            
    }
    else    
    {
        if(AUTHENTICATION_PROC_ON(p_link->auth_super_state)) 
            p_link->auth_super_state ^= AUTHENTICATION;

        if(HCI_CHANGE_COMBKEY_PROC_ON(p_link->auth_super_state))    
        {   
            if(ORIG_CHANGE_COMBKEY == p_link->auth_super_state)
            {

#if (PRH_BS_CFG_SYS_LMP_PAUSE_ENCRYPTION_SUPPORTED==1)
                if ((p_link->encrypt_enable) && mFeat_Check_Pause_Encryption(p_link->remote_features))
                {
                    p_link->encr_super_state = SET_ENCR_OFF;             
                    p_link->encr_sub_state = W4_STOP_ENCR_REQ;
                    p_link->event_status = status;
                    p_link->encrypt_paused = ENCRYPT_PAUSED_INITIATED_BY_LOCAL_DEVICE
                        | ENCRYPT_RESUME_PENDING_LINK_KEY_CHANGE_EVENT;
                    LMsec_peer_Send_LMP_Pause_Encryption_Req(p_link);
                }
                else if (!p_link->encrypt_enable)
#endif
                {
                    LMsec_upper_Send_HCI_Event(p_link, 
                        HCI_CHANGE_CONNECTION_LINK_KEY_COMPLETE_EVENT, status);

					p_link->auth_super_state ^= ORIG_CHANGE_COMBKEY;  
                }
#if 0 // GF 11 Sept
                p_link->auth_super_state ^= ORIG_CHANGE_COMBKEY;   
#endif
            }
            else
                p_link->auth_super_state ^= TERM_CHANGE_COMBKEY;        
        }
        else if(HCI_MASTER_LINKKEY_PROC_ON(p_link->auth_super_state))   
        {   
            if((p_link->encrypt_enable == OFF) || (p_link->role == SLAVE))
            {
                p_link->auth_super_state ^= HCI_MASTER_LINKKEY;
            }
            LMsec_upper_Send_HCI_Event(p_link, 
                HCI_MASTER_LINK_KEY_COMPLETE_EVENT, status);
            if(status == NO_ERROR)
            {
                if((p_link->encrypt_enable == ON) && (p_link->role == MASTER))
                {
                    if(!p_link->sec_timer)
                        p_link->sec_timer = LMtmr_Set_Timer(
                            PRH_DEFAULT_LMP_TRANSACTION_TIMEOUT,
                            LMsec_core_TimeoutHandler,
                            p_link, /* current device link */
                            1 /* dynamic */);
                    LMsec_core_Set_Encryption(p_link, OFF);
                }

            /* 
             * If Authentication fails on a link during a master link key 
             * Procedure the link is detached immediately.
             */
            }
            else
            {
                LMconnection_LM_Disconnect_Req(p_link->handle,status);
            }
        }   
    }   
}


/******************************************************************************
 * FUNCTION: LMsec_core_Auth_Response
 *
 ******************************************************************************/
void LMsec_core_Auth_Response(t_lmp_link *p_link, u_int8 tid_role, 
    t_rand au_rand)
{
    t_sres sres;
    t_aco aco;
    u_int8* p_current_link_key;
    const t_bd_addr* p_local_bd_addr;
    
    p_local_bd_addr = SYSconfig_Get_Local_BD_Addr_Ref();
    
    if(SEMI_PERMANENT == LMacl_Get_Current_Key_Persistance(p_link))
        p_current_link_key = p_link->link_key;
    else /* TEMPORARY key */
        p_current_link_key = g_LM_config_info.master_key;

    LM_SecEng_Authenticate_E1(au_rand, p_current_link_key, 
        p_local_bd_addr, sres, aco);

    if(SEMI_PERMANENT == LMacl_Get_Current_Key_Persistance(p_link))
        LMutils_Array_Copy(ACO_SIZE, p_link->aco, aco);  

    LMsec_peer_Send_LMP_Message(p_link, LMP_SRES, tid_role, sres);
}


/******************************************************************************
 * FUNCTION: LMsec_core_Get_Link_Key
 *
 ******************************************************************************/
t_link_key_search_res LMsec_core_Get_Link_Key(t_lmp_link* p_link)
{
    u_int8* local_link_key;
    
    /* check if link key stored in p_link is valid */
    if(p_link->link_key_exists)
	{
#if (PRH_BS_CFG_SYS_PIN_CODE_REQ_EXTEND == 1) /* 2778 */
		extern t_slots LMsec_pin_code_request_extended_timeout;
        LMsec_pin_code_request_extended_timeout = PRH_LMP_MSG_TIMEOUT - 500;
#endif
        return LINK_KEY_EXISTS;
	}
    
    local_link_key = LMkeydb_Read_Link_Key(&p_link->bd_addr);
    if (0 == local_link_key)
    {
        if(W4_ENCR_MODE_REPLY != p_link->encr_sub_state)
            p_link->auth_sub_state = W4_LINK_KEY_REQ_REPLY;  
        /* Generate Host Link Key Request Event */
        LMsec_upper_Send_HCI_Event(p_link, 
            HCI_LINK_KEY_REQUEST_EVENT, NO_ERROR);
        return NO_LINK_KEY_EXISTS;
    }
    else
    {
        LMutils_Array_Copy(LINK_KEY_SIZE, p_link->link_key, local_link_key);
        return LINK_KEY_EXISTS;
    }
}


/******************************************************************************
 * FUNCTION: LMsec_core_Init_CombKey_Setup
 *
 ******************************************************************************/
void LMsec_core_Init_CombKey_Setup(t_lmp_link* p_link)
{
    t_rand rand;
     
    t_link_key local_comb_key_part, encode_local_comb_rand;
    u_int8* p_current_link_key;
    const t_bd_addr* p_local_bd_addr;

    p_local_bd_addr = SYSconfig_Get_Local_BD_Addr_Ref();

    SYSrand_Get_Rand_128_Ex(rand);
    LM_SecEng_Generate_Key_E21(rand, p_local_bd_addr, local_comb_key_part);
    /* 
     * STORE in rand - this Result - this is used later to compute 
     * combination Link Key 
     */
    LMacl_Write_CombKey_Part(p_link, local_comb_key_part);
    p_current_link_key = p_link->link_key;

    /* encode_local_comb_rand = current_link_key XOR rand */
    LMsec_core_Array_Xor(LINK_KEY_SIZE,
        (u_int8*)encode_local_comb_rand, p_current_link_key,(u_int8*)rand);
    LMsec_peer_Send_LMP_Message(p_link, LMP_COMB_KEY, 
        p_link->sec_current_proc_tid_role, encode_local_comb_rand);
}


/******************************************************************************
 * FUNCTION: LMsec_core_Initialise
 *
 * INPUTS :- new_key_type
 *
 * DESCRIPTION :- Performs initialisation for the lmp security module. 
 *     Sets up the callback array for HCI events. It also sets the 
 *     key_type for the device (UNIT or COMBINATION)
 *
 ******************************************************************************/
void LMsec_core_Initialise(t_link_key_type new_key_type)
{
    int i;

#if (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED==1)
    g_LM_config_info.key_type = new_key_type;
#endif
    for(i=0; i<MAX_REPEATATTEMPTENTRIES; i++)
    {
        repeatAttemptEntries[i].waitInterval = 0;
        repeatAttemptEntries[i].lastFailureUpdate = 0;
        BDADDR_Set_LAP(&repeatAttemptEntries[i].address,0x0);
        BDADDR_Set_UAP(&repeatAttemptEntries[i].address,0x0);
        BDADDR_Set_NAP(&repeatAttemptEntries[i].address,0x0);
    }

    LMsec_link_key_request_timeout = PRH_BS_CFG_SYS_LINK_KEY_REQUEST_TIMEOUT;
    LMsec_pin_code_request_timeout = PRH_BS_CFG_SYS_PIN_CODE_REQUEST_TIMEOUT;
    LMsec_pin_code_request_timeout_extend = FALSE;
}

/******************************************************************************
 *  FUNCTION :- LMsec_core_Init_Authentication 
 *
 *  INPUTS :- p_link
 *
 *  DESCRIPTION :- Invoked by the LMP ch or the HC to initiate 
 *      Authentication of peer device.
 * 
 *  STEPS :-
 *      1/ Set Super_State to ORIG_PAIRING_PENDING 
 *      2/  Call LMsec_core_Get_Link_Key to get current link key for device
 *      3/ IF ( Link Key exists)
 *             Set Super_State to SUPER_STATE_IDLE
 *             Invoke authentication challenge
 *          ELSE
 *             LMsec_core_Get_Link_Key will send a Link_Key_Request to the Host
 *             Wait for LM_Sec_Pr_Link_Key_Request[_Negative]_Reply to be calld 
 *               
 ******************************************************************************/
t_error LMsec_core_Init_Authentication(t_lmp_link* p_link)
{
    static int i;
    
    i++;
    
    if(!p_link)
    {
        return INVALID_HCI_PARAMETERS;
    }

    if(!LMsec_core_Auth_Allowed(&(p_link->bd_addr)))
    {
        if((p_link->setup_complete_bit_mask != 3) && (p_link->role == MASTER))
            LMconnection_Send_LMP_Detach(p_link, REPEATED_ATTEMPTS);

        return REPEATED_ATTEMPTS;
    }
    
    /*
     * Start a timer to guard the security
     * transaction.
     */
     if(!p_link->sec_timer)
        p_link->sec_timer = LMtmr_Set_Timer(PRH_DEFAULT_LMP_TRANSACTION_TIMEOUT,
            LMsec_core_TimeoutHandler,
            p_link, /* current device link */
            1 /* dynamic */);

    if(TERM_PAIRING == p_link->auth_super_state)
    {
        /* 
         * peer has already initiated pairing, will callback lmp_ch 
         * when pairing is complete 
         */
        p_link->auth_super_state = ACL_TERM_PAIRING;
        return NO_ERROR;
    }
    
    if((SUB_STATE_IDLE == p_link->auth_sub_state) 
        && (SUPER_STATE_IDLE == p_link->auth_super_state))
    {
        /* Assume No Link Key and set SuperState=ORIG_PAIRING_PENDING */
        p_link->auth_super_state = ACL_ORIG_PAIRING;  
        p_link->sec_current_proc_tid_role = p_link->role;
        /* 
         * If a Link Key exists, start the Auth Challenge, 
         * otherwise wait for key from Host 
         * (appropriate sub state is set in _LM_Get_Link_Key() 
         */
        if (LINK_KEY_EXISTS == LMsec_core_Get_Link_Key(p_link)) 
        {
            /* Link Key Exists => Set SuperState=IDLE */
            p_link->auth_super_state = ACL_AUTHENTICATION;
            LMsec_core_Auth_Challenge(p_link, (u_int8)p_link->role);
        }
        
    } /* Make sure that Authentication has not been initiated locally before */
    else if (W4_LINK_KEY_REQ_REPLY == p_link->auth_sub_state && 
             !ORIG_PAIRING_PROC_ON(p_link->auth_super_state) )
    {
        p_link->auth_super_state = ACL_ORIG_PAIRING;
        p_link->sec_current_proc_tid_role = p_link->role;
        p_link->auth_sub_state = W4_LINK_KEY_REQ_REPLY_CHALL_PENDING;
        return NO_ERROR;
    }
    else
        return UNSPECIFIED_ERROR;
    
    return NO_ERROR;
}


/******************************************************************************
 * FUNCTION: LMsec_core_Init_Encryption
 * 
 ******************************************************************************/
t_error LMsec_core_Init_Encryption(t_lmp_link* p_link, t_encrypt_mode mode)
{   
    if(SET_ENCR_ON_PROC_ON(p_link->encr_super_state))
    {
        p_link->encr_super_state |= 0x3; /* SET_ENCR_ON to ACL_SET_ENCR_ON */
    }
    else if( (SUB_STATE_IDLE == p_link->encr_sub_state) 
        && (SUPER_STATE_IDLE == p_link->encr_super_state))
    {
        /*
         * Start a timer to guard the security
         * transaction.
         */
         if(!p_link->sec_timer)
             p_link->sec_timer = LMtmr_Set_Timer(
                 PRH_DEFAULT_LMP_TRANSACTION_TIMEOUT,
                 LMsec_core_TimeoutHandler,
                 p_link, /* current device link */
                 1 /* dynamic */);

        p_link->encr_sub_state = W4_ENCR_MODE_REPLY;
        p_link->encr_super_state = ACL_SET_ENCR_ON;       
        p_link->sec_current_proc_tid_role = p_link->role;
        p_link->encrypt_mode = mode; 
        LMch_Disable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_ENC);
        LMsec_peer_Send_LMP_Message(p_link, 
            LMP_ENCRYPTION_MODE_REQ, (u_int8)p_link->role, &mode);
    }
    else
        return COMMAND_DISALLOWED;
    
    return NO_ERROR;
}


/******************************************************************************
 * FUNCTION: LMsec_core_Init_LinkKey_Negotiation
 *
 ******************************************************************************/
void LMsec_core_Init_LinkKey_Negotiation(t_lmp_link *p_link)
{
#if (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED==1)
    p_link->key_type = g_LM_config_info.key_type;
#endif

#if (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED==1)
    if (COMBINATION_KEY_DEVICE == g_LM_config_info.key_type )
    {
        p_link->auth_sub_state = W4_COMB_KEY_RESPONSE;
        LMsec_core_Init_CombKey_Setup(p_link);
    }
    else
    {
        LMsec_core_Init_UnitKey_Setup(p_link);
        p_link->auth_sub_state = W4_UNIT_KEY_RESPONSE;
    }
#else
    p_link->auth_sub_state = W4_COMB_KEY_RESPONSE;
    LMsec_core_Init_CombKey_Setup(p_link);
#endif
}


/******************************************************************************
 * FUNCTION: LMsec_core_PeerData_Tx_Go_Callback
 *
 ******************************************************************************/
void LMsec_core_PeerData_Tx_Go_Callback(t_deviceIndex device_index)
{
    /* Completes Encryption -- Raises event Encry_Change */
    /* Check sub_state, super_state */
    t_lmp_link *p_link;
    
    p_link = LMaclctr_Find_Device_Index(device_index);
    if(p_link)
    {

        if(W4_START_ENCR_REPLY == p_link->encr_sub_state)
        {       
            p_link->encrypt_enable = ON;      
            p_link->encr_sub_state = SUB_STATE_IDLE;  
            
            if(SET_ENCR_ON_PROC_ON(p_link->encr_super_state))
                LMsec_core_Set_Encryption_Complete(p_link, NO_ERROR);
        }  
        else if(W4_STOP_ENCR_REPLY == p_link->encr_sub_state)
        {
            p_link->encrypt_enable = OFF;
            LMsec_core_Set_Encryption_Complete(p_link, NO_ERROR);
        }
    }
}


/******************************************************************************
 * FUNCTION: LMsec_core_Enable_Rx_Encryption
 *
 * DESCRIPTION 
 * This function is called back on the receipt of the Ack for a 
 * LMP_Start_Encryption_Req
 * At this point the encryption keys are written into the LC.
 ******************************************************************************/
void LMsec_core_Enable_Rx_Encryption(t_deviceIndex device_index)
{
    t_lmp_link *p_link;
    
    p_link = LMaclctr_Find_Device_Index(device_index);
    if(p_link)
    {    
        /* 
         * NB :- Why is link key size written down into the device 
         * instead of the negotiated key size 
         */
        LC_Write_Encryption_Key_Ref_And_Length(p_link->device_index, 
            p_link->encry_key, LINK_KEY_SIZE);
        LC_Write_Encryption_Mode(p_link->device_index, ENCRYPT_MASTER_RX_ONLY);
    }
}


/******************************************************************************
 * FUNCTION: LMsec_core_PeerData_Tx_Stop_Callback
 *
 ******************************************************************************/
void LMsec_core_PeerData_Tx_Stop_Callback(t_deviceIndex device_index)
{
    t_lmp_link *p_link;
    
    p_link = LMaclctr_Find_Device_Index(device_index);
    if(p_link)
    {
        /* Disable L2CAP Data */
        if( W4_START_ENCR_REPLY == p_link->encr_sub_state)
        {
            /* 
             * Cannot generate encryption key here as this is called in 
             * interrupt space and takes about 40 slots
             * Note : If broadcast encryption is enabled the Encryption key 
             * size used has to be the lowest key size
             * of all the peer devices in the piconet
             */
#if 0 // GF 13 November - ENV/BV16 Conformance ???
            if(!(p_link->encrypt_paused ))
            {
            LMsec_peer_Send_LMP_Message(p_link, LMP_START_ENCRYPTION_REQ, 
                p_link->sec_current_proc_tid_role, p_link->rand);
            }
            else
            {
                LMsec_peer_Resume_Encryption(p_link);
            }
#else
            LMsec_peer_Send_LMP_Message(p_link, LMP_START_ENCRYPTION_REQ,
                 p_link->sec_current_proc_tid_role, p_link->rand);
#endif
        }
        
        else if(W4_STOP_ENCR_REPLY == p_link->encr_sub_state)
        {
            LC_Write_Encryption_Mode(p_link->device_index, 
                ENCRYPT_MASTER_TX_ONLY);
            LMsec_peer_Send_LMP_Message(p_link, LMP_STOP_ENCRYPTION_REQ, 
                p_link->sec_current_proc_tid_role, 0);
        }
    }
}

extern t_rand temp_rand;
void LMsec_core_Change_To_Temporary_Key(t_lmp_link* p_link)
{
    u_int8* p_current_link_key;
    t_link_key overlay, temp_key;

   /* 
    * if key is already temporary then
    *    use this procedure to CHANGE the current temporary key 
    *    (spec ambiguous, below based on discussions with other solution 
    *    providers)
    * endif
    */
    
    LMsec_peer_Send_LMP_Message(p_link, LMP_TEMP_RAND, 
        (u_int8)p_link->role, temp_rand);
    p_current_link_key = p_link->link_key;
    
   /* 
    * Pass Null bd_addr to E22 since rand2 length is 16 bytes 
    *   => no Augmenting will be required 
    */
    LM_SecEng_Generate_Key_E22(temp_rand, p_current_link_key, 
        RANDOM_NUM_SIZE, 0, overlay);
    LMsec_core_Array_Xor(LINK_KEY_SIZE, temp_key, overlay, 
        g_LM_config_info.master_key);
    LMsec_peer_Send_LMP_Message(p_link, LMP_TEMP_KEY, (u_int8)p_link->role, 
        temp_key);
    LMacl_Set_Key_Persistance(p_link, TEMPORARY);
    
   /* 
    * Authentication follows change to temporary link key.
    */
    p_link->sec_current_proc_tid_role = (u_int8)p_link->role;
    p_link->auth_super_state = HCI_MASTER_LINKKEY|AUTHENTICATION;
    LMsec_core_Auth_Challenge(p_link, (u_int8)p_link->role);
}


void LMsec_core_Change_To_Semi_Permanent_Key(t_lmp_link* p_link)
{
    p_link->auth_sub_state = W4_USE_SEMI_PERM_REPLY;
    p_link->auth_super_state = HCI_MASTER_LINKKEY;
    
    LMacl_Set_Key_Persistance(p_link, SEMI_PERMANENT);
    LMsec_peer_Send_LMP_Message(p_link, LMP_USE_SEMI_PERMANENT_KEY, 
        (u_int8)p_link->role,0);

    /* 
     * Authentication follows change to temporary link key 
     */
    p_link->sec_current_proc_tid_role = (u_int8)p_link->role;
    p_link->auth_super_state = HCI_MASTER_LINKKEY|AUTHENTICATION;
}


/******************************************************************************
 * FUNCTION: LMsec_core_RecordAuthFailure
 *
 * DESCRIPTION:
 ******************************************************************************/
t_error LMsec_core_RecordAuthFailure(t_bd_addr *address)
{
    int i;
    
    /* 
     * Update the wait intervals for the current list, will free up any 
     * entries whose waitInterval have
     * decreased back to zero, leaving space for any new entries. 
     */
    _AgeRepeatAttemptsList();
    /* Check for previous entry */
    for(i=0; i<MAX_REPEATATTEMPTENTRIES; i++)
    {
        if(LMutils_Bd_Addr_Match(&(repeatAttemptEntries[i].address), address))
            break;
    }
    
    if(i >= MAX_REPEATATTEMPTENTRIES)
    {
        for(i=0; i<MAX_REPEATATTEMPTENTRIES; i++)
        {
            if(0 == repeatAttemptEntries[i].waitInterval)
            {
                LMutils_Set_Bd_Addr(&(repeatAttemptEntries[i].address),address);
                break;
            }
        }
    }
    
    /* Check if a free entry was found */
    if(i >= MAX_REPEATATTEMPTENTRIES)
        return UNSPECIFIED_ERROR;
    
    if(0 == repeatAttemptEntries[i].waitInterval)
    {
        repeatAttemptEntries[i].waitInterval = 
            (t_clock)INITIAL_REPEATATTEMPTWAITINTERVAL;
    }
    else
    {
        /* Double next wait interval */
        if( ((repeatAttemptEntries[i].waitInterval)<<1) >= 
            MAX_REPEATATTEMPTWAITINTERVAL)
        {
            repeatAttemptEntries[i].waitInterval = 
                (t_clock)MAX_REPEATATTEMPTWAITINTERVAL;
        }
        else
        {
            repeatAttemptEntries[i].waitInterval = 
                (t_clock)((repeatAttemptEntries[i].waitInterval)<<2);
        }
    }
    repeatAttemptEntries[i].lastFailureUpdate = BTtimer_Get_Native_Clock();
    
    return NO_ERROR;
}


/******************************************************************************
 * FUNCTION: LMsec_core_RecordAuthSuccess
 *
 * DESCRIPTION:
 * Invoked normally on receipt of a valid sres in an LMP_sres message to
 * record the authentication success.
 ******************************************************************************/
t_error LMsec_core_RecordAuthSuccess(t_bd_addr *address)
{
    int i;
    
    /* Check for previous entry */
    for(i=0; i<MAX_REPEATATTEMPTENTRIES; i++)
    {
        if(LMutils_Bd_Addr_Match(&(repeatAttemptEntries[i].address), address))
            break;
    }
    
    if(i >= MAX_REPEATATTEMPTENTRIES)
    {
        return UNSPECIFIED_ERROR;
    }

    repeatAttemptEntries[i].waitInterval = 0;
    repeatAttemptEntries[i].lastFailureUpdate = 0;
    BDADDR_Set_LAP(&repeatAttemptEntries[i].address,0x0);
    BDADDR_Set_UAP(&repeatAttemptEntries[i].address,0x0);
    BDADDR_Set_NAP(&repeatAttemptEntries[i].address,0x0);
    
    return NO_ERROR;
}


/******************************************************************************
 * FUNCTION: LMsec_core_Set_Encryption
 *
 ******************************************************************************/
void LMsec_core_Set_Encryption(t_lmp_link *p_link, t_flag encrypt_enable)
{
	u_int8 newEncryptMode;

    p_link->restore_current_encrypt_mode = p_link->encrypt_mode;

    if (ON == encrypt_enable)
    {   
        /* 
         * Assumes encrypt_mode is > 0 (1 or 2) 
         * ALTERNATIVE:  (use device encrypt_mode unless equal to ENCRYPT_NONE 
         */
        
        if (TEMPORARY == LMacl_Get_Current_Key_Persistance(p_link))
            p_link->encrypt_mode = ENCRYPT_POINT2POINT_BROADCAST;
        else
            p_link->encrypt_mode = ENCRYPT_POINT2POINT;
        
        p_link->encr_super_state |= HCI_SET_ENCR_ON;
    }
    else
    {
        p_link->encr_super_state |= HCI_SET_ENCR_OFF;
		p_link->encrypt_mode = ENCRYPT_NONE;
    }
    
    p_link->encr_sub_state = W4_ENCR_MODE_REPLY;
    LMch_Disable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_ENC);

	if (p_link->encrypt_mode > 0)
		newEncryptMode = 0x01;
	else
		newEncryptMode = 0x00;

#if 0
    LMsec_peer_Send_LMP_Message(p_link, LMP_ENCRYPTION_MODE_REQ, 
        p_link->sec_current_proc_tid_role, &(p_link->encrypt_mode));
#else
    LMsec_peer_Send_LMP_Message(p_link, LMP_ENCRYPTION_MODE_REQ, 
        p_link->sec_current_proc_tid_role, &newEncryptMode);
#endif
}


/******************************************************************************
 * FUNCTION: LMsec_core_Set_Encryption_Complete
 *
 ******************************************************************************/
void LMsec_core_Set_Encryption_Complete(t_lmp_link *p_link, t_error status)
{
    /*
     * Turn off the timer guard.
     */
    LMtmr_Clear_Timer(p_link->sec_timer);
    p_link->sec_timer = 0;

    /*
     * turn off encr_sub_state
     */
    p_link->encr_sub_state = SUB_STATE_IDLE;
    
    if(ACL_SUPER_STATE(p_link->encr_super_state))
    {
        p_link->encr_super_state ^= ACL_SET_ENCR_ON;
        LMconnection_LM_Encrypt_Complete(p_link, status);
    }
    else if (HCI_SUPER_STATE(p_link->encr_super_state))
    {
        p_link->encr_super_state = SUPER_STATE_IDLE;
        LMch_Enable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_ENC);
        LMsec_upper_Send_HCI_Event(p_link, HCI_ENCRYPTION_CHANGE_EVENT, status);

#if 1 // GF 11 Sept
		if ((p_link->encrypt_enable) && (p_link->auth_super_state & ORIG_CHANGE_COMBKEY))
		{
			if (!mFeat_Check_Pause_Encryption(p_link->remote_features))
			{
				LMsec_upper_Send_HCI_Event(p_link, 
                HCI_CHANGE_CONNECTION_LINK_KEY_COMPLETE_EVENT, status);

				p_link->auth_super_state ^= ORIG_CHANGE_COMBKEY;
			}

		}
#endif
    }
    else
        p_link->encr_super_state = SUPER_STATE_IDLE;
}


/******************************************************************************
 * FUNCTION: LMsec_core_TimeoutHandler
 *
 ******************************************************************************/
void LMsec_core_TimeoutHandler(t_lmp_link *p_link)
{
    LMtmr_Clear_Timer(p_link->sec_timer);
    p_link->sec_timer = 0;
    
    if(p_link->setup_complete_bit_mask != 3)
    {
        LMconnection_LM_Auth_Complete(p_link, LMP_RESPONSE_TIMEOUT);
    }
    else if(HCI_SUPER_STATE(p_link->auth_super_state))
    {
        LMsec_upper_Send_HCI_Event(p_link, HCI_AUTHENTICATION_COMPLETE_EVENT, 
            LMP_RESPONSE_TIMEOUT);
    }
    else if((p_link->auth_super_state & ORIG_CHANGE_COMBKEY) &&
            (!(p_link->auth_super_state & 0x40)))
    {
        LMsec_upper_Send_HCI_Event(p_link, 
            HCI_CHANGE_CONNECTION_LINK_KEY_COMPLETE_EVENT, 
            LMP_RESPONSE_TIMEOUT);
    }
    else if((p_link->auth_super_state & HCI_MASTER_LINKKEY) && 
            (!(p_link->auth_super_state & 0x80)))
    {
        if(p_link->encr_super_state != HCI_SET_ENCR_OFF)
        {
            /*
             * MLK procedure has aggressively changed key_persistance to 
             * the new key persistance. However, at this point the host 
             * controller has no knowledge of what the key
             * persistance was before the attempted changeover to the 
             * new key persistance.
             * There is currently no safe way back to the previous key 
             * persistance.
             *
             * E.g. the IUT could have been on either temp key or 
             * semi-p key when it was
             * asked by its host to change to temp key.
             */
            LMacl_Set_Key_Persistance(p_link, 
                LMacl_Get_Previous_Key_Persistance(p_link));
            LMsec_upper_Send_HCI_Event(p_link, 
                HCI_MASTER_LINK_KEY_COMPLETE_EVENT, LMP_RESPONSE_TIMEOUT);
        }
    }

    p_link->auth_super_state = SUPER_STATE_IDLE;
    p_link->auth_sub_state = SUB_STATE_IDLE; 

    if(ACL_SUPER_STATE(p_link->encr_super_state))
        LMconnection_LM_Encrypt_Complete(p_link, LMP_RESPONSE_TIMEOUT);

    else if (HCI_SUPER_STATE(p_link->encr_super_state))
    {
		if ( (W4_STOP_ENCR_REPLY == p_link->encr_sub_state) ||
			 (W4_STOP_ENCR_REQ == p_link->encr_sub_state) ||
			 (W4_ENCR_MODE_REPLY == p_link->encr_sub_state) )
		{
			LMsec_peer_LMP_Not_Accepted(p_link, LMP_STOP_ENCRYPTION_REQ,
				LMP_RESPONSE_TIMEOUT);
		}
		else
		{
			LMch_Enable_L2CAP_Traffic(p_link,LM_CH_L2CAP_EN_ENC);
			LMsec_upper_Send_HCI_Event(p_link, HCI_ENCRYPTION_CHANGE_EVENT, 
				LMP_RESPONSE_TIMEOUT);
		}
    }

    p_link->encr_super_state = SUPER_STATE_IDLE;
    p_link->encr_sub_state = SUB_STATE_IDLE;
}

#if (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED==1)
/******************************************************************************
 * FUNCTION: LM_sec_core_Init_UnitKey_Setup
 *
 ******************************************************************************/
void LMsec_core_Init_UnitKey_Setup(t_lmp_link* p_link)
{
    u_int8* p_unit_key;
    u_int8* p_current_init_key;
    t_link_key encode_unit_key;
  
    p_unit_key = SYSconfig_Get_Unit_Key_Ref();
    p_current_init_key = p_link->link_key;
   
    LMsec_core_Array_Xor(LINK_KEY_SIZE,(u_int8*)encode_unit_key,
        p_current_init_key,p_unit_key);
    LMsec_peer_Send_LMP_Message(p_link, LMP_UNIT_KEY, 
        p_link->sec_current_proc_tid_role, encode_unit_key);
}

#endif
/******************************************************************************
 * FUNCTION: _AgeRepeatAttemptsList
 *
 ******************************************************************************/
void _AgeRepeatAttemptsList()
{
    t_clock timeElapsed;
    t_clock currentFailure;
    t_clock currentFailureUpdate;
    t_clock newWaitInterval;
    int i;
    
    for(i=0; i<MAX_REPEATATTEMPTENTRIES; i++)
    {
        newWaitInterval = repeatAttemptEntries[i].waitInterval;
        currentFailure = BTtimer_Get_Native_Clock();
        currentFailureUpdate = repeatAttemptEntries[i].lastFailureUpdate;
        timeElapsed = 
            (currentFailure > repeatAttemptEntries[i].lastFailureUpdate)?
            (currentFailure - repeatAttemptEntries[i].lastFailureUpdate):
            (BT_CLOCK_MAX_TICKS - repeatAttemptEntries[i].lastFailureUpdate + 
             currentFailure);
        
        /* 
         * Use exponential decrease of repeatAttemptWaitInterval in 
         * the calculation of the next wait interval 
         */
        while(timeElapsed >= (newWaitInterval<<1))
        {         
            currentFailureUpdate += (newWaitInterval<<1);
            timeElapsed -= (newWaitInterval<<1);
            if(newWaitInterval < INITIAL_REPEATATTEMPTWAITINTERVAL)
            {
                newWaitInterval = 0;
                break;
            }
            newWaitInterval >>= 1;
        }
        repeatAttemptEntries[i].waitInterval = newWaitInterval;
        repeatAttemptEntries[i].lastFailureUpdate = currentFailureUpdate;
    }
}


/******************************************************************************
 * FUNCTION: LMsec_core_Get_Broadcast_Key_Length
 *
 * DESCRIPTION 
 * This function is called to determine the 
 * most suitable broadcast encryption key length to propose to
 * slave devices.
 *
 * In Bluetooth 1.1 implementations, the master device will have
 * no information regarding its peers' preferred key lengths, so
 * it may as well start with the longest key available to itself
 * and work down from that.
 *
 * In Bluetooth 1.2 implementations, the master device will have
 * some information from some 1.2 peers regarding their preferred
 * key lengths, so it should take this information into account
 * when determining the preferred key length
 *
 ******************************************************************************/
extern t_link_entry link_container[PRH_MAX_ACL_LINKS];
u_int8 LMsec_core_Get_Broadcast_Key_Length(void)
{
#if (PRH_BS_CFG_SYS_LMP_BROADCAST_ENCRYPTION_SUPPORTED==1)
    u_int16 enc_key_size_mask =
        g_LM_config_info.bcast_enc_key_size_mask;

    u_int8 num_links, key_length, possible_key_length, most_popular_key_length;
    u_int8 num_lengths[16] = { 0 };

    for(num_links = 0; num_links < PRH_MAX_ACL_LINKS; num_links++)
    {
        if((link_container[num_links].used == 1) && 
            (link_container[num_links].entry.role == MASTER))
        {
            enc_key_size_mask &= 
                link_container[num_links].entry.bcast_enc_key_size_mask;

            /*
             * Place key lengths in buckets.
             */
            for(key_length = 1; key_length <= 16; key_length++)
            {
                if((1<<(key_length-1)) & 
                    link_container[num_links].entry.bcast_enc_key_size_mask)
                {
                    num_lengths[key_length-1]++;
                }
            }
        }
    }

    /*
     * Now have a key size mask.
     * Choose the longest key size
     * available in the mask.
     */
    for(key_length = g_LM_config_info.max_encrypt_key_size; key_length > 0; key_length--)
    {
        if((1<<(key_length-1)) & enc_key_size_mask)
            break;
    }

    /*
     * If cannot find a common key length,
     * then
     * 1) try to find most common key length
     */
    if(key_length == 0)
    {
        /*
         * Run UP the lengths looking for a number which
         * corresponds to the most common link key
         * which is compatible with the local device.
         * Running up the list with a <= term should
         * replace a smaller key with a larger key
         * if they share a commonality.
         */
        most_popular_key_length = 0;
        for(possible_key_length = 1; possible_key_length <= 16; possible_key_length++)
        {
             if((most_popular_key_length <= num_lengths[possible_key_length-1]) &&
                 (num_lengths[possible_key_length-1] != 0))
             {
                 if(1<<(possible_key_length-1) & g_LM_config_info.bcast_enc_key_size_mask)
                 {
                     key_length = possible_key_length;
                     most_popular_key_length = num_lengths[possible_key_length-1];
                 }
             }
        }

        /*
         * If key_length is still 0, then just use the local
         * device's maximum encryption key length.
         * This is to prevent a hacker adding a
         * plaintext device to a piconet and
         * exploiting a 0-length broadcast key.
         * Anyway, don't want to trip over
         * any bugs that may be in other
         * host controllers in the field.
         */
        if(key_length == 0)
        {
            key_length = g_LM_config_info.max_encrypt_key_size;
        }
    }

    return key_length;
#else
    return g_LM_config_info.max_encrypt_key_size ;
#endif
}


/******************************************************************************
 *
 * FUNCTION: LMsec_core_Is_Broadcast_Key_Length_Possible
 *
 * DESCRIPTION 
 * This function is called to determine the 
 * if a broadcast key is possible.
 *
 * The function has one criteria.
 * If there are no slaves at all with a compatible key length to the
 * local device, then there is no broadcast key length possible.
 * else there is.
 *
 ******************************************************************************/
boolean LMsec_core_Is_Broadcast_Key_Length_Possible(void)
{
#if (PRH_BS_CFG_SYS_LMP_BROADCAST_ENCRYPTION_SUPPORTED==1)
    u_int16 enc_key_size_mask =
        g_LM_config_info.bcast_enc_key_size_mask;
    boolean is_possible = 0;
    u_int16 num_links;

    for(num_links = 0; num_links < PRH_MAX_ACL_LINKS; num_links++)
    {
        if((link_container[num_links].used == 1) && 
            (link_container[num_links].entry.role == MASTER))
        {
            if(link_container[num_links].entry.bcast_enc_key_size_mask
                & enc_key_size_mask)
            {
                is_possible = 1;
            }
        }
    }
    return is_possible;
#else
    return 1;
#endif
}

#if (PRH_BS_CFG_SYS_LMP_PAUSE_ENCRYPTION_SUPPORTED==1)
void LMsec_core_Encryption_Key_Refresh(t_lmp_link *p_link)
{
    if((MASTER == p_link->role) && (p_link->encrypt_enable) && !(p_link->state & LMP_DETACH_ACK_PENDING)
        && mFeat_Check_Pause_Encryption(p_link->remote_features))
    {
        p_link->encr_super_state = SET_ENCR_OFF;
        p_link->encr_sub_state = W4_STOP_ENCR_REQ;
        p_link->encrypt_paused = ENCRYPT_PAUSED_INITIATED_BY_LOCAL_DEVICE;
        LMsec_peer_Send_LMP_Pause_Encryption_Req(p_link);
    }

    LMtmr_Reset_Timer(p_link->encryption_key_refresh_index,
        PRH_ENCRYPTION_KEY_REFRESH_TIMEOUT);
}
#endif
