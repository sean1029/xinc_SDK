#ifndef __XC_KBS_EVENT_H
#define __XC_KBS_EVENT_H

#include    <stdio.h>
#include    <string.h> 
#include		<stdint.h> 
#include "btstack_run_loop.h"

#define KEYBOARD_MAX_COL_SIZE 4
#define KEYBOARD_MAX_ROW_SIZE 4

  
#define RLS           0x02   //释放按键检测间隔 1ms
#define PRS           0x02   //按键检测间隔 1ms 
#define	DBC		        0x05   //DBC间隔 5ms
#define	rprs_val      50  //重复按键间隔为 50ms
#define	lprs_val      40   //长按键间隔40ms long key 2000ms = 50 x 40


typedef enum
{
	BUTTON_PUSH_EVT,
	BUTTON_LONG_PUSH_EVT,
	BUTTON_RELEASE_EVT,
}button_active_t;

typedef enum
{
	UNKOWN_LEY,
	MEDIA_KEY,
	VOICE_KEY,
	KEYBOARD_KEY,
}key_type_t;

typedef struct key_event
{
	button_active_t active;
	key_type_t keyType;
	uint8_t keycode;
}key_event_t;



typedef	struct	{
	  uint8_t prs_intval;//按键检测间隔
    uint8_t rls_intval;//释放键检测间隔
	
	  uint8_t dbc_intval;//debounce 间隔
		
		uint16_t rprs_intval;//重复按键间隔配置
		uint8_t lprs_intval;//长按键间隔配置
}kbs_config_t;


/* 中断状态 */
typedef	struct	{
    uint8_t prs_int;//按键中断
    uint8_t fifo_update_int;//键值 FIFO 更新中断
		uint8_t	rprs_int;//重复按键中断
	  uint8_t lprs_int;//长按键中断
	  uint8_t rls_int;//释放键中断 
	  uint8_t key_value;//1~ 128  (1~64)
	  uint8_t press_state;
}kbs_state_t;


//HID键盘码值
#define KEY_A	0x04
#define KEY_B	0x05
#define KEY_C	0x06
#define KEY_D	0x07
#define KEY_E	0x08
#define KEY_F	0x09
#define KEY_G	0x0A
#define KEY_H	0x0B
#define KEY_I 0x0C
#define KEY_J	0x0D
#define KEY_K	0x0E
#define KEY_L	0x0F
#define KEY_M	0x10
#define KEY_N	0x11
#define KEY_O	0x12
#define KEY_P	0x13
#define KEY_Q	0x14
#define KEY_R	0x15
#define KEY_S	0x16
#define KEY_T	0x17
#define KEY_U	0x18
#define KEY_V	0x19
#define KEY_W	0x1A
#define KEY_X	0x1B
#define KEY_Y	0x1C
#define KEY_Z	0x1D
#define KEY_1	0x1E
#define KEY_2	0x1F
#define KEY_3	0x20
#define KEY_4	0x21
#define KEY_5	0x22
#define KEY_6	0x23
#define KEY_7	0x24
#define KEY_8	0x25
#define KEY_9	0x26
#define KEY_0	0x27
#define KEY_ENTER	0x28	
#define KEY_ESC	0x29	
#define KEY_BACKSPACE	0x2A	
#define KEY_TAB	0x2B	
#define KEY_SPACE	0x2C	
#define KEY_SUB	0x2D	// - and _
#define KEY_EQUAL	0x2E	// = and +
#define KEY_LEFT_BRACKET	0x2F	// [and {
#define KEY_RIGHT_BRACKET 0x30	// ]and }
#define KEY_VERTICAL_LINE	0x31	// "\" and |
#define KEY_WAVE 	0x32	//' and ?
#define KEY_SEMICOLON	0x33	//; and :
#define KEY_QUOTE	0x34	// 'and "
#define KEY_THROW	0x35	//	~and '
#define KEY_COMMA	0x36	//,and<
#define KEY_DOT	0x37	//. and >
#define KEY_QUESTION	0x38	// /and?
#define KEY_CAPS_LOCK	0x39	
#define KEY_F1	0x3A
#define KEY_F2	0x3B
#define KEY_F3	0x3C
#define KEY_F4	0x3D
#define KEY_F5	0x3E
#define KEY_F6	0x3F
#define KEY_F7	0x40
#define KEY_F8	0x41
#define KEY_F9	0x42
#define KEY_F10	0x43
#define KEY_Fll	0x44
#define KEY_F12	0x45
#define KEY_PRT_SCR	0x46
#define KEY_SCOLL_LOCK	0x47
#define KEY_PAUSE	0x48
#define KEY_INS	0x49
#define KEY_HOME	0x4A
#define KEY_PAGEUP	0x4B
#define KEY_DEL	0x4C
#define KEY_END	0x4D
#define KEY_PAGEDOWN	0x4E	
#define KEY_RIGHT_ARROW	0x4F	
#define KEY_LEFT_ARROW	0x50	
#define KEY_DOWN_ARROW	0x51	
#define KEY_UP_ARROW	0x52	
#define KEY_PAD_NUMLOCK	0x53
#define KEY_PAD_DIV	0x54
#define KEY_PAD_MUL	0x55
#define KEY_PAD_SUB	0x56
#define KEY_PAD_ADD	0x57
#define KEY_PAD_ENTER	0x58
#define KEY_PAD_1	0x59
#define KEY_PAD_2	0x5A
#define KEY_PAD_3	0x5B
#define KEY_PAD_4	0x5C
#define KEY_PAD_5	0x5D
#define KEY_PAD_6	0x5E
#define KEY_PAD_7	0x5F
#define KEY_PAD_8	0x60
#define KEY_PAD_9	0x61
#define KEY_PAD_0	0x62
#define KEY_PAD_DOT	0x63
#define KEY_PRESSED	0x00
#define KEY_RELEASED	0x01

//控制
#define KEY_LCTRL	0xE0	//左 Ctrl	
#define KEY_LALT	0xE2	//左 Alt	
#define KEY_LSHFIT	0xE1	//左 Sliift	
#define KEY_LWIN	0xE3	//左 windows	
#define KEY_RWIN	0xE7	//右 windows	
#define KEY_RSHU_1	0xE5	//右 Sliift	
#define KEY_RALT	0xE6	//右 Alt	
#define KEY_RCTRL	0xE4	//右 Ctrl	

#define KEY_VOLUME_SUB 0xEA

#define KEY_VOLUME_ADD 0xE9

typedef struct kbs_env
{
	 // callback to call for enabled callback types
    void  (*process)(struct btstack_data_source *ds, btstack_data_source_callback_type_t callback_type);
}kbs_env_t;


void kb_ring_buffer_reset(void);
int pop_kb_from_ring_buff(key_event_t *key_evt);

void xc620_kbs_init(void);
int xc620_kbs_scan(void);

void kbs_gpio_map(void);

#endif

