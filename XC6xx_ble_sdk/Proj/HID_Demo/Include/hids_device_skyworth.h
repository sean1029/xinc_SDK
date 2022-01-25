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
#ifndef HIDS_DEVICE_SKYWORTH_H
#define HIDS_DEVICE_SKYWORTH_H

#include <stdint.h>
#include "btstack_defines.h"
#include "bluetooth.h"

#if defined __cplusplus
extern "C" {
#endif


#define	APP_HID_ID_1_INPUT_ENDPORT			0
#define	APP_HID_ID_1_OUTPUT_ENDPORT		1
#define APP_HID_ID_2_INPUT_ENDPORT			2
#define APP_HID_ID_43_INPUT_ENDPORT				3
#define APP_HID_ID_93_INPUT_ENDPORT				4
#define APP_HID_ID_94_OUTPUT_ENDPORT			5
#define	APP_HID_ID_95_INPUT_ENDPORT			6


typedef struct{
    uint16_t        con_handle;

    uint8_t         hid_country_code;
    const uint8_t * hid_descriptor;
    uint16_t        hid_descriptor_size;

    uint16_t        hid_report_map_handle;
		uint16_t        hid_report_info_handle;
	  uint8_t         hid_report_info[4];
	
    uint16_t        hid_control_point_value_handle;
    uint8_t         hid_control_point_suspend;
	
    uint16_t        hid_protocol_mode;
    uint16_t        hid_protocol_mode_value_handle;


    uint16_t        hid_boot_keyboard_input_value_handle;
    uint16_t        hid_boot_keyboard_input_client_configuration_handle;
    uint16_t        hid_boot_keyboard_input_client_configuration_value;

    uint16_t        hid_boot_keyboard_output_value_handle;
    uint16_t        hid_boot_keyboard_output_client_configuration_handle;
    uint16_t        hid_boot_keyboard_output_client_configuration_value;
		
    uint16_t        hid_report_1_input_value_handle;
    uint16_t        hid_report_1_input_client_configuration_handle;
    uint16_t        hid_report_1_input_client_configuration_value;
		
		uint16_t        hid_report_1_output_value_handle;
    uint16_t        hid_report_1_output_client_configuration_handle;
    uint16_t        hid_report_1_output_client_configuration_value;
		
    uint16_t        hid_report_2_input_value_handle;
    uint16_t        hid_report_2_input_client_configuration_handle;
    uint16_t        hid_report_2_input_client_configuration_value;
		
		uint16_t        hid_report_43_input_value_handle;
    uint16_t        hid_report_43_input_client_configuration_handle;
    uint16_t        hid_report_43_input_client_configuration_value;
		
		uint16_t        hid_report_93_input_value_handle;
		uint16_t        hid_report_93_input_client_configuration_handle;
		uint16_t        hid_report_93_input_client_configuration_value;

		uint16_t        hid_report_94_output_value_handle;
    uint16_t        hid_report_94_output_client_configuration_handle;
    uint16_t        hid_report_94_output_client_configuration_value;
		uint8_t 				hid_report_94_output_value[20];

		uint16_t        hid_report_95_input_value_handle;
		uint16_t        hid_report_95_input_client_configuration_handle;
		uint16_t        hid_report_95_input_client_configuration_value;
	
		uint16_t        hid_report_handle_idx[7];
		
		uint8_t 				hids_output_report_buff[100];

    btstack_context_callback_registration_t  hids_device_callback;
} hids_device_t;
/**
 * Implementation of the GATT HIDS Device
 * To use with your application, add '#import <hids.gatt>' to your .gatt file
 */

/**
 * @brief Set up HIDS Device
 */
void hids_device_init(uint8_t hid_country_code, const uint8_t * hid_descriptor, uint16_t hid_descriptor_size);

/**
 * @brief Register callback for the HIDS Device client.
 * @param callback
 */
void hids_device_register_packet_handler(btstack_packet_handler_t callback);

/**
 * @brief Request can send now event to send HID Report
 * Generates an HIDS_SUBEVENT_CAN_SEND_NOW subevent
 * @param hid_cid
 */
void hids_device_request_can_send_now_event(hci_con_handle_t con_handle);

void hids_device_request_voice_can_send_now_event(hci_con_handle_t con_handle,uint16_t startOrStop);
/**
 * @brief Send HID Report: Input
 */
void hids_device_send_input_report(hci_con_handle_t con_handle,  uint8_t report_idx,const uint8_t * report, uint16_t report_len);

void hids_device_send_media_report(hci_con_handle_t con_handle,  uint8_t report_idx,const uint8_t * report, uint16_t report_len);

void hids_device_send_keyboard_report(hci_con_handle_t con_handle, uint8_t report_idx,const uint8_t * report, uint16_t report_len);

/**
 * @brief Send HID Report: Output
 */
void hids_device_send_output_report(hci_con_handle_t con_handle, const uint8_t * report, uint16_t report_len);

/**
 * @brief Send HID Report: Feature
 */
void hids_device_send_feature_report(hci_con_handle_t con_handle, const uint8_t * report, uint16_t report_len);

/**
 * @brief Send HID Boot Mouse Input Report
 */
void hids_device_send_boot_mouse_input_report(hci_con_handle_t con_handle, const uint8_t * report, uint16_t report_len);

/**
 * @brief Send HID Boot Mouse Input Report
 */
void hids_device_send_boot_keyboard_input_report(hci_con_handle_t con_handle, const uint8_t * report, uint16_t report_len);


hids_device_t * hids_device_get_instance_for_con_handle(uint16_t con_handle);
#if defined __cplusplus
}
#endif

#endif