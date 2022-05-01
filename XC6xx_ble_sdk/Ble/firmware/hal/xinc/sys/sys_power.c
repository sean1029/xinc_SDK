/*****************************************************************************
 *
 * MODULE NAME:    sys_power.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    Low Power Clock Activation
 * MAINTAINER:     Tom Kerwick
 * DATE:           10 December 2001
 *
 * SOURCE CONTROL: $Id: sys_power.c,v 1.15 2013/09/24 15:04:13 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2008 Ceva Inc.
 *     All rights reserved.
 *
 *****************************************************************************/

#include "sys_config.h"
#include "sys_types.h"
#include "sys_const.h"
#include "sys_power.h"

#include "hw_delay.h"
#include "uslc_chan_ctrl.h"
#include "lslc_irq.h"
#include "sys_hal_features.h"
#include "xinc_m0.h"
#include "sys_debug_config.h"

#if (PRH_BS_CFG_SYS_LOW_POWER_MODE_SUPPORTED==1)


#if (TRA_HCIT_GENERIC_SUPPORTED==1)
extern void (*hci_generic_exit_hc_callback)(void);
#endif

/* can be increased if CPU is slown down and execution takes time */
#define SYS_PWR_MIN_FRAMES_TO_SLEEP      1
#define SYS_PWR_AUX_TIMER_MIN_RESOLUTION 4


/*****************************************************************************
 * FUNCTION:   SYSpwr_Setup_Sleep_Timer()
 * PURPOSE:    Generic Function to setup sleep timer - can be implemented
 *             using any clock source capable of generating interrupt
 *
 *             this version uses AUX_TIMER
 ****************************************************************************/
//static u_int32 syspwr_num_frames_to_sleep_;
/*****************************************************************************
 * FUNCTION:   SYSpwr_Initialise
 * PURPOSE:    
 ****************************************************************************/
void SYSpwr_Initialise(void)
{
  //  syspwr_num_frames_to_sleep_ = 0;
  //  syspwr_low_power_mode_active_ = FALSE;

	/*
	 * Leave low power oscillator disabled until enabled by TCI.
	 */
  //  LowFrequencyOscillatorAvailable = SYS_LF_OSCILLATOR_PRESENT;
}

#endif
