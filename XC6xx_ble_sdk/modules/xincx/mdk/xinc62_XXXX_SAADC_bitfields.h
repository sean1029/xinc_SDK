/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef __XINC62_xxxx_SAADC_BITS_H
#define __XINC62_xxxx_SAADC_BITS_H

/*lint ++flb "Enter library region" */

/* Peripheral: SAADC */
/* Description: SAADC Interface  */

/* SAADC BASE ADDR:0x40018000UL */

/* Register:GPADC_MAIN_CTL    RW */
/* Offse: 0x00 */
/* Description:主要控制寄存器
 */
/* Bit 0 : SAADC GPADC_EN  RW */
/* Description: GPADC 模块开关：
 */
#define SAADC_GPADC_MAIN_CTL_GPADC_EN_Pos (0UL) /*!< Position of GPADC_EN field. */
#define SAADC_GPADC_MAIN_CTL_GPADC_EN_Msk (0x1UL << SAADC_GPADC_MAIN_CTL_GPADC_EN_Pos) /*!< Bit mask of GPADC_EN field. */
#define SAADC_GPADC_MAIN_CTL_GPADC_EN_Open (0x1) /*!< 1：打开 */
#define SAADC_GPADC_MAIN_CTL_GPADC_EN_Close (0x0) /*!< 0：关闭 */

/* Bit 1 : SAADC DMAS_ON  RW */
/* Description: DMAS_ON 功能开关：
 */
#define SAADC_GPADC_MAIN_CTL_DMAS_ON_Pos (1UL) /*!< Position of DMAS_ON field. */
#define SAADC_GPADC_MAIN_CTL_DMAS_ON_Msk (0x1UL << SAADC_GPADC_MAIN_CTL_DMAS_ON_Pos) /*!< Bit mask of DMAS_ON field. */
#define SAADC_GPADC_MAIN_CTL_DMAS_ON_Open (0x1) /*!< 1：打开 */
#define SAADC_GPADC_MAIN_CTL_DMAS_ON_Close (0x0) /*!< 0：关闭 */

/* Bit 2 : SAADC AUTO_SW  RW */
/* Description: GPADC 输入通道自动切换功能
 */
#define SAADC_GPADC_MAIN_CTL_AUTO_SW_Pos (2UL) /*!< Position of AUTO_SW field. */
#define SAADC_GPADC_MAIN_CTL_AUTO_SW_Msk (0x1UL << SAADC_GPADC_MAIN_CTL_AUTO_SW_Pos) /*!< Bit mask of AUTO_SW field. */
#define SAADC_GPADC_MAIN_CTL_AUTO_SW_Open (0x1) /*!< 1：打开 */
#define SAADC_GPADC_MAIN_CTL_AUTO_SW_Close (0x0) /*!< 0：关闭 */

/* Bit 3 : SAADC EDGE_SEL  RW */
/* Description: 数据采样沿选择：
 */
#define SAADC_GPADC_MAIN_CTL_EDGE_SEL_Pos (3UL) /*!< Position of EDGE_SEL field. */
#define SAADC_GPADC_MAIN_CTL_EDGE_SEL_Msk (0x1UL << SAADC_GPADC_MAIN_CTL_EDGE_SEL_Pos) /*!< Bit mask of EDGE_SEL field. */
#define SAADC_GPADC_MAIN_CTL_EDGE_SEL_Rise (0x1) /*!< 上升沿 */
#define SAADC_GPADC_MAIN_CTL_EDGE_SEL_Fall (0x0) /*!< 下降沿 */

#define SAADC_GPADC_MAIN_CTL_ALL_CLOSE (0UL)


/* Register:GPADC_CHAN_CTL    RW */
/* Offse: 0x04 */
/* Description:GPADC 通道控制寄存器
 */
/* Bit 0..3 : SAADC SELECT_CHAN  RW */
/* Description: ADC 输入通道 开关
 */
#define SAADC_GPADC_CHAN_CTL_SELECT_CHAN_Pos (0UL) /*!< Position of SELECT_CHAN field. */
#define SAADC_GPADC_CHAN_CTL_SELECT_CHAN_Msk (0xFUL << SAADC_GPADC_CHAN_CTL_SELECT_CHAN_Pos) /*!< Bit mask of SELECT_CHAN field. */


/* Bit 4..5: SAADC VCM_CHAN_AUTO  RW */
/* Description: 差分输入的自动切换功能使能，bit0 对应通
道 1 和 2 的差分，bit1 对应通道 3 和 4 的差分
 */
#define SAADC_GPADC_CHAN_CTL_VCM_CHAN_AUTO_Pos (4UL) /*!< Position of VCM_CHAN_AUTO field. */
#define SAADC_GPADC_CHAN_CTL_VCM_CHAN_AUTO_Msk (0x3UL << SAADC_GPADC_CHAN_CTL_VCM_CHAN_AUTO_Pos) /*!< Bit mask of VCM_CHAN_AUTO field. */
#define SAADC_GPADC_CHAN_CTL_VCM_CHAN_1_AUTO_Open (0x1) /*!< 1：打开通道 1 和 2 的差分  */
#define SAADC_GPADC_CHAN_CTL_VCM_CHAN_2_AUTO_Open (0x2) /*!< 2：打开通道 3 和 4的差分 */
#define SAADC_GPADC_CHAN_CTL_VCM_CHAN_ALL_AUTO_Close (0x0) /*!< 0：关闭 */

/* Bit 8..11: SAADC CHAN_AUTO  RW */
/* Description: 单端输入时，选择哪个通道参与自动通道切
换功能，每 1bit 对应一个通道
 */
#define SAADC_GPADC_CHAN_CTL_CHAN_AUTO_Pos (8UL) /*!< Position of CHAN_AUTO field. */
#define SAADC_GPADC_CHAN_CTL_CHAN_AUTO_Msk (0xFUL << SAADC_GPADC_CHAN_CTL_CHAN_AUTO_Pos) /*!< Bit mask of CHAN_AUTO field. */
#define SAADC_GPADC_CHAN_CTL_CHAN_1_AUTO_Open (0x1) /*!< 1：打开通道 1 */
#define SAADC_GPADC_CHAN_CTL_CHAN_2_AUTO_Open (0x2) /*!< 1：打开通道 2 */
#define SAADC_GPADC_CHAN_CTL_CHAN_3_AUTO_Open (0x4) /*!< 1：打开通道 3 */
#define SAADC_GPADC_CHAN_CTL_CHAN_4_AUTO_Open (0x8) /*!< 1：打开通道 4 */
#define SAADC_GPADC_CHAN_CTL_CHAN_ALL_AUTO_Close (0x0) /*!< 0：关闭 */


/* Register:GPADC_FIFO_CTL    RW */
/* Offse: 0x08 */
/* Description:FIFO 控制寄存器
 */
/* Bit 0..3 : SAADC READ_REQ_THRESH  RW */
/* Description: 读请求中断阈值
 */
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_Pos (0UL) /*!< Position of CHAN_AUTO field. */
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_Msk (0xFUL << SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_Pos) /*!< Bit mask of CHAN_AUTO field. */
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_1 (1UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_2 (2UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_3 (3UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_4 (4UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_5 (5UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_6 (6UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_7 (7UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_8 (8UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_9 (9UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_10 (10UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_11 (11UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_12 (12UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_13 (13UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_14 (14UL)
#define SAADC_GPADC_FIFO_CTL_READ_REQ_THRESH_LEN_15 (15UL)

/* Bit 4 : SAADC FIFO_FLUSH  RW */
/* Description: 清空 FIFO
 */
#define SAADC_GPADC_FIFO_CTL_FIFO_FLUSH_Pos (4UL) /*!< Position of FIFO_FLUSH field. */
#define SAADC_GPADC_FIFO_CTL_FIFO_FLUSH_Msk (0x1UL << SAADC_GPADC_FIFO_CTL_FIFO_FLUSH_Pos) /*!< Bit mask of FIFO_FLUSH field. */
#define SAADC_GPADC_FIFO_CTL_FIFO_FLUSH_Empty (1UL)/*!< 1：清空 */
#define SAADC_GPADC_FIFO_CTL_FIFO_FLUSH_NotEmpty (0UL)/*!< 0：不清空 */


/* Register:GPADC_TIMER0    RW */
/* Offse: 0x0C */
/* Description:通道切换时间计数器
 */
/* Bit 0..7 : SAADC SW_WAIT_CNT  RW */
/* Description: 通 道 切 换 等 待 时 间 = 总 线 时 钟 周 期
*SW_WAIT_CNT
 */
#define SAADC_GPADC_TIMER0_SW_WAIT_CNT_Pos (0UL) /*!< Position of SW_WAIT_CNT field. */
#define SAADC_GPADC_TIMER0_SW_WAIT_CNT_Msk (0xFFUL << SAADC_GPADC_TIMER0_SW_WAIT_CNT_Pos) /*!< Bit mask of SW_WAIT_CNT field. */

/* Register:GPADC_TIMER1    RW */
/* Offse: 0x10 */
/* Description:通道自动切换时间间隔计数器
 */
/* Bit 0..15 : SAADC AUTO_SW_CNT  RW */
/* Description: 通 道 切 换 等 待 时 间 = 总 线 时 钟 周 期
*AUTO_SW_CNT
 */
#define SAADC_GPADC_TIMER1_AUTO_SW_CNT_Pos (0UL) /*!< Position of AUTO_SW_CNT field. */
#define SAADC_GPADC_TIMER1_AUTO_SW_CNT_Msk (0xFFFFUL << SAADC_GPADC_TIMER1_AUTO_SW_CNT_Pos) /*!< Bit mask of AUTO_SW_CNT field. */


/* Register:GPADC_INT    RW */
/* Offse: 0x14 */
/* Description:中断状态寄存器
 */
/* Bit 0 : SAADC READ_REQ_INT  RW */
/* Description: FIFO 读请求中断
向本比特位写‘1’清除本中断状态位，同
时清除相应的中断原始状态位
 */
#define SAADC_GPADC_INT_READ_REQ_INT_Pos (0UL) /*!< Position of READ_REQ_INT field. */
#define SAADC_GPADC_INT_READ_REQ_INT_Msk (0x1UL << SAADC_GPADC_INT_READ_REQ_INT_Pos) /*!< Bit mask of READ_REQ_INT field. */
#define SAADC_GPADC_INT_READ_REQ_INT_Generated (1UL) /*!< 产生了 FIFO 读请求中断 */
#define SAADC_GPADC_INT_READ_REQ_INT_NotGenerated (0UL) /*!< 没有产生 FIFO 读请求中断 */
#define SAADC_GPADC_INT_READ_REQ_INT_Clear (1UL) /*!< 清除本中断状态位 */



/* Bit 1 : SAADC FIFO_ERROR_INT  RW */
/* Description: FIFO 错误中断
向本比特位写‘1’清除本中断状态位，同
时清除相应的中断原始状态位
 */
#define SAADC_GPADC_INT_FIFO_ERROR_INT_Pos (1UL) /*!< Position of FIFO_ERROR_INT field. */
#define SAADC_GPADC_INT_FIFO_ERROR_INT_Msk (0x1UL << SAADC_GPADC_INT_FIFO_ERROR_INT_Pos) /*!< Bit mask of FIFO_ERROR_INT field. */
#define SAADC_GPADC_INT_FIFO_ERROR_INT_Generated (1UL) /*!< 产生了读空或溢出错误中断 */
#define SAADC_GPADC_INT_FIFO_ERROR_INT_NotGenerated (0UL) /*!< 没有读空或溢出错误中断 */
#define SAADC_GPADC_INT_FIFO_ERROR_INT_Clear (1UL) /*!< 清除本中断状态位 */



/* Register:GPADC_INT_RAW    RW */
/* Offse: 0x18 */
/* Description:中断状态寄存器
 */
/* Bit 0 : SAADC READ_REQ_RAW  RW */
/* Description: FIFO 读请求原始中断
 */
#define SAADC_GPADC_INT_RAW_READ_REQ_RAW_Pos (0UL) /*!< Position of READ_REQ_RAW field. */
#define SAADC_GPADC_INT_RAW_READ_REQ_RAW_Msk (0x1UL << SAADC_GPADC_INT_RAW_READ_REQ_RAW_Pos) /*!< Bit mask of READ_REQ_RAW field. */
#define SAADC_GPADC_INT_RAW_READ_REQ_RAW_Generated (1UL) /*!< 产生了 FIFO 读请求中断 */
#define SAADC_GPADC_INT_RAW_READ_REQ_RAW_NotGenerated (0UL) /*!< 没有产生 FIFO 读请求中断 */




/* Bit 1 : SAADC FIFO_ERROR_RAW  RW */
/* Description: FIFO 错误原始中断
 */
#define SAADC_GPADC_INT_RAW_FIFO_ERROR_RAW_Pos (1UL) /*!< Position of FIFO_ERROR_RAW field. */
#define SAADC_GPADC_INT_RAW_FIFO_ERROR_RAW_Msk (0x1UL << SAADC_GPADC_INT_RAW_FIFO_ERROR_RAW_Pos) /*!< Bit mask of FIFO_ERROR_RAW field. */
#define SAADC_GPADC_INT_RAW_FIFO_ERROR_RAW_Generated (1UL) /*!< 产生了读空或溢出错误中断 */
#define SAADC_GPADC_INT_RAW_FIFO_ERROR_RAW_NotGenerated (0UL) /*!< 没有读空或溢出错误中断 */



/* Register:GPADC_INT_EN    RW */
/* Offse: 0x1C */
/* Description:中断状态寄存器
 */
/* Bit 0 : SAADC READ_REQ_EN  RW */
/* Description: FIFO 读请求中断使能
 */
#define SAADC_GPADC_INT_EN_READ_REQ_EN_Pos (0UL) /*!< Position of READ_REQ_EN field. */
#define SAADC_GPADC_INT_EN_READ_REQ_EN_Msk (0x1UL << SAADC_GPADC_INT_EN_READ_REQ_EN_Pos) /*!< Bit mask of READ_REQ_EN field. */
#define SAADC_GPADC_INT_EN_READ_REQ_EN_Enable (1UL) /*!< 1：使能 */
#define SAADC_GPADC_INT_EN_READ_REQ_EN_Disable (0UL) /*!< 0：不使能 */



/* Bit 1 : SAADC FIFO_ERROR_EN  RW */
/* Description: FIFO 溢出中断使能
 */
#define SAADC_GPADC_INT_EN_FIFO_ERROR_EN_Pos (1UL) /*!< Position of FIFO_ERROR_EN field. */
#define SAADC_GPADC_INT_EN_FIFO_ERROR_EN_Msk (0x1UL << SAADC_GPADC_INT_EN_FIFO_ERROR_EN_Pos) /*!< Bit mask of FIFO_ERROR_EN field. */
#define SAADC_GPADC_INT_EN_FIFO_ERROR_EN_Enable (1UL) /*!< 1：使能 */
#define SAADC_GPADC_INT_EN_FIFO_ERROR_EN_Disable (0UL) /*!< 0：不使能 */


/* Register:GPADC_FIFO    RW */
/* Offse: 0x20 */
/* Description:FIFO 读数据接口
 */
/* Bit 0..31 : SAADC FIFO_DOUT  RW */
/* Description: FIFO 数据输出
 */
#define SAADC_GPADC_FIFO_FIFO_DOUT_Pos (0UL) /*!< Position of FIFO_DOUT field. */
#define SAADC_GPADC_FIFO_FIFO_DOUT_Msk (0xFFFFFFFFUL << SAADC_GPADC_FIFO_FIFO_DOUT_Pos) /*!< Bit mask of FIFO_DOUT field. */
#define SAADC_GPADC_FIFO_FIFO_DOUT_LEN (0x10UL)


/* Register:GPADC_RF_CTL    RW */
/* Offse: 0x24 */
/* Description:GPADC 控制寄存器
 */
/* Bit 0 : SAADC pd_gpadc  RW */
/* Description: 射频 GPADC 断电信号
 */
#define SAADC_GPADC_RF_CTL_pd_gpadc_Pos (0UL) /*!< Position of pd_gpadc field. */
#define SAADC_GPADC_RF_CTL_pd_gpadc_Msk (0x1UL << SAADC_GPADC_RF_CTL_pd_gpadc_Pos) /*!< Bit mask of pd_gpadc field. */
#define SAADC_GPADC_RF_CTL_pd_gpadc_PowerOff (1UL)  /*!< 1：断电 */
#define SAADC_GPADC_RF_CTL_pd_gpadc_PowerOn (0UL)  /*!< 0：不断电 */

/* Bit 1 : SAADC pd_vcmgpadc  RW */
/* Description: 射频 GPADC 差分模式断电信号
 */
#define SAADC_GPADC_RF_CTL_pd_vcmgpadc_Pos (1UL) /*!< Position of pd_vcmgpadc field. */
#define SAADC_GPADC_RF_CTL_pd_vcmgpadc_Msk (0x1UL << SAADC_GPADC_RF_CTL_pd_vcmgpadc_Pos) /*!< Bit mask of pd_vcmgpadc field. */
#define SAADC_GPADC_RF_CTL_pd_vcmgpadc_PowerOff (1UL)  /*!< 1：断电 */
#define SAADC_GPADC_RF_CTL_pd_vcmgpadc_PowerOn (0UL)  /*!< 0：不断电 */

/* Bit 2 : SAADC diffsel_gpadc  RW */
/* Description: 射频 GPADC 差分模式选择信号
 */
#define SAADC_GPADC_RF_CTL_diffsel_gpadc_Pos (2UL) /*!< Position of diffsel_gpadc field. */
#define SAADC_GPADC_RF_CTL_diffsel_gpadc_Msk (0x1UL << SAADC_GPADC_RF_CTL_diffsel_gpadc_Pos) /*!< Bit mask of diffsel_gpadc field. */
#define SAADC_GPADC_RF_CTL_diffsel_gpadc_Diff (1UL)  /*!< 差分模式 */
#define SAADC_GPADC_RF_CTL_diffsel_gpadc_Single (0UL)  /*!< 单端模式 */
#define SAADC_CH_CONFIG_MODE_SE (0UL) /*!< Single-ended, PSELN will be ignored, negative input to SAADC shorted to GND */
#define SAADC_CH_CONFIG_MODE_Diff (1UL) /*!< Differential */



/* Bit 3 : SAADC rst_gpadc  RW */
/* Description: 射频 GPADC 复位控制信号
 */
#define SAADC_GPADC_RF_CTL_rst_gpadc_Pos (3UL) /*!< Position of rst_gpadc field. */
#define SAADC_GPADC_RF_CTL_rst_gpadc_Msk (0x1UL << SAADC_GPADC_RF_CTL_rst_gpadc_Pos) /*!< Bit mask of rst_gpadc field. */
#define SAADC_GPADC_RF_CTL_rst_gpadc_Rst (1UL)  /*!< 复位控制 */
#define SAADC_GPADC_RF_CTL_rst_gpadc_NotRst (0UL)  /*!< 不复位控制 */

/* Bit 4 : SAADC gpadc_ctl_mux  RW */
/* Description: 射频 GPADC 复位控制信号
 */
#define SAADC_GPADC_RF_CTL_gpadc_ctl_mux_Pos (4UL) /*!< Position of gpadc_ctl_mux field. */
#define SAADC_GPADC_RF_CTL_gpadc_ctl_mux_Msk (0x1UL << SAADC_GPADC_RF_CTL_rst_gpadc_Pos) /*!< Bit mask of gpadc_ctl_mux field. */
#define SAADC_GPADC_RF_CTL_gpadc_ctl_mux_Dig (1UL)  /*!< 数字 GPADC 控制 */
#define SAADC_GPADC_RF_CTL_gpadc_ctl_mux_Spi (0UL)  /*!< SPI 控制 */

/*lint --flb "Leave library region" */
#endif
