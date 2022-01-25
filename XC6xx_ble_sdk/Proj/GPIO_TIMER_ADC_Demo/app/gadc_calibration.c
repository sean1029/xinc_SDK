#include    "platform.h"
#include    "fix_bsp_gpadc.h"
#include    "bsp_spi_master.h"

#define PROG_SECTOR_NUM 60 // = 4*60 = 240k
#define CALI_CENTER_VL  10000
#define CALI_OFFSET_VL  1000

int gadc_calibration_get(uint32_t *value)
{
	  uint32_t vl = 0;
	  uint32_t neg = 0;
	  uint8_t get_buf[12] = {0};
    Init_spi_master(0, SPIM_CLK_16MHZ);
	  spi_flash_Release_powerdown();
		spi_flash_Read(PROG_SECTOR_NUM*FLASH_SECTOR_SIZE,get_buf,8);
		
		spi_flash_Enter_powerdown();
		
		vl  = get_buf[0]*0x1000000 + get_buf[1]*0x10000 + get_buf[2]*0x100 + get_buf[3] ;
		neg = get_buf[4]*0x1000000 + get_buf[5]*0x10000 + get_buf[6]*0x100 + get_buf[7] ;
//		printf("\nflash data=");
//		for(int j=0;j<8;j++)
//		  printf(" %02x",get_buf[j]);
		if( (neg == ~vl) && ( (vl > CALI_CENTER_VL-CALI_OFFSET_VL)&&(vl < CALI_CENTER_VL+CALI_OFFSET_VL) ))
		{
			  *value = vl;
		    return 1;
		}	
		else
		{
		    return -1;
		}	
		  
}	