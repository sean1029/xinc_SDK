/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef __XINC66_xxxx_AUDIO_ADC_BITS_H
#define __XINC66_xxxx_AUDIO_ADC_BITS_H

/*lint ++flb "Enter library region" */

/* Peripheral: CDC */
/* Description: CDC */


/* CDC BASE ADDR:0x50004000UL */



/* Register: CDC_ANA_REG0 */
/* Description:cdc模拟寄存器 */
/* Offse: 0x00 */

/* Bit 0..5 : REG00_D_VOL */
/* Description: 
*/
#define CDC_CDC_ANA_REG0_REG00_D_VOL_Pos (0UL) // /*!< Position of REG00_D_VOL field. */
#define CDC_CDC_ANA_REG0_REG00_D_VOL_Msk (0x3FUL << CDC_CDC_ANA_REG0_REG00_D_VOL_Pos) 

/* Bit 6 : REG00_HPF_EN */
/* Description: 
*/
#define CDC_CDC_ANA_REG0_REG00_HPF_EN_Pos (6UL) // /*!< Position of REG00_HPF_EN field. */
#define CDC_CDC_ANA_REG0_REG00_HPF_EN_Msk (0x1UL << CDC_CDC_ANA_REG0_REG00_HPF_EN_Pos) 
#define CDC_CDC_ANA_REG0_REG00_HPF_EN_Enable (1UL)
#define CDC_CDC_ANA_REG0_REG00_HPF_EN_Disable (0UL)

/* Bit 7 : REG00_HPF_BYPASS */
/* Description: 
*/
#define CDC_CDC_ANA_REG0_REG00_HPF_BYPASS_Pos (7UL) // /*!< Position of REG00_HPF_BYPASS field. */
#define CDC_CDC_ANA_REG0_REG00_HPF_BYPASS_Msk (0x1UL << CDC_CDC_ANA_REG0_REG00_HPF_BYPASS_Pos) 
#define CDC_CDC_ANA_REG0_REG00_HPF_BYPASS_Enable (1UL)
#define CDC_CDC_ANA_REG0_REG00_HPF_BYPASS_Disable (0UL)

/* Bit 8..15 : REG01 */
/* Description: 
*/
#define CDC_CDC_ANA_REG0_REG01_Pos (8UL) // /*!< Position of REG01 field. */
#define CDC_CDC_ANA_REG0_REG01_Msk (0xFFUL << CDC_CDC_ANA_REG0_REG01_Pos) 

/* Register: CDC_RXFIFO_DATA */
/* Description:cdc FIFO寄存器 */
/* Offset   :0x20             */

/* Bit 0..23 : RXFIFO_DATA */
/* Description: 
*/
#define CDC_CDC_RXFIFO_DATA_Pos (0UL) // /*!< Position of RXFIFO_DATA field. */
#define CDC_CDC_CDC_RXFIFO_DATA_Msk (0xFFFFFUL << CDC_CDC_RXFIFO_DATA_Pos) 



/* Register: CDC_RXFIFO_STATUS */
/* Description:cdc FIFO STATUS 寄存器 */
/* Offset   :0x24             */

/* Bit 0 : RXFIFO_AEMPTY */
/* Description: 
*/
#define CDC_CDC_RXFIFO_STATUS_RXFIFO_AEMPTY_Pos (0UL) // /*!< Position of RXFIFO_AEMPTY field. */
#define CDC_CDC_RXFIFO_STATUS_RXFIFO_AEMPTY_Msk (0x1UL << CDC_CDC_RXFIFO_STATUS_RXFIFO_AEMPTY_Pos)  
#define CDC_CDC_RXFIFO_STATUS_RXFIFO_AEMPTY_Generated (1UL) /*!< RX FIFO almost empty status */
#define CDC_CDC_RXFIFO_STATUS_RXFIFO_AEMPTY_NotGenerated (0UL) /*!<  */

/* Bit 1 : RXFIFO_EMPTY */
/* Description: 
*/
#define CDC_CDC_RXFIFO_STATUS_RXFIFO_EMPTY_Pos (1UL) // /*!< Position of RXFIFO_EMPTY field. */
#define CDC_CDC_RXFIFO_STATUS_RXFIFO_EMPTY_Msk (0x1UL << CDC_CDC_RXFIFO_STATUS_RXFIFO_EMPTY_Pos)  
#define CDC_CDC_RXFIFO_STATUS_RXFIFO_EMPTY_Generated (1UL) /*!< RX FIFO  empty status */
#define CDC_CDC_RXFIFO_STATUS_RXFIFO_EMPTY_NotGenerated (0UL) /*!<  */


/* Bit 2 : RXFIFO_AFULL */
/* Description: 
*/
#define CDC_CDC_RXFIFO_STATUS_RXFIFO_AFULL_Pos (2UL) // /*!< Position of RXFIFO_AFULL field. */
#define CDC_CDC_RXFIFO_STATUS_RXFIFO_AFULL_Msk (0x1UL << CDC_CDC_RXFIFO_STATUS_RXFIFO_AFULL_Pos)  
#define CDC_CDC_RXFIFO_STATUS_RXFIFO_AFULL_Generated (1UL) /*!< RX FIFO almost full status */
#define CDC_CDC_RXFIFO_STATUS_RXFIFO_AFULL_NotGenerated (0UL) /*!<  */

/* Bit 3 : RXFIFO_FULL */
/* Description: 
*/
#define CDC_CDC_RXFIFO_STATUS_RXFIFO_FULL_Pos (3UL) // /*!< Position of RXFIFO_FULL field. */
#define CDC_CDC_RXFIFO_STATUS_RXFIFO_FULL_Msk (0x1UL << CDC_CDC_RXFIFO_STATUS_RXFIFO_FULL_Pos)  
#define CDC_CDC_RXFIFO_STATUS_RXFIFO_FULL_Generated (1UL) /*!< RX FIFO full status */
#define CDC_CDC_RXFIFO_STATUS_RXFIFO_FULL_NotGenerated (0UL) /*!<  */

/* Bit 4 : RXFIFO_RXLVL_TRIG */
/* Description: 
*/
#define CDC_CDC_RXFIFO_STATUS_RXFIFO_RXLVL_TRIG_Pos (4UL) // /*!< Position of RXFIFO_RXLVL_TRIG field. */
#define CDC_CDC_RXFIFO_STATUS_RXFIFO_RXLVL_TRIG_Msk (0x1UL << CDC_CDC_RXFIFO_STATUS_RXFIFO_RXLVL_TRIG_Pos)  
#define CDC_CDC_RXFIFO_STATUS_RXFIFO_RXLVL_TRIG_Generated (1UL) /*!< RXFIFO level trigger status */
#define CDC_CDC_RXFIFO_STATUS_RXFIFO_RXLVL_TRIG_NotGenerated (0UL) /*!<  */

/* Bit 7 : RXFIFO_FLUSH */
/* Description: 
*/
#define CDC_CDC_RXFIFO_STATUS_RXFIFO_FLUSH_Pos (7UL) // /*!< Position of RXFIFO_RXLVL_TRIG field. */
#define CDC_CDC_RXFIFO_STATUS_RXFIFO_FLUSH_Msk (0x1UL << CDC_CDC_RXFIFO_STATUS_RXFIFO_RXLVL_TRIG_Pos)  
#define CDC_CDC_RXFIFO_STATUS_RXFIFO_FLUSH_Enable (1UL) /*!< 1: flush RX FIFO */
#define CDC_CDC_RXFIFO_STATUS_RXFIFO_FLUSH_Disable (0UL) /*!< 0: no action */

/* Register: CDC_DMA_CONFIG */
/* Description:cdc DMA控制寄存器 */
/* Offset   :0x28            */

/* Bit 0..7 : DMA_RXLVL */
/* Description: RXFIFO DMA trigger level
*/
#define CDC_CDC_DMA_CONFIG_DMA_RXLVL_Pos (0UL) // /*!< Position of DMA_RXLVL field. */
#define CDC_CDC_DMA_CONFIG_DMA_RXLVL_Msk (0xFFUL << CDC_CDC_DMA_CONFIG_DMA_RXLVL_Pos)  

/* Bit 16 : DMA_RX_EN */
/* Description: RX DMA enabe
*/
#define CDC_CDC_DMA_CONFIG_DMA_RX_EN_Pos (16UL) // /*!< Position of DMA_RX_EN field. */
#define CDC_CDC_DMA_CONFIG_DMA_RX_EN_Msk (0x1UL << CDC_CDC_DMA_CONFIG_DMA_RX_EN_Pos)  
#define CDC_CDC_DMA_CONFIG_DMA_RX_EN_Enable (1UL) /*!< 1: enable RX DMA */
#define CDC_CDC_DMA_CONFIG_DMA_RX_EN_Disable (0UL) /*!< 0: disable RX DMA */

/* Register: CDC_INT */
/* Description:cdc 中断清除寄存器 */
/* Offset   :0x2C            */

/* Bit 0 : RXFIFO_DATA_READY */
/* Description: RXFIFO data number are more than DMA_RXLVL setting
*/
#define CDC_CDC_INT_RXFIFO_DATA_READY_Pos (0UL) // /*!< Position of RXFIFO_DATA_READY field. */
#define CDC_CDC_INT_RXFIFO_DATA_READY_Msk (0x1UL << CDC_CDC_INT_RXFIFO_DATA_READY_Pos)  
#define CDC_CDC_INT_RXFIFO_DATA_READY_Generated (1UL) /*!<  */
#define CDC_CDC_INT_RXFIFO_DATA_READY_NotGenerated (0UL) /*!<  */

/* Bit 1 : RXFIFO_DATA_OVERFLOW */
/* Description: RX FIFO is overflow
*/
#define CDC_CDC_INT_RXFIFO_DATA_OVERFLOW_Pos (1UL) // /*!< Position of RXFIFO_DATA_OVERFLOW field. */
#define CDC_CDC_INT_RXFIFO_DATA_OVERFLOW_Msk (0x1UL << CDC_CDC_INT_RXFIFO_DATA_OVERFLOW_Pos)  
#define CDC_CDC_INT_RXFIFO_DATA_OVERFLOW_Generated (1UL) /*!< */
#define CDC_CDC_INT_RXFIFO_DATA_OVERFLOW_NotGenerated (0UL) /*!<  */

/* Bit 2 : RXFIFO_DATA_UNDERFLOW */
/* Description: RX FIFO is underflow
*/
#define CDC_CDC_INT_RXFIFO_DATA_UNDERFLOW_Pos (2UL) // /*!< Position of RXFIFO_DATA_UNDERFLOW field. */
#define CDC_CDC_INT_RXFIFO_DATA_UNDERFLOW_Msk (0x1UL << CDC_CDC_INT_RXFIFO_DATA_UNDERFLOW_Pos)  
#define CDC_CDC_INT_RXFIFO_DATA_UNDERFLOW_Generated (1UL) /*!<  */
#define CDC_CDC_INT_RXFIFO_DATA_UNDERFLOW_NotGenerated (0UL) /*!<  */

/* Register: CDC_INT_RAW */
/* Description:cdc  中断状态寄存器 */
/* Offset   :0x30            */

/* Bit 0 : RXFIFO_DATA_READY_RAW */
/* Description: Raw status of the interrupt XFIFO_DATA_READY
*/
#define CDC_CDC_INT_RXFIFO_DATA_READY_RAW_Pos (0UL) // /*!< Position of RXFIFO_DATA_READY_RAW field. */
#define CDC_CDC_INT_RXFIFO_DATA_READY_RAW_Msk (0x1UL << CDC_CDC_INT_RXFIFO_DATA_READY_RAW_Pos)  
#define CDC_CDC_INT_RXFIFO_DATA_READY_RAW_Generated (1UL) /*!<  */
#define CDC_CDC_INT_RXFIFO_DATA_READY_RAW_NotGenerated (0UL) /*!<  */

/* Bit 1 : RXFIFO_OVERFLOW_RAW */
/* Description: Raw status of the interrupt RXFIFO_OVERFLOW
*/
#define CDC_CDC_INT_RXFIFO_OVERFLOW_RAW_Pos (1UL) // /*!< Position of RXFIFO_OVERFLOW_RAW field. */
#define CDC_CDC_INT_RXFIFO_OVERFLOW_RAW_Msk (0x1UL << CDC_CDC_INT_RXFIFO_OVERFLOW_RAW_Pos)  
#define CDC_CDC_INT_RXFIFO_OVERFLOW_RAW_Generated (1UL) /*!<  */
#define CDC_CDC_INT_RXFIFO_OVERFLOW_RAW_NotGenerated (0UL) /*!<  */

/* Bit 2 : RXFIFO_UNDERFLOW_RAW */
/* Description: Raw status of the interrupt RXFIFO_UNDERFLOW
*/
#define CDC_CDC_INT_RXFIFO_UNDERFLOW_RAW_Pos (2UL) // /*!< Position of RXFIFO_UNDERFLOW_RAW field. */
#define CDC_CDC_INT_RXFIFO_UNDERFLOW_RAW_Msk (0x1UL << CDC_CDC_INT_RXFIFO_UNDERFLOW_RAW_Pos)  
#define CDC_CDC_INT_RXFIFO_UNDERFLOW_RAW_Generated (1UL) /*!<  */
#define CDC_CDC_INT_RXFIFO_UNDERFLOW_RAW_NotGenerated (0UL) /*!<  */

/* Register: CDC_INT_EN */
/* Description:cdc  中断使能寄存器 */
/* Offset   :0x30            */

/* Bit 0 : RXFIFO_DATA_READY_EN */
/* Description: Enable of the interrupt RXFIFO_DATA_READY
*/
#define CDC_CDC_INT_RXFIFO_DATA_READY_EN_Pos (0UL) // /*!< Position of RXFIFO_DATA_READY_EN field. */
#define CDC_CDC_INT_RXFIFO_DATA_READY_EN_Msk (0x1UL << CDC_CDC_INT_RXFIFO_DATA_READY_EN_Pos)  
#define CDC_CDC_INT_RXFIFO_DATA_READY_EN_Enable (1UL) /*!<  */
#define CDC_CDC_INT_RXFIFO_DATA_READY_EN_Disable (0UL) /*!<  */

/* Bit 1 : RXFIFO_OVERFLOW_EN */
/* Description: Enable of the interrupt RXFIFO_OVERFLOW
*/
#define CDC_CDC_INT_RXFIFO_OVERFLOW_EN_Pos (1UL) // /*!< Position of RXFIFO_OVERFLOW_EN field. */
#define CDC_CDC_INT_RXFIFO_OVERFLOW_EN_Msk (0x1UL << CDC_CDC_INT_RXFIFO_OVERFLOW_EN_Pos)  
#define CDC_CDC_INT_RXFIFO_OVERFLOW_EN_Enable (1UL) /*!<  */
#define CDC_CDC_INT_RXFIFO_OVERFLOW_EN_Disable (0UL) /*!<  */

/* Bit 2 : RXFIFO_UNDERFLOW_EN */
/* Description: Enable of the interrupt RXFIFO_UNDERFLOW
*/
#define CDC_CDC_INT_RXFIFO_UNDERFLOW_EN_Pos (2UL) // /*!< Position of RXFIFO_UNDERFLOW_EN field. */
#define CDC_CDC_INT_RXFIFO_UNDERFLOW_EN_Msk (0x1UL << CDC_CDC_INT_RXFIFO_UNDERFLOW_EN_Pos)  
#define CDC_CDC_INT_RXFIFO_UNDERFLOW_EN_Enable (1UL) /*!<  */
#define CDC_CDC_INT_RXFIFO_UNDERFLOW_EN_Disable (0UL) /*!<  */


/*lint --flb "Leave library region" */
#endif
