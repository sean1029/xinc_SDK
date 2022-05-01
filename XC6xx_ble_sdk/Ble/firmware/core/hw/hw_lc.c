/*******************************************************************************
 * MODULE NAME:    hw_lc.c
 * PROJECT CODE:   Ceva Bluetooth
 * DESCRIPTION:    Hardware Access Functions
 * MAINTAINER:     Conor Morris
 * CREATION DATE:  1 Jun 1999
 *
 * SOURCE CONTROL: $Id: hw_lc.c,v 1.110 2013/10/25 16:41:21 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2002-2007 Ceva Inc.
 *     All rights reserved.
 *
 * SOURCE:
 *    Based on C100 Jalapeno access code, redone use macros.
 *    Changed order of RF control parameters
 *
 * ISSUES:
 *    Initialisation of serial ports.
 *    Once off configuration calculation.  Store as single u_int32.
 *    Redefinition of functions as optimised macros.
 *
 * NOTES TO USERS:
 *    Where possible the natural int size (type u_int) is used.  Where a 32 bit
 *    value is required, it is explicitly given. This will have implications
 *    on the predefined constants should a 16 bit architecture be used.
 *******************************************************************************/

#include "sys_config.h"

#include "hw_register.h"
#include "hw_lc.h"
#include "hw_le_lc.h"
#include "hw_radio.h"
#include "hw_habanero.h"
#include "hw_delay.h"
#include "patch_function_id.h"
/*
 * Local helper functions
 */
static void _HWlc_Clear_Jalapeno_Registers(void);

/************************************************************************
 *
 * FUNCTION:  HWlc_Reset
 *
 ************************************************************************/
void HWlc_Reset(void)
{
	if(PATCH_FUN[HWLC_RESET_ID]){
         ((void (*)(void))PATCH_FUN[HWLC_RESET_ID])();
         return ;
    }

    /* Write sequence to reset JAL CoreLogic */
    HWjal_Set_Rst_Code(0xD2);
    HWjal_Set_Rst_Code(0x32);
    /* Write sequence to reset JAL Registers */
    /* WARNING: This actually resets the entire Tabasco chip and
                is not suitable to use on some platforms */
    HWjal_Set_Rst_Code(0xD2);
    HWjal_Set_Rst_Code(0x34);    
}

/************************************************************************
 * HWlc_Initialise
 *
 * Main hardware initialisation function
 *
 * This is responsible for setting up the Jalapeno registers
 * establishing default values for all registers.
 *
 ************************************************************************/
void HWlc_Initialise(void)
{
	if(PATCH_FUN[HWLC_INITIALISE_ID]){
         ((void (*)(void))PATCH_FUN[HWLC_INITIALISE_ID])();
         return ;
    }

    HWlc_Reset();

    /*
     * Initialisation of all Hardware Registers to default values
     */
    _HWlc_Clear_Jalapeno_Registers();

    /* 
     * Seed native clk:
     */
    HW_set_native_clk(0);

    HW_set_rx_mode(0x0);

    /* 
     * set initial whitening value 
     */
    HW_set_whiten(SYSconfig_Get_Data_Whitening_Enable());

    HW_set_intraslot_offset(0);

    HW_set_use_hab_crl1(1);

    HW_set_vci_clk_sel_map(0);

    HW_set_win_ext(SYSconfig_Get_Win_Ext());

	//HW_set_add_bt_clk_relative(1);
    if ( 1 == g_le_enable_observer_flag)
    {
		HW_set_add_bt_clk_relative(0); // 0: as master use native clk   1: as slave use bt clk	  for scan use native clk avoid clk change  TBD
	}
	else
	{
		HW_set_add_bt_clk_relative(1);
	}



    HW_set_abort_on_wrong_am_addr(1);

#if ((PRH_BS_CFG_TABASCO_VERSION >= PRH_BS_HW_TABASCO_VERSION_T1_0_0) && (BUILD_TYPE!=UNIT_TEST_BUILD))
	HW_set_freeze_bt_clk(0);
#endif

}


/************************************************************************
 * _HWlc_Clear_Jalapeno_Registers
 *
 * This function is responsible for clearing the jalapeno and
 * establishing default values for all registers.
 *
 ************************************************************************/
void _HWlc_Clear_Jalapeno_Registers(void)
{
	
	if(PATCH_FUN[_HWLC_CLEAR_JALAPENO_REGISTERS_ID]){
         ((void (*)(void))PATCH_FUN[_HWLC_CLEAR_JALAPENO_REGISTERS_ID])();
         return ;
    }

    int i;

#ifdef HW_DATA32
    /*
     * 32 bit interface supported.
     * Set bd_addr register to 0, and syncword to all 1's to prevent correlation
     */
    t_bd_addr bd_addr;
    BDADDR_Set_LAP_UAP_NAP(&bd_addr, 0x000000, 0x00, 0x0000);
    HW_set_bd_addr(&bd_addr);
    
#if 1// (LE_INCLUDED == 1)
	HWle_clear_le_mode();
#endif

    /* 
     *Syncword to all 1's, effectively prevent correlator from correlating
     */
    HW_set_sync_U32(0xFFFFFFFF, 0xFFFFFFFF);
#else
    /*
     * Byte move if supported.
     * Set bd_addr register to 0, and syncword to all 1's to prevent correlation
     */
    hw_memset8((u_int8*)JAL_BD_ADDR_ADDR, 0x00, 8);
    hw_memset8((u_int8*)JAL_SYNC_ADDR, 0xFF, 8);
#endif

    /*
     * Clear JAL_COM_CTRL_GP1_REG, JAL_COM_CTRL_GP2_REG 
     */
    mHWreg_Assign_Register(JAL_COM_CTRL_GP1_REG, 0);
    mHWreg_Assign_Register(JAL_COM_CTRL_GP2_REG, 0);

    /*
     * Clear JAL Interrupt Registers  two steps, first mask, then clear!
     * Note registers are volatile
     */
    mHWreg_Assign_Register(JAL_COM_CTRL_IRQ_REG, 0xFFFFFFFF);
    mHWreg_Assign_Register(JAL_COM_CTRL_IRQ_REG, 0xFFFFFFFF);

    /*
     * CLEAR TRANSMIT CONTROL REGISTERS
     */
    mHWreg_Assign_Register(JAL_TX_CTRL_REG, 0);

   /*
    * CLEAR RECEIVE CONTROL REGISTERS
    */
    mHWreg_Assign_Register(JAL_RX_CTRL_REG, 0);

   /*
    * CLEAR PIA CONTROL REGISTERS
    */
    mHWreg_Assign_Register(JAL_PIA_CTRL_REG, 0);
    mHWreg_Assign_Register(JAL_PIA_DATA_REG, 0);

    /*
     * AUX TIMER
     */
    HW_set_aux_timer(0);

    /*
     * RX AND TX DELAY CONTROLS
     */
    HW_set_rx_delay(0);
    HW_set_tx_delay(0);

    /*
     * ZERO ACL TX BUFFERS
     */
    for(i=0; i < JAL_ACL_BUF_LEN; i+=4)
    {
        *((unsigned long *)(JAL_BASE_ADDR + JAL_TX_ACL_BUF_OFFSET +i)) = 0;
    }

    HW_toggle_tx_buf();

    for(i=0; i < JAL_ACL_BUF_LEN; i+=4)
    {
           *((unsigned long *)(JAL_BASE_ADDR + JAL_TX_ACL_BUF_OFFSET +i)) = 0;
    }
    HW_toggle_tx_buf();
}



/* Get Bluetooth Clock */
t_clock HW_get_bt_clk(void)
{
#if (BUILD_TYPE!=UNIT_TEST_BUILD)
#if (PRH_BS_DEV_USE_CACHED_BT_CLOCKS==1)
    mHWreg_Load_Cache_Register(JAL_BT_CLK_OFFSET);
    return mHWreg_Get_Cache_Register(JAL_BT_CLK_OFFSET);
#else
    return mGetHWEntry32(JAL_BT_CLK_OFFSET);
#endif
#else
    register t_clock return_code;

    if ( HW_get_slave() )
        return_code =  mGetHWEntry32(JAL_BT_CLK_OFFSET);
    else  /* If master then native clock == bt clock */
        return_code =  HW_get_native_clk();          

    return (return_code);

#endif /*(BUILD_TYPE!=UNIT_TEST_BUILD)*/
}

t_clock HW_Get_Bt_Clk_Avoid_Race(void)
{
    t_clock piconet_clk;

    {
        piconet_clk = HW_get_bt_clk();
    
        {
            if (HW_get_bt_clk() != piconet_clk)
            {
                piconet_clk = HW_get_bt_clk(); /* re-read, to ensure value has
                                                * settled and is sane! */
            }
        }
    }
    
    return piconet_clk;
}

t_clock HW_Get_Native_Clk_Avoid_Race(void)
{
    t_clock native_clk;

#if (PRH_BS_DEV_USE_CACHED_BT_CLOCKS==1)
    /* only read from HW when cached value is invalidated as zero */
    native_clk = mHWreg_Get_Cache_Register(JAL_NATIVE_CLK);
    if(0 == native_clk)
#endif
    {
        native_clk = HW_get_native_clk();
    
        HWdelay_Wait_For_us(1);
        /* workaround 16 bit register separation in the clock */
      //  if (0xffff == (native_clk & 0xffff))
        {
            if (HW_get_native_clk() != native_clk)
            {
                native_clk = HW_get_native_clk(); /* re-read, to ensure value has
                                                   * settled and is sane! */
            }
        }
    }

    return native_clk;
}
