#ifndef _GLOBAL_VAL_H_
#define _GLOBAL_VAL_H_
#include "sys_features.h"
#include "sys_hal_types.h"
#include "sys_config.h"
#include "tra_queue.h" 
#include "lmp_config.h"
#include "sys_debug_config.h"
#include "hw_register.h"
#include "hc_flow_control.h"
#include "le_adv.h"
#include "lE_config.h"
#include "lE_connection.h"
#include "le_link_layer.h"
#include "le_frequency.h"
#include "le_test.h"
#include "tra_hcit.h"
#include "tra_uart.h"
#include "tra_usb.h"
#include "tra_queue.h"
#include "hw_radio.h" 
#include "btstack_debug.h"
#include "btstack_run_loop.h"
#include "btstack_defines.h"
#include "global_val.h"
#include "btstack_tlv.h"
#include "patch_function_id.h"

///#include "global_val.h"


//bt_mini_sched.c
extern u_int8 _BTms_pending_hc_reset;//= 0;

//sys_rand_num_gen.c
extern u_int32 srand_seed;

//sys_config.c
extern t_SYS_Config g_sys_config /*= { 0 }*/;


#define   ATT_MAX    2

typedef union
{
    u_int32 _dummy;
    u_int8 hci_event_heap_array_[PRH_BS_CFG_SYS_SIZEOF_HCI_EVENT_HEAP];
} t_event_heap;
typedef union
{
    u_int32 _dummy;
    u_int8 hci_command_heap_array_[PRH_BS_CFG_SYS_SIZEOF_HCI_COMMAND_HEAP];
} t_command_heap;
extern t_command_heap command_heap_;

typedef union
{
    u_int32 _dummy;
    u_int8 lmp_in_heap_array_[PRH_BS_CFG_SYS_SIZEOF_LMP_IN_HEAP];
} t_lmp_in_heap;
extern t_lmp_in_heap lmp_in_heap_;

//tra_queue.c
extern t_event_heap event_heap_;
extern u_int8* hci_event_heap;// = event_heap_.hci_event_heap_array_;
extern t_q_descr hci_event_headers[PRH_BS_CFG_SYS_MAX_HCI_EVENTS];
extern t_command_heap command_heap_;
extern u_int8* hci_command_heap;// = command_heap_.hci_command_heap_array_;
extern t_q_descr hci_command_headers[PRH_BS_CFG_SYS_MAX_HCI_COMMANDS];
extern t_lmp_in_heap lmp_in_heap_;
extern u_int8* lmp_in_heap;// = lmp_in_heap_.lmp_in_heap_array_;
extern t_q_descr lmp_in_headers[PRH_BS_CFG_SYS_MAX_LMP_IN_MESSAGES];
extern u_int8 lmp_out_heaps[MAX_ACTIVE_DEVICE_LINKS][PRH_BS_CFG_SYS_SIZEOF_LMP_OUT_HEAP];
extern t_q_descr lmp_out_headers[MAX_ACTIVE_DEVICE_LINKS][PRH_BS_CFG_SYS_MAX_LMP_OUT_MESSAGES];    
extern t_queue BTQueues[TRA_QUEUE_TOTAL_NUM_QUEUES];
extern t_data_queue BTq_l2cap_in;
extern t_data_queue BTq_l2cap_out;


#if (PRH_BS_CFG_SYS_TRACE_VIA_HCI_SUPPORTED==1)
/*
 * Setup a distinct queue for logging
 */
extern u_int8 hci_debug_heap[PRH_BS_CFG_SYS_SIZEOF_HCI_DEBUG_HEAP];
extern t_q_descr hci_debug_headers[PRH_BS_CFG_SYS_MAX_HCI_DEBUG_EVENTS];
#endif

#if (PRH_BS_CFG_SYS_LMP_EXTENDED_SCO_SUPPORTED==1)
extern t_q_descr eSCO_Tx_Descriptor[PRH_BS_CFG_SYS_MAX_SYN_LINKS_VIA_HCI];
extern u_int8 eSCO_Tx_Buffers[PRH_BS_CFG_SYS_MAX_SYN_LINKS_VIA_HCI][PRH_BS_CFG_SYS_SIZEOF_ESCO_OUT_HEAP];
#endif


//struct s_all_queues BTq_data_memory;
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
t_data_queue BTq_sco_in;
t_data_queue BTq_sco_out;
boolean BTq_sco_in_data_aggregation;
#endif



//hw_delay.c
#define SYS_HAL_PRECALIBRATED_DELAY_MS 1500
#define SYS_HAL_PRECALIBRATED_DELAY_10US 15
#define SYS_HAL_PRECALIBRATED_DELAY_US 1
extern u_int32 calibration_counter_1ms;//=SYS_HAL_PRECALIBRATED_DELAY_MS;
extern u_int32 calibration_counter_10u;//s=SYS_HAL_PRECALIBRATED_DELAY_10US;
extern u_int32 calibration_counter_1us;//=SYS_HAL_PRECALIBRATED_DELAY_US;
extern volatile u_int32 hw_delay_counter; /* extern'd for TCI use */


//lmp_config.c
extern t_LMconfig g_LM_config_info;
extern t_role g_LM_test_device_role;
extern u_int32 g_LM_device_state;// = 0;


////lslc_slot.c
extern boolean _LSLCslot_local_piconet_request;
typedef enum {
    W4_TX_START         = 0,
    TX_INTR_ERROR       = 1,
    W4_RX_START         = 2,
    RX_INTR_ERROR       = 3,
    W4_RX_INTR          = 4,

    W2_PREPARE_TX       = 5,
    W4_3SLOT_INTR_RX    = 6,
    W4_5SLOT_INTR_RX    = 7,
    W4_5SLOT_1ST_TIM2   = 8,
    W2_COMPLETE_RX      = 9,
    W2_SETUP_ENABLE_RXR = 10,
    W4_PKD_INTR         = 11,

    W2_ENABLE_RXR       = 12,
    W4_3SLOT_INTR_TX    = 13,
    W4_5SLOT_INTR_TX    = 14,
    W4_5SLOT_1ST_TIM0   = 15,
    W4_1ST_PKA_OF2      = 16,

    SCAN_ACTIVE         = 17,
    SCAN_CHECK          = 18,

    W4_FHS_2ND_HALF_TIM0 = 19,
    W4_FHS_2ND_HALF_PKD  = 20,

    W4_RADIO_TO_GO_INTO_STANDBY = 21,
    W4_INTRASLOT_OFFSET_ALIGNMENT = 22,

    W4_PKD_INTR_EXTENDED       = 23,

     NUM_TX_RX_STATES    = 24
} t_tx_rx_state;
extern t_tx_rx_state _LSLCslot_state;

//lslc_irq.c
extern u_int8 le_mode; //= 0; //Classic BT
extern mHWreg_Create_Cache_Register(JAL_SER_ESER_CTRL_REG);// = 0;
extern mHWreg_Create_Cache_Register(JAL_COM_CTRL_IRQ_REG);
extern pka_t   pka_op;// = (pka_t)0;
extern pkd_t   pkd_op;// = (pkd_t)0;

//dl_dev_impl.c
extern u_int32 DL_LC_link_pool;

//hc_event_gen.c
extern u_int32 hci_event_mask, hci_event_mask_extended;

//hc_flow_control.c
extern t_hc_flow_ctrl hc_flow_ctrl;

//lE_advertise.c
extern u_int32 u_int32_TX_buffer[12];
extern u_int32 u_int32_RX_buffer[12];
extern t_LE_Advertising LE_advertise;

//hc_cmd_disp.c
#if (PRH_BS_CFG_SYS_INQUIRY_READ_WRITE_IAC_LAP_SUPPORTED==1)
extern u_int8 p_iacs[PRH_BS_CFG_SYS_NUMBER_OF_SUPPORTED_IAC *3];
#endif



//lE_config.c
extern t_LE_Config LE_config;
extern u_int8 g_LE_Config_Classic_Dev_Index;// = 0;
 
 
 //lE_connection.c
extern t_LE_Connection LE_connections[DEFAULT_MAX_NUM_LE_LINKS];
extern unsigned    int     Malloc_Error;// = 0;
extern unsigned	int have_get_no_crc_pkt;// = 0;
extern unsigned	int wrong_connect_req_pkt;// = 0;
extern unsigned	char update_transmit_window_num;// = 0;
extern unsigned	char continue_miss_packet_number;// = 0;
extern unsigned	char connect_no_pka_count;// = 0;
extern u_int32    rssi_val;//=0;
extern u_int16 g_LE_Slave_IntraSlot;// = 0;


//le_link_layer.c
typedef t_error (*t_ll_decode_function)(t_LE_Connection* p_connection , t_p_pdu p_pdu);
extern t_ll_decode_function LL_Pdu_Decode_Handlers[LL_NO_OPERATION];//= {
//#if (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
//	LE_LL_Connection_Update_Req,     /* LL_CONNECTION_UPDATE_REQ = 0 */
//#else
//	LE_LL_UnSupported_Pdu,
//#endif
//#if (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
//	LEfreq_Update_Channel_Map_Req,   /* LL_CHANNEL_MAP_REQ       = 1 */
//#else
//	LE_LL_UnSupported_Pdu,
//#endif
//	LE_LL_Terminate_Ind,             /* LL_TERMINATE_IND         = 2 */
//#if (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
//	LE_LL_Enc_Req,                   /* LL_ENC_REQ               = 3 */
//#else
//	LE_LL_UnSupported_Pdu,
//#endif

//#if (PRH_BS_CFG_SYS_ENCRYPTION_SUPPORTED==1) && (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1)
//	LE_LL_Enc_Rsp,                   /* LL_ENC_RSP               = 4 */
//	LE_LL_Start_Enc_Req,             /* LL_START_ENC_REQ         = 5 */
//#else
//	LE_LL_UnSupported_Pdu,
//	LE_LL_UnSupported_Pdu,
//#endif
//	LE_LL_Start_Enc_Rsp,             /* LL_START_ENC_RSP         = 6 */
//	LE_LL_Unknown_Rsp,               /* LL_UNKNOWN_RSP           = 7 */
//#if (PRH_BS_CFG_SYS_LE_PERIPHERAL_DEVICE==1)
//	LE_LL_Feature_Req,               /* LL_FEATURE_REQ           = 8 */
//#else
//	LE_LL_UnSupported_Pdu,
//#endif
//#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
//	LE_LL_Feature_Rsp,               /* LL_FEATURE_RSP           = 9 */
//#else
//	LE_LL_UnSupported_Pdu,
//#endif
//	LE_LL_Pause_Enc_Req,             /* LL_PAUSE_ENC_REQ         = 10*/
//	LE_LL_Pause_Enc_Rsp,             /* LL_PAUSE_ENC_RSP         = 11*/
//	LE_LL_Version_Ind,               /* LL_VERSION_IND           = 12*/
//	LE_LL_Reject_Ind                 /* LL_REJECT_IND            = 13*/
//};


extern t_advert_event  advertising_array[PRH_BS_CFG_LE_MAX_DEPTH_ADVERTISING_ARRAY];
extern u_int8 advertising_array_index;//= 0;
extern unsigned int after_rx_first_packet;// = 0;
extern s_int8 last_adv_rx_rssi;// = 0;
extern unsigned char ll_pdu_lenth_table[14];// = {12,8,2,23,13,1,1,2,9,9,1,1,6,2};

//le_test.c


//le_config.c


extern btstack_linked_list_t gatt_client_connections;
extern btstack_linked_list_t gatt_client_value_listeners;
extern btstack_packet_callback_registration_t hci_event_callback_registration_gatt_c;

extern uint8_t mtu_exchange_enabled;
extern uint8_t name_packet[30];//={0};


//gatt_client.c

//hids_device.c
typedef struct{
    uint16_t        con_handle;

    uint8_t         hid_country_code;
    const uint8_t * hid_descriptor;
    uint16_t        hid_descriptor_size;

    uint16_t        hid_report_map_handle;
    uint16_t        hid_protocol_mode;
    uint16_t        hid_protocol_mode_value_handle;

    uint16_t        hid_boot_mouse_input_value_handle;
    uint16_t        hid_boot_mouse_input_client_configuration_handle;
    uint16_t        hid_boot_mouse_input_client_configuration_value;

    uint16_t        hid_boot_keyboard_input_value_handle;
    uint16_t        hid_boot_keyboard_input_client_configuration_handle;
    uint16_t        hid_boot_keyboard_input_client_configuration_value;

    uint16_t        hid_report_input_value_handle;
    uint16_t        hid_report_input_client_configuration_handle;
    uint16_t        hid_report_input_client_configuration_value;
	
	uint16_t        hid_report_input_value_handle1;
	uint16_t        hid_report_input_client_configuration_handle1;
    uint16_t        hid_report_input_client_configuration_value1;


    uint16_t        hid_report_output_value_handle;
    uint16_t        hid_report_output_client_configuration_handle;
    uint16_t        hid_report_output_client_configuration_value;

    uint16_t        hid_report_feature_value_handle;
    uint16_t        hid_report_feature_client_configuration_handle;
    uint16_t        hid_report_feature_client_configuration_value;

    uint16_t        hid_control_point_value_handle;
    uint8_t         hid_control_point_suspend;

    btstack_context_callback_registration_t  battery_callback;
} hids_device_t;

typedef uint16_t hci_con_handle_t;
typedef uint16_t (*att_read_callback_t)(hci_con_handle_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size);
typedef int (*att_write_callback_t)(hci_con_handle_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);

// Read & Write Callbacks for handle range
//typedef struct att_service_handler {
//    btstack_linked_item_t * item;
//    uint16_t start_handle;
//    uint16_t end_handle;
//    att_read_callback_t read_callback;
//    att_write_callback_t write_callback;
//    btstack_packet_handler_t packet_handler;
//} att_service_handler_t;
extern hids_device_t hids_device;
#include "att_db.h"
extern btstack_packet_handler_t packet_handler_hids;
extern att_service_handler_t hid_service;





//tra_hcit.c
extern void* (*TRAhcit_Register)(void);
extern u_int32 (*hci_generic_event_callback)(u_int8 *data, u_int32 pdu_len, u_int8 *header, u_int8 head_len);

extern u_int8 hcit_tx_to_host_acl_header[5];
extern struct tra_hcit_info volatile *sys_hcit_info; 
extern volatile u_int8 _tra_hcit_active;
extern void TRAhcit_UART_Initialise(void);
extern void (*tra_hcit_initialise_table[TRA_HCIT_TYPE_ENDS])(void);// = 
//{ 
//#if (TRA_HCIT_UART_SUPPORTED==1)
//     TRAhcit_UART_Initialise,
//#endif
//#if (TRA_HCIT_USB_SUPPORTED==1)
//    TRAhcit_USB_Initialise,
//#endif
//#if (TRA_HCIT_PCMCIA_SUPPORTED==1)
//    NULL,
//#endif
//#if (TRA_HCIT_COMBINED_SUPPORTED==1)
//    TRAhcit_COMBINED_Initialise,
//#endif
//#if (TRA_HCIT_GENERIC_SUPPORTED==1)
//    NULL,
//#endif
//};

extern void (*tra_hcit_shutdown_table[TRA_HCIT_TYPE_ENDS])(void);// = 
//{
//#if (TRA_HCIT_UART_SUPPORTED==1)
//     TRAhcit_UART_Shutdown,
//#endif
//#if (TRA_HCIT_USB_SUPPORTED==1)
//    TRAhcit_USB_Shutdown,
//#endif
//#if (TRA_HCIT_PCMCIA_SUPPORTED==1)
//    NULL,
//#endif
//#if (TRA_HCIT_COMBINED_SUPPORTED==1)
//    TRAhcit_COMBINED_Shutdown,
//#endif
//#if (TRA_HCIT_GENERIC_SUPPORTED==1)
//    NULL,
//#endif
//};
extern void TRAhcit_Generic_Dispatcher(u_int8 *data, u_int32 pdu_len, u_int8 *header, u_int8 head_len, u_int8 q_type, t_deviceIndex device_index);
extern void (*tra_hcit_transmit_table[TRA_HCIT_TYPE_ENDS])
    (u_int8 *, u_int32, u_int8 *, u_int8, u_int8, t_deviceIndex);// = 
//{
//#if (TRA_HCIT_UART_SUPPORTED==1)
//    TRAhcit_UART_Transmit,
//#endif
//#if (TRA_HCIT_USB_SUPPORTED==1)
//    TRAhcit_USB_Transmit,
//#endif
//#if (TRA_HCIT_PCMCIA_SUPPORTED==1)
//    NULL,
//#endif
//#if (TRA_HCIT_COMBINED_SUPPORTED ==1)
//    TRAhcit_COMBINED_Transmit,
//#endif
//#if (TRA_HCIT_GENERIC_SUPPORTED==1)
//    TRAhcit_Generic_Dispatcher,
//#endif
//};



#ifdef ENABLE_GATT_CLIENT_PAIRING
btstack_packet_callback_registration_t sm_event_callback_registration;
#endif


//l2cap_signaling.c
extern uint8_t   sig_seq_nr;//  = 0xff;
extern uint16_t  source_cid;//  = 0x40;

//sys_main.c
extern signed  char rf_tx_power;// = 0;

//xinrf.c
extern struct tra_hcit_info sys_hcit_generic_info;
//btstack_run_loop.c
extern const btstack_run_loop_t * the_run_loop;// = NULL;

//btstack_run_loop_embedded.c
extern const btstack_run_loop_t btstack_run_loop_embedded;
extern btstack_linked_list_t data_sources;
extern btstack_linked_list_t timers;
extern volatile uint32_t system_ticks;
extern int trigger_event_received;// = 0;
//btstack_util.c
extern char uuid128_to_str_buffer[32+4+1];
extern char bd_addr_to_str_buffer[6*3];  // 12:45:78:01:34:67\0
extern /*const*/ char * char_to_nibble;// = "0123456789ABCDEF";


//att_db.c
typedef uint16_t hci_con_handle_t;
typedef int (*att_write_callback_t)(hci_con_handle_t con_handle, uint16_t attribute_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
typedef uint16_t (*att_read_callback_t)(hci_con_handle_t con_handle, uint16_t attribute_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size);
extern uint8_t const * att_db;// = NULL;
extern att_read_callback_t  att_read_callback;//  = NULL;
extern att_write_callback_t att_write_callback;// = NULL;
extern int      att_prepare_write_error_code;//   = 0;
extern uint16_t att_prepare_write_error_handle;// = 0x0000;
extern uint16_t att_persistent_ccc_handle;
extern uint16_t att_persistent_ccc_uuid16;


//att_dispatch.c
typedef struct {
    btstack_packet_handler_t packet_handler;
    uint8_t                  waiting_for_can_send;
} SUBSCRIPTIONS;
extern SUBSCRIPTIONS subscriptions[ATT_MAX];
extern uint8_t att_round_robin;
extern uint8_t can_send_now_pending;


//att_server.c
extern btstack_packet_callback_registration_t hci_event_callback_registration1;
extern btstack_packet_callback_registration_t sm_event_callback_registration;
extern btstack_packet_handler_t               att_client_packet_handler;// = NULL;
extern btstack_linked_list_t                  service_handlers;
extern btstack_context_callback_registration_t att_client_waiting_for_can_send_registration;
extern att_read_callback_t                    att_server_client_read_callback;
extern att_write_callback_t                   att_server_client_write_callback;
extern hci_con_handle_t att_server_last_can_send_now;// = HCI_CON_HANDLE_INVALID;

//btstack.c
extern btstack_packet_callback_registration_t hci_event_callback_registration2;
extern btstack_packet_handler_t  att_handler;// = (btstack_packet_handler_t)0;

//btstack_crypto.c
typedef enum {
    CMAC_IDLE,
    CMAC_CALC_SUBKEYS,
    CMAC_W4_SUBKEYS,
    CMAC_CALC_MI,
    CMAC_W4_MI,
    CMAC_CALC_MLAST,
    CMAC_W4_MLAST
} btstack_crypto_cmac_state_t;
extern uint8_t btstack_crypto_initialized;
extern btstack_linked_list_t btstack_crypto_operations;
extern btstack_packet_callback_registration_t hci_event_callback_registration3;
extern uint8_t btstack_crypto_wait_for_hci_result;
extern btstack_crypto_cmac_state_t btstack_crypto_cmac_state;
extern sm_key_t btstack_crypto_cmac_k;
extern sm_key_t btstack_crypto_cmac_x;
extern sm_key_t btstack_crypto_cmac_m_last;
extern uint8_t  btstack_crypto_cmac_block_current;
extern uint8_t  btstack_crypto_cmac_block_count;
#ifndef USE_BTSTACK_AES128
extern uint8_t btstack_crypto_ccm_s[16];
#endif

//hci_dump.c
#define HCIDUMP_HDR_SIZE 13
#define PKTLOG_HDR_SIZE 13
extern int dump_file;// = -1;
extern int dump_format;
union _XC_HEAD {
  uint8_t header_bluez[HCIDUMP_HDR_SIZE];
  uint8_t header_packetlogger[PKTLOG_HDR_SIZE];
} ;
extern union _XC_HEAD header;
// levels: debug, info, error
extern int log_level_enabled[4];// = { 1, 1, 1, 1};

//btstack_tlv.c
extern const btstack_tlv_t * btstack_tlv_singleton_impl;
extern void * 		         btstack_tlv_singleton_context;

//btstack_tick.c
extern void dummy_handler_no_param(void);
extern void (*tick_handler)(void);//= &dummy_handler_no_param;
	
//sys_hal_config.c	
extern u_int8 bd_addr[6];// = {'1', 'v', 'c', 'n', 'i', 'X'};

//hw_radio.c	
extern uint8_t _XC_ONECE_EXE ;//= 0;
#if (TX_REPREPARE_ON_PKD_SUPPORTED==1)
extern u_int8 tx_prepared_on_tim2 = FALSE;
extern u_int8 tx_reprepare_on_pkd = FALSE;
#endif
extern u_int32 jal_le_eser_time_config_word;// = 0;
extern u_int8 LE_Active_Mode;// = 1;
extern uint16_t reg_i_idac_rf;// = 0;
extern uint16_t reg_q_qdac_rf;// = 0;


#if  0
//低功耗模式下   debug_le_sleep_mode0置0， debug_le_sleep_mode2和g_xinchip_lowpower_enable置1
//非低功耗模式下 debug_le_sleep_mode0置1， debug_le_sleep_mode2和g_xinchip_lowpower_enable置0 
extern uint8_t debug_le_sleep_mode0 = 1;
extern uint8_t debug_le_sleep_mode2 = 0;
extern uint8_t g_xinchip_lowpower_enable = 0;
#endif





/*--------------------------------------------
#define      __DEBUG_LE_SLEEP           0     //- for: TP/CON/SLA/BI-01-C
												 TP/CON/ADV/BV-04-C
		 										 TP/CON/SLA/BV-15-C
		 										 TP/CON/SLA/BI-02-C
		 										 TP/SEC/SLA/BV-01-C

#define      __DEBUG_LE_SLEEP          	2	  //- for: Normol WORK.	
---------------------------------------------*/
extern u_int8 g_debug_le_sleep;// = 0;   //  no sleep:0   sleep: 2     __DEBUG_LE_SLEEP 
extern u_int8 g_debug_xinchip_rf_lowpower_enable;// = 0;

extern u_int8 g_le_enable_observer_flag;// = 1; //PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE;
extern u_int8 g_le_enable_central_flag;// = 1;   //  PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE


typedef		uint32_t	(*tHandler_callback)(uint16_t  val);

//tHandler_callback   tHandler_Callback[4] = {(tHandler_callback)0};
extern	int         sendchar(int c);	
extern	void        le_rf_init(void);
extern	void	    Init_Timer(uint32_t ch, uint32_t msTick);
extern	void	    Timer_disable(uint32_t ch);
extern	uint32_t	timer_current_count(uint32_t ch);
extern	void        stack_reset(void);
extern	void HWradio_Initialise(void);
extern	void HWradio_Reset(void);
extern	void HWradio_DisableAllSpiWrites(void);
extern	void HWradio_LE_RxComplete(void);
typedef u_int8 t_radio_freq_mask;
extern	boolean HWradio_LE_Service(t_RadioMode in_RadioMode, u_int8 io_Channel, t_radio_freq_mask freq_mask);         
extern	void HWradio_LE_TxComplete(void);
extern	void HWradio_LE_Setup_Radio_For_Next_TXRX(t_frame_pos const next_slot_posn);
extern	s_int8 HWradio_Convert_Tx_Power_Level_Units_to_Tx_Power(u_int8 power_level);
extern	s_int8 HWradio_LE_Read_RSSI(void);
extern	 void HWradio_LE_Set_Active_Mode(u_int8 mode);
extern	 void HWradio_TxComplete(void);
extern	 void HWradio_Program_Dummy_Rx(unsigned char io_Channel);
t_error HWradio_Dec_Tx_Power_Level(void);
t_error HWradio_Inc_Tx_Power_Level(void);
void HWradio_Setup_For_TIFS_Event(u_int8 io_Channel);
void _HWradioCleanUpRx(void);

//LE_test.c
void    LEtest_Test_Event_Begin(void);
t_error LEtest_Ctrl_Handle_Test_Mode_Event(u_int8 IRQ_Type);
u_int16 LEtest_Test_End(void);
t_error LEtest_Transmitter_Test(u_int8 tx_freq, u_int8 len_of_test_data,u_int8 packet_payload_type);
t_error LEtest_Receiver_Test(u_int8 rx_freq);



typedef struct {

	u_int8  address[6];
	u_int8  address_type;
	u_int8  used;
} t_ListEntry;
#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
extern t_ListEntry LE_adv_ind_duplicate_list[PRH_CFG_MAX_ADV_IND_DUPLICATE_ENTRIES];
extern t_ListEntry LE_adv_direct_duplicate_list[PRH_CFG_MAX_ADV_DIRECT_DUPLICATE_ENTRIES];
extern t_ListEntry LE_scan_ind_duplicate_list[PRH_CFG_MAX_ADV_SCAN_IND_DUPLICATE_ENTRIES];
extern t_ListEntry LE_adv_nonconn_ind_duplicate_list[PRH_CFG_MAX_ADV_NONCONN_IND_DUPLICATE_ENTRIES];
extern t_ListEntry LE_adv_scan_rsp_duplicate_list[PRH_CFG_MAX_SCAN_RSP_DUPLICATE_ENTRIES];
#endif
extern u_int8 LE_DuplicateListSize[SCAN_RSP_EVENT_TYPE+1];

#if (PRH_BS_CFG_SYS_LE_OBSERVER_DEVICE==1) || (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE==1)
extern t_ListEntry* LE_DuplicateList[SCAN_RSP_EVENT_TYPE+1] ;
#endif

struct s_all_queues
{
    /*
     * ACL Queues
     */
#if (PRH_BS_CFG_SYS_FLEXIBLE_DATA_BUFFER_SIZES_SUPPORTED==1)
    t_q_descr **l2cap_in_free_header_list;
    t_q_descr *l2cap_in_headers;
    u_int8 *l2cap_in_heaps;

    t_q_descr **l2cap_out_free_header_list;
    t_q_descr *l2cap_out_headers;
    u_int8 *l2cap_out_heaps;
#else
    t_q_descr *l2cap_in_free_header_list[PRH_BS_CFG_SYS_NUM_IN_ACL_PACKETS];
    t_q_descr l2cap_in_headers[PRH_BS_CFG_SYS_NUM_IN_ACL_PACKETS];
    u_int8 l2cap_in_heaps[PRH_BS_CFG_SYS_NUM_IN_ACL_PACKETS*
        mBTq_Get_Aligned_Data_Packet_Length(MIN_L2CAP_MTU)];

    t_q_descr *l2cap_out_free_header_list[PRH_BS_CFG_SYS_NUM_OUT_ACL_PACKETS];
    t_q_descr l2cap_out_headers[PRH_BS_CFG_SYS_NUM_OUT_ACL_PACKETS];
    u_int8 l2cap_out_heaps[PRH_BS_CFG_SYS_NUM_OUT_ACL_PACKETS*
        mBTq_Get_Aligned_Data_Packet_Length(MIN_L2CAP_MTU)];
#endif

#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
    /*
     * SCO Queues
     */
#if (PRH_BS_CFG_SYS_FLEXIBLE_DATA_BUFFER_SIZES_SUPPORTED==1)
    t_q_descr **sco_in_free_header_list;
    t_q_descr *sco_in_headers;
    u_int8 *sco_in_heaps;
    
    t_q_descr **sco_out_free_header_list;
    t_q_descr *sco_out_headers;
    u_int8 *sco_out_heaps;
#else
    t_q_descr *sco_in_free_header_list[PRH_BS_CFG_SYS_NUM_IN_SCO_PACKETS];
    t_q_descr sco_in_headers[PRH_BS_CFG_SYS_NUM_IN_SCO_PACKETS];
    u_int8 sco_in_heaps[PRH_BS_CFG_SYS_NUM_IN_SCO_PACKETS*
        mBTq_Get_Aligned_Data_Packet_Length(PRH_BS_CFG_SYS_SCO_IN_PACKET_LENGTH)];

    t_q_descr *sco_out_free_header_list[PRH_BS_CFG_SYS_NUM_OUT_SCO_PACKETS];
    t_q_descr sco_out_headers[PRH_BS_CFG_SYS_NUM_OUT_SCO_PACKETS];
    u_int8 sco_out_heaps[PRH_BS_CFG_SYS_NUM_OUT_SCO_PACKETS*
        mBTq_Get_Aligned_Data_Packet_Length(PRH_BS_CFG_SYS_SCO_OUT_PACKET_LENGTH)];
#endif

#endif
};

//le_power.c 
typedef     struct  {

    u_int32     volatile    adv_sleep_stat;
    u_int32     volatile    con_sleep_stat;

    /*u_int32     con_slave_connInterval_clk;*/
    u_int32     con_slave_current_clk;
    u_int32     con_slave_aux_timer_clk;
    u_int32     con_slave_aux_timer_Load;
    u_int32     con_slave_aux_timer_offset;
	
    u_int32     con_slave_wakeup_bt_clk;
    u_int32     con_slave_wakeup_Intraslot_offset;

    u_int32   	con_slave_wakeup_native_clk;
    u_int32 	native_clk_set;

    u_int32     con_slave_wait_tim2_Flg;

    u_int32     con_compensate_phase_flg;
    u_int32     con_intra_slot_save;
    int		    con_bt_clk_native_clk_margin;
    t_LE_Connection *pConnection; 
    u_int32     volatile    real_exit_lowpower;
	u_int8 ble_sleep_reserved[8];
}t_ble_sleep;
extern t_ble_sleep     ble_sleep;

typedef	struct {
	uint16_t	maxInterval;
	uint16_t	count;
}__attribute__((packed, aligned(4)))rc32k_t;


extern rc32k_t	rc32k_calib_table[7];

#define		RTC_BASE						0x40002000
#define		AO_TIMER_CTL					((volatile unsigned *)(RTC_BASE + 0x0040))
#define		FREQ_TIMER_VAL					((volatile unsigned *)(RTC_BASE + 0x0050))

extern uint32_t volatile native_bt_clk_restore;// = 0;
extern uint16_t	CALIB_COUNT;//	= 0;
extern uint16_t	CALIB_WAIT_TIME2_COUNT;// = 0;
extern uint32_t	volatile testlp;// = 0;

typedef struct
{
	uint32_t AO_TIMER_VALUE:16;
	uint32_t AO_TIMER_CLR:1;
	uint32_t AO_TIMER_EN:1;
	uint32_t FREQ_TIMER_EN:1;
	uint32_t RESERVED:13;
}AO_TIMER_CTL_T;
extern AO_TIMER_CTL_T* ao_timer_ctl;




extern uint8_t wakeup_clk_val ;

extern uint8_t continue_miss_packet_number_val ;
extern uint8_t next_connect_event_start_time_val;
extern uint8_t transmit_window_remaining_val ;
extern uint8_t ext_rx_window_val ;

//le_scan.c
extern u_int32    adv_rssi_val;//=0;

extern uint8_t init_scan_interval_val ;
extern uint8_t init_scan_window_val  ;

extern uint8_t adv_adjust_scan_window_val ;
extern uint8_t con_adjust_scan_window_val1 ;
extern uint8_t con_adjust_scan_window_val2 ;


//hci_dump.c
extern uint8_t dump_flag ;




//lE_advertise.c
extern u_int32 pkd_error_cnt;
extern uint8_t adv_always_wake ;
extern uint32_t adv_no_pka_count ;

//le_connection.c
extern uint32_t	ext_rx_window ;
extern uint8_t     connect_always_wake;
extern volatile uint8_t more_data_enable;//more data

//hw_radio.c

#if (PRH_BS_CFG_SYS_LE_CENTRAL_DEVICE == 1)
typedef struct
{
	u_int8  host_channel_map[5];
} t_LE_Frequency;

extern t_LE_Frequency LE_frequency;
#endif

extern t_RadioMode CurrentRadioMode ;


extern  t_event_heap event_heap_;
extern  u_int8* hci_event_heap ;
extern  t_q_descr hci_event_headers[PRH_BS_CFG_SYS_MAX_HCI_EVENTS];
extern  u_int8* hci_command_heap  ;
extern  t_q_descr hci_command_headers[PRH_BS_CFG_SYS_MAX_HCI_COMMANDS];
extern  u_int8* lmp_in_heap ;
extern  t_q_descr lmp_in_headers[PRH_BS_CFG_SYS_MAX_LMP_IN_MESSAGES];
extern  u_int8 lmp_out_heaps[MAX_ACTIVE_DEVICE_LINKS][PRH_BS_CFG_SYS_SIZEOF_LMP_OUT_HEAP];
extern  t_q_descr lmp_out_headers[MAX_ACTIVE_DEVICE_LINKS][PRH_BS_CFG_SYS_MAX_LMP_OUT_MESSAGES];    
extern  t_queue BTQueues[TRA_QUEUE_TOTAL_NUM_QUEUES];
extern  t_data_queue BTq_l2cap_in;
extern  t_data_queue BTq_l2cap_out;
extern  struct s_all_queues BTq_data_memory;
#if (PRH_BS_CFG_SYS_SYN_VIA_HCI_SUPPORTED==1)
extern  t_data_queue BTq_sco_in;
extern  t_data_queue BTq_sco_out;
extern  boolean BTq_sco_in_data_aggregation;
#endif

extern  u_int32 calibration_counter_1ms ;
extern  u_int32 calibration_counter_10us ;
extern  u_int32 calibration_counter_1us ;
extern  volatile u_int32 hw_delay_counter; /* extern'd for TCI use */
extern  t_LMconfig g_LM_config_info;
extern  t_role g_LM_test_device_role;
extern  u_int32 g_LM_device_state ;

extern u_int8 le_mode ; //Classic BT
extern pka_t   pka_op ;
extern pkd_t   pkd_op ;
extern  mHWreg_Create_Cache_Register(JAL_COM_CTRL_IRQ_REG);

extern  u_int32 DL_LC_link_pool;
extern  u_int32 hci_event_mask; 
extern  u_int32 hci_event_mask_extended;
extern  t_hc_flow_ctrl hc_flow_ctrl;


extern  u_int32 u_int32_TX_buffer[12];
extern  u_int32 u_int32_RX_buffer[12];
extern  t_LE_Advertising LE_advertise;
extern  t_LE_Config LE_config;
extern  u_int8 g_LE_Config_Classic_Dev_Index  ;
extern  t_LE_Connection LE_connections[DEFAULT_MAX_NUM_LE_LINKS];
extern  unsigned    int     Malloc_Error ;
extern  unsigned	int have_get_no_crc_pkt ;
extern  unsigned	int wrong_connect_req_pkt ;
extern  unsigned	char update_transmit_window_num ;
extern  unsigned	char continue_miss_packet_number ;
extern  u_int16 g_LE_Slave_IntraSlot ;

/************************************************************************
 * Prototypes for the Functions used to handle LLC messages 
 ************************************************************************/
t_error LE_LL_Connection_Update_Req(t_LE_Connection* p_connection, t_p_pdu p_pdu);
t_error LE_LL_Version_Ind(t_LE_Connection* p_connection,t_p_pdu p_pdu);
t_error LE_LL_Start_Enc_Req(t_LE_Connection* p_connection,t_p_pdu p_pdu);
t_error LE_LL_Start_Enc_Rsp(t_LE_Connection* p_connection,t_p_pdu p_pdu);
t_error LE_LL_Pause_Enc_Req(t_LE_Connection* p_connection,t_p_pdu p_pdu);
t_error LE_LL_Pause_Enc_Rsp(t_LE_Connection* p_connection,t_p_pdu p_pdu);
t_error LE_LL_Feature_Req(t_LE_Connection* p_connection,t_p_pdu p_pdu);
t_error LE_LL_Feature_Rsp(t_LE_Connection* p_connection,t_p_pdu p_pdu);
t_error LE_LL_Enc_Req(t_LE_Connection* p_connection,t_p_pdu p_pdu);
t_error LE_LL_Enc_Rsp(t_LE_Connection* p_connection,t_p_pdu p_pdu);
t_error LE_LL_Unknown_Rsp(t_LE_Connection* p_connection,t_p_pdu p_pdu);
t_error LE_LL_Terminate_Ind(t_LE_Connection* p_connection,t_p_pdu p_pdu);
t_error LE_LL_Reject_Ind(t_LE_Connection* p_connection,t_p_pdu p_pdu);
t_error LE_LL_UnSupported_Pdu(t_LE_Connection* p_connection,t_p_pdu p_pdu);

extern  t_ll_decode_function LL_Pdu_Decode_Handlers[];
extern  unsigned int after_rx_first_packet  ;
extern  s_int8 last_adv_rx_rssi ;
extern   unsigned char ll_pdu_lenth_table[14] ;

typedef struct
{

	u_int8  scanning_channel_index;
	u_int8  scanning_channels_used;
	u_int8  initiating_channel_index;
	u_int8  initiating_channels_used;
#if (PRH_BS_CFG_SYS_EXTENDED_HCI_COMMANDS_SUPPORTED==1)
	u_int8  scanning_channel_map;
	u_int8  initiating_channel_map;
	u_int8  backoff_enable;
#endif
//	u_int16 current_scan_window;
	u_int8  scan_type;
	u_int16 scan_interval;
	u_int16 scan_window;
	u_int8  scan_own_address_type;
	u_int8  scan_filter_policy;
	u_int8  scan_filter_duplicates;
	u_int8  scan_enable;
	t_timer next_scan_timer;
	t_timer current_scan_window_timer;
	u_int8   scan_backoff_count;
	u_int16  scan_upper_limit;
	u_int8  scan_num_consecutive_scan_resp_failure;
	u_int8  scan_num_consecutive_scan_resp_success;
	u_int8 scan_rf_freq;
	u_int8 num_tims_during_tx;
	u_int8 num_tims_during_rx;

} t_LE_Scan;
extern t_LE_Scan LE_scan;

typedef struct
{
u_int8 test_freq;
u_int8 len_of_test_data;
t_letestScenario packet_payload_type;
u_int32 num_packets_received;
u_int32 num_packets_received_withCRC;
u_int8 sub_state;
t_p_pdu data_payload;
u_int8 end_flag;
} t_LE_Test;
extern t_LE_Test LE_test;

extern u_int32 u_int32_payload[MAX_TEST_DATA_SIZE]; 

extern  void (*tra_hcit_initialise_table[TRA_HCIT_TYPE_ENDS])(void); 
extern  void (*tra_hcit_shutdown_table[TRA_HCIT_TYPE_ENDS])(void);
// LE Device db implemenation using static memory
#define BD_ADDR_LEN 6
typedef uint8_t bd_addr_t[BD_ADDR_LEN];
typedef struct le_device_memory_db {

    // Identification
    int addr_type;
    bd_addr_t addr;
    sm_key_t irk;

    // Stored pairing information allows to re-establish an enncrypted connection
    // with a peripheral that doesn't have any persistent memory
    sm_key_t ltk;
    uint16_t ediv;
    uint8_t  rand[8];
    uint8_t  key_size;
    uint8_t  authenticated;
    uint8_t  authorized;
    uint8_t  secure_connection;

#ifdef ENABLE_LE_SIGNED_WRITE
    // Signed Writes by remote
    sm_key_t remote_csrk;
    uint32_t remote_counter;

    // Signed Writes by us
    sm_key_t local_csrk;
    uint32_t local_counter;
#endif

} le_device_memory_db_t;
extern le_device_memory_db_t le_devices[MAX_NR_LE_DEVICE_DB_ENTRIES];

#endif