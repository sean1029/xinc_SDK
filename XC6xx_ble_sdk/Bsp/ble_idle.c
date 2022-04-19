#include	"Includes.h"
#include "bsp_timer.h"

#define     RST_READY_TIME          	0x07        //- 
#define     OSC32_STABLE_TIME       	47//  2ms:47   6ms  180     
#define     WAIT_BLE_EXIT_LOWPWR_TIME   10 
#define     PRE_WAKEUP_TIME             (RST_READY_TIME + OSC32_STABLE_TIME + WAIT_BLE_EXIT_LOWPWR_TIME)
                                        
#define     CPR_SYS_TIME                    ((volatile unsigned *)(0x40002400 + 0x1C))
#define     CPR_SLP_CTL                     ((volatile unsigned *)(0x40002400 + 0x4))
#define     CPR_SLPCTL_INT_MASK             ((volatile unsigned *)(0x40002400 + 0x8))
#define     CPR_SLP_PD_MASK                 ((volatile unsigned *)(0x40002400 + 0xC))    
#define     CPR_SLP_SRC_MASK                ((volatile unsigned *)(0x40000000 + 0x000))



static  uint32_t    system_wakeup(uint16_t  val)
{

    Timer_disable(1);
    return 0;
}
#define     CPR_AO_BASE                     0x40002400

void    ble_system_idle_init(void)
{
      __write_hw_reg32(CPR_SYS_TIME, ((RST_READY_TIME<<12) | OSC32_STABLE_TIME)); 
      
      
      __write_hw_reg32(CPR_SLP_CTL, 0x00);                
      __write_hw_reg32(CPR_SLPCTL_INT_MASK, 0xFFFFFfAF); 
      __write_hw_reg32(CPR_SLP_PD_MASK,0x101);    
      __write_hw_reg32(CPR_SLP_SRC_MASK, 0x60006);

      *((volatile unsigned *)(CPR_AO_BASE + 0x40)) &=0x0F;            //ROM断电 
      *((volatile unsigned *)(CPR_AO_BASE + 0x44)) |=0x10;            //ROM断电隔离 深睡眠要将这两行注释来保证唤醒正常复位
      //*((volatile unsigned *)(CPR_AO_BASE + 0x20))  =0x2D;            //auto switch core ldo voltage
	  Timer_Register_Callback(system_wakeup, 1);
  
      /*gpio_direction_output(20);*/
}

extern	uint32_t	volatile testlp;
extern uint8_t   miss_rev_pkt_flag;
void    ble_system_idle(void)
{
        uint32_t    VAL, VAL1;
        
        VAL = ble_lowpower_remaining_clock(0);
        VAL1 = *(((volatile unsigned *)(0x40003000 + 0x0C )));


              
        if((VAL <= PRE_WAKEUP_TIME))//   || (miss_rev_pkt_flag ==1))
        {
            return;
        }
        
        __disable_irq();



        VAL = ble_lowpower_remaining_clock(1);
		if(VAL == 0xFFFFFFFF) {
			do {         
				VAL = timer_current_count(0);
				if(VAL != 0xFFFFFFFF) break;			
			} while(1);
		}
       
        if(VAL <= PRE_WAKEUP_TIME) {
            __enable_irq();
            return;        
        }

#if 0
            {
     
                    DMA_buf[0]=VAL&0XFF;
                    DMA_buf[1]=(VAL&0XFF00)>>8;
                    DMA_buf[2]=VAL1&0xff;
                    DMA_buf[3]=0XCC;
                    DMA_Uart_SendChar(1);
            }        
#endif      
        VAL = VAL - PRE_WAKEUP_TIME;      
        Init_Timer(1, VAL); 
        
        ble_enter_lowpower();

        __write_hw_reg32(CPR_SLP_PD_MASK,0x101);                         //加上此句 auto switch core ldo voltage 1.17v to 0.89v 才能一直生效
     
        //xc_rf_ldo_off();                                               //close rf_ldo
        *((volatile unsigned *)(CPR_AO_BASE + 0x50)) &=0xFFFFFFFE;       //close rf digital 手动开关RF总开关  
        *((volatile unsigned *)(CPR_AO_BASE + 0x40)) &=0x1E;             //cpr_ao_vdd_switch_en BT_MODEM断电
        *((volatile unsigned *)(CPR_AO_BASE + 0x44)) |=0x01;             //cpr_ao_vdd_iso_en    BT_MODEM断电隔离
        //*((volatile unsigned *)(CPR_AO_BASE + 0x20))  =0x2C;  
        __NOP();							
		__NOP();							
		__NOP(); 
		__WFI();							
		__NOP();							
		__NOP();							
		__NOP(); 
        *((volatile unsigned *)(CPR_AO_BASE + 0x40)) |=0x1;             //cpr_ao_vdd_switch_en BT_MODEM断电
        *((volatile unsigned *)(CPR_AO_BASE + 0x44)) &=0x1E;            //cpr_ao_vdd_iso_en    BT_MODEM断电隔离
        *((volatile unsigned *)(CPR_AO_BASE + 0x50)) |=0x1;             //open rf digital 手动开关RF总开关
        //xc_rf_ldo_on();                                                //open rf_ldo        
        ble_exit_lowpower(VAL);


        __enable_irq();
}
