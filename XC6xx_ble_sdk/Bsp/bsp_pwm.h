#ifndef _BSP_PWM_H_
#define _BSP_PWM_H_
#include <stdint.h>
void xc_pwm_init(uint8_t ch,uint8_t ocpy_ratio, uint16_t period);//pwm初始化

void xc_set_pwm(uint8_t ch,uint8_t ocpy_ratio, uint16_t period);
#endif
