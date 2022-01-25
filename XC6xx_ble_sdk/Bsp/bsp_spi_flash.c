
/*----------------------------------------------------------------------------------------------------
	INCLUDE HEADE FILES
----------------------------------------------------------------------------------------------------*/
#include    <string.h>
#include    "Platform.h"
#include    "bsp_spi_flash.h"
#include    "bsp_register_macro.h"

#define alignmentDown(a,size)		(a & (~ (size -1)))
#define alignmentUp(a,size)		  ((a + size - 1) & (~ (size -1)))

#define  TH25D_20UA   //紫光青藤的flash 型号
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

/*-------------------------------------------------------------------
NOTE: bsp_spi0_flash文件下的变量要放在SHRAM0区域.
-------------------------------------------------------------------*/
//uint8_t		__attribute__((aligned(4)))	txbuff[(PACKET_FULL_LEN+4)];
//uint8_t		__attribute__((aligned(4)))	rxbuff[(PACKET_FULL_LEN+4)];
uint8_t		__attribute__((at(127*1024+0x10000000)))	txbuff[(PACKET_FULL_LEN+4)];
uint8_t		__attribute__((at(127*1024+512+0x10000000)))	rxbuff[(PACKET_FULL_LEN+4)];

/* ---------------------------------------------------------------------------------------------------
- 函数名称: Init_spi_master
- 函数功能: 初始化spi主模式
- 创建日期: 2015-09-14
----------------------------------------------------------------------------------------------------*/
void	Init_spi_master(uint32_t ch, uint32_t freq)
{
    uint32_t    val;
    
    __write_hw_reg32(CPR_SPIx_MCLK_CTL(ch), 0x110010);//1分频			//- spi(x)_mclk = 32Mhz(When TXCO=32Mhz).
    __write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL , (0x1000100<<ch)); 	//- 打开spi(x) pclk.
    __read_hw_reg32(CPR_SSI_CTRL, val);
    val |= (ch==0)? 0x01: 0x30;
    __write_hw_reg32(CPR_SSI_CTRL, val);
	
    __write_hw_reg32(SSIx_EN(ch), 0x00);

    __write_hw_reg32(SSIx_IE(ch), 0x00);
    __write_hw_reg32(SSIx_CTRL0(ch) , 0x0f);					/* 16bit SPI data */

    __write_hw_reg32(SSIx_SE(ch), 0x01);
    __write_hw_reg32(SSIx_BAUD(ch), freq);						//- spix_mclk 分频.

    __write_hw_reg32(SSIx_RXFTL(ch), 0x00);
    __write_hw_reg32(SSIx_TXFTL(ch), 0x00);

    //__write_hw_reg32(SSIx_EN(ch) , 0x01);
}
/* ---------------------------------------------------------------------------------------------------
- 函数名称: spi_flash_Read_128bitsID
- 函数功能: 读取flash  128bits ID 读独立的ID序列 该指令可以读取一个芯片已经固化的只读的128位ID
- 创建日期: 2015-09-14
----------------------------------------------------------------------------------------------------*/
void spi_flash_Read_128bitsID(uint8_t *buff)
{
    while(spi_flash_wait_till_ready());
    uint32_t    iWK = 0;
  //  memset(txbuff,0,PACKET_FULL_LEN+4);
 //   memset(rxbuff,0,PACKET_FULL_LEN+4);
		txbuff[0] = 0x0;
	  txbuff[1] = CMD_ID;
		txbuff[2] = 0x0;
		txbuff[3] = 0x0;
    
    __write_hw_reg32(SSI0_EN , 0x00);
    __write_hw_reg32(SSI0_DMAS , 0x03);
    __write_hw_reg32(SSI0_DMATDL, 0x4);          //-
    __write_hw_reg32(SSI0_DMARDL, 0x4);          //- 1/4 FIFO
    __write_hw_reg32(SSI0_EN , 0x01);

	//- RX Channel
	__write_hw_reg32(DMAS_CHx_SAR(10) , 0x40013060);
    __write_hw_reg32(DMAS_CHx_DAR(10) , (uint32_t)rxbuff);
    __write_hw_reg32(DMAS_CHx_CTL1(10) ,((2 << 8)|  1));
    __write_hw_reg32(DMAS_CHx_CTL0(10) ,24);
    __write_hw_reg32(DMAS_EN , 10);

		//- TX Channel
	__write_hw_reg32(DMAS_CHx_SAR(2) , (uint32_t)txbuff);
    __write_hw_reg32(DMAS_CHx_DAR(2) , 0x40013060);
    __write_hw_reg32(DMAS_CHx_CTL1(2) ,((2 << 8)|  1));
    __write_hw_reg32(DMAS_CHx_CTL0(2) ,24);//接收缓冲区首地址必须是4的倍数而且大小必须是四的倍数
    __write_hw_reg32(DMAS_EN , 2);

    do	{
    	__read_hw_reg32(DMAS_INT_RAW , iWK);
    }while((iWK&0x404) != 0x404);

    __write_hw_reg32(DMAS_INT_RAW, 0x404);
    __write_hw_reg32(DMAS_CLR , 10);
    __write_hw_reg32(DMAS_CLR , 2);
    __write_hw_reg32(SSI0_EN , 0x00);  
   
//    for(int i=0;i<12;i++)
//    {
//        uint8_t temp=rxbuff[2*i+1];
//        rxbuff[2*i+1]=rxbuff[2*i];
//        rxbuff[2*i]=temp;
//    }
    for(int i=0;i<16;i++)
          buff[i]=rxbuff[5+i]; 

}

/* ---------------------------------------------------------------------------------------------------
- 函数名称: spi_flash_Read_Page
- 函数功能: 从SPI FLASH中读取一页数据到指定地址
- 创建日期: 2015-09-14
----------------------------------------------------------------------------------------------------*/
void	spi_flash_Read_Page(uint32_t	PageAddR, uint8_t *buff,uint16_t len)
{
	uint32_t	addr = PageAddR;
	uint32_t    iWK = 0;
	
	uint16_t totalLen =  alignmentUp(len,4) + 4;
  printf("spi_flash_Read_Page,ReadAddR:0x%x,len:%d,totalLen:%d\n",PageAddR,len,totalLen);  
	txbuff[0] = (uint8_t)(addr>>16);
	txbuff[1] = CMD_READ_DATA;
	txbuff[2] = (uint8_t)(addr);
	txbuff[3] = (uint8_t)(addr>>8);		
	
    __write_hw_reg32(SSI0_EN , 0x00);
    __write_hw_reg32(SSI0_DMAS , 0x03);
    __write_hw_reg32(SSI0_DMATDL, 0x4);          //-
    __write_hw_reg32(SSI0_DMARDL, 0x4);          //- 1/4 FIFO
    __write_hw_reg32(SSI0_EN , 0x01);

	//- RX Channel
	__write_hw_reg32(DMAS_CHx_SAR(10) , 0x40013060);
    __write_hw_reg32(DMAS_CHx_DAR(10) , (uint32_t)buff);
    __write_hw_reg32(DMAS_CHx_CTL1(10) ,((2 << 8)|  1));
    __write_hw_reg32(DMAS_CHx_CTL0(10) ,(totalLen));
    __write_hw_reg32(DMAS_EN , 10);

		//- TX Channel
	__write_hw_reg32(DMAS_CHx_SAR(2) , (uint32_t)txbuff);
    __write_hw_reg32(DMAS_CHx_DAR(2) , 0x40013060);
    __write_hw_reg32(DMAS_CHx_CTL1(2) ,((2 << 8)|  1));
    __write_hw_reg32(DMAS_CHx_CTL0(2) ,(totalLen));
    __write_hw_reg32(DMAS_EN , 2);

    do	{
    	__read_hw_reg32(DMAS_INT_RAW , iWK);
    }while((iWK&0x404) != 0x404);

    __write_hw_reg32(DMAS_INT_RAW, 0x404);
    __write_hw_reg32(DMAS_CLR , 10);
    __write_hw_reg32(DMAS_CLR , 2);
    __write_hw_reg32(SSI0_EN , 0x00);    
}

/* ---------------------------------------------------------------------------------------------------
- 函数名称: spi_flash_wait_till_ready
- 函数功能: 检查spi flash处于ready状态
- 创建日期: 2015-09-14
----------------------------------------------------------------------------------------------------*/
uint8_t	spi_flash_wait_till_ready (void)
{
    uint16_t	cmd = (CMD_READ_STATUS<<8);

    uint32_t	iWK = 0;
    uint16_t	dWK = 0;

    __write_hw_reg32(SSI0_EN , 0x00);
    __write_hw_reg32(SSI0_DMAS , 0x00);			/* turn off dma*/
    __write_hw_reg32(SSI0_DMATDL, 0x0);      	//-
    __write_hw_reg32(SSI0_DMARDL, 0x0);      	//-
    __write_hw_reg32(SSI0_EN , 0x01);

    __write_hw_reg32(SSI0_DATA , cmd);

    do	{
        	__read_hw_reg32(SSI0_STS, iWK);
    }while((iWK&0x05) != 0x04);

    __read_hw_reg32(SSI0_DATA , dWK);

    __write_hw_reg32(SSI0_EN , 0x00);    
    
    return	(uint8_t)(dWK&0x01);
}

/* ---------------------------------------------------------------------------------------------------
- 函数名称: spi_flash_chip_erase
- 函数功能: 擦除整个芯片的内容
- 创建日期: 2015-09-16
----------------------------------------------------------------------------------------------------*/
void	spi_flash_chip_erase(void)
{
	uint32_t	bWk, cWk;

    __read_hw_reg32(SSI0_CTRL0 , bWk);

    __write_hw_reg32(SSI0_EN , 0x00);
    __write_hw_reg32(SSI0_DMAS , 0x00);
    __write_hw_reg32(SSI0_CTRL0 , 0x07);				/* 8bit SPI data */
    __write_hw_reg32(SSI0_EN , 0x01);


    __write_hw_reg32(SSI0_DATA, CMD_CHIP_ERASE);

    do	{
        	__read_hw_reg32(SSI0_STS, cWk);
    }while((cWk&0x05) != 0x04);


    __write_hw_reg32(SSI0_EN , 0x00);
    __write_hw_reg32(SSI0_CTRL0 , bWk);
}


/* ---------------------------------------------------------------------------------------------------
- 函数名称: spi_flash_erase_block_num
- 函数功能: 计算需擦除block的数目
- 创建日期: 2015-09-16
----------------------------------------------------------------------------------------------------*/
uint32_t	spi_flash_block_num(uint32_t	size)
{
		uint32_t	blk = 0;

		blk = size/FLASH_BLOCK_SIZE;

		if(size % FLASH_BLOCK_SIZE) blk++;

		return blk;
}

/* ---------------------------------------------------------------------------------------------------
- 函数名称: spi_flash_block_erase
- 函数功能: 擦除芯片指定block的内容
- 创建日期: 2015-09-16
----------------------------------------------------------------------------------------------------*/
void	spi_flash_block_erase(uint32_t	blknum)
{
    uint32_t	addr = (blknum*FLASH_BLOCK_SIZE);
    uint32_t  	iWK = 0;

    txbuff[0] = (uint8_t)(addr>>16);
    txbuff[1] = CMD_BLOCK_ERASE;
    txbuff[2] = (uint8_t)(addr);
    txbuff[3] = (uint8_t)(addr>>8);

    __write_hw_reg32(SSI0_EN , 0x00);
    __write_hw_reg32(SSI0_DMAS , 0x03);
    __write_hw_reg32(SSI0_DMATDL, 0x4);          //-
    __write_hw_reg32(SSI0_DMARDL, 0x4);          //- 1/4 FIFO
    __write_hw_reg32(SSI0_EN , 0x01);

		//- RX Channel
	__write_hw_reg32(DMAS_CHx_SAR(10) , 0x40013060);
    __write_hw_reg32(DMAS_CHx_DAR(10) , (uint32_t)rxbuff);
    __write_hw_reg32(DMAS_CHx_CTL1(10) ,((2 << 8)|  1));
    __write_hw_reg32(DMAS_CHx_CTL0(10) ,4);
    __write_hw_reg32(DMAS_EN , 10);

		//- TX Channel
	__write_hw_reg32(DMAS_CHx_SAR(2) , (uint32_t)txbuff);
    __write_hw_reg32(DMAS_CHx_DAR(2) , 0x40013060);
    __write_hw_reg32(DMAS_CHx_CTL1(2) ,((2 << 8)|  1));
    __write_hw_reg32(DMAS_CHx_CTL0(2) ,4);
    __write_hw_reg32(DMAS_EN , 2);

    do	{
    	__read_hw_reg32(DMAS_INT_RAW , iWK);
    }while((iWK&0x404) != 0x404);

    __write_hw_reg32(DMAS_INT_RAW, 0x404);
    __write_hw_reg32(DMAS_CLR , 10);
    __write_hw_reg32(DMAS_CLR , 2);
    __write_hw_reg32(SSI0_EN , 0x00);
    
}
/* ---------------------------------------------------------------------------------------------------
- 函数名称: spi_flash_write_enable
- 函数功能: 使能对芯片的写操作
- 创建日期: 2015-09-16
----------------------------------------------------------------------------------------------------*/
void	spi_flash_write_enable(void)
{
    uint32_t	bWk, cWk;

    __read_hw_reg32(SSI0_CTRL0 , bWk);

    __write_hw_reg32(SSI0_EN, 0x00);
    __write_hw_reg32(SSI0_DMAS , 0x00);
    __write_hw_reg32(SSI0_CTRL0 , 0x07);				/* 8bit SPI data */
    __write_hw_reg32(SSI0_EN, 0x01);


    __write_hw_reg32(SSI0_DATA, CMD_WRITE_ENABLE);

    do	{
        	__read_hw_reg32(SSI0_STS, cWk);
    }while((cWk&0x05) != 0x04);


    __write_hw_reg32(SSI0_EN, 0x00);
    __write_hw_reg32(SSI0_CTRL0, bWk);

}

/* ---------------------------------------------------------------------------------------------------
- 函数名称: spi_flash_write_page
- 函数功能: 向SPI FLASH指定地址写入一页数据
- 创建日期: 2015-09-16
----------------------------------------------------------------------------------------------------*/

static void	spi_flash_write_page(uint32_t	PageAddR, const uint8_t *buff, uint16_t len)
{
    uint32_t	aligAddr = alignmentDown(PageAddR,4);
 
		uint16_t headLen = PageAddR - aligAddr;
		uint16_t totalLen = alignmentUp(len + headLen,4);  
		uint16_t tailLen = totalLen - headLen - len;
		uint32_t  iWK = 0;
		uint32_t	i;
		uint8_t tmpRxbuff[8];
	//  printf("spi_flash_write_page PageAddR:0x%x,aligAddr:0x%x, len:%d,totalLen:%d,headLen:%d,tailLen:%d\r\n",PageAddR,aligAddr,len,totalLen,headLen,tailLen);
	
		if(headLen)			
		{
			spi_flash_Read_Page(aligAddr,tmpRxbuff,headLen);
			memcpy(&txbuff[4],&tmpRxbuff[4],headLen);
//			printf("headbuff\r\n");
//			for(i=0; i<headLen; i++)
//			{
//				 printf("txbuff[%d]:0x%x\r\n",i + 4,txbuff[i + 4]);
//			}
		}
		
		
		if(tailLen)
		{
			spi_flash_Read_Page(aligAddr + totalLen - 4,tmpRxbuff,4);
			memcpy(&txbuff[totalLen + 4 - tailLen],&tmpRxbuff[8 - tailLen],tailLen);
				
		}
	
    for(i=0; i<len; i++)
    {
        txbuff[headLen + i+ 4] = buff[i];
    }
		
		while(spi_flash_wait_till_ready())
		{
			__nop();
		}
		
    txbuff[0] = (uint8_t)(aligAddr>>16);
    txbuff[1] = CMD_PAGE_PROGRAM;
    txbuff[2] = (uint8_t)(aligAddr);
    txbuff[3] = (uint8_t)(aligAddr>>8);

		
		totalLen += 4;
//		printf("totalbuff\r\n");
//		for(i=0; i<totalLen; i++)
//    {
//       printf("txbuff[%d]:0x%x\r\n",i,txbuff[i]);
//    }
		
    __write_hw_reg32(SSI0_EN , 0x00);
    __write_hw_reg32(SSI0_DMAS , 0x03);
    __write_hw_reg32(SSI0_DMATDL, 0x4);          //-
    __write_hw_reg32(SSI0_DMARDL, 0x4);          //- 1/2FIFO
    __write_hw_reg32(SSI0_EN , 0x01);

		//- RX Channel
	__write_hw_reg32(DMAS_CHx_SAR(10), 0x40013060);
    __write_hw_reg32(DMAS_CHx_DAR(10), (uint32_t)rxbuff);
    __write_hw_reg32(DMAS_CHx_CTL1(10),((2 << 8)|  1));
    __write_hw_reg32(DMAS_CHx_CTL0(10),(totalLen));
    __write_hw_reg32(DMAS_EN, 10);

		//- TX Channel
	__write_hw_reg32(DMAS_CHx_SAR(2), (uint32_t)txbuff);
    __write_hw_reg32(DMAS_CHx_DAR(2), 0x40013060);
    __write_hw_reg32(DMAS_CHx_CTL1(2),((2 << 8)|  1));
    __write_hw_reg32(DMAS_CHx_CTL0(2),(totalLen));
    __write_hw_reg32(DMAS_EN, 2);

    do	{
    	__read_hw_reg32(DMAS_INT_RAW , iWK);
    }while((iWK&0x404) != 0x404);

    __write_hw_reg32(DMAS_INT_RAW, 0x404);
    __write_hw_reg32(DMAS_CLR , 10);
    __write_hw_reg32(DMAS_CLR , 2);
    __write_hw_reg32(SSI0_EN , 0x00);
    
}



#ifdef TH25D_20UA  //使用紫光青藤flash
/* ---------------------------------------------------------------------------------------------------
- 函数名称: spi_flash_page_erase   
- 函数功能: 擦除芯片指定page的内容 ---紫光青藤flash有此功能，GD兆易 和 MD普软 flash没有此功能
- 创建日期: 2016-06-15
----------------------------------------------------------------------------------------------------*/
void	spi_flash_page_erase(uint32_t	eraseAddr)
{
    uint32_t	addr = eraseAddr;//(no*FLASH_PAGE_SIZE);
    uint32_t  	iWK = 0;

		while(spi_flash_wait_till_ready());
    txbuff[0] = (uint8_t)(addr>>16);
    txbuff[1] = CMD_PAGE_ERASE;
    txbuff[2] = (uint8_t)(addr);
    txbuff[3] = (uint8_t)(addr>>8);

    __write_hw_reg32(SSI0_EN, 0x00);
    __write_hw_reg32(SSI0_DMAS, 0x03);
    __write_hw_reg32(SSI0_DMATDL, 0x4);          //-
    __write_hw_reg32(SSI0_DMARDL, 0x4);          //- 1/4 FIFO
    __write_hw_reg32(SSI0_EN, 0x01);

    //- RX Channel
    __write_hw_reg32(DMAS_CHx_SAR(10), 0x40013060);
    __write_hw_reg32(DMAS_CHx_DAR(10), (uint32_t)rxbuff);
    __write_hw_reg32(DMAS_CHx_CTL1(10),((2 << 8)|  1));
    __write_hw_reg32(DMAS_CHx_CTL0(10),4);
    __write_hw_reg32(DMAS_EN , 10);

    //- TX Channel
    __write_hw_reg32(DMAS_CHx_SAR(2), (uint32_t)txbuff);
    __write_hw_reg32(DMAS_CHx_DAR(2), 0x40013060);
    __write_hw_reg32(DMAS_CHx_CTL1(2),((2 << 8)|  1));
    __write_hw_reg32(DMAS_CHx_CTL0(2),4);
    __write_hw_reg32(DMAS_EN, 2);

    do	{
    	__read_hw_reg32(DMAS_INT_RAW , iWK);
    }while((iWK&0x404) != 0x404);

    __write_hw_reg32(DMAS_INT_RAW, 0x404);
    __write_hw_reg32(DMAS_CLR, 10);
    __write_hw_reg32(DMAS_CLR, 2);
    __write_hw_reg32(SSI0_EN, 0x00);    
}
#else
/* ---------------------------------------------------------------------------------------------------
- 函数名称: spi_flash_sector_erase
- 函数功能: 擦除芯片指定sector的内容
- 创建日期: 2016-06-15
----------------------------------------------------------------------------------------------------*/
void	spi_flash_sector_erase(uint32_t	no)
{
    uint32_t	addr = (no*FLASH_SECTOR_SIZE);
    uint32_t  	iWK = 0;

    txbuff[0] = (uint8_t)(addr>>16);
    txbuff[1] = CMD_SECTOR_ERASE;
    txbuff[2] = (uint8_t)(addr);
    txbuff[3] = (uint8_t)(addr>>8);

    __write_hw_reg32(SSI0_EN, 0x00);
    __write_hw_reg32(SSI0_DMAS, 0x03);
    __write_hw_reg32(SSI0_DMATDL, 0x4);          //-
    __write_hw_reg32(SSI0_DMARDL, 0x4);          //- 1/4 FIFO
    __write_hw_reg32(SSI0_EN, 0x01);

    //- RX Channel
    __write_hw_reg32(DMAS_CHx_SAR(10), 0x40013060);
    __write_hw_reg32(DMAS_CHx_DAR(10), (uint32_t)rxbuff);
    __write_hw_reg32(DMAS_CHx_CTL1(10),((2 << 8)|  1));
    __write_hw_reg32(DMAS_CHx_CTL0(10),4);
    __write_hw_reg32(DMAS_EN , 10);

    //- TX Channel
    __write_hw_reg32(DMAS_CHx_SAR(2), (uint32_t)txbuff);
    __write_hw_reg32(DMAS_CHx_DAR(2), 0x40013060);
    __write_hw_reg32(DMAS_CHx_CTL1(2),((2 << 8)|  1));
    __write_hw_reg32(DMAS_CHx_CTL0(2),4);
    __write_hw_reg32(DMAS_EN, 2);

    do	{
    	__read_hw_reg32(DMAS_INT_RAW , iWK);
    }while((iWK&0x404) != 0x404);

    __write_hw_reg32(DMAS_INT_RAW, 0x404);
    __write_hw_reg32(DMAS_CLR, 10);
    __write_hw_reg32(DMAS_CLR, 2);
    __write_hw_reg32(SSI0_EN, 0x00);    
}
#endif
/* ---------------------------------------------------------------------------------------------------
- 函数名称: spi_flash_Release_powerdown
- 函数功能: SPI_FLASH Exit Power-Down
- 创建日期: 2016-10-24
----------------------------------------------------------------------------------------------------*/
void	spi_flash_Release_powerdown(void)
{
	uint32_t	bWk, cWk;

    __read_hw_reg32(SSI0_CTRL0, bWk);

    __write_hw_reg32(SSI0_EN, 0x00);
    __write_hw_reg32(SSI0_DMAS, 0x00);
    __write_hw_reg32(SSI0_CTRL0, 0x07);				/* 8bit SPI data */
    __write_hw_reg32(SSI0_EN, 0x01);


    __write_hw_reg32(SSI0_DATA, CMD_RELEASE_PWRDWN);
    do	{
        	__read_hw_reg32(SSI0_STS, cWk);
    }while((cWk&0x05) != 0x04);

    __write_hw_reg32(SSI0_EN, 0x00);
    __write_hw_reg32(SSI0_CTRL0, bWk);

}

/* ---------------------------------------------------------------------------------------------------
- 函数名称: spi_flash_powerdown
- 函数功能: SPI_FLASH Enter Power-Down
- 创建日期: 2016-11-07
----------------------------------------------------------------------------------------------------*/
void	spi_flash_Enter_powerdown(void)
{
	uint32_t	bWk, cWk;

    __read_hw_reg32(SSI0_CTRL0, bWk);

    __write_hw_reg32(SSI0_EN, 0x00);
    __write_hw_reg32(SSI0_DMAS, 0x00);
    __write_hw_reg32(SSI0_CTRL0, 0x07);				/* 8bit SPI data */
    __write_hw_reg32(SSI0_EN, 0x01);


    __write_hw_reg32(SSI0_DATA, CMD_PWRDWN);
    do	{
        	__read_hw_reg32(SSI0_STS, cWk);
    }while((cWk&0x05) != 0x04);


    __write_hw_reg32(SSI0_EN, 0x00);
    __write_hw_reg32(SSI0_CTRL0, bWk);

}    

/* ---------------------------------------------------------------------------------------------------
- 函数名称: spi1_write_read_uint16
- 函数功能: spi1 向外发送两个字节, 并读回两个字节
- 创建日期: 2016-11-07
----------------------------------------------------------------------------------------------------*/
uint16_t    spi1_write_read_uint16(uint16_t    val)
{
    uint32_t	cWk;
    
    __write_hw_reg32(SSI1_EN, 0x00);
    __write_hw_reg32(SSI1_DMAS, 0x00);
    __write_hw_reg32(SSI1_EN, 0x01);

    __write_hw_reg32(SSI1_DATA, val);

    do	{
        __read_hw_reg32(SSI1_STS, cWk);
    }while((cWk&0x05) != 0x4);
    
    __read_hw_reg32(SSI1_DATA, val);
    
    __write_hw_reg32(SSI1_EN, 0x00);    
    
    return	(val);    
}

/* ---------------------------------------------------------------------------------------------------
- 函数名称: spi1_write_read_stream
- 函数功能: spi1 向外发送并读回长度为Length的一串16BIT宽度数据.
- 创建日期: 2016-11-07
----------------------------------------------------------------------------------------------------*/
void    spi1_write_read_stream(uint16_t *input, uint16_t *output, uint16_t length)
{ 
    uint32_t    iwk;
    __write_hw_reg32(SSI1_EN, 0x00);
    __write_hw_reg32(SSI1_DMAS, 0x03);
    __write_hw_reg32(SSI1_DMATDL, 0x2);          
    __write_hw_reg32(SSI1_DMARDL, 0x2);              
    __write_hw_reg32(SSI1_EN, 0x01);

    //- RX Channel
    __write_hw_reg32(DMAS_CHx_SAR(11), 0x40014060);
    __write_hw_reg32(DMAS_CHx_DAR(11), (uint32_t)output);
    __write_hw_reg32(DMAS_CHx_CTL1(11),((2 << 8)|  1));
    __write_hw_reg32(DMAS_CHx_CTL0(11),(length<<1));
    __write_hw_reg32(DMAS_EN, 11);

    //- TX Channel
    __write_hw_reg32(DMAS_CHx_SAR(3), (uint32_t)input);
    __write_hw_reg32(DMAS_CHx_DAR(3), 0x40014060);
    __write_hw_reg32(DMAS_CHx_CTL1(3),((2 << 8)|  1));
    __write_hw_reg32(DMAS_CHx_CTL0(3),(length<<1));
    __write_hw_reg32(DMAS_EN, 3);

    do	{
    	__read_hw_reg32(DMAS_INT_RAW, iwk);
    }while((iwk&0x808) != 0x808);

    __write_hw_reg32(DMAS_INT_RAW, 0x808);
    __write_hw_reg32(DMAS_CLR, 11);
    __write_hw_reg32(DMAS_CLR, 3);
        
    __write_hw_reg32(SSI1_EN, 0x00);
   
}


#ifndef  TH25D_20UA

/******************************************2019.04.08新增******************************************/
/* ---------------------------------------------------------------------------------------------------
- 函数名称: spi_flash_Read
- 函数功能: 在指定的地址开始读取指定长度的数据
- 创建日期: 2019.04.08
- 作    者:陈俊伟
----------------------------------------------------------------------------------------------------*/
void	spi_flash_Read(uint32_t	ReadAddR, uint8_t *buff,uint16_t  ReadLength)
{
	
    uint32_t PagePos=ReadAddR/FLASH_PAGE_SIZE;//页地址
	uint16_t PageOff=ReadAddR%FLASH_PAGE_SIZE;//页偏移地址
	uint16_t i=0,PageMain=FLASH_PAGE_SIZE-PageOff;//页剩余地址
    if(ReadLength<=PageMain)  PageMain=ReadLength; 
	while(1)
	{   	
        spi_flash_Read_Page(PagePos*FLASH_PAGE_SIZE,rxbuff);
		for(i=0;i<PageMain;i++)
		{
			buff[i]=rxbuff[PageOff+i+4];	  
		}
        if(ReadLength==PageMain) break;
		else
		{
			PagePos++;
			PageOff=0;
			buff+=PageMain;
            ReadAddR+=PageMain;
			ReadLength-=PageMain;
			if(ReadLength>FLASH_PAGE_SIZE) PageMain=FLASH_PAGE_SIZE;
			else PageMain=ReadLength;
		}
    }
		
  
}
/* ---------------------------------------------------------------------------------------------------
- 函数名称: spi_flash_Read_Sector
- 函数功能: 从SPI FLASH中读取一个扇区数据到指定地址
- 创建日期: 2019.04.08
- 作    者:陈俊伟
----------------------------------------------------------------------------------------------------*/
void spi_flash_Read_Sector(uint32_t	ReadAddR, uint8_t *buff)
{
	uint8_t i=0,j=FLASH_SECTOR_SIZE/FLASH_PAGE_SIZE;
    uint16_t k=0;
	for(i=0;i<j;i++)
	{
		spi_flash_Read_Page(ReadAddR,rxbuff);
		for(k=0;k<FLASH_PAGE_SIZE;k++)
		{
			*buff=rxbuff[4+k];
			buff++;
		}
		ReadAddR+=FLASH_PAGE_SIZE;
	}
}
/* ---------------------------------------------------------------------------------------------------
- 函数名称: spi_flash_write_Sector
- 函数功能: 向SPI FLASH指定扇区写入数据
- 创建日期: 2019.04.08
- 作    者:陈俊伟
---------------------------------------------------------------*/
void	spi_flash_write_Sector(uint32_t	WriteAddR, uint8_t *buff)
{
	uint8_t i=0,j=FLASH_SECTOR_SIZE/FLASH_PAGE_SIZE;
	for(i=0;i<j;i++)
	{
        spi_flash_write_enable();
		spi_flash_write_page(WriteAddR,buff);
        while(spi_flash_wait_till_ready());
		WriteAddR+=FLASH_PAGE_SIZE;
		buff+=FLASH_PAGE_SIZE;	
	}
}
/* ---------------------------------------------------------------------------------------------------
- 函数名称: spi_flash_write
- 函数功能: 在指定的地址开始写入指定长度的数据
- 创建日期: 2019.04.08
- 作    者:陈俊伟
----------------------------------------------------------------------------------------------------*/

static volatile  uint8_t FlashBuf[FLASH_SECTOR_SIZE];
void	spi_flash_write(uint32_t	WriteAddR, uint8_t *buff,uint16_t WriteLength)
{
	uint32_t sectorpos;//扇区地址
	uint16_t sectoroff;//扇区偏移地址
	uint16_t sectorremain;//扇区剩余地址	   
 	uint16_t i,j;
	sectorpos=WriteAddR/FLASH_SECTOR_SIZE;
	sectoroff=WriteAddR%FLASH_SECTOR_SIZE;
	sectorremain=FLASH_SECTOR_SIZE-sectoroff;	
	if(WriteLength<=sectorremain)  sectorremain=WriteLength; 	
	while(1)
	{   
		spi_flash_Read_Sector(sectorpos*FLASH_SECTOR_SIZE,FlashBuf);        
		for(i=0;i<sectorremain;i++)
		{
			FlashBuf[sectoroff+i]=buff[i];	  
		}		
		spi_flash_write_enable();
		spi_flash_sector_erase(sectorpos);
		while(spi_flash_wait_till_ready());		
		spi_flash_write_Sector(sectorpos*FLASH_SECTOR_SIZE,FlashBuf);		
		if(WriteLength==sectorremain) break;
		else
		{
			sectorpos++;
			sectoroff=0;
			buff+=sectorremain;
            WriteAddR+=sectorremain;
			WriteLength-=sectorremain;
			if(WriteLength>FLASH_SECTOR_SIZE) sectorremain=FLASH_SECTOR_SIZE;
			else sectorremain=WriteLength;
		}
		
	}
}
#else  //使用紫光青藤flash
/******2020.10.14新增 以下接口目前适用于紫光青藤flash 以下接口目的是为省掉读写扇区所需要的大的缓冲区********/
/* ---------------------------------------------------------------------------------------------------
- 函数名称: spi_flash_Read
- 函数功能: 在指定的地址开始读取指定长度的数据
- 创建日期: 2020.10.14
----------------------------------------------------------------------------------------------------*/
void spi_flash_Read(uint32_t	ReadAddR, uint8_t *buff,uint16_t  ReadLength)
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
			spi_flash_Read_Page(pre_address,rxbuff,pre_len);	
			memcpy(&buff[buffIdx],&rxbuff[4],pre_len);
			buffIdx+= pre_len;
//			printf("rxbuff\r\n");
//			for(int i=0; i<pre_len; i++)
//			{
//				 printf("rxbuff[%d]:0x%x\r\n",i,rxbuff[i]);
//			}
			
		}
		pre_address+= pre_len;
		while(buffIdx < ReadLength)
		{
				if((ReadLength - buffIdx) >= FLASH_PAGE_SIZE)
				{
			//		printf("spi_flash_Read_Page0:0x%x,buffIdx:%d\n",pre_address,buffIdx);
					spi_flash_Read_Page(pre_address,rxbuff,FLASH_PAGE_SIZE);
					memcpy(&buff[buffIdx],&rxbuff[4],FLASH_PAGE_SIZE);
					buffIdx += FLASH_PAGE_SIZE;
					pre_address += FLASH_PAGE_SIZE;
				}else
				{
			//		printf("spi_flash_Read_Page1:0x%x,buffIdx:%d\n",pre_address,buffIdx);
					spi_flash_Read_Page(pre_address,rxbuff,ReadLength - buffIdx);
					memcpy(&buff[buffIdx],&rxbuff[4],ReadLength - buffIdx);
					buffIdx+=(ReadLength - buffIdx);
					pre_address+= (ReadLength - buffIdx);
				}
		}
		
	}
	return;
    uint32_t PagePos=ReadAddR/FLASH_PAGE_SIZE;//页地址
	
	uint32_t pageNum = alignmentUp(ReadLength,256) / FLASH_PAGE_SIZE;

	// ReadLength ReadLength / FLASH_PAGE_SIZE;
	uint16_t PageOff = ReadAddR % FLASH_PAGE_SIZE;//页偏移地址
	
	uint16_t i=0,PageMain=FLASH_PAGE_SIZE-PageOff;//页剩余地址
	
//	if()
	
    if(ReadLength<=PageMain)  PageMain=ReadLength; 
	while(1)
	{   	
        spi_flash_Read_Page(PagePos*FLASH_PAGE_SIZE,rxbuff,256);
		for(i=0;i<PageMain;i++)
		{
			buff[i]=rxbuff[PageOff+i+4];	  
		}
        if(ReadLength==PageMain) break;
		else
		{
			PagePos++;
			PageOff=0;
			buff+=PageMain;
            ReadAddR+=PageMain;
			ReadLength-=PageMain;
			if(ReadLength>FLASH_PAGE_SIZE) PageMain=FLASH_PAGE_SIZE;
			else PageMain=ReadLength;
		}
    }
		
  
}
/* ---------------------------------------------------------------------------------------------------
- 函数名称: spi_flash_write
- 函数功能: 在指定的地址开始写入指定长度的数据
- 创建日期:2020.10.14
 ----------------------------------------------------------------------------------------------------*/
void	spi_flash_write(uint32_t	WriteAddR, const uint8_t *buff,uint16_t WriteLength)
{
	
	uint32_t pre_address;
	uint32_t pre_len;
	uint32_t page0;
	uint32_t buffIdx = 0;
	
	page0 = (WriteAddR + FLASH_PAGE_SIZE) &(~FLASH_PAGE_MASK);
	
	{
		pre_address = WriteAddR;
		pre_len = page0 - pre_address;
		if(pre_len)
		{
			while(spi_flash_wait_till_ready());
			spi_flash_write_enable();
			while(spi_flash_wait_till_ready());
			spi_flash_write_page(pre_address,&buff[buffIdx],pre_len);	
			while(spi_flash_wait_till_ready());
			buffIdx+= pre_len;	
		}
		pre_address+= pre_len;
		while(buffIdx < WriteLength)
		{
				if((WriteLength - buffIdx) >= FLASH_PAGE_SIZE)
				{
					while(spi_flash_wait_till_ready());
					spi_flash_write_enable();
					while(spi_flash_wait_till_ready());
					spi_flash_write_page(pre_address,&buff[buffIdx],FLASH_PAGE_SIZE);
					buffIdx += FLASH_PAGE_SIZE;
					pre_address += FLASH_PAGE_SIZE;
				}else
				{
					while(spi_flash_wait_till_ready());
					spi_flash_write_enable();
					while(spi_flash_wait_till_ready());
					spi_flash_write_page(pre_address,&buff[buffIdx],WriteLength - buffIdx);
					while(spi_flash_wait_till_ready());
					buffIdx += (WriteLength - buffIdx);
					pre_address += (WriteLength - buffIdx);
				}
		}
		
	}
	return;
	
	uint8_t FlashBuf[FLASH_PAGE_SIZE ];
	uint32_t PagePos=WriteAddR/FLASH_PAGE_SIZE;//页地址
	uint16_t PageOff=WriteAddR%FLASH_PAGE_SIZE;//页偏移地址
	uint16_t i=0,PageMain=FLASH_PAGE_SIZE-PageOff;//页剩余地址
    if(WriteLength<=PageMain)  PageMain=WriteLength; 
	while(1)
	{   	
        spi_flash_Read_Page(PagePos*FLASH_PAGE_SIZE,rxbuff,256);
		for(i=0;i<FLASH_PAGE_SIZE;i++) FlashBuf[i]=rxbuff[i+4];
		for(i=0;i<PageMain;i++) FlashBuf[PageOff+i]=buff[i];	  	
		spi_flash_write_enable();
		spi_flash_page_erase(PagePos);
		while(spi_flash_wait_till_ready());	
        spi_flash_write_enable();
		spi_flash_write_page(PagePos*FLASH_PAGE_SIZE,FlashBuf,256 );
        while(spi_flash_wait_till_ready());			
        if(WriteLength==PageMain) break;
		else
		{
			PagePos++;
			PageOff=0;
			buff+=PageMain;
            WriteAddR+=PageMain;
			WriteLength-=PageMain;
			if(WriteLength>FLASH_PAGE_SIZE) PageMain=FLASH_PAGE_SIZE;
			else PageMain=WriteLength;
		}
    }
}

void	spi_flash_write_1(uint32_t	WriteAddR, uint8_t *buff,uint16_t WriteLength)
{

		while(spi_flash_wait_till_ready());	
        spi_flash_write_enable();
	//	spi_flash_write_data(WriteAddR,buff,WriteLength);
		//spi_flash_write_page(PagePos*FLASH_PAGE_SIZE,FlashBuf );
        while(spi_flash_wait_till_ready());			
      
}

#endif
/* END */


#if  0

#define SPI_CS_HIGHT     	    while(((*(SSI1_STS))&0x05)!=0x04);gpio_output_high(0)
#define SPI_CS_LOW   		    while(((*(SSI1_STS))&0x05)!=0x04);gpio_output_low(0)

void	Init_spi1_mode3_master(uint32_t freq)
{
    uint32_t    val;
    
    __write_hw_reg32(CPR_SPIx_MCLK_CTL(1), 0x110010);//1分频			//- spi(x)_mclk = 32Mhz(When TXCO=32Mhz).
    __write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL , (0x1000100<<1)); 	//- 打开spi(x) pclk.
    __read_hw_reg32(CPR_SSI_CTRL, val);
    val |= (1==0)? 0x01: 0x30;
    __write_hw_reg32(CPR_SSI_CTRL, val);
	
    __write_hw_reg32(SSIx_EN(1), 0x00);

    __write_hw_reg32(SSIx_IE(1), 0x00);
    __write_hw_reg32(SSIx_CTRL0(1) , 0xC7);					/* 8bit SPI data */

    __write_hw_reg32(SSIx_SE(1), 0x01);
    __write_hw_reg32(SSIx_BAUD(1), freq);						//- spix_mclk 分频.

    __write_hw_reg32(SSIx_RXFTL(1), 0x00);
    __write_hw_reg32(SSIx_TXFTL(1), 0x00);

    __write_hw_reg32(SSIx_EN(1) , 0x01);
    
    //cs线由GPIO0手动拉低拉高 手动模拟
    gpio_mux_ctl(0,0);gpio_fun_inter(0,0);gpio_fun_sel(0,0);gpio_direction_output(0);gpio_output_high(0);
    //sclk(GPIO1)
    gpio_mux_ctl(1,0);gpio_fun_inter(1,0);gpio_fun_sel(1,14);
    //miso(GPIO4)
    gpio_mux_ctl(4,0);gpio_fun_inter(4,0);gpio_fun_sel(4,16);
    //mosi(GPIO5)
    gpio_mux_ctl(5,0);gpio_fun_inter(5,0);gpio_fun_sel(5,17);
    
    
}

uint8_t SPI_Send_Dat(uint8_t dat)
{   
	__write_hw_reg32(SSI1_DATA, dat);
    while(((*(SSI1_STS))&0xC)!=0xC);
    return (*(SSI1_DATA));
}

uint8_t SPI_Receiver_Dat(void)
{
    __write_hw_reg32(SSI1_DATA, 0xFF);
    while(((*(SSI1_STS))&0xC)!=0xC);
    return (*(SSI1_DATA));
}

void w25qxx_Release_powerdown(void)
{
    SPI_CS_LOW;
    SPI_Send_Dat(0xAB);
    SPI_CS_HIGHT;
}

void w25qxx_Enter_powerdown(void)
{
    SPI_CS_LOW;
    SPI_Send_Dat(0xB9);
    SPI_CS_HIGHT;
}

uint8_t W25QXX_ReadSR(void)   
{  
	uint8_t byte=0;   
	SPI_CS_LOW;                              //使能器件 
	SPI_Send_Dat(0x05);                 //发送读取状态寄存器命令   
	byte=SPI_Receiver_Dat();          //读取一个字节  
	SPI_CS_HIGHT;                            //取消片选     
    return byte;  
       
} 

void W25QXX_Write_SR(uint8_t sr)   
{   
	SPI_CS_LOW;                            //使能器件   
	SPI_Send_Dat(0x01);                 //发送写取状态寄存器命令    
	SPI_Send_Dat(sr);               	//写入一个字节  
	SPI_CS_HIGHT;                            //取消片选     	      
}   

#endif

uint8_t test_tx_buff[360];

uint8_t test_rx_buff[560];
void  flash_test(void)
{
	Init_spi_master(0, SPIM_CLK_16MHZ);
	spi_flash_Read_128bitsID(rxbuff);
	
	spi_flash_Release_powerdown();
	printf("spi_flash_Read_128bitsID\n");
	for(int i = 0;i < 22;i++)
	{
	//	printf("buf[%d]:0x%x\n",i,rxbuff[i]); 
	}
	for(int i = 0; i < 360;i++)
	{
		test_tx_buff[i] = 0x1 + i;
	}
//	
//	while(spi_flash_wait_till_ready());	
//	spi_flash_write_enable();
//	while(spi_flash_wait_till_ready());	
//	spi_flash_write_page(132 * 1024+1,&test_tx_buff[0],250);
//	while(spi_flash_wait_till_ready());	
//	
//	
//	spi_flash_write_enable();
//	while(spi_flash_wait_till_ready());	
//	spi_flash_write_page(132 * 1024 +256 + 2,&test_tx_buff[0],250);
//	while(spi_flash_wait_till_ready());
	
#if 0		

		
	spi_flash_write_enable();
	while(spi_flash_wait_till_ready());	
	spi_flash_write_page(132 * 1024 + 4,&test_tx_buff[4],3);
	while(spi_flash_wait_till_ready());
	
	
		
	spi_flash_write_enable();
	while(spi_flash_wait_till_ready());	
	spi_flash_write_page(132 * 1024 + 7,&test_tx_buff[7],3);
	while(spi_flash_wait_till_ready());
	
	
		
	spi_flash_write_enable();
	while(spi_flash_wait_till_ready());	
	spi_flash_write_page(132 * 1024 + 10,&test_tx_buff[10],4);
	while(spi_flash_wait_till_ready());
	
	
		
//	spi_flash_write_enable();
//	while(spi_flash_wait_till_ready());	
//	spi_flash_write_page(132 * 1024 + 6,&test_tx_buff[6],1);
//	while(spi_flash_wait_till_ready());
	
	#endif
	
//	spi_flash_Read_Page(132 * 1024,rxbuff,18);
//	
//	for(int i = 0;i < 18;i++)
//	{
//		printf("buf0[%d]:%d\n",i,rxbuff[i]); 
//	}while(spi_flash_wait_till_ready());	
//	
//	for(int i = 0; i < 250;i++)
//	{
//		test_tx_buff[i] = 0XFF;
//	}
//	spi_flash_write_enable();
//	while(spi_flash_wait_till_ready());	
////	spi_flash_write_page(132 * 1024,txbuff,200);
//	while(spi_flash_wait_till_ready());	
	spi_flash_page_erase(132 * 1024);
	spi_flash_page_erase(132 * 1024 + 256);
	spi_flash_write(132 * 1024 + 1,test_tx_buff,357);;
	spi_flash_Read(132 * 1024 + 1 ,test_rx_buff,560);
	//spi_flash_Read_Page(132 * 1024 ,rxbuff,520);
//	p_buf = (uint8_t *)0x10000000;
//	
	for(int i = 0;i < 560;i++)
	{
		printf("rxbuf1[%d]:%d\n",i,test_rx_buff[i]); 
	}while(spi_flash_wait_till_ready());	

}
