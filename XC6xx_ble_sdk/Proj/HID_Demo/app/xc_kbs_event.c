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
    kbs_config.rls_intval = RLS;//释放按键间隔
 	  kbs_config.prs_intval = PRS;//按键检测间隔
    kbs_config.dbc_intval = DBC;//debounce 间隔
	  kbs_config.rprs_intval = rprs_val;//重复按键间隔
		kbs_config.lprs_intval = lprs_val;//长按键间隔
}	

/* ---------------------------------------------------------------------------------------------------
- 函数名称: KBS_Handler
- 函数功能: KBS中断处理函数
- 输入参数: 无
- 创建日期: 2021-02-24
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
	uint32_t		reg_kbs_int_en = 0;//中断使能
	
	memset(&kbs_env,0,sizeof(kbs_env));
	kb_ring_buffer_init();
	//开启KBS,GPIO时钟
	__write_hw_reg32(CPR_CTLAPBCLKEN_GRCTL,0x840084);
	__write_hw_reg32(CPR_OTHERCLKEN_GRCTL,0x90009);
	
	//GPIO配置
	kbs_gpio_map();
	keyValueInit();
	kbs_reg_default_config();//默认值
	
	reg_kbs_ctl = KBS_CTL_KEY_EN_MASK | KBS_CTL_KEY_AUTOSCAN_EN_MASK | KBS_CTL_KEY_RPRS_RAW_EN_MASK | \
	    KBS_CTL_KEY_LPRS_RAW_EN_MASK; 
	__write_hw_reg32(KBS_CTL , reg_kbs_ctl);//写KBS_CTL寄存器
	
	
	reg_kbs_mask = 0xffffffff; // 8col and 8row input all  mask 
	reg_kbs_mask &= ~(0x0F << 10); 
	reg_kbs_mask &= ~(0x0F << 0); 
	__write_hw_reg32(KBS_MASK , reg_kbs_mask);//写KBS_MASK寄存器
	//printf("reg_kbs_mask value=0x%08x\n",reg_kbs_mask);
	
	reg_kbs_detect_intval = ((kbs_config.prs_intval << KBS_DETECT_KEY_PRS_INTVAL_SHIFT) & KBS_DETECT_KEY_PRS_INTVAL_MASK)+ \
	      ((kbs_config.rls_intval << KBS_DETECT_KEY_RLS_INTVAL_SHIFT) & KBS_DETECT_KEY_RLS_INTVAL_MASK);
  __write_hw_reg32(KBS_DETECT_INTVAL , reg_kbs_detect_intval);//写KBS_DETECT_INTVAL寄存器
	
	
	reg_kbs_dbc_intval = (kbs_config.dbc_intval << KBS_DBC_KEY_DBC_INTVAL_SHIFT) & KBS_DBC_KEY_DBC_INTVAL_MASK;
	__write_hw_reg32(KBS_DBC_INTVAL,reg_kbs_dbc_intval);//写KBS_DBC_INTVAL寄存器
	
	
	reg_kbs_lprs_intval = ((kbs_config.rprs_intval << KBS_LPRS_KBS_RPRS_INTVAL_SHIFT) & KBS_LPRS_KBS_RPRS_INTVAL_MASK) + \
			  ((kbs_config.lprs_intval << KBS_LPRS_KBS_LPRS_INTVAL_SHIFT) & KBS_LPRS_KBS_LPRS_INTVAL_MASK);
	__write_hw_reg32(KBS_LPRS_INTVAL,reg_kbs_lprs_intval);//写KBS_LPRS_INTVAL寄存器
	
	
	/* 使能中断 */
	reg_kbs_int_en = KBS_INT_PRS_INT_EN_MASK | KBS_INT_FIFO_INT_EN_MASK | KBS_INT_RPRS_INT_EN_MASK | \
			KBS_INT_LPRS_INT_EN_MASK | KBS_INT_RLS_INT_EN_MASK;
	__write_hw_reg32(KBS_MTXKEY_INT_EN,reg_kbs_int_en);//写KBS_MTXKEY_INT_EN寄存器
	
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

/* 键盘矩阵自动扫描模式的键值检测 */
int xc620_kbs_scan(void)
{
	 uint32_t reg_kbs_int = 0;//中断状态
	 uint32_t reg_kbs_int_lprs = 0;//长按
	 uint32_t reg_kbs_int_prs = 0;//按键
	 uint32_t reg_kbs_fifo = 0;//FIFO
	 uint32_t reg_kbs_int_rls = 0;//释放
	 uint32_t reg_kbs_int_rprs = 0;//重复
	 uint32_t reg_kbs_int_en = 0;//中断使能
	 uint8_t interrupt_bits = 0;
	 
	 
	 __read_hw_reg32(KBS_MTXKEY_INT,reg_kbs_int);//读取中断状态寄存器的值到变量reg_kbs_int
	
	 reg_kbs_int_lprs = reg_kbs_int;//长按
	 reg_kbs_int_prs = reg_kbs_int;//按键
	 reg_kbs_int_rls = reg_kbs_int;//释放
	 reg_kbs_int_rprs = reg_kbs_int;//重复
	 reg_kbs_fifo = reg_kbs_int;//FIFO
	 
	
	 kbs_state.prs_int = (reg_kbs_int_prs & KBS_INT_PRS_INT_MASK) >> KBS_INT_PRS_INT_SHIFT;//读取按键中断状态位
	 kbs_state.lprs_int = (reg_kbs_int_lprs & KBS_INT_LPRS_INT_MASK) >> KBS_INT_LPRS_INT_SHIFT;//读取长按键中断状态位
	 kbs_state.rprs_int = (reg_kbs_int_rprs & KBS_INT_RPRS_INT_MASK) >> KBS_INT_RPRS_INT_SHIFT;//读取重复按键中断状态位
	 kbs_state.rls_int = (reg_kbs_int_rls & KBS_INT_RLS_MASK) >> KBS_INT_RLS_SHIFT;//读取释放按键中断状态位
	 kbs_state.fifo_update_int = (reg_kbs_fifo & KBS_INT_FIFO_INT_MASK) >> KBS_INT_FIFO_INT_SHIFT;//读取键值FIFO更新中断状态位
	 
	 
	 //产生按键中断
	 if(kbs_state.prs_int)
	 {
		// printf("kbs_state.prs_int = 0x%x,key press\n",kbs_state.prs_int);
		// hids_device_request_can_send_now_event(0x10);
		 
		 
		 
     kbs_state.press_state = 1;
		 
		 //写1清除按键中断位
		 reg_kbs_int_prs |= KBS_INT_PRS_INT_MASK;
		 __write_hw_reg32(KBS_MTXKEY_INT,reg_kbs_int_prs);
		 
		 //屏蔽按键中断
		 __read_hw_reg32(KBS_MTXKEY_INT_EN,reg_kbs_int_en);
		 //reg_kbs_int_en &= 0x1E;
		 reg_kbs_int_en &= ~(KBS_INT_PRS_INT_EN_MASK);
		 __write_hw_reg32(KBS_MTXKEY_INT_EN,reg_kbs_int_en);
		 
		  
	 }
 
	 
	 //判断键盘矩阵键值 FIFO 是否更新中断
	 if(kbs_state.fifo_update_int)
	 {
		//   printf("kbs_state.fifo_update_int = 0x%x\n",kbs_state.fifo_update_int);
		  
			//清除FIFO中断,写 1 确认中断清除后，需要再对该位写 0		 
			reg_kbs_fifo |= KBS_INT_FIFO_INT_MASK;//FIFO中断位或上1
			__write_hw_reg32(KBS_MTXKEY_INT,reg_kbs_fifo);
		 
		 
			//再对该位写0
			//reg_kbs_fifo &= 0x0000001B;
		  reg_kbs_fifo &= ~(KBS_INT_FIFO_INT_MASK);
			__write_hw_reg32(KBS_MTXKEY_INT,reg_kbs_fifo);
		 
		 
		 
		  //屏蔽FIFO中断使能
		  __read_hw_reg32(KBS_MTXKEY_INT_EN,reg_kbs_int_en);
		  //reg_kbs_int_en &= 0x1D;
		  reg_kbs_int_en &= ~(KBS_INT_FIFO_INT_EN_MASK);
			__write_hw_reg32(KBS_MTXKEY_INT_EN,reg_kbs_int_en);
		 
			//读取键值队列 FIFO 接口寄存器的值到变量reg_kbs_fifo
			__read_hw_reg32(KBS_MTXKEY_FIFO,reg_kbs_fifo);


			//判断FIFO是否为空,不为空则读取
			while(! ((reg_kbs_fifo & KBS_FIFO_EMPTY_MASK) >> KBS_FIFO_EMPTY_SHIFT))
			{
				
				uint8_t col = (reg_kbs_fifo & KBS_FIFO_KEY_COL_MASK) >> KBS_FIFO_KEY_COL_SHIFT;//获取列
				uint8_t row = (reg_kbs_fifo & KBS_FIFO_KEY_ROW_MASK) >> KBS_FIFO_KEY_ROW_SHIFT;//获取行
					
		//		printf("row = %d\n",row);
		//		printf("col = %d\n",col);
					 
				kbs_state.key_value = row*8 + col;//计算键值
				
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
	 
	 
	 //检测重复按键中断
	 //用以检测SHIFT按下的情况下，另外的字母按键有没有释放的操作
	 if(kbs_state.rprs_int)
	 {
		// printf("kbs_state.rprs_int = 0x%x,rep key\n",kbs_state.rprs_int);
		 //向本比特位写‘1’清除本中断状态位
		 reg_kbs_int_rprs |= KBS_INT_RPRS_INT_MASK;
		 __write_hw_reg32(KBS_MTXKEY_INT,reg_kbs_int_rprs);
		 
		 
		 //屏蔽重复按键中断
		 __read_hw_reg32(KBS_MTXKEY_INT_EN,reg_kbs_int_en);
		 //reg_kbs_int_en &= 0x1B;
		 reg_kbs_int_en &= ~(KBS_INT_RPRS_INT_EN_MASK);
		 __write_hw_reg32(KBS_MTXKEY_INT_EN,reg_kbs_int_en);
	 }
	 else //无重复按键中断，说明在组合按键的情况中有按键被释放
	 {
		  static uint8_t count = 0; 
		 
	    if((1 == kbs_state.press_state) && (count++>50))
			{
				  count = 0; 
				  kbs_state.press_state = 0;
			   //压入循环队列
		   //  push_key_event_to_ble(interrupt_bits,0x00);
			}	
	 }	 	
		
		 
	 //检测长按中断
	 if(kbs_state.lprs_int)
	 {
	//	 printf("kbs_state.lprs_int = 0x%x,long key\n",kbs_state.lprs_int);
		 //向本比特位写‘1’清除本中断状态位
		 reg_kbs_int_lprs |= KBS_INT_RLS_MASK;
		 __write_hw_reg32(KBS_MTXKEY_INT,reg_kbs_int_lprs);
		 
			key_event_t key_evt;
			
			key_evt.active = BUTTON_LONG_PUSH_EVT;
			key_evt.keycode = keyValue2code(kbs_state.key_value);
			
			key_evt.keyType = keyCode2Type(key_evt.keycode);
			
			kbs_key_event_push(BUTTON_LONG_PUSH_EVT,key_evt);
					
			
		 //屏蔽长按中断
		 __read_hw_reg32(KBS_MTXKEY_INT_EN,reg_kbs_int_en);
		 //reg_kbs_int_en &= 0x17;
		 reg_kbs_int_en &= ~(KBS_INT_LPRS_INT_EN_MASK);
		 __write_hw_reg32(KBS_MTXKEY_INT_EN,reg_kbs_int_en);
	 }
	  
	 
	 
	 //检测释放按键中断
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

		 
		 //向本比特位写‘1’清除本中断状态位
		 reg_kbs_int_rls |= KBS_INT_RLS_MASK;
		 __write_hw_reg32(KBS_MTXKEY_INT,reg_kbs_int_rls);
		 
		 
		 //屏蔽释放中断
		 __read_hw_reg32(KBS_MTXKEY_INT_EN,reg_kbs_int_en);
		 //reg_kbs_int_en &= 0x0F;
		 reg_kbs_int_en &= ~(KBS_INT_RLS_INT_EN_MASK);
		 __write_hw_reg32(KBS_MTXKEY_INT_EN,reg_kbs_int_en);
	 }

	 
	 
	  //重新使能所有中断
	 reg_kbs_int_en = KBS_INT_PRS_INT_EN_MASK | KBS_INT_FIFO_INT_EN_MASK | \
	 KBS_INT_RPRS_INT_EN_MASK | KBS_INT_LPRS_INT_EN_MASK | KBS_INT_RLS_INT_EN_MASK;
	 __write_hw_reg32(KBS_MTXKEY_INT_EN,reg_kbs_int_en);
	 
}	

void kbs_gpio_map(void)
{
 /* 行 */	
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
	

 /* 列 */
 
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
