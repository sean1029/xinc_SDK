/*************************************************************************
 *
 * MODULE NAME:    lslc_clk.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    Bluetooth Slave Clock Management in Software
 *
 * MAINTAINER:     Ivan Griffin
 * CREATION DATE:  23 September 2001
 *
 * SOURCE CONTROL: $Id: lslc_clk.c,v 1.19 2005/01/21 16:43:11 namarad Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2001-2004 Ceva Inc.
 *     All rights reserved.
 *     
 * NOTES:
 *     Currently this is inherently a single piconet solution.  The strategy
 *     needs investigation for the scatternet case.
 *
 ***************************************************************************/

#include "sys_config.h"
#include "sys_types.h"

#include "lc_types.h"
#include "lslc_clk.h"

#include "hw_lc.h"
#include "tra_hcit.h"

void LSLCclk_Initialise(void)
{
}

boolean LSLCclk_Is_Safe_To_Read_Native_Clk(t_frame_pos frame_pos)
{
    u_int32 intraslot_offset = HW_get_intraslot_offset();

#if 1
    if ((TX_START == frame_pos) && (intraslot_offset >= 625))
    {
        return 1;
    }
    if ((RX_START == frame_pos) && (intraslot_offset < 625))
    {
        return 1;
    }

    return 0;
#else
    return ((TX_START == frame_pos) ^ (intraslot_offset < 625));
#endif
}

void LSLCclk_Wait_For(t_frame_pos frame_posn)
{
#if (BUILD_TYPE != UNIT_TEST_BUILD)
    u_int8 tx_start_clk_value, rx_start_clk_value;

    if (HW_get_slave()) 
    {
        tx_start_clk_value = 2;
        rx_start_clk_value = 0;
    }
    else /* i am a master */
    {
        tx_start_clk_value = 0;
        rx_start_clk_value = 2;
    }

    if (frame_posn == TX_START)
    {
        while ((HW_get_bt_clk() & 0x3) != tx_start_clk_value)
        {
            /* SPIN for TIM0, clear any TRAhcit FIFOs */
            TRAhcit_Call_Poll_Rx();
        }
    }
    else
    {
        while ((HW_get_bt_clk() & 0x3) != rx_start_clk_value)
        {
            /* SPIN for TIM2, clear any TRAhcit FIFOs */
            TRAhcit_Call_Poll_Rx();
        }
    }
#endif
}
