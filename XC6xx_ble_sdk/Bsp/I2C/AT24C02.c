#include "Platform.h"
#include "bsp.h"
#include "AT24C02.h"
#include "xinc_drv_i2c.h"
#include "xinc_i2c.h"
#include <string.h>

#define I2C_INSTACE_ID	0


static const xinc_drv_i2c_t m_i2c = XINC_DRV_I2C_INSTANCE(I2C_INSTACE_ID);

#define	I2C_SCL_M	2				//i2c SCL pin
#define	I2C_SDA_M	3				//i2c SDA pin

//#define  CHIP_AT24C01 1
//#define  CHIP_AT24C02 2
//#define  CHIP_AT24C04 4
//#define  CHIP_AT24C08 8
#define  CHIP_AT24C16 16

#define AT24Cxx_ADDRESS		(0xA0 >> 1) // AT24C02 地址

#define AT24Cxx_ADDRESS_LEN		1

#if (CHIP_AT24C01)
#define AT24Cxx_PAGESIZE        0x8
#define AT24Cxx_ENDADDR         0x7F
#define AT24Cxx_PAGENUM					16
#elif (CHIP_AT24C02)
#define AT24Cxx_PAGESIZE        0x8
#define AT24Cxx_ENDADDR         0xFF
#define AT24Cxx_PAGENUM					32
#elif (CHIP_AT24C04)
#define AT24Cxx_PAGESIZE        0x10
#define AT24Cxx_ENDADDR         0x1FF
#define AT24Cxx_PAGENUM					32
#elif (CHIP_AT24C08)
#define AT24Cxx_PAGESIZE        0x10
#define AT24Cxx_ENDADDR         0x3FF
#define AT24Cxx_PAGENUM					64
#elif (CHIP_AT24C16)
#define AT24Cxx_PAGESIZE        0x10
#define AT24Cxx_ENDADDR         0x7FF
#define AT24Cxx_PAGENUM					128
#endif




#define AT24Cxx_DELAY			delay_ms(10)

static volatile bool m_xfer_done = false;

//I2C 事件处理函数

void i2c_event_handler(xinc_drv_i2c_evt_t const *p_event,void *p_context)
{
	switch (p_event->type)
	{
		//传输完成事件
	case NRF_DRV_I2C_EVT_DONE:
		m_xfer_done = true; //置位传输完成标志
		break;
	
	default:
		break;
	}
}

void i2c_at24c02_init(void)
{
	ret_code_t err_code;
	
	//定义并初始化 I2C 配置结构体
	const xinc_drv_i2c_config_t i2c_24c02_config = {
		.scl = I2C_SCL_M, // 定义 scl 引脚
		.sda = I2C_SDA_M, // 定义 sda 引脚
		.frequency = NRF_DRV_I2C_FREQ_400K,//
		.interrupt_priority = 0,
		.clear_bus_init = false //初始化期间不发送9个scl 时钟
	};

	err_code = xinc_drv_i2c_init(&m_i2c,&i2c_24c02_config,i2c_event_handler,NULL);

	APP_ERROR_CHECK(err_code);

	xinc_drv_i2c_enable(&m_i2c);

}

/*************************************************************
 * 功能 ：向AT24C02 指定的地址写入一个字节数据
 * 参数 ：WriteAddr[in]:地址
 * 		：dat[in]:写入的数据
 * 返回值：NRF_SUCCESS:写数据成功
 * ***********************************************************/
ret_code_t AT24Cxx_write_byte(uint16_t WriteAddr,uint8_t dat)
{
	ret_code_t err_code;
	uint8_t tx_buf[AT24Cxx_ADDRESS_LEN + 1];
	
	uint8_t PAGE;

	//检查写入数据的地址是否合法
	if (WriteAddr > AT24Cxx_ENDADDR)
	{
		return NRF_ERROR_INVALID_ADDR;
	}
	
	PAGE = WriteAddr / 256;
	//准备写入的数据
	tx_buf[0] = WriteAddr;
	tx_buf[1] = dat;

	//I2C 标志设置位false
	m_xfer_done = false;

	//写入数据
	err_code = xinc_drv_i2c_tx(&m_i2c,AT24Cxx_ADDRESS | PAGE,tx_buf,AT24Cxx_ADDRESS_LEN + 1,false);

	APP_ERROR_CHECK(err_code);

	//等待I2C 传输完成
//	while (false == m_xfer_done)
	{
		__nop();
	}
	//延时确保AT24C02将接收到的数据写入到eeprom
	AT24Cxx_DELAY;
	
	return err_code;
}

/*************************************************************
 * 功能 ：向AT24Cxx 指定的页面写入数据,数据长度不能超过一个页面的大小8个字节
 * 参数 ：page[in]:页面，1-32
 * 		：pdata[in]:指定待写入的数据缓存
 * 		size[in]:写入的数据长度，不能超过一个页面的大小8个字节
 * 返回值：NRF_SUCCESS:写数据成功
 * ***********************************************************/
ret_code_t AT24Cxx_write_page(uint8_t page,uint8_t const *pdata,uint8_t size)
{
	ret_code_t err_code;
	uint8_t tx_buf[AT24Cxx_ADDRESS_LEN + AT24Cxx_PAGESIZE];
	
	uint8_t PAGE;
	uint32_t sizeI;

	//检查写入数据的地址是否合法,写入的长度不能超过页面的大小
	if (size > AT24Cxx_PAGESIZE)
	{
		return NRF_ERROR_INVALID_LENGTH;
	}
	
	//检查写入的页面是否需合法，页面定义为 1~128，不在这个范围内判定为无效
	if ((page == 0) || (page > AT24Cxx_PAGENUM))
	{
		return NRF_ERROR_INVALID_ADDR;
	}

	//准备写入的数据
	tx_buf[0] = ((page - 1) * AT24Cxx_PAGESIZE)% 256;
	PAGE = ((page - 1) * AT24Cxx_PAGESIZE)/ 256;
	
	memcpy(tx_buf + AT24Cxx_ADDRESS_LEN,pdata,size);

	//I2C 标志设置位false
	m_xfer_done = false;

	//写入数据
	err_code = xinc_drv_i2c_tx(&m_i2c,AT24Cxx_ADDRESS | PAGE,tx_buf,AT24Cxx_ADDRESS_LEN + size,false);

	APP_ERROR_CHECK(err_code);
	//等待I2C 传输完成
//	while (false == m_xfer_done)
	{
		__nop();
	}
	//延时确保AT24C02将接收到的数据写入到eeprom
	AT24Cxx_DELAY;

	return err_code;
	
}

/*************************************************************
 * 功能 ：向AT24Cxx 指定的地址写入数据,函数内部实现了跨页写
 * 		：函数会检查AT24Cxx的地址空间是否足够写入数据
 * 参数 ：WriteAddr[in]:写入数据的起始地址
 * 		：p_buf[in]:指定待写入的数据缓存
 * 		size[in]:写入的数据长度
 * 返回值：NRF_SUCCESS:写数据成功
 * ***********************************************************/
ret_code_t AT24Cxx_write_buf(uint16_t WriteAddr,uint8_t  *p_buf,uint16_t size)
{
	ret_code_t err_code;	
	uint16_t current_addr = WriteAddr;
	uint8_t  page_addr = WriteAddr / 256;
	uint16_t sendlen = 0;
	uint8_t tx_buf[AT24Cxx_ADDRESS_LEN + AT24Cxx_PAGESIZE];

	//保存写入数据的起始地址
	tx_buf[0] = WriteAddr % 256;

	//检查AT24C02 剩余空间能否存放写入的数据
	if ((AT24Cxx_ENDADDR - WriteAddr) < size)
	{
		return NRF_ERROR_INVALID_LENGTH;
	}

	//连续写入数据，如果跨页，重新启动写流程
	while(size--)
	{
		if ((current_addr % AT24Cxx_PAGESIZE) == 0)
		{
			//到达页面起始地址并且发送长度不等于0，表示即将跨页
			if (sendlen != 0)
			{
				//I2C 标志设置位false
				m_xfer_done = false;
				page_addr = (current_addr - sendlen)/256;
				//执行一次写入操作
				err_code = xinc_drv_i2c_tx(&m_i2c,AT24Cxx_ADDRESS | page_addr,tx_buf,AT24Cxx_ADDRESS_LEN + sendlen,false);

				APP_ERROR_CHECK(err_code);
				//等待I2C 传输完成
				while (false == m_xfer_done)
				{
					__nop();
				}

				//清零发送长度
				sendlen = 0;
				
				//保存写入数据的起始地址
				tx_buf[0] = current_addr % 256;

				//延时确保AT24C02将接收到的数据写入到eeprom
				AT24Cxx_DELAY;
			}

			//数据保存到发送缓存tx_buf
			tx_buf[AT24Cxx_ADDRESS_LEN + sendlen++] = *(p_buf++);
			
		}
		else
		{
			//数据保存到发送缓存tx_buf
			tx_buf[AT24Cxx_ADDRESS_LEN + sendlen++] = *(p_buf++);

			//写入到最后的页面的数据读取完成
			if(size == 0)
			{
				//I2C 标志设置位false
				m_xfer_done = false;
				page_addr = (current_addr - sendlen)/256;
				//执行一次写入操作
				err_code = xinc_drv_i2c_tx(&m_i2c,AT24Cxx_ADDRESS | page_addr,tx_buf,AT24Cxx_ADDRESS_LEN + sendlen,false);

				APP_ERROR_CHECK(err_code);
				//等待I2C 传输完成
				while (false == m_xfer_done)
				{
					__nop();
				}

				//清零发送长度
				sendlen = 0;
				
				//延时确保AT24C02将接收到的数据写入到eeprom
				AT24Cxx_DELAY;	
			}

		}

		//地址加 1
		current_addr++;
		
		
	}
	return err_code;
}

/*************************************************************
 * 功能 ：从AT24Cxx 指定的地址连续读出数据
 * 参数 ：ReadAddr[in]:读出数据的起始地址
 * 		：p_buf[in]:指定保存读出的数据缓存
 * 		size[in]:读出的数据长度
 * 返回值：NRF_SUCCESS:读数据成功
 * ***********************************************************/
ret_code_t AT24Cxx_read_buf(uint16_t ReadAddr,uint8_t  *p_buf,uint16_t size)
{
	ret_code_t err_code;
	m_xfer_done = false;
	uint8_t  PAGE = ReadAddr / 256;
	//读数据的长度已经超出了AT24Cxx的地址范围
	if ((AT24Cxx_ENDADDR - ReadAddr) < size)
	{
		return NRF_ERROR_INVALID_LENGTH;
	}

	uint8_t tx_buf[AT24Cxx_ADDRESS_LEN];
	tx_buf[0] = ReadAddr;

	//I2C 标志设置位false
	m_xfer_done = false;
	//执行一次写入操作,写入读取数据的起始地址
	err_code = xinc_drv_i2c_tx(&m_i2c,AT24Cxx_ADDRESS | PAGE,tx_buf,AT24Cxx_ADDRESS_LEN ,false);
	
	APP_ERROR_CHECK(err_code);
	//等待I2C 传输完成
	while (false == m_xfer_done)
	{
		__nop();
	}

	
	//I2C 标志设置位false
	m_xfer_done = false;
	//读取数据
	err_code = xinc_drv_i2c_rx(&m_i2c,AT24Cxx_ADDRESS | PAGE,p_buf,size);
	
	APP_ERROR_CHECK(err_code);
	//等待I2C 传输完成
	while (false == m_xfer_done)
	{
		__nop();
	}

	return err_code;
}
uint8_t i2c_tx_buf[80];
uint8_t i2c_rx_buf[80];
void  i2c_at24c02_test(void)
{
 	i2c_at24c02_init();
	
	
	uint8_t dat ;
	uint8_t addr ;
	dat = 0x58;
	addr = 0;
	
//	AT24Cxx_write_byte(addr,dat);
//	printf("write %x to AT24Cxx addr:%d\r\n",dat,addr);
//	dat++;
//	addr++;
//	delay_ms(100);
//		
//	AT24Cxx_write_byte(addr,dat);
//	printf("write %x to AT24Cxx addr:%d\r\n",dat,addr);


//	delay_ms(100);
//	
//	printf("read AT24Cxx_read_buf \r\n");
//	AT24Cxx_read_buf(0,i2c_rx_buf,1);
//	printf("read data from address %d :%02x,%02x\r\n",0,i2c_rx_buf[0],i2c_rx_buf[1]);

//	delay_ms(200);
	//return;
	
	
//	for(int i = 0; i < 20;i++)
//	{
//		i2c_tx_buf[i] = 1 + i;
//	}

//	AT24Cxx_write_page(1,i2c_tx_buf,8);
//	printf("write data to AT24Cxx page 1\r\n");
//	delay_ms(200);
//	AT24Cxx_write_page(2,&i2c_tx_buf[8],8);
//	delay_ms(200);
//	AT24Cxx_read_buf(0,i2c_rx_buf,1);
//	printf("read data from address %d \r\n",0);
//	for(int i = 0; i < 1;i++)
//	{
//		printf("0x%02x ",i2c_rx_buf[i]);
//	}printf("\r\n");
//	
//	AT24Cxx_read_buf(0,i2c_rx_buf,2);
//	printf("read data from address %d \r\n",0);
//	for(int i = 0; i < 2;i++)
//	{
//		printf("0x%02x ",i2c_rx_buf[i]);
//	}printf("\r\n");
//	
//	AT24Cxx_read_buf(0,i2c_rx_buf,3);
//	printf("read data from address %d \r\n",0);
//	for(int i = 0; i < 3;i++)
//	{
//		printf("0x%02x ",i2c_rx_buf[i]);
//	}printf("\r\n");

//	delay_ms(200);
//	return;

	addr = 0xE3;
	for(int i = 0; i < 80;i++)
	{
		i2c_tx_buf[i] = 1 + i;
	}


	AT24Cxx_write_buf(addr,i2c_tx_buf,80);
  printf("write 20 bytes to AT24Cxx at address:%d\r\n",addr);

	delay_ms(200);
//return;
	AT24Cxx_read_buf(addr,i2c_rx_buf,80);
	
	
	printf("read data from address 0x%X \r\n",addr);
	for(int i = 0; i < 80;i++)
	{
		printf("%02d ",i2c_rx_buf[i]);
		if(i % 16 == 15)
		{
			printf("\r\n");
		}
	}printf("\r\n");
	
//	AT24Cxx_read_buf(addr,i2c_rx_buf,80);
//	
//	
//	printf("read data from address 0x%X \r\n",addr);
//	for(int i = 0; i < 80;i++)
//	{
//		printf("%02d ",i2c_rx_buf[i]);
//		if(i % 16 == 15)
//		{
//			printf("\r\n");
//		}
//	}printf("\r\n");


}

