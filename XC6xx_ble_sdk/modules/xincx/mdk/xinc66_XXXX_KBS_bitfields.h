/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef __XINC66_xxxx_KBS_BITS_H
#define __XINC66_xxxx_KBS_BITS_H

/*lint ++flb "Enter library region" */

/* Peripheral: KBS */
/* Description: KBS 接口模块 */

/* KBS Base Addr: 0x40015000 */

/* Register: KBS_CTL */
/* Description: 控制寄存器 */
/* Offset   :0x00             */

/* Bit 0 : MTXKEY_EN */
/* Description: 
键盘矩阵使能：
1：使能键盘矩阵；
0：不使能键盘矩阵
若该 bit 无效则键盘矩阵所有状 态回复到初始状态，
等同于内部 复位
*/
#define KBS_KBS_CTL_MTXKEY_EN_Pos (0UL) // /*!< Position of MTXKEY_EN field. */
#define KBS_KBS_CTL_MTXKEY_EN_Msk (0x1UL << KBS_KBS_CTL_MTXKEY_EN_Pos)  
#define KBS_KBS_CTL_MTXKEY_EN_Enable (1UL) 
#define KBS_KBS_CTL_MTXKEY_EN_Disable (0UL) 

/* Bit 1 : MTXKEY_AUTOSCAN_EN */
/* Description: 
键盘矩阵自动扫描使能：
1：使能键盘矩阵自动扫描；
0：不使能键盘矩阵自动扫描
*/
#define KBS_KBS_CTL_MTXKEY_AUTOSCAN_EN_Pos (1UL) // /*!< Position of MTXKEY_AUTOSCAN_EN field. */
#define KBS_KBS_CTL_MTXKEY_AUTOSCAN_EN_Msk (0x1UL << KBS_KBS_CTL_MTXKEY_AUTOSCAN_EN_Pos)  
#define KBS_KBS_CTL_MTXKEY_AUTOSCAN_EN_Enable (1UL) 
#define KBS_KBS_CTL_MTXKEY_AUTOSCAN_EN_Disable (0UL) 

/* Bit 2 : MTXKEY_RPRS_RAW_EN */
/* Description: 
键盘矩阵自动扫描使能：
1：使能键盘矩阵自动扫描；
0：不使能键盘矩阵自动扫描
*/
#define KBS_KBS_CTL_MTXKEY_RPRS_RAW_EN_Pos (2UL) // /*!< Position of MTXKEY_RPRS_RAW_EN field. */
#define KBS_KBS_CTL_MTXKEY_RPRS_RAW_EN_Msk (0x1UL << KBS_KBS_CTL_MTXKEY_RPRS_RAW_EN_Pos)  
#define KBS_KBS_CTL_MTXKEY_RPRS_RAW_EN_Enable (1UL) 
#define KBS_KBS_CTL_MTXKEY_RPRS_RAW_EN_Disable (0UL) 

/* Bit 3 : MTXKEY_LPRS_RAW_EN */
/* Description: 
键盘矩阵长按键使能：
1：使能键盘矩阵长按键功能；
0：不使能键盘矩阵长按键功能
*/
#define KBS_KBS_CTL_MTXKEY_LPRS_RAW_EN_Pos (3UL) // /*!< Position of MTXKEY_LPRS_RAW_EN field. */
#define KBS_KBS_CTL_MTXKEY_LPRS_RAW_EN_Msk (0x1UL << KBS_KBS_CTL_MTXKEY_LPRS_RAW_EN_Pos)  
#define KBS_KBS_CTL_MTXKEY_LPRS_RAW_EN_Enable (1UL) 
#define KBS_KBS_CTL_MTXKEY_LPRS_RAW_EN_Disable (0UL) 

/* Bit 4 : MTXKEY_PULL_CFG */
/* Description: 
键盘矩阵上下拉配置：
1：外部输入管脚需配置为上拉；
0：外部输入管脚需配置为下拉
*/
#define KBS_KBS_CTL_MTXKEY_PULL_CFG_Pos (4UL) // /*!< Position of MTXKEY_PULL_CFG field. */
#define KBS_KBS_CTL_MTXKEY_PULL_CFG_Msk (0x1UL << KBS_KBS_CTL_MTXKEY_PULL_CFG_Pos)  
#define KBS_KBS_CTL_MTXKEY_PULL_CFG_Pull (1UL) 
#define KBS_KBS_CTL_MTXKEY_PULL_CFG_PullDown (0UL) 

/* Bit 5 : MKO_EN_SOFTEN */
/* Description: 
MKO输出状态配置：
1：MKO固定为输出
0：MKO[i]只有在扫描第i行时为输出，其他状态下为输入。
i=0~7
*/
#define KBS_KBS_CTL_MKO_EN_SOFTEN_Pos (5UL) // /*!< Position of MKO_EN_SOFTEN field. */
#define KBS_KBS_CTL_MKO_EN_SOFTEN_Msk (0x1UL << KBS_KBS_CTL_MKO_EN_SOFTEN_Pos)  
#define KBS_KBS_CTL_MKO_EN_SOFTEN_Enable (1UL) 
#define KBS_KBS_CTL_MKO_EN_SOFTEN_Disable (0UL) 

/* Bit 6 : MKO_PULLEN_ENABLE */
/* Description: 
键盘矩阵上下拉配置：
MKO管脚的自动上下拉使能
*/
#define KBS_KBS_CTL_MKO_PULLEN_Pos (6UL) // /*!< Position of MKO_PULLEN_ENABLE field. */
#define KBS_KBS_CTL_MKO_PULLEN_Msk (0x1UL << KBS_KBS_CTL_MKO_PULLEN_Pos)  
#define KBS_KBS_CTL_MKO_PULLEN_Enable (1UL) 
#define KBS_KBS_CTL_MKO_PULLEN_Disable (0UL) 

/* Register: KBS_MASK */
/* Description: 按键 MASK 寄存器 
 屏蔽对应的键盘矩阵的行和列。
 若键盘矩阵的对应行或列被屏蔽，对应键值为无效值 0。*/
/* Offset   :0x04             */

/* Bit 0..7 : MTXKEY_ROW_MASK */
/* Description: 
键盘矩阵行 MASK 设置
设置为 1 则屏蔽键盘矩阵的对应行 驱动输出。
如若设置为 0x1，则屏蔽 键盘矩阵输出管脚 kbs_mko[0]
*/
#define KBS_KBS_MASK_MTXKEY_ROW_MASK_Pos (0UL) // /*!< Position of MTXKEY_ROW_MASK field. */
#define KBS_KBS_MASK_MTXKEY_ROW_MASK_Msk (0xFFUL << KBS_KBS_MASK_MTXKEY_ROW_MASK_Pos)  

/* Bit 8..25 : MTXKEY_ROW_MASK */
/* Description: 
键盘矩阵列 MASK 设置
设置为 1 则屏蔽键盘矩阵的对应列 输入。
如若设置为 0x1，则屏蔽键盘 矩阵输入管脚 kbs_mki[0]
*/
#define KBS_KBS_MASK_MTXKEY_COL_MASK_Pos (8UL) // /*!< Position of MTXKEY_COL_MASK field. */
#define KBS_KBS_MASK_MTXKEY_COL_MASK_Msk (0x3FFFFUL << KBS_KBS_MASK_MTXKEY_COL_MASK_Pos)  

/* Register: KBS_DETECT_INTVAL */
/* Description: 按键和释放键间隔寄存器 
 配置按键和释放键检测间隔，以 0.5ms 为基本单位，
 检测到按键或释放键的操作稳定 持续到寄存器配置的时间后，
 才认为有按键或释放键操作发生。*/
/* Offset   :0x08             */

/* Bit 0..7 : MTXKEY_PRS_INTVAL */
/* Description: 
键盘矩阵按键检测间隔
以 0.5ms 为单位，
如配置为 0x2 则键盘矩阵按键检测 间隔为 1ms。
*/
#define KBS_DETECT_INTVAL_MTXKEY_PRS_INTVAL_Pos (0UL) // /*!< Position of MTXKEY_PRS_INTVAL field. */
#define KBS_DETECT_INTVAL_MTXKEY_PRS_INTVAL_Msk (0xFFUL << KBS_DETECT_INTVAL_MTXKEY_PRS_INTVAL_Pos)   

/* Bit 8..15 : MTXKEY_RLS_INTVAL */
/* Description: 
键盘矩阵释放键检测间隔
以 0.5ms 为单位，如配置为 0x2
则键盘矩阵释放键检测间隔为1ms。
*/
#define KBS_DETECT_INTVAL_MTXKEY_RLS_INTVAL_Pos (0UL) // /*!< Position of MTXKEY_RLS_INTVAL field. */
#define KBS_DETECT_INTVAL_MTXKEY_RLS_INTVAL_Msk (0xFFUL << KBS_DETECT_INTVAL_MTXKEY_RLS_INTVAL_Pos)  


/* Register: KBS_DBC_INTVAL */
/* Description: Debounce 间隔寄存器
 配置按键和释放键检测间隔，以 0.5ms 为基本单位，
 检测到按键或释放键的操作稳定 持续到寄存器配置的时间后，
 才认为有按键或释放键操作发生。*/
/* Offset   :0x0C             */

/* Bit 0..7 : MTXKEY_DBC_INTVAL */
/* Description:键盘矩阵 debounce 间隔 不能配置为 0，以 1ms 为单 位 ， 
如 配 置 为 0x5 则 debounce 间隔为 5ms。
*/
#define KBS_DBC_INTVAL_MTXKEY_DBC_INTVAL_Pos (0UL) // /*!< Position of MTXKEY_DBC_INTVAL field. */
#define KBS_DBC_INTVAL_MTXKEY_DBC_INTVAL_Msk (0xFFUL << KBS_DBC_INTVAL_MTXKEY_DBC_INTVAL_Pos)  


/* Register: KBS_LPRS_INTVAL */
/* Description: 长按键间隔寄存器*/
/* Offset   :0x10             */

/* Bit 0..9 : KBS_RPRS_INTVAL */
/* Description:重复按键间隔配置 配置检测重复按键的时间间隔，
 当按键按下并保持不变超过该间 隔值时认定按键被按下，产生重 复按键中断。
不能配置为 0，以 1ms 为单位， 如配置为 0x100 则重复按键间隔 为 256ms。
注意：对于键盘矩阵，重复按键间隔不 能小于按键检测间隔与 debounce 间隔之和。
*/
#define KBS_LPRS_INTVAL_KBS_RPRS_INTVAL_Pos (0UL) // /*!< Position of KBS_RPRS_INTVAL field. */
#define KBS_LPRS_INTVAL_KBS_RPRS_INTVAL_Msk (0x3FFUL << KBS_DBC_INTVAL_MTXKEY_DBC_INTVAL_Pos)  

/* Bit 10..15 : KBS_LPRS_INTVAL */
/* Description:长按键间隔配置 配置检测长按键的时间间隔，
当 按键按下并保持不变超过该间隔 值时，认定按键被长按下，产生 长按键中断。
不能配置为 0，以重复按键计数 间隔为单位。
如配置为 0x100 且重复按键间隔 配置为 0x8，则实际的长按键间 隔为 2048ms。。
*/
#define KBS_LPRS_INTVAL_KBS_LPRS_INTVAL_Pos (10UL) // /*!< Position of KBS_LPRS_INTVAL field. */
#define KBS_LPRS_INTVAL_KBS_LPRS_INTVAL_Msk (0x3FUL << KBS_LPRS_INTVAL_KBS_LPRS_INTVAL_Pos)  

/* Register: KBS_MTXKEY_MANUAL_ROWOUT */
/* Description: 手动扫描的配置输出的有效行，
自动扫描使能时该寄存器无效。*/
/* Offset   :0x14             */

/* Bit 0..7 : ROW_SCAN_DRIVE */
/* Description:手动扫描行驱动输出
8bits 中每次仅能有 1 个 bits 有 效，即每次扫描 1 行，
否则扫描 结果无效。自动扫描使能时该寄 存器无效。该寄存器手动扫描时 
驱 动 键 盘 矩 阵 管 脚 kbs_mko[7:0]： 0x01：扫描驱动第 0 行；
0x02：扫描驱动第 1 行；
0x04：扫描驱动第 2 行；
0x08：扫描驱动第 3 行；
0x10：扫描驱动第 4 行；
0x20：扫描驱动第 5 行；
0x40：扫描驱动第 6 行；
0x80：扫描驱动第 7 行； 其他值：无效*/
#define KBS_MTXKEY_MANUAL_ROWOUT_ROW_SCAN_DRIVE_Pos (0UL) // /*!< Position of ROW_SCAN_DRIVE field. */
#define KBS_MTXKEY_MANUAL_ROWOUT_ROW_SCAN_DRIVE_Msk (0xFFUL << KBS_MTXKEY_MANUAL_ROWOUT_ROW_SCAN_DRIVE_Pos)  

/* Bit 8 : MANUAL_RLS */
/* Description:手动扫描释放键指示：
1：所有按键已被释放；
0：有键按下 当手动扫描检测到所有按键均释 放时，
将该位置 1，通知 KBS 所 有按键释放，不再产生按键中断。*/
#define KBS_MTXKEY_MANUAL_ROWOUT_MANUAL_RLS_Pos (8UL) // /*!< Position of MANUAL_RLS field. */
#define KBS_MTXKEY_MANUAL_ROWOUT_MANUAL_RLS_Msk (0x1UL << KBS_MTXKEY_MANUAL_ROWOUT_MANUAL_RLS_Pos)  
#define KBS_MTXKEY_MANUAL_ROWOUT_MANUAL_RLS_AllRlease (1UL)
#define KBS_MTXKEY_MANUAL_ROWOUT_MANUAL_RLS_NoRlease (0UL)

/* Register: KBS_MTXKEY_MANUAL_COLIN */
/* Description:  键盘矩阵手动扫描列输入寄存器*/
/* Offset   :0x18             */

/* Bit 0..17 : COL_SCAN_VALUE */
/* Description:手动扫描列线状态输入 
指示键盘矩阵当前驱动行对应的列线输入
管脚kbs_mki[17:0] 的 值。。*/
#define KBS_MTXKEY_MANUAL_COLIN_COL_SCAN_VALUE_Pos (0UL) // /*!< Position of ROW_SCAN_DRIVE field. */
#define KBS_MTXKEY_MANUAL_COLIN_COL_SCAN_VALUE_Msk (0x3FFFFUL << KBS_MTXKEY_MANUAL_COLIN_COL_SCAN_VALUE_Pos)  

/* Register: KBS_MTXKEY_FIFO */
/* Description:  键值队列 FIFO 接口，读出 FIFO 的数据以及 FIFO 的标志位。
读 1 次该寄存器，则从 键值队列 FIFO 中 pop 一次 FIFO 的键值。*/
/* Offset   :0x1C             */

/* Bit 0..7 : KEY_VALUE */
/* Description:按键坐标
按键在8x18矩阵键盘中的位置*/

#define KBS_MTXKEY_FIFO_KEY_VALUE_Pos (0UL) // /*!< Position of KEY_VALUE field. */
#define KBS_MTXKEY_FIFO_KEY_VALUE_Msk (0xFFUL << KBS_MTXKEY_FIFO_KEY_VALUE_Pos)  

/* Bit 9 : FIFO_AL_EMPTY */
/* Description:FIFO 近空标志：
1 有效
fifo 内部还有一个数据时近空标志有效。*/

#define KBS_MTXKEY_FIFO_AL_EMPTY_Pos (9UL) // /*!< Position of FIFO_AL_EMPTY field. */
#define KBS_MTXKEY_FIFO_AL_EMPTY_Msk (0x1UL << KBS_MTXKEY_FIFO_AL_EMPTY_Pos)  
#define KBS_MTXKEY_FIFO_AL_EMPTY_Generated (1UL)
#define KBS_MTXKEY_FIFO_AL_EMPTY_NotGenerated (0UL) 

/* Bit 10 : FIFO_EMPTY */
/* Description:FIFO 空标志：
1 有效
*/

#define KBS_MTXKEY_FIFO_EMPTY_Pos (10UL) // /*!< Position of FIFO_EMPTY field. */
#define KBS_MTXKEY_FIFO_EMPTY_Msk (0x1UL << KBS_MTXKEY_FIFO_EMPTY_Pos)  
#define KBS_MTXKEY_FIFO_EMPTY_Generated (1UL)
#define KBS_MTXKEY_FIFO_EMPTY_NotGenerated (0UL) 

/* Bit 11 : FIFO_AL_FULL */
/* Description:FIFO 近满标志：
1 有效
*/

#define KBS_MTXKEY_FIFO_AL_FULL_Pos (11UL) // /*!< Position of FIFO_AL_FULL field. */
#define KBS_MTXKEY_FIFO_AL_FULL_Msk (0x1UL << KBS_MTXKEY_FIFO_AL_FULL_Pos)  
#define KBS_MTXKEY_FIFO_AL_FULL_Generated (1UL)
#define KBS_MTXKEY_FIFO_AL_FULL_NotGenerated (0UL) 

/* Bit 12 : FIFO_FULL */
/* Description:FIFO 近满标志：
1 有效
*/

#define KBS_MTXKEY_FIFO_FULL_Pos (12UL) // /*!< Position of FIFO_FULL field. */
#define KBS_MTXKEY_FIFO_FULL_Msk (0x1UL << KBS_MTXKEY_FIFO_FULL_Pos)  
#define KBS_MTXKEY_FIFO_FULL_Generated (1UL)
#define KBS_MTXKEY_FIFO_FULL_NotGenerated (0UL) 

/* Bit 13 : FIFO_POP_ERROR */
/* Description:FIFO POP 出错状态标志：
1：FIFO 内部有错误；
0：FIFO 正常
*/

#define KBS_MTXKEY_FIFO_POP_ERROR_Pos (13UL) // /*!< Position of FIFO_POP_ERROR field. */
#define KBS_MTXKEY_FIFO_POP_ERROR_Msk (0x1UL << KBS_MTXKEY_FIFO_POP_ERROR_Pos)  
#define KBS_MTXKEY_FIFO_POP_ERROR_Generated (1UL)
#define KBS_MTXKEY_FIFO_POP_ERROR_NotGenerated (0UL) 

/* Bit 14 : FIFO_PUSH_ERROR */
/* Description:FIFO PUSH 出错状态标志：
1：FIFO 内部有错误；
0：FIFO 正常
*/

#define KBS_MTXKEY_FIFO_PUSH_ERROR_Pos (14UL) // /*!< Position of FIFO_PUSH_ERROR field. */
#define KBS_MTXKEY_FIFO_PUSH_ERROR_Msk (0x1UL << KBS_MTXKEY_FIFO_PUSH_ERROR_Pos)  
#define KBS_MTXKEY_FIFO_PUSH_ERROR_Generated (1UL)
#define KBS_MTXKEY_FIFO_PUSH_ERROR_NotGenerated (0UL) 

/* Bit 15 : FIFO_FLUSH */
/* Description:
清空 FIFO： 1：清空 FIFO;
0：无效
*/

#define KBS_MTXKEY_FIFO_FLUSH_Pos (15UL) // /*!< Position of FIFO_FLUSH field. */
#define KBS_MTXKEY_FIFO_FLUSH_Msk (0x1UL << KBS_MTXKEY_FIFO_FLUSH_Pos)  
#define KBS_MTXKEY_FIFO_FLUSH_Enable (1UL)
#define KBS_MTXKEY_FIFO_FLUSH_Disable (0UL) 


/* Register: KBS_MTXKEY_INT */
/* Description:  该寄存器为中断原始状态经过使能控制之后的状态。*/
/* Offset   :0x28            */

/* Bit 0: MTXKEY_PRS_INT */
/* Description:键盘矩阵按键中断 向本比特位写‘1’清除本 中断状态位，
同时清除相应 的中断原始状态位：
 0：没有产生键盘矩阵按键 中断； 1：产生了键盘矩阵按键中 断*/

#define KBS_MTXKEY_INT_PRS_INT_Pos (0UL) // /*!< Position of MTXKEY_PRS_INT field. */
#define KBS_MTXKEY_INT_PRS_INT_Msk (0x1UL << KBS_MTXKEY_INT_PRS_INT_Pos)  
#define KBS_MTXKEY_INT_PRS_INT_Generated (1UL)
#define KBS_MTXKEY_INT_PRS_INT_NotGenerated (0UL) 

/* Bit 1: MTXKEY_FIFO_UPDATE_INT */
/* Description:键盘矩阵键值 FIFO 更新中 断 
向本比特位写‘1’清除本 中断状态位，同时清除相应 的中断原始状态位。
注意： 该位的中断清除信号不能自 清 0，写 1 确认中断清除后， 需要再对该位写 0： 
0：没有产生键盘矩阵键值 FIFO 更新中断；
1 ： 产 生 了 键 盘 矩 阵 键值 
FIFO 更新中断*/

#define KBS_MTXKEY_INT_FIFO_UPDATE_INT_Pos (1UL) // /*!< Position of MTXKEY_FIFO_UPDATE_INT field. */
#define KBS_MTXKEY_INT_FIFO_UPDATE_INT_Msk (0x1UL << KBS_MTXKEY_INT_FIFO_UPDATE_INT_Pos)  
#define KBS_MTXKEY_INT_FIFO_UPDATE_INT_Generated (1UL)
#define KBS_MTXKEY_INT_FIFO_UPDATE_INT_NotGenerated (0UL) 

/* Bit 2: MTXKEY_RPRS_INT */
/* Description:键盘矩阵重复按键中断 向本比特位写‘1’清除本 中断状态位，
同时清除相应 的中断原始状态位：
 0：没有产生键盘矩阵重复按键中断； 
 1：产生了键盘矩阵重复按键中断*/

#define KBS_MTXKEY_INT_RPRS_INT_Pos (2UL) // /*!< Position of MTXKEY_RPRS_INT field. */
#define KBS_MTXKEY_INT_RPRS_INT_Msk (0x1UL << KBS_MTXKEY_INT_RPRS_INT_Pos)  
#define KBS_MTXKEY_INT_RPRS_INT_Generated (1UL)
#define KBS_MTXKEY_INT_RPRS_INT_NotGenerated (0UL) 

/* Bit 3: MTXKEY_LPRS_INT */
/* Description:键盘矩阵重复按键中断 向本比特位写‘1’清除本 中断状态位，
同时清除相应 的中断原始状态位：
 0：没有产生键盘矩阵重复按键中断； 
 1：产生了键盘矩阵重复按键中断*/

#define KBS_MTXKEY_INT_LPRS_INT_Pos (3UL) // /*!< Position of MTXKEY_LPRS_INT field. */
#define KBS_MTXKEY_INT_LPRS_INT_Msk (0x1UL << KBS_MTXKEY_INT_LPRS_INT_Pos)  
#define KBS_MTXKEY_INT_LPRS_INT_Generated (1UL)
#define KBS_MTXKEY_INT_LPRS_INT_NotGenerated (0UL) 


/* Bit 4: MTXKEY_RLS */
/* Description:键盘矩阵释放键中断 
向本比特位写‘1’清除本 中断状态位，同时清除相应 的中断原始状态位：
 0：没有产生键盘矩阵释放 键中断； 
 1：产生了键盘矩阵释放键 中断*/

#define KBS_MTXKEY_INT_RLS_INT_Pos (4UL) // /*!< Position of MTXKEY_RLS_INT field. */
#define KBS_MTXKEY_INT_RLS_INT_Msk (0x1UL << KBS_MTXKEY_INT_RLS_INT_Pos)  
#define KBS_MTXKEY_INT_RLS_INT_Generated (1UL)
#define KBS_MTXKEY_INT_RLS_INT_NotGenerated (0UL) 


/* Register: KBS_MTXKEY_INT_RAW */
/* Description:  中断原始状态寄存器记录中断的原始状态，当中断使能寄存器相应的控制比特为 1 时，
与之相对的原始状态会触发中断状态寄存器。*/
/* Offset   :0x2C            */

/* Bit 0: MTXKEY_PRS_INT_RAW */
/* Description:键盘矩阵按键原始中断 手动扫描时，键盘矩阵 有按键活动，该原始状 态置 1。 自动扫描时，
键盘矩阵 有按键活动且扫描完成 所有的键值后，该原始 状态置 1。*/

#define KBS_MTXKEY_INT_PRS_INT_RAW_Pos (0UL) // /*!< Position of MTXKEY_PRS_INT_RAW field. */
#define KBS_MTXKEY_INT_PRS_INT_RAW_Msk (0x1UL << KBS_MTXKEY_INT_PRS_INT_RAW_Pos)  
#define KBS_MTXKEY_INT_PRS_INT_RAW_Generated (1UL)
#define KBS_MTXKEY_INT_PRS_INT_RAW_NotGenerated (0UL) 

/* Bit 1: MTXKEY_FIFO_UPDATE_INT_RAW */
/* Description:键盘矩阵键值 FIFO 更 新原始中断 手动扫描时，该位置 0。
 自动扫描时，更新完键 值 FIFO 后，该原始状 态置 1。*/

#define KBS_MTXKEY_INT_FIFO_UPDATE_INT_RAW_Pos (1UL) // /*!< Position of MTXKEY_FIFO_UPDATE_INT_RAW field. */
#define KBS_MTXKEY_INT_FIFO_UPDATE_INT_RAW_Msk (0x1UL << KBS_MTXKEY_INT_FIFO_UPDATE_INT_RAW_Pos)  
#define KBS_MTXKEY_INT_FIFO_UPDATE_INT_RAW_Generated (1UL)
#define KBS_MTXKEY_INT_FIFO_UPDATE_INT_RAW_NotGenerated (0UL) 

/* Bit 2: MTXKEY_RPRS_INT */
/* Description:键盘矩阵重复按键原始中断 手动扫描时，
该位置 0。 自动扫描时，满足重复 按键间隔时，该原始状 态置 1。*/

#define KBS_MTXKEY_INT_RPRS_INT_RAW_Pos (2UL) // /*!< Position of MTXKEY_RPRS_INT_RAW field. */
#define KBS_MTXKEY_INT_RPRS_INT_RAW_Msk (0x1UL << KBS_MTXKEY_INT_RPRS_INT_RAW_Pos)  
#define KBS_MTXKEY_INT_RPRS_INT_RAW_Generated (1UL)
#define KBS_MTXKEY_INT_RPRS_INT_RAW_NotGenerated (0UL) 

/* Bit 3: MTXKEY_LPRS_INT */
/* Description:
键盘矩阵长按键原始中断手动扫描时，该位置 0。
自动扫描时，满足长按 键间隔时，该原始状态 置 1。*/

#define KBS_MTXKEY_INT_LPRS_INT_RAW_Pos (3UL) // /*!< Position of MTXKEY_LPRS_INT_RAW field. */
#define KBS_MTXKEY_INT_LPRS_INT_RAW_Msk (0x1UL << KBS_MTXKEY_INT_LPRS_INT_RAW_Pos)  
#define KBS_MTXKEY_INT_LPRS_INT_RAW_Generated (1UL)
#define KBS_MTXKEY_INT_LPRS_INT_RAW_NotGenerated (0UL) 


/* Bit 4: MTXKEY_RLSINT_RAW */
/* Description:
键盘矩阵释放键原始中断键盘矩阵 所有键释放时,该原始状态置 1*/

#define KBS_MTXKEY_INT_RLS_INT_RAW_Pos (4UL) // /*!< Position of MTXKEY_RLS_INT_RAW field. */
#define KBS_MTXKEY_INT_RLS_INT_RAW_Msk (0x1UL << KBS_MTXKEY_INT_RLS_INT_RAW_Pos)  
#define KBS_MTXKEY_INT_RLS_INT_RAW_Generated (1UL)
#define KBS_MTXKEY_INT_RLS_INT_RAW_NotGenerated (0UL) 


/* Register: KBS_MTXKEY_INT_EN */
/* Description:  键盘矩阵中断使能寄存器。*/
/* Offset   :0x30            */

/* Bit 0: MTXKEY_PRS_INT_EN */
/* Description:键盘矩阵按键按下中断使能：
0：中断不使能；
1：中断使能*/

#define KBS_MTXKEY_INT_PRS_INT_EN_Pos (0UL) // /*!< Position of MTXKEY_PRS_INT_EN field. */
#define KBS_MTXKEY_INT_PRS_INT_EN_Msk (0x1UL << KBS_MTXKEY_INT_PRS_INT_EN_Pos)  
#define KBS_MTXKEY_INT_PRS_INT_EN_Enable (1UL)
#define KBS_MTXKEY_INT_PRS_INT_EN_Disable (0UL) 

/* Bit 1: MTXKEY_FIFO_UPDATE_INT_EN */
/* Description:
键盘矩阵键值 FIFO 更新 中断使能：
 0：中断不使能；
1：中断使能。*/

#define KBS_MTXKEY_INT_FIFO_UPDATE_INT_EN_Pos (1UL) // /*!< Position of MTXKEY_FIFO_UPDATE_INT_EN field. */
#define KBS_MTXKEY_INT_FIFO_UPDATE_INT_EN_Msk (0x1UL << KBS_MTXKEY_INT_FIFO_UPDATE_INT_EN_Pos)  
#define KBS_MTXKEY_INT_FIFO_UPDATE_INT_EN_Enable (1UL)
#define KBS_MTXKEY_INT_FIFO_UPDATE_INT_EN_Disable (0UL) 

/* Bit 2: MTXKEY_RPRS_INT */
/* Description:
键盘矩阵重复按键中断 使能：
0：中断不使能；
1：中断使能*/

#define KBS_MTXKEY_INT_RPRS_INT_EN_Pos (2UL) // /*!< Position of MTXKEY_RPRS_INT_EN field. */
#define KBS_MTXKEY_INT_RPRS_INT_EN_Msk (0x1UL << KBS_MTXKEY_INT_RPRS_INT_EN_Pos)  
#define KBS_MTXKEY_INT_RPRS_INT_EN_Enable (1UL)
#define KBS_MTXKEY_INT_RPRS_INT_EN_Disable (0UL) 

/* Bit 3: MTXKEY_LPRS_INT */
/* Description:
键盘矩阵长按键中断使 能：
0：中断不使能；
1：中断使能*/

#define KBS_MTXKEY_INT_LPRS_INT_EN_Pos (3UL) // /*!< Position of MTXKEY_LPRS_INT_EN field. */
#define KBS_MTXKEY_INT_LPRS_INT_EN_Msk (0x1UL << KBS_MTXKEY_INT_LPRS_INT_EN_Pos)  
#define KBS_MTXKEY_INT_LPRS_INT_EN_Enable (1UL)
#define KBS_MTXKEY_INT_LPRS_INT_EN_Disable (0UL) 


/* Bit 4: MTXKEY_RLS_INT_EN */
/* Description:
键盘矩阵释放键中断使 能：
0：中断不使能；
1：中断使能*/

#define KBS_MTXKEY_INT_RLS_INT_EN_Pos (4UL) // /*!< Position of MTXKEY_RLS_INT_EN field. */
#define KBS_MTXKEY_INT_RLS_INT_EN_Msk (0x1UL << KBS_MTXKEY_INT_RLS_INT_EN_Pos)  
#define KBS_MTXKEY_INT_RLS_INT_EN_Enable (1UL)
#define KBS_MTXKEY_INT_RLS_INT_EN_Disable (0UL) 



/*lint --flb "Leave library region" */
#endif
