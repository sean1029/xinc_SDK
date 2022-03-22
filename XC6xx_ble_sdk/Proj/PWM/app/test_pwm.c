#include    "platform.h"
#include    "bsp_pwm.h"

int main(void)
{   
	Init_gpio();
    uint8_t cnt = 0;
    printf("test PWM");
    #if 0
    //参数：pwm口（0或1）,占空比设置（范围0--99），周期计数器设置影响pwm信号输出频率(输出频率=pwm_clk/((period + 1)*100))
	gpio_mux_ctl(GPIO_PIN_11,1);gpio_fun_sel(GPIO_PIN_11,PWM0);     //GPIO11(bootctrl) 复用功能为pwm0
    gpio_mux_ctl(GPIO_PIN_22,0);gpio_fun_sel(GPIO_PIN_22,PWM0_INV); //GPIO22 复用功能为pwm0_inv pwm0与pwm0_inv为一对互补输出
    xc_pwm_init(0,66,100);//初始化PWM0
	 
    gpio_mux_ctl(GPIO_PIN_24,0);gpio_fun_sel(GPIO_PIN_24,PWM1);    //GPIO22 复用功能为pwm1 
    gpio_mux_ctl(GPIO_PIN_25,0);gpio_fun_sel(GPIO_PIN_25,PWM1_INV);//GPIO23 复用功能为pwm1_inv   pwm1与pwm1_inv为一对互补输出
    xc_pwm_init(1,77,100);//初始化PWM1
	
    
    gpio_mux_ctl(GPIO_PIN_0,2);  xc_pwm_init(2,11,100);//初始化PWM2  --mux=2时 pwm2固定在GPIO0上输出
	gpio_mux_ctl(GPIO_PIN_1,2);  xc_pwm_init(3,22,100);//初始化PWM3  --mux=2时 pwm3固定在GPIO1上输出
    gpio_mux_ctl(GPIO_PIN_12,3); xc_pwm_init(4,33,100);//初始化PWM4  --mux=3时 pwm4固定在GPIO12(swclk)上输出
	gpio_mux_ctl(GPIO_PIN_13,3); xc_pwm_init(5,44,100);//初始化PWM5  --mux=3时 pwm5固定在GPIO13(swd)上输出
    #endif
    
    
    #if 1
    //注意period最大为255
    //pwm输出频率=1.6M/((period+1)*100)
    //比如要输出200HZ的频率 period=(1.6M/(200*100))-1=79
    //比如要输出(1000/6)HZ的频率 period=95
    //比如要输出100HZ的频率 period=(1.6M/(100*100))-1=159
    gpio_fun_sel(GPIO_PIN_4,PWM0);//配置GPIO0复用成PWM0
	gpio_fun_sel(GPIO_PIN_5,PWM1);//配置GPIO1复用成PWM1
	xc_pwm_init(0,20,79);//初始化PWM0 -占空比百分之六十六 -输出频率200HZ
	xc_pwm_init(1,80,79);//初始化PWM1 -占空比百分之六十六 -输出频率200HZ
    uint32_t val;
    __read_hw_reg32(CPR_CTLAPBCLKEN_GRCTL,val);
	printf("CPR_CTLAPBCLKEN_GRCTL val: 0x%x\r\n",val);
    for(cnt = 1 ; cnt <= 99;cnt++)
    {
        for(int i=0;i<0x45500;i++);//延时1s
        xc_pwm_init(0,(cnt)*1,79);//初始化PWM0 -占空比百分之六十六 -输出频率200HZ
        xc_pwm_init(1,100 - (cnt)*1,79);//初始化PWM1 -占空比百分之六十六 -输出频率200HZ
        printf("set:%d\r\n",cnt);
        if(cnt == 99)
        {
            cnt = 1;
        }            
	
    }
	
	
//	gpio_fun_sel(GPIO_PIN_0,GPIO_Dx);//配置GPIO0复用成普通GPIO口
//	gpio_fun_sel(GPIO_PIN_1,GPIO_Dx);//配置GPIO1复用成普通GPIO口
//	gpio_direction_output(GPIO_PIN_0);//GPIO0配置成输出模式
//	gpio_direction_output(GPIO_PIN_1);//GPIO1配置成输出模式
//	gpio_output_high(GPIO_PIN_0);//GPIO0输出高电平
//	gpio_output_high(GPIO_PIN_1);//GPIO1输出高电平
	
	for(int i=0;i<0x455000;i++);//延时1s
	
//	gpio_fun_sel(GPIO_PIN_0,PWM0);//配置GPIO0复用成PWM0
//	gpio_fun_sel(GPIO_PIN_1,PWM1);//配置GPIO1复用成PWM1
	xc_set_pwm(GPIO_PIN_0,33,159);//-占空比百分之三十三 -输出频率100HZ
	xc_set_pwm(GPIO_PIN_1,44,159);//-占空比百分之四十四 -输出频率100HZ
	
	
	for(int i=0;i<0x455000;i++);//延时1s
	
	gpio_fun_sel(GPIO_PIN_0,GPIO_Dx);//配置GPIO0复用成普通GPIO口
	gpio_fun_sel(GPIO_PIN_1,GPIO_Dx);//配置GPIO1复用成普通GPIO口
	gpio_direction_output(GPIO_PIN_0);//GPIO0配置成输出模式
	gpio_direction_output(GPIO_PIN_1);//GPIO1配置成输出模式
	gpio_output_low(GPIO_PIN_0);//GPIO0输出低电平
	gpio_output_low(GPIO_PIN_1);//GPIO1输出低电平
	
	
	for(int i=0;i<0x455000;i++);//延时1s
	
	gpio_fun_sel(GPIO_PIN_0,PWM0);//配置GPIO0复用成PWM0
	gpio_fun_sel(GPIO_PIN_1,PWM1);//配置GPIO1复用成PWM1
	xc_set_pwm(GPIO_PIN_0,11,95);//-占空比百分之十一   -输出频率(1000/6)HZ
	xc_set_pwm(GPIO_PIN_1,22,95);//-占空比百分之二十二 -输出频率(1000/6)HZ

    
    
    #endif
    
	while(1);
}
