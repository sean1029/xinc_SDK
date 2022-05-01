/*****************************************************************************
 * MODULE NAME:    bt_init.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    Routines to initialise Bluetooth stack higher layers.
 * MAINTAINER:     Ivan Griffin
 * CREATION DATE:  06 August 1999
 *
 * SOURCE CONTROL: $Id: bt_init.c,v 1.90 2013/11/19 02:15:54 garyf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 *****************************************************************************/

#include "sys_config.h"
#include "sys_irq.h"
#include "sys_rand_num_gen.h"
#include "sys_mmap.h"
#include "sys_mmi.h"
#include "sys_power.h"

#include "tra_hcit.h"
#include "tra_queue.h"
#include "tra_codec.h"

#include "bt_init.h"
#include "bt_test.h"
#include "bt_tester.h"
#include "bt_pump.h"

#include "lc_interface.h"

#include "hw_lc.h"
#include "hw_radio.h"
#include "hw_delay.h"

#include "lmp_init.h"
#include "lmp_scan.h" /* for Write Scan Enable */
#include "lmp_ssp.h"
#include "hc_flow_control.h"
#include "hc_event_gen.h"

#include "lslc_irq.h"
#include "sys_init.h"
#include "le_power.h"
#include "patch_function_id.h"
/*****************************************************************************
 * BT_Initialise
 *
 * Initialise all layers and major functions in the device.
 *
 ****************************************************************************/
void BT_Initialise(void)
{
	
	if(PATCH_FUN[BT_INITIALISE_ID]){
         ((void (*)(void))PATCH_FUN[BT_INITIALISE_ID])();
         return ;
    }


	

    /* 
     * Initialise the BT Modules: Transport Queues, Tester, and Pump (Optional)
     */
    
    memset_ble_sleep(); 
    
    BTq_Initialise();
#ifndef BLUETOOTH_MODE_LE_ONLY
    BTtst_Initialise();
    BTpump_Initialise();

    BTtmt_Initialise();
#endif//BLUETOOTH_MODE_LE_ONLY

    /*
     * Map Hardware into Memory
     */
    SYSmmap_Map_BlueStream();
    
    /*
     * Initialise the Random Number Generator with the default seed
     * and some relatively nondeterminitic hardware readings, using
     * pre-reset values so re-seed on HCI reset isn't determinisitc.
     */
    SYSrand_Seed_Rand(SYSconfig_Get_SYS_Rand_Seed()
                      ^ HW_get_bt_clk()
                      ^ HW_get_intraslot_offset()
                      ^ HW_get_uap_lap());

    /* 
     * Initialise the LC Hardware 
     */
    HWlc_Initialise();
 
    /*
     * Reset the radio.
     */
    //HWradio_Reset();
    ((void (*)(void))PATCH_FUN[HWRADIO_RESET])();
    /*
     * For some systems there is additional hardware setup required
     * between radios, therefore sequence here is critical
     * E.g. Clock divider init etc required for radio.
     *      must be after HWradioReset and before HWradio_Initialise.
     */
    SYSinit_Initialise();

    /* 
     * Initialise the RF/DSP Hardware 
     */
    //HWradio_Initialise();
    ((void (*)(void))PATCH_FUN[HWRADIO_INITIALISE])();
    /*
     * Initialise the self-calibrating HW Delay function
     */
    HWdelay_Initialise();

    HWdelay_Calibrate();
    
    /* 
     * Initialise the HCI transport subsystem
     */
    TRAhcit_Initialise();

	/*
	 * Initialise the codec FIFO transport interface
	 */
	TRAcodec_Initialise();

    /*
     * Initialise the IRQ Handlers
     */
    SYSirq_Initialise();

    /* 
     * Initialise the LMP Modules 
     */
    LM_Initialise();

    /*
     * Initialise the HC modules
     */
    HCeg_Initialise();
    HCfc_Initialise();

    /* 
     * Initialise the low power mode sub-system if available
     */
#if (BUILD_TYPE!=UNIT_TEST_BUILD)
    SYSpwr_Initialise();
#endif
    /*
     * Initialise the Link Control Modules (must be last)
     * - Last task in LC_initialise is interrupt initialisation
     * - sys_main will have enabled the IRQs
     */
    LC_Initialise();
}

