/******************************************************************************
 *        
 * MODULE NAME:    lslc_irq.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    Hardware Interrupt Interface
 * MAINTAINER:     Ivan Griffin
 * DATE:           09 August 1999
 *
 * SOURCE CONTROL: $Id: lslc_irq.c,v 1.46 2013/11/27 15:16:55 vladimirf Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 1999-2004 Ceva Inc.
 *     All rights reserved.
 *
 ******************************************************************************/

#include "sys_config.h"
#include <assert.h>                     /* Must be after sys_config.h           */
    
#include "lslc_irq.h"

#include "lslc_slot.h"
#include "lslc_access.h"

#include "sys_mmi.h"

#include "hw_lc.h"
#include "hw_register.h"
#include "hw_macro_defs.h"
#include "hw_radio.h"

#if 1 //(LE_INCLUDED == 1)
#include "le_config.h"
#include "le_link_layer.h"
#include "le_const.h"
#endif
#if (PRH_BS_CFG_SYS_LOW_POWER_MODE_SUPPORTED==1)
#include "sys_power.h"
#endif

#include    "sys_debug_config.h"
#include    "hw_radio_defs.h"
#include    "le_power.h"
#include    "global_val.h"
#include "patch_function_id.h"

#define MAX_TIM0_TIME        0
#define MAX_TIM1_TIME        1
#define MAX_TIM2_TIME        2
#define MAX_TIM3_TIME        3
#define MAX_SYNC_DETECT_TIME 4
#define MAX_PKD_RX_HDR_TIME  5
#define MAX_PKD_TIME         6
#define MAX_NO_PKT_RCVD_TIME 7
#define MAX_PKA_TIME         8
#define MAX_AUX_TIM_TIME     9

#if (BUILD_TYPE == UNIT_TEST_BUILD)
void SYSirq_IRQ_Tabasco_ISR(void);
#endif
#if 1 //(LE_INCLUDED == 1)
extern t_LE_Config LE_config;
#endif
/*
 * If Debug LSLC irq enabled then count the interrupts else do nothing
 */
#if (PRH_BS_DBG_LC_LSLC_IRQ==1)
/*static */ t_LSLCirq_Counters _LSLCirq_Counters;
#define LSLCirq_Inc_Cntr(cntr) (++_LSLCirq_Counters.##cntr##_count) 
#else
#define LSLCirq_Inc_Cntr(cntr) 
#endif

#if (PRH_BS_DEV_USE_CACHED_BT_CLOCKS==1)
#include "hw_register.h"
#include "hw_jal_defs.h"
mHWreg_Create_Cache_Register(JAL_NATIVE_CLK) = 0;
mHWreg_Create_Cache_Register(JAL_BT_CLK_OFFSET) = 0;
#endif

#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 1)
static _LSLCirq_ignore_next_pka = FALSE;
#endif

//u_int8 le_mode = 0; //Classic BT
//mHWreg_Create_Cache_Register(JAL_COM_CTRL_IRQ_REG);
//pka_t   pka_op = (pka_t)0;
//pkd_t   pkd_op = (pkd_t)0;
#ifndef REDUCE_ROM
/*register pka callback.*/
void    register_pka_callback(pka_t op)
{
    pka_op = op;
}
/*register pkd callback.*/
void    register_pkd_callback(pkd_t op)
{
    pkd_op = op;
}
#endif
/******************************************************************************
 *
 * FUNCTION:  LSLCirq_Initialise
 * PURPOSE:   Initialise Tabasco Interrupts
 *
 ******************************************************************************/
void LSLCirq_Initialise(void)
{
	if(PATCH_FUN[LSLCIRQ_INITIALISE_ID]){
         ((void (*)(void))PATCH_FUN[LSLCIRQ_INITIALISE_ID])();
         return ;
    }

    mHWreg_Create_Cache_Register(JAL_COM_STAT_IRQ_REG);

    mHWreg_Load_Cache_Register(JAL_COM_STAT_IRQ_REG);
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_IRQ_REG);

    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM1_INTR_CLR);

#if (TABASCO_OLD_SER_BLK_SUPPORTED==1)
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM1_INTR_MSK);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM3_INTR_MSK);
#else
    /* TIM1 used for synchronisation in LSLCslot during startup */
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM1_INTR_MSK);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM3_INTR_MSK);
#endif

    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM0_INTR_MSK);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM2_INTR_MSK);

    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_SYNC_DET_INTR_MSK);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_NO_PKT_RCVD_INTR_MSK);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKD_RX_HDR_INTR_MSK);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKD_INTR_MSK);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKA_INTR_MSK);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_AES_INTR_MSK);

    mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    
    le_mode = 1; 

#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 1)
    _LSLCirq_ignore_next_pka = FALSE;
#endif
}

#if (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 1)

/******************************************************************************
 *
 * FUNCTION:    LSLCirq_Tabasco_ISR
 * PURPOSE:     Process the Tabasco interrupt and returns a pointer to the delayed service
 *                      routine associated with the processed interrupt (if any).
 *
 * NOTE: 
 *  The interrupt source is masked off in this function if there is a delayed service routine
 *  associated with that interrupt. The interrupt dource should only be re-enabled once the
 *  dsr has completed.
 ******************************************************************************/
void (*LSLCirq_Tabasco_ISR(void))(void)
{
    mHWreg_Create_Cache_Register(JAL_COM_STAT_IRQ_REG);
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_IRQ_REG);

    t_dsr_pointer delayed_service_routine = 0;


#if (PRH_BS_DEV_USE_CACHED_BT_CLOCKS==1)
    mHWreg_Clear_Cache_Register(JAL_NATIVE_CLK);
    mHWreg_Clear_Cache_Register(JAL_BT_CLK_OFFSET);
#endif

    mHWreg_Load_Cache_Register(JAL_COM_STAT_IRQ_REG);
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_IRQ_REG);
        
    /*
     * TIM 2 
     */
    if (mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_TIM2_INTR))
    {
        delayed_service_routine = &LSLCslot_Handle_TIM2;
        
        HWradio_Setup_Radio_For_Next_Half_Slot(RX_MID);
        
        /* mask Tim2, need to be unmasked in LSLCslot_Handle_TIM2() */
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM2_INTR_MSK);

        mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_NO_PKT_RCVD_INTR_MSK);
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM2_INTR_CLR);
        mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    } else
    
    /*
     * SYNC_DET
     */
    if (mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_SYNC_DET_INTR))
    {
        HWradio_Rx_PacketExtend();
        LSLCacc_Attempt_RSSI_Read(FALSE);

        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_NO_PKT_RCVD_INTR_MSK);
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_NO_PKT_RCVD_INTR_CLR);
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_SYNC_DET_INTR_CLR);
        mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    }

    /*
     * PKD_RX_HDR
     */
    else if (mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_PKD_RX_HDR_INTR))
    {
        delayed_service_routine = &LSLCslot_Handle_PKD_RX_HDR;

        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKD_RX_HDR_INTR_MSK);

        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_NO_PKT_RCVD_INTR_CLR);
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_NO_PKT_RCVD_INTR_MSK);
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKD_RX_HDR_INTR_CLR);
        mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);        
    }
    
    /*
     * PKD
     */
    else if (mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_PKD_INTR))
    {
        delayed_service_routine = &LSLCslot_Handle_PKD;

        HWradio_RxComplete();
#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
        HWpta_Rx_Access_Complete();     //- LC note: This function don't use.
#endif
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKD_INTR_MSK);

        mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_NO_PKT_RCVD_INTR_MSK);
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_NO_PKT_RCVD_INTR_CLR);

        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKD_INTR_CLR);
        mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);
//         SYSmmi_Display_Event(eSYSmmi_PKD_Event);
    }
    
    /*
     * NO_PKT_RCVD
     */
    else if (mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_NO_PKT_RCVD_INTR))
    {
    	delayed_service_routine = &LSLCslot_Handle_NO_PKT_RCVD;

    	HWradio_RxComplete();
#if (PRH_BS_CFG_SYS_PTA_DRIVER_SUPPORTED==1)
        HWpta_Rx_Access_Complete();     //- LC note: This function don't use.
#endif

        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_NO_PKT_RCVD_INTR_MSK);

        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_NO_PKT_RCVD_INTR_CLR);
        mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    }
    
    /*
     * TIM3
     */
    else if (mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_TIM3_INTR))
    {
        HWradio_Setup_Radio_For_Next_Half_Slot(TX_START);
        delayed_service_routine = &LSLCslot_Handle_TIM3;
    	
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM3_INTR_MSK);
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM3_INTR_CLR);
        mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    }

    /*
     * TIM0
     */
    else if (mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_TIM0_INTR))
    {
        t_Transmitter_Mode  tx_mode;
        t_packet tx_packet_type;
        
        tx_mode = (t_Transmitter_Mode) HW_get_tx_mode();

        if (tx_mode != TXm_TRANSMITTER_DISABLED)
        {
            if (tx_mode == TXm_SLAVE_1ST_HALF__MASTER_BOTH)
            {   /* Avoid having to process two PKA interrupts during page/inquiry. */
                _LSLCirq_ignore_next_pka = TRUE;
            }
            else
            {
                _LSLCirq_ignore_next_pka = FALSE;
            }
            
            tx_packet_type = (t_packet) HW_get_tx_type();

            if (is_Packet_Multi_Slot(tx_packet_type)|| 
					is_eSCO_Packet_3_Slot(tx_packet_type))
            {
                HWradio_Tx_PacketExtend();
            }

        }

    	delayed_service_routine = &LSLCslot_Handle_TIM0;
    	
    	HWradio_Setup_Radio_For_Next_Half_Slot(TX_MID);

        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM0_INTR_MSK);
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM0_INTR_CLR);
        mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);
//         SYSmmi_Display_Event(eSYSmmi_TX_START_Event);
    }

    /*
     * PKA
     */
    else if (mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_PKA_INTR))
    {
        if (_LSLCirq_ignore_next_pka)
        {
            _LSLCirq_ignore_next_pka = FALSE;
            mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKA_INTR_CLR);
            mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);
        }
        else
        {
            delayed_service_routine = &LSLCslot_Handle_PKA;

            ((void (*)(void))PATCH_FUN[HWRADIO_TXCOMPLETE])();//HWradio_TxComplete();

            mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKA_INTR_MSK);
            mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKA_INTR_CLR);
            mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);
        }
    }
            
    /*
     * TIM1
     */
    else if (mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_TIM1_INTR))
    {
        HWradio_Setup_Radio_For_Next_Half_Slot(RX_START);
        
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM1_INTR_CLR);
        mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    }

    /*
     * AUX_TIM
     */
    else if (mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_AUX_TIM_INTR))
    {
        /* important that the AUX_TIM is CLEARED before it is reset... */
        HW_set_aux_tim_intr_clr(1); /* force the clear immediately */
    	delayed_service_routine = &LSLCslot_Handle_AUX_TIM;
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_AUX_TIM_INTR_MSK);
    }

//    SYSmmi_Display_Event(eSYSmmi_BTCLK_Tick_Event);
    return(delayed_service_routine);
}

/******************************************************************************
 *
 *
 ******************************************************************************/
void LSLCirq_Unmask_Tim0(void)
{
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM0_INTR_MSK);
    mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);
}

/******************************************************************************
 *
 *
 ******************************************************************************/
void LSLCirq_Unmask_Tim1(void)
{
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM1_INTR_MSK);
    mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);
}

/******************************************************************************
 *
 *
 ******************************************************************************/
void LSLCirq_Unmask_Tim2(void)
{
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM2_INTR_MSK);
    mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);
}

/******************************************************************************
 *
 *
 ******************************************************************************/
void LSLCirq_Unmask_Tim3(void)
{
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM3_INTR_MSK);
    mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);
}

/******************************************************************************
 *
 *
 ******************************************************************************/
void LSLCirq_Unmask_PKA(void)
{
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKA_INTR_MSK);
    mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);
}

/******************************************************************************
 *
 *
 ******************************************************************************/
void LSLCirq_Unmask_SYNC_DET(void)
{
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_SYNC_DET_INTR_MSK);
    mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);
}

/******************************************************************************
 *
 *
 ******************************************************************************/
void LSLCirq_Unmask_AUX_TIM(void)
{
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_AUX_TIM_INTR_MSK);
    mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);
}

/******************************************************************************
 *
 *
 ******************************************************************************/
void LSLCirq_Unmask_NO_PKT_RCVD(void)
{
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_NO_PKT_RCVD_INTR_MSK);
    mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);
}

/******************************************************************************
 *
 *
 ******************************************************************************/
void LSLCirq_Unmask_PKD_RX_HDR(void)
{
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKD_RX_HDR_INTR_MSK);
    mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);
}

/******************************************************************************
 *
 *
 ******************************************************************************/
void LSLCirq_Unmask_PKD(void)
{
    mHWreg_Create_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKD_INTR_MSK);
    mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);
}

#if (BUILD_TYPE == UNIT_TEST_BUILD)
void LSLCirq_IRQ_Handler(void)
{
    SYSirq_IRQ_Tabasco_ISR();
}
#endif

#else   /* (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 1) */
/******************************************************************************
 *
 * FUNCTION:  LSLCirq_IRQ_Handler
 * PURPOSE:   Services Interrupt Requests to micro-controller
 *
 ******************************************************************************/
void LSLCirq_IRQ_Handler(void)
{
	
	if(PATCH_FUN[LSLCIRQ_IRQ_HANDLER_ID]){
         ((void (*)(void))PATCH_FUN[LSLCIRQ_IRQ_HANDLER_ID])();
         return ;
    }

    mHWreg_Create_Cache_Register(JAL_COM_STAT_IRQ_REG);

#if (PRH_BS_CFG_SYS_LOW_POWER_MODE_SUPPORTED==1)
    //SYSpwr_Exit_Halt_System();
#endif

#if (PRH_BS_DEV_USE_CACHED_BT_CLOCKS==1)
    /* invalidate cached clocks on each interrupt */
    mHWreg_Clear_Cache_Register(JAL_NATIVE_CLK);
    mHWreg_Clear_Cache_Register(JAL_BT_CLK_OFFSET);
#endif

    mHWreg_Load_Cache_Register(JAL_COM_STAT_IRQ_REG);
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_IRQ_REG);
       
    /*
     * SYNC_DET
     */
    if (mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_SYNC_DET_INTR))
    {
		if (le_mode)
			LE_LL_Ctrl_Handle_LE_IRQ(LE_SYNC_DET);	
			
        LSLCirq_Inc_Cntr(sync_det);
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_NO_PKT_RCVD_INTR_MSK);
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_NO_PKT_RCVD_INTR_CLR);
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_SYNC_DET_INTR_CLR);
        mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);

    }
    else if (mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_AES_INTR))
    {

    	/*
    	 	* This Interrupt is raised when the HW AES Engine is complete !!
    		 */
    	//LE_LL_Ctrl_Handle_LE_IRQ(LE_AES_COMPLETE);

        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_AES_INTR_CLR);
        mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);


    }
    /*
     * PKD_RX_HDR
     */
    else if (mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_PKD_RX_HDR_INTR))
    {
		if (le_mode)
			LE_LL_Ctrl_Handle_LE_IRQ(LE_RX_HDR);

        LSLCirq_Inc_Cntr(pkd_rx_hdr);
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_NO_PKT_RCVD_INTR_CLR);
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_NO_PKT_RCVD_INTR_MSK);
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKD_RX_HDR_INTR_CLR);
        mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);  

    }
    
    /*
     * PKD
     */
    else if (mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_PKD_INTR))
    {
        if(pkd_op != (pkd_t) 0) pkd_op();
        
		if (le_mode)
			LE_LL_Ctrl_Handle_LE_IRQ(LE_PKD);

        LSLCirq_Inc_Cntr(pkd);
        mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_NO_PKT_RCVD_INTR_MSK);
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_NO_PKT_RCVD_INTR_CLR);

        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKD_INTR_CLR);
        mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);

	//	LSLCirq_DEBUG_Max_Irq_Times(init_nat_cnt,MAX_PKD_TIME);

    }
    
    /*
     * NO_PKT_RCVD
     */
    else if (mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_NO_PKT_RCVD_INTR))
    {
        if(pkd_op != (pkd_t) 0) pkd_op();
        
		if (le_mode)
			LE_LL_Ctrl_Handle_LE_IRQ(LE_NO_PKD);

        LSLCirq_Inc_Cntr(no_pkt_rcvd);
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_NO_PKT_RCVD_INTR_CLR);
        mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);

	//	LSLCirq_DEBUG_Max_Irq_Times(init_nat_cnt,MAX_NO_PKT_RCVD_TIME);
    }

    /*
     * PKA
     */
    else if (mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_PKA_INTR))
    {
        if(pka_op != (pka_t) 0) pka_op();
        
		if (le_mode)
			LE_LL_Ctrl_Handle_LE_IRQ(LE_PKA);

        LSLCirq_Inc_Cntr(pka);
        mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKA_INTR_CLR);
        mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);
    }

    /*
     * AUX_TIM
     */
    else if (mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_AUX_TIM_INTR))
    {
        /* important that the AUX_TIM is CLEARED before it is reset... */
        HW_set_aux_tim_intr_clr(1); /* force the clear immediately */
        LSLCslot_Handle_AUX_TIM();
        LSLCirq_Inc_Cntr(aux_tim);
    }
    
    else 
    {
        	u_int32 tim_handled = 0;        
			/*
             * TIM 2 (and not pending TIM1) 
             */
            if(((mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_TIM2_INTR))&(~tim_handled)) &&
              !((mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_TIM1_INTR))&(~tim_handled)))
			{
			
				if (le_mode)	{
					if((do_Connection_Enter_LowPower() == 0) && (do_Connection_Exit_LowPower(LE_TIM_2) == 0))	{
						LE_LL_Ctrl_Handle_LE_IRQ(LE_TIM_2);
					}
				}

                tim_handled |= JAL_TIM2_INTR_MASK;
                LSLCirq_Inc_Cntr(tim2);
                HW_set_no_pkt_rcvd_intr_mask(0);

            }
        
            /*
             * TIM3 (and not pending TIM2)
             */
            else if(((mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_TIM3_INTR))&(~tim_handled)) &&
              !((mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_TIM2_INTR))&(~tim_handled)))
            {
            
				if (le_mode) {
					
					if(do_Connection_Exit_LowPower(LE_TIM_3)==0) {

                        /*- T3 for Adv Event inside interval: 1.25ms. -*/
                        LE_LL_Ctrl_Handle_LE_IRQ(LE_TIM_3);
                    }            						
				}
                tim_handled |= JAL_TIM3_INTR_MASK;
                LSLCirq_Inc_Cntr(tim3);
            }
        
            /*
             * TIM0 (and not pending TIM3)
             */
            else if(((mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_TIM0_INTR))&(~tim_handled)) &&
              !((mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_TIM3_INTR))&(~tim_handled)))
            {
            
				if (le_mode) {
					
					if( do_Connection_Exit_LowPower(LE_TIM_0) == 0)	{

						LE_LL_Ctrl_Handle_LE_IRQ(LE_TIM_0);	
					}
				}	

				tim_handled |= JAL_TIM0_INTR_MASK;
                LSLCirq_Inc_Cntr(tim0);

            }
        
            /*
             * TIM1 (and not pending TIM0)
             */
            else if(((mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_TIM1_INTR))&(~tim_handled)) &&
              !((mHWreg_If_Cache_Bit_is_1(JAL_COM_STAT_IRQ_REG, JAL_TIM0_INTR))&(~tim_handled)))
            {	
            
				if (le_mode) {
					
					if( do_Connection_Exit_LowPower(LE_TIM_1) == 0)	{

						native_bt_clk_restore_done();
						LE_LL_Ctrl_Handle_LE_IRQ(LE_TIM_1);	
					}
				}	
				tim_handled |= JAL_TIM1_INTR_MASK;
                LSLCirq_Inc_Cntr(tim1);
            }
			else   // All 4 Tims are Set
			{
				tim_handled = (JAL_TIM1_INTR_MASK|JAL_TIM2_INTR_MASK|JAL_TIM3_INTR_MASK|JAL_TIM0_INTR_MASK);
			}

			HW_set_tim_intr_clr(tim_handled);
    }

    LSLCirq_Inc_Cntr(total_irq);
    return;
}

#endif  /* (PRH_BS_DEV_USE_DELAYED_SERVICE_ROUTINES == 1) */
#ifndef REDUCE_ROM
void LSLCirq_Disable_All_Intr_Except(u_int32 except_tim_intr_mask)
{
	
	if(PATCH_FUN[LSLCIRQ_DISABLE_ALL_INTR_EXCEPT_ID]){
         ((void (*)(u_int32 except_tim_intr_mask))PATCH_FUN[LSLCIRQ_DISABLE_ALL_INTR_EXCEPT_ID])(except_tim_intr_mask);
         return ;
    }

    /* NOTE: JAL_COM_CTRL_IRQ_REG already cached globally */

    /*
     * Mask out all interrupts except 'except_tim_intr'
     */    
    mHWreg_Assign_Cache_Field(JAL_COM_CTRL_IRQ_REG, JAL_TIM_INTR_MSK,
           (JAL_TIM_INTR_MSK_MASK & ~(except_tim_intr_mask)));

    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_SYNC_DET_INTR_MSK);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_NO_PKT_RCVD_INTR_MSK);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKD_RX_HDR_INTR_MSK);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKD_INTR_MSK);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKA_INTR_MSK);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_AES_INTR_MSK);

    /*
     * Clear any outstanding interrupts
     */
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM0_INTR_CLR);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM1_INTR_CLR);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM2_INTR_CLR);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM3_INTR_CLR);

    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_SYNC_DET_INTR_CLR);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_NO_PKT_RCVD_INTR_CLR);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKD_RX_HDR_INTR_CLR);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKD_INTR_CLR);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKA_INTR_CLR);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_AES_INTR_CLR);

    mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);
}
#endif

void LSLCirq_Disable_All_Tim_Intr_Except(u_int32 except_tim_intr_mask)
{
	if(PATCH_FUN[LSLCIRQ_DISABLE_ALL_TIM_INTR_EXCEPT_ID]){
         ((void (*)(u_int32 except_tim_intr_mask))PATCH_FUN[LSLCIRQ_DISABLE_ALL_TIM_INTR_EXCEPT_ID])(except_tim_intr_mask);
         return ;
    }

    mHWreg_Assign_Cache_Field(JAL_COM_CTRL_IRQ_REG, JAL_TIM_INTR_MSK,
           (JAL_TIM_INTR_MSK_MASK & ~(except_tim_intr_mask)));

    /*
     * Clear any outstanding interrupts
     */
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM0_INTR_CLR);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM1_INTR_CLR);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM2_INTR_CLR);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM3_INTR_CLR);

    mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);

}
#if	0
/* change name! R2P -> Return to Piconet, not return to own piconet? */
void LSLCirq_R2P_Turn_On_Intr(void)
{    
    /* NOTE: JAL_COM_CTRL_IRQ_REG already cached globally */

    /*
     * Turn on all interrupts.
     */
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM0_INTR_MSK);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM2_INTR_MSK);


#if (TABASCO_OLD_SER_BLK_SUPPORTED==1)
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM1_INTR_MSK);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM3_INTR_MSK);
#else
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM1_INTR_MSK);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM3_INTR_MSK);
#endif


    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_SYNC_DET_INTR_MSK);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_NO_PKT_RCVD_INTR_MSK);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKD_RX_HDR_INTR_MSK);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKD_INTR_MSK);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKA_INTR_MSK);
    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_AES_INTR_MSK);

    /*
     * Clear any outstanding interrupts
     */
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM0_INTR_CLR);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM1_INTR_CLR);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM2_INTR_CLR);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_TIM3_INTR_CLR);

    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_SYNC_DET_INTR_CLR);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_NO_PKT_RCVD_INTR_CLR);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKD_RX_HDR_INTR_CLR);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKD_INTR_CLR);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_PKA_INTR_CLR);
    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_AES_INTR_CLR);


    mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);

}
#endif


/******************************************************************************
 *
 * FUNCTION:  LSLCirq_Disable_And_Clear_Tabasco_Intr()
 * PURPOSE:   Disable and Clear all Tabasco Interrupts
 *
 ******************************************************************************/
void LSLCirq_Disable_And_Clear_Tabasco_Intr(void)
{
    /* note: currently only requires handling Jalapeno interrupts */
    mHWreg_Assign_Register(JAL_COM_CTRL_IRQ_REG, 0xFFFFFFFF);
    mHWreg_Assign_Register(JAL_COM_CTRL_IRQ_REG, 0xFFFFFFFF);
}
#ifndef REDUCE_ROM
void LSLCirq_Enable_Aux_Tim_Intr(void)
{
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_IRQ_REG);

    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_AUX_TIM_INTR_MSK);

    mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);

}

void LSLCirq_Disable_Aux_Tim_Intr(void)
{
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_IRQ_REG);

    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_AUX_TIM_INTR_MSK);

    mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);

}
#endif
#if (SYS_CFG_LE_END_SCAN_WINDOW_FROM_SCHED_CONTEST==1)
void LSLCirq_Enable_Sync_Det_Intr(void)
{
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_IRQ_REG);

    mHWreg_Clear_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_SYNC_DET_INTR_MSK);

    mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);

}

void LSLCirq_Disable_Sync_Det_Intr(void)
{
    mHWreg_Load_Cache_Register(JAL_COM_CTRL_IRQ_REG);

    mHWreg_Assert_Cache_Bit(JAL_COM_CTRL_IRQ_REG, JAL_SYNC_DET_INTR_MSK);

    mHWreg_Store_Cache_Register(JAL_COM_CTRL_IRQ_REG);

}
#endif

