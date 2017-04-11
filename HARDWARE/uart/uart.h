#ifndef _UART_H
#define _UART_H
#include "stm32f0xx.h"
#include "modbus.h"

extern void Uart_Init(uint32_t bound); 
extern UART_HandleTypeDef UART1_Handler;
#endif
