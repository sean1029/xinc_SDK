/*************************************************************************
 * MODULE NAME:    uslc_testmode.c
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    Bluetooth Test Mode: Baseband Test Procedures
 * DATE:           24 Sept 1999
 *
 * SOURCE CONTROL?: $Id: uslc_testmode.c,v 1.104 2013/05/30 16:38:34 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *    V1.0     24 Sept 1999 -   jn       - Initial Version V1.0
 *    v1.1     22 Oct  1999 -   jn       - 1st Release
 *
 * SOURCE: Based directly on Part I:1 Bluetooth Test Mode V1.1 compliant
 * ISSUES:
 *     1. Whitening not affected by Setup.
 * NOTES TO USERS:
 *     1. refer to design doc - there are Parthus extensions to testmode
 *     2. Code below by default is the spec compliant minimal test mode 
 *************************************************************************/
#include "sys_config.h"

#include "lslc_access.h"
#include "lslc_pkt.h"
#include "lslc_hop.h"
#include "lc_interface.h"

#include "uslc_testmode.h"
#include "hw_lc.h"
#include "dl_dev.h"

#include "bt_timer.h"
#include "bt_test.h"


t_tm_control testmode_ctrl;
static u_int8 tm_pkt_count =0;

#ifdef USLC_TEST_MODE_EXTENSIONS
/*  All packets structure for all packet generation */
const static struct s_testmodePacket
{
    t_packet    packetType;
    t_state     packetContext;
    t_length    packetLength;
} allPackets[] = {

   { IDpkt,   Inquiry,            0},
   { IDpkt,   Page,               0},
   { IDpkt,   Page_Scan,          0},
   { IDpkt,   SlavePageResponse,  0},
   { FHSpkt,  MasterPageResponse, FHS_PACKET_LENGTH},
   { FHSpkt,  InquiryResponse,    FHS_PACKET_LENGTH},
   { NULLpkt, Connection,         0},
   { POLLpkt, Connection,         0},
   { DM1,     Connection,         MAX_DM1_USER_PDU},
   { DM3,     Connection,         MAX_DM3_USER_PDU},
   { DM5,     Connection,         MAX_DM5_USER_PDU},
   { DH1,     Connection,         MAX_DH1_USER_PDU},
   { DH3,     Connection,         MAX_DH3_USER_PDU},
   { DH5,     Connection,         MAX_DH5_USER_PDU},
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
   { EDR_2DH1,     Connection,    MAX_2DH1_USER_PDU},
   { EDR_2DH3,     Connection,    MAX_2DH3_USER_PDU},
   { EDR_2DH5,     Connection,    MAX_2DH5_USER_PDU},
   { EDR_3DH1,     Connection,    MAX_3DH1_USER_PDU},
   { EDR_3DH3,     Connection,    MAX_3DH3_USER_PDU},
   { EDR_3DH5,     Connection,    MAX_3DH5_USER_PDU},
#endif
   { AUX1,    Connection,         MAX_AUX1_USER_PDU} };
#endif

/*
 * Bit field for scenarios supported.
 * Support TX_Off, Tx_0, Tx_1, Tx_10, Tx_PS, LB_ACL_W, LB_SCO_W, 
 *  LB_ACL_NoW, LB_SCO_NoW
 */
const static u_int32 test_scenarios_supported = 0x00003FF;

/*
 * Bit field for packets supported.
 * 
 * Changes to support EDR ACL packet types:
 * (t_packet) EDR_3DH5 = 31, so shifting << 31 is no possible with u_int32
 * To resolve this issue all "<<" are reduced by 1 in order to faciliate the
 * "<<" assoc with EDR_3DH5.
 * Note: This " << (x-1)" must be taken in account when testing for packet type supported.
 */
#define PACKET_TYPES_SUPPORTED  ((1<< (AUX1-1)) | (1<< (DH1-1)) | (1<< (DM1-1)) | \
   ((3*PRH_BS_CFG_SYS_LMP_THREE_SLOT_PKT_SUPPORTED)<< (10-1)) | /* DM3, DH3 */ \
   ((3*PRH_BS_CFG_SYS_LMP_THREE_SLOT_PKT_SUPPORTED)<< (14-1)) | /* DM5, DH5 */ \
   ( PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED << (HV1-1)) |            /* HV1      */ \
   ( PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED << (DV-1)) |             /* DV       */ \
   ( PRH_BS_CFG_SYS_LMP_HV2_SUPPORTED << (HV2-1)) |            /* HV2      */ \
   ( PRH_BS_CFG_SYS_LMP_HV3_SUPPORTED << (HV3-1)) |            /* HV3      */ \
   ( PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED << (7-1) ) |    /* EV3      */ \
   ( PRH_BS_CFG_SYS_LMP_EV4_PACKETS_SUPPORTED << (EV4-1) ) |   /* EV4      */ \
   ( PRH_BS_CFG_SYS_LMP_EV5_PACKETS_SUPPORTED << (EV5-1) ) |   /* EV5      */ \
   ( PRH_BS_CFG_SYS_LMP_EDR_ACL_2MBPS_MODE_SUPPORTED << (EDR_2DH1-1) ) |     \
   ( PRH_BS_CFG_SYS_LMP_EDR_ACL_2MBPS_MODE_SUPPORTED << (EDR_AUX1-1) ) |     \
   ( (PRH_BS_CFG_SYS_LMP_EDR_ACL_2MBPS_MODE_SUPPORTED &                   \
      PRH_BS_CFG_SYS_LMP_3SLOT_EDR_ACL_PACKETS_SUPPORTED) << (EDR_2DH3-1) ) | \
   ( (PRH_BS_CFG_SYS_LMP_EDR_ACL_2MBPS_MODE_SUPPORTED &                   \
      PRH_BS_CFG_SYS_LMP_5SLOT_EDR_ACL_PACKETS_SUPPORTED) << (EDR_2DH5-1) ) | \
   ( PRH_BS_CFG_SYS_LMP_EDR_ACL_3MBPS_MODE_SUPPORTED << (EDR_3DH1-1) ) |      \
   ( (PRH_BS_CFG_SYS_LMP_EDR_ACL_3MBPS_MODE_SUPPORTED &                   \
      PRH_BS_CFG_SYS_LMP_3SLOT_EDR_ACL_PACKETS_SUPPORTED) << (EDR_3DH3-1) ) | \
   ( (PRH_BS_CFG_SYS_LMP_EDR_ACL_3MBPS_MODE_SUPPORTED &                   \
      PRH_BS_CFG_SYS_LMP_5SLOT_EDR_ACL_PACKETS_SUPPORTED) << (EDR_3DH5-1) ) | \
   ( PRH_BS_CFG_SYS_LMP_EDR_ESCO_2MBPS_MODE_SUPPORTED << (EDR_2EV3-1)) | \
   ( PRH_BS_CFG_SYS_LMP_EDR_ESCO_3MBPS_MODE_SUPPORTED << (EDR_3EV3-1)) | \
      ( (PRH_BS_CFG_SYS_LMP_EDR_ESCO_2MBPS_MODE_SUPPORTED &                   \
      PRH_BS_CFG_SYS_LMP_3SLOT_EDR_ESCO_PACKETS_SUPPORTED) << (EDR_2EV5-1) ) | \
      ( (PRH_BS_CFG_SYS_LMP_EDR_ESCO_3MBPS_MODE_SUPPORTED &                   \
      PRH_BS_CFG_SYS_LMP_3SLOT_EDR_ESCO_PACKETS_SUPPORTED) << (EDR_3EV5-1) ) )

#ifdef USLC_TEST_MODE_EXTENSIONS
/*
 * Support NULL, POLL, FHS, DM1, DH1, AUX1, DM3, DH3, DM5, DH5, ID
 *    if allowed (all above packets is 0x0001CE1F)
 */
const static u_int32 packet_types_supported = 0x0001001F | PACKET_TYPES_SUPPORTED;
#else
/*
 * Support DM1, DH1, AUX1, DM3, DH3, DM5, DH5 
 *     if allowed (all above packets is 0x0000CE18)
 */
const static u_int32 packet_types_supported = PACKET_TYPES_SUPPORTED;
#endif

#define ESCO_TEST_MODE_PACKET_MASK 0x10

boolean USLCtm_Is_Packet_Type_Supported(t_packet packetType)
{
    /*
     * Is EV3
     */
    if((packetType & ESCO_TEST_MODE_PACKET_MASK) && (packetType == EV3 ))
    {
        return PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED;
    }

	/* 
	 * Note: EDL ACL changes
	 * When testing if a packet type is suppported must subtract 1 in the
	 * following shift operation "1<<packetType"
	 */

	if (packetType & 0x20)
	{
		packetType = ((packetType & 0x0F)  | 0x10);
	}
    return (((1<<(packetType-1)) & packet_types_supported))>>(packetType-1);
}

/* 
 * Buffer to store generated packet   can be LOCAL_BUFFER or L2CAP_BUFFER 
 */
#define USLC_TEST_MODE_LOCAL_BUFFER 1                        
#define USLC_TEST_MODE_L2CAP_BUFFER 2

#define USLC_TEST_MODE_BUFFER_SELECTION USLC_TEST_MODE_L2CAP_BUFFER
#if (USLC_TEST_MODE_BUFFER_SELECTION==USLC_TEST_MODE_LOCAL_BUFFER)
static u_int8 _testmode_buffer[MAX_USER_PDU];
#endif

 extern t_SYS_Config g_sys_config;

/************************************************************************
 * USLCtm_Testmode_Request
 *
 * Request from the higher layers to start the Testmode procedure
 *
 * The settings do not become active until LMP_accepted is acked
 ************************************************************************/
t_error USLCtm_Testmode_Request(t_deviceIndex deviceIndex,
                               t_p_pdu p_LMP_test_control_PDU)
{
    t_error        status = NO_ERROR; 
    t_tm_control   volatile *tm_ctrl = &testmode_ctrl;
    t_testScenario testScenario;

    /*
     * V1.1, all parameters of the lmp_test_control message are XOR'ed 
     */
    {
        u_int8 i;
        for (i=0; i <= 8; i++)
        {
            p_LMP_test_control_PDU[i] ^= TESTMODE_WHITEN;
        }
    }

    testScenario = (t_testScenario) p_LMP_test_control_PDU[0];
    /* 
     * Extract the LMP_test_control_parameters as presented directly in the payload 
     */
    status = USLCtm_Store_Test_Parameters(p_LMP_test_control_PDU);
    if (status != NO_ERROR)
    {
        return status;
    }

    /* 
     * if the scenario is to exit test mode then do no further processing
     *  - cancel testmode will be called in change_settings when 
     *    LC level ACK received for LMP_accepted
     * if scenario is to turn of the transmission of packets (pause test) then
     *   go no further as the running test has been stopped in USLCtm_Initialise
     */
    if ((TX_Off == testScenario) || (Test_Mode_Exit == testScenario))
    {
        /* 
         * Turn off any running test 
         */
        USLCtm_Reinitialise();
		if (!BTq_Is_Queue_Empty(L2CAP_OUT_Q, (t_deviceIndex)deviceIndex))
		{
			// Now Clear the L2CAP OUT Q - of all entries.
			struct q_desc *tx_qd;

			tx_qd = BTq_BB_Dequeue_Next(L2CAP_OUT_Q, deviceIndex, EDR_3DH5_BIT_MASK);
			if (tx_qd)
				BTq_Ack_Last_Dequeued(L2CAP_OUT_Q, deviceIndex, tx_qd->length);
		}

#if 1 // GF 12 Jan 2010 - If we were in a Transmitter Test 
	  // we need to remove the Ack Pending

		if (BTtst_Get_DUT_Mode() == DUT_ACTIVE_TXTEST)
			DL_Set_Tx_Ack_Pending(DL_Get_Device_Ref(deviceIndex),FALSE);
#endif
        BTtst_Set_DUT_Mode(DUT_ACTIVE);
        return status;
    }

    /* 
     * Create the requested packet type once, in the local buffer or L2CAP queue.
     */
    if (tm_ctrl->transmitter_test)
    {
        u_int16 initialised_payload_length;
        /*
         * Ensure that initialised payload is always long enough 
         * to handle SCOs HV1,2,3 and DV
         */
        initialised_payload_length = tm_ctrl->packetLength;
        if (initialised_payload_length < MAX_HV3_USER_PDU)
        {
            initialised_payload_length = MAX_HV3_USER_PDU;
        }
  

#if (USLC_TEST_MODE_BUFFER_SELECTION==USLC_TEST_MODE_LOCAL_BUFFER)
        tm_ctrl->p_data_buffer = _testmode_buffer;
        #define USLC_TEST_MODE_BUFFER_MAX_LENGTH sizeof(_testmode_buffer)

#elif (USLC_TEST_MODE_BUFFER_SELECTION==USLC_TEST_MODE_L2CAP_BUFFER)
        /*
         * Use the non used ACL Out buffers in tra_queue.c as a temporary scrap region
         */
        tm_ctrl->p_data_buffer = BTq_Get_L2CAP_Out_Heap_Ref();
        #define USLC_TEST_MODE_BUFFER_MAX_LENGTH \
          ((g_sys_config.hc_buffer_size.numAclDataPackets)*PRH_BS_CFG_SYS_ACL_OUT_PACKET_LENGTH)
#else
  #error USLC_TEST_MODE_BUFFER_SELECTION not defined
#endif

        if (initialised_payload_length < USLC_TEST_MODE_BUFFER_MAX_LENGTH)
        {
            USLCtm_Build_Test_Packet_Payload(tm_ctrl->testScenario, 
                tm_ctrl->p_data_buffer, initialised_payload_length);

            /*
             * If Direct Baseband Copy (Big Endian) Then Change Data to Big Endian
             */
            if (mLSLCacc_Is_ACL_Packet(tm_ctrl->packetType))
            {
                BTq_Change_Endianness_If_Required(
                    tm_ctrl->p_data_buffer, initialised_payload_length);
            }
        }
        else
        {
            status = UNSUPPORTED_PARAMETER_VALUE;
        }
    }
    /*
     * Ensure that if SCO is supported, the Tabasco SCO configuration is 
     * 9 for RGF/APB.
     * Hence there will be no conversion, and the ACL registers will be
     * will be used for the SCO payload.
     */
    DL_Set_Active_SCO_Index(DL_Get_Device_Ref(deviceIndex), 0);

    LC_Sync_Setup_SCO_CFG_Ex(0,  SCO_CFG_VIA_ACL);
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
        LC_Set_PTT_ACL(deviceIndex, (boolean)is_EDR_ACL_packet(tm_ctrl->packetType));
#endif

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
    {
        t_devicelink *p_escoDL = DL_Get_Device_Ref(deviceIndex);
        DL_Set_eSCO_Tx_Length(p_escoDL, (tm_ctrl->packetLength));
        DL_Set_eSCO_Rx_Length(p_escoDL, (tm_ctrl->packetLength));
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
        LC_Set_PTT_ESCO(deviceIndex, (boolean)is_EDR_ESCO_packet(tm_ctrl->packetType));
#endif
    }
#endif

    return status;
}

/******************************************************************
 * USLCtm_Initialise
 *
 *  Initialise appropriate fields.
 *****************************************************************/
void USLCtm_Initialise(void)
{
    t_tm_control volatile *tm_ctrl=&testmode_ctrl;

    tm_ctrl->transmitter_test = 0;
    tm_ctrl->loopback_test = 0;
    tm_ctrl->packetContext = No_State;
}

/******************************************************************
 * USLCtm_Reinitialise
 *
 * Ensure there is no test running.
 *****************************************************************/
void USLCtm_Reinitialise(void)
{
    t_tm_control volatile *tm_ctrl=&testmode_ctrl;

    tm_ctrl->transmitter_test = 0;
    tm_ctrl->loopback_test = 0;
}

/******************************************************************
 * USLCtm_Store_Test_Parameters
 *
 * Takes the test config params from the test control payload and
 * stores them in the test structure
 *****************************************************************/
t_error USLCtm_Store_Test_Parameters(t_p_pdu p_LMP_test_control_PDU)
{
    t_tm_control   volatile *tm_ctrl = &testmode_ctrl;
    t_testScenario testScenario;
    t_packet packetType;

    tm_ctrl->testScenario     = (t_testScenario) *p_LMP_test_control_PDU++;

	if (TX_Off != tm_ctrl->testScenario)
	{ 
        tm_ctrl->rfHoppingMode    = (t_rfSelection)  *p_LMP_test_control_PDU++;
        tm_ctrl->txDutFrequency   = (t_freq)         *p_LMP_test_control_PDU++;
		tm_ctrl->rxDutFrequency   = (t_freq)         *p_LMP_test_control_PDU++;
        tm_ctrl->powerControlMode = (u_int8)         *p_LMP_test_control_PDU++;
        tm_ctrl->pollPeriod       = (t_TDDframes)    *p_LMP_test_control_PDU++;
        tm_ctrl->packetType       = (t_packet)       *p_LMP_test_control_PDU++;
        tm_ctrl->packetLength     = (t_length)       *p_LMP_test_control_PDU++;
        tm_ctrl->packetLength += (*p_LMP_test_control_PDU<<8);
	}
	else
	{
		/* When the test scenario is set to Pause Test all the other fields in the
		   LMP_test_control PDU shall be ignored. There shall be no change in hopping
		   scheme or whitening as a result of a request to pause test.*/
		tm_ctrl->powerControlMode = 0;
		tm_ctrl->pollPeriod = 0;
		tm_ctrl->packetType = 0; 
		tm_ctrl->packetLength = 0;
	}

    if(tm_ctrl->packetType == (0x10|EV4))
        tm_ctrl->packetType = EV4;
    else if(tm_ctrl->packetType == (0x10|EV5))
        tm_ctrl->packetType = EV5;
	else if(tm_ctrl->packetType == 0x17)
        tm_ctrl->packetType = EV3;

	else if (tm_ctrl->packetType & 0x20)
	{
		tm_ctrl->packetType = ((tm_ctrl->packetType & 0x0F) | 0x10);
	}
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
	else if ( tm_ctrl->packetType & 0x30)
	{
		/* EDR ACL or eSCO */
		/* Must convert from "Testmode" packet type" (see Spec Table 4.31)*/
		/* to t_packet                                                    */
		/* t_packet = (TM_packet Bits 3-0) | 0x10                         */
		tm_ctrl->packetType = ( tm_ctrl->packetType & 0x0f) | 0x10;
	}
#endif

	if ((tm_ctrl->testScenario != 0x05) &&  // 
	    (tm_ctrl->testScenario != 0x07))
	{
		if (tm_ctrl->packetType == HV1)
			tm_ctrl->packetLength = MAX_HV1_USER_PDU;
		else if (tm_ctrl->packetType == HV2)
			tm_ctrl->packetLength = MAX_HV2_USER_PDU;
		else if (tm_ctrl->packetType == HV3)
			tm_ctrl->packetLength = MAX_HV3_USER_PDU;
#if 0  // For EVx let user select the length.

		else if (tm_ctrl->packetType == EV3)
			tm_ctrl->packetLength = MAX_EV3_USER_PDU;
		else if (tm_ctrl->packetType == EV4)
			tm_ctrl->packetLength = MAX_EV4_USER_PDU;
		else if (tm_ctrl->packetType == EV5)
			tm_ctrl->packetLength = MAX_EV5_USER_PDU;

		else if (tm_ctrl->packetType == EDR_2EV3)
			tm_ctrl->packetLength = MAX_2EV3_USER_PDU;
		else if (tm_ctrl->packetType == EDR_3EV3)
			tm_ctrl->packetLength = MAX_3EV3_USER_PDU;
		else if (tm_ctrl->packetType == EDR_2EV5)
			tm_ctrl->packetLength = MAX_2EV5_USER_PDU;
		else if (tm_ctrl->packetType == EDR_3EV5)
			tm_ctrl->packetLength = MAX_3EV5_USER_PDU;
#endif
	}

	if ((tm_ctrl->packetType == DV) && (tm_ctrl->testScenario >= 5) &&
        (tm_ctrl->testScenario < 9))
		tm_ctrl->packetLength = 9;
	
    packetType = tm_ctrl->packetType;
    testScenario = tm_ctrl->testScenario;

    /* 
     * Code below for the tester as it calls this function directly 
     */
    if ((Test_Mode_Exit == testScenario) || (TX_Off == testScenario))
    {
        return NO_ERROR;
    }

#ifdef USLC_TEST_MODE_EXTENSIONS
   
    /* 
     * Check if an extension test TX_CYCLE_ALL_LENGTHS is selected
     *  If is note it and determine actual TX pattern requested. 
     */
    tm_ctrl->cycle_length_test = ( testScenario & PT_TX_CYCLE_ALL_LENGTHS);
    if (tm_ctrl->cycle_length_test)
    {
        testScenario = (t_testScenario) (testScenario & PT_TEST_MASK);
        tm_ctrl->next_cycle_length = 0;
    }

    /* 
     * If INVALID packet selected then transmit all packets
     *  Later will check and restrict to packets supported for this device 
     */
    tm_ctrl->transmitter_test_all_packets = (tm_ctrl->packetType == INVALIDpkt);
    if (tm_ctrl->transmitter_test_all_packets)
    {
        tm_ctrl->packetLength = MAX_USER_PDU;
    }

    /* 
     * Extension - Check if should send test packet every poll period 
     */
    tm_ctrl->send_packet_every_poll_period = (testScenario & PT_TX_EVERY_POLL_PERIOD);
    if (tm_ctrl->send_packet_every_poll_period)
    {
        testScenario = (t_testScenario) (testScenario & PT_TEST_MASK);
    }
    /* update any changes to parameters due to extensions */
    tm_ctrl->testScenario = testScenario;
    tm_ctrl->packetType = packetType;
   
#endif


    /* 
     * Check the parameters 
     */
    if ( !((1<<testScenario) & test_scenarios_supported) ||
           !(USLCtm_Is_Packet_Type_Supported(packetType)))
    {
        return UNSUPPORTED_PARAMETER_VALUE;
    }
    
     /* 
      * Determine if a transmitter test or loopback test
      */
    tm_ctrl->transmitter_test = ( (testScenario >= TX_0_Pattern) &&
                                  (testScenario <= TX_Psuedo_Random) ) ||
                                  (testScenario == TX_11110000_Pattern);
    tm_ctrl->loopback_test = ( (1 << testScenario) &
                              ( (1 << LB_ACL_Packets_Whitening_On ) |
                                (1 << LB_ACL_Packets_Whitening_Off) |
                                (1 << LB_SCO_Packets_Whitening_On ) |
                                (1 << LB_SCO_Packets_Whitening_Off) ) ) != 0;

    /* 
     * store current device settings if first time here 
     */
    if (tm_ctrl->packetContext == 0)
    {
        tm_ctrl->original_data_whitening_state = SYSconfig_Get_Data_Whitening_Enable();
        tm_ctrl->original_hopping_mode = (t_rfSelection) SYSconfig_Get_Hopping_Mode();
        tm_ctrl->original_tx_freq = SYSconfig_Get_Tx_Freq();
        tm_ctrl->original_rx_freq = SYSconfig_Get_Rx_Freq();
    }

#if 0 /* testers may request poll period 1 for multislot */
    if (tm_ctrl->transmitter_test)
    {
        if ( (is_Packet_Multi_Slot(packetType) && (tm_ctrl->pollPeriod == 1))
          || (is_Packet_5_Slot(packetType) && (tm_ctrl->pollPeriod == 2)) )
              return UNSUPPORTED_PARAMETER_VALUE;
    }
#endif

    tm_ctrl->packetContext = Connection;

    return NO_ERROR;
}

/*************************************************************************
 * USLCtm_Testmode_Change_Settings
 *
 * This is called when the LC acknowledges the sending of the LMP_Accepted
 * to the Test_Control message from the Tester 
 * - now safe to change frequency and whitening settings
 * The 'device_index' input is added to satisfy the call requirments of
 * qd->callback in lmp_encode_pdu.c
 *
 * Note:
 * Data Whitening must be off for Transmitter Test, Optional on Loopback.
 * For Transmitter Test, Test Packet is generated and placed on Queue.
 * This function must be called after the LMP_Accepted message has 
 * been acknowledged (on the correct whitening mode).  There is an
 * anomaly between frequency changes and whitening mode changes where
 * for frequency changes the LMP_Accepted isn't acknowledged before
 * the frequency is changed.
 *
 * V1.1 Spec   I1: 2.1.5 Power Control
 * If adaptive power control is tested, the normal LMP commands will be used.
 * The DUT starts to transmit at the maximum power and reduces/increases its
 * power by one step on every command received.
 *
 * It is interpreted that for fixed power control we should transmit at
 * maximum power.
 *
 * Hence for both transmit and loopback tests, the device starts at 
 * maximum power, but does not prevent tester from doing adaptive control.
 *************************************************************************/
void USLCtm_Testmode_Change_Settings(t_deviceIndex device_index)
{
    t_tm_control   volatile *tm_ctrl = &testmode_ctrl;

    /* 
     * If its a transmitter test then disable whitening 
     */
    if (tm_ctrl->transmitter_test)
    {
        BTtst_Set_DUT_Mode(DUT_ACTIVE_TXTEST);
        SYSconfig_Set_Data_Whitening_Enable(0);
    }
    else if (tm_ctrl->loopback_test) 
    {
        BTtst_Set_DUT_Mode(DUT_ACTIVE_LOOPBACK);
        SYSconfig_Set_Data_Whitening_Enable((u_int8)
                       ( tm_ctrl->testScenario==LB_ACL_Packets_Whitening_On ||
                         tm_ctrl->testScenario==LB_SCO_Packets_Whitening_On ));

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
        if(is_eSCO_Packet(testmode_ctrl.packetType))
        {
            DL_Set_Active_Tx_LT(DL_Get_Device_Ref(device_index), ACL_SCO_LT); /* deliberate */
            DL_Set_eSCO_LT_Address(DL_Get_Device_Ref(device_index), ESCO_LT, 
                DL_Get_Am_Addr(DL_Get_Device_Ref(device_index)));
            DL_Set_eSCO_Rx_Length(DL_Get_Device_Ref(device_index), tm_ctrl->packetLength );
            DL_Set_eSCO_Tx_Length(DL_Get_Device_Ref(device_index), tm_ctrl->packetLength );
        }
        else
        {
            DL_Set_Active_Tx_LT(DL_Get_Device_Ref(device_index), ACL_SCO_LT);
            DL_Set_eSCO_LT_Address(DL_Get_Device_Ref(device_index), ESCO_LT, 
                (u_int8)(1+DL_Get_Am_Addr(DL_Get_Device_Ref(device_index))));
        }
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
            LC_Set_PTT_ESCO(device_index, (boolean)is_EDR_ESCO_packet(testmode_ctrl.packetType));
#endif
#endif /* PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1 */

#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
            LC_Set_PTT_ACL(device_index, (boolean)is_EDR_ACL_packet(testmode_ctrl.packetType));
#endif
    }


    if (Test_Mode_Exit == tm_ctrl->testScenario)
    {
        USLCtm_Testmode_Exit();

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
        /*
         * Ensure that eSCO LT ADDR is kept separate from ACL/SCO LT_ADDR
         */
        DL_Set_Active_Tx_LT(DL_Get_Device_Ref(device_index), ACL_SCO_LT);
        DL_Set_eSCO_LT_Address(DL_Get_Device_Ref(device_index), ESCO_LT, 
            (u_int8)(1+DL_Get_Am_Addr(DL_Get_Device_Ref(device_index))));
#endif
    }
    else
    {
#if 0 // TK 17/05/2013 Anritsu MT8825 expects NOT to reinit Power Level on each Test Control
        /*
         * Set tx power change to maximum
         */
        DL_Set_Power_Level(DL_Get_Device_Ref(device_index), MAX_POWER_LEVEL_UNITS);
#endif
    
        /* 
         * Change hopping mode and/or freqs 
         */
        LSLChop_Initialise(tm_ctrl->rfHoppingMode,
                              tm_ctrl->rxDutFrequency, 
                              tm_ctrl->txDutFrequency );
    }
}

/******************************************************************
 * USLCtm_Testmode_Exit
 *
 * Request from the higher layers to exit the Testmode procedure
 *
 * LC_Testmode_Exit will be invoked before each disconnection hence
 * the check below.
 *****************************************************************/
void USLCtm_Testmode_Exit(void)
{
    t_tm_control   volatile *tm_ctrl;
    
    if (BTtst_Get_DUT_Mode() > DUT_ENABLED)
    {
        tm_ctrl = &testmode_ctrl;

        /* 
         * Turn off any running test 
         */
        USLCtm_Reinitialise();

        /* 
         * If a test_control was sent since startup then restore setup
         *   prior to test_control command - packetContext will be 0 unless
         *   a TC command was sent - could use other params to check 
         */
        if (tm_ctrl->packetContext != 0)
        {
            SYSconfig_Set_Data_Whitening_Enable( (u_int8)
                tm_ctrl->original_data_whitening_state);

            LSLChop_Initialise((t_rfSelection) tm_ctrl->original_hopping_mode,
                tm_ctrl->original_tx_freq, tm_ctrl->original_rx_freq );
        }
    
        /* 
         * If SCO supported Then Turn off codec (SCO configuration is inactive)
         */
        LC_Sync_Setup_SCO_CFG_Ex(0, SCO_CFG_INACTIVE);


        /* 
         * Demote mode back to DUT mode 
         */
        BTtst_Set_DUT_Mode(DUT_ENABLED);
    }
}

/*****************************************************************************
 * USLCtm_prepareTx
 *
 * TestMode Transmission preparation
 *
 * If LMP_message to be Tx Then
 *    Get of Queue and Send as normal ACL
 * Else If Transmitter Test Then
 *    Transmitter Test Tx
 * Else
 *    Loopback Test Tx
 * Endif
 *
 ****************************************************************************/
void USLCtm_prepareTx(t_devicelink *pDL)
{
    t_tm_control *tm_ctrl=&testmode_ctrl;

#if (PRH_BS_CFG_SYS_TEST_MODE_TESTER_SUPPORTED==1)
    /*
     * TestMode Transmission Preparation where the device 
     * is in Standalone test mode 
     */
    if (DUT_STANDALONE_TXTEST == BTtst_Get_DUT_Mode())
    {
        /*
         * Check if we should send at next transmit interval
         * based on Poll Period which is in TxRx Frames 
         * Note this is not clock wrap safe, but it is simple/efficient.
         */
        if ( ((((HW_get_bt_clk()>>2)+1) % testmode_ctrl.pollPeriod) == 0)
               || (testmode_ctrl.pollPeriod == 0))
        {
            USLCtm_Test_Transmitter(pDL);
        }
    }
    else
#endif

    if (tm_ctrl->transmitter_test)
    {
        if (POLLpkt == DL_Get_Rx_Packet_Type(pDL))
        {
            USLCtm_Test_Transmitter(pDL);
        }
        else
        {
            LSLCpkt_Generate_NULL(pDL);
        }
    }
    else
    {
        /* RGB check if ever goes here */
        LSLCpkt_Generate_NULL(pDL);
    }
}

/***********************************************************************
 * USLCtm_Test_Transmitter
 *
 * The Test Transmitter can be activated for both a Master and Slave
 * 1. DUT Test Mode    V1.0b p808
 *    The DUT (acting as slave) transmits a constant bit pattern under 
 *    the control of the Tester (acting as master).
 *    The DUT starts transmission after the first Poll received.
 *    The DUT may (V1.0b may!, V1.0a shall!) continue to transmit
 *    at Polling interval even if no packet received.
 * 2. Master Transmitter Test (Proprietary Extension).
 *    The same function can be used for setting a Master role device
 *    to transmit according to the normal LMP_test_control parameters. 
 *    Code is added to ensure that ID/FHS packets can be tested in 
 *    context.  It uses the ctrl_state in the Device Link as context
 *    and maps to actual state context.
 * 
 * Note:  Is it possible that a test packet would be delayed?  
 *
 ***********************************************************************/
void USLCtm_Test_Transmitter(t_devicelink *pDL)
{
    t_tm_control *tm_ctrl = &testmode_ctrl;

    t_length packetLength = tm_ctrl->packetLength;
    t_packet packetType    = tm_ctrl->packetType;

	if (tm_ctrl->packetType > 0x0F)
	{
		packetType = 0x10 | (tm_ctrl->packetType & 0x0F);
	}

    {
#ifdef USLC_TEST_MODE_EXTENSIONS
        /*
         * State variable for next packet in all packet sequencer
         */
        static u_int _next_packet = 0;
        t_state  packetContext = tm_ctrl->packetContext;

        if (tm_ctrl->transmitter_test_all_packets)
        {
            /*
             * Continuously cycle through all packets defined in allPackets.
             * Note that all packets are output on same device link!
             */
            packetContext = allPackets[_next_packet].packetContext;
            packetType    = allPackets[_next_packet].packetType;
            packetLength  = allPackets[_next_packet].packetLength;

            if (packetLength > tm_ctrl->packetLength)
                packetLength = tm_ctrl->packetLength;
            ++_next_packet;
            if (_next_packet >= sizeof(allPackets)/sizeof(allPackets[0]))
               _next_packet=0;
            /*
             * Same state for receive
             */
            tm_ctrl->packetContext = packetContext;
        }


        else if (tm_ctrl->cycle_length_test)
        {
            /*
             * Cycle through all lengths for packet type
             * Both one-shot and continuous supported
             */
             packetLength = tm_ctrl->next_cycle_length;

             if (packetLength > tm_ctrl->packetLength)
             {
#ifdef TESTMODE_CYCLE_ONCE
                 packetType = POLLpkt;
#else
                 packetLength = 0;
                 tm_ctrl->next_cycle_length = 0;
#endif
             }
             else
             {
                 ++tm_ctrl->next_cycle_length;
             }
        }

        /*tm_ctrl->tx_Next_Packet_Timer = BTtimer_Reset_Slots
               (tm_ctrl-> tx_Next_Packet_Timer, tm_ctrl->pollPeriod<<1);*/

        /* 
         * Only send test packet every poll period if enabled 
         */
        if (!tm_ctrl->send_packet_every_poll_period)
        {
            tm_ctrl->start_tx_test_packet = 0;
        }

        if (packetType==IDpkt)
        {
            status = LSLCpkt_Generate_ID(DL_Get_Local_Device_Ref(), packetContext);
        }

        else if (packetType==FHSpkt)
        {
            status = LSLCpkt_Generate_FHS(DL_Get_Local_Device_Ref(), packetContext);
        }

        else if (packetType==NULLpkt)
        {
            status = LSLCpkt_Generate_NULL(pDL);
        }

        else if (packetType==POLLpkt)
        {
            status = LSLCpkt_Generate_POLL(pDL);
        }

        else 
#endif  /*USLC_TEST_MODE_EXTENSIONS */   
       
        /* 
         * Default is normal ACL or SCO 
         */
        {
            /*
             * Ensure that build descriptor is Local Descriptor
             * Note DVs need both ACL/SCO setup and tx length is ACL
             */
            DL_Use_Local_Tx_Descriptor(pDL);
            DL_Set_Tx_Message_Type(pDL, LCH_start);
            DL_Set_Tx_Packet_Type(pDL, packetType);
            /*
             * If the packet type is HV3, ensure
             * that the esco LT ADDR is not set to the HW_am_addr() field
             *
             * If the packet is EV3, ensure that the eSCO LT ADDR
             * is set to the HW_am_addr() field.
             */
#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
            if(is_eSCO_Packet(packetType))
            {
                DL_Set_Active_Tx_LT(pDL, ESCO_LT);
                DL_Set_eSCO_LT_Address(pDL, ESCO_LT, DL_Get_Am_Addr(pDL));
            }
#endif
            DL_Set_Tx_Length(pDL, packetLength);

            /*
             * Pull the ACL/SCO packet payload from the defined buffer
             */
#if (BUILD_TYPE !=UNIT_TEST_BUILD) // GF 5 Jan 2009 -- Removed for ATS testmode transmitter tests
// Below are two lines of test code used to insert Seqnence numbers into packets
//			tm_ctrl->p_data_buffer[0] = tm_pkt_count++; 
//			tm_ctrl->p_data_buffer[packetLength-1] = tm_pkt_count; 
#endif
            DL_Set_Tx_Qpdu(pDL,  tm_ctrl->p_data_buffer);


#if (PRH_BS_CFG_SYS_LMP_SCO_SUPPORTED==1) || (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
            if (is_SYN_Packet(packetType))
            {
                DL_Set_Tx_SCO_Packet_Type(pDL, packetType);
                LC_Sync_Setup_SCO_CFG_Ex(pDL->active_sco_fifo, SCO_CFG_VIA_ACL);
                LSLCpkt_Generate_SCO(pDL);
            }
            else /*ACL*/
#endif
            
            {
#if (PRH_BS_CFG_SYS_LMP_EDR_SUPPORTED==1)
				if(is_EDR_ACL_packet(packetType))
				{
					LC_Set_PTT_ACL(pDL->device_index, (boolean)is_EDR_ACL_packet(packetType));
					DL_Set_PTT_ACL(pDL, TRUE);
				}
				else
				{
					DL_Set_PTT_ACL(pDL, FALSE);
				}
#endif
                LSLCpkt_Generate_ACL(pDL);
            }
            
            /*
             * Not recommended to perform data retransmissions in test modes,
             * so no ack pending and can toggle TX SEQN early where required.
             */
#if (PRH_BS_DEV_ADOPT_ERRATUM_E2088==0)
            if(is_ACL_CRC_Packet(packetType)) 
            {
                DL_Toggle_Tx_LT_Seqn(pDL);
            }
#endif
            DL_Set_Tx_Ack_Pending(pDL, FALSE);
        }
    }
}


/***********************************************************************
 * USLCtm_Build_Test_Packet_Payload
 *
 * Either 0, 1, 10101010 (lsb first), 1111 0000 1111 (lsb first)
 *     or PsuedoRandom using PRBS-9 bit feedback: b4 xor b0
 * The PRBS-9 has been reduced to faster byte update.
 ***********************************************************************/
void USLCtm_Build_Test_Packet_Payload(t_testScenario testScenario, 
                        u_int8 *txBuf, t_length packetLength)
{
    u_int TXpattern = 0x00;
    u_int32 i;

    if (testScenario==TX_0_Pattern  || testScenario==TX_1_Pattern ||
        testScenario==TX_10_Pattern || testScenario==TX_11110000_Pattern)
    {
        if (testScenario==TX_0_Pattern)
            TXpattern = 0x00;
        else if (testScenario==TX_1_Pattern)
            TXpattern = 0xFF;
        else if (testScenario==TX_10_Pattern)
            TXpattern = 0x55;
        else if (testScenario==TX_11110000_Pattern)
            TXpattern = 0x0F;
        
        for (i=0; i<packetLength; i++)
            *txBuf++ = TXpattern;
    }

    else if (testScenario==TX_Psuedo_Random)
    {
        TXpattern = 0x1FF;

		if (BTtst_Get_DUT_Mode() == DUT_TESTMODE_TESTER)
		{
			*txBuf++ = tm_pkt_count++;
			i=1;
		}
		else
		{
			i=0;
		}


        for (/*i=1*/; i<packetLength; i++)
        {
            *txBuf++ = TXpattern & 0xFF;
            TXpattern = ((TXpattern ^ (TXpattern>>4))<<1) ^ (TXpattern>>8);
            TXpattern = (TXpattern ^ (TXpattern>>1)<<6) & 0x1FF;
        }
    }
}
