
/** @addtogroup Nordic Semiconductor
  * @{
  */


/** @addtogroup nrf52840
  * @{
  */


#ifndef NRF52840_H
#define NRF52840_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif


/** @addtogroup Configuration_of_CMSIS
  * @{
  */

/* following defines should be used for structure members */
#define     __IM     volatile const      /*! Defines 'read only' structure member permissions */
#define     __OM     volatile            /*! Defines 'write only' structure member permissions */
#define     __IOM    volatile            /*! Defines 'read / write' structure member permissions */

#define setbit(x,y) ((x) |= (1<<(y)))
#define clrbit(x,y) ((x) &= ~(1<<(y)))
/* =========================================================================================================================== */
/* ================                                Interrupt Number Definition                                ================ */
/* =========================================================================================================================== */
#if 0
typedef enum {
/* =======================================  ARM Cortex-M4 Specific Interrupt Numbers  ======================================== */
  Reset_IRQn                = -15,              /*!< -15  Reset Vector, invoked on Power up and warm reset                     */
  NonMaskableInt_IRQn       = -14,              /*!< -14  Non maskable Interrupt, cannot be stopped or preempted               */
  HardFault_IRQn            = -13,              /*!< -13  Hard Fault, all classes of Fault                                     */
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
  TIMER0_IRQn               =   8,              /*!< 8  TIMER0                                                                 */
  TIMER1_IRQn               =   9,              /*!< 9  TIMER1                                                                 */
  TIMER2_IRQn               =  10,              /*!< 10 TIMER2                                                                 */
  RTC0_IRQn                 =  11,              /*!< 11 RTC0                                                                   */
  TEMP_IRQn                 =  12,              /*!< 12 TEMP                                                                   */
  RNG_IRQn                  =  13,              /*!< 13 RNG                                                                    */
  ECB_IRQn                  =  14,              /*!< 14 ECB                                                                    */
  CCM_AAR_IRQn              =  15,              /*!< 15 CCM_AAR                                                                */
  WDT_IRQn                  =  16,              /*!< 16 WDT                                                                    */
  RTC1_IRQn                 =  17,              /*!< 17 RTC1                                                                   */
  QDEC_IRQn                 =  18,              /*!< 18 QDEC                                                                   */
  COMP_LPCOMP_IRQn          =  19,              /*!< 19 COMP_LPCOMP                                                            */
  SWI0_EGU0_IRQn            =  20,              /*!< 20 SWI0_EGU0                                                              */
  SWI1_EGU1_IRQn            =  21,              /*!< 21 SWI1_EGU1                                                              */
  SWI2_EGU2_IRQn            =  22,              /*!< 22 SWI2_EGU2                                                              */
  SWI3_EGU3_IRQn            =  23,              /*!< 23 SWI3_EGU3                                                              */
  SWI4_EGU4_IRQn            =  24,              /*!< 24 SWI4_EGU4                                                              */
  SWI5_EGU5_IRQn            =  25,              /*!< 25 SWI5_EGU5                                                              */
  TIMER3_IRQn               =  26,              /*!< 26 TIMER3                                                                 */
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
  SPIM3_IRQn                =  47               /*!< 47 SPIM3                                                                  */
} IRQn_Type;
#endif 

#include "xinc_m0.h"


 
/* =========================================================================================================================== */
/* ================                                           NVMC                                            ================ */
/* =========================================================================================================================== */


/**
  * @brief Non Volatile Memory Controller (NVMC)
  */

typedef struct {                                /*!< (@ 0x4001E000) NVMC Structure                                             */
  __IM  uint32_t  RESERVED[256];
  __IM  uint32_t  READY;                        /*!< (@ 0x00000400) Ready flag                                                 */
  __IM  uint32_t  RESERVED1;
  __IM  uint32_t  READYNEXT;                    /*!< (@ 0x00000408) Ready flag                                                 */
  __IM  uint32_t  RESERVED2[62];
  __IOM uint32_t  CONFIG;                       /*!< (@ 0x00000504) Configuration register                                     */
  
  union {
    __IOM uint32_t ERASEPAGE;                   /*!< (@ 0x00000508) Register for erasing a page in code area                   */
    __IOM uint32_t ERASEPCR1;                   /*!< (@ 0x00000508) Deprecated register - Register for erasing a
                                                                    page in code area. Equivalent to ERASEPAGE.                */
  };
  __IOM uint32_t  ERASEALL;                     /*!< (@ 0x0000050C) Register for erasing all non-volatile user memory          */
  __IOM uint32_t  ERASEPCR0;                    /*!< (@ 0x00000510) Deprecated register - Register for erasing a
                                                                    page in code area. Equivalent to ERASEPAGE.                */
  __IOM uint32_t  ERASEUICR;                    /*!< (@ 0x00000514) Register for erasing user information configuration
                                                                    registers                                                  */
  __IOM uint32_t  ERASEPAGEPARTIAL;             /*!< (@ 0x00000518) Register for partial erase of a page in code
                                                                    area                                                       */
  __IOM uint32_t  ERASEPAGEPARTIALCFG;          /*!< (@ 0x0000051C) Register for partial erase configuration                   */
  __IM  uint32_t  RESERVED3[8];
  __IOM uint32_t  ICACHECNF;                    /*!< (@ 0x00000540) I-code cache configuration register.                       */
  __IM  uint32_t  RESERVED4;
  __IOM uint32_t  IHIT;                         /*!< (@ 0x00000548) I-code cache hit counter.                                  */
  __IOM uint32_t  IMISS;                        /*!< (@ 0x0000054C) I-code cache miss counter.                                 */
} NRF_NVMC_Type;                                /*!< Size = 1360 (0x550)                                                       */



/**
  * @brief GPIO Port 1 (P0)
  */
#if 0
typedef struct {                                /*!< (@ 0x50000000) P0 Structure                                               */
  __IM  uint32_t  RESERVED[321];
  __IOM uint32_t  OUT;                          /*!< (@ 0x00000504) Write GPIO port                                            */
  __IOM uint32_t  OUTSET;                       /*!< (@ 0x00000508) Set individual bits in GPIO port                           */
  __IOM uint32_t  OUTCLR;                       /*!< (@ 0x0000050C) Clear individual bits in GPIO port                         */
  __IM  uint32_t  IN;                           /*!< (@ 0x00000510) Read GPIO port                                             */
  __IOM uint32_t  DIR;                          /*!< (@ 0x00000514) Direction of GPIO pins                                     */
  __IOM uint32_t  DIRSET;                       /*!< (@ 0x00000518) DIR set register                                           */
  __IOM uint32_t  DIRCLR;                       /*!< (@ 0x0000051C) DIR clear register                                         */
  __IOM uint32_t  LATCH;                        /*!< (@ 0x00000520) Latch register indicating what GPIO pins that
                                                                    have met the criteria set in the PIN_CNF[n].SENSE
                                                                    registers                                                  */
  __IOM uint32_t  DETECTMODE;                   /*!< (@ 0x00000524) Select between default DETECT signal behaviour
                                                                    and LDETECT mode                                           */
  __IM  uint32_t  RESERVED1[118];
  __IOM uint32_t  PIN_CNF[32];                  /*!< (@ 0x00000700) Description collection: Configuration of GPIO
                                                                    pins                                                       */
} NRF_GPIO_Type;    

#else

typedef struct {                                /*!< (@ 0x40001000) P0 Structure                                               */
	__IOM uint32_t  DR[2];   										/*!< (@ 0x40001000) GPIO_PORT_DRx */
  __IM  uint32_t  RESERVED[6];
	__IOM uint32_t  DDR[2];   										/*!< (@ 0x40001020) GPIO_PORT_DATA DRx */
	__IM  uint32_t  RESERVED1[6];
  __IOM uint32_t  EXT_PORT0;                    /*!< (@ 0x40001040) Read GPIO port                                            */
	__IM  uint32_t  RESERVED2[47];
	__IOM uint32_t  INTR_CTRL[8];									/*!< (@ 0x40001100) Intr ctr GPIO port                                         */
	__IM  uint32_t  RESERVED3[24];
  __IOM uint32_t  DEBOUNCE[2];                   /*!< (@ 0x40001180) Set individual bits in GPIO port                           */
	__IM  uint32_t  RESERVED4[6];
  __IM uint32_t   INTR_RAW0;                      /*!< (@ 0x400011A0) Intr RAW                         */
  __IM  uint32_t  RESERVED5[3];
  __IOM uint32_t  INTR_CLR0;                     /*!< (@ 0x400011B0) Intr Clear                                      */
	__IM  uint32_t  RESERVED6[19];
  __IOM uint32_t  INTR_MASK_C0[2];                       /*!< (@ 0x40001200) DIR set register                                           */
	__IM  uint32_t  RESERVED7[6];
  __IM uint32_t   INTR_STATUS_C0;                       /*!< (@ 0x40001220) DIR clear register                                         */
                                                    
} NRF_GPIO_Type;//XINC_GPIO_Type;    /*!< Size = 548 (0x224)   */
#endif
typedef struct { 
	 __IM  uint32_t  RESERVED[13]; /*!< (@ 0x40002400) CPR AO Structure                                               */
	__IOM uint32_t  PU_CTRL1;  /*!< (@ 0x40002434) CPR AO  GPIO PU Ctrl                                               */
  __IOM uint32_t  PE_CTRL2; 	/*!< (@ 0x40002438) CPR AO  GPIO PE Ctrl                                               */
}XINC_CPRA_AO_Type;


#if 0
typedef struct {                                /*!< (@ 0x40006000) GPIOTE Structure                                           */
  __OM  uint32_t  TASKS_OUT[8];                 /*!< (@ 0x00000000) Description collection: Task for writing to pin
                                                                    specified in CONFIG[n].PSEL. Action on pin
                                                                    is configured in CONFIG[n].POLARITY.                       */
  __IM  uint32_t  RESERVED[4];
  __OM  uint32_t  TASKS_SET[8];                 /*!< (@ 0x00000030) Description collection: Task for writing to pin
                                                                    specified in CONFIG[n].PSEL. Action on pin
                                                                    is to set it high.                                         */
  __IM  uint32_t  RESERVED1[4];
  __OM  uint32_t  TASKS_CLR[8];                 /*!< (@ 0x00000060) Description collection: Task for writing to pin
                                                                    specified in CONFIG[n].PSEL. Action on pin
                                                                    is to set it low.                                          */
  __IM  uint32_t  RESERVED2[32];
  __IOM uint32_t  EVENTS_IN[8];                 /*!< (@ 0x00000100) Description collection: Event generated from
                                                                    pin specified in CONFIG[n].PSEL                            */
  __IM  uint32_t  RESERVED3[23];
  __IOM uint32_t  EVENTS_PORT;                  /*!< (@ 0x0000017C) Event generated from multiple input GPIO pins
                                                                    with SENSE mechanism enabled                               */
  __IM  uint32_t  RESERVED4[97];
  __IOM uint32_t  INTENSET;                     /*!< (@ 0x00000304) Enable interrupt                                           */
  __IOM uint32_t  INTENCLR;                     /*!< (@ 0x00000308) Disable interrupt                                          */
  __IM  uint32_t  RESERVED5[129];
  __IOM uint32_t  CONFIG[8];                    /*!< (@ 0x00000510) Description collection: Configuration for OUT[n],
                                                                    SET[n], and CLR[n] tasks and IN[n] event                   */
} NRF_GPIOTE_Type;                              /*!< Size = 1328 (0x530)   */
#else
typedef struct {                                /*!< (@ 0x40006000) GPIOTE Structure                                           */
  __OM  uint32_t  TASKS_OUT[8];                 /*!< (@ 0x00000000) Description collection: Task for writing to pin
                                                                    specified in CONFIG[n].PSEL. Action on pin
                                                                    is configured in CONFIG[n].POLARITY.                       */

  __OM  uint32_t  TASKS_SET[8];                 /*!< (@ 0x00000030) Description collection: Task for writing to pin
                                                                    specified in CONFIG[n].PSEL. Action on pin
                                                                    is to set it high.                                         */

  __OM  uint32_t  TASKS_CLR[8];                 /*!< (@ 0x00000060) Description collection: Task for writing to pin
                                                                    specified in CONFIG[n].PSEL. Action on pin
                                                                    is to set it low.                                          */

  __IOM uint32_t  EVENTS_IN[8];                 /*!< (@ 0x00000100) Description collection: Event generated from
                                                                    pin specified in CONFIG[n].PSEL                            */

  __IOM uint32_t  EVENTS_PORT;                  /*!< (@ 0x0000017C) Event generated from multiple input GPIO pins
                                                                    with SENSE mechanism enabled                               */

  __IOM uint32_t  INTENSET;                     /*!< (@ 0x00000304) Enable interrupt                                           */
  __IOM uint32_t  INTENCLR;                     /*!< (@ 0x00000308) Disable interrupt                                          */
  __IOM uint32_t  CONFIG[8];                    /*!< (@ 0x00000510) Description collection: Configuration for OUT[n],
                                                                    SET[n], and CLR[n] tasks and IN[n] event                   */
} NRF_GPIOTE_Type;                             
#endif
/* =========================================================================================================================== */
/* ================                          Device Specific Peripheral Address Map                           ================ */
/* =========================================================================================================================== */



typedef union
{
		__IM      uint32_t      RBR;
		__OM      uint32_t      THR;
	 __IOM      uint32_t      DLL;
} REG0;
   
typedef union
{
	 __IOM      uint32_t      IER;
	 __IOM      uint32_t      DLH;
} REG1;

typedef union
{
			 __IM      uint32_t      IIR;
			 __OM      uint32_t      FCR;
} REG2;

typedef struct {                                /*!< (@ 0x40011000) UART1 Structure                                            */
   REG0  RBR_THR_DLL;                                  
   REG1  IER_DLH;                                                         
   REG2  IIR_FCR;                
  __IOM  uint32_t  TCR;                
  __IOM  uint32_t  MCR;
  __IM  uint32_t  TSR;               
  __IM  uint32_t  MSR;
  __IM  uint32_t  USR;

} NRF_UART_Type;  


/* =========================================================================================================================== */
/* ================                                         SPIM0_NS                                          ================ */
/* =========================================================================================================================== */


/**
  * @brief Serial Peripheral Interface Master with EasyDMA 0 (SPIM0_NS)
  */
/**
  * @brief SPIM_PSEL [PSEL] (Unspecified)
  */
typedef struct {
  __IOM uint32_t  SCK;                          /*!< (@ 0x00000000) Pin select for SCK                                         */
  __IOM uint32_t  MOSI;                         /*!< (@ 0x00000004) Pin select for MOSI signal                                 */
  __IOM uint32_t  MISO;                         /*!< (@ 0x00000008) Pin select for MISO signal                                 */
} SPIM_PSEL_Type;  
/**
  * @brief SPIM_RXD [RXD] (RXD EasyDMA channel)
  */
typedef struct {
  __IOM uint32_t  PTR;                          /*!< (@ 0x00000000) Data pointer                                               */
  __IOM uint32_t  MAXCNT;                       /*!< (@ 0x00000004) Maximum number of bytes in receive buffer                  */
  __IM  uint32_t  AMOUNT;                       /*!< (@ 0x00000008) Number of bytes transferred in the last transaction        */
  __IOM uint32_t  LIST;                         /*!< (@ 0x0000000C) EasyDMA list type                                          */
} SPIM_RXD_Type;                                /*!< Size = 16 (0x10)                                                          */


/**
  * @brief SPIM_TXD [TXD] (TXD EasyDMA channel)
  */
typedef struct {
  __IOM uint32_t  PTR;                          /*!< (@ 0x00000000) Data pointer                                               */
  __IOM uint32_t  MAXCNT;                       /*!< (@ 0x00000004) Maximum number of bytes in transmit buffer                 */
  __IM  uint32_t  AMOUNT;                       /*!< (@ 0x00000008) Number of bytes transferred in the last transaction        */
  __IOM uint32_t  LIST;                         /*!< (@ 0x0000000C) EasyDMA list type                                          */
} SPIM_TXD_Type;     

typedef struct {                                /*!< (@ 0x40014000) SPIM1_NS Structure                                         */
  __IOM  uint32_t  CTRL0;
	__IOM  uint32_t  CTRL1;
	__IOM  uint32_t  EN;
	__IM  uint32_t  RESERVED0;
	__IOM  uint32_t  SE;
	__IOM  uint32_t  BAUD;
	__IOM  uint32_t  TXFLT;
	__IOM  uint32_t  RXFLT;
	__IM  uint32_t  TXFL;
	__IM  uint32_t  RXFL;
	__IM  uint32_t  STS;
	__IOM  uint32_t  IE;
	__IM  uint32_t  IS;
	__IM  uint32_t  RIS;
	__IM  uint32_t  TXOIC;
	__IM  uint32_t  RXOIC;
	__IM  uint32_t  RXUIC;
	__IM  uint32_t  RESERVED1;
	__IM  uint32_t  IC;
	__IOM  uint32_t DMAS;
	__IOM  uint32_t DMATDL;
	__IOM  uint32_t DMARDL;
	__IM  uint32_t  RESERVED2[2];
	__IOM  uint32_t DATA;
} NRF_SPIM_Type;                               

/** @addtogroup Device_Peripheral_peripheralAddr
  * @{
  */
#define NRF_P0_BASE                 0x40001000UL
#define XINC_CPR_AO_BASE            0x40002400UL
#define NRF_GPIOTE_BASE             0x40006000UL
#define NRF_UART0_BASE              0x40010000UL
#define NRF_UART1_BASE              0x40011000UL
#define NRF_SPIM1_BASE              0x40014000UL

/* =========================================================================================================================== */
/* ================                                  Peripheral declaration                                   ================ */
/* =========================================================================================================================== */


/** @addtogroup Device_Peripheral_declaration
  * @{
  */
#define NRF_P0                       ((NRF_GPIO_Type*)            NRF_P0_BASE)
#define NRF_CPR_AO               		 ((XINC_CPRA_AO_Type*)        XINC_CPR_AO_BASE)
#define NRF_GPIOTE                   ((NRF_GPIOTE_Type*)          NRF_GPIOTE_BASE)
#define NRF_UART0                    ((NRF_UART_Type*)            NRF_UART0_BASE)
#define NRF_UART1                    ((NRF_UART_Type*)            NRF_UART1_BASE)
#define NRF_SPIM1										 ((NRF_SPIM_Type*)            NRF_SPIM1_BASE)
#ifdef __cplusplus
}
#endif

#endif /* NRF52840_H */


/** @} */ /* End of group nrf52840 */

/** @} */ /* End of group Nordic Semiconductor */
