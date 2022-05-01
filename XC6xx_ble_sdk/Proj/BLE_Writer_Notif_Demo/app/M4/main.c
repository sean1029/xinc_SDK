
#include    "profile.h"
#include    "ble.h"
#include    "bsp_spi_master.h"
#include    "sys_config.h"
//#include "DMA_uart_debug.h"
#include    "btstack_defines.h"
#include <stdlib.h>
#include "rom_variable.h"


#include    <stdio.h>
#include    <string.h>
#include	"Includes.h"

#define APP_BLE_FEATURE_ENABLED  XINCX_CHECK(XINC_BLE_STACK_ENABLED) 

#include "bsp_gpio.h"
#include "sbc.h"

#include "bsp_timer.h"
#include "bsp_pwm.h"
#include    "bsp_spi_flash.h"

#include "xinc_gpio.h"
#include "bsp_uart.h"

#include "xincx_gpio.h"
#include "xinc_drv_spi.h"

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


extern uint32_t  GulSystickCount;
uint32_t  LastTimeGulSystickCount=0xFF;


static uint16_t user_data_client_configuration;
static btstack_context_callback_registration_t  user_data_callback;
static hci_con_handle_t user_data_client_configuration_connection;
static void user_start(void);

uint8_t u_buff[49]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
					21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
					41,42,43,44,45,46,47,48,49,};
uint8_t *p_buff = NULL;
uint16_t u_buff_length=0;

static uint32_t min(uint32_t a, uint32_t b){
    return a < b ? a : b;
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
uint8_t read_test[2] = {0x03,0x00};
// read requests
static uint16_t app_att_read_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size){

    printf("%s, con_handle=%x, att_handle=%x, offset=%x, buffer=%x, size=%x\n",__func__, con_handle, att_handle, offset,(uint32_t)buffer, buffer_size);
    
//    if((att_handle != ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE) && 
//       (att_handle != ATT_CHARACTERISTIC_0000FF12_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE)) return 0;
    

    return att_read_callback_handle_blob((const uint8_t *)read_test, sizeof(read_test), offset, buffer, buffer_size);
}

// write requests
static int app_att_write_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size){
    
    uint32_t le_notification_enabled;
    printf("%s, con_handle=%x, att_handle=%x, offset=%x, buffer=%x, size=%x\n", __func__, con_handle, att_handle, offset, (uint32_t)buffer, buffer_size);  
    if (transaction_mode != ATT_TRANSACTION_MODE_NONE) return 0;
    switch(att_handle)
    {
    case    ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_CLIENT_CONFIGURATION_HANDLE:
        le_notification_enabled = little_endian_read_16(buffer, 0) == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION;
        printf("Notifications enabled %u\n",le_notification_enabled); 
		user_data_client_configuration = le_notification_enabled;
		user_data_client_configuration_connection = con_handle;
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
uint8_t con_state = 0;
uint32_t data_count=0;
static void app_packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    if (packet_type != HCI_EVENT_PACKET) return;
    switch(hci_event_packet_get_type(packet))
    {
    case BTSTACK_EVENT_STATE:
        if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) return;
        printf("BTstack up and running.\n");
#ifdef ENABLE_LE_CENTRAL	
		gap_set_scan_parameters(0,0x0030, 0x0030);
		gap_start_scan();
#endif	
        break;
        
    case HCI_EVENT_LE_META:
        switch (hci_event_le_meta_get_subevent_code(packet)) 
        {
        case HCI_SUBEVENT_LE_CONNECTION_COMPLETE: {
                hci_con_handle_t connection_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
                printf("\n CONNECT RIGHT ! (HANDLE = 0x%x)\n", connection_handle);
				user_start();
                //gap_request_connection_parameter_update(connection_handle, 12, 12, 0, 0x0048);
                //printf("Connected, requesting conn param update for handle 0x%04x\n", connection_handle); 
            }              
            break;               
        default:
            break;
        }
        break;
            
    case HCI_EVENT_DISCONNECTION_COMPLETE:
        printf("\n DISCONNECT ! (HANDLE = 0x%x) ", hci_event_disconnection_complete_get_connection_handle(packet)); 
        printf("REASON = 0x%x\n", hci_event_disconnection_complete_get_reason(packet));
		data_count=0;
        break;
    
    case L2CAP_EVENT_CONNECTION_PARAMETER_UPDATE_RESPONSE:   
        printf("L2CAP Connection Parameter Update Complete, response: %x\n", l2cap_event_connection_parameter_update_response_get_result(packet));
        break;

    case ATT_EVENT_MTU_EXCHANGE_COMPLETE:
        printf("ATT MTU = %u\n", att_event_mtu_exchange_complete_get_MTU(packet));
        break;
    
    case ATT_EVENT_HANDLE_VALUE_INDICATION_COMPLETE:
        (att_event_handle_value_indication_complete_get_status(packet) == 0) ? printf("INDICATION OK\n") : printf("INDICATION FAIL\n");
        break;
    default:
        break;
    }
}

static const blestack_init_t blestack_init = {
    
    /* uint16_t (*att_read_callback)(...);*/ app_att_read_callback,
    /* int (*att_write_callback)(...);    */ app_att_write_callback,
    /* void (*packet_handler)(...)        */ app_packet_handler,
    /* const uint8_t *ble_device_db       */ profile_data
};

const uint8_t adv_data[] = {
    // Flags general discoverable, BR/EDR not supported
    0x02, 0x01, 0x06, 
    // Name
    
	0x0d, 0x09, 'M', 'a', 's', 't', 'e', 'r', '&', 'S', 'l','a','v','e', 
};
uint8_t adv_data_len = sizeof(adv_data);

const uint8_t scanresp_data[] = {
    //- Manufacture.
    0x0c, 0xff, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1',  
};
uint8_t scanresp_data_len = sizeof(scanresp_data);

static void set_bd_addr()          
{ 
    extern uint8_t bd_addr[6];    
    bd_addr[0]=0xa4;   
    bd_addr[1]=0x3a;   
    bd_addr[2]=0x22;    
    bd_addr[3]=0x20;   
    bd_addr[4]=0x9a;    
    bd_addr[5]=0x98;
}

void key_init(void)
{
	Init_gpio();

}
void stack_reset(void)
{
	hci_power_control(1);
	if(g_le_enable_observer_flag == 1)
	{
		master_init();
	}
    uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
    uint8_t adv_type = 0;
    bd_addr_t null_addr;
    memset(null_addr, 0, 6);
    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data(adv_data_len, (uint8_t*) adv_data);
    gap_scan_response_set_data(scanresp_data_len , (uint8_t*) scanresp_data);
    gap_advertisements_enable(1);
}


//typedef struct btstack_timer_source {
//    btstack_linked_item_t item; 
//    // timeout in system ticks (HAVE_EMBEDDED_TICK) or milliseconds (HAVE_EMBEDDED_TIME_MS)
//    uint32_t timeout;
//    // will be called when timer fired
//    void  (*process)(struct btstack_timer_source *ts); 
//    void * context;
//} btstack_timer_source_t;

static  btstack_timer_source_t user_timer;

static void data_can_send_now(void * context){
	
	 hci_con_handle_t con_handle = (hci_con_handle_t) (int) context;     
	 att_server_notify(con_handle, ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE, p_buff, u_buff_length);

}

void data_sent(uint8_t *buffer,uint16_t length)//用户数据发送  ble->手机
{
	p_buff = buffer;
	u_buff_length = length;
	if (user_data_client_configuration){
		user_data_callback.callback = &data_can_send_now;
		user_data_callback.context  = (void*) (int) user_data_client_configuration_connection;
		att_server_register_can_send_now_callback(&user_data_callback, user_data_client_configuration_connection);
	}
}

static void le_data_up(btstack_timer_source_t * ts)
{
//	printf("data_number:%d\n",data_count++);
	data_sent(u_buff,30);
    btstack_run_loop_set_timer(ts, 50);
    btstack_run_loop_add_timer(ts);
}

//上行数据
static void user_start(void)//客户可以在这里添加自己的应用
{
    user_timer.process = &le_data_up;
    btstack_run_loop_set_timer(&user_timer, 4000); //获得属性后，开始发送数据
    btstack_run_loop_add_timer(&user_timer);
}


extern int btstack_main(void);

void rom_variable_init();

#define SCHED_MAX_EVENT_DATA_SIZE           APP_TIMER_SCHED_EVENT_DATA_SIZE             //!< Maximum size of the scheduler event data.
#define SCHED_QUEUE_SIZE                    10                                          //!< Size of the scheduler queue.

static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}

static btstack_timer_source_t sys_run_timer;

static void system_run_timer_handler(btstack_timer_source_t * ts){

    static uint32_t num = 0;
    printf("system_run_timer num:%d\r\n",num++);
	 btstack_run_loop_set_timer(ts, 500);
    btstack_run_loop_add_timer(&sys_run_timer);

}


int	main(void)
{

    rom_variable_init();//not remove !!!!!
    key_init();
    xincx_gpio_init();
    
    xinc_mem_init();   
	
    scheduler_init();
    app_timer_init();
    
    
    dump_flag = 1;
      
    set_bd_addr();
    ble_init((void *)&blestack_init);
	  
	btstack_main();
	
	if(g_le_enable_observer_flag == 1)
	{
	//	master_init();
	}
    // setup advertisements
    uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
    uint8_t adv_type = 0;
    bd_addr_t null_addr;
    memset(null_addr, 0, 6);
    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data(adv_data_len, (uint8_t*) adv_data);
    gap_scan_response_set_data(scanresp_data_len , (uint8_t*) scanresp_data);
    gap_advertisements_enable(1);

	if(2 == g_debug_le_sleep)
	{
	//	ble_system_idle_init();
	}
    
    sys_run_timer.process = &system_run_timer_handler;
	btstack_run_loop_set_timer(&sys_run_timer, 100);
	btstack_run_loop_add_timer(&sys_run_timer);
	  


    while(1) 
    {
        ble_mainloop(); 
        if (2 == g_debug_le_sleep)
        {
         //   ble_system_idle();
        }       
				
       if(LastTimeGulSystickCount!=GulSystickCount)//10ms???h??
	   {	
           if(LastTimeGulSystickCount % 200 == 0)
           {
               printf("LastTimeGulSystickCount:%d\n",LastTimeGulSystickCount/200);            
           }           

		   LastTimeGulSystickCount=GulSystickCount;
          
			 
	   }

     }
}




