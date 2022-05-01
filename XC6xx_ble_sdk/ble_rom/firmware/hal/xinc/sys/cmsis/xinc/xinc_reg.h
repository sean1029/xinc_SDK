#ifndef		__XINC_HW_H
#define		__XINC_HW_H

#include    	"xinc_map.h"

typedef		short 			int16_t;
typedef		int 			int32_t;
typedef		long long 		int64_t;
typedef		unsigned char 		uint8_t;
typedef 	unsigned short 		uint16_t;
typedef 	unsigned int 		uint32_t;
typedef 	unsigned long long 	uint64_t;


#define     __write_hw_reg32(reg,val)  ((*reg) = (val))
#define     __read_hw_reg32(reg, val)  ((val) = (*reg))  
#define     setbit(x,y) ((x) |= (1<<(y)))
#define     clrbit(x,y) ((x) &= ~(1<<(y)))


#endif
