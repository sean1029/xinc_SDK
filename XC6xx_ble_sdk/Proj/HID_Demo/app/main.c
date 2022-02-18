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
#include "nrf_gpio.h"
#include "bsp_uart.h"

#include "nrfx_gpiote.h"
#include "nrf_drv_spi.h"

static const nrf_drv_spi_t m_spi = NRF_DRV_SPI_INSTANCE(1);  /**< SPI instance. */

uint8_t flag_show_hci = 0;


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
	Init_gpio();

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


	static uint8_t ocpy_ratio = 10;

	ocpy_ratio++;
//	g_sys_time++;
	if(ocpy_ratio >= 99)
	{
		ocpy_ratio = 10;	
	}
	//xc_set_pwm(2,ocpy_ratio, 159);
	if(on_off == 0)
	{
		
		GPIO_OUTPUT_HIGH(4);
		on_off = 1;
	//	led_value = nrf_gpio_pin_read(LED1);

	//	printf("led_value:%d,led_value1:%d\n",led_value,led_value1);
	}else
	{
	
		
		GPIO_OUTPUT_LOW(4);
		on_off = 0;
	//	printf("LED1 ON\n");
	}	
	 btstack_run_loop_set_timer(ts, 500);
   btstack_run_loop_add_timer(ts);

}


#include "app_button.h"
#include "app_error.h"
#include "app_timer.h"
#include "bsp.h"

uint8_t buff1[320];
uint8_t buff2[320];
uint8_t buff3[320];

extern uint8_t list_handler_sched_flag;

extern void extern_timer_list_handler(void);


#include "bsp.h"
static void bsp_evt_handler(bsp_event_t evt)
{
    switch (evt)
    {
        case BSP_EVENT_KEY_0:
						printf("button 0 push,evt: %d\n",BSP_EVENT_KEY_0);
						nrf_drv_spi_transfer(&m_spi,buff2,280,buff3,280);
						bsp_board_led_on(0);
            break;

        case BSP_EVENT_KEY_1:
					
				printf("button 0 long push,evt: %d\n",BSP_EVENT_KEY_1);
            break;

        case BSP_EVENT_KEY_2:
					printf("button 0 release ,evt: %d\n",BSP_EVENT_KEY_2);
				bsp_board_led_off(0);
            break;

         case BSP_EVENT_KEY_3:
						printf("button 1 push,evt: %d\n",BSP_EVENT_KEY_3);
				 bsp_board_led_on(1);
            break;

        case BSP_EVENT_KEY_4:
						printf("button 1 long push,evt: %d\n",BSP_EVENT_KEY_4);
            break;

        case BSP_EVENT_KEY_5:
          printf("button 1 release ,evt: %d\n",BSP_EVENT_KEY_5);
				 bsp_board_led_off(1);
            break;

        default:
            break; //No implementation needed
    }

   
}
#include "app_scheduler.h"
// SCHEDULER CONFIGS
#define SCHED_MAX_EVENT_DATA_SIZE           APP_TIMER_SCHED_EVENT_DATA_SIZE             //!< Maximum size of the scheduler event data.
#define SCHED_QUEUE_SIZE                    10                                          //!< Size of the scheduler queue.

static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}
#include "nrf_cli_uart.h"
#include "nrf_cli.h"
#include "app_uart.h"

NRF_CLI_UART_DEF(cli_uart, 1, 64, 64);
NRF_CLI_DEF(m_cli_uart, "cli:~$ ", &cli_uart.transport, '\r', 4);

void cli_init(void)
{

    nrf_drv_uart_config_t uart_config;
    uart_config.pseltxd = TX_PIN_NUMBER;
    uart_config.pselrxd = RX_PIN_NUMBER;
		uart_config.hwfc    = NRF_UART_HWFC_DISABLED;
		uart_config.baudrate = UART_BAUDRATE_BAUDRATE_Baud115200;
	
    ret_code_t err_code = nrf_cli_init(&m_cli_uart, &uart_config, false, false, NRF_LOG_SEVERITY_NONE);
    APP_ERROR_CHECK(err_code);
}

//void uart_event_handle(app_uart_evt_t * p_event)
//{
//    static uint8_t data_array[20];
//    static uint16_t index = 0;
//    uint32_t ret_val;
//	uint32_t err_code;
//	int i = 0;
//	//  printf("uart_event_handle ,evt_type:%d\r\n",p_event->evt_type);
//    switch (p_event->evt_type)
//    {
//        /**@snippet [Handling data from UART] */
//        case APP_UART_DATA_READY:
//            UNUSED_VARIABLE(app_uart_get(&data_array[0]));
//            index++;
//						app_uart_get(&data_array[0]);
//					//	printf("Data:%c\r\n",data_array[0]);
//					//	for( i = 0; i< 16;i++)
//						{
//								err_code = app_uart_put(data_array[0] + 0);
//								if(err_code != 0)
//								{
//									break;
//								}
//						}
//					//	printf("uart_put %d data\r\n",i);	
//            index = 0;
//            
//            break;

//        /**@snippet [Handling data from UART] */
//        case APP_UART_COMMUNICATION_ERROR:
//         
//            break;

//        case APP_UART_FIFO_ERROR:
//         
//            break;
//				
//				case APP_UART_DATA:
//						app_uart_get(&data_array[0]);
//				//	  printf("data:%c \r\n",data_array[0]);
//				app_uart_put(data_array[0]);
//            break;

//        default:
//            break;
//    }
//}


//#define UART_TX_BUF_SIZE        64                                     /**< UART TX buffer size. */
//#define UART_RX_BUF_SIZE        64                                     /**< UART RX buffer size. */
//static void uart_init(void)
//{
//    ret_code_t err_code;

//    app_uart_comm_params_t const comm_params =
//    {
//        .rx_pin_no    = RX_PIN_NUMBER,
//        .tx_pin_no    = TX_PIN_NUMBER,
//        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
//        .use_parity   = false,
//        .baud_rate    = UART_BAUDRATE_BAUDRATE_Baud115200
//    };

//    APP_UART_FIFO_INIT(&comm_params,
//                       UART_RX_BUF_SIZE,
//                       UART_TX_BUF_SIZE,
//                       uart_event_handle,
//                       APP_IRQ_PRIORITY_LOWEST,
//                       err_code);

//    APP_ERROR_CHECK(err_code);
//}


void ff11_test_loop(void);

static void spi_handler(nrf_drv_spi_evt_t const * p_event,
                        void *                    p_context)
{
	printf("%s,type:%d\r\n",__func__,p_event->type);
	
	const uint8_t *tx_data = p_event->data.done.p_tx_buffer;

	printf("tx_length %d\r\n",p_event->data.done.tx_length);
}


	
int	main(void)
{


	//ssize_t encoded;




	set_bd_addr();


	
//	printf("%s\r\n",__func__);

  ble_init((void *)&blestack_init);

	btstack_main();
	scheduler_init();
	app_timer_init();
		key_init();
	//nrfx_gpiote_init();
	  bsp_init(BSP_INIT_BUTTONS | BSP_INIT_LEDS,bsp_evt_handler);
		
		
	//	nrf_gpio_cfg_input(1, NRF_GPIO_PIN_PULLDOWN);
  // nrf_gpio_cfg_input(0, NRF_GPIO_PIN_PULLDOWN);
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
	int codesize = 0;
	sbc_init_msbc(&sbc, 0);
	codesize = sbc_get_codesize(&sbc);
#endif

#if SBC_DECODER_EN
	int framelen;
	size_t decoded;
	framelen = sbc_decode(&sbc, output, 57, input, 240, &decoded);
	printf("decoded:%d,framelen:%d\n",decoded,framelen);
	sbc_enc_params_print(input,240);
#endif
	sys_run_timer.process = &system_run_timer_handler;
	btstack_run_loop_set_timer(&sys_run_timer, 100);
	btstack_run_loop_add_timer(&sys_run_timer);
	
	
	



//	gpio_mux_ctl(1,0);
//	gpio_fun_sel(1,12);//pwm0
//	gpio_fun_inter(1,0);
//	gpio_direction_output(1);
//	
//	xc_pwm_init(0,10,159);
//	Init_uart(1,BAUD_115200);
//	uart_init();

//	uart_init();
	//cli_init();
	//	Uart_Send_String(1, "hello---1\n ");
	printf("\r\n cli_init ok!!!\r\n");
	//nrf_cli_start(&m_cli_uart);
		void spi1_test(void);
		ret_code_t err_code;
		    const nrf_drv_spi_config_t spi_cfg = {
                            .sck_pin      = 2,
                            .mosi_pin     = 7,
                            .miso_pin     = 6,
                            .ss_pin       = 3,
                            .irq_priority = 0,
                            .orc          = 0xFF,
                            .frequency    = (nrf_drv_spi_frequency_t) SPIM_CLK_16MHZ,
                            .mode         = NRF_DRV_SPI_MODE_1,
                            .bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST,
                        };
    err_code = nrf_drv_spi_init(&m_spi, &spi_cfg, spi_handler, NULL);
								
												
					for(int i = 0; i < 320;i++)
					{
					buff1[i] = 0x1 + i;
					}

					for(int i = 0; i < 320;i++)
					{
					if(i & 0x01)
					{
					buff2[i] = buff1[i - 1];
					}else
					{
					buff2[i] = buff1[i + 1];
					}

					}					
												
				nrf_drv_spi_transfer(&m_spi,buff2,32,buff3,32);
	//	spi1_test();
    while(1) {
	//		nrf_cli_process(&m_cli_uart);
       ble_mainloop();
			if(list_handler_sched_flag > 0)
			{
				list_handler_sched_flag--;
				extern_timer_list_handler();
			}
			app_sched_execute();
		//	ff11_test_loop();
	//   ble_system_idle();
       if(LastTimeGulSystickCount!=GulSystickCount)//10ms÷¥––“ª¥Œ
	   {		   
		   LastTimeGulSystickCount=GulSystickCount;
//			 if(LastTimeGulSystickCount % 100 == 0)
//			 {
//					GPIO_OUTPUT_HIGH(5);
//			 }
//			 
//			 if(LastTimeGulSystickCount % 100 == 50)
//			 {
//					GPIO_OUTPUT_LOW(5);
//			 }
			
		 //  key_press();
			// xc620_kbs_scan();
	   }		   


    }
}

#if 1
void sbc_enc_params_print(uint8_t *out_put,uint16_t len)
{
	int i = 0;
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
