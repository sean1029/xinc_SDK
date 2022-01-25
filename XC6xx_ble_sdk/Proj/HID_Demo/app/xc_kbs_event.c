#include    <stdio.h>
#include    <string.h>
#include    "platform.h"
#include	 "Includes.h"
#include "hci.h"


#include   "xc_kbs_event.h"
#include "hids_device_skyworth.h"


#include "btstack_ring_buffer.h"


static kbs_env_t kbs_env;
kbs_state_t kbs_state;
kbs_config_t kbs_config; 

key_event_t key1;
// Buffer for 20 characters
static key_event_t ascii_input_storage[80];
static btstack_ring_buffer_t ascii_input_buffer;

uint8_t gpio_buff_col_ki[KEYBOARD_MAX_COL_SIZE]= {20,21,22,23}; //
uint8_t gpio_buff_row_ko[KEYBOARD_MAX_ROW_SIZE]= {0,1,2,3}; //

uint8_t keyValue[KEYBOARD_MAX_COL_SIZE * KEYBOARD_MAX_ROW_SIZE];

uint8_t keyCode[KEYBOARD_MAX_COL_SIZE * KEYBOARD_MAX_ROW_SIZE] = 
{
	KEY_UP_ARROW,KEY_DOWN_ARROW,KEY_LEFT_ARROW,KEY_RIGHT_ARROW,
	0x41,KEY_ESC,KEY_VOLUME_ADD,KEY_VOLUME_SUB,
	62,130,101,102,
	250

};
void keyValueInit(void)
{
	for(int i = 0;i < KEYBOARD_MAX_ROW_SIZE;i++)
	{
		for(int j = 0;j < KEYBOARD_MAX_COL_SIZE;j++)
		{
			keyValue[j + i * KEYBOARD_MAX_ROW_SIZE] = (gpio_buff_row_ko[i] - 0) * 8 + (gpio_buff_col_ki[j] - 18);
		//	printf("keyValue[%d]:%d\r\n",j + i * KEYBOARD_MAX_ROW_SIZE,keyValue[j + i * KEYBOARD_MAX_ROW_SIZE]);
		//	printf("keyCode[%d]:%d\r\n",j + i * KEYBOARD_MAX_ROW_SIZE,keyCode[j + i * KEYBOARD_MAX_ROW_SIZE]);
		}
	}
}

uint8_t keyValue2code(uint8_t value)
{
	for(int i = 0;i < KEYBOARD_MAX_COL_SIZE * KEYBOARD_MAX_ROW_SIZE;i++)
	{
		if(value == keyValue[i])
		{
			return keyCode[i];
		}
	}
	
	return 0;
}

uint8_t keyCode2Type(uint8_t keyCode)
{
	key_type_t keytype;
	switch(keyCode)
	{
		case 0x41:
		case KEY_VOLUME_ADD:
		case KEY_VOLUME_SUB:			
			keytype = MEDIA_KEY;
		break;

		
		case 130:	
			case 101:
				case 102:
		case KEY_UP_ARROW:
		case KEY_DOWN_ARROW:
		case KEY_LEFT_ARROW:
		case KEY_RIGHT_ARROW:
		case KEY_ESC:
			keytype = KEYBOARD_KEY;
		break;
			
		case 250:
			case 62:
			keytype = VOICE_KEY;
			break;
		
		
		default:
			keytype = UNKOWN_LEY;	
		break;
	}
	
	return keytype;
}

btstack_data_source_t data_source;

void kb_ring_buffer_init(void)
{
   btstack_ring_buffer_init(&ascii_input_buffer, (uint8_t*)&ascii_input_storage, sizeof(ascii_input_storage));
}	

void kb_ring_buffer_reset(void)
{
   kb_ring_buffer_init();
}	


/**
 * Check if kbs ring buffer is empty
 * @param void
 * @return TRUE if empty
 */
int kb_ring_buffer_empty(void)
{
   return btstack_ring_buffer_empty(&ascii_input_buffer);
}	

void kb_process_handler_register(void (*process)(btstack_data_source_t * _data_source, btstack_data_source_callback_type_t callback_type))
{
		kbs_env.process = process;

}
//void key_event_process(btstack_data_source_t *_data_source,  btstack_data_source_callback_type_t callback_type)

int push_kb_to_ring_buff(key_event_t key_evt)
{
  
	return btstack_ring_buffer_write(&ascii_input_buffer, (uint8_t*)&key_evt, sizeof(key_event_t));;
}	




int pop_kb_from_ring_buff(key_event_t *key_evt)
{
	uint32_t num_bytes_read = 0;
	btstack_ring_buffer_read(&ascii_input_buffer, (uint8_t*)key_evt, sizeof(key_event_t), &num_bytes_read);
	return num_bytes_read;
}

void key_event_process(btstack_data_source_t *_data_source,  btstack_data_source_callback_type_t callback_type)
{
	key_event_t keyEvt;
	int emptyBuffer;
	btstack_run_loop_remove_data_source(_data_source);
	
	if(hci_connection_for_handle(0x10))
	{
			hids_device_request_can_send_now_event(0x10);
	}
	else
	{		
		pop_kb_from_ring_buff(&keyEvt);
		emptyBuffer = kb_ring_buffer_empty();
	//	printf("local procee active:%d,keycode:%d,emptyBuffer:%d\n",keyEvt.active,keyEvt.keycode,emptyBuffer);
	}
	
}


void kbs_key_event_push(uint8_t event,key_event_t evt)
{	
//	printf("kbs_key_event_push,event:%d,keycode:%d,keyType:%d\n",event,evt.keycode,evt.keyType);

	push_kb_to_ring_buff(evt);
	btstack_run_loop_set_data_source_handler(&data_source,key_event_process);
	btstack_run_loop_enable_data_source_callbacks(&data_source,DATA_SOURCE_CALLBACK_POLL);
	btstack_run_loop_add_data_source(&data_source);
}

void delay(uint32_t i)
{
	while(i--);
}

static void kbs_reg_default_config(void)
{
    kbs_config.rls_intval = RLS;//�ͷŰ������
 	  kbs_config.prs_intval = PRS;//���������
    kbs_config.dbc_intval = DBC;//debounce ���
	  kbs_config.rprs_intval = rprs_val;//�ظ��������
		kbs_config.lprs_intval = lprs_val;//���������
}	

/* ---------------------------------------------------------------------------------------------------
- ��������: KBS_Handler
- ��������: KBS�жϴ�����
- �������: ��
- ��������: 2021-02-24
----------------------------------------------------------------------------------------------------*/
extern	void KBS_Handler(void)
{
		xc620_kbs_scan();
}

void xc620_kbs_init(void)
{
	uint32_t    reg_kbs_ctl=0;
	uint32_t    reg_kbs_mask=0;
	uint32_t    reg_kbs_detect_intval=0;
	uint32_t    reg_kbs_dbc_intval=0;
	uint32_t    reg_kbs_lprs_intval=0;
	uint32_t    reg_kbs_mtxkey_manual_rowout=0;
	uint32_t		reg_kbs_int_en = 0;//�ж�ʹ��
	
	memset(&kbs_env,0,sizeof(kbs_env));
	kb_ring_buffer_init();
	//����KBS,GPIOʱ��
	__write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL,0x840084);
	__write_hw_reg32(CPR_OTHERCLKEN_GRCTL,0x90009);
	
	//GPIO����
	kbs_gpio_map();
	keyValueInit();
	kbs_reg_default_config();//Ĭ��ֵ
	
	reg_kbs_ctl = KBS_CTL_KEY_EN_MASK | KBS_CTL_KEY_AUTOSCAN_EN_MASK | KBS_CTL_KEY_RPRS_RAW_EN_MASK | \
	    KBS_CTL_KEY_LPRS_RAW_EN_MASK; 
	__write_hw_reg32(KBS_CTL , reg_kbs_ctl);//дKBS_CTL�Ĵ���
	
	
	reg_kbs_mask = 0xffffffff; // 8col and 8row input all  mask 
	reg_kbs_mask &= ~(0x0F << 10); 
	reg_kbs_mask &= ~(0x0F << 0); 
	__write_hw_reg32(KBS_MASK , reg_kbs_mask);//дKBS_MASK�Ĵ���
	//printf("reg_kbs_mask value=0x%08x\n",reg_kbs_mask);
	
	reg_kbs_detect_intval = ((kbs_config.prs_intval << KBS_DETECT_KEY_PRS_INTVAL_SHIFT) & KBS_DETECT_KEY_PRS_INTVAL_MASK)+ \
	      ((kbs_config.rls_intval << KBS_DETECT_KEY_RLS_INTVAL_SHIFT) & KBS_DETECT_KEY_RLS_INTVAL_MASK);
  __write_hw_reg32(KBS_DETECT_INTVAL , reg_kbs_detect_intval);//дKBS_DETECT_INTVAL�Ĵ���
	
	
	reg_kbs_dbc_intval = (kbs_config.dbc_intval << KBS_DBC_KEY_DBC_INTVAL_SHIFT) & KBS_DBC_KEY_DBC_INTVAL_MASK;
	__write_hw_reg32(KBS_DBC_INTVAL,reg_kbs_dbc_intval);//дKBS_DBC_INTVAL�Ĵ���
	
	
	reg_kbs_lprs_intval = ((kbs_config.rprs_intval << KBS_LPRS_KBS_RPRS_INTVAL_SHIFT) & KBS_LPRS_KBS_RPRS_INTVAL_MASK) + \
			  ((kbs_config.lprs_intval << KBS_LPRS_KBS_LPRS_INTVAL_SHIFT) & KBS_LPRS_KBS_LPRS_INTVAL_MASK);
	__write_hw_reg32(KBS_LPRS_INTVAL,reg_kbs_lprs_intval);//дKBS_LPRS_INTVAL�Ĵ���
	
	
	/* ʹ���ж� */
	reg_kbs_int_en = KBS_INT_PRS_INT_EN_MASK | KBS_INT_FIFO_INT_EN_MASK | KBS_INT_RPRS_INT_EN_MASK | \
			KBS_INT_LPRS_INT_EN_MASK | KBS_INT_RLS_INT_EN_MASK;
	__write_hw_reg32(KBS_MTXKEY_INT_EN,reg_kbs_int_en);//дKBS_MTXKEY_INT_EN�Ĵ���
	
	NVIC_EnableIRQ(KBS_IRQn);
	
	
	/***********read test******************/
//	reg_kbs_ctl = 0;
//	__read_hw_reg32(KBS_CTL, reg_kbs_ctl);
//	printf("TEST READ KBS_CTL value=%x\n",reg_kbs_ctl);
//	
//	__read_hw_reg32(KBS_MASK, reg_kbs_ctl);
//	printf("KBS_MASK value=%x\n",reg_kbs_ctl);
//	
//	__read_hw_reg32(KBS_DETECT_INTVAL, reg_kbs_ctl);
//	printf("KBS_DETECT_INTVAL value=%x\n",reg_kbs_ctl);
//	
//	__read_hw_reg32(KBS_DBC_INTVAL, reg_kbs_ctl);
//	printf("KBS_DBC_INTVAL value=%x\n",reg_kbs_ctl);
//	
//	__read_hw_reg32(KBS_LPRS_INTVAL, reg_kbs_ctl);
//	printf("KBS_LPRS_INTVAL value=%x\n",reg_kbs_ctl);
//	
//	__read_hw_reg32(KBS_MTXKEY_INT_EN, reg_kbs_ctl);
//	printf("KBS_MTXKEY_INT_EN value=%x\n",reg_kbs_ctl);
	/**************************************/
}	
extern int key_value ;

/* ���̾����Զ�ɨ��ģʽ�ļ�ֵ��� */
int xc620_kbs_scan(void)
{
	 uint32_t reg_kbs_int = 0;//�ж�״̬
	 uint32_t reg_kbs_int_lprs = 0;//����
	 uint32_t reg_kbs_int_prs = 0;//����
	 uint32_t reg_kbs_fifo = 0;//FIFO
	 uint32_t reg_kbs_int_rls = 0;//�ͷ�
	 uint32_t reg_kbs_int_rprs = 0;//�ظ�
	 uint32_t reg_kbs_int_en = 0;//�ж�ʹ��
	 uint8_t interrupt_bits = 0;
	 
	 
	 __read_hw_reg32(KBS_MTXKEY_INT,reg_kbs_int);//��ȡ�ж�״̬�Ĵ�����ֵ������reg_kbs_int
	
	 reg_kbs_int_lprs = reg_kbs_int;//����
	 reg_kbs_int_prs = reg_kbs_int;//����
	 reg_kbs_int_rls = reg_kbs_int;//�ͷ�
	 reg_kbs_int_rprs = reg_kbs_int;//�ظ�
	 reg_kbs_fifo = reg_kbs_int;//FIFO
	 
	
	 kbs_state.prs_int = (reg_kbs_int_prs & KBS_INT_PRS_INT_MASK) >> KBS_INT_PRS_INT_SHIFT;//��ȡ�����ж�״̬λ
	 kbs_state.lprs_int = (reg_kbs_int_lprs & KBS_INT_LPRS_INT_MASK) >> KBS_INT_LPRS_INT_SHIFT;//��ȡ�������ж�״̬λ
	 kbs_state.rprs_int = (reg_kbs_int_rprs & KBS_INT_RPRS_INT_MASK) >> KBS_INT_RPRS_INT_SHIFT;//��ȡ�ظ������ж�״̬λ
	 kbs_state.rls_int = (reg_kbs_int_rls & KBS_INT_RLS_MASK) >> KBS_INT_RLS_SHIFT;//��ȡ�ͷŰ����ж�״̬λ
	 kbs_state.fifo_update_int = (reg_kbs_fifo & KBS_INT_FIFO_INT_MASK) >> KBS_INT_FIFO_INT_SHIFT;//��ȡ��ֵFIFO�����ж�״̬λ
	 
	 
	 //���������ж�
	 if(kbs_state.prs_int)
	 {
		// printf("kbs_state.prs_int = 0x%x,key press\n",kbs_state.prs_int);
		// hids_device_request_can_send_now_event(0x10);
		 
		 
		 
     kbs_state.press_state = 1;
		 
		 //д1��������ж�λ
		 reg_kbs_int_prs |= KBS_INT_PRS_INT_MASK;
		 __write_hw_reg32(KBS_MTXKEY_INT,reg_kbs_int_prs);
		 
		 //���ΰ����ж�
		 __read_hw_reg32(KBS_MTXKEY_INT_EN,reg_kbs_int_en);
		 //reg_kbs_int_en &= 0x1E;
		 reg_kbs_int_en &= ~(KBS_INT_PRS_INT_EN_MASK);
		 __write_hw_reg32(KBS_MTXKEY_INT_EN,reg_kbs_int_en);
		 
		  
	 }
 
	 
	 //�жϼ��̾����ֵ FIFO �Ƿ�����ж�
	 if(kbs_state.fifo_update_int)
	 {
		//   printf("kbs_state.fifo_update_int = 0x%x\n",kbs_state.fifo_update_int);
		  
			//���FIFO�ж�,д 1 ȷ���ж��������Ҫ�ٶԸ�λд 0		 
			reg_kbs_fifo |= KBS_INT_FIFO_INT_MASK;//FIFO�ж�λ����1
			__write_hw_reg32(KBS_MTXKEY_INT,reg_kbs_fifo);
		 
		 
			//�ٶԸ�λд0
			//reg_kbs_fifo &= 0x0000001B;
		  reg_kbs_fifo &= ~(KBS_INT_FIFO_INT_MASK);
			__write_hw_reg32(KBS_MTXKEY_INT,reg_kbs_fifo);
		 
		 
		 
		  //����FIFO�ж�ʹ��
		  __read_hw_reg32(KBS_MTXKEY_INT_EN,reg_kbs_int_en);
		  //reg_kbs_int_en &= 0x1D;
		  reg_kbs_int_en &= ~(KBS_INT_FIFO_INT_EN_MASK);
			__write_hw_reg32(KBS_MTXKEY_INT_EN,reg_kbs_int_en);
		 
			//��ȡ��ֵ���� FIFO �ӿڼĴ�����ֵ������reg_kbs_fifo
			__read_hw_reg32(KBS_MTXKEY_FIFO,reg_kbs_fifo);


			//�ж�FIFO�Ƿ�Ϊ��,��Ϊ�����ȡ
			while(! ((reg_kbs_fifo & KBS_FIFO_EMPTY_MASK) >> KBS_FIFO_EMPTY_SHIFT))
			{
				
				uint8_t col = (reg_kbs_fifo & KBS_FIFO_KEY_COL_MASK) >> KBS_FIFO_KEY_COL_SHIFT;//��ȡ��
				uint8_t row = (reg_kbs_fifo & KBS_FIFO_KEY_ROW_MASK) >> KBS_FIFO_KEY_ROW_SHIFT;//��ȡ��
					
		//		printf("row = %d\n",row);
		//		printf("col = %d\n",col);
					 
				kbs_state.key_value = row*8 + col;//�����ֵ
				
				key_event_t key_evt;
				
				key_evt.active = BUTTON_PUSH_EVT;
				key_evt.keycode = keyValue2code(kbs_state.key_value);
				
				key_evt.keyType = keyCode2Type(key_evt.keycode);
				
				kbs_key_event_push(BUTTON_PUSH_EVT,key_evt);
				
				interrupt_bits = reg_kbs_int & 0xff;	
				
		//		printf("key_value = %d\n",kbs_state.key_value);
				__read_hw_reg32(KBS_MTXKEY_FIFO,reg_kbs_fifo);
				
			} 
	 }
	 
	 
	 //����ظ������ж�
	 //���Լ��SHIFT���µ�����£��������ĸ������û���ͷŵĲ���
	 if(kbs_state.rprs_int)
	 {
		// printf("kbs_state.rprs_int = 0x%x,rep key\n",kbs_state.rprs_int);
		 //�򱾱���λд��1��������ж�״̬λ
		 reg_kbs_int_rprs |= KBS_INT_RPRS_INT_MASK;
		 __write_hw_reg32(KBS_MTXKEY_INT,reg_kbs_int_rprs);
		 
		 
		 //�����ظ������ж�
		 __read_hw_reg32(KBS_MTXKEY_INT_EN,reg_kbs_int_en);
		 //reg_kbs_int_en &= 0x1B;
		 reg_kbs_int_en &= ~(KBS_INT_RPRS_INT_EN_MASK);
		 __write_hw_reg32(KBS_MTXKEY_INT_EN,reg_kbs_int_en);
	 }
	 else //���ظ������жϣ�˵������ϰ�����������а������ͷ�
	 {
		  static uint8_t count = 0; 
		 
	    if((1 == kbs_state.press_state) && (count++>50))
			{
				  count = 0; 
				  kbs_state.press_state = 0;
			   //ѹ��ѭ������
		   //  push_key_event_to_ble(interrupt_bits,0x00);
			}	
	 }	 	
		
		 
	 //��ⳤ���ж�
	 if(kbs_state.lprs_int)
	 {
	//	 printf("kbs_state.lprs_int = 0x%x,long key\n",kbs_state.lprs_int);
		 //�򱾱���λд��1��������ж�״̬λ
		 reg_kbs_int_lprs |= KBS_INT_RLS_MASK;
		 __write_hw_reg32(KBS_MTXKEY_INT,reg_kbs_int_lprs);
		 
			key_event_t key_evt;
			
			key_evt.active = BUTTON_LONG_PUSH_EVT;
			key_evt.keycode = keyValue2code(kbs_state.key_value);
			
			key_evt.keyType = keyCode2Type(key_evt.keycode);
			
			kbs_key_event_push(BUTTON_LONG_PUSH_EVT,key_evt);
					
			
		 //���γ����ж�
		 __read_hw_reg32(KBS_MTXKEY_INT_EN,reg_kbs_int_en);
		 //reg_kbs_int_en &= 0x17;
		 reg_kbs_int_en &= ~(KBS_INT_LPRS_INT_EN_MASK);
		 __write_hw_reg32(KBS_MTXKEY_INT_EN,reg_kbs_int_en);
	 }
	  
	 
	 
	 //����ͷŰ����ж�
	 if( kbs_state.rls_int )
	 {
		 
	//	 printf("kbs_state.rls_int = 0x%x,release key\n",kbs_state.rls_int);
			key_event_t key_evt;

			key_evt.active = BUTTON_RELEASE_EVT;
			key_evt.keycode = keyValue2code(kbs_state.key_value);

			key_evt.keyType = keyCode2Type(key_evt.keycode);

			kbs_key_event_push(BUTTON_RELEASE_EVT,key_evt);
		 kbs_state.press_state = 0;
		 interrupt_bits = reg_kbs_int & 0xff;

		 
		 //�򱾱���λд��1��������ж�״̬λ
		 reg_kbs_int_rls |= KBS_INT_RLS_MASK;
		 __write_hw_reg32(KBS_MTXKEY_INT,reg_kbs_int_rls);
		 
		 
		 //�����ͷ��ж�
		 __read_hw_reg32(KBS_MTXKEY_INT_EN,reg_kbs_int_en);
		 //reg_kbs_int_en &= 0x0F;
		 reg_kbs_int_en &= ~(KBS_INT_RLS_INT_EN_MASK);
		 __write_hw_reg32(KBS_MTXKEY_INT_EN,reg_kbs_int_en);
	 }

	 
	 
	  //����ʹ�������ж�
	 reg_kbs_int_en = KBS_INT_PRS_INT_EN_MASK | KBS_INT_FIFO_INT_EN_MASK | \
	 KBS_INT_RPRS_INT_EN_MASK | KBS_INT_LPRS_INT_EN_MASK | KBS_INT_RLS_INT_EN_MASK;
	 __write_hw_reg32(KBS_MTXKEY_INT_EN,reg_kbs_int_en);
	 
}	

void kbs_gpio_map(void)
{
 /* �� */	
	#if 0
 gpio_mux_ctl(0,1);   //map ->kbs_mko[0]
 gpio_mux_ctl(1,1);   //map ->kbs_mko[1]
 gpio_mux_ctl(2,1);   //map ->kbs_mko[2]
 gpio_mux_ctl(3,1);   //map ->kbs_mko[3]
 gpio_mux_ctl(4,1);   //map ->kbs_mko[4]
 gpio_mux_ctl(5,1);   //map ->kbs_mko[5]
 gpio_mux_ctl(6,1);   //map ->kbs_mko[6]
 gpio_mux_ctl(7,1);   //map ->kbs_mko[7]
 gpio_direction_input(0,1);
	gpio_direction_input(1,1);
	gpio_direction_input(2,1);
	gpio_direction_input(3,1);
	gpio_direction_input(4,1);
	gpio_direction_input(5,1);
	gpio_direction_input(6,1);
	gpio_direction_input(7,1);
	

 /* �� */
 
// gpio_mux_ctl(18,1);  //map ->kbs_mki[0]
// gpio_mux_ctl(19,1);  //map ->kbs_mki[1]
 gpio_mux_ctl(20,1);  //map ->kbs_mki[2]
 gpio_mux_ctl(21,1);  //map ->kbs_mki[3]
 gpio_mux_ctl(22,1);  //map ->kbs_mki[4]
 gpio_mux_ctl(23,1);  //map ->kbs_mki[5]
 gpio_mux_ctl(24,1);  //map ->kbs_mki[6]
 gpio_mux_ctl(25,1);  //map ->kbs_mki[7]
//  gpio_direction_input(18,1);
//	gpio_direction_input(19,1);
  gpio_direction_input(20,1);
	gpio_direction_input(21,1);
	gpio_direction_input(22,1);
	gpio_direction_input(23,1);
	gpio_direction_input(24,1);
	gpio_direction_input(25,1);
	
	
	//gpio_mux_ctl(24,0);
	//gpio_direction_output(24);
	//gpio_output_low(24);
	
	#endif //
	
	for(int i = 0;i < KEYBOARD_MAX_COL_SIZE;i++)
	{
		gpio_mux_ctl(gpio_buff_col_ki[i],1);
		gpio_direction_input(gpio_buff_col_ki[i],1);
	}
	
	
	for(int i = 0;i < KEYBOARD_MAX_ROW_SIZE;i++)
	{
		gpio_mux_ctl(gpio_buff_row_ko[i],1);
		gpio_direction_output(gpio_buff_row_ko[i]);
	}
	
//	gpio_mux_ctl(20,1);   //map ->kbs_mki[2]
//	gpio_direction_input(20,1);
//	gpio_mux_ctl(21,1);   //map ->kbs_mki[3]
//	gpio_direction_input(21,1);

//	
//	
//	
//	gpio_mux_ctl(0,1);   //map ->kbs_mko[0]
////	gpio_direction_input(0,1);
//	gpio_direction_output(0);
//	gpio_mux_ctl(1,1);   //map ->kbs_mko[1]
////	gpio_direction_input(1,1);
//	gpio_direction_output(1);
	
} 
