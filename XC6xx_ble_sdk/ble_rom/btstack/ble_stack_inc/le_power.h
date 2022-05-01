#ifndef     __LE_POWER_H
#define     __LE_POWER_H

#include    "sys_debug_config.h" 

#if     (__DEBUG_LE_SLEEP == 2)
void    memset_ble_sleep(void);
uint32_t   ble_lowpower_remaining_clk(uint32_t stat);
void    load_advDelay(uint32_t advDelay);
void    Adv_Enter_LowPower(void);
void    Adv_Exit_LowPower(void);
uint32_t	do_Connection_Enter_LowPower(void);
uint32_t    do_Connection_Exit_LowPower(uint8_t IRQ_Type);
void    Connection_Close_LowPower(void) ;
void	native_bt_clk_restore_done(void);
uint32_t	native_bt_clk_result(void);



#else
#define     memset_ble_sleep(void) 
#define     ble_lowpower_remaining_clk(void)  	(0)
#define     load_advDelay(...)
#define     Adv_Enter_LowPower(void)   
#define     Adv_Exit_LowPower()
#define     do_Connection_Enter_LowPower(void)	(0)
#define     do_Connection_Exit_LowPower(...)    (0)
#define     Connection_Close_LowPower(void)
#define		native_bt_clk_restore_done(void)	
#define		native_bt_clk_result(void)			(0)


#endif

#endif
