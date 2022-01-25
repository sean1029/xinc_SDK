#include    "Platform.h"
#include    "bsp_timer.h"
#include    <math.h>
#include    "fix_bsp_gpadc.h"

#define CALI_CENTER_VL  10000
/* ---------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------*/
void fix_improving_adc_accuracy(void)
{
	*((volatile unsigned *)(0x40002400 + 0x20))  =0x2e;
	for(int i=0;i<0x455;i++);
}
/* ---------------------------------------------------------------------------------------------------
- 函数名称: adc_gpio_config
- 函数功能: ADC引脚配置
- 输入参数: 16bits 0---8bit位代表0--8通道，需要配置就对相应的bit位置1
- 创建日期: 2019-04-18
----------------------------------------------------------------------------------------------------*/
void fix_adc_gpio_config(uint16_t channels)
{
    if(channels==0)//通道0--GADC_AIN0--GPIO21
	{
        gpio_mux_ctl(21,0);
		gpio_fun_sel(21,0);
	    gpio_fun_inter(21,0); 
		gpio_direction_input(21, 3);
		
	}else if(channels==1)//通道1--GADC_AIN0--GPIO20
	{
        gpio_mux_ctl(20,0);
		gpio_fun_sel(20,0);
	    gpio_fun_inter(20,0); 
		gpio_direction_input(20, 3);
	}else if(channels==2)//通道2--GADC_AIN0--GPIO19
	{
        gpio_mux_ctl(19,0);
		gpio_fun_sel(19,0);
	    gpio_fun_inter(19,0); 
		gpio_direction_input(19, 3);
	}else if(channels==3)//通道3--GADC_AIN0--GPIO18
	{
        gpio_mux_ctl(18,0);
		gpio_fun_sel(18,0);
	    gpio_fun_inter(18,0); 
		gpio_direction_input(18, 3);
	}else if(channels==4)//通道4--GADC_AIN0--GPIO0
	{
        gpio_mux_ctl(0,0);
		gpio_fun_sel(0,0);
	    gpio_fun_inter(0,0); 
		gpio_direction_input(0, 3);
	}else if(channels==5)//通道5--GADC_AIN0--GPIO1 
	{
        gpio_mux_ctl(1,0);
		gpio_fun_sel(1,0);
	    gpio_fun_inter(1,0); 
		gpio_direction_input(1, 3);
	}else if(channels==6)//通道6--GADC_AIN0--GPIO4
	{
        gpio_mux_ctl(4,0);
		gpio_fun_sel(4,0);
	    gpio_fun_inter(4,0); 
		gpio_direction_input(4, 3);
	}else if(channels==7)//通道7--GADC_AIN0--GPIO5
	{
        gpio_mux_ctl(5,0);
		gpio_fun_sel(5,0);
	    gpio_fun_inter(5,0); 
		gpio_direction_input(5, 3);
	}else if(channels==8)//芯片供电电压的2/3 例如芯片供电为3.3V那么通道8采集电压为2.2V
	{
	
	}

    
}

/* ---------------------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------------------*/
void fix_gpadc_config_channel(uint16_t channel)
{		
	 __write_hw_reg32(GPADC_CHAN_CTL ,channel);
	 __write_hw_reg32(GPADC_FIFO_CTL , 0x10);
	 __write_hw_reg32(GPADC_FIFO_CTL , 0x00);
}

uint16_t fix_data_average(uint16_t *data,uint16_t len)
{
   uint16_t add_cnt = 0;
	 uint32_t sum_data = 0;
	 uint8_t  filter_cnt = 0;
	
	 filter_cnt = len/10;
	 
	 for(int i=0;i<len-1;i++)
	 {
	    for(int j=0;j<len-i-1;j++)
		  {
			   if(data[j]>data[j+1])
				 {
				     uint16_t temp = data[j];
					   data[j] = data[j+1];
					   data[j+1] = temp;
				 }	 
			}
	 }
   
	 for(int i=6*filter_cnt;i<len-3*filter_cnt;i++)
	 {
				sum_data += data[i];
				add_cnt++;
	 }
	 
	 if(0 == add_cnt) return -1;
	 else return sum_data/add_cnt;
}


/* ---------------------------------------------------------------------------------------------------
- 函数名称: Init_adc
- 函数功能: ADC初始化
- 输入参数:无
- 创建日期: 2019-04-18
- 作    者：陈俊伟
----------------------------------------------------------------------------------------------------*/
int fix_init_adc(uint8_t freq,uint8_t gadc_ref)
{
	 uint32_t adc_reg = 0;
	 __write_hw_reg32(CPR_RSTCTL_CTLAPB_SW , 0x10000000);/*先 使GPADC_RSTN=0，再使 GPADC_RSTN=1，软复位 GPADC 模块*/
	 __write_hw_reg32(CPR_RSTCTL_CTLAPB_SW , 0x10001000);/*先 使GPADC_RSTN=0，再使 GPADC_RSTN=1，软复位 GPADC 模块*/

    __write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL , 0x20002000);//使能GPADC_PCLK_EN 的GPADC_PCLK时钟

    __write_hw_reg32(GPADC_FIFO_CTL , 0x10);//对 FIFO 进 行 一 次 清 空 操 作
    __write_hw_reg32(GPADC_FIFO_CTL , 0x00);//中断阈值
	  
	  if((freq>GADC_FREQ_500K)||(freq<GADC_FREQ_8M)) freq = GADC_FREQ_1M;
	  if(GADC_REF_2_47V == gadc_ref) 
	    adc_reg = (freq<<8)|0x10 ;
		else
			adc_reg = (freq<<8)|0x12 ;
	  __write_hw_reg32(GPADC_RF_CTL ,adc_reg);// 配置寄存器 GPADC_RF_CTL   GPADC_PCLK/(gpadc_clkdiv*2)=16M/16=1M ,select 2.4V vref
    __write_hw_reg32(GPADC_MAIN_CTL , 0x09);// 配置寄存器 GPADC_MAIN_CTL 打开GPADC模块，，数据采集上升沿. 
	
	__write_hw_reg32(GPADC_TIMER0 ,4); //通道切换等待时间
	//NVIC_EnableIRQ(GADC_IRQn);
}





int16_t fix_gpadc_val_update(uint8_t update_ch)
{
	gadc_cache_t *temp_gadc_cache = NULL;
	uint16_t gadc_value[2*FIFO_DEEP] = {0};
	uint16_t gadc_count = 0;
	uint32_t val;
	temp_gadc_cache = (gadc_cache_t*)&val;
	
	for(int t=0;t<FIFO_DEEP;t++)
	{  
		 __read_hw_reg32(GPADC_FIFO,val);
		 if(update_ch == temp_gadc_cache->chanel_1) gadc_value[gadc_count++] = temp_gadc_cache->value_1;
		 if(update_ch == temp_gadc_cache->chanel_2) gadc_value[gadc_count++] = temp_gadc_cache->value_2;
		 
     if((update_ch != 0)&&(0 == temp_gadc_cache->chanel_1)) break; //empty
	}
	
	if(gadc_count<=2) return -1;
	else return fix_data_average(gadc_value+2,gadc_count-2); //Remove the first FIFO data
}



uint16_t count = 0;
char state=4;
void test_xc_gpadc(void)
{
	  int gadc_calibration_get(uint32_t *value);
	  uint16_t value = 0;
	  uint16_t value_calibrated = 0;
	  uint32_t calibration_param = 0;
	  if(gadc_calibration_get(&calibration_param)>0)
		{
		    printf("gadc_calibration_get success!,cali param=%d\n",calibration_param);
		}
    else
    {
			  calibration_param = 10000;
		    printf("gadc_calibration_get error!\n");
		}			
		
		fix_init_adc(GADC_FREQ_1M,GADC_REF_2_47V);
	  for(int i=0;i<4;i++)
		  fix_adc_gpio_config(i+state);
	  fix_gpadc_config_channel(state);
    while(1)
		{
			 //If the delay is less, you have to test it well !
		   for(int i=0;i<0x1000;i++);//delay
			 
			 value = fix_gpadc_val_update(state);
			 value_calibrated = value * calibration_param / CALI_CENTER_VL ;
			 int old_state = state;
			
			 switch(state)
			 {
                    
					case 4:fix_gpadc_config_channel(5);state=5;break;
					case 5:fix_gpadc_config_channel(6);state=6;break;
					case 6:fix_gpadc_config_channel(7);state=7;break;
					case 7:fix_gpadc_config_channel(4);state=4;break;
					default:break;
			 }
			 
			 if(count++>=1000)
			 {
			    count =0;
				  //printf("%1d-%3d  ",old_state,value);
				  printf("channel:%d===cali=%d===before cali Voltage:%f V,after cali Voltage:%f V \r\n",\
				            old_state,calibration_param,((value)*2.47)/(1.0*1024),((value_calibrated)*2.47)/(1.0*1024));
				  //printf("channel:%d======Voltage:%f V\r\n",old_state,((value_calibrated)*3.3)/(1.0*1024));
			 }	 
		}	
}

