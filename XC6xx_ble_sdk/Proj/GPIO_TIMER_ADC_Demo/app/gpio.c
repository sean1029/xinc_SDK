
#include    "platform.h"
#include    "test_config.h"

void  xc_gpio(void) 
{    
       #if 0
       //将bootctrl swclk swd 配置成普通GPIO 浮空上拉输入无中断 模式
       gpio_mux_ctl(11,1);gpio_fun_sel(11,0);gpio_direction_input(11, 0);gpio_fun_inter(11,0);//bootctl
       gpio_mux_ctl(12,1);gpio_fun_sel(12,0);gpio_direction_input(12, 0);gpio_fun_inter(12,0);//swclk
       gpio_mux_ctl(13,1);gpio_fun_sel(13,0);gpio_direction_input(13, 0);gpio_fun_inter(13,0);//swd
       while(1)
       {
           if(gpio_input_val(11)) printf("11 High\n"); else printf("11 Low\n"); 
           if(gpio_input_val(12)) printf("12 High\n"); else printf("12 Low\n"); 
           if(gpio_input_val(13)) printf("13 High\n"); else printf("13 Low\n"); 
           for(int i=0;i<0x455000;i++);
       }
       #endif
       
       #if 0
       //将bootctrl swclk swd 配置成普通GPIO 浮空下拉输入上升沿中断 模式
       gpio_mux_ctl(11,1);gpio_fun_sel(11,0);gpio_direction_input(11, 1);gpio_fun_inter(11,5);//bootctl
       gpio_mux_ctl(12,1);gpio_fun_sel(12,0);gpio_direction_input(12, 1);gpio_fun_inter(12,5);//swclk
       gpio_mux_ctl(13,1);gpio_fun_sel(13,0);gpio_direction_input(13, 1);gpio_fun_inter(13,5);//swd
       while(1);
       #endif
       
       
       #if 0
        //将bootctrl swclk swd 配置成普通GPIO 推挽输出 模式
       gpio_mux_ctl(11,1);gpio_fun_sel(11,0);gpio_fun_inter(11,0);gpio_direction_output(11);//bootctl
       gpio_mux_ctl(12,1);gpio_fun_sel(12,0);gpio_fun_inter(12,0);gpio_direction_output(12);//swclk
       gpio_mux_ctl(13,1);gpio_fun_sel(13,0);gpio_fun_inter(13,0);gpio_direction_output(13);//swd
       while(1)
       {
          gpio_output_low(11);
          gpio_output_low(12);
          gpio_output_low(13);
          for(int i=0;i<0x455000;i++);
          gpio_output_high(11);
          gpio_output_high(12);
          gpio_output_high(13);
          for(int i=0;i<0x455000;i++);
       }
       #endif
 
}


