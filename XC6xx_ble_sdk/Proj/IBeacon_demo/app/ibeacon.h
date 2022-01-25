#ifndef _IBEACON_H_
#define _IBEACON_H_

#define SWAP_ENDIAN16(Value) ((((Value) & 0x00FF) << 8) | \
                              (((Value) & 0xFF00) >> 8)) 

#define     APPLE_PREAMLE   0x1502004C
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

extern ibeacon_t ibeacon;
#endif