/*
 * MODULE NAME:    hw_delay.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    
 * MAINTAINER:     Ivan Griffin
 * DATE:           12 September 2001
 *
 * SOURCE CONTROL: $Id: hw_delay.c,v 1.27 2014/03/11 03:13:36 garyf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2001-2004 Ceva Inc.
 *     All rights reserved.
 */

#include "sys_config.h"
#include "sys_types.h"
#include "sys_irq.h"
#include "hw_lc.h"
#include "hw_le_lc_impl.h"
#include "hw_register.h"
#include "hw_macro_defs.h"
#include "hw_delay.h"
#include "bt_timer.h"

#include "sys_mmi.h"
#include "tra_hcit.h"
#include "global_val.h"
#include "patch_function_id.h"

/*
 * This Variables will be used during Power Up
 * used average but can be defined in HAL
 */
//#ifndef SYS_HAL_PRECALIBRATED_DELAY_MS
//#define SYS_HAL_PRECALIBRATED_DELAY_MS 1500
//#endif

//#ifndef SYS_HAL_PRECALIBRATED_DELAY_10US
//#define SYS_HAL_PRECALIBRATED_DELAY_10US 15
//#endif

//#ifndef SYS_HAL_PRECALIBRATED_DELAY_US
//#define SYS_HAL_PRECALIBRATED_DELAY_US 1
//#endif

//static u_int32 calibration_counter_1ms=SYS_HAL_PRECALIBRATED_DELAY_MS;
//static u_int32 calibration_counter_10us=SYS_HAL_PRECALIBRATED_DELAY_10US;
//static u_int32 calibration_counter_1us=SYS_HAL_PRECALIBRATED_DELAY_US;
//volatile u_int32 hw_delay_counter; /* extern'd for TCI use */

static void HWdelay_Calibrate_(void);

#define US_CALIBR (64)
#define MS_CALIBR (16)

/************************************************************************
 * HWdelay_Initialise
 *
 * Initialise and calibrate the 1us/10us delay functions counters
 ************************************************************************/
void HWdelay_Initialise(void)
{
    calibration_counter_1ms = 0;
    calibration_counter_10us = 0;
    calibration_counter_1us = 0;
    HWdelay_Calibrate_();
}



/************************************************************************
 * HWdelay_Calibrate
 *
 * Try audit calibrated delays and adjust based of the known intervals
 ************************************************************************/

void HWdelay_Calibrate(void)
{
	if(PATCH_FUN[HWDELAY_CALIBRATE_ID]){
         ((void (*)(void))PATCH_FUN[HWDELAY_CALIBRATE_ID])();

         return ;
    }	
#if (BUILD_TYPE!=UNIT_TEST_BUILD)
    u_int32 isr_flags;

    u_int32 init_nat_cnt, final_nat_cnt, cnt, scale;
    u_int32 init_bt, final_bt;

    SYSirq_Disable_Interrupts_Save_Flags(&isr_flags);

    // fine tuning delay we have precalibrated delay count against known interval
    // then scale the calibration coeffs
	init_nat_cnt = mGetHWEntry(JAL_NAT_BIT_CNT);
	HWdelay_Wait_For_10us(US_CALIBR); 
	final_nat_cnt = mGetHWEntry(JAL_NAT_BIT_CNT);
	if (final_nat_cnt > init_nat_cnt)
	{
		cnt = (final_nat_cnt - init_nat_cnt);// % 1250;
	}
	else
	{
		cnt = 1250 - (init_nat_cnt - final_nat_cnt);
	}
	calibration_counter_10us = calibration_counter_10us *US_CALIBR*10 /cnt;

	//calibrate ms delay
	init_bt = BTtimer_Get_Native_Clock();
	init_nat_cnt = mGetHWEntry(JAL_NAT_BIT_CNT);
	HWdelay_Wait_For_ms(MS_CALIBR, FALSE); //
	final_bt = BTtimer_Get_Native_Clock();
	final_nat_cnt = mGetHWEntry(JAL_NAT_BIT_CNT);

	//check the clock wrap conditions
	if ((3 == (init_bt & 3) ) && (init_nat_cnt < 312))
	{	
			init_nat_cnt += 1250;
	}
	
	if ((3 == (final_bt & 3)) && (final_nat_cnt < 312))
	{	
			final_nat_cnt += 1250;
	}

	cnt = BTtimer_Clock_Difference(init_bt, final_bt);
	cnt >>= 2; // remove 2 least bits that set the native clock quarter as we have exact bit count value
	scale = (cnt*1250 + final_nat_cnt - init_nat_cnt);

	calibration_counter_1ms = (u_int64) calibration_counter_1ms * MS_CALIBR *1000 / scale;

    SYSirq_Interrupts_Restore_Flags(isr_flags);

#endif

}
/************************************************************************
 * HWdelay_Calibrate_
 *
 * Calibrate the 1us/10us delay functions
 ************************************************************************/
static void HWdelay_Calibrate_(void)
{
	if(PATCH_FUN[HWDELAY_CALIBRATE__ID]){
         ((void (*)(void))PATCH_FUN[HWDELAY_CALIBRATE__ID])();
         return ;
    }

#if (BUILD_TYPE!=UNIT_TEST_BUILD)
    u_int32 delay_counter = 0;
   
    u_int32 isr_flags;

    mHWreg_Create_Cache_Register(JAL_COM_STAT_IRQ_REG);
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_IRQ_REG);

    SYSirq_Disable_Interrupts_Save_Flags(&isr_flags);

    /* Clear all interrupts */
    mHWreg_Assign_Register(JAL_COM_CTRL_IRQ_REG, 0xFFFFFFFF);
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_IRQ_REG);

    /* Enable TIM0 */
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM0_INTR_MSK);
    mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_IRQ_REG);

    /* Wait for 1st TIM0 */
    mHWreg_Load_Cache_Register(JAL_COM_STAT_IRQ_REG);
    while (mHWreg_If_Cache_Bit_is_0(JAL_COM_STAT_IRQ_REG, JAL_TIM0_INTR))
    {
        mHWreg_Load_Cache_Register(JAL_COM_STAT_IRQ_REG);
    }

    /* clear TIM0, wait for second, incrementing a delay counter */
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM0_INTR_CLR);
    mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);

    /* Wait for 1st TIM0 */
    mHWreg_Load_Cache_Register(JAL_COM_STAT_IRQ_REG);
    while (mHWreg_If_Cache_Bit_is_0(JAL_COM_STAT_IRQ_REG, JAL_TIM0_INTR))
    {
        mHWreg_Load_Cache_Register(JAL_COM_STAT_IRQ_REG);
    }

    /* clear TIM0, wait for second, incrementing a delay counter */
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM0_INTR_CLR);
    mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);

    mHWreg_Load_Cache_Register(JAL_COM_STAT_IRQ_REG);

    while (mHWreg_If_Cache_Bit_is_0(JAL_COM_STAT_IRQ_REG, JAL_TIM0_INTR))
    {
        delay_counter++;
        mHWreg_Load_Cache_Register(JAL_COM_STAT_IRQ_REG);
    }
    
    /* use the value of the delay to calibrate a us timer, based on fact
     * that the interval between successive TIM0's is 1250us
     *
     * perform integer divide by 125 (delay function delays in 10's of us)
     */
    hw_delay_counter = delay_counter;

    calibration_counter_10us = 1; /* compensate trunc of claibrated value */

    while (delay_counter > 125)
    {
        calibration_counter_10us++; 
        delay_counter -= 125;
    }
   
    /* use the value of the delay to calibrate a us timer, based on fact
     * that the interval between successive TIM0's is 1250us
     *
     * perform integer divide by 1250 (delay function delays in  us)
     * This doesn't give great accuracy, however is  still useful for creating 
     * small delays
     *    
     */
    delay_counter = hw_delay_counter; 

    calibration_counter_1us = 1; /* compensate trunc of claibrated value */ 

    while (delay_counter > 1250)
    {
        calibration_counter_1us++; 
        delay_counter -= 1250;
    }

    /*
     * calculate 1ms delay calibration as (1250us_delay_calibration/5)*4
     */
    delay_counter = hw_delay_counter;

    calibration_counter_1ms = (delay_counter<<2)/5;

    mHWreg_Assign_Register(JAL_COM_CTRL_IRQ_REG, 0xFFFFFFFF);

    SYSirq_Interrupts_Restore_Flags(isr_flags);

#else
    /*
     * For UNIT_TEST_BUILD, actual delays are currently not important 
     */
    calibration_counter_1us = 1;
    calibration_counter_10us = 1;
    calibration_counter_1ms = 1;
#endif
}


/************************************************************************
 * HWdelay_Wait_For_us
 *
 * Wait for units of 1us function.
 ************************************************************************/
void HWdelay_Wait_For_us(u_int32 units_of_u_secs)
{
#if (BUILD_TYPE!=UNIT_TEST_BUILD)
    u_int32 i, delay_time;    
    volatile mHWreg_Create_Cache_Register(JAL_COM_STAT_IRQ_REG);
    
    delay_time = calibration_counter_1us * units_of_u_secs;
    for (i = 0; i < delay_time; i++)
    {
        mHWreg_Load_Cache_Register(JAL_COM_STAT_IRQ_REG);
    }
#endif
}

/************************************************************************
 * HWdelay_Wait_For_10us
 *
 * Wait for units of 10us function.
 ************************************************************************/
void HWdelay_Wait_For_10us(u_int32 tens_of_u_secs)
{
#if (BUILD_TYPE!=UNIT_TEST_BUILD)
    u_int32 i, delay_time;
    volatile mHWreg_Create_Cache_Register(JAL_COM_STAT_IRQ_REG);

    delay_time = calibration_counter_10us * tens_of_u_secs;
    for (i = 0; i < delay_time; i++)
    {
        mHWreg_Load_Cache_Register(JAL_COM_STAT_IRQ_REG);
    }
#endif
}

/************************************************************************
 * HWdelay_Wait_For_Serial_Interface_Idle
 *
 * Wait for the serial interface to be ready for another transfer
 ************************************************************************/
void HWdelay_Wait_For_Serial_Interface_Idle(void)
{
#if (BUILD_TYPE!=UNIT_TEST_BUILD) && (PRH_BS_DEV_WIRED_LINK==0)
    /* 
     * Poll the Busy bit on JAL_SER_CFG to ensure fully written 
     */
    while (mHWreg_Get_Register(JAL_SER_CFG_REG) & BIT31)
    {
		if (HW_get_spi_now_conflict())
		{
			HW_set_spi_now_conflict_clr(1);
			HW_set_ser0_wr_clr(1);
		}
		//TRAhcit_Call_Poll_Rx();/* Stay here until the busy bit goes low.*/
    }
#endif
}

/************************************************************************
 * HWdelay_Wait_For_Serial_Interface_Busy_10us
 *
 * Wait with timeout for serial interface to start transfer
 ************************************************************************/
void HWdelay_Wait_For_Serial_Interface_Busy_us(u_int32 units_of_u_secs)
{
#if (BUILD_TYPE!=UNIT_TEST_BUILD) && (PRH_BS_DEV_WIRED_LINK==0)
    u_int32 i, delay_time;
    delay_time = calibration_counter_1us * units_of_u_secs;
    for (i = 0; i < delay_time; i++)
    {
        if (mHWreg_Get_Register(JAL_SER_CFG_REG) & BIT31)
            break;
    }
#endif
}

/************************************************************************
 * HWdelay_Wait_For_ms
 *
 * Wait for units of 1ms function, optionally using native clock.
 ************************************************************************/
void HWdelay_Wait_For_ms(u_int32 units_of_m_secs, boolean use_native_clk)
{
#if (BUILD_TYPE!=UNIT_TEST_BUILD)
    if(use_native_clk)
    {
        /*
         * Calculate timer to expire no sooner than units_of_m_secs time.
         */
        t_clock hw_delay_timer = BTtimer_Set_Ticks(
                     (((1000*units_of_m_secs)/BT_USEC_PER_CLK_TICK)+1));

        while(!BTtimer_Is_Expired(hw_delay_timer))
        {
        	TRAhcit_Call_Poll_Rx(); /* Stay here until clock value is reached */
        }
    }
    else
    {
        /*
         * Use calibration_counter_1ms if native clock not active/stable.
         */
        u_int32 i, delay_time;
        volatile mHWreg_Create_Cache_Register(JAL_COM_STAT_IRQ_REG);
    
        delay_time = calibration_counter_1ms * units_of_m_secs;
        for (i = 0; i < delay_time; i++)
        {
            mHWreg_Load_Cache_Register(JAL_COM_STAT_IRQ_REG);
        }
    }
#endif
}
