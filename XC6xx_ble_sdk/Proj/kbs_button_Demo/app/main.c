#include    <stdio.h>
#include    <string.h>
#include "btstack_run_loop.h"
#include	"Includes.h"
#include    "profile.h"
#include    "ble.h"
#include "hids_device.h"
#include "bsp_gpio.h"
#include "sbc.h"
#include "voice_ringbuf.h"
#include "bsp_timer.h"
#include "bsp_pwm.h"
#include    "bsp_spi_flash.h"
#include "xinc_gpio.h"
#include "bsp_uart.h"

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
uint8_t flag_show_hci = 0;


void _HWradio_Go_To_Idle_State_Patch(void){
}
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
extern  int btstack_main(void);
extern 	void send_media_report(int mediacode1,int mediacode2);

static uint32_t min(uint32_t a, uint32_t b){
    return a < b ? a : b;
}

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
static uint16_t att_read_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size){

    printf("main %s, con_handle=%x, att_handle=%x, offset=%x, buffer=%x, size=%x\n",__func__, con_handle, att_handle, offset,(uint32_t)buffer, buffer_size);
    
    if((att_handle != ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE) && 
       (att_handle != ATT_CHARACTERISTIC_0000FF12_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE)) return 0;
    
    static uint8_t read_buf[10] = {0};
    return att_read_callback_handle_blob((const uint8_t *)read_buf, sizeof(read_buf), offset, buffer, buffer_size);
}

// write requests
static int att_write_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size){
    
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

static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
	
    if (packet_type != HCI_EVENT_PACKET) return;
    switch(hci_event_packet_get_type(packet))
    {
    case BTSTACK_EVENT_STATE:
        if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) return;
        printf("BTstack up and running.\n");
        break;
        
    case HCI_EVENT_LE_META:
        switch (hci_event_le_meta_get_subevent_code(packet)) 
        {
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
    
    /* uint16_t (*att_read_callback)(...);*/ att_read_callback,
    /* int (*att_write_callback)(...);    */ att_write_callback,
    /* void (*packet_handler)(...)        */ packet_handler,
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


void send_power_on_adv(void)
{
	flag_show_hci =1;
	printf("send_power_on_adv\n");
	uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
	 bd_addr_t null_addr;
    memset(null_addr, 0, 6);
	gap_advertisements_enable(0);
	 gap_advertisements_set_params(adv_int_min, adv_int_max, ADV_IND, 0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data(adv_poweron_data_len, (uint8_t*) adv_powerOn_data);
    gap_scan_response_set_data(scanresp_data_len , (uint8_t*) scanresp_data);
    gap_advertisements_enable(1);
}

void stack_reset(void)
{
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
}

extern uint8_t con_flag;

static btstack_timer_source_t sys_run_timer;

#include "xc_kbs_event.h"
#include "le_device_db.h"



#define SCHED_MAX_EVENT_DATA_SIZE           APP_TIMER_SCHED_EVENT_DATA_SIZE             //!< Maximum size of the scheduler event data.
#define SCHED_QUEUE_SIZE                    10                                          //!< Size of the scheduler queue.

static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}



/**@brief Function for handling bsp events.
 */
void bsp_evt_handler(bsp_event_t evt)
{
    switch (evt)
    {
        // 按键S1 注册的按下事件的回调
        case BSP_EVENT_LED1_ON:
        {
            //点亮 LED 指示灯 D1
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_1));
            printf("EVENT_LED1_ON\r\n");
            
        } break;
        // 按键S1 注册的释放事件的回调
        case BSP_EVENT_LED1_OFF:
        {
            //熄灭 LED 指示灯 D1
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_1));
            printf("EVENT_LED1_OFF\r\n");
        } break;
        // 按键S2 注册的长按事件的回调
        case BSP_EVENT_LED2_ON:
        {
            //点亮 LED 指示灯 D2
            bsp_board_led_on(bsp_board_pin_to_led_idx(LED_2));
            printf("EVENT_LED2_ON\r\n");
        } break;
        // 按键S2 注册的释放事件的回调
        case BSP_EVENT_LED2_OFF:
        {
            //熄灭 LED 指示灯 D2
            bsp_board_led_off(bsp_board_pin_to_led_idx(LED_2));
            printf("EVENT_LED2_OFF\r\n");
        } break;
        
        case BSP_EVENT_KEY_0:
        {
            printf("BSP_EVENT_KEY_0\r\n");
        } break;
        
        case BSP_EVENT_KEY_1:
        {
 
            printf("BSP_EVENT_KEY_1\r\n");
        } break;
        
        
           

        default:
            break; // No implementation needed
    }
}


void kbs_mtxkey_bsp_test()
{
    ret_code_t err_code = 0;
    
    bsp_board_init(BSP_INIT_LEDS);
    
    err_code = bsp_event_to_mtxkey_action_assign(2, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_LED1_ON ));
    err_code = bsp_event_to_mtxkey_action_assign(2, BSP_BUTTON_ACTION_RELEASE, (bsp_event_t)(BSP_EVENT_LED1_OFF ));
    
        err_code = bsp_event_to_mtxkey_action_assign(3, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_LED2_ON ));
    err_code = bsp_event_to_mtxkey_action_assign(3, BSP_BUTTON_ACTION_RELEASE, (bsp_event_t)(BSP_EVENT_LED2_OFF ));
    
    err_code = bsp_event_to_mtxkey_action_assign(1, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_KEY_1 ));
    err_code = bsp_event_to_mtxkey_action_assign(0, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_KEY_0 ));

        err_code = bsp_event_to_button_action_assign(0, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)(BSP_EVENT_LED1_ON ));
    err_code = bsp_event_to_button_action_assign(0, BSP_BUTTON_ACTION_RELEASE, (bsp_event_t)(BSP_EVENT_LED1_OFF ));
//    
    err_code = bsp_init(BSP_INIT_MTXKEY,bsp_evt_handler);
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
            
static void mtxkey_event_handler(int16_t mtxkey_idx,uint16_t key_val,uint8_t row_pin,uint8_t col_pin,uint8_t button_action)
{

    switch (button_action)
    {
        case KBS_MTXKEY_PUSH:
        {     
            if(mtxkey_idx == 2)
            {
                bsp_board_led_on(bsp_board_pin_to_led_idx(LED_1));
                bsp_board_led_off(bsp_board_pin_to_led_idx(LED_2));
            }
            
        }break;

        case KBS_MTXKEY_RELEASE:
        {     
            if(mtxkey_idx == 2)
            {
                bsp_board_led_on(bsp_board_pin_to_led_idx(LED_2));
                bsp_board_led_off(bsp_board_pin_to_led_idx(LED_1));
            }
        }break;

        case KBS_MTXKEY_LONG_PUSH:
        {     
            if(mtxkey_idx == 2)
            {
                bsp_board_led_on(bsp_board_pin_to_led_idx(LED_2));
                bsp_board_led_on(bsp_board_pin_to_led_idx(LED_1));
            }
            
        }break;

    }
  
}

void kbs_mtxkey_drv_test()
{
    ret_code_t err_code = 0;
    
    bsp_board_init(BSP_INIT_LEDS);
    
    err_code = xincx_kbs_init(mtxkeys, ARRAY_SIZE(mtxkeys), NULL); 

  
    printf("xincx_kbs_init err_code:0x%x\n",err_code);
    APP_ERROR_CHECK(err_code);
}


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


	set_bd_addr();

    ble_init((void *)&blestack_init);
	key_init();
    scheduler_init();
    app_timer_init();
    xincx_gpio_init();
	btstack_main();


    // setup advertisements
    uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;

    bd_addr_t null_addr;
    memset(null_addr, 0, 6);
    gap_advertisements_set_params(adv_int_min, adv_int_max, ADV_IND, 0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data(adv_pair_data_len, (uint8_t*) adv_pair_data);
    gap_scan_response_set_data(scanresp_data_len , (uint8_t*) scanresp_data);
    gap_advertisements_enable(1);
    ble_system_idle_init();
	con_flag = 1;
	printf("sbc_init_msbc\n");
    
    	sys_run_timer.process = &system_run_timer_handler;
	btstack_run_loop_set_timer(&sys_run_timer, 100);
	btstack_run_loop_add_timer(&sys_run_timer);
  //  bsp_board_init(BSP_INIT_LEDS);
    
   // kbs_mtxkey_drv_test();
    kbs_mtxkey_bsp_test();
    while(1) {

       ble_mainloop();
       app_sched_execute();
			
	   ble_system_idle();
       if(LastTimeGulSystickCount!=GulSystickCount)//10msִ��һ��
	   {		   

		   LastTimeGulSystickCount=GulSystickCount;
           if(LastTimeGulSystickCount % 600 == 0)
           {
             //  printf("LastTimeGulSystickCount:%d\n",LastTimeGulSystickCount/600);
           }
			 
	   }		   


    }
}


