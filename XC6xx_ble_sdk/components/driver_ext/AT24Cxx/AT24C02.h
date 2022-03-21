/**
 * Copyright (c) 2022 - 2025, XinChip
 *
 * All rights reserved.
 *
 * Author :sean cheng
 *
 */
#ifndef 	__AT24C02_H
#define 	__AT24C02_H


//#define  CHIP_AT24C01 1
#define  CHIP_AT24C02 2
//#define  CHIP_AT24C04 4
//#define  CHIP_AT24C08 8
//#define  CHIP_AT24C16 16

#if (CHIP_AT24C01)
#define AT24Cxx_PAGESIZE        0x8
#define AT24Cxx_ENDADDR         0x7F
#define AT24Cxx_PAGENUM         16
#elif (CHIP_AT24C02)
#define AT24Cxx_PAGESIZE        0x8
#define AT24Cxx_ENDADDR         0xFF
#define AT24Cxx_PAGENUM         32
#elif (CHIP_AT24C04)
#define AT24Cxx_PAGESIZE        0x10
#define AT24Cxx_ENDADDR         0x1FF
#define AT24Cxx_PAGENUM         32
#elif (CHIP_AT24C08)
#define AT24Cxx_PAGESIZE        0x10
#define AT24Cxx_ENDADDR         0x3FF
#define AT24Cxx_PAGENUM         64
#elif (CHIP_AT24C16)
#define AT24Cxx_PAGESIZE        0x10
#define AT24Cxx_ENDADDR         0x7FF
#define AT24Cxx_PAGENUM         128
#endif



void i2c_at24c02_init(void);
void  i2c_at24c02_test(void);
ret_code_t AT24Cxx_write_byte(uint16_t WriteAddr,uint8_t dat);

ret_code_t AT24Cxx_write_page(uint8_t page,uint8_t const *pdata,uint8_t size);

ret_code_t AT24Cxx_write_buf(uint16_t WriteAddr,uint8_t  *p_buf,uint16_t size);

ret_code_t AT24Cxx_read_buf(uint16_t ReadAddr,uint8_t  *p_buf,uint16_t size);





#endif //

