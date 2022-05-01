/***********************************************************************
 *
 * MODULE NAME:    sys_config.c
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    t_SYS_Config Wrapper Implementation
 * MAINTAINER:     Ivan Griffin
 * DATE:           25 March 2000
 *
 * SOURCE CONTROL: $Id: sys_config.c,v 1.91 2014/03/11 03:14:08 garyf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *    07 January 2000 -   IG       - first version
 *
 *    
 ***********************************************************************/

#include "sys_config.h"
#include "bt_addr_sync.h"
#include "lmp_const.h"
#include "sys_debug_config.h"
#include "patch_function_id.h"
#include "global_val.h"
/*
 * System configuration structure definition
 */
//extern t_SYS_Config g_sys_config /*= { 0 }*/;

extern void SYShal_config_Initialise(void);

/*
 * Packet attributes definition, access via SYSconst_* accessors
 */
 #ifndef REDUCE_ROM2
const  u_int16 SYSconst_Packet_Length[32]  =
{
    0 /*NULL*/,         0 /*POLL*/,        FHS_PACKET_LENGTH, MAX_DM1_USER_PDU,
    MAX_DH1_USER_PDU,   MAX_HV1_USER_PDU,  MAX_HV2_USER_PDU,  MAX_HV3_USER_PDU,
    MAX_DVACL_USER_PDU, MAX_AUX1_USER_PDU, MAX_DM3_USER_PDU,  MAX_DH3_USER_PDU,
    MAX_EV4_USER_PDU,   MAX_EV5_USER_PDU,  MAX_DM5_USER_PDU,  MAX_DH5_USER_PDU,
    0 /*ID*/,           0 /*INVALID*/,     0 /*INVALID*/,     0 /*INVALID*/,
    MAX_2DH1_USER_PDU,  MAX_EV3_USER_PDU,  MAX_2EV3_USER_PDU, MAX_3EV3_USER_PDU,
    MAX_3DH1_USER_PDU,  MAX_EDR_AUX1_PDU,  MAX_2DH3_USER_PDU, MAX_3DH3_USER_PDU,
    MAX_2EV5_USER_PDU,  MAX_3EV5_USER_PDU, MAX_2DH5_USER_PDU, MAX_3DH5_USER_PDU
};

const u_int16 SYSconst_HCI_Packet_Type[32] = 
{
    0 /*INVALID*/,     0 /*INVALID*/,     0 /*INVALID*/,     HCI_DM1,
    HCI_DH1,           HCI_SYN_HV1,       HCI_SYN_HV2,       HCI_SYN_HV3,     
    0 /*INVALID*/,     0 /*INVALID*/,     HCI_DM3,           HCI_DH3,
    HCI_SYN_EV4,       HCI_SYN_EV5,       HCI_DM5,           HCI_DH5,
    0 /*INVALID*/,     0 /*INVALID*/,     0 /*INVALID*/,     0 /*INVALID*/,
    HCI_2DH1,          HCI_SYN_EV3,       HCI_SYN_2EV3,      HCI_SYN_3EV3,
    HCI_3DH1,          0 /*INVALID*/,     HCI_2DH3,          HCI_3DH3,
    HCI_SYN_2EV5,      HCI_SYN_3EV5,      HCI_2DH5,          HCI_3DH5
};
#endif


const  u_int8 SYSconst_Build_Time_Features[16] =
{
    PRH_BS_CFG_BYTE_0,  PRH_BS_CFG_BYTE_1, 
    PRH_BS_CFG_BYTE_2,  PRH_BS_CFG_BYTE_3,
    PRH_BS_CFG_BYTE_4,  PRH_BS_CFG_BYTE_5,
    PRH_BS_CFG_BYTE_6,  PRH_BS_CFG_BYTE_7,
    PRH_BS_CFG_BYTE_8,  PRH_BS_CFG_BYTE_9,
    PRH_BS_CFG_BYTE_10, PRH_BS_CFG_BYTE_11,
    PRH_BS_CFG_BYTE_12, PRH_BS_CFG_BYTE_13,
    PRH_BS_CFG_BYTE_14, PRH_BS_CFG_BYTE_15
};


/*****************************************************************************
 * SYSconfig_Initialise
 *
 * Set up the major system configuration structure g_sys_config
 ****************************************************************************/
void SYSconfig_Initialise(void)
{    
	
	if(PATCH_FUN[SYSCONFIG_INITIALISE_ID]){
         ((void (*)(void))PATCH_FUN[SYSCONFIG_INITIALISE_ID])();
         return ;
    }

    u_int i;
	u_int8 hci_command_bytes[64];

	/*
     * Record the compile time features LMP in Bytes[0:7], PRH in Bytes [8:15]
     * Features then can be selectively be Run-Time enabled/disabled.
     */
#if 1
    for(i=0; i < sizeof(SYSconst_Build_Time_Features)/
                 sizeof(SYSconst_Build_Time_Features[0]); i++)
    {
        g_sys_config.feature_set[i] = SYSconst_Build_Time_Features[i];
    }
#else
    for(i=0; i < 8; i++)
    {
        g_sys_config.feature_set[i] = SYSconst_Build_Time_Features[i];
        g_sys_config.feature_set[i+8] = 0;
    }
#endif

	/*
	 * Filling hci_command_bytes[]
	 */
	hci_command_bytes[0] = 0x20 ;	//- Disconnect
	hci_command_bytes[1] = 0x0;
	hci_command_bytes[2] = 0x80;    //- Read Remote Version Information
	hci_command_bytes[3] = 0x0;
 	hci_command_bytes[4] = 0x0;	
	hci_command_bytes[5] = 0xC0;    //- Reset / Set Event Mask.
	hci_command_bytes[6] = 0x0;
	hci_command_bytes[7] = 0x02;    //- Read Local Name !!
	hci_command_bytes[8] = 0x0;
	hci_command_bytes[9] = 0x0;
	hci_command_bytes[10] = 0x00;   //-/*Set Host Controller To Host Flow Control*/
	hci_command_bytes[11] = 0x00;	
	hci_command_bytes[12] = 0x00;
	hci_command_bytes[13] = 0x00;
	hci_command_bytes[14] = 0x28;	//- Read Local Version Information/Read Local Support Feature
	hci_command_bytes[15] = 0x02;	//- Read BD ADDR
	hci_command_bytes[16] = 0x00;
    hci_command_bytes[17] = 0x00;
     hci_command_bytes[18] = 0x00;   
     hci_command_bytes[19] = 0x00;
     hci_command_bytes[20] = 0x00;     
	 hci_command_bytes[21] = 0; // AMP
	 hci_command_bytes[22] = 0; // AMP
	 hci_command_bytes[23] = 0; // AMP
#if	0
    hci_command_bytes[24] = (PRH_BS_CFG_SYS_LE_CONTROLLER_SUPPORTED << 5) | // Read LE Host Support
        (PRH_BS_CFG_SYS_LE_CONTROLLER_SUPPORTED << 6); // Write LE Host Support
#else
	hci_command_bytes[24] =  0;
#endif
    //- LE Set Event Mask/ LE Read Buff Size/LE Read Local Supported Feature/
    //- LE Set Rand Addr/LE Set Advertising Parameter/LE Set Advertising Data
    hci_command_bytes[25] = 0xB7;
    //- LE Set Scan Response Data/LE Set Advertise Enable/LE Read White List Size/
    //- LE Clear White List
    hci_command_bytes[26] = 0xC3;
    //- LE Add Device to White List/LE Remove Device from White List/LE Read Channel MAP/
    //- LE Read Remote Used Feature/LE Encrypt/LE Rand
    hci_command_bytes[27] = 0xF3;
    //- LE Long Term Key Request Reply/LE Long Term Key Requested Negative Reply/LE Receiver Test/
    //- LE Transmitter Test/LE Test End
    hci_command_bytes[28] = 0x76;


	for ( i=29; i< sizeof(g_sys_config.hci_command_set)/
                 sizeof(g_sys_config.hci_command_set[0]); i++)
		hci_command_bytes[i] = 0;


	/*
     * Save the compile time supported HCI commands.
     */
    for(i=0; i < sizeof(g_sys_config.hci_command_set)/
                 sizeof(g_sys_config.hci_command_set[0]); i++)
    {
        g_sys_config.hci_command_set[i] = hci_command_bytes[i];
    }

    /*
     * LM & HC Version Information
     * HCI_revision is 1, for spec compliant flow control on comp_id 14
     */
    g_sys_config.version_info.lmp_version = PRH_BS_CFG_LMP_VERSION; 
    g_sys_config.version_info.lmp_subversion = PRH_BS_CFG_LMP_SUBVERSION; 
    g_sys_config.version_info.comp_id = PRH_BS_CFG_MANUFACTURER_NAME;  
    g_sys_config.version_info.HCI_revision = PRH_BS_CFG_HCI_REVISION; 
    g_sys_config.version_info.HCI_version = PRH_BS_CFG_HCI_VERSION;  

    /*
     * HC ACL and SCO Buffer Information
     */
    g_sys_config.hc_buffer_size.aclDataPacketLength = 
        PRH_BS_CFG_SYS_ACL_OUT_PACKET_LENGTH;
	
#ifndef BLUETOOTH_MODE_LE_ONLY
    g_sys_config.hc_buffer_size.scoDataPacketLength = 
        PRH_BS_CFG_SYS_SCO_OUT_PACKET_LENGTH;
#endif//BLUETOOTH_MODE_LE_ONLY        
#if (PRH_BS_CFG_SYS_FLEXIBLE_DATA_BUFFER_SIZES_SUPPORTED==0)
    /*
     * Setup fixed buffer sizes for outgoing SCO/ACL packets.
     */
    g_sys_config.hc_buffer_size.numScoDataPackets = 
        0;//PRH_BS_CFG_SYS_NUM_OUT_SCO_PACKETS;
    g_sys_config.hc_buffer_size.numAclDataPackets =  //15;
        PRH_BS_CFG_SYS_NUM_OUT_ACL_PACKETS;
#endif

    /*
     * Setup Frequency Hopping and Whitening
     */
    g_sys_config.hopping_mode = PRH_BS_CFG_SYS_HOP_MODE;
    g_sys_config.tx_freq = 10;
    g_sys_config.rx_freq = 20;

    /*
     * Default Whitening is On except for SINGLE frequency 
     */
#if (PRH_BS_DEV_WIRED_LINK==0)
    g_sys_config.data_whitening_enable = (g_sys_config.hopping_mode != SINGLE_FREQ);
#else
    g_sys_config.data_whitening_enable = 0;
#endif

    /*
     * Tabasco IP (Jal-G3, Hab-F26) has its correlation window
     * centered 10us early when acting as a slave - compensating
     * for this by making win_ext 10
     */
    g_sys_config.win_ext = PRH_BS_CFG_SYS_DEFAULT_WIN_EXT;

    /*
     * Worse case timing accuracy (clock drift and jitter)
     */
    g_sys_config.clock_jitter_us = 10;
    g_sys_config.clock_drift_ppm = 250;

    /*
     * Set the default SYSrand seed value
     */
    g_sys_config.rand_seed = 0;

#if (PRH_BS_CFG_SYS_DEPRECATED_BT12_FEATURES_SUPPORTED==1)
    /*
     * Set the default unit key
     */
    {
    u_int8 p_unit_key[16] = {0x15, 0x9d, 0xd9, 0xf4, 0x3f, 0xc3, 0xd3, 0x28,
                      0xef, 0xba, 0x0c, 0xd8, 0xa8, 0x61, 0xfa, 0x57 };
    u_int8 i = 0;
    for(i = 0; i < 16; i++)
        g_sys_config.unit_key[i] = p_unit_key[i];
    }
#endif
    /*
     * Set the default class of device to unclassified.
     */
    g_sys_config.device_class = 0x001f00; 

    /*
     * Now do customer specific initialisation
     */
    SYShal_config_Initialise();

    /*
     * Derive the local syncword for this BD_ADDR
     */
    {
        t_syncword *p_syncword;
        p_syncword = &g_sys_config.local_device_syncword;
        BTaddr_Build_Sync_Word(BDADDR_Get_LAP(&g_sys_config.local_device_address),
            &p_syncword->high, &p_syncword->low);
    }

    g_sys_config.max_active_devices = MAX_ACTIVE_DEVICE_LINKS;

	g_sys_config.erroneous_data_reporting = 0x0;

}

