/**************************************************************************//**
 * @file     XINC_M0.h
 * @brief    CMSIS Cortex-M4 Core Peripheral Access Layer Header File 
 *           for the Device Series ...
 * @version  V1.00
 * @date     7. January 2013
 *
 * @note
 * Copyright (C) 2010 ARM Limited. All rights reserved.
 *
 * @par
 * ARM Limited (ARM) is supplying this software for use with Cortex-M 
 * processor based microcontrollers.  This file can be freely distributed 
 * within development tools that are supporting such ARM based processors. 
 *
 * @par
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 ******************************************************************************/

#ifndef __XC6XX_M0_H__
#define __XC6XX_M0_H__


#ifdef __cplusplus
 extern "C" {
#endif 

/******************************************************************************/
/*                Processor and Core Peripherals                              */
/******************************************************************************/
/** @addtogroup <Device>_CMSIS Device CMSIS Definitions
  Configuration of the Cortex-M# Processor and Core Peripherals
  @{
*/

/*
 * ==========================================================================
 * ---------- Interrupt Number Definition -----------------------------------
 * ==========================================================================
 */


 
/*
 * ==========================================================================
 * ---------- Interrupt Number Definition -----------------------------------
 * ==========================================================================
 */
typedef enum {
/* =======================================  ARM Cortex-M4 Specific Interrupt Numbers  ======================================== */
  Reset_IRQn                = -15,              /*!< -15  Reset Vector, invoked on Power up and warm reset                     */
  NonMaskableInt_IRQn       = -14,      /*!<  2 Non Maskable Interrupt                        */
  HardFault_IRQn            = -13,      /*!<  3 Hard Fault Interrupt                          */                                    
  MemoryManagement_IRQn     = -12,              /*!< -12  Memory Management, MPU mismatch, including Access Violation
                                                     and No Match                                                              */
  BusFault_IRQn             = -11,              /*!< -11  Bus Fault, Pre-Fetch-, Memory Access Fault, other address/memory
                                                     related Fault                                                             */
  UsageFault_IRQn           = -10,              /*!< -10  Usage Fault, i.e. Undef Instruction, Illegal State Transition        */
  SVCall_IRQn               =  -5,              /*!< -5 System Service Call via SVC instruction                                */                          
  DebugMonitor_IRQn         =  -4,              /*!< -4 Debug Monitor                                                          */
  PendSV_IRQn               =  -2,              /*!< -2 Pendable request for system service                                    */
  SysTick_IRQn              =  -1,              /*!< -1 System Tick Timer                                                      */
	
/* ==========================================  nrf52840 Specific Interrupt Numbers  ========================================== */
  POWER_CLOCK_IRQn          =   0,              /*!< 0  POWER_CLOCK                                                            */
  RADIO_IRQn                =   1,              /*!< 1  RADIO                                                                  */
  UARTE0_UART0_IRQn         =   2,              /*!< 2  UARTE0_UART0                                                           */
  SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQn=   3,  /*!< 3  SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0                                      */
  SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQn=   4,  /*!< 4  SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1                                      */
  NFCT_IRQn                 =   5,              /*!< 5  NFCT                                                                   */
  GPIOTE_IRQn               =   6,              /*!< 6  GPIOTE                                                                 */
  SAADC_IRQn                =   7,              /*!< 7  SAADC                                                                  */
  
  RTC0_IRQn                 =  8,              /*!< 11 RTC0                                                                   */
  TEMP_IRQn                 =  12,              /*!< 12 TEMP                                                                   */
  RNG_IRQn                  =  13,              /*!< 13 RNG                                                                    */
  ECB_IRQn                  =  14,              /*!< 14 ECB                                                                    */
  CCM_AAR_IRQn              =  15,              /*!< 15 CCM_AAR                                                                */
 
  RTC1_IRQn                 =  17,              /*!< 17 RTC1                                                                   */
  QDEC_IRQn                 =  16,              /*!< 18 QDEC                                                                   */
  COMP_LPCOMP_IRQn          =  19,              /*!< 19 COMP_LPCOMP                                                            */
  SWI0_EGU0_IRQn            =  20,              /*!< 20 SWI0_EGU0                                                              */
  SWI1_EGU1_IRQn            =  21,              /*!< 21 SWI1_EGU1                                                              */
  SWI2_EGU2_IRQn            =  22,              /*!< 22 SWI2_EGU2                                                              */
  SWI3_EGU3_IRQn            =  23,              /*!< 23 SWI3_EGU3                                                              */
  SWI4_EGU4_IRQn            =  24,              /*!< 24 SWI4_EGU4                                                              */
  SWI5_EGU5_IRQn            =  25,              /*!< 25 SWI5_EGU5                                                              */
  
  TIMER4_IRQn               =  27,              /*!< 27 TIMER4                                                                 */
  PWM0_IRQn                 =  28,              /*!< 28 PWM0                                                                   */
  PDM_IRQn                  =  29,              /*!< 29 PDM                                                                    */
  MWU_IRQn                  =  32,              /*!< 32 MWU                                                                    */
  PWM1_IRQn                 =  33,              /*!< 33 PWM1                                                                   */
  PWM2_IRQn                 =  34,              /*!< 34 PWM2                                                                   */
  SPIM2_SPIS2_SPI2_IRQn     =  35,              /*!< 35 SPIM2_SPIS2_SPI2                                                       */
  RTC2_IRQn                 =  36,              /*!< 36 RTC2                                                                   */
  I2S_IRQn                  =  37,              /*!< 37 I2S                                                                    */
  FPU_IRQn                  =  38,              /*!< 38 FPU                                                                    */
  USBD_IRQn                 =  39,              /*!< 39 USBD                                                                   */
  UARTE1_IRQn               =  40,              /*!< 40 UARTE1                                                                 */
  QSPI_IRQn                 =  41,              /*!< 41 QSPI                                                                   */
  CRYPTOCELL_IRQn           =  42,              /*!< 42 CRYPTOCELL                                                             */
  PWM3_IRQn                 =  45,              /*!< 45 PWM3                                                                   */
  SPIM3_IRQn                =  47,               /*!< 47 SPIM3  																																	*/

  BLUETOOTH_IRQn      			= 0,        /*!< Device Interrupt                                 */
  DMAS_IRQn	    						= 1,
  CPR_IRQn 			       			= 2,
  GPIO_IRQn    			    		= 3,
  RTC_IRQn			 		    		= 4,
	TIMER0_IRQn               = 5,              /*!< 5  TIMER0                                                                 */
  TIMER1_IRQn               = 6,              /*!< 6  TIMER1                                                                 */
  TIMER2_IRQn               = 7,              /*!< 7 TIMER2                                                                 */
	TIMER3_IRQn               = 8,              /*!< 8 TIMER3                                                                 */
	WDT_IRQn                  = 9,              /*!< 9 WDT                                                                    */
  I2C_IRQn      						= 10,
	UART0_IRQn	  						= 11,
	UART1_IRQn      					= 12,
	SPI0_IRQn	      					= 13,
	SPI1_IRQn		  						= 14,
	KBS_IRQn	  							= 15,
	GADC_IRQn	      					= 17,
	SIM_IRQn                  = 18,
	AES_IRQn                  = 19
} IRQn_Type;

/*
 * ==========================================================================
 * ----------- Processor and Core Peripheral Section ------------------------
 * ==========================================================================
 */

/* Configuration of the Cortex-M# Processor and Core Peripherals */
/* ToDo: set the defines according your Device                                                    */
#define __MPU_PRESENT             0         /*!< MPU present or not                               */
#define __NVIC_PRIO_BITS          2         /*!< Number of Bits used for Priority Levels          */
#define __Vendor_SysTickConfig    0         /*!< Set to 1 if different SysTick Config is used     */

#if defined ( __CC_ARM   )
  #if defined (__TARGET_FPU_VFP)
    #define __FPU_PRESENT         1         /*!< FPU present or not                               */
  #else
    #define __FPU_PRESENT         0         /*!< FPU present or not                               */
  #endif
#else
    #define __FPU_PRESENT         0         /*!< FPU present or not                               */
#endif

/*@}*/ /* end of group <Device>_CMSIS */


/* ToDo: include the correct core_cm#.h file
         core_cm0.h if your device is a CORTEX-M0 device
         core_cm3.h if your device is a CORTEX-M3 device
         core_cm4.h if your device is a CORTEX-M4 device                                          */
#include "core_cm0.h"
/* ToDo: include your system_<Device>.h file
         replace '<Device>' with your device name                                                 */


/******************************************************************************/
/*                Device Specific Peripheral registers structures             */
/******************************************************************************/
/** @addtogroup <Device>_Peripherals <Device> Peripherals
  <Device> Device Specific Peripheral registers structures
  @{
*/

#if defined ( __CC_ARM   )
#pragma anon_unions
#endif

#include	"xinc_reg.h"


#if defined ( __CC_ARM   )
#pragma no_anon_unions
#endif


#ifdef __cplusplus
}
#endif

#endif  // __<Device>_H__
