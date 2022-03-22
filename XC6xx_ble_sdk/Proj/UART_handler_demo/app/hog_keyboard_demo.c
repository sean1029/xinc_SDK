#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "btstack.h"

#include "FF10_service_server.h"
#include "battery_service_server.h"
#include "device_information_service_server.h"
#include "hids_device.h"
#include   "xc_kbs_event.h"

#define  HID_WITH_VOICE_RC


#ifdef HID_WITH_VOICE_RC
#define HIDS_KB_REPORT_ID       	1
#define HIDS_MOUSE_REPORT_ID    	5
#define RMC_VENDOR_REPORT_ID_1  	0xfd
#define RMC_VENDOR_REPORT_ID_2   	0x1e
#define HIDS_MM_KB_REPORT_ID     	3
#define RMC_SENSORS_DATA_REPORT_ID  0x32 


#define RMC_WITH_VOICE				1
#define MULTIMEDIA_KEYBOARD			1
#define RMC_WITH_SENSORS_DATA		1

const uint8_t hid_descriptor_keyboard_boot_mode[] =
{
    0x05, 0x01,
    0x09, 0x06,
    0xa1, 0x01,
    0x85, HIDS_KB_REPORT_ID,
    0x05, 0x07,
    0x19, 0xe0,
    0x29, 0xe7,
    0x15, 0x00 ,
    0x25, 0x01,
    0x75, 0x01,
    0x95, 0x08,
    0x81, 0x02,
    0x95, 0x01,
    0x75, 0x08,
    0x81, 0x01,

    0x95, 0x05,
    0x75, 0x01,
    0x05, 0x08,
    0x19, 0x01,
    0x29, 0x05,
    0x91, 0x02,
    0x95, 0x01,
    0x75, 0x03,
    0x91, 0x01,
    0x95, 0x06,
    0x75, 0x08,
    0x15, 0x00,
    0x25, 0xff,
    0x05, 0x07,
    0x19, 0x00,
    0x29, 0xff,
    0x81, 0x00,
    0xc0,

    0x05, 0x01,  /// USAGE PAGE (Generic Desktop) 定位到Generic Desktop页，这个相当于指针跳转一样的东西
    0x09, 0x02,  /// USAGE (Mouse) 表示这是一个鼠标
    0xa1, 0x01,  /// COLLECTION (Application) 是对Mouse的解释
    0x85, HIDS_MOUSE_REPORT_ID, /// REPORT ID (5) 
    0x09, 0x01,  /// USAGE (Pointer) 表示指针形式
    0xa1, 0x00,  /// COLLECTION (Physical)  是对Pointer的解释
    /**
     * ----------------------------------------------------------------------------
     * BUTTONS
     * ----------------------------------------------------------------------------
     */
    0x05, 0x09,  /// USAGE PAGE (Buttons)
    0x19, 0x01,  /// Usage Minimum (01) -Button 1
    0x29, 0x03,  /// Usage Maximum (03) -Button 3
    0x15, 0x00,  /// Logical Minimum (0)
    0x25, 0x01,  /// Logical Maximum (1)
    0x95, 0x03,  /// Report Count (3)
    0x75, 0x01,  /// Report Size (1)
    0x81, 0x02,  /// Input (Data, Variable,Absolute) - Button states
    0x95, 0x01,  /// Report Count (1)
    0x75, 0x05,  /// Report Size (5)
    0x81, 0x01,  /// Input (Constant) - Paddingor Reserved bits
    /**
     * ----------------------------------------------------------------------------
     * MOVEMENT DATA
     * ----------------------------------------------------------------------------
     */
    0x05, 0x01,  /// USAGE PAGE (Generic Desktop)
    0x09, 0x30,  /// USAGE (X)
    0x09, 0x31,  /// USAGE (Y)
    0x09, 0x38,  /// USAGE (Wheel)
    0x15, 0x81,  /// LOGICAL MINIMUM (-127)
    0x25, 0x7f,  /// LOGICAL MAXIMUM (127)
    0x75, 0x08,  /// REPORT SIZE (8)
    0x95, 0x03,  /// REPORT COUNT (3)
    0x81, 0x06,  /// INPUT 
    0xc0,
    0xc0,

#if RMC_WITH_VOICE

    0x06, 0x12, 0xff, 
    0x0a, 0x12, 0xff, 
    0xa1, 0x01, 		// Collection 
    0x85, RMC_VENDOR_REPORT_ID_1, // Report ID 
    0x09, 0x01, 		// Usage 
    0x75, 0x08, 		// Report Size (8),
    //0x95, 0xff, 		// Report Count (256), modify
    0x95, 0x13,			// Report Count (19),
    0x16, 0x00, 0x00, 	// Logical Minimum ,
    0x26, 0xff ,0x00, 	// Logical Maximum ,
    0x19, 0x00, 		// Usage Minimum (), 
    0x29, 0xff, 		// Usage Maximum (),
    0x81, 0x00, 		// Input  
    0xc0,				// END_COLLECTION

    0x06, 0x12, 0xff, 
    0x0a, 0x12, 0xff, 
    0xa1, 0x01, 		// Collection 
    0x85, RMC_VENDOR_REPORT_ID_2, // Report ID 
    0x09, 0x01, 		// Usage
    0x75, 0x08, 		// Report Size (8),
    //0x95, 0xff, 		// Report Count (256),
    0x95, 0x13,			// Report Count (19),
    0x16, 0x00, 0x00, 	// Logical Minimum ,
    0x26, 0xff, 0x00, 	// Logical Maximum ,
    0x19, 0x00, 		// Usage Minimum (), 
    0x29, 0xff, 		// Usage Maximum (),
    0x81, 0x00,		// Input 
    
    0x95, 0x08,		// Usage
    0x75, 0x01,		// Report Size (1),
    0x05, 0x08,		// Report Count (8),
    0x19, 0x01,		// Usage Minimum (), 
    0x29, 0x08,		// Usage Maximum (),
    0x91, 0x02,		// Output 
    0xc0,			// END_COLLECTION
#endif
 
#if MULTIMEDIA_KEYBOARD
     0x05, 0x0c,           // USAGE_PAGE (Consumer Devices) 
     0x09, 0x01,           // USAGE (Consumer Control) 
     0xa1, 0x01,           // COLLECTION (Application) 
     0x85, HIDS_MM_KB_REPORT_ID,  // REPORT_ID (3) 
     0x19, 0x00,          //USAGE_MINIMUM (0)
     0x2A, 0x9c, 0x02,    //USAGE_MAXIMUM (29c) 
     0x15, 0x00,          //LOGICAL_MINIMUM (0) 
     0x26, 0x9c, 0x02,    //LOGICAL_MAXIMUM (29c) 
     0x95, 0x01,          //REPORT_COUNT (1) 
     0x75, 0x10,          //REPORT_SIZE (16)
     0x81, 0x00,          //INPUT (Data,Ary,Abs) 
     0xc0,
#endif

#if RMC_WITH_SENSORS_DATA
	0x06, 0x00, 0xff, 
	0x09, 0x00,    // USAGE
	0xa1, 0x01,    // COLLECTION (Application) 
	0x85, RMC_SENSORS_DATA_REPORT_ID,  // REPORT_ID (0x32) 
	0x09, 0x00, 
	0x15, 0x80,   //LOGICAL_MINIMUM () 
	0x25, 0x7f,   //LOGICAL MAXIMUM ()
	0x75, 0x08,   // Report Size (8),
	0x95, 0x12,   // Report Count (18),
	0x81, 0x22,   // INPUT
	0xc0 
#endif	
};
#else
// from USB HID Specification 1.1, Appendix B.1
const uint8_t hid_descriptor_keyboard_boot_mode[] = {


	
    0x05, 0x01,                    // Usage Page (Generic Desktop)
    0x09, 0x06,                    // Usage (Keyboard)
    0xa1, 0x01,                    // Collection (Application)

    0x85,  0x01,                   // Report ID 1

    // Modifier byte

    0x75, 0x01,                    //   Report Size (1)
    0x95, 0x08,                    //   Report Count (8)
    0x05, 0x07,                    //   Usage Page (Key codes)
    0x19, 0xe0,                    //   Usage Minimum (Keyboard LeftControl)
    0x29, 0xe7,                    //   Usage Maxium (Keyboard Right GUI)
    0x15, 0x00,                    //   Logical Minimum (0)
    0x25, 0x01,                    //   Logical Maximum (1)
    0x81, 0x02,                    //   Input (Data, Variable, Absolute)

    // Reserved byte

    0x75, 0x01,                    //   Report Size (1)
    0x95, 0x08,                    //   Report Count (8)
    0x81, 0x01,                    //   Input (Constant, Variable, Absolute)

    // LED report + padding

    0x95, 0x05,                    //   Report Count (5)
    0x75, 0x01,                    //   Report Size (1)
    0x05, 0x08,                    //   Usage Page (LEDs)
    0x19, 0x01,                    //   Usage Minimum (Num Lock)
    0x29, 0x05,                    //   Usage Maxium (Kana)
    0x91, 0x02,                    //   Output (Data, Variable, Absolute)

    0x95, 0x01,                    //   Report Count (1)
    0x75, 0x03,                    //   Report Size (3)
    0x91, 0x01,                    //   Output (Constant, Variable, Absolute)

    // Keycodes

    0x95, 0x06,                    //   Report Count (6)
    0x75, 0x08,                    //   Report Size (8)
    0x15, 0x00,                    //   Logical Minimum (0)
    0x25, 0x65,                    //   Logical Maximum (1)
    0x05, 0x07,                    //   Usage Page (Key codes)
    0x19, 0x00,                    //   Usage Minimum (Reserved (no event indicated))
    0x29, 0x65,                    //   Usage Maxium (Reserved)
    0x81, 0x00,                    //   Input (Data, Array)
	
	0xc0,
	
	0x05, 0x0C, // Usage Pg (Consumer Devices)

	0x09, 0x01, // Usage (Consumer Control)

	0xA1, 0x01, // Collection (Application)

	0x85, 0x02, // Report Id (2)

	0x09, 0x02, // Usage (Numeric Key Pad)

	0xA1, 0x02, // Collection (Logical)

	0x05, 0x09, // Usage Pg (Button)

	0x19, 0x01, // Usage Min (Button 1)

	0x29, 0x0A, // Usage Max (Button 10)

	0x15, 0x01, // Logical Min (1)

	0x25, 0x0A, // Logical Max (10)

	0x75, 0x04, // Report Size (4)

	0x95, 0x01, // Report Count (1)

	0x81, 0x00, // Input (Data,Ary,Abs)

	0xC0, // End Collection

	0x05, 0x0C, // Usage Pg (Consumer Devices)

	0x09, 0x86, // Usage (Channel)

	0x15, 0xFF, // Logical Min (-1)

	0x25, 0x01, // Logical Max (1)

	0x75, 0x02, // Report Size (2)

	0x95, 0x01, // Report Count (1)

	0x81, 0x46, // Input (Data, Var, Rel, Null)

	0x09, 0xE9, // Usage (Volume Up)

	0x09, 0xEA, // Usage (Volume Down)

	0x15, 0x00, // Logical Min (0)

	0x75, 0x01, // Report Size (1)

	0x95, 0x02, // Report Count (2)

	0x81, 0x02, // Input (Data, Var,Abs)

	0x09, 0xE2, // Usage (Mute)

	0x09, 0x30, // Usage (Power)

	0x09, 0x40, // Usage (Menu)

	0x09, 0xB1, // Usage (Pause)

	0x09, 0xB2, // Usage (Record)

	0x0a, 0x23, 0x02, // Usage (Home)

	0x0a, 0x24, 0x02, // Usage (Back)

	0x09, 0xB3, // Usage (Fast Forward)

	0x09, 0xB4, // Usage (Rewind)

	0x09, 0xB5, // Usage (Scan Next)

	0x09, 0xB6, // Usage (Scan Prev)

	0x09, 0xB7, // Usage (Stop)

	0x15, 0x01, // Logical Min (1)

	0x25, 0x0C, // Logical Max (12)

	0x75, 0x04, // Report Size (4)

	0x95, 0x01, // Report Count (1)

	0x81, 0x00, // Input (Data, Ary,Abs)

	0x09, 0x80, // Usage (Selection)

	0xA1, 0x02, // Collection (Logical)

	0x05, 0x09, // Usage Pg (Button)

	0x19, 0x01, // Usage Min (Button 1)

	0x29, 0x03, // Usage Max (Button 3)

	0x15, 0x01, // Logical Min (1)

	0x25, 0x03, // Logical Max (3)

	0x75, 0x02, // Report Size (2)

	0x81, 0x00, // Input (Data,Ary,Abs)

	0xC0, // End Collection

	0x81, 0x03, // Input (Const, Var,Abs)

	0xC0 // End Collection

	

};
#endif



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
    hids_device_send_input_report(0x10, APP_HID_MEDIA_IN_ENDPORT,report, sizeof(report));
}

// Demo Application


// On embedded systems, send constant demo text with fixed period
static int send_mediacode1;
static int send_mediacode2;

int send_keyup = 0;
int key_value = 0;

static void send_key(int mediacode1,int mediacode2){
    send_mediacode1 = mediacode1;
    send_mediacode2 = mediacode2;
    hids_device_request_can_send_now_event(hid_con_handle);
}

void send_media_report(int send_mediacode1,int send_mediacode2){
   uint8_t report[] = { /* 0xa1, */ send_mediacode1, send_mediacode2};
	//  uint8_t report[] = { /* 0xa1, */ send_mediacode1};
	 printf("send_media_report:%x,%x,%x\r\n",protocol_mode,send_mediacode1,send_mediacode2);
    switch (protocol_mode){
        case 0:
					
          //  hids_device_send_boot_keyboard_input_report(0x10, report, sizeof(report));
				hids_device_send_input_report(0x10, APP_HID_MEDIA_IN_ENDPORT,report, sizeof(report));
            break;
        case 1:
					 
           hids_device_send_input_report(0x10, APP_HID_MEDIA_IN_ENDPORT,report, sizeof(report));
           break;
        default:
            break;
    }
}
int pop_kb_from_ring_buff(key_event_t *key_evt);
static void typing_can_send_now(void){
    key_event_t key_evt;
		if(pop_kb_from_ring_buff(&key_evt))
		{
				switch(key_evt.event)
				{
					case 1:
					{
						if(key_evt.keycode == 2)
						{
								send_media_report(0xea,0);
						}
						if(key_evt.keycode == 3)
						{
								send_media_report(0xe9,0);
						}
						
					}break;
					
					case 3:
					{
						send_media_report(0,0);
					}break;
					
					
					default:	
					break;
				}
		}
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
													  printf("can send now\n");
                            typing_can_send_now();
														
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
