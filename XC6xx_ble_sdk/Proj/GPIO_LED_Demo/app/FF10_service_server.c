/*
 * Copyright (C) 2022 XINCHIP
 * @DATA 2022.01.13
 * @AUTH feng.Cheng
 */
 
#define __BTSTACK_FILE__ "FF10_service_server.c"



#include <string.h>
#include <stdio.h>
#include "btstack_defines.h"
#include "att_db.h"
#include "att_server.h"
#include "btstack_util.h"
#include "bluetooth_gatt.h"
#include "FF10_service_server.h"
 
#define CUSTOMER_VALUE_LEN  20


static uint8_t 	customer_value_A[CUSTOMER_VALUE_LEN];
static uint8_t 	customer_value_B[CUSTOMER_VALUE_LEN];


uint8_t g_test_value = 0;

typedef struct{
		uint16_t        con_handle;


		hci_con_handle_t customer_valueB_client_configuration_connection;

		uint16_t customer_valueA_handle_value;
		uint16_t customer_valueA_handle_client_configuration;
    uint16_t customer_valueB_handle_value;
	
		btstack_context_callback_registration_t  customer_callback;
	
}customer_ff10s_t;

static customer_ff10s_t cust_server;


static att_service_handler_t       customer_service;
static btstack_packet_handler_t packet_handler;

// TODO: store customer connection into list
static customer_ff10s_t * customer_server_get_instance_for_con_handle(uint16_t con_handle){
    UNUSED(con_handle);
    return &cust_server;
}

static customer_ff10s_t * customer_server_create_instance(void){
    return &cust_server;
}


//static uint8_t read_buf[10] = {0};
// read requests
static uint16_t att_read_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size){

    printf("ff10 %s, con_handle=%x, att_handle=%x, offset=%x, buffer=%x, size=%x\n",__func__, con_handle, att_handle, offset,(uint32_t)buffer, buffer_size);
    
//    if((att_handle != ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE) && 
//       (att_handle != ATT_CHARACTERISTIC_0000FF12_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE)) return 0;
    
    static uint8_t read_buf[10] = {0};
    return att_read_callback_handle_blob((const uint8_t *)read_buf, sizeof(read_buf), offset, buffer, buffer_size);
}

// write requests
static int att_write_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size){
    
    uint32_t le_notification_enabled;
    printf("ff10 %s, con_handle=%x, att_handle=%x, offset=%x, buffer=%x, size=%x\n", __func__, con_handle, att_handle, offset, (uint32_t)buffer, buffer_size);  
    if (transaction_mode != ATT_TRANSACTION_MODE_NONE) return 0;
	
	  customer_ff10s_t * instance = customer_server_get_instance_for_con_handle(con_handle);
    if (!instance){
        printf("no instance for handle 0x%02x", con_handle);
        return -1;
    }
		
    
    if(  att_handle ==   instance->customer_valueA_handle_client_configuration)
		{
			 le_notification_enabled = little_endian_read_16(buffer, 0) == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION;
       printf("Notifications enabled %u\n",le_notification_enabled); 
		}
        
		if(  att_handle ==   instance->customer_valueA_handle_value)
		{
			le_notification_enabled = little_endian_read_16(buffer, 0) == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION;
			printf("att_handle=0x%x, offset=0x%x, length=0x%x\n", att_handle, offset, buffer_size);
			printf("att data: ");
			for(uint32_t i=0; i<buffer_size; i++) printf("0x%x ", buffer[i]);
			printf("\n");
		}

		if(  att_handle ==   instance->customer_valueB_handle_value)
		{
			printf("att_handle=0x%x, offset=0x%x, length=0x%x\n", att_handle, offset, buffer_size);
			printf("att data: ");
			for(uint32_t i=0; i<buffer_size; i++) printf("0x%x ", buffer[i]);
			printf("\n");
			memcpy(customer_value_B,buffer,buffer_size);
			if(buffer[0] == 0x55)
			{
				buffer[0]++;
				ff11_char_send_report(con_handle,buffer, buffer_size);
			}
		}

     
    return 0;
} 
/**
 * @brief Set up FF10 Server
 */
int gatt_server_get_get_handle_range_for_service_with_uuid128(const uint8_t * uuid128, uint16_t * start_handle, uint16_t * end_handle);

uint8_t ff10_server[] = CUST_SERVER_0000FF10_0000_1000_8000_00805F9B34FB;
uint8_t ff11_char[] = CUST_CHAR_0000FF11_0000_1000_8000_00805F9B34FB;
uint8_t ff12_char[] = CUST_CHAR_0000FF12_0000_1000_8000_00805F9B34FB;
uint8_t buf[200] ;
void ff10_service_server_init(void){
    customer_ff10s_t * instance = customer_server_create_instance();
    if (!instance){
        printf("ff10_service: instance could not be created, not enough memory");
        return;
    }
		 printf("ff10_service_server_init\r\n");
		    // get service handle range
    uint16_t start_handle = 0;
    uint16_t end_handle   = 0xfff;
		
		memset(buf,0,20);
		memset(customer_value_B,0,20);
		
		
		 int service_found = gatt_server_get_get_handle_range_for_service_with_uuid128(ff10_server, &start_handle, &end_handle);

		 printf("service_found1: %d\r\n",service_found);

    if (!service_found) return;
		instance->customer_valueA_handle_value = gatt_server_get_value_handle_for_characteristic_with_uuid128(start_handle, end_handle, ff11_char);
		
		instance->customer_valueA_handle_client_configuration =  gatt_server_get_client_configuration_handle_for_characteristic_with_uuid128(start_handle, end_handle, ff11_char);
		
		instance->customer_valueB_handle_value = gatt_server_get_value_handle_for_characteristic_with_uuid128(start_handle, end_handle, ff12_char);

		printf("valueA_handle_value: 0x%x,conf_handle:0x%x\r\n",instance->customer_valueA_handle_value,instance->customer_valueA_handle_client_configuration);
		printf("valueB_handle: 0x%x\r\n",instance->customer_valueB_handle_value);
    // register service with ATT Server
    customer_service.start_handle   = start_handle;
    customer_service.end_handle     = end_handle;
    customer_service.read_callback  = &att_read_callback;
    customer_service.write_callback = &att_write_callback;
    att_server_register_service_handler(&customer_service);
	}

	static void ff11_can_send_now(void * context){
		
		
		buf[0]++;
		hci_con_handle_t con_handle = (hci_con_handle_t) (uintptr_t) context;
		customer_ff10s_t * instance = customer_server_get_instance_for_con_handle(con_handle);
    att_server_notify(con_handle, instance->customer_valueA_handle_value, buf, 100);
	}
	/**
 * @brief Send FF11 Report
 */
void ff11_char_send_report(hci_con_handle_t con_handle, const uint8_t * report, uint16_t report_len){
    customer_ff10s_t * instance = customer_server_get_instance_for_con_handle(con_handle);
    if (!instance){
        printf("no instance for handle 0x%02x", con_handle);
        return;
    }
		instance->customer_callback.callback = &ff11_can_send_now;
    instance->customer_callback.context  = (void*) (uintptr_t) con_handle;
    att_server_register_can_send_now_callback(&instance->customer_callback, con_handle);
		
		
}


void ff11_test_loop(void)
{
		if(customer_value_B[0] == 0xAA || g_test_value == 0xaa)
		{
		if(att_server_can_send_packet_now(0x10))
		{
			ff11_char_send_report(0x10,buf,20);
		}else{
		//	printf("can_not send_packet_now\r\n");
		}
		}
}

