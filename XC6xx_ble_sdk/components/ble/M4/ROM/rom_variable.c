#include "rom_variable.h"
#include "patch_function_id.h"


void rom_variable_init()
{
    hci_event_heap = event_heap_.hci_event_heap_array_;

    hci_command_heap = command_heap_.hci_command_heap_array_;

    lmp_in_heap = lmp_in_heap_.lmp_in_heap_array_;

    calibration_counter_1ms=SYS_HAL_PRECALIBRATED_DELAY_MS;
    calibration_counter_10us=SYS_HAL_PRECALIBRATED_DELAY_10US;
    calibration_counter_1us=SYS_HAL_PRECALIBRATED_DELAY_US;

    LL_Pdu_Decode_Handlers[LL_CONNECTION_UPDATE_REQ ] = LE_LL_Connection_Update_Req;
    LL_Pdu_Decode_Handlers[LL_CHANNEL_MAP_REQ ] = LEfreq_Update_Channel_Map_Req;
    LL_Pdu_Decode_Handlers[LL_TERMINATE_IND] = LE_LL_Terminate_Ind;

    LL_Pdu_Decode_Handlers[LL_ENC_REQ] = LE_LL_Enc_Req;
    LL_Pdu_Decode_Handlers[LL_ENC_RSP]   =LE_LL_Enc_Rsp ,
    LL_Pdu_Decode_Handlers[LL_START_ENC_REQ] = LE_LL_Start_Enc_Req;
    LL_Pdu_Decode_Handlers[LL_START_ENC_RSP] = LE_LL_Start_Enc_Rsp;
    LL_Pdu_Decode_Handlers[LL_UNKNOWN_RSP] = LE_LL_Unknown_Rsp;
    LL_Pdu_Decode_Handlers[LL_FEATURE_REQ] =      LE_LL_Feature_Req;
    LL_Pdu_Decode_Handlers[LL_FEATURE_RSP] =       LE_LL_Feature_Rsp;
    LL_Pdu_Decode_Handlers[LL_PAUSE_ENC_REQ] =        LE_LL_Pause_Enc_Req;
    LL_Pdu_Decode_Handlers[LL_PAUSE_ENC_RSP ] =       LE_LL_Pause_Enc_Rsp;
    LL_Pdu_Decode_Handlers[LL_VERSION_IND ] =         LE_LL_Version_Ind;
    LL_Pdu_Decode_Handlers[LL_REJECT_IND ] =         LE_LL_Reject_Ind;

    uint8_t temp_table[14]  = {12,8,2,23,13,1,1,2,9,9,1,1,6,2};
    memcpy(ll_pdu_lenth_table,temp_table,14);

    LE_DuplicateList[0] = LE_adv_ind_duplicate_list;
    LE_DuplicateList[1] = LE_adv_direct_duplicate_list;
    LE_DuplicateList[2] = LE_scan_ind_duplicate_list;
    LE_DuplicateList[3] = LE_adv_nonconn_ind_duplicate_list;
    LE_DuplicateList[4] = LE_adv_scan_rsp_duplicate_list;

    u_int8 LE_DuplicateListSize_tmp[SCAN_RSP_EVENT_TYPE+1] = {PRH_CFG_MAX_ADV_IND_DUPLICATE_ENTRIES,
    PRH_CFG_MAX_ADV_DIRECT_DUPLICATE_ENTRIES,
    PRH_CFG_MAX_ADV_SCAN_IND_DUPLICATE_ENTRIES,
    PRH_CFG_MAX_ADV_NONCONN_IND_DUPLICATE_ENTRIES,
    PRH_CFG_MAX_SCAN_RSP_DUPLICATE_ENTRIES};

    memcpy(LE_DuplicateListSize,LE_DuplicateListSize_tmp,SCAN_RSP_EVENT_TYPE+1);

    tra_hcit_transmit_table[TRA_HCIT_TYPE_GENERIC] = TRAhcit_Generic_Dispatcher;

    sig_seq_nr= 0xff;
    source_cid  = 0x40;

    char_to_nibble = "0123456789ABCDEF";


    att_server_last_can_send_now = HCI_CON_HANDLE_INVALID;

    dump_file = -1;

    log_level_enabled[0] = 0;
    log_level_enabled[1] = 0;
    log_level_enabled[2] = 0;
    log_level_enabled[3] = 0;

 //   tick_handler = &dummy_handler_no_param;

    u_int8 bd_addr_tmp[6] = {'1', 'v', 'c', 'n', 'i', 'X'};

    memcpy(bd_addr,bd_addr_tmp,6);


    rc32k_t	rc32k_calib_table_tmp[] =
    {
        [0] = {	
                .maxInterval = 100,	 
                .count = 2,  
                },
        [1] = {
                .maxInterval = 150,	 
                .count = 2, 
                },
        [2] = {
                .maxInterval = 300,	 
                .count = 2, 
                },		  
        [3] = {
                .maxInterval = 500,
                .count = 2, 
                },
        [4] = {
                .maxInterval = 1000,
                .count = 2, 
                },
        [5] = {
                .maxInterval = 2000,
                .count = 3,
                },
        [6] = {
                .maxInterval = 4000,
                .count = 3,
                }			
    };

    memcpy(rc32k_calib_table,rc32k_calib_table_tmp,sizeof(rc32k_calib_table_tmp)); 

    #define		RTC_BASE						0x40002000
    #define		AO_TIMER_CTL					((volatile unsigned *)(RTC_BASE + 0x0040))
    ao_timer_ctl = (AO_TIMER_CTL_T*)AO_TIMER_CTL;

    g_le_enable_observer_flag = 0;
    g_le_enable_central_flag = 0;

    wakeup_clk_val = 5;
    continue_miss_packet_number_val = 3;
    next_connect_event_start_time_val= 5;
    transmit_window_remaining_val = 3;
    ext_rx_window_val = 5;

    init_scan_interval_val = 0x10;
    init_scan_window_val   = 0x10;

    adv_adjust_scan_window_val = 0x20;
    con_adjust_scan_window_val1 = 0x10;
    con_adjust_scan_window_val2 = 40;

    ext_rx_window = 1;

    CurrentRadioMode = RADIO_MODE_STANDBY;

    uint8_t packet_sent_event_tmp[] = { HCI_EVENT_TRANSPORT_PACKET_SENT, 0};
    memcpy(packet_sent_event,packet_sent_event_tmp,sizeof(packet_sent_event_tmp));


    hci_packet =  &hci_packet_with_pre_buffer[HCI_INCOMING_PRE_BUFFER_SIZE];

    sm_auth_req = 1 ;
    sm_io_capabilities = IO_CAPABILITY_NO_INPUT_NO_OUTPUT;

    setup = &the_setup;

    sm_active_connection_handle = HCI_CON_HANDLE_INVALID;

    packet_handler = dummy_handler;

    LE_Active_Mode = 1;


    for(int i = 0 ;i < MAX_PATCH_NUM;i++)
    {
        PATCH_FUN[i] = 0;
    }
	////rom 调用 ram的函数指针

    PATCH_FUN[XINC_TRACE_U32_PRINT] = xinc_trace_u32_print;
    PATCH_FUN[SENDCHAR] = sendchar;
    PATCH_FUN[LE_RF_INIT] = le_rf_init;
    PATCH_FUN[INIT_TIMER] = Init_Timer;
    PATCH_FUN[TIMER_DISABLE] = Timer_disable;
    PATCH_FUN[TIMER_CURRENT_COUNT] = timer_current_count;

    PATCH_FUN[STACK_RESET] = 	  stack_reset;
    PATCH_FUN[HWRADIO_LE_SET_ACTIVE_MODE] = 	  HWradio_LE_Set_Active_Mode;   
    PATCH_FUN[HWRADIO_INITIALISE] = 	  HWradio_Initialise;
    PATCH_FUN[HWRADIO_RESET] = 	  HWradio_Reset;
    PATCH_FUN[HWRADIO_DISABLEALLSPIWRITES] =   HWradio_DisableAllSpiWrites;
    PATCH_FUN[HWRADIO_LE_RXCOMPLETE] = 	  HWradio_LE_RxComplete;
    PATCH_FUN[HWRADIO_LE_SERVICE] = 	  HWradio_LE_Service;
    PATCH_FUN[HWRADIO_LE_TXCOMPLETE] =    HWradio_LE_TxComplete;
    PATCH_FUN[HWRADIO_LE_SETUP_RADIO_FOR_NEXT_TXRX] = 	  HWradio_LE_Setup_Radio_For_Next_TXRX;
    PATCH_FUN[HWRADIO_CONVERT_TX_POWER_LEVEL_UNITS_TO_TX_POWER] = 	  HWradio_Convert_Tx_Power_Level_Units_to_Tx_Power;
    PATCH_FUN[HWRADIO_LE_READ_RSSI] = 	  HWradio_LE_Read_RSSI;
    PATCH_FUN[HWRADIO_DEC_TX_POWER_LEVEL] = 	  HWradio_Dec_Tx_Power_Level;
    PATCH_FUN[HWRADIO_INC_TX_POWER_LEVEL] = 	  HWradio_Inc_Tx_Power_Level;
    PATCH_FUN[HWRADIO_SETUP_FOR_TIFS_EVENT] = 	  HWradio_Setup_For_TIFS_Event;
    PATCH_FUN[_HWRADIOCLEANUPRX] = 	  _HWradioCleanUpRx;
    //	
    PATCH_FUN[HWRADIO_TXCOMPLETE] = 	  HWradio_TxComplete;
    PATCH_FUN[HWRADIO_PROGRAM_DUMMY_RX] = 	  HWradio_Program_Dummy_Rx;


}