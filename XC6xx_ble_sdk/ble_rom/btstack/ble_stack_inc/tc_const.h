#ifndef _PARTHUS_TC_CONST_
#define _PARTHUS_TC_CONST_

/**********************************************************************
 *
 * MODULE NAME:    tc_const.h
 * PROJECT:       BlueStream
 * DESCRIPTION:   Constants used by the TCI (Command Opcodes etc.)
 * MAINTAINER:     Daire McNamara <Daire McNamara@sslinc.com>
 * CREATION DATE:  13 April 2000
 *
 * SOURCE CONTROL: $Id: tc_const.h,v 1.65 2013/06/20 11:54:05 garyf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *
 **********************************************************************/

/*
 * TCI SYSTEM TEST OP CODES - OPCODE GROUP SUBFIELD = 0x3F
 */
#define ST_G ( 0x3F << 10)

#define TCI_ACTIVATE_REMOTE_DUT                         0x0002 + ST_G
#define TCI_TEST_CONTROL                                0x0003 + ST_G
#define TCI_INCREASE_REMOTE_POWER                       0x0004 + ST_G
#define TCI_WRITE_LOCAL_HOP_FREQUENCIES                 0x0005 + ST_G
#define TCI_READ_LOCAL_HARDWARE_VERSION                 0x0006 + ST_G
#define TCI_DECREASE_REMOTE_POWER                       0x0007 + ST_G
#define TCI_INCREASE_LOCAL_VOLUME                       0x0008 + ST_G
#define TCI_DECREASE_LOCAL_VOLUME                       0x0009 + ST_G
#define TCI_WRITE_LOCAL_NATIVE_CLOCK                    0x000A + ST_G
#define TCI_READ_LOCAL_NATIVE_CLOCK                     0x000B + ST_G
#define TCI_READ_LOCAL_HOST_CONTROLLER_RELATIVE_MIPS    0x000C + ST_G
#define TCI_TYPE_APPROVAL_TEST_CONTROL                  0x000D + ST_G
#define TCI_SET_LOCAL_FAILED_ATTEMPTS_COUNTER           0x000E + ST_G
#define TCI_CLEAR_LOCAL_FAILED_ATTEMPTS_COUNTER         0x000F + ST_G
#define TCI_READ_LOCAL_DEFAULT_PACKET_TYPE              0x0010 + ST_G
#define TCI_WRITE_LOCAL_DEFAULT_PACKET_TYPE             0x0011 + ST_G
#define TCI_WRITE_LOCAL_SYNCWORD                        0x0012 + ST_G
#define TCI_WRITE_LOCAL_HOPPING_MODE                    0x0013 + ST_G
#define TCI_READ_LOCAL_HOPPING_MODE                     0x0014 + ST_G
#define TCI_WRITE_LOCAL_WHITENING_ENABLE                0x0015 + ST_G
#define TCI_READ_LOCAL_WHITENING_ENABLE                 0x0016 + ST_G
#define TCI_WRITE_LOCAL_RADIO_POWER                     0x0017 + ST_G
#define TCI_READ_LOCAL_RADIO_POWER                      0x0018 + ST_G
#define TCI_SET_LOCAL_NEXT_AVAILABLE_AM_ADDR            0x0019 + ST_G
#define TCI_SET_LOCAL_BD_ADDR                           0x001A + ST_G
#define TCI_WRITE_LOCAL_LINK_KEY_TYPE                   0x001B + ST_G
#define TCI_READ_LOCAL_LINK_KEY_TYPE                    0x001C + ST_G
#define TCI_READ_LOCAL_EXTENDED_FEATURES                0x001D + ST_G
#define TCI_WRITE_LOCAL_FEATURES                        0x001E + ST_G
#define TCI_WRITE_LOCAL_EXTENDED_FEATURES               0x001F + ST_G
#define TCI_READ_LOCAL_TIMING_INFORMATION               0x002A + ST_G
#define TCI_WRITE_LOCAL_TIMING_INFORMATION              0x002B + ST_G
#define TCI_READ_REMOTE_TIMING_INFORMATION              0x002C + ST_G
#define TCI_WRITE_LOCAL_HARDWARE_REGISTER               0x002D + ST_G
#define TCI_RESET_LOCAL_BASEBAND_MONITORS               0x002E + ST_G

#define TCI_WRITE_LOCAL_RADIO_REGISTER                  0x0030 + ST_G
#define TCI_READ_LOCAL_RADIO_REGISTER                   0x0031 + ST_G
#define TCI_CHANGE_RADIO_MODULATION                     0x0032 + ST_G
#define TCI_READ_RADIO_MODULATION                       0x0033 + ST_G
#define TCI_SET_HCIT_UART_BAUD_RATE                     0x0034 + ST_G

#define TCI_SEND_ENCRYPTION_KEY_SIZE_MASK_REQ           0x0039 + ST_G
#define TCI_RESET_LOCAL_PUMP_MONITORS                   0x003A + ST_G
#define TCI_READ_LOCAL_PUMP_MONITORS                    0x003B + ST_G
#define TCI_WRITE_LOCAL_ENCRYPTION_KEY_LENGTH           0x003C + ST_G
#define TCI_READ_LOCAL_ENCRYPTION_KEY_LENGTH            0x003D + ST_G
#define TCI_READ_LOCAL_HOP_FREQUENCIES                  0x003E + ST_G
#define TCI_READ_LOCAL_BASEBAND_MONITORS                0x003F + ST_G
#define TCI_SET_DISABLE_LOW_POWER_MODE                  0x0040 + ST_G
#define TCI_SET_ENABLE_LOW_POWER_MODE                   0x0041 + ST_G
#define TCI_READ_R2P_MIN_SEARCH_WINDOW                  0x0042 + ST_G
#define TCI_WRITE_R2P_MIN_SEARCH_WINDOW                 0x0043 + ST_G
#define TCI_SET_DISABLE_SCO_VIA_HCI                     0x0044 + ST_G
#define TCI_SET_ENABLE_SCO_VIA_HCI                      0x0045 + ST_G

#define TCI_WRITE_ESCO_RETRANSMISSION_MODE              0x0046 + ST_G
#define TCI_READ_ESCO_RETRANSMISSION_MODE               0x0047 + ST_G

#define TCI_WRITE_EPC_ENABLE                            0x0048 + ST_G
#define TCI_WRITE_PTA_ENABLE							0x0049 + ST_G

#define TCI_VCI_CLK_OVERRIDE                            0x004E + ST_G
#define TCI_SET_BROADCAST_SCAN_WINDOW                   0x004F + ST_G
#define TCI_WRITE_PARK_PARAMETERS                       0x0050 + ST_G
#define TCI_READ_UNUSED_STACK_SPACE                     0x0051 + ST_G

#define TCI_WRITE_AFH_CONTROL                           0x0060 + ST_G

#define TCI_READ_RAW_RSSI                               0x0061 + ST_G
#define TCI_READ_BER                                    0x0062 + ST_G
#define TCI_READ_PER                                    0x0063 + ST_G
#define TCI_READ_RAW_RSSI_PER_BER                       0x0064 + ST_G

#define TCI_WRITE_SECURITY_TIMEOUTS                     0x0066 + ST_G


#if(PRH_BS_CFG_SYS_BROADCAST_NULL_IN_INQ_PAGE_SUPPORTED==1)
#define TCI_SET_EMERGENCY_POLL_INTERVAL					0x0069 + ST_G
#endif

#define TCI_WRITE_FEATURES                              0x0067 + ST_G

#define TCI_LE_SET_TRANSMIT_WINDOW_PARAMS               0x006A + ST_G
#define TCI_LE_SET_DIRECT_ADVERTISING_TIMEOUT           0x006B + ST_G
#define TCI_LE_SET_TIFS_TX_ADJUSTMENT                   0x006C + ST_G
#define TCI_LE_SET_SEARCH_WINDOW_DELAY                  0x006D + ST_G

#define TCI_LE_AUTO_ADVERTISE_AFTER_SLAVE_DISCONNECT   0x70 + ST_G
#define TCI_LE_AUTO_INITIATE_AFTER_MASTER_DISCONNECT   0x71 + ST_G
#define TCI_LE_READ_ADV_PARAMS                         0x72 + ST_G
#define TCI_LE_WRITE_ADVERTISING_DELTA                 0x73 + ST_G
#define TCI_LE_WRITE_NUM_PACKETS_PER_CE                0x74 + ST_G
#define TCI_LE_ECHO_TRANSMIT_WINDOW_SIZE_AND_OFFSET    0x75 + ST_G
#define TCI_LE_GET_DEVICE_STATES                       0x76 + ST_G
#define TCI_LE_READ_SCAN_BACKOFF_INFO                  0x77 + ST_G
#define TCI_LE_WRITE_SCAN_FREQUENCIES                  0x78 + ST_G
#define TCI_LE_READ_SCAN_FREQUENCIES                   0x79 + ST_G
#define TCI_LE_WRITE_INITIATING_FREQUENCIES            0x7A + ST_G
#define TCI_LE_READ_INITIATING_FREQUENCIES             0x7B + ST_G
#define TCI_LE_NUM_TIMES_MASTER_DOESNT_TX_FIRST_WIN    0x7C + ST_G
#define TCI_LE_SLAVE_LISTEN_OUTSIDE_LATENCY            0x7D + ST_G
#define TCI_LE_READ_PEER_SCA                           0x7E + ST_G
#define TCI_LE_READ_SESSION_KEY                        0x7F + ST_G
#define TCI_LE_READ_HOP_INCREMENT                      0x80 + ST_G
#define TCI_LE_READ_ACCESS_CODE                        0x81 + ST_G
#define TCI_LE_WHITENING_ENABLE                        0x82 + ST_G
#define TCI_LE_ENABLE_SCAN_BACKOFF                     0x83 + ST_G
#define TCI_LE_PRECONFIGURE_HOP_INC					   0x84 + ST_G
#define TCI_LE_PRECONFIGURE_ACCESS_CODE				   0x85 + ST_G
#define TCI_LE_DIRECT_ADV_TIMEOUT                      0x86 + ST_G
#define TCI_LE_READ_SCAN_PARAMS                        0x87 + ST_G
#define TCI_LE_SET_TRACE_LEVEL                         0x88 + ST_G
#define TCI_LE_READ_LOCAL_POWER_LEVEL                  0x89 + ST_G
#define TCI_LE_INC_LOCAL_POWER_LEVEL                   0x8A + ST_G
#define TCI_LE_DEC_LOCAL_POWER_LEVEL                   0x8B + ST_G
/*
 * Pure Debug TCIs - only usefull to assist stress testing the system
 */
//#define TCI_LE_AUTO_REENABLE_ADVERTISING_IN_SLAVE_AFTER_DISCONNECT
//#define TCI_LE_AUTO_RESTART_INITIATING_IN_MASTER_AFTER_DISCONNECT

#endif
