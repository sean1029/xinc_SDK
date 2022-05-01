#ifndef _PATCH_FUNCTION_ID_H_
#define _PATCH_FUNCTION_ID_H_


/*
#include "patch_function_id.h"
 
if(PATCH_FUN[STACK_RESET]){
         
         return ;
    }

*/


enum{
	//rom 调用 ram的函数指针	
	XINC_TRACE_U32_PRINT,
	SENDCHAR,
	LE_RF_INIT,
	INIT_TIMER,
	TIMER_DISABLE,
	TIMER_CURRENT_COUNT,
	STACK_RESET,
    HWRADIO_LE_SET_ACTIVE_MODE,
	////rom 调用 ram的函数指针 --hw_radio.c
	HWRADIO_INITIALISE,
	HWRADIO_RESET,
	HWRADIO_DISABLEALLSPIWRITES,
	HWRADIO_LE_RXCOMPLETE,
	HWRADIO_LE_SERVICE,
	HWRADIO_LE_TXCOMPLETE,
	HWRADIO_LE_SETUP_RADIO_FOR_NEXT_TXRX,
	HWRADIO_CONVERT_TX_POWER_LEVEL_UNITS_TO_TX_POWER,
	HWRADIO_LE_READ_RSSI,
	HWRADIO_DEC_TX_POWER_LEVEL,
    HWRADIO_INC_TX_POWER_LEVEL,
    HWRADIO_SETUP_FOR_TIFS_EVENT,
    _HWRADIOCLEANUPRX,

	//patch.c
	HWRADIO_TXCOMPLETE,
	HWRADIO_PROGRAM_DUMMY_RX,
	//LE_test.c
	LETEST_TEST_EVENT_BEGIN,
	LETEST_CTRL_HANDLE_TEST_MODE_EVENT,
	LETEST_TEST_END,
	LETEST_TRANSMITTER_TEST,
	LETEST_RECEIVER_TEST,
	//hci_transport.c
	BTSTACK_EMBEDDED_PROCESS_ID,
	HCI_TRANSPORT_RAM_RECEIVE_PACKET_ID,
	HCI_TRANSPORT_RAM_OPEN_ID,
	HCI_TRANSPORT_RAM_CLOSE_ID,
	HCI_TRANSPORT_RAM_SEND_PACKET_ID,
	//I2cap_signaling.c
	L2CAP_NEXT_SIG_ID_ID,
	L2CAP_CREATE_SIGNALING_INTERNAL_ID,
	//I2cap.c
	L2CAP_INIT_ID,
	L2CAP_REQUEST_CAN_SEND_FIX_CHANNEL_NOW_EVENT_ID,
	L2CAP_SETUP_HEADER_ID,
	L2CAP_SEND_PREPARED_CONNECTIONLESS_ID,
	L2CAP_SEND_CONNECTIONLESS_ID,
	L2CAP_EMIT_CAN_SEND_NOW_ID,
	L2CAP_CHANNEL_ITEM_BY_CID_ID,
	L2CAP_FIXED_CHANNEL_FOR_CHANNEL_ID_ID,
	L2CAP_IS_LE_CHANNEL_TYPE_ID,
    L2CAP_RUN_ID,
    L2CAP_NOTIFY_CHANNEL_CAN_SEND_ID,
    L2CAP_HCI_EVENT_HANDLER_ID,
    L2CAP_REGISTER_SIGNALING_RESPONSE_ID,
	L2CAP_EMIT_CONNECTION_PARAMETER_UPDATE_RESPONSE_ID,
	L2CAP_LE_SIGNALING_HANDLER_DISPATCH_ID,
	L2CAP_ACL_CLASSIC_HANDLER_ID,
	L2CAP_ACL_LE_HANDLER_ID,
	L2CAP_ACL_HANDLER_ID,
	L2CAP_REGISTER_FIXED_CHANNEL_ID,
	//hci_cmd.c
	HCI_CMD_CREATE_FROM_TEMPLATE_ID,
	//hci.c
	CREATE_CONNECTION_FOR_BD_ADDR_AND_TYPE_ID,
	GAP_CONNECTION_PARAMETER_RANGE_INCLUDED_ID,
	HCI_CONNECTION_FOR_HANDLE_ID,
	HCI_CONNECTION_FOR_BD_ADDR_AND_TYPE_ID,
	HCI_IS_LE_CONNECTION_ID,
	HCI_NUMBER_FREE_ACL_SLOTS_FOR_CONNECTION_TYPE_ID,
	HCI_CAN_SEND_COMAND_PACKET_TRANSPORT_ID,
	HCI_SEND_ACL_PACKET_FRAGMENTS_ID,
	HCI_SEND_ACL_PACKET_BUFFER_ID,
	ACL_HANDLER_ID,
	HCI_SHUTDOWN_CONNECTION_ID,
	GAP_LE_GET_OWN_ADDRESS_ID,
	HCI_REENABLE_ADVERTISEMENTS_IF_NEEDED_ID,
	HCI_TRANSPORT_UART_GET_MAIN_BAUD_RATE_ID,
	HCI_INITIALIZATION_TIMEOUT_HANDLER_ID,
	HCI_REPLACE_BD_ADDR_PLACEHOLDER_ID,
	HCI_INITIALIZING_RUN_ID,
	HCI_INIT_DONE_ID,
	HCI_INITIALIZING_EVENT_HANDLER_ID,
	HCI_HANDLE_CONNECTION_FAILED_ID,
	EVENT_HANDLER_ID,
	HCI_PACKET_HANDLER_ID,
	HCI_STATE_RESET_ID,
	HCI_INIT_ID,
	HCI_SET_CHIPSET_ID,//
	HCI_CLOSE_ID,//
	HCI_POWER_CONTROL_ON_ID,
	HCI_POWER_CONTROL_OFF_ID,
	HCI_POWER_CONTROL_SLEEP_ID,
	HCI_POWER_CONTROL_WAKE_ID,
	HCI_POWER_TRANSITION_TO_INITIALIZING_ID,
	HCI_POWER_CONTROL_ID,
	HCI_HALTING_TIMEOUT_HANDLER_ID,
	HCI_RUN_ID,
	HCI_SEND_CMD_PACKET_ID,
	HCI_SEND_CMD_VA_ARG_ID,
	HCI_EMIT_EVENT_ID,
	HCI_EMIT_STATE_ID,
	HCI_EMIT_DISCONNECTION_COMPLETE_ID,
	HCI_EMIT_NR_CONNECTIONS_CHANGED_ID,
	HCI_EMIT_HCI_OPEN_FAILED_ID,
	HCI_EMIT_DEDICATED_BONDING_RESULT_ID,
	GAP_UPDATE_CONNECTION_PARAMETERS_ID,
	GAP_REQUEST_CONNECTION_PARAMETER_UPDATE_ID,
	GAP_ADVERTISMENTS_CHANGED_ID,
	GAP_ADVERTISEMENTS_SET_DATA_ID,
	GAP_SCAN_RESPONSE_SET_DATA_ID,
	HCI_LE_ADVERTISEMENTS_SET_PARAMS_ID,
	GAP_ADVERTISEMENTS_ENABLE_ID,
	HCI_LE_SET_OWN_ADDRESS_TYPE_ID,//
	GAP_DISCONNECT_ID,
	GAP_GET_CONNECTION_TYPE_ID,//
	GAP_LE_SET_PHY_ID,//
	HCI_DISCONNECT_ALL_ID,
	GAP_ENCRYPTION_KEY_SIZE_ID,
	GAP_AUTHENTICATED_ID,
	GAP_SECURE_CONNECTION_ID,
	GAP_AUTHORIZATION_STATE_ID,
	HCI_HALTING_DEFER_ID,
	//btstack_tlv.c---BTSTACK_TLV_SET_INSTANCE_ID,
	//btstack_memory_pool.c
	BTSTACK_MEMORY_POOL_CREATE_ID,
	BTSTACK_MEMORY_POOL_GET_ID,
	BTSTACK_MEMORY_POOL_FREE_ID,
	//btstack_memory.c
	BTSTACK_MEMORY_HCI_CONNECTION_GET_ID,
	BTSTACK_MEMORY_GATT_CLIENT_GET_ID,
	BTSTACK_MEMORY_WHITELIST_ENTRY_GET_ID,
	BTSTACK_MEMORY_SM_LOOKUP_ENTRY_GET_ID,
	BTSTACK_MEMORY_INIT_ID,
	//btstack_linked_list.c
	BTSTACK_LINKED_LIST_GET_LAST_ITEM_ID,
	BTSTACK_LINKED_LIST_ADD_ID,
	BTSTACK_LINKED_LIST_ADD_TAIL_ID,
	BTSTACK_LINKED_LIST_REMOVE_ID,
	BTSTACK_LINKED_LIST_COUNT_ID,//
	BTSTACK_LINKED_LIST_POP_ID,
	BTSTACK_LINKED_LIST_ITERATOR_INIT_ID,
	BTSTACK_LINKED_LIST_ITERATOR_HAS_NEXT_ID,
	BTSTACK_LINKED_LIST_ITERATOR_NEXT_ID,
	BTSTACK_LINKED_LIST_ITERATOR_REMOVE_ID,
	//btstack_crypto.c
	BTSTACK_CRYPTO_AES128_START_ID,
	BTSTACK_CRYPTO_CMAC_HANDLE_AES_ENGINE_READY_ID,
	BTSTACK_CRYPTO_CMAC_SHIFT_LEFT_BY_ONE_BIT_INPLACE_ID,
	BTSTACK_CRYPTO_CMAC_HANDLE_ENCRYPTION_RESULT_ID,
	BTSTACK_CRYPTO_CMAC_START_ID,
	BTSTACK_CRYPTO_CCM_CALC_XN_ID,
	BTSTACK_CRYPTO_CCM_CALC_AAD_XN_ID,
	BTSTACK_CRYPTO_CCM_NEXT_BLOCK_ID,
	BTSTACK_CRYPTO_RUN_ID,
	BTSTACK_CRYPTO_HANDLE_RANDOM_DATA_ID,
	BTSTACK_CRYPTO_HANDLE_ENCRYPTION_RESULT_ID,
	BTSTACK_CRYPTO_EVENT_HANDLER_ID,
	BTSTACK_CRYPTO_INIT_ID,
	//sm.c
	SM_TIMEOUT_HANDLER_ID,
	SM_TIMEOUT_START_ID,
	SM_DISPATCH_EVENT_ID,
	SM_NOTIFY_CLIENT_INDEX_ID,
	SM_SETUP_TK_ID,
	SM_KEY_DISTRIBUTION_FLAGS_FOR_SET_ID,
	SM_ADDRESS_RESOLUTION_LOOKUP_ID,//
	SM_TRIGGER_USER_RESPONSE_ID,
	SM_KEY_DISTRIBUTION_ALL_RECEIVED_ID,
	SM_DONE_FOR_HANDLE_ID,
	SM_KEY_DISTRIBUTION_FLAGS_FOR_AUTH_REQ_ID,
	SM_INIT_SETUP_ID,
	SM_STK_GENERATION_INIT_ID,
	SM_ADDRESS_RESOLUTION_HANDLE_EVENT_ID,
	SM_KEY_DISTRIBUTION_HANDLE_ALL_RECEIVED_ID,
	SM_START_CALCULATING_LTK_FROM_EDIV_AND_RAND_ID,
	SM_RUN_ID,
	SM_EVENT_PACKET_HANDLER_ID ,
	SM_VALIDATE_STK_GENERATION_METHOD_ID,
	SM_PDU_HANDLER_ID,
	SM_SET_AUTHENTICATION_REQUIREMENTS_ID,
	SM_INIT_ID,
	SM_SEND_SECURITY_REQUEST_FOR_CONNECTION_ID,
	SM_REQUEST_PAIRING_ID,
	SM_BONDING_DECLINE_ID,
	SM_JUST_WORKS_CONFIRM_ID,
	SM_PASSKEY_INPUT_ID,
	SM_KEYPRESS_NOTIFICATION_ID,
	GAP_RANDOM_ADDRESS_TYPE_REQUIRES_UPDATES_ID,//
	OWN_ADDRESS_TYPE_ID,
	GAP_RANDOM_ADDRESS_SET_MODE_ID,//
	GAP_RANDOM_ADDRESS_SET_UPDATE_PERIOD_ID,//
	GAP_RANDOM_ADDRESS_SET_ID,//
	GAP_RECONNECT_SECURITY_SETUP_ACTIVE_ID,
	//le_device_db_memory.c
	LE_DEVICE_DB_ADD_ID,
 	//att_server.c
	ATT_HANDLE_VALUE_INDICATION_NOTIFY_CLIENT_ID,
	ATT_EMIT_EVENT_TO_ALL_ID,
	ATT_EMIT_MTU_EVENT_ID,
	ATT_HANDLE_VALUE_INDICATION_TIMEOUT_ID,
	ATT_EVENT_PACKET_HANDLER_ID,
	ATT_SERVER_PROCESS_VALIDATED_REQUEST_ID,
	ATT_RUN_FOR_CONTEXT_ID,
	ATT_SERVER_DATA_READY_FOR_PHASE_ID,
	ATT_SERVER_TRIGGER_SEND_FOR_PHASE_ID,
	ATT_SERVER_HANDLE_CAN_SEND_NOW_ID,
	ATT_PACKET_HANDLER_ID,
	ATT_SERVER_PERSISTENT_CCC_WRITE_ID,
	ATT_SERVER_PERSISTENT_CCC_CLEAR_ID,
	ATT_SERVER_PERSISTENT_CCC_RESTORE_ID,
	ATT_SERVER_WRITE_CALLBACK_ID,
	ATT_SERVER_INIT_ID,
	ATT_SERVER_NOTIFY_ID,
	ATT_SERVER_INDICATE_ID,
	//att_dispatch.c
	ATT_DISPATCH_ATT_PACKET_HANDLER_ID, 
	//att_db.c
	ATT_VALIDATE_SECURITY_ID,
	HANDLE_FIND_INFORMATION_REQUEST2_ID,
	HANDLE_FIND_BY_TYPE_VALUE_REQUEST2_ID,
	HANDLE_READ_BY_TYPE_REQUEST2_ID,
	HANDLE_READ_REQUEST2_ID,
	HANDLE_READ_BLOB_REQUEST2_ID,
	HANDLE_READ_MULTIPLE_REQUEST2_ID,
	HANDLE_READ_BY_GROUP_TYPE_REQUEST2_ID,
	HANDLE_WRITE_REQUEST_ID,
	HANDLE_PREPARE_WRITE_REQUEST_ID,
	HANDLE_EXECUTE_WRITE_REQUEST_ID,
	ATT_HANDLE_REQUEST_ID,
	GATT_SERVER_GET_GET_HANDLE_RANGE_FOR_SERVICE_WITH_UUID16_ID,
	GATT_SERVER_GET_DESCRIPTOR_HANDLE_FOR_CHARACTERISTIC_WITH_UUID16_ID,
	GATT_SERVER_GET_GET_HANDLE_RANGE_FOR_SERVICE_WITH_UUID128_ID,
	GATT_SERVER_GET_CLIENT_CONFIGURATION_HANDLE_FOR_CHARACTERISTIC_WITH_UUID128_ID,
	//att_parser.c
	AD_DATA_CONTAINS_UUID16_ID,
	AD_DATA_CONTAINS_UUID128_ID,//
	//btstack_tick.c
	HAL_TICK_INIT_ID,
	HAL_TICK_SET_HANDLER_ID,
	//btstack.c
	BLE_STACK_INIT_ID,
	//bt_addr_sync.c
	BTADDR_BUILD_SYNC_WORD_ID,
	//bt_mini_sched.c
	BTMS_SCHED_ID,
	_BTMS_HANDLE_HC_RESET_IF_REQD_ID,
	//bt_init.c
	BT_INITIALISE_ID,
	//sys_config.c
	SYSCONFIG_INITIALISE_ID,
	//tra_queue.c
	_BTQ_INITIALISE_VARIABLE_CHUNK_QUEUE_ID,
	_BTQ_GET_DATA_QUEUE_REF_ID,
	BTQ_INITIALISE_DATA_QUEUE_TYPE_ID,
	_BTQ_INITIALISE_ALL_DATA_QUEUES_ID,
	BTQ_REDUCE_QUEUE_NUMBER_OF_PROCESSED_CHUNKS_ID,
	BTQ_INITIALISE_ID,
	BTQ_RESET_ID,
	BTQ_ENQUEUE_ID,
	BTQ_COMMIT_ID,
	BTQ_DEQUEUE_NEXT_ID,
	BTQ_LE_DEQUEUE_NEXT_ID,
	BTQ_DEQUEUE_NEXT_DATA_BY_LENGTH_ID,
	BTQ_ACK_LAST_DEQUEUED_ID,
	//hw_lc.c
	HWLC_RESET_ID,
	HWLC_INITIALISE_ID,
	_HWLC_CLEAR_JALAPENO_REGISTERS_ID,
	//hw_delay.c
	HWDELAY_CALIBRATE_ID,
	HWDELAY_CALIBRATE__ID,
	//lmp_init.c
	LM_INITIALISE_ID,
	//lmp_cmd_disp.c
	LMDISP_LMP_COMMAND_DISPATCHER_ID,
	//lmp_link_policy.c
	LMPOL_SLEEP_ID,
	//lslc_slot.c
	LSLCSLOT_INITIALISE_ID,
	LSLCSLOT_HANDLE_AUX_TIM_ID,
	LSLCSLOT_WRITE_BT_CLK_AVOID_RACE_ID,
	//lslc_irq.c
	LSLCIRQ_INITIALISE_ID,
	LSLCIRQ_IRQ_HANDLER_ID,
	LSLCIRQ_DISABLE_ALL_INTR_EXCEPT_ID,
	LSLCIRQ_DISABLE_ALL_TIM_INTR_EXCEPT_ID,
	LSLCIRQ_R2P_TURN_ON_INTR_ID,
	//dll_dev_impl.c
	DL_ALLOC_LINK_ID,
	//lc_initerface.c
	LC_INITIALISE_ID,
	//hc_cmd_disp.c
	HC_COMMAND_DISPATCHER_ID,
	_DISPATCH_TEST_COMMAND_ID,
	_DISPATCH_INFO_COMMAND_ID,
	_DISPATCH_LINK_CONTROL_COMMAND_ID,
	_DISPATCH_LINK_POLICY_COMMAND_ID,
	_DISPATCH_LOCAL_STATUS_COMMAND_ID,
	_DISPATCH_HC_BB_COMMAND_ID,
	_DISPATCH_LE_COMMAND_ID,
	//hc_event_gen.c
	///HCEG_INQUIRY_RESULT_EVENT_ID,//
	HCEG_COMMAND_COMPLETE_EVENT_ID,
	HCEG_NUMBER_OF_COMPLETED_PACKETS_EVENT_ID,
	HCEG_GENERATE_EVENT_ID,
	HCFC_INITIALISE_ID,
	//hc_flow_control.c
	HCFC_HANDLE_HOST_TO_HOST_CONTROLLER_FLOW_CONTROL_ID,
	HCFC_HOST_NUMBER_OF_COMPLETED_PACKETS_ID,
	HCFC_RELEASE_ALL_DATA_QUEUE_PACKETS_ID,
	//tc_cmd_disp.c
	TCI_DISPATCH_PROP_COMMAND_ID,
	//tc_event_gen.c
	TCEG_COMMAND_COMPLETE_EVENT_ID,
	///TCEG_READ_PUMP_MONITORS_EVENT_ID,//
	//LE_advertise.c
	LEADV_INIT_ID,
	LEADV_SET_ADVERTISING_PARAMS_ID,
	LEADV_SET_ADVERTISE_ENABLE_ID,
	LEADV_DISABLE_CONNECTABLE_ADVERTISING_ID,
	LEADV_ADVERTISING_EVENT_BEGIN_ID,
	_LEADV_SET_NEXT_ADVERTISING_EVENT_START_TIMER_ID,
	LEADV_ADJUST_ADVERTISING_EVENT_START_TIMER_ID,
	LEADV_RESET_ADV_FREQUENCY_ID,
	LEADV_TRY_ADVANCE_ADV_FREQUENCY_ID,
	LEADV_PREP_FOR_LE_ADVERT_TX_ID,
	_LEADV_ADVERTISING_EVENT_COMPLETE_ID,
	LEADV_GET_SLOTS_TO_NEXT_ADVERTISING_TIMER_ID,
	_LEADV_ADVANCE_FOR_NEXT_ADV_TX_ID,
	LEADV_HANDLE_ADVERTISING_ID,
	LEADV_ENCODE_OWN_ADDRESS_IN_PAYLOAD_ID,
	LEADV_SET_ADV_HEADER_ID,
	LEADV_WRITE_SCAN_RESP_DATA_ID,
	LEADV_PREP_FOR_LE_SCAN_RESP_TX_ID,
	LEADV_TCI_READ_ADVERTISING_PARAMS_ID,
	//LE_config.c
	LECONFIG_INIT_ID,
    LECONFIG_CHECK_CURRENT_ADDRESS_ID,
	LECONFIG_ALLOCATE_LINK_ID_ID,
	///LECONFIG_SWITCH_TO_LE_ID,///
	//lE_connect.c
	EXT_RX_WINDOW_INIT_ID,
	EXT_RX_WINDOW_UPDATE_ID,
	LECONNECTION_INIT_ID,
	LE_CONNECTION_EXTACT_CONNECTION_REQ_PARAMETERS_ID,
	LECONNECTION_READ_REMOTE_VERSION_INFO_ID,
	LELLC_ENCODE_PDU_ID,
	LECONNECTION_HANDLE_CONNECTION_EVENT_ID,
	LE_LL_CTRL_HANDLE_CONNECTION_EVENT_ID,
	_LECONNECTION_ENCODE_DATAPDU_ID,
	_LECONNECTION_DECODE_DATAPDU_ID,
	_LECONNECTION_DETERMINE_WINDOW_WIDENING_IN_FRAMES_ID,
	_LECONNECTION_CLOSE_CONNECTION_EVENT_ID,
	LECONNECTION_SETUP_FIRST_CONNECTION_EVENT_ID,
	_LECONNECTION_RESET_CONNECTION_ID,
	LECONNECTION_CONNECTION_EVENT_STILL_OPEN_ID,
	LECONNECTION_CONNECTION_UPDATE_ID,
	LEEVENTGEN_GENERATE_LE_META_EVENT_ID,
	LECONNECTION_ENCRYPT_PAYLOAD_ID,
	LECONNECTION_DECRYPT_INCOMING_PDU_ID,
	LECONNECTION_FIND_NEXT_CONNECTION_EVENT_ID,
	LECONNECTION_HANDLE_DATA_TO_ENCRYPT_ID,
	//LE_link_layer.c
	LELL_DECODE_ADVERTISING_CHANNELPDU_ID,
	LELL_ENCODE_ADVERTISING_CHANNELPDU_ID,
	LE_LL_CHECK_FOR_PENDING_CONNECTION_COMPLETES_ID,
	LE_DECODE_LINK_LAYER_PDU_ID,
	LE_LL_CONNECTION_UPDATE_REQ_ID,
	LE_LL_VERSION_IND_ID,
	LE_LL_START_ENC_RSP_ID,
	LE_LL_PAUSE_ENC_REQ_ID,
	LE_LL_PAUSE_ENC_RSP_ID,
	LE_LL_ENC_REQ_ID,
	LE_LL_UNKNOWN_RSP_ID,
	LE_LL_REJECT_IND_ID,
	LE_LL_CTRL_HANDLE_LE_IRQ_ID,
	LE_LL_ENCODE_OWN_ADDRESS_IN_PAYLOAD_ID,
	LE_LL_INACTIVESLOTS_ID,
	LE_LL_SLOTS_TO_NEXT_LE_ACTIVITY_ID,
	LE_LL_HANDLE_LLC_ACK_ID,
	LE_LL_CHECK_TIMERS_ID,
	LE_LL_RESPONSE_TIMEOUT_ID,
	//LE_security.c
	LESECURITY_HW_AES_ENCRYPT_128_ID,
	LESECURITY_START_ENCRYPTION_ID,
	LESECURITY_LONG_TERM_KEY_REQUEST_REPLY_ID,
	LESECURITY_LONG_TERM_KEY_REQUEST_NEGATIVE_REPLY_ID,
	//LE_Scanf.c
	LESCAN_INIT_ID,
	//le_power.c
	RC32K_CALIB_FIND_COUNT_ID,
	RC32K_CALIB_START_ID,
	RC32K_CALIB_FINISH_ID,
	BLE_LOWPOWER_REMAINING_CLK_ID,
	ADV_ENTER_LOWPOWER_ID,
	ADV_EXIT_LOWPOWER_ID,
	RESTORE_SLEEP_CLK_ID,
	CAL_INTRASLOTOFFSET_COMPENSATE_ID,
	CONNECTION_EXIT_LOWPOWER_ID,
	DO_CONNECTION_EXIT_LOWPOWER_ID,
	CONNECTION_ENTER_LOWPOWER_ID,
	DO_CONNECTION_ENTER_LOWPOWER_ID,
	BLE_FIRMWARE_EXIT_LOWPOWER_ID,
	//tra_hcit.c
	TRAHCIT_INITIALISE_ID,
	TRAHCIT_SHUTDOWN_ID,
	TRAHCIT_GENERIC_GET_RX_BUF_ID,
	TRAHCIT_GENERIC_COMMIT_RX_BUF_ID,
	TRAHCIT_GENERIC_ACKNOWLEDGE_COMPLETE_TX_ID,
	TRAHCIT_DISPATCH_PENDING_DATA_ID,
	TRAHCIT_DISPATCH_PENDING_EVENT_ID,
	//sys_init.c
	SYSINIT_INITIALISE_ID,
	
	PACKET_HANDLER_ID,
	LECONNECTION_CREATE_CONNECTION_ID,
	LECONNECTION_ENCODE_CONNECT_REQ_PDU_ID,
	LECONNECTION_INITIATOR_SCAN_INTERVAL_TIMEOUT_ID,
	LECONNECTION_CREATE_CONNECTION_CANCEL_ID,
	LECONNECTION_HCI_CONNECTION_UPDATE_ID,
	LECONNECTION_READ_REMOTE_USED_FEATURES_ID,
	VAIDATE_ACCESS_ADDRESS_ID,
	LE_LL_FEATURE_RSP_ID,
	LE_LL_START_ENC_REQ_ID,
	LE_LL_ENC_RSP_ID,
	LESCAN_SET_SCAN_PARAMETERS_ID,
	LESCAN_SET_SCAN_ENABLE_ID,
	LESCAN_SCAN_INTERVAL_TIMEOUT_ID,
	LESCAN_ADVANCE_INITIATING_FREQUENCY_ID,
	LESCAN_ADVANCE_SCAN_FREQUENCY_ID,
	LESCAN_GENERTATE_ADVERTISING_REPORT_EVENT_ID,
	LESCAN_CTRL_HANDLE_SCANNING_ID,
	_LESCAN_RETURN_SCANNING_ID,
	_LESCAN_SCAN_WINDOW_COMPLETE_ID,
	LESCAN_PREP_FOR_LE_SCAN_RX_ID,
	LESCAN_BACKOFF_SUCCESSFULL_SCAN_RSP_RXED_ID,
	_LESCAN_BACKOFF_FAILURE_SCAN_RSP_RXED_ID,
	_LESCAN_END_INITIATING_ID,
	LESCAN_GET_FREE_ADV_EVENT_POINTER_ID,
	LESCAN_CHECK_FOR_PENDING_ADVERTISING_REPORTS_ID,
	LE_LL_ADJUST_SCAN_TIMER_ID,
	LE_ADV_ADJUST_SCAN_TIMER_ID,
	LEWL_ADD_DEVICE_TO_DUPLICATE_LIST_ID,
	LEWL_INIT_DUPLICATES_LIST_ID,
	GATT_CLIENT_INIT_ID,
	PROVIDE_CONTEXT_FOR_CONN_HANDLE_ID,
	GATT_CLIENT_RUN_FOR_PERIPHERAL_ID,
	GATT_CLIENT_RUN_ID,
	GATT_CLIENT_EVENT_PACKET_HANDLER_ID,
	GATT_CLIENT_ATT_PACKET_HANDLER_ID,
	MAX_PATCH_NUM,
};
#endif


extern void * PATCH_FUN[MAX_PATCH_NUM];




#define PRINT_LEVEL0 0
#define PRINT_LEVEL1 1
#define PRINT_LEVEL2 2
#define PRINT_LEVEL3 3


#define TRACE_INDEX_1 0X1
#define TRACE_INDEX_2 0X2
#define TRACE_INDEX_3 0X3
#define TRACE_INDEX_4 0X4
#define TRACE_INDEX_5 0X5
#define TRACE_INDEX_6 0X6
#define TRACE_INDEX_7 0X7
#define TRACE_INDEX_8 0X8
#define TRACE_INDEX_9 0X9
#define TRACE_INDEX_A 0XA
#define TRACE_INDEX_B 0XB
#define TRACE_INDEX_C 0XC
#define TRACE_INDEX_D 0XD
#define TRACE_INDEX_E 0XE
#define TRACE_INDEX_F 0XF
#define TRACE_INDEX_10 0X10
#define TRACE_INDEX_11 0X11

extern void xinc_trace_u32_print(unsigned int  value, unsigned char  index) ;
#define XINC_TRACE_U16_PRINT(VALUE16, INDEX8, LEVEL) 
#define XINC_TRACE_U32_PRINT(VALUE32, INDEX8)      ((void (*)(unsigned int , unsigned char))PATCH_FUN[XINC_TRACE_U32_PRINT])(VALUE32, INDEX8)
//#define XINC_TRACE_U32_PRINT(VALUE32, INDEX8)      xinc_trace_u32_print(VALUE32, INDEX8)

/*
#include "patch_function_id.h"
 
if(PATCH_FUN[  ]){
         
         return ;
    }

*/































