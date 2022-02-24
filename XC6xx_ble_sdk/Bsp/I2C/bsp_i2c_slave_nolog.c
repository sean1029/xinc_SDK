
#include	"Platform.h"


//#define printf(...)  (0) 

extern	void	i2c_slave_init(uint16_t	i2cAddR);
extern  void    i2c_slave_WriteSingle(uint8_t val);
extern  uint8_t i2c_slave_ReadSingle(void);
typedef	void	(*i2cHandler_callback_t)(uint32_t stat);

#define     I2C_SLAVE_RX_DONE       (1<<7)
#define     I2C_SLAVE_RD_REQ        (1<<5)
#define     I2C_SLAVE_RX_FULL       (1<<2)
#define     I2C_SLAVE_STOP          (1<<9)

 

/* ---------------------------------------------------------------------------------------------------
- ��������: i2c_master_StatusCheck
- ��������: ״̬���
- �������: �ڼ�bit; ��Ӧbit�ϵ�ֵ
- ��������: 
----------------------------------------------------------------------------------------------------*/
static	void	i2c_master_StatusCheck(uint8_t	nbit, uint8_t bitval)
{
	uint8_t	val;
	
	for(; ;)	{
		__read_hw_reg32(I2C_STATUS , val);
		if( bitval == ((val>>nbit)&0x01) ) break;
	}	

}


/* ---------------------------------------------------------------------------------------------------
- ��������: i2c_slave_init
- ��������: ��ʼ��i2c SLAVEģʽ
- �������: i2c������ַ
- ��������: 
----------------------------------------------------------------------------------------------------*/
extern	void	i2c_slave_init(uint16_t	i2cAddR)
{
	__write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL, 0x8000800);
	__write_hw_reg32(CPR_RSTCTL_SUBRST_SW, 0x400000);
	__write_hw_reg32(CPR_RSTCTL_SUBRST_SW, 0x400040);
	__write_hw_reg32(CPR_I2C_CLK_CTL, 0x110011);                //-> i2c_mclk = 16mhz.
    
    i2c_master_StatusCheck(0, 0);
	__write_hw_reg32(I2C_ENABLE, 0x00);  
	__write_hw_reg32(I2C_SAR, i2cAddR);		

	__write_hw_reg32(I2C_RX_TL, 0x00);
	__write_hw_reg32(I2C_TX_TL, 0x00);
		
	__write_hw_reg32(I2C_CON, 0x00);
   
 	__write_hw_reg32(I2C_INTR_EN, (I2C_SLAVE_RX_DONE | I2C_SLAVE_RD_REQ | I2C_SLAVE_RX_FULL | I2C_SLAVE_STOP )); 
    
    __write_hw_reg32(I2C_FS_SCL_HCNT, 12);
    __write_hw_reg32(I2C_FS_SCL_LCNT, 19); 
    __write_hw_reg32(I2C_SS_SCL_HCNT, 72);
    __write_hw_reg32(I2C_SS_SCL_LCNT, 79); 

    i2c_master_StatusCheck(0, 0);
    __write_hw_reg32(I2C_ENABLE, 0x1);
    
    NVIC_EnableIRQ(I2C_IRQn);

}
#define SELVE_ADDRESS 0xA0
/* ---------------------------------------------------------------------------------------------------
- ��������: I2C_Handler
- ��������: I2C�жϴ�����
- �������: ��
- ��������: 
----------------------------------------------------------------------------------------------------*/
void _Delay(uint32_t val)
{
  //Optimization level O0
  for(int i=0;i<val;i++)  __NOP();   //400 error
}
uint8_t I2C_buf[256]={0};
volatile uint8_t  I2C_index=0;

extern	void	I2C_Handler(void)
{
    uint32_t    stat, VAL;
    
     __read_hw_reg32(I2C_INTR_STAT, stat);
     //printf("%#x=>",stat);                                                                                                    //if(stat == 0) return; //if(stat & 0x200) __read_hw_reg32(I2C_CLR_STOP_DET, VAL);//-�� �� �� �� �� �� �� �� ��STOP_DET    //if(stat & 0x40)  __read_hw_reg32(I2C_CLR_TX_ABRT, VAL); //- ���˼Ĵ�������ж� TX_ABRT
    _Delay(800);
    __read_hw_reg32(I2C_CLR_INTR, VAL);//- ����ж� ���˼Ĵ����������жϣ����������ж��Լ�I2C_TX_ABRT_SOURCE �Ĵ���   
     if(stat & 0x80)//RX_DONE-��Ϊ slave ��������ʱ�����master û����Ӧ�Ļ����ڷ������һ���ֽں�������жϣ���ʾ���ͽ���
     {           
         VAL=*(I2C_DATA_CMD);         
         //printf("RX_DONE--%#x\n",VAL);  
     }    
     if(stat & 0x200)//IIC ֹͣ�ź�  --STOP_DET  ����������־ 1�����߳��ֽ�������  0����Ч
     {         
         VAL=*(I2C_DATA_CMD);    
         //printf("STOP_DET--%#x\n",VAL);  
     } 
     if(stat & 0x4) //��վ�յ���վ�ķ��͵ļĴ�����ַ (��Ӧ�������Ķ���д������)--RX_FULL RX FIFO ����־ 1��RX FIFO ���ݴ��ڻ��ߵ��� RX FIFO ��ֵ���˱����� FIFO״̬�Զ�����  0����Ч
     {
        uint8_t val= *(I2C_DATA_CMD);  
        if(val==(0xA0)) //д����
        {
            //*(I2C_DATA_CMD)=0x22;
            for(int i=0;i<6;i++)
            {
                _Delay(400);
               I2C_buf[I2C_index++]= *(I2C_DATA_CMD);
               //printf("write:%#2x\n",VAL);
            }
        }
        else if(val==0xA1)  //������
        {
           _Delay(100);
           *(I2C_DATA_CMD)=0xBB;
            //printf("read:0xBB\n");   
        }
        else if(val==0x03)//������2
        {
            *(I2C_DATA_CMD)=0xAA;
            //printf("read:0xAA\n");   
        } 
        else if(val==0x04)//������3
        {
            *(I2C_DATA_CMD)=0xCC;
            //printf("read:0xCC\n");     
        } 
        else//����������3
        { 
              *(I2C_DATA_CMD)=0xEE;
              //printf("read:0xEE\n");
        }            
         
     } 
     if(stat & 0x20) //RD_REQ  //��վ�յ���վ���ٴζ������жϣ�    
     {   //��Ϊ slave��������һ�� master���Դ� slave ������ʱ�������жϡ�Slave ���� I2C ����Ϊ�ȴ�״̬��SCL=0��ֱ���жϷ�����Ӧ��
         //���ʾ slave ��Զ�˵�master Ѱַ������Ҫ���䷢�����ݡ�������������Ӧ����жϣ��� �� �� �� �� �� �� �� д ��I2C_DATA_CMD �Ĵ���  
         static uint8_t count=1;
         *(I2C_DATA_CMD)=count++;
         //printf("read:%02X\n",count++); 
     } 
}

void test_slave_i2c()
{
    uint32_t active_count=0;
    //IIC ��������1 �Ƕ���0��д  ����״̬��IIC ��SCL SDA��Ϊ�ߵ�ƽ �ȴ�ʱ��סSCL�ߵ��͵�ƽ
    gpio_mux_ctl(4,0);gpio_fun_inter(4,0);gpio_fun_sel(4,6);//sda  6:I2C_SDA
    gpio_mux_ctl(5,0);gpio_fun_inter(5,0);gpio_fun_sel(5,5);//scl  5:I2C_SCL
    //i2c_slave_init(0x3D);//IIC��λ������IIC�ӻ���ַΪ0x7A(����0x3D<<1)
    i2c_slave_init((SELVE_ADDRESS>>1));
    
    while(1)
    {
       //if(*I2C_RAW_INTR_STAT&0x0100)//I2C error
       if(*I2C_STATUS&0x01){
           if((active_count++)>20000){
               active_count=0;
               i2c_slave_init((SELVE_ADDRESS>>1));
           }
       }else{
           active_count=0;
       }
    }

}
 