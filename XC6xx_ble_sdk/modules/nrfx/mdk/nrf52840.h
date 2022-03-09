
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

typedef struct { 
	 __IM  uint32_t  RESERVED[13]; /*!< (@ 0x40002400) CPR AO Structure                                               */
	__IOM uint32_t  PU_CTRL1;  /*!< (@ 0x40002434) CPR AO  GPIO PU Ctrl                                               */
  __IOM uint32_t  PE_CTRL2; 	/*!< (@ 0x40002438) CPR AO  GPIO PE Ctrl                                               */
}XINC_CPRA_AO_Type;

                           
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
} XINC_SPIM_Type;          


/* =========================================================================================================================== */
/* ================                                           I2C                                            ================ */
/* =========================================================================================================================== */


/**
  * @brief I2C compatible Two-Wire Interface 0 (I2C0)
  */
                                                        
typedef struct {                                /*!< (@ 0x40006000) I2C Structure                                             */
	__IOM uint32_t i2c_CON;
	__IOM uint32_t i2c_TAR;
	__IOM uint32_t i2c_SAR;
	__IM  uint32_t  RESERVED0;
	
	__IOM uint32_t i2c_DATA_CMD;
	__IOM uint32_t i2c_SS_SCL_HCNT;
	__IOM uint32_t i2c_SS_SCL_LCNT;
	__IOM uint32_t i2c_FS_SCL_HCNT;
	
	__IOM uint32_t i2c_FS_SCL_LCNT;
	__IM  uint32_t  RESERVED1[2];
	__IOM uint32_t i2c_INTR_STAT;
	
	__IOM uint32_t i2c_INTR_EN;
	__IOM uint32_t i2c_RAW_INTR_STAT;
	__IOM uint32_t i2c_RX_TL;
	__IOM uint32_t i2c_TX_TL;
	
	__IM uint32_t i2c_CLR_INTR;
	__IM uint32_t i2c_CLR_RX_UNDER;
	__IM uint32_t i2c_CLR_RX_OVER;
	__IM uint32_t i2c_CLR_TX_OVER;
	
	__IM uint32_t i2c_CLR_RD_REQ;
	__IM uint32_t i2c_CLR_TX_ABRT;
	__IM uint32_t i2c_CLR_RX_DONE;
	__IM uint32_t i2c_CLR_ACTIVITY;
	
	__IM uint32_t i2c_CLR_STOP_DET;
	__IM uint32_t i2c_CLR_START_DET;
	__IM uint32_t i2c_CLR_GEN_CALL;
	__IOM uint32_t i2c_ENABLE;
	
	__IM uint32_t i2c_STATUS;
	__IM uint32_t i2c_TXFLR;
	__IM uint32_t i2c_RXFLR;
	__IOM uint32_t i2c_SDA_HOLD;
	
	__IOM uint32_t i2c_TX_ABRT_SOURCE;
	
} XINC_I2C_Type; 


/* =========================================================================================================================== */
/* ================                                           SAADC                                           ================ */
/* =========================================================================================================================== */


/**
  * @brief Successive approximation register (SAR) analog-to-digital converter (SAADC)
  */

typedef struct {                          /*!< (@ 0x40018000) SAADC Structure                                            */
  __IOM  uint32_t  MAIN_CTL;                  /*!< (@ 0x00000000) Starts the SAADC and prepares the result buffer                                         */
  __IOM  uint32_t  CHAN_CTL;                  /*!< (@ 0x00000004) Takes one SAADC sample                                     */
  __IOM  uint32_t  FIFO_CTL;                  /*!< (@ 0x00000008) Stops the SAADC and terminates all on-going conversions    */
  __IOM  uint32_t  TIMER0;                    /*!< (@ 0x0000000C) Starts offset auto-calibration                             */
  __IOM  uint32_t  TIMER1 ;                   /*!< (@ 0x00000010) Starts offset auto-calibration                             */
  __IOM  uint32_t  INT ;                      /*!< (@ 0x00000014) The SAADC has started                                      */
  __IM   uint32_t  INT_RAW;                   /*!< (@ 0x00000018) The SAADC has filled up the result buffer                  */
  __IOM  uint32_t  INT_EN;                    /*!< (@ 0x0000001C) A conversion task has been completed. Depending            */
  __IM   uint32_t  FIFO;                      /*!< (@ 0x00000020) Result ready for transfer to RAM                                   */
  __IOM  uint32_t  RF_CTL;                    /*!< (@ 0x00000054) Calibration is complete                                    */

} XINC_SAADC_Type;  


/* =========================================================================================================================== */
/* ================                                           RTC0                                            ================ */
/* =========================================================================================================================== */
/**
  * @brief Real time counter 0 (RTC0)
  */

typedef struct {              /*!< (@ 0x4000B000) RTC0 Structure                                                                                  */
  __IM   uint32_t  CCVR;                     /*!< (@ 0x00000000) current count value register                                                     */
  __IOM  uint32_t  CLR;                      /*!< (@ 0x00000004) Counter initial value setting register                                           */
  __IOM  uint32_t  CMR_ONE;                  /*!< (@ 0x00000008) Timing Match Register 1                                                          */
  __IOM  uint32_t  CMR_TWO;                  /*!< (@ 0x0000000C) Timing Match Register 2                                                          */
  __IOM  uint32_t  CMR_THREE;				 /*!< (@ 0x00000010) Timing Match Register 3                                                          */
  __IOM  uint32_t  ICR;                      /*!< (@ 0x00000014) Interrupt Control Register                                                       */
  __IOM  uint32_t  ISR_EOI;                  /*!< (@ 0x00000018) The interrupt status register and the interrupt clear register share one         */
  __IM   uint32_t  WVR	;                    /*!< (@ 0x0000001C) Current Week Counter Value Register                                              */
  __IOM  uint32_t  WLR;                      /*!< (@ 0x00000020) Week counter initial value setting register                                      */
  __IOM  uint32_t  RAW_LIMIT;                /*!< (@ 0x00000024) Counting frequency setting register                                              */
  __IOM  uint32_t  SECOND_LIMIT;             /*!< (@ 0x00000028) Second count upper limit control register                                        */
  __IOM  uint32_t  MINUTE_LIMIT;             /*!< (@ 0x0000002C) Minute count upper limit control register                                        */
  __IOM  uint32_t  HOUR_LIMIT;               /*!< (@ 0x00000030) hour count upper limit control register                                          */
  __IM   uint32_t  ISR_RAW;                  /*!< (@ 0x00000034) interrupt raw status register                                                    */
  __IM   uint32_t  RVR;                      /*!< (@ 0x00000038) Current second count value register                                              */
	__IOM  uint32_t  RESERVED0;
  __IOM  uint32_t  AO_TIMER_CTL;             /*!< (@ 0x00000040) 16-bit Counter Control Register                                                  */
  __IOM  uint32_t  AO_GPIO_MODE_;             /*!< (@ 0x00000044) GPIO Mode Configuration Register                                                 */
  __IOM  uint32_t  AO_GPIO_CTL;              /*!< (@ 0x00000048) GPIO Control Register                                                            */
  __IOM  uint32_t  ALL_INTR_AO; 			 /*!< (@ 0x0000004C) AO Interrupt Register                                                            			*/													
  __IM   uint32_t  FREQ_32K_TIMER_VAL;       /*!< (@ 0x00000050) FREQ_TIMER value register                                                        */			
  
} XINC_RTC_Type;   
/* =========================================================================================================================== */
/* ================                                          TIMER0                                           ================ */
/* =========================================================================================================================== */


/**
  * @brief Timer/Counter 0 (TIMER0)
  */

typedef struct {                             /*!< (@ 0x40003000) TIMER0 Structure                                              */
  __IOM  uint32_t  TLC;                   /*!< (@ 0x00000000) Load count value into register                             */
  __IOM  uint32_t  TCV;                   /*!< (@ 0x00000004) current count value register                               */
  __IOM  uint32_t  TCR;                   /*!< (@ 0x00000008) control register                                           */
  __IOM  uint32_t  TIC;                   /*!< (@ 0x0000000C) interrupt clear register                                   */
  __IOM  uint32_t  TIS;                   /*!< (@ 0x00000010) Interrupt Status Register                                  */

} XINC_TIMER_Type;  


typedef struct {                             /*!< (@ 0x40003000) TIMER0 Structure                      */                        

  __IOM  uint32_t  GLOBAL_TIS;                    /*!< (@ 0x000000A0) Global Interrupt Status Register                                  */
  __IOM  uint32_t  GLOBAL_TIC;                    /*!< (@ 0x000000A4) Global interrupt clear register                                   */
  __IOM  uint32_t  GLOBAL_RTIS;                   /*!< (@ 0x000000A8) Global Raw Interrupt Status register                              */
	
} XINC_TIMER_GLOBAL_Type; 


/* =========================================================================================================================== */
/* ================                                          CPR                                           ================ */
/* =========================================================================================================================== */

typedef struct {                             /*!< (@ 0x40000000) CPR Structure                      */                        

	__IM   uint32_t  RESERVED0[9];
  __IOM  uint32_t  PCLK_GRCTL;              /*!< (@ 0x00000024) PCLK GR  */   
  __IM   uint32_t  RESERVED1[2];        
	__IOM  uint32_t  UART0_GRCTL;             /*!< (@ 0x00000030) UART0 CLK GR  */ 
  __IOM  uint32_t  UART0_CLK_CTL;           /*!< (@ 0x00000034) UART0 CLK CTL  */ 
	__IOM  uint32_t  UART1_GRCTL;             /*!< (@ 0x00000038) UART1 CLK GR  */ 
  __IOM  uint32_t  UART1_CLK_CTL;           /*!< (@ 0x0000003C) UART1 CLK CTL  */  
  __IOM  uint32_t  BT_CLK_CTL;              /*!< (@ 0x00000040) BT CLK CTL  */ 
  __IM   uint32_t  RESERVED2;     
  __IOM  uint32_t  BT_MODEM_CTL;              /*!< (@ 0x00000048) BT MODEM CTL  */ 
  __IM   uint32_t  RESERVED3;
  __IOM  uint32_t  SSI0_MCLK_CTL;              /*!< (@ 0x00000050) SSI0_MCLK CTL  */ 
  __IOM  uint32_t  SSI1_MCLK_CTL;              /*!< (@ 0x00000054) SSI1_MCLK CTL  */ 
  __IOM  uint32_t  TIMER0_CLK_CTL;             /*!< (@ 0x00000058) TIMER0 CLK CTL  */
  __IOM  uint32_t  TIMER1_CLK_CTL;             /*!< (@ 0x0000005C) TIMER1 CLK CTL  */ 
  __IOM  uint32_t  TIMER2_CLK_CTL;             /*!< (@ 0x00000060) TIMER2 CLK CTL  */
  __IOM  uint32_t  TIMER3_CLK_CTL;             /*!< (@ 0x00000064) TIMER2 CLK CTL  */
  __IOM  uint32_t  PWM_CLK_CTL;                /*!< (@ 0x00000068) PWM CLK CTL  */  
  __IOM  uint32_t  AHBCLKEN_GRCTL;             /*!< (@ 0x0000006C) AHBCLKEN_GRCTL  */
  __IOM  uint32_t  CTLAPBCLKEN_GRCTL;          /*!< (@ 0x00000070) CTLAPBCLKEN_GRCTL  */ 
  __IOM  uint32_t  OTHERCLKEN_GRCTL;           /*!< (@ 0x00000074) OTHERCLKEN_GRCTL  */ 
  __IOM  uint32_t  I2C_CLK_CTL;                /*!< (@ 0x00000078) I2C_CLK_CTL  */ 
  __IM   uint32_t  RESERVED4[17];
  __IOM  uint32_t  GPIO_FUN_SEL0;                /*!< (@ 0x000000C0) GPIO_FUNSEL0  */ 
  __IOM  uint32_t  GPIO_FUN_SEL1;                /*!< (@ 0x000000C4) GPIO_FUNSEL1  */ 
  __IOM  uint32_t  GPIO_FUN_SEL2;                /*!< (@ 0x000000C8) GPIO_FUNSEL2  */ 
  __IOM  uint32_t  GPIO_FUN_SEL3;                /*!< (@ 0x000000CC) GPIO_FUNSEL3  */ 
  __IOM  uint32_t  GPIO_FUN_SEL4;                /*!< (@ 0x000000D0) GPIO_FUNSEL4  */ 
  __IOM  uint32_t  GPIO_FUN_SEL5;                /*!< (@ 0x000000D4) GPIO_FUNSEL5  */ 
  __IOM  uint32_t  GPIO_FUN_SEL6;                /*!< (@ 0x000000D8) GPIO_FUNSEL6  */ 
  __IOM  uint32_t  GPIO_FUN_SEL7;                /*!< (@ 0x000000DC) GPIO_FUNSEL7  */ 
  __IM   uint32_t  RESERVED5[8];
  __IOM  uint32_t  RSTCTL_CTLAPB_SW;             /*!< (@ 0x00000100) RSTCTL_CTLAPB_SW  */ 
  __IOM  uint32_t  RSTCTL_SUBRST_SW;             /*!< (@ 0x00000104) RSTCTL_SUBRST_SW  */ 

  __IOM  uint32_t  RSTCTL_M0RST_SW;             /*!< (@ 0x00000108) RSTCTL_M0RST_SW  */ 
  __IOM  uint32_t  RSTCTL_M0RST_MASK;             /*!< (@ 0x0000010C) RSTCTL_M0RST_MASK  */  
  __IM   uint32_t  RESERVED6;
  __IOM  uint32_t  RSTCTL_WDTRST_MASK;             /*!< (@ 0x00000114) RSTCTL_WDTRST_MASK  */ 
  __IM   uint32_t  RESERVED7[18];
  __IOM  uint32_t  SSI_CTRL;                    /*!< (@ 0x00000160) SSI_CTRL  */
} XINC_CPR_CTL_Type; 



/* =========================================================================================================================== */
/* ================                                           PWM0                                            ================ */
/* =========================================================================================================================== */


/**
  * @brief Pulse width modulation unit 0 (PWM0)
  */

typedef struct {                                /*!< (@ 0x40017000UL) PWM Structure                 */
	__IOM  uint32_t  EN;                   /*!< (@ 0x00000000) PWM EN Register                                  */
	__IOM  uint32_t  UP;                   /*!< (@ 0x00000004) PWM UPDATE Register                                  */
	__IOM  uint32_t  RST;                   /*!< (@ 0x00000008) PWM RST  Register                                  */
	__IOM  uint32_t  PERIOD;                   /*!< (@ 0x00000010) PWM Period Register                                  */
	__IOM  uint32_t  OCPY;                   /*!< (@ 0x00000010) PWM OCPY Register                                  */
	__IOM  uint32_t  PWMCOMPEN;                   /*!< (@ 0x0000001C) PWM  COMP EN Register                                  */
	__IOM  uint32_t  PWMCOMPTIME;                   /*!< (@ 0x00000018) PWM OCPYCOMP TIME Register              */
} XINC_PWM_Type;                                 /*!< Size =  ()                                                       */


/* =========================================================================================================================== */
/* ================                                            WDT                                            ================ */
/* =========================================================================================================================== */


/**
  * @brief Watchdog Timer (WDT)
  */

typedef struct {                  /*!< (@ 0x40010000) WDT Structure                                              */
  __IOM  uint32_t  CR;                          /*!< (@ 0x00000000) control register                             */
  __IOM  uint32_t  TORR;                        /*!< (@ 0x00000004) Timeout Range Register                       */
  __IOM  uint32_t  CCVR;                        /*!< (@ 0x00000008) current count value register                 */
  __OM   uint32_t  CRR;                         /*!< (@ 0x0000000c) Counter restart register                     */
  __IM   uint32_t  STAT;                        /*!< (@ 0x00000010) Interrupt Status Register                    */
  __IM   uint32_t  ICR;                         /*!< (@ 0x00000014) interrupt clear register                     */
 
} XINC_WDT_Type;                                 /*!< Size = ?                                                    */

/** @addtogroup Device_Peripheral_peripheralAddr
  * @{
  */
#define NRF_P0_BASE                 0x40001000UL
#define XINC_CPR_BASE               0x40000000UL
#define XINC_CPR_AO_BASE            0x40002400UL
#define XINC_GPIOTE_BASE             0x40006000UL
#define XINC_UART0_BASE              0x40010000UL
#define XINC_UART1_BASE              0x40011000UL
#define XINC_SPIM0_BASE              0x40013000UL
#define XINC_SPIM1_BASE              0x40014000UL
#define XINC_I2C0_BASE               0x40006000UL
#define XINC_SAADC_BASE              0x40018000UL
#define XINC_RTC0_BASE               0x40002000UL
#define XINC_WDT_BASE                0x40004000UL
#define XINC_TIMER0_BASE             0x40003000UL
#define XINC_TIMER1_BASE             0x40003014UL
#define XINC_TIMER2_BASE             0x40003028UL
#define XINC_TIMER3_BASE             0x4000303CUL
#define XINC_TIMER_GLOBAL_BASE       0x400030A0UL
#define XINC_TIMER0_CLKCTL_BASE       0x40000058UL
#define XINC_TIMER1_CLKCTL_BASE       0x4000005CUL
#define XINC_TIMER2_CLKCTL_BASE       0x40000060UL
#define XINC_TIMER3_CLKCTL_BASE       0x40000064UL

#define	XINC_PWM0_BASE				0x40017000UL
#define	XINC_PWM1_BASE				0x40017040UL
#define	XINC_PWM2_BASE				0x40017400UL
#define	XINC_PWM3_BASE				0x40017440UL
#define	XINC_PWM4_BASE				0x40017800UL
#define	XINC_PWM5_BASE				0x40017840UL
/* =========================================================================================================================== */
/* ================                                  Peripheral declaration                                   ================ */
/* =========================================================================================================================== */


/** @addtogroup Device_Peripheral_declaration
  * @{
  */
#define NRF_P0                       ((NRF_GPIO_Type*)            NRF_P0_BASE)
#define XINC_CPR                     ((XINC_CPR_CTL_Type*)    XINC_CPR_BASE)

#define NRF_CPR_AO               		 ((XINC_CPRA_AO_Type*)        XINC_CPR_AO_BASE)
#define NRF_GPIOTE                   ((NRF_GPIOTE_Type*)          XINC_GPIOTE_BASE)
#define NRF_UART0                    ((NRF_UART_Type*)            XINC_UART0_BASE)
#define NRF_UART1                    ((NRF_UART_Type*)            XINC_UART1_BASE)
#define XINC_SPIM0										 ((XINC_SPIM_Type*)         XINC_SPIM0_BASE)
#define XINC_SPIM1										 ((XINC_SPIM_Type*)         XINC_SPIM1_BASE)

#define XINC_I2C0										 ((XINC_I2C_Type*)           XINC_I2C0_BASE)
#define XINC_SAADC                   ((XINC_SAADC_Type*)          XINC_SAADC_BASE)
#define XINC_RTC0                    ((XINC_RTC_Type*)           XINC_RTC0_BASE)
#define XINC_WDT                     ((XINC_WDT_Type*)           XINC_WDT_BASE)
#define XINC_TIMER0                  ((XINC_TIMER_Type*)         XINC_TIMER0_BASE)
#define XINC_TIMER1                  ((XINC_TIMER_Type*)         XINC_TIMER1_BASE)
#define XINC_TIMER2                  ((XINC_TIMER_Type*)         XINC_TIMER2_BASE)
#define XINC_TIMER3                  ((XINC_TIMER_Type*)         XINC_TIMER3_BASE)
#define XINC_TIMER_GLOBAL            ((XINC_TIMER_GLOBAL_Type*)  XINC_TIMER_GLOBAL_BASE)
#define NRF_CLKCTL_TIMER0            ((XINC_TIMER_CLKCTL_Type*)  XINC_TIMER0_CLKCTL_BASE)
#define NRF_CLKCTL_TIMER1            ((XINC_TIMER_CLKCTL_Type*)  XINC_TIMER1_CLKCTL_BASE)
#define NRF_CLKCTL_TIMER2            ((XINC_TIMER_CLKCTL_Type*)  XINC_TIMER2_CLKCTL_BASE)
#define NRF_CLKCTL_TIMER3            ((XINC_TIMER_CLKCTL_Type*)  XINC_TIMER3_CLKCTL_BASE)


#define XINC_PWM0                    ((XINC_PWM_Type*)           XINC_PWM0_BASE)
#define XINC_PWM1                    ((XINC_PWM_Type*)           XINC_PWM1_BASE)
#define XINC_PWM2                    ((XINC_PWM_Type*)           XINC_PWM2_BASE)
#define XINC_PWM3                    ((XINC_PWM_Type*)           XINC_PWM3_BASE)
#define XINC_PWM4                    ((XINC_PWM_Type*)           XINC_PWM4_BASE)
#define XINC_PWM5                    ((XINC_PWM_Type*)           XINC_PWM5_BASE)
#ifdef __cplusplus
}
#endif

#endif /* NRF52840_H */


/** @} */ /* End of group nrf52840 */

/** @} */ /* End of group Nordic Semiconductor */
