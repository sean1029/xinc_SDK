#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "btstack.h"

#include "FF10_service_server.h"
#include "battery_service_server.h"
#include "device_information_service_server.h"
#include "hids_device_skyworth.h"
#include "xc_kbs_event.h"
#include "voice_ringbuf.h"

#define  HID_WITH_VOICE_RC


#define HIDS_REPORT_ID_1     	1
#define HIDS_REPORT_ID_2     	2
#define HIDS_REPORT_ID_43     	43
#define HIDS_REPORT_ID_93     	93
#define HIDS_REPORT_ID_94     	94
#define HIDS_REPORT_ID_95     	95




#define RMC_WITH_VOICE				1
#define MULTIMEDIA_KEYBOARD			1
#define RMC_WITH_SENSORS_DATA		1

const uint8_t hid_descriptor_keyboard_boot_mode[] =
{
    0x06,	0x00,	0xFF, //Usage Page (Vendor-defined 0xFF00)	06 00 FF
		0x09,	0x00, 			//Usage (Vendor-defined 0x0000)	09 00
		0xA1,	0x01,				//Collection (Application)	A1 01
		0xA1,	0x02,
		0x85,	HIDS_REPORT_ID_95,				//        Report Id (95)	85 5F
		0x09,	0x00,				//        Usage (Vendor-defined 0x0000)	09 00
		0x15,	0x80,				//        Logical minimum (-128)	15 80
		0x25,	0x7F,				//        Logical maximum (127)	25 7F
		0x75,	0x08,				//        Report Size (8)	75 08
	  0x95,	0x14,				//      Report Count (20)	95 14
		0x81,	0x22,				//        Input (Data,Value,Absolute,Bit Field)	81 22
		0xC0,							//    End Collection	C0 
	
		0xA1,	0x02,				//    Collection (Logical)	A1 02
		0x85,	HIDS_REPORT_ID_93,				//        Report Id (93)	85 5D
		0x09,	0x00,				//Usage (Vendor-defined 0x0000)	09 00
		0x15,	0x80,				//        Logical minimum (-128)	15 80
		0x25,	0x7F,				//        Logical maximum (127)	25 7F
		0x75,	0x08,				//        Report Size (8)	75 08
	  0x95,	0x14,				//      Report Count (20)	95 14
		0x81,	0x22,				//        Input (Data,Value,Absolute,Bit Field)	81 22
		0xC0,							//    End Collection	C0 
		
		0xA1,	0x02,				//    Collection (Logical)	A1 02
		0x85,	HIDS_REPORT_ID_94,				//        Report Id (94)	85 5E
		0x09,	0x00,				//Usage (Vendor-defined 0x0000)	09 00
		0x15,	0x80,				//        Logical minimum (-128)	15 80
		0x25,	0x7F,				//        Logical maximum (127)	25 7F
		0x75,	0x08,				//        Report Size (8)	75 08
	  0x95,	0x14,				//      Report Count (20)	95 14
		0x81,	0x22,				//        Input (Data,Value,Absolute,Bit Field)	81 22
		0xC0,							//    End Collection	C0 
		
		0xA1,	0x02,				//    Collection (Logical)	A1 02
		0x85,	HIDS_REPORT_ID_43,				//        Report Id (43)	85 2B
		0x09,	0x00,				//Usage (Vendor-defined 0x0000)	09 00
		0x15,	0x80,				//        Logical minimum (-128)	15 80
		0x25,	0x7F,				//        Logical maximum (127)	25 7F
		0x75,	0x08,				//        Report Size (8)	75 08
	  0x95,	0x14,				//      Report Count (20)	95 14
		0x81,	0x22,				//        Input (Data,Value,Absolute,Bit Field)	81 22
		0xC0,							//    End Collection	C0 
		0xC0,							//End Collection	C0 
		
		0x05,	0x0C,				//Usage Page (Consumer)	05 0C
		0x09,	0x01,				//Usage (Consumer Control)	09 01
		0xA1,	0x01,				//Collection (Application)	A1 01
		0x85,	HIDS_REPORT_ID_2,				//    Report Id (2)	85 02
		0x19,	0x00,				//    Usage Minimum (Unassigned)	19 00
		0x2A,	0x9C,	0x02, //     Usage Maximum (AC Distribute Vertically)	2A 9C 02  
		0x15,	0x00,				//    Logical minimum (0)	15 00
		0x26,	0x9C,	0x02,	//    Logical maximum (668)	26 9C 02
		0x95,	0x02,				//    Report Count (2)	95 02
		0x75,	0x10,				//    Report Size (16)	75 10
		0x80,							//    Input (Data,Array,Absolute,Bit Field)	80  
		0xC0,							//End Collection	C0 
		
		0x05,	0x01,				//Usage Page (Generic Desktop)	05 01
		0x09,	0x07,				//Usage (Keypad)	09 07
		0xA1,	0x01,				//Collection (Application)	A1 01
		0x85,	HIDS_REPORT_ID_1,				//    Report Id (1)	85 01
		0x05,	0x07,				//    Usage Page (Keyboard)	05 07
		0x19,	0xE0,				//    Usage Minimum (Keyboard Left Control)	19 E0
		0x29,	0xE7,				//    Usage Maximum (Keyboard Right GUI)	29 E7
		0x15,	0x00,				//    Logical minimum (0)	15 00
		0x25,	0x01,				//    Logical maximum (1)	25 01
		0x75,	0x01,				//    Report Size (1)	75 01
		0x95,	0x08,				//    Report Count (8)	95 08
		0x81,	0x02,				//    Input (Data,Value,Absolute,Bit Field)	81 02
		0x95,	0x01,				//    Report Count (1)	95 01
		0x75,	0x08,				//    Report Size (8)	75 08
		0x81,	0x03,				//    Input (Constant,Value,Absolute,Bit Field)	81 03
		0x95,	0x05,				//    Report Count (5)	95 05	
		0x75,	0x01,				//    Report Size (1)	75 01
		0x05,	0x08,				//    Usage Page (LEDs)	05 08
		0x19,	0x01,				//    Usage Minimum (Num Lock)	19 01
		0x29,	0x05,				//    Usage Maximum (Kana)	29 05
		0x91,	0x02,				//    Output (Data,Value,Absolute,Non-volatile,Bit Field)	91 02
		0x95,	0x01,				//    Report Count (1)	95 01
		0x75,	0x03,				//    Report Size (3)	75 03
		0x91,	0x03,				//    Output (Constant,Value,Absolute,Non-volatile,Bit Field)	91 03
		0x95,	0x06,				//    Report Count (6)	95 06
		0x75,	0x08,				//    Report Size (8)	75 08
		0x15,	0x00,				//    Logical minimum (0)	15 00
		0x25,	0xFF,				//    Logical maximum (255)	25 FF
		0x05,	0x07,				//    Usage Page (Keyboard)	05 07	
		0x19,	0x00,				//    Usage Minimum (No event indicated)	19 00
		0x29,	0xFF,				//    Usage Maximum (Reserved (0x00FF))	29 FF
		0x81,	0x00,				//    Input (Data,Array,Absolute,Bit Field)	81 00
		0xC0,							//End Collection	C0 
};




#define CHAR_ILLEGAL     0xff
#define CHAR_RETURN     '\n'
#define CHAR_ESCAPE      27
#define CHAR_TAB         '\t'
#define CHAR_BACKSPACE   0x7f

// Simplified US Keyboard with Shift modifier

/**
 * English (US)
 */
static const uint8_t keytable_us_none [] = {
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /*   0-3 */
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',                   /*  4-13 */
    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',                   /* 14-23 */
    'u', 'v', 'w', 'x', 'y', 'z',                                       /* 24-29 */
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',                   /* 30-39 */
    CHAR_RETURN, CHAR_ESCAPE, CHAR_BACKSPACE, CHAR_TAB, ' ',            /* 40-44 */
    '-', '=', '[', ']', '\\', CHAR_ILLEGAL, ';', '\'', 0x60, ',',       /* 45-54 */
    '.', '/', CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,   /* 55-60 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 61-64 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 65-68 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 69-72 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 73-76 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 77-80 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 81-84 */
    '*', '-', '+', '\n', '1', '2', '3', '4', '5',                       /* 85-97 */
    '6', '7', '8', '9', '0', '.', 0xa7,                                 /* 97-100 */
};

static const uint8_t keytable_us_shift[] = {
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /*  0-3  */
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',                   /*  4-13 */
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',                   /* 14-23 */
    'U', 'V', 'W', 'X', 'Y', 'Z',                                       /* 24-29 */
    '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',                   /* 30-39 */
    CHAR_RETURN, CHAR_ESCAPE, CHAR_BACKSPACE, CHAR_TAB, ' ',            /* 40-44 */
    '_', '+', '{', '}', '|', CHAR_ILLEGAL, ':', '"', 0x7E, '<',         /* 45-54 */
    '>', '?', CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,   /* 55-60 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 61-64 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 65-68 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 69-72 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 73-76 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 77-80 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 81-84 */
    '*', '-', '+', '\n', '1', '2', '3', '4', '5',                       /* 85-97 */
    '6', '7', '8', '9', '0', '.', 0xb1,                                 /* 97-100 */
};

// static btstack_timer_source_t heartbeat;
static btstack_packet_callback_registration_t hci_event_callback_registration;
static btstack_packet_callback_registration_t sm_event_callback_registration;
static uint8_t battery = 90;
hci_con_handle_t hid_con_handle = HCI_CON_HANDLE_INVALID;
static uint8_t protocol_mode = 1;

static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

static void le_keyboard_setup(void){

    l2cap_init();

    // setup le device db
    le_device_db_init();

    // setup SM: Display only
    sm_init();
    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
    sm_set_authentication_requirements(SM_AUTHREQ_BONDING);
    extern const uint8_t profile_data[];
    // setup ATT server
//    att_server_init(profile_data, NULL, NULL);

		ff10_service_server_init();
    // setup battery service
    battery_service_server_init(battery);

    // setup device information service
    device_information_service_server_init();
	
	  device_information_service_server_set_system_id(0x12345678,0x0a0b0c0d);

    // setup HID Device service
    hids_device_init(0, hid_descriptor_keyboard_boot_mode, sizeof(hid_descriptor_keyboard_boot_mode));


    // register for HCI events
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
	
//    // register for SM events
    sm_event_callback_registration.callback = &packet_handler;
    sm_add_event_handler(&sm_event_callback_registration);

    // register for HIDS
    hids_device_register_packet_handler(packet_handler);
}

// HID Keyboard lookup
static int lookup_keycode(uint8_t character, const uint8_t * table, int size, uint8_t * keycode){
    int i;
    for (i=0;i<size;i++){
        if (table[i] != character) continue;
        *keycode = i;
        return 1;
    }
    return 0;
}

static int keycode_and_modifer_us_for_character(uint8_t character, uint8_t * keycode, uint8_t * modifier){
    int found;
    found = lookup_keycode(character, keytable_us_none, sizeof(keytable_us_none), keycode);
    if (found) {
        *modifier = 0;  // none
        return 1;
    }
    found = lookup_keycode(character, keytable_us_shift, sizeof(keytable_us_shift), keycode);
    if (found) {
        *modifier = 2;  // shift
        return 1;
    }
    return 0;
}

// HID Report sending

//Send keyboard code values
void send_key_report(int modifier, int keycode){
    uint8_t report[] = { /* 0xa1, */ modifier, 0, keycode, 0, 0, 0, 0, 0,};
    hids_device_send_keyboard_report(0x10, APP_HID_ID_2_INPUT_ENDPORT,report, sizeof(report));
}

// Demo Application


// On embedded systems, send constant demo text with fixed period
static int send_mediacode1;
static int send_mediacode2;

int send_keyup = 0;
int key_value = 0;


void send_media_report(int send_mediacode1,int send_mediacode2){
   uint16_t report[] = {  send_mediacode1, send_mediacode2};
	 printf("send_media_report:%x,%x,%x\r\n",protocol_mode,send_mediacode1,send_mediacode2);
    switch (protocol_mode){
        case 0:
						hids_device_send_media_report(0x10, APP_HID_ID_2_INPUT_ENDPORT,(uint8_t *)report, sizeof(report));
            break;
        case 1:
					 
           hids_device_send_media_report(0x10, APP_HID_ID_2_INPUT_ENDPORT,(uint8_t *)report, sizeof(report));
           break;
        default:
            break;
    }
}

void send_keyboard_report(uint8_t kecode){
   uint8_t report[8] = {0};
	 memset(report,0,8);
	 report[2] = kecode;
	 printf("send_keyboard_report:%x,%x\r\n",protocol_mode,kecode);
    switch (protocol_mode){
        case 0:
						hids_device_send_keyboard_report(0x10, APP_HID_ID_1_INPUT_ENDPORT,(uint8_t *)report, sizeof(report));
            break;
        case 1:
					 
           hids_device_send_keyboard_report(0x10, APP_HID_ID_1_INPUT_ENDPORT,(uint8_t *)report, sizeof(report));
           break;
        default:
            break;
    }
}

void send_voice_report(uint8_t* report,uint8_t len){

	// printf("send_voice_report\r\n");
    switch (protocol_mode){
        case 0:
						hids_device_send_keyboard_report(0x10, APP_HID_ID_43_INPUT_ENDPORT,(uint8_t *)report, len);
            break;
        case 1:
					 
           hids_device_send_keyboard_report(0x10, APP_HID_ID_43_INPUT_ENDPORT,(uint8_t *)report, len);
           break;
        default:
            break;
    }
}

extern uint8_t g_test_value ;


static void sky_keyEvt_can_send_now(void){
    key_event_t key_evt;
		
		if(pop_kb_from_ring_buff(&key_evt))
		{
			//  printf("active:%d, key_evt.keyType:%d\r\n",key_evt.active,key_evt.keyType);
				switch(key_evt.active)
				{
					case BUTTON_PUSH_EVT:
					{
								if(key_evt.keyType == MEDIA_KEY)
								{
									send_media_report(key_evt.keycode,0);
								}
								else if(key_evt.keyType == KEYBOARD_KEY)
								{
									send_keyboard_report(key_evt.keycode);
								}
								else if(key_evt.keyType == VOICE_KEY)
								{
									send_keyboard_report(key_evt.keycode);
									g_test_value = 0xaa;
								}
							
						
					}break;
					
					case BUTTON_LONG_PUSH_EVT:
					{		
							uint8_t buff[20];
							store_encode_data(buff);
							hids_device_request_voice_can_send_now_event(0x10,2);
								
					}break;
					
					case BUTTON_RELEASE_EVT:
					{
								if(key_evt.keyType == MEDIA_KEY)
								{
									send_media_report(0,0);
								}
								else if(key_evt.keyType == KEYBOARD_KEY)
								{
									send_keyboard_report(0);
								}
								else if(key_evt.keyType == VOICE_KEY)
								{
									
									printf("stop voice send\r\n");	
									uint8_t buff[20] = {"nanosic voice stop "};
									send_voice_report(buff,20);									
									hids_device_request_voice_can_send_now_event(0x10,0);
	
								}
					}break;
					
					
					default:	
					break;
				}
		}
}
#include "sbc.h"
extern sbc_t sbc;
uint8_t inputdata[240];
uint8_t buff0[20];
uint8_t buff[20];
static void sky_voice_can_send_now(void){
			
			ssize_t encoded;
			sbc_encode(&sbc, inputdata, 240,
				buff, 240,
				&encoded);
			buff0[0]++;
		//	if(read_encode_data(buff))
			{
					send_voice_report(buff0,20);
			}
			store_encode_data(buff);
			hids_device_request_voice_can_send_now_event(0x10,2);
							
}

static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    UNUSED(channel);
    UNUSED(size);
    switch (packet_type) {
        case HCI_EVENT_PACKET:
            switch (hci_event_packet_get_type(packet)) {
                case HCI_EVENT_DISCONNECTION_COMPLETE:
                    hid_con_handle = HCI_CON_HANDLE_INVALID;
                    printf("Disconnected\n");
                    break;
                case SM_EVENT_JUST_WORKS_REQUEST:
                    printf("Just Works requested\n");
                    sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
                    break;
                case SM_EVENT_NUMERIC_COMPARISON_REQUEST:
                    printf("Confirming numeric comparison: %"PRIu32"\n", sm_event_numeric_comparison_request_get_passkey(packet));
                    sm_numeric_comparison_confirm(sm_event_passkey_display_number_get_handle(packet));
                    break;
                case SM_EVENT_PASSKEY_DISPLAY_NUMBER:
                    printf("Display Passkey: %"PRIu32"\n", sm_event_passkey_display_number_get_passkey(packet));
                    break;
                case HCI_EVENT_HIDS_META:
                    switch (hci_event_hids_meta_get_subevent_code(packet)){
                        case HIDS_SUBEVENT_INPUT_REPORT_ENABLE:
                            hid_con_handle = hids_subevent_input_report_enable_get_con_handle(packet);
                            printf("Report Characteristic Subscribed %u\n", hids_subevent_input_report_enable_get_enable(packet));
                            break;
												case HIDS_SUBEVENT_OUTPUT_REPORT_ENABLE:
														hid_con_handle = hids_subevent_output_report_enable_get_con_handle(packet);
                            printf("Output Report Characteristic Subscribed %u\n", hids_subevent_output_report_enable_get_enable(packet));
							
                        case HIDS_SUBEVENT_BOOT_KEYBOARD_INPUT_REPORT_ENABLE:
                            hid_con_handle = hids_subevent_boot_keyboard_input_report_enable_get_con_handle(packet);
                            printf("Boot Keyboard Characteristic Subscribed %u\n", hids_subevent_boot_keyboard_input_report_enable_get_enable(packet));
                            break;
                        case HIDS_SUBEVENT_PROTOCOL_MODE:
														protocol_mode = hids_subevent_protocol_mode_get_protocol_mode(packet);
                            printf("Protocol Mode: %s mode %d\n", hids_subevent_protocol_mode_get_protocol_mode(packet) ? "Report" : "Boot",protocol_mode);
                            break;
                        case HIDS_SUBEVENT_CAN_SEND_NOW:
													//  printf("can send now\n");
                            sky_keyEvt_can_send_now();
														
                            break;
												 case HIDS_SUBEVENT_VOICE_CAN_SEND_NOW:
												 {
													 uint32_t start = little_endian_read_16(packet,5);
													 
												//	 printf("voice can send now,start:%d\n",start);
													
									
													 if(start == 1)
													 {
														  uint8_t buff[20] = {"nanosic voice start"};
														//	store_encode_data(buff);
															send_voice_report(buff,20);
																	
													 }else if(start == 0)
													 {
															voice_ring_buffer_init();
															send_keyboard_report(0);
													 }
													 else if(start == 2)
													 {
															sky_voice_can_send_now();
													 }
														
															
												} break;
												
												case HIDS_SUBEVENT_OUTPUT_REPORT_DATA:
												{
													
													uint16_t conHandle = little_endian_read_16(packet, 3);
													uint16_t att_handle = little_endian_read_16(packet, 5);
													uint16_t offset = little_endian_read_16(packet, 7);
													uint16_t buffLen = little_endian_read_16(packet, 9);
													uint8_t *p_buff = (uint8_t *)&packet[11];
													
													hids_device_t * instance = hids_device_get_instance_for_con_handle(conHandle);
													if (!instance){
														log_error("no instance for handle 0x%02x", conHandle);
														return;
												}
													printf("hid handle:%d,id offset:%d,hid buffLen:%d\r\n",att_handle,offset,buffLen);
													printf("data:");
													for(int i = 0;i < buffLen;i ++)
													{
														printf("0x%02x ",p_buff[i]);
													}printf("\r\n");
													
													if(att_handle == instance->hid_report_94_output_value_handle)
													{
														if((p_buff[0] == 0x5A) && (p_buff[1] == 0X01))
														{	
																voice_ring_buffer_init();
																printf("dev allow start voice send\r\n");
															
																hids_device_request_voice_can_send_now_event(0x10,1);
														}
														
														if((p_buff[0] == 0x5A) && (p_buff[1] == 0X00))
														{	
																voice_ring_buffer_init();
																printf("dev stop voice send\r\n");
														}
													}
												}
													
												
												break;
												
                        default:
                            break;
                    }
            }
            break;
    }

}

int btstack_main(void);
int btstack_main(void)
{
    le_keyboard_setup();
	

#ifdef HAVE_BTSTACK_STDIN
    btstack_ring_buffer_init(&ascii_input_buffer, ascii_input_storage, sizeof(ascii_input_storage));
    btstack_stdin_setup(stdin_process);
#endif

    // turn on!
    hci_power_control(HCI_POWER_ON);

    return 0;
}
/* EXAMPLE_END */
