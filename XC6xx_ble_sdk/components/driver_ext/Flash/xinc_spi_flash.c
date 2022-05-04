/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#include <string.h>
#include "boards.h"
#include "xinc_drv_spi.h"
#include "xinc_spi_flash.h"
#if XINCX_CHECK(XINCX_SPIM_ENABLED)

#define alignmentDown(a,size)		(a & (~ (size -1)))
#define alignmentUp(a,size)		  ((a + size - 1) & (~ (size -1)))

#ifdef XC60XX_M0

__ALIGN(4) uint8_t		__attribute__((section("DMA_BUFF"))) txbuff[(PACKET_FULL_LEN+4)];

__ALIGN(4) uint8_t		__attribute__((section("DMA_BUFF"))) rxbuff[(PACKET_FULL_LEN+4)];

#else
__ALIGN(4) uint8_t		txbuff[(PACKET_FULL_LEN+4)];

__ALIGN(4) uint8_t		rxbuff[(PACKET_FULL_LEN+4)];// ((aligned(4)))

#endif 
static const xinc_drv_spi_t m_spi = XINC_DRV_SPI_INSTANCE(1);  /**< SPI instance. */
static volatile bool spi_xfer_done;
static void spi_handler(xinc_drv_spi_evt_t const* p_event,
                        void *                    p_context)
{
//	printf("%s,type:%d\r\n",__func__,p_event->type);
//	
	uint8_t *tx_data = p_event->data.done.p_tx_buffer;
	uint8_t *rx_data = p_event->data.done.p_rx_buffer;
	spi_xfer_done = true;
    (void)tx_data;
    (void)rx_data;
}

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
#define		CMD_RDID                  0x9F 
#define		PACKET_FULL_LEN			(FLASH_PAGE_SIZE )
void spim_flash_Read_128bitsID(uint8_t *buff)
{
    memset(txbuff,0,24);
    txbuff[0] = CMD_ID; 

    spi_xfer_done = false;									
    xinc_drv_spi_transfer(&m_spi,txbuff,21,rxbuff,21);
    while (!spi_xfer_done)
    {
        __WFE();
    }
    memcpy(buff,&rxbuff[5],16);
}

void spim_flash_Read_RDID(uint8_t *buff)
{

    memset(txbuff,0,4);
    txbuff[0] = CMD_RDID; 
    spi_xfer_done = false;									
    xinc_drv_spi_transfer(&m_spi,txbuff,4,rxbuff,4);
    uint32_t dma_reg;
    while (!spi_xfer_done)
    {
        __WFE();

    }
    memcpy(buff,&rxbuff[1],3);
}

void spim_flash_Release_powerdown(void)
{
    txbuff[0] = CMD_RELEASE_PWRDWN; 
    spi_xfer_done = false;									
    xinc_drv_spi_transfer(&m_spi,txbuff,1,rxbuff,1);
    while (!spi_xfer_done)
    {
        __WFE();
    }
}

void spim_flash_Enter_powerdown(void)
{
    txbuff[0] = CMD_PWRDWN; 
    spi_xfer_done = false;									
    xinc_drv_spi_transfer(&m_spi,txbuff,1,rxbuff,1);
    while (!spi_xfer_done)
    {
        __WFE();
    }
}
uint8_t spim_read_flash_status(void)
{
    txbuff[0] = 0x05;
    txbuff[1] = 0;

    spi_xfer_done = false;									
    xinc_drv_spi_transfer(&m_spi,txbuff,2,rxbuff,2);
    while (!spi_xfer_done)
    {
        __WFE();
    }
    
    return rxbuff[1];
}

void spim_flash_write_enable(void)
{
    txbuff[0] = CMD_WRITE_ENABLE;
    txbuff[1] = 0;
    txbuff[2] = 0x0;
    txbuff[3] = 0x0;		
    
    spi_xfer_done = false;									
    xinc_drv_spi_transfer(&m_spi,txbuff,1,rxbuff,1);
    while (!spi_xfer_done)
    {
        __WFE();
    }	
}

void spim_flash_write_page(uint32_t aligAddr,uint8_t *data,uint16_t len)
{
    txbuff[0] = CMD_PAGE_PROGRAM;
    txbuff[1] = (uint8_t)(aligAddr>>16);
    txbuff[2] = (uint8_t)(aligAddr>>8);		
    txbuff[3] = (uint8_t)(aligAddr);

    memcpy(&txbuff[4],data,len);
    spi_xfer_done = false;									
    xinc_drv_spi_transfer(&m_spi,txbuff,len + 4,rxbuff,len + 4);
    while (!spi_xfer_done)
    {
        __WFE();
    }	
}

void spim_flash_read_page(uint32_t aligAddr,uint8_t *data,uint16_t len)
{	
    txbuff[0] = CMD_READ_DATA;
    txbuff[1] = (uint8_t)(aligAddr>>16);
    txbuff[2] = (uint8_t)(aligAddr>>8);		
    txbuff[3] = (uint8_t)(aligAddr);
  //  printf("read_page aligAddr:0x%x\r\n",aligAddr);
    memset(&txbuff[4],0xff,len);
    spi_xfer_done = false;									
    xinc_drv_spi_transfer(&m_spi,txbuff,len + 4,rxbuff,len + 4);
    while (!spi_xfer_done)
    {
        __WFE();
//        printf("read_page wait\r\n");
    }	
    memcpy(data,&rxbuff[4],len);
}
void spim_flash_page_erase(uint32_t	eraseAddr)
{
    spim_flash_write_enable();
    while(spim_read_flash_status() & 0x01)
    {
        __nop();
    }

    txbuff[0] = CMD_PAGE_ERASE;
    txbuff[1] = (uint8_t)(eraseAddr>>16);
    txbuff[2] = (uint8_t)(eraseAddr>>8);		
    txbuff[3] = (uint8_t)(eraseAddr);

    spi_xfer_done = false;									
    xinc_drv_spi_transfer(&m_spi,txbuff,4,rxbuff,4);
    while (!spi_xfer_done)
    {
        __WFE();
    }	

}
void spim_flash_sector_erase(uint32_t sectorAddr)
{
    spim_flash_write_enable();
    while(spim_read_flash_status() & 0x01)
    {
        __nop();
    }

    txbuff[0] = CMD_SECTOR_ERASE;
    txbuff[1] = (uint8_t)(sectorAddr>>16);
    txbuff[2] = (uint8_t)(sectorAddr>>8);		
    txbuff[3] = (uint8_t)(sectorAddr);

    spi_xfer_done = false;									
    xinc_drv_spi_transfer(&m_spi,txbuff,4,rxbuff,4);
    while (!spi_xfer_done)
    {
        __WFE();
    }	
    
    while(spim_read_flash_status() & 0x01)
    {
        __nop();
    }
}
void spim_flash_block_erase(uint32_t blkAddr)
{
    spim_flash_write_enable();
    while(spim_read_flash_status() & 0x01)
    {
        __nop();
    }

    txbuff[0] = CMD_BLOCK_ERASE;
    txbuff[1] = (uint8_t)(blkAddr>>16);
    txbuff[2] = (uint8_t)(blkAddr>>8);		
    txbuff[3] = (uint8_t)(blkAddr);

    spi_xfer_done = false;									
    xinc_drv_spi_transfer(&m_spi,txbuff,4,rxbuff,4);
    while (!spi_xfer_done)
    {
        __WFE();
    }	
}

void spim_flash_chip_erase(void)
{
    spim_flash_write_enable();
    while(spim_read_flash_status() & 0x01)
    {
        __nop();
    }
    txbuff[0] = CMD_CHIP_ERASE;

    spi_xfer_done = false;									
    xinc_drv_spi_transfer(&m_spi,txbuff,1,rxbuff,1);
    while (!spi_xfer_done)
    {
        __WFE();
    }
    while(spim_read_flash_status() & 0x01)
    {
        __nop();
    }	
}

void spim_flash_read(uint32_t	ReadAddR, uint8_t *buff,uint16_t  ReadLength)
{
    uint32_t pre_address;
    uint32_t pre_len;
    uint32_t page0;
    uint32_t buffIdx = 0;
    uint32_t	aligAddr = alignmentDown(ReadAddR,4);
    page0 = alignmentUp(ReadAddR,FLASH_PAGE_SIZE);//(ReadAddR +FLASH_PAGE_SIZE) &(~FLASH_PAGE_MASK);

    {
        pre_address = aligAddr;
        pre_len = ReadAddR - aligAddr;
        
        if(ReadAddR - aligAddr + ReadLength >= FLASH_PAGE_SIZE)
        {
                pre_len = page0  - aligAddr;
        }else
        {
                pre_len = ReadLength + ReadAddR - aligAddr;
        }
        
    //	printf("spi_flash_Read,ReadAddR:0x%x,page0:0x%x,pre_len:%d\n",ReadAddR,page0,pre_len);
        if(pre_len)
        {
    //		printf("spi_flash_Read_Page2:0x%x,buffIdx:%d\n",pre_address,buffIdx);
            spim_flash_read_page(pre_address,buff,pre_len);	
            
            buffIdx+= pre_len;
            
        }
        pre_address+= pre_len;
        while(buffIdx < ReadLength)
        {
            if((ReadLength - buffIdx) >= FLASH_PAGE_SIZE)
            {
        //		printf("spi_flash_Read_Page0:0x%x,buffIdx:%d\n",pre_address,buffIdx);
                spim_flash_read_page(pre_address,&buff[buffIdx],FLASH_PAGE_SIZE);
                buffIdx += FLASH_PAGE_SIZE;
                pre_address += FLASH_PAGE_SIZE;
            }else
            {
        //		printf("spi_flash_Read_Page1:0x%x,buffIdx:%d\n",pre_address,buffIdx);
                spim_flash_read_page(pre_address,&buff[buffIdx],ReadLength - buffIdx);
                buffIdx+=(ReadLength - buffIdx);
                pre_address+= (ReadLength - buffIdx);
            }
        }
		
	}
	return;

}
/* ---------------------------------------------------------------------------------------------------
 ----------------------------------------------------------------------------------------------------*/
void spim_flash_write(uint32_t	WriteAddR, uint8_t *buff,uint16_t WriteLength)
{
	
    uint32_t pre_address;
    uint32_t pre_len;
    uint32_t aliup_address;
    uint32_t buffIdx = 0;

    aliup_address = alignmentUp(WriteAddR,FLASH_PAGE_SIZE);

        
    pre_address = WriteAddR;
    pre_len = (aliup_address - pre_address);
    if(pre_len)
    {
        pre_len = (pre_len > WriteLength) ? WriteLength:pre_len;
        while(spim_read_flash_status() & 0x01);
        spim_flash_write_enable();
        spim_flash_write_page(pre_address,&buff[buffIdx],pre_len);	
        while(spim_read_flash_status() & 0x01);
        buffIdx+= pre_len;	
        
    }
    pre_address+= pre_len;
        
    while(buffIdx < WriteLength)
    {
        if((WriteLength - buffIdx) >= FLASH_PAGE_SIZE)
        {
            while(spim_read_flash_status() & 0x01);
            spim_flash_write_enable();
            spim_flash_write_page(pre_address,&buff[buffIdx],FLASH_PAGE_SIZE);
            while(spim_read_flash_status() & 0x01);
            buffIdx += FLASH_PAGE_SIZE;
            pre_address += FLASH_PAGE_SIZE;
        }else
        {
            while(spim_read_flash_status() & 0x01);
            spim_flash_write_enable();
            spim_flash_write_page(pre_address,&buff[buffIdx],WriteLength - buffIdx);
            while(spim_read_flash_status() & 0x01);
            buffIdx += (WriteLength - buffIdx);
            pre_address += (WriteLength - buffIdx);
        }
    }

	return;
}

void spim_flash_init(void)
{
	
    ret_code_t err_code;
    const xinc_drv_spi_config_t spi_cfg = {
                                            .sck_pin      = SPIM1_SCK_PIN,
                                            .mosi_pin     = SPIM1_MOSI_PIN,
                                            .miso_pin     = SPIM1_MISO_PIN,
                                            .ss_pin       = SPIM1_SS_PIN,
                                            #if defined (XC66XX_M4) && XINCX_CHECK(XINCX_SPIM2_ENABLED)
                                            .d2_pin       = XINCX_SPIM_PIN_NOT_USED,
                                            .d3_pin       = XINCX_SPIM_PIN_NOT_USED,
                                            #endif
                                            .irq_priority = 0,
                                            .orc          = 0xFF,
                                            .frequency    = (xinc_drv_spi_frequency_t) SSI_FREQUENCY_FREQUENCY_M1,
                                            .mode         = XINC_DRV_SPI_MODE_0,
                                            .bit_order    = XINC_DRV_SPI_BIT_ORDER_MSB_FIRST,
                                    };

    err_code = xinc_drv_spi_init(&m_spi, &spi_cfg, spi_handler, NULL);//
        
    APP_ERROR_CHECK(err_code);                                    
    printf("xinc_drv_spi_init:err_code:0x%x\r\n",err_code);
                                    
    uint32_t mid;

    spim_flash_Read_RDID((uint8_t*)&mid);
    printf("flash mid :0x%x\r\n",mid);
           
//    uint8_t id[16];                                    
//    spim_flash_Read_128bitsID(id);
//    printf("flash id :");
//    for(int i = 0;i < 16;i++)
//    {
//        printf("%02x ",id[i]);
//    }printf("\r\n");                           
                                    
   // spim_flash_sector_erase(1024* 128);
   //  spim_flash_sector_erase(1024* 132);

}
#endif //
