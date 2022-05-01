/*****************************************************************************
 *
 * MODULE NAME:    sys_main.c
 * PROJECT CODE:   BlueStream
 * DESCRIPTION:    Entry point to the Bluetooth stack
 * MAINTAINER:     Tom Kerwick
 * DATE:           21 January 2000
 *
 * SOURCE CONTROL: $Id: sys_main.c,v 1.56 2008/07/03 12:26:45 tomk Exp $
 *
 * LICENSE:
 *     This source code is copyright (c) 2000-2008 Ceva Inc.
 *     All rights reserved.
 * 
 *****************************************************************************/

#include "sys_config.h"
#include "sys_types.h"
#include "sys_const.h"
#include "sys_post.h"

#include "bt_init.h"
#include "bt_mini_sched.h"

#include "sys_irq.h"

#include "hw_lc.h"
#include "hw_leds.h"
#include "hw_register.h"
#include "hw_hab_defs.h"

#include "tra_hcit.h"
#include "xinc_m0.h"
#include "sys_debug_config.h"


void CPR_CTL()
{
	//- bt clk
	*((u_int32 volatile*)0x40000074) = 0x40004; 			//- bt32k_clk
	*((u_int32 volatile*)0x40000070) = 0x4000400;			//- bt_pclk 
#if	(CPU_CLK == 16000000ul)	//cpu_clk = 16mhz	
	*((u_int32 volatile*)0x40000040) = 0x110010;			//- cpr_bt_clk_ctl
	*((u_int32 volatile*)0x40000048) = 0x110010;			//- cpr_bt_modem_clk_ctl
#else						//cpu_clk = 32mhz		
	*((u_int32 volatile*)0x40000040) = 0x110011;			//- cpr_bt_clk_ctl
	*((u_int32 volatile*)0x40000048) = BLE_MODEM_CLK_ON/*0x110011*/; //- cpr_bt_modem_clk_ctl	
#endif

	uint32_t	val;
#if (BUILD_TYPE != GENERIC_BUILD)
	//- uart1 clk	
	*((u_int32 volatile*)0x40000070) = 0x200020;
	//- uart1 pin	
	__read_hw_reg32(CPR_GPIO_FUN_SEL0, val);
	val &= ~(0x1f<<0);
	val |= (8<<0);
	__write_hw_reg32(CPR_GPIO_FUN_SEL0, val);		//- gpio0 > tx

	__read_hw_reg32(CPR_GPIO_FUN_SEL0, val);	 
	val &= ~(0x1f<<8);
	val |= (7<<8);
	__write_hw_reg32(CPR_GPIO_FUN_SEL0,val);		//- gpio1 > rx
#endif
	
#if(__DEBUG_TESTPIN_ENABLE == 1)
	// tx rx
    //gpio_mux_ctl(6,2);
    gpio_mux_ctl(7,3);
    gpio_mux_ctl(3,3);//- Test_Pin0 ( gpio3)
    gpio_mux_ctl(4,3);//- Test_Pin1 ( gpio4)
//    gpio_mux_ctl(18,3);//- Test_Pin2 ( gpio18)
    gpio_mux_ctl(6,3);//- Test_Pin3 ( gpio6)
    gpio_mux_ctl(20,3);//- Test_Pin4 ( gpio20)    
    gpio_mux_ctl(21,3);//- Test_Pin5 ( gpio21)
    gpio_mux_ctl(22,3);//- Test_Pin6 ( gpio22)
    gpio_mux_ctl(23,3);//- Test_Pin7 ( gpio23)
    gpio_mux_ctl(24,3);//- Test_Pin8 ( gpio24)
    gpio_mux_ctl(25,3);//- Test_Pin9 ( gpio25)



    //gpio_mux_ctl(20,3);//- Test_Pin20 ( gpio24)
    //gpio_mux_ctl(9,3);//- Test_Pin9 ( gpio25)
#if 0
    *((u_int32 volatile*)0x40000170) = 68;  //- Test_Pin0 ( gpio3) > RX MODE/tx_enable
    *((u_int32 volatile*)0x40000174) = 69;  //- Test_Pin1 ( gpio4) > rx_enable

    *((u_int32 volatile*)0x40000178) = 13;  //- Test_Pin2 ( gpio7) > pka_intr
//    *((u_int32 volatile*)0x4000017C) = 28;  //- Test_Pin3 ( gpio6) > rf_tx_en
	*((u_int32 volatile*)0x4000017C) = 5;  //- Test_Pin3 ( gpio6) > tim0
//    *((u_int32 volatile*)0x40000180) = 27;  //- Test_Pin4 ( gpio20) >rsm_finsh 
    *((u_int32 volatile*)0x40000180) = 26;  //- Test_Pin4 ( gpio20) >tsm_finsh 

    *((u_int32 volatile*)0x40000184) = 10;  //- Test_Pin5 ( gpio21) > tim2
    //*((u_int32 volatile*)0x40000188) = 9;  //- Test_Pin6 ( gpio22) > pkd
    *((u_int32 volatile*)0x40000188) = 34;  //- Test_Pin6 ( gpio22) > rxdata
    *((u_int32 volatile*)0x4000018c) = 2;  //- Test_Pin7 ( gpio23) > pkd_crc_err
    *((u_int32 volatile*)0x40000190) = 6;  //- Test_Pin8 ( gpio24) > tim1  

    //*((u_int32 volatile*)0x40000194) = 13;  //- Test_Pin9 ( gpio25) > pka
    *((u_int32 volatile*)0x40000194) = 33;  //- Test_Pin9 ( gpio25) > txdata
#endif
    *((u_int32 volatile*)0x40000170) = 68;  //- Test_Pin0 ( gpio3) > RX MODE/tx_enable
    *((u_int32 volatile*)0x40000174) = 69;  //- Test_Pin1 ( gpio4) > rx_enable

    *((u_int32 volatile*)0x40000178) = 9;  //- Test_Pin2 ( gpio7) > pka_intr
//    *((u_int32 volatile*)0x4000017C) = 28;  //- Test_Pin3 ( gpio6) > rf_tx_en
	*((u_int32 volatile*)0x4000017C) = 5;  //- Test_Pin3 ( gpio6) > tim0
//    *((u_int32 volatile*)0x40000180) = 27;  //- Test_Pin4 ( gpio20) >rsm_finsh 
    *((u_int32 volatile*)0x40000180) = 1;  //- Test_Pin4 ( gpio20) >tsm_finsh 

    *((u_int32 volatile*)0x40000184) = 10;  //- Test_Pin5 ( gpio21) > tim2
    //*((u_int32 volatile*)0x40000188) = 9;  //- Test_Pin6 ( gpio22) > pkd
    *((u_int32 volatile*)0x40000188) = 34;  //- Test_Pin6 ( gpio22) > rxdata
    *((u_int32 volatile*)0x4000018c) = 2;  //- Test_Pin7 ( gpio23) > pkd_crc_err
    *((u_int32 volatile*)0x40000190) = 6;  //- Test_Pin8 ( gpio24) > tim1  

    //*((u_int32 volatile*)0x40000194) = 13;  //- Test_Pin9 ( gpio25) > pka
    *((u_int32 volatile*)0x40000194) = 33;  //- Test_Pin9 ( gpio25) > txdata	
#endif
	
#if (__DEBUG_IO_ENABLE_XINC == 1)
	__DEBUG_IO_0_INIT_LOW_XINC();		
#endif
    
}


#if (BUILD_TYPE != GENERIC_BUILD)

void _SYS_Main_Loop(void);
void SYS_Main_Initialise(void);
void SYS_Main(void);


/*****************************************************************************
 *
 * FUNCTION:    main
 * PURPOSE:     c entry
 *
*****************************************************************************/
int main (int argc, char *argv[])
{
#if defined(SYS_HAL_OS_EVENT_DRIVEN_HOST_CONTROLLER)  //- add #if #endif.
    os_main();
#else	
    SYS_Main(); 									 //- never returns 
#endif
    return 0;
}

/*****************************************************************************
 *
 * FUNCTION:    SYS_Main
 * PURPOSE:     Instantiation
 *
 *****************************************************************************/
void SYS_Main(void)
{
	CPR_CTL();
	
    SYS_Main_Initialise();

    _SYS_Main_Loop();
}

/******************************************************************************
 *
 * FUNCTION:  SYS_Main_Initialise
 * PURPOSE:   Controller Initialisations
 *
 ******************************************************************************/

void SYS_Main_Initialise(void)
{
    u_int32 flags;

    SYSirq_Disable_Interrupts_Save_Flags(&flags);

    SYSconfig_Initialise();

    /*
     * Set up the HCI transport registration function (this function
     * is called on intialisation or re-initialisation of the HCI
     * transport sub-system
     */
#if (BUILD_TYPE==GENERIC_BUILD)
    TRAhcit_Set_Register_Function(HCI_Generic_register_hcit);
#else
    TRAhcit_Set_Register_Function(TRAhcit_Register_UART_Driver);
#endif

    BT_Initialise();


    SYSirq_Enable_Interrupts_Save_Flags(&flags);
}


/******************************************************************************
 *
 * FUNCTION:  _SYS_Main_Loop
 * PURPOSE:   Controller Main Loop
 *
 ******************************************************************************/
static void _SYS_Main_Loop(void)
{
   while(1)
   {
#if defined(SYS_HAL_OS_EVENT_DRIVEN_HOST_CONTROLLER)
       BTms_Callback();
#else
       BTms_Sched(0x1);
#endif
   }
    
}

#elif  (BUILD_TYPE == GENERIC_BUILD)


void SYS_Main_Initialise(void);
extern struct tra_hcit_info sys_hcit_generic_info;
void *HCI_Generic_register_hcit(void)
{
    //static struct tra_hcit_info sys_hcit_generic_info;

    sys_hcit_generic_info.type = TRA_HCIT_TYPE_GENERIC;

    sys_hcit_generic_info.tx_evt_busy = 0;
    sys_hcit_generic_info.tx_acl_busy = 0;
    sys_hcit_generic_info.tx_sco_busy = 0;

    return (void*)&sys_hcit_generic_info;
}
//void (*hci_generic_exit_hc_callback)(void) = NULL;

/*****************************************************************************
 *
 * FUNCTION:    ble_linklayer_init
 * PURPOSE:     Instantiation
 *
 *****************************************************************************/
void ble_linklayer_init(void)
{
    CPR_CTL();

    SYS_Main_Initialise();

#if defined(SYS_HAL_OS_EVENT_DRIVEN_HOST_CONTROLLER)  
    bt_os_attach();
#endif

}

/******************************************************************************
 *
 * FUNCTION:  SYS_Main_Initialise
 * PURPOSE:   Controller Initialisations
 *
 ******************************************************************************/

void SYS_Main_Initialise(void)
{
    u_int32 flags;

    SYSirq_Disable_Interrupts_Save_Flags(&flags);


    SYSconfig_Initialise();

    /*
     * Set up the HCI transport registration function (this function
     * is called on intialisation or re-initialisation of the HCI
     * transport sub-system
     */
#if (BUILD_TYPE==GENERIC_BUILD)
    TRAhcit_Set_Register_Function(HCI_Generic_register_hcit);
#else
    TRAhcit_Set_Register_Function(TRAhcit_Register_UART_Driver);
#endif

    BT_Initialise();

    SYSirq_Enable_Interrupts_Save_Flags(&flags);
}

/******************************************************************************
 *
 * FUNCTION:  _SYS_Main_Loop
 * PURPOSE:   Controller Main Loop
 *
 ******************************************************************************/
void	ble_linklayer_schedule(void)
{
#if defined(SYS_HAL_OS_EVENT_DRIVEN_HOST_CONTROLLER)
       BTms_Callback();
#else
       BTms_Sched(0x1);
#endif

}

#endif
