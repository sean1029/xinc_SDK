#include    "platform.h"


//void sm_peripheral_setup (void){

//}
//void _HWradio_Go_To_Idle_State_Patch (void){

//}
uint8_t		__attribute__((aligned(4)))	Wbuff[FLASH_PAGE_SIZE+4];
uint8_t		__attribute__((aligned(4)))	Rbuff[FLASH_PAGE_SIZE+4];

int test_flash_wrd(void) 
{
		
		uint16_t    i;

		Init_spi_master(0, SPIM_CLK_16MHZ);

        spi_flash_Release_powerdown();
		/****测试flash任意长度任意位置的读写****/
		uint8_t testWrite[162]={'A','B','C','D','E','F','G','H','I','J','K','L','M','N',
								 'O','P','Q','R','S','T','U','V','W','X','Y','Z',
								 'a','b','c','d','e','f','g','h','i','j','k','l','m','n',
								 'o','p','q','r','s','t','u','v','w','x','y','z',
								 '0','1','2','3','4','5','6','7','8','9'};

		uint8_t testRead[162]="";
		printf("\n#######1####################\n");
	//	spi_flash_write(132*1024,testWrite,126); //最大读写4096个字节
								 
void	spi_flash_write_1(uint32_t	WriteAddR, uint8_t *buff,uint16_t WriteLength);
								 
			spi_flash_write_enable();					 
			spi_flash_page_erase(0);					 
								 spi_flash_write_1(0,testWrite,32);
								 spi_flash_write_1(0 + 32,&(testWrite[32]),32);

		spi_flash_Read(0,testRead,62);
		for(i=0; i<62; i++) {
				printf(" %c ", testRead[i]);
		} 
		for(i=0; i<62; i++) {
				printf(" %x ", testRead[i]);
		} 


		/****读独立的ID序列 该指令可以读取一个芯片已经固化的只读的128位ID****/	
		printf("\n-----***----\n");
		uint8_t	ID_buf[16]={0};
		spi_flash_Read_128bitsID(ID_buf);	
		for(i=0; i<16; i++) {
			printf(" %#x ", ID_buf[i]);
		} 	
		printf("\n-----***----\n");

		spi_flash_Enter_powerdown();

		while(1);
			
}


