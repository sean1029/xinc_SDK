
/*----------------------------------------------------------------------------------------------------
	INCLUDE HEADE FILES
----------------------------------------------------------------------------------------------------*/
#include    <string.h>
#include    "Platform.h"
#include    "bsp_spi_flash.h"
#include    "bsp_register_macro.h"
#include 		"bsp.h"
#define alignmentDown(a,size)		(a & (~ (size -1)))
#define alignmentUp(a,size)		  ((a + size - 1) & (~ (size -1)))

#define  TH25D_20UA   //�Ϲ����ٵ�flash �ͺ�
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
NOTE: bsp_spi0_flash�ļ��µı���Ҫ����SHRAM0����.
-------------------------------------------------------------------*/
uint8_t		__attribute__((aligned(4)))	txbuff[(PACKET_FULL_LEN+4)];
uint8_t		__attribute__((aligned(4)))	rxbuff[(PACKET_FULL_LEN+4)];
//uint8_t		__attribute__((at(127*1024+0x10000000)))	txbuff[(PACKET_FULL_LEN+4)];
//uint8_t		__attribute__((at(127*1024+512+0x10000000)))	rxbuff[(PACKET_FULL_LEN+4)];

/* ---------------------------------------------------------------------------------------------------
- ��������: Init_spi_master
- ��������: ��ʼ��spi��ģʽ
- ��������: 2015-09-14
----------------------------------------------------------------------------------------------------*/
void	Init_spi_master(uint32_t ch, uint32_t freq)
{
    uint32_t    val;
    
    __write_hw_reg32(CPR_SPIx_MCLK_CTL(ch), 0x110010);//1��Ƶ			//- spi(x)_mclk = 32Mhz(When TXCO=32Mhz).
    __write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL , (0x1000100<<ch)); 	//- ��spi(x) pclk.
    __read_hw_reg32(CPR_SSI_CTRL, val);
    val |= (ch==0)? 0x01: 0x30;
    __write_hw_reg32(CPR_SSI_CTRL, val);
	
    __write_hw_reg32(SSIx_EN(ch), 0x00);

    __write_hw_reg32(SSIx_IE(ch), 0x00);
    __write_hw_reg32(SSIx_CTRL0(ch) , 0x0f);					/* 16bit SPI data */

    __write_hw_reg32(SSIx_SE(ch), 0x01);
    __write_hw_reg32(SSIx_BAUD(ch), freq);						//- spix_mclk ��Ƶ.

    __write_hw_reg32(SSIx_RXFTL(ch), 0x00);
    __write_hw_reg32(SSIx_TXFTL(ch), 0x00);

    //__write_hw_reg32(SSIx_EN(ch) , 0x01);
}
/* ---------------------------------------------------------------------------------------------------
- ��������: spi_flash_Read_128bitsID
- ��������: ��ȡflash  128bits ID ��������ID���� ��ָ����Զ�ȡһ��оƬ�Ѿ��̻���ֻ����128λID
- ��������: 2015-09-14
----------------------------------------------------------------------------------------------------*/
void spi_flash_Read_128bitsID(uint8_t *buff)
{
    while(spi_flash_wait_till_ready());
    uint32_t    iWK = 0;
  //  memset(txbuff,0,PACKET_FULL_LEN+4);
 //   memset(rxbuff,0,PACKET_FULL_LEN+4);
		txbuff[1] = 0x0;
	  txbuff[0] = CMD_ID;
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
    __write_hw_reg32(DMAS_CHx_CTL0(2) ,24);//���ջ������׵�ַ������4�ı������Ҵ�С�������ĵı���
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

void spi_flash_Read_MID(uint8_t *buff)
{
    while(spi_flash_wait_till_ready());
    uint32_t    iWK = 0;
  //  memset(txbuff,0,PACKET_FULL_LEN+4);
 //   memset(rxbuff,0,PACKET_FULL_LEN+4);
		txbuff[1] = 0x0;
	  txbuff[0] = 0X9F;
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
    __write_hw_reg32(DMAS_CHx_CTL1(10) ,((2 << 8)|  0));
    __write_hw_reg32(DMAS_CHx_CTL0(10) ,4);
    __write_hw_reg32(DMAS_EN , 10);

		//- TX Channel
	__write_hw_reg32(DMAS_CHx_SAR(2) , (uint32_t)txbuff);
    __write_hw_reg32(DMAS_CHx_DAR(2) , 0x40013060);
    __write_hw_reg32(DMAS_CHx_CTL1(2) ,((2 << 8)|  0));
    __write_hw_reg32(DMAS_CHx_CTL0(2) ,4);//���ջ������׵�ַ������4�ı������Ҵ�С�������ĵı���
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
    for(int i=0;i<4;i++)
          buff[i]=rxbuff[i]; 

}


/* ---------------------------------------------------------------------------------------------------
- ��������: spi_flash_Read_Page
- ��������: ��SPI FLASH�ж�ȡһҳ���ݵ�ָ����ַ
- ��������: 2015-09-14
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
- ��������: spi_flash_wait_till_ready
- ��������: ���spi flash����ready״̬
- ��������: 2015-09-14
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
- ��������: spi_flash_chip_erase
- ��������: ��������оƬ������
- ��������: 2015-09-16
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
- ��������: spi_flash_erase_block_num
- ��������: ���������block����Ŀ
- ��������: 2015-09-16
----------------------------------------------------------------------------------------------------*/
uint32_t	spi_flash_block_num(uint32_t	size)
{
		uint32_t	blk = 0;

		blk = size/FLASH_BLOCK_SIZE;

		if(size % FLASH_BLOCK_SIZE) blk++;

		return blk;
}

/* ---------------------------------------------------------------------------------------------------
- ��������: spi_flash_block_erase
- ��������: ����оƬָ��block������
- ��������: 2015-09-16
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
- ��������: spi_flash_write_enable
- ��������: ʹ�ܶ�оƬ��д����
- ��������: 2015-09-16
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
- ��������: spi_flash_write_page
- ��������: ��SPI FLASHָ����ַд��һҳ����
- ��������: 2015-09-16
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


#ifdef TH25D_20UA  //ʹ���Ϲ�����flash
/* ---------------------------------------------------------------------------------------------------
- ��������: spi_flash_page_erase   
- ��������: ����оƬָ��page������ ---�Ϲ�����flash�д˹��ܣ�GD���� �� MD���� flashû�д˹���
- ��������: 2016-06-15
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
- ��������: spi_flash_sector_erase
- ��������: ����оƬָ��sector������
- ��������: 2016-06-15
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
- ��������: spi_flash_Release_powerdown
- ��������: SPI_FLASH Exit Power-Down
- ��������: 2016-10-24
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
- ��������: spi_flash_powerdown
- ��������: SPI_FLASH Enter Power-Down
- ��������: 2016-11-07
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

 




/******2020.10.14���� ���½ӿ�Ŀǰ�������Ϲ�����flash ���½ӿ�Ŀ����Ϊʡ����д��������Ҫ�Ĵ�Ļ�����********/
/* ---------------------------------------------------------------------------------------------------
- ��������: spi_flash_Read
- ��������: ��ָ���ĵ�ַ��ʼ��ȡָ�����ȵ�����
- ��������: 2020.10.14
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

}
/* ---------------------------------------------------------------------------------------------------
- ��������: spi_flash_write
- ��������: ��ָ���ĵ�ַ��ʼд��ָ�����ȵ�����
- ��������:2020.10.14
 ----------------------------------------------------------------------------------------------------*/
void	spi_flash_write(uint32_t	WriteAddR, const uint8_t *buff,uint16_t WriteLength)
{
	
	uint32_t pre_address;
	uint32_t pre_len;
	uint32_t aliup_address;
	uint32_t buffIdx = 0;
	
	aliup_address = alignmentUp(WriteAddR,FLASH_PAGE_SIZE);

	
	pre_address = WriteAddR;
	pre_len = aliup_address - pre_address;
	if(pre_len)
	{
		pre_len = (pre_len > WriteLength) ? WriteLength:pre_len;
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
		
	
	return;

}



uint8_t test_tx_buff[360];

uint8_t test_rx_buff[560];

void  spi_flash_test(void)
{
	Init_spi_master(0, SPIM_CLK_16MHZ);
	spi_flash_Read_128bitsID(rxbuff);
	
	spi_flash_Release_powerdown();
	printf("spi_flash_Read_128bitsID\n");
	for(int i = 0;i < 22;i++)
	{
		printf("buf[%d]:0x%x\n",i,rxbuff[i]); 
	}
	
	spi_flash_Read_MID(test_rx_buff);
	printf("spi_flash_Read_MID\n");
	for(int i = 0;i < 4;i++)
	{
		printf("buf[%d]:0x%x\n",i,test_rx_buff[i]); 
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
	
#if 1		

		
//	spi_flash_write_enable();
//	while(spi_flash_wait_till_ready());	
//	spi_flash_write_page(132 * 1024 + 4,&test_tx_buff[4],3);
//	while(spi_flash_wait_till_ready());
//	
//	
//		
//	spi_flash_write_enable();
//	while(spi_flash_wait_till_ready());	
//	spi_flash_write_page(132 * 1024 + 7,&test_tx_buff[7],3);
//	while(spi_flash_wait_till_ready());
//	
//	
//		
//	spi_flash_write_enable();
//	while(spi_flash_wait_till_ready());	
//	spi_flash_write_page(132 * 1024 + 10,&test_tx_buff[10],4);
//	while(spi_flash_wait_till_ready());
//	
	
		
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
//	spi_flash_page_erase(132 * 1024);
//	spi_flash_page_erase(132 * 1024 + 256);
//	spi_flash_write(132 * 1024 + 1,test_tx_buff,357);;
//	spi_flash_Read(132 * 1024 + 1 ,test_rx_buff,560);
	//spi_flash_Read_Page(132 * 1024 ,rxbuff,520);
//	p_buf = (uint8_t *)0x10000000;
//	
//	for(int i = 0;i < 255;i++)
//	{
//		printf("rxbuf1[%d]:%d\n",i,test_rx_buff[i]); 
//	}while(spi_flash_wait_till_ready());	

}




uint8_t buff16[60];

#include "xinc_drv_spi.h"
#if NRFX_CHECK(XINCX_SPIM_ENABLED)
static const xinc_drv_spi_t m_spi = XINC_DRV_SPI_INSTANCE(0);  /**< SPI instance. */
static volatile bool spi_xfer_done;
static void spi_handler(xinc_drv_spi_evt_t const* p_event,
                        void *                    p_context)
{
//	printf("%s,type:%d\r\n",__func__,p_event->type);
//	
	uint8_t *tx_data = p_event->data.done.p_tx_buffer;
	uint8_t *rx_data = p_event->data.done.p_rx_buffer;
	spi_xfer_done = true;

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
#define		CMD_MID                  0x9F 
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

void spim_flash_Read_MID(uint8_t *buff)
{

		memset(txbuff,0,4);
		txbuff[0] = CMD_MID; 
		spi_xfer_done = false;									
		xinc_drv_spi_transfer(&m_spi,txbuff,4,rxbuff,4);
		while (!spi_xfer_done)
		{
			__WFE();
		}
		memcpy(buff,&rxbuff[1],3);
}

void	spim_flash_Release_powerdown(void)
{
		txbuff[0] = CMD_RELEASE_PWRDWN; 
		spi_xfer_done = false;									
		xinc_drv_spi_transfer(&m_spi,txbuff,1,rxbuff,1);
		while (!spi_xfer_done)
		{
			__WFE();
		}

}

void	spim_flash_Enter_powerdown(void)
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
   
		memset(&txbuff[4],0xff,len);
		spi_xfer_done = false;									
		xinc_drv_spi_transfer(&m_spi,txbuff,len + 4,rxbuff,len + 4);
		while (!spi_xfer_done)
		{
			__WFE();
		}	
		memcpy(data,&rxbuff[4],len);
}
void	spim_flash_page_erase(uint32_t	eraseAddr)
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
}
void	spim_flash_block_erase(uint32_t	blkAddr)
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

void	spim_flash_chip_erase(void)
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
- ��������: spi_flash_write
- ��������: ��ָ���ĵ�ַ��ʼд��ָ�����ȵ�����
- ��������:2020.10.14
 ----------------------------------------------------------------------------------------------------*/
void	spim_flash_write(uint32_t	WriteAddR, uint8_t *buff,uint16_t WriteLength)
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
	#define TEST_FLASH_ADDR   (132* 1024)
void spim_flash_test(void)
{
	
	//spi_flash_test();
	
//	return;

	ret_code_t err_code;
	uint8_t w_buf[200];
	uint8_t r_buf[200];
	const xinc_drv_spi_config_t spi_cfg = {
											.sck_pin      = SPIM1_SCK_PIN,
											.mosi_pin     = SPIM1_MOSI_PIN,
											.miso_pin     = SPIM1_MISO_PIN,
											.ss_pin       = SPIM1_SS_PIN,
											.irq_priority = 0,
											.orc          = 0xFF,
											.frequency    = (xinc_drv_spi_frequency_t) SPIM_FREQUENCY_FREQUENCY_K125,
											.mode         = XINC_DRV_SPI_MODE_0,
											.bit_order    = XINC_DRV_SPI_BIT_ORDER_MSB_FIRST,
									};
		err_code = xinc_drv_spi_init(&m_spi, &spi_cfg, spi_handler, NULL);
								
		printf("xinc_drv_spi_init:err_code:0x%x\r\n",err_code);
//	spim_flash_Read_MID(r_buf);
	//
		
		spim_flash_Read_128bitsID(r_buf);
		printf("ID:");
		for(int i = 0;i < 16;i++)
		{
			printf("%02x ",r_buf[i]);
			if(i % 16 == 15)
			{
				printf("\r\n");
			}
		}
		printf("\r\n");

		for(int i = 0;i < 0x80;i++)
		{
			w_buf[i] = i + 1;
		}
	  spim_flash_sector_erase(TEST_FLASH_ADDR);
		
		spim_flash_write(TEST_FLASH_ADDR,w_buf,0x20);
		

		
		spim_flash_read(TEST_FLASH_ADDR,r_buf,0x20);
		printf("read_data:\r\n");
		for(int i = 0;i < 0x20;i++)
		{
			printf("%02x ",r_buf[i]);
			if(i % 16 == 15)
			{
				printf("\r\n");
			}
		}
		printf("\r\n");
		

		

}
#endif //

