#include    "Platform.h"
#include    "bsp_register_macro.h"
//#define     __DEBUG_AT
#ifdef      __DEBUG_AT
#define     dbg_at      printf
#else
#define     dbg_at(...) 
#endif  
//16bits数据高八位低八位交换位置
#define SWAP_ENDIAN16(Value) ((((Value) & 0x00FF) << 8) | \
                              (((Value) & 0xFF00) >> 8)) 
typedef struct {
    uint8_t	index;
    uint8_t	buff[AT_BUFF_LEN];
}at_t;

at_t  at = {
	.index = 0,
};

typedef void (*at_operation)(uint8_t *c, uint8_t len);

typedef	struct {
    void            *string;
    uint32_t        string_len;
		uint32_t        hash;
		at_operation		op;
}atcmd_t;								

void    at_sldn(uint8_t *c, uint8_t len);
void    at_gldn(uint8_t *c, uint8_t len);
void    at_glbn(uint8_t *c, uint8_t len);
void    at_gbeacon(uint8_t *c, uint8_t len);
void    at_beacon(uint8_t *c, uint8_t len);
void    at_version(uint8_t *c, uint8_t len);
void    at_reset(uint8_t *c, uint8_t len);
void    at_error(uint8_t *c, uint8_t len);
/*
蓝牙AT命令：
1.获取蓝牙mac地址
命令：AT+B GLBD\r  响应：AT-B GLBD 0,58696E637631  
2.获取蓝牙版本信息
命令：AT+B GVER\r  响应：AT-B GVER XINC_BLE_v1.0 Apr  2 2019 16:05:42
*/
atcmd_t	at_table[] = {
	[0] = { "AT+B SLDN" ,   sizeof("AT+B SLDN"),     0,  (at_operation)at_sldn     },
	[1] = { "AT+B GLDN" ,   sizeof("AT+B GLDN"),     0,  (at_operation)at_gldn     },
	[2] = { "AT+B GLBD" ,   sizeof("AT+B GLBD"),     0,  (at_operation)at_glbn     },
	[3] = { "AT+B GBEACON", sizeof("AT+B GBEACON"),  0,  (at_operation)at_gbeacon, },
	[4] = { "AT+B BEACON",  sizeof("AT+B BEACON" ),  0,  (at_operation)at_beacon,  },
	[5] = { "AT+B GVER",    sizeof("AT+B GVER" ),    0,  (at_operation)at_version, },
	[6] = { "AT+B RESET",   sizeof("AT+B RESET" ),   0,  (at_operation)at_reset,   }
    
};

static	uint32_t bkdrhash(uint8_t *str, uint8_t	len)  
{  
    uint8_t	 ch;
    uint32_t hash = 0;  
    
    for(uint8_t i=0; i<len; i++) {
        ch = *str++;
        hash = hash * 131 + (uint32_t)ch; 
    }
    return hash; 
}

void at_init(void)  
{
    for(uint32_t i=0; i<sizeof(at_table)/sizeof(at_table[0]); i++)
    {
        at_table[i].hash = bkdrhash(at_table[i].string, at_table[i].string_len - 1);
        dbg_at("[%d].hash = 0x%04x\n", i, at_table[i].hash);
    }
}

void  at_done(uint8_t ch)
{
    if(ch == '\n') return;
    
    at.buff[at.index++] = ch;			
    if(ch != '\r') return;
      
    for(uint8_t i=0; i<sizeof(at_table)/sizeof(at_table[0]); i++) {
        
        uint8_t len = at_table[i].string_len;    
        uint32_t hash = bkdrhash(at.buff, len - 1);
        
        if(hash == at_table[i].hash) {          
            at_table[i].op(&at.buff[len-1], (at.index - (len - 1)));
            break;
        }
        if(i == (sizeof(at_table)/sizeof(at_table[0]) - 1)) at_error(0, 0);            
    }
    at.index = 0;
}

typedef struct {
    void    *name;
    void    *date;
    void    *time;   
}firmware_version_t;

firmware_version_t  version = {
    .name = "XINC_BLE_v1.0 ",
    .date = __DATE__,
    .time = __TIME__   
};

uint8_t dev_name[20] = "Xinc_ble";

void    at_sldn(uint8_t *c, uint8_t len) {
    
    dbg_at("%s \n", __func__);
    c[len] = '\0';
    dbg_at("%s \n", c);
    
    uint8_t i = 0, j = 0; 
  
    do  {
        if(c[i] != ' ') break;
        i++;
    } while(1);
    
    for(; i<len; i++) {
        if(c[i] == '\r') break;
        dev_name[j++] = c[i];
        if(j >= sizeof(dev_name) - 1) break;
    }
    dev_name[j] = '\0';  
    Uart_Send_String(AT_USE_UART, (uint8_t *)"AT-B SLDN 0\r");
}

void    at_gldn(uint8_t *c, uint8_t len){
    dbg_at("%s\n", __func__);
    
    Uart_Send_String(AT_USE_UART, (uint8_t *)"AT-B GLDN ");
    Uart_Send_String(AT_USE_UART, dev_name);
    Uart_Send_String(AT_USE_UART, (uint8_t *)"\r");
}

uint8_t const hex[] = {'0' , '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

void    at_glbn(uint8_t *c, uint8_t len){
    dbg_at("%s\n", __func__);
    //extern  uint8_t		ble_mac_data[6];
    extern unsigned char bd_addr[6];
    for(uint8_t i=0,j=5; i<6; j--,i++) {
        c[i*2] = hex[(bd_addr[j] >> 4)];
        c[i*2+1] = hex[(bd_addr[j]&0x0F)];
    }
    c[12] = '\0';
    
    Uart_Send_String(AT_USE_UART, (uint8_t *)"AT-B GLBD 0,");   
    Uart_Send_String(AT_USE_UART, (uint8_t *)c);
    Uart_Send_String(AT_USE_UART, (uint8_t *)"\r");          
}

typedef struct {
    uint8_t     length;
    uint8_t     type;
    uint16_t    apple_preamble1;
    uint16_t    apple_preamble2;
    uint8_t     uuid[16];
    uint16_t    major;
    uint16_t    minor;
    uint8_t     tx_power;    
}__attribute__((packed, aligned(1))) ibeacon_t;


#define     APPLE_PREAMLE   0x1502004C

ibeacon_t ibeacon = {
    .length = sizeof(ibeacon_t) - 1,
    .type = 0xff,
    .apple_preamble1 = (uint16_t)(APPLE_PREAMLE & 0xFFFF),
    .apple_preamble2 = (uint16_t)((APPLE_PREAMLE>>16) & 0xFFFF),
    .uuid = { 0xFD, 0xA5, 0x06, 0x93, 0xA4, 0xE2, 0x4F, 0xB1, 0xAF, 0xCF, 0xC6, 0xEB, 0x07, 0x64, 0x78, 0x25 },
    .major = 0x2744,
    .minor = 0x8BE9,
    .tx_power = 0xC5    
};

void    at_gbeacon(uint8_t *c, uint8_t len){
    dbg_at("%s\n", __func__);
    
    Uart_Send_String(AT_USE_UART, (uint8_t *)"AT-B GBEACON "); 
    c[0] = hex[(ibeacon.major>>12)&0x0F]; c[1] = hex[(ibeacon.major>>8)&0x0F]; 
    c[2] = hex[(ibeacon.major>>4)&0x0F]; c[3] = hex[(ibeacon.major>>0)&0x0F];
    c[4] = ','; c[5] = '\0';   
    Uart_Send_String(AT_USE_UART, (uint8_t *)c); 
    c[0] = hex[(ibeacon.minor>>12)&0x0F]; c[1] = hex[(ibeacon.minor>>8)&0x0F]; 
    c[2] = hex[(ibeacon.minor>>4)&0x0F]; c[3] = hex[(ibeacon.minor>>0)&0x0F];
    c[4] = ','; c[5] = '\0';   
    Uart_Send_String(AT_USE_UART, (uint8_t *)c);
    c[0] = hex[(ibeacon.tx_power>>4)&0x0F]; c[1] = hex[(ibeacon.tx_power>>0)&0x0F];
    c[2] = ','; c[3] = '\0';
    Uart_Send_String(AT_USE_UART, (uint8_t *)c);    
    for(uint8_t i=0; i<16; i++) {
        c[i*2] = hex[(ibeacon.uuid[i] >> 4)];
        c[i*2+1] = hex[(ibeacon.uuid[i]&0x0F)];
    }
    c[32] = '\0';
    Uart_Send_String(AT_USE_UART, (uint8_t *)c);
    Uart_Send_String(AT_USE_UART, (uint8_t *)"\r");    
}
//AT+B BEACON 2768,9BA1,C5,AB8190D5D11E4941ACC442F30510B408
void    at_beacon(uint8_t *c, uint8_t len) {
    
    dbg_at("%s, len =%d\n", __func__, len);
    
    uint8_t res[3] = {'0', '\r', '\0'};  
    uint8_t i = 0, j = 0; 
    do  {
        if(c[i] != ' ') break;
        i++;
    } while(1);
    
    ibeacon.major = 0;
    for(j=0; j<4; j++) {
        if(c[i] == ',') break;
        if(res[0] == '1') break;
        
        if((c[i] >= '0') && (c[i] <= '9')) c[i] -= '0';
        else if((c[i] >= 'A') && (c[i] <= 'F')) c[i] = c[i] - 'A' + 10; 
        else if((c[i] >= 'a') && (c[i] <= 'f')) c[i] = c[i] - 'a' + 10;
        else res[0] = '1';
        ibeacon.major <<= 4;
        ibeacon.major |= c[i];
        i++;
    }
    
    do 	{
        if(c[i] != ',') break;
        if(res[0] == '1') break;        
        i++;
    } while(1);
    
    ibeacon.minor = 0;
    for(j=0; j<4; j++) {
        if(c[i] == ',') break;
        if(res[0] == '1') break;        
        if((c[i] >= '0') && (c[i] <= '9')) c[i] -= '0';
        else if((c[i] >= 'A') && (c[i] <= 'F')) c[i] = c[i] - 'A' + 10; 
        else if((c[i] >= 'a') && (c[i] <= 'f')) c[i] = c[i] - 'a' + 10;
        else res[0] = '1';        
        ibeacon.minor <<= 4;
        ibeacon.minor |= c[i];
        i++;
    }    
    do  {
        if(c[i] != ',') break;
        if(res[0] == '1') break;        
        i++;
    } while(1);
    
    ibeacon.tx_power = 0;
    for(j=0; j<2; j++) {
        if(c[i] == ',') break;
        if(res[0] == '1') break;
        if((c[i] >= '0') && (c[i] <= '9')) c[i] -= '0';
        else if((c[i] >= 'A') && (c[i] <= 'F')) c[i] = c[i] - 'A' + 10;
        else if((c[i] >= 'a') && (c[i] <= 'f')) c[i] = c[i] - 'a' + 10;
        else res[0] = '1';        
        ibeacon.tx_power <<= 4;
        ibeacon.tx_power |= c[i];
        i++;
    }      

    do  {
        if(c[i] != ',') break;
        if(res[0] == '1') break;        
        i++;
    } while(1);
    
    if((i+ 32) != (len - 1)) res[0] = '1';
    
    for(j=0; j<32; j++) {
        if(res[0] == '1') break;        
        if((c[i] >= '0') && (c[i] <= '9')) c[i] -= '0';
        else if((c[i] >= 'A') && (c[i] <= 'F')) c[i] = c[i] - 'A' + 10; 
        else if((c[i] >= 'a') && (c[i] <= 'f')) c[i] = c[i] - 'a' + 10;
        else res[0] = '1';        
        ibeacon.uuid[j/2] <<= 4;
        ibeacon.uuid[j/2] |= c[i];
        i++;
    }  
    Uart_Send_String(AT_USE_UART, (uint8_t *)"AT-B BEACON ");
    Uart_Send_String(AT_USE_UART, (uint8_t *)res);
    
    extern  uint32_t    ble_set_advertising_data(uint8_t length, uint8_t *s);      
    if(res[0] == '0') {
			  //将ibeacon.minor ibeacon.major 高八位和低八位进行交换
		    ibeacon.minor=SWAP_ENDIAN16(ibeacon.minor);//
		    ibeacon.major=SWAP_ENDIAN16(ibeacon.major);//16bits高八位低八位交换位置，解决大小端数据存储问题
        uint8_t *s = (uint8_t *)&ibeacon;
        c[0] = 0x02; c[1] = 0x01; c[2] = 0x06;
        for(uint8_t i=0; i<sizeof(ibeacon_t); i++) {
            c[3+i] = *s++;
        }
		extern uint32_t LEadv_Set_Advertising_Data(uint8_t length,uint8_t* p_pdu);
        LEadv_Set_Advertising_Data(sizeof(ibeacon_t)+3, c);
				//将ibeacon.minor ibeacon.major 还原
				ibeacon.minor=SWAP_ENDIAN16(ibeacon.minor);//
		    ibeacon.major=SWAP_ENDIAN16(ibeacon.major);//16bits高八位低八位交换位置
    } 
}
void    at_version(uint8_t *c, uint8_t len)
{
    dbg_at("%s\n", __func__);
    Uart_Send_String(AT_USE_UART, (uint8_t *)"AT-B GVER ");     
    Uart_Send_String(AT_USE_UART, (uint8_t *)version.name);
    Uart_Send_String(AT_USE_UART, (uint8_t *)version.date);
    Uart_Send_String(AT_USE_UART, (uint8_t *)" ");      
    Uart_Send_String(AT_USE_UART, (uint8_t *)version.time);    
    Uart_Send_String(AT_USE_UART, (uint8_t *)"\r");    
}

void    at_reset(uint8_t *c, uint8_t len)
{
    dbg_at("%s\n", __func__);
    Uart_Send_String(AT_USE_UART, (uint8_t *)"AT-B RESET 0\r");   
    for(uint16_t i=0; i<2000; i++);

    __write_hw_reg32(CPR_RSTCTL_SUBRST_SW , 0x30000);					
    __write_hw_reg32(CPR_RSTCTL_SUBRST_SW , 0x30003);	
    __write_hw_reg32(CPR_RSTCTL_M0RST_MASK, 0x17);
    NVIC_SystemReset();  
}
void    at_error(uint8_t *c, uint8_t len)
{
    dbg_at("%s\n", __func__);    
    Uart_Send_String(AT_USE_UART, (uint8_t *)"AT-B ERROR\r");      
}

