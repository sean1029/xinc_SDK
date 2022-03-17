#include "app_test.h"


#include "nrf_cli_uart.h"
#include "nrf_cli.h"
#include "app_uart.h"

#if CLI_TEST_EN

#if NRF_MODULE_ENABLED(NRF_CLI)

NRF_CLI_UART_DEF(cli_uart, 1, 64, 64);
NRF_CLI_DEF(m_cli_uart, "cli:~$ ", &cli_uart.transport, '\r', 4);
#endif
void cli_test(void)
{
    #if NRF_MODULE_ENABLED(NRF_CLI)
    xinc_drv_uart_config_t uart_config;
    uart_config.pseltxd = CLI_TX_PIN_NUMBER;
    uart_config.pselrxd = CLI_RX_PIN_NUMBER;
    uart_config.hwfc    = XINC_UART_HWFC_DISABLED;
    uart_config.baudrate = UART_BAUDRATE_BAUDRATE_Baud115200;

    ret_code_t err_code = nrf_cli_init(&m_cli_uart, &uart_config, false, false, NRF_LOG_SEVERITY_NONE);
    APP_ERROR_CHECK(err_code);
    #endif	
}

void cli_processt(void)
{
    #if NRF_MODULE_ENABLED(NRF_CLI)
    nrf_cli_process(&m_cli_uart);
    #endif	
}
		
#endif 

#if DRV_UART_TEST_EN

#if NRFX_CHECK(XINCX_UART_ENABLED)
void uart_event_handle(app_uart_evt_t * p_event)
{
    static uint8_t data_array[20];
    static uint16_t index = 0;
    uint32_t ret_val;
    uint32_t err_code;
    int i = 0;
	 // printf("uart_event_handle ,evt_type:%d\r\n",p_event->evt_type);
    switch (p_event->evt_type)
    {
        /**@snippet [Handling data from UART] */
        case APP_UART_DATA_READY:
            UNUSED_VARIABLE(app_uart_get(&data_array[0]));
            index++;
            app_uart_get(&data_array[0]);
    	//	printf("Data:%c\r\n",data_array[0]);
        //	for( i = 0; i< 16;i++)
            {
                err_code = app_uart_put(data_array[0] + 0);
                if(err_code != 0)
                {
                    break;
                }
            }
                    //	printf("uart_put %d data\r\n",i);	
            index = 0;
            
            break;

        /**@snippet [Handling data from UART] */
        case APP_UART_COMMUNICATION_ERROR:
            
            break;

        case APP_UART_FIFO_ERROR:
            
            break;
                
        case APP_UART_DATA:
            app_uart_get(&data_array[0]);
            //  printf("data:%c \r\n",data_array[0]);
        if(app_uart_put(data_array[0]))
        {
            printf("put err\n");
        }
    //    app_uart_put(data_array[0]);
        break;

        default:
            break;
    }
    (void)i;
    (void)ret_val;
}
#endif

#define UART_TX_BUF_SIZE        64                                     /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE        64                                     /**< UART RX buffer size. */

void drv_uart_test(void)
{
    #if NRFX_CHECK(XINCX_UART_ENABLED)
    ret_code_t err_code;

    app_uart_comm_params_t const comm_params =
    {
        .rx_pin_no    = APP_UART_RX_PIN_NUMBER,
        .tx_pin_no    = APP_UART_TX_PIN_NUMBER,
        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
        .use_parity   = false,
        .data_bits = 3,
        .stop_bits = 0,
        .baud_rate    = UART_BAUDRATE_BAUDRATE_Baud115200
    };

    APP_UART_FIFO_INIT(&comm_params,
                        UART_RX_BUF_SIZE,
                        UART_TX_BUF_SIZE,
                        uart_event_handle,
                        APP_IRQ_PRIORITY_LOWEST,
                        err_code);

    APP_ERROR_CHECK(err_code);
    #endif
}
#endif 

/**
 *@brief Function for initializing logging.
 */
#if  LOG_TEST_EN
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
 void log_test(void)
{
    #if NRF_MODULE_ENABLED(NRF_LOG)
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
    printf("%s err_code:%d\r\n",__func__,err_code);
    NRF_LOG_DEFAULT_BACKENDS_INIT();
    #endif
}

void log_flush(void)
{
    #if NRF_MODULE_ENABLED(NRF_LOG)
    NRF_LOG_FLUSH();
    #endif
}
	 
#endif // LOG_TEST_EN



#if DRV_SAADC_TEST_EN

#if NRFX_CHECK(XINCX_SAADC_ENABLED)
#define SAMPLES_IN_BUFFER 32

static xinc_drv_saadc_t m_saadc = XINCX_SAADC_INSTANCE(0);


static xinc_saadc_value_t   m_buffer_pool[2][SAMPLES_IN_BUFFER];

void saadc_callback(xinc_drv_saadc_evt_t const * p_event)
{
    printf("%s\n",__func__);
    uint32_t val;
    if (p_event->type == XINC_DRV_SAADC_EVT_DONE)
    {
        val = p_event->data.done.adc_value;
        printf("1.0v,channel=%d,value=[%d], before cali Voltage:%f V, after cali Voltage:%f V \r\n",\
            p_event->data.done.channel, val,((val)*2.47)/(1.0*1024),   ((val)*2.47)/(1.0*1024));		
    }
}
#endif

void saadc_init(void)
{
    #if NRFX_CHECK(XINCX_SAADC_ENABLED)
    ret_code_t err_code;
        

    xinc_saadc_channel_config_t channel_config =  XINC_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE;

    err_code = xinc_drv_saadc_init(&m_saadc,NULL, saadc_callback);
        
    APP_ERROR_CHECK(err_code);

    err_code = xinc_drv_saadc_channel_init(&m_saadc,8, &channel_config);
    err_code = xinc_drv_saadc_channel_init(&m_saadc,5, &channel_config);

    APP_ERROR_CHECK(err_code);

        err_code = xinc_drv_saadc_buffer_convert(&m_saadc,m_buffer_pool[0], SAMPLES_IN_BUFFER);
    xincx_saadc_sample(&m_saadc,8);
    

        APP_ERROR_CHECK(err_code);
        printf("%s,%x\n",__func__,err_code);

#endif
}
 uint32_t min_val = 0xffff;
 uint32_t max_val = 0;


void drv_adc_test(void)
{
    #if NRFX_CHECK(XINCX_SAADC_ENABLED)
    saadc_init();
    #endif
}
#endif // DRV_SAADC_TEST_EN

#if RTC_TEST_EN

#if NRFX_CHECK(XINCX_RTC_ENABLED)
const xincx_rtc_t rtc = XINCX_RTC_INSTANCE(0); /**< Declaring an instance of xinc_drv_rtc for RTC0. */

static void rtc_handler(xinc_drv_rtc_int_type_t int_type)
{

    xinc_rtc_time_t rtc_time_val;
    if (int_type == XINCX_RTC_INT_SEC)
    {
        xinc_drv_rtc_date_get(&rtc,&rtc_time_val);
            
        printf("SEC:day:%d::week:%d hour:min:sec %d:%d:%d\r\n",rtc_time_val.day,rtc_time_val.week,rtc_time_val.hour,rtc_time_val.min,rtc_time_val.sec);
        if(rtc_time_val.sec == 10)
        {
            rtc_time_val.sec = 2;
            rtc_time_val.hour = 1;
            rtc_time_val.day = 1;
            rtc_time_val.week = 2;
            rtc_time_val.min = 1;
            
        //	xinc_rtc_date_set(rtc.p_reg, rtc_time_val);
        }
    }
    else if (int_type == XINCX_RTC_INT_MIN)
    {
        xinc_drv_rtc_date_get(&rtc,&rtc_time_val);
            
        printf("MIN:day:%d::week:%d hour:min:sec %d:%d:%d\r\n",rtc_time_val.day,rtc_time_val.week,rtc_time_val.hour,rtc_time_val.min,rtc_time_val.sec);
    }
    else if (int_type == XINCX_RTC_INT_HOUR)
    {
        xinc_drv_rtc_date_get(&rtc,&rtc_time_val);
            
        printf("HOUR:day:%d::week:%d hour:min:sec %d:%d:%d\r\n",rtc_time_val.day,rtc_time_val.week,rtc_time_val.hour,rtc_time_val.min,rtc_time_val.sec);
    }
    else if (int_type == XINCX_RTC_INT_DAY)
    {
        xinc_drv_rtc_date_get(&rtc,&rtc_time_val);
            
        printf("DAY:day:%d::week:%d hour:min:sec %d:%d:%d\r\n",rtc_time_val.day,rtc_time_val.week,rtc_time_val.hour,rtc_time_val.min,rtc_time_val.sec);
    }
    else if (int_type == XINCX_RTC_INT_TIME1)
    {
        xinc_drv_rtc_date_get(&rtc,&rtc_time_val);
            
        printf("TIMER1:day:%d::week:%d hour:min:sec %d:%d:%d\r\n",rtc_time_val.day,rtc_time_val.week,rtc_time_val.hour,rtc_time_val.min,rtc_time_val.sec);
    }
     else if (int_type == XINCX_RTC_INT_TIME2)
    {
        xinc_drv_rtc_date_get(&rtc,&rtc_time_val);
            
        printf("TIMER2:day:%d::week:%d hour:min:sec %d:%d:%d\r\n",rtc_time_val.day,rtc_time_val.week,rtc_time_val.hour,rtc_time_val.min,rtc_time_val.sec);
    }
     else if (int_type == XINCX_RTC_INT_TIME3)
    {
        xinc_drv_rtc_date_get(&rtc,&rtc_time_val);
            
        printf("TIMER3:day:%d::week:%d hour:min:sec %d:%d:%d\r\n",rtc_time_val.day,rtc_time_val.week,rtc_time_val.hour,rtc_time_val.min,rtc_time_val.sec);
    }

}
#endif


void rtc_test(void)
{
#if NRFX_CHECK(XINCX_RTC_ENABLED)
    uint32_t err_code;

    //Initialize RTC instance
    xincx_rtc_config_t config = XINCX_RTC_DEFAULT_CONFIG;
    config.freq = 32768;
    config.type = XINC_RTC_TYPE_RTC;
    config.date.week = RTC_WVR_WLR_Monday;
    config.date.day = 4;
    config.date.hour = 18;
    config.date.min = 34;
    config.date.sec = 0;

    err_code = xinc_drv_rtc_init(&rtc, &config, rtc_handler);
    printf("%s,err_code:0x%x ",__func__,err_code);

    xincx_rtc_match_config_t time;
    memset(&time,0,sizeof(time));
    time.times.sec = 20;
    time.times.min = 0;
    time.times.week = XINCX_RTC_WEEK_MATCH_SUNDAY | XINCX_RTC_WEEK_MATCH_MONDAY;
    xinc_drv_rtc_time_set(&rtc,XINCX_RTC_MATCH_TIME_1,time,true);
    //Power on RTC instance
    xincx_rtc_enable(&rtc);
    	xinc_drv_rtc_sec_int_enable(&rtc,true);
    //	xinc_drv_rtc_min_int_enable(&rtc,true);
#endif
	
}
#endif //



APP_TIMER_DEF(m_test_tmr);

static void test_timer_handler(void * p_context)
{
    printf("test_timer_handler \r\n");
//	app_timer_start(m_test_tmr, APP_TIMER_TICKS(10000), (void*)0);
}

void timer_test()
{
    uint32_t err_code;
    err_code = app_timer_create(&m_test_tmr,
                                                                    APP_TIMER_MODE_SINGLE_SHOT,																	test_timer_handler);
    printf("timer_test err_code:0x%x\r\n",err_code);

}

#if DRV_TIMER_TEST_EN

#if NRFX_CHECK(XINCX_TIMER_ENABLED)
const xinc_drv_timer_t TIMER_LED = XINC_DRV_TIMER_INSTANCE(2);

void timer_led_event_handler(xinc_timer_int_event_t event_type,uint8_t channel, void* p_context)
{
    static uint32_t i = 0;
        static uint8_t on_off = 0;
    xinc_saadc_value_t adc_val;
    //		printf("timer_led_event_handler event_type:[%d],channel:%d\n",event_type,channel);
    
   // 
    switch (event_type)
    {
        case XINC_TIMER_EVENT_TIMEOUT:
        {				
            if(on_off == 0)
            {						
                GPIO_OUTPUT_HIGH(5);
                on_off = 1;		
            //xincx_saadc_sample(&m_saadc,8);                
            }else
            {						
                GPIO_OUTPUT_LOW(5);
           //     xincx_saadc_sample_convert(&m_saadc,8,&adc_val);
             //   printf("adc_val:%d\n",adc_val);
                on_off = 0;
                i++;
                if(i > 99)
                {
                    i = 1;
                }
            //	printf("timer_led_event_handler event_type:[%d],channel:%d\n",event_type,channel);
            //	pwm_update_duty(i);
            }	
        }break;

		default:
            //Do nothing.
            break;
    }
} 
#endif //

void drv_timer_test(void)
{
	
#if NRFX_CHECK(XINCX_TIMER_ENABLED)
    uint32_t time_ms = 3000; //Time(in miliseconds) between consecutive compare events.
    uint32_t time_ticks;
    uint32_t err_code = NRF_SUCCESS;

    //Configure TIMER_LED for generating simple light effect - leds on board will invert his state one after the other.
    xinc_drv_timer_config_t timer_cfg = XINC_DRV_TIMER_DEFAULT_CONFIG;
    err_code = xinc_drv_timer_init(&TIMER_LED, &timer_cfg, timer_led_event_handler);
    APP_ERROR_CHECK(err_code);

    time_ticks = xinc_drv_timer_us_to_ticks(&TIMER_LED, time_ms);
    printf("time_ticks = [%d]\n",time_ticks);
    time_ticks = xinc_drv_timer_ms_to_ticks(&TIMER_LED, time_ms);
    printf("time_ticks = [%d]\n",time_ticks);


    xinc_drv_timer_compare(&TIMER_LED, time_ticks, XINC_TIMER_MODE_USER_COUNTER,true);

   // xinc_drv_timer_enable(&TIMER_LED);
#endif //

}
#endif // 

#if DRV_WDT_TEST_EN

#if NRFX_CHECK(XINCX_WDT_ENABLED)
/**
 * @brief WDT events handler.
 */
void wdt_event_handler(void)
{
    //bsp_board_leds_off();
    printf("wdt_event_handler\n");
    xincx_wdt_feed();
    //NOTE: The max amount of time we can spend in WDT interrupt is two cycles of 32768[Hz] clock - after that, reset occurs
}
#endif
 void drv_wdg_test(void)
{
    #if NRFX_CHECK(XINCX_WDT_ENABLED)
    uint32_t err_code;

    //Initialize RTC instance
    xinc_drv_wdt_config_t config = XINC_DRV_WDT_DEAFULT_CONFIG;
    err_code = xinc_drv_wdt_init(&config, wdt_event_handler);
    APP_ERROR_CHECK(err_code);

    xinc_drv_wdt_enable();

    #endif
}
#endif


#if BSP_BUTTON_TEST_EN

static void bsp_evt_handler(bsp_event_t event)
{
    printf("%s,event:%d ",__func__,event);
    switch (event)
    {
        case BSP_EVENT_KEY_0:
        {
            printf("push \r\n");
        }
        break;

        case BSP_EVENT_KEY_1:
        {
            printf("long push \r\n");
        //	while(1);
        }
        break;
                
        case BSP_EVENT_KEY_2:
        {
            printf("release \r\n");
        }break;

        default:
            return;
    }
}

void bsp_button_led_test(void)
{
    bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS,bsp_evt_handler);
}

#endif



#if APP_BUTTON_TEST_EN

static void app_button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    printf("%s,pin_no:0x%x,button_action:%d\r\n",__func__,pin_no,button_action);
    switch (pin_no)
    {
        case BSP_BUTTON_0:
            
        break;

        case BSP_BUTTON_1:
        break;
        
        default:
        APP_ERROR_HANDLER(pin_no);
        break;
    }
}

static const app_button_cfg_t app_buttons[BUTTONS_NUMBER] =
{
	
    #ifdef BSP_BUTTON_0
    {BSP_BUTTON_0, false, BUTTON_PULLDOWN, app_button_event_handler},
    #endif // BUTTON_0

    #ifdef BSP_BUTTON_1
    {BSP_BUTTON_1, false, BUTTON_PULLDOWN, app_button_event_handler},
    #endif // BUTTON_1
        
};

	
void app_button_test(void)
{
    uint32_t err_code;
        
    err_code = app_button_init((app_button_cfg_t *)app_buttons,2, APP_TIMER_TICKS(50));

    printf("%s,err_code:0x%x\r\n",__func__,err_code);

}
#endif



#if NRFX_CHECK(XINCX_PWM_ENABLED)
static xinc_drv_pwm_t m_pwm0 = XINC_DRV_PWM_INSTANCE(0);
static xinc_drv_pwm_t m_pwm1 = XINC_DRV_PWM_INSTANCE(1);
static xinc_drv_pwm_t m_pwm2 = XINC_DRV_PWM_INSTANCE(2);
static xinc_drv_pwm_t m_pwm3 = XINC_DRV_PWM_INSTANCE(3);
static xinc_drv_pwm_t m_pwm4 = XINC_DRV_PWM_INSTANCE(4);
static xinc_drv_pwm_t m_pwm5 = XINC_DRV_PWM_INSTANCE(5);
#endif



void pwm_update_duty(uint8_t duty)
{
	#if NRFX_CHECK(XINCX_PWM_ENABLED)
	printf("pwm_update_duty:%d\n",duty);
	xinc_drv_pwm_duty_cycle_update(&m_pwm0,duty);
	#endif
}

void drv_pwm_test(void)
{
#if NRFX_CHECK(XINCX_PWM_ENABLED)
    xinc_drv_pwm_config_t const config0 =
    {
        .clk_src = XINC_PWM_CLK_SRC_32K,
        .ref_clk   = XINC_PWM_REF_CLK_8MHzOr8K,//XINC_PWM_REF_CLK_8MHz,//XINC_PWM_REF_CLK_32000Hz
        .frequency       = 2,
        .duty_cycle   = 75,
        .inv_delay   = XINC_PWMCOMPTIME_VAL_4clk,
        .inv_enable   = true,
        .start   = true
    };
    APP_ERROR_CHECK(xinc_drv_pwm_init(&m_pwm0, &config0, NULL));
        
    printf("freq_valid:%d\n",xinc_drv_pwm_freq_valid_range_check(config0.clk_src,config0.ref_clk,200));;
        
	//	xinc_drv_pwm_start(&m_pwm0);
	
    xinc_drv_pwm_config_t const config1 =
    {
        .clk_src = XINC_PWM_CLK_SRC_32K,
        .ref_clk   = XINC_PWM_REF_CLK_1MHzOr1K,
        .frequency       = 6,
        .duty_cycle   = 50,
        .start   = true
    };
  //  APP_ERROR_CHECK(xinc_drv_pwm_init(&m_pwm1, &config1, NULL));
	
	xinc_drv_pwm_config_t const config2 =
    {
        .clk_src = XINC_PWM_CLK_SRC_32M_DIV,
        .ref_clk   = XINC_PWM_REF_CLK_8MHzOr8K,
        .frequency       = 2000,
        .duty_cycle   = 33
    };
//	APP_ERROR_CHECK(xinc_drv_pwm_init(&m_pwm2, &config2, NULL));
	
	xinc_drv_pwm_config_t const config3 =
    {
        .clk_src = XINC_PWM_CLK_SRC_32M_DIV,
        .ref_clk   = XINC_PWM_REF_CLK_8MHzOr8K,
        .frequency       = 2000,
        .duty_cycle   = 44
    };
//	APP_ERROR_CHECK(xinc_drv_pwm_init(&m_pwm3, &config3, NULL));
		
    xinc_drv_pwm_config_t const config4 =
    {
        .clk_src = XINC_PWM_CLK_SRC_32M_DIV,
        .ref_clk   = XINC_PWM_REF_CLK_8MHzOr8K,
        .frequency       = 2000,
        .duty_cycle   = 44
    };
//	APP_ERROR_CHECK(xinc_drv_pwm_init(&m_pwm4, &config4, NULL));
		
    xinc_drv_pwm_config_t const config5 =
    {
        .clk_src = XINC_PWM_CLK_SRC_32M_DIV,
        .ref_clk   = XINC_PWM_REF_CLK_8MHzOr8K,
        .frequency       = 2000,
        .duty_cycle   = 44
    };
	//APP_ERROR_CHECK(xinc_drv_pwm_init(&m_pwm5, &config5, NULL));
		
#endif
}

