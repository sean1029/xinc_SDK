
#ifndef		__SYS_DEBUG_CONFIG__
#define		__SYS_DEBUG_CONFIG__

#include    "sys_develop.h"
#include    "sys_features.h"
#include    "xinc_m0.h"


#define		CPU_CLK									32000000ul					//- cpu frequency

#define     __DEBUG_UART_RDATA_MONITOR__            0                           // 0:0ff/1:On.
#define     __DEBUG_RADIO_MAC_TO_MAC__              PRH_BS_DEV_WIRED_LINK       // 0:Off/1:On
#define     __DEBUG_RADIO_BYPASS__                  0                           // 0:Off/1:On
#define     __DEBUG_GPIO_OUT_ENABLE__               0                           // 0:Off/1:On
#define     __DEBUG_LE_WHITEN_OFF                   0                           // 0: on  1: OFF (normal:0)
                                           
#define		__DEBUG_RX_TEST_MODE_SET_TO_ONE__		0							// 0:Off/1:On	
#define		RX_DELAY_DEFINE							64			
#define		TX_DELAY_DEFINE							1
#define		ADV_DELTA_DEFINE						1

#define		RADIO_ZSW								0
#define		RADIO_2421								1	
#define		RADIO_0851								2
#define		RADIO_XINC								3
#define		__DEBUG_RADIO_TYPE						RADIO_XINC		//0: ZSW RF; 1: 2421

#define		__DEBUG_XINC_FPGA			    		1
#define		__DEBUG_XINC_CHIP               		1


#define		__DEBUG_VERTIBI_SWITH					0				// 0: OFF , 1 : ON
#define		__DEBUG_CW_WAVE						    0				// 0: OFF , 1 : ON
#define		__DEBUG_TX_ALIGN					    0       		//0 : OFF , 1: ON
#define		__DEBUG_TEST_RF_XINC					0				//0 : OFF , 1: ON


#define     __DEBUG_IO_HIGH()                      \
do                                                  \
{                                                   \
 /*   (*((volatile unsigned *)(0x4002803C ))) = 0x80000000;*/  \
}while(0)     

#define     __DEBUG_IO_LOW()                      \
do                                                  \
{                                                   \
/*    (*((volatile unsigned *)(0x4002803C ))) = 0x00000000;*/  \
}while(0)     

  

#define		__DEBUG_HARDWARE_PIN_ENABLE				1	    		//0:disable 1:enable

#if(__DEBUG_HARDWARE_PIN_ENABLE == 1)
#define		TX_CONST_VALUE		0x8000
#define		RX_CONST_VALUE		0x8000
#else
#if((__DEBUG_XINC_FPGA == 1) && (__DEBUG_XINC_CHIP ==0))
#define		TX_CONST_VALUE		0x8000			//- Enable HW PIN.
#define		RX_CONST_VALUE		0x5000
#else
#define		TX_CONST_VALUE		0x2000
#define		RX_CONST_VALUE		0x1000
#endif
#endif

#ifndef     __DEBUG_TESTPIN_ENABLE
#define		__DEBUG_TESTPIN_ENABLE				0				//0:disable 1:enable
#endif

#define		__DEBUG_RX_DATA_DUMP_TO_SHRAM0	    0
#define		__DUMP_DATA_WHILE					0
#define		__DUMP_DATA_WHILE_NOT_PKD			0

/* Lowpower */
#define     __DEBUG_RF_LOWPOWER_ENABLE          0   			//0:disable 1:enable
#define     __DEBUG_RF_MPW1_SHDN_ENABLE         0
#define     BTPHY_CTL0_SHDN                     0x2A7
#define     BTPHY_CTL0_NO_SHDN                  0x3A7
#define     __DEBUG_MODEM_LOWPOWER_ENABLE       0   			//0:disable 1:enable
#define     BLE_MODEM_CLK_ON           		    (0x110011)
#define     BLE_MODEM_CLK_OFF          		    (0x110000)
#define     __DEBUG_ADV_INTERVAL_LOW_PWR        0   			//-0:disable 1:enable

#define __DEBUG_XINCHIP_RF_LOWPOWER_ENABLE       1             //0:disable 1:enable

#ifndef     __DEBUG_IO_ENABLE_XINC    
#define     __DEBUG_IO_ENABLE_XINC           0   				//1:enable  0:disable
#endif
#define     __DEBUG_IO_0_INIT_LOW_XINC()                 	\
do                                                          \
{                                                           \
/*    (*((volatile unsigned *)(0x40001020 ))) = 0x00010001; */  \
/*    (*((volatile unsigned *)(0x40001000 ))) = 0x00010000; */  \
}while(0)

#define     __DEBUG_IO_0_HIGH_XINC()                       	\
do                                                          \
{                                                           \
 /*    (*((volatile unsigned *)(0x40001000 ))) = 0x00010001; */  \
}while(0)

#define     __DEBUG_IO_0_LOW_XINC()                        	\
do                                                          \
{                                                           \
/* 	(*((volatile unsigned *)(0x40001000 ))) = 0x00010000; */ \
}while(0)


#define			__DEBUG_BLE_TRM_Q					1			//0 : OFF 1: ON
/*---------------------------------------------------------------
- for Adv/connect sleep baseband configration.
---------------------------------------------------------------*/

/*--------------------------------------------
#define      __DEBUG_LE_SLEEP           0     //- for: TP/CON/SLA/BI-01-C
												 TP/CON/ADV/BV-04-C
		 										 TP/CON/SLA/BV-15-C
		 										 TP/CON/SLA/BI-02-C
		 										 TP/SEC/SLA/BV-01-C

#define      __DEBUG_LE_SLEEP          	2	  //- for: Normol WORK.	
---------------------------------------------*/
#define      __DEBUG_LE_SLEEP           2


#ifndef      __DEBUG_LE_LOWP_IO
#define      __DEBUG_LE_LOWP_IO         1
#endif
#if         (__DEBUG_LE_LOWP_IO == 1)
#define		__DEBUG_IO_LOW5()
#define		__DEBUG_IO_HIGH5()

#define     __DEBUG_IO_LOW55()                               \
do                                                          \
{                                                           \
/*	 (*((volatile unsigned *)0x40001020)) |=  0x0200020; */  \
/*    (*((volatile unsigned *)(0x40001000 ))) = 0x200000; */   \
}while(0)  

#define     __DEBUG_IO_HIGH55()                              \
do                                                          \
{                                                           \
 /*   (*((volatile unsigned *)0x40001020)) |=  0x200020;*/     \
 /*   (*((volatile unsigned *)(0x40001000 ))) = 0x200020;*/    \
}while(0) 

#define		SET_IO55(X)     do {                             \
			__DEBUG_IO_LOW55();                              \
			HWdelay_Wait_For_us(X);                         \
			__DEBUG_IO_HIGH55();                             \
			HWdelay_Wait_For_us(X);                         \
			__DEBUG_IO_LOW55();                              \
}while(0);

#define		SET_IO5(X) 

#else
#define		SET_IO5(X)
#define     __DEBUG_IO_LOW5()
#define     __DEBUG_IO_HIGH5()

#endif

/*---------------------------------------------------------------
- for acess code. 
---------------------------------------------------------------*/
#define		__DEBUG_ACCESS_CODE_FOR_BQT		1


/*--------------------------------------------------------------
- for rf specific operation
--------------------------------------------------------------*/
typedef     void    (*pka_t)(void);
typedef     void    (*pkd_t)(void);

#endif
