#ifndef __XINC_M0_H__
#define __XINC_M0_H__


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

 typedef enum IRQn
 {
 /******  Cortex-M# Processor Exceptions Numbers ***************************************************/

 /* ToDo: use this Cortex interrupt numbers if your device is a CORTEX-M4 device                   */
   NonMaskableInt_IRQn           = -14,      /*!<  2 Non Maskable Interrupt                        */
   HardFault_IRQn                = -13,      /*!<  3 Hard Fault Interrupt                          */
   SVCall_IRQn                   = -5,       /*!< 11 SV Call Interrupt                             */
   PendSV_IRQn                   = -2,       /*!< 14 Pend SV Interrupt                             */
   SysTick_IRQn                  = -1,       /*!< 15 System Tick Interrupt                         */

 /******  Device Specific Interrupt Numbers ********************************************************/
 /* ToDo: add here your device specific external interrupt numbers
          according the interrupt handlers defined in startup_Device.s
          eg.: Interrupt for Timer#1       TIM1_IRQHandler   ->   TIM1_IRQn                        */

   BLUETOOTH_IRQn      			= 0,        /*!< Device Interrupt                                 */
   DMAS_IRQn	    			= 1,
   CPR_IRQn 			        = 2,
   GPIO_IRQn    			    = 3,
   RTC_IRQn			 		    = 4,
   TIMER0_IRQn	        		= 5,
   TIMER1_IRQn	        		= 6,
   TIMER2_IRQn	        		= 7,
   TIMER3_IRQn	        		= 8,
   WDT_IRQn		      			= 9,
   I2C_IRQn      				= 10,
   UART0_IRQn	  				= 11,
   UART1_IRQn      				= 12,
   SPI0_IRQn	      			= 13,
   SPI1_IRQn		  			= 14,
   KBS_IRQn	  					= 15,
   RESV1_IRQn       			= 16,
   RESV2_IRQn	      			= 17,
   RESV3_IRQn                   = 18,
   RESV4_IRQn                   = 19
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

#include "core_cm0.h"

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
