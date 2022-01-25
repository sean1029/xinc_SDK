#include    <stdio.h>
#include    <string.h>
#include	"Includes.h"
#include    "profile.h"
#include    "ble.h"
#include    "platform.h"
#include    "ibeacon.h"

void sm_peripheral_setup (void){

}
void _HWradio_Go_To_Idle_State_Patch (void){

}

#define USER_TASK_MS    2000 //2000ms

static uint16_t task_count=0;
typedef struct btstack_linked_item {
    struct btstack_linked_item *next; // <-- next element in list, or NULL
} btstack_linked_item_t;

typedef struct btstack_timer_source {
    btstack_linked_item_t item; 
    // timeout in system ticks (HAVE_EMBEDDED_TICK) or milliseconds (HAVE_EMBEDDED_TIME_MS)
    uint32_t timeout;
    // will be called when timer fired
    void  (*process)(struct btstack_timer_source *ts); 
    void * context;
} btstack_timer_source_t;


extern  void btstack_run_loop_set_timer(btstack_timer_source_t *a, uint32_t timeout_in_ms);
extern  void btstack_run_loop_add_timer(btstack_timer_source_t *ts);
static  void user_task_fun(struct btstack_timer_source *ts);
static  btstack_timer_source_t user_timer;
static  void user_start(void);


static uint32_t min(uint32_t a, uint32_t b){
    return a < b ? a : b;
}

static int g_conn_stat = 0;
int app_get_connect_state(void)
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

// read requests
static uint16_t att_read_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size){

    printf("%s, con_handle=%x, att_handle=%x, offset=%x, buffer=%x, size=%x\n",__func__, con_handle, att_handle, offset,(uint32_t)buffer, buffer_size);
    
    if((att_handle != ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE) && 
       (att_handle != ATT_CHARACTERISTIC_0000FF12_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE)) return 0;
    
    static uint8_t read_test[10] = "012345678";
    return att_read_callback_handle_blob((const uint8_t *)read_test, sizeof(read_test), offset, buffer, buffer_size);
}

// write requests
static int att_write_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size){
    
    uint32_t le_notification_enabled;
    printf("%s, con_handle=%x, att_handle=%x, offset=%x, buffer=%x, size=%x\n", __func__, con_handle, att_handle, offset, (uint32_t)buffer, buffer_size);  
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
                hci_con_handle_t connection_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
                printf("\n CONNECT RIGHT ! (HANDLE = 0x%x)\n", connection_handle);
				g_conn_stat = 1;
				//ap_request_connection_parameter_update(connection_handle, 0x7, 0x7, 0,0x12);
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
		g_conn_stat = 0;
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
    
    /* uint16_t (*att_read_callback)(...);*/ att_read_callback,
    /* int (*att_write_callback)(...);    */ att_write_callback,
    /* void (*packet_handler)(...)        */ packet_handler,
    /* const uint8_t *ble_device_db       */ profile_data
};

const uint8_t adv_data[] = {
    // Flags general discoverable, BR/EDR not supported
    0x02, 0x01, 0x06, 
    // Name
    0x0A, 0x09, 'X', 'i', 'c', '_', 'B', 'L', 'E', 'v', '3',  
};
uint8_t adv_data_len = sizeof(adv_data);

const uint8_t scanresp_data[] = {
    //- Manufacture.
    0x0c, 0xff, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', 
};

uint8_t scanresp_data_len = sizeof(scanresp_data);

void stack_reset(void)
{
	uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
    uint8_t adv_type = 0;
    bd_addr_t null_addr; 
    memset(null_addr, 0, 6);
    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data(adv_data_len, (uint8_t*) adv_data);
    gap_scan_response_set_data(scanresp_data_len , (uint8_t*) scanresp_data);
	//at_beacon((uint8_t*)_ibeacon,sizeof(_ibeacon));
    gap_advertisements_enable(1);	
}
//修改广播的地址
static void set_bd_addr()          
{ 
	extern uint8_t bd_addr[6];
    
    bd_addr[0]=0xa5;
    bd_addr[1]=0x15;
    bd_addr[2]=0x05;
    bd_addr[3]=0x25;
    bd_addr[4]=0x75;
    bd_addr[5]=0x15;
}
static void open_ibeacon(btstack_timer_source_t * ts)
{
	#if 1 //方式1
	
	extern uint32_t LEadv_Set_Advertising_Data(uint8_t length,uint8_t* p_pdu);
    LEadv_Set_Advertising_Data(sizeof(ibeacon_t)+3, (uint8_t*)&ibeacon);
	#endif
	
	
	#if 0 //方式2
	char _ibeacon[]="2768,9BA1,C5,AB8190D5D11E4941ACC442F30510B408";
    set_beacon((uint8_t*)_ibeacon,sizeof(_ibeacon));
	#endif
}
//上行数据
static void user_start(void)//客户可以在这里添加自己的应用
{
    user_timer.process = &open_ibeacon;
    btstack_run_loop_set_timer(&user_timer, 500); //500ms后打开ibeacon
    btstack_run_loop_add_timer(&user_timer);
}


int	main(void)
{   
	set_bd_addr();
    ble_init((void *)&blestack_init);
	// setup advertisements
    uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
    uint8_t adv_type = 0;
    bd_addr_t null_addr; 
    memset(null_addr, 0, 6);
    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data(adv_data_len, (uint8_t*) adv_data);
    gap_scan_response_set_data(scanresp_data_len , (uint8_t*) scanresp_data);
	//at_beacon((uint8_t*)_ibeacon,sizeof(_ibeacon));
    gap_advertisements_enable(1);
    user_start();
    while(1) {
	   
       ble_mainloop();			//31us
    }
}
