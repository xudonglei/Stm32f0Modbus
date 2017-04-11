#ifndef _DELAY_H
#define _DELAY_H
#include "stm32f0xx.h"

extern void SysTick_Init(uint8_t SYSCLK);
extern void delay_ms(uint16_t nms);
extern void delay_us(uint16_t nus);

#endif
