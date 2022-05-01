/**************************************************************************
 * MODULE NAME:    le_test.c       
 * DESCRIPTION:    LE Test Mode Engine
 * AUTHOR:         Nicola Lenihan
 * DATE:           5-Mar-2012
 *
 * SOURCE CONTROL: 
 *
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2012 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *
 * NOTES: This file handles incoming hci test mode related commands 
 *        for LE. The role of the device is not applicable. There is no 
 *		  connection setup prior to test mode.
 *		  There is no requirements during test mode to support multiple 
 *        states or to return to a certain state on test mode end.
 *
 **************************************************************************/
#include "sys_config.h"
#include "le_test.h"
#include "le_const.h"
#include "le_config.h"
#include "le_link_layer.h"
#include "tc_const.h"
#include "tc_event_gen.h"
#include "hw_lc.h"
#include "hw_le_lc_impl.h"
#include "hw_radio.h"
#include "sys_debug_config.h"
#include "global_val.h"
#include "patch_function_id.h"

typedef struct
{
u_int8 test_freq;
u_int8 len_of_test_data;
t_letestScenario packet_payload_type;
u_int32 num_packets_received;
u_int32 num_packets_received_withCRC;
u_int8 sub_state;
t_p_pdu data_payload;
u_int8 end_flag;
} t_LE_Test;

extern t_LE_Config LE_config;

static t_LE_Test LE_test;

u_int32 u_int32_payload[MAX_TEST_DATA_SIZE];

/*
 * Internal function prototypes
 */
void _LEtest_Build_Test_Packet_Payload(t_letestScenario testScenario, 
                        t_p_pdu txBuf, t_length packetLength);
void _LEtest_Set_Test_Mode_Header(u_int8 type, u_int8 length);
void _LEtest_Prep_For_LE_Test_Tx(void);
#if		(__DEBUG_RX_TEST_MODE_SET_TO_ONE__ == 1)
void _LEtest_Prep_For_LE_Test_Rx(uint32_t  flg);
#else
void _LEtest_Prep_For_LE_Test_Rx(void);
#endif
void _LEtest_Advance_For_Next_Test_Tx(void);
void _LEtest_Test_Event_Complete(void);
void _LEtest_Packet_Send_Complete(void);


const u_int8 TxPatternArray[8] = {0x00,  /* LE_TX_Psuedo_Random_9_bit  */
				                  0x0F, /* LE_TX_11110000_Pattern     */
								  0x55, /* LE_TX_10_Pattern           */
                                  0x00, /* LE_TX_Psuedo_Random_15_bit */
				                  0xFF, /* LE_TX_1_Pattern            */
								  0x00, /* LE_TX_0_Pattern            */
								  0xF0, /* LE_TX_00001111_Pattern     */
								  0xAA /* LE_TX_01_Pattern           */ };

/*u_int32  recv_synced = 0;*/
extern	void		config_radio_rx(unsigned	int	io_Channel);
/***************************************************************************
 *
 * Function:	LEtest_Receiver_Test
 *
 * Description: This function handles an incoming hci_le_receiver_test 
 *				command.
 *
 * Role:		N/A
 **************************************************************************/
t_error LEtest_Receiver_Test(u_int8 rx_freq)
{
	LE_test.num_packets_received = 0x00;
	LE_test.num_packets_received_withCRC = 0x00;
	/*recv_synced = 0;*/
	if (LE_config.test_mode == LE_TEST_MODE_ACTIVE)
	{
		return COMMAND_DISALLOWED;
	}

	if (rx_freq > 0x27)
	{
		return INVALID_HCI_PARAMETERS;
	}
	else
	{
		LE_test.test_freq = rx_freq*2;
		
		config_radio_rx(rx_freq);
	}

	LE_test.end_flag = 0;

	LE_test.sub_state = LE_W4_T2_PRE_RX;

	LE_config.test_mode = LE_TEST_MODE_ACTIVE;
	
	return NO_ERROR;

}
extern	void		config_radio_tx(unsigned	int	io_Channel);
/***************************************************************************
 *
 * Function:	LEtest_Transmitter_Test
 *
 * Description: This function handles an incoming hci_le_transmitter_test 
 *				command.
 *
 * Role:		N/A
 **************************************************************************/
t_error LEtest_Transmitter_Test(u_int8 tx_freq, u_int8 len_of_test_data,
								u_int8 packet_payload_type)
{
	if (LE_config.test_mode == LE_TEST_MODE_ACTIVE)
	{
		return COMMAND_DISALLOWED;
	}

	if ((tx_freq > 0x27) || (len_of_test_data > 0x25) ||
		(packet_payload_type > 0x07))
	{
		return INVALID_HCI_PARAMETERS;
	}

	// Pseudo random bit sequence 15 is not supported.
	if (packet_payload_type == 0x03)
	{
		return UNSUPPORTED_PARAMETER_VALUE;
	}
	
	LE_test.data_payload = (u_int8*)u_int32_payload;

	// UPF 42 - Multiply by 2 to change it to an radio frequency
	LE_test.test_freq = tx_freq*2;

	config_radio_tx(tx_freq);
	LE_test.len_of_test_data = len_of_test_data;
	//LE_test.packet_payload_type = packet_payload_type;
	LE_test.packet_payload_type = (t_letestScenario)packet_payload_type;    

	_LEtest_Build_Test_Packet_Payload(LE_test.packet_payload_type, 
                LE_test.data_payload, LE_test.len_of_test_data);
	
	LE_test.num_packets_received = 0x00;
	LE_test.num_packets_received_withCRC = 0x00;

	LE_test.end_flag = 0;

	LE_test.sub_state = LE_W4_PRE_TEST_TX;

	LE_config.test_mode = LE_TEST_MODE_ACTIVE;  
	
	return NO_ERROR;

}
/***************************************************************************
 *
 * Function:	LEtest_Test_End
 *
 * Description: This function handles an incoming hci_le_test_end
 *				command.
 *
 * Role:		N/A
 **************************************************************************/
u_int16 LEtest_Test_End(void)
{
	*((u_int32 volatile*)0x4002c260) = (u_int32)(0x90001);
	LE_test.end_flag = 1;
	//*((u_int32 volatile*)0x4002c240) = (u_int32)(0x02000303);   //- auto control rx/tx enable.
	#if(1 == __DEBUG_BLE_TRM_Q )
	if(LE_test.num_packets_received > 1500) LE_test.num_packets_received = 1500;
	#endif
	return LE_test.num_packets_received;
}

/***********************************************************************
 * 
 * Function:	LEtest_Ctrl_Handle_Test_Mode_Event
 *
 * Description: This function handles the interrupts requests from the 
 *				microcontroller when the device is in test mode.
 *
 ***********************************************************************/

u_int32 intraslot_offset = 0;
#if(__DUMP_DATA_WHILE == 1)
u_int32	pkd_download_cnt = 0;
#endif

#if(__DUMP_DATA_WHILE_NOT_PKD == 1)
u_int32	pkd_download_cnt1 = 0;
u_int32	pkd_download_start = 0;

#endif

t_error LEtest_Ctrl_Handle_Test_Mode_Event(u_int8 IRQ_Type)
{
	static u_int8 sm_role,have_synced;
	unsigned	int	length;
	switch(LE_test.sub_state)
	{
		case LE_W4_PRE_TEST_TX :

			if (IRQ_Type==LE_TIM_2)
			{
				
				sm_role = 0;
				HW_set_slave(sm_role);
				//HWradio_LE_Setup_Radio_For_Next_TXRX(RX_START);
				//HWradio_LE_Service(RADIO_MODE_LE_TESTMODE_TX, LE_test.test_freq, TX_START_FREQ);
				_LEtest_Prep_For_LE_Test_Tx();
				
				LE_test.sub_state = LE_W4_SECOND_TX;
			}
			break;

		case LE_W4_SECOND_TX :
			if (LE_PKA == IRQ_Type)
			{
				_LEtest_Packet_Send_Complete();
				//HWradio_LE_TxComplete();
				sm_role ^= 1; //toggle role
				HW_set_intraslot_offset(625);
				HW_set_slave(sm_role);

				if (LE_test.end_flag)
				{
					HW_set_slave(0);
					_LEtest_Test_Event_Complete();
					break;
				}
				/*
				 * The radio mode set to TX at the start of frame, with both MASTER/SLAVE role
				 */
				//HWradio_LE_Service(RADIO_MODE_LE_TESTMODE_TX, LE_test.test_freq,TX_START_FREQ);
				_LEtest_Prep_For_LE_Test_Tx();
				LE_test.sub_state = LE_W4_SECOND_TX;
			}
			break;

// Receiver Test

		case LE_W4_T2_PRE_RX :
			if (IRQ_Type==LE_TIM_2)
			{
				have_synced = 0;
				sm_role = 0;
				HW_set_slave(1);
				if (LE_test.end_flag)
				{
					_LEtest_Test_Event_Complete();
					break;
				}
		//		HWle_set_le_spi_only();
		//		HWradio_LE_Setup_Radio_For_Next_TXRX(RX_START);
		#if		(__DEBUG_RX_TEST_MODE_SET_TO_ONE__ == 1)
				_LEtest_Prep_For_LE_Test_Rx(0);
		#else
				_LEtest_Prep_For_LE_Test_Rx();
		#endif
				LE_test.sub_state = LE_W4_TEST_RX;
			}
			break;
		
		case LE_W4_TEST_RX :
			switch(IRQ_Type)
			{
				case LE_TIM_2 :
					if (LE_test.end_flag)
					{	
						_LEtest_Test_Event_Complete();
						break;
					}
					#if(__DEBUG_RX_DATA_DUMP_TO_SHRAM0 == 1)
					#if(__DUMP_DATA_WHILE_NOT_PKD == 1)
					if(pkd_download_start == 1) {
					pkd_download_cnt1++;
					if(pkd_download_cnt1 > 1000) {
						*((u_int32 volatile*)0x4002c274) &= ~(0x1<<3);
						while(1);

						}
					}
					#endif
					#endif
					break;
				case LE_SYNC_DET:
					HW_set_rx_mode(0x01);
					break;
				case	LE_RX_HDR:
					/*HWle_abort_tifs_count();
					HWle_clear_tifs_default();*/	
					break;
				case LE_PKD :
					#if(__DEBUG_RX_DATA_DUMP_TO_SHRAM0 == 1)
					#if(__DUMP_DATA_WHILE == 0) 
					*((u_int32 volatile*)0x4002c274) &= ~(0x1<<3);
					#else
					pkd_download_cnt++;
					if(pkd_download_cnt > 1000) 
					{
						*((u_int32 volatile*)0x4002c274) &= ~(0x1<<3);
						while(1);
					}
					#endif
					#endif
					HWle_abort_tifs_count();
					HWle_clear_tifs_default();
					HW_set_rx_mode(0x00);					
#if 1
					HWle_clear_le_spi_only();
					//HWradio_DisableAllSpiWrites();
				    //HWradio_DisableAllSpiWrites();
		            ((void (*)(void))PATCH_FUN[HWRADIO_DISABLEALLSPIWRITES])();	
					//debug_io(2);
					
					//HWle_abort_tifs_count();
					//HWle_clear_tifs_default();
					HW_set_tx_mode(0x00);
				
					HWle_clear_tx_enable();
#endif
					//CRC of received packet needs to be checked here.
					//length = HWle_get_rx_length_adv();
					//if (!LE_LL_Get_CRC_Err() && (0x25 == length))
					if (!LE_LL_Get_CRC_Err() )
					{			
						LE_test.num_packets_received++;
						if(have_synced==0)
						{
							intraslot_offset = HW_get_intraslot_offset();
							have_synced = 1;
						}
					}
					else
					{
						/*have_synced = 0;*/
						LE_test.num_packets_received_withCRC++;
					//	LE_test.sub_state = LE_W4_T2_PRE_RX;
					}

					if(have_synced==1)
					{
						sm_role ^= 1;
						if(sm_role==1)
						{
							if(intraslot_offset>=625)
							{
								HW_set_intraslot_offset(intraslot_offset - 625);
								//debug_io(1);	
							}
							else
							{
								HW_set_intraslot_offset(intraslot_offset + 625);
								//debug_io(2);
							}
						}
						else
						{
							HW_set_intraslot_offset(intraslot_offset);
							//debug_io(3);
						}
					}
					
					/*HWradio_LE_RxComplete();
										
					HWle_clear_tifs_abort();*/
					#if		(__DEBUG_RX_TEST_MODE_SET_TO_ONE__ == 1)
					_LEtest_Prep_For_LE_Test_Rx(have_synced);
					#else
					_LEtest_Prep_For_LE_Test_Rx();
					#endif
					break;

				default:
					break;
			}
			break;
		case LE_TST_IDLE:
		default:
			break;
	}


	return NO_ERROR;
}
/***********************************************************************
 * 
 * Function:	LEtest_Test_Event_Begin
 *
 * Description: This function prepares the hardware for test mode.
 *
 ***********************************************************************/
void LEtest_Test_Event_Begin(void)
{
	//*((u_int32 volatile*)0x4002c240) = (u_int32)(0x02000333);   //- software control rx/tx enable.
	//*((u_int32 volatile*)0x4002c260) = (u_int32)(0x60001);
  	HWle_set_le_mode();
	#if(__DEBUG_XINC_FPGA == 0)
 	//HWle_set_adv_state();
 	HWle_clear_adv_state();
 	#else
	HWle_set_adv_state();
	#endif

	/*
	 * The crc shift register is preset to 0x555555 as in spec. for all test packets.
	 */
	HWle_set_crc_init(0x555555);
	
	/*
	 * The access address is a predefined value set in spec. for all test packets.
	 */
	HWle_set_acc_addr(TEST_MODE_ACCESS_ADDRESS);

	/*
	 * Whitening is disabled for all LE test packets.
	 */
	HWle_clear_whitening_enable();

	
	if (LE_test.sub_state == LE_W4_PRE_TEST_TX)
	{
		
   		HWle_set_tx_enable();
		HWle_set_master_mode();

		/*
		 * LSB, payload type, 0 0 0 0, payload length, 0 0
		 */
		_LEtest_Set_Test_Mode_Header(LE_test.packet_payload_type, LE_test.len_of_test_data);

		/*
		 * Load data packet into transmit buffers 
		 */
		hw_memcpy32((void*)HWle_get_tx_acl_buf_addr(), (void*)LE_test.data_payload,  LE_test.len_of_test_data);

	}
	else	{
		HWle_clear_tx_enable();
		#if(__DEBUG_XINC_FPGA == 0)	
		HWle_set_adv_state();
		#endif
	}			
	LE_config.state = TEST_STATE;
	
}
/***********************************************************************
 * 
 * Function:	_LEtest_Build_Test_Packet_Payload
 *
 * Description: Build the payload of the test packet.
 *				The pattern is dependent on the testScenario parameter.
 *				0x00 Pseudo-Random bit sequence 9
 *				0x01 Pattern of alternating bits ?1110000'
 *				0x02 Pattern of alternating bits ?0101010?
 *				0x03 Pseudo-Random bit sequence 15 - Not supported
 *				0x04 Pattern of All ??bits
 *				0x05 Pattern of All ??bits
 *				0x06 Pattern of alternating bits ?0001111'
 *				0x07 Pattern of alternating bits ?101?
 ***********************************************************************/
void _LEtest_Build_Test_Packet_Payload(t_letestScenario testScenario, 
                        u_int8 *txBuf, t_length packetLength)
{
    u_int TXpattern = 0x00;
    u_int32 i;

		if (testScenario==LE_TX_Psuedo_Random_9_bit)
		{//9 	x^{ 9 }+x^{ 5 }+1 	511

		    TXpattern = 0x1FF;

			*txBuf++ = 0xFF;
			
			for (i=0; i<packetLength; i++)
			{
			    
			    TXpattern = ((TXpattern ^ (TXpattern>>4))<<1) ^ (TXpattern>>8);
			    TXpattern = (TXpattern ^ (TXpattern>>1)<<6) & 0x1FF;
				*txBuf++ = TXpattern & 0xFF;

			}
		}
		else

#if (PRH_BS_CFG_SYS_TESTMODE_LE_TX_PSEUDO_RANDOM_15_BIT==1)
		if (testScenario==LE_TX_Psuedo_Random_15_bit)
		{ 
		  /*
		   *TODO 
		   * 15 	x^{ 15 }+x^{ 14 }+1 	32767
		   * No pauses or packet structures in this packet
		   */
			TXpattern = 0x1FFFF;
	
			for (i=0; i<packetLength; i++)
		    {
		        TXpattern = ((TXpattern ^ (TXpattern>>1))<<1) ^ (TXpattern>>14);
		        TXpattern = (TXpattern ^ (TXpattern>>1)<<6) & 0x1FFFF;
				*txBuf++ = TXpattern & 0xFFFF;
		    }
		}
		else
#endif
	{
#if 1
		TXpattern = TxPatternArray[testScenario];
#else
		switch (testScenario)	
		{
			case LE_TX_11110000_Pattern:
				TXpattern = 0xF0;
				break;
			case LE_TX_10_Pattern:
				TXpattern = 0xAA;
				break;
			case LE_TX_1_Pattern:
				TXpattern = 0xFF;
				break;
			case LE_TX_0_Pattern:
				TXpattern = 0x00;
				break;
			case LE_TX_00001111_Pattern:
				TXpattern = 0x0F;
				break;
			case LE_TX_01_Pattern:
				TXpattern = 0x55;
				break;
			default:
				return;
		}
#endif

		for (i=0; i<packetLength; i++)
			*txBuf++ = TXpattern;
	}

}
/***********************************************************************
 * 
 * Function:	_LEtest_Set_Test_Mode_Header
 *
 * Description: Build the test mode payload header for transmit packets
 *
 ***********************************************************************/
void _LEtest_Set_Test_Mode_Header(u_int8 type, u_int8 length)
{
	unsigned	int	val;

	// Now set the Test Mode PDU header
	HWle_set_tx_test_pdu_rfu1(0x00);
	HWle_set_tx_test_length(length);
	HWle_set_tx_test_pdu_rfu2(0x00);
	HWle_set_tx_test_pdu_type(type); 

	#if(__DEBUG_XINC_FPGA == 0)
	
	val = *((u_int32 volatile*)0x4002A040);
	val <<= 16;
	*((u_int32 volatile*)0x4002A040) = val;
	
	#endif

}

/***********************************************************************
 * 
 * Function:	_LEtest_Prep_For_LE_Test_Rx
 *
 * Description: Prepare the radio and hardware for receiving a packet
 *
 ***********************************************************************/
 #if		(__DEBUG_RX_TEST_MODE_SET_TO_ONE__ == 0)
void _LEtest_Prep_For_LE_Test_Rx(void)
{	
      u_int32 i;

	HWle_set_tifs_default();
	HWle_clear_tifs_abort();

	//*((u_int32 volatile*)0x4002c274) |= (0x1<<5);  

	 
 	//*((u_int32 volatile*)0x40028030) = (u_int32)(0x80000000);      
   	//*((u_int32 volatile*)0x40028048) = (u_int32)(0x00000000);     //- rf_rx_en high
	//*((u_int32 volatile*)0x40028044) = (u_int32)(0x8000);
  	//*((u_int32 volatile*)0x4002805c) = (u_int32)(0x0000);		  //- modem_rx_en high
#if	0	
     for (i = 0; i < 300; i++);
 	*((u_int32 volatile*)0x40028044) = (u_int32)(0x8000); 	 	  //- modem_rx_en high	
  	*((u_int32 volatile*)0x4002805c) = (u_int32)(0x0000);
#endif	
	HW_set_rx_mode(0x03);

	//*((u_int32 volatile*)0x4002c274) &= ~(0x1<<5);  


	 #if(__DEBUG_RX_DATA_DUMP_TO_SHRAM0 == 1)
	 *((u_int32 volatile*)0x4002c274) |= (0x1<<3);
	 #if(__DUMP_DATA_WHILE_NOT_PKD == 1)
	 pkd_download_start = 1;
	 #endif
	 #endif
	/*
	 * The radio mode needs to be set to full rx.
	 */
//	HWradio_LE_Service(RADIO_MODE_LE_TESTMODE_RX,LE_test.test_freq,RX_START_FREQ);

}
 #else
void _LEtest_Prep_For_LE_Test_Rx(uint32_t		flg)
{	
	HWle_set_tifs_default();
	HWle_clear_tifs_abort();

	if(flg == 0)
	HW_set_rx_mode(0x03);
	else
	HW_set_rx_mode(0x01);	

	/*
	 * The radio mode needs to be set to full rx.
	 */
	//HWradio_LE_Service(RADIO_MODE_LE_TESTMODE_RX,LE_test.test_freq,RX_START_FREQ);
    ((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_LE_TESTMODE_RX,LE_test.test_freq,RX_START_FREQ);

}
 #endif
/***********************************************************************
 * 
 * Function:	_LEtest_Prep_For_LE_Test_Tx
 *
 * Description: Prepare the radio and hardware for transmitting a packet
 *
 ***********************************************************************/
void _LEtest_Prep_For_LE_Test_Tx(void)
{
   
	HWle_set_tx_enable();

	HWle_set_tifs_default();

	HWle_clear_tifs_abort();

	/*
	 * The radio mode needs to be set to full tx, with no tifs. TODO
	 */

	//HWradio_LE_Service(RADIO_MODE_LE_INITIAL_TX,LE_test.test_freq,TX_START_FREQ);

}
void _LEtest_Packet_Send_Complete(void)
{
	HWle_clear_tx_enable();
	HWle_clear_tifs_default();
	HWle_set_tifs_abort();
}
/***********************************************************************
 * 
 * Function:	_LEtest_Test_Event_Complete
 *
 * Description: This function turns off the test mode and returns the
 *				radio and hardware to a standby state.
 *
 ***********************************************************************/
void _LEtest_Test_Event_Complete(void)
{
	//HWradio_LE_RxComplete();
    ((void (*)(void))PATCH_FUN[HWRADIO_LE_RXCOMPLETE])();
	HWle_clear_le_spi_only();
	HWle_clear_adv_state();
	HWle_clear_master_mode();

	// Turn off RX mode
	HW_set_rx_mode(0x00);

    // Abort the TIFS count
  	HWle_set_tifs_abort();
	HWle_clear_tifs_abort();

	// Turn OFF TIFS count
	HWle_clear_tifs_default();
	HWle_clear_tx_enable();

	/*
	 * Whitening is re-enabled for normal LE behaviour.
	 */
	HWle_set_whitening_enable();
    ((boolean (*)(t_RadioMode,u_int8,t_radio_freq_mask))PATCH_FUN[HWRADIO_LE_SERVICE])(RADIO_MODE_STANDBY,LE_test.test_freq,TX_START_FREQ);
	//HWradio_LE_Service(RADIO_MODE_STANDBY,LE_test.test_freq,TX_START_FREQ);
    
	LE_test.end_flag = 0;
	LE_test.sub_state = LE_TST_IDLE;
	LE_config.state = STANDBY_STATE;
	LE_config.test_mode = LE_TEST_MODE_INACTIVE;
}
