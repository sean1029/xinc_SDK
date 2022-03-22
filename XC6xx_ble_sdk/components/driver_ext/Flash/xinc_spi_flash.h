/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */

#ifndef XINC_SPI_FLASH_H__
#define XINC_SPI_FLASH_H__

/*----------------------------------------------------------------------------------------------------
    SPI 	COMMAND
-----------------------------------------------------------------------------------------------------*/
#define		CMD_READ_DATA			0x03
#define		CMD_READ_STATUS			0x05
#define		CMD_CHIP_ERASE			0xc7
#define		CMD_WRITE_ENABLE		0x06
#define		CMD_PAGE_PROGRAM		0x02
#define		CMD_BLOCK_ERASE			0xD8
#define		CMD_SECTOR_ERASE		0x20
#define		CMD_PAGE_ERASE		    0x81
#define		CMD_RELEASE_PWRDWN	    0xAB
#define		CMD_PWRDWN	            0xB9
#define		CMD_ID                  0x4B 
#define		PACKET_FULL_LEN			(FLASH_PAGE_SIZE )


#define		FLASH_PAGE_SIZE			256	
#define     FLASH_PAGE_MASK    (FLASH_PAGE_SIZE - 1)
#define		FLASH_BLOCK_SIZE		65536
#define		FLASH_SECTOR_SIZE		4096


#define     SPIM_CLK_1MHZ           32            
#define     SPIM_CLK_2MHZ           16
#define     SPIM_CLK_4MHZ           8
#define     SPIM_CLK_8MHZ           4
#define     SPIM_CLK_16MHZ          2     

void spim_flash_Read_128bitsID(uint8_t *buff);

void spim_flash_Read_MID(uint8_t *buff);

void spim_flash_Release_powerdown(void);

void spim_flash_Enter_powerdown(void);
uint8_t spim_read_flash_status(void);
void spim_flash_write_enable(void);
void spim_flash_write_page(uint32_t aligAddr,uint8_t *data,uint16_t len);
void spim_flash_read_page(uint32_t aligAddr,uint8_t *data,uint16_t len);
void spim_flash_page_erase(uint32_t	eraseAddr);
void spim_flash_sector_erase(uint32_t sectorAddr);
void spim_flash_block_erase(uint32_t blkAddr);
void spim_flash_chip_erase(void);
void spim_flash_read(uint32_t	ReadAddR, uint8_t *buff,uint16_t  ReadLength);
void spim_flash_write(uint32_t	WriteAddR, uint8_t *buff,uint16_t WriteLength);
void spim_flash_init(void);



#endif //

