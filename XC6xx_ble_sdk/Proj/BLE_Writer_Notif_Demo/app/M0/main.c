
#include    <stdio.h>
#include    <string.h>
#include	"Includes.h"

#define APP_BLE_FEATURE_ENABLED  XINCX_CHECK(XINC_BLE_STACK_ENABLED) 

#include "bsp_gpio.h"
#include "sbc.h"
#include "voice_ringbuf.h"
#include "bsp_timer.h"
#include "bsp_pwm.h"
#include    "bsp_spi_flash.h"

#include "xinc_gpio.h"
#include "bsp_uart.h"
#include "xc_kbs_event.h"
#include "xincx_gpio.h"
#include "xinc_drv_spi.h"
#include "AT24C02.h"
#include "xinc_drv_saadc.h"
#include "xinc_drv_rtc.h"
#include "bsp_clk.h"
#include "xinc_drv_timer.h"
#include "xinc_drv_pwm.h"
#include "xinc_drv_wdt.h"
#include "app_button.h"
#include "app_error.h"
#include "app_timer.h"
#include "bsp.h"
#include "app_scheduler.h"
#include "xincx_kbs.h"
#include "xinc_delay.h"
#include "mem_manager.h"
#include "xinc_pwr_mgmt.h"
#include "xinc_log_ctrl.h"
 #include "xinc_spi_flash.h"
 #include "xinc_drv_timer.h"
 
#if defined (XC60XX_M0)
#include "btstack_run_loop.h"
#include    "profile.h"
#include    "ble.h"
#include "hids_device.h"
#include "le_device_db.h"
#endif

#if defined (XC66XX_M4)
#include    "btstack_defines.h"
#include    "btstack_run_loop.h"
#include    "profile.h"
#include    "ble.h"
#include "rom_variable.h"
#endif
uint8_t flag_show_hci = 0;

#if (APP_BLE_FEATURE_ENABLED)
void _HWradio_Go_To_Idle_State_Patch(void){
}
#endif
extern uint32_t  GulSystickCount;
uint32_t  LastTimeGulSystickCount=0xFF;

void sm_peripheral_setup(void)
{
}


extern	void	gpio_mux_ctl(uint8_t num,uint8_t mux);
extern	void	gpio_fun_sel(uint8_t num,uint8_t sel);
extern	void    gpio_direction_input(uint8_t num, uint8_t pull_up_type);
extern  void    gpio_fun_inter(uint8_t num,uint8_t inter);
extern	uint8_t gpio_input_val(uint8_t num);
extern  void ble_system_idle_init(void);
extern  void    ble_system_idle(void);
#if (APP_BLE_FEATURE_ENABLED)
extern  int btstack_main(void);
extern 	void send_media_report(int mediacode1,int mediacode2);
#endif

static uint32_t min(uint32_t a, uint32_t b){
    return a < b ? a : b;
}

#if (APP_BLE_FEATURE_ENABLED)
static int g_conn_stat = 0;
static int app_get_connect_state(void)
{
	return g_conn_stat;
}

// att_read_callback helpers
static uint16_t att_read_callback_handle_blob(const uint8_t * blob, uint16_t blob_size, uint16_t offset, uint8_t * buffer, uint16_t buffer_size){
    if (buffer){
        uint16_t bytes_to_copy = min(blob_size - offset, buffer_size);
        memcpy(buffer, &blob[offset], bytes_to_copy);
        return bytes_to_copy;
    } else {
        return blob_size;
    }
}

//static uint8_t read_buf[10] = {0};
// read requests
static uint16_t app_att_read_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size){

    printf("main %s, con_handle=%x, att_handle=%x, offset=%x, buffer=%x, size=%x\n",__func__, con_handle, att_handle, offset,(uint32_t)buffer, buffer_size);
    
    if((att_handle != ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE) && 
       (att_handle != ATT_CHARACTERISTIC_0000FF12_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE)) return 0;
    
    static uint8_t read_buf[10] = {0};
    return att_read_callback_handle_blob((const uint8_t *)read_buf, sizeof(read_buf), offset, buffer, buffer_size);
}

// write requests
static int app_att_write_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size){
    
    uint32_t le_notification_enabled;
    printf("main %s, con_handle=%x, att_handle=%x, offset=%x, buffer=%x, size=%x\n", __func__, con_handle, att_handle, offset, (uint32_t)buffer, buffer_size);  
    if (transaction_mode != ATT_TRANSACTION_MODE_NONE) return 0;
    switch(att_handle)
    {
    case    ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_CLIENT_CONFIGURATION_HANDLE:
        le_notification_enabled = little_endian_read_16(buffer, 0) == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION;
        printf("Notifications enabled %u\n",le_notification_enabled); 
        break;   
    case    ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE:
    case    ATT_CHARACTERISTIC_0000FF12_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE:
        printf("att_handle=0x%x, offset=0x%x, length=0x%x\n", att_handle, offset, buffer_size);
        printf("att data: ");
        for(uint32_t i=0; i<buffer_size; i++) printf("0x%x ", buffer[i]);
        printf("\n");
        break;
    default:
        break;
    }    
    return 0;
} 

hci_con_handle_t connection_handle ;

static void app_packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
	
    if (packet_type != HCI_EVENT_PACKET) return;
    
   
    switch(hci_event_packet_get_type(packet))
    {
    case BTSTACK_EVENT_STATE:
        if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) return;
        printf("BTstack up and running.\n");
        break;
        
    case HCI_EVENT_LE_META:
         printf("HCI_EVENT_LE_META ");
        switch (hci_event_le_meta_get_subevent_code(packet)) 
        {
            printf("le_meta_get_subevent_code:0x%x\n",hci_event_le_meta_get_subevent_code(packet));
        case HCI_SUBEVENT_LE_CONNECTION_COMPLETE: {
                connection_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
                printf("\n CONNECT RIGHT ! (HANDLE = 0x%x)\n", connection_handle);
//      	   gatt_client_read_value_of_characteristics_by_uuid16(packet_handler,connection_handle,0x01,0xff,0x2A00);
				g_conn_stat = 1;
			printf("Connected, requesting conn param update for handle 0x%04x\n", connection_handle);
            }		
            break;               
        default:
            break;
        }
        break;
            
    case HCI_EVENT_DISCONNECTION_COMPLETE:
        printf("\n DISCONNECT ! (HANDLE = 0x%x) ", hci_event_disconnection_complete_get_connection_handle(packet)); 
        printf("REASON = 0x%x\n", hci_event_disconnection_complete_get_reason(packet));
		g_conn_stat = 0;
        break;
    
    case L2CAP_EVENT_CONNECTION_PARAMETER_UPDATE_RESPONSE:   
        printf("L2CAP Connection Parameter Update Complete, response: %x\n", l2cap_event_connection_parameter_update_response_get_result(packet));
        break;

    case ATT_EVENT_MTU_EXCHANGE_COMPLETE:
        printf("ATT MTU = %u\n", att_event_mtu_exchange_complete_get_MTU(packet));
        break;
    
    case ATT_EVENT_HANDLE_VALUE_INDICATION_COMPLETE:
			  printf("INDICATION_COMPLETE = %u\n", att_event_handle_value_indication_complete_get_status(packet));
        (att_event_handle_value_indication_complete_get_status(packet) == 0) ? printf("INDICATION OK\n") : printf("INDICATION FAIL\n");
        break;
	
    default:
			//	printf("default = 0x%x\n", hci_event_packet_get_type(packet));
        break;
    }
}

static const blestack_init_t blestack_init = {
    
    /* uint16_t (*att_read_callback)(...);*/ app_att_read_callback,
    /* int (*att_write_callback)(...);    */ app_att_write_callback,
    /* void (*packet_handler)(...)        */ app_packet_handler,
    /* const uint8_t *ble_device_db       */ profile_data
};

const uint8_t adv_diect_data[] = {
    // Flags general discoverable, BR/EDR not supported
    0x02, 0x01, 0x06, 

	0x03, 0x03, 0x12, 0x18,
    // Name
    0x0a, 0x09, 'H', 'I', 'D',  '-','D','E','M','O','1',

    0x03, 0x19, 0xC1, 0x03, //keyboard
	//0x03, 0x19, 0xC2, 0x03,   //mouse
	// Service Solicitation, 128-bit UUIDs - ANCS (little endian)
//	0x11,0x15,0xD0,0x00,0x2D,0x12,0x1E,0x4B,0x0F,0xA4,0x99,0x4E,0xCE,0xB5,0x31,0xF4,0x05,0x79	
};
uint8_t adv_diect_data_len = sizeof(adv_diect_data);

extern uint8_t bd_addr[6];


const uint8_t adv_powerOn_data[] = {
    // Flags general discoverable, BR/EDR not supported
    0x02, 0x01, 0x04, 

	0x03, 0x02, 0x12, 0x18,
    
  0x03, 0x19, 0x80,0x01,
	0x0d,0xFF,0x5d,0x00,
	0x03,0x00,0x01,0x03,0x47,0x53,0xE4,0xAA,0xC6,0xB8

};
uint8_t adv_poweron_data_len = sizeof(adv_powerOn_data);

const uint8_t adv_pair_data[] = {
  0x02, 0x01, 0x05, 

	0x03, 0x02, 0x12, 0x18,
    
  0x03, 0x19, 0x80,0x01,
	0x0E,0x09,0x53,0x4b,0x59,0x57,0x4f,0x52,0x54,0x48,0x5f,0x30,0x31,0x37,0x32
};
uint8_t adv_pair_data_len = sizeof(adv_pair_data);



const uint8_t scanresp_data[] = {
    //- Manufacture.

    0x0E,0x09,0x53,0x4b,0x59,0x57,0x4f,0x52,0x54,0x48,0x5f,0x30,0x31,0x37,0x32
};
uint8_t scanresp_data_len = sizeof(scanresp_data);

void set_bd_addr()          
{ 
	extern uint8_t bd_addr[6];
    bd_addr[0]=0xad;
    bd_addr[1]=0x04;
    bd_addr[2]=0x02;
    bd_addr[3]=0x7e;
    bd_addr[4]=0x31;
    bd_addr[5]=0x28;
}
#endif

extern int key_value ;

void key_init(void)
{
	Init_gpio();

}

enum adv_type{
	ADV_IND,
	ADV_DIRECT_IND,
	ADV_SCAN_IND,
	ADV_NONCONN_IND,
};

#if (APP_BLE_FEATURE_ENABLED)
void send_power_on_adv(void)
{
	flag_show_hci =1;
	printf("send_power_on_adv\n");
	uint16_t adv_int_min = 0x0130;
    uint16_t adv_int_max = 0x0130;
	 bd_addr_t null_addr;
    memset(null_addr, 0, 6);
	gap_advertisements_enable(0);
	 gap_advertisements_set_params(adv_int_min, adv_int_max, ADV_IND, 0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data(adv_poweron_data_len, (uint8_t*) adv_powerOn_data);
    gap_scan_response_set_data(scanresp_data_len , (uint8_t*) scanresp_data);
    gap_advertisements_enable(1);
}
#endif

void stack_reset(void)
{
    #if (APP_BLE_FEATURE_ENABLED)
	bd_addr_t null_addr;   
	btstack_main();
  
	voice_ring_buffer_init();

	uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
    
	memset(null_addr, 0, 6);
	gap_advertisements_set_data(sizeof(adv_pair_data), (uint8_t*) adv_pair_data);
	gap_scan_response_set_data(sizeof(scanresp_data), (uint8_t*) scanresp_data);
	gap_advertisements_set_params(adv_int_min, adv_int_max, ADV_IND, 0, null_addr, 0x07, 0x00);
	gap_advertisements_enable(1);
    #endif
}

#if (APP_BLE_FEATURE_ENABLED)
extern uint8_t con_flag;

static btstack_timer_source_t sys_run_timer;
#endif 



/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code = XINC_SUCCESS;
    err_code = xinc_pwr_mgmt_init();
    printf("xinc_pwr_mgmt_init err_code:0x%x\r\n",err_code);
        
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling the idle state (main loop).
 *
 * @details Handle any pending log operation(s), then sleep until the next event occurs.
 */
static void idle_state_handle(void)
{
//    if (XINC_LOG_PROCESS() == false)
//    {
        xinc_pwr_mgmt_run();
//    }
}


#define SCHED_MAX_EVENT_DATA_SIZE           APP_TIMER_SCHED_EVENT_DATA_SIZE             //!< Maximum size of the scheduler event data.
#define SCHED_QUEUE_SIZE                    10                                          //!< Size of the scheduler queue.

static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}

uint32_t timer_evt_cnt = 0;
const xinc_drv_timer_t TIMER_TEST = XINC_DRV_TIMER_INSTANCE(2);

void timer_TEST_event_handler(xinc_timer_int_event_t event_type,uint8_t channel, void* p_context)
{

    printf("timer,cnt:%d\n",timer_evt_cnt++);
// printf("timer_TEST_event_handler event_type:[%d],channel:%d,cnt:%d\n",event_type,channel,timer_evt_cnt++);
    switch (event_type)
    {
        case XINC_TIMER_EVENT_TIMEOUT:
        {				
         //  bsp_board_led_invert(bsp_board_pin_to_led_idx(LED_1));
        }break;

		default:
            //Do nothing.
            break;
    }
}


void timer_test()
{
    ret_code_t err_code;

    
    uint32_t time_ms = 5000; //Time(in miliseconds) between consecutive compare events.
    uint32_t time_ticks;
    xinc_drv_timer_config_t timer_TEST_cfg = XINC_DRV_TIMER_DEFAULT_CONFIG;

   

    err_code = xinc_drv_timer_init(&TIMER_TEST, &timer_TEST_cfg, timer_TEST_event_handler);
    printf("drv_timer_init err_code:%x\r\n",err_code);
     time_ticks = xinc_drv_timer_ms_to_ticks(&TIMER_TEST, time_ms);
    printf("time_ticks:%d\r\n",time_ticks);
    
    

    xinc_drv_timer_compare(&TIMER_TEST, time_ticks, XINC_TIMER_MODE_USER_COUNTER,true);
}

/**@brief Function for handling bsp events.
 */
void bsp_evt_handler(bsp_event_t evt)
{
    switch (evt)
    {
        // ??????S1 ??????????????????????????????
        case BSP_EVENT_LED1_ON:
        {
            //?????? LED ????????? D1
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_1));
            printf("EVENT_LED1_ON\r\n");
            
        } break;
        // ??????S1 ??????????????????????????????
        case BSP_EVENT_LED1_OFF:
        {
            //?????? LED ????????? D1
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_1));
            printf("EVENT_LED1_OFF\r\n");
        } break;
        // ??????S2 ??????????????????????????????
        case BSP_EVENT_LED2_ON:
        {
            //?????? LED ????????? D2
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_2));
            printf("EVENT_LED2_ON\r\n");
        } break;
        // ??????S2 ??????????????????????????????
        case BSP_EVENT_LED2_OFF:
        {
            //?????? LED ????????? D2
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_2));
            printf("EVENT_LED2_OFF\r\n");
        } break;
        
        case BSP_EVENT_KEY_0:
        {
            printf("BSP_EVENT_KEY_0 Long push\r\n");
        } break;
        
        case BSP_EVENT_KEY_1:
        {
 
            printf("BSP_EVENT_KEY_1 Long push\r\n");
        } break;
        
        
           

        default:
            break; // No implementation needed
    }
}


void bsp_button_ble_send_test()
{
    ret_code_t err_code = 0;
    
    bsp_board_init(BSP_INIT_LEDS);

    err_code = bsp_event_to_button_action_assign(0, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_LED1_ON ));
    err_code = bsp_event_to_button_action_assign(0, BSP_BUTTON_ACTION_RELEASE, (bsp_event_t)(BSP_EVENT_LED1_OFF ));
    err_code = bsp_event_to_button_action_assign(0, BSP_BUTTON_ACTION_LONG_PUSH, (bsp_event_t)(BSP_EVENT_KEY_0 ));

    err_code = bsp_event_to_button_action_assign(1, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_LED2_ON ));
    err_code = bsp_event_to_button_action_assign(1, BSP_BUTTON_ACTION_RELEASE, (bsp_event_t)(BSP_EVENT_LED2_OFF ));
    err_code = bsp_event_to_button_action_assign(1, BSP_BUTTON_ACTION_LONG_PUSH, (bsp_event_t)(BSP_EVENT_KEY_1 ));

    err_code = bsp_init(BSP_INIT_BUTTONS,bsp_evt_handler);
    
    printf("xincx_kbs_init err_code:0x%x\n",err_code);
    APP_ERROR_CHECK(err_code);
}



static void mtxkey_event_handler(int16_t mtxkey_idx,uint16_t key_val,uint8_t row_pin,uint8_t col_pin,uint8_t button_action);
static xincx_kbs_mtxkey_cfg_t mtxkeys[] =
{
  [2] = {12,KBS_ROW_BUTTON_2, KBS_COL_BUTTON_1, mtxkey_event_handler},//BUTTON_PULLDOWN
  [1] = {10,KBS_ROW_BUTTON_1, KBS_COL_BUTTON_1, mtxkey_event_handler},
  [3] = {11,KBS_ROW_BUTTON_1, KBS_COL_BUTTON_2, mtxkey_event_handler},

  
  [0] = {13,KBS_ROW_BUTTON_2, KBS_COL_BUTTON_2, mtxkey_event_handler},

};
            

static void system_run_timer_handler(btstack_timer_source_t * ts){

	static uint8_t on_off = 0;

	static uint8_t ocpy_ratio = 10;
   // printf("system_run_timer_handler\n");
	ocpy_ratio++;
//	g_sys_time++;
	if(ocpy_ratio >= 99)
	{
		ocpy_ratio = 10;	
	}
	//xc_set_pwm(2,ocpy_ratio, 159);
	if(on_off == 0)
	{
		
		//GPIO_OUTPUT_HIGH(4);
		bsp_board_led_on(0);
		bsp_board_led_off(1);
		on_off = 1;
	//	led_value = xinc_gpio_pin_read(LED1);

		
	}else
	{
	
		
	//	GPIO_OUTPUT_LOW(4);
		bsp_board_led_off(0);
		bsp_board_led_on(1);
		on_off = 0;
	//	printf("LED1 ON\n");
	}	
	 btstack_run_loop_set_timer(ts, 500);
 //  btstack_run_loop_add_timer(ts);

}


int	main(void)
{

    key_init();
    xincx_gpio_init();
    
    xinc_mem_init();   
	
    scheduler_init();
    app_timer_init();
    
    spim_flash_init();
    
    uint32_t mid;
    spim_flash_Read_MID((uint8_t *)&mid);
    
    printf("Read_MID:0x%x\n",mid);
    
    spim_flash_Enter_powerdown();
   
    #if (APP_BLE_FEATURE_ENABLED)
	set_bd_addr();
    ble_init((void *)&blestack_init);
    #else
    SysTick_Config(32000000/100);
    SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;
    #endif
   
    
    #if (APP_BLE_FEATURE_ENABLED)
	btstack_main();
    #endif


    #if (APP_BLE_FEATURE_ENABLED)
    // setup advertisements
    uint16_t adv_int_min = 50;//0x0100;
    uint16_t adv_int_max = 50;//0x0100;

    bd_addr_t null_addr;
    memset(null_addr, 0, 6);
    gap_advertisements_set_params(adv_int_min, adv_int_max, ADV_IND, 0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data(adv_pair_data_len, (uint8_t*) adv_pair_data);
    gap_scan_response_set_data(scanresp_data_len , (uint8_t*) scanresp_data);
    gap_advertisements_enable(1);
    #endif
    
    
    #if (APP_BLE_FEATURE_ENABLED)
    con_flag = 1;
    sys_run_timer.process = &system_run_timer_handler;
	btstack_run_loop_set_timer(&sys_run_timer, 100);
	btstack_run_loop_add_timer(&sys_run_timer);
    #endif

    printf("power_management_init\r\n");

    while(1) {

       #if (APP_BLE_FEATURE_ENABLED)
       ble_mainloop();
       #endif
       app_sched_execute();
	   
	   
       if(LastTimeGulSystickCount!=GulSystickCount)//10ms????????????????
	   {	
           if(LastTimeGulSystickCount % 200 == 0)
           {
               printf("LastTimeGulSystickCount:%d\n",LastTimeGulSystickCount/200);            
           }           

		   LastTimeGulSystickCount=GulSystickCount;
          
			 
	   }

       idle_state_handle();

    }
}

