
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
- 函数名称: i2c_master_StatusCheck
- 函数功能: 状态检测
- 输入参数: 第几bit; 对应bit上的值
- 创建日期: 
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
- 函数名称: i2c_slave_init
- 函数功能: 初始化i2c SLAVE模式
- 输入参数: i2c器件地址
- 创建日期: 
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
- 函数名称: I2C_Handler
- 函数功能: I2C中断处理函数
- 输入参数: 无
- 创建日期: 
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
     //printf("%#x=>",stat);                                                                                                    //if(stat == 0) return; //if(stat & 0x200) __read_hw_reg32(I2C_CLR_STOP_DET, VAL);//-读 此 寄 存 器 清 除 中 断STOP_DET    //if(stat & 0x40)  __read_hw_reg32(I2C_CLR_TX_ABRT, VAL); //- 读此寄存器清除中断 TX_ABRT
    _Delay(800);
    __read_hw_reg32(I2C_CLR_INTR, VAL);//- 清除中断 读此寄存器清除组合中断，各个独立中断以及I2C_TX_ABRT_SOURCE 寄存器   
     if(stat & 0x80)//RX_DONE-作为 slave 发送数据时，如果master 没有响应的话，在发送最后一个字节后产生此中断，表示发送结束
     {           
         VAL=*(I2C_DATA_CMD);         
         //printf("RX_DONE--%#x\n",VAL);  
     }    
     if(stat & 0x200)//IIC 停止信号  --STOP_DET  结束条件标志 1：总线出现结束条件  0：无效
     {         
         VAL=*(I2C_DATA_CMD);    
         //printf("STOP_DET--%#x\n",VAL);  
     } 
     if(stat & 0x4) //从站收到主站的发送的寄存器地址 (并应答主机的读或写的请求)--RX_FULL RX FIFO 满标志 1：RX FIFO 数据大于或者等于 RX FIFO 阈值。此比特随 FIFO状态自动更新  0：无效
     {
        uint8_t val= *(I2C_DATA_CMD);  
        if(val==(0xA0)) //写命令
        {
            //*(I2C_DATA_CMD)=0x22;
            for(int i=0;i<6;i++)
            {
                _Delay(400);
               I2C_buf[I2C_index++]= *(I2C_DATA_CMD);
               //printf("write:%#2x\n",VAL);
            }
        }
        else if(val==0xA1)  //读命令
        {
           _Delay(100);
           *(I2C_DATA_CMD)=0xBB;
            //printf("read:0xBB\n");   
        }
        else if(val==0x03)//读命令2
        {
            *(I2C_DATA_CMD)=0xAA;
            //printf("read:0xAA\n");   
        } 
        else if(val==0x04)//读命令3
        {
            *(I2C_DATA_CMD)=0xCC;
            //printf("read:0xCC\n");     
        } 
        else//其他读命令3
        { 
              *(I2C_DATA_CMD)=0xEE;
              //printf("read:0xEE\n");
        }            
         
     } 
     if(stat & 0x20) //RD_REQ  //从站收到主站的再次读请求中断，    
     {   //作为 slave，当另外一个 master尝试从 slave 读数据时产生此中断。Slave 保持 I2C 总线为等待状态（SCL=0）直到中断服务被响应。
         //这表示 slave 被远端的master 寻址，并且要求其发送数据。处理器必须响应这个中断，并 将 待 发 送 的 数 据 写 入I2C_DATA_CMD 寄存器  
         static uint8_t count=1;
         *(I2C_DATA_CMD)=count++;
         //printf("read:%02X\n",count++); 
     } 
}

void test_slave_i2c()
{
    uint32_t active_count=0;
    //IIC 主机命令1 是读，0是写  空闲状态下IIC 的SCL SDA都为高电平 等待时擎住SCL线到低电平
    gpio_mux_ctl(4,0);gpio_fun_inter(4,0);gpio_fun_sel(4,6);//sda  6:I2C_SDA
    gpio_mux_ctl(5,0);gpio_fun_inter(5,0);gpio_fun_sel(5,5);//scl  5:I2C_SCL
    //i2c_slave_init(0x3D);//IIC上位机侦测的IIC从机地址为0x7A(等于0x3D<<1)
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
 