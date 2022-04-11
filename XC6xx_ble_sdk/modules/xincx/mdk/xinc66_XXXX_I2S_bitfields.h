/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef __XINC66_xxxx_I2S_BITS_H
#define __XINC66_xxxx_I2S_BITS_H

/*lint ++flb "Enter library region" */

/* Peripheral: I2S */
/* Description: I2S接口模块 */

/* I2S Base Addr: 0x40011800 */

/* Register: I2S_TRANS_EN */
/* Description: I2S使能寄存器 */
/* Offset   :0x00             */

/* Bit 0 : I2S_ENABLE */
/* Description: 
 * 0: 禁止传输
 * 1:使能传输
*/
#define I2S_I2S_TRANS_EN_I2S_ENABLE_Pos (0UL) // /*!< Position of I2S_ENABLE field. */
#define I2S_I2S_TRANS_EN_I2S_ENABLE_Msk (0x1UL << I2S_I2S_TRANS_EN_I2S_ENABLE_Pos)  
#define I2S_I2S_TRANS_EN_I2S_ENABLE_Enable (1UL) /*!<  * 1:使能传输 */
#define I2S_I2S_TRANS_EN_I2S_ENABLE_Disable (0UL) /*!<* 0: 禁止传输  */


/* Register: I2S_CONFIG */
/* Description:  I2S配置寄存器 */
/* Offset   :0x04             */

/* Bit 4..6 : SLOT_WIDTH */
/* Description: Each Slot Width Configure
1: 8 bits
2: 12 bits 
3: 16 bits
4: 20 bits
5: 24 bits
6: 28 bits
7: 32 bits
*/
#define I2S_I2S_CONFIG_SLOT_WIDTH_Pos (4UL) // /*!< Position of SLOT_WIDTH field. */
#define I2S_I2S_CONFIG_SLOT_WIDTH_Msk (0x7UL << I2S_I2S_CONFIG_SLOT_WIDTH_Pos)  
#define I2S_I2S_CONFIG_SLOT_WIDTH_Invalid (0UL) /*!<*  0: NA */
#define I2S_I2S_CONFIG_SLOT_WIDTH_8bits (1UL) /*!<*  1: 8 bits */
#define I2S_I2S_CONFIG_SLOT_WIDTH_12bits (2UL) /*!<* 2: 12 bits   */
#define I2S_I2S_CONFIG_SLOT_WIDTH_16bits (3UL) /*!<* 3: 16 bits */
#define I2S_I2S_CONFIG_SLOT_WIDTH_20bits (4UL) /*!<* 4: 20 bits   */
#define I2S_I2S_CONFIG_SLOT_WIDTH_24bits (5UL) /*!<* 5: 24 bits   */
#define I2S_I2S_CONFIG_SLOT_WIDTH_28bits (6UL) /*!<* 6: 28 bits */
#define I2S_I2S_CONFIG_SLOT_WIDTH_32bits (7UL) /*!<* 7: 32 bits   */

/* Bit 12..14 : SLOT_RESOLUTION */
/* Description: Data resolution in each slot
1: 8 bits
2: 12 bits 
3: 16 bits
4: 20 bits
5: 24 bits
6: 28 bits
7: 32 bits
*/
#define I2S_I2S_CONFIG_SLOT_RESOLUTION_Pos (12UL) // /*!< Position of SLOT_RESOLUTION field. */
#define I2S_I2S_CONFIG_SLOT_RESOLUTION_Msk (0x7UL << I2S_I2S_CONFIG_SLOT_RESOLUTION_Pos)  
#define I2S_I2S_CONFIG_SLOT_RESOLUTION_Invalid (0UL) /*!<*  0: NA */
#define I2S_I2S_CONFIG_SLOT_RESOLUTION_8bits (1UL) /*!<*  1: 8 bits */
#define I2S_I2S_CONFIG_SLOT_RESOLUTION_12bits (2UL) /*!<* 2: 12 bits   */
#define I2S_I2S_CONFIG_SLOT_RESOLUTION_16bits (3UL) /*!<* 3: 16 bits */
#define I2S_I2S_CONFIG_SLOT_RESOLUTION_20bits (4UL) /*!<* 4: 20 bits   */
#define I2S_I2S_CONFIG_SLOT_RESOLUTION_24bits (5UL) /*!<* 5: 24 bits   */
#define I2S_I2S_CONFIG_SLOT_RESOLUTION_28bits (6UL) /*!<* 6: 28 bits */
#define I2S_I2S_CONFIG_SLOT_RESOLUTION_32bits (7UL) /*!<* 7: 32 bits   */

/* Bit 16 .. 19 : I2S_CHEN */
/* Description: 
 * 0:  Disable Channel
 * 1:  Enable Channel
*/
#define I2S_I2S_CONFIG_I2S_CHEN_Pos (16UL) // /*!< Position of I2S_CHEN field. */
#define I2S_I2S_CONFIG_I2S_CHEN_Msk (0xFUL << I2S_I2S_CONFIG_I2S_CHEN_Pos)  
#define I2S_I2S_CONFIG_I2S_CHEN_Enable (1UL) /*!<  * 1:Enable Channel */
#define I2S_I2S_CONFIG_I2S_CHEN_Disable (0UL) /*!<* 0: Disable Channel  */

/* Bit 20 .. 23 : I2S_CHTXRX */
/* Description: I2S each Channel Transimit/Receive Select
 * 0:  TX
 * 1:  RX
*/
#define I2S_I2S_CONFIG_I2S_CHTXRX_Pos (20UL) // /*!< Position of I2S_CHTXRX field. */
#define I2S_I2S_CONFIG_I2S_CHTXRX_Msk (0xFUL << I2S_I2S_CONFIG_I2S_CHTXRX_Pos)  
#define I2S_I2S_CONFIG_I2S_CHTXRX_Tx (1UL) /*!<  * 1:RX */
#define I2S_I2S_CONFIG_I2S_CHTXRX_Rx (0UL) /*!<* 0: TX  */

/* Bit 24 : DATA_EXTENTION */
/* Description: 
0: 0 padding at LSB
1: Sign extension at MSB
*/
#define I2S_I2S_CONFIG_DATA_EXTENTION_Pos (24UL) // /*!< Position of DATA_EXTENTION field. */
#define I2S_I2S_CONFIG_DATA_EXTENTION_Msk (0x1UL << I2S_I2S_CONFIG_DATA_EXTENTION_Pos)  
#define I2S_I2S_CONFIG_DATA_EXTENTION_Msb (1UL) /*!<  * 1: Sign extension at MSB*/
#define I2S_I2S_CONFIG_DATA_EXTENTION_Lsb (0UL) /*!<* 0: padding at LSB  */

/* Bit 25 : EDGE_TRANSFER */
/* Description: 
0: SDOOUT and SDIN drive data at the same SCLK edge
1: SDOOUT and SDIN drive data at the same SCLK edge
*/
#define I2S_I2S_CONFIG_EDGE_TRANSFER_Pos (25UL) // /*!< Position of EDGE_TRANSFER field. */
#define I2S_I2S_CONFIG_EDGE_TRANSFER_Msk (0x1UL << I2S_I2S_CONFIG_EDGE_TRANSFER_Pos)  
#define I2S_I2S_CONFIG_EDGE_TRANSFER_Same (1UL) /*!<* 1: SDOOUT and SDIN drive data at the same SCLK edge */
#define I2S_I2S_CONFIG_EDGE_TRANSFER_Diff (0UL) /*!<* 0: SDOOUT and SDIN drive data at the same SCLK edge  */




/* Register: I2S_CLOCK_GEN */
/* Description:  I2S时钟寄存器 */
/* Offset   :0x08             */

/* Bit 0 : MCLKEN */
/* Description: MCLK output enable
 * 1: enable
 * 0: diasble
*/
#define I2S_I2S_CLOCK_GEN_MCLKEN_Pos (0UL) // /*!< Position of MCLKEN field. */
#define I2S_I2S_CLOCK_GEN_MCLKEN_Msk (0x1UL << I2S_I2S_CONFIG_MCLKEN_Pos)  
#define I2S_I2S_CLOCK_GEN_MCLKEN_Enable (1UL) /*!<  * 1: enable */
#define I2S_I2S_CLOCK_GEN_MCLKEN_Disable (0UL) /*!<* 0: diasble  */

/* Bit 1 : MASTEREN */
/* Description: 
* 0: SCLK and WS as output
* 1: SCLK and WS as input
*/
#define I2S_I2S_CLOCK_GEN_MASTEREN_Pos (1UL) // /*!< Position of MASTEREN field. */
#define I2S_I2S_CLOCK_GEN_MASTEREN_Msk (0x1UL << I2S_I2S_CLOCK_GEN_MASTEREN_Pos)  
#define I2S_I2S_CLOCK_GEN_MASTEREN_Enable (1UL) /*!<  * 1: SCLK and WS as input */
#define I2S_I2S_CLOCK_GEN_MASTEREN_Disable (0UL) /*!<* 0: SCLK and WS as output  */

/* Bit 3 : SCLK_POLARITY */
/* Description: 
SCLK polarity
0: drive data at negative edge and sample data at posedge edge
1: drive data at posedge edge and sample data at negative edge
*/
#define I2S_I2S_CLOCK_GEN_SCLK_POLARITY_Pos (3UL) // /*!< Position of SCLK_POLARITY field. */
#define I2S_I2S_CLOCK_GEN_SCLK_POLARITY_Msk (0x1UL << I2S_I2S_CLOCK_GEN_SCLK_POLARITY_Pos)  
#define I2S_I2S_CLOCK_GEN_SCLK_POLARITY_Posedgge (0UL) /*!<  * 0: drive data at negative edge and sample data at posedge edge */
#define I2S_I2S_CLOCK_GEN_SCLK_POLARITY_Negative (1UL) /*!<*1: drive data at posedge edge and sample data at negative edge */

/* Bit 4..11 : SCLKDIV */
/* Description: 
SCLK divider from MCLK
0: same frequency
x: 2*x divided from MCLKI2S Channel Enable
0: Disable Channel
1: Enable Channel
*/
#define I2S_I2S_CLOCK_GEN_SCLKDIV_Pos (4UL) // /*!< Position of SCLKDIV field. */
#define I2S_I2S_CLOCK_GEN_SCLKDIV_Msk (0xFFUL << I2S_I2S_CLOCK_GEN_SCLKDIV_Pos)  

/* Bit 12..13 : WS_FORMAT */
/* Description: 
For I2S WS signal mode:
1: Left-justified
0: Standard DSP Mode
For I2S WS signal mode:
3: NA
2: Right-justifed
1: Left-justified
0: Standard I2S Mode
*/
#define I2S_I2S_CLOCK_GEN_WS_FORMAT_Pos (12UL) // /*!< Position of WS_FORMAT field. */
#define I2S_I2S_CLOCK_GEN_WS_FORMAT_Msk (0x3UL << I2S_I2S_CLOCK_GEN_WS_FORMAT_Pos)  
#define I2S_I2S_CLOCK_GEN_WS_FORMA_Standard (0UL)
#define I2S_I2S_CLOCK_GEN_WS_FORMA_Left_justified (1UL)
#define I2S_I2S_CLOCK_GEN_WS_FORMA_Right_justifed (2UL)

/* Bit 14 : WS_POLARITY_I2S */
/* Description: 
For I2S mode
0: WS 0 left channel, 1 right channel
1: WS 1 left channel, 0 right channel
For I2S mode
0: WS aligned to SCLK negedge edge
1: WS aligned to SCLK posedge
*/
#define I2S_I2S_CLOCK_GEN_WS_POLARITY_I2S_Pos (14UL) // /*!< Position of WS_FORMAT field. */
#define I2S_I2S_CLOCK_GEN_WS_POLARITY_I2S_Msk (0x1UL << I2S_I2S_CLOCK_GEN_WS_POLARITY_I2S_Pos)  
#define I2S_I2S_CLOCK_GEN_WS_POLARITY_I2S_0L_1R (0UL)
#define I2S_I2S_CLOCK_GEN_WS_POLARITY_I2S_1L_0R (1UL)
#define I2S_I2S_CLOCK_GEN_WS_POLARITY_I2S_Posedge (1UL)
#define I2S_I2S_CLOCK_GEN_WS_POLARITY_I2S_Negedge (0UL)
#define I2S_I2S_CLOCK_GEN_WS_POLARITY_I2S_Posedge (1UL)

/* Bit 15 : WS_WIDTH_I2S */
/* Description: 
WS signal width:
0: 1 SCLK cycle
1: 2 SCLK cycle
*/
#define I2S_I2S_CLOCK_GEN_WS_WIDTH_I2S_Pos (15UL) // /*!< Position of WS_FORMAT field. */
#define I2S_I2S_CLOCK_GEN_WS_WIDTH_I2S_Msk (0x3UL << I2S_I2S_CLOCK_GEN_WS_WIDTH_I2S_Pos)  
#define I2S_I2S_CLOCK_GEN_WS_WIDTH_I2S_1_SCLK_cycle (0UL)
#define I2S_I2S_CLOCK_GEN_WS_WIDTH_I2S_2_SCLK_cycle (1UL)


/* Register: I2S_RXFIFO_DATA */
/* Description:  I2S RX DATA寄存器 */
/* Offset   :0x0C             */

/* Bit 0 ..31 : RXFIFO_DATA */
/* Description: 
RX FIFO Received Data
Each read pop one 32bit data from RX FIFO
*/
#define I2S_I2S_RXFIFO_DATA_RXFIFO_DATA_Pos (0UL) // /*!< Position of RXFIFO_DATA field. */
#define I2S_I2S_RXFIFO_DATA_RXFIFO_DATA_Msk (0xFFFFFFFFUL << I2S_I2S_RXFIFO_DATA_RXFIFO_DATA_Pos)  

/* Register: I2S_TXFIFO_DATA */
/* Description:  I2S TX DATA寄存器 */
/* Offset   :0x10             */

/* Bit 0 ..31 : TXFIFO_DATA */
/* Description: 
TX FIFO Received Data
Each read pop one 32bit data from TX FIFO
*/
#define I2S_I2S_TXFIFO_DATA_TXFIFO_DATA_Pos (0UL) // /*!< Position of TXFIFO_DATA field. */
#define I2S_I2S_TXFIFO_DATA_TXFIFO_DATA_Msk (0xFFFFFFFFUL << I2S_I2S_TXFIFO_DATA_TXFIFO_DATA_Pos)  


/* Register: I2S_FIFO_STATUS */
/* Description:  I2S FIFO状态寄存器 */
/* Offset   :0x14             */

/* Bit 0  : RXFIFO_AEMPTY */
/* Description: 
RX FIFO almost empty status
*/
#define I2S_I2S_FIFO_STATUS_RXFIFO_AEMPTY_Pos (0UL) // /*!< Position of RXFIFO_AEMPTY field. */
#define I2S_I2S_FIFO_STATUS_RXFIFO_AEMPTY_Msk (0x1UL << I2S_I2S_FIFO_STATUS_RXFIFO_AEMPTY_Pos)  
#define I2S_I2S_FIFO_STATUS_RXFIFO_AEMPTY_NotGenerated (0UL)
#define I2S_I2S_FIFO_STATUS_RXFIFO_AEMPTY_Generated (1UL)


/* Bit 1  : RXFIFO_EMPTY */
/* Description: 
RX FIFO empty status
*/
#define I2S_I2S_FIFO_STATUS_RXFIFO_EMPTY_Pos (1UL) // /*!< Position of RXFIFO_EMPTY field. */
#define I2S_I2S_FIFO_STATUS_RXFIFO_EMPTY_Msk (0x1UL << I2S_I2S_FIFO_STATUS_RXFIFO_EMPTY_Pos)  
#define I2S_I2S_FIFO_STATUS_RXFIFO_EMPTY_NotGenerated (0UL)
#define I2S_I2S_FIFO_STATUS_RXFIFO_EMPTY_Generated (1UL)

/* Bit 2  : RXFIFO_AFULL */
/* Description: 
RX FIFO almost full status
*/
#define I2S_I2S_FIFO_STATUS_RXFIFO_AFULL_Pos (2UL) // /*!< Position of RXFIFO_AFULL field. */
#define I2S_I2S_FIFO_STATUS_RXFIFO_AFULL_Msk (0x1UL << I2S_I2S_FIFO_STATUS_RXFIFO_AFULL_Pos)  
#define I2S_I2S_FIFO_STATUS_RXFIFO_AFULL_NotGenerated (0UL)
#define I2S_I2S_FIFO_STATUS_RXFIFO_AFULL_Generated (1UL)

/* Bit 3  : RXFIFO_FULL */
/* Description: 
RX FIFO full status
*/
#define I2S_I2S_FIFO_STATUS_RXFIFO_FULL_Pos (3UL) // /*!< Position of RXFIFO_FULL field. */
#define I2S_I2S_FIFO_STATUS_RXFIFO_FULL_Msk (0x1UL << I2S_I2S_FIFO_STATUS_RXFIFO_FULL_Pos)  
#define I2S_I2S_FIFO_STATUS_RXFIFO_FULL_NotGenerated (0UL)
#define I2S_I2S_FIFO_STATUS_RXFIFO_FULL_Generated (1UL)

/* Bit 4  : RXFIFO_RXLVL_TRIG */
/* Description: 
XFIFO level trigger status
1: Data in RXFIFO  more than number in DMA_RXLVL
0: Data in RXFIFO  less than number in DMA_RXLVL
*/
#define I2S_I2S_FIFO_STATUS_RXFIFO_RXLVL_TRIG_Pos (4UL) // /*!< Position of RXFIFO_RXLVL_TRIG field. */
#define I2S_I2S_FIFO_STATUS_RXFIFO_RXLVL_TRIG_Msk (0x1UL << I2S_I2S_FIFO_STATUS_RXFIFO_RXLVL_TRIG_Pos)  
#define I2S_I2S_FIFO_STATUS_RXFIFO_RXLVL_TRIG_NotGenerated (0UL)
#define I2S_I2S_FIFO_STATUS_RXFIFO_RXLVL_TRIG_Generated (1UL)

/* Bit 5  : RXFIFO_POP_ERROR */
/* Description: 
RXFIFO pop error status
*/
#define I2S_I2S_FIFO_STATUS_RXFIFO_POP_ERROR_Pos (5UL) // /*!< Position RXFIFO_POP_ERROR  field. */
#define I2S_I2S_FIFO_STATUS_RXFIFO_POP_ERROR_Msk (0x1UL << I2S_I2S_FIFO_STATUS_RXFIFO_POP_ERROR_Pos)  
#define I2S_I2S_FIFO_STATUS_RXFIFO_POP_ERROR_NotGenerated (0UL)
#define I2S_I2S_FIFO_STATUS_RXFIFO_POP_ERROR_Generated (1UL)

/* Bit 6  : RXFIFO_PUSH_ERROR */
/* Description: 
RXFIFO PUSH error status
*/
#define I2S_I2S_FIFO_STATUS_RXFIFO_PUSH_ERROR_Pos (6UL) // /*!< Position RXFIFO_PUSH_ERROR  field. */
#define I2S_I2S_FIFO_STATUS_RXFIFO_PUSH_ERROR_Msk (0x1UL << I2S_I2S_FIFO_STATUS_RXFIFO_PUSH_ERROR_Pos)  
#define I2S_I2S_FIFO_STATUS_RXFIFO_PUSH_ERROR_NotGenerated (0UL)
#define I2S_I2S_FIFO_STATUS_RXFIFO_PUSH_ERROR_Generated (1UL)

/* Bit 7  : RXFIFO_FLUSH */
/* Description: 
0x0	RX FIFO flush:
1: flush RX FIFO
0: no action
*/
#define I2S_I2S_FIFO_STATUS_RXFIFO_FLUSH_Pos (7UL) // /*!< Position RXFIFO_FLUSH  field. */
#define I2S_I2S_FIFO_STATUS_RXFIFO_FLUSH_Msk (0x1UL << I2S_I2S_FIFO_STATUS_RXFIFO_FLUSH_Pos)  
#define I2S_I2S_FIFO_STATUS_RXFIFO_FLUSH_Action (1UL)
#define I2S_I2S_FIFO_STATUS_RXFIFO_FLUSH_NoAction (0UL)

/* Bit 8  : TXFIFO_AEMPT */
/* Description: 
TX FIFO almost empty status
*/
#define I2S_I2S_FIFO_STATUS_TXFIFO_AEMPT_Pos (8UL) // /*!< Position TXFIFO_AEMPT  field. */
#define I2S_I2S_FIFO_STATUS_TXFIFO_AEMPT_Msk (0x1UL << I2S_I2S_FIFO_STATUS_TXFIFO_AEMPT_Pos)  
#define I2S_I2S_FIFO_STATUS_TXFIFO_AEMPT_Generated (1UL)
#define I2S_I2S_FIFO_STATUS_TXFIFO_AEMPT_NotGenerated (0UL)

/* Bit 9  : TXFIFO_EMPT */
/* Description: 
TX FIFO empty status
*/
#define I2S_I2S_FIFO_STATUS_TXFIFO_EMPT_Pos (9UL) // /*!< Position TXFIFO_EMPT  field. */
#define I2S_I2S_FIFO_STATUS_TXFIFO_EMPT_Msk (0x1UL << I2S_I2S_FIFO_STATUS_TXFIFO_EMPT_Pos)  
#define I2S_I2S_FIFO_STATUS_TXFIFO_EMPT_Generated (1UL)
#define I2S_I2S_FIFO_STATUS_TXFIFO_EMPT_NotGenerated (0UL)

/* Bit 10  : TXFIFO_AFULL */
/* Description: 
TX FIFO almost full status
*/
#define I2S_I2S_FIFO_STATUS_TXFIFO_AFULL_Pos (10UL) // /*!< Position TXFIFO_AFULL  field. */
#define I2S_I2S_FIFO_STATUS_TXFIFO_AFULL_Msk (0x1UL << I2S_I2S_FIFO_STATUS_TXFIFO_AFULL_Pos)  
#define I2S_I2S_FIFO_STATUS_TXFIFO_AFULL_Generated (1UL)
#define I2S_I2S_FIFO_STATUS_TXFIFO_AFULL_NotGenerated (0UL)

/* Bit 11  : TXFIFO_FULL */
/* Description: 
TX FIFO full status
*/
#define I2S_I2S_FIFO_STATUS_TXFIFO_FULL_Pos (11UL) // /*!< Position TXFIFO_FULL field. */
#define I2S_I2S_FIFO_STATUS_TXFIFO_FULL_Msk (0x1UL << I2S_I2S_FIFO_STATUS_TXFIFO_FULL_Pos)  
#define I2S_I2S_FIFO_STATUS_TXFIFO_FULL_Generated (1UL)
#define I2S_I2S_FIFO_STATUS_TXFIFO_FULL_NotGenerated (0UL)

/* Bit 12  : TXFIFO_RXLVL_TRIG */
/* Description: 
TXFIFO level trigger status
1: Data in TXFIFO  less or equal than number in DMA_TXLVL
0: Data in TXFIFO  more than number in DMA_TXLVL
*/
#define I2S_I2S_FIFO_STATUS_TXFIFO_RXLVL_TRIG_Pos (12UL) // /*!< Position of TXFIFO_RXLVL_TRIG field. */
#define I2S_I2S_FIFO_STATUS_TXFIFO_RXLVL_TRIG_Msk (0x1UL << I2S_I2S_FIFO_STATUS_TXFIFO_RXLVL_TRIG_Pos)  
#define I2S_I2S_FIFO_STATUS_TXFIFO_RXLVL_TRIG_NotGenerated (0UL)
#define I2S_I2S_FIFO_STATUS_TXFIFO_RXLVL_TRIG_Generated (1UL)

/* Bit 13  : TXFIFO_POP_ERROR */
/* Description: 
RXFIFO pop error status
*/
#define I2S_I2S_FIFO_STATUS_TXFIFO_POP_ERROR_Pos (13UL) // /*!< Position TXFIFO_POP_ERROR  field. */
#define I2S_I2S_FIFO_STATUS_TXFIFO_POP_ERROR_Msk (0x1UL << I2S_I2S_FIFO_STATUS_TXFIFO_POP_ERROR_Pos)  
#define I2S_I2S_FIFO_STATUS_TXFIFO_POP_ERROR_NotGenerated (0UL)
#define I2S_I2S_FIFO_STATUS_TXFIFO_POP_ERROR_Generated (1UL)

/* Bit 14  : TXFIFO_PUSH_ERROR */
/* Description: 
RXFIFO PUSH error status
*/
#define I2S_I2S_FIFO_STATUS_TXFIFO_PUSH_ERROR_Pos (14UL) // /*!< Position TXFIFO_PUSH_ERROR  field. */
#define I2S_I2S_FIFO_STATUS_TXFIFO_PUSH_ERROR_Msk (0x1UL << I2S_I2S_FIFO_STATUS_TXFIFO_PUSH_ERROR_Pos)  
#define I2S_I2S_FIFO_STATUS_TXFIFO_PUSH_ERROR_NotGenerated (0UL)
#define I2S_I2S_FIFO_STATUS_TXFIFO_PUSH_ERROR_Generated (1UL)

/* Bit 15  : RXFIFO_FLUSH */
/* Description: 
0x0	RX FIFO flush:
1: flush RX FIFO
0: no action
*/
#define I2S_I2S_FIFO_STATUS_TXFIFO_FLUSH_Pos (15UL) // /*!< Position TXFIFO_FLUSH  field. */
#define I2S_I2S_FIFO_STATUS_TXFIFO_FLUSH_Msk (0x1UL << I2S_I2S_FIFO_STATUS_TXFIFO_FLUSH_Pos)  
#define I2S_I2S_FIFO_STATUS_TXFIFO_FLUSH_Action (1UL)
#define I2S_I2S_FIFO_STATUS_TXFIFO_FLUSH_NoAction (0UL)

/* Register: I2S_DMA_CONFIG */
/* Description:  I2S DMA控制寄存器 */
/* Offset   :0x18             */

/* Bit 0..7  : DMA_RXLVL */
/* Description: 
RXFIFO DMA trigger level:  config value 0~15
Configure the RX DMA trigger level to generate one DMA transfer
*/
#define I2S_I2S_DMA_CONFIG_DMA_RXLVL_Pos (0UL) // /*!< Position of DMA_RXLVL field. */
#define I2S_I2S_DMA_CONFIG_DMA_RXLVL_Msk (0xFFUL << I2S_I2S_DMA_CONFIG_DMA_RXLVL_Pos)  
#define I2S_I2S_DMA_CONFIG_DMA_RXLVL_Valid (0xFUL << I2S_I2S_DMA_CONFIG_DMA_RXLVL_Pos)  


/* Bit 8..15  : DMA_TXLVL */
/* Description: 
TXFIFO DMA trigger level:  config value 0~15
Configure the TX DMA trigger level to generate one DMA transfer
*/
#define I2S_I2S_DMA_CONFIG_DMA_TXLVL_Pos (8UL) // /*!< Position of DMA_TXLVL field. */
#define I2S_I2S_DMA_CONFIG_DMA_TXLVL_Msk (0xFFUL << I2S_I2S_DMA_CONFIG_DMA_TXLVL_Pos)  
#define I2S_I2S_DMA_CONFIG_DMA_TXLVL_Valid (0xFUL << I2S_I2S_DMA_CONFIG_DMA_TXLVL_Pos)  


/* Bit 16  : DMA_RX_EN */
/* Description: 
RX DMA enabe
1: enable RX DMA
0: disable RX DMA
*/
#define I2S_I2S_DMA_CONFIG_DMA_RX_EN_Pos (16UL) // /*!< Position of DMA_RX_EN field. */
#define I2S_I2S_DMA_CONFIG_DMA_RX_EN_Msk (0x1UL << I2S_I2S_DMA_CONFIG_DMA_RX_EN_Pos)  
#define I2S_I2S_DMA_CONFIG_DMA_RX_EN_Enable (1UL)  
#define I2S_I2S_DMA_CONFIG_DMA_RX_EN_Disable (0UL)  

/* Bit 17  : DMA_TX_EN */
/* Description: 
RX DMA enabe
1: enable TX DMA
0: disable TX DMA
*/
#define I2S_I2S_DMA_CONFIG_DMA_TX_EN_Pos (17UL) // /*!< Position of DMA_TX_EN field. */
#define I2S_I2S_DMA_CONFIG_DMA_TX_EN_Msk (0x1UL << I2S_I2S_DMA_CONFIG_DMA_TX_EN_Pos)  
#define I2S_I2S_DMA_CONFIG_DMA_TX_EN_Enable (1UL)  
#define I2S_I2S_DMA_CONFIG_DMA_TX_EN_Disable (0UL)  



/* Register: I2S_INT */
/* Description:   I2S 中断清除寄存器 */
/* Offset   :0x20             */

/* Bit 0  : RXFIFO_DATA_READY */
/* Description: 
RX FIFO data number are more than DMA_RXLVL setting
*/
#define I2S_I2S_INT_RXFIFO_DATA_READY_Pos (0UL) // /*!< Position of RXFIFO_DATA_READY field. */
#define I2S_I2S_INT_RXFIFO_DATA_READY_Msk (0x1UL << I2S_I2S_INT_RXFIFO_DATA_READY_Pos)  
#define I2S_I2S_INT_RXFIFO_DATA_READY_NotGenerated (0UL)
#define I2S_I2S_INT_RXFIFO_DATA_READY_Generated (1UL)

/* Bit 1  : RXFIFO_OVERFLOW */
/* Description: 
RX FIFO is overflow
*/
#define I2S_I2S_INT_RXFIFO_OVERFLOW_Pos (1UL) // /*!< Position of RXFIFO_OVERFLOW field. */
#define I2S_I2S_INT_RXFIFO_OVERFLOW_Msk (0x1UL << I2S_I2S_INT_RXFIFO_OVERFLOW_Pos)  
#define I2S_I2S_INT_RXFIFO_OVERFLOW_NotGenerated (0UL)
#define I2S_I2S_INT_RXFIFO_OVERFLOW_Generated (1UL)

/* Bit 2  : RXFIFO_UNDERFLOW */
/* Description: 
RX FIFO is underflow
*/
#define I2S_I2S_INT_RXFIFO_UNDERFLOW_Pos (2UL) // /*!< Position of RXFIFO_UNDERFLOW field. */
#define I2S_I2S_INT_RXFIFO_UNDERFLOW_Msk (0x1UL << I2S_I2S_INT_RXFIFO_UNDERFLOW_Pos)  
#define I2S_I2S_INT_RXFIFO_UNDERFLOW_NotGenerated (0UL)
#define I2S_I2S_INT_RXFIFO_UNDERFLOW_Generated (1UL)

/* Bit 4  : TXFIFO_DATA_READY */
/* Description: 
TX FIFO data number are more than DMA_TXLVL setting
*/
#define I2S_I2S_INT_TXFIFO_DATA_READY_Pos (4UL) // /*!< Position of TXFIFO_DATA_READY field. */
#define I2S_I2S_INT_TXFIFO_DATA_READY_Msk (0x1UL << I2S_I2S_INT_TXFIFO_DATA_READY_Pos)  
#define I2S_I2S_INT_TXFIFO_DATA_READY_NotGenerated (0UL)
#define I2S_I2S_INT_TXFIFO_DATA_READY_Generated (1UL)

/* Bit 5  : TXFIFO_OVERFLOW */
/* Description: 
TX FIFO is overflow
*/
#define I2S_I2S_INT_TXFIFO_OVERFLOW_Pos (5UL) // /*!< Position of TXFIFO_OVERFLOW field. */
#define I2S_I2S_INT_TXFIFO_OVERFLOW_Msk (0x1UL << I2S_I2S_INT_TXFIFO_OVERFLOW_Pos)  
#define I2S_I2S_INT_TXFIFO_OVERFLOW_NotGenerated (0UL)
#define I2S_I2S_INT_TXFIFO_OVERFLOW_Generated (1UL)

/* Bit 6  : TXFIFO_UNDERFLOW */
/* Description: 
TX FIFO is underflow
*/
#define I2S_I2S_INT_TXFIFO_UNDERFLOW_Pos (6UL) // /*!< Position of TXFIFO_UNDERFLOW field. */
#define I2S_I2S_INT_TXFIFO_UNDERFLOW_Msk (0x1UL << I2S_I2S_INT_TXFIFO_UNDERFLOW_Pos)  
#define I2S_I2S_INT_TXFIFO_UNDERFLOW_NotGenerated (0UL)
#define I2S_I2S_INT_TXFIFO_UNDERFLOW_Generated (1UL)


/* Register: I2S_INT_RAW */
/* Description: I2S 中断状态寄存器 */
/* Offset   :0x24             */

/* Bit 0  : RXFIFO_DATA_READY_RAW */
/* Description: 
Raw status of the interrupt RXFIFO_DATA_READY
*/
#define I2S_I2S_INT_RXFIFO_DATA_READY_RAW_Pos (0UL) // /*!< Position of RXFIFO_DATA_READY_RAW field. */
#define I2S_I2S_INT_RXFIFO_DATA_READY_RAW_Msk (0x1UL << I2S_I2S_INT_RXFIFO_DATA_READY_RAW_Pos)  
#define I2S_I2S_INT_RXFIFO_DATA_READY_RAW_NotGenerated (0UL)
#define I2S_I2S_INT_RXFIFO_DATA_READY_RAW_Generated (1UL)

/* Bit 1  : RXFIFO_OVERFLOW_RAW */
/* Description: 
Raw status of the interrupt RXFIFO_OVERFLOW
*/
#define I2S_I2S_INT_RXFIFO_OVERFLOW_RAW_Pos (1UL) // /*!< Position of RXFIFO_OVERFLOW_RAW field. */
#define I2S_I2S_INT_RXFIFO_OVERFLOW_RAW_Msk (0x1UL << I2S_I2S_INT_RXFIFO_OVERFLOW_RAW_Pos)  
#define I2S_I2S_INT_RXFIFO_OVERFLOW_RAW_NotGenerated (0UL)
#define I2S_I2S_INT_RXFIFO_OVERFLOW_RAW_Generated (1UL)


/* Bit 2  : RXFIFO_UNDERFLOW_RAW */
/* Description: 
Raw status of the interrupt RXFIFO_UNDERFLOW
*/
#define I2S_I2S_INT_RXFIFO_UNDERFLOW_RAW_Pos (2UL) // /*!< Position of RXFIFO_UNDERFLOW_RAW field. */
#define I2S_I2S_INT_RXFIFO_UNDERFLOW_RAW_Msk (0x1UL << I2S_I2S_INT_RXFIFO_UNDERFLOW_RAW_Pos)  
#define I2S_I2S_INT_RXFIFO_UNDERFLOW_RAW_NotGenerated (0UL)
#define I2S_I2S_INT_RXFIFO_UNDERFLOW_RAW_Generated (1UL)

/* Bit 4  : TXFIFO_DATA_READY_RAW */
/* Description: 
Raw status of the interrupt TXFIFO_DATA_READY
*/
#define I2S_I2S_INT_TXFIFO_DATA_READY_RAW_Pos (4UL) // /*!< Position of TXFIFO_DATA_READY_RAW field. */
#define I2S_I2S_INT_TXFIFO_DATA_READY_RAW_Msk (0x1UL << I2S_I2S_INT_TXFIFO_DATA_READY_RAW_Pos)  
#define I2S_I2S_INT_TXFIFO_DATA_READY_RAW_NotGenerated (0UL)
#define I2S_I2S_INT_TXFIFO_DATA_READY_RAW_Generated (1UL)

/* Bit 5  : TXFIFO_OVERFLOW_RAW */
/* Description: 
Raw status of the interrupt TXFIFO_OVERFLOW
*/
#define I2S_I2S_INT_TXFIFO_OVERFLOW_RAW_Pos (5UL) // /*!< Position of TXFIFO_OVERFLOW_RAW field. */
#define I2S_I2S_INT_TXFIFO_OVERFLOW_RAW_Msk (0x1UL << I2S_I2S_INT_TXFIFO_OVERFLOW_RAW_Pos)  
#define I2S_I2S_INT_TXFIFO_OVERFLOW_RAW_NotGenerated (0UL)
#define I2S_I2S_INT_TXFIFO_OVERFLOW_RAW_Generated (1UL)

/* Bit 6  : TXFIFO_UNDERFLOW_RAW */
/* Description: 
Raw status of the interrupt TXFIFO_UNDERFLOW
*/
#define I2S_I2S_INT_TXFIFO_UNDERFLOW_RAW_Pos (6UL) // /*!< Position of TXFIFO_UNDERFLOW_RAW field. */
#define I2S_I2S_INT_TXFIFO_UNDERFLOW_RAW_Msk (0x1UL << I2S_I2S_INT_TXFIFO_UNDERFLOW_RAW_Pos)  
#define I2S_I2S_INT_TXFIFO_UNDERFLOW_RAW_NotGenerated (0UL)
#define I2S_I2S_INT_TXFIFO_UNDERFLOW_RAW_Generated (1UL)



/* Register: I2S_INT_EN */
/* Description:  I2S 中断使能寄存器 */
/* Offset   :0x2C             */

/* Bit 0  : RXFIFO_DATA_READY_EN */
/* Description: 
Enable of the interrupt RXFIFO_DATA_READY
*/
#define I2S_I2S_INT_RXFIFO_DATA_READY_EN_Pos (0UL) // /*!< Position of RXFIFO_DATA_READY_EN field. */
#define I2S_I2S_INT_RXFIFO_DATA_READY_EN_Msk (0x1UL << I2S_I2S_INT_RXFIFO_DATA_READY_EN_Pos)  
#define I2S_I2S_INT_RXFIFO_DATA_READY_EN_NotGenerated (0UL)
#define I2S_I2S_INT_RXFIFO_DATA_READY_EN_Generated (1UL)


/* Bit 1  : RXFIFO_OVERFLOW_EN */
/* Description: 
Enable of the interrupt RXFIFO_OVERFLOW
*/
#define I2S_I2S_INT_RXFIFO_OVERFLOW_EN_Pos (1UL) // /*!< Position of RXFIFO_OVERFLOW_EN field. */
#define I2S_I2S_INT_RXFIFO_OVERFLOW_EN_Msk (0x1UL << I2S_I2S_INT_RXFIFO_OVERFLOW_EN_Pos)  
#define I2S_I2S_INT_RXFIFO_OVERFLOW_EN_NotGenerated (0UL)
#define I2S_I2S_INT_RXFIFO_OVERFLOW_EN_Generated (1UL)


/* Bit 2  : RXFIFO_UNDERFLOW_EN */
/* Description: 
Enable of the interrupt RXFIFO_UNDERFLOW
*/
#define I2S_I2S_INT_RXFIFO_UNDERFLOW_EN_Pos (2UL) // /*!< Position of RXFIFO_UNDERFLOW_EN field. */
#define I2S_I2S_INT_RXFIFO_UNDERFLOW_EN_Msk (0x1UL << I2S_I2S_INT_RXFIFO_UNDERFLOW_EN_Pos)  
#define I2S_I2S_INT_RXFIFO_UNDERFLOW_EN_NotGenerated (0UL)
#define I2S_I2S_INT_RXFIFO_UNDERFLOW_EN_Generated (1UL)

/* Bit 4  : TXFIFO_DATA_READY_EN */
/* Description: 
Enable of the interrupt TXFIFO_DATA_READY
*/
#define I2S_I2S_INT_TXFIFO_DATA_READY_EN_Pos (4UL) // /*!< Position of TXFIFO_DATA_READY_EN field. */
#define I2S_I2S_INT_TXFIFO_DATA_READY_EN_Msk (0x1UL << I2S_I2S_INT_TXFIFO_DATA_READY_EN_Pos)  
#define I2S_I2S_INT_TXFIFO_DATA_READY_EN_NotGenerated (0UL)
#define I2S_I2S_INT_TXFIFO_DATA_READY_EN_Generated (1UL)

/* Bit 5  : TXFIFO_OVERFLOW_EN*/
/* Description: 
Enable of the interrupt TXFIFO_OVERFLOW
*/
#define I2S_I2S_INT_TXFIFO_OVERFLOW_EN_Pos (5UL) // /*!< Position of TXFIFO_OVERFLOW_EN field. */
#define I2S_I2S_INT_TXFIFO_OVERFLOW_EN_Msk (0x1UL << I2S_I2S_INT_TXFIFO_OVERFLOW_EN_Pos)  
#define I2S_I2S_INT_TXFIFO_OVERFLOW_EN_NotGenerated (0UL)
#define I2S_I2S_INT_TXFIFO_OVERFLOW_EN_Generated (1UL)

/* Bit 6  : TXFIFO_UNDERFLOW_EN */
/* Description: 
Enable of the interrupt TXFIFO_UNDERFLOW
*/
#define I2S_I2S_INT_TXFIFO_UNDERFLOW_EN_Pos (6UL) // /*!< Position of TXFIFO_UNDERFLOW_EN field. */
#define I2S_I2S_INT_TXFIFO_UNDERFLOW_EN_Msk (0x1UL << I2S_I2S_INT_TXFIFO_UNDERFLOW_EN_Pos)  
#define I2S_I2S_INT_TXFIFO_UNDERFLOW_EN_NotGenerated (0UL)
#define I2S_I2S_INT_TXFIFO_UNDERFLOW_EN_Generated (1UL)



/*lint --flb "Leave library region" */
#endif
