/**************************************************************************
 * MODULE NAME:    lmp_timer.c
 * PROJECT CODE:    BlueStream
 * DESCRIPTION:    Host Controller Timers
 * MAINTAINER:     Tom Kerwick
 * DATE:           02-08-1999
 *
 * SOURCE CONTROL: $Id: lmp_timer.c,v 1.27 2005/05/13 09:08:52 namarad Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2004 Ceva Inc.
 *     All rights reserved.
 *
 * REVISION HISTORY:
 *    02-08-1999 -   GF          Initial version.
 *    16-07-2003 -   TK          Re-implemented timers as sorted list.
 *    10-06-2004 -   TK          Added interrupt locking.
 *
 * DESCRIPTION
 *
 *  This module provides event driven timers for usage by the higher layers.
 *  The basic premise it to allow a series of timers to be used which have 
 *  functions calls associated with them. When a timer expires the respective
 *  function is called and the timer set to UNUSED.
 *
 **************************************************************************/


#include "sys_types.h"
#include "sys_config.h"
#include "sys_irq.h"
#include "lc_interface.h"
#include "lmp_const.h"
#include "lmp_config.h"
#include "lmp_acl_connection.h"
#include "bt_timer.h"
#include "lmp_timer.h"

#define LMTMR_RESET_BIT         0x08
#define LMTMR_LOCK_BIT          0x04
#define LMTMR_ACTIVE_BIT        0x02
#define LMTMR_USED_BIT          0x01

#define UNUSED_BUT_ACTIVE       (LMTMR_ACTIVE_BIT)
#define USED_BUT_INACTIVE       (LMTMR_USED_BIT)
#define USED                    (LMTMR_USED_BIT|LMTMR_ACTIVE_BIT)
#define UNUSED                  (0)

STATIC t_lm_timer lm_timers[PRH_MAX_LM_TIMERS];
STATIC u_int8 lm_timer_head = 0;

void LMtmr_Delist_Timer(u_int8 timer_index);
void LMtmr_Enlist_Timer(u_int8 timer_index);

/**************************************************************
 *
 * Function : LMtmr_Set_Timer
 *
 * Input Params :  
 *  timer_value - The value of the timer in slots
 *  *p_funct  - A pointer to the function to be called when the timer expires
 *  p_link  - A pointer to the lmp link which should be used as a parameter for *p_funct
 *  dynamic - Indicates if the timer is to be freed after expiring.
 *
 * Description :
 * This functions sets a LM timer and stores a pointer to the function which should be
 * called when the timer expires. 
 *
 * Context: Both Task & ISR.
 *
 ***************************************************************/
u_int8 LMtmr_Set_Timer(t_slots timer_value, 
                       void ((*p_funct)(t_lmp_link*)), t_lmp_link* p_link, u_int8 dynamic)
{
    u_int8 timer_index = 1;
    t_lm_timer* p_timer;

    boolean timer_found = 0;

    while ((timer_index < PRH_MAX_LM_TIMERS) && (timer_found==0))
    {
        if (UNUSED == lm_timers[timer_index].used)
        {
            u_int32 cpu_flags;
            SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);
            if (UNUSED == lm_timers[timer_index].used)
            {
                lm_timers[timer_index].used = LMTMR_USED_BIT|LMTMR_LOCK_BIT;
                timer_found = 1;
            }
            SYSirq_Interrupts_Restore_Flags(cpu_flags);
        }

        if(timer_found==0)
            timer_index++;
    }

    if (timer_found) 
    {
        p_timer = &lm_timers[timer_index];
        p_timer->function = p_funct;
        p_timer->p_link = p_link;
        p_timer->dynamic = dynamic;
        p_timer->timer = BTtimer_Set_Slots(timer_value);
        if (timer_value)
        {
            p_timer->used |= LMTMR_ACTIVE_BIT;
            LMtmr_Enlist_Timer(timer_index);
        }

        p_timer->used &= ~LMTMR_LOCK_BIT;
    }
    else
        HCeg_Hardware_Error_Event(PRH_BS_HW_ERROR_OUT_OF_LMP_TIMERS);

    return timer_index;
}

/**************************************************************
 *
 * Function : LMtmr_Timer_Expired
 *
 * Input Params :  
 *                 
 * Description :
 * Indicates if any timers have expired. Should be called from
 * end of TIM0 (and end of TIM2 for slot accuracy). This should
 * set an event flag for mini-sched.
 *
 * Context: ISR Only.
 *
 ***************************************************************/
boolean LMtmr_Timer_Expired()
{
    return (lm_timer_head && BTtimer_Is_Expired(lm_timers[lm_timer_head].timer));
}

/**************************************************************
 *
 * Function : LMtmr_Handle_Expired_Timers
 *
 * Input Params :  
 *                 
 * Description :
 * Processes all expired timers. Should be called subsequent to
 * event flag set from interrupt space via LMtmp_Timer_Expired.
 *
 * Context: Task Only.
 *
 ***************************************************************/
void LMtmr_Handle_Expired_Timers()
{    
    t_lm_timer* p_timer;

    void  (*lmtmr_function)(t_lmp_link*);
    t_lmp_link* lmtmr_p_link;

    u_int32 cpu_flags;

    SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);

    while(lm_timer_head && BTtimer_Is_Expired(lm_timers[lm_timer_head].timer))
    {
        p_timer = &lm_timers[lm_timer_head];
        lm_timer_head = p_timer->next;

        lmtmr_function = p_timer->function;
        lmtmr_p_link = p_timer->p_link;

        SYSirq_Interrupts_Restore_Flags(cpu_flags);

        p_timer->used = (p_timer->dynamic)?UNUSED:USED_BUT_INACTIVE;

        if(lmtmr_function)
            lmtmr_function(lmtmr_p_link);
        else
            HCeg_Hardware_Error_Event(PRH_BS_HW_ERROR_CORRUPTION_OF_LMP_TIMERS);

        SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);
    }

    SYSirq_Interrupts_Restore_Flags(cpu_flags);
}

/**************************************************************
 *
 * Function : LMtmr_Delist_Timer
 *
 * Input Params : timer_index - Identifies timer to delist
 *                 
 * Description :
 * Local proc to remove timer from ordered list of used timers.
 *
 * Context: Both Task & ISR.
 *
 **************************************************************/
void LMtmr_Delist_Timer(u_int8 timer_index)
{
    u_int8 iter;

    u_int32 cpu_flags;

    SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);

    if(timer_index == lm_timer_head)
    {
        lm_timer_head = lm_timers[timer_index].next;
    }
    else if(timer_index)
    {
        for(iter=lm_timer_head; iter; iter=lm_timers[iter].next)
        {
            if(lm_timers[iter].next == timer_index)
            {
                lm_timers[iter].next = lm_timers[timer_index].next;
                break;
            }
        }
    }

    SYSirq_Interrupts_Restore_Flags(cpu_flags);
}

/**************************************************************
 *
 * Function : LMtmr_Enlist_Timer
 *
 * Input Params : timer_index - Identifies timer to enlist
 *                 
 * Description :
 * Local proc to insert timer into ordered list of used timers.
 *
 * Context: Both Task & ISR.
 *
 **************************************************************/
void LMtmr_Enlist_Timer(u_int8 timer_index)
{    
    u_int8 iter;
    u_int8 b4timer = 0;

    u_int32 cpu_flags;

    SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);
    
    iter = lm_timer_head;
    while(iter && BTtimer_Is_Expired_For_Time(lm_timers[iter].timer,lm_timers[timer_index].timer))
    {
        b4timer = iter;
        iter=lm_timers[iter].next;
    }

    if(iter == lm_timer_head)
    {
        lm_timers[timer_index].next = lm_timer_head;
        lm_timer_head = timer_index;
    }
    else
    {
        lm_timers[timer_index].next = lm_timers[b4timer].next;
        lm_timers[b4timer].next = timer_index;
    }

    SYSirq_Interrupts_Restore_Flags(cpu_flags);
}

/**************************************************************
 *
 * Function : LMtmr_Clear_Timer
 *
 * Input Params : timer_index - Identifies the timer to clear
 *                 
 * Description :
 * Clears a specific timer.
 *
 * Context: Both Task & ISR.
 *
 **************************************************************/
void LMtmr_Clear_Timer(u_int8 timer_index)
{
    if(timer_index)
    {
        t_lm_timer *p_timer = &lm_timers[timer_index];

        /*
         * Prevented by LOCK if within set/clear same timer
         */
        if (!(p_timer->used & LMTMR_LOCK_BIT))
        {
            /*
             * Prevent by LOCK from interrupt to reset same timer:
             */
            p_timer->used |= LMTMR_LOCK_BIT;

            LMtmr_Delist_Timer(timer_index);
            p_timer->function = NULL;
            p_timer->dynamic = 1;
            p_timer->p_link = 0;
            p_timer->timer = 0;
            p_timer->next = 0;
            p_timer->used = UNUSED;
        }
    }
}

/**************************************************************
 *
 * Function : LMtmr_Reset_Timer()
 *
 * Input Params : timer_index - Identifies the timer to reset
 *                timer_value - New value of the timer in slots
 *                 
 * Description :
 * Resets a specific timer.
 *
 * Context: Both Task & ISR.
 *
 **************************************************************/
void LMtmr_Reset_Timer(u_int8 timer_index,t_slots timer_value)
{
    if(timer_index)
    {
        t_lm_timer *p_timer = &lm_timers[timer_index];

        /*
         * Prevented by LOCK if within set/clear/reset same timer.
         */
        if (!(p_timer->used & (LMTMR_LOCK_BIT|LMTMR_RESET_BIT)) && (p_timer->used & LMTMR_USED_BIT))
        {
            p_timer->used |= LMTMR_RESET_BIT;

            LMtmr_Delist_Timer(timer_index);

            if (timer_value==0x0000)
            {
                p_timer->used &= ~LMTMR_ACTIVE_BIT;
            }
            else
            {
                p_timer->used |= LMTMR_ACTIVE_BIT;
                p_timer->timer = BTtimer_Set_Slots(timer_value);
                LMtmr_Enlist_Timer(timer_index);
            } 
            
            p_timer->used &= ~LMTMR_RESET_BIT;

            /*
             * If interrupted to clear same timer, re-execute clear:
             */
            if (NULL == (void*)(p_timer->function))
                LMtmr_Clear_Timer(timer_index);
        }
    }
}

/**************************************************************
 *
 * Function : LMtmr_Modify_Timer()
 *
 * Input Params : timer_index - Identifies the timer to reset
 *                timer_value - New value of the timer in slots
 *                timer_extend - Extend or shorten the timeout.
 *                 
 * Description :
 * Modifies a specific timer.
 *
 * Context: Task Only.
 *
 **************************************************************/
void LMtmr_Modify_Timer(u_int8 timer_index, t_slots timer_value, boolean timer_extend)
{
    if(timer_index)
    {
        /*
         * If the existing timeout expires before the new timeout
         * then reset the timer to extend the timeout, or apply the
         * inverse logic to shorten the timeout.
         */
        if (BTtimer_Is_Expired_For_Time(lm_timers[timer_index].timer,
              BTtimer_Set_Slots(timer_value)) == timer_extend)
        {
            LMtmr_Reset_Timer(timer_index, timer_value);
        }
    }
}

/**************************************************************
 *
 * Function: LMtmr_Get_Residual_Slots
 *
 * Input Params : timer_index - Identifies the timer to check
 *
 * Description:
 * Returns slots remaining until timeout.
 *
 * Context: Both Task & ISR
 *
 **************************************************************/
t_slots LMtmr_Get_Residual_Slots(u_int8 timer_index)
{
    return (BTtimer_Clock_Difference(LC_Get_Native_Clock(),
        lm_timers[timer_index].timer)>>1);
}


/**************************************************************
 *
 * Function : LMtmr_Initialise
 *             
 * Description :
 * Clears all the timers.
 *
 * Context: Task Only.
 *
 **************************************************************/
void LMtmr_Initialise()
{
    int i;

    u_int32 cpu_flags;

    SYSirq_Disable_Interrupts_Save_Flags(&cpu_flags);

    for(i=0; i<PRH_MAX_LM_TIMERS; i++)
    {
        lm_timers[i].used = UNUSED;
        lm_timers[i].function = NULL;
        lm_timers[i].dynamic = 1;
        lm_timers[i].p_link = 0;
        lm_timers[i].timer = 0;
        lm_timers[i].next = 0;
    }
    lm_timer_head = 0;

    SYSirq_Interrupts_Restore_Flags(cpu_flags);
}
