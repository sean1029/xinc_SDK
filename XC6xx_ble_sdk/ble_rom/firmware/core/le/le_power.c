
#include    "sys_types.h"
#include    "sys_hal_types.h"
#include    "hw_lc_impl.h"
#include    "le_adv.h"
#include    "le_config.h"
#include    "le_connection.h"
#include    "le_power.h"
#include    "le_const.h"
#include    "patch_function_id.h"


#if  (__DEBUG_LE_SLEEP == 2)

typedef     struct  {

    u_int32     volatile    adv_sleep_stat;
    u_int32     volatile    con_sleep_stat;

    /*u_int32     con_slave_connInterval_clk;*/
    u_int32     con_slave_current_clk;
    u_int32     con_slave_aux_timer_clk;
    u_int32     con_slave_aux_timer_Load;
    u_int32     con_slave_aux_timer_offset;
	
    u_int32     con_slave_wakeup_bt_clk;
    u_int32     con_slave_wakeup_Intraslot_offset;

    u_int32   	con_slave_wakeup_native_clk;
    u_int32 	native_clk_set;

    u_int32     con_slave_wait_tim2_Flg;

    u_int32     con_compensate_phase_flg;
    u_int32     con_intra_slot_save;
    int		    con_bt_clk_native_clk_margin;
    t_LE_Connection *pConnection; 
    u_int32     volatile    real_exit_lowpower;
}t_ble_sleep;
extern t_ble_sleep     ble_sleep;

extern uint8_t wakeup_clk_val;

void	ble_firmware_exit_lowpower(void);

/* ---------------------------------------------------------------------------------------------------
- μí1|o??à1?3￡êy?¨ò?
----------------------------------------------------------------------------------------------------*/
#define     HAREWARE_FIX_VAL         61UL

#define     WAKEUP_CLK_VAL           5                      //- ìá?°WAKEUP_CLK_VAL??CLKD?à′.

#define     CAL_AUX_TIMER(x,y)	    ((uint32_t)(312.5*(x)/y))
#define     CAL_OFFSET(x,y)	        ((((u_int32)(x*y + HAREWARE_FIX_VAL)))%1250)

extern      t_LE_Advertising    LE_advertise;
extern      t_LE_Config         LE_config;
extern	    u_int32		        have_get_no_crc_pkt;

#define		RTC_BASE						0x40002000
#define		AO_TIMER_CTL					((volatile unsigned *)(RTC_BASE + 0x0040))
#define		FREQ_TIMER_VAL					((volatile unsigned *)(RTC_BASE + 0x0050))

typedef struct
{
	uint32_t AO_TIMER_VALUE:16;
	uint32_t AO_TIMER_CLR:1;
	uint32_t AO_TIMER_EN:1;
	uint32_t FREQ_TIMER_EN:1;
	uint32_t RESERVED:13;
}AO_TIMER_CTL_T;

extern AO_TIMER_CTL_T* ao_timer_ctl ;

#define		CALIB_500US_BASE			(16)		//- 1/32k * 16 = 500us

extern uint16_t	CALIB_COUNT ;
extern uint16_t	CALIB_WAIT_TIME2_COUNT ;
#define		CALIB_TIME					((CALIB_500US_BASE) * CALIB_COUNT)

typedef		uint32_t	(*tHandler_callback)(uint16_t  val);
extern tHandler_callback   tHandler_Callback[] ;
/* ---------------------------------------------------------------------------------------------------
- 函数名称: Timer_Register_Callback
- 函数功能: 为TIMER注册中断处理函数
----------------------------------------------------------------------------------------------------*/
extern	void	Timer_Register_Callback(tHandler_callback callback, uint32_t ch)
{
        tHandler_Callback[ch] = callback;
}


extern uint32_t volatile native_bt_clk_restore;

static void	native_bt_clk_restore_start(void)
{
	native_bt_clk_restore = 1;
}

static void	native_bt_clk_restore_continue(void)
{
    native_bt_clk_restore = 2;
}

void	native_bt_clk_restore_done(void)
{
    if(native_bt_clk_restore != 2) return;
    native_bt_clk_restore = 0;
}

uint32_t	native_bt_clk_result(void)
{
    return native_bt_clk_restore;
}


typedef	struct {
	uint16_t	maxInterval;
	uint16_t	count;
}__attribute__((packed, aligned(4)))rc32k_t;

extern rc32k_t	rc32k_calib_table[7];
void	rc32k_calib_find_count(uint16_t coninterval, uint16_t *t2_load, uint16_t *rc32k_load )
{
	if(PATCH_FUN[RC32K_CALIB_FIND_COUNT_ID]){
         ((void	(*)(uint16_t coninterval, uint16_t *t2_load, uint16_t *rc32k_load ))PATCH_FUN[RC32K_CALIB_FIND_COUNT_ID])(coninterval, t2_load, rc32k_load);
         return ;
    }
	uint16_t index = 0;
	uint32_t count;
	uint32_t maxInterval;

	for(index = 0; index<sizeof(rc32k_calib_table)/sizeof(rc32k_t); index++) {

		maxInterval = rc32k_calib_table[index].maxInterval * 100;
		maxInterval /= 125;
		if(coninterval <= maxInterval) break;
	}
	
	count = rc32k_calib_table[index].count;
	*t2_load = count;
	*rc32k_load = (count - 1) << 1;

	return;	
}


//#define		CALIB_CLK_DIV_NUMBLER		((CALIB_500US * 10000)/625)

void rc32k_calib_start(void)
{
	if(PATCH_FUN[RC32K_CALIB_START_ID]){
         ((void (*)(void))PATCH_FUN[RC32K_CALIB_START_ID])();
         return ;
    }
    __write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL, 0x00020002);
	__write_hw_reg32(CPR_AOCLKEN_GRCTL,0x00020002);

    ao_timer_ctl->AO_TIMER_EN = 0;
    ao_timer_ctl->FREQ_TIMER_EN = 0;
	
	ao_timer_ctl->AO_TIMER_VALUE = CALIB_500US_BASE * CALIB_COUNT;
	ao_timer_ctl->AO_TIMER_EN = 0;
	ao_timer_ctl->FREQ_TIMER_EN = 1;
}

uint32_t rc32k_calib_finish(void)
{
	if(PATCH_FUN[RC32K_CALIB_FINISH_ID]){
         
         return ((uint32_t (*)(void))PATCH_FUN[RC32K_CALIB_FINISH_ID])();
    }
	uint32_t freq_timer_value = 0;
	uint32_t cycle_32k;
	
	__read_hw_reg32(FREQ_TIMER_VAL, freq_timer_value);
    ao_timer_ctl->AO_TIMER_CLR = 1;  
	freq_timer_value = 0xFFFFFF & freq_timer_value;
    ao_timer_ctl->AO_TIMER_EN = 0;
    ao_timer_ctl->FREQ_TIMER_EN = 0;

	cycle_32k = (uint32_t)(freq_timer_value * 62.5) / ((CALIB_500US_BASE) * CALIB_COUNT);
	
	//return 1000000000/cycle_32k;
	return cycle_32k;
}

/* ---------------------------------------------------------------------------------------------------
- oˉêy??3?: memset_ble_sleep
- oˉêy1|?ü: ±?á???á?
- ê?è?2?êy: ?T
----------------------------------------------------------------------------------------------------*/
void    memset_ble_sleep(void)  {
    
    u_int32 i = sizeof(ble_sleep);
    u_int8  *ptr =(u_int8*)&ble_sleep;
    while (i-- != 0)
    {
        *ptr++ = 0;
    }

	native_bt_clk_restore = 0;
}

/* ---------------------------------------------------------------------------------------------------
- oˉêy??3?: get_ble_sleep_status
- oˉêy1|?ü: ??è?ble?ù′?DY??×′ì?
- ê?è?2?êy: 0: 2??ü?±?ó??è?aux_timerê￡óàê±??
- ê?3?2?êy: 0??óDDY??, ·?0?μ±íê?DY??ê￡óàê±??(μ￥??ms)
----------------------------------------------------------------------------------------------------*/
uint32_t   ble_lowpower_remaining_clk(uint32_t stat)   
{   
	if(PATCH_FUN[BLE_LOWPOWER_REMAINING_CLK_ID]){
         
         return ((uint32_t (*)(uint32_t stat))PATCH_FUN[BLE_LOWPOWER_REMAINING_CLK_ID])(stat);
    }
    uint32_t    clk;
   
    if((0 == ble_sleep.adv_sleep_stat) && (0 == ble_sleep.con_sleep_stat)) return (0);

    if(stat == 0) {
        if(ble_sleep.real_exit_lowpower == 1) return (0);
        ble_sleep.real_exit_lowpower = 1;
    }
    
//	clk = timer_current_count(0);
	clk = ((uint32_t	(*)(uint32_t ch))PATCH_FUN[TIMER_CURRENT_COUNT])(0);
    return (clk);

}

/* ---------------------------------------------------------------------------------------------------
- oˉêy??3?: load_advDelay
- oˉêy1|?ü: ±￡′?Advê??t?????óé?μ????úêy
- ê?è?2?êy: ?T
----------------------------------------------------------------------------------------------------*/
void    load_advDelay(uint32_t advDelay) {

    LE_advertise.advDelay = advDelay;

}

/* ---------------------------------------------------------------------------------------------------
- oˉêy??3?: Adv_Enter_LowPower
- oˉêy1|?ü: Advê??t?áê?ê±??è?μí1|o?
- ê?è?2?êy: ?T
----------------------------------------------------------------------------------------------------*/
void    Adv_Enter_LowPower(void) {
	if(PATCH_FUN[ADV_ENTER_LOWPOWER_ID]){
         ((void    (*)(void))PATCH_FUN[ADV_ENTER_LOWPOWER_ID])();
         return ;
    }

    uint32_t    val;
    uint32_t    adv_interval = LE_advertise.advertising_interval*2 + LE_advertise.advDelay;
    uint32_t    current_clk = HW_Get_Native_Clk_Avoid_Race() ;
    #if 0
    uint32_t    ref_start_clk = current_clk - (LE_advertise.next_adv_timer - adv_interval);
    uint32_t    aux_timer_clk = adv_interval - ref_start_clk;
    #else
    uint32_t    aux_timer_clk = 0;
    uint32_t    aux_timer_Load ;
    if(current_clk < LE_advertise.next_adv_timer) {
        aux_timer_clk = LE_advertise.next_adv_timer - current_clk;
    }
    else 
    {   /*- Aut_time: 28bit, 0xFFFFFFF. */
        aux_timer_clk =  LE_advertise.next_adv_timer + 0xFFFFFFF - current_clk;
    }
        
    #endif
    //aux_timer_Load = (aux_timer_clk*CLOCK_SLEEP)/CLOCK_NORMAL;  //aux_timer_clk*312.5/30.52;
    //aux_timer_Load >>= 3;
   	aux_timer_Load = aux_timer_clk * 10; 	//- aux_timer_clk * 312.5 / 31.25.
    *((u_int32 volatile*)0x40028028) &= ~(1 << 7);
    
    __read_hw_reg32(CPR_SLP_PD_MASK, val);
    val |= 0x80;
    __write_hw_reg32(CPR_SLP_PD_MASK , val);						        //- ?ˉ??ê±μ???μ?????:2???μ?			
    /*__write_hw_reg32(CPR_AO_SYNC , 0x01);*/

	//LSLCirq_Disable_And_Clear_Tabasco_Intr();
	
    HW_set_use_lf(1); 
    while (0 == HW_get_sleep_status());

    /*HW_set_aux_timer(aux_timer_Load);
    HW_set_aux_tim_intr_mask(0);
    LSLCirq_Enable_Aux_Tim_Intr();*/
    ((void	(*)(uint32_t ch, uint32_t msTick))PATCH_FUN[INIT_TIMER])(0, aux_timer_Load);
    //Init_Timer(0, aux_timer_Load);
	Timer_Register_Callback(ble_firmware_exit_lowpower, 0);
  
    ble_sleep.adv_sleep_stat = 1;                                           //- BLE?ù′?ò??-DY??. 
    __DEBUG_IO_HIGH55();
}


/* ---------------------------------------------------------------------------------------------------
- oˉêy??3?: Adv_Exit_LowPower
- oˉêy1|?ü: Advê??tí?3?μí1|o?
- ê?è?2?êy: ?T
----------------------------------------------------------------------------------------------------*/
void    Adv_Exit_LowPower(void) {
    if(PATCH_FUN[ADV_EXIT_LOWPOWER_ID]){
         ((void    (*)(void) )PATCH_FUN[ADV_EXIT_LOWPOWER_ID])();
         return ;
    }
    if(0 == ble_sleep.adv_sleep_stat ) return;   
    __DEBUG_IO_LOW55();

    /*HW_set_aux_tim_intr_clr(1);
    HW_set_aux_tim_intr_mask(1);
    LSLCirq_Disable_Aux_Tim_Intr();*/
    //Timer_disable(0);    
    ((void    (*)(uint32_t ch))PATCH_FUN[TIMER_DISABLE])(0);
    HW_set_use_lf(0);
    while (1 == HW_get_sleep_status());

    
    ble_sleep.adv_sleep_stat = 0;                                           //- BLE?ù′?í?3?DY??.

}

void restore_sleep_clk()
{
	if(PATCH_FUN[RESTORE_SLEEP_CLK_ID]){
         ((void (*)(void))PATCH_FUN[RESTORE_SLEEP_CLK_ID])();
         return ;
    }
	uint32_t   _native_clk = 0;;

	//restore offset 
	HW_set_intraslot_offset(ble_sleep.con_slave_wakeup_Intraslot_offset);

	//restore bt clk
	HW_set_write_absolute_bt_clk(1);
    	HW_set_bt_clk_offset(ble_sleep.con_slave_wakeup_bt_clk);


	//restore native clk
	if (ble_sleep.con_bt_clk_native_clk_margin >= 0)
		ble_sleep.native_clk_set = (ble_sleep.con_slave_wakeup_bt_clk - ble_sleep.con_bt_clk_native_clk_margin);
	else
		ble_sleep.native_clk_set =  ((int)ble_sleep.con_slave_wakeup_bt_clk - (ble_sleep.con_bt_clk_native_clk_margin));
	

	ble_sleep.con_intra_slot_save = *((u_int32 volatile*)0x40026038);
	SET_IO5(1);

	if (ble_sleep.con_intra_slot_save%312 > 280)
	{
		while(!(*((u_int32 volatile*)0x40026038)%312 < 100));
	}
	SET_IO5(1);
	_native_clk = HW_Get_Native_Clk_Avoid_Race();
	ble_sleep.con_slave_wakeup_native_clk = ((_native_clk&0x3) + (ble_sleep.native_clk_set&0xFFFFFFFC) + 1);
	HW_set_native_clk(ble_sleep.con_slave_wakeup_native_clk);
	
}

unsigned int cal_intraslotoffset_compensate(u_int8 IRQ_Type)
{
	if(PATCH_FUN[CAL_INTRASLOTOFFSET_COMPENSATE_ID]){
         
         return ((unsigned int (*)(u_int8 IRQ_Type))PATCH_FUN[CAL_INTRASLOTOFFSET_COMPENSATE_ID])(IRQ_Type);
    }
	unsigned int intraslot_compensate_value = 0;
	SET_IO55(1);
	switch (IRQ_Type)
	{
		case LE_TIM_0:
			intraslot_compensate_value = 0;
			break;
		case LE_TIM_1:
			intraslot_compensate_value = 937;
			break;
		case LE_TIM_2:
			intraslot_compensate_value = 625;
			break;
		case LE_TIM_3:
			intraslot_compensate_value = 312;
			break;
		default:
			intraslot_compensate_value = 0xFFFFFFFF;
	}
	return intraslot_compensate_value;
}

/* ---------------------------------------------------------------------------------------------------
- oˉêy??3?: Connection_Exit_LowPower
- oˉêy1|?ü: á??óê??t?áê?ê±í?è?μí1|o?
- ê?è?2?êy: ?T
----------------------------------------------------------------------------------------------------*/
void    Connection_Exit_LowPower(void)  
{
	if(PATCH_FUN[CONNECTION_EXIT_LOWPOWER_ID]){
         ((void    (*)(void))PATCH_FUN[CONNECTION_EXIT_LOWPOWER_ID])();
         return ;
    }
    if(0 == ble_sleep.con_sleep_stat) return;

   // HW_set_aux_tim_intr_clr(1);
    //HW_set_aux_tim_intr_mask(1);
    //LSLCirq_Disable_Aux_Tim_Intr();
    ((void    (*)(uint32_t ch))PATCH_FUN[TIMER_DISABLE])(0);
    //Timer_disable(0);

	SET_IO55(2);
	
    HW_set_use_lf(0);
    while (1 == HW_get_sleep_status());
   
    __DEBUG_IO_LOW5();

	restore_sleep_clk();
	
    ble_sleep.con_sleep_stat = 0;
	ble_sleep.con_compensate_phase_flg = 1;
	
	LSLCirq_Disable_All_Tim_Intr_Except(JAL_TIM2_INTR_MSK_MASK | JAL_TIM0_INTR_MSK_MASK | 
	                                    JAL_TIM1_INTR_MSK_MASK | JAL_TIM3_INTR_MSK_MASK);
	                                
}



/* ---------------------------------------------------------------------------------------------------
- oˉêy??3?: do_Connection_Exit_LowPower
- oˉêy1|?ü: í?3?μí1|o??￡ê?￡?2￠μ÷???à??
- ê?è?2?êy: ?T
----------------------------------------------------------------------------------------------------*/
uint32_t    do_Connection_Exit_LowPower(u_int8 IRQ_Type)  {
	if(PATCH_FUN[DO_CONNECTION_EXIT_LOWPOWER_ID]){
         
         return ((uint32_t    (*)(u_int8 IRQ_Type))PATCH_FUN[DO_CONNECTION_EXIT_LOWPOWER_ID])(IRQ_Type);
    }

	u_int32 intraslot_offset_com = 0;
	u_int32 intra_slot_q = 0;
	u_int32 intra_slot_set = 0;
	u_int32 intraslot_offset_int = 0;
		
    if(ble_sleep.con_compensate_phase_flg == 0) return (0);

   	intra_slot_q = *((u_int32 volatile*)0x40026038);
   	if (intra_slot_q >= ble_sleep.con_intra_slot_save)
   		intra_slot_set = (intra_slot_q  - ble_sleep.con_intra_slot_save);
   	else
   		intra_slot_set = (intra_slot_q  +1250 - ble_sleep.con_intra_slot_save);
   	if ( intra_slot_set <= 150)
   	{
   		return NO_ERROR;
   	}
	ble_sleep.con_compensate_phase_flg = 0;
	
   	intraslot_offset_int = HW_get_intraslot_offset();
   	//intraslot_int = *((u_int32 volatile*)0x40026038);
   	intraslot_offset_com = cal_intraslotoffset_compensate(IRQ_Type);
   
   	HW_set_intraslot_offset((intraslot_offset_int + intraslot_offset_com )%1250);

	native_bt_clk_restore_continue();
   	return 1;
}
/* ---------------------------------------------------------------------------------------------------
- oˉêy??3?: Connection_Enter_LowPower
- oˉêy1|?ü: á??óê??t?D????????è?μí1|o?
- ê?è?2?êy: ?T
----------------------------------------------------------------------------------------------------*/
void    Connection_Enter_LowPower(t_LE_Connection* p_connection) {
	if(PATCH_FUN[CONNECTION_ENTER_LOWPOWER_ID]){
         ((void    (*)(t_LE_Connection* p_connection))PATCH_FUN[CONNECTION_ENTER_LOWPOWER_ID])(p_connection);
         return ;
    }

    if(LEconnection_Get_Initial_Anchor_Point_Obtained(p_connection) == 0) return;
    if(0 == have_get_no_crc_pkt) return;
#if	0
    ble_sleep.con_slave_wait_tim2_Flg = 1;
#else
	if(p_connection->connInterval <= 12) {
		ble_sleep.con_slave_wait_tim2_Flg = 0;
	}	
	else {
		rc32k_calib_find_count(p_connection->connInterval, &CALIB_WAIT_TIME2_COUNT, &CALIB_COUNT);
		ble_sleep.con_slave_wait_tim2_Flg = CALIB_WAIT_TIME2_COUNT;
		ble_sleep.pConnection = p_connection;
	}
#endif
}
extern uint32_t	volatile testlp;
/* ---------------------------------------------------------------------------------------------------
- oˉêy??3?: do_Connection_Enter_LowPower
- oˉêy1|?ü: á??óê??t?áê?ê±??è?μí1|o?
- ê?è?2?êy: ?T
----------------------------------------------------------------------------------------------------*/
uint32_t	do_Connection_Enter_LowPower(void)	
{
	if(PATCH_FUN[DO_CONNECTION_ENTER_LOWPOWER_ID]){
         
         return ((uint32_t	(*)(void))PATCH_FUN[DO_CONNECTION_ENTER_LOWPOWER_ID])();
    }
    uint32_t    clk , native_clk, bt_clk;
	float period_t = 0.0;
    if(ble_sleep.con_slave_wait_tim2_Flg == 0) return (0);
	ble_sleep.con_slave_wait_tim2_Flg--;
	if(ble_sleep.con_slave_wait_tim2_Flg == (CALIB_WAIT_TIME2_COUNT - 1)) {
		rc32k_calib_start();
		SET_IO55(1);
		SET_IO55(1);
	}
	else if (ble_sleep.con_slave_wait_tim2_Flg == 0){	
	    HW_set_tim_intr_clr(JAL_TIM2_INTR_MSK_MASK);		
	    LSLCirq_Disable_All_Tim_Intr_Except(0);
		
	    /*ble_sleep.con_slave_connInterval_clk = p_connection->connInterval*4;*/
	    ble_sleep.con_slave_current_clk = HW_Get_Bt_Clk_Avoid_Race ();
	    #if 0
	    ble_sleep.con_slave_aux_timer_clk = LE_config.next_connection_event_time - ble_sleep.con_slave_current_clk - WAKEUP_CLK_VAL;
	    #else
		if (LE_config.next_connection_event_time > ble_sleep.con_slave_current_clk) {
	        
		    ble_sleep.con_slave_aux_timer_clk = LE_config.next_connection_event_time - ble_sleep.con_slave_current_clk - wakeup_clk_val;
	    }
		else
		{
			ble_sleep.con_slave_aux_timer_clk = LE_config.next_connection_event_time + 0xFFFFFFF - ble_sleep.con_slave_current_clk - wakeup_clk_val;
		}
	    
	    #endif
#if	1
		period_t = (float)rc32k_calib_finish()/1000;
	    ble_sleep.con_slave_aux_timer_Load = CAL_AUX_TIMER(ble_sleep.con_slave_aux_timer_clk, period_t);	
	    ble_sleep.con_slave_aux_timer_offset = CAL_OFFSET(ble_sleep.con_slave_aux_timer_Load, period_t);
#else
		//uint32_t period_int = rc32k_calib_finish();
		//ble_sleep.con_slave_aux_timer_Load = (3215 * ble_sleep.con_slave_aux_timer_clk * CALIB_CLK_DIV_NUMBLER) / (period_int * 10);
		//ble_sleep.con_slave_aux_timer_offset = CAL_OFFSET(ble_sleep.con_slave_aux_timer_Load, period_int/CALIB_CLK_DIV_NUMBLER);

#endif
	    HW_set_use_lf(1); 
	    while (0 == HW_get_sleep_status()) ;
	    __DEBUG_IO_HIGH5();

		native_bt_clk_restore_start();
			
	    //HW_set_aux_timer( ble_sleep.con_slave_aux_timer_Load);
	    //HW_set_aux_tim_intr_mask(0);
	    //LSLCirq_Enable_Aux_Tim_Intr();
	    if(ble_sleep.pConnection->connInterval == 400) {
			ble_sleep.con_slave_aux_timer_Load -= 80;
	    }
		((void	(*)(uint32_t ch, uint32_t msTick))PATCH_FUN[INIT_TIMER])(0, ble_sleep.con_slave_aux_timer_Load);
	    //Init_Timer(0, ble_sleep.con_slave_aux_timer_Load);
		Timer_Register_Callback(ble_firmware_exit_lowpower, 0);

		SET_IO55(2);
		
	    native_clk = HW_Get_Native_Clk_Avoid_Race();
	    bt_clk = HW_Get_Bt_Clk_Avoid_Race(); 

	    ble_sleep.con_bt_clk_native_clk_margin = (int)((bt_clk >= native_clk)?(bt_clk - native_clk):-(native_clk - bt_clk));
		
	    //ble_sleep.con_slave_wakeup_Intraslot_offset = cal_compensate_offset(native_clk, bt_clk, ble_sleep.con_slave_aux_timer_offset);
		ble_sleep.con_slave_wakeup_Intraslot_offset = ble_sleep.con_slave_aux_timer_offset;
	    ble_sleep.con_slave_wakeup_bt_clk = LE_config.next_connection_event_time - wakeup_clk_val; //5;

	    ble_sleep.con_sleep_stat = 1;

	}
    return (1);
}

/* ---------------------------------------------------------------------------------------------------
- oˉêy??3?: Connection_Close_LowPower
- oˉêy1|?ü: ìá?°?áê?á??ó×′ì?μí1|o?
- ê?è?2?êy: ?T
----------------------------------------------------------------------------------------------------*/
void    Connection_Close_LowPower(void)  {

	ble_sleep.con_slave_wait_tim2_Flg = 0;
}

/* ---------------------------------------------------------------------------------------------------
- oˉêy??3?: Exit_LowPower
- oˉêy1|?ü: ??3y?a0, ±íê?AUX?D??μ?à′￡????yí?3?á?μí1|o?
- ê?è?2?êy: ?T
----------------------------------------------------------------------------------------------------*/
void    Exit_LowPower(void)
{
    ble_sleep.real_exit_lowpower = 0;    
}

void	ble_firmware_exit_lowpower(void)
{
	if(PATCH_FUN[BLE_FIRMWARE_EXIT_LOWPOWER_ID]){
         ((void	(*)(void))PATCH_FUN[BLE_FIRMWARE_EXIT_LOWPOWER_ID])();
         return ;
    }
    Connection_Exit_LowPower(); // first restore clk    
    Adv_Exit_LowPower();

    Exit_LowPower();
}

#endif

