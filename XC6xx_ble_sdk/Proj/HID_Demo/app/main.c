#include    <stdio.h>
#include    <string.h>
#include "btstack_run_loop.h"
#include	"Includes.h"
#include    "profile.h"
#include    "ble.h"
#include "hids_device.h"
#include "bsp_gpio.h"
#include "sbc.h"
#include "voice_ringbuf.h"
#include "bsp_timer.h"
#include "bsp_pwm.h"
#include    "bsp_spi_flash.h"

uint8_t flag_show_hci = 0;

//#define EMPTY1  __attribute__((section("empty1")))
//#define EMPTY2  __attribute__((section("empty2")))

//uint32_t EMPTY1 dead_pixel_ram0 ;//__attribute__ ((at(0x10006d4c))) ;
//uint32_t  dead_pixel_ram1 __attribute__ ((at(0x10017af0))) ;
extern uint32_t  dead_pixel_ram1; //__attribute__ ((section(".ARM.__at_0x10017af0"))) ;

void _HWradio_Go_To_Idle_State_Patch(void){
}
extern uint32_t  GulSystickCount;
uint32_t  LastTimeGulSystickCount=0xFF;

void sm_peripheral_setup(void)
{
}

extern	void	gpio_mux_ctl(uint8_t num,uint8_t mux);
extern	void	gpio_fun_sel(uint8_t num,uint8_t sel);
extern	void    gpio_direction_input(uint8_t num, uint8_t pull_up_type);
extern  void    gpio_fun_inter(uint8_t num,uint8_t inter);
extern	uint8_t gpio_input_val(uint8_t num);
extern  void ble_system_idle_init(void);
extern  void    ble_system_idle(void);
extern  int btstack_main(void);
extern 	void send_media_report(int mediacode1,int mediacode2);
#define KEYPIN 0
#define KEYPIN1 1
#define LED1 4
#define LED2 5
static uint32_t min(uint32_t a, uint32_t b){
    return a < b ? a : b;
}

static int g_conn_stat = 0;
static int app_get_connect_state(void)
{
	return g_conn_stat;
}

// att_read_callback helpers
static uint16_t att_read_callback_handle_blob(const uint8_t * blob, uint16_t blob_size, uint16_t offset, uint8_t * buffer, uint16_t buffer_size){
    if (buffer){
        uint16_t bytes_to_copy = min(blob_size - offset, buffer_size);
        memcpy(buffer, &blob[offset], bytes_to_copy);
        return bytes_to_copy;
    } else {
        return blob_size;
    }
}

//static uint8_t read_buf[10] = {0};
// read requests
static uint16_t att_read_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size){

    printf("main %s, con_handle=%x, att_handle=%x, offset=%x, buffer=%x, size=%x\n",__func__, con_handle, att_handle, offset,(uint32_t)buffer, buffer_size);
    
    if((att_handle != ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE) && 
       (att_handle != ATT_CHARACTERISTIC_0000FF12_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE)) return 0;
    
    static uint8_t read_buf[10] = {0};
    return att_read_callback_handle_blob((const uint8_t *)read_buf, sizeof(read_buf), offset, buffer, buffer_size);
}

// write requests
static int att_write_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size){
    
    uint32_t le_notification_enabled;
    printf("main %s, con_handle=%x, att_handle=%x, offset=%x, buffer=%x, size=%x\n", __func__, con_handle, att_handle, offset, (uint32_t)buffer, buffer_size);  
    if (transaction_mode != ATT_TRANSACTION_MODE_NONE) return 0;
    switch(att_handle)
    {
    case    ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_CLIENT_CONFIGURATION_HANDLE:
        le_notification_enabled = little_endian_read_16(buffer, 0) == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION;
        printf("Notifications enabled %u\n",le_notification_enabled); 
        break;   
    case    ATT_CHARACTERISTIC_0000FF11_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE:
    case    ATT_CHARACTERISTIC_0000FF12_0000_1000_8000_00805F9B34FB_01_VALUE_HANDLE:
        printf("att_handle=0x%x, offset=0x%x, length=0x%x\n", att_handle, offset, buffer_size);
        printf("att data: ");
        for(uint32_t i=0; i<buffer_size; i++) printf("0x%x ", buffer[i]);
        printf("\n");
        break;
    default:
        break;
    }    
    return 0;
} 

hci_con_handle_t connection_handle ;

static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
	
    if (packet_type != HCI_EVENT_PACKET) return;
    switch(hci_event_packet_get_type(packet))
    {
    case BTSTACK_EVENT_STATE:
        if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) return;
        printf("BTstack up and running.\n");
        break;
        
    case HCI_EVENT_LE_META:
        switch (hci_event_le_meta_get_subevent_code(packet)) 
        {
        case HCI_SUBEVENT_LE_CONNECTION_COMPLETE: {
                connection_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
                printf("\n CONNECT RIGHT ! (HANDLE = 0x%x)\n", connection_handle);
//      	   gatt_client_read_value_of_characteristics_by_uuid16(packet_handler,connection_handle,0x01,0xff,0x2A00);
				g_conn_stat = 1;
			printf("Connected, requesting conn param update for handle 0x%04x\n", connection_handle);
            }		
            break;               
        default:
            break;
        }
        break;
            
    case HCI_EVENT_DISCONNECTION_COMPLETE:
        printf("\n DISCONNECT ! (HANDLE = 0x%x) ", hci_event_disconnection_complete_get_connection_handle(packet)); 
        printf("REASON = 0x%x\n", hci_event_disconnection_complete_get_reason(packet));
		g_conn_stat = 0;
        break;
    
    case L2CAP_EVENT_CONNECTION_PARAMETER_UPDATE_RESPONSE:   
        printf("L2CAP Connection Parameter Update Complete, response: %x\n", l2cap_event_connection_parameter_update_response_get_result(packet));
        break;

    case ATT_EVENT_MTU_EXCHANGE_COMPLETE:
        printf("ATT MTU = %u\n", att_event_mtu_exchange_complete_get_MTU(packet));
        break;
    
    case ATT_EVENT_HANDLE_VALUE_INDICATION_COMPLETE:
			  printf("INDICATION_COMPLETE = %u\n", att_event_handle_value_indication_complete_get_status(packet));
        (att_event_handle_value_indication_complete_get_status(packet) == 0) ? printf("INDICATION OK\n") : printf("INDICATION FAIL\n");
        break;
	
    default:
			//	printf("default = 0x%x\n", hci_event_packet_get_type(packet));
        break;
    }
}

static const blestack_init_t blestack_init = {
    
    /* uint16_t (*att_read_callback)(...);*/ att_read_callback,
    /* int (*att_write_callback)(...);    */ att_write_callback,
    /* void (*packet_handler)(...)        */ packet_handler,
    /* const uint8_t *ble_device_db       */ profile_data
};

const uint8_t adv_diect_data[] = {
    // Flags general discoverable, BR/EDR not supported
    0x02, 0x01, 0x06, 

	0x03, 0x03, 0x12, 0x18,
    // Name
    0x0a, 0x09, 'H', 'I', 'D',  '-','D','E','M','O','1',

    0x03, 0x19, 0xC1, 0x03, //keyboard
	//0x03, 0x19, 0xC2, 0x03,   //mouse
	// Service Solicitation, 128-bit UUIDs - ANCS (little endian)
//	0x11,0x15,0xD0,0x00,0x2D,0x12,0x1E,0x4B,0x0F,0xA4,0x99,0x4E,0xCE,0xB5,0x31,0xF4,0x05,0x79	
};
uint8_t adv_diect_data_len = sizeof(adv_diect_data);

extern uint8_t bd_addr[6];


const uint8_t adv_powerOn_data[] = {
    // Flags general discoverable, BR/EDR not supported
    0x02, 0x01, 0x04, 

	0x03, 0x02, 0x12, 0x18,
    
  0x03, 0x19, 0x80,0x01,
	0x0d,0xFF,0x5d,0x00,
	0x03,0x00,0x01,0x03,0x47,0x53,0xE4,0xAA,0xC6,0xB8

};
uint8_t adv_poweron_data_len = sizeof(adv_powerOn_data);

const uint8_t adv_pair_data[] = {
  0x02, 0x01, 0x05, 

	0x03, 0x02, 0x12, 0x18,
    
  0x03, 0x19, 0x80,0x01,
	0x0E,0x09,0x53,0x4b,0x59,0x57,0x4f,0x52,0x54,0x48,0x5f,0x30,0x31,0x37,0x32
};
uint8_t adv_pair_data_len = sizeof(adv_pair_data);



const uint8_t scanresp_data[] = {
    //- Manufacture.

    0x0E,0x09,0x53,0x4b,0x59,0x57,0x4f,0x52,0x54,0x48,0x5f,0x30,0x31,0x37,0x32
};
uint8_t scanresp_data_len = sizeof(scanresp_data);

void set_bd_addr()          
{ 
	extern uint8_t bd_addr[6];
    bd_addr[0]=0xad;
    bd_addr[1]=0x04;
    bd_addr[2]=0x02;
    bd_addr[3]=0x7e;
    bd_addr[4]=0x31;
    bd_addr[5]=0x28;
}
extern int key_value ;

void key_init(void)
{
//	gpio_mux_ctl(KEYPIN,0);
//	gpio_fun_sel(KEYPIN,0);
//	gpio_fun_inter(KEYPIN,0);
//	gpio_direction_input(KEYPIN,0);
//	
//		gpio_mux_ctl(KEYPIN1,0);
//	gpio_fun_sel(KEYPIN1,0);
//	gpio_fun_inter(KEYPIN1,0);
//	gpio_direction_input(KEYPIN1,0);
	
//		

//	
	
//	gpio_mux_ctl(LED2,0);
//	gpio_fun_sel(LED2,0);
//	gpio_fun_inter(LED2,0);
//	gpio_direction_output(LED2);
//	gpio_output_low(LED2);
	
		gpio_mux_ctl(LED1,0);
	gpio_fun_sel(LED1,0);
	gpio_fun_inter(LED1,0);
	gpio_direction_output(LED1);
	gpio_output_low(LED1);
	
}

enum adv_type{
	ADV_IND,
	ADV_DIRECT_IND,
	ADV_SCAN_IND,
	ADV_NONCONN_IND,
};


void send_power_on_adv(void)
{
	flag_show_hci =1;
	printf("send_power_on_adv\n");
	uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
	 bd_addr_t null_addr;
    memset(null_addr, 0, 6);
	gap_advertisements_enable(0);
	 gap_advertisements_set_params(adv_int_min, adv_int_max, ADV_IND, 0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data(adv_poweron_data_len, (uint8_t*) adv_powerOn_data);
    gap_scan_response_set_data(scanresp_data_len , (uint8_t*) scanresp_data);
    gap_advertisements_enable(1);
}

void stack_reset(void)
{
	bd_addr_t null_addr;
	btstack_main();
	voice_ring_buffer_init();
	uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
	memset(null_addr, 0, 6);
	gap_advertisements_set_data(sizeof(adv_pair_data), (uint8_t*) adv_pair_data);
	gap_scan_response_set_data(sizeof(scanresp_data), (uint8_t*) scanresp_data);
	gap_advertisements_set_params(adv_int_min, adv_int_max, ADV_IND, 0, null_addr, 0x07, 0x00);
	gap_advertisements_enable(1);
}

extern uint8_t con_flag;
sbc_t sbc;
#define BUF_SIZE 266
static unsigned char input[BUF_SIZE]; 
//, output[BUF_SIZE + BUF_SIZE / 4];

unsigned char output[57] = {
0xad, 0x00, 0x00, 0x32, 0xfd, 0xcc, 0xbc, 0xbb, 0x7d, 0xdd, 0xad, 0x5f, 0x77, 0x6b, 0x58, 0x1d, 

0xda, 0xd6, 0x16, 0x86, 0xb5, 0xaa, 0x99, 0x31, 0x6b, 0x66, 0x6b, 0x5a, 0x68, 0xdc, 0xc5, 0x10, 
0xac, 0x92, 0x39, 0xb1, 0x8e, 0x16, 0x67, 0x6b, 0x55, 0xdd, 0xda, 0xd5, 0x97, 0x76, 0xb5, 0x6d, 
0xdd, 0xad, 0x5d, 0x77, 0x6b, 0x57, 0xdd, 0xda, 0xd4
};
void sbc_enc_params_print(uint8_t *out_put,uint16_t len);

uint32_t count_1,count_2,count_3;
static btstack_timer_source_t sys_run_timer;

#include "xc_kbs_event.h"
#include "le_device_db.h"
static void system_run_timer_handler(btstack_timer_source_t * ts){

	static uint8_t on_off = 0;
	static uint8_t key_dump = 0;
	
	static uint8_t ocpy_ratio = 10;
	
	ocpy_ratio++;
	if(ocpy_ratio >= 99)
	{
		ocpy_ratio = 10;	
	}
	xc_set_pwm(2,ocpy_ratio, 159);
	if(on_off == 0)
	{
		
		GPIO_OUTPUT_HIGH(LED1);
		on_off = 1;
	//	printf("LED1 OFF\n");
	}else
	{
	
		
		GPIO_OUTPUT_LOW(LED1);
		on_off = 0;
	//	printf("LED1 ON\n");
	}	
	 btstack_run_loop_set_timer(ts, 500);
   btstack_run_loop_add_timer(ts);

}

#include "fds.h"
typedef enum
{
    ES_FLASH_ACCESS_READ,  //!< Read data.
    ES_FLASH_ACCESS_WRITE, //!< Write data.
    ES_FLASH_ACCESS_CLEAR  //!< Clear data.
} es_flash_access_t;
/**@brief Structure used for invoking flash access function. */
typedef struct
{
    uint16_t          record_key;
    uint16_t          file_id;
    uint8_t *         p_data_buf;
    uint8_t *         p_data;
    uint16_t          size_bytes;
    es_flash_access_t access_type;
} flash_access_params_t;

#define RETURN_IF_ERROR(PARAM)                                                                     \
    if ((PARAM) != NRF_SUCCESS)                                                                    \
    {                                                                                              \
        return (PARAM);                                                                            \
    }

static volatile uint32_t m_num_pending_ops;  

/**@brief Function performing flash access (read/write/clear).
 *
 * @param[in] p_params Flash access parameters.
 */
static ret_code_t access_flash_data(const flash_access_params_t * p_params)
{
    ret_code_t         err_code;
    fds_flash_record_t record = {0};
    fds_record_desc_t  desc   = {0};
    fds_find_token_t   ft     = {0};
    fds_record_t       record_to_write =
    {
        .data.p_data = p_params->p_data_buf,
        .file_id     = p_params->file_id
    };

    err_code = fds_record_find_by_key(p_params->record_key, &desc, &ft);

    // If its a read or clear, we can not accept errors on lookup
    if (p_params->access_type == ES_FLASH_ACCESS_READ)
    {
        RETURN_IF_ERROR(err_code);
    }

    if (p_params->access_type == ES_FLASH_ACCESS_CLEAR && err_code == FDS_ERR_NOT_FOUND)
    {
        return NRF_SUCCESS;
    }

    switch (p_params->access_type)
    {
        case ES_FLASH_ACCESS_READ:
            err_code = fds_record_open(&desc, &record);
            RETURN_IF_ERROR(err_code);

            memcpy(p_params->p_data, record.p_data, p_params->size_bytes);

            err_code = fds_record_close(&desc);
            RETURN_IF_ERROR(err_code);

            break;

        case ES_FLASH_ACCESS_WRITE:
            memcpy(p_params->p_data_buf, p_params->p_data, p_params->size_bytes);

            record_to_write.data.length_words = (p_params->size_bytes +3) / 4;
            record_to_write.key               = p_params->record_key;

            if (err_code == FDS_ERR_NOT_FOUND)
            {
                err_code = fds_record_write(&desc, &record_to_write);
            }

            else
            {
                err_code = fds_record_update(&desc, &record_to_write);
            }

            RETURN_IF_ERROR(err_code);
            m_num_pending_ops++;
            break;

        case ES_FLASH_ACCESS_CLEAR:
            err_code = fds_record_delete(&desc);
            RETURN_IF_ERROR(err_code);
            m_num_pending_ops++;
            break;

        default:
            break;
    }
    return NRF_SUCCESS;
}
#define ESCS_AES_KEY_SIZE               (16)
#define SIZE_OF_LOCK_KEY ESCS_AES_KEY_SIZE  //!< Size of lock key.
#define FILE_ID_ES_FLASH_LOCK_KEY 0x1338    //!< File ID used for lock code flash access.
#define RECORD_KEY_LOCK_KEY 0x4             //!< File record for lock key.
__ALIGN(4) static uint8_t lock_key_buf[SIZE_OF_LOCK_KEY];   //!< Buffer for lock key flash access.

ret_code_t es_flash_access_lock_key(uint8_t * p_lock_key, es_flash_access_t access_type)
{
    flash_access_params_t params = {.record_key  = RECORD_KEY_LOCK_KEY,
                                    .file_id     = FILE_ID_ES_FLASH_LOCK_KEY,
                                    .p_data_buf  = lock_key_buf,
                                    .p_data      = (uint8_t *)p_lock_key,
                                    .size_bytes  = SIZE_OF_LOCK_KEY,
                                    .access_type = access_type};

    return access_flash_data(&params);
}

static void fds_evt_handler(fds_evt_t const * const p_fds_evt)
{
	printf("fds_evt_handler id:%d\n",p_fds_evt->id);
}

int	main(void)
{
	int codesize = 0;
	unsigned char *inp, *outp;
	ssize_t encoded;
	size_t decoded;
	ssize_t len;
	ssize_t outp_len;
	int framelen;
	set_bd_addr();
  ble_init((void *)&blestack_init);

	btstack_main();
	key_init();
		uint32_t idx = 0;
	
	uint32_t value ;
//	uint8_t *p_buf = 0x10000000 + 1222 * 1024;
#define RAM_BASE(idx)         ((volatile unsigned *)(0x10000000 + 122 * 1024 + idx))


for(int j = 0 ; j < 6 * 1024; j+=4)
{
	__write_hw_reg32(RAM_BASE(j),0xffffffff);
}
 idx = 0;
	__write_hw_reg32(RAM_BASE(idx),0xDEADC0DE);__read_hw_reg32(RAM_BASE(idx),value);
printf("value0 ret:%x\n",value);
idx = 4;
	__write_hw_reg32(RAM_BASE(idx),0xF11E01FE);__read_hw_reg32(RAM_BASE(idx),value);
printf("value1 ret:%x\n",value);

idx = 0 + 512;
	__write_hw_reg32(RAM_BASE(idx),0xDEADC0DE);__read_hw_reg32(RAM_BASE(idx),value);
printf("value2 ret:%x\n",value);
idx = 0 + 512 + 4;
	__write_hw_reg32(RAM_BASE(idx),0xF11E01FE);__read_hw_reg32(RAM_BASE(idx),value);
printf("value3 ret:%x\n",value);


idx = 512 * 2;
	__write_hw_reg32(RAM_BASE(idx),0xDEADC0DE);__read_hw_reg32(RAM_BASE(idx),value);
printf("value4 ret:%x\n",value);
idx=  512 * 2 + 4;
	__write_hw_reg32(RAM_BASE(idx),0xF11E01FE);__read_hw_reg32(RAM_BASE(idx),value);
printf("value5 ret:%x\n",value);




	ret_code_t ret;
//	ret = fds_register(fds_evt_handler);
//	printf("fds_register ret:%d\n",ret);
//	ret = fds_init();
//	printf("fds_init ret:%d\n",ret);
	uint8_t buf[80];
	
	for(int i = 0;i < 30;i++)
	{
		buf[i] = i;
	}
	
//	es_flash_access_lock_key(buf,ES_FLASH_ACCESS_WRITE);
//	for(int i = 0;i < 30;i++)
//	{
//		buf[i] = 0;
//	}
//	
//	es_flash_access_lock_key(buf,ES_FLASH_ACCESS_READ);
//	
//	for(int i = 0;i < 30;i++)
//	{
//		printf("buf[%d]:%d\n",i,buf[i]); 
//	}
//	
	uint32_t *p_buf = 0;
	uint32_t value1;
	//buf = 0;
	#define RAM_BASE1(idx)         ((volatile unsigned *)(0x00000000 + idx))

//	for(uint32_t i = 0;i < 30;i+=4)
//	{
//		__read_hw_reg32(RAM_BASE1(i),value1);
//		printf("p_buf[%d]:0x%x,0x%x\n",i,*p_buf++,value1); 
//	}
flash_test();
uint8_t len1;uint16_t totalLen;
	for(len1 = 0; len1 < 10; len1++)
	{
		totalLen =   ((len1 + 3) & ~(0x4 -1)) + 4;
		printf("len:%d,totalLen:%d\n",len1,totalLen); 
	}

//	xc620_kbs_init();
	voice_ring_buffer_init();
	
    // setup advertisements
    uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;

    bd_addr_t null_addr;
    memset(null_addr, 0, 6);
    gap_advertisements_set_params(adv_int_min, adv_int_max, ADV_IND, 0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data(adv_pair_data_len, (uint8_t*) adv_pair_data);
    gap_scan_response_set_data(scanresp_data_len , (uint8_t*) scanresp_data);
    gap_advertisements_enable(1);
	//  ble_system_idle_init();
	con_flag = 1;
	printf("sbc_init_msbc\n");
#ifdef SBC_ENABLE
	sbc_init_msbc(&sbc, 0);
	codesize = sbc_get_codesize(&sbc);
#endif
	inp = input;
	outp = output;
	outp_len = codesize;
	memset(input,0,240);
//	memset(output,0,BUF_SIZE);
	for(int i = 0;i < BUF_SIZE;i++)
	{
		input[i] = 0x50  + i;
	}
//	xc_timer_init(TIMER_CH2,500000);
//		xc_timer_init(TIMER_CH3,1000000);
	GPIO_OUTPUT_HIGH(LED1);
	GPIO_OUTPUT_LOW(LED1);
#if SBC_ENCODER_EN
	len = sbc_encode(&sbc, inp, 240,
				outp, 240,
				&encoded);
#endif
	GPIO_OUTPUT_HIGH(LED1);
	GPIO_OUTPUT_LOW(LED1);
	GPIO_OUTPUT_HIGH(LED1);
	GPIO_OUTPUT_LOW(LED1);
	
	printf("encoded:%d,len:%d\n",encoded,len);
	sbc_enc_params_print(output,encoded);
	

	printf("encoded:%d,len:%d\n",encoded,len);
	
	gpio_mux_ctl(0,2);
	gpio_fun_sel(0,12);//pwm0
	gpio_fun_inter(0,0);
	gpio_direction_output(0);
	
	xc_pwm_init(2,10,159);
	
	
#if SBC_DECODER_EN
	framelen = sbc_decode(&sbc, output, 57, input, 240, &decoded);
	printf("decoded:%d,framelen:%d\n",decoded,framelen);
	sbc_enc_params_print(input,240);
#endif
	sys_run_timer.process = &system_run_timer_handler;
	btstack_run_loop_set_timer(&sys_run_timer, 100);
	btstack_run_loop_add_timer(&sys_run_timer);
	

void ff11_test_loop(void);
    while(1) {
       ble_mainloop();
			
		//	ff11_test_loop();
	//   ble_system_idle();
       if(LastTimeGulSystickCount!=GulSystickCount)//10msÖ´ÐÐÒ»´Î
	   {		   
		   LastTimeGulSystickCount=GulSystickCount;
		 //  key_press();
			// xc620_kbs_scan();
	   }		   


    }
}

#if 1
void sbc_enc_params_print(uint8_t *out_put,uint16_t len)
{
	int i = 0, j = 0;
	for(i = 0;i < len; i++ )
	{
		printf("0x%02x, ",out_put[i]);
		if(i % 16 == 15)
		{
			printf("\r\n");
		}
	}
	printf("\r\n");	
//	printf("s16SamplingFreq:%d\n",param.s16SamplingFreq);                         /* 16k, 32k, 44.1k or 48k*/
//    printf("s16ChannelMode:%d\n",param.s16ChannelMode); ;                          /* mono, dual, streo or joint streo*/
//    printf("s16NumOfSubBands:%d\n",param.s16NumOfSubBands) ;                        /* 4 or 8 */
//    printf("s16NumOfChannels:%d\n",param.s16NumOfChannels) ;
//    printf("s16NumOfBlocks:%d\n",param.s16NumOfBlocks) ;                          /* 4, 8, 12 or 16*/
//    printf("s16AllocationMethod:%d\n",param.s16AllocationMethod) ;                     /* loudness or SNR*/
//    printf("s16BitPool:%d\n",param.s16BitPool) ;                              /* 16*numOfSb for mono & dual;
//                                                       32*numOfSb for stereo & joint stereo */
//    printf("u16BitRate:%d\n",param.u16BitRate) ;
//    printf("sbc_mode:%d\n",param.sbc_mode)  ;                                /* SBC_MODE_STD or SBC_MODE_MSBC */
//    printf("u8NumPacketToEncode:%d\n",param.u8NumPacketToEncode)   ;                    /* number of sbc frame to encode. Default is 1 */

//    printf("s16MaxBitNeed:%d\n",param.s16MaxBitNeed) ;
//	printf("FrameHeader:0x%x\n",param.FrameHeader) ;
	
}

#endif
