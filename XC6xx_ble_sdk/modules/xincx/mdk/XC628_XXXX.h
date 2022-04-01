/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
/** @addtogroup XinChip
  * @{
  */


/** @addtogroup xc628_XX
  * @{
  */


#ifndef XINC628_XX_H
#define XINC628_XX_H

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

#include "xinc_m4.h"


 
/* =========================================================================================================================== */
/* ================                                           DMAS                                            ================ */
/* =========================================================================================================================== */


/**
  * @brief 精简直接存储器存取控制器（DMAS） (DMAS)
  */

typedef struct {                                /*!< (@ 0x50001000) DMAS Structure                 */
    __OM    uint32_t  DMAs_EN;                   /*!< (@ 0x50001000) DMA 通道使能寄存器              */
    __OM    uint32_t  DMAs_CLR;                 /*!< (@ 0x50001004) DMA 通道清除寄存器         */
    __IM    uint32_t  DMAs_STA;                /*!< (@ 0x50001008) DMA 通道清除寄存器      */
    __IM    uint32_t  DMAs_INT_RAW;            /*!< (@ 0x5000100C) DMA 中断原始状态寄存器      */

    __IOM   uint32_t  DMAs_INT_EN0;      /*!< (@ 0x50001010) DMA 中断使能寄存器 0  */
    __IM    uint32_t  RESERVED0[2];
    __IOM   uint32_t  DMAs_INT0;      /*!< (@ 0x5000101C) DMA 中断状态寄存器 0  */                      

    __IM    uint32_t  RESERVED1[2];
    __OM    uint32_t  DMAs_INT_CLR; /*!< (@ 0x50001028) DMA 中断清除寄存器 */ 
    __IOM   uint32_t  DMAs_INTV_UNIT; /*!< (@ 0x5000102C) DMA 定时单位寄存器 */  

    __IM    uint32_t  RESERVED2[4];

    __IOM   uint32_t  DMAs_CH0_SAR;   /*!< (@ 0x50001040) 发送通道 0 源起始地址 */ 
    __IOM   uint32_t  DMAs_CH0_DAR;   /*!< (@ 0x50001044) 发送通道 0 目的起始地址 */ 
    __IOM   uint32_t  DMAs_CH0_CTL0;   /*!< (@ 0x50001048) 发送通道 0 控制寄存器 0 */ 
    __IOM   uint32_t  DMAs_CH0_CTL1;   /*!< (@ 0x5000104C) 发送通道 0 控制寄存器 1*/ 

    __IM    uint32_t  DMAs_CH0_CA;   /*!< (@ 0x50001050) 发送通道 0 当前源地址寄存器 */ 
    __IM    uint32_t  RESERVED3[3];

    __IOM   uint32_t  DMAs_CH1_SAR;   /*!< (@ 0x50001060) 发送通道 1 源起始地址 */ 
    __IOM   uint32_t  DMAs_CH1_DAR;   /*!< (@ 0x50001064) 发送通道 1 目的起始地址 */ 
    __IOM   uint32_t  DMAs_CH1_CTL0;   /*!< (@ 0x50001068) 发送通道 1 控制寄存器 0 */ 
    __IOM   uint32_t  DMAs_CH1_CTL1;  /*!< (@ 0x5000106C) 发送通道 1 控制寄存器 1*/ 

    __IM    uint32_t  DMAs_CH1_CA;   /*!< (@ 0x50001070) 发送通道 1 当前源地址寄存器 */ 
    __IM    uint32_t  RESERVED4[3];

    __IOM   uint32_t  DMAs_CH2_SAR;   /*!< (@ 0x50001080) 发送通道 2 源起始地址 */ 
    __IOM   uint32_t  DMAs_CH2_DAR;   /*!< (@ 0x50001084) 发送通道 2 目的起始地址 */ 
    __IOM   uint32_t  DMAs_CH2_CTL0;   /*!< (@ 0x50001088) 发送通道 2 控制寄存器 0 */ 
    __IOM   uint32_t  DMAs_CH2_CTL1;  /*!< (@ 0x5000108C) 发送通道 2 控制寄存器 1*/ 

    __IM    uint32_t  DMAs_CH2_CA;   /*!< (@ 0x50001090) 发送通道 2 当前源地址寄存器 */ 
    __IM    uint32_t  RESERVED5[3];

    __IOM   uint32_t  DMAs_CH3_SAR;   /*!< (@ 0x500010A0) 发送通道 3 源起始地址 */ 
    __IOM   uint32_t  DMAs_CH3_DAR;   /*!< (@ 0x500010A4) 发送通道 3 目的起始地址 */ 
    __IOM   uint32_t  DMAs_CH3_CTL0;   /*!< (@ 0x500010A8) 发送通道 3 控制寄存器 0 */ 
    __IOM   uint32_t  DMAs_CH3_CTL1;  /*!< (@ 0x500010AC) 发送通道 3 控制寄存器 1*/ 

    __IM    uint32_t  DMAs_CH3_CA;   /*!< (@ 0x500010B0) 发送通道 3 当前源地址寄存器 */ 
    __IM    uint32_t  RESERVED6[3];

    __IM    uint32_t  RESERVED7[32];   //(@ 0x500010C0) -->/(@ 0x5000113C)


    __IOM   uint32_t  DMAs_CH8_SAR;   /*!< (@ 0x50001140) 接收通道 8 源起始地址 */ 
    __IOM   uint32_t  DMAs_CH8_DAR;   /*!< (@ 0x50001144) 接收通道 8 目的起始地址 */ 
    __IOM   uint32_t  DMAs_CH8_CTL0;   /*!< (@ 0x50001148) 接收通道 8 控制寄存器 0 */ 
    __IOM   uint32_t  DMAs_CH8_CTL1;  /*!< (@ 0x5000114C) 接收通道 8 控制寄存器 1*/ 

    __IM    uint32_t  DMAs_CH8_CA;   /*!< (@ 0x50001150) 接收通道 8 当前源地址寄存器 */ 
    __IM    uint32_t  RESERVED8[3];

    __IOM   uint32_t  DMAs_CH9_SAR;   /*!< (@ 0x50001160) 接收通道 9 源起始地址 */ 
    __IOM   uint32_t  DMAs_CH9_DAR;   /*!< (@ 0x50001164) 接收通道 9 目的起始地址 */ 
    __IOM   uint32_t  DMAs_CH9_CTL0;   /*!< (@ 0x50001168) 接收通道 9 控制寄存器 0 */ 
    __IOM   uint32_t  DMAs_CH9_CTL1;  /*!< (@ 0x5000116C) 接收通道 9 控制寄存器 1*/ 

    __IM    uint32_t  DMAs_CH9_CA;   /*!< (@ 0x50001170) 接收通道 9 当前源地址寄存器 */ 
    __IM    uint32_t  RESERVED9[3];

    __IOM   uint32_t  DMAs_CH10_SAR;   /*!< (@ 0x50001180) 接收通道 10 源起始地址 */ 
    __IOM   uint32_t  DMAs_CH10_DAR;   /*!< (@ 0x50001184) 接收通道 10 目的起始地址 */ 
    __IOM   uint32_t  DMAs_CH10_CTL0;   /*!< (@ 0x50001188) 接收通道 10 控制寄存器 0 */ 
    __IOM   uint32_t  DMAs_CH10_CTL1;  /*!< (@ 0x5000118C) 接收通道 10 控制寄存器 1*/ 

    __IM    uint32_t  DMAs_CH10_CA;   /*!< (@ 0x50001190) 接收通道 10 当前源地址寄存器 */ 
    __IM    uint32_t  RESERVED10[3];

    __IOM   uint32_t  DMAs_CH11_SAR;   /*!< (@ 0x500011A0) 接收通道 11 源起始地址 */ 
    __IOM   uint32_t  DMAs_CH11_DAR;   /*!< (@ 0x500011A4) 接收通道 11 目的起始地址 */ 
    __IOM   uint32_t  DMAs_CH11_CTL0;   /*!< (@ 0x500011A8) 接收通道 11 控制寄存器 0 */ 
    __IOM   uint32_t  DMAs_CH11_CTL1;  /*!< (@ 0x500011AC) 接收通道 11 控制寄存器 1*/ 

    __IM    uint32_t  DMAs_CH11_CA;   /*!< (@ 0x500011B0) 接收通道 11 当前源地址寄存器 */ 
    __IM    uint32_t  RESERVED11[3];

    __IOM   uint32_t  DMAs_CH12_SAR;   /*!< (@ 0x500011C0) 接收通道 12 源起始地址 */ 
    __IOM   uint32_t  DMAs_CH12_DAR;   /*!< (@ 0x500011C4) 接收通道 12 目的起始地址 */ 
    __IOM   uint32_t  DMAs_CH12_CTL0;   /*!< (@ 0x500011C8) 接收通道 12 控制寄存器 0 */ 
    __IOM   uint32_t  DMAs_CH12_CTL1;  /*!< (@ 0x500011CC) 接收通道 12 控制寄存器 1*/ 

    __IM    uint32_t  DMAs_CH12_CA;   /*!< (@ 0x500011D0) 接收通道 12 当前源地址寄存器 */ 
    __IM    uint32_t  RESERVED12[3];

    __IM    uint32_t  RESERVED13[24];//(@ 0x500011E0) -->/(@ 0x5000123C)

    __IOM   uint32_t  DMAs_CH0_WD; /*!< (@ 0x50001240) 发送通道 0 缺省写入数据 */
    __IOM   uint32_t  DMAs_CH1_WD; /*!< (@ 0x50001244) 发送通道 0 缺省写入数据 */
    __IOM   uint32_t  DMAs_CH2_WD; /*!< (@ 0x50001248) 发送通道 0 缺省写入数据 */
    __IOM   uint32_t  DMAs_CH3_WD; /*!< (@ 0x5000124C) 发送通道 0 缺省写入数据 */

    __IM    uint32_t  RESERVED14[107];//(@ 0x50001250) -->/(@ 0x500013F8)
    __IOM   uint32_t  DMAs_LP_CTL; /*!< (@ 0x500013FC) DMA 低功耗控制寄存器 */  
                    
} XINC_DMAS_Type;                                /*!< Size = 1024 (0x400)                                                       */



/**
  * @brief GPIO Port 1 (P0)
  */

typedef struct {                                /*!< (@ 0x40001000) P0 Structure                                               */
    __IOM uint32_t  PORT_DR[2];   										/*!< (@ 0x40001000) GPIO_PORT_DRx */
    __IM  uint32_t  RESERVED[6];
    __IOM uint32_t  PORT_DDR[2];   										/*!< (@ 0x40001020) GPIO_PORT_DATA DRx */
    __IM  uint32_t  RESERVED1[6];
    __IOM uint32_t  EXT_PORT[1];                    /*!< (@ 0x40001040) Read GPIO port                                            */
    __IM  uint32_t  RESERVED2[47];
    __IOM uint32_t  INTR_CTRL[8];									/*!< (@ 0x40001100) Intr ctr GPIO port                                         */
    __IM  uint32_t  RESERVED3[24];
    __IOM uint32_t  DEBOUNCE[2];                   /*!< (@ 0x40001180) Set individual bits in GPIO port                           */
    __IM  uint32_t  RESERVED4[6];
    __IM uint32_t   INTR_RAW[1];                      /*!< (@ 0x400011A0) Intr RAW                         */
    __IM  uint32_t  RESERVED5[3];
    __IOM uint32_t  INTR_CLR[1];                     /*!< (@ 0x400011B0) Intr Clear                                      */
    __IM  uint32_t  RESERVED6[19];
    __IOM uint32_t  INTR_MASK_C0[2];                       /*!< (@ 0x40001200) DIR set register                                           */
    __IM  uint32_t  RESERVED7[6];
    __IM uint32_t   INTR_STATUS_C0[1];                       /*!< (@ 0x40001220) 中断状态寄存器                                         */
                                                    
} XINC_GPIO_Type;//XINC_GPIO_Type;    /*!< Size = 548 (0x224)   */


                           
/* =========================================================================================================================== */
/* ================                          Device Specific Peripheral Address Map                           ================ */
/* =========================================================================================================================== */



typedef union
{
    __IM      uint32_t      RBR;
    __OM      uint32_t      THR;
    __IOM     uint32_t      DLL;
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
    __IM  uint32_t  RESERVED4[24];
    __IM  uint32_t  USR;

} XINC_UART_Type;  


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
    __IM   uint32_t  RESERVED0;
    __IOM  uint32_t  SE;
    __IOM  uint32_t  BAUD;
    __IOM  uint32_t  TXFLT;
    __IOM  uint32_t  RXFLT;
    __IM   uint32_t  TXFL;
    __IM   uint32_t  RXFL;
    __IM   uint32_t  STS;
    __IOM  uint32_t  IE;
    __IM   uint32_t  IS;
    __IM   uint32_t  RIS;
    __IM   uint32_t  TXOIC;
    __IM   uint32_t  RXOIC;
    __IM   uint32_t  RXUIC;
    __IM   uint32_t  RESERVED1;
    __IM   uint32_t  IC;
    __IOM  uint32_t DMAS;
    __IOM  uint32_t DMATDL;
    __IOM  uint32_t DMARDL;
    __IM   uint32_t  RESERVED2[2];
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

    __IM  uint32_t i2c_CLR_INTR;
    __IM  uint32_t i2c_CLR_RX_UNDER;
    __IM  uint32_t i2c_CLR_RX_OVER;
    __IM  uint32_t i2c_CLR_TX_OVER;

    __IM  uint32_t i2c_CLR_RD_REQ;
    __IM  uint32_t i2c_CLR_TX_ABRT;
    __IM  uint32_t i2c_CLR_RX_DONE;
    __IM  uint32_t i2c_CLR_ACTIVITY;

    __IM  uint32_t i2c_CLR_STOP_DET;
    __IM  uint32_t i2c_CLR_START_DET;
    __IM  uint32_t i2c_CLR_GEN_CALL;
    __IOM uint32_t i2c_ENABLE;

    __IM  uint32_t i2c_STATUS;
    __IM  uint32_t i2c_TXFLR;
    __IM  uint32_t i2c_RXFLR;
    __IOM uint32_t i2c_SDA_HOLD;

    __IOM uint32_t i2c_TX_ABRT_SOURCE;
	
} XINC_I2C_Type; 


/* =========================================================================================================================== */
/* ================                                           SAADC                                           ================ */
/* =========================================================================================================================== */


/**
  * @brief Successive approximation register (SAR) analog-to-digital converter (SAADC)
  */

typedef struct {                                /*!< (@ 0x40018000) SAADC Structure */
    __IOM  uint32_t  MAIN_CTL;                  /*!< (@ 0x00000000)                 */
    __IOM  uint32_t  CHAN_CTL;                  /*!< (@ 0x00000004)                 */
    __IOM  uint32_t  FIFO_CTL;                  /*!< (@ 0x00000008)                 */
    __IOM  uint32_t  TIMER0;                    /*!< (@ 0x0000000C)                 */
    __IOM  uint32_t  TIMER1 ;                   /*!< (@ 0x00000010)                 */
    __IOM  uint32_t  INT ;                      /*!< (@ 0x00000014)                 */
    __IM   uint32_t  INT_RAW;                   /*!< (@ 0x00000018)                 */
    __IOM  uint32_t  INT_EN;                    /*!< (@ 0x0000001C)                 */
    __IM   uint32_t  FIFO;                      /*!< (@ 0x00000020)                 */
    __IOM  uint32_t  RF_CTL;                    /*!< (@ 0x00000024)                 */

} XINC_SAADC_Type;  


/* =========================================================================================================================== */
/* ================                                           RTC0                                            ================ */
/* =========================================================================================================================== */
/**
  * @brief Real time counter 0 (RTC0)
  */

typedef struct {              /*!< (@ 0x4000B000) RTC0 Structure                                            */
    __IM   uint32_t  CCVR;                      /*!< (@ 0x00000000) current count value register             */
    __IOM  uint32_t  CLR;                       /*!< (@ 0x00000004) Counter initial value setting register   */
    __IOM  uint32_t  CMR_ONE;                   /*!< (@ 0x00000008) Timing Match Register 1                  */
    __IOM  uint32_t  CMR_TWO;                   /*!< (@ 0x0000000C) Timing Match Register 2                  */
    __IOM  uint32_t  CMR_THREE;                 /*!< (@ 0x00000010) Timing Match Register 3                    */
    __IOM  uint32_t  ICR;                       /*!< (@ 0x00000014) Interrupt Control Register               */
    __IOM  uint32_t  ISR_EOI;                   /*!< (@ 0x00000018) The interrupt status register and the interrupt clear register share one         */
    __IM   uint32_t  WVR;                       /*!< (@ 0x0000001C) Current Week Counter Value Register    */
    __IOM  uint32_t  WLR;                       /*!< (@ 0x00000020) Week counter initial value setting register  */
    __IOM  uint32_t  RAW_LIMIT;                 /*!< (@ 0x00000024) Counting frequency setting register          */
    __IOM  uint32_t  SECOND_LIMIT;              /*!< (@ 0x00000028) Second count upper limit control register    */
    __IOM  uint32_t  MINUTE_LIMIT;              /*!< (@ 0x0000002C) Minute count upper limit control register    */
    __IOM  uint32_t  HOUR_LIMIT;                /*!< (@ 0x00000030) hour count upper limit control register      */
    __IM   uint32_t  ISR_RAW;                   /*!< (@ 0x00000034) interrupt raw status register                */
    __IM   uint32_t  RVR;                       /*!< (@ 0x00000038) Current second count value register          */
    __IOM  uint32_t  RESERVED0;
    __IOM  uint32_t  AO_TIMER_CTL;              /*!< (@ 0x00000040) 16-bit Counter Control Register             */
    __IOM  uint32_t  AO_GPIO_MODE;              /*!< (@ 0x00000044) GPIO Mode Configuration Register            */
    __IOM  uint32_t  AO_GPIO_CTL;               /*!< (@ 0x00000048) GPIO Control Register                       */
    __IOM  uint32_t  ALL_INTR_AO;               /*!< (@ 0x0000004C) AO Interrupt Register                       */													
    __IM   uint32_t  FREQ_32K_TIMER_VAL;        /*!< (@ 0x00000050) FREQ_TIMER value register                   */			
  
} XINC_RTC_Type;   
/* =========================================================================================================================== */
/* ================                                          TIMER0                                           ================ */
/* =========================================================================================================================== */


/**
  * @brief Timer/Counter 0 (TIMER0)
  */

typedef struct {                            /*!< (@ 0x40003000) TIMER0 Structure                                            */
    __IOM  uint32_t  TLC;                   /*!< (@ 0x00000000) Load count value into register                              */
    __IOM  uint32_t  TCV;                   /*!< (@ 0x00000004) current count value register                                */
    __IOM  uint32_t  TCR;                   /*!< (@ 0x00000008) control register                                            */
    __IOM  uint32_t  TIC;                   /*!< (@ 0x0000000C) interrupt clear register                                    */
    __IOM  uint32_t  TIS;                   /*!< (@ 0x00000010) Interrupt Status Register                                   */

} XINC_TIMER_Type;  


typedef struct {                            /*!< (@ 0x40003000) TIMER0 Structure                                            */                        

    __IOM  uint32_t  GLOBAL_TIS;            /*!< (@ 0x000000A0) Global Interrupt Status Register                            */
    __IOM  uint32_t  GLOBAL_TIC;            /*!< (@ 0x000000A4) Global interrupt clear register                             */
    __IOM  uint32_t  GLOBAL_RTIS;           /*!< (@ 0x000000A8) Global Raw Interrupt Status register                        */
	
} XINC_TIMER_GLOBAL_Type; 


/* =========================================================================================================================== */
/* ================                                          CPR                                           ================ */
/* =========================================================================================================================== */

typedef struct {                                /*!< (@ 0x40000000) CPR PD 域寄存器地址映射 Structure       */                        
    __IOM  uint32_t  SLP_SRC_MASK;              /*!< (@ 0x40000000) 睡眠源屏蔽寄存器       */ 
    __IM   uint32_t  RESERVED[3];

    __IOM  uint32_t  SLP_CNT_LIMIT;             /*!< (@ 0x40000010) 睡眠进入阈值寄存器       */ 
    __IOM  uint32_t  SLP_ST;                    /*!< (@ 0x40000014) 睡眠启动寄存器       */
    __IOM  uint32_t  SLP_FSM_STATE;             /*!< (@ 0x40000018) 睡眠状态机状态寄存器       */
    __IM   uint32_t  RESERVED01[1];

    __IOM  uint32_t  M0_FCLK_CTL;               /*!< (@ 0x40000020) M0 fclk 时钟控制寄存器       */
    __IOM  uint32_t  CTL_PCLK_GRCTL;            /*!< (@ 0x00000024) CTL_PCLK 时钟控制寄存器 GR  */   
    __IOM  uint32_t  SIM_CLK_GRCTL;             /*!< (@ 0x00000028) SIM_CLK时钟GR控制寄存器  */ 
    __IOM  uint32_t  SIM_CLK_CTLL;              /*!< (@ 0x0000002C) SIM_CLK 时钟控制寄存器  */ 

    __IOM  uint32_t  UART0_CLK_GRCTL;           /*!< (@ 0x00000030) UART0 CLK GR  */ 
    __IOM  uint32_t  UART0_CLK_CTL;             /*!< (@ 0x00000034) UART0 CLK CTL  */ 
    __IOM  uint32_t  UART1_CLK_GRCTL;           /*!< (@ 0x00000038) UART1 CLK GR  */ 
    __IOM  uint32_t  UART1_CLK_CTL;             /*!< (@ 0x0000003C) UART1 CLK CTL  */  
    
    __IOM  uint32_t  BT_CLK_CTL;                /*!< (@ 0x00000040) BT_CLK 时钟控制寄存器  */ 
    __IM   uint32_t  RESERVED02;     
    __IOM  uint32_t  BT_MODEM_CTL;              /*!< (@ 0x00000048) BT_MODEM_CLK时钟控制寄存器  */ 
    __IOM  uint32_t  QDEC_CLK_CTL;              /*!< (@ 0x0000004C) QDEC 时钟控制寄存器  */ 

    __IOM  uint32_t  SSI0_MCLK_CTL;             /*!< (@ 0x00000050) SSI0_MCLK CTL  */ 
    __IOM  uint32_t  SSI1_MCLK_CTL;             /*!< (@ 0x00000054) SSI1_MCLK CTL  */ 
    __IOM  uint32_t  TIMER0_CLK_CTL;            /*!< (@ 0x00000058) TIMER0 CLK CTL  */
    __IOM  uint32_t  TIMER1_CLK_CTL;            /*!< (@ 0x0000005C) TIMER1 CLK CTL  */ 
   
    __IOM  uint32_t  TIMER2_CLK_CTL;            /*!< (@ 0x00000060) TIMER2 CLK CTL  */
    __IOM  uint32_t  TIMER3_CLK_CTL;            /*!< (@ 0x00000064) TIMER2 CLK CTL  */
    __IOM  uint32_t  PWM_CLK_CTL;               /*!< (@ 0x00000068) PWM CLK CTL    */  
    __IOM  uint32_t  AHBCLKEN_GRCTL;            /*!< (@ 0x0000006C) AHBCLKEN_GRCTL  */
   
    __IOM  uint32_t  CTLAPBCLKEN_GRCTL;         /*!< (@ 0x00000070) CTLAPBCLKEN_GRCTL  */ 
    __IOM  uint32_t  OTHERCLKEN_GRCTL;          /*!< (@ 0x00000074) OTHERCLKEN_GRCTL  */ 
    __IOM  uint32_t  I2C_CLK_CTL;               /*!< (@ 0x00000078) I2C_CLK_CTL  */ 
    __IM   uint32_t  RESERVED03[9];

    __IOM  uint32_t  INT;                       /*!< (@ 0x000000A0) M0 的 CPR 中断状态寄存器  */ 
    __IOM  uint32_t  INT_EN;                    /*!< (@ 0x000000A4) M0 的 CPR 中断状态寄存器  */ 
    __IOM  uint32_t  INT_RAW;                   /*!< (@ 0x000000A8) M0 的 CPR 中断状态寄存器  */ 
    __IM   uint32_t  RESERVED04[5];


    __IOM  uint32_t  GPIO_FUN_SELx[8];             /*!< (@ 0x000000C0) GPIO_FUNSEL0..7  */ 
//    __IOM  uint32_t  GPIO_FUN_SEL1;             /*!< (@ 0x000000C4) GPIO_FUNSEL1  */ 
//    __IOM  uint32_t  GPIO_FUN_SEL2;             /*!< (@ 0x000000C8) GPIO_FUNSEL2  */ 
//    __IOM  uint32_t  GPIO_FUN_SEL3;             /*!< (@ 0x000000CC) GPIO_FUNSEL3  */ 
//   
//    __IOM  uint32_t  GPIO_FUN_SEL4;             /*!< (@ 0x000000D0) GPIO_FUNSEL4  */ 
//    __IOM  uint32_t  GPIO_FUN_SEL5;             /*!< (@ 0x000000D4) GPIO_FUNSEL5  */ 
//    __IOM  uint32_t  GPIO_FUN_SEL6;             /*!< (@ 0x000000D8) GPIO_FUNSEL6  */ 
//    __IOM  uint32_t  GPIO_FUN_SEL7;             /*!< (@ 0x000000DC) GPIO_FUNSEL7  */ 
   
    __IM   uint32_t  RESERVED05[8];
   
    __IOM  uint32_t  RSTCTL_CTLAPB_SW;          /*!< (@ 0x00000100) CTLAPB 模块软复位寄存器  */ 
    __IOM  uint32_t  RSTCTL_SUBRST_SW;          /*!< (@ 0x00000104) SUB 模块软复位寄存器  */ 
    __IOM  uint32_t  RSTCTL_M0RST_SW;           /*!< (@ 0x00000108) M0 相关软复位寄存器  */ 
    __IOM  uint32_t  RSTCTL_M0RST_MASK;         /*!< (@ 0x0000010C) M0 复位屏蔽寄存器  */  
   
    __IOM  uint32_t  RSTCTL_LOCKUP_STATE;       /*!< (@ 0x00000110) M0 死锁状态指示寄存器 */ 
    __IOM  uint32_t  RSTCTL_WDTRST_MASK;        /*!< (@ 0x00000114) WDT 复位屏蔽寄存 */ 
    __IOM  uint32_t  LP_CTL;                    /*!< (@ 0x00000118) 低功耗控制寄存器  */ 
    __IM   uint32_t  RESERVED06[1];

    __IOM  uint32_t  CTL_SHRAM_ICM_PRIORITY;    /*!< (@ 0x00000120) SHRAM 的 ICM 优先级配置寄存器  */ 
    __IOM  uint32_t  CTL_CTLAPB_ICM_PRIORITY;   /*!< (@ 0x00000124) CTL_APB 桥的 ICM优先级配置寄存器  */ 
    __IOM  uint32_t  CTL_MUXCTL1_2[2];               /*!< (@ 0x00000128) MUXPIN 控制寄存器1 2  */ 


    __IOM  uint32_t  CTL_PECTL1_2[2];                /*!< (@ 0x00000130) 管脚上下拉配置寄存器1 2  */ 
    __IM   uint32_t  RESERVED07[1];
    __IOM  uint32_t  REMAP_CTRL;                /*!< (@ 0x0000013C) REMAP 控制寄存器  */ 

    __IOM  uint32_t  CTL_BOOTCTL;               /*!< (@ 0x00000140) BOOTCTL 指示寄存器  */ 
    __IOM  uint32_t  CTL_M0_STCALIB;            /*!< (@ 0x00000144) M0 配置寄存器  */ 
    __IOM  uint32_t  CTL_M0_IRQLATENCY;         /*!< (@ 0x00000148) M0 IRQ 延迟寄存器  */ 
    __IM  uint32_t  RESERVED08[5];

    // SSI 控制寄存器
    __IOM  uint32_t  SSI_CTRL;                  /*!< (@ 0x00000160) SSI 控制寄存器  */
    __IM  uint32_t  RESERVED09[3];

    __IOM  uint32_t  TESTCTRL0;                 /*!< (@ 0x00000170) 测试控制寄存器 0  */
    __IOM  uint32_t  TESTCTRL1;                 /*!< (@ 0x00000174) 测试控制寄存器 1  */
    __IOM  uint32_t  TESTCTRL2;                 /*!< (@ 0x00000178) 测试控制寄存器 2  */
    __IOM  uint32_t  TESTCTRL3;                 /*!< (@ 0x0000017C) 测试控制寄存器 3  */

    __IOM  uint32_t  TESTCTRL4;                 /*!< (@ 0x00000180) 测试控制寄存器 4  */
    __IOM  uint32_t  TESTCTRL5;                 /*!< (@ 0x00000184) 测试控制寄存器 5  */
    __IOM  uint32_t  TESTCTRL6;                 /*!< (@ 0x00000188) 测试控制寄存器 6  */
    __IOM  uint32_t  TESTCTRL7;                 /*!< (@ 0x0000018C) 测试控制寄存器 7  */

    __IOM  uint32_t  TESTCTRL8;                 /*!< (@ 0x00000190) 测试控制寄存器 8  */
    __IOM  uint32_t  TESTCTRL9;                 /*!< (@ 0x00000194) 测试控制寄存器 9  */
    __IM   uint32_t  RESERVED10[1];
    __IOM  uint32_t  CTL_MUXCTL3[1];               /*!< (@ 0x0000019C) MUXPIN 控制寄存器 3  */

    __IOM  uint32_t  CTL_PUCTL;                 /*!< (@ 0x000001A0) 管脚上下拉配置寄存 3  */


} XINC_CPR_CTL_Type; 

typedef struct {                                /*!< (@ 0x40002400) CPR AO Structure       */    
    __IM  uint32_t  RESERVED00[1]; 
    __IOM uint32_t  SLP_CTL;  /*!< (@ 0x00000004) 睡眠控制寄存器     */
    __IOM uint32_t  SLPCTL_INT_MASK;  /*!< (@ 0x00000008) 睡眠唤醒 M0 中断屏蔽寄存器     */
    __IOM uint32_t  SLP_PD_MASK;  /*!< (@ 0x0000000C) 睡眠断电屏蔽寄存器     */

    __IOM uint32_t  MCLK_DIV_CLK_CTL;  /*!< (@ 0x00000010) MCLK_DIV_CLK(32K)分频控制寄存器     */
    __IOM uint32_t  RFPMU_SPI_CTL;  /*!< (@ 0x00000014) SPI 控制寄存器     */
    __IM  uint32_t  RESERVED01[1]; 
    __IOM uint32_t  SYS_TIME;  /*!< (@ 0x0000001C) SPI 控制寄存器     */

    __IM  uint32_t  RESERVED02[5]; 
    __IOM uint32_t  PE_CTRLx[2];  /*!< (@ 0x00000034) CPR AO  GPIO PE Ctrl1..2 */
  
    __IM  uint32_t  RESERVED03[16]; 
    __IOM uint32_t  AOCLKEN_GRCTL;  /*!< (@ 0x0000007C) AO 域时钟使能寄存器     */

    __IM  uint32_t  RESERVED04[51]; 
    __IOM uint32_t  CTL_CLK32K;  /*!< (@ 0x0000014C) 32K 时钟配置寄存器     */

    __IM  uint32_t  RESERVED05[1]; 
    __IOM uint32_t  AO_REG0;  /*!< (@ 0x00000154) AO 通用器 0     */
    __IOM uint32_t  AO_REG1;  /*!< (@ 0x00000158) AO 通用器 0     */
    __IOM uint32_t  AO_CTRL;  /*!< (@ 0x0000015C) AO 控制寄存器     */

}XINC_CPR_AO_CTL_Type;



/* =========================================================================================================================== */
/* ================                                           PWM0                                            ================ */
/* =========================================================================================================================== */


/**
  * @brief Pulse width modulation unit 0 (PWM0)
  */

typedef struct {                            /*!< (@ 0x40017000UL) PWM Structure                     */
    __IOM  uint32_t  EN;                    /*!< (@ 0x00000000) PWM EN Register                     */
    __IOM  uint32_t  UP;                    /*!< (@ 0x00000004) PWM UPDATE Register                 */
    __IOM  uint32_t  RST;                   /*!< (@ 0x00000008) PWM RST  Register                   */
    __IOM  uint32_t  PERIOD;                /*!< (@ 0x0000000c) PWM Period Register                 */
    __IOM  uint32_t  OCPY;                  /*!< (@ 0x00000010) PWM OCPY Register                   */
    __IM   uint32_t  RESERVED[2];
    __IOM  uint32_t  PWMCOMPEN;             /*!< (@ 0x0000001C) PWM  COMP EN Register               */
    __IOM  uint32_t  PWMCOMPTIME;           /*!< (@ 0x00000018) PWM OCPYCOMP TIME Register          */
} XINC_PWM_Type;                            /*!< Size =  ()                                         */


/* =========================================================================================================================== */
/* ================                                            WDT                                            ================ */
/* =========================================================================================================================== */


/**
  * @brief Watchdog Timer (WDT)
  */

typedef struct {                                  /*!< (@ 0x40010000) WDT Structure                                */
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
#define XINC_GPIO0_BASE             0x40001000UL
#define XINC_CPR_BASE               0x40000000UL
#define XINC_CPR_AO_BASE            0x40002400UL
#define XINC_DMAS_BASE              0x50001000UL
#define XINC_UART0_BASE             0x40010000UL
#define XINC_UART1_BASE             0x40011000UL
#define XINC_SPIM0_BASE             0x40013000UL
#define XINC_SPIM1_BASE             0x40014000UL
#define XINC_SPIM2_BASE             0x40014800UL
#define XINC_I2C0_BASE              0x40006000UL
#define XINC_SAADC_BASE             0x40018000UL
#define XINC_SAADC0_BASE            0x40018000UL
#define XINC_RTC0_BASE              0x40002000UL
#define XINC_WDT0_BASE              0x40004000UL
#define XINC_TIMER0_BASE            0x40003000UL
#define XINC_TIMER1_BASE            0x40003014UL
#define XINC_TIMER2_BASE            0x40003028UL
#define XINC_TIMER3_BASE            0x4000303CUL
#define XINC_TIMER_GLOBAL_BASE      0x400030A0UL
#define XINC_TIMER0_CLKCTL_BASE     0x40000058UL
#define XINC_TIMER1_CLKCTL_BASE     0x4000005CUL
#define XINC_TIMER2_CLKCTL_BASE     0x40000060UL
#define XINC_TIMER3_CLKCTL_BASE     0x40000064UL

#define	XINC_PWM0_BASE              0x40017000UL
#define	XINC_PWM1_BASE              0x40017040UL
#define	XINC_PWM2_BASE              0x40017400UL
#define	XINC_PWM3_BASE              0x40017440UL
#define	XINC_PWM4_BASE              0x40017800UL
#define	XINC_PWM5_BASE              0x40017840UL
/* =========================================================================================================================== */
/* ================                                  Peripheral declaration                                   ================ */
/* =========================================================================================================================== */


/** @addtogroup Device_Peripheral_declaration
  * @{
  */
#define XINC_GPIO0                          ((XINC_GPIO_Type*)              XINC_GPIO0_BASE)
#define XINC_CPR                            ((XINC_CPR_CTL_Type*)           XINC_CPR_BASE)
#define XINC_CPR_AO                         ((XINC_CPR_AO_CTL_Type*)        XINC_CPR_AO_BASE)
#define XINC_DMAS0                          ((XINC_DMAS_Type*)              XINC_DMAS_BASE)

#define XINC_UART0                          ((XINC_UART_Type*)              XINC_UART0_BASE)
#define XINC_UART1                          ((XINC_UART_Type*)              XINC_UART1_BASE)
#define XINC_UARTE0                         ((XINC_UARTE_Type*)              XINC_UART0_BASE)
#define XINC_UARTE1                         ((XINC_UARTE_Type*)              XINC_UART1_BASE)
#define XINC_SPIM0                          ((XINC_SPIM_Type*)              XINC_SPIM0_BASE)
#define XINC_SPIM1                          ((XINC_SPIM_Type*)              XINC_SPIM1_BASE)
#define XINC_SPIM2                          ((XINC_SPIM_Type*)              XINC_SPIM2_BASE)

#define XINC_I2C0                           ((XINC_I2C_Type*)               XINC_I2C0_BASE)
#define XINC_SAADC0                         ((XINC_SAADC_Type*)             XINC_SAADC_BASE)
#define XINC_RTC0                           ((XINC_RTC_Type*)               XINC_RTC0_BASE)
#define XINC_WDT0                           ((XINC_WDT_Type*)               XINC_WDT0_BASE)
#define XINC_TIMER0                         ((XINC_TIMER_Type*)             XINC_TIMER0_BASE)
#define XINC_TIMER1                         ((XINC_TIMER_Type*)             XINC_TIMER1_BASE)
#define XINC_TIMER2                         ((XINC_TIMER_Type*)             XINC_TIMER2_BASE)
#define XINC_TIMER3                         ((XINC_TIMER_Type*)             XINC_TIMER3_BASE)
#define XINC_TIMER_GLOBAL                   ((XINC_TIMER_GLOBAL_Type*)      XINC_TIMER_GLOBAL_BASE)
#define XINC_CLKCTL_TIMER0                  ((XINC_TIMER_CLKCTL_Type*)      XINC_TIMER0_CLKCTL_BASE)
#define XINC_CLKCTL_TIMER1                  ((XINC_TIMER_CLKCTL_Type*)      XINC_TIMER1_CLKCTL_BASE)
#define XINC_CLKCTL_TIMER2                  ((XINC_TIMER_CLKCTL_Type*)      XINC_TIMER2_CLKCTL_BASE)
#define XINC_CLKCTL_TIMER3                  ((XINC_TIMER_CLKCTL_Type*)      XINC_TIMER3_CLKCTL_BASE)


#define XINC_PWM0                           ((XINC_PWM_Type*)               XINC_PWM0_BASE)
#define XINC_PWM1                           ((XINC_PWM_Type*)               XINC_PWM1_BASE)
#define XINC_PWM2                           ((XINC_PWM_Type*)               XINC_PWM2_BASE)
#define XINC_PWM3                           ((XINC_PWM_Type*)               XINC_PWM3_BASE)
#define XINC_PWM4                           ((XINC_PWM_Type*)               XINC_PWM4_BASE)
#define XINC_PWM5                           ((XINC_PWM_Type*)               XINC_PWM5_BASE)
#ifdef __cplusplus
}
#endif

#endif /* XINC628_XX_H */


/** @} */ /* End of group XC628_XX */

/** @} */ /* End of group XINCHIP */
