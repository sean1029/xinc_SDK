
#include    "stdint.h"
#include "xinc_reg.h"

#define RF_BASE 0X4002F000

//#define QFN32
#define SSOP16
extern  void SER_WRITE(unsigned int regAddr,unsigned int regValue);
extern  unsigned int SER_READ(unsigned int regAddr,unsigned int *regValue);     
void ble_rccali(void);

typedef struct
{
    int chan;
    char* div_integer;
    char* div_fractional;    
}div_t;
// set bit
#define setbit(x,y) ((x) |= (1<<(y)))
#define clrbit(x,y) ((x) &= ~(1<<(y)))

#define SPI_CTL setbit(*(uint32_t volatile*)(0x40000000+0x130),0)
#define AHB_CTL clrbit(*(uint32_t volatile*)(0x40000000+0x130),0)

#define reg_read16(reg) (*(volatile unsigned short*)(reg))
#define reg_write16(reg,val) (*(volatile unsigned short*)(reg) = (val))


void wbit(uint16_t reg_addr,int start,int end,char* bit_str);
void set_ch(int ch);
                           
                            
uint16_t fpga_spi_read(uint32_t reg_addr)
{
    uint32_t spi_addr = reg_addr/4;   // read: MSB is 1
    uint32_t value; 
    //在这里添加spi读接口
    SER_READ(spi_addr, &value);
    
    return value;
}

int fpga_spi_write(uint32_t reg_addr, uint32_t reg_val)
{
    uint32_t spi_addr = reg_addr/4;             //write:MSB  is 0
    //在这个添加spi写接口  
    SER_WRITE(spi_addr, reg_val);
    
    return 0;
}


#define reversebit(x,y) (x)^=(1<<(y))
void Dcoc_Calib(uint16_t i,uint16_t q)
{
	uint16_t   dac_i;
	uint16_t   dac_q;
	uint16_t   calib_val;


    //求原码,高位取反
	reversebit(i,9);
	reversebit(q,9);
	

	float I = abs(i - 512);
	float Q = abs(q - 512);
	
	float adc_i = I*733/1024;
	float adc_q = Q*733/1024;
	dac_i = (uint16_t)(adc_i/13.6+0.5);
	dac_q = (uint16_t)(adc_q/13.6+0.5);
	
	if(i > 512)
	{
		dac_i = dac_i | 0x80;  //最高位符号1
	}

	if(q < 512)
	{
		dac_q = dac_q<<8 | 0x8000;   //最高位符号1,Q高16位
	}
	
	calib_val = dac_i | dac_q;

	fpga_spi_write(0x0098,calib_val);
}

void le_rf_init(void)
{
    wbit(0x54,14,12,"011");//tx dac gc def110   Optimization wave 
    wbit(0xB0,15,14,"11");
	
    //wbit(0x0054,11,6,"010000");// txgm gc  default 0dbm
	//wbit(0x0054,11,6,"010000");// txgm gc  default 0  dbm
	wbit(0x0054,11,6,"010100");// txgm gc  default 2  dbm
	//wbit(0x0054,11,6,"100000");// txgm gc  default 3.2dbm
	//wbit(0x0054,11,6,"100100");// txgm gc  default 4.5dbm
	//wbit(0x0054,11,6,"101000");// txgm gc  default 5.7dbm
    //wbit(0x0054,11,6,"101010");// txgm gc  default 6.3dbm
	//wbit(0x0054,11,6,"101100");// txgm gc  default 6.8dbm
	//wbit(0x0054,11,6,"110000");// txgm gc  default 7.7dbm
	
    ble_rccali(); // rc calibration	
}

// The format of the string is binary
void le_set_rf_tx_power(char* bit_str)
{
	wbit(0x0054,12,7,bit_str);
}

void wbit(uint16_t reg_addr,int end,int start,char* bit_str)
{
    uint16_t reg_val = 0;
    char ch = 0;
	int i = 0;


	if(strlen(bit_str) != (end-start+1))
	{
		while(1); //error
	}
#ifdef APB_REG
	reg_val = _reg_read16(RF_BASE+reg_addr); 
#else
    reg_val = fpga_spi_read(reg_addr);
#endif
   
    for(i=start;i<=end;i++)
    {
		
        int bit_idx = i-start;  //index of bit string
        
        ch = bit_str[(end-start)-bit_idx] - '0';
        if(ch==1)
        {
            setbit(reg_val,i);
        }
        else if(ch==0)
        {
            clrbit(reg_val,i);
        }
    }
	
#ifdef APB_REG
	_reg_write16(RF_BASE+reg_addr,reg_val);
#else
    fpga_spi_write(reg_addr,reg_val);
#endif
}

void w_regbit(uint32_t reg_addr,int end,int start,char* bit_str)
{
		uint16_t reg_val = 0;
		char ch = 0;
		int i = 0;
		
		if(strlen(bit_str) != (end-start+1))
		{
			while(1); //error
		}
		
		reg_val = reg_read16(RF_BASE+reg_addr);
		
		for(i=start;i<=end;i++)
		{
			
			int bit_idx = i-start;	//index of bit string
			
			ch = bit_str[(end-start)-bit_idx] - '0';
			if(ch==1)
			{
				setbit(reg_val,i);
			}
			else if(ch==0)
			{
				clrbit(reg_val,i);
			}
		}
		

		reg_write16(RF_BASE+reg_addr,reg_val);
}
//void xc_rf_ldo_on(void)
//{
//    AHB_CTL;
//    w_regbit(0x0034,15,8,"11111111");       
//    SPI_CTL;
//}
//void xc_rf_ldo_off(void)
//{
//    AHB_CTL;
//    w_regbit(0x0034,15,8,"00000000");
//    SPI_CTL;
//}
void ldo_on_rx(void)
{
	AHB_CTL;
	//ldoandsx en
		w_regbit(0x0034,15,0,"1111111111111111");//reg	sx 26m en and sx trx ldo en
		w_regbit(0x0064,15,9,"1111111");//reg sx trx ldo fc
		//en trx
	//	wbit(0x003C,14,10,"11110");// reg En rx
	//	wbit(0x003C,9,7,"111");// reg En tx
		w_regbit(0x003C,14,7,"11110000");// reg En rx
		
		//en adc
	//	wbit(0x0298,15,15,"1");// reg ISM ADC enable signal, high enable, Default 0
	//	wbit(0x0298,14,14,"1");// Enable for WF/BT ADC clk
		w_regbit(0x0298,15,14,"11");// reg ISM ADC enable signal, high enable, Default 0
	
		//en dac
	//	wbit(0x0260,15,15,"1");// reg Powerdown DAC, 1,enable
	//	wbit(0x0260,14,14,"1");// TX bias enable ,including DAC bias,0:pd 1en
	//	wbit(0x0260,13,13,"1");// Enable for WF/BT DAC clk
		w_regbit(0x0260,15,13,"111");
		w_regbit(0x0260,11,11,"1");// A signal resets DAC digital part,0:(def) reset, 1
	SPI_CTL;
}

void ldo_on_tx(void)
{
	AHB_CTL;
	//ldoandsx en
		w_regbit(0x0034,15,0,"1111111111111111");//reg	sx 26m en and sx trx ldo en
		w_regbit(0x0064,15,9,"1111111");//reg sx trx ldo fc
		//en trx
	//	wbit(0x003C,14,10,"11110");// reg En rx
	//	wbit(0x003C,9,7,"111");// reg En tx
		w_regbit(0x003C,14,7,"00000111");// reg En rx
		
		//en adc
	//	wbit(0x0298,15,15,"1");// reg ISM ADC enable signal, high enable, Default 0
	//	wbit(0x0298,14,14,"1");// Enable for WF/BT ADC clk
		w_regbit(0x0298,15,14,"11");// reg ISM ADC enable signal, high enable, Default 0
	
		//en dac
	//	wbit(0x0260,15,15,"1");// reg Powerdown DAC, 1,enable
	//	wbit(0x0260,14,14,"1");// TX bias enable ,including DAC bias,0:pd 1en
	//	wbit(0x0260,13,13,"1");// Enable for WF/BT DAC clk
		w_regbit(0x0260,15,13,"111");
		w_regbit(0x0260,11,11,"1");// A signal resets DAC digital part,0:(def) reset, 1
		SPI_CTL;
}

void ldo_on(void)
{
	AHB_CTL;
	//ldoandsx en
	w_regbit(0x0034,15,0,"1111111111111111");//reg  sx 26m en and sx trx ldo en
	w_regbit(0x0064,15,9,"1111111");//reg sx trx ldo fc
	//en trx
//	wbit(0x003C,14,10,"11110");// reg En rx
//	wbit(0x003C,9,7,"111");// reg En tx
	w_regbit(0x003C,14,7,"11110111");// reg En rx
	
	//en adc
//	wbit(0x0298,15,15,"1");// reg ISM ADC enable signal, high enable, Default 0
//	wbit(0x0298,14,14,"1");// Enable for WF/BT ADC clk
	w_regbit(0x0298,15,14,"11");// reg ISM ADC enable signal, high enable, Default 0

	//en dac
//	wbit(0x0260,15,15,"1");// reg Powerdown DAC, 1,enable
//	wbit(0x0260,14,14,"1");// TX bias enable ,including DAC bias,0:pd 1en
//	wbit(0x0260,13,13,"1");// Enable for WF/BT DAC clk
	w_regbit(0x0260,15,13,"111");
	w_regbit(0x0260,11,11,"1");// A signal resets DAC digital part,0:(def) reset, 1
	SPI_CTL;
}

void ldo_on_debug(void)
{
	AHB_CTL;
    w_regbit(0x0034,15,8,"11111111");//reg  sx 26m en and sx trx ldo en wbit(0x0014,15,15,"1");// reg_LDO_DEBUG, ADDA_BBVCO cap-ldo enable
    w_regbit(0x0014,15,15,"1");// reg_LDO_DEBUG, ADDA_BBVCO cap-ldo enable
    //w_regbit(0x0298,15,15,"1");// reg ISM ADC enable signal, high enable, Default 0
	SPI_CTL;
}
void ldo_off_debug(void)
{
	AHB_CTL;
    w_regbit(0x0034,15,8,"00000000");//reg  sx 26m en and sx trx ldo en 
    w_regbit(0x0014,15,15,"0");// reg_LDO_DEBUG, ADDA_BBVCO cap-ldo enable
    //w_regbit(0x0298,15,15,"0");// reg ISM ADC enable signal, high enable, Default 0
	SPI_CTL;			
}


void ldo_off(void)
{
	AHB_CTL;

		//ldoandsx off
		w_regbit(0x0034,15,0,"0000000000000000");//reg	sx 26m en and sx trx ldo en
		w_regbit(0x0064,15,9,"0000000");//reg sx trx ldo fc
		//en trx
	//	wbit(0x003C,14,10,"11110");// reg En rx
	//	wbit(0x003C,9,7,"111");// reg En tx
		w_regbit(0x003C,14,7,"00000000");// reg En rx
		
		//en adc
	//	wbit(0x0298,15,15,"1");// reg ISM ADC enable signal, high enable, Default 0
	//	wbit(0x0298,14,14,"1");// Enable for WF/BT ADC clk
		w_regbit(0x0298,15,14,"00");// reg ISM ADC enable signal, high enable, Default 0
	
		//en dac
	//	wbit(0x0260,15,15,"1");// reg Powerdown DAC, 1,enable
	//	wbit(0x0260,14,14,"1");// TX bias enable ,including DAC bias,0:pd 1en
	//	wbit(0x0260,13,13,"1");// Enable for WF/BT DAC clk
		w_regbit(0x0260,15,13,"000");
		w_regbit(0x0260,11,11,"0");// A signal resets DAC digital part,0:(def) reset, 1
	SPI_CTL;			
}

uint16_t rbit(uint16_t reg_addr)
{
    uint16_t reg_val = 0;
#ifdef APB_REG
	reg_val = _reg_read16(RF_BASE+reg_addr);
#else
    reg_val = fpga_spi_read(reg_addr);
#endif
    return reg_val;
}


void ble_rccali(void)
{
    uint16_t val=0;
    
    //wbit(0x0080,10,8,"101");//RG_RCCAL_CTRL(Rccal 输出码值控制) "101"
    wbit(0x0080,10,8,"110");//RG_RCCAL_CTRL(Rccal 输出码值控制) "101"
    
    wbit(0x0080,5,5,"1");//RG_RCCAL_RESETN=1
    wbit(0x0080,3,3,"0");//RG_RCCAL_SEL =0
    wbit(0x0080,2,2,"1");//RG_RCCAL_EN=1
    wbit(0x0080,4,4,"1");//RG_RCCAL_START=1
    
    while(!(rbit(0x0084)&0x8000));//等待AD_RCCAL_FINISH拉高 得出校准值 AD_RCCAL_CTRIM
    val=rbit(0x0084);
    val=(val&0x7FFF)>>10;// 将校准值AD_RCCAL_CTRIM保存
    
    wbit(0x0080,3,3,"1");//RG_RCCAL_SEL =1
    wbit(0x0080,2,2,"0");//RG_RCCAL_EN=0
    wbit(0x0080,4,4,"0");//RG_RCCAL_START=0

    //将保存的校准值转换成字符串
    char temp_buf[5]="";
    temp_buf[4]=((val&0x01) ? '1':'0');
    temp_buf[3]=((val&0x02) ? '1':'0');
    temp_buf[2]=((val&0x04) ? '1':'0');
    temp_buf[1]=((val&0x08) ? '1':'0');
    temp_buf[0]=((val&0x10) ? '1':'0');
    
    wbit(0x0080,15,11,temp_buf);
    
	
	
}
