/******************************************************************************
 * MODULE NAME:    lmp_sec_upper.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    LMP Security Protocol Upper Interface.
 *
 *  This module handles security related requests from the Host/upper 
 *  layers and initiates the security procedures requested. It also reports 
 *  back security related events to the Host/upper layers.
 *  
 *  This module, together with lmp_sec_peer.c and lmp_sec_core.c maintains 
 *  the security state machine for each peer device and interacts 
 *  with lmp_security_peer.c to execute security procedures with a peer device.
 *     
 * MAINTAINER:     Daire McNamara
 * CREATION DATE:  23 January 2000
 *
 * SOURCE CONTROL: $Id: lmp_sec_upper.c,v 1.67 2011/01/19 17:24:19 garyf Exp $
 *
 * LICENSE:
 *   This source code is copyright (c) 2000-2004 Ceva Inc.
 *   All rights reserved.
 *
 *************************************************************************/

#include "sys_config.h"
#include "lmp_const.h"
#include "lmp_config.h"
#include "lmp_sec_upper.h"
#include "lmp_sec_peer.h"
#include "lmp_sec_core.h"
#include "lmp_link_key_db.h"
#include "lmp_timer.h"

#include "lmp_acl_container.h"
#include "lmp_acl_connection.h"
#include "lmp_sec_engine.h"
#include "lmp_ch.h"
#include "lmp_link_policy.h"
#include "lmp_features.h"
#include "lmp_utils.h"
#include "hc_event_gen.h"
#include "hc_const.h"
#include "sys_rand_num_gen.h"
#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
#include "lmp_ssp.h"
#endif
extern t_link_entry link_container[PRH_MAX_ACL_LINKS];

t_slots LMsec_link_key_request_timeout = PRH_BS_CFG_SYS_LINK_KEY_REQUEST_TIMEOUT;
t_slots LMsec_pin_code_request_timeout = PRH_BS_CFG_SYS_PIN_CODE_REQUEST_TIMEOUT; 
boolean LMsec_pin_code_request_timeout_extend = FALSE;

#if (PRH_BS_CFG_SYS_PIN_CODE_REQ_EXTEND == 1 )
t_slots LMsec_pin_code_request_extended_timeout = PRH_BS_CFG_SYS_LINK_KEY_REQUEST_TIMEOUT +
												  PRH_BS_CFG_SYS_PIN_CODE_REQUEST_TIMEOUT;
#endif

/**************************************************************************
* FUNCTION: LMsec_upper_LM_Authentication_Requested
*
* INPUTS :- p_link
*
* ASSOCIATED HCI COMMAND :- HCI_Authentication_Requested
*
* DESCRIPTION :- Invoked by the host to authenticate a remote device 
* associated with p_link. Sets the host_auth_req_flag to ON and invokes
* the same authentication procedure that is used by the
* lmp_ch module.
*
*************************************************************************/

t_error LMsec_upper_LM_Authentication_Requested(t_lmp_link* p_link)
{ 
    t_error cmdStatus = NO_ERROR;
    
    if(!p_link)
        cmdStatus = INVALID_HCI_PARAMETERS;

    if(p_link->operation_pending == LMP_DETACH)
        cmdStatus = COMMAND_DISALLOWED;

    
    if(!LMsec_core_Auth_Allowed(&(p_link->bd_addr)))
        cmdStatus = REPEATED_ATTEMPTS;

#if 0 // (PRH_BS_CFG_SELECT_LEVEL != 5)  /* #3049 */
	if(p_link->encrypt_mode != 0)
        cmdStatus = COMMAND_DISALLOWED;
#endif

    if(NO_ERROR == cmdStatus)
    {
        if((SUB_STATE_IDLE == p_link->auth_sub_state) &&
            (SUPER_STATE_IDLE == p_link->auth_super_state))
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

            HCeg_Command_Status_Event(NO_ERROR, HCI_AUTHENTICATION_REQUESTED);
            /* Assume No Link Key and set super state = HCI_ORIG_PAIRING */
            p_link->auth_super_state = HCI_ORIG_PAIRING;  
            /* 
             * If a Link Key exists, start the Auth Challenge, 
             * otherwise wait for key from Host 
             * (appropriate sub state is set in _LM_Get_Link_Key() 
             */
            p_link->sec_current_proc_tid_role = (u_int8)p_link->role;
#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
            // Added by GF 20 May 2009 to be compatable with description of HCI_Authentication_Req when 
	        // SSP is enabled

			// NB :- This is actually recommended (but not required) behaviour for non SSP solutions also. As it allows us
			// to upgrade the security on a link (longer pin code) which has already been authenticated. MUST change non-ssp
			// to this later (lots of ATS changes required).

			if (LMssp_SSP_Enabled_On_Link(p_link))
			{
				if (W4_ENCR_MODE_REPLY != p_link->encr_sub_state)
					p_link->auth_sub_state = W4_LINK_KEY_REQ_REPLY;

				/* Generate Host Link Key Request Event */
				LMsec_upper_Send_HCI_Event(p_link,HCI_LINK_KEY_REQUEST_EVENT,NO_ERROR);
			}
			else
#endif
            if (LINK_KEY_EXISTS == LMsec_core_Get_Link_Key(p_link)) 
            {
                /* Link Key Exists => Set SuperState=IDLE */
                p_link->auth_super_state = HCI_AUTHENTICATION;
                LMsec_core_Auth_Challenge(p_link, (u_int8)p_link->role);
            } 
        }
        else 
            cmdStatus = COMMAND_DISALLOWED; 
    }
    
    if(NO_ERROR != cmdStatus)
        HCeg_Command_Status_Event(cmdStatus, HCI_AUTHENTICATION_REQUESTED);
    
    return NO_ERROR;
}



/**************************************************************************
* FUNCTION: LMsec_upper_LM_Change_Connection_Link_Key
*
* INPUTS:- p_link.
*
* ASSOCIATED HCI COMMAND :- HCI_Change_Connection_Link_Key
*
* DESCRIPTION:-
**************************************************************************/
t_error LMsec_upper_LM_Change_Connection_Link_Key(t_lmp_link* p_link)
{
    t_error cmdStatus = NO_ERROR;

    if(p_link->operation_pending == LMP_DETACH)
        cmdStatus = COMMAND_DISALLOWED;
#if (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED==1)   
    if(UNIT_KEY_DEVICE == g_LM_config_info.key_type)
        cmdStatus = UNIT_KEY_USED;
#endif
    if((TEMPORARY == LMacl_Get_Current_Key_Persistance(p_link)) || 
       (!p_link->link_key_exists))
    {
        cmdStatus = COMMAND_DISALLOWED;
    }
    
    if(NO_ERROR == cmdStatus)
    {
        if((SUPER_STATE_IDLE == p_link->auth_super_state) && 
            ((SUB_STATE_IDLE == p_link->auth_sub_state)||
            (W4_SRES == p_link->auth_sub_state )))
        {
            HCeg_Command_Status_Event(NO_ERROR, HCI_CHANGE_CONNECTION_LINK_KEY);
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
            p_link->auth_super_state = ORIG_CHANGE_COMBKEY;
            p_link->auth_sub_state = W4_COMB_KEY_RESPONSE;
            p_link->sec_current_proc_tid_role = (u_int8)p_link->role;
            LMsec_core_Init_CombKey_Setup(p_link);
        }
        else
            cmdStatus = UNSPECIFIED_ERROR;
    }
    
    if(NO_ERROR != cmdStatus)
        HCeg_Command_Status_Event(cmdStatus, HCI_CHANGE_CONNECTION_LINK_KEY);
    
    return NO_ERROR;
}

#if (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED==1)
/******************************************************************************
* FUNCTION: LMsec_upper_LM_Create_New_Unit_Key
*
* INPUTS :- 
*
* ASSOCIATED HCI COMMAND :- HCI_Create_New_Unit_Key.
*
* DESCRIPTION :- Calls the lmp security engine to create a new unit key for the
* device and stores this locally to the lmp security module.
*
******************************************************************************/
t_error LMsec_upper_LM_Create_New_Unit_Key()
{
    t_rand rand;
    const t_bd_addr *p_local_bd_addr;
    u_int8 new_unit_key[16];
    
    SYSrand_Get_Rand_128_Ex(rand);
    p_local_bd_addr =  SYSconfig_Get_Local_BD_Addr_Ref();
    LM_SecEng_Generate_Key_E21(rand, p_local_bd_addr, new_unit_key);
    SYSconfig_Set_Unit_Key(new_unit_key);
    
    return NO_ERROR;
}
#endif

/******************************************************************************
* FUNCTION :- LMsec_upper_LM_Link_Key_Request_Negative_Reply
*
* INPUTS :- p_bd_addr
*           p_cmd_complete_info
*
* ASSOCIATED HCI COMMAND :- HCI_Link_Key_Request_Negative_Reply
*
* DESCRIPTION :- Invoked by the host when no link key was found for a peer 
*                device. The t_lmp_link for the peer must be in sub state 
*                W4_LINK_KEY_REQ_REPLY. The super state of the t_lmp_link 
*                is checked:
*     
*       SUPER_STATE_IDLE: 
*       Local device has been challenged by peer.
*       An LMP_not_accepted is sent to the peer.
*
*       ORIG_PAIRING_PENDING:
*       Local device is initiating authentication using the link key.
*       Since no link key exists, Pairing is started in order to get 
*       a PIN from the host.
*
*       AUTH_CHALLENGE_PENDING:
*       Local device has initiated authentication and has also received 
*       challenge from the peer while waiting for the link key from host. 
*       In this case, the peer has a link key but the local device does
*       not. The local device changes state and starts Pairing.      
*                         
**************************************************************************/ 
t_error LMsec_upper_LM_Link_Key_Request_Negative_Reply(t_bd_addr* p_bd_addr, 
    t_cmd_complete_event* p_cmd_complete_info)
{
    t_lmp_link* p_link;
#if (PRH_BS_CFG_SYS_PIN_CODE_REQ_EXTEND == 1)
	t_slots unextended_pin_code_request_timeout = LMsec_pin_code_request_timeout; /* 2778 */
#endif    
    p_link = LMaclctr_Find_Bd_Addr(p_bd_addr);
    p_cmd_complete_info->p_bd_addr = p_bd_addr;
    
    if(p_link)
    {
        if ((p_link->auth_sub_state == W4_LINK_KEY_REQ_REPLY) || 
            (p_link->auth_sub_state == W4_LINK_KEY_REQ_REPLY_CHALL_PENDING))
        {
            p_cmd_complete_info->status = NO_ERROR;

            if (p_link->gen_security_timeout_index)
            {
#if (PRH_BS_CFG_SYS_PIN_CODE_REQ_EXTEND == 1)
                if (LMsec_pin_code_request_timeout_extend)
                {
                    /*
                     * Add the remaining of link key request timeout to
                     * calculate maximum pin code request timeout possible.
                     */
                    LMsec_pin_code_request_timeout = PRH_LMP_MSG_TIMEOUT
                        - LMsec_link_key_request_timeout - 500
                        + LMtmr_Get_Residual_Slots(p_link->gen_security_timeout_index);
                }
#endif
                LMtmr_Clear_Timer(p_link->gen_security_timeout_index);
                p_link->gen_security_timeout_index = 0;
            }
        }
        else
        {
            p_cmd_complete_info->status = COMMAND_DISALLOWED;
        }
        HCeg_Command_Complete_Event(p_cmd_complete_info);
        
		if(ORIG_PAIRING_PROC_ON(p_link->auth_super_state))
		{
#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)
			if ((p_link->setup_complete_bit_mask == LMP_Both_Setup_Completes) &&
				LMssp_SSP_Enabled_On_Link(p_link))
			{
				if ((p_link->auth_sub_state == W4_LINK_KEY_REQ_REPLY_CHALL_PENDING) ||
					(p_link->auth_sub_state == W4_LINK_KEY_REQ_REPLY))
				{
					// 
					// Initiate Simple Pairing 
					t_lm_event_info event_info;

					// Need to clear any timers associated with Pre-2.1 security

					if (p_link->sec_timer)
					{
						LMtmr_Clear_Timer(p_link->sec_timer);
						p_link->sec_timer = 0;
					}
					event_info.p_bd_addr = &p_link->bd_addr;
					HCeg_Generate_Event(HCI_IO_CAPABILITY_REQUEST_EVENT,&event_info);
					p_link->ssp_hci_timer_index = LMtmr_Set_Timer(PRH_LMP_MSG_TIMEOUT,LMssp_HCI_Timeout,p_link,1);

					p_link->auth_sub_state = W4_HCI_IO_CAP_REQUEST_REPLY;
					p_link->ssp_initiator = 0x01;
				}
			}
			else
#endif
			{
				switch(p_link->auth_sub_state)
				{
				case W4_LINK_KEY_REQ_REPLY_CHALL_PENDING:
					LMsec_peer_Send_LMP_Not_Accepted(p_link, LMP_AU_RAND, 
						(u_int8)(!(p_link->role)), PIN_MISSING);
					/* Intentional Fall Through */
				case W4_LINK_KEY_REQ_REPLY:
					p_link->auth_sub_state = W4_PIN_CODE_REQ_REPLY;
					LMsec_upper_Send_HCI_Event(p_link, HCI_PIN_CODE_REQUEST_EVENT, 
						NO_ERROR);
					break;
				}
			}
		}
		else if (W4_LINK_KEY_REQ_REPLY == p_link->auth_sub_state &&
			SUPER_STATE_IDLE == p_link->auth_super_state)
		{
			p_link->auth_sub_state = SUB_STATE_IDLE;
			LMsec_peer_Send_LMP_Not_Accepted(p_link, LMP_AU_RAND, 
				(u_int8)(!(p_link->role)), PIN_MISSING);
		}

#if (PRH_BS_CFG_SYS_PIN_CODE_REQ_EXTEND == 1) 
		/*
		* Previous (unextended) pin code request timeout can be restored after
		* timeout setup from pin code request event.
		* #2778.
		*/
		LMsec_pin_code_request_timeout = unextended_pin_code_request_timeout;
#endif

	}
	else
	{
		p_cmd_complete_info->status = COMMAND_DISALLOWED;
		HCeg_Command_Complete_Event(p_cmd_complete_info);
	} 
	return NO_ERROR;    
}


/**************************************************************************
* FUNCTION :- LMsec_upper_LM_Link_Key_Request_Timeout
*
* INPUTS :- p_link
*
*                              
**************************************************************************/ 
void LMsec_upper_LM_Link_Key_Request_Timeout(t_lmp_link* p_link)
{
    if ((p_link) && 
        ((p_link->auth_sub_state == W4_LINK_KEY_REQ_REPLY) ||
        (p_link->auth_sub_state == W4_LINK_KEY_REQ_REPLY_CHALL_PENDING)))
    {
        if(ORIG_PAIRING_PROC_ON(p_link->auth_super_state))
        {
            switch(p_link->auth_sub_state)
            {
            case W4_LINK_KEY_REQ_REPLY_CHALL_PENDING:
                LMsec_peer_Send_LMP_Not_Accepted(p_link, LMP_AU_RAND, 
                    (u_int8)(!(p_link->role)), PIN_MISSING);
                /* Intentional Fall Through */
            case W4_LINK_KEY_REQ_REPLY:
                p_link->auth_sub_state = W4_PIN_CODE_REQ_REPLY;
                LMsec_upper_Send_HCI_Event(p_link, HCI_PIN_CODE_REQUEST_EVENT, 
                    NO_ERROR);
                break;
            }
        }
        else if ((W4_LINK_KEY_REQ_REPLY == p_link->auth_sub_state) &&
            (SUPER_STATE_IDLE == p_link->auth_super_state))
        {
            p_link->auth_sub_state = SUB_STATE_IDLE;
            LMsec_peer_Send_LMP_Not_Accepted(p_link, LMP_AU_RAND, 
                (u_int8)(!(p_link->role)), PIN_MISSING);
        }
    }
}


/**************************************************************************
* FUNCTION :- LMsec_upper_LM_Link_Key_Request_Reply
*
* INPUTS :- p_bd_addr
*           link_key
*           p_cmd_complete_info
*
* ASSOCIATED HCI COMMAND :- HCI_Link_Key_Request_Reply
*
* DESCRIPTION :- Invoked by the host in reponse to a previous 
* HCI_Link_Key_Request_Event. The t_lmp_link must therefore in a sub state
* of W4_LINK_KEY_REQ_REPLY for this function to process the command. The new
* link key is stored in t_lmp_link for the peer device and is also stored in
* the link key database. The super state of the t_lmp_link is then checked:
*
*         ORIG_PAIRING_PENDING: Local device is initiating authentication using
*                               the link key. A Challenge(LMP_au_rand) is sent
*                               to the peer.
* 
*       AUTH_CHALLENGE_PENDING: Local device has initiated authentication and
*                               also received challenge from the peer while
*                               waiting for the link key from host. A 
*                               response (LMP_sres) and a challenge
*                               (LMP_au_rand) is sent to the peer.
*
*             SUPER_STATE_IDLE: Local device has been challenged by peer.
*                               A response (LMP_sres) is sent to the peer.
*
*****************************************************************************/ 
t_error LMsec_upper_LM_Link_Key_Request_Reply(t_bd_addr* p_bd_addr,
    t_link_key link_key, t_cmd_complete_event* p_cmd_complete_info)
{
    t_lmp_link* p_link;
    
    p_link = LMaclctr_Find_Bd_Addr(p_bd_addr);
    p_cmd_complete_info->p_bd_addr = p_bd_addr;
    p_cmd_complete_info->status = NO_ERROR;
    
    
    if((p_link) && 
      ((p_link->auth_sub_state == W4_LINK_KEY_REQ_REPLY_CHALL_PENDING) || 
      (p_link->auth_sub_state == W4_LINK_KEY_REQ_REPLY)))
    {
        HCeg_Command_Complete_Event(p_cmd_complete_info);
#if (PRH_BS_CFG_SYS_PIN_CODE_REQ_EXTEND == 1) /* 2778 */
        if (LMsec_pin_code_request_timeout_extend)
           {
             /*
              * Add the remaining of link key request timeout to
              * calculate maximum pin code request timeout possible.
              */
            LMsec_pin_code_request_extended_timeout = PRH_LMP_MSG_TIMEOUT
                - LMsec_link_key_request_timeout - 500
                + LMtmr_Get_Residual_Slots(p_link->gen_security_timeout_index);
           }
#endif  
        LMtmr_Clear_Timer(p_link->gen_security_timeout_index);
        p_link->gen_security_timeout_index = 0;
        
        /* Check for previous authentication failures */
        if(!LMsec_core_Auth_Allowed(&(p_link->bd_addr)))
        {
            if(ORIG_PAIRING_PROC_ON(p_link->auth_super_state))
            {
                switch(p_link->auth_sub_state)
                { 
                case W4_LINK_KEY_REQ_REPLY_CHALL_PENDING:
                    LMsec_peer_Send_LMP_Not_Accepted(p_link, LMP_AU_RAND, 
                        (u_int8)(!(p_link->role)), REPEATED_ATTEMPTS);
                    /* Intentional Fall Through */
                case W4_LINK_KEY_REQ_REPLY:                  
                    SET_SUPER_STATE_PROC(p_link->auth_super_state, 
                        AUTHENTICATION_PROC);
                    p_link->auth_sub_state = SUB_STATE_IDLE;
                    LMsec_core_Auth_Complete(p_link, REPEATED_ATTEMPTS);
                    break;          
                    
                default:
                    break;
                }
            }
            else if((SUPER_STATE_IDLE == p_link->auth_super_state) &&
                (W4_LINK_KEY_REQ_REPLY == p_link->auth_sub_state))
            {            
                p_link->auth_sub_state = SUB_STATE_IDLE;
                LMsec_peer_Send_LMP_Not_Accepted(p_link, LMP_AU_RAND, 
                    (u_int8)(!(p_link->role)), REPEATED_ATTEMPTS);
            }
            
            return NO_ERROR;
        }       
        if(ORIG_PAIRING_PROC_ON(p_link->auth_super_state))
        {
            switch(p_link->auth_sub_state)
            { 
            case W4_LINK_KEY_REQ_REPLY_CHALL_PENDING:
                LMutils_Array_Copy(LINK_KEY_SIZE, p_link->link_key, link_key);
                if(MASTER == p_link->role)
                {
                    /* 
                     * For interoperation with Digianswer, send 
                     * LMP_not_accepted "after" sending the challenge 
                     */
                    SET_SUPER_STATE_PROC(p_link->auth_super_state, 
                        AUTHENTICATION_PROC);
                    LMsec_core_Auth_Challenge(p_link, (u_int8)p_link->role);
                    LMsec_peer_Send_LMP_Not_Accepted(p_link, LMP_AU_RAND, 
                        (u_int8)(!(p_link->role)), 
                        LMP_ERROR_TRANSACTION_COLLISION);
                    break;
                }
                else /* SLAVE */
                {
                    LMsec_core_Auth_Response(p_link, 
                        (u_int8)(!(p_link->role)), p_link->rand);  
                    SET_SUPER_STATE_PROC(p_link->auth_super_state, 
                        AUTHENTICATION_PROC);
                    LMsec_core_Auth_Challenge(p_link, (u_int8)p_link->role);
                }
                break;
                
            case W4_LINK_KEY_REQ_REPLY:
                LMutils_Array_Copy(LINK_KEY_SIZE, p_link->link_key, link_key);
                SET_SUPER_STATE_PROC(p_link->auth_super_state, 
                    AUTHENTICATION_PROC);
                LMsec_core_Auth_Challenge(p_link, (u_int8)p_link->role);
                break;          
                
            default:
                break;
            }
        }
        else if( (SUPER_STATE_IDLE == p_link->auth_super_state &&
            W4_LINK_KEY_REQ_REPLY == p_link->auth_sub_state ) ||
            W4_ENCR_MODE_REPLY == p_link->auth_sub_state)
        {
            LMutils_Array_Copy(LINK_KEY_SIZE, p_link->link_key, link_key);
            if(W4_LINK_KEY_REQ_REPLY == p_link->auth_sub_state)
                p_link->auth_sub_state = SUB_STATE_IDLE;
            LMsec_core_Auth_Response(p_link, (u_int8)(!(p_link->role)), 
                p_link->rand);      
        }
    }
    else
    {
        p_cmd_complete_info->status = COMMAND_DISALLOWED;
        HCeg_Command_Complete_Event(p_cmd_complete_info);
    }
    
    return p_cmd_complete_info->status;
}


/******************************************************************************
 * FUNCTION :- LMsec_upper_LM_Master_Link_Key
 *
 * INPUTS :- key_flag : SEMI_PERMANENT or TEMPORARY
 *   
 * DESCRIPTION :- Invoked by the host to force the device (which is Master) to
 * use the temporary link key of the Master device (key_flag = TEMPORARY) or 
 * the semi-permanent link keys (key_flag = SEMI_PERMANENT). 
 * TEMPORARY: 
 *    Step 1 - Generate two random numbers and pass to E22 to generate 
 *             Master Key, which is stored 
 *    Step 2 - Generate a third random number (temp_rand) and send it to 
 *             the Slave. 
 *    Step 3 - Pass temp_rand and current link key to E22 to generate overlay.
 *    Step 4 - Send temp_key (= overlay XOR Master Key) to slave
 *    Step 5 - If Encryption is enabled, restart, otherwise send 
 *             HCI_Master_Link_Key_Complete_Event
 * SEMI_PERMANENT:
 *    Step 1 - Sub State= W4_USE_SEMI_PERM_REPLY 
 *    Step 2 - Send LMP_use_semi_permanent_key()
 *   
 *****************************************************************************/
t_rand temp_rand;
t_error LMsec_upper_LM_Master_Link_Key(t_link_key_life key_flag)
{
    t_rand rand1, rand2;
    t_link_key master_key;
    t_link_key_life new_key_life;
    t_lmp_link *p_link;
    u_int8 num_links = 0;
    int i;
    
    extern t_link_entry link_container[PRH_MAX_ACL_LINKS];
    
    /* Should only issued for the Master device */
    new_key_life = key_flag;

   /*
    * Generate the master key (temporary link key) and temp_rand once 
    * for all active links
    */
    if(TEMPORARY == key_flag)
    {
        SYSrand_Get_Rand_128_Ex(rand1);            
        SYSrand_Get_Rand_128_Ex(rand2);     
        
       /*
        * Pass Null bd_addr to E22 since rand2 length is 16 bytes
        *  => no Augmenting will be required
        */
        LM_SecEng_Generate_Key_E22(rand1, rand2, RANDOM_NUM_SIZE, 0,master_key);
        LMutils_Array_Copy(LINK_KEY_SIZE, g_LM_config_info.master_key , 
            master_key);
        SYSrand_Get_Rand_128_Ex(temp_rand);
        SYSrand_Get_Rand_128_Ex(g_LM_config_info.bcast_encr_rand);
    }

    for(i=0; i<PRH_MAX_ACL_LINKS; i++)
    { 
        /* 
         * Cycle through the link_container to find all the 
         * active links on which the device is master AND
         * where nothing else is happening on the security end
         * of things AND where
         *
         * The link is not in a condition
         *  1) trying to swap over to a broadcast key
         *     where it will be impossible to negotiate
         *     a broadcast key length)
         */
        p_link = &(link_container[i].entry);
        if( ((LMP_ACTIVE & p_link->state) || (LMP_SNIFF_MODE & p_link->state)) && 
            (MASTER == p_link->role) && 
            (SUB_STATE_IDLE == p_link->auth_sub_state) &&
            (SUPER_STATE_IDLE == p_link->auth_super_state)) /* end if*/
        {
            num_links++;

            /*
             * generate a successful command status event if found at least one
             * relevant link
             */
            if(num_links==1)
                HCeg_Command_Status_Event(NO_ERROR,HCI_MASTER_LINK_KEY);

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

            /*
             *    Cur Key : New Key: Encr: Cover: Test
             * 1. TEMP      TEMP     YES   Y      TP_PRH_SEC_MLK_BV06C
             * 2. TEMP      TEMP     NO    Y      TP_PRH_SEC_MLK_BV02C
             * 3. TEMP      SP       YES   Y      AUT/BV20C
             * 4. TEMP      SP       NO    Y      AUT/BV19C
             * 5. SP        TEMP     YES   Y      TP_PRH_BV_BV06C
             * 6. SP        TEMP     NO    Y      AUT/BV18C, AUT/BV19C
             * 7. SP        SP       YES   Y      TP_PRH_SEC_MLK_BV06C
             * 8. SP        SP       NO    Y      TP_PRH_SEC_MLK_BV01C
             */

            if((SEMI_PERMANENT == key_flag) && 
               (SEMI_PERMANENT == LMacl_Get_Current_Key_Persistance(p_link)))
            {
                LMtmr_Clear_Timer(p_link->sec_timer);
                p_link->sec_timer = 0;
                LMsec_upper_Send_HCI_Event(p_link, 
                    HCI_MASTER_LINK_KEY_COMPLETE_EVENT, NO_ERROR);
            }

            else if(TEMPORARY == key_flag)
            {
                LMsec_core_Change_To_Temporary_Key(p_link);
            }

            else
            {
                LMsec_core_Change_To_Semi_Permanent_Key(p_link);
            }
        }
    }

    /*
     * generate an unsuccessful command status event if found no
     * relevant link
     */
    if(num_links==0)
        HCeg_Command_Status_Event(COMMAND_DISALLOWED, HCI_MASTER_LINK_KEY);
    
    return NO_ERROR;
}


/******************************************************************************
* FUNCTION: LMsec_upper_LM_Pin_Code_Request_Negative_Reply
*
*
* Steps :
*     0/ Check State
*     1/ Change Super_state -> IDLE
*     2/ Change Sub_State -> IDLE
*     3/ Generate_Command_Complete_Event
*     4/ Call Authentication Complete ( outcome = FAILURE )
*
******************************************************************************/ 
t_error LMsec_upper_LM_Pin_Code_Request_Negative_Reply(t_bd_addr* p_bd_addr, 
    t_cmd_complete_event* p_cmd_complete_info)
{
    t_lmp_link* p_link;
    
    p_link = LMaclctr_Find_Bd_Addr(p_bd_addr);
    p_cmd_complete_info->p_bd_addr = p_bd_addr;
    
    if( (p_link != 0) && (W4_PIN_CODE_REQ_REPLY == p_link->auth_sub_state) )
    {
        p_cmd_complete_info->status = NO_ERROR;
        HCeg_Command_Complete_Event(p_cmd_complete_info);
        LMtmr_Clear_Timer(p_link->gen_security_timeout_index);
        p_link->gen_security_timeout_index = 0;
        if(ORIG_PAIRING_PROC_ON(p_link->auth_super_state))
        {
            p_link->auth_sub_state = SUB_STATE_IDLE;
            LMsec_core_Auth_Complete(p_link, PAIRING_NOT_ALLOWED);
        }
        else if(TERM_PAIRING_PROC_ON(p_link->auth_super_state))
        {
            p_link->auth_sub_state = SUB_STATE_IDLE;
#if (PRH_BS_DEV_SECURITY_MODIFIED_ERROR_CODES == 1)
			/* #2779: Scenario 1 */
            LMsec_peer_Send_LMP_Not_Accepted(p_link, LMP_IN_RAND, 
                (u_int8)(!(p_link->role)), PIN_MISSING);
            LMsec_core_Auth_Complete(p_link, PIN_MISSING);
#else
			LMsec_peer_Send_LMP_Not_Accepted(p_link, LMP_IN_RAND, 
                (u_int8)(!(p_link->role)), PAIRING_NOT_ALLOWED);
            LMsec_core_Auth_Complete(p_link, PAIRING_NOT_ALLOWED);
#endif
        }
    }
    else
    {
        p_cmd_complete_info->status = COMMAND_DISALLOWED;
        HCeg_Command_Complete_Event(p_cmd_complete_info);
    }
    
    return p_cmd_complete_info->status;
}


/******************************************************************************
* FUNCTION: LMsec_upper_LM_Pin_Code_Request_Timeout
*
******************************************************************************/ 
void LMsec_upper_LM_Pin_Code_Request_Timeout(t_lmp_link* p_link)
{
    if( (p_link != 0) && (W4_PIN_CODE_REQ_REPLY == p_link->auth_sub_state) )
    {
        if(ORIG_PAIRING_PROC_ON(p_link->auth_super_state))
        {
            p_link->auth_sub_state = SUB_STATE_IDLE;
#if (PRH_BS_DEV_SECURITY_MODIFIED_ERROR_CODES ==1)
			/* #2779: Scenario 3 */
            LMsec_core_Auth_Complete(p_link, PAIRING_NOT_ALLOWED);
#else
            LMsec_core_Auth_Complete(p_link, PIN_MISSING);
#endif
        }
        else if(TERM_PAIRING_PROC_ON(p_link->auth_super_state))
        {
            p_link->auth_sub_state = SUB_STATE_IDLE;
#if (PRH_BS_DEV_SECURITY_MODIFIED_ERROR_CODES ==1)
			/* #2779: Scenario 2 */
            LMsec_peer_Send_LMP_Not_Accepted(p_link, LMP_IN_RAND, 
                (u_int8)(!(p_link->role)), LMP_RESPONSE_TIMEOUT);
            LMsec_core_Auth_Complete(p_link, LMP_RESPONSE_TIMEOUT);         
#else
            LMsec_peer_Send_LMP_Not_Accepted(p_link, LMP_IN_RAND, 
                (u_int8)(!(p_link->role)), PIN_MISSING);
            LMsec_core_Auth_Complete(p_link, PIN_MISSING);         

#endif
        }
    }
}


/******************************************************************************
* FUNCTION: LMsec_upper_LM_Pin_Code_Request_Reply
*
* Steps :
*     0/ Check Sub State
*     1/ Store the Pin Code
*     2/ Generate Command_Complete_Event ! 
*       [ Optionally locate functionality at HC ]
*  Originating Side
*     3/ Generate Random Num
*     4/ Send LMP_IN_RAND
*     5/ Modify Sub_State  --> W4_IN_RAND_RESP
*  Terminating Side
*     3/ Retrieve original RAND from link structure
*     4/ Perform E3
*     5/ Send LMP_Accepted for LMP_IN_RAND
*     6/ Change Sub_State to Idle.
******************************************************************************/ 
t_error LMsec_upper_LM_Pin_Code_Request_Reply(t_bd_addr* p_claimant_bd_addr,
    t_pin_code pin_code, u_int8 pin_code_length, 
    t_cmd_complete_event* p_cmd_complete_info)
{
    t_link_key key_init;
    t_rand in_rand;
    t_lmp_link* p_link;
    
    p_link = LMaclctr_Find_Bd_Addr(p_claimant_bd_addr);
    p_cmd_complete_info->p_bd_addr = p_claimant_bd_addr;
    if(!p_link)
    {
        p_cmd_complete_info->status = INVALID_HCI_PARAMETERS;
        HCeg_Command_Complete_Event(p_cmd_complete_info);
    }
    else if(W4_PIN_CODE_REQ_REPLY == p_link->auth_sub_state)
    {
        LMtmr_Clear_Timer(p_link->gen_security_timeout_index);
        p_link->gen_security_timeout_index = 0;

        if((pin_code_length < 17) && (pin_code_length > 0))
        {
            p_cmd_complete_info->status = NO_ERROR;
            HCeg_Command_Complete_Event(p_cmd_complete_info);
            LMutils_Array_Copy(pin_code_length, p_link->pin_code, pin_code);
            p_link->pin_code_length = pin_code_length;
        
            if(ORIG_PAIRING_PROC_ON(p_link->auth_super_state) || 
                (TERM_PAIRING_PROC_ON(p_link->auth_super_state) &&
                FIXED == g_LM_config_info.pin_type))
            {
                SYSrand_Get_Rand_128_Ex(in_rand);
                LMutils_Array_Copy(RANDOM_NUM_SIZE, p_link->rand, in_rand);
                /* use current sec procedure tid role */
                LMsec_peer_Send_LMP_Message(p_link, LMP_IN_RAND,  
                    p_link->sec_current_proc_tid_role, in_rand);
                p_link->auth_sub_state = W4_IN_RAND_RESPONSE;
            }
            else if(TERM_PAIRING_PROC_ON(p_link->auth_super_state) &&
                VARIABLE == g_LM_config_info.pin_type )
            {
                LM_SecEng_Generate_Key_E22(p_link->rand, pin_code, 
                    pin_code_length, SYSconfig_Get_Local_BD_Addr_Ref(), 
                    key_init);
                LMutils_Array_Copy(LINK_KEY_SIZE, p_link->link_key, key_init);
                /* 
                 * If peer 1.0b device trys to authenticate key_init, 
                 * 'link_key_exists' must be set to TRUE 
                 */
                p_link->link_key_exists = TRUE;
                LMsec_peer_Send_LMP_Accepted(p_link, LMP_IN_RAND, 
                    p_link->sec_current_proc_tid_role);
                p_link->auth_sub_state = W4_PEER_LINKKEY;
            }
        }
        else
        {
            p_cmd_complete_info->status = INVALID_HCI_PARAMETERS;
            HCeg_Command_Complete_Event(p_cmd_complete_info);
            if(ORIG_PAIRING_PROC_ON(p_link->auth_super_state))
            {
                p_link->auth_sub_state = SUB_STATE_IDLE;
                LMsec_core_Auth_Complete(p_link, PIN_MISSING);
            }
            else if(TERM_PAIRING_PROC_ON(p_link->auth_super_state))
            {
                p_link->auth_sub_state = SUB_STATE_IDLE;
                LMsec_peer_Send_LMP_Not_Accepted(p_link, LMP_IN_RAND, 
                    (u_int8)(!(p_link->role)), PIN_MISSING);
                LMsec_core_Auth_Complete(p_link, PIN_MISSING);
            }
        }
    }
    else
    {
        p_cmd_complete_info->status = COMMAND_DISALLOWED;
        HCeg_Command_Complete_Event(p_cmd_complete_info);
    }
    return p_cmd_complete_info->status;
}


/*****************************************************************************
 * FUNCTION: LMsec_upper_LM_Set_Connection_Encryption
 *
 * INPUTS :- 
 *
 * ASSOCIATED HCI COMMAND :- HCI_Set_Connection_Encryption.
 *
 * DESCRIPTION :- Called via the HCI to enable or disable 
 *     encryption of the link. 
 *
 ******************************************************************************/
t_error LMsec_upper_LM_Set_Connection_Encryption(t_lmp_link *p_link, 
    t_flag encrypt_enable)
{
#if 0 // Moved to HC_Cmd_Disp.c
    if(p_link->operation_pending == LMP_DETACH)
        return COMMAND_DISALLOWED;
    
    //
    //if(p_link->encrypt_enable == encrypt_enable)
    //    return INVALID_HCI_PARAMETERS;

    if(!p_link->link_key_exists)
        return COMMAND_DISALLOWED;
#endif
    /*
     * If asked to encrypt and there is no broadcast key size available
     * (i.e. not even one slave device can be encrypted with, then
     * reject the set connection encryption command.
     */
    if((MASTER==p_link->role) && 
       (0 ==LMsec_core_Is_Broadcast_Key_Length_Possible() ) && 
       (LMacl_Get_Current_Key_Persistance(p_link) == TEMPORARY))
        return COMMAND_DISALLOWED;

    /*
     * Start a timer to guard the security
     * transaction.
     */
     if(!p_link->sec_timer)
        p_link->sec_timer = LMtmr_Set_Timer(PRH_DEFAULT_LMP_TRANSACTION_TIMEOUT,
            LMsec_core_TimeoutHandler,
            p_link, /* current device link */
            1 /* dynamic */);

    p_link->sec_current_proc_tid_role = (u_int8)p_link->role;
    LMsec_core_Set_Encryption(p_link, encrypt_enable);
    
    return NO_ERROR;
}


/******************************************************************************
* FUNCTION: LMsec_uppper_Send_HCI_Event
*
******************************************************************************/
void LMsec_upper_Send_HCI_Event(t_lmp_link *p_link, u_int8 hci_event, 
    t_error status)
{
    t_lm_event_info event_info;
    
    switch(hci_event)
    { 
    case HCI_ENCRYPTION_CHANGE_EVENT:
    case HCI_MASTER_LINK_KEY_COMPLETE_EVENT:
        if(HCI_ENCRYPTION_CHANGE_EVENT == hci_event)
            event_info.mode = p_link->encrypt_enable;
        else
            event_info.mode = LMacl_Get_Current_Key_Persistance(p_link); 
    /* Intentional Fall Through */
        
    case HCI_AUTHENTICATION_COMPLETE_EVENT:
    case HCI_CHANGE_CONNECTION_LINK_KEY_COMPLETE_EVENT:
        event_info.handle = p_link->handle;
        event_info.status = status;
        break;
        
    case HCI_LINK_KEY_NOTIFICATION_EVENT:
        event_info.p_u_int8 =  p_link->link_key; 
        event_info.key_type = p_link->key_type;
        event_info.p_bd_addr = &(p_link->bd_addr);
        break;
        
    case HCI_LINK_KEY_REQUEST_EVENT:
        event_info.p_bd_addr = &(p_link->bd_addr);
        if (p_link->gen_security_timeout_index)
            LMtmr_Clear_Timer(p_link->gen_security_timeout_index);
        p_link->gen_security_timeout_index = LMtmr_Set_Timer(
            LMsec_link_key_request_timeout,
            &LMsec_upper_LM_Link_Key_Request_Timeout, p_link,1);
        break;
        
    case HCI_PIN_CODE_REQUEST_EVENT:
        event_info.p_bd_addr = &(p_link->bd_addr);
        if (p_link->gen_security_timeout_index)
            LMtmr_Clear_Timer(p_link->gen_security_timeout_index);
        p_link->gen_security_timeout_index = LMtmr_Set_Timer(
            LMsec_pin_code_request_timeout,
            &LMsec_upper_LM_Pin_Code_Request_Timeout, p_link,1);
        break;
        
    default:
        break;
        
    }
    
    HCeg_Generate_Event(hci_event, &event_info);
}

