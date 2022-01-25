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

#define __BTSTACK_FILE__ "hids_device_skyworth.c"

/**
 * Implementation of the GATT HIDS Device
 * To use with your application, add '#import <hids.gatt>' to your .gatt file
 */

#include "hids_device_skyworth.h"

#include "att_db.h"
#include "att_server.h"
#include "bluetooth_gatt.h"
#include "btstack_util.h"
#include "btstack_debug.h"

#define HIDS_DEVICE_ERROR_CODE_INAPPROPRIATE_CONNECTION_PARAMETERS    0x80


static hids_device_t hids_device;

static btstack_packet_handler_t packet_handler;
static att_service_handler_t hid_service;

// TODO: store hids device connection into list
/*static */ hids_device_t * hids_device_get_instance_for_con_handle(uint16_t con_handle){
    UNUSED(con_handle);
    return &hids_device;
}

static hids_device_t * hids_device_create_instance(void){
    return &hids_device;
}


static void hids_device_emit_event_with_uint8(uint8_t event, hci_con_handle_t con_handle, uint8_t value){
    hids_device_t * instance = hids_device_get_instance_for_con_handle(con_handle);
    if (!instance){
        log_error("no instance for handle 0x%02x", con_handle);
        return;
    }

    if (!packet_handler) return;
    uint8_t buffer[6];
    buffer[0] = HCI_EVENT_HIDS_META;
    buffer[1] = 4;
    buffer[2] = event;
    little_endian_store_16(buffer, 3, (uint16_t) con_handle);
    buffer[5] = value;
    (*packet_handler)(HCI_EVENT_PACKET, 0, buffer, sizeof(buffer));
}

static void hids_device_emit_event_with_buff(uint8_t event, hci_con_handle_t con_handle,uint16_t att_handle,uint16_t offset, uint8_t *buff,uint16_t buffLen){
    hids_device_t * instance = hids_device_get_instance_for_con_handle(con_handle);
    if (!instance){
        log_error("no instance for handle 0x%02x", con_handle);
        return;
    }

    if (!packet_handler) return;
		
		//hids_output_report_buff size small 100

    instance->hids_output_report_buff[0] = HCI_EVENT_HIDS_META;
    instance->hids_output_report_buff[1] = 10;
    instance->hids_output_report_buff[2] = event ;
    little_endian_store_16(instance->hids_output_report_buff, 3, (uint16_t) con_handle);
		little_endian_store_16(instance->hids_output_report_buff, 5, (uint16_t) att_handle);
		little_endian_store_16(instance->hids_output_report_buff, 7, (uint16_t) offset);
		little_endian_store_16(instance->hids_output_report_buff, 9, (uint16_t) buffLen);
		memcpy(&(instance->hids_output_report_buff[11]),buff,buffLen);

    (*packet_handler)(HCI_EVENT_PACKET, 0, instance->hids_output_report_buff, buffLen + 10);
}

static void hids_device_emit_event(uint8_t event, hci_con_handle_t con_handle){
    hids_device_t * instance = hids_device_get_instance_for_con_handle(con_handle);
    if (!instance){
        log_error("no instance for handle 0x%02x", con_handle);
        return;
    }

    if (!packet_handler) return;
    uint8_t buffer[5];
    buffer[0] = HCI_EVENT_HIDS_META;
    buffer[1] = 4;
    buffer[2] = event;
    little_endian_store_16(buffer, 3, (uint16_t) con_handle);
    (*packet_handler)(HCI_EVENT_PACKET, 0, buffer, sizeof(buffer));
}

static void hids_device_can_send_now(void * context){
    hci_con_handle_t con_handle = (hci_con_handle_t) (uintptr_t) context;
    // notify client
    hids_device_t * instance = hids_device_get_instance_for_con_handle(con_handle);
    if (!instance){
        log_error("no instance for handle 0x%02x", con_handle);
        return;
    }

    if (!packet_handler) return;
    uint8_t buffer[5];
    buffer[0] = HCI_EVENT_HIDS_META;
    buffer[1] = 3;
    buffer[2] = HIDS_SUBEVENT_CAN_SEND_NOW;
    little_endian_store_16(buffer, 3, (uint16_t) con_handle);
    (*packet_handler)(HCI_EVENT_PACKET, 0, buffer, sizeof(buffer));
}

static void hids_device_voice_can_send_now(void * context){
	
		uint32_t tmpcontext = (uintptr_t) context ;
    hci_con_handle_t con_handle = (hci_con_handle_t) tmpcontext & 0xFFFFF;
		uint16_t startOrStop = (tmpcontext >> 16)  & 0xFFFFF;
    // notify client
    hids_device_t * instance = hids_device_get_instance_for_con_handle(con_handle);
    if (!instance){
        log_error("no instance for handle 0x%02x", con_handle);
        return;
    }
//		printf("voice_can_send_now con_handle:0x%x, startOrStop:%d\r\n",con_handle,startOrStop);
    if (!packet_handler) return;
    uint8_t buffer[7];
    buffer[0] = HCI_EVENT_HIDS_META;
    buffer[1] = 3;
    buffer[2] = HIDS_SUBEVENT_VOICE_CAN_SEND_NOW;
    little_endian_store_16(buffer, 3, (uint16_t) con_handle);
		little_endian_store_16(buffer, 5, (uint16_t) startOrStop);
    (*packet_handler)(HCI_EVENT_PACKET, 0, buffer, sizeof(buffer));
}



// ATT Client Read Callback for Dynamic Data
// - if buffer == NULL, don't copy data, just return size of value
// - if buffer != NULL, copy data and return number bytes copied
static uint16_t att_read_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size){
    hids_device_t * instance = hids_device_get_instance_for_con_handle(con_handle);
    if (!instance){
        printf("no instance for handle 0x%02x", con_handle);
        return HIDS_DEVICE_ERROR_CODE_INAPPROPRIATE_CONNECTION_PARAMETERS;
    }

    if (att_handle == instance->hid_protocol_mode_value_handle){
        printf("Read protocol mode");
        return att_read_callback_handle_byte(instance->hid_protocol_mode, offset, buffer, buffer_size);
    }
    
    if (att_handle == instance->hid_report_map_handle){
        printf("Read report map");
        return att_read_callback_handle_blob(instance->hid_descriptor, instance->hid_descriptor_size, offset, buffer, buffer_size);
    }
		
		 if (att_handle == instance->hid_report_info_handle){
        printf("Read report info");
        return att_read_callback_handle_blob(instance->hid_report_info, 4, offset, buffer, buffer_size);
    }
		 
		if (att_handle == instance->hid_boot_keyboard_input_client_configuration_handle){
        return att_read_callback_handle_little_endian_16(instance->hid_boot_keyboard_input_client_configuration_handle, offset, buffer, buffer_size);
    }
		
		if (att_handle == instance->hid_boot_keyboard_output_client_configuration_handle){
        return att_read_callback_handle_little_endian_16(instance->hid_boot_keyboard_output_client_configuration_handle, offset, buffer, buffer_size);
    }
		 

    if (att_handle == instance->hid_report_1_input_client_configuration_handle){
        return att_read_callback_handle_little_endian_16(instance->hid_report_1_input_client_configuration_value, offset, buffer, buffer_size);
    }
	

    
    if (att_handle == instance->hid_report_1_output_client_configuration_handle){
        return att_read_callback_handle_little_endian_16(instance->hid_report_1_output_client_configuration_value, offset, buffer, buffer_size);
    }
		
		    if (att_handle == instance->hid_report_2_input_client_configuration_handle){
        return att_read_callback_handle_little_endian_16(instance->hid_report_2_input_client_configuration_value, offset, buffer, buffer_size);
    }
		
    if (att_handle == instance->hid_report_43_input_client_configuration_handle){
        return att_read_callback_handle_little_endian_16(instance->hid_report_43_input_client_configuration_value, offset, buffer, buffer_size);
    }

    if (att_handle == instance->hid_report_93_input_client_configuration_handle){
        return att_read_callback_handle_little_endian_16(instance->hid_report_93_input_client_configuration_value, offset, buffer, buffer_size);
    }
		
		if (att_handle == instance->hid_report_94_output_client_configuration_handle){
        return att_read_callback_handle_little_endian_16(instance->hid_report_94_output_client_configuration_value, offset, buffer, buffer_size);
    }
				
		if (att_handle == instance->hid_report_95_input_client_configuration_handle){
        return att_read_callback_handle_little_endian_16(instance->hid_report_95_input_client_configuration_value, offset, buffer, buffer_size);
    }
		
    if (att_handle == instance->hid_control_point_value_handle){
        if (buffer && buffer_size >= 1){
            buffer[0] = instance->hid_control_point_suspend;
        } 
        return 1;
    }
    return 0;
}

static int att_write_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size){
    UNUSED(transaction_mode);
    UNUSED(buffer_size);
    UNUSED(offset);

    hids_device_t * instance = hids_device_get_instance_for_con_handle(con_handle);
    if (!instance){
        log_error("no instance for handle 0x%02x", con_handle);
        return HIDS_DEVICE_ERROR_CODE_INAPPROPRIATE_CONNECTION_PARAMETERS;
    }

    if (att_handle == instance->hid_boot_keyboard_input_client_configuration_handle){
        uint16_t new_value = little_endian_read_16(buffer, 0);
        instance->hid_boot_keyboard_input_client_configuration_handle = new_value;
        hids_device_emit_event_with_uint8(HIDS_SUBEVENT_BOOT_MOUSE_INPUT_REPORT_ENABLE, con_handle, new_value);
    }
    if (att_handle == instance->hid_boot_keyboard_input_client_configuration_handle){
        uint16_t new_value = little_endian_read_16(buffer, 0);
        instance->hid_boot_keyboard_input_client_configuration_value = new_value;
        hids_device_emit_event_with_uint8(HIDS_SUBEVENT_BOOT_MOUSE_INPUT_REPORT_ENABLE, con_handle, new_value);
    }
    if (att_handle == instance->hid_report_1_input_client_configuration_handle){
        uint16_t new_value = little_endian_read_16(buffer, 0);
        instance->hid_report_1_input_client_configuration_value = new_value;
        log_info("Enable Report 1 Input notifications: %x", new_value);
        hids_device_emit_event_with_uint8(HIDS_SUBEVENT_INPUT_REPORT_ENABLE, con_handle, new_value);
    }
		if (att_handle == instance->hid_report_1_output_client_configuration_handle){
        uint16_t new_value = little_endian_read_16(buffer, 0);
        instance->hid_report_1_output_client_configuration_value = new_value;
        log_info("Enable Report 1 Output notifications: %x", new_value);
        hids_device_emit_event_with_uint8(HIDS_SUBEVENT_OUTPUT_REPORT_ENABLE, con_handle, new_value);
    }
		if (att_handle == instance->hid_report_2_input_client_configuration_handle){
        uint16_t new_value = little_endian_read_16(buffer, 0);
        instance->hid_report_2_input_client_configuration_value = new_value;
        log_info("Enable Report 2 Input notifications: %x", new_value);
        hids_device_emit_event_with_uint8(HIDS_SUBEVENT_INPUT_REPORT_ENABLE, con_handle, new_value);
    }
		
	  if (att_handle == instance->hid_report_43_input_client_configuration_handle){
        uint16_t new_value = little_endian_read_16(buffer, 0);
        instance->hid_report_43_input_client_configuration_value = new_value;
        log_info("Enable Report 43 Input notifications1: %x", new_value);
        hids_device_emit_event_with_uint8(HIDS_SUBEVENT_INPUT_REPORT_ENABLE, con_handle, new_value);
    }

	  if (att_handle == instance->hid_report_93_input_client_configuration_handle){
        uint16_t new_value = little_endian_read_16(buffer, 0);
        instance->hid_report_93_input_client_configuration_value = new_value;
        log_info("Enable Report 93 Input notifications1: %x", new_value);
        hids_device_emit_event_with_uint8(HIDS_SUBEVENT_INPUT_REPORT_ENABLE, con_handle, new_value);
    }
		
	  if (att_handle == instance->hid_report_94_output_client_configuration_handle){
        uint16_t new_value = little_endian_read_16(buffer, 0);
        instance->hid_report_94_output_client_configuration_value = new_value;
        log_info("Enable Report 94 Output notifications1: %x", new_value);
        hids_device_emit_event_with_uint8(HIDS_SUBEVENT_OUTPUT_REPORT_ENABLE, con_handle, new_value);
    }		
	  if (att_handle == instance->hid_report_95_input_client_configuration_handle){
        uint16_t new_value = little_endian_read_16(buffer, 0);
        instance->hid_report_95_input_client_configuration_value = new_value;
        log_info("Enable Report 95 Input notifications1: %x", new_value);
        hids_device_emit_event_with_uint8(HIDS_SUBEVENT_INPUT_REPORT_ENABLE, con_handle, new_value);
    }
		
		
    if (att_handle == instance->hid_report_94_output_value_handle){
      //  instance->hid_protocol_mode = buffer[0];
				memcpy(instance->hid_report_94_output_value,buffer,buffer_size);
				printf("hid handle:%d write:",att_handle);
			for(int i = 0;i < buffer_size;i ++)
			{
				printf("0x%02x ",buffer[i]);
			}printf("\r\n");
        hids_device_emit_event_with_buff(HIDS_SUBEVENT_OUTPUT_REPORT_DATA, con_handle, att_handle,offset,buffer,buffer_size);
    }
    
		if (att_handle == instance->hid_protocol_mode_value_handle){
        instance->hid_protocol_mode = buffer[0];
        log_info("Set protocol mode: %u", instance->hid_protocol_mode);
        hids_device_emit_event_with_uint8(HIDS_SUBEVENT_PROTOCOL_MODE, con_handle, instance->hid_protocol_mode);
    }
    
		
    if (att_handle == instance->hid_control_point_value_handle){
        if (buffer_size < 1){
            return ATT_ERROR_INVALID_OFFSET;
        }
        instance->hid_control_point_suspend = buffer[0];
        instance->con_handle = con_handle;
        log_info("Set suspend tp: %u", instance->hid_control_point_suspend );
        if (instance->hid_control_point_suspend == 0){
            hids_device_emit_event(HIDS_SUBEVENT_SUSPEND, con_handle);
        } else if (instance->hid_control_point_suspend == 1){ 
            hids_device_emit_event(HIDS_SUBEVENT_EXIT_SUSPEND, con_handle);
        }
    }
    return 0;
}

/**
 * @brief Set up HIDS Device
 */
void hids_device_init(uint8_t country_code, const uint8_t * descriptor, uint16_t descriptor_size){
    hids_device_t * instance = hids_device_create_instance();
    if (!instance){
        log_error("hids_device_init: instance could not be created, not enough memory");
        return;
    }

    instance->hid_country_code    = country_code;
    instance->hid_descriptor      = descriptor;
    instance->hid_descriptor_size = descriptor_size;

    // default
    instance->hid_protocol_mode   = 1;

    // get service handle range
    uint16_t start_handle = 0;
    uint16_t end_handle   = 0xfff;

    int service_found = gatt_server_get_get_handle_range_for_service_with_uuid16(ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE, &start_handle, &end_handle);
    if (!service_found) return;

    // get report map handle
    instance->hid_report_map_handle                               = gatt_server_get_value_handle_for_characteristic_with_uuid16(start_handle, end_handle, ORG_BLUETOOTH_CHARACTERISTIC_REPORT_MAP);

		    // get report info handle
    instance->hid_report_info_handle                               = gatt_server_get_value_handle_for_characteristic_with_uuid16(start_handle, end_handle, ORG_BLUETOOTH_CHARACTERISTIC_HID_INFORMATION);

    // get report map handle
    instance->hid_protocol_mode_value_handle                      = gatt_server_get_value_handle_for_characteristic_with_uuid16(start_handle, end_handle, ORG_BLUETOOTH_CHARACTERISTIC_PROTOCOL_MODE);

    // get value and client configuration handles for boot mouse input, boot keyboard input and report input
    instance->hid_boot_keyboard_input_value_handle                   = gatt_server_get_value_handle_for_characteristic_with_uuid16(start_handle, end_handle, ORG_BLUETOOTH_CHARACTERISTIC_BOOT_KEYBOARD_INPUT_REPORT);
    instance->hid_boot_keyboard_input_client_configuration_handle    = gatt_server_get_client_configuration_handle_for_characteristic_with_uuid16(start_handle, end_handle, ORG_BLUETOOTH_CHARACTERISTIC_BOOT_KEYBOARD_INPUT_REPORT);

    instance->hid_boot_keyboard_output_value_handle                = gatt_server_get_value_handle_for_characteristic_with_uuid16(start_handle, end_handle, ORG_BLUETOOTH_CHARACTERISTIC_BOOT_KEYBOARD_OUTPUT_REPORT);
    instance->hid_boot_keyboard_output_client_configuration_handle = gatt_server_get_client_configuration_handle_for_characteristic_with_uuid16(start_handle, end_handle, ORG_BLUETOOTH_CHARACTERISTIC_BOOT_KEYBOARD_OUTPUT_REPORT);

    instance->hid_report_1_input_value_handle                       = gatt_server_get_value_handle_for_characteristic_with_uuid16(start_handle, end_handle, ORG_BLUETOOTH_CHARACTERISTIC_REPORT);
    instance->hid_report_1_input_client_configuration_handle        = gatt_server_get_client_configuration_handle_for_characteristic_with_uuid16(start_handle, end_handle, ORG_BLUETOOTH_CHARACTERISTIC_REPORT);
		instance->hid_report_handle_idx[APP_HID_ID_1_INPUT_ENDPORT] = instance->hid_report_1_input_value_handle;
		
		instance->hid_report_1_output_value_handle                      = gatt_server_get_value_handle_for_characteristic_with_uuid16(instance->hid_report_1_input_client_configuration_handle+1, end_handle, ORG_BLUETOOTH_CHARACTERISTIC_REPORT);
		instance->hid_report_1_output_client_configuration_handle       = gatt_server_get_client_configuration_handle_for_characteristic_with_uuid16(instance->hid_report_1_input_client_configuration_handle, end_handle, ORG_BLUETOOTH_CHARACTERISTIC_REPORT);
		instance->hid_report_handle_idx[APP_HID_ID_1_OUTPUT_ENDPORT] = instance->hid_report_1_output_value_handle;
		
		instance->hid_report_2_input_value_handle                       = gatt_server_get_value_handle_for_characteristic_with_uuid16(instance->hid_report_1_output_client_configuration_handle + 1, end_handle, ORG_BLUETOOTH_CHARACTERISTIC_REPORT);
    instance->hid_report_2_input_client_configuration_handle        = gatt_server_get_client_configuration_handle_for_characteristic_with_uuid16(instance->hid_report_1_output_client_configuration_handle, end_handle, ORG_BLUETOOTH_CHARACTERISTIC_REPORT);
		instance->hid_report_handle_idx[APP_HID_ID_2_INPUT_ENDPORT] = instance->hid_report_2_input_value_handle;
		
		
		instance->hid_report_43_input_value_handle                       = gatt_server_get_value_handle_for_characteristic_with_uuid16(instance->hid_report_2_input_client_configuration_handle+1, end_handle, ORG_BLUETOOTH_CHARACTERISTIC_REPORT);
		instance->hid_report_43_input_client_configuration_handle        = gatt_server_get_client_configuration_handle_for_characteristic_with_uuid16(instance->hid_report_2_input_client_configuration_handle, end_handle, ORG_BLUETOOTH_CHARACTERISTIC_REPORT);
		instance->hid_report_handle_idx[APP_HID_ID_43_INPUT_ENDPORT] = instance->hid_report_43_input_value_handle;
		
		instance->hid_report_93_input_value_handle                       = gatt_server_get_value_handle_for_characteristic_with_uuid16(instance->hid_report_43_input_client_configuration_handle+1, end_handle, ORG_BLUETOOTH_CHARACTERISTIC_REPORT);
		instance->hid_report_93_input_client_configuration_handle        = gatt_server_get_client_configuration_handle_for_characteristic_with_uuid16(instance->hid_report_43_input_client_configuration_handle, end_handle, ORG_BLUETOOTH_CHARACTERISTIC_REPORT);
		instance->hid_report_handle_idx[APP_HID_ID_93_INPUT_ENDPORT] = instance->hid_report_93_input_value_handle;

		instance->hid_report_94_output_value_handle                      = gatt_server_get_value_handle_for_characteristic_with_uuid16(instance->hid_report_93_input_client_configuration_handle+1, end_handle, ORG_BLUETOOTH_CHARACTERISTIC_REPORT);
		instance->hid_report_94_output_client_configuration_handle       = gatt_server_get_client_configuration_handle_for_characteristic_with_uuid16(instance->hid_report_93_input_client_configuration_handle, end_handle, ORG_BLUETOOTH_CHARACTERISTIC_REPORT);
		instance->hid_report_handle_idx[APP_HID_ID_94_OUTPUT_ENDPORT] = instance->hid_report_94_output_value_handle;
		
		instance->hid_report_95_input_value_handle                       = gatt_server_get_value_handle_for_characteristic_with_uuid16(instance->hid_report_94_output_client_configuration_handle+1, end_handle, ORG_BLUETOOTH_CHARACTERISTIC_REPORT);
		instance->hid_report_95_input_client_configuration_handle        = gatt_server_get_client_configuration_handle_for_characteristic_with_uuid16(instance->hid_report_94_output_client_configuration_handle, end_handle, ORG_BLUETOOTH_CHARACTERISTIC_REPORT);
		instance->hid_report_handle_idx[APP_HID_ID_95_INPUT_ENDPORT] = instance->hid_report_95_input_value_handle;
		

		instance->hid_control_point_value_handle = gatt_server_get_value_handle_for_characteristic_with_uuid16(start_handle, end_handle, ORG_BLUETOOTH_CHARACTERISTIC_HID_CONTROL_POINT);
    
		log_info("hid_report_map_handle                               0x%02x", instance->hid_report_map_handle);
		log_info("hid_protocol_mode_value_handle                      0x%02x", instance->hid_protocol_mode_value_handle);

		log_info("hid_report_1_input_value_handle                       0x%02x", instance->hid_report_1_input_value_handle);
		log_info("hid_report_1_input_client_configuration_handle        0x%02x", instance->hid_report_1_input_client_configuration_handle);

		log_info("hid_report_1_output_value_handle                       0x%02x", instance->hid_report_1_output_value_handle);			
		log_info("hid_report_1_output_client_configuration_handle        0x%02x", instance->hid_report_1_output_client_configuration_handle);

		log_info("hid_report_fd_input_value_handle1                       0x%02x", instance->hid_report_2_input_value_handle);
		log_info("hid_report_fd_input_client_configuration_handle1        0x%02x", instance->hid_report_2_input_client_configuration_handle);

		log_info("hid_report_1e_input_value_handle1                       0x%02x", instance->hid_report_43_input_value_handle);
		log_info("hid_report_1e_input_client_configuration_handle1        0x%02x", instance->hid_report_43_input_client_configuration_handle);

		log_info("hid_report_1e_output_value_handle                       0x%02x", instance->hid_report_94_output_value_handle);			
		log_info("hid_report_1e_output_client_configuration_handle        0x%02x", instance->hid_report_94_output_client_configuration_handle);

		log_info("hid_report_3_input_value_handle1                       0x%02x", instance->hid_report_93_input_value_handle);
		log_info("hid_report_3_input_client_configuration_handle1        0x%02x", instance->hid_report_93_input_client_configuration_handle);

		log_info("hid_report_50_input_value_handle1                       0x%02x", instance->hid_report_95_input_value_handle);
		log_info("hid_report_50_input_client_configuration_handle1        0x%02x", instance->hid_report_95_input_client_configuration_handle);

		log_info("hid_control_point_value_handle                      0x%02x", instance->hid_control_point_value_handle);
		
		
		
		printf("hid_report_map_handle:%d\r\n", instance->hid_report_map_handle);
		printf("hid_report_info_handle:%d\r\n", instance->hid_report_info_handle);
		printf("hid_control_point_value_handle:%d\r\n", instance->hid_control_point_value_handle);
		printf("hid_protocol_mode_value_handle:%d\r\n", instance->hid_protocol_mode_value_handle);

		printf("hid_report_1_input_value_handle:%d\r\n", instance->hid_report_1_input_value_handle);
		printf("hid_report_1_input_client_configuration_handle:%d\r\n", instance->hid_report_1_input_client_configuration_handle);

		printf("hid_report_1_output_value_handle:%d\r\n", instance->hid_report_1_output_value_handle);			
		printf("hid_report_1_output_client_configuration_handle:%d\r\n", instance->hid_report_1_output_client_configuration_handle);

		printf("hid_report_2_input_value_handle1:%d\r\n", instance->hid_report_2_input_value_handle);
		printf("hid_report_2_input_client_configuration_handle:%d\r\n", instance->hid_report_2_input_client_configuration_handle);

		printf("hid_report_43_input_value_handle1:%d\r\n", instance->hid_report_43_input_value_handle);
		printf("hid_report_43_input_client_configuration_handle:%d\r\n", instance->hid_report_43_input_client_configuration_handle);

		printf("hid_report_93_input_value_handle1:%d\r\n", instance->hid_report_93_input_value_handle);
		printf("hid_report_93_input_client_configuration_handle:%d\r\n", instance->hid_report_93_input_client_configuration_handle);

		printf("hid_report_94_output_value_handle:%d\r\n", instance->hid_report_94_output_value_handle);			
		printf("hid_report_94_output_client_configuration_handle:%d\r\n", instance->hid_report_94_output_client_configuration_handle);


		printf("hid_report_95_input_value_handle1:%d\r\nx", instance->hid_report_95_input_value_handle);
		printf("hid_report_95_input_client_configuration_handle:%d\r\n", instance->hid_report_95_input_client_configuration_handle);



    // register service with ATT Server
    hid_service.start_handle   = start_handle;
    hid_service.end_handle     = end_handle;
    hid_service.read_callback  = &att_read_callback;
    hid_service.write_callback = &att_write_callback;
    att_server_register_service_handler(&hid_service);
}

/**
 * @brief Register callback for the HIDS Device client.
 * @param callback
 */
void hids_device_register_packet_handler(btstack_packet_handler_t callback){
    packet_handler = callback;
}

/**
 * @brief Request can send now event to send HID Report
 * Generates an HIDS_SUBEVENT_CAN_SEND_NOW subevent
 * @param hid_cid
 */
void hids_device_request_can_send_now_event(hci_con_handle_t con_handle){
    hids_device_t * instance = hids_device_get_instance_for_con_handle(con_handle);
    if (!instance){
        log_error("no instance for handle 0x%02x", con_handle);
        return;
    }

    instance->hids_device_callback.callback = &hids_device_can_send_now;
    instance->hids_device_callback.context  = (void*) (uintptr_t) con_handle;
    att_server_register_can_send_now_callback(&instance->hids_device_callback, con_handle);
}

/**
 * @brief Request can send now event to send HID Report
 * Generates an HIDS_SUBEVENT_CAN_SEND_NOW subevent
 * @param hid_cid
 */
void hids_device_request_voice_can_send_now_event(hci_con_handle_t con_handle,uint16_t startOrStop){
    hids_device_t * instance = hids_device_get_instance_for_con_handle(con_handle);
    if (!instance){
        log_error("no instance for handle 0x%02x", con_handle);
        return;
    }
		uint32_t tmp_context = con_handle | (startOrStop << 16);
    instance->hids_device_callback.callback = &hids_device_voice_can_send_now;
    instance->hids_device_callback.context  = (void*) (uintptr_t) tmp_context;
    att_server_register_can_send_now_callback(&instance->hids_device_callback, con_handle);
}



/**
 * @brief Send HID Report: Input
 */
void hids_device_send_keyboard_report(hci_con_handle_t con_handle, uint8_t report_idx,const uint8_t * report, uint16_t report_len){
    hids_device_t * instance = hids_device_get_instance_for_con_handle(con_handle);
    if (!instance){
        log_error("no instance for handle 0x%02x", con_handle);
        return;
    }
		uint16_t att_handle = instance->hid_report_handle_idx[report_idx];
//		printf("keyboard att_server_notify handle:%d\r\n",att_handle);
    att_server_notify(con_handle, att_handle, report, report_len);
}


/**
 * @brief Send HID Report: Input
 */
void hids_device_send_keyboard_report_req(hci_con_handle_t con_handle, uint8_t report_idx,const uint8_t * report, uint16_t report_len){
    hids_device_t * instance = hids_device_get_instance_for_con_handle(con_handle);
    if (!instance){
        log_error("no instance for handle 0x%02x", con_handle);
        return;
    }
		uint16_t att_handle = instance->hid_report_handle_idx[report_idx];
		printf("keyboard att_server_notify handle:%d\r\n",att_handle);
    att_server_notify(con_handle, att_handle, report, report_len);
}



void hids_device_send_media_report(hci_con_handle_t con_handle, uint8_t report_idx,const uint8_t * report, uint16_t report_len){
    hids_device_t * instance = hids_device_get_instance_for_con_handle(con_handle);
    if (!instance){
        log_error("no instance for handle 0x%02x", con_handle);
        return;
    }
		uint16_t att_handle = instance->hid_report_handle_idx[report_idx];
		printf("media att_server_notify handle:%d\r\n",att_handle);
    att_server_notify(con_handle, att_handle, report, report_len);
}


/**
 * @brief Send HID Report: Output
 */
void hids_device_send_output_report(hci_con_handle_t con_handle, const uint8_t * report, uint16_t report_len){
    hids_device_t * instance = hids_device_get_instance_for_con_handle(con_handle);
    if (!instance){
        log_error("no instance for handle 0x%02x", con_handle);
        return;
    }
    att_server_notify(con_handle, instance->hid_report_1_output_value_handle, report, report_len);
}

/**
 * @brief Send HID Report: Feature
 */
void hids_device_send_feature_report(hci_con_handle_t con_handle, const uint8_t * report, uint16_t report_len){
    hids_device_t * instance = hids_device_get_instance_for_con_handle(con_handle);
    if (!instance){
        log_error("no instance for handle 0x%02x", con_handle);
        return;
    }
    att_server_notify(con_handle, instance->hid_report_1_output_value_handle, report, report_len);
}

/**
 * @brief Send HID Boot Keyboard Input Report
 */
void hids_device_send_boot_keyboard_input_report(hci_con_handle_t con_handle, const uint8_t * report, uint16_t report_len){
    hids_device_t * instance = hids_device_get_instance_for_con_handle(con_handle);
    if (!instance){
        log_error("no instance for handle 0x%02x", con_handle);
        return;
    }
    att_server_notify(con_handle, instance->hid_boot_keyboard_input_value_handle, report, report_len);
}

/**
 * @brief Send HID Boot Keyboard Output Report
 */
void hids_device_send_boot_keyboard_output_report(hci_con_handle_t con_handle, const uint8_t * report, uint16_t report_len){
    hids_device_t * instance = hids_device_get_instance_for_con_handle(con_handle);
    if (!instance){
        log_error("no instance for handle 0x%02x", con_handle);
        return;
    }
    att_server_notify(con_handle, instance->hid_boot_keyboard_output_value_handle, report, report_len);
}
