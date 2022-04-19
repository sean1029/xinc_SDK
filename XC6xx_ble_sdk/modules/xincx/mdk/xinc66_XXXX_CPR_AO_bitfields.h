/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef __XINC66_xxxx_CPR_AO_BITS_H
#define __XINC66_xxxx_CPR_AO_BITS_H

/*lint ++flb "Enter library region" */

/* Peripheral: CPR AO */
/* Description: CPR AO Clock control */

/* CPR PD BASE ADDR:0x40002400 */

/* Register: CPR_AO_SLP_CTL */
/* Offse: 0x04 */
/* Description :睡眠控制寄存器
备注：此寄存器位于 AO 域，始终保持上电状态
*/

/* Bit 0: OSCEN_CTL */
/* Description:OSCEN 控制：
0：OSCEN 在睡眠时被
拉低；
1：OSCEN 在睡眠时不
被拉低
*/
#define CPR_AO_SLP_CTL_OSCEN_CTL_Pos (0UL) 
#define CPR_AO_SLP_CTL_OSCEN_CTL_Msk (0x1UL << CPR_AO_SLP_CTL_OSCEN_CTL_Pos) 
#define CPR_AO_SLP_CTL_OSCEN_CTL_Low (0UL)
#define CPR_AO_SLP_CTL_OSCEN_CTL_High (1UL) 


/* Register: CPR_AO_SLPCTL_INT_MASK */
/* Offse: 0x08 */
/* Description : 睡眠唤 醒 M0 中断屏蔽寄存器
备注：此寄存器位于 AO 域，始终保持上电状态
*/

/* Bit 0..31: INT_MASK */
/* Description:屏蔽唤醒 M0 的中断，对应
bit 为 1 时屏蔽
*/
#define CPR_AO_SLPCTL_INT_MASK_Pos (0UL) 
#define CPR_AO_SLPCTL_INT_MASK_Msk (0xFFFFFFFFUL << CPR_AO_SLPCTL_INT_MASK_Pos) 


/* Register: CPR_AO_SLP_PD_MASK */
/* Offse: 0x0C */
/* Description :  睡眠断电屏蔽寄存器
备注：此寄存器位于 AO 域，始终保持上电状态
*/


/* Bit 0: RAM_SLP_PD_MASK */
/* Description:屏蔽 RAM PD域在睡眠时
的断电，1 有效
*/
#define CPR_AO_SLP_PD_MASK_RAM_SLP_PD_MASK_Pos (0UL) 
#define CPR_AO_SLP_PD_MASK_RAM_SLP_PD_MASK_Msk (0x1UL << CPR_AO_SLP_PD_MASK_RAM_SLP_PD_MASK_Pos) 
#define CPR_AO_SLP_PD_MASK_RAM_SLP_PD_MASK_Enable (1UL)
#define CPR_AO_SLP_PD_MASK_RAM_SLP_PD_MASK_Disable (0UL)

/* Bit 7: SYS_SLP_PD_MASK */
/* Description:屏蔽系统PD域在睡眠时
的断电，1 有效
*/
#define CPR_AO_SLP_PD_MASK_SYS_SLP_PD_MASK_Pos (7UL) 
#define CPR_AO_AO_AO_SLP_PD_MASK_SYS_SLP_PD_MASK_Msk (0x1UL << CPR_AO_SLP_PD_MASK_SYS_SLP_PD_MASK_Pos) 
#define CPR_AO_AO_SLP_PD_MASK_SYS_SLP_PD_MASK_Enable (1UL)
#define CPR_AO_SLP_PD_MASK_SYS_SLP_PD_MASK_Disable (0UL)

/* Bit 8: SYS_SLP_PMU_PD_MASK */
/* Description:屏蔽系统 PMU PD域在睡眠时
的断电，1 有效
*/
#define CPR_AO_SLP_PD_MASK_SYS_SLP_PMU_PD_MASK_Pos (8UL) 
#define CPR_AO_SLP_PD_MASK_SYS_SLP_PMU_PD_MASK_Msk (0x1UL << CPR_AO_SLP_PD_MASK_SYS_SLP_PMU_PD_MASK_Pos) 
#define CPR_AO_SLP_PD_MASK_SYS_SLP_PMU_PD_MASK_Enable (1UL)
#define CPR_AO_SLP_PD_MASK_SYS_SLP_PMU_PD_MASK_Disable (0UL)


/* Register: CPR_AO_MCLK_DIV_CLK_CTL */
/* Offse: 0x10 */
/* Description :  MCLK_DIV_CLK 分频控制寄存器
备注：此寄存器位于 AO 域，始终保持上电状态
*/

/* Bit 0..10: MCLK_DIV_CLK_DIV */
/* Description:MCLK_DIV_CLK 时钟分频
系数
*/
#define CPR_AO_MCLK_DIV_CLK_CTL_MCLK_DIV_CLK_DIV_Pos (0UL) 
#define CPR_AO_MCLK_DIV_CLK_CTL_MCLK_DIV_CLK_DIV_Msk (0x7FFUL << CPR_AO_MCLK_DIV_CLK_CTLK_MCLK_DIV_CLK_DIV_Pos) 

/* Bit 12: MCLK_DIV_CLK_EN */
/* Description: MCLK_DIV_CLK 时钟使能
*/
#define CPR_AO_MCLK_DIV_CLK_CTL_MCLK_DIV_CLK_EN_Pos (12UL) 
#define CPR_AO_MCLK_DIV_CLK_CTL_MCLK_DIV_CLK_EN_Msk (0x1UL << CPR_AO_MCLK_DIV_CLK_CTLK_MCLK_DIV_CLK_DIV_Pos) 
#define CPR_AO_MCLK_DIV_CLK_CTL_MCLK_DIV_CLK_EN_Disable (0UL) 
#define CPR_AO_MCLK_DIV_CLK_CTL_MCLK_DIV_CLK_EN_Enable (1UL) 


/* Register: CPR_AO_RFPMU_SPI_CTL */
/* Offse: 0x14 */
/* Description :  SPI 选择控制寄存器
备注：此寄存器位于 AO 域，始终保持上电状态
*/

/* Bit 0: RFPMU_SPI_SEL */
/* Description:
RFPMU_SPI 选择：
0：选择 RF SPI
1：选择 PMU SPI
*/
#define CPR_AO_RFPMU_SPI_CTL_RFPMU_SPI_SEL_Pos (0UL) 
#define CPR_AO_RFPMU_SPI_CTL_RFPMU_SPI_SEL_Msk (0x1UL << CPR_AO_RFPMU_SPI_CTL_RFPMU_SPI_SEL_Pos) 
#define CPR_AO_RFPMU_SPI_CTL_RFPMU_SPI_SEL_RF_SPI (0UL) 
#define CPR_AO_RFPMU_SPI_CTL_RFPMU_SPI_SEL_PMU_SPI (1UL) 


/* Register: CPR_AO_SYS_TIME */
/* Offse: 0x1C */
/* Description :  系统定时控制寄存器
备注：此寄存器位于 AO 域，始终保持上电状态
*/

/* Bit 0..11: MCLK_STABLE_TIME */
/* Description:
OSC32M/16M 晶振稳定时
间：默认值设定为 16ms
*/
#define CPR_AO_SYS_TIME_MCLK_STABLE_TIME_Pos (0UL) 
#define CPR_AO_SYS_TIME_MCLK_STABLE_TIME_Msk (0xFFFUL << CPR_AO_SYS_TIME_MCLK_STABLE_TIME_Pos) 

/* Bit 12..19: MRST_READY_TIME */
/* Description:
系统复位稳定时间
*/
#define CPR_AO_SYS_TIME_MRST_READY_TIME_Pos (12UL) 
#define CPR_AO_SYS_TIME_MRST_READY_TIME_Msk (0xFFUL << CPR_AO_SYS_TIME_MRST_READY_TIME_Pos) 


/* Register: CPR_AO_BBLDO_ADJ */
/* Offse: 0x20 */
/* Description :
*/

/* Bit 0..3: CORELOD_ADJ_WORK */
/* Description: 
工作模式下coreldo的工作电压配置。
睡眠模式和工作模式的切换由硬件睡眠信号sys_mclk_off控制。
该信号为0时是工作模式，为1时是睡眠模式
*/
#define CPR_AO_BBLDO_ADJ_CORELOD_ADJ_WORK_Pos (0UL) 
#define CPR_AO_BBLDO_ADJ_CORELOD_ADJ_WORK_Msk (0xFUL << CPR_AO_BBLDO_ADJ_CORELOD_ADJ_WORK_Pos) 

/* Bit 4..7: CORELOD_ADJ_SLEEP */
/* Description: 
睡眠模式下coreldo的工作电压配置
*/
#define CPR_AO_BBLDO_ADJ_CORELOD_ADJ_SLEEP_Pos (4UL) 
#define CPR_AO_BBLDO_ADJ_CORELOD_ADJ_SLEEP_Msk (0xFUL << CPR_AO_BBLDO_ADJ_CORELOD_ADJ_WORK_Pos) 


/* Register: CPR_AO_VDD_RETMEM */
/* Offse: 0x24 */
/* Description :
*/

/* Bit 0: VDD_RETMEM_DELAY */
/* Description: 
*/
#define CPR_AO_VDD_RETMEM_VDD_RETMEM_DELAY_Pos (0UL) 
#define CPR_AO_VDD_RETMEM_VDD_RETMEM_DELAY_Msk (0x1UL << CPR_AO_VDD_RETMEM_VDD_RETMEM_DELAY_Pos) 
#define CPR_AO_VDD_RETMEM_VDD_RETMEM_DELAY_Disable (0UL)
#define CPR_AO_VDD_RETMEM_VDD_RETMEM_DELAY_Enable (1UL)

/* Bit 1..2: VDD_RETMEM_EN */
/* Description: 
*/
#define CPR_AO_AO_VDD_RETMEM_VDD_RETMEM_EN_Pos (1UL) 
#define CPR_AO_VDD_RETMEM_VDD_RETMEM_EN_Msk (0x3UL << CPR_AO_VDD_RETMEM_VDD_RETMEM_EN_Pos) 
#define CPR_AO_VDD_RETMEM_VDD_RETMEM_EN_Disable (0UL)

/* Bit 3: VDD_RETMEM_HP_EN */
/* Description: 
*/
#define CPR_AO_AO_VDD_RETMEM_VDD_RETMEM_HP_EN_Pos (3UL) 
#define CPR_AO_VDD_RETMEM_VDD_RETMEM_HP_EN_Msk (0x1UL << CPR_AO_VDD_RETMEM_VDD_RETMEM_HP_EN_Pos) 
#define CPR_AO_VDD_RETMEM_VDD_RETMEM_HP_EN_Disable (0UL)
#define CPR_AO_VDD_RETMEM_VDD_RETMEM_HP_EN_Enable (1UL)


/* Register: CPR_AO_LPO_TRIM */
/* Offse: 0x28 */
/* Description :
*/

/* Bit 0..6: LPO_RTM */
/* Description: 
*/
#define CPR_AO_LPO_TRIM_LPO_RTM_Pos (0UL) 
#define CPR_AO_LPO_TRIM_LPO_RTM_Msk (0x7FUL << CPR_AO_LPO_TRIM_LPO_RTM_Pos) 

/* Bit 8..9: LPO_CTM */
/* Description: 
*/
#define CPR_LPO_TRIM_LPO_CTM_Pos (8UL) 
#define CPR_LPO_TRIM_LPO_CTM_Msk (0x3UL << CPR_LPO_TRIM_LPO_CTM_Pos) 


/* Register: CPR_XTL32K_VAL */
/* Offse: 0x2C */
/* Description :
*/

/* Bit 0..3: XTL32K_V */
/* Description: 
*/
#define CPR_AO_XTL32K_VAL_XTL32K_V_Pos (0UL) 
#define CPR_AO_XTL32K_VAL_XTL32K_V_Msk (0xFUL << CPR_AO_XTL32K_VAL_XTL32K_V_Pos) 

/* Bit 4: XTL32K_EN */
/* Description: 
*/
#define CPR_AO_XTL32K_VAL_XTL32K_EN_Pos (4UL) 
#define CPR_AO_XTL32K_VAL_XTL32K_EN_Msk (0xFUL << CPR_AO_XTL32K_VAL_XTL32K_EN_Pos) 
#define CPR_AO_XTL32K_VAL_XTL32K_EN_Disable (0UL)
#define CPR_AO_XTL32K_VAL_XTL32K_EN_Enable (1UL)



/* Register: CPR_AO_RF_AON */
/* Offse: 0x30 */
/* Description :
*/

#if 0
/* Bit 0: DA_DCXO_IC */
/* Description: 
0：不使能时钟
1：使能时钟
*/
#define CPR_RF_AON_DA_DCXO_IC_Pos (0UL) 
#define CPR_RF_AON_DA_DCXO_IC_Msk (0x1UL << CPR_RF_AON_DA_DCXO_IC_Pos) 
#define CPR_RF_AON_DA_DCXO_IC_Disable (0UL)
#define CPR_RF_AON_DA_DCXO_IC_Enable (1UL) 

/* Bit 1: DA_LDO_SEL */
/* Description: 
RF LDO和BG的控制源选择：
0：LDO和BG由硬件自动控制使能
1：LDO和BG由da_bg_en和da_ldo_en寄存器软件控制使能
*/
#define CPR_RF_AON_DA_LDO_SEL_Pos (1UL) 
#define CPR_RF_AON_DA_LDO_SEL_Msk (0x1UL << CPR_RF_AON_DA_LDO_SEL_Pos) 
#define CPR_RF_AON_DA_LDO_SEL_BY_HW (0UL)
#define CPR_RF_AON_DA_LDO_SEL_BY_SW (1UL) 

/* Bit 2: DA_LDO_EN */
/* Description: 
DA_LDO软件使能，
0 不使能
1 使能
*/
#define CPR_RF_AON_DA_LDO_EN_Pos (2UL) 
#define CPR_RF_AON_DA_LDO_EN_Msk (0x1UL << CPR_RF_AON_DA_LDO_EN_Pos) 
#define CPR_RF_AON_DA_LDO_EN_Disable (0UL)
#define CPR_RF_AON_DA_LDO_EN_Enable (1UL) 

/* Bit 3: OSCEN_AO */
/* Description: 
32M晶体常开使能，睡眠时候不会关断32M晶体，
1：使能常开，DA_DCXO_EN和DA_CLK_BB_EN保持为1
0：DA_DCXO_EN和DA_CLK_BB_EN由硬件状态机控制
*/
#define CPR_RF_AON_OSCEN_AO_Pos (3UL) 
#define CPR_RF_AON_OSCEN_AO_Msk (0x1UL << CPR_RF_AON_OSCEN_AO_Pos) 
#define CPR_RF_AON_OSCEN_AO_Disable (0UL)
#define CPR_RF_AON_OSCEN_AO_Enable (1UL) 


/* Bit 4: DA_BG_EN */
/* Description: 
RF BG软件使能
0 不使能
1 使能
*/
#define CPR_RF_AON_DA_BG_EN_Pos (4UL) 
#define CPR_RF_AON_DA_BG_EN_Msk (0x1UL << CPR_RF_AON_DA_BG_EN_Pos) 
#define CPR_RF_AON_DA_BG_EN_Disable (0UL)
#define CPR_RF_AON_DA_BG_EN_Enable (1UL) 

#endif //

/* Bit 0..2: RG_CLK_BB_DRIVE */
/* Description: 
*/
#define CPR_AO_RF_AON_RG_CLK_BB_DRIVE_Pos (0UL) 
#define CPR_AO_RF_AON_RG_CLK_BB_DRIVE_Msk (0x7UL << CPR_AO_RF_AON_RG_CLK_BB_DRIVE_Pos)

/* Bit 4..6: RG_CLK_PATH_VDDRES */
/* Description: 
*/
#define CPR_AO_AO_RF_AON_RG_CLK_PATH_VDDRES_Pos (6UL) 
#define CPR_AO_RF_AON_RG_CLK_PATH_VDDRES_Msk (0x7UL << CPR_AO_RF_AON_RG_CLK_PATH_VDDRES_Pos)

/* Bit 8..9: RG_CLK_LDO_VOUT */
/* Description: 
*/
#define CPR_RF_AON_RG_CLK_LDO_VOUT_Pos (8UL) 
#define CPR_RF_AON_RG_CLK_LDO_VOUT_Msk (0x3UL << CPR_RF_AON_RG_CLK_LDO_VOUT_Pos)

/* Bit 10..11: RG_ISM_LVSHIFT_LDO_VOUT */
/* Description: 
*/
#define CPR_RF_AON_RG_ISM_LVSHIFT_LDO_VOUT_Pos (10UL) 
#define CPR_RF_AON_RG_ISM_LVSHIFT_LDO_VOUT_Msk (0x3UL << CPR_RF_AON_RG_ISM_LVSHIFT_LDO_VOUT_Pos)

/* Bit 12..17: RG_DCXO_GMTUNE */
/* Description: 
*/
#define CPR_RF_AON_RG_DCXO_GMTUNE_Pos (10UL) 
#define CPR_RF_AON_RG_DCXO_GMTUNE_Msk (0x3FUL << CPR_RF_AON_RG_DCXO_GMTUNE_Pos)

/* Bit 25..20: RG_DCXO_CTUNE */
/* Description: 
*/
#define CPR_RF_AON_RG_DCXO_CTUNE_Pos (20UL) 
#define CPR_RF_AON_RG_DCXO_CTUNE_Msk (0x3FUL << CPR_RF_AON_RG_DCXO_CTUNE_Pos)

/* Bit 31: BB_LDO_ADDA_BBVC0_EN */
/* Description: 
*/
#define CPR_RF_AON_BB_LDO_ADDA_BBVCO_EN_Pos (32UL) 
#define CPR_RF_AON_BB_LDO_ADDA_BBVCO_EN_Msk (0x1UL << CPR_RF_AON_BB_LDO_ADDA_BBVC0_EN_Pos)
#define CPR_RF_AON_BB_LDO_ADDA_BBVCO_EN_Disable (0UL)
#define CPR_RF_AON_BB_LDO_ADDA_BBVCO_EN_Enable (1UL) 

/* Register: CPR_AO_PECTRL */
/* Offse: 0x34 --0x38*/
/* Description :
*/


/* Register: CPR_AO_PUCTRL */
/* Offse: 0x3C*/
/* Description :
*/


/* Register: CPR_AO_VDD_SWITCH_EN */
/* Offse: 0x40*/
/* Description :
*/

/* Bit 0: VDDMDM_EN */
/* Description: BT MODEM断电
*/
#define CPR_AO_VDD_SWITCH_EN_VDDMDM_EN_Pos (0UL) 
#define CPR_AO_VDD_SWITCH_EN_VDDMDM_EN_Msk (0x1UL << CPR_AO_VDD_SWITCH_EN_VDDMDM_EN_Pos) 
#define CPR_AO_VDD_SWITCH_EN_VDDMDM_EN_Disable (0UL)
#define CPR_AO_VDD_SWITCH_EN_VDDMDM_EN_Enable (1UL)

/* Bit 1: VDDLTRAM_EN */
/* Description: BT RAM断电
*/
#define CPR_AO_VDD_SWITCH_EN_VDDLTRAM_EN_Pos (1UL) 
#define CPR_AO_VDD_SWITCH_EN_VDDLTRAM_EN_Msk (0x1UL << CPR_AO_VDD_SWITCH_EN_VDDLTRAM_EN_Pos) 
#define CPR_AO_VDD_SWITCH_EN_VDDLTRAM_EN_Disable (0UL)
#define CPR_AO_VDD_SWITCH_EN_VDDLTRAM_EN_Enable (1UL)

/* Bit 2: VDDRAM1_EN */
/* Description: SHRAM0 H bank 断电
*/
#define CPR_AO_VDD_SWITCH_EN_VDDRAM1_EN_Pos (2UL) 
#define CPR_AO_VDD_SWITCH_EN_VDDRAM1_EN_Msk (0x1UL << CPR_AO_VDD_SWITCH_EN_VDDRAM1_EN_Pos) 
#define CPR_AO_VDD_SWITCH_EN_VDDRAM1_EN_Disable (0UL)
#define CPR_AO_VDD_SWITCH_EN_VDDRAM1_EN_Enable (1UL)

/* Bit 3: VDDRAM0_EN */
/* Description: SHRAM0 L bank 断电
*/
#define CPR_AO_VDD_SWITCH_EN_VDDRAM0_EN_Pos (3UL) 
#define CPR_AO_VDD_SWITCH_EN_VDDRAM0_EN_Msk (0x1UL << CPR_AO_VDD_SWITCH_EN_VDDRAM0_EN_Pos) 
#define CPR_AO_VDD_SWITCH_EN_VDDRAM0_EN_Disable (0UL)
#define CPR_AO_VDD_SWITCH_EN_VDDRAM0_EN_Enable (1UL)

/* Bit 4: VDDROM_EN */
/* Description: ROM断电
*/
#define CPR_AO_VDD_SWITCH_EN_VDDROM_EN_Pos (4UL) 
#define CPR_AO_VDD_SWITCH_EN_VDDROM_EN_Msk (0x1UL << CPR_AO_VDD_SWITCH_EN_VDDROM_EN_Pos) 
#define CPR_AO_VDD_SWITCH_EN_VDDROM_EN_Disable (0UL)
#define CPR_AO_VDD_SWITCH_EN_VDDROM_EN_Enable (1UL)


/* Register: CPR_AO_VDD_ISO_EN */
/* Offse: 0x44*/
/* Description :
*/

/* Bit 0: VDDMDM_EN */
/* Description: BT MODEM断电隔离
*/
#define CPR_AO_VDD_ISO_EN_VDDMDM_EN_Pos (0UL) 
#define CPR_AO_VDD_ISO_EN_VDDMDM_EN_Msk (0x1UL << CPR_AO_VDD_ISO_EN_VDDMDM_EN_Pos) 
#define CPR_AO_VDD_ISO_EN_VDDMDM_EN_Disable (0UL)
#define CPR_AO_VDD_ISO_EN_VDDMDM_EN_Enable (1UL)

/* Bit 1: VDDLTRAM_EN */
/* Description: BT RAM断电隔离
*/
#define CPR_AO_VDD_ISO_EN_VDDLTRAM_EN_Pos (1UL) 
#define CPR_AO_VDD_ISO_EN_VDDLTRAM_EN_Msk (0x1UL << CPR_AO_VDD_SWITCH_EN_VDDLTRAM_EN_Pos) 
#define CPR_AO_VDD_ISO_EN_VDDLTRAM_EN_Disable (0UL)
#define CPR_AO_VDD_ISO_EN_VDDLTRAM_EN_Enable (1UL)

/* Bit 2: VDDRAM1_EN */
/* Description: SHRAM0 H bank 断电隔离
*/
#define CPR_AO_VDD_ISO_EN_VDDRAM1_EN_Pos (2UL) 
#define CPR_AO_VDD_ISO_EN_VDDRAM1_EN_Msk (0x1UL << CPR_AO_VDD_ISO_EN_VDDRAM1_EN_Pos) 
#define CPR_AO_VDD_ISO_EN_VDDRAM1_EN_Disable (0UL)
#define CPR_AO_VDD_ISO_EN_VDDRAM1_EN_Enable (1UL)

/* Bit 3: VDDRAM0_EN */
/* Description: SHRAM0 L bank 断电隔离
*/
#define CPR_AO_VDD_ISO_EN_VDDRAM0_EN_Pos (3UL) 
#define CPR_AO_VDD_ISO_EN_VDDRAM0_EN_Msk (0x1UL << CPR_AO_VDD_ISO_EN_VDDRAM0_EN_Pos) 
#define CPR_AO_VDD_ISO_EN_VDDRAM0_EN_Disable (0UL)
#define CPR_AO_VDD_ISO_EN_VDDRAM0_EN_Enable (1UL)

/* Bit 4: VDDROM_EN */
/* Description: ROM断电隔离
*/
#define CPR_AO_VDD_ISO_EN_VDDROM_EN_Pos (4UL) 
#define CPR_AO_VDD_ISO_EN_VDDROM_EN_Msk (0x1UL << CPR_AO_VDD_ISO_EN_VDDROM_EN_Pos) 
#define CPR_AO_VDD_ISO_EN_VDDROM_EN_Disable (0UL)
#define CPR_AO_VDD_ISO_EN_VDDROM_EN_Enable (1UL)


/* Register: CPR_AO_LPOLDO_ADJ */
/* Offse: 0x48 */
/* Description :
*/

/* Bit 0..2: BB_LPOLDO_ADJ */
/* Description: LPOLDO档位配置
*/
#define CPR_AO_LPOLDO_ADJ_BB_LPOLDO_ADJ_Pos (0UL) 
#define CPR_AO_LPOLDO_ADJ_BB_LPOLDO_ADJ_Msk (0x7UL << CPR_AO_LPOLDO_ADJ_BB_LPOLDO_ADJ_Pos) 


/* Register: CPR_AO_RETMEMLDO_ADJ */
/* Offse: 0x4C */
/* Description :
*/

/* Bit 0..2: BB_RETMEMLDO_ADJ */
/* Description: RETMEMLDO档位配置
*/
#define CPR_AO_RETMEMLDO_ADJ_BB_RETMEMLDO_ADJ_Pos (0UL) 
#define CPR_AO_RETMEMLDO_ADJ_BB_RETMEMLDO_ADJ_Msk (0x7UL << CPR_AO_RETMEMLDO_ADJ_BB_RETMEMLDO_ADJ_Pos) 


/* Register: CPR_AO_CORERFLDO_EN */
/* Offse: 0x50 */
/* Description :
*/

/* Bit 0: BB_CORERFLDO_EN */
/* Description: RETMEMLDO档位配置
*/
#define CPR_AO_CORERFLDO_ADJ_BB_CORERFLDO_EN_Pos (0UL) 
#define CPR_AO_CORERFLDO_ADJ_BB_CORERFLDO_EN_Msk (0x1UL << CPR_AO_CORERFLDO_ADJ_BB_CORERFLDO_EN_Pos) 
#define CPR_AO_CORERFLDO_ADJ_BB_CORERFLDO_EN_Disable (0UL)
#define CPR_AO_CORERFLDO_ADJ_BB_CORERFLDO_EN_Enable (1UL)



/* Register: DCDC_CTRL0 */
/* Offse: 0x54 */
/* Description :
*/

/* Bit 0..1: DCDC_PDRSLOW */
/* Description: 
*/
#define CPR_AO_DCDC_CTRL0_PDRSLOW_Pos (0UL) 
#define CPR_AO_DCDC_CTRL0_PDRSLOW_Msk (0x3UL << CPR_AO_DCDC_CTRL0_PDRSLOW_Pos) 

/* Bit 2: DCDC_CL_CTRL */
/* Description: 
*/
#define CPR_AO_DCDC_CTRL0_CL_CTR_Pos (2UL) 
#define CPR_AO_DCDC_CTRL0_CL_CTR_Msk (0x1UL << CPR_AO_DCDC_CTRL0_CL_CTR_Pos) 

/* Bit 3: DCDC_VOUTDOWM */
/* Description: 
dcdc寄存器，控制dcdc输出下拉到地，1下拉到地，默认0
*/
#define CPR_AO_DCDC_CTRL0_VOUTDOWM_Pos (3UL) 
#define CPR_AO_DCDC_CTRL0_VOUTDOWM_Msk (0x1UL << CPR_AO_DCDC_CTRL0_VOUTDOWM_Pos) 
#define CPR_AO_DCDC_CTRL0_VOUTDOWM_Disable (0UL)
#define CPR_AO_DCDC_CTRL0_VOUTDOWM_Enable (1UL) 

/* Bit 4: DCDC_EN_SOFT */
/* Description: 
  dcdc  en信号，1 work，硬件控制，默认0
*/
#define CPR_AO_DCDC_CTRL0_EN_SOFT_Pos (4UL) 
#define CPR_AO_DCDC_CTRL0_EN_SOFT_Msk (0x1UL << CPR_AO_DCDC_CTRL0_EN_SOFT_Pos) 
#define CPR_AO_DCDC_CTRL0_EN_SOFT_Disable (0UL)
#define CPR_AO_DCDC_CTRL0_EN_SOFT_HW (1UL) 


/* Bit 5: DCDC_BG_EN_SOFT */
/* Description: 
dcdc bandgap en信号，1 work，要比dcdc_en早一个32K周期，硬件控制
，默认0
*/
#define CPR_AO_DCDC_CTRL0_BG_EN_SOFT_Pos (5UL) 
#define CPR_AO_DCDC_CTRL0_BG_EN_SOFT_Msk (0x1UL << CPR_AO_DCDC_CTRL0_BG_EN_SOFT_Pos) 
#define CPR_AO_AO_DCDC_CTRL0_BG_EN_SOFT_Disable (0UL)
#define CPR_AO_DCDC_CTRL0_BG_EN_SOFT_HW (1UL) 

/* Bit 6: DCDC_EN_SEL */
/* Description: 
dcdc ctrl select, 1:software ctrl, 0:hardware ctrl
*/
#define CPR_AO_DCDC_CTRL0_EN_SEL_Pos (6UL) 
#define CPR_AO_DCDC_CTRL0_EN_SEL_Msk (0x1UL << CPR_AO_DCDC_CTRL0_EN_SEL_Pos) 
#define CPR_AO_DCDC_CTRL0_EN_SEL_HW (0UL)
#define CPR_AO_DCDC_CTRL0_EN_SEL_SW (1UL) 


/* Register: DCDC_CTRL1 */
/* Offse: 0x58 */
/* Description :
*/

/* Bit 0: DCDC_LP_EN */
/* Description: 
*/
#define CPR_AO_DCDC_CTRL1_LP_EN_Pos (0UL) 
#define CPR_AO_AO_DCDC_CTRL1_LP_EN_Msk (0x1UL << CPR_AO_DCDC_CTRL1_LP_EN_Pos) 
#define CPR_AO_DCDC_CTRL1_LP_EN_Disable (0UL)
#define CPR_AO_DCDC_CTRL1_LP_EN_Enable (1UL) 

/* Bit 1..2: DCDC_CF */
/* Description: 
 dcdc寄存器，控制时钟分频比，默认01
*/
#define CPR_AO_DCDC_CTRL1_CF_Pos (1UL) 
#define CPR_AO_DCDC_CTRL1_CF_Msk (0x3UL << CPR_AO_DCDC_CTRL1_CF_Pos) 

/* Bit 3: DCDC_DISPFM */
/* Description: 
*/
#define CPR_AO_DCDC_CTRL1_DISPFM_Pos (3UL) 
#define CPR_AO_DCDC_CTRL1_DISPFM_Msk (0x1UL << CPR_AO_DCDC_CTRL1_DISPFM_Pos) 
#define CPR_AO_DCDC_CTRL1_DISPFM_Disable (0UL)
#define CPR_AO_DCDC_CTRL1_DISPFM_Enable (1UL) 


/* Bit 4..8: DCDC_FREQCAL */
/* Description: 
dcdc寄存器，控制时钟频率，默认01000
*/
#define CPR_AO_DCDC_CTRL1_FREQCAL_Pos (4UL) 
#define CPR_AO_DCDC_CTRL1_FREQCAL_Msk (0x1FUL << CPR_AO_DCDC_CTRL1_FREQCAL_Pos) 

/* Bit 9: DCDC_ZCD_EN */
/* Description: 
*/
#define CPR_AO_DCDC_CTRL1_ZCD_EN_Pos (9UL) 
#define CPR_AO_DCDC_CTRL1_ZCD_EN_Msk (0x1UL << CPR_AO_DCDC_CTRL1_ZCD_EN_Pos) 
#define CPR_AO_DCDC_CTRL1_DISPFM_Disable (0UL)
#define CPR_AO_DCDC_CTRL1_DISPFM_Enable (1UL) 

/* Bit 10: DCDC_DEADTIME */
/* Description: 
*/
#define CPR_DCDC_CTRL1_DEADTIME_Pos (10UL) 
#define CPR_DCDC_CTRL1_DEADTIME_Msk (0x1UL << CPR_AO_DCDC_CTRL1_DEADTIME_Pos) 
#define CPR_DCDC_CTRL1_DEADTIME_Disable (0UL)
#define CPR_DCDC_CTRL1_DEADTIME_Enable (1UL) 

/* Bit 11..16: DCDC_STBOP */
/* Description: 
*/
#define CPR_AO_DCDC_CTRL1_STBOP_Pos (11UL) 
#define CPR_AO_DCDC_CTRL1_STBOP_Msk (0x1FUL << CPR_AO_DCDC_CTRL1_STBOP_Pos) 

/* Bit 17..21: DCDC_CAL */
/* Description: 
 dcdc寄存器，微调输出电压，默认10000
*/
#define CPR_AO_DCDC_CTRL1_CAL_Pos (17UL) 
#define CPR_AO_DCDC_CTRL1_CAL_Msk (0x1FUL << CPR_AO_DCDC_CTRL1_CAL_Pos) 

/* Bit 22..26: DCDC_VREF_CAL */
/* Description: 
  dcdc寄存器，微调参考电压，默认10000
*/
#define CPR_AO_DCDC_CTRL1_VREF_CAL_Pos (22UL) 
#define CPR_AO_DCDC_CTRL1_VREF_CAL_Msk (0x1FUL << CPR_AO_DCDC_CTRL1_VREF_CAL_Pos) 

/* Bit 27: DCDC_DCM */
/* Description: 
  dcdc寄存器，微调参考电压，默认10000
*/
#define CPR_AO_DCDC_CTRL1_DCM_Pos (27UL) 
#define CPR_AO_DCDC_CTRL1_DCM_Msk (0x1UL << CPR_AO_DCDC_CTRL1_DCM_Pos) 
#define CPR_AO_DCDC_CTRL1_DCM_Disable (0UL)
#define CPR_AO_DCDC_CTRL1_DCM_Enable (1UL) 

/* Bit 28..30: DCDC_CTRL */
/* Description: 
  dcdc寄存器，控制输出电压，默认100
*/
#define CPR_AO_DCDC_CTRL1_CTRL_Pos (28UL) 
#define CPR_AO_DCDC_CTRL1_CTRL_Msk (0x7UL << CPR_AO_DCDC_CTRL1_CTRL_Pos) 



/* Register: AUDIO_ADC_CTRL0 */
/* Offse: 0x5C */
/* Description :
*/

/* Bit 0..4: RESADJ */
/* Description: 
 bias寄存器控制信号
*/
#define CPR_AO_AUDIO_ADC_CTRL0_RESADJ_Pos (0UL) 
#define CPR_AO_AUDIO_ADC_CTRL0_RESADJ_Msk (0x1FUL << CPR_AO_AUDIO_ADC_CTRL0_RESADJ_Pos) 

/* Bit 5..8: VOLR */
/* Description:
pga第2级增益 
*/
#define CPR_AO_AUDIO_ADC_CTRL0_VOLR_Pos (5UL) 
#define CPR_AO_AUDIO_ADC_CTRL0_VOLR_Msk (0xFUL << CPR_AO_AUDIO_ADC_CTRL0_VOLR_Pos) 

/* Bit 9: MICGAIN */
/* Description:
pga第1级增益 
*/
#define CPR_AO_AUDIO_ADC_CTRL0_MICGAIN_Pos (9UL) 
#define CPR_AO_AUDIO_ADC_CTRL0_MICGAIN_Msk (0x1UL << CPR_AO_AUDIO_ADC_CTRL0_MICGAIN_Pos) 
#define CPR_AO_AUDIO_ADC_CTRL0_MICGAIN_Disable (0UL)
#define CPR_AO_AUDIO_ADC_CTRL0_MICGAIN_Enable (1UL) 

/* Bit 10: VBCTRL_L */
/* Description:
输出micbias寄存器控制信号
*/
#define CPR_AO_AUDIO_ADC_CTRL0_VBCTRL_L_Pos (10UL) 
#define CPR_AO_AO_AUDIO_ADC_CTRL0_VBCTRL_L_Msk (0x1UL << CPR_AO_AUDIO_ADC_CTRL0_VBCTRL_L_Pos) 
#define CPR_AO_AUDIO_ADC_CTRL0_VBCTRL_L_Disable (0UL)
#define CPR_AO_AUDIO_ADC_CTRL0_VBCTRL_L_Enable (1UL) 

/* Bit 11: PDVBIAS */
/* Description:
mic bias PD信号
*/
#define CPR_AO_AUDIO_ADC_CTRL0_PDVBIAS_Pos (11UL) 
#define CPR_AO_AUDIO_ADC_CTRL0_PDVBIAS_Msk (0x1UL << CPR_AO_AUDIO_ADC_CTRL0_PDVBIAS_Pos) 
#define CPR_AO_AUDIO_ADC_CTRL0_PDVBIAS_Disable (0UL)
#define CPR_AO_AUDIO_ADC_CTRL0_PDVBIAS_Enable (1UL) 

/* Bit 12: PDSDMR */
/* Description:
sdm PD信号
*/
#define CPR_AO_AUDIO_ADC_CTRL0_PDSDMR_Pos (12UL) 
#define CPR_AO_AUDIO_ADC_CTRL0_PDSDMR_Msk (0x1UL << CPR_AO_AUDIO_ADC_CTRL0_PDSDMR_Pos) 
#define CPR_AO_AUDIO_ADC_CTRL0_PDSDMR_Disable (0UL)
#define CPR_AO_AUDIO_ADC_CTRL0_PDSDMR_Enable (1UL) 

/* Bit 13: PDPGAR */
/* Description:
pga PD信号
*/
#define CPR_AO_AUDIO_ADC_CTRL0_PDPGAR_Pos (13UL) 
#define CPR_AO_AUDIO_ADC_CTRL0_PDPGAR_Msk (0x1UL << CPR_AO_AUDIO_ADC_CTRL0_PDPGAR_Pos) 
#define CPR_AO_AUDIO_ADC_CTRL0_PDPGAR_Disable (0UL)
#define CPR_AO_AUDIO_ADC_CTRL0_PDPGAR_Enable (1UL) 

/* Bit 14: PDMIC */
/* Description:
mic输入buf PD信号
*/
#define CPR_AO_AUDIO_ADC_CTRL0_PDMIC_Pos (14UL) 
#define CPR_AO_AUDIO_ADC_CTRL0_PDMIC_Msk (0x1UL << CPR_AO_AUDIO_ADC_CTRL0_PDMIC_Pos) 
#define CPR_AO_AUDIO_ADC_CTRL0_PDMIC_Disable (0UL)
#define CPR_AO_AUDIO_ADC_CTRL0_PDMIC_Enable (1UL) 

/* Bit 15: PDCHG */
/* Description:
模拟内部寄存器控制信号
*/
#define CPR_AO_AUDIO_ADC_CTRL0_PDCHG_Pos (15UL) 
#define CPR_AO_AUDIO_ADC_CTRL0_PDCHG_Msk (0x1UL << CPR_AO_AUDIO_ADC_CTRL0_PDCHG_Pos) 
#define CPR_AO_AUDIO_ADC_CTRL0_PDCHG_Disable (0UL)
#define CPR_AO_AUDIO_ADC_CTRL0_PDCHG_Enable (1UL) 



/* Bit 16: PDBIAS */
/* Description:
adc bias PD信号
*/
#define CPR_AO_AUDIO_ADC_CTRL0_PDBIAS_Pos (16UL) 
#define CPR_AO_AUDIO_ADC_CTRL0_PDBIAS_Msk (0x1UL << CPR_AO_AUDIO_ADC_CTRL0_PDBIAS_Pos) 
#define CPR_AO_AUDIO_ADC_CTRL0_PDBIAS_Disable (0UL)
#define CPR_AO_AUDIO_ADC_CTRL0_PDBIAS_Enable (1UL) 



/* Bit 17: PD_USB */
/* Description:
usb phy Power down
*/
#define CPR_AO_AUDIO_ADC_CTRL0_PD_USB_Pos (17UL) 
#define CPR_AO_AUDIO_ADC_CTRL0_PD_USB_Msk (0x1UL << CPR_AO_AUDIO_ADC_CTRL0_PD_USB_Pos) 
#define CPR_AO_AUDIO_ADC_CTRL0_PD_USB_Disable (0UL)
#define CPR_AO_AUDIO_ADC_CTRL0_PD_USB_Enable (1UL) 



/* Register: CPR_AOCLKEN_GRCTL */
/* Offse: 0x7C */
/* Description :USB AUDIO 2.4G 相关控制寄存器
 * AO 域时钟使能寄存器
 * 高 16BITS 写屏蔽低 16BITS
*/

#define CPR_AO_AOCLKEN_GRCTL_MASK_OFFSET (16UL)

/* Bit 0: TIMER_AO_PCLK_EN */
/* Description: timer ao 总线时钟使能
0：不使能时钟
1：使能时钟
*/
#define CPR_AO_AOCLKEN_GRCTL_TIMER_AO_PCLK_EN_Pos (0UL) 
#define CPR_AO_AOCLKEN_GRCTL_TIMER_AO_PCLK_EN_Msk (0x1UL << CPR_AO_AOCLKEN_GRCTL_TIMER_AO_PCLK_EN_Pos) 
#define CPR_AO_AOCLKEN_GRCTL_TIMER_AO_PCLK_EN_Disable (0UL)
#define CPR_AO_AOCLKEN_GRCTL_TIMER_AO_PCLK_EN_Enable (1UL) 


/* Bit 1: RTC_CLK_EN */
/* Description: RTC_CLK 时钟使能
0：不使能时钟
1：使能时钟
*/
#define CPR_AO_AOCLKEN_GRCTL_RTC_CLK_EN_Pos (1UL) // /*!< Position of RTC_CLK_EN field. */
#define CPR_AO_AOCLKEN_GRCTL_RTC_CLK_EN_Msk (0x1UL << CPR_AO_AOCLKEN_GRCTL_RTC_CLK_EN_Pos) 
#define CPR_AO_AOCLKEN_GRCTL_RTC_CLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_AO_AOCLKEN_GRCTL_RTC_CLK_EN_Enable (1UL) /*!< 1：使能时钟*/

/* Bit 2: TIMER0_AO_PCLK_EN */
/* Description: timer0 ao 总线时钟使能
0：不使能时钟
1：使能时钟
*/
#define CPR_AO_AOCLKEN_GRCTL_TIMER0_AO_PCLK_EN_Pos (2UL) 
#define CPR_AO_AOCLKEN_GRCTL_TIMER0_AO_PCLK_EN_Msk (0x1UL << CPR_AO_AOCLKEN_GRCTL_TIMER0_AO_PCLK_EN_Pos) 
#define CPR_AO_AOCLKEN_GRCTL_TIMER0_AO_PCLK_EN_Disable (0UL)
#define CPR_AO_AOCLKEN_GRCTL_TIMER0_AO_PCLK_EN_Enable (1UL) 

/* Bit 3: TIMER1_AO_PCLK_EN */
/* Description: timer1 ao 总线时钟使能
0：不使能时钟
1：使能时钟
*/
#define CPR_AO_AOCLKEN_GRCTL_TIMER1_AO_PCLK_EN_Pos (3UL) 
#define CPR_AO_AOCLKEN_GRCTL_TIMER1_AO_PCLK_EN_Msk (0x1UL << CPR_AO_AOCLKEN_GRCTL_TIMER1_AO_PCLK_EN_Pos) 
#define CPR_AO_AOCLKEN_GRCTL_TIMER1_AO_PCLK_EN_Disable (0UL)
#define CPR_AO_AOCLKEN_GRCTL_TIMER1_AO_PCLK_EN_Enable (1UL) 

/* Bit 4: TIMER_AO_RSTN */
/* Description: timer_aon软复位，0复位
0：复位
1：不复位
*/
#define CPR_AO_AOCLKEN_GRCTL_TIMER_AO_RSTN_Pos (4UL) 
#define CPR_AO_AOCLKEN_GRCTL_TIMER_AO_RSTN_Msk (0x1UL << CPR_AO_AOCLKEN_GRCTL_TIMER_AO_RSTN_Pos) 
#define CPR_AO_AOCLKEN_GRCTL_TIMER_AO_RSTN_Disable (RSTN_INVALID)
#define CPR_AO_AOCLKEN_GRCTL_TIMER_AO_RSTN_Enable (RSTN_VALID) 


/* Register: CPR_AO_CTL_CLK32K */
/* Offse: 0x14C */
/* Description :32K 时钟设置寄存器
*/

/* Bit 0: CLK32K_GLITCH_BYPASS */
/* Description: 32K 时钟滤毛刺 BYPASS： 
1：使能对 32K 时钟的滤毛刺功能；
0：不使能对 32K 时钟的滤毛刺功能
*/
#define CPR_AO_CTL_CLK32K_CLK32K_GLITCH_BYPASS_Pos (0UL) 
#define CPR_AO_CTL_CLK32K_CLK32K_GLITCH_BYPASS_Msk (0x1UL << CPR_AO_CTL_CLK32K_CLK32K_GLITCH_BYPASS_Pos) 
#define CPR_AO_CTL_CLK32K_CLK32K_GLITCH_BYPASS_Disable (0UL)
#define CPR_AO_CTL_CLK32K_CLK32K_GLITCH_BYPASS_Enable (1UL) 

/* Bit 1: OSC32K_RESTART */
/* Description: OSC32K 晶振控制
*/
#define CPR_AO_CTL_CLK32K_OSC32K_RESTART_Pos (1UL) 
#define CPR_AO_CTL_CLK32K_OSC32K_RESTART_Msk (0x1UL << CPR_AO_CTL_CLK32K_OSC32K_RESTART_Pos) 
#define CPR_AO_CTL_CLK32K_OSC32K_RESTART_Disable (0UL)
#define CPR_AO_CTL_CLK32K_OSC32K_RESTART_Enable (1UL) 

/* Bit 2..4: OSC32K_DS */
/* Description: OSC32K 晶振控制
*/
#define CPR_AO_CTL_CLK32K_OSC32K_DS_Pos (2UL) 
#define CPR_AO_CTL_CLK32K_OSC32K_DS_Msk (0x7UL << CPR_AO_CTL_CLK32K_OSC32K_DS_Pos) 

/* Bit 8..9: OSC32K_SEL */
/* Description: 32K 时钟源选择：
00：由 mclk_chip_i 分频得到
01：来自 clk32kin
10：来自 clk32k_rc_i
11：由 mclk_chip_i 分频得到
*/
#define CPR_AO_CTL_CLK32K_OSC32K_SEL_Pos (8UL) 
#define CPR_AO_CTL_CLK32K_OSC32K_SEL_Msk (0x3UL << CPR_AO_CTL_CLK32K_OSC32K_SEL_Pos) 
#define CPR_AO_CTL_CLK32K_OSC32K_SEL_mclk_chip_i (0UL)
#define CPR_AO_CTL_CLK32K_OSC32K_SEL_clk32kin (1UL)
#define CPR_AO_CTL_CLK32K_OSC32K_SEL_clk32k_rc_i (2UL)


/* Register: CPR_AO_REG1 */
/* Offse: 0x158 */
/* Description :
*/

/* Bit 22: RAM_RETATION_EN*/
/* Description: ram retation使能，1有效，大数字断电时候retation memory
该寄存器为1时，大数字可以关闭，LDO_DIG不会关闭，提供memory retation的供电。
*/
#define CPR_AO_REG1_RAM_RETATION_EN_Pos (22UL) 
#define CPR_AO_REG1_RAM_RETATION_EN_Msk (0x1UL << CPR_AO_REG1_RAM_RETATION_EN_Pos) 
#define CPR_AO_REG1_RAM_RETATION_EN_Disable (0UL)
#define CPR_AO_REG1_RAM_RETATION_EN_Enable (1UL) 

/* Bit 23: BB_CORELDO_DCDC_SW_SOFT*/
/* Description: 
BB_CORELDO_DCDC_SW软件模式的配置值。
*/
#define CPR_AO_REG1_BB_CORELDO_DCDC_SW_SOFT_Pos (23UL) 
#define CPR_AO_REG1_BB_CORELDO_DCDC_SW_SOFT_Msk (0x1UL << CPR_AO_REG1_BB_CORELDO_DCDC_SW_SOFT_Pos) 
#define CPR_AO_REG1_BB_CORELDO_DCDC_SW_SOFT_Disable (0UL)
#define CPR_AO_REG1_BB_CORELDO_DCDC_SW_SOFT_Enable (1UL) 

/* Bit 24: BB_CORELDO_DCDC_SW_SOFTSEL*/
/* Description: 
0: BB_CORELDO_DCDC_SW由硬件自动开关，在睡眠时候自动关断
1：BB_CORELDO_DCDC_SW_MUX由BB_CORELDO_DCDC_SW_SOFT配置
*/
#define CPR_AO_REG1_BB_CORELDO_DCDC_SW_SOFTSEL_Pos (24UL) 
#define CPR_AO_REG1_BB_CORELDO_DCDC_SW_SOFTSEL_Msk (0x1UL << CPR_AO_REG1_BB_CORELDO_DCDC_SW_SOFTSEL_Pos) 
#define CPR_AO_REG1_BB_CORELDO_DCDC_SW_SOFTSEL_HW (0UL)
#define CPR_AO_REG1_BB_CORELDO_DCDC_SW_SOFTSEL_SW (1UL) 

/* Bit 25: BB_CORELDO_DCDC_SW_MUX*/
/* Description: 
BB_CORELDO_DCDC_SW_MUX
*/
#define CPR_AO_REG1_BB_CORELDO_DCDC_SW_MUX_Pos (25UL) 
#define CPR_AO_REG1_BB_CORELDO_DCDC_SW_MUX_Msk (0x1UL << CPR_AO_REG1_BB_CORELDO_DCDC_SW_MUX_Pos) 
#define CPR_AO_REG1_BB_CORELDO_DCDC_SW_MUX_Disable (0UL)
#define CPR_AO_REG1_BB_CORELDO_DCDC_SW_MUX_Enable (1UL) 

/* Bit 26: RETMEM0_ISO_EN */
/* Description: 
*/

#define CPR_AO_REG1_RETMEM1_ISO_EN_Pos (26UL) 
#define CPR_AO_REG1_RETMEM1_ISO_EN_Msk (0x1UL << CPR_AO_REG1_RETMEM1_ISO_EN_Pos) 
#define CPR_AO_REG1_RETMEM1_ISO_EN_Disable (0UL)
#define CPR_AO_REG1_RETMEM1_ISO_EN_Enable (1UL) 


/* Bit 27: RETMEM1_ISO_EN */
/* Description: 
*/

#define CPR_AO_REG1_RETMEM0_ISO_EN_Pos (27UL) 
#define CPR_AO_REG1_RETMEM0_ISO_EN_Msk (0x1UL << CPR_AO_REG1_RETMEM0_ISO_EN_Pos) 
#define CPR_AO_REG1_RETMEM0_ISO_EN_Disable (0UL)
#define CPR_AO_REG1_RETMEM0_ISO_EN_Enable (1UL) 

/* Bit 28..31: DA_BG_RF_RCT */
/* Description: 
DA_BG_RF_RCT，晶体控制寄存器？？
*/
#define CPR_AO_REG1_DA_BG_RF_RCT_Pos (28UL) 
#define CPR_AO_REG1_DA_BG_RF_RCT_Msk (0xFUL << CPR_AO_REG1_DA_BG_RF_RCT_Pos) 









/*lint --flb "Leave library region" */
#endif
