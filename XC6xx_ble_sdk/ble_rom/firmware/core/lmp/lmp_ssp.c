/**************************************************************************
 * MODULE NAME:    lmp_ssp.c       
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    LMP Secure Simple Pairing Protocol
 * AUTHOR:         Gary Fleming
 * DATE:           02 Feb 2009
 *
 * SOURCE CONTROL: $Id: lmp_ssp.c,v 1.22 2012/12/06 19:48:53 garyf Exp $
 *
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2009 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *
 * NOTES:
 * This module encompasses the protocol exchange to support Secure Simple Pairing
 * 
 * This module encompasses the protocol only and depends on the cryptographic functions
 * which are defined in lmp_ssp_engine.c and lmp_ecc.c
 *
 **************************************************************************/

#include "sys_config.h"

#include "hc_event_gen.h"
#include "hc_const.h"

#include "lmp_const.h"
#include "lmp_acl_connection.h"
#include "lmp_acl_container.h"
#include "lmp_ch.h"
#include "lmp_config.h"
#include "lmp_debug.h"
#include "lmp_encode_pdu.h"
#include "lmp_features.h"
#include "lmp_timer.h"
#include "lmp_ssp_engine.h"
#include "sys_mmi.h"
#include "sys_rand_num_gen.h"
#include "tra_queue.h"
#include "lmp_sec_core.h"
#include "lmp_ecc.h"
#include "lmp_ssp.h"
#include "lmp_sec_upper.h"

#if (PRH_BS_CFG_SYS_LMP_SECURE_SIMPLE_PAIRING_SUPPORTED==1)

#define OUT_OF_BAND          0x01
#define NUMERIC_COMPARISON   0x02
#define PASSKEY_ENTRY        0x03

#define DisplayOnly       0x00
#define DisplayYesNo      0x01
#define KeyboardOnly      0x02
#define NoInputNoOutput   0x03

#define SSP_PEER          0x01
#define SSP_LOCAL         0x02

u_int8 Zero_Array[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

void LMssp_Send_LMP_Accepted(t_lmp_link* p_link, u_int8 opcode, 
    u_int8 tid_role);
void LMssp_Send_LMP_Not_Accepted(t_lmp_link* p_link, u_int8 opcode,
								 u_int8 tid_role, t_error reason);
t_error _LMssp_Init_DHKey_Calculation(t_lmp_link* p_link);
void _LMssp_Determine_Auth_Type(t_lmp_link* p_link);
static boolean _Compare_16byte_Arrays(u_int8* array1, u_int8* array2);

static t_error _LMssp_Send_LMP_IO_Capability_Request(t_lmp_link* p_link,
											 u_int8 io_cap,
											 u_int8 oob_data_present,
											 u_int8 auth_requirements);
static t_error _LMssp_Send_LMP_IO_Capability_Response(t_lmp_link* p_link,
											 u_int8 io_cap,
											 u_int8 oob_data_present,
											 u_int8 auth_requirements);
static t_error _LMspp_Send_LMP_Encapsulated_Header(t_lmp_link* p_link,t_role tid_role);
static t_error _LMssp_Send_LMP_Numeric_Comparison_Failed(t_lmp_link* p_link);
static t_error _LMssp_Send_LMP_DH_Key_Check(t_lmp_link* p_link);
static t_error LMssp_Send_LMP_Simple_Pairing_Confirm(t_lmp_link* p_link,u_int8* commitment);
#if (PRH_BS_CFG_SYS_SSP_PASSKEY_SUPPORTED==1)
static t_error _LMssp_Send_LMP_Passkey_Entry_Failed(t_lmp_link* p_link);
#endif
static t_error _LMssp_Calculate_Link_Key(t_lmp_link* p_link);
static void _LMssp_Mutual_Authentication(t_lmp_link* p_link);
static void _LMssp_Send_Simple_Pairing_Complete_Event(t_lmp_link* p_link,
													  t_error reason);
static t_error LMssp_Send_LMP_Simple_Pairing_Number(t_lmp_link* p_link);
static void LMssp_Calculate_DHKEY_Check(t_lmp_link* p_link,u_int8 side);
extern u_int8 BasePoint_x[];
extern u_int8 BasePoint_y[];

extern u_int8 DebugPublicKey_x[];
extern u_int8 DebugPublicKey_y[];
extern u_int8 DebugSecretKey[];

u_int8 LMssp_SSP_Enabled_On_Link(t_lmp_link* p_link)
{
	if((g_LM_config_info.ssp_enabled) && mFeat_Check_Secure_Simple_Pairing(p_link->remote_features) &&
	    mFeat_Check_Encapsulated_PDU(p_link->remote_features) && (p_link->ssp_host_support==0x01))
		return 1;
	else
		return 0;
}

t_error LMssp_LM_Write_SSP_Debug_Mode(u_int8 mode)
{
	if (mode == 0x00)
	{
		boolean blocking = TRUE;
		LMssp_Generate_New_Public_Private_Key_Pair(blocking);
		g_LM_config_info.ssp_debug_mode = 0;
	}
	else
	{
		int i;

		g_LM_config_info.ssp_debug_mode = 1;

		for (i=0;i<24;i++)
		{
			g_LM_config_info.public_key_x[i] = DebugPublicKey_x[i];
			g_LM_config_info.public_key_y[i] = DebugPublicKey_y[i];
			g_LM_config_info.secret_key[i] = DebugSecretKey[i];
		}
	}
	return NO_ERROR;

}

void LMssp_Generate_New_Public_Private_Key_Pair(boolean blocking)
{
	// Need to get a 24 Byte random number - which we will use as the secret key.
	// We then ECC multiply this by the Base Points, to get a new Public Key.
	//

	do
	{
		SYSrand_Get_Rand_192_Ex(g_LM_config_info.secret_key);
	}
	while(!LMecc_isValidSecretKey(g_LM_config_info.secret_key));
#if 1 // LC FIX FOR SSP- Blocking
	g_LM_config_info.new_public_key_generation = 0x01; // Key Generation Started
#endif
	LMecc_Generate_ECC_Key(g_LM_config_info.secret_key,BasePoint_x,BasePoint_y,0,blocking);
  
}
/**************************************************************************************
 * Interface to the HCI
 *----------------------
 * This section contains the following functions which are used to interface to the HCI
 * and have one-to-one mapping to HCI commands
 *
 * LMssp_LM_IO_Capability_Request_Reply
 * LMssp_LM_IO_Capability_Request_Negative_Reply
 * 
 * LMssp_LM_User_Confirmation_Request_Reply
 * LMssp_LM_User_Confirmation_Request_Negative_Reply
 *
 * LMssp_LM_User_Passkey_Request_Reply
 * LMssp_LM_User_Passkey_Request_Negative_Reply
 *
 * LMssp_LM_Remote_OOB_Data_Request_Reply
 * LMssp_LM_Remote_OOB_Data_Request_Negative_Reply
 *
 * LMssp_LM_Read_Local_OOB_Data
 **************************************************************************************/

t_error LMssp_LM_IO_Capability_Request_Reply(t_lmp_link* p_link,
											 u_int8 io_cap,
											 u_int8 oob_data_present,
											 u_int8 auth_requirements)
{
	t_error status;

	if (p_link->auth_sub_state == W4_HCI_IO_CAP_REQUEST_REPLY)
	{
		// Clear the HCI timer it there is one associated with the link
		if (p_link->ssp_hci_timer_index)
		{
			LMtmr_Clear_Timer(p_link->ssp_hci_timer_index);
		}
		// store the capabilities
		g_LM_config_info.io_cap = io_cap;
#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)
		g_LM_config_info.oob_data_present = oob_data_present;
#endif
		g_LM_config_info.auth_requirements = auth_requirements;

		p_link->peer_user_authenticated = 0x00;
		p_link->local_user_authenticated = 0x00;
#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)
		p_link->local_oob_verified = 0x00;
#endif
	    p_link->rxed_simple_pairing_number = 0x00;

		if (p_link->ssp_initiator)
		{
			p_link->current_proc_tid_role = p_link->ssp_tid = p_link->role;
			_LMssp_Send_LMP_IO_Capability_Request(p_link,io_cap,oob_data_present,auth_requirements);
			p_link->auth_sub_state = W4_LMP_IO_CAP_RESPONSE;
		}
		else
		{
			if (p_link->role == MASTER)
				p_link->ssp_tid = SLAVE;
			else
				p_link->ssp_tid = MASTER;

			p_link->current_proc_tid_role = p_link->ssp_tid;
			_LMssp_Determine_Auth_Type(p_link);
			_LMssp_Send_LMP_IO_Capability_Response(p_link,io_cap,oob_data_present,auth_requirements);
			p_link->auth_sub_state = W4_LMP_PUBLIC_KEY_HEADER;
		}
		p_link->sec_current_proc_tid_role = p_link->ssp_tid;
		status = NO_ERROR;
	}
	else
	{
        status = UNSPECIFIED_ERROR;
	}
	return status;
}

t_error LMssp_LM_IO_Capability_Request_Negative_Reply(t_lmp_link* p_link,t_error reason)
{
	if (p_link->auth_sub_state == W4_HCI_IO_CAP_REQUEST_REPLY)
	{

		// Clear the HCI timer it there is one associated with the link
		if (p_link->ssp_hci_timer_index)
		{
			LMtmr_Clear_Timer(p_link->ssp_hci_timer_index);
		}

		if(!p_link->ssp_initiator)
		{
			// If im the responder - I send not accepted to the LMP_IO_Capability request

			t_lmp_pdu_info pdu_info;
			pdu_info.tid_role = p_link->ssp_tid;
			pdu_info.opcode = LMP_NOT_ACCEPTED_EXT;
			pdu_info.return_opcode = LMP_IO_CAPABILITY_REQUEST;
			pdu_info.reason = reason;

			LM_Encode_LMP_PDU(p_link->device_index, &pdu_info);

		}
		// Need to Raise the simple pairing complete event.
        _LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);
	}
	else
		return COMMAND_DISALLOWED;

	return NO_ERROR;
}

void _LMssp_Send_Simple_Pairing_Complete_Event(t_lmp_link* p_link,t_error reason)
{	
	t_lm_event_info event_info;

	// Raise a Simple Pairing Complete Event
	event_info.p_bd_addr = &p_link->bd_addr;
	event_info.status = reason;

	HCeg_Generate_Event(HCI_SIMPLE_PAIRING_COMPLETE_EVENT,&event_info);

	// Need to Raise the simple pairing complete event.
	if (reason != NO_ERROR)
	{
		if (p_link->auth_super_state==HCI_ORIG_PAIRING)
		  LMsec_upper_Send_HCI_Event(p_link, HCI_AUTHENTICATION_COMPLETE_EVENT, 
            reason); 

		p_link->auth_sub_state = SUB_STATE_IDLE;
		p_link->auth_super_state = SUPER_STATE_IDLE;
		p_link->ssp_initiator = 0x00;
	}

}



t_error LMssp_LM_User_Confirmation_Request_Reply(t_lmp_link* p_link)
{
	if (p_link->auth_sub_state != W4_HCI_USER_CONFIRMATION_REQUEST_REPLY)
		return COMMAND_DISALLOWED;


	// Clear the HCI timer it there is one associated with the link
	if (p_link->ssp_hci_timer_index)
	{
		LMtmr_Clear_Timer(p_link->ssp_hci_timer_index);
		p_link->ssp_hci_timer_index=0;
	}

	if (p_link->ssp_initiator)
	{
		// If I am the initiator of SSP and the DH Key calculation is complete
		// then I send the LMP_DHkey_Check
		p_link->local_user_authenticated = 0x01;
		if (p_link->DHkeyCalculationComplete)
		{
			_LMssp_Send_LMP_DH_Key_Check(p_link);
			p_link->auth_sub_state = W4_LMP_ACK_DHKEY_CHECK;
		}
	}
	else
	{
		p_link->local_user_authenticated = 0x01;
		// Peer User Authenticated is set when we get the DH_KEYCHECK from the peer.
		if((p_link->peer_user_authenticated) && (p_link->DHkeyCalculationComplete))
		{
			// If the peer has already sent a DH_KEYCHECK 
			// I respond with LMP_Accepted at this point and Send my own LMP_DH_KEYCHECK

			/* From the spec page 260
			   If the user on the responding side indicates that the confirm values do not
               match (as indicated by the HCI_User_Confirmation_Negative_Reply command)
               the responding LM shall send an LMP_not_accepted PDU in response
               to the LMP_dhkey_check PDU.
			 */

			// The Above implies that the responder does not send the LMP_Accepted to the KEYCHECK until it has got
			// the User_Confirmation_Request_Reply.

			if (p_link->local_user_authenticated == 0x01)
			{
				LMssp_Calculate_DHKEY_Check(p_link,SSP_PEER);

				if (TRUE == _Compare_16byte_Arrays(p_link->peer_key_check,p_link->DHkeyCheck))
				{
					LMssp_Send_LMP_Accepted(p_link,LMP_DHKEY_CHECK,p_link->ssp_tid);
					// Should only send the LMP_DH_KeyCheck when we get the User Confirmation Request Reply 
					_LMssp_Send_LMP_DH_Key_Check(p_link);
				}
				else
				{
					LMssp_Send_LMP_Not_Accepted(p_link,LMP_DHKEY_CHECK,p_link->ssp_tid,AUTHENTICATION_FAILURE);		
					_LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);	
				}
			}
		}
		else
		{
			p_link->auth_sub_state = W4_LMP_PEER_DHKEY_CHECK;
		}
	}
    return NO_ERROR;
}

t_error LMssp_CallBack_DH_Key_Complete(t_lmp_link* p_link)
{
	p_link->DHkeyCalculationComplete = 0x01;
	if (((p_link->auth_type != OUT_OF_BAND) && (p_link->local_user_authenticated == 0x01))
#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED!=1)
		)
#else
		||
		((p_link->auth_type == OUT_OF_BAND) && (p_link->local_oob_verified == 0x01)))
#endif
	{
		if (p_link->ssp_initiator)
		{
			// If I am the initiator of SSP and the DH Key calculation is complete
			// then I send the LMP_DHkey_Check

            if ((p_link->auth_type == NUMERIC_COMPARISON) && (p_link->local_user_authenticated==0x01))
			{
				_LMssp_Send_LMP_DH_Key_Check(p_link);
				p_link->auth_sub_state = W4_LMP_ACK_DHKEY_CHECK;
			}
#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)
			else if(p_link->auth_type == OUT_OF_BAND)
			{
				_LMssp_Send_LMP_DH_Key_Check(p_link);
				p_link->auth_sub_state = W4_LMP_ACK_DHKEY_CHECK;
			}
#endif
		}
		else
		{
			if((p_link->peer_user_authenticated) && (p_link->auth_type != OUT_OF_BAND))
			{
				if (p_link->auth_type == NUMERIC_COMPARISON)
				{
				LMssp_Send_LMP_Accepted(p_link,LMP_DHKEY_CHECK,p_link->ssp_tid);
				// Should only send the LMP_DH_KeyCheck when we get the User Confirmation Request Reply 
				_LMssp_Send_LMP_DH_Key_Check(p_link);
				}
			}
			else if (p_link->auth_sub_state == W4_LMP_PEER_DHKEY_CHECK)
			{
				if((p_link->local_user_authenticated == 0x02) && (p_link->auth_type == NUMERIC_COMPARISON))
				{
					// User Sent User_Confirmation_Request_Negative_Reply 
					// so we reject the DH_Key
					LMssp_Send_LMP_Not_Accepted(p_link,LMP_DHKEY_CHECK,p_link->ssp_tid,AUTHENTICATION_FAILURE);
			        _LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);

				}
				else
				{
                    // GF 28 Jan 2010 Conformance Issue Test  Conf_7
#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)
					if (p_link->auth_type == OUT_OF_BAND)
					{
						LMssp_Calculate_DHKEY_Check(p_link,SSP_PEER);
					}
					else 
#endif
					if (p_link->auth_type == NUMERIC_COMPARISON)
					{
						if ((p_link->local_user_authenticated == 0x01) && 
						   (p_link->peer_user_dh_key_check_rxed == 0x01))

						{
							// Calculate the p_link->DHkeyCheck
							LMssp_Calculate_DHKEY_Check(p_link,SSP_PEER);
						}
					}

					// Executed in SSP_Bv07, SSP_BV19, SSP_Bv21, SSP_BV23
					if (TRUE == _Compare_16byte_Arrays(p_link->peer_key_check,p_link->DHkeyCheck))
					{
							
						p_link->peer_user_authenticated = 0x1;
						
						if ((p_link->local_user_authenticated == 0x01) || 
							(p_link->auth_type == OUT_OF_BAND))
						{
                        //************************************************************************
                        // GF 3 Dec 2010 :- Problem with interop to CSR dongle and Windows 7 host.
                        // This was due to bug where we send the LMP_Accepted to a DH_Key_Check 
						// without having Rxed the LMP_Accept
						//************************************************************************
#if 1 // GF 3 Dec -- Fix for Bug #3091
							if(p_link->peer_user_dh_key_check_rxed)
							{
								LMssp_Send_LMP_Accepted(p_link,LMP_DHKEY_CHECK,p_link->ssp_tid);
								_LMssp_Send_LMP_DH_Key_Check(p_link);
							}
#else // End of Fix for Bug #3091
							LMssp_Send_LMP_Accepted(p_link,LMP_DHKEY_CHECK,p_link->ssp_tid);

							_LMssp_Send_LMP_DH_Key_Check(p_link);

#endif
						}
					}
					else
					{
						LMssp_Send_LMP_Not_Accepted(p_link,LMP_DHKEY_CHECK,p_link->ssp_tid,AUTHENTICATION_FAILURE);
						
						_LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);
						
					}
				} 
			} 
			else
			{
				p_link->auth_sub_state = W4_LMP_PEER_DHKEY_CHECK;
			}
		}
	}
	else
	{
         if((p_link->local_user_authenticated == 0x02) && (p_link->auth_type == NUMERIC_COMPARISON))
		 {
			 // User Sent User_Confirmation_Request_Negative_Reply 
			 // so we reject the DH_Key
			LMssp_Send_LMP_Not_Accepted(p_link,LMP_DHKEY_CHECK,p_link->ssp_tid,AUTHENTICATION_FAILURE);
			_LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);
		 }

	}
	return NO_ERROR;
}
t_error LMssp_LM_User_Confirmation_Request_Negative_Reply(t_lmp_link* p_link)
{
	// Have to send LMP_Numeric_Comparision_Failed to the Peer.
	// Also have to send Simple_Pairing_Complete(Failure) to the HCI.

	//
	// Important BUG Fix.
    //
	//  If we are the initiator then we can send the LMP_Numeric_Comparison_Failed
	//  If we are a responder then we send LMP_Not_Accepted to the LMP_DH_KEY_CHECK.

	if (p_link->auth_sub_state != W4_HCI_USER_CONFIRMATION_REQUEST_REPLY)
		return COMMAND_DISALLOWED;

	// Clear the HCI timer it there is one associated with the link
	if (p_link->ssp_hci_timer_index)
	{
		LMtmr_Clear_Timer(p_link->ssp_hci_timer_index);
        p_link->ssp_hci_timer_index = 0;
	}

	if (p_link->ssp_initiator == 0x01)
	{
		_LMssp_Send_LMP_Numeric_Comparison_Failed(p_link);
		_LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);
	}
	else
	{
		if (p_link->peer_user_authenticated == 0x01)
		{
			// If this case I can send a direct LMP Not accepted to the peer.
			//
			LMssp_Send_LMP_Not_Accepted(p_link,LMP_DHKEY_CHECK,p_link->ssp_tid,AUTHENTICATION_FAILURE);
			_LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);
		}
		else
		{
			p_link->local_user_authenticated = 0x02;
		}
	}
    return NO_ERROR;
}

#if (PRH_BS_CFG_SYS_SSP_PASSKEY_SUPPORTED==1)
t_error LMssp_LM_User_Passkey_Request_Reply(t_lmp_link* p_link,u_int32 passKey)
{

	if (p_link->auth_sub_state == W4_HCI_KEY_PRESS_NOTIFICATION)
	{
		u_int8 commitment[16];

		// Clear the HCI timer it there is one associated with the link
		if (p_link->ssp_hci_timer_index)
		{
			LMtmr_Clear_Timer(p_link->ssp_hci_timer_index);
			p_link->ssp_hci_timer_index = 0;
		}
		p_link->local_user_authenticated = 0x01;
		// Store the PassKey
		p_link->passKey = passKey;

		// 1. Get new Random Number
		SYSrand_Get_Rand_128_Ex((u_int8*)p_link->rand);

		// 2. Calculate the commitment
		p_link->passKeyCount = 0;
		// check first bit of the passkey - rai is one bit value of the passkey expanded to 8 bits (either 0x80 or 0x81).
		if (passKey & 0x00000001)
			Zero_Array[0] = 0x81;
		else
			Zero_Array[0] = 0x80;

		if (p_link->ssp_initiator)
		{
			F1(g_LM_config_info.public_key_x,p_link->peer_public_key_x,
				p_link->rand,Zero_Array,commitment);
			LMssp_Send_LMP_Simple_Pairing_Confirm(p_link,commitment);
			p_link->auth_sub_state = W4_LMP_SIMPLE_PAIRING_CONFIRM;

		}
		else
		{
			if (p_link->peer_user_authenticated)
			{
				// If I have got a LMP_Simple_Pairing_Confirm form the peer
				// then I can proceed to calculate my commitment and send to peer.

				F1(g_LM_config_info.public_key_x,p_link->peer_public_key_x,
					p_link->rand,Zero_Array,commitment);
				LMssp_Send_LMP_Simple_Pairing_Confirm(p_link,commitment);
				p_link->auth_sub_state = W4_LMP_SIMPLE_PAIRING_NUMBER;
			}


		}

	}
	else
	{
		return COMMAND_DISALLOWED;
	}

	return NO_ERROR;
}


t_error LMssp_LM_User_Passkey_Request_Negative_Reply(t_lmp_link* p_link)
{

	if (p_link->auth_sub_state == W4_HCI_KEY_PRESS_NOTIFICATION)
	{
		// Clear the HCI timer it there is one associated with the link
		if (p_link->ssp_hci_timer_index)
		{
			LMtmr_Clear_Timer(p_link->ssp_hci_timer_index);
			p_link->ssp_hci_timer_index = 0;
		}
	}
	else
		return COMMAND_DISALLOWED;

	if((p_link->peer_user_authenticated) && (!p_link->ssp_initiator))
	{
		// We have recieved the LMP_Simple_Pairing_Confirm from the peer.
		// Which indicates it has presented its Passkey Notification.

		LMssp_Send_LMP_Not_Accepted(p_link,LMP_SIMPLE_PAIRING_CONFIRM,p_link->ssp_tid,AUTHENTICATION_FAILURE);
	}

	if (p_link->ssp_initiator)
	{
		_LMssp_Send_LMP_Passkey_Entry_Failed(p_link);
	}

    _LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);
	return NO_ERROR;
}

#endif

#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)

t_error LMssp_LM_Read_Local_OOB_Data(void)
{
	u_int8 i;
	// Generate a 16 Byte Random and store in g_LM_config_info.randomizer_R
	
	SYSrand_Get_Rand_128_Ex((u_int8*)g_LM_config_info.randomizer_R);

	// Generate a 16 Byte Commitment and store in g_LM_config_info.hash_C

	for(i=0;i<16;i++)
	{
		Zero_Array[i] = 0x00;
	}

	// Calculate Cb - the peers commitment Cb = F1 (Pkb,Pkb,rb,0);

	F1(g_LM_config_info.public_key_x,g_LM_config_info.public_key_x,
		g_LM_config_info.randomizer_R,Zero_Array,g_LM_config_info.hash_C);

	return NO_ERROR;
}



// Interface to the LMP
/******************************************************************************************
 * Function :- LMssp_LM_Remote_OOB_Data_Request_Reply
 * 
 *
 *
 * Initiator
 *  Remote_OOB_Req_Reply(C,R)
 * -------------------------->
 *
 *                       < Check C >
 *                       OK or NOK
 *                              LMP_Simple_Pairing_Number
 *                            --------------------------->
 *                              LMP_Accepted
 *                            <---------------------------
 *                              LMP_Simple_Pairing_Number        
 *                            <---------------------------
 * 
 * ALT 1 ----------------------------------------------------------
 *                       If Check C was NOK
 *                             LMP_Not_Accepted                            
 *                            ---------------------------->
 * ALT 2 ----------------------------------------------------------
 *                       If Check C was OK
 *                             LMP_Accepted                            
 *                            ---------------------------->
 *-----------------------------------------------------------------
 */

t_error LMssp_LM_Remote_OOB_Data_Request_Reply(t_lmp_link* p_link,u_int8* p_C, u_int8* p_R)
{
	u_int8 commitment_b[16];

	if ((p_link) && (p_link->auth_sub_state == W4_HCI_REMOTE_OOB_DATA_REQUEST_REPLY))
	{
		int i;

		for(i=0;i<16;i++)
		{
			p_link->peer_commitment[i] = p_C[15-i];
			p_link->Rpeer[i] = p_R[15-i];
			Zero_Array[i] = 0x00;
		}

		// Calculate Cb - the peers commitment Cb = F1 (Pkb,Pkb,rb,0);

		F1(p_link->peer_public_key_x,p_link->peer_public_key_x,
		   p_link->Rpeer,Zero_Array,commitment_b);

		// Compare against peer commitment -from OOB
        if (TRUE == _Compare_16byte_Arrays(commitment_b,p_link->peer_commitment))
		{
			p_link->local_oob_verified = 0x01; // 0x01 - Local OOB Commitment verified Success.
			                                   // 0x02 - Local OOB Commitment verified failure.
			                                   // 0x00 - Local OOB Commitment not yet verified.
			if (p_link->oob_data_present == 0x00)
			{
				//
				// If peers IO capability does not indicate OOB authentication data present
				// the we set the local randomiser (g_LM_config_info.randomizer_r) to zero.

				for(i=0;i<16;i++)
					g_LM_config_info.randomizer_R[i] = 0x00;
			}
		}
		else
		{
			p_link->local_oob_verified = 0x02; // Failure.
		}

		if (p_link->ssp_initiator==0x01)
		{

			// The Initiator always sends the LMP_SIMPLE_PAIRING_NUMBER. As this gives the 
			// peer a chance to reject it and indicate failure.
			SYSrand_Get_Rand_128_Ex((u_int8*)p_link->rand);
			LMssp_Send_LMP_Simple_Pairing_Number(p_link);
			p_link->auth_sub_state = W4_LMP_ACK_SIMPLE_PAIRING_NUMBER;
		}
		else
		{   // If Im the responder there can be two cases :
			//
			//  1. I have already recieved LMP Simple Pairing Number.
			//     in this case I use the commitment check to determine if
			//     I send LMP_Accepted/LMP_Not_Accepted
			//     And send LMP_Simple_Pairing_Number.
			//
			//  2. If I have not recieved the LMP Simple Pairing Number then
			//     I perform equivalent processing in the handling of the PDU.

			if (p_link->rxed_simple_pairing_number)
			{
				if (p_link->local_oob_verified == 1)
				{
					LMssp_Send_LMP_Accepted(p_link,LMP_SIMPLE_PAIRING_NUMBER,p_link->ssp_tid);
					SYSrand_Get_Rand_128_Ex((u_int8*)p_link->rand);
					LMssp_Send_LMP_Simple_Pairing_Number(p_link);
					p_link->auth_sub_state = W4_LMP_ACK_SIMPLE_PAIRING_NUMBER;

				}
				else if (p_link->local_oob_verified == 2)
				{
					LMssp_Send_LMP_Not_Accepted(p_link,LMP_SIMPLE_PAIRING_NUMBER,p_link->ssp_tid,AUTHENTICATION_FAILURE);
					_LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);
				}
			}

			// For OOB we can initiate the DH Key Check :
			// in the Initiator :- When Tx LMP_Accepted(LMP_Simple_Pairing_Number)
			// in the responder :- When Rx LMP_Accepted(LMP_Simple_Pairing_Number)
		}
		return NO_ERROR;
	}
	else
		return COMMAND_DISALLOWED;
}

#endif

/**************************************************************************************
 * Interface to the LMP
 *----------------------
 * This section contains the following functions which are used to interface to the peer
 * LMP have one-to-one mapping to LMP messages
 *
 * First the Functions for Transmission of LMP_Messages are :
 * _LMssp_Send_LMP_IO_Capability_Request
 * _LMssp_Send_LMP_OOB_Failed
 * _LMssp_Send_LMP_Numeric_Comparison_Failed
 * _LMssp_Send_LMP_Passkey_Entry_Failed
 * _LMssp_Send_LMP_IO_Capability_Response
 * LMssp_Send_Keypress_Notification
 * LMspp_Send_LMP_Encapsulated_Payload
 * _LMspp_Send_LMP_Encapsulated_Header
 * LMssp_Send_LMP_Accepted
 * LMssp_Send_LMP_Not_Accepted
 * LMssp_Send_LMP_Simple_Pairing_Confirm
 * LMssp_Send_LMP_Simple_Pairing_Number
 * _LMssp_Send_LMP_DH_Key_Check
 * 
 * The functions for handling the reception of LMP messages are :
 *
 * LMspp_LMP_IO_Capability_Response
 * LMssp_LMP_Passkey_Entry_Failed
 * LMssp_LMP_Keypress_Notification
 * LMspp_LMP_IO_Capability_Request
 * LMssp_LMP_Encapsulated_Header
 * LMssp_LMP_DHkey_Check
 * LMssp_LMP_Encapsulated_Payload
 * LMssp_LMP_Simple_Pairing_Confirm
 * LMssp_LMP_Simple_Pairing_Confirm
 * LMssp_LMP_Simple_Pairing_Number
 * LMssp_LMP_Numeric_Comparison_Failure
 * LMssp_LMP_Accepted
 * LMssp_LMP_Not_Accepted
 */

t_error _LMssp_Send_LMP_IO_Capability_Request(t_lmp_link* p_link,
											 u_int8 io_cap,
											 u_int8 oob_data_present,
											 u_int8 auth_requirements)
{
    u_int8 _LMssp_LMP_IO_Capability_Request_PDU[5];

	p_link->current_proc_tid_role = p_link->ssp_tid;

    _LMssp_LMP_IO_Capability_Request_PDU[0] = (127<<1) + p_link->ssp_tid;
    _LMssp_LMP_IO_Capability_Request_PDU[1] = (LMP_IO_CAPABILITY_REQUEST & 0xff);
	_LMssp_LMP_IO_Capability_Request_PDU[2] = io_cap;
	_LMssp_LMP_IO_Capability_Request_PDU[3] = oob_data_present;
	_LMssp_LMP_IO_Capability_Request_PDU[4] = auth_requirements;

    LM_Encode_LMP_PDU_Fully_Encoded(p_link->device_index,
        LMP_IO_CAPABILITY_REQUEST, _LMssp_LMP_IO_Capability_Request_PDU);

	return NO_ERROR;
}

#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)

t_error _LMssp_Send_LMP_OOB_Failed(t_lmp_link* p_link)
{
    u_int8 _LMssp_LMP_OOB_Failed_PDU[2];

	_LMssp_LMP_OOB_Failed_PDU[0] = (127<<1) + p_link->ssp_tid ;
    _LMssp_LMP_OOB_Failed_PDU[1] = (LMP_OOB_FAILED & 0xff);

    LM_Encode_LMP_PDU_Fully_Encoded(p_link->device_index,
		LMP_OOB_FAILED, _LMssp_LMP_OOB_Failed_PDU);

	return NO_ERROR;

}

#endif

static t_error _LMssp_Send_LMP_Numeric_Comparison_Failed(t_lmp_link* p_link)
{
    u_int8 _LMssp_LMP_Numeric_Comparison_Failed_PDU[2];

	_LMssp_LMP_Numeric_Comparison_Failed_PDU[0] = (127<<1) + p_link->ssp_tid ;
    _LMssp_LMP_Numeric_Comparison_Failed_PDU[1] = (LMP_NUMERIC_COMPARISON_FAILED & 0xff);

    LM_Encode_LMP_PDU_Fully_Encoded(p_link->device_index,
		LMP_NUMERIC_COMPARISON_FAILED, _LMssp_LMP_Numeric_Comparison_Failed_PDU);

	return NO_ERROR;
}

#if (PRH_BS_CFG_SYS_SSP_PASSKEY_SUPPORTED==1)

static t_error _LMssp_Send_LMP_Passkey_Entry_Failed(t_lmp_link* p_link)
{
    u_int8 _LMssp_LMP_Passkey_Entry_Failed_PDU[2];

	_LMssp_LMP_Passkey_Entry_Failed_PDU[0] = (127<<1) + p_link->ssp_tid ;
    _LMssp_LMP_Passkey_Entry_Failed_PDU[1] = (LMP_PASSKEY_ENTRY_FAILED & 0xff);

    LM_Encode_LMP_PDU_Fully_Encoded(p_link->device_index,
		LMP_PASSKEY_ENTRY_FAILED, _LMssp_LMP_Passkey_Entry_Failed_PDU);

	return NO_ERROR;
}

#endif

static t_error _LMssp_Send_LMP_IO_Capability_Response(t_lmp_link* p_link,
											 u_int8 io_cap,
											 u_int8 oob_data_present,
											 u_int8 auth_requirements)
{
    u_int8 _LMssp_LMP_IO_Capability_Response_PDU[5];

	_LMssp_LMP_IO_Capability_Response_PDU[0] = (127<<1) + p_link->ssp_tid ;
    _LMssp_LMP_IO_Capability_Response_PDU[1] = (LMP_IO_CAPABILITY_RESPONSE & 0xff);
	_LMssp_LMP_IO_Capability_Response_PDU[2] = io_cap;
	_LMssp_LMP_IO_Capability_Response_PDU[3] = oob_data_present;
	_LMssp_LMP_IO_Capability_Response_PDU[4] = auth_requirements;

    LM_Encode_LMP_PDU_Fully_Encoded(p_link->device_index,
		LMP_IO_CAPABILITY_RESPONSE, _LMssp_LMP_IO_Capability_Response_PDU);

	return NO_ERROR;

}

#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)

t_error LMssp_LMP_OOB_Failed(t_lmp_link* p_link, t_p_pdu p_pdu)
{

	if (p_link->auth_type == OUT_OF_BAND) // Should expand to check state here !!
	{
		_LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);
	}
	else
	{
		return LMP_PDU_NOT_ALLOWED;
	}

	return NO_ERROR;
}

#endif


#if (PRH_BS_CFG_SYS_SSP_KEYPRESS_SUPPORTED==1)

t_error LMssp_Send_Keypress_Notification(t_lmp_link* p_link,u_int8 notification_type)
{
	// A side with the KeyboardOnly IO capability may send notifications on key
    // presses to the remote side using the LMP_keypress_notification PDU.

	if (g_LM_config_info.io_cap == KeyboardOnly)
	{
    u_int8 _LMssp_LMP_Keypress_Notification_PDU[3];

	_LMssp_LMP_Keypress_Notification_PDU[0] = (127<<1) + p_link->ssp_tid ;
    _LMssp_LMP_Keypress_Notification_PDU[1] = (LMP_KEYPRESS_NOTIFICATION & 0xff);
	_LMssp_LMP_Keypress_Notification_PDU[2] = notification_type;


    LM_Encode_LMP_PDU_Fully_Encoded(p_link->device_index,
		LMP_KEYPRESS_NOTIFICATION, _LMssp_LMP_Keypress_Notification_PDU);

	return NO_ERROR;
    }
	else
	{
		return COMMAND_DISALLOWED;
	}
}

#endif

#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)

t_error LMssp_LM_Remote_OOB_Data_Request_Negative_Reply(t_lmp_link* p_link)
{
	if ((p_link) && (p_link->auth_sub_state == W4_HCI_REMOTE_OOB_DATA_REQUEST_REPLY) &&
		(p_link->ssp_initiator == 0x01))
	{
        _LMssp_Send_LMP_OOB_Failed(p_link);
		_LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);
		return NO_ERROR;
	}
	else
		return UNSPECIFIED_ERROR;
}

#endif

t_error LMspp_LMP_IO_Capability_Response(t_lmp_link* p_link, t_p_pdu p_pdu)
{
	u_int8 io_cap;
	u_int8 oob_data_present;
	u_int8 auth_requirements;
	
	io_cap = *p_pdu;
	oob_data_present = *(p_pdu+1);
	auth_requirements = *(p_pdu+2);

	if ((!p_link->ssp_initiator) || (p_link->auth_sub_state != W4_LMP_IO_CAP_RESPONSE))
		return UNSPECIFIED_ERROR;

	if ((io_cap < 0x04) && (oob_data_present < 0x02) && (auth_requirements < 0x06))
	{
		// Raise HCI_io capability request reply

		t_lm_event_info event_info;

		p_link->io_cap = io_cap;
#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)
		p_link->oob_data_present = oob_data_present;
#endif
		p_link->auth_requirements = auth_requirements;
		_LMssp_Determine_Auth_Type(p_link);

		event_info.p_bd_addr = &p_link->bd_addr;
		event_info.io_cap = io_cap;
        event_info.oob_data_present = oob_data_present;
		event_info.auth_requirements = auth_requirements;
		HCeg_Generate_Event(HCI_IO_CAPABILITY_RESPONSE_EVENT,&event_info);

		p_link->auth_sub_state = W4_LMP_ACK_PUBLIC_KEY_HEADER;
		_LMspp_Send_LMP_Encapsulated_Header(p_link,MASTER);
	}
	else
	{
		// Send LMP not accepted 
		LM_Encode_LMP_Not_Accepted_Ext_PDU(p_link, 
                       LMP_IO_CAPABILITY_RESPONSE, 
                       INVALID_LMP_PARAMETERS);

#if 1 // Very usefull for test to force the Simple Pairing Complete Event
	  // Temp for testing and debug we send Simple Pairing Complete Event
	  // to be removed from Deployed system
            _LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);

#endif
	}
	return NO_ERROR;
}
#if (PRH_BS_CFG_SYS_SSP_PASSKEY_SUPPORTED==1)

t_error LMssp_LMP_Passkey_Entry_Failed(t_lmp_link* p_link, t_p_pdu p_pdu)
{
	// Need to do state check here. 
	if (p_link->auth_type == PASSKEY_ENTRY) // Should expand to check state here !!
	{
		// Raise a Simple Pairing Complete Event with failure 
		_LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);

	}
	else
	{
		return LMP_PDU_NOT_ALLOWED;
	}

	return NO_ERROR;
}
#endif

#if (PRH_BS_CFG_SYS_SSP_KEYPRESS_SUPPORTED==1)

t_error LMssp_LMP_Keypress_Notification(t_lmp_link* p_link, t_p_pdu p_pdu)
{
	t_lm_event_info event_info;

	event_info.p_bd_addr = &p_link->bd_addr;
	event_info.numeric_value = *(p_pdu);
	HCeg_Generate_Event(HCI_KEYPRESS_NOTIFICATION_EVENT,&event_info);

	return NO_ERROR;
}

#endif
t_error LMspp_LMP_IO_Capability_Request(t_lmp_link* p_link, t_p_pdu p_pdu)
{
    // Check State 

	u_int8 io_cap;
	u_int8 oob_data_present;
	u_int8 auth_requirements;
	
	io_cap = *p_pdu;
	oob_data_present = *(p_pdu+1);
	auth_requirements = *(p_pdu+2);

	p_link->DHkeyCalculationComplete =0;
	// if the Simple Pairing is not set then we respond with not accepted

	if (!g_LM_config_info.ssp_enabled)
	{
        LM_Encode_LMP_Not_Accepted_Ext_PDU(p_link, 
                            LMP_IO_CAPABILITY_REQUEST, 
                            EC_SSP_NOT_SUPPORTED_BY_HOST);

		return NO_ERROR;
	}
    // If we also act as SSP initiator - then we have a transaction collision
	else if (p_link->ssp_initiator)
	{
		if(p_link->role == MASTER)
		{
			// Reject the LMP_IO_CAP_REQ indicating Transaction Collision

			return LMP_ERROR_TRANSACTION_COLLISION;
		}
		else
		{
			// Im a slave - so when I get transaction collision - I let the master 
			// proceed and am not longer the SSP_Initiator
            //
			p_link->ssp_initiator = 0x00;
			p_link->ssp_tid = MASTER;
			p_link->current_proc_tid_role = p_link->ssp_tid;
			_LMssp_Determine_Auth_Type(p_link);
#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)
			_LMssp_Send_LMP_IO_Capability_Response(p_link,g_LM_config_info.io_cap,
				g_LM_config_info.oob_data_present,g_LM_config_info.auth_requirements);
#else
			_LMssp_Send_LMP_IO_Capability_Response(p_link,g_LM_config_info.io_cap,
				0x00,g_LM_config_info.auth_requirements);
#endif
			// Also need to change the state here !.

			p_link->auth_sub_state = W4_LMP_PUBLIC_KEY_HEADER;
		}
	}
	
	if ((io_cap < 0x04) && (oob_data_present < 0x02) && (auth_requirements < 0x06))
	{
		t_lm_event_info event_info;
		// Check state and raise HCI event.
        // If initiator and in the correct Security State raise the below event.

		// Raise HCI_io capability request reply
		// Raise HCI_io capability request

		p_link->io_cap = io_cap;
#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)
        p_link->oob_data_present = oob_data_present;
#endif
        p_link->auth_requirements = auth_requirements;

		event_info.p_bd_addr = &p_link->bd_addr;
		event_info.io_cap = io_cap;
        event_info.oob_data_present = oob_data_present;
		event_info.auth_requirements = auth_requirements;
		HCeg_Generate_Event(HCI_IO_CAPABILITY_RESPONSE_EVENT,&event_info);

		if (p_link->auth_sub_state != W4_LMP_PUBLIC_KEY_HEADER)
		{
		event_info.p_bd_addr = &p_link->bd_addr;
		HCeg_Generate_Event(HCI_IO_CAPABILITY_REQUEST_EVENT,&event_info);
		p_link->ssp_hci_timer_index = LMtmr_Set_Timer(PRH_LMP_MSG_TIMEOUT,LMssp_HCI_Timeout,p_link,1);

		// Need to clear any timers associated with Pre-2.1 security

		if (p_link->sec_timer)
		{
			LMtmr_Clear_Timer(p_link->sec_timer);
			p_link->sec_timer = 0;
		}

        p_link->auth_sub_state = W4_HCI_IO_CAP_REQUEST_REPLY;

		}
	}
	else
	{
		// Send LMP not accepted 
		// Reject the LMP_IO_CAP_REQ indicating Transaction Collision
		LM_Encode_LMP_Not_Accepted_Ext_PDU(p_link, 
                       LMP_IO_CAPABILITY_REQUEST, 
                       INVALID_LMP_PARAMETERS);
#if 1 // Very usefull for test to force the Simple Pairing Complete Event
        _LMssp_Send_Simple_Pairing_Complete_Event(p_link,INVALID_LMP_PARAMETERS);

#endif
	}
	return NO_ERROR;
}




t_error LMssp_LMP_Encapsulated_Header(t_lmp_link* p_link, t_p_pdu p_pdu)
{
	if (p_link->auth_sub_state == W4_LMP_PUBLIC_KEY_HEADER)
	{
		if (((*(p_pdu)) == 0x01) && ((*(p_pdu+1)) == 0x01) && ((*(p_pdu+2)) ==  0x30))
		{
			p_link->Incoming_Encapsulated_P192_len_pending = 0x30;

			// LMssp_LMP_Send_LMP_Accepted
			LMssp_Send_LMP_Accepted(p_link, LMP_ENCAPSULATED_HEADER, 
				p_link->ssp_tid );
			p_link->auth_sub_state = W4_LMP_PUBLIC_KEY_PAYLOAD;
		return NO_ERROR;
		}
		else
		{
#if 1 // Very usefull for test to force the Simple Pairing Complete Event
            _LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);
#endif
			return INVALID_LMP_PARAMETERS;
		}


	}
	else
		return COMMAND_DISALLOWED;

}

t_error LMssp_LMP_DHkey_Check(t_lmp_link* p_link, t_p_pdu p_pdu)
{
	u_int8 i;
	u_int8 io_cap[3];
	u_int8 Address_Local[6];
	u_int8 Address_Peer[6];
//	u_int8 Ra[16];
	const t_bd_addr *my_bdaddr;

	my_bdaddr = SYSconfig_Get_Local_BD_Addr_Ref();

	for (i=0;i<6;i++)
	{
		Address_Peer[i] = p_link->bd_addr.bytes[5-i];
		Address_Local[i] = my_bdaddr->bytes[5-i];
	}

	if ((p_link->auth_sub_state == W4_LMP_PEER_DHKEY_CHECK) ||
	    (p_link->auth_sub_state == W4_HCI_USER_CONFIRMATION_REQUEST_REPLY))
	{
		p_link->peer_user_dh_key_check_rxed = 0x01;

		// GF 5 Feb -- Not sure about the below line
		p_link->peer_user_authenticated = 0x01;

		for (i=0;i<16;i++)
			p_link->peer_key_check[i] = *(p_pdu+15-i);

		// Once I have the key check from the peer - I need to compare it with the 
		// locally generated KeyCheck. Send accepted if they match OR not_accepted if 
		// they dont.
		io_cap[2] = p_link->io_cap;
#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)
		io_cap[1] = p_link->oob_data_present;
#else
		io_cap[1] = 0x00;
#endif
		io_cap[0] = p_link->auth_requirements;

		if (p_link->auth_type != OUT_OF_BAND)
		{
			u_int8 i;

#if (PRH_BS_CFG_SYS_SSP_PASSKEY_SUPPORTED==1)
			if (p_link->auth_type == PASSKEY_ENTRY)
			{
                LMssp_Calculate_DHKEY_Check(p_link,SSP_PEER);
			}
			else 
#endif
			if (p_link->auth_type == NUMERIC_COMPARISON)
			{
				if ((p_link->DHkeyCalculationComplete) && (p_link->ssp_initiator))
				{
					LMssp_Calculate_DHKEY_Check(p_link,SSP_PEER);
				}
			}
			else
			{
				return COMMAND_DISALLOWED;
			}

			for (i=0;i<16;i++)
			{
				Zero_Array[i] = 0;
			}
		}
		else
		{
            if (p_link->DHkeyCalculationComplete)
			{
				LMssp_Calculate_DHKEY_Check(p_link,SSP_PEER);
			}

		}
		if (p_link->ssp_initiator)
		{
			if (TRUE == _Compare_16byte_Arrays(p_link->peer_key_check,p_link->DHkeyCheck))
			{
				LMssp_Send_LMP_Accepted(p_link,LMP_DHKEY_CHECK,p_link->ssp_tid);

				// Now Raise the Simple Pairing Complete Event

                _LMssp_Send_Simple_Pairing_Complete_Event(p_link,NO_ERROR);
		
				_LMssp_Calculate_Link_Key(p_link);

				// Begin Mutual Authentication 
			    _LMssp_Mutual_Authentication(p_link);

				// Only change the initiator after mutual auth.
				p_link->ssp_initiator = 0x00;
			}
			else
			{
				LMssp_Send_LMP_Not_Accepted(p_link,LMP_DHKEY_CHECK,p_link->ssp_tid,AUTHENTICATION_FAILURE);

                _LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);
			}
		}
		else // If the responder -- I have to wait until my DHkey Calculation is 
			 // complete before responding.
		{
			if(p_link->DHkeyCalculationComplete)
			{
                if((p_link->local_user_authenticated == 0x02) && (p_link->auth_type == NUMERIC_COMPARISON))
				{
					// User Sent User_Confirmation_Request_Negative_Reply 
					// so we reject the DH_Key
					LMssp_Send_LMP_Not_Accepted(p_link,LMP_DHKEY_CHECK,p_link->ssp_tid,AUTHENTICATION_FAILURE);
			        _LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);

				}
				else if ((p_link->local_user_authenticated == 0x01) || 
						(p_link->auth_type == OUT_OF_BAND))
				{
					p_link->peer_user_authenticated = 0x1;
                    // GF 3 Dec 2010 - Fix for Bug #3091
			     	LMssp_Calculate_DHKEY_Check(p_link,SSP_PEER); 

					if (TRUE == _Compare_16byte_Arrays(p_link->peer_key_check,p_link->DHkeyCheck))
					{
						LMssp_Send_LMP_Accepted(p_link,LMP_DHKEY_CHECK,p_link->ssp_tid);
						// Should only send the LMP_DH_KeyCheck when we get the User Confirmation Request Reply 
						_LMssp_Send_LMP_DH_Key_Check(p_link);
					}
					else
					{
						LMssp_Send_LMP_Not_Accepted(p_link,LMP_DHKEY_CHECK,p_link->ssp_tid,AUTHENTICATION_FAILURE);
						_LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);
					}
				}
				else
				{
					// Do Nothing 
					p_link->peer_user_authenticated = 0x1;
					p_link->peer_user_dh_key_check_rxed = 0x01;	                
				}

			}
		}
	}
	else
	{
		return COMMAND_DISALLOWED;
	}

    return NO_ERROR;

}

t_error LMssp_LMP_Encapsulated_Payload(t_lmp_link* p_link, t_p_pdu p_pdu)
{

	/*
	 * Note :- The Public Key is transfered in Little Endian format.
	 *         The least significant byte is transmitted first.
     * 
	 *         We store them in Big Endian format. Most Significant byte first.
	 *         as the functions P-192,F1,F2,F3,G all operate in Big Endian format.
	 *    
	 *         The public keys are stored in Big Endian format.
	 */ 

	u_int8 offset;
	u_int8 i;
	u_int8 big_num_offset;
     // Need to copy 16 bytes from the payload into Peer Public Key.

	if (p_link->auth_sub_state != W4_LMP_PUBLIC_KEY_PAYLOAD)
		return COMMAND_DISALLOWED;

	offset = 0x30 - p_link->Incoming_Encapsulated_P192_len_pending;
	// Temp offset used due to the nature of the bigHex type 
	// This offset should be removed later in development
	big_num_offset = 3;

	if (offset==0)
	{
		for (i=0;i < 16;i++)
		{
			p_link->peer_public_key_x[23-i] = (*(p_pdu+i)) & 0x00FF;
		}
	}
	else if (offset == 16)
	{
		for (i=0;i < 8;i++)
		{
			p_link->peer_public_key_x[7-i] = (*(p_pdu+i)) & 0x00FF;
		}
		for (i=0;i < 8;i++)
		{
			p_link->peer_public_key_y[23-i] = (*(p_pdu+i+8)) & 0x00FF;
		}
	}
	else if (offset == 32)
	{
		for (i=0;i < 16; i++)
		{
			p_link->peer_public_key_y[15-i] =  (*(p_pdu+i)) & 0x00FF;
		}

		// Need to set the number lenghts here too
	}

	p_link->Incoming_Encapsulated_P192_len_pending-=16;
	if (p_link->Incoming_Encapsulated_P192_len_pending !=0)
		LMssp_Send_LMP_Accepted(p_link, LMP_ENCAPSULATED_PAYLOAD, p_link->ssp_tid);

	if (p_link->Incoming_Encapsulated_P192_len_pending==0)
	{

		if(p_link->ssp_initiator)
		{
			boolean DebugKey = TRUE;
			// Determine if this is the debug ssp key

			for (i=0;i<24;i++)
			{
				if ((p_link->peer_public_key_x[i] != DebugPublicKey_x[i]) ||
					(p_link->peer_public_key_y[i] != DebugPublicKey_y[i]))
				{
					DebugKey = FALSE;
				}
			}

			if (DebugKey == TRUE)
			{
				p_link->ssp_debug_mode = 0x01;
			}
			else
			{
				p_link->ssp_debug_mode = 0x00;
			}
			// Determine the Authentication Type which will be used 
			_LMssp_Determine_Auth_Type(p_link);

			switch(p_link->auth_type)
			{
#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)
			case OUT_OF_BAND:
				_LMssp_Init_DHKey_Calculation(p_link);
				LMssp_Send_LMP_Accepted(p_link, LMP_ENCAPSULATED_PAYLOAD, p_link->ssp_tid);
			if ((g_LM_config_info.oob_data_present == 0x01)) 
			{
				t_lm_event_info event_info;

					event_info.p_bd_addr = &p_link->bd_addr;
					HCeg_Generate_Event(HCI_REMOTE_OOB_DATA_REQUEST_EVENT,&event_info);

					p_link->auth_sub_state = W4_HCI_REMOTE_OOB_DATA_REQUEST_REPLY;
			}
			else
			{
				// Need to add code here to handle the case where I dont have OOB data.
				for(i=0;i<16;i++)
				{
					p_link->peer_commitment[i] = 0x00;
					p_link->Rpeer[i] = 0x00;
					Zero_Array[i] = 0x00;
				}

				// Calculate Cb - the peers commitment Cb = F1 (Pkb,Pkb,rb,0);

				// Post UPF change :
				// I copy the calculated commitment directly into the 
				// p_link->peer_commitment 

				F1(p_link->peer_public_key_x,p_link->peer_public_key_x,
						p_link->Rpeer,Zero_Array,p_link->peer_commitment);


				p_link->local_oob_verified = 0x01; 

				if (p_link->oob_data_present == 0x00)
				{
					for(i=0;i<16;i++)
						g_LM_config_info.randomizer_R[i] = 0x00;
				}

				if (p_link->ssp_initiator==0x01)
				{
					// The Initiator always sends the LMP_SIMPLE_PAIRING_NUMBER. As this gives the 
					// peer a chance to reject it and indicate failure.
					SYSrand_Get_Rand_128_Ex((u_int8*)p_link->rand);
					LMssp_Send_LMP_Simple_Pairing_Number(p_link);
					p_link->auth_sub_state = W4_LMP_ACK_SIMPLE_PAIRING_NUMBER;
				}
				else
				{   // If Im the responder there can be two cases :

					if (p_link->rxed_simple_pairing_number)
					{
						if (p_link->local_oob_verified == 1)
						{
							LMssp_Send_LMP_Accepted(p_link,LMP_SIMPLE_PAIRING_NUMBER,p_link->ssp_tid);
							SYSrand_Get_Rand_128_Ex((u_int8*)p_link->rand);
							LMssp_Send_LMP_Simple_Pairing_Number(p_link);
							p_link->auth_sub_state = W4_LMP_ACK_SIMPLE_PAIRING_NUMBER;
						}
						else if (p_link->local_oob_verified == 2)
						{
							LMssp_Send_LMP_Not_Accepted(p_link,LMP_SIMPLE_PAIRING_NUMBER,p_link->ssp_tid,AUTHENTICATION_FAILURE);
							_LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);
						}
					}
				}
			}

				break;
#endif
			case NUMERIC_COMPARISON:
				p_link->auth_sub_state = W4_LMP_SIMPLE_PAIRING_CONFIRM;
                _LMssp_Init_DHKey_Calculation(p_link);
				LMssp_Send_LMP_Accepted(p_link, LMP_ENCAPSULATED_PAYLOAD, p_link->ssp_tid);
				break;

#if (PRH_BS_CFG_SYS_SSP_PASSKEY_SUPPORTED==1)
			case PASSKEY_ENTRY :
			// Must investigatee further. 
				if(((g_LM_config_info.io_cap == DisplayOnly) ||
					(g_LM_config_info.io_cap == DisplayYesNo)) &&
					(p_link->io_cap ==KeyboardOnly))
				{
					// Peer device has a keyboard and local device has display
					// Raise a user Pass Key Notification Event.
					t_lm_event_info event_info;
					u_int8 commitment[16];
					//u_int32 user_confirm_value;

					// Need to generate a Pass Key number.
					// Store PassKey and reset Passkey count.
					event_info.p_bd_addr = &p_link->bd_addr;
					p_link->passKey = ((SYSrand_Get_Rand()) % 0xF4240);
					event_info.numeric_value = p_link->passKey;
					p_link->passKeyCount = 0;
					p_link->local_user_authenticated = 0x01;
					HCeg_Generate_Event(HCI_USER_PASSKEY_NOTIFICATION_EVENT,&event_info);

					// Once the USER_PASSKEY NOTIFICATION has been raised. If I am the initiator
					// I can send the LMP simple pairing confirm 

					// 1. Get new Random Number
					SYSrand_Get_Rand_128_Ex((u_int8*)p_link->rand);

					// check first bit of the passkey - rai is one bit value of the passkey expanded to 8 bits (either 0x80 or 0x81).
					if (p_link->passKey & 0x00000001)
						Zero_Array[0] = 0x81;
					else
						Zero_Array[0] = 0x80;

					F1(g_LM_config_info.public_key_x,p_link->peer_public_key_x,
						p_link->rand,Zero_Array,commitment);

					_LMssp_Init_DHKey_Calculation(p_link);
					LMssp_Send_LMP_Accepted(p_link, LMP_ENCAPSULATED_PAYLOAD, p_link->ssp_tid);

					LMssp_Send_LMP_Simple_Pairing_Confirm(p_link,commitment);
					p_link->auth_sub_state = W4_LMP_SIMPLE_PAIRING_CONFIRM;

				}
				else if ((g_LM_config_info.io_cap == KeyboardOnly) &&
					((p_link->io_cap == DisplayYesNo) ||
					 (p_link->io_cap == DisplayOnly ) ||
					 (p_link->io_cap == KeyboardOnly)))
				{
					t_lm_event_info event_info;

					// Need to generate a Pass Key number.

					event_info.p_bd_addr = &p_link->bd_addr;
					HCeg_Generate_Event(HCI_USER_PASSKEY_REQUEST_EVENT,&event_info);
				    p_link->ssp_hci_timer_index = LMtmr_Set_Timer(PRH_LMP_MSG_TIMEOUT,LMssp_HCI_Timeout,p_link,1);
					_LMssp_Init_DHKey_Calculation(p_link);
					LMssp_Send_LMP_Accepted(p_link, LMP_ENCAPSULATED_PAYLOAD, p_link->ssp_tid);

					p_link->auth_sub_state = W4_HCI_KEY_PRESS_NOTIFICATION;
				}
				break;
#endif
			default :
				break;
			}
			
		}
		else // ssp responder
		{
			_LMssp_Init_DHKey_Calculation(p_link);
			LMssp_Send_LMP_Accepted(p_link, LMP_ENCAPSULATED_PAYLOAD, p_link->ssp_tid);
			p_link->auth_sub_state = W4_LMP_ACK_PUBLIC_KEY_HEADER;
			_LMspp_Send_LMP_Encapsulated_Header(p_link,MASTER);
		}
	}
	return NO_ERROR;
}

t_error LMspp_Send_LMP_Encapsulated_Payload(t_lmp_link* p_link,t_role tid_role)
{
	u_int8 offset;
    t_lmp_pdu_info pdu_info;
	u_int8 temp_pdu_array[16];
	u_int8 i;
    
	pdu_info.tid_role = p_link->ssp_tid; /* ROLE_TID | PEER_ROLE_TID */
    pdu_info.opcode = LMP_ENCAPSULATED_PAYLOAD;

	offset = 0x30 - g_LM_config_info.Outgoing_Encapsulated_P192_len_pending;
	if (offset == 0)
	{
		for (i=0;i<16;i++)
		{
			temp_pdu_array[i] = g_LM_config_info.public_key_x[23-i] & 0xFF;
		}
		pdu_info.ptr.p_uint8 = temp_pdu_array;

	}
	else if (offset == 16)
	{
		for (i=0;i<8;i++)
		{
            temp_pdu_array[i] = g_LM_config_info.public_key_x[7-i] & 0xFF;
		}

		for (i=0;i<8;i++)
		{
            temp_pdu_array[i+8] = g_LM_config_info.public_key_y[23-i] & 0xFF;
		}

		pdu_info.ptr.p_uint8 = temp_pdu_array;
	}
	else if (offset == 32)
	{
		for (i=0;i<16;i++)
		{
			temp_pdu_array[i] = g_LM_config_info.public_key_y[15-i] & 0xFF;
		}

		pdu_info.ptr.p_uint8 = temp_pdu_array;
	}
	if (offset != 48)
	{
		g_LM_config_info.Outgoing_Encapsulated_P192_len_pending-=16;
		LM_Encode_LMP_PDU(p_link->device_index, &pdu_info);
	}

	return NO_ERROR;
}

static t_error _LMspp_Send_LMP_Encapsulated_Header(t_lmp_link* p_link,t_role tid_role)
{
    t_lmp_pdu_info pdu_info;
	u_int8 ensapsulated_header[3] = {1,1,48};
    
	pdu_info.tid_role = p_link->ssp_tid; /* ROLE_TID | PEER_ROLE_TID */
    pdu_info.opcode = LMP_ENCAPSULATED_HEADER;
	pdu_info.ptr.p_uint8 = ensapsulated_header;

	g_LM_config_info.Outgoing_Encapsulated_P192_len_pending = 48;

    LM_Encode_LMP_PDU(p_link->device_index, &pdu_info);

	return NO_ERROR;
}


void LMssp_Send_LMP_Accepted(t_lmp_link* p_link, u_int8 opcode,u_int8 tid_role)
{
   t_lmp_pdu_info pdu_info;

   pdu_info.tid_role = p_link->ssp_tid; /* ROLE_TID | PEER_ROLE_TID */
   pdu_info.opcode = LMP_ACCEPTED;
   pdu_info.return_opcode = opcode ;
   LM_Encode_LMP_PDU(p_link->device_index,&pdu_info);
}


void LMssp_Send_LMP_Not_Accepted(t_lmp_link* p_link, u_int8 opcode,u_int8 tid_role,t_error reason)
{
   t_lmp_pdu_info pdu_info;

   pdu_info.tid_role = p_link->ssp_tid; /* ROLE_TID | PEER_ROLE_TID */
   pdu_info.opcode = LMP_NOT_ACCEPTED;
   pdu_info.reason = reason;
   pdu_info.return_opcode = opcode ;
   LM_Encode_LMP_PDU(p_link->device_index,&pdu_info);
}



t_error _LMssp_Init_DHKey_Calculation(t_lmp_link* p_link)
{
    // The DH Key calculation is done by partitioning the Scalar calculation into 192 steps.
	// The Algorithm is called and when one iteration through the loop is complete an event 

    boolean blocking = FALSE;
	LMecc_Generate_ECC_Key(g_LM_config_info.secret_key,p_link->peer_public_key_x,
					       p_link->peer_public_key_y,p_link,blocking);

	return NO_ERROR;
}

t_error LMssp_Send_LMP_Simple_Pairing_Confirm(t_lmp_link* p_link,u_int8* commitment)
{
    t_lmp_pdu_info pdu_info;
	u_int8 temp_pdu_info[16];
	u_int8 i;
    
	pdu_info.tid_role =  p_link->ssp_tid;
    pdu_info.opcode = LMP_SIMPLE_PAIRING_CONFIRM;
	for (i=0;i<16;i++)
		temp_pdu_info[i] = commitment[15-i];

	pdu_info.ptr.p_uint8 = temp_pdu_info;
    LM_Encode_LMP_PDU(p_link->device_index, &pdu_info);

	return NO_ERROR;
}

t_error LMssp_Send_LMP_Simple_Pairing_Number(t_lmp_link* p_link)
{
    t_lmp_pdu_info pdu_info;
	u_int8 temp_pdu_info[16];
	u_int8 i;
    
	for (i=0;i<16;i++)
		temp_pdu_info[i] = p_link->rand[15-i];

    pdu_info.tid_role = p_link->ssp_tid;
    pdu_info.opcode = LMP_SIMPLE_PAIRING_NUMBER;
	pdu_info.ptr.p_uint8 = temp_pdu_info;
    LM_Encode_LMP_PDU(p_link->device_index, &pdu_info);

	return NO_ERROR;
}

static t_error _LMssp_Send_LMP_DH_Key_Check(t_lmp_link* p_link)
{
    t_lmp_pdu_info pdu_info;
	u_int8 temp_pdu_info[16];
	u_int8 i;

    // First Calculate the DHKey Check

	LMssp_Calculate_DHKEY_Check(p_link,SSP_LOCAL);

    pdu_info.tid_role = p_link->ssp_tid;
    pdu_info.opcode = LMP_DHKEY_CHECK;

	for(i=0;i<16;i++)
		temp_pdu_info[i] = p_link->DHkeyCheck[15-i];

	pdu_info.ptr.p_uint8 = temp_pdu_info;

	p_link->auth_sub_state = W4_LMP_ACK_DHKEY_CHECK;

    LM_Encode_LMP_PDU(p_link->device_index, &pdu_info);

	return NO_ERROR;
}


t_error LMssp_LMP_Simple_Pairing_Confirm(t_lmp_link* p_link, t_p_pdu p_pdu)
{
	
	u_int8 i;
#if (PRH_BS_CFG_SYS_SSP_PASSKEY_SUPPORTED==1)
	u_int8 commitment[16];
#endif

	if (p_link->auth_type == NUMERIC_COMPARISON)
	{
		// 
		// Need to determine the states where this message is valid ? What States?
		//
		if ((p_link->ssp_initiator) && (p_link->auth_sub_state == W4_LMP_SIMPLE_PAIRING_CONFIRM))
		{
			for (i=0;i<16;i++)
				p_link->peer_commitment[i] = *(p_pdu+15-i);

			p_link->auth_sub_state = W4_LMP_ACK_SIMPLE_PAIRING_NUMBER;
			SYSrand_Get_Rand_128_Ex((u_int8*)p_link->rand);
			LMssp_Send_LMP_Simple_Pairing_Number(p_link);
		}
		else
		{
			return COMMAND_DISALLOWED;
		}
	}
#if (PRH_BS_CFG_SYS_SSP_PASSKEY_SUPPORTED==1)
	else if (p_link->auth_type == PASSKEY_ENTRY)
	{
       // if initiator

		if (p_link->ssp_initiator)
		{
			// Now I have the Simple pairing confirm from the responder.
			for (i=0;i<16;i++)
				p_link->peer_commitment[i] = *(p_pdu+15-i);

			p_link->auth_sub_state = W4_LMP_ACK_SIMPLE_PAIRING_NUMBER;
		//  The Rand was previously calculated on 
			LMssp_Send_LMP_Simple_Pairing_Number(p_link);
		}
		else
		{
			// if Im the Responder then :
			// 1. Save the Peer commitment.
			// IF User has send passKey
			// THEN
			// 2. Calculate local commitment and send to peer
			// 3. Wait for the peers pairing. 
			// ELSE
			//   Wait for user to send passKey.

			p_link->peer_user_authenticated = 0x01;

			for (i=0;i<16;i++)
				p_link->peer_commitment[i] = *(p_pdu+15-i);


			if (p_link->local_user_authenticated == 0x01)
			{
			    SYSrand_Get_Rand_128_Ex((u_int8*)p_link->rand);
				Zero_Array[0] = ((p_link->passKey >> p_link->passKeyCount) & 0x01) + 0x80;

				F1(g_LM_config_info.public_key_x,p_link->peer_public_key_x,
					p_link->rand,Zero_Array,commitment);
				LMssp_Send_LMP_Simple_Pairing_Confirm(p_link,commitment);
				p_link->auth_sub_state = W4_LMP_SIMPLE_PAIRING_NUMBER;
			}

		}
	}
#endif
	else 
	{
		return COMMAND_DISALLOWED;
	}
	return NO_ERROR;
}

t_error LMssp_LMP_Simple_Pairing_Number(t_lmp_link* p_link, t_p_pdu p_pdu)
{
	u_int8 i;
	t_lm_event_info event_info;
	u_int32 user_confirm_value = 0x00;
	u_int32 hexNumber;
	u_int8 OutPut[4];
	u_int8 commitment_b[16];


	for (i=0;i<16;i++)
		p_link->peer_random[i] = *(p_pdu+15-i);

	if (p_link->auth_type != OUT_OF_BAND)
	{
		// In the responder we have to check the state = W4_LMP_SIMPLE_PAIRING_NUMBER
		if ((!p_link->ssp_initiator) && (p_link->auth_sub_state == W4_LMP_SIMPLE_PAIRING_NUMBER))
		{
			if (p_link->auth_type == NUMERIC_COMPARISON)
			{
				LMssp_Send_LMP_Accepted(p_link,LMP_SIMPLE_PAIRING_NUMBER,p_link->ssp_tid);

				// Generate our own simple pairing number !
				p_link->auth_sub_state = W4_LMP_ACK_SIMPLE_PAIRING_NUMBER;
				LMssp_Send_LMP_Simple_Pairing_Number(p_link);
			}
#if (PRH_BS_CFG_SYS_SSP_PASSKEY_SUPPORTED==1)
			else if (p_link->auth_type == PASSKEY_ENTRY)
			{
				Zero_Array[0] = ((p_link->passKey >> p_link->passKeyCount) & 0x01) + 0x80;
				F1(p_link->peer_public_key_x,g_LM_config_info.public_key_x,
					p_link->peer_random,Zero_Array,commitment_b);

				if (TRUE == _Compare_16byte_Arrays(commitment_b,p_link->peer_commitment))
				{
					LMssp_Send_LMP_Accepted(p_link,LMP_SIMPLE_PAIRING_NUMBER,p_link->ssp_tid);

					// Generate our own simple pairing number !
					p_link->auth_sub_state = W4_LMP_ACK_SIMPLE_PAIRING_NUMBER;
					LMssp_Send_LMP_Simple_Pairing_Number(p_link);
				}
				else
				{
  					LMssp_Send_LMP_Not_Accepted(p_link,LMP_SIMPLE_PAIRING_NUMBER,p_link->ssp_tid,AUTHENTICATION_FAILURE);

					_LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);
				}
			}
#endif

		} 
		// In the initiator - we take the random number and use it to calculate the commitment value
		// we check if it matchs the commitment value from the peer previously recieved in the 
		// LMP_SIMPLE_PAIRING_CONFIRM
		else if ((p_link->auth_sub_state == W4_LMP_SIMPLE_PAIRING_NUMBER) /*&& (p_link->ssp_initiator == 0x01)*/)
		{
			// First calculate the commitment value and compare with what was Rxed.
			//  Cb = f1(PKbx, PKax, Nb, 0)
			// void F1(u_int8 *U,u_int8 *V, u_int8 *X, u_int8 *Z,u_int8* OutPut);

			if (p_link->auth_type == NUMERIC_COMPARISON)
			{
				Zero_Array[0] = 0x00;
				F1(p_link->peer_public_key_x,g_LM_config_info.public_key_x,
					p_link->peer_random,Zero_Array,commitment_b);
			}
#if (PRH_BS_CFG_SYS_SSP_PASSKEY_SUPPORTED==1)
			else if (p_link->auth_type == PASSKEY_ENTRY)
			{
				Zero_Array[0] = ((p_link->passKey >> p_link->passKeyCount) & 0x01) + 0x80;
				F1(p_link->peer_public_key_x,g_LM_config_info.public_key_x,
					p_link->peer_random,Zero_Array,commitment_b);
			}
#endif
			if (TRUE == _Compare_16byte_Arrays(commitment_b,p_link->peer_commitment))
			{
				LMssp_Send_LMP_Accepted(p_link,LMP_SIMPLE_PAIRING_NUMBER,p_link->ssp_tid);

				// Now have to calculate the "User Confirm Value", using G(...).
				// Then send HCI_User_Confirmation_Request using the value output from G.

				if (p_link->auth_type == NUMERIC_COMPARISON)
				{
					G(g_LM_config_info.public_key_x, p_link->peer_public_key_x,
						p_link->rand,p_link->peer_random,OutPut);

					hexNumber = (OutPut[0] * 0x01000000) + (OutPut[1] * 0x010000) + (OutPut[2] * 0x0100) + (OutPut[3] * 0x01);
					user_confirm_value = hexNumber % 0xF4240;

					event_info.p_bd_addr = &p_link->bd_addr;
					event_info.numeric_value = user_confirm_value;
					HCeg_Generate_Event(HCI_USER_CONFIRMATION_REQUEST_EVENT,&event_info);
					p_link->ssp_hci_timer_index = LMtmr_Set_Timer(PRH_LMP_MSG_TIMEOUT,LMssp_HCI_Timeout,p_link,1);
					p_link->auth_sub_state = W4_HCI_USER_CONFIRMATION_REQUEST_REPLY;
				}
#if (PRH_BS_CFG_SYS_SSP_PASSKEY_SUPPORTED==1)
				else if (p_link->auth_type == PASSKEY_ENTRY)
				{
					// If I am the initiator ! We can now safely increment the p_link->passKeyCount.
					// In the responder this action is performed on the LMP_Accepted (Pairing_Number)
					if (p_link->ssp_initiator)
					{
						p_link->passKeyCount++;

						Zero_Array[0] = ((p_link->passKey >> p_link->passKeyCount) & 0x01) + 0x80;

						if (p_link->passKeyCount != 20)
						{
							//  Generate New Random - and calculate new commitment value. 
							//  The new commitment is then sent to the peer.

							SYSrand_Get_Rand_128_Ex((u_int8*)p_link->rand);

							if (p_link->ssp_initiator)
							{
								u_int8 commitment[16];

								F1(g_LM_config_info.public_key_x,p_link->peer_public_key_x,
									p_link->rand,Zero_Array,commitment);
								LMssp_Send_LMP_Simple_Pairing_Confirm(p_link,commitment);
								p_link->auth_sub_state = W4_LMP_SIMPLE_PAIRING_CONFIRM;
							}
						}
						else
						{

							// At the end of the PASSKEY simple pairing confirm/number exchange/
							// Initiate the DHkeyCheck.
							p_link->local_user_authenticated = 0x01;
							if (p_link->DHkeyCalculationComplete)
							{
								_LMssp_Send_LMP_DH_Key_Check(p_link);
								p_link->auth_sub_state = W4_LMP_ACK_DHKEY_CHECK;
							}

						}
					}
				}
#endif
			}
			else // Commitment comparison Failure. 
			{
				LMssp_Send_LMP_Not_Accepted(p_link,LMP_SIMPLE_PAIRING_NUMBER,p_link->ssp_tid,AUTHENTICATION_FAILURE);

				_LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);
			}
		}
		else
		{
			return COMMAND_DISALLOWED;
		}
	}
#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)
	else if (p_link->auth_type == OUT_OF_BAND)
	{
		if (p_link->ssp_initiator == 0x01)
		{
			if ((p_link->local_oob_verified == 1) || (g_LM_config_info.oob_data_present == 0x00))
			{
				LMssp_Send_LMP_Accepted(p_link,LMP_SIMPLE_PAIRING_NUMBER,p_link->ssp_tid);

				// Move on to the DHkeyCheck.
				if (p_link->DHkeyCalculationComplete)
				{
					_LMssp_Send_LMP_DH_Key_Check(p_link);
					p_link->auth_sub_state = W4_LMP_ACK_DHKEY_CHECK;
				}
			}
			else if (p_link->local_oob_verified == 2)
			{
				LMssp_Send_LMP_Not_Accepted(p_link,LMP_SIMPLE_PAIRING_NUMBER,p_link->ssp_tid,AUTHENTICATION_FAILURE);
				_LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);
			}
		}
		else
		{
			p_link->rxed_simple_pairing_number = 0x01;

			if (p_link->local_oob_verified == 1)
			{
				LMssp_Send_LMP_Accepted(p_link,LMP_SIMPLE_PAIRING_NUMBER,p_link->ssp_tid);
				SYSrand_Get_Rand_128_Ex((u_int8*)p_link->rand);
				LMssp_Send_LMP_Simple_Pairing_Number(p_link);
				p_link->auth_sub_state = W4_LMP_ACK_SIMPLE_PAIRING_NUMBER;

			}
			else if (p_link->local_oob_verified == 2)
			{
				LMssp_Send_LMP_Not_Accepted(p_link,LMP_SIMPLE_PAIRING_NUMBER,p_link->ssp_tid,AUTHENTICATION_FAILURE);
				_LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);
			}
		}
	}
#endif
	return NO_ERROR;
}


t_error LMssp_LMP_Numeric_Comparison_Failure(t_lmp_link* p_link, t_p_pdu p_pdu)
{
    _LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);
    
	// Need to sort out states here !!!!

	return NO_ERROR;
}

t_error LMssp_LMP_Accepted(t_lmp_link* p_link, t_p_pdu p_pdu)
{
    u_int8 accepted_opcode;
    
    accepted_opcode = *p_pdu;
 
    switch(accepted_opcode)
    {
    case LMP_ENCAPSULATED_HEADER :

		//if (p_link->auth_sub_state == W4_LMP_ACK_PUBLIC_KEY_HEADER)
		{
			p_link->auth_sub_state = W4_LMP_ACK_PUBLIC_KEY_PAYLOAD;
			// Begin Transmission of the Public Key
			LMspp_Send_LMP_Encapsulated_Payload(p_link,MASTER);
		}
		break;

	case LMP_ENCAPSULATED_PAYLOAD :
		if (p_link->auth_sub_state == W4_LMP_ACK_PUBLIC_KEY_PAYLOAD)
		{
			if (g_LM_config_info.Outgoing_Encapsulated_P192_len_pending!=0)
				LMspp_Send_LMP_Encapsulated_Payload(p_link,MASTER);
			else
			{
				// Got to wait for the public key from the peer.
				if (p_link->ssp_initiator)
				{
					// UPF33 :- Need to add logic here to support 
					// the Scenario where I have given OOB data to the peer
					// device, BUT I have not recieved any OOB data for peer device
					// from the host.  

					p_link->auth_sub_state = W4_LMP_PUBLIC_KEY_HEADER;
				}
				else // Im the responder
				{
					if (p_link->auth_type == NUMERIC_COMPARISON)
					{
						u_int8 commitment_b[16];

						SYSrand_Get_Rand_128_Ex((u_int8*)p_link->rand);
						F1(g_LM_config_info.public_key_x,p_link->peer_public_key_x,
						p_link->rand,Zero_Array,commitment_b);
						LMssp_Send_LMP_Simple_Pairing_Confirm(p_link,commitment_b);
						p_link->auth_sub_state = W4_LMP_SIMPLE_PAIRING_NUMBER;
					}
#if (PRH_BS_CFG_SYS_SSP_PASSKEY_SUPPORTED==1)
					else if (p_link->auth_type == PASSKEY_ENTRY)
					{
						// On the responder once I get the accepted to the 
						// final part of Encapsulated Payload, I can begin the passkey entry.


						if(((g_LM_config_info.io_cap == DisplayOnly) ||
							(g_LM_config_info.io_cap == DisplayYesNo)) &&
							(p_link->io_cap ==KeyboardOnly))
						{
							// Peer device has a keyboard and local device has display
							// Raise a user Pass Key Notification Event.
							t_lm_event_info event_info;

							// Need to generate a Pass Key number.

							event_info.p_bd_addr = &p_link->bd_addr;
							p_link->passKey = ((SYSrand_Get_Rand()) % 0xF4240);
							event_info.numeric_value = p_link->passKey;
							p_link->passKeyCount = 0;
							p_link->local_user_authenticated = 0x01;
							HCeg_Generate_Event(HCI_USER_PASSKEY_NOTIFICATION_EVENT,&event_info);

					        // Wait for the Simple Pairing Confirm from the peer.

							// UPF issue here !!
							// We were not generating a new random number here 

							SYSrand_Get_Rand_128_Ex((u_int8*)p_link->rand);

						}
						else if ((g_LM_config_info.io_cap == KeyboardOnly) &&
							((p_link->io_cap == DisplayYesNo) ||
							(p_link->io_cap == KeyboardOnly) || (p_link->io_cap == DisplayOnly)))
						{
							t_lm_event_info event_info;

							// Need to generate a Pass Key number.

						// Raise a User Passkey Request Event. 
							event_info.p_bd_addr = &p_link->bd_addr;
							HCeg_Generate_Event(HCI_USER_PASSKEY_REQUEST_EVENT,&event_info);
				            p_link->ssp_hci_timer_index = LMtmr_Set_Timer(PRH_LMP_MSG_TIMEOUT,LMssp_HCI_Timeout,p_link,1);
							p_link->auth_sub_state = W4_HCI_KEY_PRESS_NOTIFICATION;
						}
					}
#endif
#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)
					else if (p_link->auth_type == OUT_OF_BAND) 
					{
						if (g_LM_config_info.oob_data_present == 0x01)
						{
						t_lm_event_info event_info;

						// Need to generate a Pass Key number.

							// Raise a OOB Data Request Event. 
						event_info.p_bd_addr = &p_link->bd_addr;
						HCeg_Generate_Event(HCI_REMOTE_OOB_DATA_REQUEST_EVENT,&event_info);

						p_link->auth_sub_state = W4_HCI_REMOTE_OOB_DATA_REQUEST_REPLY;
						}
						else
						{
							// No OOB from the remote device.
							int i;

							for(i=0;i<16;i++)
							{
								p_link->peer_commitment[i] = 0x00;
								p_link->Rpeer[i] = 0x00;
								Zero_Array[i] = 0x00;
							}

							// Calculate Cb - the peers commitment Cb = F1 (Pkb,Pkb,rb,0);

							// Post UPF change :
							// I copy the calculated commitment directly into the 
							// p_link->peer_commitment 
							F1(p_link->peer_public_key_x,p_link->peer_public_key_x,
								p_link->Rpeer,Zero_Array,p_link->peer_commitment);


							p_link->local_oob_verified = 0x01; 

							if (p_link->oob_data_present == 0x00)
							{
								for(i=0;i<16;i++)
									g_LM_config_info.randomizer_R[i] = 0x00;
							}

							if (p_link->ssp_initiator==0x01)
							{
								// The Initiator always sends the LMP_SIMPLE_PAIRING_NUMBER. As this gives the 
								// peer a chance to reject it and indicate failure.
								SYSrand_Get_Rand_128_Ex((u_int8*)p_link->rand);
								LMssp_Send_LMP_Simple_Pairing_Number(p_link);
								p_link->auth_sub_state = W4_LMP_ACK_SIMPLE_PAIRING_NUMBER;
							}
							else
							{   // If Im the responder there can be two cases :

								if (p_link->rxed_simple_pairing_number)
								{
									if (p_link->local_oob_verified == 1)
									{
										LMssp_Send_LMP_Accepted(p_link,LMP_SIMPLE_PAIRING_NUMBER,p_link->ssp_tid);
										SYSrand_Get_Rand_128_Ex((u_int8*)p_link->rand);
										LMssp_Send_LMP_Simple_Pairing_Number(p_link);
										p_link->auth_sub_state = W4_LMP_ACK_SIMPLE_PAIRING_NUMBER;
									}
									else if (p_link->local_oob_verified == 2)
									{
										LMssp_Send_LMP_Not_Accepted(p_link,LMP_SIMPLE_PAIRING_NUMBER,p_link->ssp_tid,AUTHENTICATION_FAILURE);
										_LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);
									}
								}
							}
						}
					}
#endif
				}
			}
		}
		break;

	case LMP_SIMPLE_PAIRING_NUMBER :
		if (p_link->auth_sub_state == W4_LMP_ACK_SIMPLE_PAIRING_NUMBER)
		{
			if (p_link->ssp_initiator)
				p_link->auth_sub_state = W4_LMP_SIMPLE_PAIRING_NUMBER;
			else
			{
				if (p_link->auth_type == NUMERIC_COMPARISON)
				{
					t_lm_event_info event_info;
					u_int32 user_confirm_value = 0x00;
					u_int32 hexNumber;
					u_int8 OutPut[4];

					G(p_link->peer_public_key_x,g_LM_config_info.public_key_x,
						p_link->peer_random,p_link->rand,OutPut);

					hexNumber = (OutPut[0] * 0x01000000) + (OutPut[1] * 0x010000) + (OutPut[2] * 0x0100) + (OutPut[3] * 0x01);
					user_confirm_value = hexNumber % 0xF4240;

					event_info.p_bd_addr = &p_link->bd_addr;
					event_info.numeric_value = user_confirm_value;
					HCeg_Generate_Event(HCI_USER_CONFIRMATION_REQUEST_EVENT,&event_info);
					p_link->ssp_hci_timer_index = LMtmr_Set_Timer(PRH_LMP_MSG_TIMEOUT,LMssp_HCI_Timeout,p_link,1);
					p_link->auth_sub_state = W4_HCI_USER_CONFIRMATION_REQUEST_REPLY;
				}
#if (PRH_BS_CFG_SYS_SSP_PASSKEY_SUPPORTED==1)
				else if (p_link->auth_type == PASSKEY_ENTRY)
				{
					p_link->passKeyCount++;

					Zero_Array[0] = ((p_link->passKey >> p_link->passKeyCount) & 0x01) + 0x80;

					if (p_link->passKeyCount != 20)
					{
						//  Generate New Random - and calculate new commitment value. 
						//  The new commitment is then sent to the peer.

						if (p_link->ssp_initiator)
						{
							p_link->auth_sub_state = W4_LMP_SIMPLE_PAIRING_CONFIRM;
						}
					}
					else
					{
						p_link->auth_sub_state = W4_LMP_PEER_DHKEY_CHECK;

					}
				}
#endif
#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)
				else if (p_link->auth_type == OUT_OF_BAND)
				{
					p_link->auth_sub_state = W4_LMP_PEER_DHKEY_CHECK;
				}
#endif
			}

		}		
		break;

	case LMP_DHKEY_CHECK :
		if (p_link->auth_sub_state ==  W4_LMP_ACK_DHKEY_CHECK)
		{
			if (p_link->ssp_initiator)
			{
				if (!p_link->peer_user_authenticated)
					p_link->auth_sub_state = W4_LMP_PEER_DHKEY_CHECK;
			
			}
			else
			{
				// Raise the Simple pairing Complete event at the HCI.

				if (((p_link->peer_user_authenticated==0x01) && (p_link->local_user_authenticated)) ||
					(p_link->auth_type == OUT_OF_BAND))
				{
					_LMssp_Send_Simple_Pairing_Complete_Event(p_link,NO_ERROR);

					
					_LMssp_Calculate_Link_Key(p_link);

					_LMssp_Mutual_Authentication(p_link);

					p_link->ssp_initiator = 0x00;
				}

			}
		}
		break;
	}

	return NO_ERROR;
 
}


t_error LMssp_LMP_Not_Accepted(t_lmp_link* p_link, u_int16 return_opcode, t_error reason)
{
    switch(return_opcode)
    {
		// NOTE :- Need to add more logic here, we dont always want a LMP_NOT_ACCEPTED from the peer to
		//         result in use exiting simple pairing.
    case LMP_ENCAPSULATED_HEADER :
	case LMP_ENCAPSULATED_PAYLOAD :
	case LMP_SIMPLE_PAIRING_NUMBER :
	case LMP_SIMPLE_PAIRING_CONFIRM :
	case LMP_DHKEY_CHECK :
	case LMP_IO_CAPABILITY_RESPONSE :
        _LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);
		break;

	case LMP_IO_CAPABILITY_REQUEST :
		// If we get a not accepted with a transaction collision to the LMP_IO_CAPABILITY_REQUEST
		if ((reason == LMP_ERROR_TRANSACTION_COLLISION) && (p_link->role == SLAVE))
		{
			p_link->ssp_initiator = 0x00;
			p_link->ssp_tid = 0x00;
		}
		else
		{
		    _LMssp_Send_Simple_Pairing_Complete_Event(p_link,AUTHENTICATION_FAILURE);
		}
		break;
	}
	return NO_ERROR;

}
static boolean _Compare_16byte_Arrays(u_int8* array1, u_int8* array2)
{
    int i=0;

    do
    {
        if( array1[i] != array2[i] )
            return FALSE;
        i++;
    } while (i <16);

    return TRUE;  
}


static void _LMssp_Mutual_Authentication(t_lmp_link* p_link)
{	
	if (p_link->ssp_initiator)
	{
		SET_SUPER_STATE_PROC(p_link->auth_super_state,AUTHENTICATION_PROC);
		SET_SUPER_STATE_PROC(p_link->auth_super_state, ORIG_PAIRING_PROC);
		p_link->auth_sub_state = SUB_STATE_IDLE;
		p_link->link_key_exists = TRUE;

		// Must test to ensure that the below Auth_Challenge only need to be done when 
		// we are initiator. At moment done for initiator and responder.
		LMsec_core_Auth_Challenge(p_link,(u_int8)p_link->role);
	}
	else
	{
		//p_link->sec_current_proc_tid_role
//		p_link->auth_super_state = HCI_AUTHENTICATION;
		SET_SUPER_STATE_PROC(p_link->auth_super_state,AUTHENTICATION_PROC);
		SET_SUPER_STATE_PROC(p_link->auth_super_state, TERM_PAIRING_PROC);
		p_link->auth_sub_state = SUB_STATE_IDLE;
		p_link->link_key_exists = TRUE;
	}

}


void _LMssp_Determine_Auth_Type(t_lmp_link* p_link)
{
/***************************************************************************************
 * Further Expand using GAP Table 5.6: IO Capability Mapping to Authentication Stage 1
 ***************************************************************************************/
	// determine the type of authentication to be used
#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)
	// If OOB data is sent or recieved then we use OOB.
	if(g_LM_config_info.oob_data_present || p_link->oob_data_present)
	{
		// set auth type to OUT OF BAND
		p_link->auth_type = OUT_OF_BAND;
	}
	// If the LSB of authentication requirements is 0x00 then we have to use Numeric comparison.
	// If the LSB of authentication requirements is 0x01 then we determine using the IO Cap.

	else
#endif	
	if ((!(g_LM_config_info.auth_requirements & 0x01)) && (!(p_link->auth_requirements & 0x01)))
	{
		// numeric comparison can be used
		/* Second, if neither device has received OOB authentication data and if both
		   devices have set the Authentication_Requirements parameter to one of the
           MITM Protection Not Required options, authentication stage 1 shall function as
           if both devices set their IO capabilities to DisplayOnly (e.g. Numeric comparison
           with automatic confirmation on both devices).
*/
		p_link->auth_type = NUMERIC_COMPARISON;
	}
	else if (((g_LM_config_info.auth_requirements & 0x01)) || ((p_link->auth_requirements & 0x01)))
	{
		// if Local_IO_Cap = Keyboard Only & Remote_IO_Cap = NoInputNoOutPut

		// Was Bug here missmatch from table in GAP
#if (PRH_BS_CFG_SYS_SSP_PASSKEY_SUPPORTED==1)
		if(((g_LM_config_info.io_cap == KeyboardOnly) && (p_link->io_cap != NoInputNoOutput)) ||
		  ((g_LM_config_info.io_cap != NoInputNoOutput) && (p_link->io_cap == KeyboardOnly)))
		{
			p_link->auth_type = PASSKEY_ENTRY;
		}
		else
#endif
		{
			// Note :- If both local and remote have KeyboardOnly CSR thinks it is 
			// a PASSKEY_ENTRY. Is this a CSR Issue ???. 
			// 
			p_link->auth_type = NUMERIC_COMPARISON;
		}
	}
	else
	{
		p_link->auth_type = NUMERIC_COMPARISON;
	}
}


static t_error _LMssp_Calculate_Link_Key(t_lmp_link* p_link)
{

	u_int8 keyId[4] = {0x62,0x74,0x6c,0x6b};
	u_int8 Address1[6];
	u_int8 Address2[6];
	u_int8 i;
	u_int8 link_key[16];

	for (i=0;i<6;i++)
	{
	Address1[i] = p_link->bd_addr.bytes[5-i];
	Address2[i] = (SYSconfig_Get_Local_BD_Addr_Ref()->bytes)[5-i];
	}

	if (p_link->role == MASTER)
		F2(p_link->DHkey,p_link->rand,p_link->peer_random,keyId,Address2,Address1,link_key);
	else
		F2(p_link->DHkey,p_link->peer_random,p_link->rand,keyId,Address1,Address2,link_key);

	for (i=0;i<16;i++)
		p_link->link_key[i] = link_key[15-i];

	p_link->link_key_exists = TRUE;

	// Determine association model used 
	// 
	if ((g_LM_config_info.ssp_debug_mode) || (p_link->ssp_debug_mode))
	{
		// In SSP Debug Mode
		p_link->key_type = DEBUG_COMBINATION_KEY;
	}
	else if ((p_link->io_cap == NoInputNoOutput) ||
		(g_LM_config_info.io_cap == NoInputNoOutput))
	{
		// "Just Works" association model
		p_link->key_type = UNAUTHENTICATED_COMBINATION_KEY;
	}
	else 
	{
		// Simple pairing and NOT "Just Works" association model 
		p_link->key_type = AUTHENTICATED_COMBINATION_KEY;
	}

	return NO_ERROR;
}

//GF changed this function for an issue found at UPF42.
void LMssp_HCI_Timeout(t_lmp_link* p_link) {
	switch(p_link->auth_sub_state)
	{
	case W4_HCI_USER_CONFIRMATION_REQUEST_REPLY :
		LMssp_LM_User_Confirmation_Request_Negative_Reply(p_link);
		break;

	case W4_HCI_KEY_PRESS_NOTIFICATION : // Passkey
		LMssp_LM_User_Passkey_Request_Negative_Reply(p_link);
		break;

	case W4_HCI_IO_CAP_REQUEST_REPLY :
		LMssp_LM_IO_Capability_Request_Negative_Reply(p_link,UNSPECIFIED_ERROR);
		break;

	case W4_HCI_REMOTE_OOB_DATA_REQUEST_REPLY :
		LMssp_LM_Remote_OOB_Data_Request_Negative_Reply(p_link);
		break;

	default:
		break;
	}
}


void LMssp_Calculate_DHKEY_Check(t_lmp_link* p_link,u_int8 side)
{
	
	u_int8 io_cap[3];
	int i;
	u_int8 Address_Local[6];
	u_int8 Address_Peer[6];
	const t_bd_addr *my_bdaddr;
	
	my_bdaddr = SYSconfig_Get_Local_BD_Addr_Ref();
	
	for (i=0;i<6;i++)
	{
		Address_Peer[i] = p_link->bd_addr.bytes[5-i];
		Address_Local[i] = my_bdaddr->bytes[5-i];
	}

	for (i=0;i<16;i++)
		Zero_Array[i] = 0x00;


	if (side == SSP_PEER)
	{
		io_cap[2] = p_link->io_cap;
#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)
		io_cap[1] = p_link->oob_data_present;
#else
		io_cap[1] = 0;
#endif
		io_cap[0] = p_link->auth_requirements;

		if (p_link->auth_type == NUMERIC_COMPARISON)
		{
			F3(p_link->DHkey,p_link->peer_random,p_link->rand,Zero_Array,io_cap,
				Address_Peer,Address_Local,p_link->DHkeyCheck);
		}
#if (PRH_BS_CFG_SYS_SSP_PASSKEY_SUPPORTED==1)
		else if (p_link->auth_type == OUT_OF_BAND)
		{
			F3(p_link->DHkey,p_link->peer_random,p_link->rand,g_LM_config_info.randomizer_R,
			io_cap,Address_Peer,Address_Local,p_link->DHkeyCheck);
		}
#endif
#if (PRH_BS_CFG_SYS_SSP_PASSKEY_SUPPORTED==1)
		else if (p_link->auth_type == PASSKEY_ENTRY)
		{
			u_int8 Ra[16];

			for (i=0;i<16;i++)
			{
					Ra[i] = 0;
			}
			Ra[15] = (u_int8)(p_link->passKey  & 0x000000FF);
			Ra[14] = (u_int8)((p_link->passKey & 0x0000FF00)>>8);
			Ra[13] = (u_int8)((p_link->passKey & 0x00FF0000)>>16);
			Ra[12] = (u_int8)((p_link->passKey & 0xFF000000)>>24);
			F3(p_link->DHkey,p_link->peer_random,p_link->rand,Ra,io_cap,
			Address_Peer,Address_Local,p_link->DHkeyCheck);
		}
#endif
	}
	else
	{
		io_cap[2] = g_LM_config_info.io_cap;
#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)
		io_cap[1] = g_LM_config_info.oob_data_present;
#else
		io_cap[1] = 0;
#endif
		io_cap[0] = g_LM_config_info.auth_requirements;

		for (i=0;i<16;i++)
			Zero_Array[i] = 0x00;

		if (p_link->auth_type == NUMERIC_COMPARISON)
		{
			F3(p_link->DHkey,p_link->rand,p_link->peer_random,Zero_Array,io_cap,Address_Local,Address_Peer,p_link->DHkeyCheck);
		}
#if (PRH_BS_CFG_SYS_SSP_PASSKEY_SUPPORTED==1)
		else if (p_link->auth_type == PASSKEY_ENTRY)
		{
			Zero_Array[15] = (u_int8)(p_link->passKey  & 0x000000FF);
			Zero_Array[14] = (u_int8)((p_link->passKey & 0x0000FF00)>>8);
			Zero_Array[13] = (u_int8)((p_link->passKey & 0x00FF0000)>>16);
			Zero_Array[12] = (u_int8)((p_link->passKey & 0xFF000000)>>24);

			F3(p_link->DHkey,p_link->rand,p_link->peer_random,Zero_Array,io_cap,Address_Local,Address_Peer,p_link->DHkeyCheck); 
		}
#endif
#if (PRH_BS_CFG_SYS_SSP_OOB_SUPPORTED==1)
		else if (p_link->auth_type == OUT_OF_BAND)
		{
			F3(p_link->DHkey,p_link->rand,p_link->peer_random,p_link->Rpeer,
				io_cap,Address_Local,Address_Peer,p_link->DHkeyCheck);
		}
#endif
	}
}

#else


#endif
