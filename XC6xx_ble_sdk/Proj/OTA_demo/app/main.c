#include    <stdio.h>
#include    <string.h>
#include	"Includes.h"
#include    "profile.h"
#include    "ble.h"
#include    "platform.h"
#include    "btstack_defines.h"
#include    "xinchip_ota.h"

static uint16_t user_data_client_configuration;

 

void sm_peripheral_setup (void){

}
void _HWradio_Go_To_Idle_State_Patch (void){

}


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
		user_data_client_configuration = le_notification_enabled;
         break;   
    case    ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE:
    case    ATT_CHARACTERISTIC_0000FF12_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE:
        printf("att_handle=0x%x, offset=0x%x, length=0x%x\n", att_handle, offset, buffer_size);
        printf("att data: ");
        for(uint32_t i=0; i<buffer_size; i++) printf("0x%x ", buffer[i]);
        printf("\n");
        break;
#ifdef OTA_TEST	
     case    ATT_CHARACTERISTIC_0000FF01_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE://OTA updata server val	
	     ota_rev_packet(buffer,buffer_size);
	     break;
#endif
    default:
        break;
    }    
    return 0;
} 
static hci_con_handle_t connection_handle;
static volatile uint8_t updata_parameter=0;
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
                //printf("\n CONNECT RIGHT ! (HANDLE = 0x%x)\n", connection_handle);
				g_conn_stat = 1;
//			    gap_request_connection_parameter_update(connection_handle, 0x7, 0x7, 0,0x12);
                //printf("Connected, requesting conn param update for handle 0x%04x\n", connection_handle); 
				gatt_client_read_value_of_characteristics_by_uuid16(packet_handler,connection_handle,0x01,0xff,0x2A00);
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
        //printf("ATT MTU = %u\n", att_event_mtu_exchange_complete_get_MTU(packet));
       if(270==att_event_mtu_exchange_complete_get_MTU(packet))//mtu set 270  ----> OTA 
       {
            updata_parameter=1;
            //gap_request_connection_parameter_update(connection_handle, 0x7, 0x7, 0,0x12);
            //Uart_DMA_Send(0,"requesting conn param update!\n",sizeof("requesting conn param update!\n"));
       }
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
    0x0A, 0x09, 'X', 'I', 'C', '_', 'O', 'T', 'A', '_', '1',  
};
uint8_t adv_data_len = sizeof(adv_data);

const uint8_t scanresp_data[] = {
    //- Manufacture.
    0x0c, 0xff, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', 
};

uint8_t scanresp_data_len = sizeof(scanresp_data);



void stack_reset(void)
{

}

//设置MAC地址
static void set_bd_addr() 
{
 
     extern uint8_t bd_addr[6];
     #if 0 //方式1：手动设置
     bd_addr[0]=0x13;
     bd_addr[1]=0x11;
     bd_addr[2]=0x02;
     bd_addr[3]=0x20;
     bd_addr[4]=0x7a;
     bd_addr[5]=0x91;
     #endif
     #if 0 //方式2：配合机台烧录器或者上位机烧录器将唯一mac烧录到flash的地址第136个字节处
     Init_spi_master(0,SPIM_CLK_16MHZ);
     spi_flash_Release_powerdown();
     spi_flash_Read(136, bd_addr,6);
     spi_flash_Enter_powerdown();
     #endif
     #if 1 //方式3：从flash的ID(16个字节)中选取6个字节作为mac,经与flash厂商沟通选取按照如下规则
     uint8_t ID_buf[16]={0};
     Init_spi_master(0, SPIM_CLK_16MHZ);
     spi_flash_Release_powerdown();
     spi_flash_Read_128bitsID(ID_buf);
     bd_addr[0]= ID_buf[11]; 
     bd_addr[1]= (ID_buf[10]&0x01)|(ID_buf[9]<<1); 
     bd_addr[2]= (ID_buf[9]>>7)|((ID_buf[8]&0x01)<<1)|(ID_buf[7]<<2); 
     bd_addr[3]= (ID_buf[6]&0x0F)|((ID_buf[5]&0x0F)<<4); 
     bd_addr[4]= (ID_buf[4]&0x0F)|((ID_buf[3]&0x0F)<<4); 
     bd_addr[5]= ((ID_buf[3]&0xF0)>>4)|((ID_buf[2]&0xF)<<4);
     spi_flash_Enter_powerdown();
     #endif
}
 
 
enum adv_type{
	ADV_IND,
	ADV_DIRECT_IND,
	ADV_SCAN_IND,
	ADV_NONCONN_IND,
};

int	main(void)
{
    gpio_fun_sel(18,UART0_TX);
    Init_uart(0 ,BAUD_115200);
    Uart_DMA_Send(0,"---OTA TEST---\n",sizeof("---OTA TEST---!\n"));
	set_bd_addr();  
    ble_init((void *)&blestack_init);
	gatt_client_init();
	// setup advertisements
    uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
   
    bd_addr_t null_addr; 
    memset(null_addr, 0, 6);
    gap_advertisements_set_params(adv_int_min, adv_int_max, ADV_IND, 0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data(adv_data_len, (uint8_t*) adv_data);
    gap_scan_response_set_data(scanresp_data_len , (uint8_t*) scanresp_data);
    gap_advertisements_enable(1);
    
    while(1) {
        ble_mainloop();			//31us
#ifdef OTA_TEST	
		ota_fun();
        if(updata_parameter)
        {
           updata_parameter=0;
           gap_request_connection_parameter_update(connection_handle, 0x7, 0x7, 0,0x12);
           Uart_DMA_Send(0,"requesting conn param update!\n",sizeof("requesting conn param update!\n"));
        }
#endif

     }
}
