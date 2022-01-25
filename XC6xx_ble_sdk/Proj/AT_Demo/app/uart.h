#ifndef		_UART_H_
#define		_UART_H_

#include <stdint.h>

void queue_init(void);

uint8_t queue_empty(void);
uint8_t queue_read(void);
void queue_write(uint8_t value);

#endif
