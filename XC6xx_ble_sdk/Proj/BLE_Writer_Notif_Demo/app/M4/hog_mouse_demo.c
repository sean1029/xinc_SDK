/*
 * Copyright (C) 2017 BlueKitchen GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 4. Any redistribution, use, or modification is done solely for
 *    personal benefit and not for any commercial purpose or for
 *    monetary gain.
 *
 * THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MATTHIAS
 * RINGWALD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Please inquire about commercial licensing options at
 * contact@bluekitchen-gmbh.com
 *
 */

#define BTSTACK_FILE__ "hog_mouse_demo.c"

// *****************************************************************************
/* EXAMPLE_START(hog_mouse_demo): HID Mouse LE
 */
// *****************************************************************************

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "bsp_gpio.h"
#include "btstack.h"

///#include "xc_dma_debug.h"
//#include "ble/gatt-service/battery_service_server.h"
//#include "ble/gatt-service/device_information_service_server.h"
//#include "ble/gatt-service/hids_device.h"
#include "battery_service_server.h"
#include "device_information_service_server.h"
#include "hids_device.h"
void send_str(char * sstr);
// from USB HID Specification 1.1, Appendix B.2
const uint8_t hid_descriptor_mouse_boot_mode[] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,                    // USAGE (Mouse)
    0xa1, 0x01,                    // COLLECTION (Application)

    0x85,  0x01,                    // Report ID 1

    0x09, 0x01,                    //   USAGE (Pointer)

    0xa1, 0x00,                    //   COLLECTION (Physical)

#if 1
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
    0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x05,                    //     REPORT_SIZE (5)
    0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
#endif

#if 1
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
	0x09, 0x38,                    // Usage (Wheel) hjl add
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    //0x95, 0x02,                    //     REPORT_COUNT (2)
	0x95, 0x03,                    //     REPORT_COUNT (3) hjl add
    0x81, 0x06,                    //     INPUT (Data,Var,Rel)
#endif

    0xc0,                          //   END_COLLECTION
    0xc0                           // END_COLLECTION
};

static btstack_packet_callback_registration_t hci_event_callback_registration;
static btstack_packet_callback_registration_t sm_event_callback_registration;
static uint8_t battery = 100;
static hci_con_handle_t con_handle = HCI_CON_HANDLE_INVALID;
static uint8_t protocol_mode = 1;

static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
//Hi-lux-121F0000001
#if 0
const uint8_t adv_data[] = {
    // Flags general discoverable, BR/EDR not supported
   31,0x09,'H','i','-','l','u','x','-','1','2',
	'1','F','0','0','0','0','0','0','1',0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00
};
#endif

/*const uint8_t adv_data[] = {
    // Flags general discoverable, BR/EDR not supported
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, 0x06,
    // Name
    0x0a, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'H', 'I', 'D', ' ', 'M', 'o', 'u', 's', 'e',
    // 16-bit Service UUIDs
    0x03, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS, ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE & 0xff, ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE >> 8,
    // Appearance HID - Mouse (Category 15, Sub-Category 2)
    0x03, BLUETOOTH_DATA_TYPE_APPEARANCE, 0xC2, 0x03,
};*/
extern const uint8_t adv_data[];
extern uint8_t adv_data_len ;

static void hog_mouse_setup(void){

//    extern const uint8_t profile_data[];
    // setup l2cap and register for connection parameter updates
    l2cap_init();
    l2cap_register_packet_handler(&packet_handler);

    // setup le device db
    le_device_db_init();

    // setup SM: Display only
//    sm_init();
//    sm_set_io_capabilities(IO_CAPABILITY_DISPLAY_ONLY);
//    // sm_set_authentication_requirements(SM_AUTHREQ_SECURE_CONNECTION | SM_AUTHREQ_BONDING);
//    sm_set_authentication_requirements(SM_AUTHREQ_BONDING);

//	sm_init();
//    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
//    sm_set_authentication_requirements(SM_AUTHREQ_BONDING);
	
    // setup ATT server
//    att_server_init(profile_data, NULL, NULL);

    // setup battery service
    battery_service_server_init(battery);

    // setup device information service
    device_information_service_server_init();

    // setup HID Device service
    hids_device_init(0, hid_descriptor_mouse_boot_mode, sizeof(hid_descriptor_mouse_boot_mode));

    // setup advertisements
//    uint16_t adv_int_min = 0x0030;
//    uint16_t adv_int_max = 0x0030;
//    uint8_t adv_type = 0;
//    bd_addr_t null_addr;
//    memset(null_addr, 0, 6);
//    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
//    gap_advertisements_set_data(adv_data_len, (uint8_t*) adv_data);
//    gap_advertisements_enable(1);

    // register for events
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    sm_event_callback_registration.callback = &packet_handler;
    sm_add_event_handler(&sm_event_callback_registration);

    hids_device_register_packet_handler(packet_handler);
}

// HID Report sending
int send_report(uint8_t buttons, int8_t dx, int8_t dy,int8_t Wheel){
	
    uint8_t report[] = { buttons, (uint8_t) dx, (uint8_t) dy, (uint8_t) Wheel};
	printf("----hids sending:%d/%d\n", dx, dy);
    switch (protocol_mode){
        case 0:
            hids_device_send_boot_mouse_input_report(0x10, report, sizeof(report));
        case 1:
            ///hids_device_send_input_report(con_handle, report, sizeof(report));
		    hids_device_send_input_report(0x10, report, sizeof(report));
            break;
        default:
            break;
    }
    //printf("Mouse: %d/%d - buttons: %02x\n", dx, dy, buttons);
	
	return protocol_mode;
	
}

static int dx;
static int dy;
static uint8_t buttons;

static void mousing_can_send_now(void){
	printf("--input value:x:%d,y:%d\n",dx,dy);
    send_report(buttons, dx, dy,1);
    // reset
    dx = 0;
    dy = 0;
    if (buttons){
        buttons = 0;
        hids_device_request_can_send_now_event(con_handle);
    }
}

// Demo Application

#ifdef HAVE_BTSTACK_STDIN

static const int MOUSE_SPEED = 30;

// On systems with STDIN, we can directly type on the console

static void stdin_process(char character){

    if (con_handle == HCI_CON_HANDLE_INVALID) {
        printf("Mouse not connected, ignoring '%c'\n", character);
        return;
    }

    switch (character){
        case 'a':
            dx -= MOUSE_SPEED;
            break;
        case 's':
            dy += MOUSE_SPEED;
            break;
        case 'd':
            dx += MOUSE_SPEED;
            break;
        case 'w':
            dy -= MOUSE_SPEED;
            break;
        case 'l':
            buttons |= 1;
            break;
        case 'r':
            buttons |= 2;
            break;
        default:
            return;
    }
    hids_device_request_can_send_now_event(con_handle);
}

#else

// On embedded systems, simulate clicking on 4 corners of a square

#define MOUSE_PERIOD_MS 7

static const int STEPS_PER_DIRECTION = 20;
static const int MOUSE_SPEED = 20;

static btstack_timer_source_t mousing_timer;
static int mousing_active = 0;
static int step=0;

static struct {
    int dx;
    int dy;
} directions[] = {
	{1,0},
	{0,1},
	{-1,0},
	{0,-1},
    /*{  1,  2 },
	{  3,  4 },
    {  2,  1 },
    {  4,  3 },
    { -1, -2 },
    { -3, -4 },
    { -2, -1 },
    { -4, -3 },*/
};


static void mousing_timer_handler(btstack_timer_source_t * ts){

	//printf("---mousing_timer,con_handle:%d\n",con_handle);
	
    if (con_handle == HCI_CON_HANDLE_INVALID) {
        mousing_active = 0;
        return;
    }

    // simulate move
    int direction_index = step / STEPS_PER_DIRECTION;
    dx += directions[direction_index].dx * MOUSE_SPEED;
    dy += directions[direction_index].dy * MOUSE_SPEED;

    // next
    step++;
    if (step >= STEPS_PER_DIRECTION * 4) {
        step = 0;
    }
    
	
    // trigger send
    hids_device_request_can_send_now_event(con_handle);

    // set next timer
    btstack_run_loop_set_timer(ts, MOUSE_PERIOD_MS);
    btstack_run_loop_add_timer(ts);
}

static void hid_embedded_start_mousing(void){
    if (mousing_active) return;
    mousing_active = 1;

    printf("Start mousing..\n");

    step = 0;

    // set one-shot timer
    mousing_timer.process = &mousing_timer_handler;
    btstack_run_loop_set_timer(&mousing_timer, MOUSE_PERIOD_MS);
    btstack_run_loop_add_timer(&mousing_timer);
}
#endif


	

void mouse_simulate_run(void)
{
	int  	sendchar(int c);
	static uint32_t time_count =0;
	int i =0;
    // simulate left click when corner reached
    if (step % STEPS_PER_DIRECTION == 0){
        buttons |= 1;
    }
    // simulate move
    int direction_index = step / STEPS_PER_DIRECTION;
    dx += directions[direction_index].dx * MOUSE_SPEED;
    dy += directions[direction_index].dy * MOUSE_SPEED;

    // next
    step++;
    if (step >= STEPS_PER_DIRECTION * 4) {
        step = 0;
    }
	
	i = send_report(0, dx, dy,0);
	time_count++;
	if(time_count%2)
	   gpio_output_high(4);
	else
		gpio_output_low(4);
	
	if(time_count%200 == 0) //500ms
	{	
	   ;/// sendchar(' ');sendchar(' ');sendchar(time_count/0x100);sendchar(time_count%0x100);sendchar(i);
    }
	
}	

static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    //xc_dma_printf("hog_mouse_demo packet_handler in packet size:%d\nbuf:",size);
	UNUSED(channel);
    UNUSED(size);
    uint16_t conn_interval;
	for(int i=0;i<size;i++)
	   //xc_dma_printf("%02x ",packet[i]); 
	//xc_dma_printf("\n");

    if (packet_type != HCI_EVENT_PACKET) return;

    switch (hci_event_packet_get_type(packet)) {
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            con_handle = HCI_CON_HANDLE_INVALID;
            //xc_dma_printf("\n\nDisconnected\n");
            break;
        case SM_EVENT_JUST_WORKS_REQUEST:
            //xc_dma_printf("\n\nJust Works requested\n");
            sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
            break;
        case SM_EVENT_NUMERIC_COMPARISON_REQUEST:
            //xc_dma_printf("\n\nConfirming numeric comparison: %d\n", sm_event_numeric_comparison_request_get_passkey(packet));
            sm_numeric_comparison_confirm(sm_event_passkey_display_number_get_handle(packet));
            break;
        case SM_EVENT_PASSKEY_DISPLAY_NUMBER:
            //xc_dma_printf("\n\nDisplay Passkey: %d\n", sm_event_passkey_display_number_get_passkey(packet));
            break;
        case L2CAP_EVENT_CONNECTION_PARAMETER_UPDATE_RESPONSE:
            //xc_dma_printf("\n\nL2CAP Connection Parameter Update Complete, response: %x\n", l2cap_event_connection_parameter_update_response_get_result(packet));
		    printf("\n-- cpup=%d\n",l2cap_event_connection_parameter_update_response_get_result(packet));
		    break;
        case HCI_EVENT_LE_META:
            switch (hci_event_le_meta_get_subevent_code(packet)) {
                case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
                    // print connection parameters (without using float operations)
                    conn_interval = hci_subevent_le_connection_complete_get_conn_interval(packet);
                    //xc_dma_printf("\n\nLE Connection Complete:\n");
                    //xc_dma_printf("- Connection Interval: %u.%02u ms\n", conn_interval * 125 / 100, 25 * (conn_interval & 3));
                    //xc_dma_printf("- Connection Latency: %u\n", hci_subevent_le_connection_complete_get_conn_latency(packet));
                    break;
                case HCI_SUBEVENT_LE_CONNECTION_UPDATE_COMPLETE:
                    // print connection parameters (without using float operations)
                    conn_interval = hci_subevent_le_connection_update_complete_get_conn_interval(packet);
                    //xc_dma_printf("\n\nLE Connection Update:\n");
                    //xc_dma_printf("- Connection Interval: %u.%02u ms\n", conn_interval * 125 / 100, 25 * (conn_interval & 3));
                    //xc_dma_printf("- Connection Latency: %u\n", hci_subevent_le_connection_update_complete_get_conn_latency(packet));
//				send_str("\nCon Update:");
//				send_digit(conn_interval);
				printf("\nCon Update:%d\n",conn_interval);
				    break;
                default:
                    break;
            }
            break;  
        case HCI_EVENT_HIDS_META:
            switch (hci_event_hids_meta_get_subevent_code(packet)){
                case HIDS_SUBEVENT_INPUT_REPORT_ENABLE:
                    con_handle = hids_subevent_input_report_enable_get_con_handle(packet);
                    //xc_dma_printf("\n\nReport Characteristic Subscribed %u\n", hids_subevent_input_report_enable_get_enable(packet));
#ifndef HAVE_BTSTACK_STDIN
                    ///hid_embedded_start_mousing();
#endif
                    // request connection param update via L2CAP following Apple Bluetooth Design Guidelines
                    // gap_request_connection_parameter_update(con_handle, 12, 12, 4, 100);    // 15 ms, 4, 1s

                    // directly update connection params via HCI following Apple Bluetooth Design Guidelines
                    // gap_update_connection_parameters(con_handle, 12, 12, 4, 100);    // 60-75 ms, 4, 1s

                    break;
                case HIDS_SUBEVENT_BOOT_KEYBOARD_INPUT_REPORT_ENABLE:
                    con_handle = hids_subevent_boot_keyboard_input_report_enable_get_con_handle(packet);
                    //xc_dma_printf("\n\nBoot Keyboard Characteristic Subscribed %x\n", hids_subevent_boot_keyboard_input_report_enable_get_enable(packet));
                    break;
                case HIDS_SUBEVENT_BOOT_MOUSE_INPUT_REPORT_ENABLE:
                    con_handle = hids_subevent_boot_mouse_input_report_enable_get_con_handle(packet);
                    //xc_dma_printf("\n\nBoot Mouse Characteristic Subscribed %x\n", hids_subevent_boot_mouse_input_report_enable_get_enable(packet));
                    break;
                case HIDS_SUBEVENT_PROTOCOL_MODE:
                    protocol_mode = hids_subevent_protocol_mode_get_protocol_mode(packet);
                    //xc_dma_printf("\n\nProtocol Mode: %s mode\n", hids_subevent_protocol_mode_get_protocol_mode(packet) ? "Report" : "Boot");
                    break;
                case HIDS_SUBEVENT_CAN_SEND_NOW: //在每个连接间隔时间发送
					printf("@@@\n");
                    ///mousing_can_send_now();
				    mouse_simulate_run();
				    ////xc_dma_printf("hog_mouse_demo HIDS_SUBEVENT_CAN_SEND_NOW\n");
				    ///mouse_sensor_run();
                    break;
                default:
                    break;
            }
            break;
            
        default:
            break;
    }
}

int btstack_main(void);
int btstack_main(void)
{
    hog_mouse_setup();

#ifdef HAVE_BTSTACK_STDIN
    btstack_stdin_setup(stdin_process);
#endif

    // turn on!
    hci_power_control(HCI_POWER_ON);

    return 0;
}
