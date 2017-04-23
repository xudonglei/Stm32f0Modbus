#ifndef _MODBUS_H
#define _MODBUS_H
#include "stm32f0xx.h"
#include "uart.h"

#define ID                  0x01           //slave ID
#define MAXRECLEN           32
#define MAXSENDLEN          128

#define READHOLDREG         0x03
#define WRITEMULTIHOLDREG   0x10

#define RTUOK           0
#define RTUERRCMD       0x01
#define RTUERRADDR      0x02
#define RTUERRVALUE     0x03
#define RTUERRWRITE     0x04

#define REG_HOLDING_NREGS 128
#define REG_INPUT_NREGS   16
typedef struct
{
	 FlagStatus RecEndFlag;      //frame receive end
	 uint8_t RecLen;             //frame length(including address and CRC)
	 uint8_t RecData[MAXRECLEN]; //receive buffer 
	 
	 uint8_t Response;
	 uint8_t SendData[MAXSENDLEN];
	 uint8_t SendLen;
}T_ModbusStruct;

typedef struct
{
		uint16_t RegHoldingBuf[REG_HOLDING_NREGS];
	  uint16_t RegInputBuf[REG_INPUT_NREGS];
}T_ModbusReg;

extern void Modbus_Poll(void);

#endif
