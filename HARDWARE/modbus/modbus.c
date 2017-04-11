#include "modbus.h"

extern tModbusRecStruct ModbusRec;  
extern uint8_t CRC_counter;
uint16_t Get_Crc16(volatile uint8_t *ptr,uint8_t len) 
{ 
    uint8_t i; 
    uint16_t crc = 0xFFFF; 
    if(len==0) 
    {
        len = 1;
    } 
    while(len--)  
    {   
        crc ^= *ptr; 
        for(i=0; i<8; i++)  
    	{ 
            if(crc&1) 
        	{ 
                crc >>= 1;  
                crc ^= 0xA001; 
        	}  
        	else 
    		  {
                crc >>= 1;
        	} 
        }         
        ptr++; 
    } 
    return(crc); 
} 

void Copy_Data(uint8_t des[],uint8_t sou[],uint8_t num)
{
    uint8_t i;
    for(i=0; i<num ; i++)
    {
       des[i] = sou[i];
    }
}

void Modbus_Function_0(void)
{
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8);
}

void Modbus_Function_1(void)
{
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9); 
}

void Modbus_Function_2(void)
{
    //add your handle code here
}

void (*modbus_list[3])() =
{	
   Modbus_Function_0,
   Modbus_Function_1,
   Modbus_Function_2
};

uint8_t Uart0_send_buff[255]; 
void Modbus_Poll(void) 
{  
	  uint16_t crcresult=0; 
	  uint8_t  temp[2]; 
	  if(ModbusRec.RecEndFlag == SET)//Receive end then handle,otherwise quit
      { 
            if(ModbusRec.RecData[0] == ID)
			{
				crcresult = Get_Crc16(ModbusRec.RecData,CRC_counter-2); 
				temp[1] = crcresult & 0xff; 
				temp[0] = (crcresult >> 8) & 0xff;
				if((ModbusRec.RecData[CRC_counter-1] == temp[0])&&(ModbusRec.RecData[CRC_counter-2] == temp[1]))
				{
					if(ModbusRec.RecData[1]<=0x02)  
					{				
						(*modbus_list[ModbusRec.RecData[1]])();							   
					}	
				}
				else
				{
					//CRC fail
				}
			}
			ModbusRec.RecEndFlag = RESET;
	  }
}
