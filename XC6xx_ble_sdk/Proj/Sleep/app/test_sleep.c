#include    "platform.h"
 
 
extern void GPIO_Sleep_Config(void);
 void hardsleep_test(void)
{
    Init_spi_master(0, SPIM_CLK_16MHZ);
	spi_flash_Enter_powerdown();       //深睡将FLASH 设置为powerdown模式，可以优化8.3ua电流
    
    *((volatile unsigned *)(CPR_AO_BASE + 0x40)) &=0x0F;            //ROM断电 
    *((volatile unsigned *)(CPR_AO_BASE + 0x44)) |=0x10;            //ROM断电隔离
    *((volatile unsigned *)(CPR_AO_BASE + 0x40)) &=0x1E;             //cpr_ao_vdd_switch_en BT_MODEM断电
    *((volatile unsigned *)(CPR_AO_BASE + 0x44)) |=0x01;             //cpr_ao_vdd_iso_en    BT_MODEM断电隔离
    *((volatile unsigned *)(CPR_AO_BASE + 0x50)) &=0xFFFFFFFE;       //close rf digital 手动开关RF总开关

    
    GPIO_Sleep_Config();
	gpio_mux_ctl(GPIO_PIN_1,0);
	gpio_fun_sel(GPIO_PIN_1,GPIO_Dx);                     //配置睡眠唤醒脚GPIO1为普通GPIO脚
	gpio_direction_input(GPIO_PIN_1,1);                   //配置睡眠唤醒脚GPIO1输入下拉
	gpio_fun_inter(GPIO_PIN_1,RIS_EDGE_INT);              //配置 GPIO1上升沿中断
	NVIC_EnableIRQ(GPIO_IRQn);                            //使能GPIO中断 
	
    __write_hw_reg32(CPR_SLP_CTL, 0x00); //OSCEN_CTL =0x0, SYS_SLP_PD_MASK =0x1 关32M时钟，
	__write_hw_reg32(CPR_SLPCTL_INT_MASK, 0xFFFFFFF7);//将GPIO中断作为唤醒源，其他中断屏蔽掉
    __write_hw_reg32(CPR_SLP_PD_MASK, 0x101);   
	__write_hw_reg32(CPR_SLP_SRC_MASK, 0x60006);//睡眠源 M0 sleep 信号
 	__NOP();
    __NOP();
    __NOP();
    __WFI();//Wait For Interrupt 等待中断发生(任意中断)
	__NOP();							
	__NOP();							
	__NOP();
	__NOP();							
	__NOP();
  

}


typedef	struct	t_rtc       {
    
		uint32_t	    day;
		uint32_t	    hour;
		uint32_t	    minute;
		uint32_t	    second; 
		uint32_t	    week;
    
        uint32_t        week_alarm;
        uint32_t        interrupt;
}rtc_t ;
#define     RTC_INT_DAY         (1<<0)
#define     RTC_INT_HOUR        (1<<1)
#define     RTC_INT_MIN         (1<<2)
#define     RTC_INT_SEC         (1<<3)   
#define     RTC_INT_T1          (1<<5)
#define     RTC_INT_T2          (1<<4)
#define     RTC_INT_T3          (1<<6)


#define     RTC_ALARM_SUN       (1<<17)  
#define     RTC_ALARM_MON       (1<<18)
#define     RTC_ALARM_TUES      (1<<19)
#define     RTC_ALARM_WED       (1<<20)
#define     RTC_ALARM_THUR      (1<<21)
#define     RTC_ALARM_FRI       (1<<22)
#define     RTC_ALARM_SAT       (1<<23)
/* ---------------------------------------------------------------------------------------------------
- 函数名称: Init_rtc
- 函数功能: rtc初始化
- 输入参数: rtc初始化值
- 创建日期: 
----------------------------------------------------------------------------------------------------*/
void	Init_rtc(rtc_t *rtc)
{
        uint32_t    VAL;
    
        VAL = ((rtc->day)<<17) | ((rtc->hour)<<12) | ((rtc->minute)<<6) | (rtc->second);    
    
		__write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL, 0x20002);
		__write_hw_reg32(CPR_AOCLKEN_GRCTL, 0x20002);
		
        __write_hw_reg32(RTC_ICR, 0x00);
        __write_hw_reg32(RTC_CLR, VAL);
        __write_hw_reg32(RTC_WLR, rtc->week);					

        __write_hw_reg32(RTC_RAW_LIMIT, 32768);
        __write_hw_reg32(RTC_SECOND_LIMIT, 60);	
        __write_hw_reg32(RTC_MINUTE_LIMIT, 60);
        __write_hw_reg32(RTC_HOUR_LIMIT, 24);	
        
        VAL = 0x100;
        if(rtc->interrupt != 0)    {
            
            VAL |= (rtc->interrupt);
         	NVIC_EnableIRQ(RTC_IRQn);	   
        }
        __write_hw_reg32(RTC_ICR, VAL);             //- 使能RTC开始工作          
        
				
}
/* ---------------------------------------------------------------------------------------------------
- 函数名称: rtc_Current_VAL
- 函数功能: rtc当前时间
- 输入参数: 无
- 创建日期: 
----------------------------------------------------------------------------------------------------*/
void   rtc_Current_VAL(rtc_t *rtc)
{
        uint32_t    VAL;
        __read_hw_reg32(RTC_CCVR, VAL);
    
        rtc->day = (VAL >> 17) & 0x7FFF;
        rtc->hour = (VAL >> 12) & 0x1F;
        rtc->minute = (VAL >> 6)& 0x3F; 
        rtc->second =  VAL & 0x3F;
    
        __read_hw_reg32(RTC_WVR, VAL);    
        rtc->week = VAL & 0x07; 
        printf("%d===%d:%d:%d\n",rtc->day,rtc->hour,rtc->minute,rtc->second);
}

/* ---------------------------------------------------------------------------------------------------
- 函数名称: rtc_Alarm_Init
- 函数功能: rtc闹铃初始化
- 输入参数: rtc闹铃no(1~3); 闹铃值;
- 创建日期: 
----------------------------------------------------------------------------------------------------*/
void    rtc_Alarm_Init(uint32_t no, rtc_t * rtc)
{
        uint8_t     TAB[3] = {RTC_INT_T1, RTC_INT_T2, RTC_INT_T3};
        uint32_t    VAL;
        
        __ASSERT((no!=0)&&(no<4));
        no = no - 1;
    
        VAL = ((rtc->hour)<<12) | ((rtc->minute)<<6) | (rtc->second);  
        //VAL |= 1<<((rtc->week)+17);
        VAL |= rtc->week_alarm;
        __write_hw_reg32(RTC_CMR_X(no), VAL);
        __read_hw_reg32(RTC_ICR, VAL);
        VAL |= TAB[no];
        __write_hw_reg32(RTC_ICR, VAL);         //- 使能定时匹配中
		NVIC_EnableIRQ(RTC_IRQn);//使能RTC中断
}

 /* ---------------------------------------------------------------------------------------------------
- 函数名称: Init_rtc_gpio
- 函数功能: 初始化RTC当中的GPIO
- 输入参数: gpio编号
- 创建日期: 
- 双边沿中断 0x0c
- 下降沿中断 0x0b
- 上升沿中断 0x0a
- 低电平中断 0x09
- 高电平中断 0x08
----------------------------------------------------------------------------------------------------*/
void	Init_rtc_gpio(uint32_t	num,uint8_t inter_mode)
{
		uint32_t	val;
	    if(!((inter_mode>=0x08) &&(inter_mode<=0x0c))) return;
		__write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL, 0x20002);
		__write_hw_reg32(CPR_AOCLKEN_GRCTL, 0x20002);
    
		__read_hw_reg32(AO_GPIO_MODE , val);
		val &= ~(0x0f<<(num<<2));
		val |= (inter_mode<<(num<<2));
		__write_hw_reg32(AO_GPIO_MODE, val);
	
		__read_hw_reg32(AO_GPIO_CTL , val);
		val |= (1<<(8+num));
		__write_hw_reg32(AO_GPIO_CTL, val);
		NVIC_EnableIRQ(RTC_IRQn);	    
}
void	RTC_Handler(void)
{       
    __write_hw_reg32(RTC_EOI, 0x7f);      //清除RTC all timer中断
    __write_hw_reg32(AO_GPIO_CTL, 0x1F);  //清除RTC all GPIOx中断
    __write_hw_reg32(AO_GPIO_CTL, 0x100<<GPIO_PIN_1); //使能 RTC GPIO1中断
	for(int i=0;i<0x4450;i++);            //for a moment
}	

void deepsleep_test(void)
{
	rtc_t rtc={0};
    __write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL, 0x20002);
//    __write_hw_reg32(CPR_AOCLKEN_GRCTL, 0x20002);
    printf("RTC_ICR:%02X\n",*(RTC_ICR));
    rtc_Current_VAL(&rtc);
    for(int i=0;i<0x455000;i++);
    rtc_Current_VAL(&rtc);
    for(int i=0;i<0x455000;i++);
    rtc_Current_VAL(&rtc);
    for(int i=0;i<0x455000;i++);
    rtc_Current_VAL(&rtc);
    for(int i=0;i<0x455000;i++);
    rtc_Current_VAL(&rtc);
    
	Init_rtc(&rtc);
    if((rtc.second+15)>=60)
    {
        rtc.second=rtc.second+15-60;
        if((rtc.minute+1)>=60)
        {
            rtc.minute=0;
            if((rtc.hour+1)>=24)
            {
                rtc.hour=0;
                rtc.day+=1;
            }else
            {
                rtc.hour+=1;
            
            }
        }else
        {
           rtc.minute +=1;
        }
           
    }else
    {
    	rtc.second=rtc.second+15;//设定闹钟时间30S后
    }
	rtc.week_alarm=0xFE0000; //设置闹钟星期几有效，周一到周日都有效
	rtc_Alarm_Init(1,&rtc);  //设置闹钟--30s后唤醒--注唤醒后boot脚拉低或悬空则从flash启动	
    printf("RTC_ICR:%02X\n",*(RTC_ICR));
    rtc_Current_VAL(&rtc);
    for(int i=0;i<0x455000;i++);
    rtc_Current_VAL(&rtc);
    for(int i=0;i<0x455000;i++);
    rtc_Current_VAL(&rtc);
    for(int i=0;i<0x455000;i++);
    rtc_Current_VAL(&rtc);
    for(int i=0;i<0x455000;i++);
    rtc_Current_VAL(&rtc);
    
	Init_spi_master(0, SPIM_CLK_16MHZ);
	spi_flash_Enter_powerdown();     //深睡将FLASH 设置为powerdown模式，可以优化8.3ua电流
    
    SysTick->CTRL = 0;               //close systick
	NVIC_DisableIRQ(BLUETOOTH_IRQn); //close BLE Interrupt
   	__write_hw_reg32((volatile unsigned *)(0x40002400+0x3c),0xF);        //spi0 flash脚上下拉配置（如果不配置会漏电80多微安）

 	GPIO_Sleep_Config();
	
    
	gpio_mux_ctl(GPIO_PIN_1,0);
	gpio_fun_sel(GPIO_PIN_1,GPIO_Dx);                     //配置睡眠唤醒脚GPIO1为普通GPIO脚
	gpio_direction_input(GPIO_PIN_1,1);                   //配置睡眠唤醒脚GPIO1输入上拉	       
	Init_rtc_gpio(GPIO_PIN_1,0x0a);                       //使能 RTC GPIO1 上升沿中断 
	              
    __write_hw_reg32(CPR_SLP_CTL,0x0);                    //OSCEN_CTL =0x0, SYS_SLP_PD_MASK =0x0 关32M时钟，断电(已经测试过，7.7uA)
	__write_hw_reg32(CPR_SLP_PD_MASK,0x00);
    __write_hw_reg32(CPR_SLPCTL_INT_MASK,0xFFFFFFEF);// 不屏蔽，无法睡眠 -只将RTC中断唤醒解屏蔽
    //__write_hw_reg32(CPR_SLP_ST,0x00010001);
    __write_hw_reg32(CPR_SLP_SRC_MASK, 0x60006);//睡眠源 M0 sleep 信号
    __NOP();
    __NOP();
    __NOP();
    __WFI();//Wait For Interrupt 等待中断发生(任意中断)
	__NOP();							
	__NOP();							
	__NOP();
	__NOP();							
	__NOP();
   
}

#define TEST_DEEPSLEEP
int  main(void)
{
#ifdef TEST_DEEPSLEEP
   deepsleep_test();//一点几微安电流(注意防止串口或其他GPIO口，以及板子上其他东西漏电如:LED灯漏电)
#else
hardsleep_mark:
    gpio_fun_sel(GPIO_PIN_18,UART0_TX);gpio_fun_sel(GPIO_PIN_19,UART0_RX);
	printf("HARD SLEEP\n");
    hardsleep_test();//十多微安电流(注意防止串口或其他GPIO口，以及板子上其他东西漏电如:LED灯漏电)
	goto hardsleep_mark;
#endif
   while(1);

}

