#include    "Platform.h"
#include    "bsp_register_macro.h"
#include    "ibeacon.h"

ibeacon_t ibeacon = {
    .length = sizeof(ibeacon_t) - 1,
    .type = 0xff,
    .apple_preamble1 = (uint16_t)(APPLE_PREAMLE & 0xFFFF),
    .apple_preamble2 = (uint16_t)((APPLE_PREAMLE>>16) & 0xFFFF),
    .uuid = { 0xFD, 0xA5, 0x06, 0x93, 0xA4, 0xE2, 0x4F, 0xB1, 0xAF, 0xCF, 0xC6, 0xEB, 0x07, 0x64, 0x78, 0x25 },
    .major = 0x6827 ,
    .minor = 0xA19B,
    .tx_power = 0xC5    
};


void    set_beacon(uint8_t *c, uint8_t len) {
    
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





#if 0
{
    char _ibeacon[]="2768,9BA1,C5,AB8190D5D11E4941ACC442F30510B408";
    set_beacon((uint8_t*)_ibeacon,sizeof(_ibeacon));
}

#endif


