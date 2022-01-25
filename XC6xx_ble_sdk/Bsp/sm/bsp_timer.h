#ifndef		__BSP_TIMER_H_
#define		__BSP_TIMER_H_


typedef		short 			int16_t;
typedef		int 			int32_t;
typedef		long long 		int64_t;
typedef		unsigned char 		uint8_t;
typedef 	unsigned short 		uint16_t;
typedef 	unsigned int 		uint32_t;
typedef 	unsigned long long 	uint64_t;

typedef		uint32_t	(*tHandler_callback)(uint16_t  val);


extern	void	Init_Timer(uint32_t ch, uint32_t msTick);			/* TIMER≥ı ºªØ */
extern	void	Timer_Register_Callback(tHandler_callback callback, uint32_t ch);
extern	void	Timer_disable(uint32_t ch);
extern	uint32_t	timer_current_count(uint32_t ch);

#endif

