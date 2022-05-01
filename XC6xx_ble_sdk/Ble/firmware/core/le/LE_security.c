/**************************************************************************
 * MODULE NAME:    le_security.c       
 * DESCRIPTION:    LE Security Engine
 * AUTHOR:         Gary Fleming & Nicola Lenihan
 * DATE:           5-Jan-2012
 *
 * SOURCE CONTROL: 
 *
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *
 * NOTES: This function handles incoming hci encryption related commands 
 *        for LE.
 *
 **************************************************************************/

#include "sys_types.h"
#include "sys_features.h"
#include "sys_config.h"
#include "sys_irq.h"
#include "hc_const.h"
#include "hw_lc_impl.h"
#include "hci_params.h"
#include "sys_rand_num_gen.h"
#include "le_aes.h"
#include "le_connection.h"
#include "le_security.h"
#include "lmp_utils.h"
#include "le_config.h"
#include "le_link_layer.h"
#include "bt_timer.h"

#include "hw_lc.h"
#include "hw_le_lc.h"
#include "hw_memcpy.h"
#include "sys_debug_config.h"
#include "patch_function_id.h"

void _LEsecurity_Link_Key_Request_Timeout (void);

extern t_LE_Config LE_config;

#if (PRH_BS_CFG_SYS_LE_SMP_INCLUDED==0)
/***************************************************************************
 *
 * Function:	LEsecurity_Encrypt
 *
 * Description: This function handles an incoming hci_le_encrypt  
 *				command. The encrypted data is returned as a pointer in the 
 *              parameter list.
 *
 * Role:		Both
 **************************************************************************/
void LEsecurity_Encrypt(t_p_pdu p_pdu, u_int8 *encrypted_data)
{
	u_int8 i;
	u_int8 output_key[16];
	u_int8 key[16];
	u_int8 plaintext_data[16];
	u_int8 inv_encrypted_data[16];

	for (i=0;i<16;i++)
	{
		key[15-i] = p_pdu[i];
	}
	p_pdu+=16;

	for (i=0;i<16;i++)
	{
		plaintext_data[15-i] = p_pdu[i];
	}

	LEsecurity_HW_aes_encrypt_128(plaintext_data, inv_encrypted_data, key, output_key);

	for (i=0;i<16;i++)
	{
		encrypted_data[15-i] = inv_encrypted_data[i];
	}
}
/***************************************************************************
 *
 * Function:	LEsecurity_Rand
 *
 * Description: This function handles an incoming hci_le_random
 *				command.
 *
 * Role:		Both
 **************************************************************************/
void LEsecurity_Rand(u_int8 *random_number_for_host)
{
	SYSrand_Get_Rand_128_Ex(random_number_for_host);

}
#endif

/***************************************************************************
 *
 * Function:	LEsecurity_HW_aes_encrypt_128
 *
 * Description: This function uses the Tabasco HW to perform AES_128.
 *
 * Inputs     : const u_int8* plaintext_data  :- The Plaintext data to be encrypted
                      u_int8* inv_encrypted_data :- The encrypted data in inverted/reversed format.
				const u_int8* key :- The encryption key.
				u_int8* output_key :- Not Used - legacy only. 

                                            
 * Role:		Schedular
 **************************************************************************/

void LEsecurity_HW_aes_encrypt_128(const u_int8* plaintext_data, u_int8* inv_encrypted_data,
								   const u_int8* key,            u_int8* output_key)
{
	if(PATCH_FUN[LESECURITY_HW_AES_ENCRYPT_128_ID]){
         ((void (*)(const u_int8* plaintext_data, u_int8* inv_encrypted_data,const u_int8* key, u_int8* output_key))PATCH_FUN[LESECURITY_HW_AES_ENCRYPT_128_ID])(plaintext_data,inv_encrypted_data,key, output_key);
         return ;
    }


	u_int8 length=16;
	u_int32 u_int32_Temp_buffer[12] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	u_int8* temp = (u_int8*)u_int32_Temp_buffer ;
	u_int32 u_int32_key[4];
	u_int8* temp_key = (u_int8*)u_int32_key;
	u_int8 i;
	u_int32 cpu_flags=0;

	for(i=0;i<16;i++)
		temp_key[i] = key[i];
	/*
	 * NOTE Session Key has to be determined from the SDK and LTK prior to encryption.
	 */
	SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);
	HWle_set_aes_mode(0x01); // AES - encryption
	HWle_set_aes_enable(); // AES-Enable Bit
	SYSirq_Interrupts_Restore_Flags(cpu_flags);

	{
		hw_memcpy32((void*)HWle_get_aes_key_addr(), (void*)temp_key, 16);
	}

	for (i=0;i<length;i++)
		temp[i] = plaintext_data[i];

	// The +4 is to account for the MIC which will be written in the first
	// four bytes of the AES buffer

	hw_memcpy32((void*)((u_int32)(HWle_get_aes_data_addr()+4)),(void*)((u_int32)temp),16);

	SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);
	// Last Thing done on the AES
	HWle_set_aes_start();
	SYSirq_Interrupts_Restore_Flags(cpu_flags);

	while(HWle_get_aes_active())
		;

	while (!HWle_get_aes_finished())
		;

	hw_memcpy32((void*)temp,(void*)((u_int32)(HWle_get_aes_data_addr()+4)),(length));

	SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);
	HWle_clear_aes_enable();
	HWle_set_aes_mode(0x00);
	SYSirq_Interrupts_Restore_Flags(cpu_flags);

	//gf 3 feb Test if had negative side effect 	HWle_set_aes_mode(0x00);
	LE_config.AES_complete = 0x00;


	for(i=0;i<(length);i++)
	{
		inv_encrypted_data[i] = temp[i];
	}
}

#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1)
#if (PRH_BS_CFG_SYS_LE_SMP_INCLUDED!=1)
/***************************************************************************
 *
 * Function:	LEsecurity_Start_Encryption
 *
 * Description: This function handles an incoming hci_le_start_encryption 
 *				command.
 *
 * Role:		Master
 **************************************************************************/
t_error LEsecurity_Start_Encryption(u_int16 handle,u_int8* random,u_int16 encry_div,u_int8* ltk)
{
	if(PATCH_FUN[LESECURITY_START_ENCRYPTION_ID]){
         
         return ((t_error (*)(u_int16 handle,u_int8* random,u_int16 encry_div,u_int8* ltk))PATCH_FUN[LESECURITY_START_ENCRYPTION_ID])(handle, random, encry_div, ltk);
    }

	u_int8 i;
	u_int8 link_id;
	t_LE_Connection* p_connection;
	u_int8 temp_rand_buff[16];

	//Find the link id
	if (NO_ERROR != LEconnection_Find_Link_Id(handle,&link_id))
		return NO_CONNECTION;

	p_connection = LEconnection_Find_Link_Entry(link_id);


	// The command is not allowed as a slave
	if (LEconnection_Get_Role(p_connection) == SLAVE)
	{
		return COMMAND_DISALLOWED;
	}
	
	/*
	 * If there is a start encryption procedure already in the process return
	 * a command disallowed as there can only be one start encryption procedure 
	 * occurring at any time.
	 */
	if (p_connection->ll_action_pending & LL_ENC_REQ_PENDING)
	{
		return COMMAND_DISALLOWED;
	}

	//Store the encryption parameters
	for (i=0; i<8; i++)
	{
		LE_config.random_number[i] = random[i];
	}

	LE_config.encrypt_diversifier = encry_div;
	
	for (i=0; i<16; i++)
	{
		LE_config.long_term_key[i] = ltk[i];
	}

	/*
	 * Get Rands for SKD master and IV master and store them.
	 * These have to be done before the LL_ENC_REQ is sent as they
	 * are part of the payload.
	 */

	SYSrand_Get_Rand_128_Ex(temp_rand_buff);
	
	for(i=0;i<8;i++)
		LE_config.SKD[i] = temp_rand_buff[i];

	p_connection->ll_action_pending |= LL_ENC_REQ_PENDING;

#if 1 // GF - Need review of Data handling in Pause/Resume Encryption..
	  // This flag is used to stop data being sent during encryption and pause encryption procedure.
	p_connection->ll_action_pending |= LL_ENCRYPTION_TRANSITIONING;
#endif

	if (p_connection->current_security_state & LE_ENCRYPTION_ACTIVE)
	{
		for(i=0;i<4;i++)
			p_connection->IVnew[i] = temp_rand_buff[i+8];
		// if link is already encrypted pause encryption
		// send out ll_pause_enc_req and wait for response
		LEllc_Encode_PDU( LL_PAUSE_ENC_REQ, p_connection, 0x00);

		p_connection->current_security_state |= LE_W4_PAUSE_ENC_RSP;
		
	}
	else
	{
		for(i=0;i<4;i++)
			p_connection->IV[i] = temp_rand_buff[i+8];

		// if link is not already encrypted request encryption
		// by sending out ll_enc_req and wait for response
		//p_connection->ll_action_pending |= LL_DATA_TX_STOPPED;
		LEllc_Encode_PDU( LL_ENC_REQ, p_connection, 0x00);
		p_connection->current_security_state |= LE_W4_ENC_RSP;

	}
	p_connection->ll_response_timer = BTtimer_Set_Ticks(LE_LL_RESPONSE_TIMEOUT);
	return NO_ERROR;
}

#endif

/***************************************************************************
 *
 * Function:	LEsecurity_Long_Term_Key_Request_Reply
 *
 * Description: This function handles an incoming 
 *				hci_le_long_term_key_request_reply command.
 *
 * Role:		Slave
 **************************************************************************/
t_error LEsecurity_Long_Term_Key_Request_Reply(t_LE_Connection* p_connection,t_p_pdu p_pdu)
{
	
	if(PATCH_FUN[LESECURITY_LONG_TERM_KEY_REQUEST_REPLY_ID]){
         
         return ((t_error (*)(t_LE_Connection* p_connection,t_p_pdu p_pdu))PATCH_FUN[LESECURITY_LONG_TERM_KEY_REQUEST_REPLY_ID])(p_connection,  p_pdu);
    }

	// Validate that the connection handle is correct
	u_int8 i;
	u_int8 dummy_key[16];

	/* 
	 * This ll pdu is only accepted when in state LE_W4_LTK_REPLY.
	 */
	if ( !(p_connection->current_security_state & LE_W4_LTK_REPLY))
	{
		return COMMAND_DISALLOWED;
	}

	LE_config.link_key_request_timer = BT_TIMER_OFF;

	for (i=0; i<16; i++)
	{
		LE_config.long_term_key[i] = p_pdu[i];
	}

	/*
	 * Now Determine the Session Key
	 * This is done using the SKD as plaintext input to AES with the LTK as the Key
	 */
	/*
	 *
	 * The most significant octet of key corresponds to key[0], the most significant
	 * octet of plaintextData corresponds to in[0] and the most significant octet of
	 * encryptedData corresponds to out[0] using the notation specified in [1].
	 */
{
	u_int8 inverted_SKD[16];

	u_int8 inverted_LTK[16];
	for(i=0;i<16;i++)
	{
		inverted_SKD[15-i] = LE_config.SKD[i];
		inverted_LTK[15-i] = LE_config.long_term_key[i];
	}
	LEsecurity_HW_aes_encrypt_128(inverted_SKD,p_connection->session_key, inverted_LTK,dummy_key);

}

	// Send llc_start_enc_req to master
	LEllc_Encode_PDU(LL_START_ENC_REQ,p_connection,0);
		
	/*
	 * Set timer as slave waits to receive ll_start_enc_rsp from master
	 */
	p_connection->ll_response_timer = BTtimer_Set_Ticks(LE_LL_RESPONSE_TIMEOUT);

	/*
	 * The current_security_state needs to be updated.
	 * Turn off the flag for LE_W4_LTK_REPLY and turn on the flag for LE_W4_START_ENC_RSP
	 */
	p_connection->current_security_state &= ~LE_W4_LTK_REPLY;
	// Wait to receive the LL_START_ENC_RSP pdu
	p_connection->current_security_state |= LE_W4_START_ENC_RSP;

	return NO_ERROR;
}
/***************************************************************************
 *
 * Function:	LEsecurity_Long_Term_Key_Request_Negative_Reply
 *
 * Description: This function handles an incoming 
 *				hci_le_long_term_key_negative_reply command.
 *
 * Role:		Slave
 **************************************************************************/
t_error LEsecurity_Long_Term_Key_Request_Negative_Reply(t_LE_Connection* p_connection)
{
	if(PATCH_FUN[LESECURITY_LONG_TERM_KEY_REQUEST_NEGATIVE_REPLY_ID]){
         
         return ((t_error (*)(t_LE_Connection* p_connection))PATCH_FUN[LESECURITY_LONG_TERM_KEY_REQUEST_NEGATIVE_REPLY_ID])(p_connection);
    }

	/* 
	 * This ll pdu is only accepted when in state LE_W4_LTK_REPLY.
	 */
	if ( !(p_connection->current_security_state & LE_W4_LTK_REPLY))
	{
		return COMMAND_DISALLOWED;
	}

	LE_config.link_key_request_timer = BT_TIMER_OFF;

	p_connection->ll_action_pending &= ~LL_ENCRYPTION_TRANSITIONING;

	if (p_connection->current_security_state & LE_ENCRYPTION_PAUSED)
	{
		/* 
		 * IF encryption has been paused and the slave does not respond with a
		 * link key, then the slave terminates the link.
		 */
		LEconnection_Disconnect(p_connection,PIN_MISSING);
	}
	else
		LEllc_Encode_PDU(LL_REJECT_IND,p_connection,PIN_MISSING);

	p_connection->ll_action_pending &= ~LL_ENC_REQ_PENDING;
	/*
	 * The current security state needs to be updated.
	 * Turn off the flags for encryption paused and for w4 ltk reply.
	 */
	p_connection->current_security_state &=  (~LE_W4_LTK_REPLY & ~LE_ENCRYPTION_PAUSED);


	return NO_ERROR;
}
/***************************************************************************
 *
 * Function:	LEsecurity_Long_Term_Key_Request_Timeout
 *
 * Description: This function handles a link key request timeout
 *
 * Role:		Slave
 **************************************************************************/
void LEsecurity_Long_Term_Key_Request_Timeout (void)
{
	t_LE_Connection* p_connection; 

	p_connection = LEconnection_Find_Link_Entry(LE_config.active_link);
	LEconnection_Disconnect(p_connection,PIN_MISSING);

	p_connection->ll_action_pending &= (~LL_ENC_REQ_PENDING & ~LL_ENCRYPTION_TRANSITIONING);
	LE_config.link_key_request_timer = BT_TIMER_OFF;
	p_connection->ll_response_timer = BT_TIMER_OFF;

}

#endif






