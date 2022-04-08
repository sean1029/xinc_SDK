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

/* Register: CPR_RF_AON */
/* Offse: 0x30 */
/* Description :
*/

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


/* Register: DCDC_CTRL0 */
/* Offse: 0x54 */
/* Description :
*/

/* Bit 0..1: DCDC_PDRSLOW */
/* Description: 
*/
#define CPR_DCDC_CTRL0_PDRSLOW_Pos (0UL) 
#define CPR_DCDC_CTRL0_PDRSLOW_Msk (0x3UL << CPR_DCDC_CTRL0_PDRSLOW_Pos) 

/* Bit 2: DCDC_CL_CTRL */
/* Description: 
*/
#define CPR_DCDC_CTRL0_CL_CTR_Pos (2UL) 
#define CPR_DCDC_CTRL0_CL_CTR_Msk (0x1UL << CPR_DCDC_CTRL0_CL_CTR_Pos) 

/* Bit 3: DCDC_VOUTDOWM */
/* Description: 
dcdc寄存器，控制dcdc输出下拉到地，1下拉到地，默认0
*/
#define CPR_DCDC_CTRL0_VOUTDOWM_Pos (3UL) 
#define CPR_DCDC_CTRL0_VOUTDOWM_Msk (0x1UL << CPR_DCDC_CTRL0_VOUTDOWM_Pos) 
#define CPR_DCDC_CTRL0_VOUTDOWM_Disable (0UL)
#define CPR_DCDC_CTRL0_VOUTDOWM_Enable (1UL) 

/* Bit 4: DCDC_EN_SOFT */
/* Description: 
  dcdc  en信号，1 work，硬件控制，默认0
*/
#define CPR_DCDC_CTRL0_EN_SOFT_Pos (4UL) 
#define CPR_DCDC_CTRL0_EN_SOFT_Msk (0x1UL << CPR_DCDC_CTRL0_EN_SOFT_Pos) 
#define CPR_DCDC_CTRL0_EN_SOFT_Disable (0UL)
#define CPR_DCDC_CTRL0_EN_SOFT_HW (1UL) 


/* Bit 5: DCDC_BG_EN_SOFT */
/* Description: 
dcdc bandgap en信号，1 work，要比dcdc_en早一个32K周期，硬件控制
，默认0
*/
#define CPR_DCDC_CTRL0_BG_EN_SOFT_Pos (5UL) 
#define CPR_DCDC_CTRL0_BG_EN_SOFT_Msk (0x1UL << CPR_DCDC_CTRL0_BG_EN_SOFT_Pos) 
#define CPR_DCDC_CTRL0_BG_EN_SOFT_Disable (0UL)
#define CPR_DCDC_CTRL0_BG_EN_SOFT_HW (1UL) 

/* Bit 6: DCDC_EN_SEL */
/* Description: 
dcdc ctrl select, 1:software ctrl, 0:hardware ctrl
*/
#define CPR_DCDC_CTRL0_EN_SEL_Pos (6UL) 
#define CPR_DCDC_CTRL0_EN_SEL_Msk (0x1UL << CPR_DCDC_CTRL0_EN_SEL_Pos) 
#define CPR_DCDC_CTRL0_EN_SEL_HW (0UL)
#define CPR_DCDC_CTRL0_EN_SEL_SW (1UL) 


/* Register: DCDC_CTRL1 */
/* Offse: 0x58 */
/* Description :
*/

/* Bit 0: DCDC_LP_EN */
/* Description: 
*/
#define CPR_DCDC_CTRL1_LP_EN_Pos (0UL) 
#define CPR_DCDC_CTRL1_LP_EN_Msk (0x1UL << CPR_DCDC_CTRL1_LP_EN_Pos) 
#define CPR_DCDC_CTRL1_LP_EN_Disable (0UL)
#define CPR_DCDC_CTRL1_LP_EN_Enable (1UL) 

/* Bit 1..2: DCDC_CF */
/* Description: 
 dcdc寄存器，控制时钟分频比，默认01
*/
#define CPR_DCDC_CTRL1_CF_Pos (1UL) 
#define CPR_DCDC_CTRL1_CF_Msk (0x3UL << CPR_DCDC_CTRL1_CF_Pos) 

/* Bit 3: DCDC_DISPFM */
/* Description: 
*/
#define CPR_DCDC_CTRL1_DISPFM_Pos (3UL) 
#define CPR_DCDC_CTRL1_DISPFM_Msk (0x1UL << CPR_DCDC_CTRL1_DISPFM_Pos) 
#define CPR_DCDC_CTRL1_DISPFM_Disable (0UL)
#define CPR_DCDC_CTRL1_DISPFM_Enable (1UL) 


/* Bit 4..8: DCDC_FREQCAL */
/* Description: 
dcdc寄存器，控制时钟频率，默认01000
*/
#define CPR_DCDC_CTRL1_FREQCAL_Pos (4UL) 
#define CPR_DCDC_CTRL1_FREQCAL_Msk (0x1FUL << CPR_DCDC_CTRL1_FREQCAL_Pos) 

/* Bit 9: DCDC_ZCD_EN */
/* Description: 
*/
#define CPR_DCDC_CTRL1_ZCD_EN_Pos (9UL) 
#define CPR_DCDC_CTRL1_ZCD_EN_Msk (0x1UL << CPR_DCDC_CTRL1_ZCD_EN_Pos) 
#define CPR_DCDC_CTRL1_DISPFM_Disable (0UL)
#define CPR_DCDC_CTRL1_DISPFM_Enable (1UL) 

/* Bit 10: DCDC_DEADTIME */
/* Description: 
*/
#define CPR_DCDC_CTRL1_DEADTIME_Pos (10UL) 
#define CPR_DCDC_CTRL1_DEADTIME_Msk (0x1UL << CPR_DCDC_CTRL1_DEADTIME_Pos) 
#define CPR_DCDC_CTRL1_DEADTIME_Disable (0UL)
#define CPR_DCDC_CTRL1_DEADTIME_Enable (1UL) 

/* Bit 11..16: DCDC_STBOP */
/* Description: 
*/
#define CPR_DCDC_CTRL1_STBOP_Pos (11UL) 
#define CPR_DCDC_CTRL1_STBOP_Msk (0x1FUL << CPR_DCDC_CTRL1_STBOP_Pos) 

/* Bit 17..21: DCDC_CAL */
/* Description: 
 dcdc寄存器，微调输出电压，默认10000
*/
#define CPR_DCDC_CTRL1_CAL_Pos (17UL) 
#define CPR_DCDC_CTRL1_CAL_Msk (0x1FUL << CPR_DCDC_CTRL1_CAL_Pos) 

/* Bit 22..26: DCDC_VREF_CAL */
/* Description: 
  dcdc寄存器，微调参考电压，默认10000
*/
#define CPR_DCDC_CTRL1_VREF_CAL_Pos (22UL) 
#define CPR_DCDC_CTRL1_VREF_CAL_Msk (0x1FUL << CPR_DCDC_CTRL1_VREF_CAL_Pos) 

/* Bit 27: DCDC_DCM */
/* Description: 
  dcdc寄存器，微调参考电压，默认10000
*/
#define CPR_DCDC_CTRL1_DCM_Pos (27UL) 
#define CPR_DCDC_CTRL1_DCM_Msk (0x1UL << CPR_DCDC_CTRL1_DCM_Pos) 
#define CPR_DCDC_CTRL1_DCM_Disable (0UL)
#define CPR_DCDC_CTRL1_DCM_Enable (1UL) 

/* Bit 28..30: DCDC_CTRL */
/* Description: 
  dcdc寄存器，控制输出电压，默认100
*/
#define CPR_DCDC_CTRL1_CTRL_Pos (28UL) 
#define CPR_DCDC_CTRL1_CTRL_Msk (0x7UL << CPR_DCDC_CTRL1_CTRL_Pos) 



/* Register: AUDIO_ADC_CTRL0 */
/* Offse: 0x5C */
/* Description :
*/

/* Bit 0..4: RESADJ */
/* Description: 
 bias寄存器控制信号
*/
#define CPR_AUDIO_ADC_CTRL0_RESADJ_Pos (0UL) 
#define CPR_AUDIO_ADC_CTRL0_RESADJ_Msk (0x1FUL << CPR_AUDIO_ADC_CTRL0_RESADJ_Pos) 

/* Bit 5..8: VOLR */
/* Description:
pga第2级增益 
*/
#define CPR_AUDIO_ADC_CTRL0_VOLR_Pos (5UL) 
#define CPR_AUDIO_ADC_CTRL0_VOLR_Msk (0xFUL << CPR_AUDIO_ADC_CTRL0_VOLR_Pos) 

/* Bit 9: MICGAIN */
/* Description:
pga第1级增益 
*/
#define CPR_AUDIO_ADC_CTRL0_MICGAIN_Pos (9UL) 
#define CPR_AUDIO_ADC_CTRL0_MICGAIN_Msk (0x1UL << CPR_AUDIO_ADC_CTRL0_MICGAIN_Pos) 
#define CPR_AUDIO_ADC_CTRL0_MICGAIN_Disable (0UL)
#define CPR_AUDIO_ADC_CTRL0_MICGAIN_Enable (1UL) 

/* Bit 10: VBCTRL_L */
/* Description:
输出micbias寄存器控制信号
*/
#define CPR_AUDIO_ADC_CTRL0_VBCTRL_L_Pos (10UL) 
#define CPR_AUDIO_ADC_CTRL0_VBCTRL_L_Msk (0x1UL << CPR_AUDIO_ADC_CTRL0_VBCTRL_L_Pos) 
#define CPR_AUDIO_ADC_CTRL0_VBCTRL_L_Disable (0UL)
#define CPR_AUDIO_ADC_CTRL0_VBCTRL_L_Enable (1UL) 

/* Bit 11: PDVBIAS */
/* Description:
mic bias PD信号
*/
#define CPR_AUDIO_ADC_CTRL0_PDVBIAS_Pos (11UL) 
#define CPR_AUDIO_ADC_CTRL0_PDVBIAS_Msk (0x1UL << CPR_AUDIO_ADC_CTRL0_PDVBIAS_Pos) 
#define CPR_AUDIO_ADC_CTRL0_PDVBIAS_Disable (0UL)
#define CPR_AUDIO_ADC_CTRL0_PDVBIAS_Enable (1UL) 

/* Bit 12: PDSDMR */
/* Description:
sdm PD信号
*/
#define CPR_AUDIO_ADC_CTRL0_PDSDMR_Pos (12UL) 
#define CPR_AUDIO_ADC_CTRL0_PDSDMR_Msk (0x1UL << CPR_AUDIO_ADC_CTRL0_PDSDMR_Pos) 
#define CPR_AUDIO_ADC_CTRL0_PDSDMR_Disable (0UL)
#define CPR_AUDIO_ADC_CTRL0_PDSDMR_Enable (1UL) 

/* Bit 13: PDPGAR */
/* Description:
pga PD信号
*/
#define CPR_AUDIO_ADC_CTRL0_PDPGAR_Pos (13UL) 
#define CPR_AUDIO_ADC_CTRL0_PDPGAR_Msk (0x1UL << CPR_AUDIO_ADC_CTRL0_PDPGAR_Pos) 
#define CPR_AUDIO_ADC_CTRL0_PDPGAR_Disable (0UL)
#define CPR_AUDIO_ADC_CTRL0_PDPGAR_Enable (1UL) 

/* Bit 14: PDMIC */
/* Description:
mic输入buf PD信号
*/
#define CPR_AUDIO_ADC_CTRL0_PDMIC_Pos (14UL) 
#define CPR_AUDIO_ADC_CTRL0_PDMIC_Msk (0x1UL << CPR_AUDIO_ADC_CTRL0_PDMIC_Pos) 
#define CPR_AUDIO_ADC_CTRL0_PDMIC_Disable (0UL)
#define CPR_AUDIO_ADC_CTRL0_PDMIC_Enable (1UL) 

/* Bit 15: PDCHG */
/* Description:
模拟内部寄存器控制信号
*/
#define CPR_AUDIO_ADC_CTRL0_PDCHG_Pos (15UL) 
#define CPR_AUDIO_ADC_CTRL0_PDCHG_Msk (0x1UL << CPR_AUDIO_ADC_CTRL0_PDCHG_Pos) 
#define CPR_AUDIO_ADC_CTRL0_PDCHG_Disable (0UL)
#define CPR_AUDIO_ADC_CTRL0_PDCHG_Enable (1UL) 



/* Bit 16: PDBIAS */
/* Description:
adc bias PD信号
*/
#define CPR_AUDIO_ADC_CTRL0_PDBIAS_Pos (16UL) 
#define CPR_AUDIO_ADC_CTRL0_PDBIAS_Msk (0x1UL << CPR_AUDIO_ADC_CTRL0_PDBIAS_Pos) 
#define CPR_AUDIO_ADC_CTRL0_PDBIAS_Disable (0UL)
#define CPR_AUDIO_ADC_CTRL0_PDBIAS_Enable (1UL) 



/* Bit 17: PD_USB */
/* Description:
usb phy Power down
*/
#define CPR_AUDIO_ADC_CTRL0_PD_USB_Pos (17UL) 
#define CPR_AUDIO_ADC_CTRL0_PD_USB_Msk (0x1UL << CPR_AUDIO_ADC_CTRL0_PD_USB_Pos) 
#define CPR_AUDIO_ADC_CTRL0_PD_USB_Disable (0UL)
#define CPR_AUDIO_ADC_CTRL0_PD_USB_Enable (1UL) 



/* Register: CPR_AOCLKEN_GRCTL */
/* Offse: 0x7C */
/* Description :USB AUDIO 2.4G 相关控制寄存器
 * AO 域时钟使能寄存器
 * 高 16BITS 写屏蔽低 16BITS
*/

#define CPR_AOCLKEN_GRCTL_MASK_OFFSET (16UL)

/* Bit 0: TIMER_AO_PCLK_EN */
/* Description: timer ao 总线时钟使能
0：不使能时钟
1：使能时钟
*/
#define CPR_AOCLKEN_GRCTL_TIMER_AO_PCLK_EN_Pos (0UL) 
#define CPR_AOCLKEN_GRCTL_TIMER_AO_PCLK_EN_Msk (0x1UL << CPR_AOCLKEN_GRCTL_TIMER_AO_PCLK_EN_Pos) 
#define CPR_AOCLKEN_GRCTL_TIMER_AO_PCLK_EN_Disable (0UL)
#define CPR_AOCLKEN_GRCTL_TIMER_AO_PCLK_EN_Enable (1UL) 


/* Bit 1: RTC_CLK_EN */
/* Description: RTC_CLK 时钟使能
0：不使能时钟
1：使能时钟
*/
#define CPR_AOCLKEN_GRCTL_RTC_CLK_EN_Pos (1UL) // /*!< Position of RTC_CLK_EN field. */
#define CPR_AOCLKEN_GRCTL_RTC_CLK_EN_Msk (0x1UL << CPR_AOCLKEN_GRCTL_RTC_CLK_EN_Pos) 
#define CPR_AOCLKEN_GRCTL_RTC_CLK_EN_Disable (0UL) /*!< 0：不使能时钟 */
#define CPR_AOCLKEN_GRCTL_RTC_CLK_EN_Enable (1UL) /*!< 1：使能时钟*/

/* Bit 2: TIMER0_AO_PCLK_EN */
/* Description: timer0 ao 总线时钟使能
0：不使能时钟
1：使能时钟
*/
#define CPR_AOCLKEN_GRCTL_TIMER0_AO_PCLK_EN_Pos (2UL) 
#define CPR_AOCLKEN_GRCTL_TIMER0_AO_PCLK_EN_Msk (0x1UL << CPR_AOCLKEN_GRCTL_TIMER0_AO_PCLK_EN_Pos) 
#define CPR_AOCLKEN_GRCTL_TIMER0_AO_PCLK_EN_Disable (0UL)
#define CPR_AOCLKEN_GRCTL_TIMER0_AO_PCLK_EN_Enable (1UL) 

/* Bit 3: TIMER1_AO_PCLK_EN */
/* Description: timer1 ao 总线时钟使能
0：不使能时钟
1：使能时钟
*/
#define CPR_AOCLKEN_GRCTL_TIMER1_AO_PCLK_EN_Pos (3UL) 
#define CPR_AOCLKEN_GRCTL_TIMER1_AO_PCLK_EN_Msk (0x1UL << CPR_AOCLKEN_GRCTL_TIMER1_AO_PCLK_EN_Pos) 
#define CPR_AOCLKEN_GRCTL_TIMER1_AO_PCLK_EN_Disable (0UL)
#define CPR_AOCLKEN_GRCTL_TIMER1_AO_PCLK_EN_Enable (1UL) 

/* Bit 4: TIMER_AO_RSTN */
/* Description: timer_aon软复位，0复位
0：复位
1：不复位
*/
#define CPR_AOCLKEN_GRCTL_TIMER_AO_RSTN_Pos (4UL) 
#define CPR_AOCLKEN_GRCTL_TIMER_AO_RSTN_Msk (0x1UL << CPR_AOCLKEN_GRCTL_TIMER1_AO_PCLK_EN_Pos) 
#define CPR_AOCLKEN_GRCTL_TIMER_AO_RSTN_Disable (RSTN_INVALID)
#define CPR_AOCLKEN_GRCTL_TIMER_AO_RSTN_Enable (RSTN_VALID) 

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
