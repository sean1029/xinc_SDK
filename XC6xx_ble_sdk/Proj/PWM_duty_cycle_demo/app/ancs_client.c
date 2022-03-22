/*
 * Copyright (C) 2014 BlueKitchen GmbH
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

#define __BTSTACK_FILE__ "ancs_client.c"

#include "btstack_config.h"

#include <stdint.h>
#include <string.h>

#include "ancs_client.h"

#include "att_db.h"
#include "core.h"
#include "gatt_client.h"
#include "sm.h"
#include "btstack_debug.h"
#include "btstack_event.h"
#include "btstack_run_loop.h"
#include "sdp_util.h"
#include "gap.h"


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

// ancs_client.h Start
typedef enum ancs_chunk_parser_state {
    W4_ATTRIBUTE_ID,
    W4_ATTRIBUTE_LEN,
    W4_ATTRIBUTE_COMPLETE,
} ancs_chunk_parser_state_t;

typedef enum {
    TC_IDLE,
    TC_W4_ENCRYPTED_CONNECTION,
    TC_W4_SERVICE_RESULT,
    TC_W4_CHARACTERISTIC_RESULT,
    TC_W4_DATA_SOURCE_SUBSCRIBED,
    TC_W4_NOTIFICATION_SOURCE_SUBSCRIBED,
    TC_SUBSCRIBED,
    TC_W4_DISCONNECT
} tc_state_t;

static const char * ancs_attribute_names[] = { 
    "AppIdentifier",
    "IDTitle",
    "IDSubtitle",
    "IDMessage",
    "IDMessageSize",
    "IDDate"
};
static const int ANCS_ATTRBUTE_NAMES_COUNT = sizeof(ancs_attribute_names) / sizeof(char *);

static const uint8_t ancs_service_uuid[] =             {0x79,0x05,0xF4,0x31,0xB5,0xCE,0x4E,0x99,0xA4,0x0F,0x4B,0x1E,0x12,0x2D,0x00,0xD0};
static const uint8_t ancs_notification_source_uuid[] = {0x9F,0xBF,0x12,0x0D,0x63,0x01,0x42,0xD9,0x8C,0x58,0x25,0xE6,0x99,0xA2,0x1D,0xBD};
static const uint8_t ancs_control_point_uuid[] =       {0x69,0xD1,0xD8,0xF3,0x45,0xE1,0x49,0xA8,0x98,0x21,0x9B,0xBD,0xFD,0xAA,0xD9,0xD9};
static const uint8_t ancs_data_source_uuid[] =         {0x22,0xEA,0xC6,0xE9,0x24,0xD6,0x4B,0xB5,0xBE,0x44,0xB3,0x6A,0xCE,0x7C,0x7B,0xFB};

static uint32_t ancs_notification_uid;
static uint16_t gc_handle;
static gatt_client_notification_t ancs_notification_source_notification;
static gatt_client_notification_t ancs_data_source_notification;
static int ancs_service_found;
static gatt_client_service_t  ancs_service;
static gatt_client_characteristic_t ancs_notification_source_characteristic;
static gatt_client_characteristic_t ancs_control_point_characteristic;
static gatt_client_characteristic_t ancs_data_source_characteristic;
static int ancs_characteristcs;
static tc_state_t tc_state = TC_IDLE;

static ancs_chunk_parser_state_t chunk_parser_state;
static uint8_t  ancs_notification_buffer[50];
static uint16_t ancs_bytes_received;
static uint16_t ancs_bytes_needed;
static uint8_t  ancs_attribute_id;
static uint16_t ancs_attribute_len;

static btstack_packet_handler_t client_handler;
static btstack_packet_callback_registration_t hci_event_callback_registration;

void ancs_client_register_callback(btstack_packet_handler_t handler){
    client_handler = handler; 
}

static void notify_client_text(int event_type){
    if (!client_handler) return;
    uint8_t event[7 + sizeof(ancs_notification_buffer) + 1];
    event[0] = HCI_EVENT_ANCS_META;
    event[1] = 5 + ancs_attribute_len;
    event[2] = event_type;
    little_endian_store_16(event, 3, gc_handle);
    little_endian_store_16(event, 5, ancs_attribute_id);
    memcpy(&event[7], ancs_notification_buffer, ancs_attribute_len);
    // we're nice
    event[7+ancs_attribute_len] = 0;
    (*client_handler)(HCI_EVENT_PACKET, 0, event, event[1] + 2);
}

static void notify_client_simple(int event_type){
    if (!client_handler) return;
    uint8_t event[5];
    event[0] = HCI_EVENT_ANCS_META;
    event[1] = 3;
    event[2] = event_type;
    little_endian_store_16(event, 3, gc_handle);
    (*client_handler)(HCI_EVENT_PACKET, 0, event, sizeof(event));
}

static void ancs_chunk_parser_init(void){
    chunk_parser_state = W4_ATTRIBUTE_ID;
    ancs_bytes_received = 0;
    ancs_bytes_needed = 6;
}

const char * ancs_client_attribute_name_for_id(int id){
    if (id >= ANCS_ATTRBUTE_NAMES_COUNT) return 0;
    return ancs_attribute_names[id];
}

static void ancs_chunk_parser_handle_byte(uint8_t data){
    ancs_notification_buffer[ancs_bytes_received++] = data;
    if (ancs_bytes_received < ancs_bytes_needed) return;
    switch (chunk_parser_state){
        case W4_ATTRIBUTE_ID:
            ancs_attribute_id   = ancs_notification_buffer[ancs_bytes_received-1];
            ancs_bytes_received = 0;
            ancs_bytes_needed   = 2;
            chunk_parser_state  = W4_ATTRIBUTE_LEN;
            break;
        case W4_ATTRIBUTE_LEN:
            ancs_attribute_len  = little_endian_read_16(ancs_notification_buffer, ancs_bytes_received-2);
            ancs_bytes_received = 0;
            ancs_bytes_needed   = ancs_attribute_len;
            if (ancs_attribute_len == 0) {
                ancs_bytes_needed   = 1;
                chunk_parser_state  = W4_ATTRIBUTE_ID;
                break;
            }
            chunk_parser_state  = W4_ATTRIBUTE_COMPLETE;
            break;
        case W4_ATTRIBUTE_COMPLETE:
            ancs_notification_buffer[ancs_bytes_received] = 0;
            notify_client_text(ANCS_SUBEVENT_CLIENT_NOTIFICATION);
            ancs_bytes_received = 0;
            ancs_bytes_needed   = 1;
            chunk_parser_state  = W4_ATTRIBUTE_ID;
            break;
    }
}

static void handle_hci_event(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){

    UNUSED(packet_type); // ok: only hci events
    UNUSED(channel);     // ok: there is no channel
    UNUSED(size);        // ok: fixed format events read from HCI buffer

    int connection_encrypted;

    // handle connect / disconncet events first
    switch (hci_event_packet_get_type(packet)) {
        case HCI_EVENT_LE_META:
            switch (packet[2]) {
                case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
                    gc_handle = little_endian_read_16(packet, 4);
                    log_info("Connection handle 0x%04x, request encryption", gc_handle);

                    // we need to be paired to enable notifications
                    tc_state = TC_W4_ENCRYPTED_CONNECTION;
                    sm_send_security_request(gc_handle);
                    break;
                default:
                    break;
            }
            return;

        case HCI_EVENT_ENCRYPTION_CHANGE: 
            if (gc_handle != little_endian_read_16(packet, 3)) return;
            connection_encrypted = packet[5];
            log_info("Encryption state change: %u", connection_encrypted);
            if (!connection_encrypted) return;
            if (tc_state != TC_W4_ENCRYPTED_CONNECTION) return;

            // let's start
            log_info("\nANCS Client - CONNECTED, discover ANCS service");
            tc_state = TC_W4_SERVICE_RESULT;
            gatt_client_discover_primary_services_by_uuid128(handle_hci_event, gc_handle, ancs_service_uuid);
            return;
            
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            if (hci_event_disconnection_complete_get_connection_handle(packet) != gc_handle) break;
            if (tc_state == TC_SUBSCRIBED){
                notify_client_simple(ANCS_SUBEVENT_CLIENT_DISCONNECTED);
            }
            tc_state = TC_IDLE;
            gc_handle = 0;
            return;

        default:
            break;
    }

    gatt_client_characteristic_t characteristic;
    uint8_t *           value;
    uint16_t            value_handle;
    uint16_t            value_length;

    switch(tc_state){
        case TC_W4_SERVICE_RESULT:
            switch(hci_event_packet_get_type(packet)){
                case GATT_EVENT_SERVICE_QUERY_RESULT:
                    gatt_event_service_query_result_get_service(packet, &ancs_service);
                    ancs_service_found = 1;
                    break;
                case GATT_EVENT_QUERY_COMPLETE:
                    if (!ancs_service_found){
                        log_info("ANCS Service not found");//识别安卓设备
                        tc_state = TC_IDLE;
						hids_device_init(0, hid_descriptor_keyboard_boot_mode, sizeof(hid_descriptor_keyboard_boot_mode));
                        break;
                    }
                    tc_state = TC_W4_CHARACTERISTIC_RESULT;
                    log_info("ANCS Client - Discover characteristics for ANCS SERVICE ");//识别iOS设备
                    gatt_client_discover_characteristics_for_service(handle_hci_event, gc_handle, &ancs_service);
//					hids_device_init(0, hid_descriptor_keyboard_boot_mode, sizeof(hid_descriptor_keyboard_boot_mode));
                    break;
                default:
                    break;
            }
            break;
            
        case TC_W4_CHARACTERISTIC_RESULT:
            switch(hci_event_packet_get_type(packet)){
                case GATT_EVENT_CHARACTERISTIC_QUERY_RESULT:
                    gatt_event_characteristic_query_result_get_characteristic(packet, &characteristic);
                    if (memcmp(characteristic.uuid128, ancs_notification_source_uuid, 16) == 0){
                        log_info("ANCS Notification Source found, attribute handle %u", characteristic.value_handle);
                        ancs_notification_source_characteristic = characteristic;
                        ancs_characteristcs++;
                        break;                        
                    }
                    if (memcmp(characteristic.uuid128, ancs_control_point_uuid, 16) == 0){
                        log_info("ANCS Control Point found, attribute handle %u", characteristic.value_handle);
                        ancs_control_point_characteristic = characteristic;
                        ancs_characteristcs++;
                        break;                        
                    }
                    if (memcmp(characteristic.uuid128, ancs_data_source_uuid, 16) == 0){
                        log_info("ANCS Data Source found, attribute handle %u", characteristic.value_handle);
                        ancs_data_source_characteristic = characteristic;
                        ancs_characteristcs++;
                        break;                        
                    }
                    break;
                case GATT_EVENT_QUERY_COMPLETE:
                    log_info("ANCS Characteristcs count %u", ancs_characteristcs);
                    tc_state = TC_W4_NOTIFICATION_SOURCE_SUBSCRIBED;
                    gatt_client_listen_for_characteristic_value_updates(&ancs_notification_source_notification, &handle_hci_event, gc_handle, &ancs_notification_source_characteristic);
                    gatt_client_write_client_characteristic_configuration(handle_hci_event, gc_handle, &ancs_notification_source_characteristic,
                        GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION);
                    break;
                default:
                    break;
            }
            break;
        case TC_W4_NOTIFICATION_SOURCE_SUBSCRIBED:
            switch(hci_event_packet_get_type(packet)){
                case GATT_EVENT_QUERY_COMPLETE:
                    log_info("ANCS Notification Source subscribed");
                    tc_state = TC_W4_DATA_SOURCE_SUBSCRIBED;
                    gatt_client_listen_for_characteristic_value_updates(&ancs_data_source_notification, &handle_hci_event, gc_handle, &ancs_data_source_characteristic);
                    gatt_client_write_client_characteristic_configuration(handle_hci_event, gc_handle, &ancs_data_source_characteristic,
                        GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION);
                    break;
                default:
                    break;
            }
            break;
        case TC_W4_DATA_SOURCE_SUBSCRIBED:
            switch(hci_event_packet_get_type(packet)){
                case GATT_EVENT_QUERY_COMPLETE:
                    log_info("ANCS Data Source subscribed");
                    tc_state = TC_SUBSCRIBED;
                    notify_client_simple(ANCS_SUBEVENT_CLIENT_CONNECTED);
                    break;
                default:
                    break;
            }
            break;
        case TC_SUBSCRIBED:
            if (hci_event_packet_get_type(packet) != GATT_EVENT_NOTIFICATION && hci_event_packet_get_type(packet) != GATT_EVENT_INDICATION ) break;

            value_handle = little_endian_read_16(packet, 4);
            value_length = little_endian_read_16(packet, 6);
            value = &packet[8];

            log_info("ANCS Notification, value handle %u", value_handle);

            if (value_handle == ancs_data_source_characteristic.value_handle){
                int i;
                for (i=0;i<value_length;i++) {
                    ancs_chunk_parser_handle_byte(value[i]);
                }
            } else if (value_handle == ancs_notification_source_characteristic.value_handle){
                ancs_notification_uid = little_endian_read_32(value, 4);
                log_info("Notification received: EventID %02x, EventFlags %02x, CategoryID %02x, CategoryCount %u, UID %04x",
                    value[0], value[1], value[2], value[3], (int) ancs_notification_uid);
                static uint8_t get_notification_attributes[] = {0, 0,0,0,0,  0,  1,32,0,  2,32,0, 3,32,0, 4, 5};
                little_endian_store_32(get_notification_attributes, 1, ancs_notification_uid);
                ancs_notification_uid = 0;
                ancs_chunk_parser_init();
                gatt_client_write_value_of_characteristic(handle_hci_event, gc_handle, ancs_control_point_characteristic.value_handle, 
                    sizeof(get_notification_attributes), get_notification_attributes);
            } else {
                log_info("Unknown Source: ");
                log_info_hexdump(value , value_length);
            }
            break;
        default:
            break;
    }    
    // app_run();
}

void ancs_client_init(void){
    hci_event_callback_registration.callback = &handle_hci_event;
    hci_add_event_handler(&hci_event_callback_registration);
}
