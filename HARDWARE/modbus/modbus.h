#ifndef _MODBUS_H
#define _MODBUS_H
#include "stm32f0xx.h"
#include "uart.h"

#define  ID       0x01           //slave ID
#define MAXRECLEN 255
typedef struct
{
	 FlagStatus RecEndFlag;      //frame receive end
	 uint8_t RecLen;             //frame length(including address and CRC)
	 uint8_t RecData[MAXRECLEN]; //receive buffer 
}tModbusRecStruct;

extern void Modbus_Poll(void);

#endif
