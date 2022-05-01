#ifndef		__RFC_H
#define		__RFC_H

typedef	struct	{
	unsigned    int freq_int;        // 整数部分
	unsigned    int freq_dec;		 // 小数部分  
} rf_freq_t;

extern	rf_freq_t const	rf_rx_freq[40];
extern	rf_freq_t const	rf_tx_freq[40];

void	rf_init(void);


//#define		REG_CTL_0x15c			(0x15c>>2)			
//#define		REG_CTL_0x3c			(0x3c>>2)			
//#define		REG_CTL_0xbfc			(0xbfc>>2)			
//#define     REG_CTL_0x6c            (0x6c>>2)

//#define		REG_CTL_0x3c_TX			(0x7<<7)
//#define		REG_CTL_0x3c_RX			(0xf0<<7)  // open dcoc
//#define		REG_CTL_0xbfc_TX		(0x01)
//#define		REG_CTL_0xbfc_RX		(0x06)
//#define     REG_CTL_0x6c_v0         (0x3<<13)
//#define     REG_CTL_0x6c_v1         (0x7<<13)

#define		REG_CTL_0x15c			(0xB4>>2)			
#define		REG_CTL_0x3c			(0x7c>>2)			
#define		REG_CTL_0xbfc			(0xbfc>>2)			
#define     REG_CTL_0x6c            (0x6c>>2)



#define		REG_CTL_0x3c_TX			(0x3F<<2)
#define		REG_CTL_0x3c_RX			(0xFC<<8)  // open dcoc
#define		REG_CTL_0xbfc_TX		(0x01)
#define		REG_CTL_0xbfc_RX		(0x06)
#define     REG_CTL_0x6c_v0         (0x3<<13)
#define     REG_CTL_0x6c_v1         (0x7<<13)

#endif	
