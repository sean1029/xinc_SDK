#include    <stdio.h>
#include    <string.h>
#include 	"btstack.h"

static btstack_packet_callback_registration_t hci_event_callback_registration;
static void handle_gatt_client_event(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

static gatt_client_service_t le_streamer_service;
static gatt_client_characteristic_t le_streamer_characteristic_rx;
static gatt_client_characteristic_t le_streamer_characteristic_tx;

static gatt_client_notification_t notification_listener;
static int listener_registered;
static hci_con_handle_t connection_handle;

static bd_addr_t      le_streamer_addr;
static bd_addr_type_t le_streamer_addr_type;

uint8_t notify_buff[50];

typedef enum {
    TC_OFF,
    TC_IDLE,
    TC_W4_SCAN_RESULT,
    TC_W4_CONNECT,
    TC_W4_SERVICE_RESULT,
    TC_W4_CHARACTERISTIC_RX_RESULT,
    TC_W4_CHARACTERISTIC_TX_RESULT,
    TC_W4_ENABLE_NOTIFICATIONS_COMPLETE,
    TC_W4_TEST_DATA
} gc_state_t;

typedef struct {
    char name;
    int le_notification_enabled;
} le_streamer_connection_t;

static le_streamer_connection_t le_streamer_connection;

static gc_state_t state = TC_OFF;

// On the GATT Server, RX Characteristic is used for receive data via Write, and TX Characteristic is used to send data via Notifications
static uint8_t le_streamer_service_uuid[16]           = { 0x00, 0x00, 0xFF, 0x10, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};
static uint8_t le_streamer_characteristic_rx_uuid[16] = { 0x00, 0x00, 0xFF, 0x12, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};
static uint8_t le_streamer_characteristic_tx_uuid[16] = { 0x00, 0x00, 0xFF, 0x12, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB};
uint8_t uart_buff[50];


static const char * ad_types[] = {
    "", 
    "Flags",
    "Incomplete List of 16-bit Service Class UUIDs",
    "Complete List of 16-bit Service Class UUIDs",
    "Incomplete List of 32-bit Service Class UUIDs",
    "Complete List of 32-bit Service Class UUIDs",
    "Incomplete List of 128-bit Service Class UUIDs",
    "Complete List of 128-bit Service Class UUIDs",
    "Shortened Local Name",
    "Complete Local Name",
    "Tx Power Level",
    "", 
    "", 
    "Class of Device",
    "Simple Pairing Hash C",
    "Simple Pairing Randomizer R",
    "Device ID",
    "Security Manager TK Value",
    "Slave Connection Interval Range",
    "",
    "List of 16-bit Service Solicitation UUIDs",
    "List of 128-bit Service Solicitation UUIDs",
    "Service Data",
    "Public Target Address",
    "Random Target Address",
    "Appearance",
    "Advertising Interval"
};

static const char * flags[] = {
    "LE Limited Discoverable Mode",
    "LE General Discoverable Mode",
    "BR/EDR Not Supported",
    "Simultaneous LE and BR/EDR to Same Device Capable (Controller)",
    "Simultaneous LE and BR/EDR to Same Device Capable (Host)",
    "Reserved",
    "Reserved",
    "Reserved"
};


int notify_count = 0;
static void handle_gatt_client_event(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    UNUSED(packet_type);
    UNUSED(channel);
    UNUSED(size);

    uint16_t mtu;
    switch(state){
        case TC_W4_SERVICE_RESULT:
            switch(hci_event_packet_get_type(packet)){
                case GATT_EVENT_SERVICE_QUERY_RESULT:
                    // store service (we expect only one)
                    gatt_event_service_query_result_get_service(packet, &le_streamer_service);
                    break;
                case GATT_EVENT_QUERY_COMPLETE:
                    if (packet[4] != 0){
                        printf("SERVICE_QUERY_RESULT - Error status %x.\n", packet[4]);
                        gap_disconnect(connection_handle);
                        break;  
                    } 
                    // service query complete, look for characteristic
                    state = TC_W4_CHARACTERISTIC_RX_RESULT;
                    printf("Search for LE Streamer RX characteristic.\n");
                    gatt_client_discover_characteristics_for_service_by_uuid128(handle_gatt_client_event, connection_handle, &le_streamer_service, le_streamer_characteristic_rx_uuid);
                    break;
                default:
                    break;
            }
            break;
            
        case TC_W4_CHARACTERISTIC_RX_RESULT:
            switch(hci_event_packet_get_type(packet)){
                case GATT_EVENT_CHARACTERISTIC_QUERY_RESULT:
                    gatt_event_characteristic_query_result_get_characteristic(packet, &le_streamer_characteristic_rx);
                    break;
                case GATT_EVENT_QUERY_COMPLETE:
                    if (packet[4] != 0){
                        printf("CHARACTERISTIC_QUERY_RESULT - Error status %x.\n", packet[4]);
                        gap_disconnect(connection_handle);
                        break;  
                    } 
                    // rx characteristiic found, look for tx characteristic
                    state = TC_W4_CHARACTERISTIC_TX_RESULT;
                    printf("Search for LE Streamer TX characteristic.\n");
                    gatt_client_discover_characteristics_for_service_by_uuid128(handle_gatt_client_event, connection_handle, &le_streamer_service, le_streamer_characteristic_tx_uuid);
                    break;
                default:
                    break;
            }
            break;

        case TC_W4_CHARACTERISTIC_TX_RESULT:
            switch(hci_event_packet_get_type(packet)){
                case GATT_EVENT_CHARACTERISTIC_QUERY_RESULT:
                    gatt_event_characteristic_query_result_get_characteristic(packet, &le_streamer_characteristic_tx);
                    break;
                case GATT_EVENT_QUERY_COMPLETE:
                    if (packet[4] != 0){
                        printf("CHARACTERISTIC_QUERY_RESULT - Error status %x.\n", packet[4]);
                        gap_disconnect(connection_handle);
                        break;  
                    } 
                    // register handler for notifications
                    listener_registered = 1;
                    gatt_client_listen_for_characteristic_value_updates(&notification_listener, handle_gatt_client_event, connection_handle, &le_streamer_characteristic_tx);
                    // setup tracking
                   
                    gatt_client_get_mtu(connection_handle, &mtu);                   
                    printf("ATT MTU = %u \n", mtu);
					printf("Enable notify on test characteristic.\n");
                    // enable notifications
                    printf("Start streaming - enable notify on test characteristic.\n");
                    state = TC_W4_ENABLE_NOTIFICATIONS_COMPLETE;
                    gatt_client_write_client_characteristic_configuration(handle_gatt_client_event, connection_handle,
                        &le_streamer_characteristic_tx, GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION);
                    break;
                    state = TC_W4_TEST_DATA;
                    break;
                default:
                    break;
            }
            break;

        case TC_W4_ENABLE_NOTIFICATIONS_COMPLETE:
            switch(hci_event_packet_get_type(packet)){
                case GATT_EVENT_QUERY_COMPLETE:
                    printf("Notifications enabled, ATT status %02x\n", gatt_event_query_complete_get_att_status(packet));
                    if (gatt_event_query_complete_get_att_status(packet) != 0) break;
                    state = TC_W4_TEST_DATA;
                    break;
                default:
                    break;
            }
            break;

        case TC_W4_TEST_DATA:
            switch(hci_event_packet_get_type(packet)){
                case GATT_EVENT_NOTIFICATION:
                    printf("Notify Packet Value from slave:%d\n",gatt_event_notification_get_value(packet));
					for(int i=0;i<size;i++)
					{
						notify_buff[i] = packet[i+8];
						printf("%x",notify_buff[i]);
					}
					printf("\r\n");
//					Uart_Send_Buf(1,notify_buff,size-8);
					printf("notify recive number:%d\n",notify_count++);
					
                    break;
                case GATT_EVENT_QUERY_COMPLETE:
                    break;
                case GATT_EVENT_CAN_WRITE_WITHOUT_RESPONSE:
					 gatt_client_write_value_of_characteristic(handle_gatt_client_event,connection_handle,0x0009,4,"1234");
                    break;
                default:
                    printf("Unknown packet type %x\n", hci_event_packet_get_type(packet));
                    break;
            }
            break;

        default:
            printf("error\n");
            break;
    }
    
}

static void hci_event_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    UNUSED(channel);
    UNUSED(size);

    if (packet_type != HCI_EVENT_PACKET) return;
    
    uint16_t conn_interval;
    uint8_t event = hci_event_packet_get_type(packet);
    switch (event) {
        case BTSTACK_EVENT_STATE:
            // BTstack activated, get started
            if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
			printf("Start scanning!\n");
            } else {
                state = TC_OFF;
            }
            break;
        case GAP_EVENT_ADVERTISING_REPORT:{
            gap_event_advertising_report_get_address(packet, le_streamer_addr);
            uint8_t event_type = gap_event_advertising_report_get_advertising_event_type(packet);
            uint8_t le_streamer_addr_type = gap_event_advertising_report_get_address_type(packet);
            int8_t rssi = gap_event_advertising_report_get_rssi(packet);
            uint8_t length = gap_event_advertising_report_get_data_length(packet);
            const uint8_t * data = gap_event_advertising_report_get_data(packet);
            
//            printf("Advertisement event: evt-type %u, addr-type %u, addr %s, rssi %d, data[%u] ", event_type,
//               le_streamer_addr_type, bd_addr_to_str(le_streamer_addr), rssi, length);
//			
//            printf_hexdump(data, length);
			if (!ad_data_contains_uuid16(length, (uint8_t *) data, 0x1800)) break;
            printf("Found remote with UUID %04x, connecting...\n", 0x1800);
#ifdef ENABLE_LE_CENTRAL
			gap_stop_scan();
#endif
            // store address and type
            gap_event_advertising_report_get_address(packet, le_streamer_addr);
            // stop scanning, and connect to the device
            state = TC_W4_CONNECT;

            printf("Stop scan. Connect to device with addr %s.\n", bd_addr_to_str(le_streamer_addr));
#ifdef ENABLE_LE_CENTRAL
            gap_connect(le_streamer_addr,le_streamer_addr_type);
#endif
            break;
		}
        case HCI_EVENT_LE_META:
            // wait for connection complete
            if (hci_event_le_meta_get_subevent_code(packet) !=  HCI_SUBEVENT_LE_CONNECTION_COMPLETE) break;
            if (state != TC_W4_CONNECT) return;
            connection_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
            // print connection parameters (without using float operations)
            conn_interval = hci_subevent_le_connection_complete_get_conn_interval(packet);
            printf("Connection Interval: %u.%02u ms\n", conn_interval * 125 / 100, 25 * (conn_interval & 3));
            printf("Connection Latency: %u\n", hci_subevent_le_connection_complete_get_conn_latency(packet));  
            // initialize gatt client context with handle, and add it to the list of active clients
            // query primary services
            printf("Search for LE Streamer service.\n");
            state = TC_W4_SERVICE_RESULT;
            gatt_client_discover_primary_services_by_uuid128(handle_gatt_client_event, connection_handle, le_streamer_service_uuid);
            break;
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            // unregister listener
          connection_handle = HCI_CON_HANDLE_INVALID;
            if (listener_registered){
                listener_registered = 0;
                gatt_client_stop_listening_for_characteristic_value_updates(&notification_listener);
            }
            printf("Disconnected %s\n", bd_addr_to_str(le_streamer_addr));
			printf("Start scanning!\n");
#ifdef ENABLE_LE_CENTRAL            
			gap_set_scan_parameters(0,0x0030, 0x0030);
			gap_start_scan();
#endif            
            break;
        default:
            break;
    }
}

void master_init(void)
{
//    l2cap_init();

//    sm_init();
//    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);

    // sm_init needed before gatt_client_init
    gatt_client_init();

    hci_event_callback_registration.callback = &hci_event_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // use different connection parameters: conn interval min/max (* 1.25 ms), slave latency, supervision timeout, CE len min/max (* 0.6125 ms) 
    // gap_set_connection_parameters(0x06, 0x06, 4, 1000, 0x01, 0x06 * 2);

    // turn on!
//    hci_power_control(1);	
}
