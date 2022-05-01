/************************************************************************
 * MODULE NAME:    bt_timer.c
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    Bluetooth System Wide Baseband Slot Based Timer
 * MAINTAINER:     John Nelson
 * DATE:           27 May 1999
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *    27 May 1999 -   jn       - Initial Version V0.9
 *    23 Jul 1999 -   jn       - New function added for Tick Timing
 *
 * SOURCE CONTROL: $Id: bt_timer.c,v 1.24 2013/11/19 02:15:54 garyf Exp $
 *
 * NOTES TO USERS:
 *    Beware that a slot is 625us!
 *    To convert to seconds, multiply by BT_CLOCK_TICKS_PER_SECOND [Avoids
 *    floating point version/calculation].
 *    This timer is currently based on the fact that there may be no guaranteed
 *    interrupt to the baseband every slot.  This requires that the end time
 *    is stored. 
 *    Assumes that t_timer is unsigned.
 *    Baseband timers now over 20.48 seconds, which means an optimised 16 bits 
 *    version will no longer be supported.
 *
 *    Timers expire at beginning of requested tick/slot.
 *    e.g.  Current native clock = 2
 *          Set Slot_Timer for 4 slots
 *          Timer will expire when native clock = 10
 ************************************************************************/

#include "sys_config.h"
#include "sys_types.h"

#include "hw_lc.h"

#include "bt_timer.h"




/*
 * May be worthwhile for code size optimisation for ARM to make const!
const t_timer _BT_CLOCK_MAX_TIMEOUT = BT_CLOCK_MAX_TIMEOUT;
 */

/************************************************************************
 * BTtimer_Get_Native_Clock
 *
 * Used to abstract higher layers requiring time from hardware access
 ************************************************************************/
t_clock BTtimer_Get_Native_Clock(void)
{
    return HW_Get_Native_Clk_Avoid_Race();
}

/************************************************************************
 * BTtimer_Set_Slots
 *
 * Set Timeout based on a Number of Slots.  Slot is 625us.
 * 2 counts per slot, always align with beginning of slot
 * Ensure that timer expires on beginning of slot rf to if Expired
 ************************************************************************/
t_timer BTtimer_Set_Slots(t_slots timeSlots)
{
    return ( (HW_Get_Native_Clk_Avoid_Race() & (BT_CLOCK_MAX_TICKS - 1))
             + ((t_timer) timeSlots << 1) - 1) &  BT_CLOCK_MAX_TICKS;
}

/************************************************************************
 * BTtimer_Set_Ticks
 *
 * Set Timeout based on Clocks Ticks.  
 * To convert to seconds multiply seconds by BT_CLOCK_TICKS_PER_SECOND.
 ************************************************************************/
t_timer BTtimer_Set_Ticks(t_clock timeTicks)
{
    return ( HW_Get_Native_Clk_Avoid_Race() + timeTicks ) & BT_CLOCK_MAX_TICKS;
}
#if 0
/************************************************************************
 * BTtimer_Reset_Slots
 *
 * Set Timeout based on a Number of Slots relative to existing
 * timer value.
 ************************************************************************/
t_timer BTtimer_Reset_Slots(t_timer timer, t_slots timeSlots)
{
    return (timer + ( (t_timer) timeSlots << 1) ) & BT_CLOCK_MAX_TICKS;
}

/************************************************************************
 * BTtimer_Reset_Ticks
 *
 * Set Timeout based on a Number of Slots relative to existing
 * timer value.
 ***********************************************************************/
t_timer BTtimer_Reset_Ticks(t_timer timer, t_slots timeTicks)
{
    return (timer + timeTicks) & BT_CLOCK_MAX_TICKS;
}
#endif
/************************************************************************
 *  BTtimer_Is_Expired
 *
 *  The conditional expression will evaluate to false until the
 *  native clock becomes greater than the set time out value.
 *  Since the check is for greater than, the setting of the timer must
 *  ensure that the timer is actually set to the preceding tick
 *  after which the timeout occurs.
 ************************************************************************/
boolean BTtimer_Is_Expired(t_timer ptimer)
{
    boolean test;

    if (((ptimer - HW_Get_Native_Clk_Avoid_Race()) & BT_CLOCK_MAX_TICKS)
        >= BT_CLOCK_MAX_TIMEOUT)
    {
        test = TRUE;
    }
    else
    {
        test = FALSE;
    }

    return test;
}

#if 0

/************************************************************************
 * BTtimer_Piconet_Set_Slots
 *
 * Set Timeout based on a Number of Slots.  Slot is 625us.
 * Slot timing alligned to BT clock
 * 2 counts per slot, always align with beginning of slot
 * Ensure that timer expires on beginning of slot rf to if Expired
 ************************************************************************/
t_timer BTtimer_Piconet_Set_Slots(t_slots timeSlots)
{
    return ( (HW_get_bt_clk() & (BT_CLOCK_MAX_TICKS - 1))
             + ((t_timer) timeSlots << 1) - 1) &  BT_CLOCK_MAX_TICKS;
}


/************************************************************************
 * BTtimer_Piconet_Reset_Slots
 *
 * Set Timeout based on a Number of Slots relative to existing
 * timer value.
 * Slot timing alligned to BT clock
 *
 ************************************************************************/
t_timer BTtimer_Piconet_Reset_Slots(t_timer timer, t_slots timeSlots)
{
    return (timer + ( (t_timer) timeSlots << 1) ) & BT_CLOCK_MAX_TICKS;
}


/************************************************************************
 *  BTtimer_Is_Expired
 *
 *  The conditional expression will evaluate to false until the
 *  native clock becomes greater than the set time out value.
 *  Since the check is for greater than, the setting of the timer must
 *  ensure that the timer is actually set to the preceding tick
 *  after which the timeout occurs.
 *
 * Slot timing alligned to BT clock
 *
 ************************************************************************/
boolean BTtimer_Piconet_if_Expired(t_timer ptimer)
{
    boolean test;

    if (((ptimer - HW_get_bt_clk()) & BT_CLOCK_MAX_TICKS)
        >= BT_CLOCK_MAX_TIMEOUT)
    {
        test = TRUE;
    }
    else
    {
        test = FALSE;
    }

    return test;
}


/************************************************************************
 * BTtimer_Piconet_Reset_Delta_Slots
 *
 * Set Timeout based on the Delta Slots relative to existing
 * timer value ensuring that the timer value is greater that
 * the reference target time.
 *
 * new_timer = timer + N * T_proc_slots st, new_timer > ref_time_ticks
 *
 * Since T_proc_slots can be 0, must protect against it.
 ************************************************************************/
t_timer BTtimer_Piconet_Reset_Delta_Slots(
    t_timer timer,                      /* -i   Timer to Adjust             */
    t_clock ref_time_ticks,             /* -i   Time to check equations at  */
    t_slots T_proc_slots                /* -i   Procedure Interval in Slots */
    )
{
    /*
     * First just offset number of slots in ticks from existing timer
     */
    t_timer T_proc_ticks = (t_timer) T_proc_slots << 1;
    t_timer new_timer = (timer + T_proc_ticks ) & BT_CLOCK_MAX_TICKS;


    /*
     * adjust if the abnormal new_timer is less than target time then
     *
     * (10 - 12)    (0x0ffffffc - 12)   (14 - 12)  (12 - 0x0ffffffc)
     */
    if ( T_proc_ticks > 0 && ((new_timer - ref_time_ticks) & BT_CLOCK_MAX_TICKS)
           >= BT_CLOCK_MAX_TIMEOUT)
    {
        /*
         * Timeout has passed adjust to future piconet
         * new_timer += (ref_target_time - new_timer + delta_slots)/delta_slots*delta_slots
         * Subtract one from ref_time_ticks to reflect Is_Expired implementation 
         */
        new_timer += ((ref_time_ticks - 1 - new_timer + T_proc_ticks)/
            T_proc_ticks) * T_proc_ticks;
        new_timer &= BT_CLOCK_MAX_TICKS;
    }
    return new_timer;
}

/************************************************************************
 * BTtimer_Set_Timer_On_Timing_Equation
 *
 * Set Timeout based on the timing equation to next time for which 
 * equation will be satisfied.
 *
 *  CLK 27-1 mod T_proc = D_proc           for initialization 1
 *  ______
 * (CLK 27 ,CLK 26-1 ) mod T_proc = D_proc for initialization 2
 *
 * The idea of initialisation is to ensure the clock is not going
 * to wrap in the vicinity of initialisation.
 ************************************************************************/
t_timer BTtimer_Set_Timer_On_Timing_Equation( 
    t_clock ref_time_ticks,             /* -i   Time to check equations at  */
    boolean init_proc_2,                /* -i   Initialisation 1/2 if 0/1   */
    t_slots T_proc_slots,               /* -i   Procedure Interval in Slots */
    t_slots D_proc_slots                /* -i   Procedure Offset in Slots   */
    )
{
    t_timer new_timer;
    t_clock T_proc_ticks = T_proc_slots << 1; 
    t_clock D_proc_ticks = D_proc_slots << 1; 

    /*
     * Convert T_proc/D_proc from slots to ticks (requires 32 bit accuracy)
     */
    ref_time_ticks ^= (u_int32)init_proc_2<<27;
    
    /*
     * Find the nearest time match to ref_time_ticks
     * Then must ensure that it is greater than current time
     */
    new_timer = ref_time_ticks/T_proc_ticks * T_proc_ticks + D_proc_ticks;
    if (new_timer <= ref_time_ticks)
    {
        new_timer += T_proc_ticks;
    }

    new_timer ^= (u_int32)init_proc_2<<27;

    /*
     * Subtract one to reflect implementation of timer
     */

    new_timer = (new_timer-1) & BT_CLOCK_MAX_TICKS;
    return new_timer;
}

/************************************************************************
 * BTtimer_Is_Timing_Equation_Satisfied
 *
 * Is the timing equation for this procedure satisfied:
 *  CLK 27-1 mod T_proc = D_proc           for initialization 1
 *  ______
 * (CLK 27 ,CLK 26-1 ) mod T_proc = D_proc for initialization 2
 *
 * ref_time_slots = (ref_time_ticks >> 1) ^ ((u_int32)init_proc_2<<26);
 * equation satisfied if ((re_time_slots % T_proc) == D_proc);
 *
 * Given the modulus operation this procedure is relatively slow
 ************************************************************************/
boolean BTtimer_Is_Timing_Equation_Satisfied(                        
    t_clock ref_time_ticks,             /* -i   Time to check equations at  */
    boolean init_proc,                  /* -i   Initialisation 1/2 if 0/1   */
    t_slots T_proc_slots,               /* -i   Procedure Interval in Slots */
    t_slots D_proc_slots                /* -i   Procedure Offset in Slots   */
    ) 
{
    return ( ((ref_time_ticks >> 1) ^ ((u_int32)init_proc<<26)) 
             % T_proc_slots) == D_proc_slots;
}
#endif

/************************************************************************
 *  BTtimer_Is_Expired_For_Time
 *
 *  The conditional expression will evaluate to false until the
 *  reference time becomes greater than the set time out value.
 *  Since the check is for greater than, the setting of the timer must
 *  ensure that the timer is actually set to the preceding tick
 *  after which the timeout occurs.
 ************************************************************************/
boolean BTtimer_Is_Expired_For_Time(
    t_timer timer,                      /* -i   Timer to Adjust             */
    t_clock ref_time_ticks              /* -i   Time to check equations at  */
    )
{
    boolean test;
	
	if (BT_TIMER_OFF == timer )
	{
		return FALSE;
	}
	
    if (((timer - (ref_time_ticks& BT_CLOCK_MAX_TICKS)) & BT_CLOCK_MAX_TICKS)
        >= BT_CLOCK_MAX_TIMEOUT)
    {
        test = TRUE;
    }
    else
    {
        test = FALSE;
    }

    return test;
}
#if 0
/************************************************************************
 *  BTtimer_Is_Expired_For_Time_Instant
 *
 *  The conditional expression will evaluate to false unless the
 *  reference time is one greater than the set time out value.
 *  Hence evaluates to true only on first instant timer has expired.
 *  Useful alternative where we want to avoid events occuring late.
 *  
 ************************************************************************/
boolean BTtimer_Is_Expired_For_Time_Instant(
    t_timer timer,                      /* -i   Timer to Adjust             */
    t_clock ref_time_ticks              /* -i   Time to check equations at  */
    )
{
    return (((timer+1)&BT_CLOCK_MAX_TICKS)==(ref_time_ticks&BT_CLOCK_MAX_TICKS));
}

/************************************************************************
 * BTtimer_Extract_Timer_D_proc_slots
 *
 * Extract the current D procedure in slots for the presented timer.  
 *
 * D_proc continuously changes as clock wraps through each bit27 
 * transition i.e. every 11h 40m.  Therefore before setting up  
 * associated devices the current D proc must be determined.  This
 * is also a function of the timing control mechanism selected.
 *
 *  CLK 27-1 mod T_proc = D_proc           for initialization 1
 *  ______
 * (CLK 27 ,CLK 26-1 ) mod T_proc = D_proc for initialization 2
 *
 *
 *  If Initialisation 1 Then
 *     D_proc = NextActivityTime_27_1 % T_proc
 *  Elseif Initialisation 2 Then
 *     D_proc = (!NextActivityTime27,NextActivityTime26_1) % T_proc
 *  Endif
 *
 *  The timer parameter can be actual frame start time or a timer for
 *  the next active time for the procedure.
 ************************************************************************/
t_clock BTtimer_Extract_Timer_D_proc_slots( 
    t_timer timer,                      /* -i   Timer to extract D_proc     */
    boolean init_proc_2,                /* -i   Initialisation 1/2 if 0/1   */
    t_slots T_proc_slots                /* -i   Procedure Interval in Slots */
    )
{
    /*
     * Timers are defined as - 1 from actual time
     */
    t_timer ref_time_ticks = timer + 1;
    t_clock D_proc_slots;

    /*
     * Convert T_proc/D_proc from slots to ticks (requires 32 bit accuracy)
     */

    if (init_proc_2)
    {
        D_proc_slots = ((ref_time_ticks^(1<<27))>>1) % T_proc_slots;        
    }
    else
    {
        D_proc_slots = (ref_time_ticks>>1) % T_proc_slots;
    }

    return D_proc_slots;
}
#endif

