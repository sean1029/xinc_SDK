/**
 * Copyright (c) 2016 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "sdk_common.h"
#if XINC_MODULE_ENABLED(XINC_PWR_MGMT)

#include "xinc_pwr_mgmt.h"
#include "xinc.h"
#include "xinc_mtx.h"
//#include "xinc_power.h"
#include "xincx_gpio.h"
#include "app_error.h"
#include "xinc_assert.h"
#include "xinc_log_ctrl.h"
#include "app_util_platform.h"

#include "xinc_delay.h"
#define     CPR_AO_BASE                     0x40002400

#define XINC_LOG_MODULE_NAME pwr_mgmt
#if XINC_PWR_MGMT_CONFIG_LOG_ENABLED
    #define XINC_LOG_LEVEL       XINC_PWR_MGMT_CONFIG_LOG_LEVEL
    #define XINC_LOG_INFO_COLOR  XINC_PWR_MGMT_CONFIG_INFO_COLOR
    #define XINC_LOG_DEBUG_COLOR XINC_PWR_MGMT_CONFIG_DEBUG_COLOR
#else
    #define XINC_LOG_LEVEL       0
#endif // XINC_PWR_MGMT_CONFIG_LOG_ENABLED
#include "xinc_log.h"
XINC_LOG_MODULE_REGISTER();



#if XINC_PWR_MGMT_CONFIG_USE_SCHEDULER
    #if (APP_SCHEDULER_ENABLED != 1)
        #error "APP_SCHEDULER is required."
    #endif
    #include "app_scheduler.h"
#endif // XINC_PWR_MGMT_CONFIG_USE_SCHEDULER


// Create section "pwr_mgmt_data".
XINC_SECTION_SET_DEF(pwr_mgmt_data,
                    xinc_pwr_mgmt_shutdown_handler_t,
                    XINC_PWR_MGMT_CONFIG_HANDLER_PRIORITY_COUNT);

static xinc_pwr_mgmt_evt_t   m_pwr_mgmt_evt;     /**< Event type which will be passed to the shutdown
                                                     handlers.*/
static xinc_mtx_t            m_sysoff_mtx;       /**< Module API lock.*/
static bool                 m_shutdown_started; /**< True if application started the shutdown preparation. */
static xinc_section_iter_t   m_handlers_iter;    /**< Shutdown handlers iterator. */

#if (XINC_PWR_MGMT_CONFIG_FPU_SUPPORT_ENABLED && __FPU_PRESENT)
    #define PWR_MGMT_FPU_SLEEP_PREPARE()     pwr_mgmt_fpu_sleep_prepare()

     __STATIC_INLINE void pwr_mgmt_fpu_sleep_prepare(void)
     {
        uint32_t original_fpscr;

        CRITICAL_REGION_ENTER();
        original_fpscr = __get_FPSCR();
        /*
         * Clear FPU exceptions.
         * Without this step, the FPU interrupt is marked as pending,
         * preventing system from sleeping. Exceptions cleared:
         * - IOC - Invalid Operation cumulative exception bit.
         * - DZC - Division by Zero cumulative exception bit.
         * - OFC - Overflow cumulative exception bit.
         * - UFC - Underflow cumulative exception bit.
         * - IXC - Inexact cumulative exception bit.
         * - IDC - Input Denormal cumulative exception bit.
         */
        __set_FPSCR(original_fpscr & ~0x9Fu);
        __DMB();
        NVIC_ClearPendingIRQ(FPU_IRQn);
        CRITICAL_REGION_EXIT();

        /*
         * The last chance to indicate an error in FPU to the user 
         * as the FPSCR is now cleared
         *
         * This assert is related to previous FPU operations 
         * and not power management.
         *
         * Critical FPU exceptions signaled:
         * - IOC - Invalid Operation cumulative exception bit.
         * - DZC - Division by Zero cumulative exception bit.
         * - OFC - Overflow cumulative exception bit.
         */
        ASSERT((original_fpscr & 0x7) == 0);
     }
#else
    #define PWR_MGMT_FPU_SLEEP_PREPARE()
#endif // XINC_PWR_MGMT_CONFIG_FPU_SUPPORT_ENABLED


#define     RST_READY_TIME          	0x07        //- 
#define     OSC32_STABLE_TIME       	47//  2ms:47   6ms  180     
#define     WAIT_BLE_EXIT_LOWPWR_TIME   10 
#define     PRE_WAKEUP_TIME             (RST_READY_TIME + OSC32_STABLE_TIME + WAIT_BLE_EXIT_LOWPWR_TIME)

#if XINC_MODULE_ENABLED(XINC_BLE_STACK)  

#if XINC_PWR_MGMT_CONFIG_BLE_STACK_ENABLED

    #undef  PWR_MGMT_SLEEP_IN_CRITICAL_SECTION_REQUIRED
    #define PWR_MGMT_SLEEP_IN_CRITICAL_SECTION_REQUIRED
    
 
    #define PWR_MGMT_BLE_STACK_MONITOR_INIT()    pwr_mgmt_ble_stack_monitor_init()
    #define PWR_MGMT_BLE_STACK_MONITOR_UPDATE()  pwr_mgmt_ble_stack_monitor_update()

    #define PWR_MGMT_BLE_STACK_MONITOR_SECTION_ENTER() pwr_mgmt_ble_stack_monitor_enter()

    #define PWR_MGMT_BLE_STACK_MONITOR_SECTION_EXIT()  pwr_mgmt_ble_stack_monitor_exit()
          
    static uint32_t m_ble_sleeping;    /**< Number of ticks spent in sleep mode (__WFI()). */
    static uint32_t m_ble_timer_last;        /**< Number of ticks from the last BLE usage. */
    static uint32_t m_ble_sleep_flag;    /**< Number of ticks spent in sleep mode (__WFI()). */
    static uint32_t remain_timer;
        

    __STATIC_INLINE void pwr_mgmt_ble_stack_monitor_init(void)
    {
        XINC_CPR_AO_CTL_Type * p_regAO = XINC_CPR_AO;
        m_ble_sleeping      = 0;
        m_ble_timer_last    = 0;
        m_ble_sleep_flag    = 0;
        
        p_regAO->VDD_SWITCH_EN   &=0x1E;             //cpr_ao_vdd_switch_en BT_MODEM??
        
        p_regAO->VDD_ISO_EN     |=0x01;             //cpr_ao_vdd_iso_en    BT_MODEM????
    }
    
    __STATIC_INLINE uint32_t pwr_mgmt_ble_stack_monitor_enter(void)
    {
        
            remain_timer = ble_lowpower_remaining_clock(0); 
            uint32_t VAL1 = *(((volatile unsigned *)(0x40003000 + 0x0C )));
            if((remain_timer <= PRE_WAKEUP_TIME))                                
            {                                                            
                m_ble_sleep_flag = 0;                                                 
            }else           
            {               
                remain_timer = ble_lowpower_remaining_clock(1);
                if(remain_timer == 0xFFFFFFFF) 
                {        
                    do          
                    {                    
                        remain_timer = timer_current_count(0);  
                        if(remain_timer != 0xFFFFFFFF) break;	    
                    } while(1);         
                 }           
                                                
                if(remain_timer <= PRE_WAKEUP_TIME) 
                {           
                    m_ble_sleep_flag = 0;             
                }                                   
                else                                        
                {               
                    m_ble_sleep_flag = 1; 
                } 
            } 
            if(m_ble_sleep_flag == 1)
            {
                remain_timer = remain_timer - PRE_WAKEUP_TIME;      
                Init_Timer(1, remain_timer); 
                ble_enter_lowpower();
                
                        //xc_rf_ldo_off();                                               //close rf_ldo
        *((volatile unsigned *)(CPR_AO_BASE + 0x50)) &=0xFFFFFFFE;       //close rf digital ˖֯ߪژRF؜ߪژ  
        *((volatile unsigned *)(CPR_AO_BASE + 0x40)) &=0x1E;             //cpr_ao_vdd_switch_en BT_MODEM׏֧
        *((volatile unsigned *)(CPR_AO_BASE + 0x44)) |=0x01;             //cpr_ao_vdd_iso_en    BT_MODEM׏֧ٴk
        //*((volatile unsigned *)(CPR_AO_BASE + 0x20))  =0x2C;  
     //   *((volatile unsigned *)(CPR_AO_BASE + 0x3C))  =0xF; 
//        __NOP();							
//		__NOP();							
//		__NOP(); 
//		__WFI();							
//		__NOP();							
//		__NOP();							
//		__NOP();                 
            }
            
            return m_ble_sleep_flag;
    
    }
    
    __STATIC_INLINE void pwr_mgmt_ble_stack_monitor_exit(void)
    {
        if(m_ble_sleep_flag == 1)
        {
           //   ble_exit_lowpower(remain_timer);
            
                    *((volatile unsigned *)(CPR_AO_BASE + 0x40)) |=0x1;             //cpr_ao_vdd_switch_en BT_MODEM׏֧
        *((volatile unsigned *)(CPR_AO_BASE + 0x44)) &=0x1E;            //cpr_ao_vdd_iso_en    BT_MODEM׏֧ٴk
        *((volatile unsigned *)(CPR_AO_BASE + 0x50)) |=0x1;             //open rf digital ˖֯ߪژRF؜ߪژ
        //xc_rf_ldo_on();      //open rf_ldo    

//                void SysTick_set(uint32_t sys_tick);   
//                SysTick_set(SysTick_get() + ((remain_timer >> 5) / 10));
            ble_exit_lowpower(remain_timer);
        }
    
    }

    __STATIC_INLINE void pwr_mgmt_ble_stack_monitor_update(void)
    {
        uint32_t delta;
        uint32_t ticks;
        uint8_t  cpu_usage;

  
    }
#else
    #define PWR_MGMT_BLE_STACK_MONITOR_INIT()
    #define PWR_MGMT_BLE_STACK_MONITOR_UPDATE()
    #define PWR_MGMT_BLE_STACK_MONITOR_SECTION_ENTER()    0
    #define PWR_MGMT_BLE_STACK_MONITOR_SECTION_EXIT()
#endif // XINC_PWR_MGMT_CONFIG_BLE_STACK_MONITOR_ENABLED
    
#else
    #define PWR_MGMT_BLE_STACK_MONITOR_INIT()
    #define PWR_MGMT_BLE_STACK_MONITOR_UPDATE()
    #define PWR_MGMT_BLE_STACK_MONITOR_SECTION_ENTER()    1
    #define PWR_MGMT_BLE_STACK_MONITOR_SECTION_EXIT()
    
#endif //XINC_BLE_STACKENABLED
    
    


#if XINC_PWR_MGMT_CONFIG_CPU_USAGE_MONITOR_ENABLED
    #undef  PWR_MGMT_SLEEP_IN_CRITICAL_SECTION_REQUIRED
    #define PWR_MGMT_SLEEP_IN_CRITICAL_SECTION_REQUIRED

    #undef  PWR_MGMT_TIMER_REQUIRED
    #define PWR_MGMT_TIMER_REQUIRED
    #include "app_timer.h"

    #define PWR_MGMT_CPU_USAGE_MONITOR_INIT()    pwr_mgmt_cpu_usage_monitor_init()
    #define PWR_MGMT_CPU_USAGE_MONITOR_UPDATE()  pwr_mgmt_cpu_usage_monitor_update()
    #define PWR_MGMT_CPU_USAGE_MONITOR_SUMMARY() XINC_LOG_INFO("Maximum CPU usage: %u%%", \
                                                              m_max_cpu_usage)
    #define PWR_MGMT_CPU_USAGE_MONITOR_SECTION_ENTER()  \
        {                                               \
            uint32_t sleep_start = app_timer_cnt_get(); \
           /* printf("sleep_start :%d\r\n", sleep_start);  */    
    #define PWR_MGMT_CPU_USAGE_MONITOR_SECTION_EXIT()                       \
            uint32_t sleep_end = app_timer_cnt_get();                       \
            uint32_t sleep_duration;                                        \
            sleep_duration = app_timer_cnt_diff_compute(sleep_end,          \
                                                       sleep_start);        \
            m_ticks_sleeping += sleep_duration;                             \
            /*printf("sleep_end :%d,sleep_duration:%d\r\n", sleep_end,sleep_duration); */\
        }

    static uint32_t m_ticks_sleeping;    /**< Number of ticks spent in sleep mode (__WFE()). */
    static uint32_t m_ticks_last;        /**< Number of ticks from the last CPU usage computation. */
    static uint8_t  m_max_cpu_usage;     /**< Maximum observed CPU usage (0 - 100%). */

    __STATIC_INLINE void pwr_mgmt_cpu_usage_monitor_init(void)
    {
        m_ticks_sleeping    = 0;
        m_ticks_last        = 0;
        m_max_cpu_usage     = 0;
    }

    __STATIC_INLINE void pwr_mgmt_cpu_usage_monitor_update(void)
    {
        uint32_t delta;
        uint32_t ticks;
        uint8_t  cpu_usage;

        ticks = app_timer_cnt_get();
        delta = app_timer_cnt_diff_compute(ticks, m_ticks_last);
        cpu_usage = 100 * (delta - m_ticks_sleeping) / delta;

        XINC_LOG_INFO("CPU Usage: %u%%", cpu_usage);
        printf("cpu_usage_monitor_update ticks: %d,delta:%d\r\n", ticks,delta);
        if (m_max_cpu_usage < cpu_usage)
        {
            m_max_cpu_usage = cpu_usage;
        }

        m_ticks_last        = ticks;
        m_ticks_sleeping    = 0;
    }

#else
    #define PWR_MGMT_CPU_USAGE_MONITOR_INIT()
    #define PWR_MGMT_CPU_USAGE_MONITOR_UPDATE()
    #define PWR_MGMT_CPU_USAGE_MONITOR_SUMMARY()
    #define PWR_MGMT_CPU_USAGE_MONITOR_SECTION_ENTER()
    #define PWR_MGMT_CPU_USAGE_MONITOR_SECTION_EXIT()
#endif // XINC_PWR_MGMT_CONFIG_CPU_USAGE_MONITOR_ENABLED


#if XINC_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_ENABLED
    #undef  PWR_MGMT_TIMER_REQUIRED
    #define PWR_MGMT_TIMER_REQUIRED

    #define PWR_MGMT_STANDBY_TIMEOUT_INIT()  pwr_mgmt_standby_timeout_clear()
    #define PWR_MGMT_STANDBY_TIMEOUT_CLEAR() pwr_mgmt_standby_timeout_clear()
    #define PWR_MGMT_STANDBY_TIMEOUT_CHECK() pwr_mgmt_standby_timeout_check()

    static uint16_t m_standby_counter;     /**< Number of seconds from the last activity
                                                (@ref pwr_mgmt_feed). */

    __STATIC_INLINE void pwr_mgmt_standby_timeout_clear(void)
    {
        m_standby_counter = 0;
    }

    __STATIC_INLINE void pwr_mgmt_standby_timeout_check(void)
    {
        if (m_standby_counter < XINC_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_S)
        {
            m_standby_counter++;
        }
        else if (m_shutdown_started == false)
        {
            xinc_pwr_mgmt_shutdown(XINC_PWR_MGMT_SHUTDOWN_GOTO_SYSOFF);
        }
    }

#else
    #define PWR_MGMT_STANDBY_TIMEOUT_INIT()
    #define PWR_MGMT_STANDBY_TIMEOUT_CLEAR()
    #define PWR_MGMT_STANDBY_TIMEOUT_CHECK()
#endif // XINC_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_ENABLED


#if XINC_PWR_MGMT_CONFIG_AUTO_SHUTDOWN_RETRY
    #undef  PWR_MGMT_TIMER_REQUIRED
    #define PWR_MGMT_TIMER_REQUIRED

    #define PWR_MGMT_AUTO_SHUTDOWN_RETRY() pwr_mgmt_auto_shutdown_retry()

    __STATIC_INLINE void pwr_mgmt_auto_shutdown_retry(void)
    {
        if (m_shutdown_started)
        {
            // Try to continue the shutdown procedure.
            xinc_pwr_mgmt_shutdown(XINC_PWR_MGMT_SHUTDOWN_CONTINUE);
        }
    }

#else
    #define PWR_MGMT_AUTO_SHUTDOWN_RETRY()
#endif // XINC_PWR_MGMT_CONFIG_AUTO_SHUTDOWN_RETRY


#ifdef PWR_MGMT_SLEEP_IN_CRITICAL_SECTION_REQUIRED
    #define PWR_MGMT_SLEEP_INIT()           pwr_mgmt_sleep_init()
    #define PWR_MGMT_SLEEP_LOCK_ACQUIRE()   CRITICAL_REGION_ENTER()
    #define PWR_MGMT_SLEEP_LOCK_RELEASE()   CRITICAL_REGION_EXIT()

    __STATIC_INLINE void pwr_mgmt_sleep_init(void)
    {
        
        XINC_CPR_AO_CTL_Type * p_regAO = XINC_CPR_AO;
        XINC_CPR_CTL_Type   * p_reg = XINC_CPR;
        
       // __write_hw_reg32(CPR_SYS_TIME, ((RST_READY_TIME<<12) | OSC32_STABLE_TIME)); 
        p_regAO->SYS_TIME = 0x07 << 12 | 47;
        

      
      
      //__write_hw_reg32(CPR_SLP_CTL, 0x00);
        p_regAO->SLP_CTL = 0x00;        
        
     // __write_hw_reg32(CPR_SLPCTL_INT_MASK, 0xFFFFFfAF); 
        p_regAO->SLPCTL_INT_MASK = 0;
        
        p_regAO->SLPCTL_INT_MASK &= ~((1 << TIMER1_IRQn) | (1 << KBS_IRQn)  | (1 << GPIO_IRQn) | (1 << RTC_IRQn) );

        //  __write_hw_reg32(CPR_SLP_PD_MASK,0x181);  
        p_regAO->SLP_PD_MASK = CPR_AO_SLP_PD_MASK_RAM_SLP_PD_MASK_Msk |  CPR_AO_SLP_PD_MASK_SYS_SLP_PMU_PD_MASK_Msk;	
        
     // __write_hw_reg32(CPR_SLP_SRC_MASK, 0x60006);
        p_reg->SLP_SRC_MASK = 0x60006;
        
        p_regAO->VDD_SWITCH_EN  &=0x0F;//ROM 断电
        
        p_regAO->VDD_ISO_EN     |=0x10;   
        
                p_regAO->VDD_SWITCH_EN   &=0x1E;             //cpr_ao_vdd_switch_en BT_MODEM??
        
        
        p_regAO->VDD_ISO_EN     |=0x01;             //cpr_ao_vdd_iso_en    BT_MODEM????
                *((volatile unsigned *)(CPR_AO_BASE + 0x50)) &=0xFFFFFFFE;       //close rf digital ˖֯ߪژRF؜ߪژ  
        *((volatile unsigned *)(CPR_AO_BASE + 0x40)) &=0x1E;             //cpr_ao_vdd_switch_en BT_MODEM׏֧
        *((volatile unsigned *)(CPR_AO_BASE + 0x44)) |=0x01;             //cpr_ao_vdd_iso_en    BT_MODEM׏֧ٴk

        

 
      //  SCB->SCR |= SCB_SCR_SEVONPEND_Msk;
    }

#else
    #define PWR_MGMT_SLEEP_INIT()
    #define PWR_MGMT_SLEEP_LOCK_ACQUIRE()
    #define PWR_MGMT_SLEEP_LOCK_RELEASE()
#endif // PWR_MGMT_SLEEP_IN_CRITICAL_SECTION_REQUIRED


#ifdef PWR_MGMT_TIMER_REQUIRED
    #include "app_timer.h"
    #define PWR_MGMT_TIMER_CREATE()     pwr_mgmt_timer_create()

    APP_TIMER_DEF(m_pwr_mgmt_timer);    /**< Timer used by this module. */

    /**@brief Handle events from m_pwr_mgmt_timer.
     */
    static void xinc_pwr_mgmt_timeout_handler(void * p_context)
    {
        printf("xinc_pwr_mgmt_timeout_handler\r\n");
        PWR_MGMT_CPU_USAGE_MONITOR_UPDATE();
        //PWR_MGMT_AUTO_SHUTDOWN_RETRY();
        PWR_MGMT_STANDBY_TIMEOUT_CHECK();
    }

    __STATIC_INLINE ret_code_t pwr_mgmt_timer_create(void)
    {
        ret_code_t ret_code = app_timer_create(&m_pwr_mgmt_timer,
                                               APP_TIMER_MODE_REPEATED,
                                               xinc_pwr_mgmt_timeout_handler);
        if (ret_code != XINC_SUCCESS)
        {
            return ret_code;
        }

        return 0;//app_timer_start(m_pwr_mgmt_timer, APP_TIMER_TICKS(1000), NULL);
    }
#else
    #define PWR_MGMT_TIMER_CREATE() XINC_SUCCESS
#endif // PWR_MGMT_TIMER_REQUIRED
void PWR_gpio_sleep_config(void)
{
    bool is_set;
    xinc_gpio_pin_input_cfg_t io_cfg;
    io_cfg.input_int = XINC_GPIO_PIN_INPUT_NOINT;
    io_cfg.pin_pulll= XINC_GPIO_PIN_PULLDOWN;
	for(int i=0;i<32;i++)
	{
        is_set = xincx_pin_use_is_set(i);
        
       
        if(is_set)
        {
          //  printf("pin:%d is use:%s\r\n",i,is_set ? "true" : "false");
//           
//            gpio_mux_ctl(i,0);
//            gpio_fun_sel(i,0);
//            gpio_direction_input(i,1);
//                             
        }
         if(i >=0 &&  i<= 1)
        {
//            gpio_mux_ctl(i,0);
//            gpio_fun_sel(i,0);
//            gpio_direction_input(i,1);
         //   xinc_gpio_cfg_input(i,io_cfg);
            //            gpio_mux_ctl(i,0);
//            gpio_fun_sel(i,0);
//            gpio_direction_input(i,1);
        }
        if(i >=2 &&  i<= 3)
        {
            xinc_gpio_cfg_input(i,io_cfg);
        }
        if(i >=6 &&  i<= 7)
        {
            xinc_gpio_cfg_input(i,io_cfg);
        }
        
        if(i >=18 &&  i<= 19)
        {
               xinc_gpio_cfg_input(i,io_cfg);
        }

	}
}
    
ret_code_t xinc_pwr_mgmt_init(void)
{
    XINC_LOG_INFO("Init");

    m_shutdown_started = false;
    xinc_mtx_init(&m_sysoff_mtx);
    xinc_section_iter_init(&m_handlers_iter, &pwr_mgmt_data);
    
    PWR_MGMT_SLEEP_INIT();
//    PWR_MGMT_DEBUG_PINS_INIT();
    PWR_MGMT_STANDBY_TIMEOUT_INIT();
    PWR_MGMT_CPU_USAGE_MONITOR_INIT();
    PWR_MGMT_BLE_STACK_MONITOR_INIT();

    return PWR_MGMT_TIMER_CREATE();
}




void xinc_pwr_mgmt_run(void)
{
    uint32_t ble_sleep;
    PWR_MGMT_FPU_SLEEP_PREPARE();
    PWR_MGMT_SLEEP_LOCK_ACQUIRE();
    PWR_MGMT_CPU_USAGE_MONITOR_SECTION_ENTER();
    ble_sleep = PWR_MGMT_BLE_STACK_MONITOR_SECTION_ENTER();
 //   PWR_MGMT_DEBUG_PIN_SET();
    PWR_gpio_sleep_config();
    if(ble_sleep)
    {
        __NOP();							
		__NOP();							
		__NOP(); 
		__WFI();							
		__NOP();							
		__NOP();							
		__NOP(); 
    }

  //  PWR_MGMT_DEBUG_PIN_CLEAR();
    PWR_MGMT_BLE_STACK_MONITOR_SECTION_EXIT();
    PWR_MGMT_CPU_USAGE_MONITOR_SECTION_EXIT(); 
    PWR_MGMT_SLEEP_LOCK_RELEASE();
}

void xinc_pwr_mgmt_feed(void)
{
    XINC_LOG_DEBUG("Feed");
    // It does not stop started shutdown process.
    PWR_MGMT_STANDBY_TIMEOUT_CLEAR();
}

/**@brief Function runs the shutdown procedure.
 */
static void shutdown_process(void)
{
    XINC_LOG_INFO("Shutdown started. Type %d", m_pwr_mgmt_evt);
    printf("Shutdown started. Type %d\r\n", m_pwr_mgmt_evt);
    {
        while(1);
    }
    // Executing all callbacks.
    for (/* m_handlers_iter is initialized in xinc_pwr_mgmt_init(). Thanks to that each handler is
            called only once.*/;
         xinc_section_iter_get(&m_handlers_iter) != NULL;
         xinc_section_iter_next(&m_handlers_iter))
    {
        xinc_pwr_mgmt_shutdown_handler_t * p_handler =
            (xinc_pwr_mgmt_shutdown_handler_t *) xinc_section_iter_get(&m_handlers_iter);
        if ((*p_handler)(m_pwr_mgmt_evt))
        {
            XINC_LOG_INFO("SysOff handler 0x%08X => ready", (unsigned int)*p_handler);
            printf("SysOff handler 0x%08X => ready", (unsigned int)*p_handler);
        }
        else
        {
            // One of the modules is not ready.
            XINC_LOG_INFO("SysOff handler 0x%08X => blocking", (unsigned int)*p_handler);
            printf("SysOff handler 0x%08X => blocking", (unsigned int)*p_handler);           
            return;
        }
    }

    PWR_MGMT_CPU_USAGE_MONITOR_SUMMARY();
    XINC_LOG_INFO("Shutdown complete.");
    XINC_LOG_FINAL_FLUSH();

    if ((m_pwr_mgmt_evt == XINC_PWR_MGMT_EVT_PREPARE_RESET)
     || (m_pwr_mgmt_evt == XINC_PWR_MGMT_EVT_PREPARE_DFU))
    {
        NVIC_SystemReset();
    }
    else
    {
        // Enter System OFF.
        xinc_power_system_off();
    }
}

#if XINC_PWR_MGMT_CONFIG_USE_SCHEDULER
/**@brief Handle events from app_scheduler.
 */
static void scheduler_shutdown_handler(void * p_event_data, uint16_t event_size)
{
    UNUSED_PARAMETER(p_event_data);
    UNUSED_PARAMETER(event_size);
    shutdown_process();
}
#endif // XINC_PWR_MGMT_CONFIG_USE_SCHEDULER

void xinc_pwr_mgmt_shutdown(xinc_pwr_mgmt_shutdown_t shutdown_type)
{
    // Check if shutdown procedure is not started.
    if (!xinc_mtx_trylock(&m_sysoff_mtx))
    {
        return;
    }

    if (shutdown_type != XINC_PWR_MGMT_SHUTDOWN_CONTINUE)
    {
        if (m_shutdown_started)
        {
            xinc_mtx_unlock(&m_sysoff_mtx);
            return;
        }
        else
        {
            m_pwr_mgmt_evt      = (xinc_pwr_mgmt_evt_t)shutdown_type;
            m_shutdown_started  = true;
        }
    }

    ASSERT(m_shutdown_started);
    XINC_LOG_INFO("Shutdown request %d", shutdown_type);

#if XINC_PWR_MGMT_CONFIG_USE_SCHEDULER
    ret_code_t ret_code = app_sched_event_put(NULL, 0, scheduler_shutdown_handler);
    APP_ERROR_CHECK(ret_code);
#else
    shutdown_process();
#endif // XINC_PWR_MGMT_CONFIG_USE_SCHEDULER

    xinc_mtx_unlock(&m_sysoff_mtx);
}

#endif // XINC_MODULE_ENABLED(XINC_PWR_MGMT)
