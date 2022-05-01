/*****************************************************************************
 *
 * MODULE NAME:    hw_radio_defs.h
 * PROJECT CODE:   XpertBlue
 * DESCRIPTION:    ORC2110-12 radio driver for Tabasco.
 * MAINTAINER:     Tom Kerwick
 * CREATION DATE:  04.07.12
 *
 * SOURCE CONTROL: $Id: hw_radio_defs.h,v 1.5 2013/11/01 16:10:15 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2012 Ceva Inc.
 *     All rights reserved.
 *
 * RECENT REVISION HISTORY:
 *
 *  04.07.12 TK    initial reference version, untested.
 *  11.10.12 TK    first working version - verified for BR/EDR rates.
 *  20.11.12 TK    added support for BT LE (untested).
 *  01.11.13 TK    updated to BlueRF naming conventions.
 *
 *****************************************************************************/
#ifndef HW_RADIO_DEFS
#define HW_RADIO_DEFS

#include "hw_hab_defs.h"
#include "hw_register.h"
#include "sys_debug_config.h"

/*****************************************************************************
* Hardware Physical Config word:
******************************************************************************/
/*
#if ((__DEBUG_RADIO_MAC_TO_MAC__==1)||(__DEBUG_RADIO_BYPASS__==1))
    #define HWhab_PHY_CFG          0x37016ab8 //- 24MHZ Internal Clock
#else
    #define HWhab_PHY_CFG          0x37016a98 //- 24MHZ External Clock               
#endif
*/
//modify by ql
#if ((__DEBUG_RADIO_MAC_TO_MAC__==1)||(__DEBUG_RADIO_BYPASS__==1))
    #define HWhab_PHY_CFG          0x37016290 //- 16MHZ Internal Clock
#else
    #define HWhab_PHY_CFG          0x37016290 //- 16MHZ External Clock               
#endif

/*****************************************************************************
* ORC2110-12 PROPOGATION DELAYS BETWEEN ANTENNA AND CTRLDATA:
******************************************************************************/
#define mHWradio_RX_TAB_DELAY 5
#define mHWradio_TX_TAB_DELAY 1

#define mHWradio_RX_DELAY (5 + mHWradio_RX_TAB_DELAY)
#define mHWradio_TX_DELAY (4 + mHWradio_TX_TAB_DELAY)

/*****************************************************************************
* Orca Radio Power Level Definitions:
******************************************************************************/
#define HW_RADIO_MAX_TX_POWER_LEVEL 3 /* 4 power levels */
#define HW_RADIO_TX_POWER_STEP_SIZE 5 /* 5dBm step sizes */

#define HW_RADIO_MIN_1MBIT_TX_POWER 0
#define HW_RADIO_MIN_2MBIT_TX_POWER 0
#define HW_RADIO_MIN_3MBIT_TX_POWER 0
 
#define HW_RADIO_MAX_1MBIT_TX_POWER HW_RADIO_MAX_TX_POWER_LEVEL
#define HW_RADIO_MAX_2MBIT_TX_POWER HW_RADIO_MAX_TX_POWER_LEVEL
#define HW_RADIO_MAX_3MBIT_TX_POWER HW_RADIO_MAX_TX_POWER_LEVEL

/*****************************************************************************
* Define the times for GIO line transitions:
******************************************************************************/
//#define HW_RADIO_SETUP_TIME 120 /* SPI TxRxnSel to CTRLDATA available */
#define HW_RADIO_PRE_TX_SYNC_TIME_T_SYNC 20 /* CTRLDATA sync pulse to TX */
#define HW_RADIO_PRE_TX_PULSE_LENGTH 1 /* CTRLDATA sync pulse width */

#define HW_RADIO_TXEN_SETUP_TIME 5 /* TXEN high to CTRLDATA transmit available */
#define HW_RADIO_TXEN_HOLD_TIME 1 /* CTRLDATA transmit complete to TXEN low */

#define HW_RADIO_TIME_TX_SLOT 390 /* max radio time tx slot (acl/syn) */
#define HW_RADIO_TIME_RX_SLOT 390 /* max radio time rx slot (acl/syn) */
#define HW_RADIO_TIME_TX_ID_PACKET_SLOT 80 /* radio time tx slot (id) */

#define HW_RADIO_SLOT_TIME_POS_TX_START         (0)
#define HW_RADIO_SLOT_TIME_POS_TX_MID           (312)
#define HW_RADIO_SLOT_TIME_POS_RX_START         (625)
#define HW_RADIO_SLOT_TIME_POS_RX_MID           (937)
#define HW_RADIO_TOTAL_SLOT_TIME                (1250)
#define HW_RADIO_NORMAL_TIME_FOR_SPI_WRITE      (16)


// BDATA1 signal (Blue RF) denotes CTRLDATA0 signal in Orca's ORC2110-12 Radio Specification
#define HW_RADIO_PRE_TX_BDATA1_PULSE_HIGH_NORM (HW_RADIO_TOTAL_SLOT_TIME - HW_RADIO_PRE_TX_SYNC_TIME_T_SYNC - mHWradio_TX_DELAY)
#define HW_RADIO_PRE_TX_BDATA1_PULSE_HIGH_SECOND (HW_RADIO_SLOT_TIME_POS_TX_MID - HW_RADIO_PRE_TX_SYNC_TIME_T_SYNC  - mHWradio_TX_DELAY)
#define HW_RADIO_PRE_TX_BDATA1_PULSE_LOW_NORM (HW_RADIO_PRE_TX_BDATA1_PULSE_HIGH_NORM + HW_RADIO_PRE_TX_PULSE_LENGTH)
#define HW_RADIO_PRE_TX_BDATA1_PULSE_LOW_SECOND (HW_RADIO_PRE_TX_BDATA1_PULSE_HIGH_SECOND + HW_RADIO_PRE_TX_PULSE_LENGTH)

// BPKTCTL signal (Blue RF) denotes TXRXEN signal in Orca's ORC2110-12 Radio Specification
#define HW_RADIO_SET_BPKTCTL_HIGH_PRE_TX_NORM ((HW_RADIO_TOTAL_SLOT_TIME - HW_RADIO_TXEN_SETUP_TIME - mHWradio_TX_DELAY - 1) <<16)
#define HW_RADIO_SET_BPKTCTL_HIGH_PRE_TX_FIRST ((HW_RADIO_TOTAL_SLOT_TIME - HW_RADIO_TXEN_SETUP_TIME - mHWradio_TX_DELAY - 1) <<16)
#define HW_RADIO_SET_BPKTCTL_HIGH_PRE_TX_SECOND ((HW_RADIO_SLOT_TIME_POS_TX_MID - HW_RADIO_TXEN_SETUP_TIME - mHWradio_TX_DELAY - 1) <<16)

#define HW_RADIO_SET_BPKTCTL_LOW_END_TX_NORM ((HW_RADIO_SLOT_TIME_POS_TX_START + HW_RADIO_TIME_TX_SLOT + HW_RADIO_TXEN_HOLD_TIME) <<16)
#define HW_RADIO_SET_BPKTCTL_LOW_END_TX_FIRST ((HW_RADIO_SLOT_TIME_POS_TX_START + HW_RADIO_TIME_TX_ID_PACKET_SLOT + HW_RADIO_TXEN_HOLD_TIME) <<16)
#define HW_RADIO_SET_BPKTCTL_LOW_END_TX_SECOND ((HW_RADIO_SLOT_TIME_POS_TX_MID + HW_RADIO_TIME_TX_ID_PACKET_SLOT + HW_RADIO_TXEN_HOLD_TIME) <<16)
#define HW_RADIO_SET_BPKTCTL_LOW_END_TX_SLAVE_PAGE_RESP ((HW_RADIO_SLOT_TIME_POS_TX_START + HW_RADIO_TIME_TX_ID_PACKET_SLOT + HW_RADIO_TXEN_HOLD_TIME) <<16)

#if 1//(PRH_BS_CFG_SYS_LE_CONTROLLER_SUPPORTED==1)

#define HW_RADIO_LE_TIFS                        (150)
#define HW_RADIO_LE_TIFS_B4_TX                 (0x0700) // LE_check = 3. Tx_next = 1.
#define HW_RADIO_LE_TIFS_B4_RX                 (0x0600) // LE_check = 3. Tx_next = 0.

//#define HW_RADIO_LE_PRE_TX_BDATA1_PULSE_HIGH_TIFS (HW_RADIO_LE_TIFS_B4_TX + HW_RADIO_LE_TIFS - HW_RADIO_PRE_TX_SYNC_TIME_T_SYNC - mHWradio_TX_DELAY)
//#define HW_RADIO_LE_PRE_TX_BDATA1_PULSE_LOW_TIFS (HW_RADIO_LE_PRE_TX_BDATA1_PULSE_HIGH_TIFS + HW_RADIO_PRE_TX_PULSE_LENGTH)

//#define HW_RADIO_LE_PRE_TX_BDATA1_PULSE_HIGH_TIFS   (0x0700 + 50+15)
//#define HW_RADIO_LE_PRE_TX_BDATA1_PULSE_LOW_TIFS   (0x0700 + 51+15)
#define HW_RADIO_LE_PRE_TX_BDATA1_PULSE_HIGH_TIFS   (0x0700 + 85+15)
#define HW_RADIO_LE_PRE_TX_BDATA1_PULSE_LOW_TIFS   (0x0700 + 86+15)

//#define HW_RADIO_LE_SET_BPKTCTL_HIGH_PRE_TX_TIFS ((HW_RADIO_LE_TIFS_B4_TX + HW_RADIO_LE_TIFS - HW_RADIO_TXEN_SETUP_TIME - mHWradio_TX_DELAY - 1) <<16)
//#define HW_RADIO_LE_SET_BPKTCTL_LOW_END_TX_TIFS ((HW_RADIO_LE_TIFS_B4_RX + HW_RADIO_TXEN_HOLD_TIME) <<16)

//#define HW_RADIO_LE_SET_BPKTCTL_HIGH_PRE_TX_TIFS ((0x0700 + 90)<<16)
#define HW_RADIO_LE_SET_BPKTCTL_HIGH_PRE_TX_TIFS ((0x0700  + 115)<<16)
#define HW_RADIO_LE_SET_BPKTCTL_LOW_END_TX_TIFS ((0x600 + 15)<<16)

#endif


/*************************************************************************************
 *  The following is a defines the allocation of GIO lines in the Orca RF driver
 *
 * GIO 0	 Used to change BDATA1 direction from receive to transmit.
 * GIO 1	 Unused
 *
 * GIO 2	 Unused
 * GIO 3	 PDN Enable/Disable Crystal. Sleep mode
 *
 * GIO 4	 RESETN - Set low, places radio in lowest Power state. Config data is lost.
 * GIO 5	 Unused
 *
 * GIO 6	 Combine with GIO 8 for Sync Tx pulse for BDATA1 when in page mode.
 * GIO 7	 Unused
 *
 * GIO 8	 Creates Sync Tx pulse for combination with BDATA1.
 * GIO 9	 BPKTCTL - In Transmit mode its a strobe to Enable the PA.
 *                     In receive the baseband generates this to  
 *					   indicate that the access code has been decoded.
 * GIO 10	 Unused
 * GIO 11	 BPKTCTL combine for page mode.
 *************************************************************************************/
#define HW_RADIO_GIO_HIGH_DATA_DIR					HAB_GIO_HIGH_CTRL_1_AND_0_REG
#define HW_RADIO_GIO_LOW_DATA_DIR					HAB_GIO_LOW_CTRL_1_AND_0_REG
#define HW_RADIO_GIO_OVERRIDE_MASK_DATA_DIR			0x00008000

#define HW_RADIO_GIO_HIGH_PDN						HAB_GIO_HIGH_CTRL_3_AND_2_REG
#define HW_RADIO_GIO_LOW_PDN						HAB_GIO_LOW_CTRL_3_AND_2_REG
#define HW_RADIO_GIO_OVERRIDE_MASK_PDN				0x80000000

#define HW_RADIO_GIO_HIGH_RESETN					HAB_GIO_HIGH_CTRL_5_AND_4_REG
#define HW_RADIO_GIO_LOW_RESETN						HAB_GIO_LOW_CTRL_5_AND_4_REG
#define HW_RADIO_GIO_OVERRIDE_MASK_RESETN			0x00008000

#define HW_RADIO_GIO_HIGH_TX_EN						HAB_GIO_HIGH_CTRL_5_AND_4_REG//HAB_GIO_HIGH_CTRL_7_AND_6_REG
#define HW_RADIO_GIO_LOW_TX_EN						HAB_GIO_LOW_CTRL_5_AND_4_REG//HAB_GIO_LOW_CTRL_7_AND_6_REG
#define HW_RADIO_GIO_OVERRIDE_MASK_TX_EN			0x80000000

#define HW_RADIO_GIO_HIGH_RX_EN						HAB_GIO_HIGH_CTRL_3_AND_2_REG//HAB_GIO_HIGH_CTRL_B_AND_A_REG
#define HW_RADIO_GIO_LOW_RX_EN						HAB_GIO_LOW_CTRL_3_AND_2_REG//HAB_GIO_LOW_CTRL_B_AND_A_REG
#define HW_RADIO_GIO_OVERRIDE_MASK_RX_EN			0x00008000

#define HW_RADIO_GIO_HIGH_TX_SYNC_2		            HAB_GIO_HIGH_CTRL_7_AND_6_REG
#define HW_RADIO_GIO_LOW_TX_SYNC_2			        HAB_GIO_LOW_CTRL_7_AND_6_REG
#define HW_RADIO_GIO_OVERRIDE_TX_SYNC_2 			0x00008000

#define HW_RADIO_GIO_HIGH_BPKTCTL					HAB_GIO_HIGH_CTRL_9_AND_8_REG
#define HW_RADIO_GIO_LOW_BPKTCTL						HAB_GIO_LOW_CTRL_9_AND_8_REG
#define HW_RADIO_GIO_OVERRIDE_MASK_BPKTCTL			0x80000000

#define HW_RADIO_GIO_HIGH_TX_SYNC_1					HAB_GIO_HIGH_CTRL_9_AND_8_REG
#define HW_RADIO_GIO_LOW_TX_SYNC_1					HAB_GIO_LOW_CTRL_9_AND_8_REG
#define HW_RADIO_GIO_OVERRIDE_MASK_TX_SYNC_1		0x00008000

#define HW_RADIO_GIO_HIGH_BPKTCTL_AND_TX_SYNC_1		HAB_GIO_HIGH_CTRL_9_AND_8_REG
#define HW_RADIO_GIO_LOW_BPKTCTL_AND_TX_SYNC_1		HAB_GIO_LOW_CTRL_9_AND_8_REG
#define HW_RADIO_GIO_OVERRIDE_MASK_BPKTCTL_AND_TX_SYNC_1		0x80008000

#define HW_RADIO_GIO_HIGH_BPKTCTL_2	                HAB_GIO_HIGH_CTRL_B_AND_A_REG
#define HW_RADIO_GIO_LOW_BPKTCTL_2	                HAB_GIO_LOW_CTRL_B_AND_A_REG
#define HW_RADIO_GIO_OVERRIDE_MASK_BPKTCTL_2		0x80000000

#define HW_RADIO_COMB_BPKTCTL_SYNC_FOR_DOUBLE_WIN	HAB_GIO_CTRL_COMB_B_A_9_8_REG

/*****************************************************************************
* Register bit definitions
******************************************************************************/
#define mBIT(X) (((u_int32)1) << (X))

/*****************************************************************************
*
* ORC2110-12 REGISTER READS:
* bit 31 = 0 for SPI block to read
* bits 22..24 => 101 device address.
* Read from Radio - bit 21 = 1.
* reg address N - bit 16..20
*
******************************************************************************/
//#define READ_REG(n)  (0x01600000|((n)<<16))

/*****************************************************************************
*
* ORC2110-12 REGISTER WRITES:
* bit 31 = 1 for SPI block to write
* bits 22..24 => 101 device address.
* Write to Radio - bit 21 = 0.
* reg address N - bit 16..20
*
******************************************************************************/
//#define WRITE_REG(n,val)  (0x81400000|(((u_int32)n)<<16)|(val))
/*****************************************************************************
*
* ORC2110-12 REGISTER WRITES:
* bit 31..30 = 11 for SPI block to write
* bits 16..27 =>  device address.
* Write data to Radio - bit 0..15.
*
******************************************************************************/
//#define WRITE_REG_ZSW(n,val)  (0x80000000|(((u_int32)n)<<16)|(val))
#define WRITE_REG_RF(regAddr,regValue) (0x80000000|((regAddr)<<16)|(regValue))
/*****************************************************************************
*
* ORC2110-12 REGISTER CONFIGURATIONS:
*
******************************************************************************/
/*#define WRITE_REG1_BT_RX(ch)		WRITE_REG(0x01,0x8000|(ch<<1))
#define WRITE_REG1_BT_TX(ch)		WRITE_REG(0x01,0x4001|(ch<<1))
#define WRITE_REG1_LE_RX(ch)		WRITE_REG(0x01,0x8100|(ch<<1))
#define WRITE_REG1_LE_TX(ch)		WRITE_REG(0x01,0x4101|(ch<<1))

#define WRITE_REG1_IDLE				WRITE_REG(0x01,0x0001)

#define WRITE_REG8_TX_POWER(x)      WRITE_REG(0x08,(u_int16)Power_control_table[x])
*/
/*****************************************************************************
*
* Defines to support high/low override control of GIO lines (debug purposes):
*
* HWradio_SetOverrideLow(GIO_N);
* HWradio_SetOverrideHigh(GIO_N);
*
******************************************************************************/
#define HW_RADIO_GIO_LOW_GIO_0              HAB_GIO_HIGH_CTRL_1_AND_0_REG
#define HW_RADIO_GIO_HIGH_GIO_0             HAB_GIO_LOW_CTRL_1_AND_0_REG
#define HW_RADIO_GIO_OVERRIDE_MASK_GIO_0    0x00008000

#define HW_RADIO_GIO_LOW_GIO_1              HAB_GIO_HIGH_CTRL_1_AND_0_REG
#define HW_RADIO_GIO_HIGH_GIO_1             HAB_GIO_LOW_CTRL_1_AND_0_REG
#define HW_RADIO_GIO_OVERRIDE_MASK_GIO_1    0x80000000

#define HW_RADIO_GIO_LOW_GIO_2              HAB_GIO_HIGH_CTRL_3_AND_2_REG
#define HW_RADIO_GIO_HIGH_GIO_2             HAB_GIO_LOW_CTRL_3_AND_2_REG
#define HW_RADIO_GIO_OVERRIDE_MASK_GIO_2    0x00008000

#define HW_RADIO_GIO_LOW_GIO_3              HAB_GIO_HIGH_CTRL_3_AND_2_REG
#define HW_RADIO_GIO_HIGH_GIO_3             HAB_GIO_LOW_CTRL_3_AND_2_REG
#define HW_RADIO_GIO_OVERRIDE_MASK_GIO_3    0x80000000

#define HW_RADIO_GIO_LOW_GIO_4              HAB_GIO_HIGH_CTRL_5_AND_4_REG
#define HW_RADIO_GIO_HIGH_GIO_4             HAB_GIO_LOW_CTRL_5_AND_4_REG
#define HW_RADIO_GIO_OVERRIDE_MASK_GIO_4    0x00008000

#define HW_RADIO_GIO_LOW_GIO_5              HAB_GIO_HIGH_CTRL_5_AND_4_REG
#define HW_RADIO_GIO_HIGH_GIO_5             HAB_GIO_LOW_CTRL_5_AND_4_REG
#define HW_RADIO_GIO_OVERRIDE_MASK_GIO_5    0x80000000

#define HW_RADIO_GIO_LOW_GIO_6              HAB_GIO_HIGH_CTRL_7_AND_6_REG
#define HW_RADIO_GIO_HIGH_GIO_6             HAB_GIO_LOW_CTRL_7_AND_6_REG
#define HW_RADIO_GIO_OVERRIDE_MASK_GIO_6    0x00008000

#define HW_RADIO_GIO_LOW_GIO_7              HAB_GIO_HIGH_CTRL_7_AND_6_REG
#define HW_RADIO_GIO_HIGH_GIO_7             HAB_GIO_LOW_CTRL_7_AND_6_REG
#define HW_RADIO_GIO_OVERRIDE_MASK_GIO_7    0x80000000

#define HW_RADIO_GIO_LOW_GIO_8              HAB_GIO_HIGH_CTRL_9_AND_8_REG
#define HW_RADIO_GIO_HIGH_GIO_8             HAB_GIO_LOW_CTRL_9_AND_8_REG
#define HW_RADIO_GIO_OVERRIDE_MASK_GIO_8    0x00008000

#define HW_RADIO_GIO_LOW_GIO_9              HAB_GIO_HIGH_CTRL_9_AND_8_REG
#define HW_RADIO_GIO_HIGH_GIO_9             HAB_GIO_LOW_CTRL_9_AND_8_REG
#define HW_RADIO_GIO_OVERRIDE_MASK_GIO_9    0x80000000

#define HW_RADIO_GIO_LOW_GIO_A              HAB_GIO_HIGH_CTRL_B_AND_A_REG
#define HW_RADIO_GIO_HIGH_GIO_A             HAB_GIO_LOW_CTRL_B_AND_A_REG
#define HW_RADIO_GIO_OVERRIDE_MASK_GIO_A    0x00008000

#define HW_RADIO_GIO_LOW_GIO_B              HAB_GIO_HIGH_CTRL_B_AND_A_REG
#define HW_RADIO_GIO_HIGH_GIO_B             HAB_GIO_LOW_CTRL_B_AND_A_REG
#define HW_RADIO_GIO_OVERRIDE_MASK_GIO_B    0x80000000

/*****************************************************************************
* GIO override macros
******************************************************************************/
#define HWradio_SetOverrideLow(GIO_NAME)	\
				mHWreg_Logical_OR_With_Register(HW_RADIO_GIO_LOW_##GIO_NAME, \
					HW_RADIO_GIO_OVERRIDE_MASK_##GIO_NAME); \
				mHWreg_Logical_AND_With_Register(HW_RADIO_GIO_HIGH_##GIO_NAME, \
					~(HW_RADIO_GIO_OVERRIDE_MASK_##GIO_NAME))

#define HWradio_SetOverrideHigh(GIO_NAME)	\
				mHWreg_Logical_OR_With_Register(HW_RADIO_GIO_HIGH_##GIO_NAME, \
					HW_RADIO_GIO_OVERRIDE_MASK_##GIO_NAME); \
				mHWreg_Logical_AND_With_Register(HW_RADIO_GIO_LOW_##GIO_NAME, \
					~(HW_RADIO_GIO_OVERRIDE_MASK_##GIO_NAME)) 

#define HWradio_CancelOverride(GIO_NAME) \
				mHWreg_Logical_AND_With_Register(HW_RADIO_GIO_LOW_##GIO_NAME, \
					~(HW_RADIO_GIO_OVERRIDE_MASK_##GIO_NAME)); \
				mHWreg_Logical_AND_With_Register(HW_RADIO_GIO_HIGH_##GIO_NAME, \
					~(HW_RADIO_GIO_OVERRIDE_MASK_##GIO_NAME))

#endif // HW_RADIO_DEFS
